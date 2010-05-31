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

#include "dact.h"
#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include "parse.h"
#include "dendian.h"
#include "crc.h"
#include "math.h"
#include "dact_common.h"
#include "algorithms.h"
#include "libdact.h"
#include "ciphers.h"
#include "module.h"
#include "header.h"
#include "parse.h"
#include "net.h"
#include "ui.h"
#ifdef HAVE_ZLIB_H
#include <zlib.h>
#endif
#ifdef HAVE_BZLIB_H
#include <bzlib.h>
#endif


int print_help(int argc, char **argv);
int dact_shutdown(int retval);
int main(int argc, char **argv);

extern char *optarg;
extern int optind, opterr, optopt;

int print_help(int argc, char **argv) {
	printf("DACT %i.%i.%i-%s by Keene Enterprises <dact@rkeene.org>\n", DACT_VER_MAJOR, DACT_VER_MINOR, DACT_VER_REVISION, DACT_VER_SUB);
	printf("usage: %s [options...] [file ...]\n",argv[0]);
	printf("Options:\n");
	printf("  -d          Decompress instead of compressing.\n");
	printf("  -s          Give statistics rather than compress or decompress.\n");
	printf("  -f          Force unsafe things to happen.\n");
	printf("  -c          (De)compress to stdout.\n");
	printf("  -v          Increase verbosity.\n");
	printf("  -l          List available algorithms.\n");
	printf("  -n          Toggle use of CRCs.\n");
	printf("  -i          Use stdin to read information from instead of /dev/tty.\n");
#ifndef DACT_UNSAFE
	printf("  -C          Complain when compression errors occur.\n");
#endif
	printf("  -H          Write only header (no data).\n");
	printf("  -O          Toggle writing original file name.\n");
	printf("  -S          Use speed-size as a metric rather than size.\n");
	printf("  -h          Give this help.\n");
	printf("  -V          Display DACT version (%i.%i.%i-%s).\n", DACT_VER_MAJOR, DACT_VER_MINOR, DACT_VER_REVISION, DACT_VER_SUB);
	printf("  -N          Upgrade DACT.\n");
	printf("  -a          Upgrade DACT modules.\n");
	printf("  -x          Create self-extracting DACT file.\n");
	printf("  -b NN       Use a block size of NN bytes for compression.\n");
	printf("  -e NN       Exclude algorithm NN from being used.\n");
	printf("  -m CONF     Load config file CONF.\n");
	printf("  -o FILE     Send output to FILE.\n");
	printf("  -u URL      Specify download location as URL.\n");
	printf("  -p URL      Parse URL and print results, then exit.\n");
	printf("  -M COMMAND  Execute COMMAND as if it had appeared in a config file.\n");
	printf("  -D DESC     Specify a description of DESC.\n");
	printf("  -I NN       Use ONLY 2 algorithms, NN and 0.\n");
	printf("  -U FILE     Use FILE to select download location.\n");
	printf("  -E CIPHER   Use CIPHER to encrypt data (LIST  lists available ciphers.)\n");
	printf("  file...     File(s) to (de)compress.  (If none given, use standard input).\n");
	return(0);
}

int dact_upgrade_file(const char *name, const char *url_get, const char *url_ver, uint32_t version, const char *dest, const unsigned char *options) {
	int newver, ifd=-1, ofd=-1, x=-1;
	char *real_dest, *real_url_get, buf[4096];

	if (dest==NULL) {
		real_dest=parse_url_subst(DACT_BIN_DIR "@@FILE@@.so", name);
	} else {
		real_dest=parse_url_subst(dest, name);
	}
	real_url_get=parse_url_subst(url_get, name);

	newver=dact_upgrade_file_checkver(name, url_ver, options);
	if (newver>version) {
		if (options[DACT_OPT_UPGRADE]) {
			fprintf(stderr, "There is a new version of %s.  NEW: %i.%i.%i, CURR: %i.%i.%i, fetching...\n", name, DACT_VER_PARTS(newver), DACT_VER_PARTS(version));
			if ((ifd=open_net(real_url_get, O_RDONLY, 0))>=0) {
				if ((ofd=open_net(real_dest, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU))>=0) {
					while ((x=read(ifd, buf, sizeof(buf)))>0) {
						write(ofd, buf, x);
					}
					close(ifd);
					close(ofd);
					if (x<0) PERROR("read");
				} else {
					PERROR_L(ofd, "open");
				}
			} else {
				PERROR_L(ifd, "open");
			}
		} else {
			fprintf(stderr, "There is a new version of %s.  NEW: %i.%i.%i, CURR: %i.%i.%i\n", name, DACT_VER_PARTS(newver), DACT_VER_PARTS(version));
		}
	}
	free(real_dest);
	free(real_url_get);
	return(x);
}

