/*
 * Gaivn Stankovsky
 *
 * CS441/541: Project 3
 *
 */
#include "mysh.h"

int main(int argc, char *argv[])
{
    int ret;

    /*
     * Parse Command line arguments to check if this is an interactive or batch
     * mode run.
     */
    if (0 != (ret = parse_args_main(argc, argv)))
    {
        fprintf(stderr, "Error: Invalid command line!\n");
        return -1;
    }

    /*
     * If in batch mode then process all batch files
     */
    if (TRUE == is_batch)
    {
        if (TRUE == is_debug)
        {
            printf("Batch Mode!\n");
        }

        if (0 != (ret = batch_mode(argc, argv)))
        {
            fprintf(stderr, "Error: Batch mode returned a failure!\n");
        }
    }
    /*
     * Otherwise proceed in interactive mode
     */
    else if (FALSE == is_batch)
    {
        if (TRUE == is_debug)
        {
            printf("Interactive Mode!\n");
        }

        if (0 != (ret = interactive_mode()))
        {
            fprintf(stderr, "Error: Interactive mode returned a failure!\n");
        }
    }
    /*
     * This should never happen, but otherwise unknown mode
     */
    else
    {
        fprintf(stderr, "Error: Unknown execution mode!\n");
        return -1;
    }
    /*
     * Display counts
     */


    printf("-------------------------------\n");
    printf("Total number of jobs               = %d\n", total_jobs);
    printf("Total number of jobs in history    = %d\n", total_history);
    printf("Total number of jobs in background = %d\n", total_jobs_bg);

    /*
     * Cleanup
     */


    return 0;
} /* end main */

int parse_args_main(int argc, char **argv)
{
    /*
     * If no command line arguments were passed then this is an interactive
     * mode run.
     */
    if (argc < 2) // if we have only 1 arg (program name)
    {
        // interactive mode
        is_batch = FALSE;
    }
    /*
     * If command line arguments were supplied then this is batch mode.
     */
    else
    {
        // batch mode
        is_batch = TRUE;
    }

    return 0;
}

int batch_mode(int argc, char **argv)
{
    int i;
    FILE *fptr;
    size_t len;
    char *line = NULL;
    int checkBgRet = 0, getlineRet = 0;
    /*
     * For each file...
     */
    for(i = 1; i < argc; i++) // this loop opens and reads from a file at each iteration.
    {
        fptr = fopen(argv[i],"r");
        if(fptr == NULL)
        {
            fprintf(stderr, "Error opening file\n");
        }
        while((getlineRet = getline(&line, &len, fptr)) != -1) {
            stripNewline(&line);
            parseUserin(&line, len);
            if(getlineRet == EOF) {
                total_jobs++;
                builtin_wait();
                builtin_exit();
                return 0;
            }

            while (curFg != fgSize)
            {
                historyArray[total_history++] = fgJobList[curFg];
                if(launch_job(&fgJobList[curFg]) == 2) {
                    printf("Exiting Shell!\n");
                    return 0;
                }
            }
            // run bg Jobs
            while (curBg != bgSize)
            {
                historyArray[total_history++] = bgJobList[curBg];
                if(launch_job(&bgJobList[curBg]) == 2) {
                    printf("Exiting Shell!\n");
                    return 0;
                }
            }
            // check if our last job fin, along with print any complete.
            checkBgRet = checkBgJobs();
            if (checkBgRet == 1) // last job fin reset count
            {
                bgSize = 0;
                curBg = 0;
                checkBgRet = 0;
            }
        }


       fclose(fptr); 
    }

    freeJobs();
    free(line);

    return 0;
}

job_t *freePointer;

