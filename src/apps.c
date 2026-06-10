#include "apps.h"
#include "filesystem.h"
#include "logs.h"

/* ══════════════════════════════════════════
   COMMAND HISTORY (stack)
   ══════════════════════════════════════════ */
static char history[MAX_HISTORY][MAX_CMD];
static int history_top = 0;

void history_push(const char *cmd){
    if(STR_EMPTY(cmd)) return;
    strncpy(history[history_top%MAX_HISTORY],cmd,MAX_CMD-1);
    history_top++;
}

void cmd_history(int argc, char **argv){
    (void)argc;(void)argv;
    int start=(history_top>MAX_HISTORY)?history_top-MAX_HISTORY:0;
    printf("\n");
    for(int i=start;i<history_top;i++)
        printf("  %4d  %s\n",i+1,history[i%MAX_HISTORY]);
    printf("\n");
}

/* ══════════════════════════════════════════
   PACKAGE MANAGER
   ══════════════════════════════════════════ */
typedef struct { char name[32]; char version[16]; int installed; } Package;
static Package packages[]={
    {"calculator","1.0",0},{"calendar","1.0",0},{"notes","1.2",0},
    {"snake","1.0",0},{"chess","1.0",0},{"sudoku","1.1",0},
    {"minesweeper","1.0",0},{"todo","1.3",0},{"texteditor","2.0",0},
    {""} /* sentinel */
};

static Package *pkg_find(const char *name){
    for(int i=0;packages[i].name[0];i++) if(STR_EQ(packages[i].name,name)) return &packages[i];
    return NULL;
}
static void pkgs_save(void){
    FILE *f=fopen(PKG_FILE,"wb"); if(!f) return;
    fwrite(packages,sizeof(packages),1,f); fclose(f);
}
static void pkgs_load(void){
    FILE *f=fopen(PKG_FILE,"rb"); if(!f) return;
    fread(packages,sizeof(packages),1,f); fclose(f);
}

void cmd_packages(int argc, char **argv){
    (void)argc;(void)argv;
    pkgs_load();
    printf("\n  %-16s %-10s %-10s\n","Package","Version","Status");
    printf("  %s\n","--------------------------------------------");
    for(int i=0;packages[i].name[0];i++)
        printf("  %-16s %-10s %s\n",packages[i].name,packages[i].version,
               packages[i].installed?CLR_BGREEN"Installed"CLR_RESET:"Not installed");
    printf("\n");
}
void cmd_install(int argc, char **argv){
    if(argc<2){ printf("Usage: install <package>\n"); return; }
    pkgs_load();
    Package *p=pkg_find(argv[1]);
    if(!p){ PRINT_ERR("Package not found. Use 'packages' to list available."); return; }
    if(p->installed){ PRINT_WARN("Already installed."); return; }
    printf("  Installing %s v%s ...",p->name,p->version); fflush(stdout);
    SLEEP_MS(600);
    p->installed=1; pkgs_save();
    printf(CLR_BGREEN " Done!\n" CLR_RESET);
}
void cmd_remove(int argc, char **argv){
    if(argc<2){ printf("Usage: remove <package>\n"); return; }
    pkgs_load();
    Package *p=pkg_find(argv[1]);
    if(!p||!p->installed){ PRINT_ERR("Package not installed."); return; }
    p->installed=0; pkgs_save();
    printf("  Package '%s' removed.\n",argv[1]);
}
void cmd_update(int argc, char **argv){
    (void)argc;(void)argv;
    printf("  Updating package registry"); fflush(stdout);
    for(int i=0;i<5;i++){ SLEEP_MS(200); printf("."); fflush(stdout); }
    printf(CLR_BGREEN " Up to date!\n" CLR_RESET);
}
void cmd_store(int argc, char **argv){
    (void)argc;(void)argv;
    printf("\n%s╔═══════════════════════════════════════════╗%s\n",CLR_MAGENTA,CLR_RESET);
    printf("%s║          CORTEX APP STORE                 ║%s\n",CLR_MAGENTA,CLR_RESET);
    printf("%s╚═══════════════════════════════════════════╝%s\n\n",CLR_MAGENTA,CLR_RESET);
    cmd_packages(0,NULL);
    printf("  Use: install <name>  to install an app.\n\n");
}

/* ══════════════════════════════════════════
   TEXT EDITOR
   ══════════════════════════════════════════ */
