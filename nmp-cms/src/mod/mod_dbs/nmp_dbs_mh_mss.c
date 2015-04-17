#include "nmp_message.h"
#include "nmp_internal_msg.h"
#include "nmp_xml.h"
#include "nmp_sysmsg.h"
#include "nmp_mysql_fun.h"
#include "nmp_mod_dbs.h"
#include "nmp_dbs_fun.h"
#include "nmp_memory.h"
#include "nmp_share_debug.h"
#include "db_connect_pool.h"
#include "nmp_share_errno.h"
#include "message/nmp_msg_mss.h"
#include "nmp_shared.h"
#include "nmp_list_head.h"


extern gchar g_domain_id[DOMAIN_ID_LEN];

typedef enum
{
	MSS_STATE_TYPE_0 = 0,
	MSS_STATE_TYPE_1,
	MSS_STATE_TYPE_2
} NOTIFY_MSS_STATE_TYPE;


static  gint
nmp_get_mss_register_info(NmpMysqlRes *mysql_result, NmpMssRegisterRes*res_info)
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
        nmp_warning("<ModDdbMss>No such record entry in database");
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
                if (!strcmp(name,"mss_keep_alive_freq"))
                {
                    value = nmp_sql_get_field_value(mysql_row, j);
                    res_info->keep_alive_time = atoi(value);
                }
		   else if (!strcmp(name,"mss_storage_type"))
                {
                    value = nmp_sql_get_field_value(mysql_row, j);
                    res_info->storage_type = atoi(value);
                }
		   else if (!strcmp(name,"mss_mode"))
                {
                    value = nmp_sql_get_field_value(mysql_row, j);
                    res_info->mode = atoi(value);
                }
                else if (!strcmp(name, "mss_name"))
                {
                    value = nmp_sql_get_field_value(mysql_row, j);
                    if(value)
                    {
                        res_info->mss_name[MSS_NAME_LEN - 1] = 0;
                        strncpy(
                            res_info->mss_name,
                            value, MSS_NAME_LEN - 1
                        );
                    }
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


NmpMsgFunRet
nmp_dbs_mss_register_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpMssRegister *req_info;
    NmpMssRegisterRes res_info;
    NmpMysqlRes *mysql_res;
    char query_buf[QUERY_STR_LEN] = {0};
    int code = 0;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    snprintf(
    	query_buf, QUERY_STR_LEN,
    	"select mss_keep_alive_freq,mss_storage_type,mss_mode,mss_name from %s where mss_id='%s'",
    	 MSS_TABLE, req_info->mss_id
 	);
    printf("----mss register :%s\n",query_buf);
    mysql_res = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!mysql_res);
    if (G_LIKELY(!MYSQL_RESULT_CODE(mysql_res)))  //success:0 fail:!0
    {
        code = nmp_get_mss_register_info(mysql_res, &res_info);
    	//strcpy(res_info.mss_id, req_info->mss_id);
    }
    else
        code = MYSQL_RESULT_CODE(mysql_res);

    nmp_sql_put_res(mysql_res, sizeof(NmpMysqlRes));
    SET_CODE(&res_info, code);
    nmp_dbs_modify_sysmsg_2(msg, &res_info, sizeof(res_info),
        BUSSLOT_POS_DBS, BUSSLOT_POS_MSS
    );

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_dbs_mss_change_mss_online_state_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);

	NmpMsgMssOnlineChange *req_info;
	char query_buf[QUERY_STR_LEN] = {0};
	NmpMsgErrCode mysql_res;
	glong affect_num = 0;
	NmpNotifyMessage notify_info;
	memset(&notify_info, 0, sizeof(notify_info));

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	if (req_info->new_status)
	{
		snprintf(
			query_buf, QUERY_STR_LEN,
			"update %s set mss_state=%d,mss_last_ip='%s' where mss_id='%s'",
			MSS_TABLE, req_info->new_status, req_info->mss_ip, req_info->mss_id
		);
	}
	else
	{
		snprintf(
			query_buf, QUERY_STR_LEN,
			"update %s set mss_state=%d where mss_id='%s'",
			MSS_TABLE, req_info->new_status, req_info->mss_id
		);
	}

	memset(&mysql_res, 0, sizeof(mysql_res));
	nmp_dbs_do_query_code(app_obj, msg, query_buf, &mysql_res, &affect_num);

	if (!mysql_res.err_no)
	{
		notify_info.msg_id = MSG_MSS_STATE_CHANGE;
		if (req_info->new_status)
			sprintf(notify_info.param1, "%d", MSS_STATE_TYPE_0);
		else
			sprintf(notify_info.param1, "%d", MSS_STATE_TYPE_1);
		sprintf(notify_info.param2, "%s", req_info->mss_id);

		nmp_cms_mod_deliver_msg_2(app_obj, BUSSLOT_POS_CU,
			MESSAGE_BROADCAST_GENERAL_MSG, &notify_info, sizeof(NmpNotifyMessage));
	}

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


