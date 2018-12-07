/// file: threads.c
/// description: main file for project3 Posix Attacks
/// author: Nicholas Chieppa <nrc4867>
///

#define _DEFAULT_SOURCE 

#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include "city.h"

#define INTERRUPT 2 /// code for kill signal from ctrl-c

/**
 * stop()
 *      closes the program
 * args -
 *      EXIT - program return code
 */
void stop(int EXIT) {
    destroy_city();
    endwin();
    exit(EXIT);
}

/**
 * interrupt_h()
 *      used to handle signal interrupts
 * args -
 *      sig - unused
 */
void interrupt_h(int sig) {
    (void) sig;
    stop(0);
}


/**
 * usage()
 *      prints program usage information
 * args -
 *      the program executable path
 */
void usage(char* path) {
    fprintf(stderr, "Usage: %s game-file\n", path);
    stop(EXIT_FAILURE);
}

/**
 * city_fail()
 *      prints error message if the city fails to draw, then
 *      closes the program
 * args -
 *      id - the failure code
 */
void city_fail(int id) {
    if (!id) return;
    char* errors[] = {
        "city drawn successfully",
        "file cannot be read",
        "missing defender name",
        "missing attacker name",
        "missing missile specification",
        "missing city layout",
    };
    fprintf(stderr, "Error: %s.\n", errors[id % 6]);
   
    stop(EXIT_FAILURE); 
}

/**
 * main()
 *      main program thread
 * args - 
 *      agrc - the amount of arguments passed to the program
 *      argv - see usage information
 */
int main(int argc, char* argv[]) {
    if(argc == 1) {
        usage(argv[0]);
    }    

    FILE* f = fopen(argv[1], "r");
    if(f == NULL) {
        usage(argv[0]);
    } 
   
    city_fail(init_city(f)); // initialize city

    signal(INTERRUPT, interrupt_h);

    fclose(f);

    pthread_t attack, defense; // start missiles and defense threads
    pthread_create(&attack, NULL, attack_t, NULL);
    pthread_create(&defense, NULL, defense_t, NULL);
    pthread_join(attack, NULL);
    pthread_join(defense, NULL);

    nodelay(stdscr, false);
    getch(); // pause screen one last time

    stop(EXIT_SUCCESS);
    return EXIT_FAILURE;
}