void app_text_editor(const char *filename){
    printf("\n%s╔════════════════════════════════════════════╗%s\n",CLR_CYAN,CLR_RESET);
    printf("%s║  CORTEX EDITOR  - %s%s\n",CLR_CYAN,filename,CLR_RESET);
    printf("%s╚════════════════════════════════════════════╝%s\n",CLR_CYAN,CLR_RESET);
    printf("  Commands: :w save  :q quit  :wq save+quit  :a append mode\n\n");

    char *existing = fs_read(filename);
    char content[FS_MAX_CONTENT]="";
    if(existing) strncpy(content,existing,FS_MAX_CONTENT-1);
    if(content[0]) printf("--- existing content ---\n%s\n--- end ---\n\n",content);

    int append_mode=0;
    char line[512];
    while(1){
        printf(append_mode?CLR_YELLOW" [A]> "CLR_RESET:CLR_CYAN"  >> "CLR_RESET);
        fflush(stdout);
        if(!fgets(line,sizeof(line),stdin)) break;
        line[strcspn(line,"\r\n")]='\0';
        if(STR_EQ(line,":q")){ printf("  Quit without saving.\n"); return; }
        if(STR_EQ(line,":w")){ fs_write(filename,content,0); printf("  Saved.\n"); continue; }
        if(STR_EQ(line,":wq")){ fs_write(filename,content,0); printf("  Saved and closed.\n"); return; }
        if(STR_EQ(line,":a")){ append_mode=!append_mode; printf("  Append mode %s.\n",append_mode?"ON":"OFF"); continue; }
        if(append_mode){
            strncat(content,line,FS_MAX_CONTENT-strlen(content)-2);
            strncat(content,"\n",FS_MAX_CONTENT-strlen(content)-2);
        } else {
            strncpy(content,line,FS_MAX_CONTENT-1);
            strncat(content,"\n",FS_MAX_CONTENT-strlen(content)-2);
        }
    }
}
void cmd_edit(int argc, char **argv){
    if(argc<2){ printf("Usage: edit <filename>\n"); return; }
    app_text_editor(argv[1]);
}

/* ══════════════════════════════════════════
   CALCULATOR
   ══════════════════════════════════════════ */
void app_calculator(void){
    printf("\n%s╔═════════════════════════════╗%s\n",CLR_CYAN,CLR_RESET);
    printf("%s║     CORTEX CALCULATOR       ║%s\n",CLR_CYAN,CLR_RESET);
    printf("%s╚═════════════════════════════╝%s\n",CLR_CYAN,CLR_RESET);
    printf("  Format: <num> <op> <num>  (op: + - * /)\n  'exit' to quit\n\n");
    char expr[64];
    while(1){
        printf(CLR_CYAN "  calc> " CLR_RESET); fflush(stdout);
        if(!fgets(expr,sizeof(expr),stdin)) break;
        expr[strcspn(expr,"\r\n")]='\0';
        if(STR_EQ(expr,"exit")||STR_EQ(expr,"q")) break;
        double a,b; char op;
        if(sscanf(expr,"%lf %c %lf",&a,&op,&b)==3){
            double res=0; int err=0;
            if(op=='+') res=a+b;
            else if(op=='-') res=a-b;
            else if(op=='*') res=a*b;
            else if(op=='/'){ if(b==0){ PRINT_ERR("Division by zero."); err=1; } else res=a/b; }
            else { PRINT_ERR("Unknown operator."); err=1; }
            if(!err) printf("  = %g\n",res);
        } else printf("  Invalid expression. Format: 5 + 3\n");
    }
}
void cmd_calc(int argc, char **argv){ (void)argc;(void)argv; app_calculator(); }

/* ══════════════════════════════════════════
   CALENDAR
   ══════════════════════════════════════════ */
