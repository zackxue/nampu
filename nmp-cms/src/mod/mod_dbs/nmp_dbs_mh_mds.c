#include "nmp_message.h"
#include "nmp_internal_msg.h"
#include "nmp_xml.h"
#include "nmp_sysmsg.h"
#include "nmp_mysql_fun.h"
#include "nmp_mod_dbs.h"
#include "nmp_dbs_fun.h"
#include "nmp_memory.h"
#include "nmp_debug.h"
#include "db_connect_pool.h"
#include "nmp_errno.h"
#include "message/nmp_msg_mds.h"
#include "nmp_shared.h"
#include "nmp_list_head.h"


extern gchar g_domain_id[DOMAIN_ID_LEN];
#define LOCAL_IP "127.0.0.1"

static __inline__ gint
jpf_get_mds_register_info(JpfMysqlRes *mysql_result, JpfMdsRegisterRes*res_info)
{
    G_ASSERT(mysql_result != NULL && res_info != NULL);

    unsigned long field_num;
    JpfMysqlField *mysql_fields;    //modify: mysql_fields -> *msql_fields
    JpfMysqlRow mysql_row;
    char *value, *name;
    int j,row_num;

    row_num = jpf_sql_get_num_rows(mysql_result);
    if (row_num == 0)
    {
        jpf_warning("<GetUserInfo>No such record entry in database");
        return -E_NODBENT;
    }
    else
    {
        field_num = jpf_sql_get_num_fields(mysql_result);
        while ((mysql_row = jpf_sql_fetch_row(mysql_result)))
        {
            jpf_sql_field_seek(mysql_result, 0);
            mysql_fields = jpf_sql_fetch_fields(mysql_result);

            for (j = 0; j < field_num; j++)
            {
                name = jpf_sql_get_field_name(mysql_fields, j);
                if (!strcmp(name,"mdu_keep_alive_freq"))
                {
                    value = jpf_sql_get_field_value(mysql_row, j);
                    res_info->keep_alive_time = atoi(value);
                }
		  else if (!strcmp(name,"mdu_pu_port"))
                {
                    value = jpf_sql_get_field_value(mysql_row, j);
                    res_info->pu_port = atoi(value);
                }
		  else if (!strcmp(name,"mdu_rtsp_port"))
                {
                    value = jpf_sql_get_field_value(mysql_row, j);
                    res_info->rtsp_port = atoi(value);
                }
                else if (!strcmp(name,"auto_get_ip_enable"))
                {
                    value = jpf_sql_get_field_value(mysql_row, j);
                    res_info->get_ip_enable = atoi(value);
                }
                else
                    cms_debug(
                         "Ignore table field %s while getting user info",name
                    );
            }
        }

         return 0;
    }
}


void jpf_insert_mds_ip(NmpAppObj *app_obj, gchar *mds_id,
	gchar *cms_ip, gchar *mds_ip)
{
    JpfMsgErrCode result;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    memset(&result, 0, sizeof(result));
    snprintf(
       query_buf, QUERY_STR_LEN, "insert ignore into %s values('%s','%s','%s')",
       MDS_IP_TABLE, mds_id, cms_ip, mds_ip
       );

    jpf_dbs_do_query_code(app_obj, NULL, query_buf, &result, &affect_num);

}


static void jpf_del_mds_ip(NmpAppObj *app_obj, gchar *mds_id)
{
    JpfMsgErrCode result;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    memset(&result, 0, sizeof(result));
    snprintf(
       query_buf, QUERY_STR_LEN, "delete from %s where mdu_id='%s'",
       MDS_IP_TABLE, mds_id
    );

    jpf_dbs_do_query_code(app_obj, NULL, query_buf, &result, &affect_num);
}


static void jpf_update_mds_ip(NmpAppObj *app_obj, gchar *mds_id,
	gchar *mds_ip, gchar *cms_ip, gint clear)
{
    JpfHostIps ips;
    gint index;

    if (clear)
    {
        jpf_del_mds_ip(app_obj, mds_id);
    }

    if (!strcmp(cms_ip, LOCAL_IP))
    {
        jpf_get_host_ips(&ips);
        for (index = 0; index < ips.count; index++)
        {
            jpf_insert_mds_ip(app_obj, mds_id, ips.ips[index].ip,
            ips.ips[index].ip);
        }
    }
    else
    {
        jpf_insert_mds_ip(app_obj, mds_id, cms_ip,
            mds_ip);
        jpf_insert_mds_ip(app_obj, mds_id, DEFALUT_CMS_IP,
            mds_ip);
    }
}

