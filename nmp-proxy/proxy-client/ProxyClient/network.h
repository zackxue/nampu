
#ifndef __NETWORK_H__
#define __NETWORK_H__

int socket_set_noblock(int fd);
int socket_connect(char *ip, unsigned int port);
int socket_write(int fd, char *buf, int size);
int socket_read(int fd, char *buf, int size);

#endif