/*
 * Copyright (C) 2001, 2002, and 2003  Roy Keene
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *      email: dact@rkeene.org
 */

#ifndef _DACT_H
#define _DACT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "win32.h"
#include "comp_fail.h"

#ifndef DACT_CONTACT
#define DACT_CONTACT "<dact@rkeene.org>"
#endif

#ifndef DACT_KEY_SIZE
#define DACT_KEY_SIZE 2048
#endif

#define DACT_VER_MAJOR 0
#define DACT_VER_MINOR 8
#define DACT_VER_REVISION 38
#define DACT_VERSION "0.8.38"
#ifdef DEBUG
#define DACT_VER_SUB	"dev"
#else
#define DACT_VER_SUB	"rel"
#endif

#define DACT_MODE_COMPR 1
#define DACT_MODE_DECMP 2
#define DACT_MODE_STAT  3
#define DACT_MODE_RET   4
#define DACT_MODE_CINIT	5
#define DACT_MODE_CENC	6
#define DACT_MODE_CDEC	7
#define DACT_MODE_CIENC 11   /* CINIT+CENC */
#define DACT_MODE_DIDEC 12   /* CINIT+CDEC */

#define DACT_OPT_FORCE	0
#define DACT_OPT_STDOUT	1
#define DACT_OPT_VERB	2
#define DACT_OPT_COMPLN	3
#define DACT_OPT_VERCHK	4
#define DACT_OPT_BINCHK	5
#define DACT_OPT_TIME	6
#define DACT_OPT_PERM	7
#define DACT_OPT_ORIG	8
#define DACT_OPT_HDONLY	9
#define DACT_OPT_SZSPD	10
#define DACT_OPT_NOCRC	11
#define DACT_OPT_NETWORK	12
#define DACT_OPT_URL	13
#define DACT_OPT_UPGRADE	14

/* These should only be changed at the risk of breaking files. 
 * (in other words, they should not be changed if possible.
 *  Adding on to them is fine.)
 */
#define DACT_HDR_CRC0	0
#define DACT_HDR_CRC1	1
#define DACT_HDR_TIME	2
#define DACT_HDR_PERM	3
#define DACT_HDR_NAME	4
#define DACT_HDR_MD5SUM	5
#define DACT_HDR_DESC	6
#define DACT_HDR_URL	7
#define DACT_HDR_URLFILE	8
#define DACT_HDR_CIPHER	9
#define DACT_HDR_NOP	10
#define DACT_HDR_IDXDATA	11

#ifndef DACT_BLK_SIZE_MAX
#define DACT_BLK_SIZE_MAX 2147483647
#endif

#ifndef DACT_BLK_SIZE_DEF
#define DACT_BLK_SIZE_DEF 8192
#endif

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else
#include "getopt.h"
#endif

#ifndef HAVE_STRSEP
#include "strsep.h"
#endif
#ifndef HAVE_GETPASS
#include "getpass.h"
#endif
#ifndef HAVE_MKSTEMP
#include "mkstemp.h"
#endif
#ifndef HAVE_UNAME
#include "uname.h"
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif


#define DACT_MAGIC_NUMBER 0x444354C3
#define DACT_MAGIC_OFFSET 0
#define DACT_MAGIC_SIZE   4
#define DACT_MAGIC_PEOF   0xFF3FDE08

#define DACT_HDR_REG_SIZE 24

extern char dact_nonetwork;
struct dact_header {
	uint32_t size;
	unsigned char algo;
};

#if defined(__FILE__) && defined(__LINE__) && defined(DEBUG)
#define PRINT_LINE fprintf(stderr, "%s:%07i:%s(): ", __FILE__, __LINE__, __func__)
#else
#define PRINT_LINE /**/
#endif

#define PERROR_L(errnum, error) { PRINT_LINE; fprintf(stderr, "dact: %s: %s\n", error, strerror(abs(errnum))); }
#define PRINTERR(error) { PRINT_LINE; fprintf(stderr, "dact: " error "\n"); }
#define PERROR(error) PERROR_L(errno, error)
/* #define PERROR(error) { PRINT_LINE; perror("dact: " error); } */

#ifdef DEBUG
#define SHOWVAL(x...) { PRINT_LINE; fprintf(stderr, "dact: %s=%li\n", #x, (long) (x)); }
#define SPOTVAR_NUM(x) { PRINT_LINE; fprintf(stderr, "dact: %s=%li\n", #x, (long) x); }
#define SPOTVAR_STR(x) { PRINT_LINE; fprintf(stderr, "dact: %s=\"%s\"\n", #x, x); }
#define CHECKPOINT PRINTERR("Checkpoint reached.")
#define DPRINTF(x...) { PRINT_LINE; fprintf(stderr, x); fprintf(stderr, "\n"); }
#else
#define SHOWVAL(x...) /**/
#define SPOTVAR_NUM(x) /**/
#define SPOTVAR_STR(x) /**/
#define CHECKPOINT /**/
#define DPRINTF(x...) /**/
#endif

#ifndef DACT_FAILED_ALGO
#define DACT_FAILED_ALGO comp_fail_algo
#endif

#ifdef HAVE_DEV_URANDOM
#define RANDOM_DEV "/dev/urandom"
#endif

#ifndef DACT_VER_PARTS
#define DACT_VER_PARTS(x) ((x&0xff0000)>>16),((x&0xff00)>>8),(x&0xff)
#endif

#ifndef DACT_BIN_URL
#define DACT_BIN_URL "http://www.rkeene.org/projects/compression/dact/@@OSNM@@-@@ARCH@@/dact.bin"
#endif
#ifndef DACT_BIN_URL_VER
#define DACT_BIN_URL_VER "http://www.rkeene.org/projects/compression/dact/@@OSNM@@-@@ARCH@@/dact.ver"
#endif
#ifndef DACT_BIN_DIR
#define DACT_BIN_DIR "@@HOME@@/.dact/@@OSNM@@-@@ARCH@@/"
#endif
#ifndef DACT_BIN
#ifdef EXEEXT
#define DACT_BIN DACT_BIN_DIR "dact" EXEEXT
#else
#define DACT_BIN DACT_BIN_DIR "dact.bin"
#endif
#endif
#define DACT_BIN_VER ((DACT_VER_MAJOR<<16)|(DACT_VER_MINOR<<8)|DACT_VER_REVISION)

/* We need to determine when we have network support */
#if !defined(HAVE_SOCKET) || !defined(HAVE_GETHOSTBYNAME) || (!defined(HAVE_INET_ATON) && !defined(HAVE_INET_ADDR))
#ifndef NO_NETWORK
#define NO_NETWORK 1
#endif
#endif


int print_help(int argc, char **argv);
int dact_blksize_calc(int fsize);
int dact_upgrade_file_checkver(const char *name, const char *url_ver, const char *options);
int dact_upgrade_file(const char *name, const char *url_get, const char *url_ver, uint32_t version, const char *dest, const char *options);
int dact_shutdown(int retval);
char *dact_getoutfilename(const char *orig, const int mode);
uint32_t dact_process_other(int src, const int dest, const uint32_t magic, const char *options);
int main(int argc, char **argv);

#endif/*_DACT_H*/
