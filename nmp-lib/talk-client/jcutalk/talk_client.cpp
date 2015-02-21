
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "log.h"
#include "tinydump.h"
#include "macros.h"
#include "fd_set.h"
#include "sched.h"
#include "conn.h"
#include "hi_talk_cli.h"
#include "jcutalk.h"
#include "talk_client.h"

#ifdef WIN32  
typedef int socklen_t;  
typedef int ssize_t;  
#endif 

typedef enum _RECV_STAT {
	RECV_HDR        = 0x00
	,RECV_DATA       = 0x01
}RECV_STAT_E;

static int socket_connect(int fd, char *ip, unsigned short port);
static int socket_set_noblock(int fd);
static int socket_write(int fd, char *buf, int size);
static int socket_read(int fd, char *buf, int size);

static int conn_send_req(struct fd_node_s *_this);
static int conn_recv_rsp(struct fd_node_s *_this);
static int conn_recv_data(struct fd_node_s *_this);
static int conn_read(struct fd_node_s *_this);

void set_user_data(talk_hdl *hdl, void *u)
{
	talk_client_t *cli = (talk_client_t *)hdl;

	if (!hdl)
	{
		return ;
	}

	cli->u = u;
}

void* get_user_data(talk_hdl *hdl)
{
	talk_client_t *cli = (talk_client_t *)hdl;

	if (!hdl)
	{
		return NULL;
	}

	return cli->u;
}

void talk_cli_ref(talk_hdl *hdl)
{
	talk_client_t *cli = (talk_client_t *)hdl;
	
	if (!hdl)
	{
		return ;
	}

	client_ref(cli);
}

void talk_cli_unref(talk_hdl *hdl)
{
	talk_client_t *cli = (talk_client_t *)hdl;
	
	if (!hdl)
	{
		return ;
	}

	client_unref(cli);
}

static inline int __get_device_type(char *pu_id)
{
	int dev_type = 0;
	if (!pu_id)
	{
		return dev_type;
	}

	if (!strncmp(pu_id, TYPE_JXJ, strlen(TYPE_JXJ)))
	{
		dev_type = JXJ;
	}
	else if (!strncmp(pu_id, TYPE_HIK, strlen(TYPE_HIK))
		|| !strncmp(pu_id, TYPE_DAH, strlen(TYPE_DAH)))
	{
		dev_type = PROXY;
	}
	else
	{
		dev_type = PF;
	}

	return dev_type;
}

int jcu_talk_cli_init(jcu_talk_parm_t *parm)
{
	LOG("jcu_talk_cli_init\n");

	sched_t *sched = NULL;
	static int init = 0;
	if (!init)
	{//@{not thread safe}
		init = 1;
		//td_init(NULL, 1, "zoucheng2006@126.com");
		return hi_talk_cli_init((talk_parm_t *)parm);
	}
	return -1;
}

int jcu_talk_cli_uninit()
{
	return hi_talk_cli_uninit();
}

talk_hdl*jcu_talk_cli_open(char *ip, unsigned short port, void *ctx, media_info_t *mi)
{
	LOG("jcu_talk_cli_open ip:%s port:%d\n", ip, port);

	if (!ip || !port || !ctx || !mi)
	{
		assert(-1);
		return NULL;
	}

	if (!__get_device_type((char *)mi->pu_id))
	{
		assert(-1);
		return NULL;
	}

	return hi_talk_cli_open(ip, port, ctx, mi);
}

int jcu_talk_cli_close(talk_hdl *hdl)
{
	if (!hdl)
	{
		assert(-1);
		return -1;
	}
	
	LOG("jcu_talk_cli_close hdl:%x\n", hdl);
	return hi_talk_cli_close(hdl);
}

int jcu_talk_cli_send(talk_hdl *hdl, talk_frame_hdr_t *frm)
{
	if (!hdl || !frm)
	{
		assert(-1);
		return -1;
	}
	return hi_talk_cli_send(hdl, (HI_TALK_FRMAE_HDR *)frm);
}

