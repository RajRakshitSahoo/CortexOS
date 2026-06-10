#include "database.h"
#include "logs.h"

static Database db;

/* ─── Persistence ─── */
void db_save(void){
    char path[MAX_PATH]; sprintf(path,"%scortex.db",DB_DIR);
    FILE *f=fopen(path,"wb");
    if(!f) return;
    fwrite(&db,sizeof(Database),1,f);
    fclose(f);
}
void db_load(void){
    char path[MAX_PATH]; sprintf(path,"%scortex.db",DB_DIR);
    FILE *f=fopen(path,"rb");
    if(!f){ memset(&db,0,sizeof(db)); strcpy(db.name,"cortex"); return; }
    fread(&db,sizeof(Database),1,f);
    fclose(f);
}
void db_init(void){ db_load(); }

/* ─── Find table ─── */
static DBTable *find_table(const char *name){
    for(int i=0;i<db.table_count;i++) if(STR_EQ(db.tables[i].name,name)) return &db.tables[i];
    return NULL;
}

/* ─── CREATE TABLE ─── */
int db_create_table(const char *tname, const char **cols, int ncols){
    if(find_table(tname)){ PRINT_ERR("Table already exists."); return 0; }
    if(db.table_count>=DB_MAX_TABLES){ PRINT_ERR("Max tables reached."); return 0; }
    DBTable *t=&db.tables[db.table_count++];
    memset(t,0,sizeof(DBTable));
    strncpy(t->name,tname,63);
    for(int i=0;i<ncols&&i<DB_MAX_COLS;i++) strncpy(t->col_names[i],cols[i],DB_COL_WIDTH-1);
    t->col_count=ncols;
    db_save();
    return 1;
}

/* ─── INSERT ─── */
int db_insert(const char *tname, const char **vals, int nvals){
    DBTable *t=find_table(tname);
    if(!t){ PRINT_ERR("Table not found."); return 0; }
    if(t->row_count>=DB_MAX_ROWS){ PRINT_ERR("Table full."); return 0; }
    int r=t->row_count++;
    for(int i=0;i<nvals&&i<t->col_count;i++) strncpy(t->rows[r][i],vals[i],DB_COL_WIDTH-1);
    db_save();
    return 1;
}

/* ─── SELECT ─── */
void db_select(const char *tname, const char *where_col, const char *where_val){
    DBTable *t=find_table(tname);
    if(!t){ PRINT_ERR("Table not found."); return; }
    /* header */
    printf("\n  ");
    for(int c=0;c<t->col_count;c++) printf("%-*s  ",DB_COL_WIDTH-2,t->col_names[c]);
    printf("\n  ");
    for(int c=0;c<t->col_count;c++){ for(int k=0;k<DB_COL_WIDTH-1;k++) printf("-"); printf("  "); }
    printf("\n");
    /* find where col index */
    int wc=-1;
    if(where_col) for(int c=0;c<t->col_count;c++) if(STR_EQ(t->col_names[c],where_col)) wc=c;
    int count=0;
    for(int r=0;r<t->row_count;r++){
        if(wc>=0&&where_val&&!STR_EQ(t->rows[r][wc],where_val)) continue;
        printf("  ");
        for(int c=0;c<t->col_count;c++) printf("%-*s  ",DB_COL_WIDTH-2,t->rows[r][c]);
        printf("\n"); count++;
    }
    printf("\n  %d row(s) returned.\n\n",count);
}

/* ─── DELETE ─── */
int db_delete(const char *tname, const char *where_col, const char *where_val){
    DBTable *t=find_table(tname);
    if(!t){ PRINT_ERR("Table not found."); return 0; }
    int wc=-1;
    for(int c=0;c<t->col_count;c++) if(STR_EQ(t->col_names[c],where_col)) wc=c;
    if(wc<0){ PRINT_ERR("Column not found."); return 0; }
    int del=0;
    for(int r=0;r<t->row_count;r++){
        if(STR_EQ(t->rows[r][wc],where_val)){
            for(int k=r;k<t->row_count-1;k++) memcpy(t->rows[k],t->rows[k+1],sizeof(t->rows[k]));
            t->row_count--; r--; del++;
        }
    }
    db_save();
    printf("  %d row(s) deleted.\n",del);
    return del;
}

void db_show_tables(void){
    printf("\n  Tables in '%s':\n",db.name);
    if(db.table_count==0){ printf("  (none)\n\n"); return; }
    for(int i=0;i<db.table_count;i++)
        printf("  [%d] %s (%d rows, %d cols)\n",i+1,db.tables[i].name,db.tables[i].row_count,db.tables[i].col_count);
    printf("\n");
}

