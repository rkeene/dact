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

#ifndef _ALGORITHMS_H
#define _ALOGIRTHMS_H
/*
	Algorithms!
*/

#include "dact.h"
#include "comp_plain.h"
#include "comp_rle.h"
#include "comp_delta.h"
#ifdef HAVE_LIBZ
#include "comp_zlib.h"
#include "comp_mzlib.h"
#include "comp_mzlib2.h"
#endif
#ifdef HAVE_LIBBZ2
#include "comp_bzlib.h"
#endif
#ifdef DEBUG
#include "comp_factor.h"
#include "comp_bitsums.h"
#include "comp_textrle.h"
#endif
#include "comp_snibble.h"
#include "comp_text.h"
#include "comp_fail.h"
#include "comp_lzoox.h"
#include "comp_lzooy.h"
#include "comp_lzota.h"

#ifdef __DACT_C
int (*(algorithms[256]))()={    comp_plain_algo,
                                comp_rle_algo,
                                comp_delta_algo,
				comp_text_algo,
#ifdef HAVE_LIBZ
				comp_zlib_algo,
				comp_mzlib_algo,
#else
				DACT_FAILED_ALGO,
				DACT_FAILED_ALGO,
#endif
				comp_snibble_algo,
				DACT_FAILED_ALGO,
#ifdef HAVE_LIBZ
				comp_mzlib2_algo,
#else
				DACT_FAILED_ALGO,
#endif
#ifdef HAVE_LIBBZ2
				comp_bzlib_algo,
#else
				DACT_FAILED_ALGO,
#endif
#ifdef DEBUG
				comp_factor_algo,
				comp_bitsums_algo,
				comp_textrle_algo,
#else
				DACT_FAILED_ALGO,
				DACT_FAILED_ALGO,
				DACT_FAILED_ALGO,
#endif
				comp_lzoox_algo,
				comp_lzooy_algo,
				comp_lzota_algo,
				NULL, NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				NULL, NULL, 
                                NULL};

char *algorithm_names[256]={	"Uncompressed",
				"RLE Compression",
				"Delta Compression",
				"Text Compression",
#ifdef HAVE_LIBZ
				"Zlib Compression",
				"Modified Zlib Compression",
#else
				"Unsupported Algorithm (zlib)",
				"Unsupported Algorithm (mzlib)",
#endif
				"Seminibble Encoding",
				"Range Encoding",
#ifdef HAVE_LIBZ
				"Second Modified Zlib Compression",
#else
				"Unsupported Algorithm (mzlib2)",
#endif
#ifdef HAVE_LIBBZ2
				"Bzip2 Compression",
#else
				"Unsupported Algorithm (bzlib)",
#endif
#ifdef DEBUG
				"Factor Compression",
				"Bitsums Compression",
				"Text RLE Compression",
#else
				"Unsupported Algorithm (factor)",
				"Unsupported Algorithm (bitsums)",
				"Unsupported Algorithm (textrle)",
#endif
#ifdef HAVE_LIBLZO
				"LZO-1x Compression",
				"LZO-1y Compression",
				"LZO-2a Compression",
#else
				"Unsupported Algorithm (comp_lzoox)",
				"Unsupported Algorithm (comp_lzooy)",
				"Unsupported Algorithm (comp_lzota)",
#endif
				NULL
			};
#else
extern int (*(algorithms[256]))();
extern char *algorithm_names[256];
#endif
#endif
