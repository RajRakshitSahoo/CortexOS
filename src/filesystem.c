#include "filesystem.h"
#include "logs.h"

/* ─── Global VFS root ─── */
static FSNode *vfs_root = NULL;
static FSNode *vfs_cwd  = NULL;

/* ─── Helpers ─── */
static void set_time(char buf[32]){
    time_t t=time(NULL); struct tm *tm=localtime(&t);
    strftime(buf,32,"%Y-%m-%d %H:%M",tm);
}

static FSNode *node_alloc(const char *name, FSNodeType type){
    FSNode *n = (FSNode*)calloc(1,sizeof(FSNode));
    if(!n) return NULL;
    strncpy(n->name,name,MAX_FILENAME-1);
    n->type = type;
    strncpy(n->owner, g_current_user[0]?g_current_user:"root", MAX_USERNAME-1);
    n->permissions = PERM_READ|PERM_WRITE|(type==FS_DIR?PERM_EXECUTE:0);
    set_time(n->created);
    set_time(n->modified);
    return n;
}

static void node_free(FSNode *n){
    if(!n) return;
    for(int i=0;i<n->child_count;i++) node_free(n->children[i]);
    free(n);
}

/* ─── Path resolution ─── */
/* Returns the node for the given absolute path */
static FSNode *resolve_abs(const char *path){
    if(!path||path[0]=='\0') return vfs_cwd;
    FSNode *cur = (path[0]=='/')?vfs_root:vfs_cwd;
    char buf[MAX_PATH]; strncpy(buf,path,MAX_PATH-1);
    char *tok = strtok(buf,"/\\");
    while(tok){
        if(STR_EQ(tok,".")) { tok=strtok(NULL,"/\\"); continue; }
        if(STR_EQ(tok,"..")){
            if(cur->parent) cur=cur->parent;
            tok=strtok(NULL,"/\\"); continue;
        }
        FSNode *found=NULL;
        for(int i=0;i<cur->child_count;i++)
            if(STR_EQ(cur->children[i]->name,tok)){ found=cur->children[i]; break; }
        if(!found) return NULL;
        cur=found;
        tok=strtok(NULL,"/\\");
    }
    return cur;
}

/* Split path into parent-path and filename */
static void split_path(const char *path, char *parent, char *name){
    char buf[MAX_PATH]; strncpy(buf,path,MAX_PATH-1);
    /* trim trailing slash */
    int len=(int)strlen(buf);
    while(len>1&&(buf[len-1]=='/'||buf[len-1]=='\\')) buf[--len]='\0';
    char *last = strrchr(buf,'/');
    if(!last) last=strrchr(buf,'\\');
    if(last){ *last='\0'; strcpy(parent,buf); strcpy(name,last+1); }
    else { strcpy(parent,""); strcpy(name,buf); }
}

/* Full path string of a node */
static void node_path(FSNode *n, char *buf){
    if(!n){ strcpy(buf,"/"); return; }
    if(n==vfs_root){ strcpy(buf,"/"); return; }
    char parent_buf[MAX_PATH];
    node_path(n->parent, parent_buf);
    if(STR_EQ(parent_buf,"/")) snprintf(buf,MAX_PATH,"/%s",n->name);
    else snprintf(buf,MAX_PATH,"%s/%s",parent_buf,n->name);
}

/* ─── Init ─── */
void fs_init(void){
    vfs_root = node_alloc("/", FS_DIR);
    vfs_root->parent = vfs_root; /* root's parent is itself */
    vfs_cwd  = vfs_root;

    /* Create default directories */
    const char *defaults[] = {"/home","/etc","/tmp","/bin","/var",NULL};
    for(int i=0;defaults[i];i++){
        FSNode *d=node_alloc(defaults[i]+1,FS_DIR);
        d->parent=vfs_root;
        vfs_root->children[vfs_root->child_count++]=d;
    }
    /* Create current user's home */
    if(g_current_user[0]){
        char hdir[64]; sprintf(hdir,"/home/%s",g_current_user);
        fs_mkdir(hdir);
        fs_cd(hdir);
    }
}

