#include "blockchain.h"
#include "logs.h"

static Block chain[BC_MAX_BLOCKS];
static int chain_len = 0;

static void block_hash(Block *b, char out[65]){
    char input[512];
    snprintf(input,sizeof(input),"%d%s%s%s%d",
             b->index,b->timestamp,b->data,b->prev_hash,b->nonce);
    sha256_hex(input,out);
}

static void get_ts(char buf[32]){
    time_t t=time(NULL); struct tm *tm=localtime(&t);
    strftime(buf,32,"%Y-%m-%d %H:%M:%S",tm);
}

/* ─── Genesis block ─── */
static void create_genesis(void){
    Block *g=&chain[0];
    g->index=0;
    get_ts(g->timestamp);
    strcpy(g->data,"Genesis Block - CortexOS Blockchain");
    memset(g->prev_hash,'0',64); g->prev_hash[64]='\0';
    g->nonce=0;
    block_hash(g,g->hash);
    chain_len=1;
}

void bc_init(void){
    bc_load();
    if(chain_len==0) create_genesis();
}

/* ─── Mine ─── */
void bc_mine(const char *data){
    if(chain_len>=BC_MAX_BLOCKS){ PRINT_ERR("Chain full."); return; }
    Block *b=&chain[chain_len];
    b->index=chain_len;
    get_ts(b->timestamp);
    strncpy(b->data,data,255);
    memcpy(b->prev_hash,chain[chain_len-1].hash,65);
    b->nonce=0;
    /* Proof of work: hash must start with "00" */
    printf("  Mining block %d",b->index); fflush(stdout);
    do {
        b->nonce++;
        block_hash(b,b->hash);
        if(b->nonce%1000==0){ printf("."); fflush(stdout); }
    } while(strncmp(b->hash,"00",2)!=0);
    chain_len++;
    printf("\n");
    bc_save();
    printf(CLR_BGREEN "  Block %d mined! Nonce: %d\n  Hash: %s\n" CLR_RESET,
           b->index,b->nonce,b->hash);
    log_write(LOG_INFO,"Block mined on blockchain.");
}

/* ─── Print ─── */
void bc_print_chain(void){
    printf("\n%s╔═══════════════════════════════════════════════════════════╗%s\n",CLR_CYAN,CLR_RESET);
    printf("%s║                  CORTEX BLOCKCHAIN                       ║%s\n",CLR_CYAN,CLR_RESET);
    printf("%s╚═══════════════════════════════════════════════════════════╝%s\n\n",CLR_CYAN,CLR_RESET);
    for(int i=0;i<chain_len;i++){
        Block *b=&chain[i];
        printf("  %s┌─ Block #%d ─────────────────────────────────────────┐%s\n",CLR_YELLOW,b->index,CLR_RESET);
        printf("  │  Time  : %s\n",b->timestamp);
        printf("  │  Data  : %s\n",b->data);
        printf("  │  Prev  : %.20s...\n",b->prev_hash);
        printf("  │  Hash  : %s%.20s...%s\n",CLR_BGREEN,b->hash,CLR_RESET);
        printf("  │  Nonce : %d\n",b->nonce);
        printf("  %s└───────────────────────────────────────────────────┘%s\n",CLR_YELLOW,CLR_RESET);
    }
    printf("\n");
}

/* ─── Verify ─── */
int bc_verify(void){
    for(int i=1;i<chain_len;i++){
        char computed[65]; block_hash(&chain[i],computed);
        if(!STR_EQ(computed,chain[i].hash)){
            printf(CLR_RED "  INVALID: Block %d hash mismatch!\n" CLR_RESET,i); return 0;
        }
        if(!STR_EQ(chain[i].prev_hash,chain[i-1].hash)){
            printf(CLR_RED "  INVALID: Block %d previous hash mismatch!\n" CLR_RESET,i); return 0;
        }
    }
    printf(CLR_BGREEN "  Chain verified. All %d blocks are valid.\n\n" CLR_RESET,chain_len);
    return 1;
}

/* ─── Transaction ─── */
void bc_transaction(const char *from, const char *to, double amount){
    char data[256];
    snprintf(data,sizeof(data),"TX: %s -> %s : %.2f CortexCoin",from,to,amount);
    bc_mine(data);
}

/* ─── Persistence ─── */
void bc_save(void){
    FILE *f=fopen(BLOCKCHAIN_FILE,"wb");
    if(!f) return;
    fwrite(&chain_len,sizeof(int),1,f);
    fwrite(chain,sizeof(Block),chain_len,f);
    fclose(f);
}
void bc_load(void){
    FILE *f=fopen(BLOCKCHAIN_FILE,"rb");
    if(!f){ chain_len=0; return; }
    fread(&chain_len,sizeof(int),1,f);
    if(chain_len<0||chain_len>BC_MAX_BLOCKS) chain_len=0;
    fread(chain,sizeof(Block),chain_len,f);
    fclose(f);
}

/* ─── Shell commands ─── */
void cmd_blockchain(int argc, char **argv){ (void)argc;(void)argv; bc_print_chain(); }
void cmd_mine(int argc, char **argv){
    if(argc<2){ printf("Usage: mine \"<data>\"\n"); return; }
    bc_mine(argv[1]);
}
void cmd_bc_verify(int argc, char **argv){ (void)argc;(void)argv; bc_verify(); }
void cmd_transaction(int argc, char **argv){
    if(argc<4){ printf("Usage: transaction <from> <to> <amount>\n"); return; }
    bc_transaction(argv[1],argv[2],atof(argv[3]));
}
