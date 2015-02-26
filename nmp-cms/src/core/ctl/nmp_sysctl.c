#include <string.h>
#include <stdlib.h>
#include <nmp_rw_file.h>
#include <syslog.h>
#include <fcntl.h>
#include <errno.h>
#include <libgen.h>
#include "nmp_sysctl.h"
#include "nmp_debug.h"


#define SC_PORT_LEN                             32

#define SC_CONFIG_ENV_NAME		"CMS_CONFIG_PATH"
#define SC_DEFAULT_CONFIG_PATH	 "/etc/"
#define SC_CONFIG_FILE_NAME		"jxj_server.conf"
#define SC_PLATFORM_UPGRADE_SCRIPT  "upgrade-platform-system"

JpfSysCtl jpf_cms_params =
{
	.base_port			= 9902,
	.max_log_file_size	= 50,
	.wait_after_connected = 3,
	.db_min_conn_num = 5,
	.db_max_conn_num = 100,
	.log_file_path = "./log",
	.db_host = "127.0.0.1",
	.db_name = "jxj_platform_db",
	.db_user_name = "root",
	.db_user_password = "",
	.db_backup_path = "/home",
	.my_cnf_path = "/etc/my.cnf",
	.decoder_path = "./var/decoder",
	.map_path = "/tmp",
	.time_zone = "GMT-8",
	.sre_type = "general"
};

void __jpf_set_sys_parm_int(JpfSysCtl *sc, gint id, gint value)
{
    switch(id)
    {
    case SYS_PARM_BASEPORT:
	if (value > 0)
    	    sc->base_port = value;
    	break;
    case SYS_PARM_LOGFILESIZE:
	if (value >= 1)
           sc->max_log_file_size = value;
       break;
    case SYS_PARM_WAIT_AFTER_CONNECTED:
	if (value >= 1)
           sc->wait_after_connected = value;
       break;
    case SYS_PARM_DBMINCONNNUM:
	if (value >= 1)
           sc->db_min_conn_num= value;
       break;
    case SYS_PARM_DBMAXCONNNUM:
	if (value >= 1)
           sc->db_max_conn_num= value;
       break;
    default:
      	BUG();
    	break;
    }

}

void jpf_set_sys_parm_int(gint id, gint value)
{
	__jpf_set_sys_parm_int(&jpf_cms_params, id, value);
}


gint __jpf_get_sys_parm_int(JpfSysCtl *sc, gint id)
{
    switch(id)
    {
    case SYS_PARM_BASEPORT:
    	return sc->base_port;
    	break;
    case SYS_PARM_LOGFILESIZE:
       return sc->max_log_file_size;
       break;
    case SYS_PARM_WAIT_AFTER_CONNECTED:
       return sc->wait_after_connected;
       break;
    case SYS_PARM_DBMINCONNNUM:
       return sc->db_min_conn_num;
       break;
    case SYS_PARM_DBMAXCONNNUM:
       return sc->db_max_conn_num;
       break;
    default:
    	BUG();
    	break;
    }

    return 0;
}


gint jpf_get_sys_parm_int(gint id)
{
    return __jpf_get_sys_parm_int(&jpf_cms_params, id);
}


void __jpf_set_sys_parm_str(JpfSysCtl *sc, gint id, gchar *value)
{
	 switch(id)
    {
	case SYS_PARM_LOGFILEPATH:
		   if (value)
		       strncpy(sc->log_file_path, value, FILE_PATH_LEN - 1);
		   break;
       case SYS_PARM_DBHOST:
		   if (value)
		       strncpy(sc->db_host, value, HOST_NAME_LEN - 1);
		   break;
	case SYS_PARM_DBNAME:
		   if (value)
		       strncpy(sc->db_name, value, DB_NAME_LEN - 1);
			break;
	case SYS_PARM_DBADMINNAME:
		   if (value)
		       strncpy(sc->db_user_name, value, ADMIN_NAME_LEN - 1);
			break;
	case SYS_PARM_DBADMINPASSWORD:
		   if (value)
		   {
		       strncpy(sc->db_user_password, value, PASSWD_LEN - 1);
		   }
		  else
		      strncpy(sc->db_user_password, "", PASSWD_LEN - 1);
		      break;
	case SYS_PARM_DBBACKUPPATH:
		   if (value)
		       strncpy(sc->db_backup_path, value, FILE_PATH_LEN - 1);
			break;
	case SYS_PARM_MYCNFPATH:
		   if (value)
		       strncpy(sc->my_cnf_path, value, FILE_PATH_LEN - 1);
			break;
	case SYS_PARM_DECODERPATH:
		   if (value)
		       strncpy(sc->decoder_path, value, FILE_PATH_LEN - 1);
			break;
	case SYS_PARM_MAPPATH:
		   if (value)
		       strncpy(sc->map_path, value, FILE_PATH_LEN - 1);
			break;
	case SYS_PARM_TIMEZONE:
		   if (value)
		       strncpy(sc->time_zone, value, TIME_ZONE_LEN - 1);
			break;
	case SYS_PARM_SRETYPE:
		   if (value)
		   	strncpy(sc->sre_type, value, SRE_NAME_LEN - 1);
		   break;
    default:
    	BUG();
			break;
    }
}


