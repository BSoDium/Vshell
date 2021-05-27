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

struct job bg_jobs[MAXJOBS];

int bg_jobc;

struct job fg_jobs[MAXCOMMANDS];

int fg_jobc;

int print_job(int index)
{
    if (index < bg_jobc)
    {
        char indicator; // indicates which job will be summoned when calling fg
        if (index == bg_jobc - 1)
        {
            indicator = '+';
        }
        else if (index == bg_jobc - 2)
        {
            indicator = '-';
        }
        else
        {
            indicator = ' ';
        }
        printf("[%d]%c  %s                 %s\n", index, indicator, get_status_string(bg_jobs[index].status), bg_jobs[index].name);
        return 0;
    }
    else
    {
        return 1;
    }
}

void show_jobs()
{
    for (int i = 0; i < bg_jobc; i++)
    {
        print_job(i);
    }
}

int find_job(int pid)
{
    for (int i = 0; i < bg_jobc; i++)
    {
        if (bg_jobs[i].pid == pid)
        {
            return i;
        }
    }
    return -1;
}

int append(struct job *array, int *count, int max, int pid, enum status status, char *name)
{
    if (*count < MAXJOBS)
    {
        array[*count].pid = pid;
        array[*count].name = calloc(MAXJOBSTRLEN, sizeof(char));
        array[*count].status = status;
        strcpy(array[*count].name, name);
        (*count)++;
        return EXIT_SUCCESS;
    }
    else
    {
        return EXIT_FAILURE;
    }
}

int append_bg_job(int pid, enum status status, char *name)
{
    int code = append(bg_jobs, &bg_jobc, MAXJOBS, pid, status, name);
    printf("[%d] %d\n", bg_jobc - 1, pid);
    return code;
}

int append_fg_job(int pid, enum status status, char *name)
{
    return append(fg_jobs, &fg_jobc, MAXCOMMANDS, pid, status, name);
}

int delete (struct job *array, int *count, int jid)
{
    if (jid < *count)
    {
        for (int i = jid; i < *count - 1; i++)
        {
            array[i] = array[i + 1];
        }
        (*count)--;
        return 0;
    }
    else
    {
        return 1;
    }
}

int delete_bg_job(int jid)
{
    return delete (bg_jobs, &bg_jobc, jid);
}

int delete_fg_job(int jid)
{
    return delete (fg_jobs, &fg_jobc, jid);
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
            iothrow("\nInternal error : waitpid call failed in update_job_state, code below :\n");
            printf("%dx%d@%s\n", job_id, child_pid, bg_jobs[job_id].name);
            exit(EXIT_FAILURE);
        }
        else if ((child_pid > 0) && (job_id >= 0))
        {
            if (WIFSTOPPED(child_status))
            {
                bg_jobs[job_id].status = Suspended;
            }
            else if (WIFCONTINUED(child_status))
            {
                bg_jobs[job_id].status = Running;
            }
            else if (WIFEXITED(child_status))
            {
                bg_jobs[job_id].status = Done;
                printf("\n");
                print_job(job_id);
                prompt();
                fflush(stdout);
                delete_bg_job(job_id);
            }
            else if (WIFSIGNALED(child_status))
            {
                bg_jobs[job_id].status = Unknown;
            }
        }
    } while (child_pid > 0);

    //     int child_status, child_pid;
    // for (int i = 0; i < bg_jobc; i++)
    // {
    //     int code = waitpid(bg_jobs[i].pid, &child_status, WNOHANG); // get job status
    //     if (code < 0)
    //     {
    //         iothrow("\nInternal error : waitpid call failed in update_job_state, code below :\n");
    //         printf("%dx%d@%s\n", i, bg_jobs[i].pid, bg_jobs[i].name);
    //         exit(EXIT_FAILURE);
    //     }
    //     if (WIFEXITED(child_status))
    //     {
    //         bg_jobs[i].status = Done;
    //         printf("\n");
    //         print_job(i);
    //         prompt();
    //         fflush(stdout);
    //         delete_bg_job(i);
    //     }
    //     else if (WIFSIGNALED(child_status))
    //     {
    //         bg_jobs[i].status = Unknown;
    //     }
    //     else if (WIFCONTINUED(child_status))
    //     {
    //         bg_jobs[i].status = Running;
    //     }
    //     else if (WIFSTOPPED(child_status))
    //     {
    //         bg_jobs[i].status = Suspended;
    //     }
    // }
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
