/*
 * Copyright (C) 2013  Roy Keene
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 *      email: dact@rkeene.org
 */
#ifndef DACT_ALGORITHMS_H
#define DACT_ALGORITHMS_H 1

#include "algorithm-defs.h"

typedef enum {
	DACT_MODE_COMPR = 1,
	DACT_MODE_DECMP = 2
} dact_mode_t;

struct dact_compression_algo {
	char *name;
	int id;

	int (*function)(dact_mode_t mode, unsigned char *prev, unsigned char *block, unsigned char *out, int size, int bufsize);
};

#endif