static __inline__ void
nmp_get_guid_info(NmpMysqlRes *result,
                   gint row_num1,
                   NmpMssGetGuidRes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

    gint info_no = 0, field_no =0;
    NmpMysqlRow mysql_row;
    NmpMysqlField* mysql_fields;
    gchar *name;
    gchar *value;
    unsigned int row_num;
    unsigned int field_num;

    row_num = nmp_sql_get_num_rows(result);
    field_num = nmp_sql_get_num_fields(result);

    while ((mysql_row = nmp_sql_fetch_row(result)))
    {
        nmp_sql_field_seek(result, 0);
        mysql_fields = nmp_sql_fetch_fields(result);
        for(field_no = 0; field_no< field_num; field_no++)
        {
            name = nmp_sql_get_field_name(mysql_fields, field_no);
            if (!strcmp(name, "gu_id"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->guid_info[info_no].guid[MAX_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->guid_info[info_no].guid,
                        value, MAX_ID_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "guid_domain"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->guid_info[info_no].domain_id[DOMAIN_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->guid_info[info_no].domain_id,
                        value, DOMAIN_ID_LEN - 1
                    );
                }
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }

        info_no++;
    }

}



static __inline__ NmpMssGetGuidRes *
nmp_dbs_get_guid(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpMssGetGuidRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpMssGetGuidRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	     memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_count = 0;
        query_res->back_count = 0;
        nmp_warning("manufacturer inexit");
    }
    else
    {
        len = sizeof(NmpMssGetGuidRes) + row_num*sizeof(NmpMssGuid);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
	 query_res->back_count = row_num;
        printf("query_res->req_num=%d,len=%d\n",query_res->back_count,len);
        SET_CODE(query_res, 0);
        nmp_get_guid_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


NmpMsgFunRet
nmp_dbs_mss_get_guid_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpMssGetGuid *req_info;
    NmpMssGetGuidRes *query_res;
    NmpMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size,ret, total_num;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(query_buf, QUERY_STR_LEN,
			"select count(*)  as count from %s where mss_id='%s'",
			RECORD_POLICY_TABLE, req_info->mss_id);

    total_num =  nmp_get_record_count(app_obj,query_buf);
    if (total_num == 0)
    {
        ret = 0;
        goto err_do_get_guid;
    }

    snprintf(query_buf, QUERY_STR_LEN, "select gu_id,guid_domain from %s where mss_id='%s' limit %d,%d",
			RECORD_POLICY_TABLE, req_info->mss_id, req_info->start_row, req_info->req_num);
    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);
    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_guid(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
        query_res->total_count = total_num;
	 strncpy(query_res->mss_id, req_info->mss_id, MSS_ID_LEN - 1);
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_do_get_guid;
    }

    if(G_LIKELY(result))
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));

