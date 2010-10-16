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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 *      email: dact@rkeene.org
 */

#ifndef _LOCAL_MATH_H
#define _LOCAL_MATH_H
#include <math.h>

#ifndef M_LN2l
#define M_LN2l 0.6931471805599453094172321214581766L  /* log_e 2 */
#endif


#define BYTESIZE(x) ((int) ((float) (((log(x+1)/M_LN2l) + 7.99999999) / 8.0000)))


#endif