int conn_read(struct fd_node_s *_this)
{
    int ret;
	talk_client_t *cli = (talk_client_t *)_this;
    conn_t *conn = (conn_t*)_this->context;
	loop_t *loop = GET_OWNER(cli, loop_t);
	int dev_type = cli->dev_type;
	/*
	 * 每一个watch创建以后,每带有一个超时时间,超时将会调用stop
	 * 超时出现的情况:
	 * 1.
	 *
	 */
    if((_this->revent & FD_T_FLAGS) && (conn->stat != CONN_FIN)) 
    {
        printf("%s => %d\n", __FUNCTION__, __LINE__);
        if(cli->parm.stop) cli->parm.stop(_this, conn->ctx);
        conn->stat = CONN_FIN;
        return 0;
    }
	
    switch(conn->stat)
    {
        case CONN_CONNECTING:
            if(_this->revent & FD_W_FLAGS)
            {
                ret = conn_send_req(_this);
                if(ret < 0)
                {
//                    printf("%s => %d\n", __FUNCTION__, __LINE__);
                    if(cli->parm.stop) cli->parm.stop(_this, conn->ctx);
                    conn->stat = CONN_FIN;
                }
                else
                {
                    fd_set_mod_node2(loop->_set, _this, FD_R_FLAGS);
                    conn->stat = CONN_WAIT;
                }
            }
            break;
        case CONN_WAIT:
            if(_this->revent & FD_R_FLAGS)
            {
                ret = conn_recv_rsp(_this);
                if(ret < 0)
                {
   //                 printf("%s => %d\n", __FUNCTION__, __LINE__);
                    if(cli->parm.stop) cli->parm.stop(_this, conn->ctx);
                    conn->stat = CONN_FIN;
                }
                else if(ret == 1)
                {
                    if(cli->parm.start) cli->parm.start(_this, conn->ctx);
                    //clear buf;
                    _this->in_buf_len = 0;
                    conn->stat = CONN_RW;
                }
            }
            break;
        case CONN_RW:
            if(_this->revent & FD_R_FLAGS)
            {
                ret = conn_recv_data(_this);
                if(ret < 0)
                {
                    printf("%s => %d\n", __FUNCTION__, __LINE__);
                    if(cli->parm.stop) cli->parm.stop(_this, conn->ctx);
                    conn->stat = CONN_FIN;
                }
                else if(ret == 1)
                {
					if (dev_type == JXJ)
					{
						if(cli->parm.recv) cli->parm.recv(_this, (HI_TALK_FRMAE_HDR*)_this->in_buf, conn->ctx);
					}
					else if (dev_type == PROXY)
					{
						HI_TALK_FRMAE_HDR *hdr = (HI_TALK_FRMAE_HDR *)_this->in_buf;
						hdr->u32Magic = ntohl(hdr->u32Magic);
						hdr->u32FrameNo = ntohl(hdr->u32FrameNo);
						hdr->u32Len = ntohl(hdr->u32Len);
						hdr->u32Pts = ntohl(hdr->u32Pts);

						if(cli->parm.recv) cli->parm.recv(_this, (HI_TALK_FRMAE_HDR*)_this->in_buf, conn->ctx);
					}
					else
					{// 不可能执行到这里
						assert(-1);
					}
					//clear buf;
                    _this->in_buf_len = 0;
                }
            }
            break;
        case CONN_FIN:
			//@{处理select 的水平触发, 否则一旦服务器断开连接, select会一直触发}
			Sleep(20);
			break;
    }
    return 0;
}

void client_finalize(talk_client_t *cli)
{//@{fixme}
	LOG("del client:0x%p fd:%d\n", cli, cli->fd);
	client_del(cli);
}

talk_client_t *client_ref(talk_client_t *cli)
{
	__OBJECT_REF(cli);    
}

void client_unref(talk_client_t *cli)
{
	LOG("client_unref cli:0x%p fd:%d\n", cli, cli->fd);
	__OBJECT_UNREF(cli, client);
}

