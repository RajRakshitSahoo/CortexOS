#include "scheduler.h"

/* ─── Demo job set ─── */
static SchedJob demo_jobs[] = {
    {"P1",10,3,0,0,0,0,0},
    {"P2",6, 1,1,0,0,0,0},
    {"P3",2, 4,2,0,0,0,0},
    {"P4",4, 2,3,0,0,0,0},
    {"P5",8, 5,4,0,0,0,0},
};
#define DEMO_N 5

static void reset_jobs(SchedJob *jobs, int n){
    for(int i=0;i<n;i++){ jobs[i].start=0; jobs[i].finish=0; jobs[i].waiting=0; jobs[i].turnaround=0; }
}

/* ─── FCFS ─── */
void sched_fcfs(SchedJob *jobs, int n){
    reset_jobs(jobs,n);
    int time=0;
    for(int i=0;i<n;i++){
        if(time<jobs[i].arrival) time=jobs[i].arrival;
        jobs[i].start=time;
        jobs[i].finish=time+jobs[i].burst;
        jobs[i].turnaround=jobs[i].finish-jobs[i].arrival;
        jobs[i].waiting=jobs[i].turnaround-jobs[i].burst;
        time=jobs[i].finish;
    }
}

/* ─── SJF (non-preemptive) ─── */
void sched_sjf(SchedJob *jobs, int n){
    reset_jobs(jobs,n);
    int done[64]={0};
    int time=0;
    for(int count=0;count<n;count++){
        int shortest=-1;
        for(int i=0;i<n;i++){
            if(!done[i]&&jobs[i].arrival<=time){
                if(shortest==-1||jobs[i].burst<jobs[shortest].burst) shortest=i;
            }
        }
        if(shortest==-1){ time++; count--; continue; }
        int i=shortest;
        jobs[i].start=time;
        jobs[i].finish=time+jobs[i].burst;
        jobs[i].turnaround=jobs[i].finish-jobs[i].arrival;
        jobs[i].waiting=jobs[i].turnaround-jobs[i].burst;
        time=jobs[i].finish;
        done[i]=1;
    }
}

/* ─── Priority (lower number = higher priority) ─── */
void sched_priority(SchedJob *jobs, int n){
    reset_jobs(jobs,n);
    int done[64]={0};
    int time=0;
    for(int count=0;count<n;count++){
        int best=-1;
        for(int i=0;i<n;i++){
            if(!done[i]&&jobs[i].arrival<=time){
                if(best==-1||jobs[i].priority<jobs[best].priority) best=i;
            }
        }
        if(best==-1){ time++; count--; continue; }
        int i=best;
        jobs[i].start=time;
        jobs[i].finish=time+jobs[i].burst;
        jobs[i].turnaround=jobs[i].finish-jobs[i].arrival;
        jobs[i].waiting=jobs[i].turnaround-jobs[i].burst;
        time=jobs[i].finish;
        done[i]=1;
    }
}

/* ─── Round Robin ─── */
void sched_rr(SchedJob *jobs, int n, int quantum){
    reset_jobs(jobs,n);
    int remaining[64]; for(int i=0;i<n;i++) remaining[i]=jobs[i].burst;
    int started[64]={0};
    int time=0, finished=0;
    while(finished<n){
        int any=0;
        for(int i=0;i<n;i++){
            if(remaining[i]>0&&jobs[i].arrival<=time){
                any=1;
                if(!started[i]){ jobs[i].start=time; started[i]=1; }
                int run=MIN(quantum,remaining[i]);
                time+=run; remaining[i]-=run;
                if(remaining[i]==0){
                    jobs[i].finish=time;
                    jobs[i].turnaround=jobs[i].finish-jobs[i].arrival;
                    jobs[i].waiting=jobs[i].turnaround-jobs[i].burst;
                    finished++;
                }
            }
        }
        if(!any) time++;
    }
}

