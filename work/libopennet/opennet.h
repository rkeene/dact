#ifndef _OPENNET_H 
#define _OPENNET_H
#include <sys/types.h>
#include <unistd.h>


int createlisten(int port);
void closeconnection(int sockfd);
int createconnection_tcp(char *host, int port);
int open_net(const char *pathname, int flags, ...);
off_t lseek_net(int filedes, off_t offset, int whence);
ssize_t read_net(int fd, void *buf, size_t count);
#endif
