#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "shellio.h"
#include "jobs.h"

enum status;

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
        printf("[%d]%c  %s                 %s\n", index, indicator, get_status_string(jobs[index].status), jobs[index].name);
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

int append_job(int pid, enum status status, char *name)
{
    if (jobc < MAXJOBS)
    {
        jobs[jobc].pid = pid;
        jobs[jobc].name = calloc(MAXJOBSTRLEN, sizeof(char));
        jobs[jobc].status = status;
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
    // int child_status, child_pid;
    // do
    // {
    //     child_pid = (int)waitpid(-1, &child_status, WNOHANG | WUNTRACED | WCONTINUED);
    //     int j_objd = find_job(child_pid);
    //     if ((child_pid == -1) && (errno != ECHILD))
    //     {
    //         iothrow("Internal error : waitpid call failed in update_job_state");
    //     }
    //     else if ((child_pid > 0) && (j_objd >= 0))
    //     {
    //         if (WIFSTOPPED(child_status))
    //         {
    //             jobs[j_objd].status = Suspended;
    //         }
    //         else if (WIFCONTINUED(child_status))
    //         {
    //             jobs[j_objd].status = Running;
    //         }
    //         else if (WIFEXITED(child_status))
    //         {
    //             jobs[j_objd].status = Done;
    //             printf("\n");
    //             print_job(j_objd);
    //             prompt();
    //             fflush(stdout);
    //             delete_job(j_objd);
    //         }
    //         else if (WIFSIGNALED(child_status))
    //         {
    //             jobs[j_objd].status = Unknown;
    //         }
    //     }
    // } while (child_pid > 0);

    int child_status, child_pid;
    for (int i = 0; i < jobc; i++)
    {
        struct job j_obj = jobs[i];
        int code = waitpid(j_obj.pid, &child_status, WNOHANG); // get job status
        if (code < 0)
        {
            iothrow("Internal error : waitpid call failed in update_job_state, code below :\n");
            printf("%dx%d@%s\n", i, j_obj.pid, j_obj.name);
            // exit(EXIT_FAILURE);
        }
        if (WIFSTOPPED(child_status))
        {
            j_obj.status = Suspended;
        }
        else if (WIFCONTINUED(child_status))
        {
            j_obj.status = Running;
        }
        else if (WIFEXITED(child_status))
        {
            j_obj.status = Done;
            printf("\n");
            print_job(i);
            prompt();
            fflush(stdout);
            delete_job(i);
        }
        else if (WIFSIGNALED(child_status))
        {
            j_obj.status = Unknown;
        }
    }
}

char *get_status_string(enum status s)
{
    char *strings[] = {
        "Running",
        "Suspended",
        "Done",
        "Unknown"};
    return strings[s];
}
