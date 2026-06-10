#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "common.h"

#define FS_MAX_CHILDREN 32
#define FS_MAX_CONTENT  8192

typedef enum { FS_FILE=0, FS_DIR=1 } FSNodeType;

typedef struct FSNode {
    char         name[MAX_FILENAME];
    FSNodeType   type;
    char         owner[MAX_USERNAME];
    int          permissions; /* rwx bits for owner */
    char         content[FS_MAX_CONTENT];
    int          size;
    char         created[32];
    char         modified[32];
    struct FSNode *children[FS_MAX_CHILDREN];
    int           child_count;
    struct FSNode *parent;
} FSNode;

/* ── API ── */
void     fs_init(void);
void     fs_shutdown(void);
FSNode  *fs_mkdir(const char *path);
FSNode  *fs_touch(const char *path);
FSNode  *fs_find(const char *path);
int      fs_rm(const char *path);
int      fs_mv(const char *src, const char *dst);
int      fs_cp(const char *src, const char *dst);
void     fs_ls(const char *path, int long_fmt);
char    *fs_pwd(void);
int      fs_cd(const char *path);
int      fs_write(const char *path, const char *content, int append);
char    *fs_read(const char *path);
int      fs_chmod(const char *path, int perms);
int      fs_chown(const char *path, const char *owner);
void     fs_tree(FSNode *node, int depth);

/* ── Shell commands ── */
void cmd_mkdir(int argc, char **argv);
void cmd_touch(int argc, char **argv);
void cmd_ls(int argc, char **argv);
void cmd_cd(int argc, char **argv);
void cmd_pwd(int argc, char **argv);
void cmd_rm(int argc, char **argv);
void cmd_mv(int argc, char **argv);
void cmd_cp(int argc, char **argv);
void cmd_chmod(int argc, char **argv);
void cmd_chown(int argc, char **argv);
void cmd_cat(int argc, char **argv);
void cmd_tree(int argc, char **argv);

#endif
