#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "shellio.h"
#include "jobs.h"

void sigint_handler(int sig_num)
{
    if (fg_job != NULL)
    {
        kill(fg_job->pid, SIGINT);
        printf("\n");
        // printf("killed process with pid %d\n", *fg_job);
    }
    else
    {
        // printf("nothing to kill\n");
        printf("\n");
        prompt();
        fflush(stdout);
    }
}
void sigstop_handler(int sig_num)
{
    if (fg_job != NULL)
    {
        printf("\n");
        append_job(fg_job->pid, fg_job->name);
        kill(fg_job->pid, SIGSTOP);
        prompt();
        fflush(stdout);
    }
}
