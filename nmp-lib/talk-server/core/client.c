#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>

#include "macros.h"
#include "log.h"
#include "def.h"
#include "media_struct.h"
#include "watcher.h"
#include "listener.h"
#include "media_set.h"
#include "client.h"
#include "unix_sock.h"

#ifdef AUDIO_TRANSMIT_MODE
talk_handle_t *talk_handle_ref(talk_handle_t *hdl)
{
    //media_node_t *mn = (client_node_t *)hdl;
    //return (talk_handle_t *)client_node_ref(mn);
}

void talk_handle_unref(talk_handle_t *hdl)
{
    //media_node_t *mn = (media_node_t *)hdl;
    //client_node_unref(mn);
}
#else
talk_handle_t *talk_handle_ref(talk_handle_t *hdl)
{
    return (talk_handle_t *)client_ref((client_t *)hdl);
}

void talk_handle_unref(talk_handle_t *hdl)
{
    client_unref((client_t *)hdl);
}
#endif

#ifndef AUDIO_TRANSMIT_MODE
void set_user_data(talk_handle_t *hdl, void *u)
{
    client_t *cli = NULL;
    if (hdl)
    {
        cli = (client_t *)hdl;
        cli->user_data = u;
    }
}

void *get_user_data(talk_handle_t *hdl)
{
    client_t *cli = NULL;
    if (hdl)
    {
        cli = (client_t *)hdl;
        return cli->user_data;
    }
    return NULL;
}

#else

void set_user_data(talk_handle_t *hdl, void *u)
{
    media_node_t *mn = NULL;
    if (hdl)
    {
        mn = (media_node_t *)hdl;
        mn->user_data = u;
    }
}

void *get_user_data(talk_handle_t *hdl)
{
    media_node_t *mn = NULL;
    if (hdl)
    {
        mn = (media_node_t *)hdl;
        return mn->user_data;
    }
    return NULL;
}

#endif

static int __client_recv_req(struct fd_node_s *_this)
{
    //media_info_t mi;
    client_t *cli = (client_t *)_this;
    int ret = socket_read(_this->fd, (char *)(_this->in_buf+_this->in_buf_len)
            , sizeof(proxy_talk_req_t)-_this->in_buf_len);
    if(ret <= 0)
    {
        return -1;
    }
    _this->in_buf_len += ret;
    
    if(_this->in_buf_len == sizeof(proxy_talk_req_t))
    {
        proxy_talk_req_t *req = (proxy_talk_req_t*)_this->in_buf;
        if (ntohl(req->magic) != TALK_MAGIC)
        {
            return -1;
        }
    }
    else
    {
        LOG_I("normal header's size:%d recv header's %d", sizeof(proxy_talk_req_t), _this->in_buf_len);
        return -1;
    }
    
    return 0;
}

static int __client_send_req(struct fd_node_s *_this)
{
    client_t * cli = (client_t *)_this;
    watcher_t *w = (watcher_t *)_this->context;
    media_info_t *mi = &cli->info;
    talk_req_t base_req;
    proxy_talk_req_t req;
    memset(&req, 0, sizeof(proxy_talk_req_t));

    LOG_I("server to send seq pu_id:%s channel:%d", mi->pu_id, mi->channel);
    
    req.magic = htonl(TALK_MAGIC);
    strcpy(req.pu_id, mi->pu_id);
    req.channel = htonl(mi->channel);
    
    memset(&base_req, 0, sizeof(talk_req_t));
    base_req.magic = htonl(TALK_MAGIC);
    strcpy(base_req.user, "admin");
    strcpy(base_req.psw, "admin");
    base_req.res = htonl(cli->response); // ok
    memcpy(&base_req.attr, &mi->attr, sizeof(talk_audio_attr_t));

    memcpy(&req.talk_req, &base_req, sizeof(talk_req_t));
        
    if(socket_write(cli->fd, (char*)&req, sizeof(proxy_talk_req_t)) != sizeof(proxy_talk_req_t))
    {
        return -1;
    }
    
    _this->in_buf_len = 0;
    return 0;
}

