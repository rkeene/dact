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

/*
 * Default DACT UI, this will probably be made modulizable.
 * 
 *   -- Roy Keene <rkeene@RKeene.org>
 *
 */

#include "dact.h"
#include "ui.h"
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdio.h>

char dact_ui_statusvar[128];

void dact_ui_update(void) {
	static int drawing=0;
	float done;
	char *bar1, *bar2, *clean="";
	int width=80,percent;

	if (!dact_ui_getopt(DACT_UI_OPT_LEVEL)) return; /* LEVEL 0 is Quiet */
	percent=dact_ui_getopt(DACT_UI_OPT_PERCENT);

	if (getenv("COLUMNS")!=NULL) width=atoi(getenv("COLUMNS"));
	if (width<10) return;
	width=(width>30?10:5);

	if (percent>100) percent=100;
	if (percent>-1) {
		done=(((float) width)*(((float) percent)/100));

		bar1=malloc((int) done+2);
		bar2=malloc((int) (width-done)+3);
		memset(bar1,'#',(int) done);
		memset(bar2,'.',(int) (width-done+0.9999999));
		bar1[(int) done]=0;
		bar2[(int) (width-done+0.9999999)]=0;
	} else {
		bar2=malloc(width+1);
		memset(bar2,'?',width);
		bar2[width]=0;
		bar1=bar2+width;
		percent=0;
	}
	if (dact_ui_getopt(DACT_UI_OPT_COLOR)) {
		fprintf(stderr, "=> \033[1;30m[\033[1;32m%s\033[1;37m%s\033[1;30m] \033[1;37m%03i\033[0;31m%%\033[0m",bar1,bar2,percent);
		clean="\033[K";
	} else {
		fprintf(stderr, "=> [%s%s] %3i%%",bar1,bar2,percent);
	}
	fprintf(stderr, " [%c] | Status: %s%s\r",*("|/-\\"+(drawing&3)),dact_ui_statusvar, clean);
	fflush(stderr);

	free(bar2);
	if (bar1!=(bar2+width)) free(bar1);

	drawing++;
	return;
}

void dact_ui_init(void /*for now*/) {
	dact_ui_setopt(DACT_UI_OPT_COLOR, 0);
	dact_ui_status(0, "Initialized.");
}

void dact_ui_deinit(void) {
	if (dact_ui_getopt(DACT_UI_OPT_LEVEL)) fprintf(stderr, "\n");
}

void dact_ui_percentdone(int percent) {
	dact_ui_setopt(DACT_UI_OPT_PERCENT,percent);
	dact_ui_update();
}

void dact_ui_incrblkcnt(int n) {
	static uint32_t blkcnt=0;
	uint32_t blocks;

	if (n==0) blkcnt=0;

	blkcnt+=n;
	blocks=dact_ui_getopt(DACT_UI_OPT_FILEBLKS);
	if (blocks==0) { 
		dact_ui_percentdone(-1);
		return;
	}
	dact_ui_percentdone((int) ((float) (((float) blkcnt/(float) blocks)*100)));
}

int32_t dact_ui_optmanip(int action, int opt, int32_t val) {
	static int opts[10]={0,0,0,0,0,0,0,0,0};

	if (opt>=(sizeof(opts)/sizeof(int))) return(-1);

	switch (action) {
		case DACT_UI_MANIP_GET:
			return(opts[opt]);
			break;
		case DACT_UI_MANIP_SET:
			return(opts[opt]=val);
			break;
	}
	return(-1);
}

int32_t dact_ui_getopt(int opt) {
	return(dact_ui_optmanip(DACT_UI_MANIP_GET, opt, 0));
}

int dact_ui_setopt(int opt, int32_t val) {
	return(dact_ui_optmanip(DACT_UI_MANIP_SET, opt, val));
}

void dact_ui_status(int level, const char *status) {
	if (level>dact_ui_getopt(DACT_UI_OPT_LEVEL)) return;
	strncpy(dact_ui_statusvar,status,sizeof(dact_ui_statusvar)-1);
	dact_ui_update();
}

void dact_ui_status_append(int level, const char *status) {
	if (level>dact_ui_getopt(DACT_UI_OPT_LEVEL)) return;
	strncat(dact_ui_statusvar,status,sizeof(dact_ui_statusvar)-strlen(dact_ui_statusvar)-2);
	dact_ui_update();
}

void dact_ui_setup(uint32_t blocks) {
	dact_ui_setopt(DACT_UI_OPT_FILEBLKS,blocks);
	dact_ui_incrblkcnt(0);
}

char *dact_ui_getuserinput(char *prompt, unsigned int n, int password) {
	char *ret;
	FILE *fd;

	if (password) {
		if (n<128) return(NULL);
		ret=getpass(prompt);
		return(ret);
	}
	if (dact_ui_getopt(DACT_UI_OPT_PASSSTDIN)==1) {
		fd=stdin;
	} else {
		fd=fopen("/dev/tty", "r");
	}
	if ((ret=malloc(n))==NULL) return(NULL);
	fprintf(stderr, "%s", prompt);
	fflush(stderr);
	fgets(ret, n, fd);
	ret=strsep(&ret, "\n\r");
	if (fd!=stdin) fclose(fd);
	return(ret);
}


/*
void dact_ui_debug()
*/
