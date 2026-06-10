#ifndef PROCESS_H
#define PROCESS_H

#include "common.h"

typedef enum { PROC_READY=0, PROC_RUNNING=1, PROC_SLEEPING=2, PROC_TERMINATED=3 } ProcState;

typedef struct Process {
    int        pid;
    char       name[64];
    char       owner[MAX_USERNAME];
    ProcState  state;
    int        priority;    /* 1-10 */
    int        burst_time;  /* ms, simulated */
    int        arrival;
    time_t     started;
    struct Process *next;
} Process;

void     proc_init(void);
Process *proc_create(const char *name, int burst, int priority);
int      proc_kill(int pid);
void     proc_list(void);
Process *proc_find(int pid);

void cmd_run(int argc, char **argv);
void cmd_ps(int argc, char **argv);
void cmd_kill_cmd(int argc, char **argv);

#endif
