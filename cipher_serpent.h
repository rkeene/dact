#ifndef _CIPHER_SERPENT_H
#define _CIPHER_SERPENT_H
#include "dact.h"
#include "comp_fail.h"

#if defined(USE_MODULES) && !defined(AS_MODULE)
#define cipher_serpent DACT_FAILED_ALGO
#else

#if defined(HAVE_LIBMCRYPT) && defined(HAVE_MCRYPT_H)
#define HAVE_MCRYPT 1
#include <mcrypt.h>
int cipher_serpent_init(const int mode, char *key, unsigned char *IV);
int cipher_serpent_encrypt(const char *inblk, char *outblk, int blksize, char *key, unsigned char *IV);
int cipher_serpent_decrypt(const char *inblk, char *outblk, int blksize, char *key, unsigned char *IV);
#endif

int cipher_serpent(const char *inblock, char *outblock, const int blksize, char *key, const int mode);
#endif
#endif
