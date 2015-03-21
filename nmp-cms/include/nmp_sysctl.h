#ifndef __NMP_SYSCTL_H__
#define __NMP_SYSCTL_H__

#include <glib.h>
#include "nmp_msg_share.h"

#define HOST_NAME_LEN			16
#define DB_NAME_LEN			32
#define ADMIN_NAME_LEN			16
#define PASSWD_LEN				16
#define FILE_PATH_LEN			256
#define MAX_FILE_PATH			4096		/* sc_ */
#define SC_MAX_LOG_PATH		1024
#define SC_MAX_ID_LEN			64
#define SC_MAX_HOST_LEN		64

#define SYS_PARM_BASEPORT						0
#define SYS_PARM_LOGFILESIZE					1
#define SYS_PARM_LOGFILEPATH      			 	2
#define SYS_PARM_WAIT_AFTER_CONNECTED		3
#define SYS_PARM_DBMINCONNNUM				4
#define SYS_PARM_DBMAXCONNNUM				5
#define SYS_PARM_DBHOST					6
#define SYS_PARM_DBNAME 					7
#define SYS_PARM_DBADMINNAME				8
#define SYS_PARM_DBADMINPASSWORD		9
#define SYS_PARM_DBBACKUPPATH			10
#define SYS_PARM_MYCNFPATH				11
#define SYS_PARM_DECODERPATH				12
#define SYS_PARM_MAPPATH					13
#define SYS_PARM_AVMAXNUM				14
#define SYS_PARM_DSMAXNUM				15
#define SYS_PARM_AIMAXNUM					16
#define SYS_PARM_AOMAXNUM				17
#define SYS_PARM_TIMEZONE					18
#define SYS_PARM_SRETYPE					19

typedef struct _JpfSysCtl JpfSysCtl;

struct _JpfSysCtl
{
    gint		 base_port;			/* Platform base port */
    gint		 max_log_file_size;	/* Top log file size limit (MB) */
    gint      wait_after_connected;
    gint      db_min_conn_num;
    gint      db_max_conn_num;
    gchar   log_file_path[FILE_PATH_LEN];
    gchar   db_host[HOST_NAME_LEN];
    gchar   db_name[DB_NAME_LEN];
    gchar   db_user_name[ADMIN_NAME_LEN];
    gchar   db_user_password[PASSWD_LEN];
    gchar   db_backup_path[FILE_PATH_LEN];
    gchar   my_cnf_path[FILE_PATH_LEN];
    gchar   decoder_path[FILE_PATH_LEN];
    gchar   map_path[FILE_PATH_LEN];
    gchar   time_zone[TIME_ZONE_LEN];
    gchar   sre_type[SRE_NAME_LEN];
};


typedef struct _JpfMdsCtl JpfMdsCtl;
struct _JpfMdsCtl
{
	gchar	mds_id[MDS_ID_LEN];
	gint		start_port;
	gint		end_port;
};


typedef struct _JpfMssCtl JpfMssCtl;
struct _JpfMssCtl
{
	gchar   mss_id[MSS_ID_LEN];
	gchar   stor_type[MAX_STOR_TYPE_LEN];
};


typedef struct _JpfIvsCtl JpfIvsCtl;
struct _JpfIvsCtl
{
	gchar	ivs_id[IVS_ID_LEN];
};


void jpf_set_sys_parm_int(gint id, gint value);
gint jpf_get_sys_parm_int(gint id);
void jpf_set_sys_parm_str(gint id, gchar *value);
gchar* jpf_get_sys_parm_str(gint id);
void jpf_sysctl_init( void );
gint jpf_get_platform_upgrade_script(gchar script_path[]);
extern JpfSysCtl	nmp_cms_params;

void jpf_get_mds_parm(JpfMdsCtl *gc);
void jpf_set_mds_parm(JpfMdsCtl gc);
void jpf_get_mss_parm(JpfMssCtl *gc);
void jpf_set_mss_parm(JpfMssCtl gc);
void jpf_get_ivs_parm(JpfIvsCtl *gc);
void jpf_set_ivs_parm(JpfIvsCtl gc);


#endif	//__NMP_SYSCTL_H__