extern talk_parm_t *__get_talk_ops();
talk_client_t* client_new(const char *ip, int port, media_info_t *mi, void *ctx, talk_parm_t *ops, loop_t *loop)
{
#if 0
	talk_client_t *cli = NULL;
	
	cli = (talk_client_t *)fd_node_new(sizeof(talk_client_t), fd, NULL, NULL);
#else
	int ret;
	sched_t *sched = NULL;
	conn_t *conn = NULL;
	talk_client_t *client = NULL;
	fd_node_t *node = NULL;

    conn = conn_new(ctx);
    if(!conn) return NULL;
    
	/*
	 * socket_conect
     *
	 * @describe 连接设备
	 * @toserver 创建服务器的socket并监听 
	 *
	 * @date 2013-4-28
	 */
	ret = socket_connect(conn->fd, (char *)ip, port);

#ifdef WIN32
	if (ret < 0 && ret != WSAEWOULDBLOCK)
#else
    if(ret < 0 && ret != -EINPROGRESS)
#endif
    {
        goto _error;
    }
#if 0   
    node = fd_node_new(conn->fd, conn_read, conn);
    if(!node)
    {
        goto _error;
    }
#else
	client = (talk_client_t *)fd_node_new(sizeof(talk_client_t), conn->fd, conn_read, conn);
	if (!client)
	{
		goto _error;
	}
    
	//@{}
	SET_OWNER(client, loop);
	client->ref_count = 1;
	client->fd = conn->fd;
	client->parm = *(__get_talk_ops());
	LOG("new client:%x fd:%d\n", client, client->fd);

	LOG("set client client's media_info pu_id:%s channel:%d audio samples_per_sec:%d encode_type:%d audio_bits:%d audio_channel:%d\n", 
		mi->pu_id, 
		mi->channel, 
		mi->attr.samples_per_sec, 
		mi->attr.encode_type, 
		mi->attr.audio_bits, 
		mi->attr.audio_channel);

	set_device_type(client, (char *)mi->pu_id);
	LOG("enter set_media_info client:%x mi:%x\n", client, mi);
	set_media_info(client, mi);
	LOG("out set_media_info client:%x mi:%x\n", client, mi);
	//LOOP_ADD_WEIGHT(loop);  /* loop->weight 对多线程来讲是全局的 */
#endif
    conn->stat = CONN_CONNECTING;
    
    if (gettid() == loop->set_tid) //如果是同一个线程的话, 不加锁
    {
        //fd_set_add_node3(talk->_set, node, FD_W_FLAGS, conn->timeout);
		LOOP_ADD_WEIGHT(loop);  /* loop->weight 对多线程来讲是全局的 */
		fd_set_add_node3(loop->_set, (fd_node_t *)client, FD_W_FLAGS, conn->timeout);
    }
    else
    {
#ifdef WIN32
		EnterCriticalSection(&loop->set_mutex);
#else
		pthread_mutex_lock(&loop->set_mutex);
#endif
        LOOP_ADD_WEIGHT(loop);  /* loop->weight 对多线程来讲是全局的 */
        //fd_set_add_node3(loop->_set, node, FD_W_FLAGS, conn->timeout);
		fd_set_add_node3(loop->_set, (fd_node_t *)client, FD_W_FLAGS, conn->timeout);
#ifdef WIN32
		LeaveCriticalSection(&loop->set_mutex);	
#else
        pthread_mutex_unlock(&loop->set_mutex);
#endif
    }
#if 0    
    return node;
#else
	return client;
#endif
_error:
    if(conn) conn_del(conn);
    //if(node) fd_node_del(node);
    return NULL;
#endif
	return client;
}

int client_del(talk_client_t *cli)
{
	loop_t *loop = NULL;
	fd_node_t *node = (fd_node_t*)cli;

	loop = GET_OWNER(cli, loop_t);
	//if((!hdl) || (!talk) || (!talk->set_tid)) return -1;

	if (gettid() == loop->set_tid)
	{
		LOOP_DEC_WEIGHT(loop);
		fd_set_del_node2(loop->_set, node); 
	}
	else
	{
#ifdef WIN32
		EnterCriticalSection(&loop->set_mutex);
#else
		pthread_mutex_lock(&loop->set_mutex);
#endif
		LOOP_DEC_WEIGHT(loop);
		fd_set_del_node2(loop->_set, node);    
#ifdef WIN32
		LeaveCriticalSection(&loop->set_mutex);	
#else
		pthread_mutex_unlock(&loop->set_mutex);
#endif
	}

	if(node->context) conn_del((conn_t*)node->context);

	fd_node_del(node);

	return 0;
}

