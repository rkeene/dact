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
#include "buffer.h"
#include <stdio.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

uint32_t bit_buffer_data=0;
int bit_buffer_location=0;

char *byte_buffer_data=NULL;
int byte_buffer_location=-1;


/*
    Func: bit_buffer_read
    Args: 
           bits		- (int) Number of bits to read from the buffer.
    Rets: Int, value from buffer.
    Meth: Bread high BITS bits from BUFFER_DATA and shift to the left.
    Stat: Complete

*/
int bit_buffer_read(int bits) {
        int retval;
        if (bits>bit_buffer_location) { 
		DPRINTF("Request for %i bits, buffer contains %i bits",bits,bit_buffer_location);
		bits=bit_buffer_location;
	}

        retval=(bit_buffer_data>>((sizeof(bit_buffer_data)*8)-bits));
        bit_buffer_data=(bit_buffer_data<<bits);
	bit_buffer_location-=bits;
#ifdef BUFFER_VERBOSE
	DPRINTF("bit_buffer_read(%i)=%i",bits,retval);
#endif
        return(retval);
}

/*
    Func: bit_buffer_write
    Args:
           val		- (uint) Value to write
           bits		- (uint) Number of bits to allocate
    Rets: Nothing
    Meth: Add to right of current BUFFER_DATA[BUFFER_LOCATION], incremint
            BUFFER_LOCATION
    Stat: Complete
*/
void bit_buffer_write(unsigned int val, unsigned int bits) {
        while ((val>>bits)!=0) {
                DPRINTF("Value (%i) is bigger than %i bits.",val,bits);
		bits++;
        }

        if ((bits+bit_buffer_location)>(sizeof(bit_buffer_data)*8)) {
                DPRINTF("Buffer overflow");
                return;
        }
        bit_buffer_location+=bits;
        bit_buffer_data+=(val<<((sizeof(bit_buffer_data)*8)-bit_buffer_location));
#ifdef BUFFER_VERBOSE
	DPRINTF("bit_buffer_write(%i, %i)",val,bits);
#endif
        return;
}


/*
    Func: bit_buffer_size
    Args: None
    Rets: Int, number of bits in buffer
    Meth: buffer_location
    Stat: Complete
*/
int bit_buffer_size(void) {
#ifdef BUFFER_VERBOSE
	DPRINTF("bit_buffer_size()=%i",bit_buffer_location);
#endif
        return(bit_buffer_location);
}


/*
    Func: bit_buffer_purge
    Args: None
    Rets: Nothing
    Meth: Clear BUFFER_DATA and set BUFFER_LOCATION to 0
*/
void bit_buffer_purge(void) {
        bit_buffer_location=0;
        bit_buffer_data=0;
#ifdef BUFFER_VERBOSE
	DPRINTF("bit_buffer_purge()");
#endif
        return;
}


char *byte_buffer_read(int bytes) {
	char *retval;
	if (byte_buffer_location==-1) return(NULL);
	if (bytes>byte_buffer_location) {
		DPRINTF("Request for %i bytes, buffer contains %i bytes.",bytes,byte_buffer_location);
		bytes=byte_buffer_location;
	}
	if (!(retval=malloc(bytes))) {
		memcpy(retval,byte_buffer_data,bytes);
		memmove(byte_buffer_data,byte_buffer_data+bytes,(byte_buffer_location-=bytes));
		return(retval);
	} else {
		return(NULL);
	}
}

void byte_buffer_write(char *val, unsigned int size) {
	if (byte_buffer_location==-1) {
		byte_buffer_purge();
	}
	if ((byte_buffer_location+size)>(BYTE_BUFF_SIZE)) {
		DPRINTF("Buffer overflow.");
		return;
	}
	memcpy(byte_buffer_data+byte_buffer_location,val,size);
	byte_buffer_location+=size;
}

char byte_buffer_read_1(void) {
	char *buffread;
	char retval;
	buffread=byte_buffer_read(1);
	retval=buffread[0];
	free(buffread);
	return(retval);
}

int byte_buffer_size(void) {
	return(byte_buffer_location);
}

void byte_buffer_purge(void) {
	int i;

#if 0
	if (byte_buffer_data!=NULL) free(byte_buffer_data);
	byte_buffer_data=calloc(BYTE_BUFF_SIZE, 1);
#endif
	if (byte_buffer_data==NULL) {
		if (!(byte_buffer_data=malloc(BYTE_BUFF_SIZE))) return;
	}
	if (byte_buffer_location==-1) {
		for (i=0;i<(BYTE_BUFF_SIZE);i++) byte_buffer_data[i]=0;
	} else {
		for (i=0;i<(byte_buffer_location+1);i++) byte_buffer_data[i]=0;
	}
	byte_buffer_location=0;
	return;
}

