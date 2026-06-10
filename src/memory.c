#include "memory.h"

static MemBlock mem_blocks[MEM_MAX_BLOCKS];
static int mem_block_count = 0;
static int next_block_id   = 1;

void mem_init(void){
    mem_block_count=0;
    /* One large free block */
    MemBlock *b=&mem_blocks[mem_block_count++];
    b->id=next_block_id++;
    b->start=0; b->size=MEM_TOTAL;
    b->status=MBLK_FREE;
    strcpy(b->owner,"system");
    strcpy(b->label,"[Free]");

    /* Pre-allocate OS memory */
    mem_alloc_best(64,"Kernel");
    mem_alloc_best(32,"VFS");
    mem_alloc_best(16,"Drivers");
}

static int insert_block(int after, int start, int size, MemStatus status, const char *lbl){
    if(mem_block_count>=MEM_MAX_BLOCKS) return -1;
    /* shift right */
    for(int i=mem_block_count;i>after+1;i--) mem_blocks[i]=mem_blocks[i-1];
    mem_block_count++;
    MemBlock *b=&mem_blocks[after+1];
    b->id=next_block_id++;
    b->start=start; b->size=size;
    b->status=status;
    strncpy(b->owner,g_current_user[0]?g_current_user:"system",MAX_USERNAME-1);
    strncpy(b->label,lbl,31);
    return b->id;
}

/* ─── First Fit ─── */
int mem_alloc_first(int size, const char *label){
    for(int i=0;i<mem_block_count;i++){
        if(mem_blocks[i].status==MBLK_FREE && mem_blocks[i].size>=size){
            int orig_size=mem_blocks[i].size;
            int orig_start=mem_blocks[i].start;
            /* convert this block to used */
            mem_blocks[i].id=next_block_id++;
            mem_blocks[i].size=size;
            mem_blocks[i].status=MBLK_USED;
            strncpy(mem_blocks[i].owner,g_current_user[0]?g_current_user:"system",MAX_USERNAME-1);
            strncpy(mem_blocks[i].label,label,31);
            /* insert remaining free */
            if(orig_size>size) insert_block(i,orig_start+size,orig_size-size,MBLK_FREE,"[Free]");
            return mem_blocks[i].id;
        }
    }
    PRINT_ERR("Not enough memory (First Fit)."); return -1;
}

/* ─── Best Fit ─── */
int mem_alloc_best(int size, const char *label){
    int best=-1, best_size=MEM_TOTAL+1;
    for(int i=0;i<mem_block_count;i++){
        if(mem_blocks[i].status==MBLK_FREE && mem_blocks[i].size>=size && mem_blocks[i].size<best_size){
            best=i; best_size=mem_blocks[i].size;
        }
    }
    if(best==-1){ PRINT_ERR("Not enough memory (Best Fit)."); return -1; }
    int orig_size=mem_blocks[best].size;
    int orig_start=mem_blocks[best].start;
    mem_blocks[best].id=next_block_id++;
    mem_blocks[best].size=size;
    mem_blocks[best].status=MBLK_USED;
    strncpy(mem_blocks[best].owner,g_current_user[0]?g_current_user:"system",MAX_USERNAME-1);
    strncpy(mem_blocks[best].label,label,31);
    if(orig_size>size) insert_block(best,orig_start+size,orig_size-size,MBLK_FREE,"[Free]");
    return mem_blocks[best].id;
}

/* ─── Worst Fit ─── */
int mem_alloc_worst(int size, const char *label){
    int worst=-1, worst_size=-1;
    for(int i=0;i<mem_block_count;i++){
        if(mem_blocks[i].status==MBLK_FREE && mem_blocks[i].size>=size && mem_blocks[i].size>worst_size){
            worst=i; worst_size=mem_blocks[i].size;
        }
    }
    if(worst==-1){ PRINT_ERR("Not enough memory (Worst Fit)."); return -1; }
    int orig_size=mem_blocks[worst].size;
    int orig_start=mem_blocks[worst].start;
    mem_blocks[worst].id=next_block_id++;
    mem_blocks[worst].size=size;
    mem_blocks[worst].status=MBLK_USED;
    strncpy(mem_blocks[worst].owner,g_current_user[0]?g_current_user:"system",MAX_USERNAME-1);
    strncpy(mem_blocks[worst].label,label,31);
    if(orig_size>size) insert_block(worst,orig_start+size,orig_size-size,MBLK_FREE,"[Free]");
    return mem_blocks[worst].id;
}

