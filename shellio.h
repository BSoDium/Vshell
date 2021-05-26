#ifndef __SHELLIO_H
#define __SHELLIO_H

#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KBLU "\x1B[34m"
#define KWHT "\x1B[37m"
#define KYEL "\x1B[33m"
#define KCYN "\x1B[36m"

void splash();
char *cwd(void);
void iothrow(char *report, ...);
void prompt();

#endif