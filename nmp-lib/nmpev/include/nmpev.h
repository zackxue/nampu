/* *
 * This file implements event source using glib and libev.
 *
 * Copyright (C) 2012 NAMPU
 * See COPYING for more details
 *
 * */

#ifndef __NAMPU_EV_H__
#define __NAMPU_EV_H__

#include <stddef.h>
#include <ev.h>
#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EVENTS_MASK (EV_READ | EV_WRITE)
#define EV_NONE EV_NONE

#ifndef REF_DEBUG_TEST
#define MAX_REF_COUNT_VALUE (10000)	/* Maybe enough */
#define REF_DEBUG_TEST(pobj)	\
do {\
	g_assert((pobj) && g_atomic_int_get(&(pobj)->ref_count) > 0); \
	g_assert(g_atomic_int_get(&(pobj)->ref_count) < MAX_REF_COUNT_VALUE); \
} while (0)
#endif

typedef struct _GEvent GEvent;
typedef struct _GEventLoop GEventLoop;

typedef gboolean (*GEventDispath)(GEvent *ev, gint revents, void *user_data);
typedef void (*GEventFinalize)(GEvent *ev);	/* on destroy */

struct _GEvent
{
	gint ref_count;
	gsize event_size;
	struct ev_io io;
	struct ev_timer timer;
	gint ev_fd;
	gint timeout;		/* milli-secs */
	gint events;		/* atomic get/set */
	gint flags;			/* flags internal */
	void *user_data;
	GEventLoop *loop;

	GEventDispath dispath;
	GEventFinalize finalize;
};

GEvent *g_event_new(size_t size, gint fd, gint events);

GEvent *g_event_ref(GEvent *event);
void g_event_unref(GEvent *event);

/**
 *  can only be invoked before g_event_loop_attach().
 *	timeout: milli-secs
 */
void g_event_set_timeout(GEvent *event, gint timeout);

void g_event_set_callback(GEvent *event, GEventDispath dispath,
	void *user_data, GEventFinalize fin);

#define g_event_fd(ev) (((GEvent*)ev)->ev_fd)
#define g_event_u(ev) (((GEvent*)ev)->user_data)	/* in fin() */

void g_event_remove_events(GEvent *event, gint events);
void g_event_add_events(GEvent *event, gint events);

void g_event_remove(GEvent *event);	/* remove from it's loop */

/**
 * Note: extension
 * These can only be invoked in event callback function itself
 */
//void g_event_mod_timer_sync(GEvent *event, gint milli_secs);
void g_event_mod_timer_sync(GEvent *event, ev_tstamp after);	/* triggered after 'after' secs */
void g_event_remove_events_sync(GEvent *event, gint events);
void g_event_add_events_sync(GEvent *event, gint events);
ev_tstamp g_event_time_now_sync(GEvent *event);
/* ============================================================ */

GEventLoop *g_event_loop_new( void );
GEventLoop *g_event_loop_ref(GEventLoop *loop);
void g_event_loop_unref(GEventLoop *loop);
void g_event_loop_run(GEventLoop *loop);
void g_event_loop_quit(GEventLoop *loop);

/**
 * Note: restriction!!
 * Each event object can be added to loop only once, even that
 * it was removed from the loop it has been ever added. It can
 * NOT been added to any loop any more.
 */
gboolean g_event_loop_attach(GEventLoop *loop, GEvent *event);

#ifdef __cplusplus
}
#endif

#endif	/* __G_EV_H__ */
