/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/


#ifndef __TINY_RAIN_PROTO_WATCH_H__
#define __TINY_RAIN_PROTO_WATCH_H__

#include "nmplib.h"
#include "list.h"
#include "proto_parser.h"

BEGIN_NAMESPACE

typedef void (*proto_watch_on_fin)(void *u);
typedef struct __proto_watch proto_watch;
typedef struct __proto_watch_ops proto_watch_ops;

struct __proto_watch
{
	JEvent __super;
	int32_t watch_fd;
	proto_parser *parser;
	proto_watch_on_fin fin;
	uint32_t state;
	uint32_t window_size;
	uint32_t connecting;
	void *sched;
    mem_block *write_buffer;
	struct list_head backlog_list;
	uint32_t backlog_size;
	LOCK_T	lock;
	proto_watch_ops *ops;
};

struct __proto_watch_ops
{
	proto_parser *(*create_proto_parser)(void *u);
	void    (*release_proto_parser)(proto_parser *parser, void *u);
	int32_t (*proto_msg_sent)(proto_watch *w, uint32_t seq, void *u);	//TODO
	int32_t (*proto_msg_recv)(proto_watch *w, msg *m, void *u);
	void    (*conn_closed)(proto_watch *w, void *u);
	void    (*conn_error)(proto_watch *w, int32_t err, void *u);
};

proto_watch *proto_watch_new(void *io, int32_t timeout, proto_watch_ops *ops,
	void *u, proto_watch_on_fin on_finalize);

proto_watch *proto_watch_ref(proto_watch *w);
void proto_watch_unref(proto_watch *w);
void proto_watch_kill_unref(proto_watch *w);

int32_t proto_watch_set_window(proto_watch *w, uint32_t win_size);

int32_t proto_watch_attach(proto_watch *w, void *sched);

int32_t proto_watch_write(proto_watch *w, msg *m, uint32_t seq, uint32_t flags);
int32_t proto_watch_write_mb(proto_watch *w, mem_block *mb, uint32_t flags);

int32_t proto_watch_writeable(proto_watch *w, uint32_t size);
int32_t proto_watch_set_dst(proto_watch *w, uint8_t *ip, int32_t port);

END_NAMESPACE

#endif	//__TINY_RAIN_PROTO_WATCH_H__
