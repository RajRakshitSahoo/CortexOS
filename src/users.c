#include "users.h"
#include "security.h"
#include "logs.h"

static User users[MAX_USERS];
static int  user_count = 0;

/* ─── Internal helpers ─── */
static void hash_password(const char *pass, char out[65]){
    sha256_hex(pass, out);
}

static void get_timestamp(char buf[32]){
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buf,32,"%Y-%m-%d %H:%M:%S",tm_info);
}

/* ─── Persistence ─── */
int users_save(void){
    FILE *f = fopen(USERS_FILE,"wb");
    if(!f) return 0;
    fwrite(&user_count,sizeof(int),1,f);
    fwrite(users,sizeof(User),user_count,f);
    fclose(f);
    return 1;
}

int users_load(void){
    FILE *f = fopen(USERS_FILE,"rb");
    if(!f) return 0;
    fread(&user_count,sizeof(int),1,f);
    if(user_count<0||user_count>MAX_USERS) user_count=0;
    fread(users,sizeof(User),user_count,f);
    fclose(f);
    return 1;
}

/* ─── Init ─── */
int users_init(void){
    users_load();
    /* create default admin if no users */
    if(user_count==0){
        user_signup("admin","Admin@123","admin@cortexos.local",1);
        printf(CLR_YELLOW "  [BOOT] Default admin created: admin / Admin@123\n" CLR_RESET);
    }
    return 1;
}

/* ─── Lookup ─── */
int user_exists(const char *username){
    for(int i=0;i<user_count;i++)
        if(users[i].active && STR_EQ(users[i].username,username)) return 1;
    return 0;
}

User *user_find(const char *username){
    for(int i=0;i<user_count;i++)
        if(users[i].active && STR_EQ(users[i].username,username)) return &users[i];
    return NULL;
}

/* ─── Sign up ─── */
int user_signup(const char *username, const char *password,
                const char *email, int is_admin){
    if(user_count>=MAX_USERS){ PRINT_ERR("User limit reached."); return 0; }
    if(user_exists(username)){  PRINT_ERR("Username already taken."); return 0; }
    if(strlen(username)<3){     PRINT_ERR("Username too short (min 3)."); return 0; }
    if(strlen(password)<6){     PRINT_ERR("Password too short (min 6)."); return 0; }

    User *u = &users[user_count++];
    strncpy(u->username, username, MAX_USERNAME-1);
    strncpy(u->email, email?email:"", 63);
    hash_password(password, u->password_hash);
    u->is_admin = is_admin;
    u->active   = 1;
    get_timestamp(u->created_at);
    users_save();

    char msg[128]; sprintf(msg,"New user registered: %s",username);
    log_write(LOG_AUTH,msg);
    return 1;
}

/* ─── Login ─── */
int user_login(const char *username, const char *password){
    User *u = user_find(username);
    if(!u){ PRINT_ERR("User not found."); return 0; }
    char hash[65]; hash_password(password,hash);
    if(!STR_EQ(hash,u->password_hash)){ PRINT_ERR("Invalid password."); return 0; }

    strncpy(g_current_user,username,MAX_USERNAME-1);
    g_is_admin  = u->is_admin;
    g_logged_in = 1;
    sprintf(g_current_dir,"/%s",username);

    char msg[128]; sprintf(msg,"User logged in: %s",username);
    log_write(LOG_AUTH,msg);
    return 1;
}

/* ─── Logout ─── */
void user_logout(void){
    char msg[128]; sprintf(msg,"User logged out: %s",g_current_user);
    log_write(LOG_AUTH,msg);
    g_current_user[0]='\0';
    g_logged_in=0;
    g_is_admin=0;
    strcpy(g_current_dir,"/");
    PRINT_OK("Logged out successfully.");
}

/* ─── Change password ─── */
int user_change_password(const char *username, const char *old_pass, const char *new_pass){
    User *u = user_find(username);
    if(!u){ PRINT_ERR("User not found."); return 0; }
    char hash[65]; hash_password(old_pass,hash);
    if(!STR_EQ(hash,u->password_hash)){ PRINT_ERR("Current password incorrect."); return 0; }
    if(strlen(new_pass)<6){ PRINT_ERR("New password too short (min 6)."); return 0; }
    hash_password(new_pass,u->password_hash);
    users_save();
    PRINT_OK("Password changed successfully.");
    log_write(LOG_AUTH,"Password changed.");
    return 1;
}

