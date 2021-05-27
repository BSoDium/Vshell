#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "shellio.h"
#include "jobs.h"

// Sigint signal handler (ctrl + c)
void sigint_handler(int sig_num)
{
    if (fg_job != NULL)
    {
        kill(fg_job->pid, SIGINT);
        int jobid = find_job(fg_job->pid);
        if (jobid > 0) {
            delete_job(jobid);
        }
        fg_job = NULL;
        printf("\n");
    }
    else
    {
        // printf("nothing to kill\n");
        printf("\n");
        prompt();
        fflush(stdout);
    }
}

// Sigstop signal handler (ctrl + z)
void sigstop_handler(int sig_num)
{
    if (fg_job != NULL)
    {
        printf("\n");
        int jobid = find_job(fg_job->pid);
        if (jobid < 0) {
            append_job(fg_job->pid, Suspended, fg_job->name);
        }
        kill(fg_job->pid, SIGSTOP);
        fg_job = NULL;
    }
}
