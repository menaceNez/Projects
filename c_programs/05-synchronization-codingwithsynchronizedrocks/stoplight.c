/*
 * Gavin Stankovsky
 *
 * CS 441/541 : Synchronization Project
 *
 */
#include "stoplight.h"

int main(int argc, char *argv[])
{
    int ret, threadRet, i;

    /*
     * Parse Command Line arguments
     */
    if (0 != (ret = parse_args(argc, argv)))
    {
        fprintf(stderr, "Failed to parse arguments\n");
        return -1;
    }

    /*
     * Initialize:
     * - random number generator
     */
    srand(time(NULL));
    /*
     * Create Car Thread(s)
     */
    pthread_t threadArr[num_cars];

    initalize_semaphores();

    print_header();

    for (i = 0; i <= num_cars; i++)
    {
        threadRet = pthread_create(&threadArr[i], NULL, start_car, (void *)(intptr_t)i);

        if (threadRet != 0)
        {
            fprintf(stderr, "Error creating thread %d\n", threadRet);
            exit(1);
        }
    }
    /*
     * Wait for the TTL to expire
     */
    // sleep specified amount of time
    // set time_to_exit to TRUE
    sleep(ttl);
    time_to_exit = TRUE;
    /*
     * Reap threads
     */
    for (i = 0; i < num_cars; i++)
    {
        threadRet = pthread_join(threadArr[i], NULL);
        if (threadRet != 0)
        {
            fprintf(stderr, "Error joining threads\n");
            exit(1);
        }
    }
    /*
     * Print timing information
     */
    print_footer();
    printf("Min.  Time :  %f msec\n", minTime);
    printf("Avg.  Time :  %f msec\n", (totalTime) / num_iterations);
    printf("Max.  Time :  %f msec\n", maxTime);
    printf("Total Time :  %f msec\n", totalTime);
    /*
     * Cleanup
     *
     */
    print_footer();
    for (i = 0; i < 4; i++)
    {
        semaphore_destroy(&stop_sign[i]);
    }
    semaphore_destroy(&mutex);
    semaphore_destroy(&iteration_mut);
    semaphore_destroy(&leave);
    semaphore_destroy(&callOthers);
    /*
     * Finalize support library
     */
    support_finalize();

    return 0;
}

int parse_args(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s time_to_run number_cars\n", argv[0]);
        return 1;
    }

    ttl = atoi(argv[1]);
    num_cars = atoi(argv[2]);
    

    if (ttl <= 0 || num_cars <= 0)
    {
        fprintf(stderr, "Need to have both cars and time greater than 0!\n");
        return 1;
    }
    /* Print initalized header */
    printf("--------------------\n");
    printf("Time to live : %s\n", argv[1]);
    printf("Number of Cars: %s\n", argv[2]);
    printf("--------------------\n");

    /*
     * Initialize support library
     */
    support_init();

    for (int i = 0; i < DIRMAX; i++)
    {
        nextIn[i].car_id = -1;
    }
    return 0;
}

/*
 * Approach intersection
 * param = Car Number (car_id)
 */
