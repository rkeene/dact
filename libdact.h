#ifndef LIBDACT_H
#define LIBDACT_H

int dact_init(void);
void *dact_openfile(const char *pathname);
int dact_BuffToBuffDecompress(void);
int dact_BuffToBuffCompress(void);

#endif
