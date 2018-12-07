/// File: city.c
/// Description: Implementation of city.h
/// Author: Nicholas Chieppa <nrc4867>
///

#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <ncurses.h>
#include <pthread.h>
#include <limits.h>
#include <time.h>
#include "city.h"

typedef struct CITY {
    size_t highest; /// the highest hit in the city
    size_t lowest; /// the lowest point in the city
    unsigned int hits; /// the amount of missiles that hit
    char* attacker; /// city attacker name
    char* defender; /// city defender name
} * City;

typedef struct PLATFORM {
    unsigned int column; /// the position of the center of the platform
    unsigned int row; /// the row of the platform on the screen
} Platform;

typedef struct MISSILE {
    unsigned int row; /// the current row on the screen
    unsigned int column; /// the column the missile falls on
} Missle;


static int attack = 1; /// indicate if the attack is still continuing
static long missiles = 0; /// amount of missiles that remain (if < 0: infinite)

static long missiles_s = 0;

static size_t width = 0; /// width of the screen determined by ncurses
static size_t height = 0; /// height of the screen determined by ncurses

static City city = NULL; /// basic city stats

/// Thread lock used for moving cursor and writing to the screen
static pthread_mutex_t DRAWING_ML = PTHREAD_MUTEX_INITIALIZER;
/// Thread lock used for keeping the defense from ending 
static pthread_mutex_t ATTACKING_ML = PTHREAD_MUTEX_INITIALIZER;

#define MAX(x, y) ((x) > (y))?(x):(y)
#define MIN(x, y) ((x) > (y))?(y):(x)

/**
 * start_curses()
 *      starts curses
 */
static void start_curses() {
    initscr();
    noecho();
    nodelay(stdscr, true);
    cbreak();
    width = getmaxx(stdscr);
    height = getmaxy(stdscr);
}


/**
 * is_digit()
 *      checks to ensure that a c string is a base 10 digit
 * args - 
 *      string - a string to check
 * returns - 
 *      1 - if string is a digit, 0 otherwise
 */
unsigned int is_digit(char* string) {
    for(int i = 0; string[i] != '\0'; i++) {
        if(!(string[i] >= '0' || string[i] <= '9'))
            return 0;
    }
    return 1;
}

/**
 * draw_wall()
 *      draw a building wall in the city on the virtual stdscr
 * args -
 *      x          - x offset to move the cursor to
 *      currHeight - the height of the wall
 */
static void draw_wall(int x, unsigned long currHeight) {
    for(unsigned long i = city->lowest; i < currHeight; i++) {
        mvprintw(height - i, x, "%c", WALL_C);
    }
}

/**
 * draw_city()
 *      draw part of the city to the virtual stdscr
 * args - 
 *      buffer - character buffer of ints seperated by spaces
 *               theses values will be used as heights to build
 *               the city
 *      off    - pointer to an integer offset for the x-coord to
 *               start drawing at
 *      lastHeight - the height of the last space read
 * post -
 *      the city is partially drawn to the virtual stdscr.
 *      offset and lastHeight changed to the last position drawn
 */
static void draw_city(const char* buffer, int* off, unsigned long* lastHeight) {
    char* copy = strdup(buffer);
    char* token = strtok(copy, " \n");
    while(token != NULL) {
        if(!is_digit(token)) continue;
        unsigned long currHeight = strtol(token, NULL, 10);
        
        city->highest = MAX(city->highest, currHeight);

        // draw city wall when there is a height difference
        if(currHeight > *lastHeight) {
           draw_wall(*off, currHeight); 
        } else if (currHeight < *lastHeight) {
            mvprintw(height - currHeight, *off, "%c", FLOOR_C);
            mvprintw(height - *lastHeight, *off -1, " ");
            draw_wall(*off - 1, *lastHeight);
        } else { // draw ground
            mvprintw(height - currHeight, *off, "%c", FLOOR_C);
            refresh(); 
        }

        *lastHeight = currHeight;
        *off = *off + 1;
        token = strtok(NULL, " ");
    }
    free(copy);
}

/**
 * read_uncommented()
 *      reads stream until a line that doesn't start with COMMENT_CHARACTER
 *      is found, buffer if set to the resulting stream. 
 *      if COMMENT_CHARACTER is found in the stream it is replaced with '\0'.
 * args - 
 *      buffer - character buffer set dynamically by call to getline
 *      len - size of the character buffer
 *      stream - file stream to read
 * post -
 *      buffer and len are updated to the new stream string. 
 *      buffer length may equal 0.
 */
