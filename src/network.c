#include "network.h"
#include "logs.h"
#include "filesystem.h"

/* ══════════════════════════════════════
   VIRTUAL BROWSER
   ══════════════════════════════════════ */
typedef struct { const char *url; const char *title; const char *body; } WebPage;

static WebPage pages[] = {
    {"cortexmail.os","CortexMail - Inbox",
     "  Welcome to CortexMail!\n"
     "  ┌───────────────────────────────────────────┐\n"
     "  │  [Compose]  [Inbox]  [Sent]  [Trash]     │\n"
     "  ├───────────────────────────────────────────┤\n"
     "  │  No new messages.                         │\n"
     "  └───────────────────────────────────────────┘\n"
     "  Use the 'mail' command in shell for full email.\n"},
    {"cortexnews.os","CortexNews - Today's Headlines",
     "  ══ TOP STORIES ══════════════════════════════\n"
     "  [1] CortexOS v2.0 Released - Faster Boot Times!\n"
     "  [2] Quantum Computing Breakthrough Announced\n"
     "  [3] New AI Chip Achieves 10x Performance Gain\n"
     "  [4] Space Mission Completes Successful Landing\n"
     "  [5] Global Tech Summit Starts Next Week\n"
     "  ═════════════════════════════════════════════\n"},
    {"cortexstore.os","CortexStore - App Marketplace",
     "  Featured Apps:\n"
     "  ┌─────────────────────────────────────────────┐\n"
     "  │  Calculator   [FREE]   install calculator   │\n"
     "  │  Notes        [FREE]   install notes        │\n"
     "  │  Snake Game   [FREE]   install snake        │\n"
     "  │  Chess        [FREE]   install chess        │\n"
     "  │  Sudoku       [FREE]   install sudoku       │\n"
     "  │  Minesweeper  [FREE]   install minesweeper  │\n"
     "  └─────────────────────────────────────────────┘\n"},
    {NULL,NULL,NULL}
};

void cmd_browser(int argc, char **argv){
    (void)argc;
    char url[128]="";

    printf("\n%s╔══════════════════════════════════════════════════╗%s\n",CLR_CYAN,CLR_RESET);
    printf("%s║              CORTEX BROWSER v1.0                ║%s\n",CLR_CYAN,CLR_RESET);
    printf("%s╚══════════════════════════════════════════════════╝%s\n",CLR_CYAN,CLR_RESET);
    printf("  Known sites: cortexmail.os  cortexnews.os  cortexstore.os\n\n");

    while(1){
        printf(CLR_CYAN "  URL> " CLR_RESET); fflush(stdout);
        if(!fgets(url,sizeof(url),stdin)) break;
        url[strcspn(url,"\r\n")]='\0';
        if(STR_EQ(url,"exit")||STR_EQ(url,"quit")||STR_EQ(url,"q")) break;
        if(STR_EMPTY(url)) continue;

        int found=0;
        for(int i=0;pages[i].url;i++){
            if(STR_EQ(url,pages[i].url)){
                printf("\n%s[ %s ]%s\n",CLR_BYELLOW,pages[i].title,CLR_RESET);
                printf("%s\n",pages[i].body);
                found=1; break;
            }
        }
        if(!found) printf(CLR_RED "  404 Not Found: %s\n" CLR_RESET,url);
    }
    printf("  Browser closed.\n\n");
    log_write(LOG_INFO,"Browser session ended.");
}

/* ══════════════════════════════════════
   EMAIL SYSTEM
   ══════════════════════════════════════ */
typedef struct {
    char from[MAX_USERNAME];
    char to[MAX_USERNAME];
    char subject[128];
    char body[512];
    char timestamp[32];
} Email;

static void mail_path(const char *user, char *path){
    sprintf(path,"%s%s.mail",MAIL_DIR,user);
}

static void save_email(const Email *em){
    char path[MAX_PATH]; mail_path(em->to,path);
    FILE *f=fopen(path,"ab");
    if(!f) return;
    fwrite(em,sizeof(Email),1,f);
    fclose(f);
}