int set_device_type(talk_client_t *cli, char *pu_id)
{
	if (!cli || !pu_id)
	{
		assert(-1);
		return -1;
	}

	return (cli->dev_type = __get_device_type(pu_id)) ? 0 : -1;
}

void set_media_info(talk_client_t *cli, media_info_t *info)
{
	LOG(">>>>>> 11111111111111111111111\n");
	if (!cli || !info)
	{
		assert(-1);
		return ;
	}

	LOG(">>>>>> 22222222222222222222222\n");
	memcpy(&cli->mi, info, sizeof(media_info_t));

	LOG(">>>>>> 33333333333333333333333\n");
}

/*
 * -1: error; 0: none; 1: OK;
 */
static int conn_send_req(struct fd_node_s *_this)
{
	talk_client_t *client = (talk_client_t *)_this;

	if (client->dev_type == JXJ)
	{
		HI_TALK_REQ req;
		memset(&req, 0, sizeof(HI_TALK_REQ));
	    
		req.u32Magic = HI_TALK_MAGIC;
		strcpy(req.szUsr, "admin");
		strcpy(req.szPsw, "admin");
#if 0		
		req.stAttr.u8AudioSamples = 0;
		req.stAttr.u8EncodeType   = 1;
		req.stAttr.u8AudioChannels= 1;
		req.stAttr.u8AudioBits    = 16;
#else
		memcpy(&req.stAttr, &client->mi.attr, sizeof(HI_TALK_AUDIO_ATTR));
#endif
		if(socket_write(_this->fd, (char*)&req, sizeof(HI_TALK_REQ)) != sizeof(HI_TALK_REQ))
		{
			return -1;
		}
	}
	else if (client->dev_type == PROXY)
	{
		HI_TALK_REQ base_req;

		PROXY_TALK_REQ_T proxy_req;
		memset(&proxy_req, 0, sizeof(PROXY_TALK_REQ_T));

		proxy_req.u32Magic = htonl(HI_TALK_MAGIC);
#if 0
		memcpy(proxy_req.szPuId, "DAH-DVR-20000000", sizeof("DAH-DVR-20000000"));
		proxy_req.channel = htonl(0);
#else
		memcpy(proxy_req.szPuId, client->mi.pu_id, sizeof(client->mi.pu_id));
		proxy_req.channel = htonl(client->mi.channel);
#endif
		memset(&base_req, 0, sizeof(HI_TALK_REQ));
		strcpy(base_req.szUsr, "admin");
		strcpy(base_req.szPsw, "admin");
#if 0
		base_req.stAttr.u8AudioSamples = 0;
		base_req.stAttr.u8EncodeType   = 1;
		base_req.stAttr.u8AudioChannels= 1;
		base_req.stAttr.u8AudioBits    = 16;
#else
		memcpy(&base_req.stAttr, &client->mi.attr, sizeof(HI_TALK_AUDIO_ATTR));
#endif
		memcpy(&proxy_req.stTalkReq, &base_req, sizeof(HI_TALK_REQ)); 
		if(socket_write(_this->fd, (char*)&proxy_req, sizeof(PROXY_TALK_REQ_T)) != sizeof(PROXY_TALK_REQ_T))
		{
			return -1;
		}
	}
	else 
	{
		return -1;
	}

    return 0;
}
static int conn_recv_rsp(struct fd_node_s *_this)
{
	talk_client_t *client = (talk_client_t *)_this;

	if (client->dev_type == JXJ)
	{// jxj 设备非网络字节顺序
		int ret = socket_read(_this->fd, (char *)(_this->in_buf+_this->in_buf_len)
				, sizeof(HI_TALK_REQ)-_this->in_buf_len);
		if(ret <= 0)
		{
			return -1;
		}
		_this->in_buf_len += ret;
	    
		if(_this->in_buf_len == sizeof(HI_TALK_REQ))
		{
			HI_TALK_REQ *req = (HI_TALK_REQ*)_this->in_buf;
			if (req->u32Magic == HI_TALK_MAGIC)
			{
				if(req->s32Res == 0)
				{
					LOG("[INFO] JXJ device talk server response ok\n");
					return 1;
				}
				else
				{
					LOG("[ERR] JXJ device talk server response fail\n");
					return -1;
				}
			}
			else
			{
				LOG("[ERR] server response's HI_TALK_REQ_T magic:%d is not right\n", ntohl(req->u32Magic));
				return -1;
			}
		}
		else
		{
			LOG("[ERR] HI_TALK_REQ header's %d recv header's size\n", sizeof(HI_TALK_REQ), _this->in_buf_len);
		}
	}
	else if (client->dev_type == PROXY)
	{
		HI_TALK_REQ *base_req;
		int ret = socket_read(_this->fd, (char *)(_this->in_buf+_this->in_buf_len)
			, sizeof(PROXY_TALK_REQ_T)-_this->in_buf_len);
		if(ret <= 0)
		{
			return -1;
		}
		_this->in_buf_len += ret;

		if(_this->in_buf_len == sizeof(PROXY_TALK_REQ_T))
		{
			PROXY_TALK_REQ_T *req = (PROXY_TALK_REQ_T*)_this->in_buf;
			if (ntohl(req->u32Magic) == HI_TALK_MAGIC)
			{
				LOG("pu_id:%s channel:%d\n", req->szPuId, ntohl(req->channel));
				base_req = (HI_TALK_REQ *)&req->stTalkReq;
				if (ntohl(base_req->u32Magic) == HI_TALK_MAGIC)
				{
					if(base_req->s32Res == 0)
					{
						LOG("proxy talk server response ok\n");
						return 1;
					}
					else
					{
						LOG("proxy talk server response fail\n");
						return -1;
					}
				}
				else
				{
					LOG("base_req magic:%d is not right\n", base_req->u32Magic);	
					return -1;
				}
			}
			else
			{
				LOG("server response's proxy_talk_req_t magic:%d is not right\n", req->u32Magic);
				return -1;
			}
		}
		else
		{
			LOG("proxy_talk_req header's %d recv header's size\n", sizeof(PROXY_TALK_REQ_T), _this->in_buf_len);
			return -1;
		}
	}
	else
	{
		return -1;
	}

    return 0;
}
static int conn_recv_data(struct fd_node_s *_this)
{
    int ret;
	int dev_type = ((talk_client_t *)(_this))->dev_type;
    conn_t *conn = (conn_t*)_this->context;
    
    switch(conn->recv_stat)
    {
        case RECV_HDR:
            {
                ret = socket_read(_this->fd, (char *)(_this->in_buf+_this->in_buf_len)
                        , sizeof(HI_TALK_FRMAE_HDR)-_this->in_buf_len);
                if(ret <= 0)
                {//@{虽然_this->fd是非阻塞的,但是这种情况没有关系,因为不会出现WSAEWOULDBLOCK的,可读事件触发一次,只读一下,效率很低,参考jlive}
					printf("RECV_HDR socket_read ret:%d\n", ret);
                    return -1;
                }
                _this->in_buf_len += ret;
                if(_this->in_buf_len == sizeof(HI_TALK_FRMAE_HDR))
                {
                    conn->recv_stat = RECV_DATA;
                }
            }
            break;
        case RECV_DATA:
            {
                HI_TALK_FRMAE_HDR *hdr = (HI_TALK_FRMAE_HDR*)_this->in_buf;
                
				if (dev_type == PROXY)
				{
					if (ntohl(hdr->u32Magic) == HI_TALK_MAGIC)
					{//@{如果上一次没收全, 也没关系,接着收,此时hdr还是指向_this->in_buf,因为上一次数据没有抛给应用层}
						//printf("hdl:%p, frm => u32Magic:0x%x, u32FrameNo:%d, u32Pts:%d, u32Len:%d\n"
							//, hdr, ntohl(hdr->u32Magic), ntohl(hdr->u32FrameNo), ntohl(hdr->u32Pts), ntohl(hdr->u32Len));

						ret = socket_read(_this->fd, (char *)(_this->in_buf+_this->in_buf_len)
							, ntohl(hdr->u32Len) - (_this->in_buf_len-sizeof(HI_TALK_FRMAE_HDR)));
						if(ret <= 0)
						{
							LOG("RECV_DATA proxy device socket_read ret:%d\n", ret);
							return -1;
						}
						_this->in_buf_len += ret;
						if(_this->in_buf_len == (sizeof(HI_TALK_FRMAE_HDR)+ntohl(hdr->u32Len)))
						{// 一帧没有收全时, 还是接着接收,不抛给应用层
							conn->recv_stat = RECV_HDR;
							return 1;
						}
					}
					else 
					{
						LOG("RECV_DATA server response's proxy_talk_req_t magic:%d is not right\n", ntohl(hdr->u32Magic));
						return -1;
					}
				}
				else if (dev_type == JXJ)
				{
					if (hdr->u32Magic == HI_TALK_MAGIC)
					{//@{如果上一次没收全, 也没关系,接着收,此时hdr还是指向_this->in_buf,因为上一次数据没有抛给应用层}
						//printf("hdl:%p, frm => u32Magic:0x%x, u32FrameNo:%d, u32Pts:%d, u32Len:%d\n"
						//, hdr, ntohl(hdr->u32Magic), ntohl(hdr->u32FrameNo), ntohl(hdr->u32Pts), ntohl(hdr->u32Len));

						ret = socket_read(_this->fd, (char *)(_this->in_buf+_this->in_buf_len)
							, hdr->u32Len - (_this->in_buf_len-sizeof(HI_TALK_FRMAE_HDR)));
						if(ret <= 0)
						{
							LOG("RECV_DATA JXJ deivce socket_read ret:%d\n", ret);
							return -1;
						}
						_this->in_buf_len += ret;
						if(_this->in_buf_len == (sizeof(HI_TALK_FRMAE_HDR)+hdr->u32Len))
						{// 一帧没有收全时, 还是接着接收,不抛给应用层
							conn->recv_stat = RECV_HDR;
							return 1;
						}
					}
					else 
					{
						LOG("RECV_DATA server response's JXJ device magic:%d is not right\n", hdr->u32Magic);
						return -1;
					}
				}
				else
				{
					return -1;
				}
            }
            break;
    }
    return 0;
}

