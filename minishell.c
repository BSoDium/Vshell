#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdbool.h>
#include <wait.h>
#include <string.h>
#include <errno.h>

#include "readcmd.h"
#include "handler.h"
#include "shellio.h"
#include "jobs.h"

/**
 * Bug reports : 
 * -> kill 0 crashes the shell - wontfix
 */

// internals ----

// exit the shell
void exit_sh()
{
    int jobs_killed = 0;
    for (int i = 0; i < jobc; i++)
    {
        if (strcmp(jobs[i].status, "Suspended") || strcmp(jobs[i].status, "Running"))
        {
            kill(jobs[i].pid, SIGINT);
            jobs_killed++;
        }
    }
    if (jobs_killed)
    {
        char *plural = "";
        if (jobs_killed > 1)
            plural = "s";
        iothrow("Exiting : %d job%s killed\n", jobs_killed, plural);
    }

    exit(EXIT_SUCCESS);
}

// change directory
int cd(char *path)
{
    return chdir(path);
}

// stop (suspend) a job
int stop(int jid)
{
    if (jid < jobc)
    {
        int code = kill(jobs[jid].pid, SIGSTOP);
        return code;
    }
    else
    {
        return 1;
    }
}

// resume job as background process
int bg(int jid)
{
    if (jid < jobc)
    {
        int code = kill(jobs[jid].pid, SIGCONT);
        return code;
    }
    else
    {
        return 1;
    }
}

// resume job as foreground process
int fg(int jid)
{
    int status;
    if (jid < jobc)
    {
        int code = kill(jobs[jid].pid, SIGCONT);
        waitpid(jobs[jid].pid, &status, 0);
        return code;
    }
    else
    {
        return 1;
    }
}

/**
 * Run internal commands.
 * returns 0 if an internal command was executed, 1 if not
 */
int run_internals(struct cmdline query)
{
    // strcmp iothrows a segmentation fault on empty string comparison
    if (query.seq[0] == NULL)
    {
        return 1;
    }

    if (strcmp(query.seq[0][0], "exit") == 0)
    {
        exit_sh();
        return 0; // unreachable code
    }
    else if (strcmp(query.seq[0][0], "cd") == 0)
    {
        if (cd(query.seq[0][1]))
        {
            iothrow("Unknown path : %s\n", query.seq[0][1]);
        }
        return 0;
    }
    else if (strcmp(query.seq[0][0], "jobs") == 0)
    {
        show_jobs();
        return 0;
    }
    else if (strcmp(query.seq[0][0], "stop") == 0)
    {
        char *jobid = query.seq[0][1];
        if (jobid == NULL)
        {
            iothrow("Job Id required\n");
        }
        else if (stop(atoi(jobid)))
        {
            iothrow("Unable to stop job with id : %s\n", jobid);
        }
        return 0;
    }
    else if (strcmp(query.seq[0][0], "fg") == 0)
    {
        char *jobid = query.seq[0][1];
        if (jobid == NULL)
        {
            iothrow("Job Id required\n");
        }
        else if (fg(atoi(jobid)))
        {
            iothrow("Unable to resume foreground job with id : %s\n", jobid);
        }
        return 0;
    }
    else if (strcmp(query.seq[0][0], "bg") == 0)
    {
        char *jobid = query.seq[0][1];
        if (jobid == NULL)
        {
            iothrow("Job Id required\n");
        }
        else if ((jobid != NULL) && bg(atoi(jobid)))
        {
            iothrow("Unable to resume background job with id : %s\n", jobid);
        }
        return 0;
    }
    else
    {
        return 1;
    }
}

// run a query
void run(struct cmdline query)
{
    if (query.err != NULL)
    {
        iothrow(query.err);
    }

    signal(SIGCHLD, update_job_state);

    // run internal commands
    if (!run_internals(query))
    {
        return;
    }

    int status;
    int child = fork();

    if (child < 0)
    { // fork fail
        iothrow("Fatal error : Process creation failed and returned %d\n", child);
        exit(EXIT_FAILURE);
    }
    else if (child == 0)
    { // fork success , child code
        // reset behaviour
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        // execute child code
        execvp(query.seq[0][0], query.seq[0]);
        printf("%sUnknown command%s : %s\n", KRED, KWHT, query.seq[0][0]);
        exit(EXIT_FAILURE);
    }

    if (query.backgrounded == NULL)
    {
        fg_job = malloc(sizeof(struct job));
        fg_job->pid = child;
        fg_job->name = query.seq[0][0];
        waitpid(child, &status, WUNTRACED);
        fg_job = NULL;
    }
    else
    {
        append_job(child, query.seq[0][0]);
    }
}

int main(int argc, char *argv[])
{
    bool exit = false;

    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigstop_handler);

    splash();
    while (!exit)
    {
        prompt();
        struct cmdline *query = readcmd();
        run(*query);
    }
    return EXIT_SUCCESS;
}
