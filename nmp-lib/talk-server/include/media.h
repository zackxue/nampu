
#ifndef __MEDIA_H__
#define __MEDIA_H__

//typedef struct fd_node_s fd_node_t;

int handle_send_req(fd_node_t *_this);
int handle_recv_rsp(fd_node_t *_this);
int handle_recv_data(fd_node_t *_this);
int handle_read(fd_node_t *_this);

#endif
