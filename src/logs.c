#include "logs.h"

static const char *level_str[] = {"INFO","WARN","ERROR","AUTH","FS","PROC"};

void logs_init(void){
    FILE *f = fopen(LOGS_FILE,"a");
    if(f){ fprintf(f,"--- CortexOS Session Started ---\n"); fclose(f); }
}

void log_write(LogLevel level, const char *message){
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char ts[32];
    strftime(ts,sizeof(ts),"%Y-%m-%d %H:%M:%S",tm_info);

    FILE *f = fopen(LOGS_FILE,"a");
    if(f){
        fprintf(f,"[%s] [%s] %s: %s\n",
                ts, level_str[level], g_current_user, message);
        fclose(f);
    }
}

void logs_print(int last_n){
    FILE *f = fopen(LOGS_FILE,"r");
    if(!f){ PRINT_ERR("Cannot open log file."); return; }

    /* collect lines */
    char lines[500][MAX_LOG_LINE];
    int count=0;
    while(count<500 && fgets(lines[count],MAX_LOG_LINE,f)) count++;
    fclose(f);

    int start = (last_n>0 && last_n<count) ? count-last_n : 0;

    printf("\n%s╔══════════════════════════════════════════════════════╗%s\n",CLR_CYAN,CLR_RESET);
    printf("%s║                   SYSTEM LOGS                       ║%s\n",CLR_CYAN,CLR_RESET);
    printf("%s╚══════════════════════════════════════════════════════╝%s\n\n",CLR_CYAN,CLR_RESET);

    for(int i=start;i<count;i++){
        /* colour by level keyword */
        const char *clr = CLR_WHITE;
        if(strstr(lines[i],"ERROR")) clr = CLR_RED;
        else if(strstr(lines[i],"WARN"))  clr = CLR_YELLOW;
        else if(strstr(lines[i],"AUTH"))  clr = CLR_MAGENTA;
        else if(strstr(lines[i],"FS"))    clr = CLR_BGREEN;
        else if(strstr(lines[i],"PROC"))  clr = CLR_BLUE;
        printf("  %s%s%s",clr,lines[i],CLR_RESET);
    }
    printf("\n");
}

void cmd_logs(int argc, char **argv){
    int n = 0;
    if(argc>=2) n = atoi(argv[1]);
    if(n<=0) n=50;
    logs_print(n);
}
