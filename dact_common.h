#ifndef _DACT_COMMON_H
#define _DACT_COMMON_H
#include "dact.h"

int dact_config_execute(const char *cmd, char *options, uint32_t *blksize);
void dact_config_loadfile(const char *path, char *options, uint32_t *blksize);
uint32_t dact_blk_decompress(char *ret, const char *srcbuf, const uint32_t size, const char *options, const int algo, uint32_t bufsize);
uint32_t dact_blk_compress(char *algo, char *ret, const char *srcbuf, const uint32_t size, const char *options, uint32_t bufsize);
uint32_t dact_process_file(const int src, const int dest, const int mode, const char *options, const char *filename, uint32_t *crcs, uint32_t dact_blksize, int cipher);
#endif
