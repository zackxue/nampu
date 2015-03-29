#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "proto_watch.h"
#include "tr_log.h"
#include "unix_sock.h"


#define PROTO_WATCH_WEIGHT		1
#define MAX_BACKLOG_SIZE		MAX_FRAME_SIZE


enum
{
	STAT_OK,
	STAT_KILLED
};


static __inline__ int32_t
__proto_watch_killed(proto_watch *w)
{
	return w->state == STAT_KILLED;
}


static __inline__ void
__proto_watch_on_closed(proto_watch *w, void *u)
{
	void (*on_closed)(proto_watch *, void *);

	if (w->ops && w->ops->conn_closed)
	{
		on_closed = w->ops->conn_closed;

		RELEASE_LOCK(w->lock);
		(*on_closed)(w, u);
		AQUIRE_LOCK(w->lock);
	}

	LOG_I("pw '%p' closed, fd:'%d', u:'%p'", w, w->watch_fd, u);
	close(w->watch_fd);
	w->watch_fd = -1;
}


static __inline__ void
__proto_watch_on_error(proto_watch *w, int32_t err, void *u)
{
	void (*on_error)(proto_watch *, int32_t err, void *);

	if (w->ops && w->ops->conn_error)
	{
		on_error = w->ops->conn_error;

		RELEASE_LOCK(w->lock);
		(*on_error)(w, err, u);
		AQUIRE_LOCK(w->lock);
	}

	LOG_I("pw '%p' error, fd:'%d', u:'%p'.", w, w->watch_fd, u);
	close(w->watch_fd);
	w->watch_fd = -1;
}


static __inline__ int32_t
proto_watch_write_directly(proto_watch *w, const uint8_t *buffer,
	uint32_t *idx, uint32_t size)
{
	int32_t ret, err;
	uint32_t left;

	if (*idx > size)
	{
		return -EINVAL;
	}

	left = size - *idx;

	while (left > 0)
	{
		ret = send(w->watch_fd, &buffer[*idx], left, MSG_DONTWAIT);
		if (ret == 0)
		{
			return -EAGAIN;
		}
		else if (ret < 0)
		{
			err = -errno;
			if (err != -EINTR)
			{
				return err;
			}

			return -EAGAIN;		/* continue */
		}
		else
		{
			left -= ret;
			 *idx += ret;
		}
  	}

	return 0;
}


static __inline__ int32_t
__proto_watch_write_pending(proto_watch *w)
{
	int32_t err;

	for (;;)
	{
		if (!w->write_buffer)
		{
			if (list_empty(&w->backlog_list))
			{
				j_event_remove_events_sync((JEvent*)w, EV_WRITE);
				return 0;
			}

			w->write_buffer = list_entry(w->backlog_list.next, mem_block, list);
			list_del(w->backlog_list.next);
			w->backlog_size -= w->write_buffer->size;
		}

		err = proto_watch_write_directly(w, w->write_buffer->ptr,
			&w->write_buffer->offset, w->write_buffer->size);
		if (err)
		{
			return err;
		}

		BUG_ON(w->write_buffer->offset != w->write_buffer->size);
		free_memblock(w->write_buffer);
		w->write_buffer = NULL;
	}

	return 0;
}


static __inline__ int32_t
__proto_watch_recv(proto_watch *w, msg *m, void *u)
{
	int32_t err = 0;

	if (w->ops && w->ops->proto_msg_recv)
	{
		RELEASE_LOCK(w->lock);

		LOG_I("pw '%p' recv msg.", w);
		err = (*w->ops->proto_msg_recv)(w, m, u);
		LOG_I("pw '%p' process msg ok.", w);

		AQUIRE_LOCK(w->lock);
	}

	return err;
}


static __inline__ JBool
__proto_watch_dispath(proto_watch *w, int32_t revents, void *u)
{
	msg *m;
	int32_t err = -EAGAIN;
	socklen_t len = sizeof(err);

	if (revents & EV_TIMER)
	{
		LOG_I("__proto_watch_dispath(pw '%p')->timeout!", w);
		goto conn_err;
	}

	if (revents & EV_WRITE)
	{
		if (w->connecting)
		{
			w->connecting = 0;
			if (getsockopt(w->watch_fd, SOL_SOCKET, SO_ERROR, &err, &len))
			{
				LOG_I("__proto_watch_dispath()->getsockopt() failed.");
				err = -errno;
				goto conn_err;
			}
			else
			{
				if (err)
				{
					LOG_I("__proto_watch_dispath()->connect() failed.");
					err = -err;
					goto conn_err;
				}
			}
		}
		else
		{
			err = __proto_watch_write_pending(w);
			if (err && err != -EAGAIN)
			{
				goto conn_err;
			}
		}
	}

	if (revents & EV_READ)
	{
__parse_again:
		m = parse_proto_io(w->parser, (void*)w->watch_fd, &err);
		if (!m)
		{
			if (err != -EAGAIN)
			{
				if (err == -ECONNRESET)
				{
					LOG_I("pw '%p' connction reset by peer!", w);
					goto conn_closed;
				}
				else
				{
					LOG_I("pw '%p' connection error!", w);
					goto conn_err;
				}
			}
		}
		else
		{
			err = __proto_watch_recv(w, m, u);
			if (err)
			{//@{Note, goes to the same way as @err}
				LOG_I(
					"__proto_watch_dispath()->(*w->ops->proto_msg_recv)() failed,"
					"err:'%d'.", err);
				goto conn_err;
			}
			goto __parse_again;
		}
	}

	return TRUE;

conn_closed:
	__proto_watch_on_closed(w, u);
	return FALSE;

conn_err:
	__proto_watch_on_error(w, err, u);
	return FALSE;
}


