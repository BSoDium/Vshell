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

#define MAXCHILDREN 255

/**
 * Bug reports : 
 * -> kill 0 crashes the shell - wontfix
 */

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

        // in redirect
        if (query.in != NULL)
        {
            int fd = open(query.in, O_RDONLY, 0666);
            if (fd < 0)
            {
                iothrow("Stdin : Invalid file name '%s' - operation cancelled\n", query.out);
                exit(EXIT_FAILURE);
            }
            dup2(fd, 0);
        }
        // out redirect
        if (query.out != NULL)
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
        execvp(query.seq[0][0], query.seq[0]);
        printf("%sUnknown command%s : %s\n", KRED, KWHT, query.seq[0][0]);
        exit(EXIT_FAILURE);
    }

    if (query.backgrounded == NULL)
    {
        fg_job = malloc(sizeof(struct job));
        fg_job->pid = child;
        if (query.seq[0] != NULL)
        {
            fg_job->name = query.seq[0][0];
        }
        else
        {
            fg_job->name = "empty job";
        }
        waitpid(child, &status, WUNTRACED);
        fg_job = NULL;
    }
    else
    {
        if (query.seq[0] != NULL)
        {
            append_job(child, Running, query.seq[0][0]);
        }
        else
        {
            iothrow("illegal use of '&'\n");
        }
    }
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
