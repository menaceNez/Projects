# CS441/541 Synchronization Project

## Author(s): Gavin Stankovsky

## Date: 
### Nov 10th:
    Note for next work cycle:
        get simulate_intersection up and running
        Think about our algorithm/semaphores and how it works
        setup path[4] for each car marking 1 with direction enum tile needed for traversal. 

Stopped tracking days...

## Description:
The method used for modeling the lane goes as such:
    A car pulls into the intersection from a various direction, depending on what direction they will wait at a different stop_sign which is a binary semaphore that allows 1 thread in from each direction. These threads will test() to see if the lane they want to go to is available otherwise they will wait in test and get signaled by cars that have left the intersection.
lane_state:
    Similar to tannenbaum solution with 'Thinking' states
    Execpt each lane's availability is modeled as:
    1: available
    0: unavailable

struct car_t {
    int path[4]; 
    semaphore_t cango; // used for each car in test() if they fail, they wait
}

path[0] = NORTH1 entry tile
path[1] = WEST entry tile
path[2] = SOUTH entry tile
path[3] = EAST entry tile

## How to build the software

#### Parsing arguments: 
    Takes in num_cars and time to run, and initalizes ttl and num_cars
#### start_car
    loop body where a car will leave and enter the intersection until time ends. simulate_intersection is the driver for intersection flow.


#### simulate_intersection:
    Driver function in start_car loop.
    Takes a car and 

#### initalize_paths()            
    Takes in a car and considers its approach and final destination
    based on those two path will be initalized with required tiles/lanes to get from approach to final
    Used in test()

## How to use the software

./stoplight for usage details

## How the software was tested

Tested with various times and car sizes ranging from less than 4 cars to more than 1000 cars each with different time scales. 

Outside of compiling and testing the program, thought was put into how this would work as if it were a real intersection. Cars from each direction already have their known destination in mind before hand, hence my car model does as well. This prevents deadlock within the intersection and leaves worry of deadlock outside of the intersection or before they enter. 

Tested with print statements to go back and look at how threads are scheduled and what order they are executing in. Such as print statements before waiting in test(), after they are signaled out of their wait, and when they begin testing were some key points I looked at for my threads.

## Known bugs and problem areas

Busy waiting occurs in test() not sure if it has to do with over signaling, not sure what to do worried Ill dig myself into a hole of deadlock fixing it :D.

# Special Section: 
### Single intersection Question (1):
#### Question a:
    (a) Assume that the residents of Smallville are exceptional and follow the old (and widely ignored)
    convention that whoever arrives at the intersection first proceeds first. Using the language of
    synchronization primitives describe the way this intersection is controlled. In what ways is this
    method suboptimal?

#### Answer: a

    This intersection can be controlled by 4 mutex's representing the lanes a car will move through to get to their desired direction. If we follow the convention of whoever shows up to the intersection first procededs first, various conditions may occur.  If we only allow one resident through the intersection at a time there are opportunities where we could have 2 or more cars going through the intersection. Such as each resident wants to do a right turn, it would take significantly more time to allow 1 car though at a time when we could have all 4 residents immediately take their right hand turn. 

#### Question b:
    (b) Now, assume that the residents of Smallville are like most people and do not follow the convention
    described above. In what one instance can this four-way-stop intersection produce a deadlock?
    (It will be helpful to think of this in terms of the model we are using instead of trying to visualize
    an actual intersection).

#### Answer: b

    If we have each lane populated with a resident reaching the intersection at the same time, its possible to have a deadlock occur. For instance a resident from the north, west, south, and east all going straight though the intersection (needing 2 lane tiles to get through). Each resident will consume their lanes mutex try to consume the next tile and wait(), leading to a deadlock between the residents.
### Two Intersection Question (2):

    (a) Assume that the bridge between the two intersections had space for at most N cars in any one
    direction. In what instances can this produce deadlock? Provide at least 4 specific, distinct
    examples that use a path crossing the bridge. Include in those examples what you would consider
    the worst case scenario.

#### Answer: a
    Deadlock can occur in instances where the bridge has filled in both directions East and West with N cars ready to go. I'm assuming subsequent cars wanting to go onto the bridge are NOT going to consume a lane and will be blocked at the stop sign as there is no space on the bridge, so to deadlock West or East lane should try to consume a space in their respective starting intersection. 

**Note: East(2) for the 2 intersection model, is refering the FAR RIGHT entry point of the second intersection. Which follows that East(1) denotes the entry point for the first intersection from East.** 

