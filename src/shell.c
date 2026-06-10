#include "shell.h"
#include "users.h"
#include "filesystem.h"
#include "process.h"
#include "scheduler.h"
#include "memory.h"
#include "security.h"
#include "network.h"
#include "database.h"
#include "blockchain.h"
#include "backup.h"
#include "apps.h"
#include "logs.h"

/* ─── Parse command line into argc/argv ─── */
static int parse_cmdline(char *line, char **argv, int max_args){
    int argc=0;
    char *p=line;
    while(*p&&argc<max_args){
        while(*p==' '||*p=='\t') p++;
        if(!*p) break;
        if(*p=='"'){
            p++;
            argv[argc++]=p;
            while(*p&&*p!='"') p++;
            if(*p) *p++='\0';
        } else {
            argv[argc++]=p;
            while(*p&&*p!=' '&&*p!='\t') p++;
            if(*p) *p++='\0';
        }
    }
    return argc;
}

/* ─── Help ─── */
void shell_help(void){
    printf("\n%s╔══════════════════════════════════════════════════════════════╗%s\n",CLR_CYAN,CLR_RESET);
    printf("%s║                  CORTEX OS  -  HELP                         ║%s\n",CLR_CYAN,CLR_RESET);
    printf("%s╠══════════════════════════════════════════════════════════════╣%s\n",CLR_CYAN,CLR_RESET);
    printf("%s║  USER                                                        ║%s\n",CLR_CYAN,CLR_RESET);
    printf("  signup       signup [user]           Create account\n");
    printf("  login        login [user]            Login\n");
    printf("  logout                               Logout\n");
    printf("  whoami                               Show current user\n");
    printf("  passwd                               Change password\n");
    printf("%s║  FILE SYSTEM                                                  ║%s\n",CLR_CYAN,CLR_RESET);
    printf("  ls           ls [-l] [dir]          List directory\n");
    printf("  cd           cd <dir>               Change directory\n");
    printf("  pwd                                  Print working directory\n");
    printf("  mkdir        mkdir <dir>             Create directory\n");
    printf("  touch        touch <file>            Create file\n");
    printf("  rm           rm <path>              Remove file/dir\n");
    printf("  mv           mv <src> <dst>         Move/rename\n");
    printf("  cp           cp <src> <dst>         Copy file\n");
    printf("  cat          cat <file>             Print file contents\n");
    printf("  edit         edit <file>            Open text editor\n");
    printf("  tree         tree [dir]             Show directory tree\n");
    printf("  chmod        chmod <perm> <file>    Change permissions\n");
    printf("  chown        chown <user> <file>    Change owner\n");
    printf("%s║  PROCESS                                                      ║%s\n",CLR_CYAN,CLR_RESET);
    printf("  run          run <name> [burst] [prio] Start process\n");
    printf("  ps                                   List processes\n");
    printf("  kill         kill <pid>             Terminate process\n");
    printf("%s║  SCHEDULING                                                   ║%s\n",CLR_CYAN,CLR_RESET);
    printf("  schedule     schedule <fcfs|sjf|priority|rr> [quantum]\n");
    printf("  kernel                               Kernel info\n");
    printf("%s║  MEMORY                                                       ║%s\n",CLR_CYAN,CLR_RESET);
    printf("  memory                               Show memory map\n");
    printf("  alloc        alloc <kb> <label> [first|best|worst]\n");
    printf("  free         free <block_id>        Free memory block\n");
    printf("%s║  SECURITY                                                     ║%s\n",CLR_CYAN,CLR_RESET);
    printf("  security                             Security module info\n");
    printf("  checkpass    checkpass <pass>       Analyze password\n");
    printf("  verify       verify <file>          SHA-256 checksum\n");
    printf("%s║  NETWORK                                                      ║%s\n",CLR_CYAN,CLR_RESET);
    printf("  browser                              Virtual browser\n");
    printf("  send         send [user]            Send email\n");
    printf("  inbox                               View inbox\n");
    printf("  connect      connect <host>         LAN connect\n");
    printf("  chat                                 LAN chat\n");
    printf("  disconnect                           Disconnect LAN\n");
    printf("%s║  DATABASE                                                     ║%s\n",CLR_CYAN,CLR_RESET);
    printf("  db                                   Open SQL console\n");
    printf("%s║  BLOCKCHAIN                                                   ║%s\n",CLR_CYAN,CLR_RESET);
    printf("  blockchain                           Show blockchain\n");
    printf("  mine         mine \"<data>\"          Mine a block\n");
    printf("  verify                               Verify chain\n");
    printf("  transaction  transaction <f> <t> <amt>\n");
    printf("%s║  BACKUP                                                       ║%s\n",CLR_CYAN,CLR_RESET);
    printf("  backup                               Create snapshot\n");
    printf("  restore      restore <snapshot>    Restore snapshot\n");
    printf("  recover                              List snapshots\n");
    printf("%s║  APPS                                                         ║%s\n",CLR_CYAN,CLR_RESET);
    printf("  calc                                 Calculator\n");
    printf("  calendar                             Calendar\n");
    printf("  notes                                Notes app\n");
    printf("  todo                                 Todo manager\n");
    printf("  snake                                Snake game\n");
    printf("  chess                                Chess board\n");
    printf("  sudoku                               Sudoku\n");
    printf("  minesweeper                          Minesweeper\n");
    printf("  store                                App store\n");
    printf("  install      install <pkg>          Install package\n");
    printf("  remove       remove <pkg>           Remove package\n");
    printf("  packages                             List packages\n");
    printf("%s║  VM                                                           ║%s\n",CLR_CYAN,CLR_RESET);
    printf("  vm           vm create|start|stop|list\n");
    printf("%s║  SYSTEM                                                       ║%s\n",CLR_CYAN,CLR_RESET);
    printf("  history                              Command history\n");
    printf("  logs         logs [n]               Show last n log lines\n");
    printf("  clear                                Clear screen\n");
    printf("  sysinfo                              System information\n");
    printf("  help                                 This help page\n");
    printf("  exit / logout                        Logout\n");
    printf("%s╚══════════════════════════════════════════════════════════════╝%s\n\n",CLR_CYAN,CLR_RESET);
}

