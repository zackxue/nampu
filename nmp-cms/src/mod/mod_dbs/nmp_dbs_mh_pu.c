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
#include "message/nmp_msg_cu.h"
#include "nmp_shared.h"
#include "nmp_list_head.h"
#include "nmp_dbs_struct.h"

extern gchar g_domain_id[DOMAIN_ID_LEN];
#define MAX_CHANNEL_LEVEL 3
#define ALARM_BYPASS (1<<1)
#define MAX_ALARM_NUM     50000

//static guint msg_seq_generator = 0;


static __inline__ gint
jpf_get_pu_register_info(JpfMysqlRes *mysql_result, JpfPuRegRes *res_info)
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
        //jpf_warning("<PuRegisterInfo>No such puid:%s,domainId=%s in database", res_info->puid, g_domain_id);
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

                if (!strcmp(name,"mdu_ip"))
                {
                    value = jpf_sql_get_field_value(mysql_row, j);
                    res_info->mdu_ip[MAX_IP_LEN - 1] = 0;
                    strncpy(res_info->mdu_ip, value, MAX_IP_LEN - 1);
                }
                else if (!strcmp(name,"pu_keep_alive_freq"))
                {
                    value = jpf_sql_get_field_value(mysql_row, j);
                    res_info->keep_alive_time = atoi(value);
                }
                else if (!strcmp(name,"mdu_pu_port"))
                {
                    value = jpf_sql_get_field_value(mysql_row, j);
                    res_info->mdu_port= atoi(value);
                }
                else
                    jpf_warning(
                         "Ignore table field %s while getting user info",name
                    );
            }
        }

         return 0;
    }
}


JpfMsgFunRet
jpf_dbs_pu_register_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfPuRegInfo *req_info;
    JpfPuRegRes res_info;
    JpfMysqlRes *mysql_res;
    JpfModDbs        *dbs_obj;
    char query_buf[QUERY_STR_LEN];
    gint code = 0, res_over_flag;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    dbs_obj = JPF_MODDBS(app_obj);
    if (dbs_obj->wdd_status)
    {
        code = -E_NOWDD;
        goto end_pu_register;
    }

    if (dbs_obj->authorization_expired)
    {
        code = -E_EXPIRED;
        goto end_pu_register;
    }

    if (dbs_obj->time_status)
    {
        code = -E_TIMER;
        goto end_pu_register;
    }

    res_over_flag = dbs_obj->res_over_flag;
    if (res_over_flag)
    {
        code = -E_AVMAXNUM;
        goto end_pu_register;
    }

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select pu_keep_alive_freq from %s where pu_id='%s' and pu_domain='%s'",
        PU_TABLE, req_info->puid, g_domain_id
    );

    mysql_res = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!mysql_res);
    if (G_LIKELY(!MYSQL_RESULT_CODE(mysql_res)))  //success:0 fail:!0
    {
        strcpy(res_info.puid, req_info->puid);
        code = jpf_get_pu_register_info(mysql_res, &res_info);
	 if (code)
	     jpf_warning("<PuRegisterInfo>No such puid:%s,domainId=%s in database",
	         res_info.puid, g_domain_id);
    }
    else
        code = MYSQL_RESULT_CODE(mysql_res);

    jpf_sql_put_res(mysql_res, sizeof(JpfMysqlRes));

    if (code == 0)
    {
        snprintf(
            	query_buf, QUERY_STR_LEN,
            	"select t2.mdu_pu_port,t3.mdu_ip from \
            	%s as t1, %s as t2,%s as t3 where t1.pu_id='%s' and t1.pu_domain='%s' and \
            	t2.mdu_id=t1.pu_mdu and t3.mdu_id=t1.pu_mdu and t3.mdu_cmsip='%s'",
            	PU_TABLE, MDS_TABLE,MDS_IP_TABLE,
            	req_info->puid, g_domain_id, req_info->cms_ip
        );

        mysql_res = jpf_dbs_do_query_res(app_obj, query_buf);
     	 BUG_ON(!mysql_res);
        if (G_LIKELY(!MYSQL_RESULT_CODE(mysql_res)))  //success:0 fail:!0
     	 {
            code = jpf_get_pu_register_info(mysql_res, &res_info);
            if (code == -E_NODBENT)
            {
                jpf_warning(
			"<PuRegister> MDS of PU '%s' doesn't have a correct IP, Try to find a default one.",
			res_info.puid
		);
                code = 0;
                snprintf(
                  	query_buf, QUERY_STR_LEN,
                  	"select t2.mdu_pu_port,t3.mdu_ip from \
                  	%s as t1, %s as t2,%s as t3 where t1.pu_id='%s' and t1.pu_domain='%s' and \
                  	t2.mdu_id=t1.pu_mdu and t3.mdu_id=t1.pu_mdu and t3.mdu_cmsip='%s'",
                  	PU_TABLE, MDS_TABLE,MDS_IP_TABLE,
                  	req_info->puid, g_domain_id, DEFALUT_CMS_IP
                 );

		  jpf_sql_put_res(mysql_res, sizeof(JpfMysqlRes));
		  mysql_res = jpf_dbs_do_query_res(app_obj, query_buf);
     	         BUG_ON(!mysql_res);
                if (G_LIKELY(!MYSQL_RESULT_CODE(mysql_res)))  //success:0 fail:!0
                {
                    code = jpf_get_pu_register_info(mysql_res, &res_info);
                    if (code == -E_NODBENT)
                    {
                        jpf_warning("<PuRegister>MDS of PU '%s' doesn't have a valid IP.", res_info.puid);
                        code = 0;
                    }
                }
                else
                    code = MYSQL_RESULT_CODE(mysql_res);
            }
     	 }
     	 else
            code = MYSQL_RESULT_CODE(mysql_res);

        jpf_sql_put_res(mysql_res, sizeof(JpfMysqlRes));
    }

