#include "nmp_debug.h"
#include "nmp_errno.h"
#include "nmp_ports.h"
#include "nmp_mod_dbs.h"
#include "nmp_mods.h"
#include "db_connect_pool.h"
#include "nmp_mysql_fun.h"
#include "nmp_shared.h"
#include "nmp_message.h"
#include "nmp_internal_msg.h"

G_DEFINE_TYPE(JpfModDbs, jpf_mod_dbs, JPF_TYPE_APPMOD);

void jpf_mod_dbs_register_msg_handler(JpfModDbs *self);
void jpf_mod_dbs_register_pu_msg_handler(JpfModDbs *self);
void	jpf_mod_dbs_register_mds_msg_handler(JpfModDbs *self);
void	jpf_mod_dbs_register_mss_msg_handler(JpfModDbs *self);
void jpf_mod_dbs_register_tw_msg_handler(JpfModDbs *self);
void jpf_mod_dbs_register_ams_msg_handler(JpfModDbs *self);
void jpf_mod_dbs_register_wdd_msg_handler(JpfModDbs *self);

#define CHECK_ALARM_TIME      (1000*60)   //60 s
#define EPSINON  0.00001
//static guint msg_seq_generator = 0;


db_conn_pool_conf *
jpf_init_mysql_conf()
{
	db_conn_pool_conf *pool_conf;

	pool_conf = g_new0(db_conn_pool_conf, 1);
	if (G_UNLIKELY(!pool_conf))
		return pool_conf;

   /* SET_CONN_POOL_HOST(pool_conf->host, MYSQL_HOST);
    SET_DB_NAME(pool_conf, DB_NAME);
    SET_DB_USER(pool_conf, DB_ADMIN_NAME);
    SET_DB_PASSWORD(pool_conf, DB_ADMIN_PASSWORD); */

    SET_DB_CONN_MIN_NUM(pool_conf,  jpf_get_sys_parm_int(SYS_PARM_DBMINCONNNUM));
    SET_DB_CONN_MAX_NUM(pool_conf,  jpf_get_sys_parm_int(SYS_PARM_DBMAXCONNNUM));
    strncpy(pool_conf->host, jpf_get_sys_parm_str(SYS_PARM_DBHOST), HOST_NAME_LEN - 1);
    strncpy(pool_conf->db_name, jpf_get_sys_parm_str(SYS_PARM_DBNAME), DB_NAME_LEN - 1);
    printf("---------db name :%s--%s\n",pool_conf->db_name, jpf_get_sys_parm_str(SYS_PARM_DBNAME));
    strncpy(pool_conf->user_name, jpf_get_sys_parm_str(SYS_PARM_DBADMINNAME), ADMIN_NAME_LEN - 1);
    strncpy(pool_conf->user_password, jpf_get_sys_parm_str(SYS_PARM_DBADMINPASSWORD), PASSWD_LEN - 1);
    strncpy(pool_conf->my_cnf_path, jpf_get_sys_parm_str(SYS_PARM_MYCNFPATH), FILENAME_LEN- 1);
    return pool_conf;
}


