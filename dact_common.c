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
#define __DACT_C
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

int dact_config_execute(const char *cmd, char *options, uint32_t *blksize) {
	char *line=NULL, *line_s, *item_buf[4]={NULL, NULL, NULL, NULL};
	int i;

	line_s=line=strdup(cmd);
	if (line[0]=='#') return(0);
	while (line[strlen(line)-1]<32) line[strlen(line)-1]='\0';
	for (i=0;i<4;i++) item_buf[i]=NULL;
	for (i=0;i<4;i++) {
		if ((item_buf[i]=strsep(&line, "\t "))==NULL)  break;
		if (item_buf[i][0]==0) i--;
	}
	if (item_buf[0]==NULL || item_buf[1]==NULL) return(0); /* This means all commands must have arguments. */

	switch (ELFCRC(0, item_buf[0], strlen(item_buf[0]))) {
		case 164209419: /* binary_check */
			options[DACT_OPT_BINCHK]=!!strcmp(item_buf[1],"off");
			break;
		case 9456603: /* version_check */
			options[DACT_OPT_VERCHK]=!!strcmp(item_buf[1],"off");
			break;
		case 204349618: /* module_dir */
			if ((sizeof(moduledirectory)-strlen(moduledirectory)-1)<=0) break;
			strncat(moduledirectory,":",sizeof(moduledirectory)-strlen(moduledirectory)-1);
			strncat(moduledirectory,item_buf[1],sizeof(moduledirectory)-strlen(moduledirectory)-1);
			break;
		case 247248556: /* module_load_all */
			if (strcmp(item_buf[1], "on")==0) {
				init_modules();
				load_modules_all(options);
			}
			break;
		case 48402100:  /* module_load */
		case 106360197: /* load_module */
			init_modules();
			load_module(item_buf[1], options);
			break;
		case 164097267: /* network_access */
#ifndef NO_NETWORK
			dact_nonetwork=!strcmp(item_buf[1],"off");
#endif
			break;
		case 209445231: /* exclude_algo */
			i=(atoi(item_buf[1])&0xff);
			algorithms[i]=DACT_FAILED_ALGO;
			break;
		case 168825941: /* block_size */
			if (blksize!=NULL) {
				*blksize=atoi2(item_buf[1]);
			}
			break;
		case 162975987: /* use_urls */
			options[DACT_OPT_URL]=!!strcmp(item_buf[1],"off");
			break;
		case 104235033: /* color_ui */
			dact_ui_setopt(DACT_UI_OPT_COLOR,!!strcmp(item_buf[1],"off"));
			break;
		case 164800901: /* module_upgrade */
			if (strcmp(item_buf[1],"on")==0) options[DACT_OPT_UPGRADE]=1;
			break;
		case 63160590: /* pass_use_stdin */
		case 191551086: /* use_stdin */
			dact_ui_setopt(DACT_UI_OPT_PASSSTDIN, 1);
			break;
#ifdef DEBUG
		default:
			fprintf(stderr, "Unknown command %s (%i)\n",item_buf[0],ELFCRC(0,item_buf[0],strlen(item_buf[0])));
			break;
#endif
	}
	free(line_s);
	return(1);
}

void dact_config_loadfile(const char *path, char *options, uint32_t *blksize) {
	char *line=NULL;
	FILE *cfd;

	line=malloc(512);
	if ((cfd=fopen(path,"r"))==NULL) return;
	while (!feof(cfd)) {
		fgets(line, 511, cfd);
		dact_config_execute(line, options, blksize);
	}
	free(line);
	fclose(cfd);
}

uint32_t dact_blk_decompress(char *ret, const char *srcbuf, const uint32_t size, const char *options, const int algo, uint32_t bufsize) {
	uint32_t retval;

	if (algo==0xff) return(0);

	if (algorithms[algo]==NULL) {
		PRINTERR("Algorithm unavailble.");
		return(0);
	}

	retval=algorithms[algo](DACT_MODE_DECMP, NULL, srcbuf, ret, size, bufsize);

	return(retval);
}