void jpf_set_sys_parm_str(gint id, gchar *value)
{
	__jpf_set_sys_parm_str(&jpf_cms_params, id, value);
}


gchar* __jpf_get_sys_parm_str(JpfSysCtl *sc, gint id)
{
    switch(id)
    {
	case SYS_PARM_LOGFILEPATH:
		   return sc->log_file_path;
		   break;
       case SYS_PARM_DBHOST:
		   return sc->db_host;
		   break;
	case SYS_PARM_DBNAME:
		   return sc->db_name;
		   break;
	case SYS_PARM_DBADMINNAME:
		   return sc->db_user_name;
		   break;
	case SYS_PARM_DBADMINPASSWORD:
		   return sc->db_user_password;
		   break;
	case SYS_PARM_DBBACKUPPATH:
		   return sc->db_backup_path;
		   break;
	case SYS_PARM_MYCNFPATH:
		   return sc->my_cnf_path;
		   break;
	case SYS_PARM_DECODERPATH:
		   return sc->decoder_path;
		   break;
	case SYS_PARM_MAPPATH:
		   return sc->map_path;
		   break;
	case SYS_PARM_TIMEZONE:
		   return sc->time_zone;
		   break;
	case SYS_PARM_SRETYPE:
		   return sc->sre_type;
		   break;
    default:
    	BUG();
			break;
    }
	 return NULL;
}

gchar* jpf_get_sys_parm_str(gint id)
{
    return __jpf_get_sys_parm_str(&jpf_cms_params, id);
}


static __inline__ void
__jpf_sysctl_init(rw_file *conf)
{
	if (get_value_of(conf, "section.global", 0, "base_port"))
	{
		jpf_set_sys_parm_int(SYS_PARM_BASEPORT,
		atoi(get_value_of(conf, "section.global", 0, "base_port")));
	}

	if (get_value_of(conf, "section.global", 0, "log_max_size"))
	{
		jpf_set_sys_parm_int(SYS_PARM_LOGFILESIZE,
		atoi(get_value_of(conf, "section.global", 0, "log_max_size")));
	}

	jpf_set_sys_parm_str(SYS_PARM_LOGFILEPATH,
		(gchar*)get_value_of(conf, "section.global", 0, "log_dir"));

	jpf_set_sys_parm_str(SYS_PARM_DECODERPATH,
		(gchar*)get_value_of(conf, "section.global", 0, "decoder_dir"));

	jpf_set_sys_parm_str(SYS_PARM_MAPPATH,
		(gchar*)get_value_of(conf, "section.global", 0, "map_dir"));

	jpf_set_sys_parm_str(SYS_PARM_TIMEZONE,
		(gchar*)get_value_of(conf, "section.global", 0, "time_zone"));

	if (get_value_of(conf, "section.cms", 0, "db_min_conn_num"))
	{
		jpf_set_sys_parm_int(SYS_PARM_DBMINCONNNUM,
		atoi(get_value_of(conf, "section.cms", 0, "db_min_conn_num")));
	}

	if (get_value_of(conf, "section.cms", 0, "db_max_conn_num"))
	{
		jpf_set_sys_parm_int(SYS_PARM_DBMAXCONNNUM,
		atoi(get_value_of(conf, "section.cms", 0, "db_max_conn_num")));
	}

	if (get_value_of(conf, "section.cms", 0, "wait_after_connected"))
	{
		jpf_set_sys_parm_int(SYS_PARM_WAIT_AFTER_CONNECTED,
		atoi(get_value_of(conf, "section.cms", 0, "wait_after_connected")));
	}

	jpf_set_sys_parm_str(SYS_PARM_DBHOST,
		(gchar*)get_value_of(conf, "section.cms", 0, "db_host"));

	jpf_set_sys_parm_str(SYS_PARM_DBNAME,
		(gchar*)get_value_of(conf, "section.cms", 0, "db_name"));

	jpf_set_sys_parm_str(SYS_PARM_DBADMINNAME,
		(gchar*)get_value_of(conf, "section.cms", 0, "db_user_name"));

	jpf_set_sys_parm_str(SYS_PARM_DBADMINPASSWORD,
		(gchar*)get_value_of(conf, "section.cms", 0, "db_user_password"));

	jpf_set_sys_parm_str(SYS_PARM_DBBACKUPPATH,
		(gchar*)get_value_of(conf, "section.cms", 0, "db_backup_path"));

	jpf_set_sys_parm_str(SYS_PARM_MYCNFPATH,
		(gchar*)get_value_of(conf, "section.cms", 0, "my_cnf_path"));

	jpf_set_sys_parm_str(SYS_PARM_SRETYPE,
		(gchar*)get_value_of(conf, "section.global", 0, "sre"));
}


