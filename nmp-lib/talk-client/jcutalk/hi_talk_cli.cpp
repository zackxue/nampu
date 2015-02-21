//#include "stdafx.h"
//#include <unistd.h>
//#include <sys/socket.h>
//#include <arpa/inet.h>
//#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>

//#include "fd_select.h"
#include "sched.h"
#include "talk_client.h"
#include "fd_set.h"
#include "hi_talk_cli.h"

#define DEFAULT_TIMEOUT (5*1000)
#define MAX_SNDBUF  (100*1024)

typedef enum _CONN_STAT {
      CONN_CONNECTING   = 0x01
    , CONN_WAIT
    , CONN_RW
    , CONN_FIN
}CONN_STAT_E;

typedef enum _RECV_STAT {
     RECV_HDR        = 0x00
    ,RECV_DATA       = 0x01
}RECV_STAT_E;

typedef struct conn_s {
    int  fd;
    int  timeout;
    int  stat;
    int  recv_stat;
    void *ctx;
}conn_t;

typedef struct talk_s {
    fd_set_t        *_set;
    int             runing; 
#ifdef WIN32
	DWORD			set_tid;
	HANDLE			th_pid;
	HANDLE			exitEvent;		//线程退出事件
	CRITICAL_SECTION set_mutex;
#else
	pid_t           set_tid;
    pthread_t       th_pid; 
	pthread_mutex_t set_mutex;
#endif
    talk_parm_t     parm;
}talk_t;


static talk_t *talk = NULL;
#ifdef WIN32
typedef int	socklen_t;
#define gettid() GetCurrentThreadId()
#else
#include <sys/syscall.h>
#define gettid()  syscall(SYS_gettid)
#endif

static int socket_connect(int fd, char *ip, unsigned short port);
static int socket_set_noblock(int fd);
static int socket_write(int fd, char *buf, int size);
static int socket_read(int fd, char *buf, int size);

static int conn_send_req(struct fd_node_s *_this);
static int conn_recv_rsp(struct fd_node_s *_this);
static int conn_recv_data(struct fd_node_s *_this);
static int conn_read(struct fd_node_s *_this);

//static conn_t *conn_new(void *ctx);
//static int conn_del(conn_t *conn);

/*
#ifdef WIN32
DWORD WINAPI poll_task(LPVOID lpParam);
#else
static void *poll_task(void *parm);
#endif
*/
/////////////////////////////////////////

#define MAX_CLIENT_THREADS 1
void *__get_main_sched()
{
	static sched_t *g_sched = NULL;
	if (!g_sched)
	{
		g_sched = sched_new(MAX_CLIENT_THREADS);
	}

	return g_sched;
}

void __release_main_sched()
{
	sched_t *sched = NULL;
	if (!(sched = (sched_t *)__get_main_sched()))
	{
		return ;
	}

	sched_del(sched);
}

static talk_parm_t talk_ops;

int __set_talk_ops(talk_parm_t *parm)
{
	if (!parm)
	{
		assert(parm);
		return -1;
	}
	talk_ops = *parm;
	
	return 0;
}

talk_parm_t *__get_talk_ops()
{
	return &talk_ops;
}

/*
 * hi_talk_cli_init
 * 
 * @describe-init event loop based on select.
 *
 * @date-2-13-4-28
 */