static JBool
proto_watch_dispath(JEvent *ev, int32_t revents, void *u)
{
	JBool ret = FALSE;
	proto_watch *w = (proto_watch*)ev;

	AQUIRE_LOCK(w->lock);
	if (!__proto_watch_killed(w))
	{
		ret = __proto_watch_dispath(w, revents, u);
	}
	RELEASE_LOCK(w->lock);

	return ret;
}


static __inline__ void
free_backlog_list(struct list_head *head)
{
	struct list_head *l;
	mem_block *mb;

	while (!list_empty(head))
	{
		l = head->next;
		list_del(l);
		mb = list_entry(l, mem_block, list);
		free_memblock(mb);
	}
}


static __inline__ void
proto_watch_fin_self(proto_watch *w)
{
	void *user = j_event_u(w);

	if (w->watch_fd > 0)
	{
		//@{Default: so_linger off, close() return immediately,
		//system keep sending itself.}
		close(w->watch_fd);
	}

	if (w->parser)
	{
		if (w->ops->release_proto_parser)
		{
			(*w->ops->release_proto_parser)(w->parser, user);
		}
	}

	LOCK_DEL(w->lock);

	if (!list_empty(&w->backlog_list))
	{
		free_backlog_list(&w->backlog_list);
	}

	if (w->write_buffer)
	{
		free_memblock(w->write_buffer);
	}

	LOG_I("pw '%p' finalized, u:'%p'.", w, user);
}


static void
proto_watch_on_finalize(JEvent *ev)
{
	proto_watch *w = (proto_watch*)ev;

	proto_watch_fin_self(w);

	if (w->fin)
	{
		(*w->fin)(j_event_u(ev));
	}
}


proto_watch *
proto_watch_new(void *io, int32_t timeout, proto_watch_ops *ops, void *u,
	proto_watch_on_fin on_finalize)
{
	proto_watch *pw;
	JEvent *ev;
	proto_parser *p = NULL;

	if (!io || !ops)
	{
		LOG_W("proto_watch_new()->(io/ops == NULL).");
		return NULL;
	}

	ev = j_event_new(sizeof(proto_watch), (int32_t)io, EV_READ);
	if (!ev)
	{
		LOG_W("proto_watch_new()->j_event_new() failed.");
		return NULL;
	}

	pw = (proto_watch*)ev;
	pw->watch_fd = -1;

	if (ops->create_proto_parser)
	{
		p = (*ops->create_proto_parser)(u);
		if (!p)
		{
			LOG_W("proto_watch_new()->(*create_proto_parser)() failed.");
			j_event_unref(ev);
			return NULL;
		}
	}
	else
	{
		BUG();
	}

	pw->watch_fd = (int32_t)io;
	pw->parser = p;
	pw->fin = on_finalize;
	pw->state = STAT_OK;
	pw->window_size = 0;
	pw->connecting = 0;
	pw->sched = NULL;
	pw->write_buffer = NULL;
	INIT_LIST_HEAD(&pw->backlog_list);
	pw->backlog_size = 0;
	pw->lock = LOCK_NEW();
	pw->ops = ops;
	BUG_ON(unix_sock_set_flags(pw->watch_fd, O_NONBLOCK));

	j_event_set_callback(ev, proto_watch_dispath, u,
		proto_watch_on_finalize);

	if (timeout)
	{
		j_event_set_timeout((JEvent*)pw, timeout);
	}

	return (proto_watch*)ev;
}


proto_watch *
proto_watch_ref(proto_watch *w)
{
	j_event_ref((JEvent*)w);
	return w;
}


void
proto_watch_unref(proto_watch *w)
{
	j_event_unref((JEvent*)w);
}


static __inline__ void
__proto_watch_kill(proto_watch *w)
{
	if (!__proto_watch_killed(w))
	{
		w->state = STAT_KILLED;
		if (w->sched)
		{
			j_sched_remove(w->sched, (JEvent*)w);
			w->sched = NULL;
		}
	}
}