uint32_t dact_blk_compress(char *algo, char *ret, const char *srcbuf, const uint32_t size, const char *options, uint32_t bufsize) {
	char *tmpbuf, *smallbuf=NULL;
	int i, highest_algo=0;
	char smallest_algo;
	uint32_t smallest_size=-1, x;
#ifndef DACT_UNSAFE
	char *verif_bf=NULL;
	uint32_t m;
	if ((verif_bf=malloc(size))==NULL) { PERROR("malloc"); return(0); }
#endif

	if ((tmpbuf=malloc(bufsize))==NULL) { PERROR("malloc"); return(0); }

	for (i=0;i<256;i++) {
		if (algorithms[i]!=NULL && algorithms[i]!=DACT_FAILED_ALGO) highest_algo=i;
	}

	for (i=0;i<=highest_algo;i++) {
		if (algorithms[i]!=NULL && algorithms[i]!=DACT_FAILED_ALGO) {
			x=algorithms[i](DACT_MODE_COMPR, NULL, srcbuf, tmpbuf, size, bufsize);
#ifndef DACT_UNSAFE
			if ((x<smallest_size || smallest_size==-1) && x!=-1) {
				m=algorithms[i](DACT_MODE_DECMP, NULL, tmpbuf, verif_bf, x, size);
				if (memcmp(verif_bf, srcbuf,m) || m!=size) {
					x=-1;
					DPRINTF("Verifcation failed! [algo=%i, %s]", i, algorithm_names[i]);
					if (options[DACT_OPT_COMPLN]) {
						dact_ui_status(DACT_UI_LVL_ALL, "Compression verification failed (ignoring)");
					}
				}
			}
#endif
			if ((x<smallest_size || smallest_size==-1) && x!=-1) {
				smallest_size=x;
				smallest_algo=i;
				if (smallbuf!=NULL) free(smallbuf);
				if ((smallbuf=malloc(smallest_size))==NULL) { 
					PERROR("malloc");
					free(tmpbuf);
#ifndef DACT_UNSAFE
					free(verif_bf);
#endif
					return(0);
				}
				memcpy(smallbuf, tmpbuf, smallest_size);
			}

			if (options[DACT_OPT_VERB]>2) {
				PRINT_LINE; fprintf(stderr, "dact: \033[%im----| %03i  | %-7i | %s\033[0m\n", (smallest_algo==i)*7 , i, x, algorithm_names[i]);
			}

		}
	}

	free(tmpbuf);
#ifndef DACT_UNSAFE
	free(verif_bf);
#endif
	if (smallest_size==-1) {
		return(0);
	}
	memcpy(algo, &smallest_algo, sizeof(char));
	memcpy(ret, smallbuf, smallest_size);
/* This was MISSING !  memory leak. */
	free(smallbuf);
	return(smallest_size);
}