NmpMsgFunRet
jpf_dbs_mds_register_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);

 	JpfMdsRegister *req_info;
 	JpfMdsRegisterRes res_info;
 	JpfMysqlRes *mysql_res;
 	char query_buf[QUERY_STR_LEN];
 	int code = 0;

 	req_info = MSG_GET_DATA(msg);
 	BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    snprintf(
    	query_buf, QUERY_STR_LEN,
    	"select mdu_keep_alive_freq,mdu_pu_port,mdu_rtsp_port,auto_get_ip_enable from %s where mdu_id='%s'",
    	 MDS_TABLE,
    	req_info->mds_id
 	);

 	mysql_res = jpf_dbs_do_query_res(app_obj, query_buf);
 	BUG_ON(!mysql_res);
 	if (G_LIKELY(!MYSQL_RESULT_CODE(mysql_res)))  //success:0 fail:!0
 	{
 		code = jpf_get_mds_register_info(mysql_res, &res_info);
 		strcpy(res_info.mds_id, req_info->mds_id);
 	}
 	else
 		code = MYSQL_RESULT_CODE(mysql_res);

 	jpf_sql_put_res(mysql_res, sizeof(JpfMysqlRes));

	if (res_info.get_ip_enable)
	{
       	jpf_update_mds_ip(app_obj, req_info->mds_id, req_info->mds_ip,
       		req_info->cms_ip, 1);
 	}
 	SET_CODE(&res_info, code);
 	jpf_dbs_modify_sysmsg_2(msg, &res_info, sizeof(res_info),
 	    BUSSLOT_POS_DBS, BUSSLOT_POS_MDS
 	);

	return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_change_mds_online_state_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfMsgMdsOnlineChange *req_info;
    char query_buf[QUERY_STR_LEN] = {0};
    JpfMsgErrCode mysql_res;
    glong affect_num = 0;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
    	    query_buf, QUERY_STR_LEN,
    	    "update %s set mdu_state=%d where mdu_id='%s'",
    	    MDS_TABLE, req_info->new_status, req_info->mds_id
    );
 printf("--------get mds state =%s\n",query_buf);
    memset(&mysql_res, 0, sizeof(mysql_res));
    jpf_dbs_do_query_code(app_obj, msg, query_buf, &mysql_res, &affect_num);
    jpf_sysmsg_destroy(msg);
    return MFR_ACCEPTED;
}



NmpMsgFunRet
jpf_dbs_mds_heart_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfMdsHeart *req_info;
    gint get_ip_enable;
    JpfMdsHeartRes res_info;
    char query_buf[QUERY_STR_LEN];
    JpfModDbs        *dbs_obj;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    strncpy(res_info.mds_id, req_info->mds_id, MDS_ID_LEN - 1);
    dbs_obj = NMP_MODDBS(app_obj);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select auto_get_ip_enable  as count from %s where mdu_id='%s'",
        MDS_TABLE, req_info->mds_id
    );

    get_ip_enable =  jpf_get_record_count(app_obj, query_buf);
    if (get_ip_enable)
    {
        jpf_update_mds_ip(app_obj, req_info->mds_id, req_info->mds_ip,
        	req_info->cms_ip, 0);
    }

    SET_CODE(&res_info, 0);
    jpf_dbs_modify_sysmsg_2(msg, &res_info, sizeof(res_info),
        BUSSLOT_POS_DBS, BUSSLOT_POS_MDS);

    return MFR_DELIVER_BACK;
}

void
nmp_mod_dbs_register_mds_msg_handler(JpfModDbs *self)
{
    NmpAppMod *super_self = (NmpAppMod*)self;

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MDS_REGISTER,
        NULL,
        jpf_dbs_mds_register_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
    	 MSG_MDS_ONLINE_CHANGE,
    	 NULL,
    	 nmp_mod_dbs_change_mds_online_state_b,
    	 0
    );

    nmp_app_mod_register_msg(
        super_self,
    	 MESSAGE_MDS_HEART,
    	 NULL,
    	 jpf_dbs_mds_heart_b,
    	 0
    );
}