/* ─── Mini SQL parser ─── */
static void parse_sql(char *sql){
    /* Uppercase copy for keyword matching */
    char upper[MAX_CMD]; strncpy(upper,sql,MAX_CMD-1);
    for(int i=0;upper[i];i++) upper[i]=(char)toupper((unsigned char)upper[i]);

    if(strncmp(upper,"SHOW TABLES",11)==0){ db_show_tables(); return; }

    if(strncmp(upper,"CREATE TABLE",12)==0){
        /* CREATE TABLE name (col1, col2, ...) */
        char name[64], cols_str[256];
        if(sscanf(sql,"CREATE TABLE %63s (%255[^)])",name,cols_str)<2){
            if(sscanf(sql,"create table %63s (%255[^)])",name,cols_str)<2){
                PRINT_ERR("Syntax: CREATE TABLE name (col1,col2,...)"); return;
            }
        }
        const char *cols[DB_MAX_COLS]; int ncols=0;
        char *tok=strtok(cols_str,",");
        char col_bufs[DB_MAX_COLS][DB_COL_WIDTH];
        while(tok&&ncols<DB_MAX_COLS){
            while(*tok==' '||*tok=='\t') tok++;
            strncpy(col_bufs[ncols],tok,DB_COL_WIDTH-1);
            /* trim trailing space */
            int l=(int)strlen(col_bufs[ncols]);
            while(l>0&&(col_bufs[ncols][l-1]==' '||col_bufs[ncols][l-1]=='\t')) col_bufs[ncols][--l]='\0';
            cols[ncols]=col_bufs[ncols]; ncols++;
            tok=strtok(NULL,",");
        }
        if(db_create_table(name,cols,ncols))
            printf(CLR_BGREEN "  Table '%s' created with %d column(s).\n" CLR_RESET,name,ncols);
        return;
    }

    if(strncmp(upper,"INSERT INTO",11)==0){
        /* INSERT INTO name VALUES (v1,v2,...) */
        char name[64], vals_str[256];
        /* try both cases */
        int ok=0;
        if(sscanf(sql,"INSERT INTO %63s VALUES (%255[^)])",name,vals_str)==2) ok=1;
        if(!ok && sscanf(sql,"insert into %63s values (%255[^)])",name,vals_str)==2) ok=1;
        if(!ok){ PRINT_ERR("Syntax: INSERT INTO table VALUES (v1,v2,...)"); return; }
        const char *vals[DB_MAX_COLS]; int nv=0;
        char val_bufs[DB_MAX_COLS][DB_COL_WIDTH];
        char *tok=strtok(vals_str,",");
        while(tok&&nv<DB_MAX_COLS){
            while(*tok==' '||*tok=='\'' ) tok++;
            strncpy(val_bufs[nv],tok,DB_COL_WIDTH-1);
            int l=(int)strlen(val_bufs[nv]);
            while(l>0&&(val_bufs[nv][l-1]==' '||val_bufs[nv][l-1]=='\'')) val_bufs[nv][--l]='\0';
            vals[nv]=val_bufs[nv]; nv++;
            tok=strtok(NULL,",");
        }
        if(db_insert(name,vals,nv)) printf(CLR_BGREEN "  1 row inserted.\n" CLR_RESET);
        return;
    }

    if(strncmp(upper,"SELECT",6)==0){
        /* SELECT * FROM name [WHERE col=val] */
        char name[64]; char where_col[DB_COL_WIDTH]="", where_val[DB_COL_WIDTH]="";
        char *from_ptr=strstr(sql,"FROM");
        if(!from_ptr) from_ptr=strstr(sql,"from");
        if(!from_ptr){ PRINT_ERR("Syntax: SELECT * FROM table [WHERE col=val]"); return; }
        sscanf(from_ptr+5,"%63s",name);
        char *where_ptr=strstr(sql,"WHERE");
        if(!where_ptr) where_ptr=strstr(sql,"where");
        if(where_ptr){
            char eq_str[64]; sscanf(where_ptr+6,"%63s",eq_str);
            char *eq=strchr(eq_str,'=');
            if(eq){ *eq='\0'; strncpy(where_col,eq_str,DB_COL_WIDTH-1); strncpy(where_val,eq+1,DB_COL_WIDTH-1); }
        }
        db_select(name,where_col[0]?where_col:NULL,where_val[0]?where_val:NULL);
        return;
    }

    if(strncmp(upper,"DELETE",6)==0){
        /* DELETE FROM name WHERE col=val */
        char name[64],eq_str[64];
        char *from_ptr=strstr(sql,"FROM"); if(!from_ptr) from_ptr=strstr(sql,"from");
        char *where_ptr=strstr(sql,"WHERE"); if(!where_ptr) where_ptr=strstr(sql,"where");
        if(!from_ptr||!where_ptr){ PRINT_ERR("Syntax: DELETE FROM table WHERE col=val"); return; }
        sscanf(from_ptr+5,"%63s",name);
        sscanf(where_ptr+6,"%63s",eq_str);
        char *eq=strchr(eq_str,'=');
        if(!eq){ PRINT_ERR("Syntax: WHERE col=val"); return; }
        *eq='\0';
        db_delete(name,eq_str,eq+1);
        return;
    }

    printf("  Unknown SQL. Supported: CREATE TABLE, INSERT INTO, SELECT, DELETE, SHOW TABLES\n");
}

void cmd_db(int argc, char **argv){
    (void)argc;(void)argv;
    printf("\n%s╔══════════════════════════════════════════════╗%s\n",CLR_CYAN,CLR_RESET);
    printf("%s║         CORTEX DATABASE ENGINE               ║%s\n",CLR_CYAN,CLR_RESET);
    printf("%s╚══════════════════════════════════════════════╝%s\n",CLR_CYAN,CLR_RESET);
    printf("  Type SQL or 'exit' to quit.\n\n");
    char sql[MAX_CMD];
    while(1){
        printf(CLR_CYAN "  sql> " CLR_RESET); fflush(stdout);
        if(!fgets(sql,sizeof(sql),stdin)) break;
        sql[strcspn(sql,"\r\n")]='\0';
        if(STR_EQ(sql,"exit")||STR_EQ(sql,"quit")) break;
        if(STR_EMPTY(sql)) continue;
        parse_sql(sql);
    }
    printf("  Database session closed.\n\n");
}
