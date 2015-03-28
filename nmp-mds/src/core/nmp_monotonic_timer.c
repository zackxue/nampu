#include "nmp_monotonic_timer.h"
#include "nmp_tq.h"

static volatile guint s_timer_sec = 0;

static void
nmp_monotonic_timer_inc(void *parm)
{
	g_atomic_int_add((gint*)&s_timer_sec, 1);
}


void nmp_monotonic_timer_init( void )
{
	static NmpTq mon_timer_tq;

	TQ_INIT(&mon_timer_tq, nmp_monotonic_timer_inc, NULL);
	nmp_add_tq(&mon_timer_tq);	
}


guint nmp_get_monotonic_time( void )
{
	return g_atomic_int_get(&s_timer_sec);
}


//:~ End
