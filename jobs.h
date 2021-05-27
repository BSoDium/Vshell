#ifndef __JOBS_H
#define __JOBS_H

#define MAXJOBS 1024
#define MAXJOBSTRLEN 20

/* enum containing the possible values for job statuses */
enum status
{
    Running,
    Suspended,
    Done,
    Unknown
};

/* job struct containing its pid, status and name */
struct job
{
    int pid;            // job pid
    enum status status; // current job status
    char *name;         // job name
};
/* jobs running at the moment */
extern struct job jobs[MAXJOBS];

/* pid associated to the job currently running in foreground */
extern struct job *fg_job;

/* job count */
extern int jobc;

int print_job(int index);
void show_jobs();
int find_job(int pid);
int append_job(int pid, enum status status, char *name);
int delete_job(int jid);
void update_job_state(int sig_num);
char *get_status_string(enum status s);

#endif