void app_calendar(void){
    time_t t=time(NULL); struct tm *tm=localtime(&t);
    int year=tm->tm_year+1900, month=tm->tm_mon+1;
    const char *months[]={"","January","February","March","April","May","June",
                          "July","August","September","October","November","December"};
    int days_in_month[]={0,31,28,31,30,31,30,31,31,30,31,30,31};
    if(year%4==0&&(year%100!=0||year%400==0)) days_in_month[2]=29;
    /* first weekday of month */
    struct tm first={0}; first.tm_year=year-1900; first.tm_mon=month-1; first.tm_mday=1;
    mktime(&first); int start=first.tm_wday;
    printf("\n  %s%s %d%s\n",CLR_CYAN,months[month],year,CLR_RESET);
    printf("  Su Mo Tu We Th Fr Sa\n");
    int d=1; int total=days_in_month[month];
    for(int r=0;d<=total;r++){
        printf("  ");
        for(int w=0;w<7;w++){
            if(r==0&&w<start) printf("   ");
            else if(d<=total){
                if(d==tm->tm_mday) printf(CLR_BGREEN "%2d " CLR_RESET,d);
                else printf("%2d ",d);
                d++;
            }
        }
        printf("\n");
    }
    printf("\n");
}
void cmd_calendar_cmd(int argc, char **argv){ (void)argc;(void)argv; app_calendar(); }

/* ══════════════════════════════════════════
   NOTES
   ══════════════════════════════════════════ */
void app_notes(void){
    printf("\n%s╔═════════════════════════╗%s\n",CLR_CYAN,CLR_RESET);
    printf("%s║    CORTEX NOTES         ║%s\n",CLR_CYAN,CLR_RESET);
    printf("%s╚═════════════════════════╝%s\n",CLR_CYAN,CLR_RESET);
    printf("  Commands: new, list, read <n>, delete <n>, exit\n\n");
    char notes_path[MAX_PATH]; sprintf(notes_path,"/home/%s/notes.txt",g_current_user);
    char input[256];
    while(1){
        printf(CLR_CYAN "  notes> " CLR_RESET); fflush(stdout);
        if(!fgets(input,sizeof(input),stdin)) break;
        input[strcspn(input,"\r\n")]='\0';
        if(STR_EQ(input,"exit")||STR_EQ(input,"q")) break;
        if(STR_EQ(input,"new")){
            printf("  Note text: "); fflush(stdout);
            char note[512]; fgets(note,sizeof(note),stdin); note[strcspn(note,"\r\n")]='\0';
            char entry[600];
            time_t t=time(NULL); struct tm *tm=localtime(&t); char ts[32]; strftime(ts,32,"%Y-%m-%d %H:%M",tm);
            sprintf(entry,"[%s] %s\n",ts,note);
            fs_write(notes_path,entry,1); printf("  Note saved.\n");
        } else if(STR_EQ(input,"list")){
            char *c=fs_read(notes_path);
            if(!c||!c[0]) printf("  No notes yet.\n");
            else printf("\n%s\n",c);
        } else if(STR_EQ(input,"exit")) break;
        else printf("  Unknown command.\n");
    }
}
void cmd_notes(int argc, char **argv){ (void)argc;(void)argv; app_notes(); }

/* ══════════════════════════════════════════
   TODO MANAGER
   ══════════════════════════════════════════ */
void app_todo(void){
    printf("\n%s╔═══════════════════════════╗%s\n",CLR_CYAN,CLR_RESET);
    printf("%s║     CORTEX TODO           ║%s\n",CLR_CYAN,CLR_RESET);
    printf("%s╚═══════════════════════════╝%s\n",CLR_CYAN,CLR_RESET);
    printf("  add <task>  done <n>  list  clear  exit\n\n");
    char todo_path[MAX_PATH]; sprintf(todo_path,"/home/%s/todo.txt",g_current_user);
    char line[256];
    while(1){
        printf(CLR_CYAN "  todo> " CLR_RESET); fflush(stdout);
        if(!fgets(line,sizeof(line),stdin)) break;
        line[strcspn(line,"\r\n")]='\0';
        if(STR_EQ(line,"exit")||STR_EQ(line,"q")) break;
        if(strncmp(line,"add ",4)==0){
            char entry[280]; sprintf(entry,"[ ] %s\n",line+4);
            fs_write(todo_path,entry,1); printf("  Added: %s\n",line+4);
        } else if(STR_EQ(line,"list")){
            char *c=fs_read(todo_path);
            if(!c||!c[0]) printf("  No tasks.\n");
            else { printf("\n"); int i=1; char *p=c; char *nl;
                while((nl=strchr(p,'\n'))){ *nl='\0'; printf("  %d. %s\n",i++,p); p=nl+1; } printf("\n"); }
        } else if(STR_EQ(line,"clear")){
            fs_write(todo_path,"",0); printf("  Todo cleared.\n");
        }
    }
}
void cmd_todo(int argc, char **argv){ (void)argc;(void)argv; app_todo(); }

