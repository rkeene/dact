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

#ifndef _PARSE_C
#define _PARSE_C
#include "dact.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

uint32_t atoi2(const char *n);
int parse_url(const char *url, char *scheme, char *username, char *password, char *host, int *port, char *file);
void strtolower(char *str);
char *parse_url_subst(const char *src, const char *fname);
char *parse_url_subst_dist(void);
char *mime64(unsigned char *str);
char *mimes64(unsigned char *str, int *size);
char *demime64(unsigned char *str);
int32_t read_f(int fd, void *buf, size_t count);
uint32_t hash_fourbyte(unsigned char *str, const char term);
#endif