int interactive_mode(void)
{
    int checkBgRet;
    char *line = NULL;
    size_t len = 0;

    do
    {
        /*
         * Print the prompt
         */
        printf("%s", PROMPT);
        /*
         * Read stdin, break out of loop if Ctrl-D
         */
        int getlineRet = getline(&line, &len, stdin);
        if (len > MAX_COMMAND_LINE)
        {
            fprintf(stderr, "too many commands (cmds > 1024)\n");
            return interactive_mode();
        }
        /* If user pushes enter recall interacve mode (Heap stays in tact?)*/
        if (strcmp(line, "\n") == 0)
        {
            checkBgRet = checkBgJobs();
            if (checkBgRet == 1)
            {
                bgSize = 0;
                curBg = 0;
                checkBgRet = 0;
            }
            return interactive_mode();
        }

        /* successful read */
        if (getlineRet != -1)
        {
            stripNewline(&line);
            parseUserin(&line, len);
            // run fg Jobs
            while (curFg != fgSize)
            {
                historyArray[total_history++] = fgJobList[curFg];
                if(launch_job(&fgJobList[curFg]) == 2) {
                    printf("Exiting Shell!\n");
                    return 0;
                }
            }
            // run bg Jobs
            while (curBg != bgSize)
            {
                historyArray[total_history++] = bgJobList[curBg];
                if(launch_job(&bgJobList[curBg]) == 2) {
                    printf("Exiting Shell!\n");
                    return 0;
                }
            }
            // check if our last job fin, along with print any complete.
            checkBgRet = checkBgJobs();
            if (checkBgRet == 1) // last job fin reset count
            {
                bgSize = 0;
                curBg = 0;
                checkBgRet = 0;
            }
        }
        else if (getlineRet == EOF) /* ctrl-d produces EOF */
        {
            printf("Exiting shell...\n");
            builtin_wait();
            builtin_exit();
            return 0;
        }
        else
        {
            perror("getline failed\n");
            exit(1);
        }
    } while (1 /* end condition */);

    freeJobs();
    free(line);

    return 0;
}

void freeJobs() {
    int i, j;
    for(i = 0; i < total_history; i++)
    {
        // for every job in history free its argv array
        for(j = 0; j < historyArray[i].argc; i++)
        {
            free(historyArray[i].argv[j]);
        }
        free(historyArray[i].binary);
        free(historyArray[i].full_command);
        free(historyArray[i].is_done);
        free(historyArray[i].redirFileName);
        free(freePointer);
    }
}

int checkBgJobs(void)
{
    /**iterate over bg job list, check for final index to
     * be done meaning we can reset array counts check each job for if its done to update
     * Running or Done status.
     */
    int i, status, waitRet;
    if(bgSize > 0) {
        for (i = 0; i < bgSize; i++)
        {
            waitRet = waitpid(bgJobList[i].pid, &status, WNOHANG);
            if (waitRet > 0)
            {
                strcpy(bgJobList[i].is_done, "Done");
                printf("[%d] %s\t%s\n", i, bgJobList[i].is_done, bgJobList[i].full_command);
            }
        }
        if (strcmp(bgJobList[bgSize - 1].is_done, "Done") == 0)
        {
            // final job is finished
            printf("[%d] %s\t%s\n", bgSize - 1, bgJobList[bgSize-1].is_done, bgJobList[bgSize-1].full_command);
            return 1;
        }

    }
    return 0;
}

