#ifndef SECURITY_H
#define SECURITY_H

#include "common.h"

/* ── SHA-256 context ── */
typedef struct {
    uint32_t state[8];
    uint64_t count;
    uint8_t  buf[64];
} SHA256_CTX;

void sha256_init(SHA256_CTX *ctx);
void sha256_update(SHA256_CTX *ctx, const uint8_t *data, size_t len);
void sha256_final(SHA256_CTX *ctx, uint8_t digest[32]);
void sha256_hex(const char *input, char out_hex[65]);

/* ── Password ── */
int  password_strength(const char *pass);   /* 0-4 */
void password_strength_report(const char *pass);

/* ── File integrity ── */
void file_checksum(const char *path, char out_hex[65]);

/* ── Shell commands ── */
void cmd_security(int argc, char **argv);
void cmd_checkpass(int argc, char **argv);
void cmd_verify(int argc, char **argv);

#endif