end_pu_register:
    SET_CODE(&res_info, code);
    jpf_dbs_modify_sysmsg_2(msg, &res_info, sizeof(res_info),
        BUSSLOT_POS_DBS, BUSSLOT_POS_PU
    );

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_dbs_pu_heart_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfPuHeart *req_info;
    gint num;
    JpfPuHeartResp res_info;
    glong affect_num = 0;
    char query_buf[QUERY_STR_LEN];
    JpfModDbs        *dbs_obj;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    strncpy(res_info.puid, req_info->puid, MAX_ID_LEN - 1);
    dbs_obj = JPF_MODDBS(app_obj);
    if (dbs_obj->authorization_expired)
    {
        SET_CODE(&res_info,  -E_EXPIRED);
        goto heart_end;
    }

    if (dbs_obj->res_over_flag)
    {
        SET_CODE(&res_info,  -E_AVMAXNUM);
        goto heart_end;
    }

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select count(*)  as count from %s where pu_id='%s' and pu_domain='%s'",
        PU_TABLE, req_info->puid, g_domain_id
    );

    num =  jpf_get_record_count(app_obj, query_buf);
    if (num > 0)
    {
        jpf_get_current_zone_time(res_info.server_time);
        snprintf(
            query_buf, QUERY_STR_LEN,
            "update %s set pu_state=1,pu_last_alive='%s',pu_last_ip='%s' where pu_id='%s' and \
            pu_domain='%s' and pu_registered=1",
            PU_RUNNING_TABLE, res_info.server_time, req_info->pu_ip,
            req_info->puid, g_domain_id
        );

        jpf_dbs_do_query_code(app_obj, msg, query_buf, &res_info.code, &affect_num);
    }
    else if (num == 0)
        SET_CODE(&res_info,  -E_NODBENT);
    else
        SET_CODE(&res_info,  num);

 heart_end:
    jpf_dbs_modify_sysmsg_2(msg, &res_info, sizeof(res_info),
        BUSSLOT_POS_DBS, BUSSLOT_POS_PU);

    return MFR_DELIVER_BACK;
}


static __inline__ gint
jpf_pu_get_mds_info(JpfMysqlRes *mysql_result, JpfGetMdsInfoRes *res_info)
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
        //jpf_warning("<PuRegisterInfo>No such puid in database");
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

                if (!strcmp(name,"mdu_ip"))
                {
                    value = jpf_sql_get_field_value(mysql_row, j);
                    res_info->mdu_ip[MAX_IP_LEN - 1] = 0;
                    strncpy(res_info->mdu_ip, value, MAX_IP_LEN - 1);
                }
                else if (!strcmp(name,"pu_keep_alive_freq"))
                {
                    value = jpf_sql_get_field_value(mysql_row, j);
                    res_info->keep_alive_time = atoi(value);
                }
                else if (!strcmp(name,"mdu_pu_port"))
                {
                    value = jpf_sql_get_field_value(mysql_row, j);
                    res_info->mdu_port= atoi(value);
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

JpfMsgFunRet
jpf_dbs_pu_get_mds_info_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfGetMdsInfo *req_info;
    JpfGetMdsInfoRes res_info;
    JpfMysqlRes *mysql_res;

    char query_buf[QUERY_STR_LEN];
    int code = 0;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    strcpy(res_info.puid, req_info->puid);
    snprintf(
        query_buf, QUERY_STR_LEN,
        "select t2.mdu_pu_port,t3.mdu_ip from \
        %s as t1, %s as t2,%s as t3 where t1.pu_id='%s' and t1.pu_domain='%s' and \
        t2.mdu_id=t1.pu_mdu and t3.mdu_id=t1.pu_mdu and t3.mdu_cmsip='%s'",
        PU_TABLE, MDS_TABLE,MDS_IP_TABLE,
        req_info->puid, g_domain_id, req_info->cms_ip
     );

    mysql_res = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!mysql_res);
    if (G_LIKELY(!MYSQL_RESULT_CODE(mysql_res)))  //success:0 fail:!0
    {
        code = jpf_pu_get_mds_info(mysql_res, &res_info);
        if (code == -E_NODBENT)
        {
            code = 0;
            snprintf(
                query_buf, QUERY_STR_LEN,
                "select t2.mdu_pu_port,t3.mdu_ip from \
                %s as t1, %s as t2,%s as t3 where t1.pu_id='%s' and t1.pu_domain='%s' and \
                t2.mdu_id=t1.pu_mdu and t3.mdu_id=t1.pu_mdu and t3.mdu_cmsip='%s'",
                PU_TABLE, MDS_TABLE,MDS_IP_TABLE,
                req_info->puid, g_domain_id, DEFALUT_CMS_IP
            );

            jpf_sql_put_res(mysql_res, sizeof(JpfMysqlRes));
            mysql_res = jpf_dbs_do_query_res(app_obj, query_buf);
            BUG_ON(!mysql_res);
            if (G_LIKELY(!MYSQL_RESULT_CODE(mysql_res)))  //success:0 fail:!0
            {
                code = jpf_pu_get_mds_info(mysql_res, &res_info);
                /*if (code == -E_NODBENT)
                {
                    code = 0;
                }     */
            }
            else
                code = MYSQL_RESULT_CODE(mysql_res);
        }
    }
    else
        code = MYSQL_RESULT_CODE(mysql_res);

    jpf_sql_put_res(mysql_res, sizeof(JpfMysqlRes));
    SET_CODE(&res_info, code);
    jpf_dbs_modify_sysmsg_2(msg, &res_info, sizeof(res_info),
        BUSSLOT_POS_DBS, BUSSLOT_POS_PU);

    return MFR_DELIVER_BACK;
}

