#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <sys/resource.h>
#include <rtspwatch.h>
#include "nmp_media_server.h"
#include "nmp_debug.h"
#include "nmp_signal.h"
#include "nmp_media_factory.h"
#include "nmp_timer.h"
#include "nmp_sysctl.h"
#include "nmp_scheduler.h"
#include "nmp_main_args.h"
#include "nmp_tq.h"
#include "nmp_audit.h"
#include "nmpevsched.h"
#include "nmp_monotonic_timer.h"

extern void nmp_init_xml_cmd( void );

static __inline__ void
nmp_mds_log_facility_init( void )
{//TODO: PATH
	nmp_debug_log_facility_init(
		(gchar*)nmp_get_sysctl_value(SC_LOG_PATH),
		"Jpf-mds.log"
	);
}


static __inline__ void
nmp_mds_setup_signals( void )
{
	nmp_sig_setup_signals();
}


static __inline__ void
nmp_mds_open_core_facility( void )
{
	struct rlimit rli;

	rli.rlim_cur = RLIM_INFINITY;
	rli.rlim_max = RLIM_INFINITY;

	if (setrlimit(RLIMIT_CORE, &rli) < 0)
	{
		nmp_error("<main> set core resource limit error!");
		FATAL_ERROR_EXIT;
	}
}


static __inline__ gint
nmp_mss_set_timezone(gchar *zone)	/* fixme */
{
    gint ret = 0;

    if (zone)
    {
        ret = setenv("TZ", zone, 1);
        if (ret < 0)
        {
            return -errno;
        }
        tzset();
    }
    return ret;
}


static __inline__ void
nmp_mds_server_init( void )
{
	nmp_init_tq();
	nmp_audit_init();
	nmp_monotonic_timer_init();
	nmp_rtsp_media_factory_jxj_init();
	nmp_media_server_init();
}


static __inline__ void
nmp_mds_run_server( void )
{
	GMainLoop *loop;

	loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(loop);
}


int main (int argc, char *argv[])
{
	nmp_mds_main_args(argc, argv);

 	if (getenv("BYPASS_GLIB_POOLS") != NULL)
        g_slice_set_config(G_SLICE_CONFIG_ALWAYS_MALLOC, TRUE);

    g_thread_init(NULL);
    g_type_init();

	nmp_sysctl_init();
	nmp_mss_set_timezone((gchar*)nmp_get_sysctl_value(SC_TIMEZONE));
	nmp_init_xml_cmd();
	g_scheduler_init(2);	/* todo */
	nmp_scheduler_init();

	nmp_mds_log_facility_init();
	nmp_mds_setup_signals();
	nmp_mds_open_core_facility();
	nmp_mds_server_init();
	nmp_mds_run_server();

	return 0;
}


//:~ End
