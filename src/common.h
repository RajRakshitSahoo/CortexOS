#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdint.h>

/* ── Platform ── */
#ifdef _WIN32
#include <windows.h>
#define CLEAR_SCREEN() system("cls")
#define SLEEP_MS(ms)   Sleep(ms)
#else
#include <unistd.h>
#define CLEAR_SCREEN() system("clear")
#define SLEEP_MS(ms)   usleep((ms)*1000)
#endif

/* ── Fix MAX_PATH conflict with Windows windef.h ── */
#ifdef MAX_PATH
#undef MAX_PATH
#endif
#define MAX_PATH       256

/* ── Limits ── */
#define MAX_USERNAME   32
#define MAX_PASSWORD   64
#define MAX_CMD        512
#define MAX_ARGS       16
#define MAX_FILENAME   128
#define MAX_CONTENT    4096
#define MAX_USERS      64
#define MAX_PROCESSES  64
#define MAX_PACKAGES   32
#define MAX_HISTORY    100
#define MAX_LOG_LINE   256

/* ── Paths ── */
#define DATA_DIR        "data\\"
#define USERS_FILE      "data\\users.dat"
#define LOGS_FILE       "data\\system.log"
#define MAIL_DIR        "data\\mail\\"
#define BACKUP_DIR      "data\\backups\\"
#define FS_ROOT         "data\\vfs\\"
#define PKG_FILE        "data\\packages.dat"
#define DB_DIR          "data\\databases\\"
#define BLOCKCHAIN_FILE "data\\blockchain.dat"

/* ── Colors (ANSI) ── */
#define CLR_RESET   "\033[0m"
#define CLR_BOLD    "\033[1m"
#define CLR_RED     "\033[31m"
#define CLR_GREEN   "\033[32m"
#define CLR_YELLOW  "\033[33m"
#define CLR_BLUE    "\033[34m"
#define CLR_MAGENTA "\033[35m"
#define CLR_CYAN    "\033[36m"
#define CLR_WHITE   "\033[37m"
#define CLR_BGREEN  "\033[92m"
#define CLR_BCYAN   "\033[96m"
#define CLR_BYELLOW "\033[93m"

/* ── Permissions ── */
#define PERM_READ    0x4
#define PERM_WRITE   0x2
#define PERM_EXECUTE 0x1

/* ── Boolean ── */
#ifndef bool
typedef int bool;
#define true  1
#define false 0
#endif

/* ── Utility macros ── */
#define STR_EQ(a,b)  (strcmp((a),(b))==0)
#define STR_EMPTY(s) ((s)[0]=='\0')
#define MIN(a,b)     ((a)<(b)?(a):(b))
#define MAX_VAL(a,b) ((a)>(b)?(a):(b))

/* ── Print helpers ── */
#define PRINT_OK(msg)   printf(CLR_BGREEN  "[  OK  ] " CLR_RESET msg "\n")
#define PRINT_ERR(msg)  printf(CLR_RED     "[ ERR  ] " CLR_RESET msg "\n")
#define PRINT_INFO(msg) printf(CLR_CYAN    "[ INFO ] " CLR_RESET msg "\n")
#define PRINT_WARN(msg) printf(CLR_YELLOW  "[ WARN ] " CLR_RESET msg "\n")

/* ── Session globals ── */
extern char g_current_user[MAX_USERNAME];
extern char g_current_dir[MAX_PATH];
extern int  g_is_admin;
extern int  g_logged_in;

void init_data_dirs(void);

#endif /* COMMON_H */
