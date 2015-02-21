#include <unistd.h>
#include <arpa/inet.h>
#include "rtsp_impl.h"
#include "alloc.h"
#include "tr_avs.h"
#include "timer.h"
#include "tr_log.h"
#include "unix_sock.h"
#include "server.h"

#define MAX_PUID_LEN		32
#define MAX_IP_LEN			128
#define CONNECT_INTERVAL	10

extern tr_server* avs_get_server( void );

enum
{
	TIMER, START, STOP
};

typedef struct __jpf_service_parm jpf_service_parm;
struct __jpf_service_parm
{
	uint8_t  ip[MAX_IP_LEN];
	uint16_t port;
	uint8_t  puid[MAX_PUID_LEN];
	int32_t  pu_type;
	int32_t  l4_proto;
	int32_t  ka;		/* secs */
	exp_cb   exp_notifier;
	void    *user_data;	
};

typedef struct __tp_block tp_block;
struct __tp_block
{
	uint32_t action;
	uint32_t seq;
	jpf_service_parm psp;
};

typedef struct __jpf_service jpf_service;
struct __jpf_service
{
	rtsp_ts_client *jpf_client;
	Timer *jpf_timer;
	int32_t running;
	int32_t ttl;
	uint32_t last_timer;
	uint32_t last_start;
	uint32_t last_stop;
	jpf_service_parm psp;
};

static JThreadPool *jpf_service_tp = NULL;		/* For pf service task */
static jpf_service pfs_control;


static __inline__ int32_t
jpf_service_push_op(tp_block *tb)
{
	j_thread_pool_push(jpf_service_tp, tb);
	return 0;
}


static int32_t
jpf_service_timer(Timer *self, void *data)
{
	static tp_block tb = {TIMER, 0, {}};

	++(tb.seq);
	pfs_control.last_timer = tb.seq;
	jpf_service_push_op(&tb);
	return 0;
}


static __inline__ void
jpf_service_init(jpf_service *ps)
{
	memset(ps, 0, sizeof(*ps));
	ps->jpf_timer = set_timer(1000, jpf_service_timer, NULL, ps);
	BUG_ON(!ps->jpf_timer);
}


static __inline__ int32_t
jpf_service_kill(jpf_service *ps)
{
	static uint32_t seq = 0;
	tp_block *tb = tr_alloc(sizeof(*tb));

	if (tb)
	{
		tb->action = STOP;
		tb->seq = ++seq;
		pfs_control.last_stop = tb->seq;
		jpf_service_push_op(tb);
		return 0;
	}
	return -ENOMEM;
}


static __inline__ void
jpf_service_resolve_ip(uint8_t *host, char ip[])
{
	int32_t err;
	struct sockaddr_in sin;

	err = unix_resolve_host(&sin, host, 0);
	if (err)
	{
		LOG_W(
			"jpf_service_resolve_ip()->unix_resolve_host() failed, host:%s.",
			host
		);
		goto __resolve_failed;
	}

	inet_ntop(AF_INET, &sin.sin_addr, ip, (socklen_t)MAX_IP_LEN);
	return;

__resolve_failed:
	strcpy(ip, host);
}


static __inline__ void
jpf_service_connect(jpf_service *ps)
{
	int32_t sock, err;
	struct sockaddr_in sin;
	client *c;
	tr_server *server;

	err = unix_resolve_host(&sin, ps->psp.ip, ps->psp.port);
	if (err)
	{
		LOG_W(
			"jpf_service_connect()->unix_resolve_host() failed, ip:%s.",
			ps->psp.ip
		);
		goto __conn_err;
	}

	sock = unix_sock_bind(L4_TCP, 0, 0, 0);
	if (sock < 0)
	{
		LOG_W(
			"jpf_service_connect()->unix_sock_bind() failed."
		);
		err = sock;
		goto __conn_err;
	}

	if (connect(sock, (struct sockaddr*)&sin, sizeof(sin)) < 0)
	{
		err = -errno;
		LOG_W(
			"jpf_service_connect()->connect() failed, err:'%d'.",
			err
		);
		goto __conn_err;
	}

	c = rtsp_impl_ts_client_new(0, (void*)sock);
	if (!c)
	{
		LOG_W(
			"jpf_service_connect()->rtsp_impl_ts_client_new() failed."
		);
		close(sock);
		err = -ENOMEM;
		goto __conn_err;
	}

	server = avs_get_server();
	if (!server)
	{
		LOG_W(
			"jpf_service_connect()->avs_get_server() failed."
		);
		client_kill_unref((client*)c);
		err = -EPERM;
		goto __conn_err;
	}

	client_attach(c, server->sched);
	ps->jpf_client = (rtsp_ts_client*)c;
	ps->ttl = 0;
	LOG_I(
		"Connect MDS '%s:%d' ok.", ps->psp.ip, ps->psp.port
	);
	return;

__conn_err:
	if (ps->psp.exp_notifier)
	{
		(*ps->psp.exp_notifier)(ps->psp.user_data, err);
	}
	return;
}