int hi_talk_cli_init(talk_parm_t *parm)
{
#if 0
    talk = (talk_t*)calloc(1, sizeof(talk_t));
 
    if(!talk) return -1;
#ifdef WIN32
	InitializeCriticalSection(&talk->set_mutex);
	talk->exitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
#else
	pthread_mutex_init(&talk->set_mutex, NULL);
#endif
    

    talk->parm = *parm;
    
    talk->_set = fd_set_new0();
    
    talk->runing = 1;

#ifdef WIN32
	if ((talk->th_pid = CreateThread(NULL, 0, poll_task, NULL, 0,NULL)) == NULL)
	{
		goto _error;
	}
	while(!talk->set_tid) Sleep(5);
#else
    if(pthread_create(&talk->th_pid, NULL, poll_task, NULL) < 0)
    {
        goto _error;
    }
	while(!talk->set_tid) usleep(0);
#endif

    
    return 0;
_error:
    if(talk->_set) fd_set_del(talk->_set);
#ifdef WIN32
	DeleteCriticalSection(&talk->set_mutex);
	if (talk->exitEvent)
	{
		CloseHandle(talk->exitEvent);
		talk->exitEvent = NULL;
	}	
#else
	pthread_mutex_destroy(&talk->set_mutex);
#endif
    
    free(talk);
    talk = NULL;
    return -1;
#else
	sched_t *sched = NULL;

	__set_talk_ops(parm);
	sched = (sched_t *)__get_main_sched();
	
	return (sched) ? 0 : -1;
#endif
}


int hi_talk_cli_uninit(void)
{
#if 0
    if(!talk) return -1;
    
    talk->runing = 0;
#ifdef WIN32
	WaitForSingleObject(talk->exitEvent, 2000);
#else
    pthread_join(talk->th_pid, NULL);
#endif
    
    if(talk->_set) fd_set_del(talk->_set);
#ifdef WIN32
	DeleteCriticalSection(&talk->set_mutex);	
#else
    pthread_mutex_destroy(&talk->set_mutex);
#endif
    free(talk);
    talk = NULL;
    return 0;
#else
	__release_main_sched();

	return 0;
#endif
}

/////////////////////////////////////////

/*
 * hi_talk_cli_open
 *
 * @describe 连接设备
 * @date 2013-4-28
 */
hi_talk_cli_t *hi_talk_cli_open(char *ip
                        , unsigned short port
                        , void *ctx, media_info_t *mi)
{
	talk_client_t *client = NULL;
	
	client = client_new(ip, port, mi, ctx, __get_talk_ops(), (loop_t *)get_best_loop2((sched_t *)__get_main_sched()));

	return client;
}
                             
int hi_talk_cli_close(hi_talk_cli_t *hdl)
{
	if (!hdl)
	{
		return (-1);
	}
	client_unref((talk_client_t *)hdl);
	return 0;
}

/////////////////////////////////////////
/*
enum
{
JXJ = 0x01,
PROXY,
PF
};
*/
int hi_talk_cli_send(hi_talk_cli_t *hdl, HI_TALK_FRMAE_HDR *frm)
{
	conn_t * conn = NULL;
    fd_node_t *node = (fd_node_t*)hdl;
	talk_client_t *client = (talk_client_t *)node;
	int dev_type = client->dev_type;

    if(!hdl) return -1;
    
    conn = (conn_t*)node->context;
    if(conn->stat == CONN_RW)
    {
		if (dev_type == PROXY)
		{
			HI_TALK_FRMAE_HDR *hdr = (HI_TALK_FRMAE_HDR *)frm;
			hdr->u32FrameNo = htonl(hdr->u32FrameNo);
			hdr->u32Len = htonl(hdr->u32Len);
			hdr->u32Magic = htonl(hdr->u32Magic);
			hdr->u32Pts = htonl(hdr->u32Pts);

			if(socket_write(conn->fd, (char*)frm, sizeof(HI_TALK_FRMAE_HDR)+ntohl(frm->u32Len))
            != (sizeof(HI_TALK_FRMAE_HDR)+ntohl(frm->u32Len)))
			{
				return -1;
			}
		}
		else if (dev_type == JXJ)
		{// jxj的设备不转为网络字节顺序
			if(socket_write(conn->fd, (char*)frm, sizeof(HI_TALK_FRMAE_HDR)+frm->u32Len)
				!= (sizeof(HI_TALK_FRMAE_HDR)+frm->u32Len))
			{
				return -1;
			}
		}
		else
		{
			return -1;
		}
    }
    else if(conn->stat == CONN_FIN)
    {
        return -1;        
    }
    return 0;
}


