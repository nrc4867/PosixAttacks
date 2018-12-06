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
#include "city.h"

typedef struct CITY {
    size_t highest; /// the highest hit in the city
    size_t lowest; /// the lowest point in the city
    unsigned int hits; /// the amount of missles that hit
    char* attacker; /// city attacker name
    char* defender; /// city defender name
} * City;

typedef struct PLATFORM {
    unsigned int column; /// the poisition of the center of the plaform
    unsigned int row; /// the row of the platform on the screen
} Platform;

typedef struct MISSLE {
    unsigned int row; /// the current row on the screen
    unsigned int column; /// the column the missle falls on
} Missle;


static int attack = 1; /// indicate if the attack is still continuing
static long missles = 0; /// amount of missles that remain (if < 0: infinite)

static long missles_s = 0;

static size_t width = 0; /// width of the screen determined by ncurses
static size_t height = 0; /// height of the screen determined by ncurses

static City city = NULL; /// basic city statitics 

/// Thread lock used for moving cursor and writing to the screen
static pthread_mutex_t DRAWING_ML = PTHREAD_MUTEX_INITIALIZER;
/// Thread lock used for keeping the defense from ending 
static pthread_mutex_t ATTACKING_ML = PTHREAD_MUTEX_INITIALIZER;

#define MAX(x, y) ((x) > (y))?(x):(y)
#define MIN(x, y) ((x) > (y))?(y):(x)

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
unsigned int init_city(FILE *cityFile, 
                            int screenWidth, int screenHeight) {
    assert(cityFile != NULL);
    assert(screenWidth > 0 && screenHeight > 0);
 
    width = screenWidth;
    height = screenHeight;

    city = (City)malloc(sizeof(struct CITY));
    assert(city != NULL);

    int status = 0;
    char* buffer = NULL; // file read buffer
    size_t len = -1; // file line length 
    // read defender 
    read_uncommented(&buffer, &len, cityFile);
    buffer[strlen(buffer) -1] = '\0';
    if(!strlen(buffer)) {
        status = 2; // no defender
        goto cleanup;
    }
    city->defender = strdup(buffer);
    
    // read attacker
    read_uncommented(&buffer, &len, cityFile);
    buffer[strlen(buffer) -1] = '\0';
    if(!strlen(buffer)) {
        status = 3; // no attacker
        goto cleanup;
    }
    city->attacker = strdup(buffer);
    
    // read missle count
    read_uncommented(&buffer, &len, cityFile);
    int temp;
    if(sscanf(buffer, "%li %i", &missles, &temp) != 1) {
        status = 4; // no missles
        goto cleanup;
    }

    if(missles == UNLIMITED_MISSLES) missles = -1; // unlimited missles    
    // read and create cityscape
    city->lowest = ASSUME_FLOOR;
    city->highest = ASSUME_FLOOR;   
 
    read_uncommented(&buffer, &len, cityFile);
    if(sscanf(buffer, "%i", &temp) != 1) {
        status = 5; // no city
        goto cleanup;
    }
 
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
 * missle_t()
 *      creates and controls a missle structure onscreen.
 *      Waits MISSLE_SPEED between each iteration of the
 *      threads loop. Then cleans up the created thread.
 * args - 
 *      param - Unused.
 * returns - 
 *      NULL
 */
static void *missle_t(void* param) {
    (void) param;
    Missle missle = {1, rand() % width};

    while(1) {
        pthread_mutex_lock(&DRAWING_ML);
        
        // remove the old position of the missle from the display
        mvprintw(missle.row++, missle.column, " ");

        // get the character on the current line of the missle
        char ch = mvinch(missle.row, missle.column) & A_CHARTEXT;

        // dont allow the missle to completely destroy the sheild
        if(ch == HIT_C && height - missle.row > city->highest)
            break; 

        // decide where to draw the missle based 
        // on the character already in its new position
        if(ch == ' ' || ch == EXPLODED_C) {
            mvprintw(missle.row, missle.column, "%c", MISSLE_C);
        } else if(ch != FLOOR_C && ch != HIT_C && height - missle.row != city->lowest) {
            mvprintw(missle.row - 1, missle.column, "%c", EXPLODED_C);
            mvprintw(missle.row, missle.column, "%c", HIT_C); 
            break;
        } else {
            mvprintw(missle.row, missle.column, "%c", EXPLODED_C);
            mvprintw(missle.row + 1, missle.column, "%c", HIT_C); 
            break; 
        } 
        
        // explode on the bottom of the screen
        if(height - missle.row <= city->lowest) {
            mvprintw(missle.row, missle.column, "%c", EXPLODED_C);
            mvprintw(missle.row + 1, missle.column, "%c", HIT_C); 
            break;
        }

        refresh();
        pthread_mutex_unlock(&DRAWING_ML);
        usleep((rand() % MISSLE_SPEED) * 1000);
    }
    refresh(); // redraw the final position of the missle
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

    while(attack && missles != 0) {
        int maxMissles = (missles > 0)?MIN(missles, MAX_MISSLES):MAX_MISSLES;
        if(missles > 0) missles -= maxMissles;

        // create and join a round of missle threads
        pthread_t* missleThreads = malloc(sizeof(pthread_t) * maxMissles);

        for(int i = 0; i < maxMissles; i++) {
            pthread_create(&missleThreads[i], NULL, missle_t, NULL);
            missles_s ++;
            // spread out the missle creation
            if(attack) usleep(rand() % MISSLE_SPEED * 1000);
        }
        for(int i = 0; i < maxMissles; i++) {
            pthread_join(missleThreads[i], NULL);
            missles_s --;
        }

        free(missleThreads);
    }

    pthread_mutex_lock(&DRAWING_ML);
    mvprintw(3, 0, "The %s attack has ended.", city->attacker);
    pthread_mutex_unlock(&DRAWING_ML);
    pthread_mutex_unlock(&ATTACKING_ML);

    return NULL;
}


/**
 * drawSheild()
 *      draw the sheild over the city
 * args -
 *      platform - structure representing the sheild.
 *                 the row that the sheild is on will be
 *                 cleared on redraw.
 */
static void drawSheild(const Platform* platform) {
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
    while(attack || missles_s) {
        if(input == LEFT_M && platform.column != 0) {
            platform.column--;
        } else if(input == RIGHT_M && 
                        platform.column + PLATFORM_SIZE != width) {
            platform.column++;
        } else if(input == QUIT_M) {
            attack = 0;
        }
        if(input != -1) // there was a key pressed
            drawSheild(&platform);
        // add a short time delay between input to smooth movement
        usleep(1000 * 2);
        // get the players next input
        pthread_mutex_lock(&DRAWING_ML);
        input = getch();
        pthread_mutex_unlock(&DRAWING_ML);
    }

    pthread_mutex_lock(&ATTACKING_ML);
    pthread_mutex_lock(&DRAWING_ML);
    mvprintw(6, 0, "The %s defense has ended.", city->defender);
    pthread_mutex_unlock(&DRAWING_ML);
    pthread_mutex_unlock(&ATTACKING_ML);

    return NULL;
}