/* ══════════════════════════════════════════
   SNAKE GAME
   ══════════════════════════════════════════ */
void app_snake(void){
    printf("\n%s╔══════════════════════════════════════════╗%s\n",CLR_BGREEN,CLR_RESET);
    printf("%s║  SNAKE - ASCII Edition                   ║%s\n",CLR_BGREEN,CLR_RESET);
    printf("%s╚══════════════════════════════════════════╝%s\n\n",CLR_BGREEN,CLR_RESET);
    /* Simple text-based snake turn simulator */
    printf("  Due to terminal limitations, this is turn-based Snake.\n");
    printf("  Commands: w(up) s(down) a(left) d(right) q(quit)\n\n");
    #define SN_W 20
    #define SN_H 10
    int sx=5,sy=5, fx=10,fy=3, score=0;
    char board[SN_H][SN_W];
    char dir='d';
    char inp[8];
    int playing=1;
    while(playing){
        memset(board,'.',sizeof(board));
        board[sy][sx]='O'; board[fy][fx]='*';
        /* border */
        printf("\n  +"); for(int i=0;i<SN_W;i++) printf("-"); printf("+\n");
        for(int r=0;r<SN_H;r++){
            printf("  |");
            for(int c=0;c<SN_W;c++){
                if(r==sy&&c==sx) printf(CLR_BGREEN "O" CLR_RESET);
                else if(r==fy&&c==fx) printf(CLR_RED "*" CLR_RESET);
                else printf(".");
            }
            printf("|\n");
        }
        printf("  +"); for(int i=0;i<SN_W;i++) printf("-"); printf("+\n");
        printf("  Score: %d   Direction: %c   (*=food)\n\n",score,dir);
        printf("  Move [w/a/s/d/q]: "); fflush(stdout);
        if(!fgets(inp,sizeof(inp),stdin)) break;
        inp[strcspn(inp,"\r\n")]='\0';
        if(STR_EQ(inp,"q")) break;
        if(inp[0]=='w'||inp[0]=='a'||inp[0]=='s'||inp[0]=='d') dir=inp[0];
        int nx=sx, ny=sy;
        if(dir=='w') ny--; if(dir=='s') ny++;
        if(dir=='a') nx--; if(dir=='d') nx++;
        if(nx<0||nx>=SN_W||ny<0||ny>=SN_H){ printf(CLR_RED "  Hit wall! Game over.\n\n" CLR_RESET); playing=0; break; }
        sx=nx; sy=ny;
        if(sx==fx&&sy==fy){ score+=10; fx=rand()%SN_W; fy=rand()%SN_H; printf(CLR_BGREEN "  Yum! +10\n" CLR_RESET); }
    }
    printf("  Final score: %d\n\n",score);
}
void cmd_snake(int argc, char **argv){ (void)argc;(void)argv; app_snake(); }

/* ══════════════════════════════════════════
   CHESS (display only)
   ══════════════════════════════════════════ */
void app_chess(void){
    const char *board[8][8]={
        {"r","n","b","q","k","b","n","r"},
        {"p","p","p","p","p","p","p","p"},
        {".",".",".",".",".",".",".","."},{".",".",".",".",".",".",".","."}, 
        {".",".",".",".",".",".",".","."},{".",".",".",".",".",".",".","."}, 
        {"P","P","P","P","P","P","P","P"},
        {"R","N","B","Q","K","B","N","R"}
    };
    printf("\n%s╔═══════════════════════════════╗%s\n",CLR_YELLOW,CLR_RESET);
    printf("%s║        CORTEX CHESS           ║%s\n",CLR_YELLOW,CLR_RESET);
    printf("%s╚═══════════════════════════════╝%s\n\n",CLR_YELLOW,CLR_RESET);
    printf("    a  b  c  d  e  f  g  h\n");
    printf("  +--+--+--+--+--+--+--+--+\n");
    for(int r=0;r<8;r++){
        printf("%d |",8-r);
        for(int c=0;c<8;c++){
            int dark=(r+c)%2;
            const char *p=board[r][c];
            if(!STR_EQ(p,"."))
                printf(dark?CLR_YELLOW:CLR_WHITE " %s " CLR_RESET,p);
            else printf(dark?CLR_BLUE " . " CLR_RESET:" . ");
        }
        printf("| %d\n",8-r);
    }
    printf("  +--+--+--+--+--+--+--+--+\n");
    printf("    a  b  c  d  e  f  g  h\n");
    printf("\n  Chess viewer ready. (Full move engine is a future module.)\n\n");
}
void cmd_chess(int argc, char **argv){ (void)argc;(void)argv; app_chess(); }

