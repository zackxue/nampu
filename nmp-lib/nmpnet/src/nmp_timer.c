#include "nmp_timer.h"


static nmp_bool_t
nmp_on_timer_internal(nmp_event_t *ev, int revents, void *user_data)
{
	nmp_timer_t *timer = (nmp_timer_t*)ev;

	return (*timer->on_timer)(timer->data) == 0 ? TRUE : FALSE;
}


nmp_timer_t *nmp_timer_new(int timeout, nmp_on_timer on_timer, void *data)
{
	nmp_timer_t *timer;

	timer = (nmp_timer_t*)nmp_event_new(sizeof(nmp_timer_t), -1, 0);
	timer->on_timer = on_timer;
	timer->data = data;

	nmp_event_set_callback((nmp_event_t*)timer, nmp_on_timer_internal, NULL, NULL);
	nmp_event_set_timeout((nmp_event_t*)timer, timeout);

	return timer;
}


int nmp_timer_attach(nmp_event_loop_t *loop, nmp_timer_t *timer)
{
	NMP_ASSERT(loop != NULL);

	return !nmp_event_loop_attach(loop, (nmp_event_t*)timer);
}


void nmp_timer_del(nmp_timer_t *timer)
{
	nmp_event_remove((nmp_event_t*)timer);
}

//:~ End

//for include some necessary file.o
void test()
{
    nmp_thread_pool_new(NULL, NULL, 1, NULL);
    base64_free(NULL, 0);
    return ;
}

