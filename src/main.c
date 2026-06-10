/*
 * ╔══════════════════════════════════════════════════════════════╗
 * ║               C O R T E X   O S   v 1 . 0                   ║
 * ║         A Terminal-Based OS Simulator in Pure C              ║
 * ║                  Platform: Windows (MinGW/GCC)               ║
 * ╚══════════════════════════════════════════════════════════════╝
 *
 *  main.c  –  Boot sequence, data initialisation, main menu loop
 */

#include "common.h"
#include "users.h"
#include "filesystem.h"
#include "process.h"
#include "memory.h"
#include "security.h"
#include "logs.h"
#include "database.h"
#include "blockchain.h"
#include "apps.h"
#include "shell.h"

/* ── Global session state ── */
char g_current_user[MAX_USERNAME] = "";
char g_current_dir[MAX_PATH]      = "/";
int  g_is_admin                   = 0;
int  g_logged_in                  = 0;

/* ────────────────────────────────────────
   Enable ANSI colours on Windows 10+
   ──────────────────────────────────────── */
#ifdef _WIN32
/* Some older MinGW headers don't define this constant */
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#endif

static void enable_ansi(void){
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if(hOut == INVALID_HANDLE_VALUE) return;
    DWORD dwMode = 0;
    if(!GetConsoleMode(hOut, &dwMode)) return;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
#endif
}

/* ────────────────────────────────────────
   Create required data directories
   ──────────────────────────────────────── */
void init_data_dirs(void){
    /* Windows mkdir – ignore errors if already exist */
    system("mkdir data 2>nul");
    system("mkdir data\\mail 2>nul");
    system("mkdir data\\backups 2>nul");
    system("mkdir data\\databases 2>nul");
    system("mkdir data\\vfs 2>nul");
}

/* ────────────────────────────────────────
   BIOS / Boot animation
   ──────────────────────────────────────── */
static void boot_bios(void){
    CLEAR_SCREEN();
    printf("\n\n");
    printf("  %s╔════════════════════════════════════════════════╗%s\n",CLR_YELLOW,CLR_RESET);
    printf("  %s║           C O R T E X   B I O S  v1.0         ║%s\n",CLR_YELLOW,CLR_RESET);
    printf("  %s║         Copyright (C) 2025 CortexOS Dev       ║%s\n",CLR_YELLOW,CLR_RESET);
    printf("  %s╚════════════════════════════════════════════════╝%s\n\n",CLR_YELLOW,CLR_RESET);
    SLEEP_MS(400);

    const char *steps[]={
        "Checking Memory........",
        "Initializing CPU.......",
        "Loading Kernel.........",
        "Loading Drivers........",
        "Mounting File System...",
        "Starting Services......",
        "Launching CortexOS.....",
        NULL
    };
    for(int i=0;steps[i];i++){
        printf("  %s%s%s",CLR_CYAN,steps[i],CLR_RESET);
        fflush(stdout);
        SLEEP_MS(350);
        printf(CLR_BGREEN "  [ OK ]\n" CLR_RESET);
    }
    SLEEP_MS(500);
}

/* ────────────────────────────────────────
   Welcome banner
   ──────────────────────────────────────── */
static void print_banner(void){
    CLEAR_SCREEN();
    printf("\n");
    printf("  %s╔══════════════════════════════════════════════════════════╗%s\n",CLR_CYAN,CLR_RESET);
    printf("  %s║                                                          ║%s\n",CLR_CYAN,CLR_RESET);
    printf("  %s║   ██████╗ ██████╗ ██████╗ ████████╗███████╗██╗  ██╗     ║%s\n",CLR_BCYAN,CLR_RESET);
    printf("  %s║  ██╔════╝██╔═══██╗██╔══██╗╚══██╔══╝██╔════╝╚██╗██╔╝     ║%s\n",CLR_BCYAN,CLR_RESET);
    printf("  %s║  ██║     ██║   ██║██████╔╝   ██║   █████╗   ╚███╔╝      ║%s\n",CLR_BCYAN,CLR_RESET);
    printf("  %s║  ██║     ██║   ██║██╔══██╗   ██║   ██╔══╝   ██╔██╗      ║%s\n",CLR_BCYAN,CLR_RESET);
    printf("  %s║  ╚██████╗╚██████╔╝██║  ██║   ██║   ███████╗██╔╝ ██╗     ║%s\n",CLR_BCYAN,CLR_RESET);
    printf("  %s║   ╚═════╝ ╚═════╝ ╚═╝  ╚═╝   ╚═╝   ╚══════╝╚═╝  ╚═╝    ║%s\n",CLR_BCYAN,CLR_RESET);
    printf("  %s║                                                          ║%s\n",CLR_CYAN,CLR_RESET);
    printf("  %s║         Terminal Operating System Simulator v1.0        ║%s\n",CLR_CYAN,CLR_RESET);
    printf("  %s║               Built in Pure C  |  Windows               ║%s\n",CLR_CYAN,CLR_RESET);
    printf("  %s╚══════════════════════════════════════════════════════════╝%s\n\n",CLR_CYAN,CLR_RESET);
}

