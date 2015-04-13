#ifndef __NMP_EVENT_H__
#define __NMP_EVENT_H__

#include <stddef.h>
#include <ev.h>

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
	NMP_ASSERT((pobj) && atomic_get(&(pobj)->ref_count) > 0); \
	NMP_ASSERT(atomic_get(&(pobj)->ref_count) < MAX_REF_COUNT_VALUE); \
} while (0)
#endif

typedef struct _nmp_event nmp_event_t;
typedef struct _nmp_event_loop nmp_event_loop_t;

typedef nmp_bool_t (*nmp_event_dispatch)(nmp_event_t *ev, int revents, void *user_data);
typedef void (*nmp_event_on_destroy)(nmp_event_t *ev);	/* destroy notify */

struct _nmp_event
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
	nmp_event_loop_t *loop;

	nmp_event_dispatch dispath;
	nmp_event_on_destroy destroy;
};

nmp_event_t *nmp_event_new(size_t size, int fd, int events);
nmp_event_t *nmp_event_ref(nmp_event_t *event);
void nmp_event_unref(nmp_event_t *event);
void nmp_event_set_callback(nmp_event_t *event, nmp_event_dispatch dispath,
	void *user_data, nmp_event_on_destroy notify);
void nmp_event_set_timeout(nmp_event_t *event, int timeout);
#define nmp_event_fd(ev) (((nmp_event_t*)ev)->ev_fd)
void nmp_event_remove_events(nmp_event_t *event, int events);
void nmp_event_add_events(nmp_event_t *event, int events);

void nmp_event_remove(nmp_event_t *event);	/* remove from it's loop */

void nmp_event_mod_timer_sync(nmp_event_t *event, ev_tstamp after);	/* triggered after 'after' secs */
void nmp_event_remove_events_sync(nmp_event_t *event, int events);
void nmp_event_add_events_sync(nmp_event_t *event, int events);
ev_tstamp nmp_event_time_now_sync(nmp_event_t *event);

nmp_event_loop_t *nmp_event_loop_new( void );
nmp_event_loop_t *nmp_event_loop_ref(nmp_event_loop_t *loop);
void nmp_event_loop_unref(nmp_event_loop_t *loop);
void nmp_event_loop_run(nmp_event_loop_t *loop);
void nmp_event_loop_quit(nmp_event_loop_t *loop);
nmp_bool_t nmp_event_loop_attach(nmp_event_loop_t *loop, nmp_event_t *event);

#ifdef __cplusplus
}
#endif

#endif	/* __NMP_EVENT_H__ */
