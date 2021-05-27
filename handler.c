#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "shellio.h"
#include "jobs.h"

// Sigint signal handler (ctrl + c)
void sigint_handler(int sig_num)
{
    if (fg_jobc > 0)
    {
        for (int i = 0; i < fg_jobc; i++)
        {
            struct job job = fg_jobs[i];
            kill(job.pid, SIGINT);
            int jobid = find_job(job.pid);
            if (jobid > 0)
            {
                delete_bg_job(jobid);
            }
            delete_fg_job(jobid);
        }
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
    if (fg_jobc > 0)
    {
        printf("\n");
        for (int i = 0; i < fg_jobc; i++)
        {
            struct job job = fg_jobs[i];
            int jobid = find_job(job.pid);
            kill(job.pid, SIGSTOP);
            delete_fg_job(jobid);
            if (jobid < 0)
            {
                append_bg_job(job.pid, Suspended, job.name);
            }
        }
    }
}