int __start_retransmit_data(client_t *cli)
{
    loop_t *loop = GET_OWNER(cli, loop_t);
    LOG_I("NOTICE! start retransmit data");
    fd_set_mod_node2(loop->_set, (fd_node_t *)cli, FD_W_FLAGS | FD_R_FLAGS);

    return 0;
}

int __retransmit_data(client_t *cli)
{
    int send_bytes = 0;
    int result = 0;
    int left = 0;
    int length = 0;

    AQUIRE_LOCK(cli->lock);
    left = cli->backlog_size;
    //RELEASE_LOCK(cli->lock);
    
    for (;;)
    {
        length = (left >= MAX_RESEND_FRAME_SIZE) ? MAX_RESEND_FRAME_SIZE : left;
        result = socket_write(cli->fd, cli->backlog, length);
        left -= result;
        send_bytes += result;
        LOG_I("cli:%p cli->backlog:%p send_bytes:%d left:%d", cli, cli->backlog, send_bytes, left);
        memmove(cli->backlog, cli->backlog + send_bytes, left);

        if (result != length)
        {
            if (errno == EAGAIN || errno == EINTR)
            {LOG_I("cli->backlog_size:%d send_bytes:%d", cli->backlog_size, send_bytes);
                break;
            }
            else
            {//@{will release client}
                LOG_I("retransmit data errno:%d(%s), release client", errno, strerror(errno));
                RELEASE_LOCK(cli->lock);
                return -1;
            }
        }
        else
        {            
            if (left == 0)
            {
                LOG_I("retransmit data over, stop retransmit data");
                __stop_retransmit_data(cli);
                break;
            }
        }
    }

    //AQUIRE_LOCK(cli->lock);
    cli->backlog_size = left;
    RELEASE_LOCK(cli->lock);
    
    return 0;
}

int __stop_retransmit_data(client_t *cli)
{
    loop_t *loop = GET_OWNER(cli, loop_t);

    cli->backlog_size = 0;
    fd_set_mod_node2(loop->_set, (fd_node_t *)cli, FD_R_FLAGS);
    return 0;
}

int __send_talk_data_to_all_client(media_node_t *mn, frame_t *frm)
{
    client_t *c;
    watcher_t *w;
    client_node_t *cn, *cn_next;
    
    list_for_each_entry_safe(cn, cn_next, &mn->cn_list.list, list)
    {
        c = cn->c;
        w = (watcher_t *)(((fd_node_t *)c)->context);
        if (w->stat == STAT_RW)
        {
            if (socket_write(w->fd, (char*)frm, sizeof(talk_frame_hdr_t)+ntohl(frm->frame_length))
                != (sizeof(talk_frame_hdr_t)+ntohl(frm->frame_length)))
            {//@{make sure send a complete frame, otherwise client will read a err frame}
                LOG_I("make sure send a complete frame, otherwise client will read a err frame");
                return -1;
            }
        }
        else if(w->stat == STAT_FIN)
        {
            return -1;
        }
    }

    return 0;
}

int send_talk_data(talk_handle_t *hdl, frame_t *frm)
{
    int send_bytes = 0;
    int length = 0;
#ifdef AUDIO_TRANSMIT_MODE    
    media_node_t *mn = (media_node_t *)hdl;
#else
    client_t *cli = (client_t *)hdl;
    fd_node_t *node = (fd_node_t*)cli;
    watcher_t *w = (watcher_t *)node->context;
#endif
    if (!hdl)
    {
        return -1;
    }
#ifndef AUDIO_TRANSMIT_MODE    
    if (w->stat == STAT_RW)
    {
    #if 1    
        if (socket_write(w->fd, (char*)frm, sizeof(talk_frame_hdr_t)+ntohl(frm->frame_length))
            != (sizeof(talk_frame_hdr_t)+ntohl(frm->frame_length)))
        {//@{make sure send a complete frame, otherwise client will read a err frame}
            LOG_I("make sure send a complete frame, otherwise client will read a err frame");
            return -1;
        }
    #else        
        AQUIRE_LOCK(cli->lock);

        length = sizeof(talk_frame_hdr_t) + ntohl(frm->frame_length);
        if (cli->backlog_size > 0)
        {            
            if (cli->backlog_size + length < MAX_BACKLOG_SIZE)
            {
                memcpy(cli->backlog + cli->backlog_size, (char*)frm, length);
                cli->backlog_size += length;                
            }
            else
            {//@{length of retransmit is not enough filled with this frm}
                LOG_I("left length(%d) of retransmit buffer(filled(%d)) is not enough filled with this frm(%d)", 
                    MAX_BACKLOG_SIZE - cli->backlog_size, cli->backlog_size, length);
                
                RELEASE_LOCK(cli->lock);
                return -1;
            }
        }
        else
        {   
            if((send_bytes = socket_write(w->fd, (char*)frm, length)) != length)        
            {
                if (errno == EAGAIN || errno == EINTR)
                // start retransmit
                {
                    memcpy(cli->backlog, (char*)frm + send_bytes, length - send_bytes);
                    cli->backlog_size += length - send_bytes;

                    __start_retransmit_data(cli);
                }                
                else
                {
                    LOG_I("failed to socket_write data errno:%d", errno);
                    return -1;
                }
            }
        }

        RELEASE_LOCK(cli->lock);
        
    #endif
    }
    else if(w->stat == STAT_FIN)
    {
        return -1;
    }
#else
    //__send_talk_data_to_all_client(mn, frm);

#endif
    return 0;
}