void cmd_send_mail(int argc, char **argv){
    if(!g_logged_in){ PRINT_ERR("Not logged in."); return; }
    Email em; memset(&em,0,sizeof(em));
    strncpy(em.from,g_current_user,MAX_USERNAME-1);
    if(argc>=2) strncpy(em.to,argv[1],MAX_USERNAME-1);
    else { printf("  To      : "); fflush(stdout); fgets(em.to,sizeof(em.to),stdin); em.to[strcspn(em.to,"\r\n")]='\0'; }
    printf("  Subject : "); fflush(stdout); fgets(em.subject,sizeof(em.subject),stdin); em.subject[strcspn(em.subject,"\r\n")]='\0';
    printf("  Body    : "); fflush(stdout); fgets(em.body,sizeof(em.body),stdin); em.body[strcspn(em.body,"\r\n")]='\0';
    time_t t=time(NULL); struct tm *tm=localtime(&t); strftime(em.timestamp,32,"%Y-%m-%d %H:%M",tm);
    save_email(&em);
    printf(CLR_BGREEN "  Mail sent to '%s'.\n" CLR_RESET,em.to);
    log_write(LOG_INFO,"Email sent.");
}

void cmd_inbox(int argc, char **argv){
    (void)argc;(void)argv;
    if(!g_logged_in){ PRINT_ERR("Not logged in."); return; }
    char path[MAX_PATH]; mail_path(g_current_user,path);
    FILE *f=fopen(path,"rb");
    if(!f){ printf("\n  Inbox is empty.\n\n"); return; }
    Email em; int idx=1;
    printf("\n%s╔════════════════════════════════════════════════════╗%s\n",CLR_CYAN,CLR_RESET);
    printf("%s║  INBOX: %-43s║%s\n",CLR_CYAN,g_current_user,CLR_RESET);
    printf("%s╚════════════════════════════════════════════════════╝%s\n\n",CLR_CYAN,CLR_RESET);
    while(fread(&em,sizeof(Email),1,f)==1){
        printf("  [%d] From: %-16s  Subject: %-30s  %s\n",idx++,em.from,em.subject,em.timestamp);
    }
    fclose(f);
    /* Let user read */
    printf("\n  Read email number (0 to exit): "); fflush(stdout);
    char buf[8]; fgets(buf,sizeof(buf),stdin);
    int n=atoi(buf);
    if(n<=0||n>=idx){ printf("\n"); return; }
    f=fopen(path,"rb"); if(!f) return;
    int ci=1;
    while(fread(&em,sizeof(Email),1,f)==1){
        if(ci==n){
            printf("\n  From   : %s\n  To     : %s\n  Date   : %s\n  Subject: %s\n\n  %s\n\n",
                   em.from,em.to,em.timestamp,em.subject,em.body);
            break;
        }
        ci++;
    }
    fclose(f);
}

void cmd_mail(int argc, char **argv){
    (void)argc;(void)argv;
    printf("\n  mail commands:\n    send <user>  - Send email\n    inbox        - View inbox\n\n");
}

/* ══════════════════════════════════════
   LAN SIMULATION
   ══════════════════════════════════════ */
static int connected = 0;
static char connected_to[MAX_USERNAME]="";

void cmd_connect(int argc, char **argv){
    if(argc<2){ printf("Usage: connect <hostname>\n"); return; }
    strncpy(connected_to,argv[1],MAX_USERNAME-1);
    connected=1;
    printf(CLR_BGREEN "  Connected to %s\n" CLR_RESET, argv[1]);
    log_write(LOG_INFO,"LAN connection established.");
}

void cmd_chat(int argc, char **argv){
    (void)argc;(void)argv;
    if(!connected){ PRINT_ERR("Not connected. Use 'connect <host>' first."); return; }
    printf("\n  Chat with %s (type 'exit' to quit):\n",connected_to);
    char msg[256];
    while(1){
        printf(CLR_CYAN "  you> " CLR_RESET); fflush(stdout);
        if(!fgets(msg,sizeof(msg),stdin)) break;
        msg[strcspn(msg,"\r\n")]='\0';
        if(STR_EQ(msg,"exit")) break;
        /* simulate echo */
        printf(CLR_YELLOW "  %s> " CLR_RESET "[echo] %s\n",connected_to,msg);
    }
    printf("  Chat ended.\n\n");
}

void cmd_disconnect(int argc, char **argv){
    (void)argc;(void)argv;
    if(!connected){ printf("  Not connected.\n"); return; }
    printf("  Disconnected from %s.\n",connected_to);
    connected=0; connected_to[0]='\0';
}
