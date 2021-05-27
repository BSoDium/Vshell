#include <wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "jobs.h"
#include "readcmd.h"
#include "shellio.h"

void exit_sh()
{
    int jobs_killed = 0;
    for (int i = 0; i < bg_jobc; i++)
    {
        if (bg_jobs[i].status == Suspended || bg_jobs[i].status == Running)
        {
            kill(bg_jobs[i].pid, SIGINT);
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

int cd(char *path)
{
    return chdir(path);
}

int stop(int jid)
{
    if (jid < bg_jobc)
    {
        int code = kill(bg_jobs[jid].pid, SIGSTOP);
        return code;
    }
    else
    {
        return 1;
    }
}

int bg(int jid)
{
    if (jid < bg_jobc)
    {
        int code = kill(bg_jobs[jid].pid, SIGCONT);
        return code;
    }
    else
    {
        return 1;
    }
}

int fg(int jid)
{
    int status;
    if (jid < bg_jobc)
    {
        int code = kill(bg_jobs[jid].pid, SIGCONT);
        append_fg_job(bg_jobs[jid].pid, Running, bg_jobs[jid].name);
        waitpid(bg_jobs[jid].pid, &status, WUNTRACED);
        delete_fg_job(bg_jobs[jid].pid);
        return code;
    }
    else
    {
        return 1;
    }
}

int run_internals(struct cmdline query)
{
    // strcmp raises a segmentation fault on empty string comparison
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
