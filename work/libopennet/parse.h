#ifndef _PARSE_C
#define _PARSE_C
#include "conf.h"

uint32_t atoi2(const char *n);
int parse_url(const char *url, char *scheme, char *username, char *password, char *host, int *port, char *file);
void strtolower(char *str);
char *parse_url_subst(const char *src);
char *parse_url_subst_dist(void);
char *mime64(char *str);
uint32_t hash_fourbyte(unsigned char *str, const char term);
#endif
