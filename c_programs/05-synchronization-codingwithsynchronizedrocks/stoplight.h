/*
 * Samantha Foley
 *
 * CS 441/541 : Synchronization Project
 */
#include "support.h"


/*****************************
 * Defines
 *****************************/


/*****************************
 * Structures
 *****************************/

/*****************************
 * Global Variables
 *****************************/

double minTime = 100, maxTime = 0, totalTime = 0;
/*
 * Time to live (Seconds)
 */
int ttl = 0;

/*
 * Number of cars (threads) in the system
 */
int num_cars = 0;

/**
 * Count for average
 */
int num_iterations = 0;
/*
 * Indicate when for threads to stop processing and exit
 */
int time_to_exit = FALSE;
/*
 * Tracks what lanes are available for test() - think of as the 5 states in tannenbaum -
 * Indexed to direction enum - North West South East
 */
int lane_states[] = {1,1,1,1};
/* Holds what cars are in the intersection */
car_t nextIn[4];
/*
 * 4 stop signs, allows 1 from each direction to enter
 */
semaphore_t stop_sign[4];

/*
 * Mutex used to protected critical sections
 */

semaphore_t mutex;
semaphore_t leave;
semaphore_t iteration_mut;
semaphore_t callOthers;


/*****************************
 * Function Declarations
 *****************************/

/*
 * Parse command line arguments
 */
int parse_args(int argc, char **argv);

/*
 * Main thread function that picks an arbitrary direction to approach from,
 * and to travel to for each car.
 *
 * Write and comment this function
 *
 * Arguments:
 *   param = The car ID number for printing purposes
 *
 * Returns:
 *   NULL
 */
void *start_car(void *param);

/**
 * Logic behind the intersection
 *  Logic follows as such:
 *      model as test() in dining philosophers
 *      check to see if thread appr dir has the required tiles to proceed 
 *      to its end dir through the intersection. 
 *      upon entering funct, check semaphores
 * I think i may need 4 semaphores for each direction, built upon the 4 tiles of the intersection.
 */
int simulate_intersection(car_t*);

int test(car_t*, int*);

int initalize_semaphores();

int initalize_path(car_t*);

void traverse_intersection(car_t*); 

void traverse_helper(car_t*, car_state_t);

void leave_intersect(car_t *); 

void printState(car_t *);