/* ─── Free ─── */
int mem_free(int block_id){
    for(int i=0;i<mem_block_count;i++){
        if(mem_blocks[i].id==block_id && mem_blocks[i].status==MBLK_USED){
            mem_blocks[i].status=MBLK_FREE;
            strcpy(mem_blocks[i].label,"[Free]");
            /* Coalesce with adjacent free blocks */
            /* merge with next */
            if(i+1<mem_block_count && mem_blocks[i+1].status==MBLK_FREE){
                mem_blocks[i].size+=mem_blocks[i+1].size;
                for(int j=i+1;j<mem_block_count-1;j++) mem_blocks[j]=mem_blocks[j+1];
                mem_block_count--;
            }
            /* merge with prev */
            if(i>0 && mem_blocks[i-1].status==MBLK_FREE){
                mem_blocks[i-1].size+=mem_blocks[i].size;
                for(int j=i;j<mem_block_count-1;j++) mem_blocks[j]=mem_blocks[j+1];
                mem_block_count--;
            }
            return 1;
        }
    }
    PRINT_ERR("Block ID not found or already free."); return 0;
}

/* ─── Display ─── */
void mem_display(void){
    int used=0, free=0;
    printf("\n%s╔════════════════════════════════════════════════════════════╗%s\n",CLR_CYAN,CLR_RESET);
    printf("%s║                 MEMORY MAP  (Total: %4d KB)               ║%s\n",CLR_CYAN,MEM_TOTAL,CLR_RESET);
    printf("%s╚════════════════════════════════════════════════════════════╝%s\n\n",CLR_CYAN,CLR_RESET);
    printf("  %-6s %-8s %-8s %-10s %-16s %s\n","ID","Start","Size","Status","Owner","Label");
    printf("  %s\n","----------------------------------------------------------------");
    for(int i=0;i<mem_block_count;i++){
        MemBlock *b=&mem_blocks[i];
        const char *clr=(b->status==MBLK_USED)?CLR_BGREEN:CLR_YELLOW;
        printf("  %-6d %-8d %-8d %s%-10s%s %-16s %s\n",
               b->id,b->start,b->size,
               clr,(b->status==MBLK_USED)?"USED":"FREE",CLR_RESET,
               b->owner,b->label);
        if(b->status==MBLK_USED) used+=b->size;
        else free+=b->size;
    }
    printf("\n  Used: %d KB  |  Free: %d KB  |  Total: %d KB\n",used,free,MEM_TOTAL);

    /* Visual bar */
    int bar_width=50;
    printf("\n  [");
    int used_chars=(used*bar_width)/MEM_TOTAL;
    for(int i=0;i<bar_width;i++){
        if(i<used_chars) printf(CLR_BGREEN "#" CLR_RESET);
        else printf(CLR_YELLOW "." CLR_RESET);
    }
    printf("] %d%%\n\n",(used*100)/MEM_TOTAL);
}

/* ─── Shell commands ─── */
void cmd_memory(int argc, char **argv){ (void)argc;(void)argv; mem_display(); }

void cmd_alloc(int argc, char **argv){
    if(argc<3){ printf("Usage: alloc <size_kb> <label> [first|best|worst]\n"); return; }
    int sz=atoi(argv[1]);
    const char *algo=(argc>=4)?argv[3]:"best";
    int id=-1;
    if(STR_EQ(algo,"first")) id=mem_alloc_first(sz,argv[2]);
    else if(STR_EQ(algo,"worst")) id=mem_alloc_worst(sz,argv[2]);
    else id=mem_alloc_best(sz,argv[2]);
    if(id>0) printf(CLR_BGREEN "  Allocated %d KB with ID %d (%s fit)\n" CLR_RESET,sz,id,algo);
}

void cmd_free_mem(int argc, char **argv){
    if(argc<2){ printf("Usage: free <block_id>\n"); return; }
    int id=atoi(argv[1]);
    if(mem_free(id)) printf("  Block %d freed.\n",id);
}