/* ─── sysinfo ─── */
static void cmd_sysinfo(void){
    time_t t=time(NULL);
    char ts[32]; struct tm *tm=localtime(&t); strftime(ts,32,"%Y-%m-%d %H:%M:%S",tm);
    printf("\n%s╔═══════════════════════════════════════════╗%s\n",CLR_MAGENTA,CLR_RESET);
    printf("%s║        CORTEX OS - SYSTEM INFO            ║%s\n",CLR_MAGENTA,CLR_RESET);
    printf("%s╚═══════════════════════════════════════════╝%s\n\n",CLR_MAGENTA,CLR_RESET);
    printf("  OS Name    : CortexOS v1.0\n");
    printf("  Kernel     : CortexKernel 1.0.0\n");
    printf("  Platform   : Windows (MinGW/GCC)\n");
    printf("  User       : %s%s%s\n",CLR_BGREEN,g_current_user,CLR_RESET);
    printf("  Role       : %s\n",g_is_admin?"Administrator":"User");
    printf("  Time       : %s\n",ts);
    printf("  Shell      : CortexShell 1.0\n");
    printf("  CWD        : %s\n\n",g_current_dir);
}

/* ─── Dispatcher ─── */
void shell_dispatch(char *cmdline){
    if(!cmdline||STR_EMPTY(cmdline)) return;
    /* trim newline */
    cmdline[strcspn(cmdline,"\r\n")]='\0';
    if(STR_EMPTY(cmdline)) return;

    history_push(cmdline);

    char *argv[MAX_ARGS]; int argc;
    /* make a copy so strtok doesn't corrupt */
    char buf[MAX_CMD]; strncpy(buf,cmdline,MAX_CMD-1);
    argc=parse_cmdline(buf,argv,MAX_ARGS);
    if(argc==0) return;

    char *cmd=argv[0];

    /* ── User ── */
    if(STR_EQ(cmd,"signup"))       { cmd_signup(argc,argv); return; }
    if(STR_EQ(cmd,"login"))        { cmd_login(argc,argv); return; }
    if(STR_EQ(cmd,"logout")||STR_EQ(cmd,"exit")) { user_logout(); return; }
    if(STR_EQ(cmd,"whoami"))       { cmd_whoami(argc,argv); return; }
    if(STR_EQ(cmd,"passwd"))       { cmd_passwd(argc,argv); return; }
    if(STR_EQ(cmd,"users"))        { users_list(); return; }

    /* ── Require login for most commands ── */
    if(!g_logged_in){
        printf(CLR_YELLOW "  Please login first. Commands: login | signup\n" CLR_RESET);
        return;
    }

    /* ── Filesystem ── */
    if(STR_EQ(cmd,"ls"))           { cmd_ls(argc,argv); return; }
    if(STR_EQ(cmd,"cd"))           { cmd_cd(argc,argv); return; }
    if(STR_EQ(cmd,"pwd"))          { cmd_pwd(argc,argv); return; }
    if(STR_EQ(cmd,"mkdir"))        { cmd_mkdir(argc,argv); return; }
    if(STR_EQ(cmd,"touch"))        { cmd_touch(argc,argv); return; }
    if(STR_EQ(cmd,"rm"))           { cmd_rm(argc,argv); return; }
    if(STR_EQ(cmd,"mv"))           { cmd_mv(argc,argv); return; }
    if(STR_EQ(cmd,"cp"))           { cmd_cp(argc,argv); return; }
    if(STR_EQ(cmd,"cat"))          { cmd_cat(argc,argv); return; }
    if(STR_EQ(cmd,"edit"))         { cmd_edit(argc,argv); return; }
    if(STR_EQ(cmd,"tree"))         { cmd_tree(argc,argv); return; }
    if(STR_EQ(cmd,"chmod"))        { cmd_chmod(argc,argv); return; }
    if(STR_EQ(cmd,"chown"))        { cmd_chown(argc,argv); return; }

    /* ── Process ── */
    if(STR_EQ(cmd,"run"))          { cmd_run(argc,argv); return; }
    if(STR_EQ(cmd,"ps"))           { cmd_ps(argc,argv); return; }
    if(STR_EQ(cmd,"kill"))         { cmd_kill_cmd(argc,argv); return; }

    /* ── Scheduling ── */
    if(STR_EQ(cmd,"schedule"))     { cmd_schedule(argc,argv); return; }
    if(STR_EQ(cmd,"kernel"))       { cmd_kernel(argc,argv); return; }

    /* ── Memory ── */
    if(STR_EQ(cmd,"memory"))       { cmd_memory(argc,argv); return; }
    if(STR_EQ(cmd,"alloc"))        { cmd_alloc(argc,argv); return; }
    if(STR_EQ(cmd,"free"))         { cmd_free_mem(argc,argv); return; }

    /* ── Security ── */
    if(STR_EQ(cmd,"security"))     { cmd_security(argc,argv); return; }
    if(STR_EQ(cmd,"checkpass"))    { cmd_checkpass(argc,argv); return; }
    if(STR_EQ(cmd,"verify"))       { cmd_bc_verify(argc,argv); return; }

    /* ── Network ── */
    if(STR_EQ(cmd,"browser"))      { cmd_browser(argc,argv); return; }
    if(STR_EQ(cmd,"mail"))         { cmd_mail(argc,argv); return; }
    if(STR_EQ(cmd,"send"))         { cmd_send_mail(argc,argv); return; }
    if(STR_EQ(cmd,"inbox"))        { cmd_inbox(argc,argv); return; }
    if(STR_EQ(cmd,"connect"))      { cmd_connect(argc,argv); return; }
    if(STR_EQ(cmd,"chat"))         { cmd_chat(argc,argv); return; }
    if(STR_EQ(cmd,"disconnect"))   { cmd_disconnect(argc,argv); return; }

    /* ── Database ── */
    if(STR_EQ(cmd,"db"))           { cmd_db(argc,argv); return; }

    /* ── Blockchain ── */
    if(STR_EQ(cmd,"blockchain"))   { cmd_blockchain(argc,argv); return; }
    if(STR_EQ(cmd,"mine"))         { cmd_mine(argc,argv); return; }
    if(STR_EQ(cmd,"transaction"))  { cmd_transaction(argc,argv); return; }

    /* ── Backup ── */
    if(STR_EQ(cmd,"backup"))       { cmd_backup(argc,argv); return; }
    if(STR_EQ(cmd,"restore"))      { cmd_restore(argc,argv); return; }
    if(STR_EQ(cmd,"recover"))      { cmd_recover(argc,argv); return; }

    /* ── Apps ── */
    if(STR_EQ(cmd,"calc"))         { cmd_calc(argc,argv); return; }
    if(STR_EQ(cmd,"calendar"))     { cmd_calendar_cmd(argc,argv); return; }
    if(STR_EQ(cmd,"notes"))        { cmd_notes(argc,argv); return; }
    if(STR_EQ(cmd,"todo"))         { cmd_todo(argc,argv); return; }
    if(STR_EQ(cmd,"snake"))        { cmd_snake(argc,argv); return; }
    if(STR_EQ(cmd,"chess"))        { cmd_chess(argc,argv); return; }
    if(STR_EQ(cmd,"sudoku"))       { cmd_sudoku(argc,argv); return; }
    if(STR_EQ(cmd,"minesweeper"))  { cmd_minesweeper(argc,argv); return; }
    if(STR_EQ(cmd,"store"))        { cmd_store(argc,argv); return; }
    if(STR_EQ(cmd,"install"))      { cmd_install(argc,argv); return; }
    if(STR_EQ(cmd,"remove"))       { cmd_remove(argc,argv); return; }
    if(STR_EQ(cmd,"update"))       { cmd_update(argc,argv); return; }
    if(STR_EQ(cmd,"packages"))     { cmd_packages(argc,argv); return; }

    /* ── VM ── */
    if(STR_EQ(cmd,"vm"))           { cmd_vm(argc,argv); return; }

    /* ── System ── */
    if(STR_EQ(cmd,"history"))      { cmd_history(argc,argv); return; }
    if(STR_EQ(cmd,"logs"))         { cmd_logs(argc,argv); return; }
    if(STR_EQ(cmd,"clear")||STR_EQ(cmd,"cls")) { CLEAR_SCREEN(); return; }
    if(STR_EQ(cmd,"sysinfo"))      { cmd_sysinfo(); return; }
    if(STR_EQ(cmd,"help")||STR_EQ(cmd,"?")) { shell_help(); return; }

    /* ── Unknown ── */
    printf(CLR_YELLOW "  '%s': command not found. Type 'help' for commands.\n" CLR_RESET, cmd);
    char logmsg[128]; sprintf(logmsg,"Unknown command: %s",cmd);
    log_write(LOG_WARN,logmsg);
}

/* ─── Main shell loop ─── */
void shell_run(void){
    char cmdline[MAX_CMD];
    while(1){
        /* Prompt */
        if(g_logged_in)
            printf(CLR_BGREEN "%s" CLR_RESET "@" CLR_CYAN "cortexos" CLR_RESET ":" CLR_BYELLOW "%s" CLR_RESET "$ ",
                   g_current_user, g_current_dir);
        else
            printf(CLR_RED "guest" CLR_RESET "@cortexos:~$ ");
        fflush(stdout);

        if(!fgets(cmdline,sizeof(cmdline),stdin)){
            printf("\n"); break;
        }
        cmdline[strcspn(cmdline,"\r\n")]='\0';

        /* Handle logout: break shell loop, return to boot menu */
        if(STR_EQ(cmdline,"exit")||STR_EQ(cmdline,"logout")){
            if(g_logged_in) user_logout();
            break;
        }
        shell_dispatch(cmdline);
    }
}
