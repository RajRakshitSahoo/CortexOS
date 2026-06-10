#ifndef DATABASE_H
#define DATABASE_H

#include "common.h"

#define DB_MAX_TABLES  16
#define DB_MAX_COLS    16
#define DB_MAX_ROWS    256
#define DB_COL_WIDTH   32

typedef struct {
    char col_names[DB_MAX_COLS][DB_COL_WIDTH];
    int  col_count;
    char rows[DB_MAX_ROWS][DB_MAX_COLS][DB_COL_WIDTH];
    int  row_count;
    char name[64];
} DBTable;

typedef struct {
    DBTable tables[DB_MAX_TABLES];
    int     table_count;
    char    name[64];
} Database;

void db_init(void);
int  db_create_table(const char *tname, const char **cols, int ncols);
int  db_insert(const char *tname, const char **vals, int nvals);
void db_select(const char *tname, const char *where_col, const char *where_val);
int  db_delete(const char *tname, const char *where_col, const char *where_val);
void db_show_tables(void);
void db_save(void);
void db_load(void);

void cmd_db(int argc, char **argv);

#endif