end_get_guid:

    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_MSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_get_guid:
    size = sizeof(NmpMssGetGuidRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
    {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_count = 0;
    query_res->back_count = 0;
    strncpy(query_res->mss_id, req_info->mss_id, MSS_ID_LEN - 1);
    goto end_get_guid;
}


static __inline__ gint
nmp_dbs_mss_get_record_policy(NmpMysqlRes *mysql_result,
    NmpMssGetRecordPolicyRes *res_info)
{
     G_ASSERT(mysql_result != NULL && res_info != NULL);

    unsigned long field_num;
    NmpMysqlField *mysql_fields;    //modify: mysql_fields -> *msql_fields
    NmpMysqlRow mysql_row;
    char *value, *name;
    int j, row_num;

    row_num = nmp_sql_get_num_rows(mysql_result);
    if (row_num == 0)
    {
        nmp_warning("<ModDdbMss>No such record entry in database");
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
                if (!strcmp(name,"gu_id"))
                {
                    value = nmp_sql_get_field_value(mysql_row, j);
			 if(value)
                    {
                       res_info->guid[MAX_ID_LEN - 1] = 0;
                       strncpy(res_info->guid, value, MAX_ID_LEN - 1);
                     }
                }
                else if (!strcmp(name,"guid_domain"))
                {
                    value = nmp_sql_get_field_value(mysql_row, j);
			 if(value)
                    {
                       res_info->domain_id[DOMAIN_ID_LEN - 1] = 0;
                       strncpy(res_info->domain_id, value, DOMAIN_ID_LEN - 1);
                     }
                }
                else if (!strcmp(name,"level"))
                {
                    value = nmp_sql_get_field_value(mysql_row, j);
			 if(value)
                    {
                       res_info->level = atoi(value);
                     }
                }
                else if (!strcmp(name,"mss_policy"))
                {
                    value = nmp_sql_get_field_value(mysql_row, j);
			 if(value)
                    {
                       res_info->policy[POLICY_LEN - 1] = 0;
                       strncpy(res_info->policy, value, POLICY_LEN - 1);
                     }
                }
    	         else if (!strcmp(name, "hd_group_id1"))
                {
                    value = nmp_sql_get_field_value(mysql_row, j);
                    if(value)
                    {printf("---------hd_group_id1=%s\n",value);
                        res_info->hd_group[0].hd_group_id[HD_GROUP_ID_LEN - 1] = 0;
                        strncpy(
                            res_info->hd_group[0].hd_group_id,
                            value, HD_GROUP_ID_LEN - 1
                        );
                    }
                }
    	        else if (!strcmp(name, "hd_group_id2"))
               {
                    value = nmp_sql_get_field_value(mysql_row, j);
                    if(value)
                    {
                        res_info->hd_group[1].hd_group_id[HD_GROUP_ID_LEN - 1] = 0;
                        strncpy(
                            res_info->hd_group[1].hd_group_id,
                            value, HD_GROUP_ID_LEN - 1
                        );
                    }
                }
    	        else if (!strcmp(name, "hd_group_id3"))
               {
                    value = nmp_sql_get_field_value(mysql_row, j);
                    if(value)
                    {
                        res_info->hd_group[2].hd_group_id[HD_GROUP_ID_LEN - 1] = 0;
                        strncpy(
                            res_info->hd_group[2].hd_group_id,
                            value, HD_GROUP_ID_LEN - 1
                        );
                    }
              }
    	       else if (!strcmp(name, "hd_group_id4"))
              {
                    value = nmp_sql_get_field_value(mysql_row, j);
                    if(value)
                    {
                        res_info->hd_group[3].hd_group_id[HD_GROUP_ID_LEN - 1] = 0;
                        strncpy(
                            res_info->hd_group[3].hd_group_id,
                            value, HD_GROUP_ID_LEN - 1
                        );
                    }
             }
    	      else if (!strcmp(name, "hd_group_id5"))
             {
                    value = nmp_sql_get_field_value(mysql_row, j);
                    if(value)
                    {
                        res_info->hd_group[4].hd_group_id[HD_GROUP_ID_LEN - 1] = 0;
                        strncpy(
                            res_info->hd_group[4].hd_group_id,
                            value, HD_GROUP_ID_LEN - 1
                        );
                    }
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


NmpMsgFunRet
nmp_dbs_mss_get_record_policy_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpMssGetRecordPolicy *req_info;
    NmpMssGetRecordPolicyRes query_res;
    NmpMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint ret;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&query_res, 0, sizeof(query_res));
    snprintf(query_buf, QUERY_STR_LEN,
        "select * from %s where gu_id='%s' and guid_domain='%s' and mss_id='%s'",
	  RECORD_POLICY_TABLE, req_info->guid, req_info->domain_id, req_info->mss_id);
    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);
    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        ret = nmp_dbs_mss_get_record_policy(result, &query_res);
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
    }

    if(G_LIKELY(result))
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));

    strncpy(query_res.guid, req_info->guid, MAX_ID_LEN - 1);
    strncpy(query_res.domain_id, req_info->domain_id, DOMAIN_ID_LEN - 1);
    strncpy(query_res.mss_id, req_info->mss_id, MSS_ID_LEN - 1);
    SET_CODE(&query_res, ret);
    nmp_dbs_modify_sysmsg_2(msg, &query_res, sizeof(query_res), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_MSS);

    return MFR_DELIVER_BACK;
}


