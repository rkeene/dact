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
#include "sort.h"
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include <stdio.h>


/*
	Bubble Sort
*/
void int_sort (uint32_t array[], unsigned int listsize, int mode) {
	uint32_t *mod_array=NULL;
	unsigned int temp,i,j;

	mode=(!!mode);

	if (mode) {
		mod_array=malloc(listsize*sizeof(uint32_t));
		for (i=0;i<listsize;i++) mod_array[i]=i;
	}

	for (i=0;i<(listsize);i++) {
		for (j=0;j<(listsize-1);j++) {
		        if (array[j] < array[j+1]) {
				temp = array[j];
				array[j] = array[j+1];
				array[j+1] = temp;
				switch (mode) {
					case 0 : break;
					case 1 :
						temp=mod_array[j];
						mod_array[j]=mod_array[j+1];
						mod_array[j+1]=temp;
				}
			}
		}
	}

	if (mode) {
		memcpy(array, mod_array, listsize*sizeof(uint32_t));
		free(mod_array);
	}

}


/*
	I have no idea what this sort is called, if it has a name
	(I just made one that I thought would work well.)
*/
void int_sort_fast (uint32_t array[], unsigned int elements, int mode) {
	uint32_t *hold_array;
	uint32_t *mod_array=NULL;
	unsigned int i, x, cnt=0;
	unsigned char sizeint=sizeof(uint32_t);


	mode=(!!mode);

	hold_array=calloc((elements+1),sizeint);
	if (mode) {
		mod_array=malloc(elements*sizeint);
		for (i=0;i<elements;i++) mod_array[i]=i;
	}

	for (i=0;i<elements;i++) {
		if (array[i]==0) continue;
		for (x=0;x<cnt+1;x++) {
			if (array[i] > hold_array[x]) {
				if (x<cnt) {
					memmove(hold_array+x+1,hold_array+x,(cnt-x+1)*sizeint);
				}
				hold_array[x]=array[i];
				switch (mode) {
					case 0: break;
					case 1:
						memmove(mod_array+x+1,mod_array+x,(cnt-x+1)*sizeint);
						mod_array[x]=i;
				}
				break;
			}
		}
		cnt++;
	}

	if (mode) {
		memcpy(array,mod_array,elements*sizeint);
		free(mod_array);
	} else {
		memcpy(array,hold_array,elements*sizeint);
	}
	free(hold_array);
}


/*

	Sort really quickly

*/
void int_sort_really_fast (uint32_t array[], unsigned int elements, int mode) {
	uint16_t list_items[0x10000];
/*	uint16_t list_items[0x10000][0x10001]; */
	int i,f=0,m;


	memset(list_items,0,sizeof(list_items));

	for (i=0;i<elements;i++) {
		list_items[array[i]]++;
	}

	for (i=0xffff;i;i--) {
		if (list_items[i]!=0) {
			for (m=0;m<list_items[i];m++) array[f++]=i;
		}
	}

	return;
}
