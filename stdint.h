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

#ifndef _STDINT_H
#define _STDINT_H

#include "config.h"

#ifndef uint32_t

#if SIZEOF_INT == 4
#define uint32_t unsigned int
#define int32_t int
#elif SIZEOF_SHORT == 4
#define uint32_t unsigned short
#define int32_t short
#elif SIZEOF_LONG == 4
#define uint32_t unsigned long
#define int32_t long
#endif

#endif/* !uint32_t */

#ifndef uint16_t

#if SIZEOF_INT == 2
#define uint16_t unsigned int
#define int16_t int
#elif SIZEOF_SHORT == 2
#define uint16_t unsigned short
#define int16_t short
#elif SIZEOF_LONG == 2
#define uint16_t unsigned long
#define int16_t long
#endif

#endif/* !uint16_t */


#endif/*_STDINT_H*/
