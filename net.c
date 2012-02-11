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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 *      email: dact@rkeene.org
 */


/*
 *	Network related functions
 */
#include "dact.h"
#include "net.h"
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <stdarg.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <fcntl.h>

#ifndef NO_NETWORK
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#include "parse.h"
#include "crc.h"

#ifndef O_BINARY
#define O_BINARY 0x0
#endif
#ifndef S_IFSOCK
#define S_IFSOCK 0140000
#endif
#ifndef O_LARGEFILE
#define O_LARGEFILE 0x0
#endif


struct dact_url_info {
	char *url;
	int flags;
	mode_t mode;
};
struct dact_url_info dact_urls[256];

/* Do things required for network access. */
int dact_init_net(void) {
#ifdef _USE_WIN32_  
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 0), &wsaData)!=0) {
		return(-1);                           
	}
	if (wsaData.wVersion!=MAKEWORD(2, 0)) {
		/* Cleanup Winsock stuff */    
		WSACleanup();              
		return(-1);  
	}
#endif
	return(0);
}


/*
 *	Create a listening port on tcp port PORT
 */
int createlisten(int port) {
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

	if (dact_nonetwork) return(-EPERM);

#ifdef HAVE_INET_ATON
	if (!inet_aton(host,&sock.sin_addr)) {
#else
#ifdef HAVE_INET_ADDR
	if ( (sock.sin_addr.s_addr=inet_addr(host) )==-1) {
#else
	{
#endif
#endif
		if ((hostinfo=gethostbyname(host))==NULL) return(-EPERM);
		memcpy(&sock.sin_addr.s_addr,hostinfo->h_addr_list[0],hostinfo->h_length);
	}

	sock.sin_family=AF_INET;
	sock.sin_port=htons(port);
	if ((sockid=socket(AF_INET, SOCK_STREAM, 0))<0) {
		CHECKPOINT;
		return(-EIO);
	}
	if (connect(sockid, (struct sockaddr *) &sock, sizeof(sock))<0) {
		PERROR("connect");
		CHECKPOINT;
		close(sockid);
		return(-EIO);
	}
	CHECKPOINT;
	return(sockid);
}

int open_net(const char *pathname, int flags, mode_t mode) {
	char scheme[5], host[512], file[1024];
	char username[128], password[128];
	char *read_buf, *read_buf_s=NULL;
	char *smallbuf, *tmpbuf;
	int port, fd, x, ftpfd=-1, retrmode=0;

	if (!parse_url(pathname,scheme,username,password,host,&port,file)) {
		if ((fd=createconnection_tcp(host,port))<0) return(-1);

		switch (elfcrc(0, (unsigned char *) scheme, strlen(scheme))) {
			case 457648: /* http */
				if ((flags&O_WRONLY)==O_WRONLY || (flags&O_RDWR)==O_RDWR) {
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
					tmpbuf=mime64((unsigned char *) smallbuf);
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
					return(-ENOENT);
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

				if (dact_urls[fd].url!=NULL) free(dact_urls[fd].url);
				dact_urls[fd].url=strdup(pathname);
				dact_urls[fd].flags=flags;
				dact_urls[fd].mode=mode;
				return(fd);
				break;
			case 28080: /* ftp */
				if ((flags&O_RDWR)==O_RDWR) {
					close(fd);
					return(-1);
				}

				read_buf_s=read_buf=malloc(1024);
				read_buf[0]=0;

				while (1) {
					if ((x=read(fd, read_buf, 1024))<1024) {
						if (x<0) {
							free(read_buf_s);
							return(-EIO);
						}
						/* It's waiting for input... */
					}
					read_buf[x]=0;

					while ((smallbuf=strsep(&read_buf,"\n"))!=NULL) {
						switch (elfcrc(0, (unsigned char *) smallbuf, 4)) {
							case 231456: /* 550 */
								if (ftpfd!=-1) close(ftpfd);
								close(fd);
								free(read_buf_s);
								return(-ENOENT);
								break;
							case 230944: /* 530 */
								if (ftpfd!=-1) close(ftpfd);
								close(fd);
								free(read_buf_s);
								return(-EIO);
								break;
							case 231504: /* 553 */
								if (ftpfd!=-1) close(ftpfd);
								close(fd);
								free(read_buf_s);
								return(-EPERM);
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
								if ((flags&O_RDONLY)==O_RDONLY) write(fd,"RETR ",5);
								if ((flags&O_WRONLY)==O_WRONLY) write(fd,"STOR ",5);
								write(fd, file, strlen(file));
								write(fd, "\n", 1);
								if ((ftpfd=createconnection_tcp(host,port))<0) {
									close(fd);
									return(-ENOENT);
								}
								break;
							case 215072: /* 150 */ 
								if (dact_urls[ftpfd].url!=NULL) free(dact_urls[ftpfd].url);
								dact_urls[ftpfd].url=strdup(pathname);
								dact_urls[ftpfd].flags=flags;
								dact_urls[ftpfd].mode=mode;
								return(ftpfd);
								break;
							case 218656: /* 230 */
								write(fd, "PASV\n", 5);
								break;
							default:
#ifdef DEBUG_FTP
								PRINT_LINE; fprintf(stderr, "dact: Unknown cmd %i (%s)\n",elfcrc(0,smallbuf,4),smallbuf);
#endif
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
		fd=open(pathname,flags|O_BINARY|O_LARGEFILE,mode);
		if (dact_urls[fd].url!=NULL) free(dact_urls[fd].url);
		dact_urls[fd].url=strdup(pathname);
		dact_urls[fd].flags=flags;
		dact_urls[fd].mode=mode;
		return(fd);
	}

	return(-1);
}

off_t lseek_net(int filedes, off_t offset, int whence) {
	struct stat file_status;
	char tmpbuf[1024];
	off_t retval, rset=0, i;
	size_t n;
	ssize_t read_val;
	int newfd;

	fstat(filedes, &file_status);
	if ((file_status.st_mode&S_IFSOCK)==S_IFSOCK) {
		if (whence!=SEEK_CUR || offset<=0) {
			if (whence!=SEEK_SET) return(-1); /* For now ... */
			if (dact_urls[filedes].url==NULL) return(-1);
			if ((newfd=open_net(dact_urls[filedes].url,dact_urls[filedes].flags,dact_urls[filedes].mode))<0) return(-1);
			close(filedes);
			dup2(newfd,filedes);
			if (offset==0) return(0);
		}
	} else {
		retval=lseek(filedes, offset, whence);
		if (!(retval<0 && whence==SEEK_CUR && offset>0)) {
			return(retval);
		}
	}

	retval=0;
	for (i=0; i<offset; i++) {
		n=offset-rset;
		if (n>sizeof(tmpbuf)) n=sizeof(tmpbuf);
		read_val=read(filedes, tmpbuf, n);
		if (read_val<=0) { retval=-1; break; }
		rset+=read_val;
	}
	retval=offset+1;
	return(retval);
}

#else
#ifndef O_BINARY
#define O_BINARY 0x0
#endif
#ifndef O_LARGEFILE
#define O_LARGEFILE 0x0
#endif

int dact_init_net(void) { return(-1); }
int createlisten(int port) { return(-1); }
void closeconnection(int sockfd) { return; }
int createconnection_tcp(char *host, int port) { return(-1); }
int open_net(const char *pathname, int flags, mode_t mode) {
	return(open(pathname,flags|O_BINARY|O_LARGEFILE,mode));
}
off_t lseek_net(int filedes, off_t offset, int whence) {
	char tmpbuf[1024];
	off_t retval, rset=0, i;
	size_t n;
	ssize_t read_val;

	retval=lseek(filedes, offset, whence);
	if (retval<0 && whence==SEEK_CUR && offset>0) {
		retval=0;
		for (i=0; i<offset; i++) {
			n=offset-rset;
			if (n>sizeof(tmpbuf)) n=sizeof(tmpbuf);
			read_val=read(filedes, tmpbuf, n);
			if (read_val<=0) { retval=-1; break; }
			rset+=read_val;
		}
		retval=offset+1;
	}
	return(retval);
}
#endif
