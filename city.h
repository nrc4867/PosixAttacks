/// File: city.h
/// Description: contains functions for city interaction
/// Author: Nicholas Chieppa <nrc4867>
/// 

#include <stdio.h>

#ifndef CITY
#define CITY

/// objects to drawn on screen in ncurses
#define WALL '|' /// the wall of a building
#define FLOOR '_' /// the ground or top of building
#define MISSLE '|' /// the missle that falls from the sky
#define EXPLODED '?' /// indicator for missle position before explosion
#define HIT '*' /// the explosion indicator
#define PLATFORM '#' /// the 'sheild' platfrom the player controls

#define PLATFORM_SIZE 5 /// the size of the player sheild that is drawn

/// game properties

/// delay between thread cycles, measured with usleep
#define MISSLE_SPEED 1000 /// the speed the missles drop at
#define PLATFORM_SPEED 1000 /// the speed the platform moves at

#define MAX_MISSLES 10 /// max amount of missle threads spawned by attack()

/// city file disciptors
#define INFINITE_MISSLE_BATTLE 0 /// flag for an infinte missle battle
#define COMMENT_CHARACTER '#' /// line character to indicate a comment
#define ASSUME_FLOOR 2 /// floor height assumtion for any missing spaces

/**
 * init_city()
 *      create and draw the city to ncurses 
 *      init_city does not call refresh
 * pre - 
 *      ncurses has been initialized
 * args - 
 *      city - pointer to a city description file
 *      screenWidth - the terminal width as determined by ncurses (> 0)
 *      screenHeight - the terminal height as determined by ncurses (> 0)
 * returns - 
 *      0 - the city has been successfully drawn
 *      1 - the city descriptor file cannot be read
 *      2 - missing defender name
 *      3 - missing attacker name
 *      4 - missing missle count
 *      5 - missing city layout
 * post -
 *      the city has been drawn and is waiting on refresh by ncurses
 */
unsigned int init_city(FILE *city, size_t screenWidth, size_t screenHeight);

/**
 * destroy_city()
 *      free any memory created by init_city()
 * pre -
 *      init_city() has been called
 */
void destroy_city();

/**
 * attack_t()
 *      start the attack. Each interation of the loop will create
 *      a random amount of missles until the missle count is 0 or the
 *      attack has been ended by the user.
 *      The loop will interate each time there are no remaining 
 *      missles on screen. 
 * pre - 
 *      city has been initalized with screen width/height != 0.
 * returns -
 *      NULL
 */
void *attack_t(void);

/**
 * defense_t()
 *      start the defense. Accept user input (-> or <- keys) to move the
 *      sheild platform over the city. 
 * returns -
 *      NULL
 */
void *defense_t(void);

#endif /// city.h
