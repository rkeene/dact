
/*
 *	Network related functions
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <netdb.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include "parse.h"
#include "net.h"


#define REAL_LIBC RTLD_NEXT

extern int errno;

/*
 *	Create a listening port on tcp port PORT
 */
int createlisten(int port)
{
	struct sockaddr_in localname;
	int sockFd;
	sockFd=socket(AF_INET,SOCK_STREAM,IPPROTO_IP);
	localname.sin_family=AF_INET;
	localname.sin_port=htons(port);
	localname.sin_addr.s_addr=INADDR_ANY;
	if (bind(sockFd,(struct sockaddr *) & localname,sizeof(localname))==-1) { perror("bind");return(-1); }
	if (listen(sockFd,1024)==-1) { perror("listen"); return(-1); }
	return(sockFd);
}


/*
 *	Close that socket, yeah.
 */
void closeconnection(int sockfd) {
	shutdown(sockfd, 1);
	close(sockfd);
}

int createconnection_tcp(char *host, int port) {
	int sockid;
	struct hostent *hostinfo;
	struct sockaddr_in sock;
	if ((hostinfo=gethostbyname(host))==NULL) {
#ifdef HAVE_INET_ATON
		if (!inet_aton(host,&sock.sin_addr))
#else
		if ( (sock.sin_addr.s_addr=inet_addr(host) )==-1)
#endif
			return(-1);
	} else {
		memcpy(&sock.sin_addr.s_addr,hostinfo->h_addr_list[0],hostinfo->h_length);
	}
	sock.sin_family=AF_INET;
	sock.sin_port=htons(port);
	if ((sockid=socket(AF_INET, SOCK_STREAM, 0))<0)
		return(-1);
	connect(sockid, (struct sockaddr *) &sock, sizeof(sock));
	return(sockid);
}



int __open(const char *pathname, int flags, ...) {
	static int (*old_open)(const char *, int, mode_t) = NULL;
	uint32_t scheme_val=0;
	va_list args;
	mode_t mode;
	char scheme[5], host[512], file[1024];
	char username[128], password[128];
	char *read_buf, *read_buf_s=NULL;
	int port, fd, x;

	if (!old_open) old_open=(int (*)(const char *, int, mode_t)) dlsym(REAL_LIBC, "__open");

	va_start(args, flags);
	mode = va_arg(args, mode_t);
	va_end(args);

	if (!parse_url(pathname,scheme,username,password,host,&port,file)) {
		if ((fd=createconnection_tcp(host,port))<0) {
			errno=ENOENT;
			return(-1);
		}


		for (x=0;x<strlen(scheme);x++) scheme_val|=(scheme[x]<<(x*8));
		switch (scheme_val) {
			case 1886680168: /* http */

/* No writing via HTTP yet. */
				if (flags==O_WRONLY || flags==O_RDWR) {
					errno=EACCES;
					return(-1);
				}

				read_buf_s=read_buf=malloc(50);
				write(fd, "GET ", 4);
				write(fd, file, strlen(file));
				write(fd, " HTTP/1.1\nHost: ",16);
				write(fd, host, strlen(host));
				write(fd, "\n\n", 2);
				x=read(fd, read_buf, 50);
				strsep(&read_buf," ");
				if (strncmp("200 ",read_buf,4)) {
					free(read_buf_s);
					close(fd);
					errno=ENOENT;
					return(-1);
				}
				read_buf=read_buf_s;
				while (1) {
					read(fd, read_buf, 1);
					if ((read_buf[0]==read_buf[2]&&read_buf[0]=='\n')||(read_buf[0]==read_buf[1]=='\n')) break;
					read_buf[3]=read_buf[2];
					read_buf[2]=read_buf[1];
					read_buf[1]=read_buf[0];
				}

				free(read_buf_s);

				return(fd);
				break;
			case 7369830: /* ftp */
				read_buf_s=read_buf=malloc(50);
				
				break;
		}

		free(read_buf_s);
		close(fd);
	} else {
		return(old_open(pathname,flags,mode));
	}

	errno=ENOTDIR;
	return(-1);
}

