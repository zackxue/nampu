#include "nmp_mod_log.h"
#include "nmp_debug.h"
#include "nmp_errno.h"
#include "nmp_ports.h"
#include "nmp_mods.h"
#include "db_connect_pool.h"
#include "nmp_shared.h"
#include "nmp_message.h"
#include "nmp_internal_msg.h"


static NmpModLog *g_nmp_mod_log;

LOG_MSG_ENABLE g_log_msg_enable[LOG_MAX_MESSAGE_NUM];

G_DEFINE_TYPE(NmpModLog, nmp_mod_log, NMP_TYPE_APPMOD);

void
nmp_mod_log_register_msg_handler(NmpModLog *self);


#define CHECK_LOG_TIME		(1000 * 60)	//60 s


gint jpf_log_check_hook(NmpSysMsg *msg)
{
	NmpMsgID msg_id = MSG_GETID(msg);
	guint flag;
	G_ASSERT(msg_id >= 0 && msg_id < LOG_MAX_MESSAGE_NUM);

	if (!g_log_msg_enable[msg_id].enable)
		return 1;

	flag = g_log_msg_enable[msg_id].flag;
	if (MSG_RESPONSE(msg) && (flag & LOG_MSG_RES))
		return 0;

	if (!MSG_RESPONSE(msg) && (flag & LOG_MSG_REQ))
		return 0;

	return 1;
}


static db_conn_pool_conf *
jpf_log_init_mysql_conf()
{
	db_conn_pool_conf *pool_conf;

	pool_conf = g_new0(db_conn_pool_conf, 1);
	if (G_UNLIKELY(!pool_conf))
		return pool_conf;

	SET_DB_CONN_MIN_NUM(pool_conf,  jpf_get_sys_parm_int(SYS_PARM_DBMINCONNNUM));
	SET_DB_CONN_MAX_NUM(pool_conf,  jpf_get_sys_parm_int(SYS_PARM_DBMAXCONNNUM));
	strncpy(pool_conf->host, jpf_get_sys_parm_str(SYS_PARM_DBHOST), HOST_NAME_LEN - 1);
	strncpy(pool_conf->db_name, jpf_get_sys_parm_str(SYS_PARM_DBNAME), DB_NAME_LEN - 1);
	//printf("---------db name :%s--%s\n",pool_conf->db_name, jpf_get_sys_parm_str(SYS_PARM_DBNAME));
	strncpy(pool_conf->user_name, jpf_get_sys_parm_str(SYS_PARM_DBADMINNAME), ADMIN_NAME_LEN - 1);
	strncpy(pool_conf->user_password, jpf_get_sys_parm_str(SYS_PARM_DBADMINPASSWORD), PASSWD_LEN - 1);
	strncpy(pool_conf->my_cnf_path, jpf_get_sys_parm_str(SYS_PARM_MYCNFPATH), FILENAME_LEN- 1);

	return pool_conf;
}


gint
nmp_mod_log_setup(NmpAppMod *am_self)
{
	NmpModLog *self;
	G_ASSERT(am_self != NULL);

	self = (NmpModLog*)am_self;

	nmp_app_mod_set_name(am_self, "MOD-LOG");
	nmp_mod_log_register_msg_handler(self);
	jpf_msg_bus_add_msg_hook(BUSSLOT_POS_LOG, jpf_log_check_hook);
	return 0;
}


static gboolean
nmp_mod_log_del_log_timer(gpointer user_data)
{
	NmpModLog * self = (NmpModLog *)user_data;

	self->del_log_flag = 1;

	return TRUE;
}


static void
nmp_mod_log_init(NmpModLog *self)
{
	g_nmp_mod_log = self;

	self->pool_conf = jpf_log_init_mysql_conf();
	if (G_UNLIKELY(!self->pool_conf))
		jpf_error("out of memory");

	self->pool_info = g_new0(db_conn_pool_info, 1);
	if (G_UNLIKELY(!self->pool_info))
		jpf_error("out of memory");

	self->del_log_flag = 0;
	init_db_conn_pool(self->pool_info, self->pool_conf);
	jpf_set_timer(CHECK_LOG_TIME, nmp_mod_log_del_log_timer, self);
}


static void
nmp_mod_log_class_init(NmpModLogClass *k_class)
{
	NmpAppModClass *am_class = (NmpAppModClass*)k_class;

	am_class->setup_mod	= nmp_mod_log_setup;
}

