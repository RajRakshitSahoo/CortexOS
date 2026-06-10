#ifndef USERS_H
#define USERS_H

#include "common.h"

/* ── User record ── */
typedef struct {
    char  username[MAX_USERNAME];
    char  password_hash[65]; /* SHA-256 hex */
    char  email[64];
    int   is_admin;
    char  created_at[32];
    int   active;            /* 1 = exists */
} User;

/* ── API ── */
int  users_init(void);
int  user_signup(const char *username, const char *password, const char *email, int is_admin);
int  user_login(const char *username, const char *password);
void user_logout(void);
int  user_change_password(const char *username, const char *old_pass, const char *new_pass);
int  user_exists(const char *username);
User *user_find(const char *username);
void user_whoami(void);
void users_list(void);          /* admin only */
int  users_save(void);
int  users_load(void);

/* ── Shell commands ── */
void cmd_signup(int argc, char **argv);
void cmd_login(int argc, char **argv);
void cmd_logout(int argc, char **argv);
void cmd_whoami(int argc, char **argv);
void cmd_passwd(int argc, char **argv);

#endif
