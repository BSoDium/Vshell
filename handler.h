#ifndef __HANDLER_H
#define __HANDLER_H

// shell process sigint signal handler
void sigint_handler(int sig_num);
// shell process sigstop signal handler
void sigstop_handler(int signum);

#endif