/////////////////////////////////////////

static int socket_connect(int fd, char *ip, unsigned short port)
{
	struct sockaddr_in dst_addr;
	socklen_t addrlen = sizeof(dst_addr);

	if(ip == NULL || port == 0)
    {
        return -1;
    }
   
    dst_addr.sin_family = AF_INET;
    dst_addr.sin_port = htons(port);
#ifdef WIN32
	dst_addr.sin_addr.s_addr = inet_addr(ip);
#else
    inet_aton(ip, &dst_addr.sin_addr);
#endif    
    if(connect(fd, (struct sockaddr *)&dst_addr, addrlen) < 0)
    {
#ifdef WIN32
		return WSAGetLastError();
#else
        return -errno;
#endif
    }
    return 0;
}

static int socket_write(int fd, char *buf, int size)
{
    size_t left = size;
    int ret;

    while (left > 0)
    {
#ifdef WIN32
		ret = send(fd, buf, left, 0);
#else
        ret = write(fd, buf, left);                                                                                                                                        
#endif
        if (ret < 0)
        {
#ifdef WIN32
			ret = WSAGetLastError();
			if (WSAEINTR == ret)
				continue;
#else
			ret = -errno;
            if (ret == -EINTR)
                continue;
			
            if (ret == -EAGAIN)
                break;
			return ret;
#endif
 
            
        }

        buf += ret;
        left -= ret;
    }

    return size - left;
}

static int socket_read(int fd, char *buf, int size)
{
    int ret;

    for (;;)
    {
#ifdef WIN32
		ret = recv(fd, buf, size, 0);
#else
        ret = read(fd, buf, size);
#endif
        if (ret < 0)
        {
#ifdef WIN32
			// 原来的错误值返回的正值, 上层不能判断是否正确
			// 原来的错误值(10055)返回以后, 一直出错, 结果出现了一个(10053)错误, 彩蛋啊,以前没见过, 对一个无效的fd,再次read,就会返回(10053)的错误
			ret = -WSAGetLastError();
			if (WSAEINTR == -ret)
			{
				continue;
			}
#else
            ret = -errno;
            if (ret == -EINTR)
                continue;
#endif
            return ret;
        }

        return ret;
    }

    return 0;
}