static __inline__ gint
jpf_get_domain_id(JpfMysqlRes *result)
{
    gint j, field_num;
    JpfMysqlRow mysql_row;
    JpfMysqlField* mysql_fields;
    gchar *name;
    gchar *value;

    field_num = jpf_sql_get_num_fields(result);
    if (field_num == 0)
        return -E_NODBENT;
    mysql_row = jpf_sql_fetch_row(result);
    if (!mysql_row)
	return -E_NODBENT;
    //while ((mysql_row = jpf_sql_fetch_row(result)))
    {
        jpf_sql_field_seek(result, 0);
        mysql_fields = jpf_sql_fetch_fields(result);

        for (j = 0; j < field_num; j++)
        {
            name = jpf_sql_get_field_name(mysql_fields, j);
            if (!strcmp(name, "dm_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, j);
                jpf_set_domain_id(value);
		  return 0;
            }
            else
                jpf_warning("no need mysql name %s ", name);
        }
    }

    return 0;
}


static __inline__ void
jpf_mod_dbs_del_mysql_bin(JpfAppMod *am_self)
{
	gchar query_buf[QUERY_STR_LEN];
	JpfAppObj *app_obj = (JpfAppObj *)am_self;
	JpfMsgErrCode mysql_result;
	glong affect_num = 0;

	snprintf(query_buf, QUERY_STR_LEN, "reset master");

	memset(&mysql_result, 0, sizeof(mysql_result));
	jpf_dbs_do_query_code(app_obj, NULL, query_buf, &mysql_result, &affect_num);
}


static __inline__ gint
jpf_mod_dbs_get_domain_id(JpfAppMod *am_self)
{
    gchar query_buf[QUERY_STR_LEN];
    JpfAppObj *app_obj = (JpfAppObj *)am_self;
    JpfMysqlRes *mysql_result;
    gint res, row_num;

    snprintf(
        query_buf,  QUERY_STR_LEN,
         "select dm_id from domain_table where dm_type=%d",0
     );
    mysql_result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!mysql_result);

    if (G_UNLIKELY(MYSQL_RESULT_CODE(mysql_result)))
    {
        res = MYSQL_RESULT_CODE(mysql_result);
        goto get_domain_id_failed;
    }

    row_num = jpf_sql_get_num_rows(mysql_result);
    if (G_LIKELY(row_num != 0))
        res = jpf_get_domain_id(mysql_result);
    else
        res = -E_NODBENT;

get_domain_id_failed:
    if (G_LIKELY(mysql_result))
        jpf_sql_put_res(mysql_result, sizeof(JpfMysqlRes));

    return res;
}


static __inline__ void
jpf_clear_pu_state(JpfAppMod *am_self)
{
    gchar query_buf[QUERY_STR_LEN];
    JpfAppObj *app_obj = (JpfAppObj *)am_self;
    JpfMsgErrCode mysql_result;
    glong affect_num = 0;

    snprintf(
       	query_buf, QUERY_STR_LEN,
       	"update %s set pu_state=0,pu_registered=0",
       	PU_RUNNING_TABLE
    );

    memset(&mysql_result, 0, sizeof(mysql_result));
    jpf_dbs_do_query_code(app_obj, NULL, query_buf, &mysql_result, &affect_num);
    if (RES_CODE(&mysql_result) != -292)
    	BUG_ON(RES_CODE(&mysql_result));
}


static __inline__ void
jpf_clear_mss_state(JpfAppMod *am_self)
{
    gchar query_buf[QUERY_STR_LEN];
    JpfAppObj *app_obj = (JpfAppObj *)am_self;
    JpfMsgErrCode mysql_result;
    glong affect_num = 0;

    snprintf(
       	query_buf, QUERY_STR_LEN,
       	"update %s set mss_state=0",
       	MSS_TABLE
    );

    memset(&mysql_result, 0, sizeof(mysql_result));
    jpf_dbs_do_query_code(app_obj, NULL, query_buf, &mysql_result, &affect_num);
}


static __inline__ void
jpf_clear_mds_state(JpfAppMod *am_self)
{
    gchar query_buf[QUERY_STR_LEN];
    JpfAppObj *app_obj = (JpfAppObj *)am_self;
    JpfMsgErrCode mysql_result;
    glong affect_num = 0;

    snprintf(
    	query_buf, QUERY_STR_LEN,
    	"update %s set mdu_state=0",
    	MDS_TABLE
    );

    memset(&mysql_result, 0, sizeof(mysql_result));
    jpf_dbs_do_query_code(app_obj, NULL, query_buf, &mysql_result, &affect_num);
}


static __inline__ void
jpf_clear_ams_state(JpfAppMod *am_self)
{
    gchar query_buf[QUERY_STR_LEN];
    JpfAppObj *app_obj = (JpfAppObj *)am_self;
    JpfMsgErrCode mysql_result;
    glong affect_num = 0;

    snprintf(
       	query_buf, QUERY_STR_LEN,
       	"update %s set ams_state=0",
       	AMS_TABLE
    );

    memset(&mysql_result, 0, sizeof(mysql_result));
    jpf_dbs_do_query_code(app_obj, NULL, query_buf, &mysql_result, &affect_num);
}