void *start_car(void *param)
{
    int car_id = (intptr_t)param, sem_ret = 0;
    car_t this_car;
    this_car.car_id = car_id;
    this_car.location = LOC_I1;
    this_car.isWaiting = 0;
    /*
        Initalize car semaphore:
    */
    sem_ret = semaphore_create(&this_car.cango, 0);
    if (sem_ret != 0)
    {
        fprintf(stderr, "Error cango\n");
        exit(EXIT_FAILURE);
        sem_ret = 0;
    }

    /*
     * Keep cycling through
     */
    while (time_to_exit == FALSE)
    {
        /*
         * Sleep for a bounded random amount of time before approaching the
         * intersection
         */
        this_car.state = STATE_WAITING_I1;
        usleep(random() % TIME_TO_APPROACH);

        /*
         * Setup the car's direction, where it is headed, set its state
         */
        this_car.appr_dir = get_random_direction(DIRMAX);
        this_car.dest_dir = get_random_direction(this_car.appr_dir);
        initalize_path(&this_car); // just sets up my_car.path

        /*
         * Mark start time for car
         */
        gettimeofday(&this_car.start_time, NULL);
        gettimeofday(&this_car.end_time, NULL);
        print_state(this_car, NULL);

        /*
         * Move the car in the direction and change its state accordingly
         */
        /* Wait here allowing 4 cars into intersection based on their direction ( mutex for each direction )
        index in with enum */
        simulate_intersection(&this_car);

        /*
         * Mark leave time for car
         */
        gettimeofday(&this_car.end_time, NULL);
        print_state(this_car, NULL);

        /*
         * Save statistics about the cars travel
         */
        /* Need to get average time( add up all times to get thru divide by thread count )*/
        /* need to capture min time (global var), compare each time to get thru */
        // capture one thread thru time: end - start
        struct timeval this_timeval;
        this_timeval = get_timeval_diff_as_timeval(this_car.start_time, this_car.end_time);
        double compareDiff = timeval_to_double(this_timeval) * TIME_MSEC;
        totalTime += compareDiff;

        semaphore_wait(&iteration_mut);
        if (compareDiff < minTime)
        {
            minTime = compareDiff;
        }
        if (compareDiff > maxTime)
        {
            maxTime = compareDiff;
        }
        num_iterations++;
        semaphore_post(&iteration_mut);
    }

    /*
     * All done
     */

    pthread_exit((void *)0);

    return NULL;
}
semaphore_t carsinM;
int carsin = 0;
int simulate_intersection(car_t *this_car)
{
    int can_proceed = 0, i;

    // allow 4 threads in based on their direction
    semaphore_wait(&stop_sign[this_car->appr_dir]);
    semaphore_wait(&carsinM);
    carsin++; 
    semaphore_post(&carsinM);
    /* change car state to at intersection, add car to nextIn @ its respective stop_sign*/
    this_car->state = STATE_APPROACH_I1;

    nextIn[this_car->appr_dir] = *this_car; // adds to an array of whose in intersection

    test(this_car, &can_proceed);

    if (can_proceed == 1)
    {
        traverse_intersection(this_car);
        leave_intersect(this_car);
        // call test on the other cars in array
        semaphore_wait(&callOthers);
        // printf("[%d] Calls others\n", this_car->car_id);
        for (i = 0; i < DIRMAX; i++)
        {
            // printf("[%d]Car %d inloopWaiting: %d\n", i, nextIn[i].car_id, nextIn[i].isWaiting);
            if (nextIn[i].car_id != -1 && nextIn[i].isWaiting == 1)
            {
                // printf("[%d]Car %d\n", i, nextIn[i].car_id);
                semaphore_post(&nextIn[i].cango);
            }
        }
        semaphore_post(&callOthers);
    }
    semaphore_post(&stop_sign[this_car->appr_dir]);
    semaphore_wait(&carsinM);
    carsin--;
    semaphore_post(&carsinM);
    return 0;
}

int test(car_t *this_car, int *can_proceed)
{
    printf("[carsIn]: %d\n", carsin);
    int i;
    *can_proceed = 0; // sanity check

    semaphore_wait(&mutex);
    if (this_car->state == STATE_APPROACH_I1)
    {
        for (i = 0; i < DIRMAX; i++)
        {
            /* if lane == path, we can consume even if 0, or if lane == 1 and path == 0 we can proceed */
            if ((lane_states[i] == this_car->path[i]) || (lane_states[i] == 1 && this_car->path[i] == 0))
            {
                *can_proceed = 1;
            }
            else
            {
                *can_proceed = 0;
                break;
            }
        }

        // now check if we can proceed if so decrement lane's needed
        if (*can_proceed == 1)
        {
            for (i = 0; i < DIRMAX; i++)
            {
                if (lane_states[i] == 1 && this_car->path[i] == 1)
                {
                    lane_states[i]--;
                }
            }
        }
    }
    semaphore_post(&mutex);

        /**
         * Suspected race condition part?  
         */ 
    if (*can_proceed == 0 && this_car->state == STATE_APPROACH_I1)
    {
        // we cant proceed, should wait until a thread finishes
        nextIn[this_car->appr_dir].isWaiting = 1;
        // printf("[CAR] %d isWaiting: %d\n", this_car->car_id, this_car->isWaiting);
        semaphore_wait(&this_car->cango);
        // printf("[CAR] %d isGOING\n", this_car->car_id);
        this_car->isWaiting = 0;
        test(this_car, can_proceed); // busy wait
    }

    return 0;
}
int initalize_semaphores()
{
    int sem_ret = 0, i;
    // initalize stop sign mutex's
    for (i = 0; i < 4; i++)
    {
        sem_ret = semaphore_create(&stop_sign[i], 1);
        if (sem_ret != 0)
        {
            fprintf(stderr, "Error on stop_sign\n");
            return 1;
        }
    }
    // initalize mutex used for test()
    sem_ret = semaphore_create(&mutex, 1);
    if (sem_ret != 0)
    {
        fprintf(stderr, "Error test() mutex\n");
        sem_ret = 0;
        return 1;
    }
    sem_ret = semaphore_create(&leave, 1);
    if (sem_ret != 0)
    {
        fprintf(stderr, "Error test() mutex\n");
        sem_ret = 0;
        return 1;
    }
    sem_ret = semaphore_create(&iteration_mut, 1);
    if (sem_ret != 0)
    {
        fprintf(stderr, "Error test() mutex\n");
        sem_ret = 0;
        return 1;
    }
    sem_ret = semaphore_create(&callOthers, 1);
    if (sem_ret != 0)
    {
        fprintf(stderr, "Error test() callOthers\n");
        sem_ret = 0;
        return 1;
    }
    sem_ret = semaphore_create(&carsinM, 1);
    if (sem_ret != 0)
    {
        fprintf(stderr, "Error test() mutex\n");
        sem_ret = 0;
        return 1;
    }
    return 0;
}