static __inline__ void
jpf_get_cu_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfAllCuOwnPu *cu_list
                  )
{
    G_ASSERT(cu_list != NULL);

    gint info_no = 0, field_no =0;
    JpfMysqlRow mysql_row;
    JpfMysqlField* mysql_fields;
    gchar *name;
    gchar *value;
    unsigned int row_num;
    unsigned int field_num;

    row_num = jpf_sql_get_num_rows(result);
    field_num = jpf_sql_get_num_fields(result);

    while ((mysql_row = jpf_sql_fetch_row(result)))
    {
        jpf_sql_field_seek(result, 0);
        mysql_fields = jpf_sql_fetch_fields(result);
        for(field_no = 0; field_no < field_num; field_no++)
        {
            name = jpf_sql_get_field_name(mysql_fields, field_no);
            if (!strcmp(name, "user_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    cu_list[info_no].username[USER_NAME_LEN - 1] = 0;
                    strncpy(cu_list[info_no].username, value, USER_NAME_LEN - 1);
                }
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }

        info_no++;
    }
}


static __inline__ JpfPuOwnToAllCu *
jpf_dbs_get_cu(JpfMysqlRes *mysql_res, gint *size)
{
	gint row_num, len;
	JpfPuOwnToAllCu *query_res;

	row_num = jpf_sql_get_num_rows(mysql_res);
	if (row_num == 0)
	{
        len = sizeof(JpfPuOwnToAllCu);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
        	return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
	}
	else
	{
        len = sizeof(JpfPuOwnToAllCu) + row_num*sizeof(JpfAllCuOwnPu);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        jpf_get_cu_info(mysql_res, len, &query_res->cu_list[0]);
        query_res->total_num = row_num;
	}

	*size = len;

	return query_res;
}


static __inline__ void
jpf_get_cu_tw_scr_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfTwScr *scr_list
                  )
{
    G_ASSERT(scr_list != NULL);

    gint info_no = 0, field_no =0;
    JpfMysqlRow mysql_row;
    JpfMysqlField* mysql_fields;
    gchar *name;
    gchar *value;
    unsigned int row_num;
    unsigned int field_num;

    row_num = jpf_sql_get_num_rows(result);
    field_num = jpf_sql_get_num_fields(result);

    while ((mysql_row = jpf_sql_fetch_row(result)))
    {
        jpf_sql_field_seek(result, 0);
        mysql_fields = jpf_sql_fetch_fields(result);
        for(field_no = 0; field_no < field_num; field_no++)
        {
            name = jpf_sql_get_field_name(mysql_fields, field_no);
            if (!strcmp(name, "tw_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    scr_list[info_no].tw_id = atoi(value);
                }
            }
            else if (!strcmp(name, "scr_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    scr_list[info_no].scr_id= atoi(value);
                }
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }

        info_no++;
    }
}


static __inline__ JpfPuTwScr *
jpf_dbs_get_tw_scr(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfPuTwScr *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfPuTwScr);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
        jpf_print("no cu own the pu");
    }
    else
    {
        len = sizeof(JpfPuTwScr) + row_num*sizeof(JpfTwScr);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        jpf_get_cu_tw_scr_info(mysql_res, len, &query_res->scr_list[0]);
        query_res->total_num = row_num;
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_mod_dbs_dec_online_state_notify(JpfAppObj *app_obj,
    JpfPuOnlineStatusChange *req_info)
{
    JpfPuTwScr *screens;
    JpfMysqlRes *result;
    JpfSysMsg *msg_notify;
    gchar query_buf[QUERY_STR_LEN] = {0};
    gchar dev_type[DEV_TYPE_LEN] = {0};
    gint size,ret;

    jpf_get_dev_type_from_puid(req_info->puid, dev_type);
    if (strcmp(dev_type,DECODER_TYPE))
        return;
    snprintf(query_buf, QUERY_STR_LEN,
        "select t1.scr_id,t1.tw_id from %s as t1,%s as t2 where t2.gu_puid='%s' \
        and t2.gu_domain='%s' and t1.dis_domain=t2.gu_domain and t1.dis_guid=t2.gu_id",
        SCREEN_TABLE, GU_TABLE, req_info->puid, req_info->domain_id
    );
    result = jpf_dbs_do_query_res(app_obj,query_buf);
    BUG_ON(!result);

    if (G_LIKELY(MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        return;
    }

    screens = jpf_dbs_get_tw_scr(result, &size);
    if (G_UNLIKELY(!screens))
    {
        jpf_warning("<dbs_mh_pu> alloc error");
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto end_dec_online_state_notify;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

    if (screens->total_num <= 0)
        goto end_dec_online_state_notify;

    screens->state = req_info->new_status;
    msg_notify = jpf_sysmsg_new_2(MESSAGE_DEC_ONLINE_STATE_NOTIFY,
        screens, size, ++msg_seq_generator);
    if (G_UNLIKELY(!msg_notify))
        jpf_warning("<dbs_mh_pu> alloc error");

    MSG_SET_DSTPOS(msg_notify, BUSSLOT_POS_CU);
    jpf_app_obj_deliver_out(app_obj, msg_notify);

end_dec_online_state_notify:
    jpf_mem_kfree(screens, size);
}


JpfMsgFunRet
jpf_mod_dbs_change_pu_online_state_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfPuOnlineStatusChange *req_info;
    JpfPuOwnToAllCu *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    JpfMsgErrCode mysql_res1;
    gint size,ret;
    glong affect_num = 0;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    gchar time_now[TIME_STRING_LEN] = {0};
    jpf_get_current_zone_time(time_now);
    strcpy(req_info->pu_last_alive_time, time_now);

    if (req_info->new_status)
    {
        snprintf(
            query_buf, QUERY_STR_LEN,
            "update %s set pu_state=%d,pu_registered=%d,pu_last_alive='%s',\
            pu_last_ip='%s',pu_last_cmsip='%s' \
            where pu_id='%s' and pu_domain='%s'",
            PU_RUNNING_TABLE, req_info->new_status, req_info->new_status,
            time_now, req_info->pu_ip, req_info->cms_ip,
            req_info->puid, req_info->domain_id
        );
    }
    else
    {
        snprintf(
            query_buf, QUERY_STR_LEN,
            "update %s set pu_state=%d,pu_registered=%d,pu_last_alive='%s'\
            where pu_id='%s' \
            and pu_domain='%s'",
            PU_RUNNING_TABLE, req_info->new_status, req_info->new_status,
            time_now, req_info->puid,
            req_info->domain_id
        );
    }
    memset(&mysql_res1, 0, sizeof(mysql_res1));
    jpf_dbs_do_query_code(app_obj, msg, query_buf, &mysql_res1, &affect_num);
    if (req_info->new_status == 0)
    {
        if (mysql_res1.err_no != 0)
           jpf_warning("!!!!!!!!!!!!!!!error");
    }
    jpf_mod_dbs_dec_online_state_notify(app_obj, req_info);

    snprintf(query_buf, QUERY_STR_LEN,
        "select distinct user_name from %s where user_guid like '%s%%' and \
        user_guid_domain='%s'",
        USER_OWN_GU_TABLE, req_info->puid, req_info->domain_id
    );
    result = jpf_dbs_do_query_res(app_obj,query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_cu(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_pu> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

        memcpy(&query_res->pu_state, req_info, sizeof(JpfPuOnlineStatusChange));
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_do_cu_query;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

    end_query_cu:
    jpf_sysmsg_set_private(msg, query_res, size, jpf_mem_kfree);
    MSG_SET_DSTPOS(msg, BUSSLOT_POS_CU);

    return MFR_DELIVER_BACK;

    err_do_cu_query:
    size = sizeof(JpfPuOwnToAllCu);
    query_res = jpf_mem_kalloc(size);
    if (!query_res)
    {
        jpf_warning("<dbs_mh_bss> alloc error");
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;

    goto end_query_cu;
}


JpfMsgFunRet
jpf_mod_dbs_submit_format_pro_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfSubmitFormatPos *req_info;
    JpfSubmitFormatPosRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint row_num, len;
    gint ret;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    strncpy(req_info->domain_id, jpf_get_local_domain_id(), DOMAIN_ID_LEN - 1);
    snprintf(query_buf, QUERY_STR_LEN,
        "select distinct user_name from %s where user_guid like '%s%%' and \
        user_guid_domain='%s'",
        USER_OWN_GU_TABLE, req_info->puid, req_info->domain_id
    );
    result = jpf_dbs_do_query_res(app_obj,query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        row_num = jpf_sql_get_num_rows(result);
        if (row_num == 0)
        {
            len = sizeof(JpfSubmitFormatPosRes);
            query_res = jpf_mem_kalloc(len);
            if (G_UNLIKELY(!query_res))
            {
                jpf_warning("<dbs_mh_bss> alloc error");
                jpf_sysmsg_destroy(msg);
                return MFR_ACCEPTED;
            }

            memset(query_res, 0, len);
            SET_CODE(query_res, E_NODBENT);
            query_res->total_num = 0;
            jpf_warning("no cu own the pu");
        }
        else
        {
            len = sizeof(JpfSubmitFormatPosRes) + row_num*sizeof(JpfAllCuOwnPu);
            query_res = jpf_mem_kalloc(len);
            if (G_UNLIKELY(!query_res))
            {
                jpf_warning("<dbs_mh_bss> alloc error");
                jpf_sysmsg_destroy(msg);
                return MFR_ACCEPTED;
            }

            memset(query_res, 0, len);
            SET_CODE(query_res, 0);
            jpf_get_cu_info(result, len, query_res->cu_list);
            query_res->total_num = row_num;
        }
        memcpy(&query_res->pro, req_info, sizeof(JpfSubmitFormatPos));
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_do_cu_query;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

    end_query_cu:
    jpf_sysmsg_set_private(msg, query_res, len, jpf_mem_kfree);
    MSG_SET_DSTPOS(msg, BUSSLOT_POS_CU);

    return MFR_DELIVER_BACK;

    err_do_cu_query:
    len = sizeof(JpfSubmitFormatPosRes);
    query_res = jpf_mem_kalloc(len);
    if (!query_res)
    {
        jpf_warning("<dbs_mh_bss> alloc error");
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, len);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;

    goto end_query_cu;
}


static __inline__ gint
jpf_get_guid(JpfSubmitAlarm *alarm, gchar *guid, gint level)
{
    G_ASSERT(alarm != NULL && guid != NULL);

    gchar gu_type[4] = {0};
    gchar code_type[4] = {0};

    switch(alarm->alarm_type)
    {
    case 0:
    case 1:
    case 2:
        if (level < 0 || level > MAX_CHANNEL_LEVEL)
	     return -EINVAL;

        strcpy(gu_type, "AV");
        snprintf(code_type, 4,  "%d", level);
	 break;

    case 3:
	if (level != 0)
		return -EINVAL;

	 strcpy(gu_type, "AI");
	 strcpy(code_type, "M");
	 break;

	case 4:
	 if (level != 0)
		return -EINVAL;

	 strcpy(gu_type, "AI");
	 strcpy(code_type, "M");
	 break;

    default:
	return -EINVAL;
    }

    snprintf(guid, MAX_ID_LEN, "%s-%s-%s-%02d", alarm->puid, gu_type, code_type, alarm->channel);
    return 0;
}


static __inline__ gint
jpf_get_alarm_id(JpfMysqlRes *result)
{
    guint row_num;
    guint field_num;
    JpfMysqlRow mysql_row;
    JpfMysqlField* mysql_fields;
    gchar *name;
    gchar *value;
    gint alarm_id = 0;
    gint field_no =0;

    row_num = jpf_sql_get_num_rows(result);
    field_num = jpf_sql_get_num_fields(result);

    while ((mysql_row = jpf_sql_fetch_row(result)))
    {
        jpf_sql_field_seek(result, 0);
        mysql_fields = jpf_sql_fetch_fields(result);
        for(field_no = 0; field_no < field_num; field_no++)
        {
            name = jpf_sql_get_field_name(mysql_fields, field_no);
            if (!strcmp(name, "value"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                       alarm_id = atoi(value);
			  return   alarm_id;
                }
            }
            else
                cms_debug("no need mysql name %s ", name);
        }
    }

    return alarm_id;
}


static __inline__ gint
jpf_mod_dbs_get_init_alarm_id(JpfAppObj *app_obj)
{
	char query_buf[QUERY_STR_LEN] = {0};
	JpfModDbs *dbs_obj;
	JpfMysqlRes *result = NULL;
	gint alarm_id = 0;

	dbs_obj = JPF_MODDBS(app_obj);

	snprintf(
		query_buf, QUERY_STR_LEN,
		"select value from %s where id=2",
		PARAM_CONFIG_TABLE
	);

	result = jpf_dbs_do_query_res(app_obj, query_buf);
	if (result && result->sql_res)
	{
		alarm_id = jpf_get_alarm_id(result);
	}

	jpf_sql_put_res(result, sizeof(JpfMysqlRes));
	return alarm_id;
}

static __inline__ gint
jpf_mod_dbs_get_alarm_id(JpfAppObj *app_obj, JpfSubmitAlarm *req)
{
    static gint g_alarm_id = 0;
    static volatile gsize g_init_alarm_id = 0;

    if (g_once_init_enter(&g_init_alarm_id))
    {
    	g_alarm_id = jpf_mod_dbs_get_init_alarm_id(app_obj);
	if (g_alarm_id < 0)
	{
    	    g_once_init_leave(&g_init_alarm_id, 0);
	    return g_alarm_id;
	}
	else
	{
	    ++g_alarm_id; /* safe */
	    g_once_init_leave(&g_init_alarm_id, 1);
	}
    }

    if (G_UNLIKELY(g_alarm_id < 0))
        g_alarm_id = 0;	/* int overflow */

    return g_atomic_int_exchange_and_add(&g_alarm_id, 1);
}


static __inline__ gint
jpf_mod_dbs_get_alarm_param(JpfAppObj *self, JpfAlarmParam *alarm_param)
{
    char query_buf[QUERY_STR_LEN] = {0};
    JpfMysqlRes *result;
    guint row_num;
    guint field_num;
    JpfMysqlRow mysql_row;
    JpfMysqlField* mysql_fields;
    gchar *name;
    gchar *value;
    gint field_no =0;
    gchar alarm_value[MAX_STR_LEN] = {0};
    JpfModDbs        *dbs_obj;
    dbs_obj = JPF_MODDBS(self);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select value from %s where id=1", PARAM_CONFIG_TABLE
    );

    result = jpf_dbs_do_query_res(self, query_buf);
    BUG_ON(!result);
    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))
    {
        row_num = jpf_sql_get_num_rows(result);
    	if (row_num == 0)
    	{
           jpf_sql_put_res(result, sizeof(JpfMysqlRes));
    	   dbs_obj->del_alarm_flag &= ~DEL_ALARM_FLAG;
    	   dbs_obj->del_alarm_flag &= ~ENABLE_DEL_ALARM;
           return 0;
        }

        field_num = jpf_sql_get_num_fields(result);

        while ((mysql_row = jpf_sql_fetch_row(result)))
        {
            jpf_sql_field_seek(result, 0);
            mysql_fields = jpf_sql_fetch_fields(result);
            for(field_no = 0; field_no < field_num; field_no++)
            {
                name = jpf_sql_get_field_name(mysql_fields, field_no);
                if (!strcmp(name, "value"))
                {
                    value = jpf_sql_get_field_value(mysql_row, field_no);
                    if(value)
                    {
                        alarm_value[MAX_STR_LEN - 1] = 0;
                        strncpy(
                            alarm_value,
                            value, MAX_STR_LEN - 1
                        );
                    }
                }
                else
                    cms_debug("no need mysql name %s ", name);
            }
        }

        alarm_param->del_alarm_num = atoi(strtok(alarm_value, ","));
        alarm_param->max_alarm_num = atoi(strtok(NULL, ","));
	 dbs_obj->del_alarm_flag |= ENABLE_DEL_ALARM;
	 jpf_sql_put_res(result, sizeof(JpfMysqlRes));
	return 0;
    }
    else
    {
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        return MYSQL_RESULT_CODE(result);
    }
}


static __inline__ void
jpf_mod_dbs_del_redundant_alarm(JpfAppObj *self)
{
    char query_buf[QUERY_STR_LEN] = {0};
    gint total_num;
    JpfModDbs *d_self;
    JpfAlarmParam alarm_param;
    JpfMsgErrCode result;
    glong affect_num = 0;
    gint ret;

    d_self = (JpfModDbs *)self;
    if (!d_self->del_alarm_flag)
        return;

    d_self->del_alarm_flag = 0;

    memset(&alarm_param, 0, sizeof(alarm_param));
    ret = jpf_mod_dbs_get_alarm_param(self, &alarm_param);
    if (ret)
	return;

    if (!(d_self->del_alarm_flag&ENABLE_DEL_ALARM))
	return;
    snprintf(
        query_buf, QUERY_STR_LEN,
        "select count(*) as count from %s", ALARM_INFO_TABLE
    );

    total_num =  jpf_get_record_count(self,query_buf);
    if (total_num < alarm_param.max_alarm_num)
	return;

    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s order by alarm_time limit %d ",
        ALARM_INFO_TABLE, alarm_param.del_alarm_num
    );

    jpf_dbs_do_query_code(self, NULL, query_buf, &result, &affect_num);
}


static __inline__ void
jpf_mod_dbs_write_alarm_to_db(JpfAppObj *app_obj, JpfSysMsg *msg, JpfSubmitAlarm *req)
{
    JpfModDbs * self = (JpfModDbs *)app_obj;
    gint alarm_id;
    JpfMsgErrCode result;
    char query_buf[QUERY_STR_LEN] = {0};
    glong affect_num = 0;
    gint total_num;
    JpfNotifyMessage notify_info;

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select count(*) as count from %s", ALARM_INFO_TABLE
    );

    total_num =  jpf_get_record_count(app_obj, query_buf);
    if (total_num >= MAX_ALARM_NUM)
    {
        memset(&notify_info, 0, sizeof(notify_info));
        notify_info.msg_id = MSG_ALARM;
        sprintf(notify_info.param1, "%d", MAX_ALARM_NUM);
        jpf_mods_dbs_broadcast_msg(self, &notify_info, sizeof(notify_info));
        alarm_id = jpf_mod_dbs_get_alarm_id(app_obj, req);
        if (alarm_id < 0)
        {
            jpf_warning("get alarm_id error:%d.", alarm_id);
    	      return;
        }

        req->alarm_id = alarm_id;
        jpf_mod_dbs_del_redundant_alarm(app_obj);
        return;
    }
again:
    alarm_id = jpf_mod_dbs_get_alarm_id(app_obj, req);
    if (alarm_id < 0)
    {
        jpf_warning("get alarm_id error:%d.", alarm_id);
	 return;
    }

    req->alarm_id = alarm_id;
    snprintf(
        query_buf, QUERY_STR_LEN,
       "insert into %s (alarm_id,gu_domain,gu_id,pu_id,pu_name,gu_name,alarm_type,alarm_time,\
       alarm_info,submit_time) values(%d,'%s','%s','%s','%s','%s',%d,'%s','%s','%s')",
       ALARM_INFO_TABLE, alarm_id, req->domain_id, req->guid, req->puid, req->pu_name,
       req->gu_name,1<<req->alarm_type, req->alarm_time, req->alarm_info, req->submit_time
    );

    memset(&result, 0, sizeof(JpfMsgErrCode));
    jpf_dbs_do_query_code(app_obj, msg, query_buf, &result, &affect_num);
    if (RES_CODE(&result))
    {
       if (RES_CODE(&result) == -1062)
           goto again;

       jpf_warning("insert alarm into database error:%d.", RES_CODE(&result));
    }

    jpf_mod_dbs_del_redundant_alarm(app_obj);
}


