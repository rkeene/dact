#ifndef DACT_LIBDACT_H
#define DACT_LIBDACT_H

#include "dact.h"
#include "dact_common.h"

int dact_init(void);
void *dact_openfile(const char *pathname);
int dact_BuffToBuffDecompress(void);
int dact_BuffToBuffCompress(void);

#endif