void fs_shutdown(void){ node_free(vfs_root); vfs_root=NULL; vfs_cwd=NULL; }

/* ─── mkdir ─── */
FSNode *fs_mkdir(const char *path){
    char par[MAX_PATH], name[MAX_FILENAME];
    split_path(path,par,name);
    FSNode *parent = STR_EMPTY(par)?vfs_root:resolve_abs(par);
    if(!parent){ PRINT_ERR("Parent directory not found."); return NULL; }
    if(parent->type!=FS_DIR){ PRINT_ERR("Not a directory."); return NULL; }
    /* check dup */
    for(int i=0;i<parent->child_count;i++)
        if(STR_EQ(parent->children[i]->name,name)){ PRINT_ERR("Already exists."); return NULL; }
    if(parent->child_count>=FS_MAX_CHILDREN){ PRINT_ERR("Directory full."); return NULL; }
    FSNode *n=node_alloc(name,FS_DIR);
    n->parent=parent;
    parent->children[parent->child_count++]=n;
    char msg[128]; sprintf(msg,"mkdir: %s",path); log_write(LOG_FS,msg);
    return n;
}

/* ─── touch ─── */
FSNode *fs_touch(const char *path){
    char par[MAX_PATH], name[MAX_FILENAME];
    split_path(path,par,name);
    FSNode *parent = STR_EMPTY(par)?vfs_cwd:resolve_abs(par);
    if(!parent){ PRINT_ERR("Parent directory not found."); return NULL; }
    /* if exists, update mtime */
    for(int i=0;i<parent->child_count;i++)
        if(STR_EQ(parent->children[i]->name,name)){ set_time(parent->children[i]->modified); return parent->children[i]; }
    if(parent->child_count>=FS_MAX_CHILDREN){ PRINT_ERR("Directory full."); return NULL; }
    FSNode *n=node_alloc(name,FS_FILE);
    n->parent=parent;
    parent->children[parent->child_count++]=n;
    char msg[128]; sprintf(msg,"touch: %s",path); log_write(LOG_FS,msg);
    return n;
}

/* ─── find ─── */
FSNode *fs_find(const char *path){
    if(!path||STR_EMPTY(path)) return vfs_cwd;
    /* relative path → prepend cwd */
    if(path[0]!='/'){
        char full[MAX_PATH]; char cwd_buf[MAX_PATH];
        node_path(vfs_cwd,cwd_buf);
        if(STR_EQ(cwd_buf,"/")) snprintf(full,MAX_PATH,"/%s",path);
        else snprintf(full,MAX_PATH,"%s/%s",cwd_buf,path);
        return resolve_abs(full);
    }
    return resolve_abs(path);
}

/* ─── rm ─── */
int fs_rm(const char *path){
    FSNode *n = fs_find(path);
    if(!n){ PRINT_ERR("Not found."); return 0; }
    if(n==vfs_root){ PRINT_ERR("Cannot remove root."); return 0; }
    if(n->type==FS_DIR && n->child_count>0){ PRINT_ERR("Directory not empty."); return 0; }
    FSNode *par=n->parent;
    for(int i=0;i<par->child_count;i++){
        if(par->children[i]==n){
            for(int j=i;j<par->child_count-1;j++) par->children[j]=par->children[j+1];
            par->child_count--;
            break;
        }
    }
    node_free(n);
    char msg[128]; sprintf(msg,"rm: %s",path); log_write(LOG_FS,msg);
    return 1;
}

/* ─── mv ─── */
int fs_mv(const char *src, const char *dst){
    FSNode *n=fs_find(src); if(!n){ PRINT_ERR("Source not found."); return 0; }
    char dpar[MAX_PATH], dname[MAX_FILENAME];
    split_path(dst,dpar,dname);
    FSNode *dparent=STR_EMPTY(dpar)?vfs_cwd:resolve_abs(dpar);
    if(!dparent){ PRINT_ERR("Dest dir not found."); return 0; }
    /* detach from old parent */
    FSNode *opar=n->parent;
    for(int i=0;i<opar->child_count;i++)
        if(opar->children[i]==n){ for(int j=i;j<opar->child_count-1;j++) opar->children[j]=opar->children[j+1]; opar->child_count--; break; }
    strncpy(n->name,dname,MAX_FILENAME-1);
    n->parent=dparent;
    dparent->children[dparent->child_count++]=n;
    return 1;
}

