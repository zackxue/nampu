
#include <string.h>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "fd_set.h"
#include "talk_api.h"
#include "media_struct.h"
#include "media.h"

void __build_talk_req(struct _talk_req* req)
{
    memset(req, 0, sizeof(talk_req_t));

    req->magic = TALK_MAGIC;
    strcpy(req->user, "admin");
    strcpy(req->psw, "admin");
    req->attr.samples_per_sec = 0;
    req->attr.encode_type     = 1;
    req->attr.audio_channel   = 1;
    req->attr.audio_bits      = 16;
}

int handle_send_req(fd_node_t *_this)
{
    talk_req_t req;
    proxy_talk_req_t proxy_req;
    memset(&proxy_req, 0, sizeof(proxy_req));
    
    __build_talk_req(&req);
    memcpy(&proxy_req.talk_req, &req, sizeof(talk_req_t));
    if (socket_write(_this->fd, (char*)&req, sizeof(proxy_req)) != sizeof(proxy_req))
    {
        return -1;
    }
    return 0;
}

int handle_recv_rsp(fd_node_t *_this)
{

}

int handle_recv_data(fd_node_t *_this)
{

}

int handle_read(fd_node_t *_this)
{
    
}