static __inline__ gint
jpf_get_pu_gu_name(JpfMysqlRes *result, gchar *pu_name, gchar *gu_name)
{
    guint row_num;
    guint field_num;
    JpfMysqlRow mysql_row;
    JpfMysqlField* mysql_fields;
    gchar *name;
    gchar *value;
    gint field_no =0;

    row_num = jpf_sql_get_num_rows(result);
    if (!row_num)
        return 0;

    field_num = jpf_sql_get_num_fields(result);

    while ((mysql_row = jpf_sql_fetch_row(result)))
    {
        jpf_sql_field_seek(result, 0);
        mysql_fields = jpf_sql_fetch_fields(result);
        for(field_no = 0; field_no < field_num; field_no++)
        {
            name = jpf_sql_get_field_name(mysql_fields, field_no);
            if (!strcmp(name, "pu_info"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    pu_name[PU_NAME_LEN - 1] = 0;
                    strncpy(
                        pu_name,
                        value, PU_NAME_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "gu_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    gu_name[GU_NAME_LEN - 1] = 0;
                    strncpy(
                        gu_name,
                        value, GU_NAME_LEN - 1
                    );
                }
            }
            else
                cms_debug("no need mysql name %s ", name);
        }
    }

	return row_num;
}

static __inline__ gint
jpf_mod_dbs_get_pu_gu_name(JpfAppObj *app_obj, JpfSysMsg *msg,
    gchar *guid, gchar *pu_name, gchar *gu_name, gint *num)
{
    JpfSubmitAlarm *req;
    JpfMysqlRes *result;
    gchar query_buf[QUERY_STR_LEN] = {0};

    *num = 0;
    req = MSG_GET_DATA(msg);
    BUG_ON(!req);

    snprintf(
        query_buf, QUERY_STR_LEN,
       "select t1.pu_info,t2.gu_name from %s as t1, %s as t2 where t1.pu_id='%s' \
       and t1.pu_domain='%s' and t2.gu_id='%s' and t2.gu_domain='%s'",
       PU_TABLE, GU_TABLE, req->puid, req->domain_id, guid, req->domain_id
    );
    result = jpf_dbs_do_query_res(app_obj,query_buf);
    if (!MYSQL_RESULT(result))
    {
       jpf_sql_put_res(result, sizeof(JpfMysqlRes));
	return -1;
    }
    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        *num = jpf_get_pu_gu_name(result, pu_name, gu_name);
    }

    jpf_sql_put_res(result, sizeof(JpfMysqlRes));

    return 0;
}

