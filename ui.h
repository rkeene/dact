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

#ifndef _DACT_UI_H
#define _DACT_UI_H



#define DACT_UI_MANIP_SET	0
#define DACT_UI_MANIP_GET	1



#define DACT_UI_OPT_COLOR 	0
#define DACT_UI_OPT_LEVEL	1
#define DACT_UI_OPT_FILEBLKS	2
#define DACT_UI_OPT_PERCENT	3
#define DACT_UI_OPT_PASSSTDIN	4

#define DACT_UI_LVL_ALL		0
#define DACT_UI_LVL_GEN		1
#define DACT_UI_LVL_SPEC		2
#define DACT_UI_LVL_DEBUG	3
#define DACT_UI_LVL_VSPEC	6


void dact_ui_percentdone(int percent);
void dact_ui_incrblkcnt(int n);
void dact_ui_init(void);
int32_t dact_ui_optmanip(int action, int opt, int32_t val);
int32_t dact_ui_getopt(int opt);
int dact_ui_setopt(int opt, int32_t val);
void dact_ui_status(int level, const char *status);
void dact_ui_setup(uint32_t blocks);
void dact_ui_update(void);
void dact_ui_deinit(void);
void dact_ui_status_append(int level, const char *status);
char *dact_ui_getuserinput(char *prompt, unsigned int n, int password);

#endif