/* NOTE: path is represented via the enum: car_direction_t each index representing its entry tile */
/* EX: NW=NORTH1, SW=WEST, SE=SOUTH1, NE=EAST */
int initalize_path(car_t *this_car)
{
    car_direction_t apprDir = this_car->appr_dir;
    car_direction_t destDir = this_car->dest_dir;
    /* First switch is the start direction second switch is their destination */
    switch (apprDir)
    {
    case NORTH1:
        switch (destDir)
        {
        case WEST:
            /* consume NW */
            this_car->path[NORTH1] = 1;
            this_car->path[WEST] = 0;
            this_car->path[SOUTH1] = 0;
            this_car->path[EAST] = 0;
            break;
        case SOUTH1:
            /* Consume NW, SW */
            this_car->path[NORTH1] = 1;
            this_car->path[WEST] = 1;
            this_car->path[SOUTH1] = 0;
            this_car->path[EAST] = 0;
            break;
        case EAST:
            /* Consume  NW SW SE*/
            this_car->path[NORTH1] = 1;
            this_car->path[WEST] = 1;
            this_car->path[SOUTH1] = 1;
            this_car->path[EAST] = 0;
            break;
        default:
            fprintf(stderr, "ERROR: [destDir]@NORTH1 default pathing\n");
            break;
        }
        break;

    case WEST:
        switch (destDir)
        {
        case NORTH1:
            /* Consume SW SE NE */
            this_car->path[NORTH1] = 0;
            this_car->path[WEST] = 1;
            this_car->path[SOUTH1] = 1;
            this_car->path[EAST] = 1;
            break;
        case SOUTH1:
            /* Consume SW */
            this_car->path[NORTH1] = 0;
            this_car->path[WEST] = 1;
            this_car->path[SOUTH1] = 0;
            this_car->path[EAST] = 0;
            break;
        case EAST:
            /* Consume SW SE */
            this_car->path[NORTH1] = 0;
            this_car->path[WEST] = 1;
            this_car->path[SOUTH1] = 1;
            this_car->path[EAST] = 0;
            break;
        default:
            fprintf(stderr, "ERROR: [destDir]@NORTH1 default pathing\n");
            break;
        }
        break;

    case SOUTH1:
        switch (destDir)
        {
        case WEST:
            /* Consume SE NE NW */
            this_car->path[NORTH1] = 1;
            this_car->path[WEST] = 0;
            this_car->path[SOUTH1] = 1;
            this_car->path[EAST] = 1;
            break;
        case NORTH1:
            /* Consume SE NE */
            this_car->path[NORTH1] = 0;
            this_car->path[WEST] = 0;
            this_car->path[SOUTH1] = 1;
            this_car->path[EAST] = 1;
            break;
        case EAST:
            /* Consume  SE */
            this_car->path[NORTH1] = 0;
            this_car->path[WEST] = 0;
            this_car->path[SOUTH1] = 1;
            this_car->path[EAST] = 0;
            break;
        default:
            fprintf(stderr, "ERROR: [destDir]@NORTH1 default pathing\n");
            break;
        }
        break;

    case EAST:
        switch (destDir)
        {
        case WEST:
            /* consume NE NW */
            this_car->path[NORTH1] = 1;
            this_car->path[WEST] = 0;
            this_car->path[SOUTH1] = 0;
            this_car->path[EAST] = 1;
            break;
        case SOUTH1:
            /* Consume NE NW SW */
            this_car->path[NORTH1] = 1;
            this_car->path[WEST] = 1;
            this_car->path[SOUTH1] = 0;
            this_car->path[EAST] = 1;
            break;
        case NORTH1:
            /* Consume  NE */
            this_car->path[NORTH1] = 0;
            this_car->path[WEST] = 0;
            this_car->path[SOUTH1] = 0;
            this_car->path[EAST] = 1;
            break;
        default:
            fprintf(stderr, "ERROR: [destDir]@NORTH1 default pathing\n");
            break;
        }
        break;

    default:
        fprintf(stderr, "Error hit default\n");
        break;
        return 1;
    }
    return 0;
}

