#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char **argv) {
	unsigned int freq[256]={0}, sort[256]={0};
	int fd,i,x,m;
	unsigned char buf[4096];

	for (i=1;i<argc;i++) {
		fd=open(argv[i], O_RDONLY);
		while (x=read(fd, buf, sizeof(buf))) {
			for (m=0;m<x;m++) {
				freq[buf[m]]++;
			}
		}
		close(fd);
		for (m=0;m<256;m++) {
			printf("%3i: %i\n", m, freq[m]);
		}
	}
}
