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
#include <pthreads.h>
#include "city.h"

typedef struct CITY {
    size_t highest; /// the highest hit in the city
    size_t lowest; /// the lowest point in the city
    unsigned int hits; /// the amount of missles that hit
    unsigned char* 
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
unsigned int init_city(FILE *city, size_t, screenWidth, size_t screenHeight) {
    assert(city != NULL);
    assert(screenWidth > 0 && screenHeight > 0);

    width = screenWidth;
    height = screenHeight;

    city = malloc(sizeof(struct CITY));
    assert(city != NULL);

    

    return 1;
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
    if(city != NULL)
        free(city);
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