void traverse_intersection(car_t *this_car)
{

    car_direction_t apprDir = this_car->appr_dir;
    car_direction_t destDir = this_car->dest_dir;
    // hard code each direction and dest, each should change state -> print_state() -> sleep
    // think about setting leave here, and re incrementing the lane_state
    switch (apprDir)
    {
    case NORTH1:
        switch (destDir)
        {
        case WEST:
            // north going west: turn left
            traverse_helper(this_car, STATE_GO_LEFT_I1);
            break;
        case SOUTH1:
            // North to south : go Straight
            traverse_helper(this_car, STATE_GO_STRAIGHT_I1);
            break;
        case EAST:
            // North to east go straight, go left
            traverse_helper(this_car, STATE_GO_STRAIGHT_I1);
            traverse_helper(this_car, STATE_GO_LEFT_I1);
            break;
        default:
            fprintf(stderr, "Invalid State traverse_intersection() N->...\n");
            break;
        }
        break;
    case WEST:
        switch (destDir)
        {
        case NORTH1:
            // west to north: go straight go left
            traverse_helper(this_car, STATE_GO_STRAIGHT_I1);
            traverse_helper(this_car, STATE_GO_LEFT_I1);
            break;
        case SOUTH1:
            // west to south: go right
            traverse_helper(this_car, STATE_GO_RIGHT_I1);
            break;
        case EAST:
            // west to east: go straight
            traverse_helper(this_car, STATE_GO_STRAIGHT_I1);
            break;
        default:
            fprintf(stderr, "Invalid State traverse_intersection() W->...\n");
            break;
        }
        break;
    case SOUTH1:
        switch (destDir)
        {
        case WEST:
            // south to west: go straight go left
            traverse_helper(this_car, STATE_GO_STRAIGHT_I1);
            traverse_helper(this_car, STATE_GO_LEFT_I1);
            break;
        case NORTH1:
            // south to north: go straight
            traverse_helper(this_car, STATE_GO_STRAIGHT_I1);
            break;
        case EAST:
            // south to east: go right
            traverse_helper(this_car, STATE_GO_RIGHT_I1);
            break;
        default:
            fprintf(stderr, "Invalid State traverse_intersection() S->...\n");
            break;
        }
        break;
    case EAST:
        switch (destDir)
        {
        case WEST:
            // east to west: go straight
            traverse_helper(this_car, STATE_GO_STRAIGHT_I1);
            break;
        case SOUTH1:
            // east to south go straight go left
            traverse_helper(this_car, STATE_GO_STRAIGHT_I1);
            traverse_helper(this_car, STATE_GO_LEFT_I1);
            break;
        case NORTH1:
            // east to north: go right
            traverse_helper(this_car, STATE_GO_RIGHT_I1);
            break;
        default:
            fprintf(stderr, "Invalid State traverse_intersection() E->...\n");
            break;
        }
        break;
    default:
        fprintf(stderr, "Default @ traverseIntersection(apprDir)\n");
    }
}

/* crosses a tile and prints state, and sleeps */
void traverse_helper(car_t *this_car, car_state_t state)
{
    this_car->state = state;
    print_state(*this_car, NULL);
    usleep(TIME_TO_CROSS);
}

void leave_intersect(car_t *this_car)
{
    int i;
    semaphore_wait(&leave);
    this_car->state = STATE_LEAVE_I1;
    for (i = 0; i < DIRMAX; i++)
    {
        if (this_car->path[i] == 1)
        {
            lane_states[i]++;
        }
    }
    semaphore_post(&leave);
}

void printState(car_t *this_car)
{
    printf("[CARID]:%d\n[Lane]:%d %d %d %d\n[Path]:%d %d %d %d\n", this_car->car_id,
           lane_states[0], lane_states[1], lane_states[2], lane_states[3],
           this_car->path[0], this_car->path[1], this_car->path[2], this_car->path[3]);
}
