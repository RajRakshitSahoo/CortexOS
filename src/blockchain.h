#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include "common.h"
#include "security.h"

#define BC_MAX_BLOCKS 256

typedef struct {
    int    index;
    char   timestamp[32];
    char   data[256];
    char   prev_hash[65];
    char   hash[65];
    int    nonce;
    int    mined_by[MAX_USERNAME];
} Block;

void  bc_init(void);
void  bc_mine(const char *data);
void  bc_print_chain(void);
int   bc_verify(void);
void  bc_transaction(const char *from, const char *to, double amount);
void  bc_save(void);
void  bc_load(void);

void cmd_blockchain(int argc, char **argv);
void cmd_mine(int argc, char **argv);
void cmd_bc_verify(int argc, char **argv);
void cmd_transaction(int argc, char **argv);

#endif
