
/*
 *	Network related functions
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include "conf.h"
#include "opennet.h"

#ifndef NO_NETWORK
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include "parse.h"
#include "crc.h"

extern int errno;
char *opennet_urls[256];

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
	if (connect(sockid, (struct sockaddr *) &sock, sizeof(sock))<0) {
		close(sockid);
		return(-1);
	}
	return(sockid);
}

int open_net(const char *pathname, int flags, ...) {
	char scheme[5], host[512], file[1024];
	char username[128], password[128];
	char *read_buf, *read_buf_s=NULL;
	char *smallbuf, *tmpbuf;
	int port, fd, x, ftpfd=-1, retrmode=0;
	va_list args;
	mode_t mode;

	va_start(args, flags);
	mode = va_arg(args, mode_t);
	va_end(args);

	if (!parse_url(pathname,scheme,username,password,host,&port,file)) {
		if ((fd=createconnection_tcp(host,port))<0) return(-1);

		switch (ELFCRC(0, scheme, strlen(scheme))) {
			case 457648: /* http */
				if ((flags&3)==O_WRONLY || (flags&3)==O_RDWR) {
					close(fd);
					return(-1);
				}
				read_buf_s=read_buf=malloc(50);
				write(fd, "GET ", 4);
				write(fd, file, strlen(file));
				write(fd, " HTTP/1.0\nHost: ",16);
				write(fd, host, strlen(host));
				write(fd, "\n", 1);
				if (strcmp(username, "")) {
					smallbuf=malloc(strlen(password)+strlen(username)+3);
					smallbuf[0]=0;
					strcat(smallbuf,username);
					strcat(smallbuf,":");
					strcat(smallbuf,password);
					tmpbuf=mime64(smallbuf);
					write(fd, "Authorization: Basic ", 21);
					write(fd, tmpbuf, strlen(tmpbuf));
					free(tmpbuf);
					free(smallbuf);
					write(fd, "\n", 1);
				}
				write(fd, "\n", 1);
				x=read(fd, read_buf, 50);
				strsep(&read_buf," ");
				if (strncmp("200 ",read_buf,4) && strncmp("302 ",read_buf,4)) {
					free(read_buf_s);
					close(fd);
					errno=ENOENT;
					return(-1);
				}
				retrmode=read_buf[0];
				read_buf=read_buf_s;
				read_buf[4]=0;
				while (1) {
					read(fd, read_buf, 1);
					if (read_buf[0]==read_buf[2]&&read_buf[0]=='\n') break;
					if (!strncmp(":noi", read_buf, 4) && retrmode=='3') {
						tmpbuf=smallbuf=malloc(512);
						read(fd, read_buf, 1);
						read(fd, smallbuf, 510);
						close(fd);
						return(open_net(strsep(&smallbuf,"\r\n"),flags,mode));
						free(tmpbuf);
					}
					read_buf[3]=read_buf[2];
					read_buf[2]=read_buf[1];
					read_buf[1]=read_buf[0];
				}

				free(read_buf_s);

				if (opennet_urls[fd]!=NULL) free(opennet_urls[fd]);
				opennet_urls[fd]=strdup(pathname);
				return(fd);
				break;
			case 28080: /* ftp */
				if ((flags&3)==O_RDWR) {
					close(fd);
					return(-1);
				}

				read_buf_s=read_buf=malloc(1024);
				read_buf[0]=0;

				while (1) {
					if ((x=read(fd, read_buf, 1024))<1024) {
						if (x<0) {
							free(read_buf_s);
							errno=EIO;
							return(-1);
						}
						/* It's waiting for input... */
					}
					read_buf[x]=0;

					while ((smallbuf=strsep(&read_buf,"\n"))!=NULL) {
						switch (ELFCRC(0, smallbuf, 4)) {
							case 231456: /* 550 */
								if (ftpfd!=-1) close(ftpfd);
								close(fd);
								free(read_buf_s);
								errno=ENOENT;
								return(-1);
								break;
							case 230944: /* 530 */
								if (ftpfd!=-1) close(ftpfd);
								close(fd);
								free(read_buf_s);
								errno=EIO;
								return(-1);
								break;
							case 231504: /* 553 */
								if (ftpfd!=-1) close(ftpfd);
								close(fd);
								free(read_buf_s);
								errno=EPERM;
								return(-1);
								break;
							case 218400: /* 220 */
								write(fd, "USER ", 5);
								if (strcmp(username,"")) {
									write(fd, username, strlen(username));
								} else {
									write(fd, "anonymous", 9);
								}
								write(fd, "\n", 1);
								break;
							case 222768: /* 331 */
								write(fd, "PASS ", 5);
								if (strcmp(password,"")) {
									write(fd, password, strlen(password));
								} else {
									write(fd, "user@host.com", 13);
								}
								write(fd, "\n", 1);
								break;
							case 218512: /* 227 */
								strsep(&smallbuf,"(");
								host[0]=0;
								for (x=0;x<4;x++) {
									strncat(host,strsep(&smallbuf,","),5);
									strcat(host,".");
								}
								host[strlen(host)-1]=0;
								port=(atoi(strsep(&smallbuf,","))<<8)+atoi(strsep(&smallbuf,")\n\r "));
								write(fd, "TYPE I\n", 7);
								break;
							case 217888:
								if ((flags&3)==O_RDONLY) write(fd,"RETR ",5);
								if ((flags&3)==O_WRONLY) write(fd,"STOR ",5);
								write(fd, file, strlen(file));
								write(fd, "\n", 1);
								if ((ftpfd=createconnection_tcp(host,port))<0) {
									close(fd);
									errno=ENOENT;
									return(-1);
								}
								break;
							case 215072: /* 150 */ 
								if (opennet_urls[ftpfd]!=NULL) free(opennet_urls[ftpfd]);
								opennet_urls[ftpfd]=strdup(pathname);
								return(ftpfd);
								break;
							case 218656: /* 230 */
								write(fd, "PASV\n", 5);
								break;
							default:
								break;
						}
					}
					read_buf=read_buf_s;
				}

				
				break;
		}

		free(read_buf_s);
		close(fd);
	} else {
		fd=open(pathname,flags,mode);
		if (opennet_urls[fd]!=NULL) free(opennet_urls[fd]);
		opennet_urls[fd]=strdup(pathname);
		return(fd);
	}

	return(-1);
}

off_t lseek_net(int filedes, off_t offset, int whence) {
	struct stat file_status;
	int newfd;

	fstat(filedes, &file_status);
	if (S_ISSOCK(file_status.st_mode)) {
		if (whence==SEEK_END) return(-1);
		if (offset!=0 || whence!=SEEK_SET) return(-1); /* For now ... */
		if ((newfd=open_net(opennet_urls[filedes],O_RDONLY))<0) return(-1);
		close(filedes);
		dup2(newfd,filedes);
		return(0);
	}
	return(lseek(filedes,offset,whence));
}

#else
int createlisten(int port) { return(-1); }
void closeconnection(int sockfd) { return; }
int createconnection_tcp(char *host, int port) { return(-1); }
int open_net(const char *pathname, int flags,...) {
	va_list args;
	mode_t mode;

	va_start(args, flags);
	mode = va_arg(args, mode_t);
	va_end(args);

	return(open(pathname,flags,mode));
}
off_t lseek_net(int filedes, off_t offset, int whence) {
	return(lseek(filedes,offset,whence));
}
#endif