static __inline__ void
_jpf_sysctl_init(JpfSysCtl *sc, const gchar *env_name)
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
		jpf_warning(
			"Cms open 'jxj_server.conf' failed, path too long."
		);
		return;		/* use default */
	}

	strcpy(file_name, path);
	strcat(file_name, "/");
	strcat(file_name, SC_CONFIG_FILE_NAME);

	rw_f = open_rw_file(file_name, 0, &err);
	if (!rw_f)
	{
		jpf_warning(
			"Cms open 'jxj_server.conf' failed, err: %d.", err
		);
		return;		/* use default */
	}

	__jpf_sysctl_init(rw_f);
	close_rw_file(rw_f);
}


void
jpf_sysctl_init( void )
{
	_jpf_sysctl_init(&jpf_cms_params, SC_CONFIG_ENV_NAME);
}


static __inline__ rw_file *
_jpf_get_rw_file(const gchar *env_name)
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
		jpf_warning(
			"Cms open 'jxj_server.conf' failed, path too long."
		);
		return NULL;		/* use default */
	}

	strcpy(file_name, path);
	strcat(file_name, "/");
	strcat(file_name, SC_CONFIG_FILE_NAME);

	rw_f = open_rw_file(file_name, O_RDONLY, &err);
	if (!rw_f)
	{
		jpf_warning(
			"Cms open 'jxj_server.conf' failed, err: %d.", err
		);
		return NULL;		/* use default */
	}

	return rw_f;
}


static __inline__ gint
__jpf_get_script(const gchar *script_name, gchar script_path[])
{
	gchar *path, *tmp_path;
	gchar file_path[MAX_FILE_PATH], sct_path[MAX_FILE_PATH];
	gint err;

	path = getenv(SC_CONFIG_ENV_NAME);
	if (!path)
		return -ENOENT;

	strncpy(sct_path, path, MAX_FILE_PATH - 1);
	sct_path[MAX_FILE_PATH - 1] = 0;
	tmp_path = dirname(sct_path);
	if (!tmp_path)
	    return -ENOENT;

	strcpy(file_path, tmp_path);
	if (strlen(tmp_path) + strlen("/") + strlen(script_name)
		>= MAX_FILE_PATH)
	{
		return -ENAMETOOLONG;		/* use default */
	}

	strcat(file_path, "/");
	strcat(file_path, script_name);
       err = access(file_path, F_OK|X_OK);
	if (err)
	{
	    jpf_warning(
               "access(%s) failed.", file_path
	    );
	    return -errno;
	}

	strcpy(script_path, file_path);
	return 0;
}


gint
jpf_get_platform_upgrade_script(gchar script_path[])
{
       return __jpf_get_script(SC_PLATFORM_UPGRADE_SCRIPT, script_path);
}



static __inline__ void __jpf_get_mds_parm(rw_file *conf, JpfMdsCtl *gc)
{
	gchar ports_range[SC_PORT_LEN] = {0};

	if(get_value_of(conf, "section.mds", 0, "mds_id"))
		strncpy(gc->mds_id, get_value_of(conf, "section.mds", 0, "mds_id"), MDS_ID_LEN - 1);

	if(get_value_of(conf, "section.mds", 0, "stream_ports_range"))
	{
		strncpy(ports_range, get_value_of(conf, "section.mds", 0, "stream_ports_range"), SC_PORT_LEN - 1);
		sscanf(ports_range, "%d, %d", &gc->start_port, &gc->end_port);
	}
}

static __inline__ void _jpf_get_mds_parm(JpfMdsCtl *gc)
{
	rw_file *rw_f;

	rw_f = _jpf_get_rw_file(SC_CONFIG_ENV_NAME);
	if (!rw_f)
	{
		return;		/* use default */
	}

	__jpf_get_mds_parm(rw_f, gc);
	close_rw_file(rw_f);
}

void jpf_get_mds_parm(JpfMdsCtl *gc)
{
	_jpf_get_mds_parm(gc);
}


static __inline__ void __jpf_set_mds_parm(rw_file *conf, JpfMdsCtl gc)
{
	int ret;
	gchar ports_range[SC_PORT_LEN] = {0};

	del_value_of(conf, "section.mds", "mds_id");
	del_value_of(conf, "section.mds", "stream_ports_range");

	ret = set_value_of(conf, "section.mds", "mds_id", gc.mds_id);
	if (ret)
		jpf_warning("set section.mds mds_id failed");

	snprintf(ports_range, SC_PORT_LEN, "%d, %d", gc.start_port, gc.end_port);
	//printf("--set mds ports range=%s\n", ports_range);
	ret = set_value_of(conf, "section.mds", "stream_ports_range", ports_range);
	if (ret)
		jpf_warning("set section.mds stream_ports_range failed");
}