static int __client_recv_data(struct fd_node_s * _this)
{
    int ret;

    watcher_t* w = (watcher_t*)_this->context;
    
    switch(w->recv_stat)
    {
        case RECV_HDR:
            {
                ret = socket_read(_this->fd, (char *)(_this->in_buf+_this->in_buf_len)
                        , sizeof(talk_frame_hdr_t)-_this->in_buf_len);
                if(ret <= 0)
                {
                    LOG_I("RECV_HDR sock_read ret:%d", ret);
                    return -1;
                }
                _this->in_buf_len += ret;
                if(_this->in_buf_len == sizeof(talk_frame_hdr_t))
                {
                    w->recv_stat = RECV_DATA;
                }
            }
            break;
        case RECV_DATA:
            {
                talk_frame_hdr_t *hdr = (talk_frame_hdr_t*)_this->in_buf;
                //LOG_I("hdr:%p, frm => u32Magic:0x%x, u32FrameNo:%d, u32Pts:%d, u32Len:%d\n"						
                    //, hdr, ntohl(hdr->magic), ntohl(hdr->frame_num), ntohl(hdr->pts), ntohl(hdr->frame_length));

                ret = socket_read(_this->fd, (char *)(_this->in_buf+_this->in_buf_len)
                        , ntohl(hdr->frame_length) - (_this->in_buf_len-sizeof(talk_frame_hdr_t)));
                if(ret <= 0)
                {
                    LOG_I("RECV_DATA sock_read ret:%d", ret);
                    return -1;
                }
                _this->in_buf_len += ret;
                if(_this->in_buf_len == (sizeof(talk_frame_hdr_t)+ntohl(hdr->frame_length)))
                {
                    w->recv_stat = RECV_HDR;
                    return 1;
                }
            }
            break;
    }
    return 0;
}