int stripNewline(char **line)
{
    int i;
    char *derefLine = *line;
    unsigned int length = strlen(derefLine);

    for (i = length - 1; i >= 0; i--)
    {
        if (derefLine[i] == '\n')
        {
            derefLine[i] = '\0';
            return 0;
        }
    }
    // ret -1 not a bad thing, no '\n' in string.
    return -1;
}
/* The driver for interactive_mode's job initalization stage */
void parseUserin(char **line, size_t len)
{
    // parseUserIn -> function for 1 command -> jobAttributes
    job_t *curJob;
    int retCode = 0;
    char *tok = NULL;
    char singleCmd[256];
    char *savePtr = NULL;
    int argcSingleCmd = 0;

    singleCmd[0] = '\0'; // for strcpy of first item

    tok = strtok_r(*line, " ", &savePtr);

    while (tok != NULL)
    {
        retCode = oneCmd(tok, singleCmd, &argcSingleCmd);
        if (retCode == -1)
        {
            fprintf(stderr, "error in oneCmd\n");
        }
        else if (retCode == 1) /* a ';' or '&' was hit*/
        {
            // add job to fgJobList
            curJob = jobAttributes(singleCmd, argcSingleCmd); // problem wit this somewhere
            curJob->is_background = 1;
            if (addToFg(curJob) == 1)
            {
                fprintf(stderr, "error on adding to fg\n");
            }

            strcpy(singleCmd, ""); // reset single command
            argcSingleCmd = 0;
        }
        else if (retCode == 2)
        {
            // add job to bgJobList
            curJob = jobAttributes(singleCmd, argcSingleCmd);
            curJob->is_background = 0;
            if (addToBg(curJob) == 1)
            {
                fprintf(stderr, "error on adding to fg @ bgSize: %d\n", bgSize);
            }

            strcpy(singleCmd, ""); // reset single cmd
            argcSingleCmd = 0;
        }
        // continue reading input
        tok = strtok_r(NULL, " ", &savePtr);
    }
    // a ; or & was not hit but we have the last job or first job in cmd.
    if (singleCmd[0] != '\0')
    {
        curJob = jobAttributes(singleCmd, argcSingleCmd);
        curJob->is_background = 1; // if not specified bg it is fg
        if (addToFg(curJob) == 1)
        {
            fprintf(stderr, "error on adding to fg @ fgSize: %d\n", fgSize);
        }
    }
    freePointer = curJob;
}
int addToFg(job_t *job)
{
    if (fgSize + 1 > 1024)
    {
        fprintf(stderr, "More fg jobs than we can take!\n");
        return 1;
    }
    fgJobList[fgSize] = *job;
    fgSize++;
    return 0;
}
int addToBg(job_t *job)
{
    if (bgSize + 1 > 1024)
    {
        fprintf(stderr, "More bg jobs than we can take!\n");
        return 1;
    }
    bgJobList[bgSize] = *job;
    bgSize++;
    return 0;
}

int oneCmd(char *tok, char *singleCmd, int *argc)
{
    if (tok == NULL)
    {
        return -1;
    }

    if (strcmp(tok, ";") == 0)
    {
        // fg job id
        return 1;
    }
    else if (strcmp(tok, "&") == 0)
    {
        /* bg concurrentcy identifier mark current job as bg somehow*/
        return 2;
    }
    else // no concurrency hit parse normally
    {
        if (strlen(singleCmd) + strlen(tok) + 2 < 1024) // 2 for space after token
        {
            // if no cmd yet
            if (singleCmd[0] == '\0')
            {
                strcpy(singleCmd, tok);
                strcat(singleCmd, " ");
                (*argc)++;
            }
            else
            {
                strcat(singleCmd, tok);
                strcat(singleCmd, " "); // arbitrary space @ end of command (is okay?)
                (*argc)++;
            }
        }
        else
        {
            fprintf(stderr, "Reached max cmd length, last/current command may be scrapped @ oneCmd()\n");
            return -1;
        }
    }
    return 0;
}

job_t *jobAttributes(char *cmd, int argc)
{
    char *tok = NULL;
    char *savePtr = NULL;
    job_t *thisJob = NULL;
    int targetBinary = 0, i = 0;

    mallocJob(&thisJob, argc);

    strcpy(thisJob->full_command, cmd);

    tok = strtok_r(cmd, " ", &savePtr);

    int captureRedirFile = 0;

    while (tok != NULL)
    {
        if (targetBinary == 0)
        {
            strcpy(thisJob->binary, tok);
            targetBinary++;
        }
        // move this to oneCMD
        if (captureRedirFile == 1)
        {
            strcpy(thisJob->redirFileName, tok);
            captureRedirFile = 0;
        }
        else if (strcmp(tok, "<") == 0)
        {
            thisJob->is_redirection = 0; // stdin
            captureRedirFile = 1;
            thisJob->argc -= 2;
        }
        else if (strcmp(tok, ">") == 0)
        {
            thisJob->is_redirection = 1;
            captureRedirFile = 1;
            thisJob->argc -= 2;
        }
        else // should hit everything but the > <filename>
        {

            strcpy((thisJob->argv[i]), tok);
            i++;
        }

        tok = strtok_r(NULL, " ", &savePtr);
    }
    thisJob->argv[i] = NULL;

    return thisJob;
}