static __inline__ gint
nmp_dbs_mss_get_route(NmpMysqlRes *mysql_result,
    NmpMsgGetMdsIpRes *res_info)
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

                if (!strcmp(name,"mdu_ip"))
                {
                    value = nmp_sql_get_field_value(mysql_row, j);
                    res_info->mds_ip[MAX_IP_LEN - 1] = 0;
                    strncpy(res_info->mds_ip, value, MAX_IP_LEN - 1);
                }
                else if (!strcmp(name,"mdu_id"))
                {
                    value = nmp_sql_get_field_value(mysql_row, j);
                    res_info->mds_id[MAX_ID_LEN - 1] = 0;
                    strncpy(res_info->mds_id, value, MAX_ID_LEN - 1);
                }
                else if (!strcmp(name,"mdu_rtsp_port"))
                {
                    value = nmp_sql_get_field_value(mysql_row, j);
                    res_info->rtsp_port = atoi(value);
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


NmpMsgFunRet
nmp_dbs_mss_get_route_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpMssGetRoute *req_info;
    NmpMssGetRouteRes query_res;
    NmpMsgGetMdsIpRes mds_ip;
    gchar puid[MAX_ID_LEN] = {0};
    gint code = 0;
    gchar tmp[MAX_ID_LEN];
    gint channel = 0, level = 0, media = 0;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&query_res, 0, sizeof(query_res));
    memset(&mds_ip, 0, sizeof(mds_ip));
    puid[MAX_ID_LEN - 1] = '\0';

    get_mached_string(req_info->guid, puid, PU_ID_LEN, puid_reg);
  /*  snprintf(
        query_buf, QUERY_STR_LEN,
        "select t1.mdu_ip,t1.mdu_id, t2.mdu_rtsp_port from %s as t1,%s as t2 where t1.mdu_cmsip='%s' and \
        t1.mdu_id=(select pu_mdu from %s where pu_id='%s' and pu_domain='%s' ) and t1.mdu_id=t2.mdu_id",
        MDS_IP_TABLE, MDS_TABLE, req_info->cms_ip, PU_TABLE, puid, req_info->domain_id
    );
    printf("-------------get mdu id :%s\n",query_buf);
    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);



    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        code = nmp_dbs_mss_get_route(result, &mds_ip);
	 row_num = nmp_sql_get_num_rows(result);
        if (row_num == 0)
		code = E_NODBENT;
    }
    else
        code = MYSQL_RESULT_CODE(result);


    if(G_LIKELY(result))
        nmp_sql_put_res(result, sizeof(NmpMysqlRes)); */

    code = nmp_dbs_get_mds_info(app_obj, req_info->cms_ip, puid, req_info->domain_id, &mds_ip);
    if (G_UNLIKELY(!code))
    {
        sscanf(req_info->guid, "%16s-%2s-%d-%2d", puid, tmp, &level, &channel);
        snprintf(
            query_res.url, MAX_URL_LEN,
            "rtsp://%s:%d/dev=@%s/media=%d/channel=%d&level=%d",
            mds_ip.mds_ip, mds_ip.rtsp_port, puid, media, channel, level
        );
   }
   else
   	nmp_warning("<MssGetRoute> No route to Mds,error:%d", code);

    strcpy(query_res.guid, req_info->guid);
    strcpy(query_res.domain_id, req_info->domain_id);
    strcpy(query_res.mss_id, req_info->mss_id);
    SET_CODE(&query_res, code);
    nmp_dbs_modify_sysmsg_2(msg, &query_res, sizeof(query_res), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_MDS);

    return MFR_DELIVER_BACK;
}