uint32_t dact_process_file(const int src, const int dest, const int mode, const char *options, const char *filename, uint32_t *crcs, uint32_t dact_blksize, int cipher) {
	struct stat filestats;
	FILE *extd_urlfile;
	char *file_extd_urls[256];
	unsigned char algo;
	char ch;
	char *in_buf, *out_buf, *hdr_buf, *keybuf=NULL, *tmpbuf=NULL;
	char version[3]={DACT_VER_MAJOR, DACT_VER_MINOR, DACT_VER_REVISION};
	char file_opts=0;
	uint32_t bytes_read, retsize;
	uint32_t filesize=0, blk_cnt=0, file_extd_size=0, blksize=0, fileoutsize=0, out_bufsize=0;
	uint32_t magic=0, file_extd_read=0, file_extd_urlcnt=0;
	int blksize_size;
	int x=0, new_fd, canlseek=0;

	fstat(src,&filestats);

	if (mode==DACT_MODE_COMPR) {
/*
 * Calculate the default block size.
 */
		blksize=dact_blksize;
		if (blksize==0) {
			blksize=dact_blksize_calc(filestats.st_size);
		}
		out_bufsize=blksize*2;
		if (((in_buf=malloc(blksize))==NULL) || \
			((out_buf=malloc(out_bufsize))==NULL)) {
				PERROR("malloc");
				return(0);
		}

		dact_ui_setup(((float) (filestats.st_size/blksize)+0.9999));
		if (cipher!=-1) {
			dact_hdr_ext_regn(DACT_HDR_CIPHER, cipher, sizeof(cipher));
			keybuf=malloc(DACT_KEY_SIZE);
			ciphers[cipher](NULL, NULL, 0, keybuf, DACT_MODE_CINIT+DACT_MODE_CENC);
			
		}
		blksize_size=BYTESIZE(blksize);

		if (!options[DACT_OPT_ORIG] && filename!=NULL)
			dact_hdr_ext_regs(DACT_HDR_NAME, filename, strlen(filename));
		file_extd_size=(dact_hdr_ext_size()+14); /* The +14 is for crc0 and crc1 */
		write_de(dest, DACT_MAGIC_NUMBER, 4);
		write(dest, &version[0], 1);
		write(dest, &version[1], 1);
		write(dest, &version[2], 1);
		write_de(dest, 0, 4); /* Place holder for ORIG FILE SIZE */
		write_de(dest, 0, 4); /* Place holder for NUM BLOCKS */
		write_de(dest, blksize, 4);
		write_de(dest, file_opts, 1); /* XXX: Option byte... Or not? */
		write_de(dest, file_extd_size, 4); /* Place holder for SIZEOF EXTENDED DTA */
/* Fill the header with NOPs incase we can't come back and put the CRCs */
		ch=DACT_HDR_NOP;
		for (x=0;x<file_extd_size;x++) write(dest, &ch, 1);

		if (options[DACT_OPT_VERB]>1) {
			PRINTERR("Blk | Algo | Size    | Name");
			PRINTERR("----+------+---------+---------------------------");
		}

		memset(in_buf, 0, blksize);
		while ( (bytes_read=read_f(src, in_buf, blksize))>0) {
			filesize+=bytes_read;
			blk_cnt++;

			retsize=dact_blk_compress(&algo, out_buf, in_buf, blksize, options, out_bufsize);

/* CIPHER the data if an encryption algorithm is specified. */
			if (cipher!=-1) {
				tmpbuf=malloc(retsize*2);
				x=ciphers[cipher](out_buf, tmpbuf, retsize, keybuf, DACT_MODE_CENC);
				memcpy(out_buf,tmpbuf,x);
				free(tmpbuf);
			}

			if (retsize>0) {
				if (options[DACT_OPT_VERB]>1) {
					if (options[DACT_OPT_VERB]>2) {
						PRINTERR("^^^\\ /^^^^\\ /^^^^^^^\\ /^^^^^^^^^^^^^^^^^^^^^^^^^^");
					}
					PRINT_LINE; fprintf(stderr, "dact: %03i | %03i  | %-7i | %s\n",blk_cnt,algo,retsize,algorithm_names[algo]);
					if (options[DACT_OPT_VERB]>2) {
						PRINTERR("___/ \\____/ \\_______/ \\__________________________");
					}
				}


				dact_ui_incrblkcnt(1);
				dact_ui_status(DACT_UI_LVL_GEN, "Algorithm ");
				dact_ui_status_append(DACT_UI_LVL_GEN,algorithm_names[algo]);

				crcs[0]=ELFCRC(crcs[0], out_buf, retsize);
/* Do not generate a CRC of the plaintext if encrypting */
				if (cipher==-1) {
					crcs[1]=ELFCRC(crcs[1], in_buf, blksize);
				}

				if (!options[DACT_OPT_HDONLY]) {
					write(dest, &algo, 1);
					write_de(dest, retsize, blksize_size);

					if (write(dest, out_buf, retsize)!=retsize) {
						PERROR("write");
						free(in_buf);
						free(out_buf);
						return(0);
					}
				}
			} else {
				PRINTERR("Compression resulted in 0-byte block.");
				free(in_buf);
				free(out_buf);
				return(0);
			}
			memset(in_buf, 0, blksize);
		}

		if (bytes_read<0) {
			PERROR("read");
		}

		free(in_buf);
		free(out_buf);

		if (lseek_net(dest, 7, SEEK_SET)<0) {
/* If we can't rewind the stream, put magic+fileisze */
			write_de(dest, DACT_MAGIC_PEOF, 4);
			write_de(dest, filesize, 4);
		} else {
			write_de(dest, filesize, 4);
			write_de(dest, blk_cnt, 4);
		} 

		if (lseek_net(dest, DACT_HDR_REG_SIZE, SEEK_SET)>0) {
			if (!options[DACT_OPT_NOCRC]) {
				dact_hdr_ext_regn(DACT_HDR_CRC0, crcs[0], 4);
				dact_hdr_ext_regn(DACT_HDR_CRC1, crcs[1], 4);
			}
			write(dest, dact_hdr_ext_data(), dact_hdr_ext_size());
		}

		dact_hdr_ext_clear();

		return(filesize);
	}

	if (mode==DACT_MODE_DECMP) {

		dact_ui_status(DACT_UI_LVL_GEN, "Decompressing.");

		dact_hdr_ext_clear();

		read_de(src, &magic, 4, sizeof(magic));

		if (magic!=DACT_MAGIC_NUMBER) {
			dact_ui_status(DACT_UI_LVL_GEN, "Bad DACT magic, checking others...");
			return(dact_process_other(src,dest,magic,options));
		}

		read(src, &version[0], 1);
		read(src, &version[1], 1);
		read(src, &version[2], 1);
		read_de(src, &filesize, 4, sizeof(filesize));
		read_de(src, &blk_cnt, 4, sizeof(blk_cnt));
		read_de(src, &blksize, 4, sizeof(blksize));
		read(src, &file_opts, 1);
		read_de(src, &file_extd_size, 4, sizeof(file_extd_size));



		while (file_extd_read<file_extd_size) {
			x=0;
			read(src, &ch, 1);
			if (ch!=DACT_HDR_NOP) read_de(src, &x, 2, sizeof(x)); 
			switch (ch) {
				case DACT_HDR_CRC0:
					read_de(src, &crcs[2], 4, sizeof(crcs[2]));
					if (crcs[4]!=0 && crcs[2]!=crcs[4]) {
						dact_ui_status(DACT_UI_LVL_GEN, "CRC error.");
						if (!options[DACT_OPT_NOCRC])
							return(0);
					}
					break;
				case DACT_HDR_CRC1:
					read_de(src, &crcs[3], 4, sizeof(crcs[3]));
					if (crcs[5]!=0 && crcs[3]!=crcs[5]) {
						dact_ui_status(DACT_UI_LVL_GEN, "CRC error.");
						if (!options[DACT_OPT_NOCRC])
							return(0);
					}
					break;
/*

XXX: Todo, make this do something...
		 		case DACT_HDR_NAME:
					break;
*/
				case DACT_HDR_URL:
					hdr_buf=malloc(x+1);
					read_f(src, hdr_buf, x);
					hdr_buf[x]=0;
					file_extd_urls[file_extd_urlcnt++]=parse_url_subst(hdr_buf,filename);
					free(hdr_buf);
					break;
				case DACT_HDR_URLFILE:
					hdr_buf=malloc(x+1);
					read_f(src, hdr_buf, x);
					hdr_buf[x]=0;
					extd_urlfile=fopen(hdr_buf, "r");
					free(hdr_buf);   /* We shouldn't need this anymore. */
					if (extd_urlfile==NULL) break;
					hdr_buf=malloc(4096);
					while (1) {
						fgets(hdr_buf, 4095, extd_urlfile);
						if (feof(extd_urlfile)) break;
						hdr_buf=strsep(&hdr_buf,"\n");
						file_extd_urls[file_extd_urlcnt++]=parse_url_subst(hdr_buf,filename);
					}
					free(hdr_buf);
					break;
				case DACT_HDR_DESC:
					hdr_buf=malloc(x+1);
					read_f(src, hdr_buf, x);
					hdr_buf[x]=0;
					fprintf(stderr, "DESC: %s\n",hdr_buf);
					free(hdr_buf);
					break;
				case DACT_HDR_CIPHER:
					read_de(src,&cipher,x,sizeof(cipher));
					break;
				case DACT_HDR_NOP:
					x=-2;
					break;
				default:
					hdr_buf=malloc(x);
					read_f(src, hdr_buf, x);
					free(hdr_buf);
					break;
			}



			file_extd_read+=(x+3);
		}

		if (options[DACT_OPT_URL]) {
			for (x=0;x<file_extd_urlcnt;x++) {
				dact_ui_status(DACT_UI_LVL_GEN,"Trying to get remote url ");
				dact_ui_status_append(DACT_UI_LVL_SPEC, file_extd_urls[x]);
				if ((new_fd=open_net(file_extd_urls[x],O_RDONLY,0))<0) {
					dact_ui_status(DACT_UI_LVL_GEN, "Failed.");
					continue;
				}
				close(src);
				crcs[4]=crcs[2];
				crcs[5]=crcs[3];
				return(dact_process_file(new_fd, dest, mode, options, filename, crcs, blksize, cipher));
			}
		}


/* 
   XXX: Even if we don't resolve it here, we can resolve it later...
   Should we even bother to do it here if we can?

   XXX: When CAN'T we rewind a read stream?
		When it's a pipe

*/
		if (filesize==0) {
/* See if we can rewind our stream, so when we get to the end, we can come back! */
			if (lseek_net(src, 1, SEEK_SET)==1) { /* MAJOR BUG HERE! was: lseek(src,1,SEEK_SET)==0 which will always be false. */
				canlseek=1;
				lseek_net(src, -8, SEEK_END);
				read_de(src, &magic, 4, sizeof(magic));
				if (magic!=DACT_MAGIC_PEOF) {
					dact_ui_status(DACT_UI_LVL_GEN, "File is corrupt.");
					return(0);
				}
				read_de(src, &filesize, 4, sizeof(filesize));
				lseek_net(src, DACT_HDR_REG_SIZE+file_extd_size, SEEK_SET);
			} else {
				canlseek=0;
			}
		}

		out_bufsize=blksize;
		if (((out_buf=malloc(out_bufsize))==NULL) ) {
				PERROR("malloc");
				return(0);
		}


		blksize_size=BYTESIZE(blksize);

		dact_ui_setup((int)(((float) filesize/(float) blksize)+0.9999));

		if (cipher!=-1) {
			keybuf=malloc(DACT_KEY_SIZE);
			ciphers[cipher](NULL, NULL, 0, keybuf, DACT_MODE_CINIT+DACT_MODE_CDEC);
			
		}



		while (1) {
			if (read(src, &algo, 1)==0) break;
			if (algo==0xff) break; /* 0xff is reserved for EOF */

			read_de(src, &blksize, blksize_size, sizeof(blksize));

			if ((in_buf=malloc(blksize))==NULL) {
				PERROR("malloc");
				free(out_buf);
				return(0);
			}

			read_f(src, in_buf, blksize);

			crcs[0]=ELFCRC(crcs[0],in_buf,blksize);

			if (cipher!=-1) {
				tmpbuf=malloc(blksize*2);
				x=ciphers[cipher](in_buf, tmpbuf, blksize, keybuf, DACT_MODE_CDEC);
				memcpy(in_buf,tmpbuf,x);
				free(tmpbuf);
			}

/*
 * If the filesize is not specified, try to find it in the stream...
 * this is pretty stupid, because if we can't rewind OUR read stream
 * we're SOL... do we really need the filesize that badly?  I guess
 * we do to truncate the last of it...  Adding more checks in here..
 *
 * This will never get used, canlseek will be 1 only if we can sucessfully
 * seek, in which case, we have done it above.
 */
#if 0
			if (filesize==0 && canlseek) {
				read_de(src, &magic, 4);
				read_de(src, &filesize, 4);
				if (read(src, &x, 1)==0) {
					if (magic!=DACT_MAGIC_PEOF) {
						dact_ui_status(DACT_UI_LVL_GEN, "Stream is corrupt.");
						free(in_buf);
						free(out_buf);
						return(0);
					}
				} else {
					lseek(src, -9, SEEK_CUR);
					filesize=0;
				}
			}
#endif


			if ((bytes_read=dact_blk_decompress(out_buf, in_buf, blksize, 0, algo, out_bufsize))==0) {
				if (cipher!=-1) {
					PRINTERR("Decompression resulted in 0-byte block.  Invalid key?");
				} else {
					PRINTERR("Decompression resulted in 0-byte block.");
				}
			}
			fileoutsize+=bytes_read;

/* If ciphering, don't try to calculate this CRC. */
			if (cipher==-1) {
				crcs[1]=ELFCRC(crcs[1],out_buf,bytes_read);
			}

			dact_ui_incrblkcnt(1);

			if (fileoutsize>filesize && filesize!=0) {
				write(dest, out_buf, blksize-(fileoutsize-filesize));
			} else {
				write(dest, out_buf, bytes_read);
			}



			free(in_buf);

			
		}

		free(out_buf);

		if ((crcs[0]!=crcs[2] && crcs[0]!=0 && crcs[2]!=0) \
		  || (crcs[1]!=crcs[3] && crcs[1]!=0 && crcs[3]!=0)) {
			dact_ui_status(DACT_UI_LVL_GEN, "CRC error.");
			if (!options[DACT_OPT_NOCRC] || options[DACT_OPT_FORCE]<1)
				return(0);
		}

		dact_hdr_ext_clear();

		return(filesize);

	}


	if (mode==DACT_MODE_STAT) {
		read_de(src, &magic, 4, sizeof(magic));
		read(src, &version[0], 1);
		read(src, &version[1], 1);
		read(src, &version[2], 1);
		read_de(src, &filesize, 4, sizeof(filesize));
		read_de(src, &blk_cnt, 4, sizeof(blk_cnt));
		read_de(src, &blksize, 4, sizeof(blksize));
		read(src, &file_opts, 1);
		read_de(src, &file_extd_size, 4, sizeof(file_extd_size));

		printf("File              :   %s\n", filename);
		printf("Magic             :   0x%08x",magic);
		if (magic!=DACT_MAGIC_NUMBER) {
			printf(" (bad magic)\n");
			return(0);
		} else {
			printf("\n");
		}

		if (filesize==0) {
			lseek_net(src, -8, SEEK_END);
			read_de(src, &magic, 4, sizeof(magic));
			read_de(src, &filesize, 4, sizeof(filesize));
			if (magic!=DACT_MAGIC_PEOF) {
				PRINTERR("Bad magic, corrupt stream.");
				return(0);
			}
		}
		fileoutsize=lseek_net(src, 0, SEEK_END);

		printf("Dact Version      :   %i.%i.%i\n",version[0],version[1],version[2]);
		printf("Block Size        :   %i\n", blksize);
		printf("Block Header Size :   %i\n", BYTESIZE(blksize)+1);
		printf("Compressed Size   :   %i\n", fileoutsize);
		printf("Uncompressed Size :   %i\n", filesize);
		printf("Ratio             :   %2.3f to 1.0\n", ((float) filesize)/((float) fileoutsize) );
		lseek_net(src, DACT_HDR_REG_SIZE, SEEK_SET);
		while (file_extd_read<file_extd_size) {
			x=0;
			read(src, &ch, 1);
			if (ch!=DACT_HDR_NOP) read_de(src, &x, 2, sizeof(x)); 
			switch (ch) {
				case DACT_HDR_CRC0:
					read_de(src, &crcs[2], 4, sizeof(crcs[2]));
					printf("CRC 0             :   0x%08x\n", crcs[2]);
					break;
				case DACT_HDR_CRC1:
					read_de(src, &crcs[3], 4, sizeof(crcs[3]));
					printf("CRC 1             :   0x%08x\n", crcs[3]);
					break;
		 		case DACT_HDR_NAME:
					hdr_buf=malloc(x+1);
					read_f(src, hdr_buf, x);
					hdr_buf[x]=0;
					printf("Original Name     :   %s\n", hdr_buf);
					free(hdr_buf);
					break;
				case DACT_HDR_URL:
					hdr_buf=malloc(x+1);
					read_f(src, hdr_buf, x);
					hdr_buf[x]=0;
					file_extd_urls[file_extd_urlcnt++]=parse_url_subst(hdr_buf,filename);
					free(hdr_buf);
					break;
				case DACT_HDR_URLFILE:
					hdr_buf=malloc(x+1);
					read_f(src, hdr_buf, x);
					hdr_buf[x]=0;
					printf("Download Loc File :   %s\n", hdr_buf);
					extd_urlfile=fopen(hdr_buf, "r");
					free(hdr_buf);   /* We shouldn't need this anymore. */
					if (extd_urlfile==NULL) break;
					hdr_buf=malloc(4096);
					while (1) {
						fgets(hdr_buf, 4095, extd_urlfile);
						if (feof(extd_urlfile)) break;
						hdr_buf=strsep(&hdr_buf,"\n");
						file_extd_urls[file_extd_urlcnt++]=parse_url_subst(hdr_buf,filename);
					}
					free(hdr_buf);
					break;
				case DACT_HDR_DESC:
					hdr_buf=malloc(x+1);
					read_f(src, hdr_buf, x);
					hdr_buf[x]=0;
					printf("Description       :   %s\n", hdr_buf);
					free(hdr_buf);
					break;
				case DACT_HDR_CIPHER:
					read_de(src, &cipher, x, sizeof(cipher));
					printf("Ciphered using    :   %s\n", ciphers_name[cipher]);
					break;
				case DACT_HDR_NOP:
					x=-2;
					break;
				default:
					hdr_buf=malloc(x);
					read_f(src, hdr_buf, x);
					free(hdr_buf);
					break;
			}


			file_extd_read+=(x+3);
		}

		for (x=0;x<file_extd_urlcnt;x++) {
			printf("Download Location :   %s",file_extd_urls[x]);
			if (options[DACT_OPT_VERB]) {
				if ((new_fd=open_net(file_extd_urls[x],O_RDONLY, 0))<0) {
					printf(" [broken]\n");
					continue;
				}
				close(new_fd);
			}
			printf("\n");
		}

		blk_cnt=0;
		blksize_size=BYTESIZE(blksize);
		if (options[DACT_OPT_VERB]) {
			lseek_net(src, DACT_HDR_REG_SIZE+file_extd_size, SEEK_SET);
			printf("\n");
			printf("Break down: \n");
			printf("  Blk | Algo | Size    | Name\n");
			printf("  ----+------+---------+---------------------------\n");
			while (1) {
				if (read(src, &algo, 1)==0) break;
				if (algo==0xff) break; /* 0xff is reserved for EOF */

				read_de(src, &blksize, blksize_size, sizeof(blksize));
				lseek_net(src, blksize, SEEK_CUR);
				printf("  %03i | %03i  | %-7i | %s\n",blk_cnt,algo,blksize,algorithm_names[algo]);
				blk_cnt++;
			}
		}

		printf("\n");


		return(1);
	}

	return(0);
}
