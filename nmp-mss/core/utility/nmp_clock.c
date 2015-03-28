#include "nmp_clock.h"
#include "nmp_timer.h"

#define CLOCK_WOKENUP_MS	200
static volatile time_t g_timer_cached = 0;

time_t
nmp_clock_get_time( void )
{
	if (g_timer_cached)
		return g_timer_cached;
	return time(NULL);
}


static gboolean
nmp_clock_timer(gpointer user_data)
{
	g_timer_cached = time(NULL);
	return TRUE;
}


void
nmp_init_clock( void )
{
	nmp_set_timer(
		CLOCK_WOKENUP_MS,
		nmp_clock_timer,
		NULL
	);
}

//:~ End