static __inline__ void
jpf_clear_ivs_state(JpfAppMod *am_self)
{
    gchar query_buf[QUERY_STR_LEN];
    JpfAppObj *app_obj = (JpfAppObj *)am_self;
    JpfMsgErrCode mysql_result;
    glong affect_num = 0;

    snprintf(
       	query_buf, QUERY_STR_LEN,
       	"update %s set ivs_state=0",
       	IVS_TABLE
    );

    memset(&mysql_result, 0, sizeof(mysql_result));
    jpf_dbs_do_query_code(app_obj, NULL, query_buf, &mysql_result, &affect_num);
}


static __inline__ void
jpf_check_area_dev_online_rate(JpfAppMod *am_self)
{
    gchar query_buf[QUERY_STR_LEN];
    JpfAppObj *app_obj = (JpfAppObj *)am_self;
    JpfMsgErrCode mysql_result;
    glong affect_num = 0;

    snprintf(
       	query_buf, QUERY_STR_LEN,
       	"call check_area_online_rate(1)"
    );

    memset(&mysql_result, 0, sizeof(mysql_result));
    jpf_dbs_do_query_code(app_obj, NULL, query_buf, &mysql_result, &affect_num);
}


static gboolean
jpf_mods_dbs_del_alarm_timer(gpointer user_data)
{
    JpfModDbs * self = (JpfModDbs *)user_data;

    self->del_alarm_flag = 1;

    return TRUE;
}


void
jpf_mods_dbs_broadcast_msg(JpfModDbs * self, gpointer priv, gsize size)
{
    JpfSysMsg *msg_notify;

    msg_notify = jpf_sysmsg_new_2(MESSAGE_BROADCAST_GENERAL_MSG,
        priv, size, ++msg_seq_generator);
    MSG_SET_DSTPOS(msg_notify, BUSSLOT_POS_CU);
    jpf_app_obj_deliver_out((JpfAppObj*)self, msg_notify);
}


static void
jpf_mod_dbs_init(JpfModDbs *self)
{
	JpfAppMod *a_self = ( JpfAppMod *)self;

	self->pool_conf = jpf_init_mysql_conf();
	if (G_UNLIKELY(!self->pool_conf))
		jpf_error("out of memory");

	self->pool_info = g_new0(db_conn_pool_info, 1);
	if (G_UNLIKELY(!self->pool_info))
		jpf_error("out of memory");

	init_db_conn_pool(self->pool_info, self->pool_conf);
	jpf_mod_dbs_del_mysql_bin(a_self);
	jpf_mod_dbs_get_domain_id(a_self);
	jpf_clear_pu_state(a_self);
	jpf_clear_mss_state(a_self);
	jpf_clear_mds_state(a_self);
	jpf_clear_ams_state(a_self);
	jpf_clear_ivs_state(a_self);
	jpf_check_area_dev_online_rate(a_self);
	jpf_mod_init_resource_cap();
	self->del_alarm_flag = 0;
	self->authorization_expired = 0;
	self->res_over_flag = 0;
	jpf_set_timer(CHECK_ALARM_TIME, jpf_mods_dbs_del_alarm_timer, self);
}


gint
jpf_mod_dbs_setup(JpfAppMod *am_self)
{
	G_ASSERT(am_self != NULL);

	JpfModDbs *self;

	self = (JpfModDbs*)am_self;

	jpf_app_mod_set_name(am_self, "MOD-DBS");
	jpf_mod_dbs_register_cu_msg_handler(self);
	jpf_mod_dbs_register_bss_msg_handler(self);
	jpf_mod_dbs_register_pu_msg_handler(self);
	jpf_mod_dbs_register_mds_msg_handler(self);
	jpf_mod_dbs_register_mss_msg_handler(self);
	jpf_mod_dbs_register_tw_msg_handler(self);
	jpf_mod_dbs_register_ams_msg_handler(self);
	jpf_mod_dbs_register_wdd_msg_handler(self);

	return 0;
}


static void
jpf_mod_dbs_class_init(JpfModDbsClass *k_class)
{
	JpfAppModClass *am_class = (JpfAppModClass*)k_class;

	am_class->setup_mod	= jpf_mod_dbs_setup;
}


