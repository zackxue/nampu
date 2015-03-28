#include <stdlib.h>
#include <string.h>
#include <nmp_rw_file.h>
#include <syslog.h>
#include <fcntl.h>
#include "nmp_sysctl.h"
#include "nmp_debug.h"


typedef struct _NmpSysCtl NmpSysCtl;

struct _NmpSysCtl
{
	gint		base_port;			/* Platform base port */
	gint		max_log_file_size;	/* Top log file size limit (MB) */
	gchar		log_path[SC_MAX_LOG_PATH];	/* Log file path */
	gchar		mss_id[SC_MAX_ID_LEN];	/* Mss ID */
	gchar		cms_host[SC_MAX_HOST_LEN];	/* Cms host */
	gchar		storage_type[SC_MAX_STORAGE_TYPE];	/* storage type */
	gchar		time_zone[SC_TIMEZONE_LEN];	/* timezone we're in */
};


static NmpSysCtl nmp_mss_ctl = 
{
	.base_port			= 9902,
	.max_log_file_size	= 50,
	.log_path			= "/etc",
	.mss_id				= "MSS-0001",
	.cms_host			= "127.0.0.1",
	.storage_type		= "Sata_Hds",
	.time_zone			= "GMT-8"
};


static __inline__ void
nmp_set_sysctl_value(NmpSysCtl *sc, NmpSCID id, gchar *value)
{
	gint _value;

	switch (id)
	{
	case SC_BASE_PORT:
		_value = atoi(value);
		if (_value > 0)
			sc->base_port = _value;
		break;

	case SC_MAX_LOG_SIZE:
		_value = atoi(value);
		if (_value >= 1)
			sc->max_log_file_size = _value;
		break;

	case SC_LOG_PATH:
		if (value)
			strncpy(sc->log_path, value, SC_MAX_LOG_PATH - 1);
		break;

	case SC_MSS_ID:
		if (value)
			strncpy(sc->mss_id, value, SC_MAX_ID_LEN - 1);
		break;

	case SC_CMS_HOST:
		if (value)
			strncpy(sc->cms_host, value, SC_MAX_HOST_LEN - 1);
		break;

	case SC_STORAGE_TYPE:
		if (value)
			strncpy(sc->storage_type, value, SC_MAX_STORAGE_TYPE - 1);
		break;

	case SC_TIMEZONE:
		if (value)
			strncpy(sc->time_zone, value, SC_TIMEZONE_LEN - 1);
		break;

	default:
		BUG();
		break;
	}
}


static __inline__ void *
__get_sysctl_value(NmpSysCtl *sc, NmpSCID id)
{
	void *value;

	switch (id)
	{
	case SC_BASE_PORT:
		value = (void*)sc->base_port;
		break;

	case SC_MAX_LOG_SIZE:
		value = (void*)sc->max_log_file_size;
		break;

	case SC_LOG_PATH:
		value = (void*)sc->log_path;
		break;

	case SC_MSS_ID:
		value = (void*)sc->mss_id;
		break;

	case SC_CMS_HOST:
		value = (void*)sc->cms_host;
		break;

	case SC_STORAGE_TYPE:
		value = (void*)sc->storage_type;
		break;

	case SC_TIMEZONE:
		value = (void*)sc->time_zone;
		break;

	default:
		BUG();
		break;
	}

	return value;
}


void *nmp_get_sysctl_value(NmpSCID id)
{
	return __get_sysctl_value(&nmp_mss_ctl, id);
}


/*
static __inline__ void
nmp_sysctl_ports_init(rw_file *conf)
{
	gint set_ok = 0;
	gchar *range, *ex;
	gint lower = 0, upper = 0, index = 0;

	range = (gchar*)get_value_of(
		conf,
		"section.mds",
		0, 
		"stream_ports_range"
	);

	if (range && sscanf(range, "%d,%d", &lower, &upper) == 2)
	{
		if (lower < upper && lower > 0)
		{
			set_ok = 1;
			nmp_media_ports_set_range(lower, upper);
		}
	}

	if (!set_ok)
	{
		nmp_media_ports_set_default_range();
		return;
	}

	while ((ex = (gchar*)get_value_of(conf, "section.mds", index, 
		"stream_ports_unexpected")))
	{
		nmp_media_ports_set_reserved(atoi(ex));
		++index;
	}
}
*/

static __inline__ void
__nmp_sysctl_init(NmpSysCtl *sc, rw_file *conf)
{
	nmp_set_sysctl_value(
		sc,
		SC_BASE_PORT,
		(gchar*)get_value_of(conf, "section.global", 0, "base_port")
	);

	nmp_set_sysctl_value(
		sc,
		SC_MAX_LOG_SIZE,
		(gchar*)get_value_of(conf, "section.global", 0, "log_max_size")
	);

	nmp_set_sysctl_value(
		sc,
		SC_LOG_PATH,
		(gchar*)get_value_of(conf, "section.global", 0, "log_dir")
	);

	nmp_set_sysctl_value(
		sc,
		SC_CMS_HOST,
		(gchar*)get_value_of(conf, "section.global", 0, "cms_host")
	);

	nmp_set_sysctl_value(
		sc,
		SC_TIMEZONE,
		(gchar*)get_value_of(conf, "section.global", 0, "time_zone")
	);

	nmp_set_sysctl_value(
		sc,
		SC_MSS_ID,
		(gchar*)get_value_of(conf, "section.mss", 0, "mss_id")
	);

	nmp_set_sysctl_value(
		sc,
		SC_STORAGE_TYPE,
		(gchar*)get_value_of(conf, "section.mss", 0, "storage_type")
	);

//	nmp_sysctl_ports_init(conf);
}


static __inline__ void
_nmp_sysctl_init(NmpSysCtl *sc, const gchar *env_name)
{
	gchar *path, file_name[MAX_FILE_PATH];
	rw_file *rw_f;
	gint err;

	path = getenv(env_name);
	if (!path)
		path = SC_DEFAULT_CONFIG_PATH;

	if (strlen(path) + strlen("/") + strlen(SC_CONFIG_FILE_NAME)
		>= MAX_FILE_PATH)
	{
		nmp_warning(
			"Mss open 'nmp_server.conf' failed, path too long."
		);
//		nmp_media_ports_set_default_range();
		return;		/* use default */
	}

	strcpy(file_name, path);
	strcat(file_name, "/");
	strcat(file_name, SC_CONFIG_FILE_NAME);

	rw_f = open_rw_file(file_name, 0, &err);
	if (!rw_f)
	{
		nmp_warning(
			"Mss open 'nmp_server.conf' failed, err: %d.", err
		);
//		nmp_media_ports_set_default_range();
		return;		/* use default */
	}

	__nmp_sysctl_init(sc, rw_f);
	close_rw_file(rw_f);
}


void
nmp_sysctl_init( void )
{
	_nmp_sysctl_init(&nmp_mss_ctl, SC_CONFIG_ENV_NAME);
}

//:~ End