int open(const char *pathname, int flags, ...) {
	va_list args;
	mode_t mode;

	va_start(args, flags);
	mode = va_arg(args, mode_t);
	va_end(args);

	return(__open(pathname,flags,mode));
}

int __xstat(int ver, const char *file_name, struct stat *buf) {
	static int (*old_stat)(int, const char *, struct stat *) = NULL;
	uint32_t scheme_val=0;
	char scheme[5], host[512], file[1024];
	char username[128], password[128];
	char *read_buf, *read_buf_s=NULL;
	int port, fd, x;
	
	if (!old_stat) old_stat=(int (*)(int, const char *, struct stat *)) dlsym(REAL_LIBC, "__xstat");

	if (!parse_url(file_name,scheme,username,password,host,&port,file)) {
		if ((fd=createconnection_tcp(host,port))<0) {
			errno=ENOENT;
			return(-1);
		}

		buf->st_dev=0;
		buf->st_ino=0;
		buf->st_mode=0104755;
		buf->st_nlink=1;
		buf->st_uid=0;
//		buf->st_uid=getuid();
		buf->st_gid=getgid();
		buf->st_rdev=0;
		buf->st_size=0;
		buf->st_blksize=512;
		buf->st_blocks=0;
		buf->st_atime=0;
		buf->st_mtime=0;
		buf->st_ctime=0;

		for (x=0;x<strlen(scheme);x++) scheme_val|=(scheme[x]<<(x*8));
		switch (scheme_val) {
			case 1886680168: /* http */
			case 7369830: /* ftp */
		}

		close(fd);
	} else {
		return(old_stat(ver,file_name,buf));
	}
	return(0);

}



int __lxstat(int ver, const char *file_name, struct stat *buf) {
	static int (*old_stat)(int, const char *, struct stat *) = NULL;
	uint32_t scheme_val=0;
	char scheme[5], host[512], file[1024];
	char username[128], password[128];
	char *read_buf, *read_buf_s=NULL;
	int port, fd, x;
	
	if (!old_stat) old_stat=(int (*)(int, const char *, struct stat *)) dlsym(REAL_LIBC, "__lxstat");

	if (!parse_url(file_name,scheme,username,password,host,&port,file)) {
		if ((fd=createconnection_tcp(host,port))<0) {
			errno=ENOENT;
			return(-1);
		}

		buf->st_dev=0;
		buf->st_ino=0;
		buf->st_mode=0104755;
		buf->st_nlink=1;
		buf->st_uid=0;
//		buf->st_uid=getuid();
		buf->st_gid=getgid();
		buf->st_rdev=0;
		buf->st_size=0;
		buf->st_blksize=512;
		buf->st_blocks=0;
		buf->st_atime=0;
		buf->st_mtime=0;
		buf->st_ctime=0;

		for (x=0;x<strlen(scheme);x++) scheme_val|=(scheme[x]<<(x*8));
		switch (scheme_val) {
			case 1886680168: /* http */
			case 7369830: /* ftp */
		}

		close(fd);
	} else {
		return(old_stat(ver,file_name,buf));
	}
	return(0);

}



int access(const char *pathname, int mode) {
	static int (*old_access)(const char *, int) = NULL;
	uint32_t scheme_val=0;
	char scheme[5], host[512], file[1024];
	char username[128], password[128];
	char *read_buf, *read_buf_s=NULL;
	int port, fd, x;
	
	if (!old_access) old_access=(int (*)(const char *, int)) dlsym(REAL_LIBC, "access");

	if (!parse_url(pathname,scheme,username,password,host,&port,file)) {
		if ((fd=createconnection_tcp(host,port))<0) {
			errno=ENOENT;
			return(-1);
		}

		for (x=0;x<strlen(scheme);x++) scheme_val|=(scheme[x]<<(x*8));
		switch (scheme_val) {
			case 1886680168: /* http */
				close(fd);
				if (mode&F_OK || mode&R_OK) { return(0); } else { return(-1); }
			case 7369830: /* ftp */
				close(fd);
				return(-1);
		}

		close(fd);
	} else {
		return(old_access(pathname,mode));
	}
	return(0);

}

