#include "security.h"
#include "logs.h"

/* ═══════════════════════════════════════════════
   SHA-256 implementation (public domain / FIPS)
   ═══════════════════════════════════════════════ */

static const uint32_t K[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,
    0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,
    0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,
    0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,
    0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,
    0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,
    0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,
    0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,
    0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

#define ROTR32(x,n) (((x)>>(n))|((x)<<(32-(n))))
#define S0(x) (ROTR32(x,2)^ROTR32(x,13)^ROTR32(x,22))
#define S1(x) (ROTR32(x,6)^ROTR32(x,11)^ROTR32(x,25))
#define G0(x) (ROTR32(x,7)^ROTR32(x,18)^((x)>>3))
#define G1(x) (ROTR32(x,17)^ROTR32(x,19)^((x)>>10))
#define CH(x,y,z) (((x)&(y))^(~(x)&(z)))
#define MAJ(x,y,z)(((x)&(y))^((x)&(z))^((y)&(z)))

static void sha256_transform(SHA256_CTX *ctx, const uint8_t *block) {
    uint32_t W[64], a,b,c,d,e,f,g,h,T1,T2;
    int i;
    for (i=0;i<16;i++)
        W[i]=((uint32_t)block[i*4]<<24)|((uint32_t)block[i*4+1]<<16)|
             ((uint32_t)block[i*4+2]<<8)|(uint32_t)block[i*4+3];
    for (i=16;i<64;i++)
        W[i]=G1(W[i-2])+W[i-7]+G0(W[i-15])+W[i-16];
    a=ctx->state[0]; b=ctx->state[1]; c=ctx->state[2]; d=ctx->state[3];
    e=ctx->state[4]; f=ctx->state[5]; g=ctx->state[6]; h=ctx->state[7];
    for (i=0;i<64;i++){
        T1=h+S1(e)+CH(e,f,g)+K[i]+W[i];
        T2=S0(a)+MAJ(a,b,c);
        h=g; g=f; f=e; e=d+T1;
        d=c; c=b; b=a; a=T1+T2;
    }
    ctx->state[0]+=a; ctx->state[1]+=b; ctx->state[2]+=c; ctx->state[3]+=d;
    ctx->state[4]+=e; ctx->state[5]+=f; ctx->state[6]+=g; ctx->state[7]+=h;
}

void sha256_init(SHA256_CTX *ctx){
    ctx->count=0;
    ctx->state[0]=0x6a09e667; ctx->state[1]=0xbb67ae85;
    ctx->state[2]=0x3c6ef372; ctx->state[3]=0xa54ff53a;
    ctx->state[4]=0x510e527f; ctx->state[5]=0x9b05688c;
    ctx->state[6]=0x1f83d9ab; ctx->state[7]=0x5be0cd19;
}

void sha256_update(SHA256_CTX *ctx, const uint8_t *data, size_t len){
    size_t i, part;
    uint64_t used = ctx->count & 63;
    ctx->count += len;
    if (used){
        part = 64-used;
        if (len < part){ memcpy(ctx->buf+used,data,len); return; }
        memcpy(ctx->buf+used,data,part);
        sha256_transform(ctx,ctx->buf);
        data+=part; len-=part;
    }
    for (i=0;i+64<=len;i+=64) sha256_transform(ctx,data+i);
    memcpy(ctx->buf,data+i,len-i);
}

void sha256_final(SHA256_CTX *ctx, uint8_t digest[32]){
    uint64_t bits = ctx->count*8;
    uint8_t pad=0x80;
    sha256_update(ctx,&pad,1);
    pad=0;
    while ((ctx->count&63)!=56) sha256_update(ctx,&pad,1);
    uint8_t len_bytes[8];
    int i;
    for (i=7;i>=0;i--){ len_bytes[i]=(uint8_t)(bits&0xff); bits>>=8; }
    sha256_update(ctx,len_bytes,8);
    for (i=0;i<8;i++){
        digest[i*4  ]=(ctx->state[i]>>24)&0xff;
        digest[i*4+1]=(ctx->state[i]>>16)&0xff;
        digest[i*4+2]=(ctx->state[i]>> 8)&0xff;
        digest[i*4+3]= ctx->state[i]     &0xff;
    }
}

void sha256_hex(const char *input, char out_hex[65]){
    SHA256_CTX ctx;
    uint8_t digest[32];
    sha256_init(&ctx);
    sha256_update(&ctx,(const uint8_t*)input,strlen(input));
    sha256_final(&ctx,digest);
    int i;
    for(i=0;i<32;i++) sprintf(out_hex+i*2,"%02x",digest[i]);
    out_hex[64]='\0';
}

/* ─── File checksum ─── */
void file_checksum(const char *path, char out_hex[65]){
    FILE *f = fopen(path,"rb");
    if (!f){ strcpy(out_hex,"0000000000000000000000000000000000000000000000000000000000000000"); return; }
    SHA256_CTX ctx; sha256_init(&ctx);
    uint8_t buf[1024]; size_t n;
    while ((n=fread(buf,1,sizeof(buf),f))>0) sha256_update(&ctx,buf,n);
    fclose(f);
    uint8_t digest[32]; sha256_final(&ctx,digest);
    int i; for(i=0;i<32;i++) sprintf(out_hex+i*2,"%02x",digest[i]);
    out_hex[64]='\0';
}

/* ─── Password strength ─── */
int password_strength(const char *pass){
    int score=0;
    int len=(int)strlen(pass);
    int has_upper=0,has_lower=0,has_digit=0,has_special=0;
    if(len>=8) score++;
    if(len>=12) score++;
    for(int i=0;i<len;i++){
        if(isupper((unsigned char)pass[i])) has_upper=1;
        if(islower((unsigned char)pass[i])) has_lower=1;
        if(isdigit((unsigned char)pass[i])) has_digit=1;
        if(strchr("!@#$%^&*()_+-=[]{}|;:',.<>?/`~",pass[i])) has_special=1;
    }
    score += has_upper+has_lower+has_digit+has_special;
    if(score>4) score=4;
    return score;
}

void password_strength_report(const char *pass){
    int s = password_strength(pass);
    const char *labels[] = {"Very Weak","Weak","Moderate","Strong","Very Strong"};
    const char *colors[] = {CLR_RED,CLR_RED,CLR_YELLOW,CLR_BGREEN,CLR_BGREEN};
    int len=(int)strlen(pass);
    printf("\n  %s=== Password Analysis ===%s\n",CLR_CYAN,CLR_RESET);
    printf("  Length    : %d characters\n",len);
    printf("  Strength  : %s%s%s\n",colors[s],labels[s],CLR_RESET);
    printf("  Score     : %d/4\n",s);
    printf("  Has Upper : %s\n", (strchr("ABCDEFGHIJKLMNOPQRSTUVWXYZ",pass[0])||strpbrk(pass,"ABCDEFGHIJKLMNOPQRSTUVWXYZ"))?"Yes":"No");
    printf("  Has Digit : %s\n", strpbrk(pass,"0123456789")?"Yes":"No");
    printf("  Has Symbol: %s\n", strpbrk(pass,"!@#$%^&*()_+-=[]{}|;':\",./<>?")?"Yes":"No");
    printf("\n");
}

/* ─── Shell commands ─── */
void cmd_security(int argc, char **argv){
    (void)argc; (void)argv;
    printf("\n%s╔══════════════════════════════════════╗%s\n",CLR_CYAN,CLR_RESET);
    printf("%s║      CORTEX SECURITY MODULE          ║%s\n",CLR_CYAN,CLR_RESET);
    printf("%s╚══════════════════════════════════════╝%s\n\n",CLR_CYAN,CLR_RESET);
    printf("  checkpass <password>  - Analyse password strength\n");
    printf("  verify <file>         - Verify file integrity (SHA-256)\n\n");
}

void cmd_checkpass(int argc, char **argv){
    if(argc<2){ printf("Usage: checkpass <password>\n"); return; }
    password_strength_report(argv[1]);
    char hash[65]; sha256_hex(argv[1],hash);
    printf("  SHA-256: %s\n\n",hash);
}

void cmd_verify(int argc, char **argv){
    if(argc<2){ printf("Usage: verify <file>\n"); return; }
    char hash[65]; file_checksum(argv[1],hash);
    printf("\n  File      : %s\n  SHA-256   : %s\n\n",argv[1],hash);
}