int conn_read(struct fd_node_s *_this)
{
    int ret;
	talk_client_t *cli = (talk_client_t *)_this;
    conn_t *conn = (conn_t*)_this->context;
	loop_t *loop = GET_OWNER(cli, loop_t);
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
                    if(cli->parm.recv) cli->parm.recv(_this, (HI_TALK_FRMAE_HDR*)_this->in_buf, conn->ctx);
                    //clear buf;
                    _this->in_buf_len = 0;
                }
            }
            break;
        case CONN_FIN:
            break;
    }
    return 0;
}

/*
#ifdef WIN32
DWORD WINAPI poll_task(LPVOID lpParam)
#else
static void *poll_task(void *parm)
#endif
{
    int ret;
    struct timeval tv;
    
    talk->set_tid = gettid();
    while(talk->runing)
    {
        tv.tv_sec  = 0;
        tv.tv_usec = 200*1000;
        
#ifdef WIN32
		EnterCriticalSection(&talk->set_mutex);
#else
		pthread_mutex_lock(&talk->set_mutex);
#endif
		// poll_talk 在执行hi_talk_cli_init后, 不停地刷新select中的集合.
		// 其实上, 执行体就是select本身.
        ret = fd_set_poll(talk->_set, &tv);
#ifdef WIN32
		LeaveCriticalSection(&talk->set_mutex);	
#else
        pthread_mutex_unlock(&talk->set_mutex);
#endif       
        //if( ret < 0)
        {
#ifdef WIN32
			Sleep(0);	
#else
			usleep(0);
#endif
        }
    }
	if (talk->exitEvent  != NULL)
		SetEvent(talk->exitEvent);
    return NULL;
}
*/

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
	{
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
			if(req->s32Res == 0)
				return 1;
			else
				return -1;
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
				printf("pu_id:%s channel:%d\n", req->szPuId, ntohl(req->channel));
				base_req = (HI_TALK_REQ *)&req->stTalkReq;
				if (ntohl(base_req->u32Magic) == HI_TALK_MAGIC)
				{
					if(base_req->s32Res == 0)
					{
						printf("proxy talk server response ok\n");
						return 1;
					}
					else
					{
						printf("proxy talk server response fail\n");
						return -1;
					}
				}
				else
				{
					printf("base_req magic:%d is not right\n", base_req->u32Magic);	
					return -1;
				}
			}
			else
			{
				printf("server response's proxy_talk_req_t magic:%d is not right\n", req->u32Magic);
				return -1;
			}
		}
		else
		{
			printf("proxy_talk_req header's %d recv header's size\n", sizeof(PROXY_TALK_REQ_T), _this->in_buf_len);
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
                
				if (ntohl(hdr->u32Magic) == HI_TALK_MAGIC)
				{//@{如果上一次没收全, 也没关系,接着收,此时hdr还是指向_this->in_buf,因为上一次数据没有抛给应用层}
					//printf("hdl:%p, frm => u32Magic:0x%x, u32FrameNo:%d, u32Pts:%d, u32Len:%d\n"
						//, hdr, ntohl(hdr->u32Magic), ntohl(hdr->u32FrameNo), ntohl(hdr->u32Pts), ntohl(hdr->u32Len));

					ret = socket_read(_this->fd, (char *)(_this->in_buf+_this->in_buf_len)
                        , ntohl(hdr->u32Len) - (_this->in_buf_len-sizeof(HI_TALK_FRMAE_HDR)));
					if(ret <= 0)
					{
						printf("RECV_DATA socket_read ret:%d\n", ret);
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
					printf("RECV_DATA server response's proxy_talk_req_t magic:%d is not right\n", ntohl(hdr->u32Magic));
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

static int socket_set_noblock(int fd)
{
#ifdef WIN32
    int bNoBlock = 1;
	
    if (ioctlsocket(fd, FIONBIO, (unsigned long *)&bNoBlock) < 0 )
        return -1;
#else
    int flags;

    flags = fcntl(fd, F_GETFL);
    if (flags < 0)
            return flags;
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) < 0) 
            return -1;
#endif
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
			// 原来的错误值(10055)返回以后, 一直出错, 结果出现了一个(10053)错误, 彩蛋啊,以前没见过
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
