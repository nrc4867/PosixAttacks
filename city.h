/// File: city.h
/// Description: contains functions for city interaction
/// Author: Nicholas Chieppa <nrc4867>
/// 

#include <stdio.h>

#ifndef CITY_H
#define CITY_H

/// objects to drawn on screen in ncurses
#define WALL_C '|' /// wall of a building
#define FLOOR_C '_' /// ground or top of building
#define MISSILE_C '|' /// missile that falls from the sky
#define EXPLODED_C '?' /// indicator for missile position before explosion
#define HIT_C '*' /// explosion indicator
#define SHEILD_C '#' /// shield platform the player controls

/// game properties
#define PLATFORM_SIZE 5 /// the size of the player shield that is drawn
#define QUIT_M 'q' /// Button used for when the player wants to quit
#define LEFT_M 68 /// button used for the player to move left
#define RIGHT_M 67 /// button used for the player to move right

/// delay between thread cycles, measured with usleep
#define MISSILE_SPEED 500 /// the speed the missiles drop at
#define PLATFORM_SPEED 20 /// the speed the platform moves at

#define MAX_MISSILES 8 /// max amount of missile threads spawned by attack()

/// city file descriptors
#define UNLIMITED_MISSILES 0 /// flag for an infinite missile battle
#define COMMENT_CHARACTER '#' /// line character to indicate a comment
#define ASSUME_FLOOR 2 /// floor height for any missing spaces

/**
 * init_city()
 *      create and draw the city to ncurses 
 *      init_city does not call refresh
 * args - 
 *      city - pointer to a city description file
 * returns - 
 *      0 - the city has been successfully drawn
 *      1 - the city descriptor file cannot be read
 *      2 - missing defender name
 *      3 - missing attacker name
 *      4 - missing missile count
 *      5 - missing city layout
 * post -
 *      ncurses will be initialized
 *      the city has been drawn and is waiting on refresh by ncurses
 */
unsigned int init_city(FILE *city);

/**
 * destroy_city()
 *      free any memory created by init_city()
 * pre -
 *      init_city() has been called
 */
void destroy_city(void);

/**
 * attack_t()
 *      start the attack. Each iteration of the loop will create
 *      a random amount of missiles until the missile count is 0 or the
 *      attack has been ended by the user.  
 * pre - 
 *      city has been initialized with screen width/height != 0.
 * returns -
 *      NULL
 */
void *attack_t(void* param);

/**
 * defense_t()
 *      start the defense. Accept user input (-> or <- keys) to move the
 *      sheild platform over the city. runs until the player ends the attack.
 * pre - 
 *      ncurses is in nodealy mode on stdscr
 * returns -
 *      NULL
 */
void *defense_t(void* parma);

#endif /// city.h
