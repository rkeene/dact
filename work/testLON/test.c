#include <opennet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(void) {
	int fd,x;
	char smbuf[1024];
	fd=open_net("http://www.rkeene.org/", O_RDONLY);
	printf("fd=%i\n",fd);

	while ((x=read_net(fd,&smbuf,1024) ) >0) {
		write(STDOUT_FILENO, &smbuf,x);
	}
	close(fd);
}
