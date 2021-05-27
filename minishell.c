#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdbool.h>
#include <wait.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include "readcmd.h"
#include "handler.h"
#include "shellio.h"
#include "jobs.h"
#include "internals.h"

/**
 * Bug reports : 
 * -> kill 0 crashes the shell - wontfix
 */

// close a file descriptor and check the return code
int sclose(int fd)
{
    if (close(fd))
    {
        iothrow("Fatal error : File descriptor failed to close\n");
        exit(EXIT_FAILURE);
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

    // try running internal commands
    if (!run_internals(query))
    {
        return;
    }

    // execute query content if no internal command did match
    int index = 0;
    int pipes[MAXCOMMANDS][2];
    while (query.seq[index] != NULL)
    {
        char **command = query.seq[index];
        // open pipe
        pipe(pipes[index]);

        int child_status;
        struct job child;
        child.name = command[0];
        child.pid = fork();

        if (child.pid < 0)
        { // fork failure
            iothrow("Fatal error : Process creation failed and returned %d\n", child.pid);
            // close all pipes
            if (index > 0) // not the first command in query.seq
            {
                sclose(pipes[index - 1][0]);
                sclose(pipes[index - 1][1]);
            }
            if (query.seq[index + 1] != NULL) // not the last command in query.seq
            {
                sclose(pipes[index][0]);
                sclose(pipes[index][1]);
            }
            exit(EXIT_FAILURE);
        }
        else if (child.pid == 0)
        { // fork success, child code
            // reset signal handling behaviour
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            signal(SIGCHLD, SIG_DFL);
            // link pipes
            if (index > 0) // not the first command in query.seq
            {
                dup2(pipes[index - 1][0], 0);
                sclose(pipes[index - 1][0]);
                sclose(pipes[index - 1][1]);
            }
            else if (query.in != NULL) // in redirect
            {
                int fd = open(query.in, O_RDONLY, 0666);
                if (fd < 0)
                {
                    iothrow("Stdin : Invalid file name '%s' - operation cancelled\n", query.out);
                    exit(EXIT_FAILURE);
                }
                dup2(fd, 0);
            }

            if (query.seq[index + 1] != NULL) // not the last command in query.seq
            {
                dup2(pipes[index][1], 1);
                sclose(pipes[index][0]);
                sclose(pipes[index][1]);
            }
            else if (query.out != NULL) // out redirect
            {
                int fd = open(query.out, O_WRONLY | O_CREAT, 0666);
                if (fd < 0)
                {
                    iothrow("Stdout : Invalid file name '%s' - operation cancelled\n", query.out);
                    exit(EXIT_FAILURE);
                }
                dup2(fd, 1);
            }

            // execute child code
            execvp(command[0], command);
            printf("%sUnknown command%s : %s\n", KRED, KWHT, command[0]);
            exit(EXIT_FAILURE);
        }

        // close old pipes (parent)
        if (index > 0)
        {
            sclose(pipes[index - 1][0]);
            sclose(pipes[index - 1][1]);
        }

        // check if query must be run in background
        if (query.backgrounded == NULL)
        {
            append_fg_job(child.pid, Running, child.name);
        }
        else
        {
            append_bg_job(child.pid, Running, child.name);
        }

        index++;
    }

    if (query.backgrounded == NULL)
    { // wait for children to finish their job
        for (int i = 0; i < fg_jobc; i++)
        {
            int status;
            waitpid(fg_jobs[i].pid, &status, WUNTRACED);
            delete_fg_job(fg_jobs[i].pid);
        }
    }
    return;
}

int main(int argc, char *argv[])
{
    bool exit = false;

    // Setup handlers
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigstop_handler);

    // sigaction causes a segmentation error (that's why it is commented)
    // struct sigaction intact, tstpact;

    // intact.sa_handler = sigint_handler;
    // sigemptyset(&intact.sa_mask);
    // intact.sa_flags = 0;
    // tstpact.sa_handler = sigstop_handler;
    // sigemptyset(&tstpact.sa_mask);
    // tstpact.sa_flags = 0;

    // sigaction(SIGINT, &intact, NULL);
    // sigaction(SIGTSTP, &tstpact, NULL);

    splash();
    while (!exit)
    {
        prompt();
        struct cmdline *query = readcmd();
        run(*query);
    }
    return EXIT_SUCCESS;
}
