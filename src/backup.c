#include "backup.h"
#include "logs.h"

static void backup_file(const char *src, const char *dst){
    FILE *in=fopen(src,"rb"); if(!in) return;
    FILE *out=fopen(dst,"wb"); if(!out){ fclose(in); return; }
    char buf[4096]; size_t n;
    while((n=fread(buf,1,sizeof(buf),in))>0) fwrite(buf,1,n,out);
    fclose(in); fclose(out);
}

void cmd_backup(int argc, char **argv){
    (void)argc;(void)argv;
    time_t t=time(NULL); struct tm *tm=localtime(&t);
    char ts[32]; strftime(ts,32,"%Y%m%d_%H%M%S",tm);
    char snap_dir[MAX_PATH]; snprintf(snap_dir,sizeof(snap_dir),"%s%s\\",BACKUP_DIR,ts);

    /* create snapshot dir (Windows) */
    char mkdir_cmd[MAX_PATH+20]; snprintf(mkdir_cmd,sizeof(mkdir_cmd),"mkdir \"%s\" 2>nul",snap_dir);
    system(mkdir_cmd);

    /* backup key data files */
    const char *files[]={USERS_FILE,LOGS_FILE,BLOCKCHAIN_FILE,PKG_FILE,NULL};
    const char *names[]={"users.dat","system.log","blockchain.dat","packages.dat",NULL};
    int count=0;
    for(int i=0;files[i];i++){
        char dst[MAX_PATH]; sprintf(dst,"%s%s",snap_dir,names[i]);
        backup_file(files[i],dst);
        printf("  Backed up: %s\n",names[i]);
        count++;
    }
    printf(CLR_BGREEN "\n  Snapshot created: %s (%d files)\n\n" CLR_RESET,ts,count);
    char msg[128]; sprintf(msg,"Backup snapshot created: %s",ts);
    log_write(LOG_INFO,msg);
}

void cmd_restore(int argc, char **argv){
    if(argc<2){ printf("Usage: restore <snapshot_name>\n"); return; }
    char snap_dir[MAX_PATH]; sprintf(snap_dir,"%s%s\\",BACKUP_DIR,argv[1]);
    const char *names[]={"users.dat","system.log","blockchain.dat","packages.dat",NULL};
    const char *files[]={USERS_FILE,LOGS_FILE,BLOCKCHAIN_FILE,PKG_FILE,NULL};
    for(int i=0;names[i];i++){
        char src[MAX_PATH*2]; snprintf(src,sizeof(src),"%s%s",snap_dir,names[i]);
        backup_file(src,files[i]);
        printf("  Restored: %s\n",names[i]);
    }
    printf(CLR_BGREEN "\n  Snapshot '%s' restored.\n\n" CLR_RESET,argv[1]);
    log_write(LOG_INFO,"System restored from snapshot.");
}

void cmd_recover(int argc, char **argv){
    (void)argc;(void)argv;
    /* List available snapshots */
    printf("\n  Available snapshots:\n");
    char cmd[MAX_PATH]; sprintf(cmd,"dir /b \"%s\" 2>nul",BACKUP_DIR);
    system(cmd);
    printf("\n  Use: restore <snapshot_name>\n\n");
}