int dact_upgrade_file_checkver(const char *name, const char *url_ver, const unsigned char *options) {
#ifdef CHECK_VERSION
	uint32_t rem_ver[4]={0, 0, 0, 0};
	int fd;
	char rem_vers[3][4]={{0,0,0,0},{0,0,0,0},{0,0,0,0}};
	char *urlbuf, verbuf[9];

	if (options[DACT_OPT_VERCHK]==0 && options[DACT_OPT_UPGRADE]==0) return(0);

	urlbuf=parse_url_subst(url_ver, name);
	if ((fd=open_net(urlbuf, O_RDONLY, 0))>=0) {
		read(fd, &verbuf, 9);
		close(fd);
		memcpy(rem_vers[0], verbuf, 3);
		memcpy(rem_vers[1], verbuf+3, 3);
		memcpy(rem_vers[2], verbuf+6, 3);
		rem_ver[0]=atoi(rem_vers[0]);
		rem_ver[1]=atoi(rem_vers[1]);
		rem_ver[2]=atoi(rem_vers[2]);
		rem_ver[3]=(rem_ver[0]<<16)|(rem_ver[1]<<8)|(rem_ver[2]);
	}
	free(urlbuf);
	return(rem_ver[3]);
#else
	return(0);
#endif
}

int dact_upgrade(unsigned char *options) {
#ifdef DACT_DEBIAN_UPGRADE_PROC
	char *buf;
	int status=0, i;

	if (getuid()==0) {
		buf=dact_ui_getuserinput("Executing `apt-get update' okay [y/N]? ", 5, 0);
		if (toupper(buf[0])!='Y') {
			PRINTERR("Failed to upgrade DACT.");
			free(buf);
			return(-1);
		}
		free(buf);

		if (fork()==0) {
			execl("/usr/bin/apt-get","apt-get","update",NULL);
			return(-1);  /* Couldn't run binary. */
		} else {
			i=wait(&status);
			if (WIFEXITED(status)) {
				if (WEXITSTATUS(status)) {
					PRINTERR("Failed to run `apt-get update'");
					return(-1);
				}
			}
		}

		if (fork()==0) {
			execl("/usr/bin/apt-get","apt-get","install","dact",NULL);
			return(-1);  /* Couldn't run binary. */
		} else {
			i=wait(&status);
			if (WIFEXITED(status)) {
				if (WEXITSTATUS(status)) {
					PRINTERR("Failed to run `apt-get install dact'");
					return(-1);
				}
			}
		}

		return(1);
	}
#else
	options[DACT_OPT_UPGRADE]=1;
	return(dact_upgrade_file("dact", DACT_BIN_URL, DACT_BIN_URL_VER, DACT_BIN_VER, DACT_BIN, options));
#endif
}