/* ─── cp ─── */
int fs_cp(const char *src, const char *dst){
    FSNode *s=fs_find(src); if(!s||s->type!=FS_FILE){ PRINT_ERR("Source file not found."); return 0; }
    FSNode *d=fs_touch(dst); if(!d) return 0;
    memcpy(d->content,s->content,sizeof(s->content));
    d->size=s->size;
    return 1;
}

/* ─── ls ─── */
static const char *perm_str(int p){
    static char buf[4]; buf[0]=(p&PERM_READ)?'r':'-'; buf[1]=(p&PERM_WRITE)?'w':'-'; buf[2]=(p&PERM_EXECUTE)?'x':'-'; buf[3]='\0';
    return buf;
}
void fs_ls(const char *path, int long_fmt){
    FSNode *dir = (path&&!STR_EMPTY(path))?fs_find(path):vfs_cwd;
    if(!dir){ PRINT_ERR("Directory not found."); return; }
    if(dir->type!=FS_DIR){ PRINT_ERR("Not a directory."); return; }
    if(long_fmt) printf("\n  %-4s %-10s %-16s %-8s %-20s %s\n","Type","Perm","Owner","Size","Modified","Name");
    for(int i=0;i<dir->child_count;i++){
        FSNode *c=dir->children[i];
        if(long_fmt){
            const char *clr = (c->type==FS_DIR)?CLR_CYAN:CLR_WHITE;
            printf("  %s%-4s%s %-10s %-16s %-8d %-20s %s%s%s\n",
                   clr,(c->type==FS_DIR)?"DIR":"FILE",CLR_RESET,
                   perm_str(c->permissions),c->owner,c->size,c->modified,
                   clr,c->name,CLR_RESET);
        } else {
            if(c->type==FS_DIR) printf(CLR_CYAN "  %s/  " CLR_RESET, c->name);
            else printf("  %s  ",c->name);
        }
    }
    if(!long_fmt) printf("\n");
    printf("\n");
}

/* ─── pwd ─── */
char *fs_pwd(void){
    static char buf[MAX_PATH];
    node_path(vfs_cwd,buf);
    return buf;
}

/* ─── cd ─── */
int fs_cd(const char *path){
    if(!path||STR_EQ(path,"~")){
        char hdir[64]; sprintf(hdir,"/home/%s",g_current_user);
        FSNode *n=resolve_abs(hdir);
        if(n) vfs_cwd=n; else vfs_cwd=vfs_root;
        return 1;
    }
    FSNode *n=fs_find(path);
    if(!n){ PRINT_ERR("Directory not found."); return 0; }
    if(n->type!=FS_DIR){ PRINT_ERR("Not a directory."); return 0; }
    vfs_cwd=n;
    /* sync global cwd */
    node_path(vfs_cwd,g_current_dir);
    return 1;
}

/* ─── Write / Read ─── */
int fs_write(const char *path, const char *content, int append){
    FSNode *n=fs_find(path);
    if(!n) n=fs_touch(path);
    if(!n) return 0;
    if(n->type!=FS_FILE){ PRINT_ERR("Not a file."); return 0; }
    if(append){
        int rem=FS_MAX_CONTENT-n->size-1;
        strncat(n->content,content,rem);
    } else {
        strncpy(n->content,content,FS_MAX_CONTENT-1);
    }
    n->size=(int)strlen(n->content);
    set_time(n->modified);
    return 1;
}

char *fs_read(const char *path){
    FSNode *n=fs_find(path);
    if(!n||n->type!=FS_FILE) return NULL;
    return n->content;
}