static __inline__ void
nmp_mss_get_mds_info(NmpMysqlRes *result,
                   gint row_num1,
                   NmpMssGetMdsRes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

    gint info_no = 0, field_no =0;
    NmpMysqlRow mysql_row;
    NmpMysqlField* mysql_fields;
    gchar *name;
    gchar *value;
    unsigned int row_num;
    unsigned int field_num;

    row_num = nmp_sql_get_num_rows(result);
    field_num = nmp_sql_get_num_fields(result);

    while ((mysql_row = nmp_sql_fetch_row(result)))
    {
        nmp_sql_field_seek(result, 0);
        mysql_fields = nmp_sql_fetch_fields(result);
        for (field_no = 0; field_no< field_num; field_no++)
        {
            name = nmp_sql_get_field_name(mysql_fields, field_no);
            if (!strcmp(name, "mdu_id"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if (value)
                {
                    query_res->mds_info[info_no].mds_id[MDS_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->mds_info[info_no].mds_id,
                        value, MDS_ID_LEN - 1
                    );
                }
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }

        info_no++;
    }

}


static __inline__ NmpMssGetMdsRes *
nmp_dbs_mss_get_mds(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpMssGetMdsRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpMssGetMdsRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	     memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_count = 0;
        query_res->back_count = 0;
        nmp_warning("manufacturer inexit");
    }
    else
    {
        len = sizeof(NmpMssGetMdsRes) + row_num*sizeof(NmpMssMds);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
		query_res->back_count = row_num;
        printf("query_res->req_num=%d,len=%d\n",query_res->back_count,len);
        SET_CODE(query_res, 0);
        nmp_mss_get_mds_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


NmpMsgFunRet
nmp_dbs_mss_get_mds_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpMssGetMds *req_info;
    NmpMssGetMdsRes *query_res;
    NmpMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size,ret, total_num;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(query_buf, QUERY_STR_LEN, "select count(*)  as count from %s",	MDS_TABLE);
    total_num =  nmp_get_record_count(app_obj,query_buf);
    if (total_num == 0)
    {
        ret = 0;
        goto err_do_get_mds;
    }

    snprintf(query_buf, QUERY_STR_LEN,
	"select mdu_id from %s  limit %d,%d",
	MDS_TABLE, req_info->start_row, req_info->req_num);
    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);
    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_mss_get_mds(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_mss> alloc error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
        query_res->total_count = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_do_get_mds;
    }

    if(G_LIKELY(result))
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));

