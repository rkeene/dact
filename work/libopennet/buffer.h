#ifndef _BUFFER_H
#define _BUFFER_H
#include "conf.h"

int bit_buffer_read(int bits);
void bit_buffer_write(unsigned int val, unsigned int bits);
int bit_buffer_size(void);
void bit_buffer_purge(void);

char *byte_buffer_read(int bytes);
char byte_buffer_read_1(void);
void byte_buffer_write(char *val, unsigned int size);
int byte_buffer_size(void);
void byte_buffer_purge(void);

#ifndef BYTE_BUFF_SIZE
#define BYTE_BUFF_SIZE 65535
#endif
#endif
