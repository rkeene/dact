/*
	Serpent Cipher from libmcrypt
*/
#include "dact.h"
#include "cipher_serpent.h"
#ifdef HAVE_MCRYPT
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <fcntl.h>
#include <stdio.h>
#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif
#include "parse.h"
#include "ui.h"

#if defined(USE_MODULES) && defined(AS_MODULE)
#include "module.h"
uint32_t DC_NUM=2;
uint32_t DC_TYPE=DACT_MOD_TYPE_ENC;
void *DC_ALGO=cipher_serpent;
char *DC_NAME="serpent (MOD)";
#endif

MCRYPT mcrypt_tdid;

int cipher_serpent(const char *inblock, char *outblock, const int blksize, char *key, const int mode) {
	static unsigned char *IV;

	switch (mode) {
		case (DACT_MODE_CINIT+DACT_MODE_CDEC):
		case (DACT_MODE_CINIT+DACT_MODE_CENC):
		case DACT_MODE_CINIT:
			return(cipher_serpent_init(mode, key, IV));
			break;
		case DACT_MODE_CDEC:
			return(cipher_serpent_decrypt(inblock, outblock, blksize, key, IV));
			break;
		case DACT_MODE_CENC:
			return(cipher_serpent_encrypt(inblock, outblock, blksize, key, IV));
			break;
	}
	return(0);
}


int cipher_serpent_init(const int mode, char *key, unsigned char *IV) {
	char *password;
	int plen, i;
	unsigned char buff;
	char *mime_buf;
#ifdef RANDOM_DEV
	int fd;
	fd=open(RANDOM_DEV, O_RDONLY);
	if (fd<0) {
#endif
		srand(time(NULL)+rand());
#ifdef RANDOM_DEV
	}
#endif

	mcrypt_tdid=mcrypt_module_open("serpent", NULL, "cfb", NULL);
	if (mcrypt_tdid==MCRYPT_FAILED) {
#ifdef RANDOM_DEV
		if (fd>=0) close(fd);
#endif
		return(-1);
	}

	password=dact_ui_getuserinput("Enter your passphrase: ", 128, 1);
	memset(key, 1, 16);
	plen=strlen(password);
	if (plen<16) plen=16;
	memcpy(key, password, plen);

	if ((mode-DACT_MODE_CINIT)==DACT_MODE_CENC) {
/*	mhash_keygen(KEYGEN_MCRYPT, MHASH_MD5, key, 16, NULL, 0, password, strlen(password)); */
		plen=mcrypt_enc_get_iv_size(mcrypt_tdid);
		IV=malloc(plen);

		for (i=0; i<plen; i++) {
#ifdef RANDOM_DEV
			if (fd>=0) {
				read(fd, &buff, 1);
			} else {
#endif
				srand(time(NULL)+rand());
				buff=(int) ((256.0*rand())/(RAND_MAX+1.0));
#ifdef RANDOM_DEV
			}
#endif
			IV[i]=buff;
		}
		mime_buf=mimes64(IV, &plen);
		fprintf(stderr, "Magic [needed for decryption]:  %s\n", mime_buf);
		free(mime_buf);
	} else {
		plen=mcrypt_enc_get_iv_size(mcrypt_tdid);
		IV=dact_ui_getuserinput("Enter your magic key: ", plen*3, 0);
		mime_buf=demime64(IV);
		memcpy(IV, mime_buf, plen);
		free(mime_buf);
	}


#ifdef RANDOM_DEV
	if (fd>=0) close(fd);
#endif

	i=mcrypt_generic_init(mcrypt_tdid, key, 16, IV);
	if (i<0) {
		mcrypt_perror(i);
		return(-1);
	}

	return(16);
}

int cipher_serpent_encrypt(const char *inblk, char *outblk, int blksize, char *key, unsigned char *IV) {
	int i;

	memcpy(outblk, inblk, blksize);
	for (i=0; i<blksize; i++) {
		mcrypt_generic(mcrypt_tdid, &outblk[i], 1);
	}
	return(blksize);
}

int cipher_serpent_decrypt(const char *inblk, char *outblk, int blksize, char *key, unsigned char *IV) {
	int i;

	memcpy(outblk, inblk, blksize);
	for (i=0; i<blksize; i++) {
		mdecrypt_generic(mcrypt_tdid, outblk+i, 1);
	}
	return(blksize);
}
#else
int cipher_serpent(const char *inblock, char *outblock, const int blksize, char *key, const int mode) {
	return(0);
}
#endif