static void
jpf_service_op_timer(jpf_service *ps, tp_block *tb)
{
	rtsp_message *rm = NULL;
	char url[MAX_URL_LEN];
	char ip[MAX_IP_LEN];
	RTSP_RESULT res;

	if (!ps->running)
		return;

	if (!ps->jpf_client)
	{
		if (--ps->ttl > 0)
			return;

		ps->ttl = CONNECT_INTERVAL;
		jpf_service_connect(ps);
	}

	if (!ps->jpf_client)
	{
		return;
	}

	if (rtsp_impl_ts_client_killed(ps->jpf_client))
	{
		LOG_I(
			"Mds reset connection."
		);
		client_kill_unref((client*)ps->jpf_client);
		ps->jpf_client = NULL;
		return;
	}

	if (--ps->ttl > 0)
		return;

	ps->ttl = ps->psp.ka;
	jpf_service_resolve_ip(ps->psp.ip, ip);

	snprintf(url,
		MAX_URL_LEN,
		"rtsp://%s:%d/PUID=%s/keepalive=%d/l4proto=%d",
		ip,
		ps->psp.port,
		ps->psp.puid,
		ps->psp.ka,
		ps->psp.l4_proto
	);

	res = rtsp_message_new_request(&rm, RTSP_OPTIONS, url);
	if (res != RTSP_OK || !rm)
	{
		LOG_W(
			"jpf_service_op_timer()->rtsp_message_new_request() failed."
		);
		return;
	}

	rtsp_impl_send_message((rtsp_client*)ps->jpf_client, rm);
}


static void
jpf_service_op_start(jpf_service *ps, tp_block *tb)
{
	if (ps->running)
		return;

	ps->running = 1;
	memcpy(&ps->psp, &tb->psp, sizeof(ps->psp));	
}


static void
jpf_service_op_stop(jpf_service *ps, tp_block *tb)
{
	if (!ps->running)
		return;

	ps->running = 0;
	if (ps->jpf_client)
	{
		LOG_I(
			"Stop pf service."
		);
		client_kill_unref((client*)ps->jpf_client);
		ps->jpf_client = NULL;
	}
}


static void
jpf_service_op_fun(void *data, void *user_data)
{
	jpf_service *ps = (jpf_service*)user_data;
	tp_block *tb = (tp_block*)data;

	switch (tb->action)
	{
	case TIMER:
		if (ps->last_timer == tb->seq)
		{
			jpf_service_op_timer(ps, tb);
		}
		break;

	case START:
		if (ps->last_start == tb->seq)
		{
			jpf_service_op_start(ps, tb);
		}
		tr_free(tb, sizeof(*tb));
		break;

	case STOP:
		if (ps->last_stop == tb->seq)
		{
			jpf_service_op_stop(ps, tb);
		}
		tr_free(tb, sizeof(*tb));
		break;

	default:
		break;
	}
}


int32_t
jpf_facility_init( void )
{
	if (jpf_service_tp)
		return -EEXIST;

	jpf_service_init(&pfs_control);
	jpf_service_tp = j_thread_pool_new(jpf_service_op_fun, &pfs_control, 1, NULL);
	BUG_ON(!jpf_service_tp);
	return 0;
}


int32_t
avs_start_pf_service(uint8_t *mds_ip, uint16_t port, int32_t pu_type,
	uint8_t *puid, int32_t l4_proto, int32_t ka, exp_cb exp, void *u)
{
	static uint32_t seq = 0;
	tp_block *tb = tr_alloc(sizeof(*tb));

	if (tb)
	{
		memset(tb, 0, sizeof(*tb));
		tb->action = START;
		strncpy(__str(tb->psp.ip), __str(mds_ip), MAX_IP_LEN - 1);
		tb->psp.port = port;
		strncpy(__str(tb->psp.puid), __str(puid), MAX_PUID_LEN - 1);
		tb->psp.pu_type = pu_type;
		tb->psp.l4_proto = l4_proto;
		tb->psp.ka = ka;
		tb->psp.exp_notifier = exp;
		tb->psp.user_data = u;
		tb->seq = ++seq;
		pfs_control.last_start = tb->seq;
		jpf_service_push_op(tb);
		return 0;
	}
	return -ENOMEM;
}


int32_t
avs_stop_pf_service( void )
{
	return jpf_service_kill(&pfs_control);
}


//:~ End
