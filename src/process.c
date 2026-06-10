#include "process.h"
#include "logs.h"

static Process *proc_head = NULL;
static int next_pid = 1000;

/* ─── System processes ─── */
static const char *sys_procs[][2] = {
    {"kernel","System Kernel"},
    {"init","Init Process"},
    {"scheduler","CPU Scheduler"},
    {"memman","Memory Manager"},
    {"logger","System Logger"},
    {NULL,NULL}
};

void proc_init(void){
    /* seed system processes */
    for(int i=0;sys_procs[i][0];i++){
        Process *p=(Process*)calloc(1,sizeof(Process));
        p->pid=i+1;
        strncpy(p->name,sys_procs[i][0],63);
        strcpy(p->owner,"root");
        p->state=PROC_RUNNING;
        p->priority=10;
        p->burst_time=0;
        p->started=time(NULL);
        p->next=proc_head;
        proc_head=p;
        if(i+1>=next_pid) next_pid=i+2;
    }
    next_pid=1000;
}

Process *proc_create(const char *name, int burst, int priority){
    Process *p=(Process*)calloc(1,sizeof(Process));
    if(!p) return NULL;
    p->pid=next_pid++;
    strncpy(p->name,name,63);
    strncpy(p->owner,g_current_user,MAX_USERNAME-1);
    p->state=PROC_RUNNING;
    p->priority=(priority<1)?1:(priority>10?10:priority);
    p->burst_time=burst>0?burst:100;
    p->arrival=(int)time(NULL);
    p->started=time(NULL);
    p->next=proc_head;
    proc_head=p;

    char msg[128]; sprintf(msg,"Process started: %s [PID %d]",name,p->pid);
    log_write(LOG_PROC,msg);
    return p;
}

int proc_kill(int pid){
    Process *cur=proc_head, *prev=NULL;
    while(cur){
        if(cur->pid==pid){
            /* only owner or admin can kill */
            if(!STR_EQ(cur->owner,g_current_user)&&!g_is_admin){
                PRINT_ERR("Permission denied."); return 0;
            }
            if(cur->state==PROC_TERMINATED){ PRINT_ERR("Process already terminated."); return 0; }
            cur->state=PROC_TERMINATED;
            /* remove from list */
            if(prev) prev->next=cur->next;
            else proc_head=cur->next;
            char msg[64]; sprintf(msg,"Process killed: PID %d",pid);
            log_write(LOG_PROC,msg);
            free(cur);
            return 1;
        }
        prev=cur; cur=cur->next;
    }
    PRINT_ERR("PID not found."); return 0;
}

Process *proc_find(int pid){
    for(Process *p=proc_head;p;p=p->next) if(p->pid==pid) return p;
    return NULL;
}

void proc_list(void){
    const char *state_str[]={"READY","RUNNING","SLEEPING","TERMINATED"};
    const char *state_clr[]={CLR_YELLOW,CLR_BGREEN,CLR_BLUE,CLR_RED};
    printf("\n  %-8s %-20s %-16s %-10s %-5s %-6s\n","PID","Name","Owner","State","Prio","Burst");
    printf("  %s\n","------------------------------------------------------------------------");
    for(Process *p=proc_head;p;p=p->next){
        printf("  %-8d %-20s %-16s %s%-10s%s %-5d %-6d\n",
               p->pid,p->name,p->owner,
               state_clr[p->state],state_str[p->state],CLR_RESET,
               p->priority,p->burst_time);
    }
    printf("\n");
}

/* ─── Shell commands ─── */
void cmd_run(int argc, char **argv){
    if(argc<2){ printf("Usage: run <name> [burst_time] [priority]\n"); return; }
    int burst=(argc>=3)?atoi(argv[2]):100;
    int prio =(argc>=4)?atoi(argv[3]):5;
    Process *p=proc_create(argv[1],burst,prio);
    if(p) printf(CLR_BGREEN "  Process '%s' started with PID %d\n" CLR_RESET,p->name,p->pid);
}

void cmd_ps(int argc, char **argv){ (void)argc;(void)argv; proc_list(); }

void cmd_kill_cmd(int argc, char **argv){
    if(argc<2){ printf("Usage: kill <pid>\n"); return; }
    int pid=atoi(argv[1]);
    if(proc_kill(pid)) printf("  Process %d terminated.\n",pid);
}