static void read_uncommented(char** buffer, size_t* len, FILE* stream) {
    if(*buffer != NULL) (*buffer)[0] = '\0';
    // get a line that does not start with  COMMENT_CHARACTER
    while(getline(buffer, len, stream) != -1 && 
                ((*buffer)[0] == COMMENT_CHARACTER ||
                (*buffer)[0] == '\n'));

    // remove parts of the string trailing the COMMENT_CHARACTER
    for(int i = 0; (*buffer)[i] != '\0'; i++)
        if((*buffer)[i] == COMMENT_CHARACTER) {
            (*buffer)[i] = '\0';
            break;
        }
}

/**
 * init_city()
 *      creates a city as described in city.h
 * pre - 
 *      see city.h
 * returns - 
 *      see city.h
 * post -
 *      see city.h 
 *      city global variable created and set by function
 */
unsigned int init_city(FILE *cityFile) {
    assert(cityFile != NULL);

    city = (City)malloc(sizeof(struct CITY));
    assert(city != NULL);

    srand(time(NULL));

    int status = 0;
    char* buffer = NULL; // file read buffer
    size_t len = -1; // file line length 
    // read defender 
    read_uncommented(&buffer, &len, cityFile);
    buffer[strlen(buffer) -1] = '\0';
    if(strlen(buffer) > 80) buffer[79] = '\0';
    if(!strlen(buffer)) {
        status = 2; // no defender
        goto cleanup;
    }
    city->defender = strdup(buffer);
    
    // read attacker
    read_uncommented(&buffer, &len, cityFile);
    buffer[strlen(buffer) -1] = '\0';
    if(strlen(buffer) > 80) buffer[79] = '\0';
    if(!strlen(buffer)) {
        status = 3; // no attacker
        goto cleanup;
    }
    city->attacker = strdup(buffer);
    
    // read missile count
    read_uncommented(&buffer, &len, cityFile);
    int temp;
    if(sscanf(buffer, "%li %i", &missiles, &temp) != 1) {
        status = 4; // no missiles
        goto cleanup;
    }

    if(missiles == UNLIMITED_MISSILES) missiles = -1; // unlimited missiles    
    // read and create cityscape
    city->lowest = ASSUME_FLOOR;
    city->highest = ASSUME_FLOOR;   
 
    read_uncommented(&buffer, &len, cityFile);
    if(sscanf(buffer, "%i", &temp) != 1) {
        status = 5; // no city
        goto cleanup;
    }
    
    start_curses(); 

    // Begin drawing the city by reading the file line by line
    int offset = 0;
    unsigned long lastHeight = temp;
    do {
        draw_city(buffer, &offset, &lastHeight);
        read_uncommented(&buffer, &len, cityFile);
    } while(strcspn(buffer, "\n") > 0);
 
    do { // fill in the remaining area of the screen
        mvprintw(height - ASSUME_FLOOR, offset, "%c", FLOOR_C);
        refresh();
    } while(offset++ < (int)width);

    cleanup: // stop execution
    free(buffer);
    return status;
}

/**
 * destroy_city()
 *      frees memory allocated by init_city().
 * pre - 
 *      see city.h for details
 * post - 
 *      city global variable is set to NULL
 */
void destroy_city() {
    if(city != NULL) {
        free(city->attacker);
        free(city->defender);
        free(city);
    }
    city = NULL;
}

/**
 * missile_t()
 *      creates and controls a missile structure onscreen.
 *      Waits MISSILE_SPEED between each iteration of the
 *      threads loop. Then cleans up the created thread.
 * args - 
 *      param - Unused.
 * returns - 
 *      NULL
 */