/* ────────────────────────────────────────
   Main Menu (pre-login)
   ──────────────────────────────────────── */
static void main_menu(void){
    char choice[8];
    while(1){
        print_banner();
        printf("  %s┌─────────────────────────────┐%s\n",CLR_CYAN,CLR_RESET);
        printf("  %s│       MAIN MENU             │%s\n",CLR_CYAN,CLR_RESET);
        printf("  %s├─────────────────────────────┤%s\n",CLR_CYAN,CLR_RESET);
        printf("  %s│  1. Login                   │%s\n",CLR_WHITE,CLR_RESET);
        printf("  %s│  2. Sign Up                 │%s\n",CLR_WHITE,CLR_RESET);
        printf("  %s│  3. Exit                    │%s\n",CLR_WHITE,CLR_RESET);
        printf("  %s└─────────────────────────────┘%s\n\n",CLR_CYAN,CLR_RESET);
        printf("  Choice: "); fflush(stdout);
        if(!fgets(choice,sizeof(choice),stdin)) break;
        choice[strcspn(choice,"\r\n")]='\0';

        if(STR_EQ(choice,"1")){
            /* Inline login */
            char uname[MAX_USERNAME]={0}, pass[MAX_PASSWORD]={0};
            printf("\n  Username: "); fflush(stdout);
            fgets(uname,sizeof(uname),stdin); uname[strcspn(uname,"\r\n")]='\0';
            printf("  Password: "); fflush(stdout);
            fgets(pass,sizeof(pass),stdin); pass[strcspn(pass,"\r\n")]='\0';
            if(user_login(uname,pass)){
                /* Init FS home for this user */
                char hdir[MAX_PATH]; sprintf(hdir,"/home/%s",g_current_user);
                fs_mkdir(hdir);
                fs_cd(hdir);
                printf(CLR_BGREEN "\n  Welcome back, %s!\n  Type 'help' for command list.\n\n" CLR_RESET,uname);
                SLEEP_MS(800);
                shell_run();   /* enters shell; returns on logout */
            } else {
                printf(CLR_RED "\n  Login failed.\n\n" CLR_RESET);
                SLEEP_MS(800);
            }
        } else if(STR_EQ(choice,"2")){
            char uname[MAX_USERNAME]={0}, pass[MAX_PASSWORD]={0}, email[64]={0};
            printf("\n  Username : "); fflush(stdout);
            fgets(uname,sizeof(uname),stdin); uname[strcspn(uname,"\r\n")]='\0';
            printf("  Password : "); fflush(stdout);
            fgets(pass,sizeof(pass),stdin); pass[strcspn(pass,"\r\n")]='\0';
            printf("  Email    : "); fflush(stdout);
            fgets(email,sizeof(email),stdin); email[strcspn(email,"\r\n")]='\0';
            if(user_signup(uname,pass,email,0)){
                PRINT_OK("Account created! Please login.");
            }
            SLEEP_MS(800);
        } else if(STR_EQ(choice,"3")){
            printf("\n  %sShutting down CortexOS... Goodbye!%s\n\n",CLR_YELLOW,CLR_RESET);
            log_write(LOG_INFO,"CortexOS shutdown.");
            SLEEP_MS(500);
            break;
        } else {
            printf(CLR_RED "  Invalid choice.\n\n" CLR_RESET);
            SLEEP_MS(400);
        }
    }
}

/* ────────────────────────────────────────
   Entry Point
   ──────────────────────────────────────── */
int main(void){
    enable_ansi();
    init_data_dirs();

    /* Boot sequence */
    boot_bios();

    /* Initialise all subsystems */
    logs_init();
    log_write(LOG_INFO,"CortexOS starting up.");

    users_init();
    fs_init();
    proc_init();
    mem_init();
    db_init();
    bc_init();

    log_write(LOG_INFO,"All subsystems initialised.");

    /* Launch main menu */
    main_menu();

    /* Cleanup */
    fs_shutdown();
    return 0;
}
