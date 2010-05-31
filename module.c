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
#include "module.h"
#ifdef USE_MODULES
#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#include <stdio.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <fcntl.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#include "algorithms.h"
#include "ciphers.h"
#include "parse.h"
#include "net.h"
#include "ui.h"

char moduledirectory[2048] = "@@HOME@@/.dact/@@OSNM@@-@@ARCH@@/";
void *modules[256];
int modules_initialized = 0;
int modules_count = 0;

int init_modules (void) {
	int i;

	if (modules_initialized!=0) return(DACT_MOD_FAIL);
	modules_initialized=1;

	for (i=0;i<256;i++) {
		modules[i]=NULL;
	}
	return(DACT_MOD_OK);
}

int unload_modules (void) {
	int i;

	if (modules_initialized==0) return(DACT_MOD_FAIL);

	for (i=0;i<256;i++) {
		if (modules[i]!=NULL)
			dlclose(modules[i]);
	}
	return(DACT_MOD_OK);
}

int load_modules_all(const unsigned char *options) {
#ifdef HAVE_DIRENT_H
	struct dirent *dinfo;
	char *mdircpy, *tmpbuf, *mbuf, *fname, fullfname[1024];
	DIR *dirtype;

	mdircpy=parse_url_subst(moduledirectory, "");
	mbuf=mdircpy;
	while ((tmpbuf=strsep(&mbuf, ":"))) {
		if ((dirtype=opendir(tmpbuf))==NULL) continue;
		while ((dinfo=readdir(dirtype))!=NULL) {
			fname=dinfo->d_name;
			if (strcmp(fname+strlen(fname)-3,".so")==0) {
				strncpy(fullfname, tmpbuf, sizeof(fullfname));
				strncat(fullfname, "/", sizeof(fullfname)-strlen(fullfname));
				strncat(fullfname, fname, sizeof(fullfname)-strlen(fullfname));
				load_module(fullfname, options);
			}
		}
		closedir(dirtype);
	}
	free(mdircpy);
#else
	PRINTERR("I don't know how to handle this.")
#endif
	return(DACT_MOD_OK);
}

