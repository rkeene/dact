#ifndef _OPENNET_CFG_H
#define _OPNNEET_CFG_H
#define VERSION "0.8.1"


#include "config.h"

#ifdef HAVE_STRSEP
#include <string.h>
#else
#include "strsep.h"
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
#else
#include "stdint.h"
#endif


#endif
