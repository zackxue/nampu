#ifndef __J_EVENT_H__
#define __J_EVENT_H__

#include <ev.h>
#include <stddef.h>

#include "nmp_types.h"


#ifdef __cplusplus
extern "C" {
#endif


#define EVENTS_MASK (EV_READ | EV_WRITE)
#define EV_NONE EV_NONE

#ifndef REF_DEBUG_TEST
#define MAX_REF_COUNT_VALUE (10000)	/* Maybe enough */
#define REF_DEBUG_TEST(pobj)	\
do {\
	J_ASSERT((pobj) && atomic_get(&(pobj)->ref_count) > 0); \
	J_ASSERT(atomic_get(&(pobj)->ref_count) < MAX_REF_COUNT_VALUE); \
} while (0)
#endif

typedef struct _JEvent JEvent;
typedef struct _JEventLoop JEventLoop;

typedef JBool (*JEventDispath)(JEvent *ev, int revents, void *user_data);
typedef void (*JEventOnDestroy)(JEvent *ev);	/* destroy notify */

struct _JEvent
{
	atomic_t ref_count;
	size_t event_size;		/* for memory releasing */

	struct ev_io io;
	struct ev_timer timer;
	int ev_fd;
	int timeout;	/* milli-secs */
	int events;
	int flags;		/* internal flags */
	void *user_data;
	JEventLoop *loop;

	JEventDispath dispath;
	JEventOnDestroy destroy;
};

JEvent *j_event_new(size_t size, int fd, int events);
JEvent *j_event_ref(JEvent *event);
void j_event_unref(JEvent *event);
void j_event_set_callback(JEvent *event, JEventDispath dispath,
	void *user_data, JEventOnDestroy notify);
void j_event_set_timeout(JEvent *event, int timeout);
#define j_event_fd(ev) (((JEvent*)ev)->ev_fd)
void j_event_remove_events(JEvent *event, int events);
void j_event_add_events(JEvent *event, int events);

void j_event_remove(JEvent *event);	/* remove from it's loop */

void j_event_mod_timer_sync(JEvent *event, ev_tstamp after);	/* triggered after 'after' secs */
void j_event_remove_events_sync(JEvent *event, int events);
void j_event_add_events_sync(JEvent *event, int events);
ev_tstamp j_event_time_now_sync(JEvent *event);

JEventLoop *j_event_loop_new( void );
JEventLoop *j_event_loop_ref(JEventLoop *loop);
void j_event_loop_unref(JEventLoop *loop);
void j_event_loop_run(JEventLoop *loop);
void j_event_loop_quit(JEventLoop *loop);
JBool j_event_loop_attach(JEventLoop *loop, JEvent *event);

#ifdef __cplusplus
}
#endif

#endif	/* __J_EVENT_H__ */
