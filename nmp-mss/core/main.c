#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/resource.h>
#include <glib.h>
#include <assert.h>
#include <getopt.h>
#include <time.h>
#include "nmp_debug.h"
#include "nmp_signal.h"
#include "nmp_appcore.h"
#include "nmp_tq.h"
#include "nmp_clock.h"
#include "nmp_sysctl.h"
#include "rti.h"

void nmp_init_xml_cmd();
void nmp_sig_setup_sigusr1( void );
void nmp_mod_policy_insert( void );
void nmp_mod_cms_insert( void );
void nmp_mod_disk_insert( void );
void nmp_mod_stream_insert( void );
void nmp_mod_buffering_insert( void );
void nmp_mod_vod_insert( void );


static __inline__ void
nmp_mss_setup_signals( void )
{
	nmp_sig_setup_signals();
	nmp_sig_setup_sigusr1();
}


static __inline__ void
nmp_mss_log_facility_init( void )
{//TODO: PATH
	nmp_debug_log_facility_init(
		(gchar*)nmp_get_sysctl_value(SC_LOG_PATH),
		"Nmp-mss.log"
	);
}


static __inline__ void
nmp_mss_open_core_facility( void )
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
nmp_mss_running_env_init( void )
{
#ifndef G_THREADS_ENABLED
	nmp_error("<main> CMS compiled without 'G_THREADS_ENABLED' defined!");
	FATAL_ERROR_EXIT;
#endif

	if (getenv("BYPASS_GLIB_MPOOL") != NULL)
    	g_slice_set_config(G_SLICE_CONFIG_ALWAYS_MALLOC, TRUE);   
    g_thread_init(NULL);
    g_type_init();

	nmp_sysctl_init();
    nmp_mss_set_timezone((gchar*)nmp_get_sysctl_value(SC_TIMEZONE));
    nmp_mss_log_facility_init();
    nmp_mss_setup_signals();
/*    nmp_mss_open_core_facility(); */
}


static __inline__ void
nmp_mss_mods_init( void )
{
	nmp_mod_policy_insert();
	nmp_mod_disk_insert();
	nmp_mod_stream_insert();
	nmp_mod_buffering_insert();
	nmp_mod_vod_insert();
	nmp_mod_cms_insert();
}


static __inline__ void
nmp_mss_lib_init( void )
{
	nmp_init_xml_cmd();
}


int main(int argc, char *argv[])
{
    GMainLoop *loop;

    nmp_mss_running_env_init();
    nmp_afx_core_init();
    nmp_mss_lib_init();
    nmp_init_tq();
    nmp_init_clock();
    nmp_mss_mods_init();
    nmp_rti_init();
    loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

	return 0;
}

//:~ End
