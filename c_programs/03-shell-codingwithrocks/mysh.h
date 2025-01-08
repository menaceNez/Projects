/*
 * [YOUR NAME HERE]
 *
 * CS441/541: Project 3
 *
 */
#ifndef MYSHELL_H
#define MYSHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* For fork, exec, sleep */
#include <sys/types.h>
#include <unistd.h>
/* For waitpid */
#include <sys/wait.h>
/* For open? */
#include <fcntl.h>

/******************************
 * Defines
 ******************************/
#define TRUE 1
#define FALSE 0

#define MAX_COMMAND_LINE 1024

#define PROMPT ("mysh$ ")


/******************************
 * Structures
 ******************************/
/*
 * A job struct.  Feel free to change as needed.
 */
struct job_t
{
    int pid; // pid from when we called fork() -> execvp()
    char *full_command; // holds binary and arguments 
    int argc; // holds the number of strings delimited by (' ' | ';' | '&')
    char **argv; // holds argument(s) for the binary, 64 size buffer
    int is_background; // 0 for background 1 for foreground
    char *binary; // the command to execute in execvp (binary,argv);
    char *is_done; // holds whether backgrounded job is finished or not
    /** 
     * Contains redirection to stdout: 1 (>)
     * Contains redirection to stdin: 0 (<)
     * Does not contain redirection: -1. 
     * init to -1 @ malloJob()
     */
    int is_redirection;
    char *redirFileName;
};
typedef struct job_t job_t;

/******************************
 * Global Variables
 ******************************/
/* Index into fgJobList, points to most recent fg completed. */
int curFg = 0; 
/* Current amount of jobs in fgJobList */
int fgSize = 0;
/* Array of foreground jobs to track history and such. */
job_t fgJobList[1024]; 
/* Index into bgJobList, points to most recent bg job done. */
int curBg = 0; 
/* Current amount of jobs in bgJobList */
int bgSize = 0;
/* Array of background jobs used to track history and other. */
job_t bgJobList[1024]; 

/*
 * Interactive or batch mode
 */
int is_batch = FALSE;

/*
 * Counts
 */
/* Holds the jobs in order of when they were done */
job_t historyArray[1024];
int total_jobs_display_ctr = 0;
/* increment upon a successful job */
int total_jobs = 0;
/* increment when hitting a bg job */
int total_jobs_bg = 0;
/* Holds the amount of jobs currently in historyArray */
int total_history = 0;

/*
 * Debugging mode
 */
int is_debug = TRUE;

/******************************
 * Function declarations
 ******************************/
/*
 * Parse command line arguments passed to myshell upon startup.
 *
 * Parameters:
 *  argc : Number of command line arguments
 *  argv : Array of pointers to strings
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int parse_args_main(int argc, char **argv);

/*
 * Main routine for batch mode
 *
 * Parameters:
 *  None
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int batch_mode(int argc, char **argv);

/*
 * Main routine for interactive mode
 *
 * Parameters:
 *  None *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int interactive_mode(void);
/**
 * Frees jobs in History
 */
void freeJobs();
/**
 * Checks bg job list for last index to be done, indicating a wipe of the lst
 *  this is implemented via the curBg and bgSize counters.
 * Returns:
 *  0 if no reset
 *  1 if we reset
 */
int checkBgJobs(void);
/**
 * Accepts tok until ; or & or no more token
 *  will parse userinput and fill singleCmd 
 * Returns:
 *  0 : if more input is needed or just one command
 *  1 : input is reached  
 *  -1 : upon error
 *  
 */
int oneCmd(char *tok, char *singleCmd, int *argc);
/**
 * Replaces \n with \0 
 * 
 * Parameters:
 *  Reference to a string.
 * 
 * Returns:
 *  0 on success
 *  return -1 if no \n char
 */
int stripNewline(char **line);
/**
 * Takes user input and creates a job_t
 * 
 * Parameters:
 *  Line from userin, passed by reference
 * 
 * Returns: job_t for launch_job
 */
void parseUserin(char **line, size_t len);
/**
 * Takes in one full command to be parsed into a job and returned.
 *  iterates over each word in cmd init word[0] to binary
 *  the rest into argv items
 * Returns:
 *  
 */
job_t *jobAttributes(char *cmd, int argc);
/**
 * Allocates memory for the various char pointers
 * 
 * Returns:
 * 0 on success
 * -1 on failure
 *    
 */
void mallocJob(job_t **thisJob, int argc);
/**
 * Add a job to fgArray
 * Returns:
 *  0 on success
 *  1 on failure
 */
int addToFg(job_t *job);
/**
 * Add a job to bgArray
 * Returns:
 *  0 on success
 *  1 on failure
 */
int addToBg(job_t *job);
/*
 * Launch a job
 *
 * Parameters:
 *   loc_job : job to execute
 *
 * Returns:
 *   2 on exit cmd
 *   0 on success
 *   Negative value on error
 */
int launch_job(job_t *loc_job);
/**
 * Determines if the current job is a builtin command or if its one to execvp().
 * Returns:
 *  1 on a builtin command identififed.
 *  0 on a non builtin command.
 */
int determineBuiltin(job_t *job);
/*
 * Built-in 'exit' command
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int builtin_exit(void);

/*
 * Built-in 'jobs' command
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int builtin_jobs(void);

/*
 * Built-in 'history' command
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int builtin_history(void);

/*
 * Built-in 'wait' command
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int builtin_wait(void);

/*
 * Built-in 'fg' command
 *
 * Parameters:
 *   None (use default behavior)
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int builtin_fg(void);

/*
 * Built-in 'fg' command
 *
 * Parameters:
 *   Job id
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int builtin_fg_num(int job_num);

#endif /* MYSHELL_H */