static __inline__ gint
jpf_get_gu_attributes(JpfMysqlRes *result)
{
    guint row_num;
    guint field_num;
    JpfMysqlRow mysql_row;
    JpfMysqlField* mysql_fields;
    gchar *name;
    gchar *value;
    gint gu_attributes = 0;
    gint field_no =0;

    row_num = jpf_sql_get_num_rows(result);
    field_num = jpf_sql_get_num_fields(result);

    while ((mysql_row = jpf_sql_fetch_row(result)))
    {
        jpf_sql_field_seek(result, 0);
        mysql_fields = jpf_sql_fetch_fields(result);
        for(field_no = 0; field_no < field_num; field_no++)
        {
            name = jpf_sql_get_field_name(mysql_fields, field_no);
            if (!strcmp(name, "gu_attributes"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                       gu_attributes = atoi(value);
			  return   gu_attributes;
                }
            }
            else
                cms_debug("no need mysql name %s ", name);
        }
    }

    return gu_attributes;
}


static __inline__ gint
jpf_mod_dbs_get_gu_bypass(JpfAppObj *app_obj, gchar *guid, gchar *domain_id)
{
    JpfMysqlRes *result;
    gchar query_buf[QUERY_STR_LEN] = {0};
    gint gu_attribute, bypass = 0;

    snprintf(
        query_buf, QUERY_STR_LEN,
       "select gu_attributes from %s where gu_id='%s' and gu_domain='%s'",
       GU_TABLE,guid, domain_id
    );
    result = jpf_dbs_do_query_res(app_obj,query_buf);
    if (!MYSQL_RESULT(result))
    {
       jpf_sql_put_res(result, sizeof(JpfMysqlRes));
	return -1;
    }
    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        gu_attribute = jpf_get_gu_attributes(result);
	 bypass = gu_attribute&ALARM_BYPASS;
    }

    jpf_sql_put_res(result, sizeof(JpfMysqlRes));

    return bypass;
}


