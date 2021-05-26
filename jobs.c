#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "shellio.h"
#include "jobs.h"

struct job;

struct job jobs[MAXJOBS];

struct job *fg_job;

int jobc;

int print_job(int index)
{
    if (index < jobc)
    {
        char indicator; // indicates which job will be summoned when calling fg
        if (index == jobc - 1)
        {
            indicator = '+';
        }
        else if (index == jobc - 2)
        {
            indicator = '-';
        }
        else
        {
            indicator = ' ';
        }
        printf("[%d]%c  %s                 %s\n", index, indicator, jobs[index].status, jobs[index].name);
        return 0;
    }
    else
    {
        return 1;
    }
}

void show_jobs()
{
    for (int i = 0; i < jobc; i++)
    {
        print_job(i);
    }
}

int find_job(int pid)
{
    for (int i = 0; i < jobc; i++)
    {
        if (jobs[i].pid == pid)
        {
            return i;
        }
    }
    return -1;
}

int append_job(int pid, char *name)
{
    if (jobc < MAXJOBS)
    {
        jobs[jobc].pid = pid;
        jobs[jobc].status = calloc(MAXJOBSTRLEN, sizeof(char));
        jobs[jobc].name = calloc(MAXJOBSTRLEN, sizeof(char));
        jobs[jobc].status = "Running";
        strcpy(jobs[jobc].name, name);
        printf("[%d] %d\n", jobc++, pid);
        return EXIT_SUCCESS;
    }
    else
    {
        return EXIT_FAILURE;
    }
}

int delete_job(int jid)
{
    if (jid < jobc)
    {
        for (int i = jid; i < jobc - 1; i++)
        {
            jobs[i] = jobs[i + 1];
        }
        jobc--;
        return 0;
    }
    else
    {
        return 1;
    }
}

// SIGCHLD handler
void update_job_state(int sig_num)
{
    int child_status, child_pid;
    do
    {
        child_pid = (int)waitpid(-1, &child_status, WNOHANG | WUNTRACED | WCONTINUED);
        int job_id = find_job(child_pid);
        if ((child_pid == -1) && (errno != ECHILD))
        {
            iothrow("Internal error : waitpid call failed in update_job_state");
        }
        else if ((child_pid > 0) && (job_id >= 0))
        {
            if (WIFSTOPPED(child_status))
            {
                jobs[job_id].status = "Suspended";
            }
            else if (WIFCONTINUED(child_status))
            {
                jobs[job_id].status = "Running";
            }
            else if (WIFEXITED(child_status))
            {
                jobs[job_id].status = "Done";
                printf("\n");
                print_job(job_id);
                prompt();
                fflush(stdout);
                delete_job(job_id);
            }
            else if (WIFSIGNALED(child_status))
            {
                jobs[job_id].status = "Unknown";
            }
        }
    } while (child_pid > 0);
}