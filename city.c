/// File: city.c
/// Description: Implementation of city.h
/// Author: Nicholas Chieppa <nrc4867>
///

#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ncurses.h>
#include <pthread.h>
#include "city.h"

typedef struct CITY {
    size_t highest; /// the highest hit in the city
    size_t lowest; /// the lowest point in the city
    unsigned int hits; /// the amount of missles that hit
    char* attacker; /// city attacker name
    char* defender; /// city defender name
} * City;

typedef struct PLATFORM {
    unsigned int center; /// the poisition of the center of the plaform
    unsigned int row; /// the row of the platform on the screen
} * Platform;

typedef struct MISSLE {
    unsigned int row; /// the current row on the screen
    unsigned int column; /// the column the missle falls on
    unsigned int wait; /// the wait time after the start of the game
} * Missle;


static int attack = 1; /// indicate if the attack is still continuing
static long missles = 0; /// amount of missles that remain (if < 0: infinite)

static size_t width = 0; /// width of the screen determined by ncurses
static size_t height = 0; /// height of the screen determined by ncurses

static City city = NULL; /// basic city statitics 

#define P_INT(x) printf("%s: %i\n", (#x), (int)(x))
#define P_STR(x) printf("%s: %s\n", (#x), (x))

#ifdef DEBUG

/**
 * dump_variables()
 *      prints global variable values
 * pre -
 *      init_city has been called
 */
static void dump_variables() {
    assert(city != NULL);

    P_INT(attack);
    P_INT(missles);
    P_INT(width);
    P_INT(height);

    printf("city {\n");
    P_INT(city->highest);
    P_INT(city->lowest);
    P_INT(city->hits);
    P_STR(city->attacker);
    P_STR(city->defender);
    printf("}\n");

}
 
#endif

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
        if((string[i] >= '0' || string[i] <= '9'))
            return 0;
    }
    return 1;
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
    // get a line that does not start with  COMMENT_CHARACTER
    while(getline(buffer, len, stream) != -1 && 
                (*buffer)[0] == COMMENT_CHARACTER);

    // remove parts of the string trailing the COMMENT_CHARACTER
    for(int i = 0; (*buffer)[i] != '\0'; i++)
        if(*(buffer)[i] == COMMENT_CHARACTER) {
            *(buffer)[i] = '\0';
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
                            size_t screenWidth, size_t screenHeight) {
    assert(cityFile != NULL);
    assert(screenWidth > 0 && screenHeight > 0);

    width = screenWidth;
    height = screenHeight;

    city = (City)malloc(sizeof(struct CITY));
    assert(city != NULL);

    char* buffer = NULL; // file read buffer
    size_t len = 0; // file line length 
    
    read_uncommented(&buffer, &len, cityFile);
    if(!strlen(buffer)) return 2; // no defender
    city->defender = strdup(buffer);

    read_uncommented(&buffer, &len, cityFile);
    if(!strlen(buffer)) return 3; // no attacker
    city->attacker = strdup(buffer);

    read_uncommented(&buffer, &len, cityFile);
    sscanf(buffer, "%li", &missles);
    int temp;
    if(sscanf(buffer, "%i", &temp) != 0) return 4; // no missles
    
    do {
        read_uncommented(&buffer, &len, cityFile);
        printf("%s", buffer);
    } while(strlen(buffer) > 0);

    #ifdef DEBUG 
        dump_variables();
    #endif    

    free(buffer);
    return 0;
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
 * createMissle()
 *      create a randomly placed missle structure
 * pre -
 *      city has already been initialized with 
 *      screen width/height != 0
 * returns - 
 *      a missle structure
 */
static Missle create_missle() {
    return NULL;
}

/**
 * destroy_missle()
 *      cleanup a missle structure
 * pre - 
 *      missle is not NULL
 * args - 
 *     missle - the missle to cleanup 
 * post - 
 *      missle is set equal to NULL
 */
static void destroy_missle(Missle missle) {
    assert(missle != NULL);

    missle = NULL;
}

/**
 * missle_t()
 *      creates and controls a missle structure onscreen.
 *      Waits MISSLE_SPEED between each iteration of the
 *      threads loop. Then cleans up the created thread.
 * returns - 
 *      NULL
 */
static void *missle_t(void) {

    return NULL;
}

void *attack_t(void) {
    assert(city != NULL);
    // spawn random amount of missles 
    // wait for them to fall with thread_join
    // cleanup missles
    // repeat
    return NULL;
}

void *defense_t(void) {
    assert(city != NULL);

    return NULL;
}


