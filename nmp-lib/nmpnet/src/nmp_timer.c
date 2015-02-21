#include "nmp_timer.h"


static JBool
j_on_timer_internal(JEvent *ev, int revents, void *user_data)
{
	JTimer *timer = (JTimer*)ev;

	return (*timer->on_timer)(timer->data) == 0 ? TRUE : FALSE;
}


JTimer *j_timer_new(int timeout, JOnTimer on_timer, void *data)
{
	JTimer *timer;

	timer = (JTimer*)j_event_new(sizeof(JTimer), -1, 0);
	timer->on_timer = on_timer;
	timer->data = data;

	j_event_set_callback((JEvent*)timer, j_on_timer_internal, NULL, NULL);
	j_event_set_timeout((JEvent*)timer, timeout);

	return timer;
}


int j_timer_attach(JEventLoop *loop, JTimer *timer)
{
	J_ASSERT(loop != NULL);

	return !j_event_loop_attach(loop, (JEvent*)timer);
}


void j_timer_del(JTimer *timer)
{
	j_event_remove((JEvent*)timer);
}

//:~ End

//for include some necessary file.o
void test()
{
    j_thread_pool_new(NULL, NULL, 1, NULL);
    base64_free(NULL, 0);
    return ;
}

