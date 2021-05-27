#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "shellio.h"

#define VERSION "0.3"
#define CWDBUFSIZE 255
#define CWDMAXLEN 30

// show the splash screen
void splash()
{
    printf("%ssh version %s%s\n", KBLU, KGRN, VERSION);
    printf("%sNow with useless colors !%s\n\n", KCYN, KWHT);
}

// display error messages
void iothrow(char *report, ...)
{
    va_list list;
    printf("%s", KRED);
    va_start(list, report);
    vprintf(report, list);
    va_end(list);
    printf("%s", KWHT);
}

// get the current working directory
char *cwd(void)
{
    char *buf = calloc(CWDBUFSIZE, sizeof(char));
    if (buf == NULL)
    {
        iothrow("Fatal error : malloc failure");
        exit(EXIT_FAILURE);
    }
    if (getcwd(buf, CWDBUFSIZE) == NULL)
    {
        iothrow("\nCould not retrieve current working directory\n(possible cause : string too long)\n");
    }
    int len = strlen(buf);
    if (len > CWDMAXLEN)
    {
        char *shortenedoutput = (char *)calloc(CWDMAXLEN + 5, sizeof(char));
        if (shortenedoutput == NULL)
        {
            iothrow("Fatal error : malloc failure");
            exit(EXIT_FAILURE);
        }
        strcpy(shortenedoutput, "[...]");
        strcat(shortenedoutput, buf + (len - CWDMAXLEN));
        return shortenedoutput;
    }
    return buf;
}

// ask the user for a query and process it
void prompt()
{
    // display prompt
    printf("%s%s@sh-%s%s:%s%s%s$ ", KYEL, getenv("USER"), VERSION, KWHT, KGRN, cwd(), KWHT);
}