static int __watcher_read(struct fd_node_s *_this)
{
    int ret;
    client_node_t *cn;
    media_node_t *mn;
    media_info_t mi;
    proxy_talk_req_t *req = NULL;
    client_t *cli = (client_t *)_this;
    watcher_t* w = (watcher_t *)_this->context;
    loop_t *loop = GET_OWNER(cli, loop_t);
    
    if ((_this->revent & FD_T_FLAGS) && (w->stat != STAT_FIN) 
        && (w->stat != STAT_DEAD))
    //if((_this->revent & FD_T_FLAGS) && (w->stat != STAT_DEAD))//2.BUG2,TIMEOUT会被触发两次，导致[STAT_FIN]执行两次
    {//@{超时，也需要调用close,通知设备关闭发送线程}
        LOG_I("STAT_TIMEOUT __watcher_read w->stat:%d fd:%d", w->stat, cli->fd);
        goto _error;
    }

    switch(w->stat)
    {
        case STAT_REQUEST:
            LOG_I("STAT_REQUEST __watcher_read w->stat:%d fd:%d", w->stat, cli->fd);
            if(_this->revent & FD_R_FLAGS)
            {
                ret = __client_recv_req(_this);
                if (ret < 0)
                {
                    LOG_I("__client_recv_req ret:%d", ret);
                    goto _error;
                }
                
                req = (proxy_talk_req_t*)_this->in_buf;
                if (ntohl(req->magic) != TALK_MAGIC)
                {
                    LOG_I("req->magic", ret);
                    goto _error;
                }
                
                if (cli->parm.open)
                {
                    memset(&mi, 0, sizeof(mi));
                    memcpy(&mi.attr, &req->talk_req.attr, sizeof(talk_audio_attr_t));
                    memcpy(&mi.pu_id, req->pu_id, sizeof(req->pu_id));
                    mi.channel = ntohl(req->channel);
#ifdef AUDIO_TRANSMIT_MODE                    
                    // 在打开之前，先判断是否已经打开过了
                    mn = get_media_node(req->pu_id, ntohl(req->channel));
                    if (media_node_get_count(mn) > 0)
                    {
                        LOG_I("pu_id:%s channel:%d have already opened", req->pu_id, ntohl(req->channel));
                        cli->response = 0;
                    }
                    else
#endif
                    {
#ifdef AUDIO_TRANSMIT_MODE                    
                        ret = cli->parm.open((void *)mn, req->pu_id, ntohl(req->channel), &mi);
#else
                        ret = cli->parm.open((void *)cli, req->pu_id, ntohl(req->channel), &mi);
#endif
                        if (ret < 0)
                        {
                            cli->response = -1;
                            LOG_I("[err:%d] failed to open cli:%p pu_id:%s channel:%d", ret, cli, req->pu_id, ntohl(req->channel));
                            //goto _error; // 打开失败时，不关闭连接，响应客户端
                        }
                        else
                            LOG_I("[ret:%d] succeed to open cli:%p pu_id:%s channel:%d", ret, cli, req->pu_id, ntohl(req->channel));
                    }
#ifdef AUDIO_TRANSMIT_MODE
                    if (cli->response == 0) // 成功
                    {
                        // 只有打开后，才将client_node 加入media_node中去
                        //mn = get_media_node(req->pu_id, ntohl(req->channel));
                        SET_OWNER((client_node_t *)client_get_node(cli), mn);
                        media_node_add_client(mn, (client_node_t *)client_get_node(cli));
                    }
#endif               
                    _this->in_buf_len = 0; // clear buf
                    //cli->info = strdup(&mi);
                    memcpy(&cli->info, &mi, sizeof(media_info_t));
                    fd_set_mod_node2(loop->_set, _this, FD_W_FLAGS);
                    w->stat = STAT_RESPONSE;
                }
            }
            break;
        case STAT_RESPONSE:
            LOG_I("STAT_RESPONSE __watcher_read w->stat:%d fd:%d", w->stat, cli->fd);
            if (_this->revent & FD_W_FLAGS)
            {
                ret = __client_send_req(_this);
                if (ret < 0)
                {
                    LOG_I("__client_send_req ret:%d", ret);

                    goto _error;
                }
                else
                {
                    fd_set_mod_node2(loop->_set, _this, FD_R_FLAGS);
                    w->stat = STAT_RW;
                }
            }
            break;
        case STAT_RW:
            if (_this->revent & FD_R_FLAGS)
            {
                ret = __client_recv_data(_this);                
                if (ret < 0)
                {
                    // [原因] a. 客户端断开连接，触发__watch_read, 接收一个[FIN, ACK], 大小为0的包FIN包;
                    LOG_I("__client_recv_data ret:%d", ret);

                    goto _error;
                }
                else if (ret == 1)
                {   
                    if (cli->parm.recv)
                    {   
#ifdef AUDIO_TRANSMIT_MODE
                        cli = (client_t *)_this;
                        mn = GET_OWNER(((client_node_t *)cli->client_node), media_node_t);
                        cli->parm.recv((void *)mn, (char *)(talk_frame_hdr_t*)_this->in_buf, (int)(talk_frame_hdr_t*)_this->in_buf_len);
#else
                        cli->parm.recv((void *)_this, (char *)(talk_frame_hdr_t*)_this->in_buf, (int)(talk_frame_hdr_t*)_this->in_buf_len);
#endif
                    }
                    //clear buf;
                    _this->in_buf_len = 0;
                }
            }

            if (_this->revent & FD_W_FLAGS)
            {//@{for retransmit data}
                ret = __retransmit_data(cli);
                if (ret < 0)
                {
                    goto _error;
                }
            }
            break;
        case STAT_FIN:
            LOG_I("[STAT_FIN] __watcher_read w->stat:%d fd:%d _this->revent:%d", w->stat, cli->fd, _this->revent);
            {// release client
                LOG_I("release client cli_fd:%d ref_count:%d", cli->fd, cli->ref_count);
                
                w->stat = STAT_DEAD;
#ifdef AUDIO_TRANSMIT_MODE              
                client_node_unref((client_node_t *)client_get_node(cli));
#else
                client_unref(cli);
#endif
            }
            // [BUG] 当open失败时，client_unref(cli)已经释放了cli, 也释放了cli中的watcher, 此时再赋值
            //       就崩溃了。
            // [问题] 正常时，为什么不会崩溃?
            // [原因] 在该测试程序中, 因为发送数据的线程使用了client_ref, 拥有了cli的释放权，
            //        当close时，发送线程并不会即刻退出，

            // w->stat = STAT_DEAD;
            break;
        case STAT_DEAD:
            {                
                 //LOG_I("------------------------ STAT_DEAD --------------------");
            }
            break;      
    }
    return 0;
    
_error:
    // 异常时: 如果open失败，返回-1,则不会调用close函数，w->stat = STAT_REQUEST变为w->stat = STAT_FIN,
    // cli会在客户端断开链接时, 触发可读事件，__watch_read, 然后release client。
    // 正常时: a. 客户端断开连接, __watch_read触发，收到FIN包，执行close, w->stat设为STAT_FIN, 为什么
    // __watch_read还会触发，(除了TIMEOUT外，实在想不到还有别的原因), 原因可能是TCP断开时的四次挥手造
    // 成的，client第一次挥手时，发送了[FIN], server接收后，发送[ACK]，然后发送[FIN], client最后[ACK]
    // 连接最终断开。b. 实际上，select就不停地触发，这时读fd的errno是RST错误。
    //                       四次挥手
    //
    //            S                             C
    //            S<----------[FIN]-------------C  
    //            S-----------[ACK]-------------C
    //            S-----------[FIN]-------------C
    //            S<----------[ACK]-------------C
    //
    if(cli->parm.close)
    {
        if (w->stat >= STAT_RESPONSE && cli->response == 0)
        {
            LOG_I("close send data thread _this->revent:%d", _this->revent);
#ifdef AUDIO_TRANSMIT_MODE
            cli = (client_t *)_this;
            mn = GET_OWNER(((client_node_t *)cli->client_node), media_node_t);
            if (media_node_get_count(mn) == 1)
            {
                cli->parm.close((void *)mn);
            }
#else
            cli->parm.close((void *)cli);
#endif
        }
    }
    w->stat = STAT_FIN;
    return -1;
}