int load_module (char *modulename, const unsigned char *options) {
	char modulefile[256], *tmpbuf, *mbuf, *mdircpy;
	void *mh=NULL;
	uint32_t algo_num, module_type=DACT_MOD_TYPE_COMP;
	uint32_t dc_ver=0, dc_req=0, dact_ver;
	char *dc_url_get=NULL, *dc_url_ver=NULL, *dc_sign=NULL;

	if (strchr(modulename,'/')==NULL) {
		mdircpy=parse_url_subst(moduledirectory, "");
		mbuf=mdircpy;
		while ((tmpbuf=strsep(&mbuf, ":"))) {
			snprintf(modulefile, sizeof(modulefile)-1, "%s/%s.so",tmpbuf,modulename);
			if ((mh=dlopen(modulefile, RTLD_GLOBAL|RTLD_NOW))!=NULL) break;
		}
		free(mdircpy);
	} else {
		strncpy(modulefile, modulename, sizeof(modulefile)-1);
		if ((mh=dlopen(modulefile, RTLD_GLOBAL|RTLD_NOW))==NULL) {
			PRINTERR("Could not load module.");
			return(DACT_MOD_FAIL);
		}
	}
	if (!mh) return(DACT_MOD_FAIL);

	if (dlsym(mh,"DC_NUM")==NULL \
	|| dlsym(mh, "DC_NAME")==NULL \
	|| dlsym(mh, "DC_ALGO")==NULL) {
		dact_ui_status(DACT_UI_LVL_SPEC, modulefile);
		dact_ui_status_append(DACT_UI_LVL_SPEC, " is not a dact module.");
		dlclose(mh);
		return(DACT_MOD_FAIL);
	}

	if (dlsym(mh, "DC_TYPE")!=NULL) {
		memcpy(&module_type,dlsym(mh,"DC_TYPE"),sizeof(module_type));
	}

	memcpy(&algo_num,dlsym(mh,"DC_NUM"),sizeof(algo_num));

	if (dlsym(mh, "DC_VER")!=NULL) memcpy(&dc_ver, dlsym(mh, "DC_VER"), sizeof(dc_ver));
	if (dlsym(mh, "DC_REQUIRE")!=NULL) memcpy(&dc_req, dlsym(mh, "DC_REQUIRE"), sizeof(dc_req));
	if (dlsym(mh, "DC_URL_GET")!=NULL) memcpy(&dc_url_get, dlsym(mh, "DC_URL_GET"), sizeof(dc_url_get));
	if (dlsym(mh, "DC_URL_VER")!=NULL) memcpy(&dc_url_ver, dlsym(mh, "DC_URL_VER"), sizeof(dc_url_ver));
	if (dlsym(mh, "DC_SIGN")!=NULL) memcpy(&dc_sign, dlsym(mh, "DC_SIGN"), sizeof(dc_sign));

	if (dc_url_get!=NULL && dc_url_ver!=NULL && dc_ver!=0 && modulename[0]!='/') {
		dact_upgrade_file(modulename, dc_url_get, dc_url_ver, dc_ver, NULL, options);
	}

	if (dc_req!=0) {
		dact_ver=(DACT_VER_MAJOR<<16)|(DACT_VER_MINOR<<8)|(DACT_VER_REVISION);
		switch (dc_req&0xff000000) {
			case DACT_MOD_REQ_ATLEAST:
				if (dact_ver<(dc_req&0xffffff)) {
					fprintf(stderr, "%s requires atleast DACT %i.%i.%i, this is DACT %i.%i.%i\n",modulefile,DACT_VER_PARTS(dc_req), DACT_VER_MAJOR, DACT_VER_MINOR, DACT_VER_REVISION);
					dlclose(mh);
					return(DACT_MOD_FAIL);
				}
				break;
			case DACT_MOD_REQ_EXACTLY:
				if ((dc_req&0xffffff)!=dact_ver) {
					fprintf(stderr, "%s requires DACT %i.%i.%i, this is DACT %i.%i.%i\n",modulefile, DACT_VER_PARTS(dc_req), DACT_VER_MAJOR, DACT_VER_MINOR, DACT_VER_REVISION);
					dlclose(mh);
					return(DACT_MOD_FAIL);
				}
				break;
			case DACT_MOD_REQ_ATMOST:
				if (dact_ver>(dc_req&0xffffff)) {
					fprintf(stderr, "%s requires atmost DACT %i.%i.%i, this is DACT %i.%i.%i\n",modulefile,DACT_VER_PARTS(dc_req), DACT_VER_MAJOR, DACT_VER_MINOR, DACT_VER_REVISION);
					dlclose(mh);
					return(DACT_MOD_FAIL);
				}
				break;
		}
	}



	if (modules_count<255) {
		modules[modules_count++]=mh;
	}

	switch (module_type) {
		case DACT_MOD_TYPE_COMP:

			if (algo_num>=((sizeof(algorithms))/(sizeof(*algorithms)))) return(DACT_MOD_FAIL);
			if (algorithms[algo_num]!=DACT_FAILED_ALGO && algorithms[algo_num]!=NULL) {
				dlclose(mh);
				return(DACT_MOD_FAIL);
			}

			memcpy(&algorithms[algo_num],dlsym(mh, "DC_ALGO"),sizeof(algorithms[0]));
			memcpy(&algorithm_names[algo_num],dlsym(mh,"DC_NAME"),sizeof(algorithm_names[0]));


			return(DACT_MOD_OK);
		case DACT_MOD_TYPE_ENC:
			if (algo_num>=CIPHER_COUNT) {
				printf("Encryption algorithm number too high, ignoring %i\n", algo_num);
				return(DACT_MOD_FAIL);
			}
			if (ciphers[algo_num]!=DACT_FAILED_ALGO && ciphers[algo_num]!=NULL) {
				return(DACT_MOD_FAIL);
			}
			memcpy(&ciphers[algo_num],dlsym(mh, "DC_ALGO"),sizeof(algorithms[0]));
			memcpy(&ciphers_name[algo_num],dlsym(mh, "DC_NAME"),sizeof(ciphers_name[0]));
			return(DACT_MOD_OK);
	}
	return(DACT_MOD_FAIL);
}
#else
char moduledirectory[2048] = ".";
void *modules[256];
int modules_initialized = 0;
int modules_count = 0;

int init_modules(void) { return(DACT_MOD_FAIL); }
int unload_modules(void) { return(DACT_MOD_FAIL); }
int load_module(char *modulename, const unsigned char *options) { return(DACT_MOD_FAIL); }
int load_modules_all(const unsigned char *options) { return(DACT_MOD_FAIL); }



#endif