**Its also to be assumed that the cars enter the intersection at the same time for a deadlock to occur and consume/decrement the semaphore**  

    1. Intersection(1) & Intersection(2) Deadlock:

    From:      To:         Tiles/Lanes Needed:
    East(1) -> South(1) = { NE(1), NW(1), SW(1) } 
    South(1) -> North(1) = { SE(1), NE(1) }
    West(1) -> North(1) = { SW(1), SE(1), NE(1) }
    North(1) -> South(1) = { NW(1), SW(1) }
    **This (I think) deadlocks the first intersection**
    **The bridge will now fill up to N cars in the east lane**
    Subsequent cars will deadlock intersection 2:
    East(2) -> South(2) = { NE(2), NW(2), SW(2) }
    South(2) -> North(2) = { SE(2), NE(2) }
    North(2) -> South(2) = { NW(2), SW(2) } 
    West(2) -> East(2) = { SW(2), SE(2) }

    2. 

    From:      To:         Tiles/Lanes Needed:
    *East(1) -> West(1) = { NE(1), NW(1) } 
    *South(1) -> West(1) = { SE(1), NE(1), NW(1) }
    West(1) -> North(1) = { SW(1), SE(1), NE(1) }
    North(1) -> South(1) = { NW(1), SW(1) }
    **This (I think) deadlocks the first intersection**
    **The bridge will now fill up to N cars in the east lane**
    Subsequent cars will deadlock intersection 2:
    East(2) -> South(2) = { NE(2), NW(2), SW(2) }
    South(2) -> North(2) = { SE(2), NE(2) }
    North(2) -> South(2) = { NW(2), SW(2) } 
    *West(2) -> North(2) = { SW(2), SE(2), NE(2) }

    3.

    From:      To:         Tiles/Lanes Needed:
    *East(1) -> South(1) = { NE(1), NW(1), SW(1) } 
    South(1) -> West(1) = { SE(1), NE(1), NW(1) }
    West(1) -> North(1) = { SW(1), SE(1), NE(1) }
    North(1) -> South(1) = { NW(1), SW(1) }
    **This (I think) deadlocks the first intersection**
    **The bridge will now fill up to N cars in the east lane**
    Subsequent cars will deadlock intersection 2:
    East(2) -> South(2) = { NE(2), NW(2), SW(2) }
    South(2) -> North(2) = { SE(2), NE(2) }
    *North(2) -> East(2) = { NW(2), SW(2), SE(2) } 
    West(2) -> North(2) = { SW(2), SE(2), NE(2) }

    4.

    From:      To:         Tiles/Lanes Needed:
    East(1) -> West(1) = { NE(1), NW(1) } 
    South(1) -> West(1) = { SE(1), NE(1), NW(1) }
    West(1) -> North(1) = { SW(1), SE(1), NE(1) }
    North(1) -> South(1) = { NW(1), SW(1) }
    **This (I think) deadlocks the first intersection**
    **The bridge will now fill up to N cars in the east lane**
    Subsequent cars will deadlock intersection 2:
    East(2) -> South(2) = { NE(2), NW(2), SW(2) }
    South(2) -> North(2) = { SE(2), NE(2) }
    North(2) -> East(2) = { NW(2), SW(2) } 
    *West(2) -> East(2) = { SW(2), SE(2) }

# Special Section: After coding

### Describe your solution to the problem (in words, not in code).
    
    My solution has each direction NORTH, WEST, SOUTH, EAST controlled via a binary semaphore, called stop_sign. This semaphore ensures each direction has at most, contention of 1 car in the intersection. To avoid deadlock along with ensuring a car is able to traverse the intersection I took inspiration from tannenbaums solution to dining philosophers. Our cars, if allowed in, will first change their state to approaching, then calls test(). Test() relies on our car having its path laid out in advance, thats where path[4] is used in our car struct, path[4] and lane_states are parallel arrays that can use NORTH1, WEST, SOUTH1, and EAST to index into. It maps out what tiles/lanes are needed for our car to advance safely through the intersection and compares them to another global variable called lane_states, 1 means tile/lane available, 0 means its currently in use. Upon a successful test() our car will decrement lane_states and a flag called can_proceed is set, later calling traverse_intersection(). The key part of my solution lies in test(), we allow 1 car at a time to test() and if it fails it will wait(), upon another car leaving the intersection it will signal() any waiting cars to call test again. 

### Describe how your solution meets the goals of this part of the project, namely (each in a separate section):

• How does your solution avoid deadlock?

    Dead lock is avoided in this model as only cars that are able to traverse the intersection are allowed to proceed and go through the intersection. Otherwise they wait for a car to leave and then test again.

• How does your solution prevent accidents?

    Accidents are prevented via our test() model, we only travse the intersection as long as the required tiles are available.

• How does your solution improve traffic flow?

    Traffic is improved as multiple cars are allowed in the intersection at a time, instead of just 1 at a time. One car may test at a time but after it tests it decrements its required tiles but allows for other tiles to still be available for use.  

• Does your solution preserve the order of cars approaching the intersection? If so, then how?

    Order of approaching cars is not enforced strictly. Our intersection merely allows a car from each direction to enter the intersection and test() whether they may continue, if they cannot go, they will wait until traversing car is finished. Wherein they will test again, this solution could mean a thread will starve depending on how the cpu schedules the threads. 

• How is your solution “fair”?

    This solution without a mechanism to ensure that the cars that arrive first are processed first cannot be fully fair, as we could have cars loop around and be processed again. But since we allow each lane to have only 1 contender and each car that enters the intersection to be processed for sure its "fair" in the sense that every car that enters the intersection will be processed. 

• How does your solution prevent “starvation”?

    My solution combats starvation as each car that finishes the traversal of lanes will signal to other waiting cars allowing them to then test() to see if they can traverse the intersection. One area where starvation could occur is with multiple cars waiting to test, some may not be able to have the needed lanes for traversal and be put to wait again, luck of the draw in some sense. 