client_t *client_new(int fd, talk_ops_t *ops, loop_t *loop)
{
    client_t *cli= NULL;    
    watcher_t *w = NULL;
    LOG_I("in client_new fd:%d", fd);
    w = watcher_new(fd, (void *)ops);
    if (!w)
    {
        LOG_I("failed to watcher_new");
        return NULL;
    }

    cli = (client_t *)fd_node_new(sizeof(client_t), w->fd, __watcher_read, w);
    if(!cli)
    {
        LOG_I("failed to fd_node_new");
        goto _error;
    }

    LOG_I("assign client:%p for client sock:%d", cli, fd);
    
    cli->ref_count = 1;
    cli->fd = fd;
    cli->parm = *ops;
    cli->backlog_size = 0;
    cli->lock = LOCK_NEW();
    cli->response = 0;                 // default response ok
    
    SET_OWNER(cli, (void*)loop);       // set listen's loop
    LOOP_ADD_WEIGHT(loop);             // loop->weight add 1

    w->stat = STAT_REQUEST;
    
    if(gettid() == loop->set_tid) 
    {
        fd_set_add_node3(loop->_set, (fd_node_t *)cli, FD_R_FLAGS, w->timeout);
    }
    else
    {

#ifdef WIN32
        EnterCriticalSection(&loop->set_mutex);
#else
		pthread_mutex_lock(&loop->set_mutex);
#endif
        
        fd_set_add_node3(loop->_set, (fd_node_t *)cli, FD_R_FLAGS, w->timeout);
#ifdef WIN32
		LeaveCriticalSection(&loop->set_mutex);	
#else
        pthread_mutex_unlock(&loop->set_mutex);
#endif
    }
    
    return cli;
    
_error:
    if (cli)
    {
        client_unref(cli);
    }
    if(w)
    {
        watcher_del(w);
    }
  
    return NULL;

}

