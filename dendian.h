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

#ifndef __ENDIAN_LOCAL_H
#define __ENDIAN_LOCAL_H

int write_de(const int dst, const uint64_t num, const int sze);
int read_de(const int src, void *dest, const int sze, const int out_sze);

/*
#ifdef __DACT_C
unsigned char ENDIAN_LOCAL_BUF[4]={0, 0, 0, 0};
#endif

#define WRITE_2BYTE_DE(x,y) 2; \
			ENDIAN_LOCAL_BUF[0]=((y&0xff00) >> 8); \
			ENDIAN_LOCAL_BUF[1]=((y&0x00ff)); \
			write(x,&ENDIAN_LOCAL_BUF[0],1); \
			write(x,&ENDIAN_LOCAL_BUF[1],1)

#define WRITE_4BYTE_DE(x,y) 4; \
			ENDIAN_LOCAL_BUF[0]=((y&0xff000000) >> 24); \
			ENDIAN_LOCAL_BUF[1]=((y&0x00ff0000) >> 16); \
			ENDIAN_LOCAL_BUF[2]=((y&0x0000ff00) >> 8); \
			ENDIAN_LOCAL_BUF[3]=((y&0x000000ff)); \
			write(x,&ENDIAN_LOCAL_BUF[0],1); \
			write(x,&ENDIAN_LOCAL_BUF[1],1); \
			write(x,&ENDIAN_LOCAL_BUF[2],1); \
			write(x,&ENDIAN_LOCAL_BUF[3],1)

#define READ_2BYTE_DE(x,y) (read(x,&ENDIAN_LOCAL_BUF[0],1)+ \
			    read(x,&ENDIAN_LOCAL_BUF[1],1)); \
			y=((ENDIAN_LOCAL_BUF[0]<<8)|ENDIAN_LOCAL_BUF[1])

#define READ_4BYTE_DE(x,y) (read(x,&ENDIAN_LOCAL_BUF[0],1)+\
			    read(x,&ENDIAN_LOCAL_BUF[1],1)+\
			    read(x,&ENDIAN_LOCAL_BUF[2],1)+\
			    read(x,&ENDIAN_LOCAL_BUF[3],1)); \
			y=((ENDIAN_LOCAL_BUF[0]<<24)|(ENDIAN_LOCAL_BUF[1]<<16)|(ENDIAN_LOCAL_BUF[2]<<8)|ENDIAN_LOCAL_BUF[3])

#define WRITE_NBYTE_DE(x,y,n) (((int) (n/2))*2); if (n<=2) { WRITE_2BYTE_DE(x,y); } else { WRITE_4BYTE_DE(x,y); }

#define READ_NBYTE_DE(x,y,n) (((int) (n/2))*2); if (n<=2) { READ_2BYTE_DE(x,y); } else { READ_4BYTE_DE(x,y); }

*/

#endif