void mallocJob(job_t **thisJob, int argc)
{
    int i;
    (*thisJob) = (job_t *)malloc(sizeof(job_t));
    if ((*thisJob) == NULL)
    {
        fprintf(stderr, "Failed to create job @ mallocJob()\n");
        exit(1);
    }

    (*thisJob)->argc = argc;

    (*thisJob)->binary = (char *)malloc(sizeof(char) * 32);
    if ((*thisJob)->binary == NULL)
    {
        fprintf(stderr, "Error alloc @ mallocJob()\n");
        (exit(1));
    }
    (*thisJob)->full_command = (char *)malloc(sizeof(char) * 1024);
    if ((*thisJob)->full_command == NULL)
    {
        fprintf(stderr, "Error on malloc to job->full_cmd\n");
    }
    (*thisJob)->argv = (char **)malloc(sizeof(char *) * (*thisJob)->argc);
    for (i = 0; i < (*thisJob)->argc; i++)
    {
        (*thisJob)->argv[i] = (char *)malloc(sizeof(char) * 64);
        if ((*thisJob)->argv[i] == NULL)
        {
            fprintf(stderr, "Error allocating memory @ mallocJob()\n");
            // builtin exit here if possible.
            exit(1);
        }
    }
    (*thisJob)->is_redirection = -1;
    (*thisJob)->redirFileName = (char *)malloc(sizeof(char) * 64);
    if ((*thisJob)->redirFileName == NULL)
    {
        fprintf(stderr, "Error malloc @ redirFileName\n");
        exit(1);
    }

    (*thisJob)->is_done = (char *)malloc(sizeof(char) * 32);
    if ((*thisJob)->is_done == NULL)
    {
        fprintf(stderr, "Error alloc is_done @ mallocJob()\n");
        exit(1);
    }
}


int launch_job(job_t *loc_job)
{
    strcpy(loc_job->is_done, "Running"); // set job state to running
    if (determineBuiltin(loc_job) == 1)
    {
        if(loc_job->is_background == 1)
        {
            curFg++;    
        }
        else {
            curBg++;
            total_jobs_bg++;
        }
        strcpy(loc_job->is_done, "Done");
        if(strcmp(loc_job->binary, "exit") == 0) {
            total_jobs++;
            return 2;
        }
        return 0;
    }
    else
    {
        int fd;
        int status = 0;
        pid_t pid;
        pid = fork();
        /*
         * Display the job
         */

        if (pid < 0)
        {
            fprintf(stderr, "Something forked wrong\n");
            exit(1);
        }
        else if (pid == 0) // end of the arguments.
        {
            // child case
            // check for redirections
            if (loc_job->is_redirection == 0)
            {
                // stdin redirection
                fd = open(loc_job->redirFileName, O_RDONLY, 0444); // read read read permissions
                if (fd == -1)
                {
                    fprintf(stderr, "Error finding file for stdin\n");
                }

                if (dup2(fd, STDIN_FILENO) == -1) // point stdin to fd (our open file)
                {
                    fprintf(stderr, "dup2(fd,STDIN_FILENO) error\n");
                }
                close(fd);

                execvp(loc_job->binary, loc_job->argv);

                fprintf(stderr, "Error on launch job @STDIN: %s\n", loc_job->binary);
                exit(1); // exit on undesireable job
            }
            else if (loc_job->is_redirection == 1)
            {
                // stdout redirection
                fd = open(loc_job->redirFileName, O_WRONLY | O_CREAT | O_TRUNC, 0644); // owner: read+write group: read others: read
                if (fd == -1)
                {
                    fprintf(stderr, "Error on redirection to stdout\n");
                }

                if (dup2(fd, STDOUT_FILENO) == -1) // point stdout to fd (our open file)
                {
                    fprintf(stderr, "dup2(fd,STDOUT_FILENO) error\n");
                }
                close(fd);

                execvp(loc_job->binary, loc_job->argv);

                fprintf(stderr, "Error on launch job @STDOUT: %s\n", loc_job->binary);
                exit(1); // exit on undesireable job
            }
            else
            {
                // non redirected case
                execvp(loc_job->binary, loc_job->argv);

                /* shouldnt fall out of execvp if so its an error with the passed binary*/
                fprintf(stderr, "Error on launch job: %s\n", loc_job->binary);
                exit(1); // exit on undesireable job
            }
        }
        else
        {
            // parent case
            loc_job->pid = pid;
            if (loc_job->is_background == 1)
            {
                // foreground case
                curFg++;
                waitpid(pid, &status, 0);
            }
            else if (loc_job->is_background == 0)
            {
                // background case
                curBg++;
                total_jobs_bg++;
            }
            // reset stdin and stdout
        }
    }

    total_jobs++; // increment upon successful job
    return 0;
}