void client_free(client_t *cli)
{
    watcher_t *w;
    fd_node_t *node = NULL;
    loop_t *loop = GET_OWNER(cli, loop_t);
    LOOP_DEC_WEIGHT(loop);                // loop->weigth dec 1

    //media_node_del_client(media_node_t * mn,client_node_t * cn);
    
    if (!cli)
    {
        return;
    }
    
    node = (fd_node_t*)cli;
   
    if(gettid() == loop->set_tid)
    {
        fd_set_del_node2(loop->_set, node); 
    }
    else
    {
#ifdef WIN32
        EnterCriticalSection(&loop->set_mutex);
#else
        pthread_mutex_lock(&loop->set_mutex);
#endif
        fd_set_del_node2(loop->_set, node);    
#ifdef WIN32
        LeaveCriticalSection(&loop->set_mutex); 
#else
        pthread_mutex_unlock(&loop->set_mutex);
#endif
    }
#if 1
    if (w = (watcher_t *)node->context)
    {
        watcher_del(w);
    }
#endif
    if (cli->lock)
    {
        LOCK_DEL(cli->lock);
    }
#if 0 // cli->fd 与watcher中fd是同一个，当watcher_del关闭fd后,再close(cli->fd),
      // 可能导致关闭了新创建的fd, 从而引发select中(-9)bad file descriptor的错误 
    if (cli->fd > 0)
    {
        close(cli->fd);
        cli->fd = -1;
    }
#endif
    free(cli);
}

void client_finalize(client_t *cli)
{//@{fixme}
    LOG_I("del client:0x%p fd:%d", cli, cli->fd);
    client_free(cli);
}

client_t *client_ref(client_t *cli)
{
    __OBJECT_REF(cli);    
}

void client_unref(client_t *cli)
{
    LOG_I("client_unref cli:0x%p fd:%d", cli, cli->fd);
    __OBJECT_UNREF(cli, client);
}

client_t *client_set_node(client_t *c, void *cn)
{
    c->client_node = cn;
    return c;
}

void *client_get_node(client_t *c)
{
    return c->client_node;
}

client_node_t *__client_node_alloc(client_t *c)
{
    client_node_t *cn;

    cn = (client_node_t *)calloc(1, sizeof(client_node_t));
    if (!cn)
    {
        return NULL;
    }

    cn->c = c;
    cn->ref_count = 1;
    INIT_LIST_HEAD(&cn->list);
    
    return cn;
}

client_node_t *client_node_new(int fd, talk_ops_t *ops, loop_t *loop)
{
    client_node_t *cn;
    client_t *cli;
    
    if (!ops || !loop)
    {
        goto _error;
    }

    cli = client_new(fd, ops, loop);
    if (!cli)
    {
        goto _error;
    }

    cn = __client_node_alloc(cli);
    if (!cn)
    {
        goto _error;
    }

    client_set_node(cli, cn);
    return cn;
    
_error:
    if (cli)
    {
        client_unref(cli);
    }
    
    return NULL;
}

void client_node_del(client_node_t * cn)
{
    if (!cn)
    {
        return;
    }

    if (cn->c)
    {
        client_unref(cn->c);
    }
    
    free(cn);
}

void __client_node_finalize(client_node_t *cn)
{
    media_node_del_client(GET_OWNER(cn, media_node_t), cn);
    client_node_del(cn);
}

client_node_t *client_node_ref(client_node_t *cn)
{
    __OBJECT_REF(cn); 
}

void client_node_unref(client_node_t *cn)
{
    __OBJECT_UNREF(cn, __client_node);
}

