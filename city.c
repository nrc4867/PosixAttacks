/// File: city.c
/// Description: Implementation of city.h
/// Author: Nicholas Chieppa <nrc4867>
///

#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <pthreads.h>
#include "city.h"

typedef struct CITY {
    size_t highest; /// the highest hit in the city
    size_t lowest; /// the lowest point in the city
    unsigned int hits; /// the amount of missles that hit
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

static size_t screenWidth = 0; /// width of the screen determined by ncurses
static size_t screenHeight = 0; /// height of the screen determined by ncurses

/**
 * creates a city in ncurses
 * see city.h for more details
 */
unsigned int init_city(FILE *city, size_t, screenWidth, size_t screenHeight) {

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

}

void *attack_t(void) {
    // spawn random amount of missles 
    // wait for them to fall with thread_join
    // cleanup missles
    // repeat

}

void *defense_t(void) {

}