/* ─── chmod / chown ─── */
int fs_chmod(const char *path, int perms){
    FSNode *n=fs_find(path); if(!n){ PRINT_ERR("Not found."); return 0; }
    n->permissions=perms&7;
    return 1;
}
int fs_chown(const char *path, const char *owner){
    if(!g_is_admin){ PRINT_ERR("Permission denied. Admin only."); return 0; }
    FSNode *n=fs_find(path); if(!n){ PRINT_ERR("Not found."); return 0; }
    strncpy(n->owner,owner,MAX_USERNAME-1);
    return 1;
}

/* ─── tree ─── */
void fs_tree(FSNode *node, int depth){
    if(!node) return;
    for(int i=0;i<depth;i++) printf("  ");
    if(node->type==FS_DIR) printf(CLR_CYAN "%s/" CLR_RESET "\n",node->name);
    else printf("%s\n",node->name);
    for(int i=0;i<node->child_count;i++) fs_tree(node->children[i],depth+1);
}

/* ─── Shell commands ─── */
void cmd_mkdir(int argc, char **argv){
    if(argc<2){ printf("Usage: mkdir <dir>\n"); return; }
    FSNode *n=fs_mkdir(argv[1]);
    if(n) printf("  Directory '%s' created.\n",argv[1]);
}
void cmd_touch(int argc, char **argv){
    if(argc<2){ printf("Usage: touch <file>\n"); return; }
    FSNode *n=fs_touch(argv[1]);
    if(n) printf("  File '%s' created.\n",argv[1]);
}
void cmd_ls(int argc, char **argv){
    int lflag=0; const char *path=NULL;
    for(int i=1;i<argc;i++){
        if(STR_EQ(argv[i],"-l")) lflag=1;
        else path=argv[i];
    }
    fs_ls(path,lflag);
}
void cmd_cd(int argc, char **argv){
    const char *p=(argc>=2)?argv[1]:"~";
    fs_cd(p);
}
void cmd_pwd(int argc, char **argv){ (void)argc;(void)argv; printf("  %s\n",fs_pwd()); }
void cmd_rm(int argc, char **argv){
    if(argc<2){ printf("Usage: rm <path>\n"); return; }
    if(fs_rm(argv[1])) printf("  Removed '%s'.\n",argv[1]);
}
void cmd_mv(int argc, char **argv){
    if(argc<3){ printf("Usage: mv <src> <dst>\n"); return; }
    if(fs_mv(argv[1],argv[2])) printf("  Moved '%s' → '%s'.\n",argv[1],argv[2]);
}
void cmd_cp(int argc, char **argv){
    if(argc<3){ printf("Usage: cp <src> <dst>\n"); return; }
    if(fs_cp(argv[1],argv[2])) printf("  Copied '%s' → '%s'.\n",argv[1],argv[2]);
}
void cmd_chmod(int argc, char **argv){
    if(argc<3){ printf("Usage: chmod <octal> <path>  e.g. chmod 7 file.txt\n"); return; }
    int p=(int)strtol(argv[1],NULL,8);
    if(fs_chmod(argv[2],p)) printf("  Permissions set to %o on '%s'.\n",p,argv[2]);
}
void cmd_chown(int argc, char **argv){
    if(argc<3){ printf("Usage: chown <user> <path>\n"); return; }
    if(fs_chown(argv[2],argv[1])) printf("  Owner of '%s' set to '%s'.\n",argv[2],argv[1]);
}
void cmd_cat(int argc, char **argv){
    if(argc<2){ printf("Usage: cat <file>\n"); return; }
    char *content=fs_read(argv[1]);
    if(!content){ PRINT_ERR("File not found or is a directory."); return; }
    printf("\n%s\n",content);
}
void cmd_tree(int argc, char **argv){
    FSNode *n=(argc>=2)?fs_find(argv[1]):vfs_cwd;
    if(!n){ PRINT_ERR("Not found."); return; }
    printf("\n");
    fs_tree(n,0);
    printf("\n");
}