int determineBuiltin(job_t *job)
{
    if (strcmp(job->binary, "exit") == 0)
    {
        builtin_exit();
        return 1;
    }
    else if (strcmp(job->binary, "jobs") == 0)
    {
        builtin_jobs();
        return 1;
    }
    else if (strcmp(job->binary, "history") == 0)
    {
        builtin_history();
        return 1;
    }
    else if (strcmp(job->binary, "wait") == 0)
    {
        builtin_wait();
        return 1;
    }
    else if (strcmp(job->binary, "fg") == 0)
    {
        // examine job->argc for if we have num or not
        if(job->argc > 1) {
            builtin_fg_num((int)strtol(job->argv[1], NULL, 10));
        }
        else {
            builtin_fg();

        }
        return 1;
    }

    return 0;
}

int builtin_exit(void)
{
    int currentRunning = 0;
    /** will print and finish jobs that are still running. */
    // we have jobs in both our lists possibly need to check sizes and act accordingly
    for(int i = 0; i < bgSize; i++)
    {
        if(strcmp(bgJobList[i].is_done, "Running") == 0) {
           currentRunning++; 
        }
    }
    printf("%d Jobs still running in background...\n", currentRunning);
    builtin_wait(); // wait for last bg process
    return 0;
}

int builtin_jobs(void)
{
    // take our background array and
    if(bgSize != 0)
    {
        for (int i = 0; i < bgSize; i++)
        {
            printf("[%d] %s\t%s\n", i, bgJobList[i].is_done, bgJobList[i].full_command);
        }
    }

    return 0;
}

int builtin_history(void)
{
    for(int i = 0; i < total_history; i++)
    {
        if(historyArray[i].is_background == 0) {
            // background type
            printf(" %d %s &\n",(i+1), historyArray[i].full_command);
        }
        else {
            printf(" %d %s\n",(i+1), historyArray[i].full_command);

        }
    }
    return 0;
}

int builtin_wait(void)
{
    int waitForPid,status;
    // wait involves waiting for my last background job to finish ->
    if(bgSize != 0) {
        waitForPid = bgJobList[bgSize - 1].pid;
        waitpid(waitForPid, &status, 0);
    }
    return 0;
}

int builtin_fg(void)
{
    int status;
    if(bgSize != 0)
    {
        bgJobList[bgSize-1].is_background  = 1; // becomes fg job 
        waitpid(bgJobList[bgSize-1].pid, &status,0);

    }
    else {
        fprintf(stderr, "No current background jobs!\n");
    }
    return 0;
}

int builtin_fg_num(int job_num)
{
    int status;
    if(bgSize > 0 && job_num < bgSize) {
        bgJobList[job_num].is_background  = 1; // becomes fg job 
        waitpid(bgJobList[job_num].pid, &status,0);
    }
    else {
        fprintf(stderr, "Error: job not in background!\n");
    }
    return 0;
}
