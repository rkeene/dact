#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "sort.h"

/* void bob_int_sort(uint32_t array[], int size, int mode) {
	uint32_t *mod_array=NULL;
	unsigned int i,j,temp;

	if (mode!=0) {
		mod_array=malloc(size*sizeof(uint32_t));
		for (i=0;i<size;i++) {
			mod_array[i]=i;
		}
	}

	for (i=0;i<size;i++) {
		//
	}

	if (mode!=0) {
		memcpy(array,mod_array,size*sizeof(uint32_t));
		free(mod_array);
	}

	return;
} */


int main(void) {
	uint32_t test[6];
	uint32_t *test_b;
	uint32_t *test_c;
	int mode=0;
	int i,num=sizeof(test)/sizeof(uint32_t);

//	test[0]=4294967294;
//	test[1]=4089;
//	test[2]=597869825;

	test[0]=3;
	test[1]=9;
	test[2]=21;
	test[3]=5;
	test[4]=7;
	test[5]=3;

	test_b=malloc(sizeof(test));
	test_c=malloc(sizeof(test));
	memcpy(test_b,test,sizeof(test));
	memcpy(test_c,test,sizeof(test));

	int_sort_fast(test,num,mode);
	int_sort(test_b,num,mode);
	int_sort_really_fast(test_c,num,mode);

	printf("A: %i", test[0]);
	for (i=1;i<num;i++) printf(", %i",test[i]);
	printf("\n");

	printf("B: %i", test_b[0]);
	for (i=1;i<num;i++) printf(", %i",test_b[i]);
	printf("\n");

	printf("C: %i", test_c[0]);
	for (i=1;i<num;i++) printf(", %i",test_c[i]);
	printf("\n");

	return(0);
}
