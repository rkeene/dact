#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "crc.h"

int main(int argc, char **argv) {
	uint32_t crcval=0;
	int fd;
	char *buf;
	int buflen=1024;
	int x;

	buflen=atoi(argv[1]);
	buf=malloc(buflen);
	fd=open(argv[2], O_RDONLY);
	while (1) {
		x=read(fd, buf, buflen);
		if (x<=0) break;
		crcval=crc(crcval, buf, x);
	}
	printf("adler32=%08x\n", crcval);
	return(0);
}
