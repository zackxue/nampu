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
nmp_get_mds_register_info(NmpMysqlRes *mysql_result, NmpMdsRegisterRes*res_info)
{
    G_ASSERT(mysql_result != NULL && res_info != NULL);

    unsigned long field_num;
    NmpMysqlField *mysql_fields;    //modify: mysql_fields -> *msql_fields
    NmpMysqlRow mysql_row;
    char *value, *name;
    int j,row_num;

    row_num = nmp_sql_get_num_rows(mysql_result);
    if (row_num == 0)
    {
        nmp_warning("<GetUserInfo>No such record entry in database");
        return -E_NODBENT;
    }
    else
    {
        field_num = nmp_sql_get_num_fields(mysql_result);
        while ((mysql_row = nmp_sql_fetch_row(mysql_result)))
        {
            nmp_sql_field_seek(mysql_result, 0);
            mysql_fields = nmp_sql_fetch_fields(mysql_result);

            for (j = 0; j < field_num; j++)
            {
                name = nmp_sql_get_field_name(mysql_fields, j);
                if (!strcmp(name,"mdu_keep_alive_freq"))
                {
                    value = nmp_sql_get_field_value(mysql_row, j);
                    res_info->keep_alive_time = atoi(value);
                }
		  else if (!strcmp(name,"mdu_pu_port"))
                {
                    value = nmp_sql_get_field_value(mysql_row, j);
                    res_info->pu_port = atoi(value);
                }
		  else if (!strcmp(name,"mdu_rtsp_port"))
                {
                    value = nmp_sql_get_field_value(mysql_row, j);
                    res_info->rtsp_port = atoi(value);
                }
                else if (!strcmp(name,"auto_get_ip_enable"))
                {
                    value = nmp_sql_get_field_value(mysql_row, j);
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


void nmp_insert_mds_ip(NmpAppObj *app_obj, gchar *mds_id,
	gchar *cms_ip, gchar *mds_ip)
{
    NmpMsgErrCode result;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    memset(&result, 0, sizeof(result));
    snprintf(
       query_buf, QUERY_STR_LEN, "insert ignore into %s values('%s','%s','%s')",
       MDS_IP_TABLE, mds_id, cms_ip, mds_ip
       );

    nmp_dbs_do_query_code(app_obj, NULL, query_buf, &result, &affect_num);

}


static void nmp_del_mds_ip(NmpAppObj *app_obj, gchar *mds_id)
{
    NmpMsgErrCode result;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    memset(&result, 0, sizeof(result));
    snprintf(
       query_buf, QUERY_STR_LEN, "delete from %s where mdu_id='%s'",
       MDS_IP_TABLE, mds_id
    );

    nmp_dbs_do_query_code(app_obj, NULL, query_buf, &result, &affect_num);
}


static void nmp_update_mds_ip(NmpAppObj *app_obj, gchar *mds_id,
	gchar *mds_ip, gchar *cms_ip, gint clear)
{
    NmpHostIps ips;
    gint index;

    if (clear)
    {
        nmp_del_mds_ip(app_obj, mds_id);
    }

    if (!strcmp(cms_ip, LOCAL_IP))
    {
        nmp_get_host_ips(&ips);
        for (index = 0; index < ips.count; index++)
        {
            nmp_insert_mds_ip(app_obj, mds_id, ips.ips[index].ip,
            ips.ips[index].ip);
        }
    }
    else
    {
        nmp_insert_mds_ip(app_obj, mds_id, cms_ip,
            mds_ip);
        nmp_insert_mds_ip(app_obj, mds_id, DEFALUT_CMS_IP,
            mds_ip);
    }
}

NmpMsgFunRet
nmp_dbs_mds_register_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);

 	NmpMdsRegister *req_info;
 	NmpMdsRegisterRes res_info;
 	NmpMysqlRes *mysql_res;
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

 	mysql_res = nmp_dbs_do_query_res(app_obj, query_buf);
 	BUG_ON(!mysql_res);
 	if (G_LIKELY(!MYSQL_RESULT_CODE(mysql_res)))  //success:0 fail:!0
 	{
 		code = nmp_get_mds_register_info(mysql_res, &res_info);
 		strcpy(res_info.mds_id, req_info->mds_id);
 	}
 	else
 		code = MYSQL_RESULT_CODE(mysql_res);

 	nmp_sql_put_res(mysql_res, sizeof(NmpMysqlRes));

	if (res_info.get_ip_enable)
	{
       	nmp_update_mds_ip(app_obj, req_info->mds_id, req_info->mds_ip,
       		req_info->cms_ip, 1);
 	}
 	SET_CODE(&res_info, code);
 	nmp_dbs_modify_sysmsg_2(msg, &res_info, sizeof(res_info),
 	    BUSSLOT_POS_DBS, BUSSLOT_POS_MDS
 	);

	return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_change_mds_online_state_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpMsgMdsOnlineChange *req_info;
    char query_buf[QUERY_STR_LEN] = {0};
    NmpMsgErrCode mysql_res;
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
    nmp_dbs_do_query_code(app_obj, msg, query_buf, &mysql_res, &affect_num);
    nmp_sysmsg_destroy(msg);
    return MFR_ACCEPTED;
}



NmpMsgFunRet
nmp_dbs_mds_heart_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpMdsHeart *req_info;
    gint get_ip_enable;
    NmpMdsHeartRes res_info;
    char query_buf[QUERY_STR_LEN];
    NmpModDbs        *dbs_obj;

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

    get_ip_enable =  nmp_get_record_count(app_obj, query_buf);
    if (get_ip_enable)
    {
        nmp_update_mds_ip(app_obj, req_info->mds_id, req_info->mds_ip,
        	req_info->cms_ip, 0);
    }

    SET_CODE(&res_info, 0);
    nmp_dbs_modify_sysmsg_2(msg, &res_info, sizeof(res_info),
        BUSSLOT_POS_DBS, BUSSLOT_POS_MDS);

    return MFR_DELIVER_BACK;
}

void
nmp_mod_dbs_register_mds_msg_handler(NmpModDbs *self)
{
    NmpAppMod *super_self = (NmpAppMod*)self;

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MDS_REGISTER,
        NULL,
        nmp_dbs_mds_register_b,
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
    	 nmp_dbs_mds_heart_b,
    	 0
    );
}