#if 0
int dact_upgrade(const char *options, uint32_t *crcs) {
	char *urlsubst, *buf, *dact_binfilebuf;
	char dact_binfile[256];
	int inFd, outFd;
	uint32_t i;


#ifdef DACT_DEBIAN_UPGRADE_PROC
	int status=0;

	if (getuid()==0) {
		buf=dact_ui_getuserinput("Executing `apt-get update' okay [y/N]? ", 5, 0);
		if (toupper(buf[0])!='Y') {
			PRINTERR("Failed to upgrade DACT.");
			free(buf);
			return(-1);
		}
		free(buf);

		if (fork()==0) {
			execl("/usr/bin/apt-get","apt-get","update",NULL);
			return(-1);  /* Couldn't run binary. */
		} else {
			i=wait(&status);
			if (WIFEXITED(status)) {
				if (WEXITSTATUS(status)) {
					PRINTERR("Failed to run `apt-get update'");
					return(-1);
				}
			}
		}

		if (fork()==0) {
			execl("/usr/bin/apt-get","apt-get","install","dact",NULL);
			return(-1);  /* Couldn't run binary. */
		} else {
			i=wait(&status);
			if (WIFEXITED(status)) {
				if (WEXITSTATUS(status)) {
					PRINTERR("Failed to run `apt-get install dact'");
					return(-1);
				}
			}
		}

		return(1);
	}
#endif

	if ((i=is_latest(options))) {
		PRINTERR("**+");
		PRINT_LINE; fprintf(stderr, "dact: **> CURR: DACT %i.%i.%i\n",DACT_VER_MAJOR, DACT_VER_MINOR, DACT_VER_REVISION);
		PRINT_LINE; fprintf(stderr, "dact: **> NEW:  DACT %i.%i.%i\n",i>>16,(i>>8)&0xff,i&0xff);
		PRINTERR("**>");
		PRINTERR("**-");
	}
	if ((inFd=open_net("http://www.rkeene.org/projects/rget/rget.cgi?project=dact&file=info", O_RDONLY))>=0) {
		fprintf(stderr, "------------------------\n");
		buf=malloc(1024);
		while (1) {
			i=read_f(inFd, buf, 1024);
			write(STDERR_FILENO, buf, i);
			if (i!=1024) break;
		}
		fprintf(stderr, "------------------------\n");
		close(inFd);
		free(buf);
	}

#ifdef GO32
	mkdir("c:/dact/", 0755);
	strcpy(dact_binfile,"c:/dact/dact.exe");
#else
	strncpy(dact_binfile,getenv("HOME"),sizeof(dact_binfile)-1);
	strncat(dact_binfile,"/.dact/",sizeof(dact_binfile)-strlen(dact_binfile)-1);
	mkdir(dact_binfile, 0755);
	dact_binfilebuf=parse_url_subst("@@OSNM@@-@@ARCH@@/", "");
	strncat(dact_binfile,dact_binfilebuf,sizeof(dact_binfile)-strlen(dact_binfile)-1);
	free(dact_binfilebuf);
	mkdir(dact_binfile, 0755);
	
	strncat(dact_binfile,"dact.bin",sizeof(dact_binfile)-strlen(dact_binfile)-1);
#endif
	urlsubst=parse_url_subst("http://www.rkeene.org/projects/rget/rget.cgi?os=@@OSNM@@&arch=@@ARCH@@&project=dact&file=bin&meth=gz","");
	if ((outFd=open_net(dact_binfile, O_WRONLY|O_TRUNC|O_CREAT, 0755))<0) {
		PERROR_L(outFd, "open_net");
		return(-1);
	}
	if ((inFd=open_net(urlsubst, O_RDONLY, 0))<0) {
		PERROR_L(inFd, "open_net");
		return(-1);
	}
	if (!dact_process_file(inFd, outFd, DACT_MODE_DECMP, options, "dact", crcs, DACT_BLK_SIZE_DEF, -1)) {
		close(inFd);
		close(outFd);
		unlink(dact_binfile);
		PRINTERR("Failed to upgrade DACT.");
		return(-1);
	}
	close(inFd);
	close(outFd);
	if (!options[DACT_OPT_BINCHK]) {
		PRINTERR("Note: You do not have binary_check  set to `on' in your dact.conf.");
	}
	return(1);
}

uint32_t is_latest (const char *options) {
#if defined(CHECK_VERSION)
	int fd;
	char ver_maj[4]={0,0,0,0}, ver_min[4]={0,0,0,0}, ver_rev[4]={0,0,0,0};
	char bigbuf[1024];
	int vers[3];

	if (options[DACT_OPT_VERCHK]==0) return(0);
	if (getuid()==0) return(0);

	if ((fd=createconnection_tcp("www.rkeene.org", 80))<0) return(0);

	write(fd, "GET http://www.rkeene.org/devel/dact/VERSION\n", 45);
	read(fd, &bigbuf,1024);

	memcpy(ver_maj,bigbuf,3);
	memcpy(ver_min,bigbuf+3,3);
	memcpy(ver_rev,bigbuf+6,3);

	closeconnection(fd);

	vers[0]=atoi(ver_maj);
	vers[1]=atoi(ver_min);
	vers[2]=atoi(ver_rev);

	if ( ((vers[0]<<16)|(vers[1]<<8)|vers[2]) > ((DACT_VER_MAJOR<<16)|(DACT_VER_MINOR<<8)|DACT_VER_REVISION) ) {
		return((vers[0]<<16)|(vers[1]<<8)|vers[2]);
	} else {
		return(0);
	}
#else
	return(0);
#endif
}
#endif