/* ─── Whoami ─── */
void user_whoami(void){
    if(!g_logged_in){ PRINT_ERR("Not logged in."); return; }
    User *u = user_find(g_current_user);
    printf("\n  %s╔══════════════════════════════╗%s\n",CLR_CYAN,CLR_RESET);
    printf("  %s║         WHO AM I             ║%s\n",CLR_CYAN,CLR_RESET);
    printf("  %s╚══════════════════════════════╝%s\n",CLR_CYAN,CLR_RESET);
    printf("  Username : %s%s%s\n",CLR_BGREEN,g_current_user,CLR_RESET);
    if(u){
        printf("  Email    : %s\n",u->email);
        printf("  Role     : %s\n",u->is_admin?"Administrator":"User");
        printf("  Created  : %s\n",u->created_at);
    }
    printf("  Directory: %s\n\n",g_current_dir);
}

/* ─── List users (admin) ─── */
void users_list(void){
    if(!g_is_admin){ PRINT_ERR("Permission denied. Admin only."); return; }
    printf("\n  %-16s %-24s %-12s %-20s\n","Username","Email","Role","Created");
    printf("  %s\n","-----------------------------------------------------------------");
    for(int i=0;i<user_count;i++){
        if(!users[i].active) continue;
        printf("  %-16s %-24s %-12s %-20s\n",
               users[i].username, users[i].email,
               users[i].is_admin?"admin":"user", users[i].created_at);
    }
    printf("\n");
}

/* ─── Shell commands ─── */
static char *prompt_hidden(const char *label, char *buf, int sz){
    printf("%s",label);
    fflush(stdout);
    /* Simple read (echo ON — hiding passwords requires platform API) */
    if(!fgets(buf,sz,stdin)) buf[0]='\0';
    buf[strcspn(buf,"\r\n")]='\0';
    return buf;
}

void cmd_signup(int argc, char **argv){
    char uname[MAX_USERNAME]={0}, pass[MAX_PASSWORD]={0}, email[64]={0};
    if(argc>=2) strncpy(uname,argv[1],MAX_USERNAME-1);
    else { printf("Username: "); fflush(stdout); fgets(uname,sizeof(uname),stdin); uname[strcspn(uname,"\r\n")]='\0'; }
    prompt_hidden("Password: ",pass,sizeof(pass));
    printf("Email   : "); fflush(stdout); fgets(email,sizeof(email),stdin); email[strcspn(email,"\r\n")]='\0';
    if(user_signup(uname,pass,email,0)) PRINT_OK("Account created. You can now login.");
}

void cmd_login(int argc, char **argv){
    if(g_logged_in){ PRINT_WARN("Already logged in. Use 'logout' first."); return; }
    char uname[MAX_USERNAME]={0}, pass[MAX_PASSWORD]={0};
    if(argc>=2) strncpy(uname,argv[1],MAX_USERNAME-1);
    else { printf("Username: "); fflush(stdout); fgets(uname,sizeof(uname),stdin); uname[strcspn(uname,"\r\n")]='\0'; }
    prompt_hidden("Password: ",pass,sizeof(pass));
    if(user_login(uname,pass)) printf(CLR_BGREEN "\n  Welcome, %s!\n\n" CLR_RESET, uname);
}

void cmd_logout(int argc, char **argv){ (void)argc;(void)argv; user_logout(); }

void cmd_whoami(int argc, char **argv){ (void)argc;(void)argv; user_whoami(); }

void cmd_passwd(int argc, char **argv){
    (void)argc;(void)argv;
    if(!g_logged_in){ PRINT_ERR("Not logged in."); return; }
    char old_p[MAX_PASSWORD], new_p[MAX_PASSWORD], confirm[MAX_PASSWORD];
    prompt_hidden("Current password : ",old_p,sizeof(old_p));
    prompt_hidden("New password     : ",new_p,sizeof(new_p));
    prompt_hidden("Confirm password : ",confirm,sizeof(confirm));
    if(!STR_EQ(new_p,confirm)){ PRINT_ERR("Passwords do not match."); return; }
    user_change_password(g_current_user,old_p,new_p);
}