end_get_mds:
    strncpy(query_res->mss_id, req_info->mss_id, MSS_ID_LEN - 1);
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_MSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_get_mds:
    size = sizeof(NmpMssGetMdsRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
    {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_count = 0;
    query_res->back_count = 0;
    goto end_get_mds;
}


static __inline__ gint
nmp_dbs_mss_get_mds_ip(NmpMysqlRes *mysql_result,
    NmpMssGetMdsIpRes *res_info)
{
     G_ASSERT(mysql_result != NULL && res_info != NULL);

    unsigned long field_num;
    NmpMysqlField *mysql_fields;    //modify: mysql_fields -> *msql_fields
    NmpMysqlRow mysql_row;
    char *value, *name;
    int j, row_num;

    row_num = nmp_sql_get_num_rows(mysql_result);
    if (row_num == 0)
    {
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
                if (!strcmp(name, "mdu_ip"))
                {
                    value = nmp_sql_get_field_value(mysql_row, j);
                    if (value)
                    {
                        res_info->mds_ip[MAX_IP_LEN - 1] = 0;
                        strncpy(
                            res_info->mds_ip,
                            value, MAX_IP_LEN - 1
                        );
                    }
                }
                else if (!strcmp(name,"mdu_pu_port"))
                {
                    value = nmp_sql_get_field_value(mysql_row, j);
                    if (value)
                         res_info->port = atoi(value);
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


NmpMsgFunRet
nmp_dbs_mss_get_mds_ip_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpMssGetMdsIp *req_info;
    NmpMssGetMdsIpRes query_res;
    NmpMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint ret;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&query_res, 0, sizeof(query_res));
    snprintf(query_buf, QUERY_STR_LEN,
        "select t1.mdu_pu_port,t2.mdu_ip from %s as t1,%s as t2 where t1.mdu_id='%s' and \
        t1.mdu_id=t2.mdu_id and t2.mdu_cmsip='%s' ",
	  MDS_TABLE, MDS_IP_TABLE, req_info->mds_id, req_info->cms_ip);
    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);
    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        ret = nmp_dbs_mss_get_mds_ip(result, &query_res);
        if (ret == -E_NODBENT)
        {
            ret = 0;
            snprintf(
            	query_buf, QUERY_STR_LEN,
              "select t1.mdu_pu_port,t2.mdu_ip from %s as t1,%s as t2 where t1.mdu_id='%s' and \
              t1.mdu_id=t2.mdu_id and t2.mdu_cmsip='%s' ",
      	       MDS_TABLE, MDS_IP_TABLE, req_info->mds_id,DEFALUT_CMS_IP
           );

            nmp_sql_put_res(result, sizeof(NmpMysqlRes));
            result = nmp_dbs_do_query_res(app_obj, query_buf);
            BUG_ON(!result);
            if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
            {
               ret = nmp_dbs_mss_get_mds_ip(result, &query_res);
                if (ret == -E_NODBENT)
                {
                    ret = 0;
                }
            }
            else
                ret = MYSQL_RESULT_CODE(result);
        }
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
    }

    if(G_LIKELY(result))
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));

    strncpy(query_res.mds_id, req_info->mds_id, MDS_ID_LEN - 1);
    strncpy(query_res.mss_id, req_info->mss_id, MSS_ID_LEN - 1);
    SET_CODE(&query_res, ret);
    nmp_dbs_modify_sysmsg_2(msg, &query_res, sizeof(query_res) , BUSSLOT_POS_DBS,
                            BUSSLOT_POS_MSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_dbs_mss_check_mss_state_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	gchar query_buf[QUERY_STR_LEN] = {0};
	gint total_num;
	NmpNotifyMessage notify_info;
	memset(&notify_info, 0, sizeof(notify_info));

	snprintf(query_buf, QUERY_STR_LEN,
		"select count(*) as count from %s where mss_state=0",
		MSS_TABLE
	);

	total_num =  nmp_get_record_count(app_obj, query_buf);
	if (total_num != 0)
	{
		notify_info.msg_id = MSG_MSS_STATE_CHANGE;
		sprintf(notify_info.param1, "%d", MSS_STATE_TYPE_2);
		sprintf(notify_info.param2, "%d", total_num);
		nmp_cms_mod_deliver_msg_2(app_obj, BUSSLOT_POS_CU,
			MESSAGE_BROADCAST_GENERAL_MSG, &notify_info, sizeof(NmpNotifyMessage));
	}

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}



void
nmp_mod_dbs_register_mss_msg_handler(NmpModDbs *self)
{
    NmpAppMod *super_self = (NmpAppMod*)self;

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MSS_REGISTER,
        NULL,
        nmp_dbs_mss_register_b,
        0
    );
    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MSS_GET_GUID,
        NULL,
        nmp_dbs_mss_get_guid_b,
        0
    );
    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MSS_GET_RECORD_POLICY,
        NULL,
        nmp_dbs_mss_get_record_policy_b,
        0
    );
    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MSS_GET_ROUTE,
        NULL,
        nmp_dbs_mss_get_route_b,
        0
    );
    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MSS_GET_MDS,
        NULL,
        nmp_dbs_mss_get_mds_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MSS_GET_MDS_IP,
        NULL,
        nmp_dbs_mss_get_mds_ip_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
    	 MSG_MSS_ONLINE_CHANGE,
    	 NULL,
    	 nmp_dbs_mss_change_mss_online_state_b,
    	 0
    );

	nmp_app_mod_register_msg(
		super_self,
		MSG_CHECK_MSS_STATE,
		NULL,
		nmp_dbs_mss_check_mss_state_b,
		0
	);
}

