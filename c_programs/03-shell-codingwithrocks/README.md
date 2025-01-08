# CS441/541 Shell Project

## Author(s):
### Gavin Stankovsky

## Date: Kept track of what I was working on, not sure much usful is here for documentation.

#### **Consider how we can use parseUserIn for batch and interactiveMode**
#### Session 10/1/24: What I worked on:
* Get parser online
    * begins with stripLine();
    * take strtok and split by white space
    * After strip line -> parse out cmd args(strtok) -> create job_t from tok
#### Session 10/2/24: What I worked on:
* Take user input -> strtok -> create job_t
    * Added getLongestString() -> became jobAttributes()
        * Eventaully used for **argv init 
    * Think about ordering of how to initalize job
        * load full command
        * pick out the binary while getLongestString() and get argc
        * **Think about concurrency ';' and '&'**
* Ended today with having my job_t in parseUserin populated from variables
mainly populated from getLongestString()
* Next up I should consider ways to implement concurrency in my launch_job function
#### Session 10/3/24: What I worked on:
* Stared launch_job()
* Think more about what to do when user prints with no command.
* Should start working on concurrent jobs -> should occur in parse jobs likely
* Number of jobs should be global var, history could be an array of jobs (in builtin or global?)
* & jobs should be ran and printed after jobs has been executed.
#### Session 10/3/24: What I worked on:
* Changing getLongestString() to parse jobAttributes -> jobAttributes will be passed one command (delmit by " " and ";" and "&")
* Once we hit a ";" "&" want to take that take that buffer string and pass it to jobAttributes -> returns item for job_t
    * After creation of job reset jobAttr values in loop to default to ensure no overlap of un initalized values.
* Left off working on job attributes -> malloc argv and add from tok, **Remember to eventaully free(job->argv), free(job)**
#### Session 10/5/24: What I worked on:
* Starting to work on getting my jobs running, along with possibly linkning them via a linked list or array of sorts.
#### Session 10/8/24: What I worked on:
* Get jobs into an array, parse entire user line and work to each job splitting if its fg or bg along with 1
    * fin background job after array list(s)
* **Left off dealing with strtok needing to use strtok_r, Issue being the passed input string gets consumed and our local version is also consumed :(**
* strtok_r works good, we have our launch job accepting jobs passed in from our job array and should have all our job allocation complete
    * should begin making some test cases to ensure everything is working to par
* Should look at the built in jobs now thinking about how they should be called and not our 
* Comfy with how foreground jobs are working -> 2 arrays 1 fg[] another bg[]. Take array with its size and counter for last finished job. Do jobs as they come in
this way maintains history along with info on #bg jobs instantly.
* I've realized deep into the split array scheme isnt the most conforming to history is meant to be printed 
    * I may have to create a new array for history adding the current job as they are created. or re-writing to using 1 array (dont want to tho ^_^).
* thinking on getting background array to be a temp buffer for bg jobs that way we can print the order in which they are completeted 
    * just need to reset curBg to 0 possibly just increment backward clearing buffer
    * TESTING: overwriting the buffer instead of clearing it and just using size and curBg counters. <- **It worked**
* remember to throw jobs into history before execution to catch erronius jobs. 
* Incremented total_jobs in launch_job() along with total_jobs_bg in launch_job()
* Left off with history printing all jobs that have been pushed into launch_job()
* want to work on batch_mode() along with the builtin functions.
    * Should probably do the built in fuctions getting that working with launch_job()
* also need to do file redirection FILE handling?
* **if you are using the template code, you may want to modify the job t data structure to contain information about whether redirection is used, and the files that are redirected**

#### Session 10/9/24: What I worked on:
* File redirection consists of taking a fd from an open textfile we want to use to push stdin or stdout to our C program binary, and setting that fd to either STDIN_FILENO or STDOUT_FILENO
* after setting up correct capture of stdin and stdout or not, need to take next tok after > or < and place that within a filname for redirection in our job
* Brain melted after working on finding out why execvp() was erroring on STDOUT, NEED ARGV PASSED TO EXECVP TO HAVE LAST STR IN STR ARRAY = NULL
* File redirection works for the most part should investigate at some point why i get mysh$ printed a million times
* **JOBS** implementation, currently holding the jobs in an array while that batch is still processing, if there is a way to hold the batch until jobs is called 
    * need to use waitpid() to check status of children upon pressing enter
* want my background arrray to hold jobs until the final job has finished once the final job has finished we can clear our job array by resetting counters
* Figure out background jobs and waitpid(pid,&status,WNOHANG) non blocking wait to check status of child and if its finished
* Finished with jobs, wait, and history.
    * Should fix buffer resizing or hold up to 1024


## Description:

## How to build the software
* Three arrays holding various jobs:
    * historyArray[] : holds the entire history of jobs that were input bg and fg
    * bgJobList[] : holds a background jobs while they are running when the last one finishes list resets.
    * fgJobList[] : Holds all fg jobs in the order they were added.
* Loop -> get userinput -> parse out single command to job -> add job to joblist marking if its fg or bg beforehand -> execute fg jobs then bg jobs -> increment job counters on successful execution -> print statistics

## How to use the software : ./mysh [filenames]
* Interactive and Batch mode both work mostly as expected, more testing should/could be done to ensure proper execution.
    * allows for builtin jobs along with system provided ones such as pwd, ls, sleep, and other /bin/ commands
* Type in commands like a normal terminal and aquire responsive and informative output.
* To use batch mode start the shell as usual but add a file name (1 or more files for batch mode) and it will execute commands as if it were a shell.

## How the software was tested
1. Test 1: builtin: tests the builtin commands 
2. Test 2: error: tests various errors and ensures that the shell continues to proceess on incorrect input.
3. Test 3: fgandbg: test background and foreground commands in same line.
4. test 4: redirInput.txt does some commands at beginning then takes output from text binary which prints hello to output.txt then output.txt gets redirected to input which will display any input it gets.
5. test 5: combines all builtins and other tests



## Known bugs and problem areas
* After execvp(date) PROMPT prints multiple times on same line, or puts command on PROMPT and puts userin to blank line.
* Did not have ample time to error test and bug squash but make check-interactive passes tests