/* ══════════════════════════════════════════
   SUDOKU
   ══════════════════════════════════════════ */
void app_sudoku(void){
    int puzzle[9][9]={
        {5,3,0,0,7,0,0,0,0},{6,0,0,1,9,5,0,0,0},{0,9,8,0,0,0,0,6,0},
        {8,0,0,0,6,0,0,0,3},{4,0,0,8,0,3,0,0,1},{7,0,0,0,2,0,0,0,6},
        {0,6,0,0,0,0,2,8,0},{0,0,0,4,1,9,0,0,5},{0,0,0,0,8,0,0,7,9}
    };
    printf("\n%s╔═════════════════════════╗%s\n",CLR_MAGENTA,CLR_RESET);
    printf("%s║     CORTEX SUDOKU       ║%s\n",CLR_MAGENTA,CLR_RESET);
    printf("%s╚═════════════════════════╝%s\n\n",CLR_MAGENTA,CLR_RESET);
    printf("  Enter row col val (e.g. 1 3 4) or 'quit'\n\n");
    int playing=1;
    while(playing){
        printf("    1 2 3   4 5 6   7 8 9\n");
        printf("  +-------+-------+-------+\n");
        for(int r=0;r<9;r++){
            if(r==3||r==6) printf("  +-------+-------+-------+\n");
            printf("%d |",r+1);
            for(int c=0;c<9;c++){
                if(c==3||c==6) printf("|");
                if(puzzle[r][c]) printf(CLR_CYAN "%d " CLR_RESET,puzzle[r][c]);
                else printf(". ");
            }
            printf("|\n");
        }
        printf("  +-------+-------+-------+\n\n");
        printf("  sudoku> "); fflush(stdout);
        char inp[32]; if(!fgets(inp,sizeof(inp),stdin)) break;
        inp[strcspn(inp,"\r\n")]='\0';
        if(STR_EQ(inp,"quit")||STR_EQ(inp,"q")) break;
        int row,col,val;
        if(sscanf(inp,"%d %d %d",&row,&col,&val)==3){
            row--; col--;
            if(row<0||row>8||col<0||col>8||val<1||val>9){ PRINT_ERR("Invalid. row 1-9, col 1-9, val 1-9."); continue; }
            if(puzzle[row][col]){ PRINT_WARN("Cell already filled."); continue; }
            puzzle[row][col]=val; printf(CLR_BGREEN "  Placed %d at (%d,%d)\n" CLR_RESET,val,row+1,col+1);
        } else printf("  Format: row col val\n");
    }
}
void cmd_sudoku(int argc, char **argv){ (void)argc;(void)argv; app_sudoku(); }

/* ══════════════════════════════════════════
   MINESWEEPER
   ══════════════════════════════════════════ */