JpfMsgFunRet
jpf_mod_dbs_submit_alarm_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfSubmitAlarm *req_info;
    JpfSubmitAlarmRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    char guid[MAX_ID_LEN] = {0};
    gchar pu_name[PU_NAME_LEN] = {0};
    gchar gu_name[GU_NAME_LEN] = {0};
    gint row_num, len;
    gint ret, found = 0, level = 0;
    gint bypass;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

try_again:
    ret = jpf_get_guid(req_info, guid, level);
    if (ret || !regex_mached(guid, guid_reg))
    {
        jpf_warning("alarm guid doesn't exist!");
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

//----------------------------------------
    ret = jpf_mod_dbs_get_pu_gu_name(app_obj, msg, guid, pu_name, gu_name, &found);
    if (ret)
    {
        jpf_warning("alarm get guid error");
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    if (!found)
    {
        ++level;
        goto try_again;
    }

    strncpy(req_info->pu_name, pu_name, PU_NAME_LEN - 1);
    strncpy(req_info->gu_name, gu_name, GU_NAME_LEN - 1);
    strncpy(req_info->guid, guid, MAX_ID_LEN - 1);

    bypass = jpf_mod_dbs_get_gu_bypass(app_obj, guid, req_info->domain_id);
    if (bypass)
    {
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    jpf_get_current_zone_time(req_info->alarm_time);
    if (req_info->action_type == 0)
    {
        jpf_mod_dbs_write_alarm_to_db(app_obj, msg, req_info);
        jpf_cms_mod_deliver_msg_2(app_obj, BUSSLOT_POS_AMS, MESSAGE_SUBMIT_ALARM,
        	req_info, sizeof(JpfSubmitAlarm));  //ÔÝÊ±ÆÁ±Î
    }

    snprintf(query_buf, QUERY_STR_LEN,
        "select distinct user_name from %s where user_guid like '%s%%' and \
        user_guid_domain='%s'",
        USER_OWN_GU_TABLE, req_info->puid, req_info->domain_id
    );
    result = jpf_dbs_do_query_res(app_obj,query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        row_num = jpf_sql_get_num_rows(result);
        if (row_num == 0)
        {
            len = sizeof(JpfSubmitAlarmRes);
            query_res = jpf_mem_kalloc(len);
            if (G_UNLIKELY(!query_res))
            {
                jpf_warning("<dbs_mh_bss> alloc error");
                jpf_sysmsg_destroy(msg);
                return MFR_ACCEPTED;
            }

            memset(query_res, 0, len);
            SET_CODE(query_res, E_NODBENT);
            query_res->total_num = 0;
            jpf_warning("no cu own the pu");
        }
        else
        {
            len = sizeof(JpfSubmitAlarmRes) + row_num*sizeof(JpfAllCuOwnPu);
            query_res = jpf_mem_kalloc(len);
            if (G_UNLIKELY(!query_res))
            {
                jpf_warning("<dbs_mh_bss> alloc error");
                jpf_sysmsg_destroy(msg);
                return MFR_ACCEPTED;
            }

            memset(query_res, 0, len);
            SET_CODE(query_res, 0);
            jpf_get_cu_info(result, len, query_res->cu_list);
            query_res->total_num = row_num;
        }

        memcpy(&query_res->alarm, req_info, sizeof(JpfSubmitAlarm));
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_do_cu_query;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

    end_query_cu:
    jpf_sysmsg_set_private(msg, query_res, len, jpf_mem_kfree);
    MSG_SET_DSTPOS(msg, BUSSLOT_POS_CU);

    return MFR_DELIVER_BACK;

    err_do_cu_query:
    len = sizeof(JpfSubmitAlarmRes);
    query_res = jpf_mem_kalloc(len);
    if (!query_res)
    {
        jpf_warning("<dbs_mh_bss> alloc error");
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, len);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;

    goto end_query_cu;
}


static __inline__ gint
jpf_pu_get_mf_name(JpfMysqlRes *mysql_result, JpfMsgGetManufactRes *res_info)
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

                if (!strcmp(name,"mf_name"))
                {
                    value = jpf_sql_get_field_value(mysql_row, j);
                    res_info->mf_name[MF_NAME_LEN - 1] = 0;
                    strncpy(res_info->mf_name, value, MF_NAME_LEN - 1);
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


JpfMsgFunRet
jpf_dbs_pu_get_device_manufact_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfMsgGetManufact *req_info;
    JpfMsgGetManufactRes res_info;
    JpfMysqlRes *mysql_res;

    char query_buf[QUERY_STR_LEN];
    int code = 0;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    snprintf(
        query_buf, QUERY_STR_LEN,
        "select mf_name from %s where mf_id='%s'",
        MANUFACTURER_TABLE, req_info->mf_id
     );

    mysql_res = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!mysql_res);
    if (G_LIKELY(!MYSQL_RESULT_CODE(mysql_res)))  //success:0 fail:!0
    {
        code = jpf_pu_get_mf_name(mysql_res, &res_info);
    }
    else
        code = MYSQL_RESULT_CODE(mysql_res);

    jpf_sql_put_res(mysql_res, sizeof(JpfMysqlRes));
    SET_CODE(&res_info, code);
    jpf_dbs_modify_sysmsg_2(msg, &res_info, sizeof(res_info),
        BUSSLOT_POS_DBS, BUSSLOT_POS_PU);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_dbs_pu_get_div_mode_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfPuGetDivMode *req_info;;
    JpfGetDivModeRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size,ret;
    gint total_num;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(query_buf, QUERY_STR_LEN,
	"select count(*)  as count from %s",
	SCREEN_DIVISION_TABLE
    );
    total_num =  jpf_get_record_count(app_obj,query_buf);
    if (total_num == 0)
    {
        ret = 0;
        goto err_get_scr_div;
    }

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select * from %s",
         SCREEN_DIVISION_TABLE
    );

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_div_mode(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_cu> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

        query_res->total_num = total_num;

        strcpy(query_res->session, req_info->puid);
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_get_scr_div;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_get_scr_div:

    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_PU, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_get_scr_div:
    size = sizeof(JpfGetDivModeRes);
    query_res = jpf_mem_kalloc(size);
    if (!query_res)
    {
        jpf_warning("<dbs_mh_cu> alloc error");
        jpf_sysmsg_destroy(msg);

        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;
    strcpy(query_res->session, req_info->puid);

    goto end_get_scr_div;
}


void
jpf_mod_dbs_register_pu_msg_handler(JpfModDbs *self)
{
    JpfAppMod *super_self = (JpfAppMod*)self;

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_PU_REGISTER,
        NULL,
        jpf_dbs_pu_register_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_PU_HEART,
        NULL,
        jpf_dbs_pu_heart_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
    	 MESSAGE_CHANGED_PU_ONLINE_STATE,
    	 NULL,
    	 jpf_mod_dbs_change_pu_online_state_b,
    	 0
    );

    jpf_app_mod_register_msg(
        super_self,
    	 MESSAGE_SUBMIT_FORMAT_POS,
    	 NULL,
    	 jpf_mod_dbs_submit_format_pro_b,
    	 0
    );

    jpf_app_mod_register_msg(
        super_self,
    	 MESSAGE_SUBMIT_ALARM,
    	 NULL,
    	 jpf_mod_dbs_submit_alarm_b,
    	 0
    );

    jpf_app_mod_register_msg(
        super_self,
    	 MESSAGE_GET_MDS_INFO,
    	 NULL,
    	 jpf_dbs_pu_get_mds_info_b,
    	 0
    );

    jpf_app_mod_register_msg(
        super_self,
    	 MSG_GET_DEVICE_MANUFACT,
    	 NULL,
    	 jpf_dbs_pu_get_device_manufact_b,
    	 0
    );

    jpf_app_mod_register_msg(
        super_self,
    	 MESSAGE_PU_GET_DIV_MODE,
    	 NULL,
    	 jpf_dbs_pu_get_div_mode_b,
    	 0
    );


}