static __inline__ void
proto_watch_kill(proto_watch *w)
{
	AQUIRE_LOCK(w->lock);
	__proto_watch_kill(w);
	RELEASE_LOCK(w->lock);
}


void proto_watch_kill_unref(proto_watch *w)
{
	proto_watch_kill(w);
	proto_watch_unref(w);
}


int32_t
proto_watch_set_window(proto_watch *w, uint32_t win_size)
{
	if (w && win_size > 0)
	{
		w->window_size = win_size;
		return 0;
	}

	return -EINVAL;
}


static __inline__ int32_t
__proto_watch_attach(proto_watch *w, void *sched)
{
	if (__proto_watch_killed(w) || !sched)
		return -EINVAL;

	if (w->sched)
		return -EEXIST;

	w->sched = sched;
	j_sched_add(sched, (JEvent*)w, PROTO_WATCH_WEIGHT);
	return 0;
}


int32_t proto_watch_attach(proto_watch *w, void *sched)
{
	int32_t err;

	if (!w || !sched || __proto_watch_killed(w))
		return -EINVAL;

	AQUIRE_LOCK(w->lock);
	err = __proto_watch_attach(w, sched);
	RELEASE_LOCK(w->lock);

	return err;
}


static __inline__ void
proto_watch_adjust_window(proto_watch *w, uint32_t seq)
{
	mem_block *mb;
	struct list_head *l, *next;

	if (w->window_size)
	{
		l = w->backlog_list.next;
		while (l != &w->backlog_list)
		{
			mb = list_entry(l, mem_block, list);
			if (!MEMBLOCK_DROPABLE(mb))
			{
				l = l->next;
				continue;
			}
			if (seq - MEMBLOCK_GET_SEQ(mb) <= w->window_size)
				break;
			next = l->next;
			list_del(l);
			w->backlog_size -= mb->size;
			free_memblock(mb);
			l = next;
		}
	}
}


int32_t
proto_watch_write_mb(proto_watch *w, mem_block *mb, uint32_t flags)
{
	int32_t err, pending = 0;

	AQUIRE_LOCK(w->lock);

	if (__proto_watch_killed(w))
	{
		free_memblock(mb);
		err = -EKILLED;
		goto write_done;
	}

	if (!w->connecting && !w->write_buffer && list_empty(&w->backlog_list))
	{
		err = proto_watch_write_directly(w, mb->ptr, &mb->offset, mb->size);
		if (err != -EAGAIN)
		{
			free_memblock(mb);
			goto write_done;
		}

		w->write_buffer = mb;
		pending = 1;
		err = 0;
		goto write_done;
	}

	proto_watch_adjust_window(w, MEMBLOCK_GET_SEQ(mb));
	if (w->backlog_size + mb->size > MAX_BACKLOG_SIZE)
	{
		free_memblock(mb);
		err = -EAGAIN;
		goto write_done;
	}

	pending = 1;
	list_add_tail(&mb->list, &w->backlog_list);
	w->backlog_size += mb->size;
	err = 0;

write_done:
	RELEASE_LOCK(w->lock);

	if (pending)
	{
		j_event_add_events((JEvent*)w, EV_WRITE);
	}

	return err;
}


int32_t
proto_watch_write(proto_watch *w, msg *m, uint32_t seq, uint32_t flags)
{//@{Destroy msg certainly}
	mem_block *mb = msg_to_mb(m);

	if (mb)
	{
		MEMBLOCK_SET_SEQ(mb, seq);
		return proto_watch_write_mb(w, mb, flags);
	}

	msg_unref(m);
	return -EINVAL;
}


static __inline__ int32_t
__proto_watch_set_dst(proto_watch *w, uint8_t *ip, int32_t port)
{
	int32_t err;

	err = unix_sock_connect(w->watch_fd, ip, port);
	if (!err)
	{
		return 0;
	}

	if (err == -EINPROGRESS)
	{
		w->connecting = 1;
		j_event_add_events((JEvent*)w, EV_WRITE);
		return 0;
	}

	return err;
}


int32_t
proto_watch_writeable(proto_watch *w, uint32_t size)
{//@{writeable: return 0}
	int32_t err = -EAGAIN;

	if (size > MAX_BACKLOG_SIZE)
	{//@{size too large, writeable, maybe drop}
		return 0;
	}

	AQUIRE_LOCK(w->lock);
	if (w->backlog_size + size <= MAX_BACKLOG_SIZE)
		err = 0;
	RELEASE_LOCK(w->lock);

	return err;
}


int32_t
proto_watch_set_dst(proto_watch *w, uint8_t *ip, int32_t port)
{
	int32_t err = -EKILLED;

	AQUIRE_LOCK(w->lock);
	if (!__proto_watch_killed(w))
	{
		err = __proto_watch_set_dst(w, ip, port);
	}
	RELEASE_LOCK(w->lock);

	return err;
}


//:~ End