static __inline__ void _jpf_set_mds_parm(JpfMdsCtl gc)
{
	rw_file *rw_f;

	rw_f = _jpf_get_rw_file(SC_CONFIG_ENV_NAME);
	if (!rw_f)
	{
		return;		/* use default */
	}

	__jpf_set_mds_parm(rw_f, gc);
	flush_rw_file(rw_f);
	close_rw_file(rw_f);
}

void jpf_set_mds_parm(JpfMdsCtl gc)
{
	_jpf_set_mds_parm(gc);
}


static __inline__ void __jpf_get_mss_parm(rw_file *conf, JpfMssCtl *gc)
{
    if (get_value_of(conf, "section.mss", 0, "mss_id"))
        strncpy(gc->mss_id, get_value_of(conf, "section.mss", 0, "mss_id"), MSS_ID_LEN - 1);
    if (get_value_of(conf, "section.mss", 0, "storage_type"))
        strncpy(gc->stor_type, get_value_of(conf, "section.mss", 0, "storage_type"),
        MAX_STOR_TYPE_LEN - 1);
}

static __inline__ void _jpf_get_mss_parm(JpfMssCtl *gc)
{
	rw_file *rw_f;

	rw_f = _jpf_get_rw_file(SC_CONFIG_ENV_NAME);
	if (!rw_f)
	{
		return;		/* use default */
	}

	__jpf_get_mss_parm(rw_f, gc);
	close_rw_file(rw_f);
}

void jpf_get_mss_parm(JpfMssCtl *gc)
{
     _jpf_get_mss_parm(gc);
}


static __inline__ void __jpf_set_mss_parm(rw_file *conf, JpfMssCtl gc)
{
    int ret;

    del_value_of(conf, "section.mss", "mss_id");
    del_value_of(conf, "section.mss", "storage_type");

    ret = set_value_of(conf, "section.mss", "mss_id", gc.mss_id);
    if (ret)
	jpf_warning("set section.mss mss_id failed");

    ret = set_value_of(conf, "section.mss", "storage_type", gc.stor_type);
    if (ret)
	jpf_warning("set section.mss storage_type failed");

}

static __inline__ void _jpf_set_mss_parm(JpfMssCtl gc)
{
	rw_file *rw_f;

	rw_f = _jpf_get_rw_file(SC_CONFIG_ENV_NAME);
	if (!rw_f)
	{
		return;		/* use default */
	}

	__jpf_set_mss_parm(rw_f, gc);
	flush_rw_file(rw_f);
	close_rw_file(rw_f);
}

void jpf_set_mss_parm(JpfMssCtl gc)
{
     _jpf_set_mss_parm(gc);
}


static __inline__ void __jpf_get_ivs_parm(rw_file *conf, JpfIvsCtl *gc)
{
	if (get_value_of(conf, "section.ivs", 0, "ivs_id"))
		strncpy(gc->ivs_id, get_value_of(conf, "section.ivs", 0, "ivs_id"), IVS_ID_LEN - 1);
}

static __inline__ void _jpf_get_ivs_parm(JpfIvsCtl *gc)
{
	rw_file *rw_f;

	rw_f = _jpf_get_rw_file(SC_CONFIG_ENV_NAME);
	if (!rw_f)
	{
		return;		/* use default */
	}

	__jpf_get_ivs_parm(rw_f, gc);
	close_rw_file(rw_f);
}

void jpf_get_ivs_parm(JpfIvsCtl *gc)
{
     _jpf_get_ivs_parm(gc);
}


static __inline__ void __jpf_set_ivs_parm(rw_file *conf, JpfIvsCtl gc)
{
	int ret;

	del_value_of(conf, "section.ivs", "ivs_id");

	ret = set_value_of(conf, "section.ivs", "ivs_id", gc.ivs_id);
	if (ret)
		jpf_warning("set section.ivs ivs_id failed");
}

static __inline__ void _jpf_set_ivs_parm(JpfIvsCtl gc)
{
	rw_file *rw_f;

	rw_f = _jpf_get_rw_file(SC_CONFIG_ENV_NAME);
	if (!rw_f)
	{
		return;		/* use default */
	}

	__jpf_set_ivs_parm(rw_f, gc);
	flush_rw_file(rw_f);
	close_rw_file(rw_f);
}

void jpf_set_ivs_parm(JpfIvsCtl gc)
{
     _jpf_set_ivs_parm(gc);
}


//:~ End