static void *missile_t(void* param) {
    (void) param;
    Missle missile = {1, rand() % width};

    while(1) {
        pthread_mutex_lock(&DRAWING_ML);
        
        char ch = mvinch(missile.row, missile.column) & A_CHARTEXT;
        if(ch == HIT_C) { // another missile has just exploded here
            mvprintw(missile.row, missile.column, " ");
            break;
        }

        // remove the old position of the missile from the display
        mvprintw(missile.row++, missile.column, " ");

        // get the character on the current line of the missile
        ch = mvinch(missile.row, missile.column) & A_CHARTEXT;

        // dont allow the missile to completely destroy the sheild
        if(ch == HIT_C && height - missile.row > city->highest)
            break; 

        // decide where to draw the missile based 
        // on the character already in its new position
        if(ch == ' ' || ch == EXPLODED_C) {
            mvprintw(missile.row, missile.column, "%c", MISSILE_C);
        } else if(ch != FLOOR_C && ch != HIT_C && height - missile.row != city->lowest) {
            mvprintw(missile.row - 1, missile.column, "%c", EXPLODED_C);
            mvprintw(missile.row, missile.column, "%c", HIT_C); 
            break;
        } else {
            mvprintw(missile.row, missile.column, "%c", EXPLODED_C);
            mvprintw(missile.row + 1, missile.column, "%c", HIT_C); 
            break; 
        } 
        
        // explode on the bottom of the screen
        if(height - missile.row <= city->lowest) {
            mvprintw(missile.row, missile.column, "%c", EXPLODED_C);
            mvprintw(missile.row + 1, missile.column, "%c", HIT_C); 
            break;
        }

        refresh();
        pthread_mutex_unlock(&DRAWING_ML);
        usleep((rand() % MISSILE_SPEED) * 1000);
    }
    refresh(); // redraw the final position of the missile
    pthread_mutex_unlock(&DRAWING_ML); // make sure to give up lock!

    return NULL;
}

/**
 * attack_t()
 *      see city.h for details
 */
void *attack_t(void* param) {
    (void) param;
    assert(city != NULL);

    pthread_mutex_lock(&ATTACKING_ML);

    while(attack && missiles != 0) {
        int maxMissles = (missiles > 0)?MIN(missiles, MAX_MISSILES):MAX_MISSILES;
        if(missiles > 0) missiles -= maxMissles;

        // create and join a round of missile threads
        pthread_t* missileThreads = malloc(sizeof(pthread_t) * maxMissles);

        for(int i = 0; i < maxMissles; i++) {
            pthread_create(&missileThreads[i], NULL, missile_t, NULL);
            missiles_s ++;
            // spread out the missile creation
            if(attack) usleep(rand() % MISSILE_SPEED * 1000);
        }
        for(int i = 0; i < maxMissles; i++) {
            pthread_join(missileThreads[i], NULL);
            missiles_s --;
        }

        free(missileThreads);
    }

    pthread_mutex_lock(&DRAWING_ML);
    mvprintw(3, 0, "The %s attack has ended.", city->attacker);
    pthread_mutex_unlock(&DRAWING_ML);
    pthread_mutex_unlock(&ATTACKING_ML);

    return NULL;
}


/**
 * drawShield()
 *      draw the shield over the city
 * args -
 *      platform - structure representing the shield.
 *                 the row that the shield is on will be
 *                 cleared on redraw.
 */
static void drawShield(const Platform* platform) {
    pthread_mutex_lock(&DRAWING_ML);
    move(height - platform->row, 0);
    clrtoeol();
    for(int i = 0; i < PLATFORM_SIZE; i++) {
        mvprintw(height - platform->row, 
                    platform->column + i, "%c", SHEILD_C);
    }
    move(height - platform->row, platform->column + PLATFORM_SIZE / 2);
    pthread_mutex_unlock(&DRAWING_ML);
}

/**
 * defense_t()
 *      see city.h for details
 * post - 
 *      attack global variable is set to 0
 */
void *defense_t(void* param) {
    (void) param;
    assert(city != NULL);

    pthread_mutex_lock(&DRAWING_ML);
    mvprintw(0 ,0, "Enter 'q' to quit at end of attack, " 
                        "or control-c");
    pthread_mutex_unlock(&DRAWING_ML);

    Platform platform = {width / 2, city->highest + 1};
    char input = ' ';
    while(attack || missiles_s) {
        if(input == LEFT_M && platform.column != 0) {
            platform.column--;
        } else if(input == RIGHT_M && 
                        platform.column + PLATFORM_SIZE != width) {
            platform.column++;
        } else if(input == QUIT_M) {
            attack = 0;
        }
        if(input != -1) // there was a key pressed
            drawShield(&platform);
        // add a short time delay between input to smooth movement
        usleep(1000 * 2);
        // get the players next input
        pthread_mutex_lock(&DRAWING_ML);
        input = getch();
        pthread_mutex_unlock(&DRAWING_ML);
    }

    pthread_mutex_lock(&ATTACKING_ML);
    pthread_mutex_lock(&DRAWING_ML);
    mvprintw(6, 0, "The %s defense has ended.\nHit enter to close...", 
                        city->defender);
    pthread_mutex_unlock(&DRAWING_ML);
    pthread_mutex_unlock(&ATTACKING_ML);

    return NULL;
}

