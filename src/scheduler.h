#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "common.h"
#include "process.h"

typedef struct {
    char  name[64];
    int   burst;
    int   priority;
    int   arrival;
    int   start;
    int   finish;
    int   waiting;
    int   turnaround;
} SchedJob;

void sched_fcfs(SchedJob *jobs, int n);
void sched_sjf(SchedJob *jobs, int n);
void sched_priority(SchedJob *jobs, int n);
void sched_rr(SchedJob *jobs, int n, int quantum);
void sched_print_stats(SchedJob *jobs, int n, const char *algo);

void cmd_schedule(int argc, char **argv);
void cmd_kernel(int argc, char **argv);

#endif