#define MS_W 9
#define MS_H 9
#define MS_MINES 10
void app_minesweeper(void){
    int mines[MS_H][MS_W]={0};
    int revealed[MS_H][MS_W]={0};
    int flagged[MS_H][MS_W]={0};
    /* place mines */
    srand((unsigned)time(NULL)); int placed=0;
    while(placed<MS_MINES){
        int r=rand()%MS_H, c=rand()%MS_W;
        if(!mines[r][c]){ mines[r][c]=1; placed++; }
    }
    printf("\n%s╔════════════════════════════════╗%s\n",CLR_RED,CLR_RESET);
    printf("%s║       CORTEX MINESWEEPER       ║%s\n",CLR_RED,CLR_RESET);
    printf("%s╚════════════════════════════════╝%s\n",CLR_RED,CLR_RESET);
    printf("  reveal <row> <col> | flag <row> <col> | quit\n\n");
    int game_over=0, won=0;
    while(!game_over){
        printf("    ");
        for(int c=0;c<MS_W;c++) printf("%d ",c+1); printf("\n");
        printf("  +-"); for(int c=0;c<MS_W;c++) printf("--"); printf("+\n");
        for(int r=0;r<MS_H;r++){
            printf("%2d|",r+1);
            for(int c=0;c<MS_W;c++){
                if(flagged[r][c]) printf(CLR_YELLOW "F " CLR_RESET);
                else if(!revealed[r][c]) printf("# ");
                else if(mines[r][c]) printf(CLR_RED "* " CLR_RESET);
                else {
                    int adj=0;
                    for(int dr=-1;dr<=1;dr++) for(int dc=-1;dc<=1;dc++){
                        int nr=r+dr,nc=c+dc;
                        if(nr>=0&&nr<MS_H&&nc>=0&&nc<MS_W&&mines[nr][nc]) adj++;
                    }
                    if(adj) printf(CLR_CYAN "%d " CLR_RESET,adj);
                    else printf(". ");
                }
            }
            printf("|\n");
        }
        printf("  +-"); for(int c=0;c<MS_W;c++) printf("--"); printf("+\n\n");
        printf("  ms> "); fflush(stdout);
        char inp[32]; if(!fgets(inp,sizeof(inp),stdin)) break;
        inp[strcspn(inp,"\r\n")]='\0';
        if(STR_EQ(inp,"quit")) break;
        int row,col; char cmd2[16];
        if(sscanf(inp,"%15s %d %d",cmd2,&row,&col)==3){
            row--; col--;
            if(row<0||row>=MS_H||col<0||col>=MS_W){ PRINT_ERR("Out of bounds."); continue; }
            if(STR_EQ(cmd2,"reveal")){
                revealed[row][col]=1;
                if(mines[row][col]){ printf(CLR_RED "  BOOM! You hit a mine! Game Over.\n\n" CLR_RESET); game_over=1; }
            } else if(STR_EQ(cmd2,"flag")){
                flagged[row][col]=!flagged[row][col];
            }
        }
        /* check win */
        int unrevealed=0;
        for(int r=0;r<MS_H;r++) for(int c=0;c<MS_W;c++) if(!revealed[r][c]&&!mines[r][c]) unrevealed++;
        if(unrevealed==0){ won=1; game_over=1; }
    }
    if(won) printf(CLR_BGREEN "  Congratulations! You cleared the board!\n\n" CLR_RESET);
}
void cmd_minesweeper(int argc, char **argv){ (void)argc;(void)argv; app_minesweeper(); }

/* ══════════════════════════════════════════
   VIRTUAL MACHINE MANAGER
   ══════════════════════════════════════════ */
typedef struct { char name[32]; int running; char os[32]; int ram; } VM;
static VM vms[8]; static int vm_count=0;
void cmd_vm(int argc, char **argv){
    if(argc<2){ printf("\n  vm create <name> <os> <ram_mb>  |  vm start <name>  |  vm stop <name>  |  vm list\n\n"); return; }
    if(STR_EQ(argv[1],"create")){
        if(argc<5){ printf("Usage: vm create <name> <os> <ram_mb>\n"); return; }
        if(vm_count>=8){ PRINT_ERR("VM limit reached."); return; }
        VM *v=&vms[vm_count++];
        strncpy(v->name,argv[2],31); strncpy(v->os,argv[3],31);
        v->ram=atoi(argv[4]); v->running=0;
        printf(CLR_BGREEN "  VM '%s' created (OS: %s, RAM: %d MB)\n" CLR_RESET,v->name,v->os,v->ram);
    } else if(STR_EQ(argv[1],"start")){
        if(argc<3){ printf("Usage: vm start <name>\n"); return; }
        for(int i=0;i<vm_count;i++) if(STR_EQ(vms[i].name,argv[2])){ vms[i].running=1; printf("  VM '%s' started.\n",argv[2]); return; }
        PRINT_ERR("VM not found.");
    } else if(STR_EQ(argv[1],"stop")){
        if(argc<3){ printf("Usage: vm stop <name>\n"); return; }
        for(int i=0;i<vm_count;i++) if(STR_EQ(vms[i].name,argv[2])){ vms[i].running=0; printf("  VM '%s' stopped.\n",argv[2]); return; }
        PRINT_ERR("VM not found.");
    } else if(STR_EQ(argv[1],"list")){
        printf("\n  %-16s %-16s %-8s %-8s\n","Name","OS","RAM (MB)","Status");
        printf("  %s\n","-----------------------------------------------");
        for(int i=0;i<vm_count;i++)
            printf("  %-16s %-16s %-8d %s\n",vms[i].name,vms[i].os,vms[i].ram,
                   vms[i].running?CLR_BGREEN"Running"CLR_RESET:"Stopped");
        printf("\n");
    }
}