/* ─── Print stats ─── */
void sched_print_stats(SchedJob *jobs, int n, const char *algo){
    printf("\n%s╔═══════════════════════════════════════════════════════╗%s\n",CLR_CYAN,CLR_RESET);
    printf("%s║  CPU Scheduling: %-37s║%s\n",CLR_CYAN,algo,CLR_RESET);
    printf("%s╚═══════════════════════════════════════════════════════╝%s\n\n",CLR_CYAN,CLR_RESET);
    printf("  %-6s %-6s %-6s %-6s %-8s %-8s %-10s %-10s\n",
           "Job","Burst","Prio","Arr","Start","Finish","Waiting","Turnaround");
    printf("  %s\n","-------------------------------------------------------------------");
    double avg_wait=0, avg_tat=0;
    for(int i=0;i<n;i++){
        printf("  %-6s %-6d %-6d %-6d %-8d %-8d %-10d %-10d\n",
               jobs[i].name,jobs[i].burst,jobs[i].priority,jobs[i].arrival,
               jobs[i].start,jobs[i].finish,jobs[i].waiting,jobs[i].turnaround);
        avg_wait+=jobs[i].waiting;
        avg_tat +=jobs[i].turnaround;
    }
    printf("\n  Avg Waiting Time   : %.2f\n",avg_wait/n);
    printf("  Avg Turnaround Time: %.2f\n\n",avg_tat/n);

    /* Gantt chart */
    printf("  Gantt Chart:\n  |");
    for(int i=0;i<n;i++) printf(" %-4s |",jobs[i].name);
    printf("\n  0");
    for(int i=0;i<n;i++) printf("    %3d",jobs[i].finish);
    printf("\n\n");
}

/* ─── Shell commands ─── */
void cmd_schedule(int argc, char **argv){
    if(argc<2){
        printf("\n  Usage: schedule <fcfs|sjf|priority|rr> [quantum]\n\n");
        return;
    }
    SchedJob jobs[DEMO_N];
    memcpy(jobs,demo_jobs,sizeof(demo_jobs));

    if(STR_EQ(argv[1],"fcfs")){
        sched_fcfs(jobs,DEMO_N);
        sched_print_stats(jobs,DEMO_N,"First Come First Served (FCFS)");
    } else if(STR_EQ(argv[1],"sjf")){
        sched_sjf(jobs,DEMO_N);
        sched_print_stats(jobs,DEMO_N,"Shortest Job First (SJF)");
    } else if(STR_EQ(argv[1],"priority")){
        sched_priority(jobs,DEMO_N);
        sched_print_stats(jobs,DEMO_N,"Priority Scheduling");
    } else if(STR_EQ(argv[1],"rr")){
        int q=(argc>=3)?atoi(argv[2]):3;
        sched_rr(jobs,DEMO_N,q);
        char label[64]; sprintf(label,"Round Robin (Quantum=%d)",q);
        sched_print_stats(jobs,DEMO_N,label);
    } else {
        printf("  Unknown algorithm. Use: fcfs | sjf | priority | rr\n");
    }
}

void cmd_kernel(int argc, char **argv){
    (void)argc;(void)argv;
    printf("\n%s╔═══════════════════════════════════════════╗%s\n",CLR_MAGENTA,CLR_RESET);
    printf("%s║          CORTEX KERNEL v1.0               ║%s\n",CLR_MAGENTA,CLR_RESET);
    printf("%s╚═══════════════════════════════════════════╝%s\n\n",CLR_MAGENTA,CLR_RESET);
    printf("  Scheduler      : Round Robin (default)\n");
    printf("  Memory Mgr     : Best Fit\n");
    printf("  VFS            : CortexFS (tree-based)\n");
    printf("  Security       : SHA-256 hashing\n");
    printf("  Network        : Simulated LAN\n\n");
    printf("  Commands:\n");
    printf("    schedule fcfs|sjf|priority|rr   - Run CPU scheduling sim\n");
    printf("    memory                           - Show memory map\n");
    printf("    ps                               - List processes\n\n");
}