int dact_shutdown(int retval) {
	unload_modules();
	dact_ui_deinit();
	return(retval);
}

char *dact_getoutfilename(const char *orig, const int mode, const char *ext) {
	char *ret=NULL;
	size_t retlen;

	switch (mode) {
		case DACT_MODE_COMPR:
			retlen=strlen(orig)+strlen(ext)+10;
			ret=malloc(retlen);
			snprintf(ret, retlen, "%s%s", orig, ext);
			break;
		case DACT_MODE_DECMP:
			if (strcmp(&orig[strlen(orig)-4],".dct") && \
				strcmp(&orig[strlen(orig)-4], ".exe") && \
				strcmp(&orig[strlen(orig)-4], ".bin") && \
				strcmp(&orig[strlen(orig)-4], ".bz2") && \
				strcmp(&orig[strlen(orig)-5], ".tbz2") && \
				strcmp(&orig[strlen(orig)-4], ".tgz") && \
				strcmp(&orig[strlen(orig)-3], ".gz")) {
				return(NULL);
			}
/* XXX: I wonder if this breaks easily... */
			ret=strdup(orig);
			(*strrchr(ret, '.'))='\0';
			break;
		case DACT_MODE_STAT:
			return(NULL);
			break;
	}
	return(ret);
}

int main(int argc, char **argv) {
	unsigned char options[20]={0};
	signed char opt;
	struct stat stat_buf;
	char **in_files, *in_file=NULL, *out_file=NULL;
	char dact_binfilebuf[256], *dact_binfile;
	char *ext=".dct";
	int filecnt=0;
	int in_fd, out_fd;
	int mode=DACT_MODE_COMPR, ciphernum=-1;
	uint32_t dact_blk_size=0;
	uint32_t crcs[6]={0};
	uint32_t i,x;

	dact_ui_init();
	dact_init();

/* hack, to make upgrade work even if DACT_OPT_BINCHK is enabled, we must
 *      get the new version before executing it.
 */
	if (argv[1]!=NULL) {
		if (!strcmp(argv[1],"-N")) return(dact_upgrade(options));
		if (!strcmp(argv[1],"-a")) options[DACT_OPT_UPGRADE]=1;
	}

	dact_config_loadfile(DACT_CONFIG_FILE, options, &dact_blk_size);
#ifndef NO_BINCHECK
	dact_binfilebuf[0]='\0';
	if (getenv("HOME")) {
		strncpy(dact_binfilebuf,getenv("HOME"),sizeof(dact_binfilebuf)-1);
	}
	strncat(dact_binfilebuf,"/.dact/dact.conf",sizeof(dact_binfilebuf)-strlen(dact_binfilebuf)-1);
	dact_config_loadfile(dact_binfilebuf, options, &dact_blk_size);
#endif

	if (options[DACT_OPT_BINCHK]) {
		dact_binfile=parse_url_subst(DACT_BIN, "");
		if (strcmp(argv[0],dact_binfile)) {
			if (!access(dact_binfile,X_OK)) {
				argv[0]=dact_binfile;
				/* This fixes a strange warning..*/
#ifndef __MINGW32__
				execv(dact_binfile, argv);
#else
				execv(dact_binfile, (const char **) argv);
#endif
			}
		}
	}

	while ((opt=getopt(argc,argv,"adfsvcnhixNVHCM:E:p:I:m:e:lb:u:U:TPOD:o:"))!=-1) {
		switch (opt) {
			case 'a':
				options[DACT_OPT_UPGRADE]=1;
				break;
			case 'd':
				mode=DACT_MODE_DECMP;
				break;
			case 'f':
				options[DACT_OPT_FORCE]++;
				break;
			case 's':
				mode=DACT_MODE_STAT;
				break;
			case 'i':
				dact_ui_setopt(DACT_UI_OPT_PASSSTDIN, 1);
				break;
			case 'x':
				if (strcmp(EXEEXT, "")==0) {
					ext=".bin";
				} else {
					ext=EXEEXT;
				}
				options[DACT_OPT_SFX]=!options[DACT_OPT_SFX];
				break;
			case 'c':
				options[DACT_OPT_STDOUT]=!options[DACT_OPT_STDOUT];
				break;
			case 'b':
				i=atoi2(optarg);
				if (i<DACT_BLK_SIZE_ABSMAX) dact_blk_size=i;
				break;
			case 'v':
				options[DACT_OPT_VERB]++;
				dact_ui_setopt(DACT_UI_OPT_LEVEL,dact_ui_getopt(DACT_UI_OPT_LEVEL)+1);
				break;
			case 'n':
				options[DACT_OPT_NOCRC]=!options[DACT_OPT_NOCRC];
				break;
			case 'p':
				PRINT_LINE; fprintf(stderr, "dact: %s\n",parse_url_subst(optarg,"@@file@@"));
				mode=DACT_MODE_RET;
				break;
			case 'C':
				options[DACT_OPT_COMPLN]++;
				break;
			case 'm':
				dact_config_loadfile(optarg, options, &dact_blk_size);
				break;
			case 'e':
				i=(atoi(optarg)&0xff);
				algorithms[i]=DACT_FAILED_ALGO;
				break;
			case 'H':
				options[DACT_OPT_HDONLY]=!options[DACT_OPT_HDONLY];
				break;
			case 'o':
				out_file=parse_url_subst(optarg, "");
				break;
			case 'M':
				dact_config_execute(optarg, options, &dact_blk_size);
				break;
			case 'N':
				PRINTERR("The `-N\' option must be the first and only argument passed to dact.");
				return(0);
				break;
			case 'E':
				strtolower(optarg);
				x=hash_fourbyte((unsigned char *) optarg, ' ');
				if (x==hash_fourbyte((unsigned char *) "list", ' ')) {
					PRINT_LINE; fprintf(stderr, "dact: Num | Name\n");
					for (i=0;i<CIPHER_COUNT;i++) {
						if (ciphers_name[i]!=NULL && ciphers[i]!=DACT_FAILED_ALGO) {
							PRINT_LINE; fprintf(stderr, "dact: %3i | %s\n",i,ciphers_name[i]);
						}
					}
					return(0);
				}
				for (i=0;i<CIPHER_COUNT;i++) {
					if (ciphers_name[i]!=NULL && ciphers[i]!=DACT_FAILED_ALGO) {
						if (x==hash_fourbyte((unsigned char *) ciphers_name[i], ' ')) {
							break;
						}
					}
				}
				if (i==CIPHER_COUNT) i=-1;
				ciphernum=i;
				if (ciphernum==-1) {
					PRINTERR("No such cipher.");
					return(-1);
				}
				break;
			case 'l':
				PRINT_LINE; fprintf(stderr, "dact: Num | Name\n");
				for (i=0;i<255;i++) {
					if (algorithms[i]==NULL || algorithms[i]==DACT_FAILED_ALGO) continue;
					PRINT_LINE; fprintf(stderr, "dact: %3i | %s\n",i,algorithm_names[i]);
			
				}
				mode=DACT_MODE_RET;
				break;
			case 'I':
				x=atoi(optarg);
				for (i=1;i<255;i++) {
					if (i!=x) algorithms[i]=NULL;
				}
				break;
			case 'V':
				printf("DACT %i.%i.%i-%s", DACT_VER_MAJOR, DACT_VER_MINOR, DACT_VER_REVISION, DACT_VER_SUB);
#if defined(__DATE__) && defined(__TIME__)
				printf("  built on %s at %s",__DATE__,__TIME__);
#endif
#ifdef DACT_CONTACT
				printf(" %s",DACT_CONTACT);
#endif
				printf("\n");
				return(0);
				break;
			case 'u':
				dact_hdr_ext_regs(DACT_HDR_URL, optarg, strlen(optarg));
				break;
			case 'U':
				dact_hdr_ext_regs(DACT_HDR_URLFILE, optarg, strlen(optarg));
				break;
			case 'D':
				dact_hdr_ext_regs(DACT_HDR_DESC, optarg, strlen(optarg));
				break;
			case 'T':
				options[DACT_OPT_TIME]=!options[DACT_OPT_TIME];
				break;
			case 'P':
				options[DACT_OPT_PERM]=!options[DACT_OPT_PERM];
				break;
			case 'O':
				options[DACT_OPT_ORIG]=!options[DACT_OPT_ORIG];
				break;
			case 'S':
				options[DACT_OPT_SZSPD]=!options[DACT_OPT_SZSPD];
				break;
			case '?':
			case ':':
			case 'h':
				return(print_help(argc,argv));
		}

	}

/* 
 * Check for a new version of DACT 
 */
	if ((i=dact_upgrade_file_checkver("dact", DACT_BIN_URL_VER, options))>DACT_BIN_VER) {
		PRINTERR("**+");
		PRINTERR("**> There is a new version of DACT available.");
		PRINTERR("**>");
		PRINT_LINE; fprintf(stderr, "dact: **> [CURR: DACT %i.%i.%i]\n",DACT_VER_MAJOR, DACT_VER_MINOR, DACT_VER_REVISION);
		PRINT_LINE; fprintf(stderr, "dact: **> [NEW:  DACT %i.%i.%i]\n",i>>16,(i>>8)&0xff,i&0xff);
		PRINTERR("**>");
		PRINTERR("**> Run `dact -N' to get it.");
		PRINTERR("**> or get the source at: http://www.rkeene.org/devel/dact.tar.gz");
		PRINTERR("**>");
		PRINTERR("**-");
	}

	if (mode==DACT_MODE_RET) return(0);

	in_files=&argv[optind];

/* Loop through extra parameters (files ...) and setup FDs for them */
	do {
		in_fd=-1;
		out_fd=-1;

		in_file=in_files[filecnt];
		if (in_file!=NULL) {
/* Determine resulting file name */
			if (out_file==NULL) out_file=dact_getoutfilename(in_file, mode, ext);
			if (strcmp("-",in_file)==0) {
				in_fd=STDIN_FILENO;
			} else {
				if (stat(in_file, &stat_buf)>=0) {
					if (S_ISDIR(stat_buf.st_mode)) {
						fprintf(stderr, "dact: %s is a directory.\n",in_file);
						continue;
					}
				}
				if ((in_fd=open_net(in_file, O_RDONLY, 0))<0) {
					fprintf(stderr, "dact: Can't open %s.\n",in_file);
					PERROR_L(in_fd, "open");
					continue;
				}
			}
			if (out_file!=NULL) {
				if (!strcmp("-",out_file)) options[DACT_OPT_STDOUT]=1;
/*
 *
 * This is a bad thing if this program is SUID root, which it NEVER EVER 
 * should be.
 *
 *  DO NOT MAKE DACT SUID ROOT OR YOU WILL BE HACKED
 * 
 * that should be a suffcient warning.
 * (this is a mere race condition, but a severe warning should prevent
 *  people from complaining about it to me.)
 *
 */
				if (access(out_file,F_OK)!=-1 && options[DACT_OPT_FORCE]==0 && options[DACT_OPT_STDOUT]==0) {
					fprintf(stderr, "dact: %s exists.\n",out_file);
					close(in_fd);
					continue;
				}
				if (!options[DACT_OPT_STDOUT]) {
					if ((out_fd=open_net(out_file,O_WRONLY|O_CREAT|O_TRUNC,0644))<0) {
						fprintf(stderr, "dact: Can't open %s for writing.\n",out_file);
						PERROR_L(out_fd, "open");
						continue;
					}
				}
			}
			if (options[DACT_OPT_STDOUT]) out_fd=STDOUT_FILENO;
		}

/* Use STDIN/STDOUT if no files specified ... unless an outfile was specified... */
		if (in_file==NULL && filecnt==0) {
/* ... But only if STDOUT isn't a terminal */
			if (isatty(STDOUT_FILENO) && options[DACT_OPT_FORCE]==0) {
				fprintf(stderr, "dact: Refusing to write compressed output to a terminal.\n");
			} else {
				out_fd=STDOUT_FILENO;
				in_fd=STDIN_FILENO;
			}
		}

/* Okay, we're all done, now pass these to something to do the real stuff */
		if (in_fd!=-1 && (out_fd!=-1 || mode==DACT_MODE_STAT)) {
			crcs[1]=crcs[0]=0;
			if (dact_process_file(in_fd, out_fd, mode, options, in_file, crcs, dact_blk_size, ciphernum)==0) {
				close(in_fd);
				close(out_fd);
				if (out_fd!=STDOUT_FILENO) {
					unlink(out_file);
				}
				return(dact_shutdown(-1));
			}
		}
/* Cleanup */
		if (out_fd!=-1) close(out_fd);
		if (in_fd!=-1) close(in_fd);
	} while (in_files[filecnt++]!=NULL);

	return(dact_shutdown(0));
}
