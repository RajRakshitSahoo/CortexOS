#ifndef MEMORY_H
#define MEMORY_H

#include "common.h"

#define MEM_TOTAL      1024   /* KB */
#define MEM_MAX_BLOCKS  64

/* Renamed to avoid conflict with Windows windef.h MEM_FREE / MEM_USED */
typedef enum { MBLK_FREE=0, MBLK_USED=1 } MemStatus;

typedef struct {
    int       id;
    int       start;    /* KB offset */
    int       size;     /* KB */
    MemStatus status;
    char      owner[MAX_USERNAME];
    char      label[32];
} MemBlock;

void mem_init(void);
int  mem_alloc_first(int size, const char *label);
int  mem_alloc_best(int size, const char *label);
int  mem_alloc_worst(int size, const char *label);
int  mem_free(int block_id);
void mem_display(void);

void cmd_memory(int argc, char **argv);
void cmd_alloc(int argc, char **argv);
void cmd_free_mem(int argc, char **argv);

#endif
