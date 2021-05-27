#ifndef __INTERNALS_H
#define __INTERNALS_H

// exit the shell
void exit_sh();
// change directory
int cd(char *path);
// stop (suspend) a job
int stop(int jid);
// resume job as background process
int bg(int jid);
// resume job as foreground process
int fg(int jid);
/**
 * Run internal commands.
 * returns 0 if an internal command was executed, 1 if not
 */
int run_internals(struct cmdline query);

#endif