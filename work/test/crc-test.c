#define DEBUG 1
#include <stdio.h>
#include "crc.h"

int main(void) {
	unsigned long crc0=0,crc1=0;
	crc0=ELFCRC(crc0,"joe", 3);
	crc0=ELFCRC(crc0,"bob", 3);
	crc1=ELFCRC(crc1,"joeboa", 6);
	SPOTVAR_NUM(crc0);
	SPOTVAR_NUM(crc1);
	return(0);
}
