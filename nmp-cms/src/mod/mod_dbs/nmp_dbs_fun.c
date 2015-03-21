#include "nmp_mysql_fun.h"
#include "nmp_internal_msg.h"
#include "nmp_errno.h"
#include "nmp_shared.h"
#include "nmp_dbs_fun.h"
#include "nmp_memory.h"


#define DEFAULT_CMS_IP  "0.0.0.0"
#define RESOURCE_NO_LIMITTED  (-1)

void
nmp_dbs_modify_sysmsg(NmpSysMsg *sys_msg,
					 gpointer   priv_data,
					 gsize size,
					 NmpBusSlotPos src_pos,
					 NmpBusSlotPos dst_pos,
					 NmpMsgPrivDes msg_priv_destroy
				     )
{
	nmp_sysmsg_set_private(sys_msg, priv_data, size, msg_priv_destroy);
	MSG_SET_DSTPOS(sys_msg, dst_pos);
	MSG_SET_RESPONSE(sys_msg);
}


void
nmp_dbs_modify_sysmsg_2(NmpSysMsg *sys_msg,
					 gpointer   priv_data,
					 gsize size,
					 NmpBusSlotPos src_pos,
					 NmpBusSlotPos dst_pos
				     )
{
	gint ret;
	ret = nmp_sysmsg_set_private_2(sys_msg, priv_data, size);
	printf("nmp_dbs_modify_sysmsg_2 ret = %d\n",ret);
	MSG_SET_DSTPOS(sys_msg, dst_pos);
	MSG_SET_RESPONSE(sys_msg);
}


gint
nmp_get_mds_ip(NmpMysqlRes *mysql_result, NmpMsgGetMdsIpRes *res_info)
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
                    nmp_warning(
                         "Ignore table field %s while getting user info",name
                    );
            }
        }
         return 0;
    }
}



gint nmp_dbs_get_mds_info(NmpAppObj *app_obj, gchar *cms_ip,
    gchar *puid, gchar *domain_id, NmpMsgGetMdsIpRes *mds_ip)
{
    NmpMysqlRes *mysql_res;
    char query_buf[QUERY_STR_LEN] = {0};
    gint code = 0;

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select t1.mdu_ip,t1.mdu_id, t2.mdu_rtsp_port from %s as t1,%s as t2 where t1.mdu_cmsip='%s' and \
        t1.mdu_id=(select pu_mdu from %s where pu_id='%s' and pu_domain='%s' ) and t1.mdu_id=t2.mdu_id",
        MDS_IP_TABLE, MDS_TABLE, cms_ip, PU_TABLE, puid, domain_id
    );
    printf("-------------get mdu id :%s\n",query_buf);
    mysql_res = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!mysql_res);

    if (G_LIKELY(!MYSQL_RESULT_CODE(mysql_res)))  //success:0 fail:!0
   {
        code = nmp_get_mds_ip(mysql_res, mds_ip);
	 if (code == -E_NODBENT)
	 {
             nmp_sql_put_res(mysql_res, sizeof(NmpMysqlRes));
             snprintf(
                query_buf, QUERY_STR_LEN,
                "select t1.mdu_ip,t1.mdu_id, t2.mdu_rtsp_port from %s as t1,%s as t2 where t1.mdu_cmsip='%s' and \
                t1.mdu_id=(select pu_mdu from %s where pu_id='%s' and pu_domain='%s' ) and t1.mdu_id=t2.mdu_id",
                MDS_IP_TABLE, MDS_TABLE, DEFAULT_CMS_IP, PU_TABLE, puid, domain_id
            );
            printf("-------------get mdu id :%s\n",query_buf);
            mysql_res = nmp_dbs_do_query_res(app_obj, query_buf);
            BUG_ON(!mysql_res);
            if (G_LIKELY(!MYSQL_RESULT_CODE(mysql_res)))  //success:0 fail:!0
            {
                code = nmp_get_mds_ip(mysql_res, mds_ip);
            }
	    else
               code = MYSQL_RESULT_CODE(mysql_res);
	 }
   }
    else
        code = MYSQL_RESULT_CODE(mysql_res);
    if (mysql_res)
        nmp_sql_put_res(mysql_res, sizeof(NmpMysqlRes));

    return code;
}


static __inline__ gint
nmp_get_display_guid(NmpMysqlRes *mysql_result,
                        tw_general_guid *guid)
{
    unsigned long field_num;
    NmpMysqlField *mysql_fields;    //modify: mysql_fields -> *msql_fields
    NmpMysqlRow   mysql_row;
    char *value, *name;
    int j,row_num;

    row_num = nmp_sql_get_num_rows(mysql_result);

    if (row_num == 0)
    {
        nmp_warning("<GetUserGroupInfo>No such record entry in database");
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
                if (!strcmp(name, "dis_guid"))
                {
                    value = nmp_sql_get_field_value(mysql_row, j);
                    if(value)
                    {
                        guid->guid[TW_ID_LEN - 1] = 0;
                        strncpy(guid->guid, value, TW_ID_LEN - 1);
                    }
                }
                else if (!strcmp(name, "dis_domain"))
                {
                    value = nmp_sql_get_field_value(mysql_row, j);
                    if(value)
                    {
                        guid->domain_id[TW_ID_LEN - 1] = 0;
                        strncpy(guid->domain_id, value, TW_ID_LEN - 1);
                    }
                }
                else
                {
                    cms_debug(
                          "Ignore table field %s while getting user group info.",
                          name
                    );
                }
            }
        }
         return 0;
    }
}

gint
nmp_get_scr_display_guid(NmpAppObj *app_obj,
                        tw_general_guid *guid, gint screen_id, gint tw_id)
{
    NmpMysqlRes *mysql_res;
    char query_buf[QUERY_STR_LEN] = {0};
    int code;

    snprintf(query_buf, QUERY_STR_LEN,
        "select dis_guid,dis_domain from %s where scr_id=%d and tw_id=%d",
        SCREEN_TABLE, screen_id,tw_id
    );

    mysql_res = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!mysql_res);

    if (G_LIKELY(!MYSQL_RESULT_CODE(mysql_res)))  //success:0 fail:!0
        code = nmp_get_display_guid(mysql_res, guid);
    else
        code = MYSQL_RESULT_CODE(mysql_res);

    nmp_sql_put_res(mysql_res, sizeof(NmpMysqlRes));

    return code;
}


static __inline__ gint
nmp_get_cms_ip(NmpMysqlRes *mysql_result,
                        gchar *cms_ip)
{
    unsigned long field_num;
    NmpMysqlField *mysql_fields;    //modify: mysql_fields -> *msql_fields
    NmpMysqlRow   mysql_row;
    char *value, *name;
    int j,row_num;

    row_num = nmp_sql_get_num_rows(mysql_result);

    if (row_num == 0)
    {
        nmp_warning("<GetUserGroupInfo>No such record entry in database");
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
                if (!strcmp(name, "pu_last_cmsip"))
                {
                    value = nmp_sql_get_field_value(mysql_row, j);
                    cms_ip[MAX_IP_LEN - 1] = 0;
                    if (value)
                        strncpy(cms_ip, value, MAX_IP_LEN - 1);

                    break;
                }
                else
                {
                    cms_debug(
                          "Ignore table field %s while getting user group info.",
                          name
                    );
                }
            }
        }
         return 0;
    }
}

gint
nmp_get_pu_cms_ip(NmpAppObj *app_obj, gchar *puid,
	char *domain_id, gchar *cms_ip)
{
    NmpMysqlRes *mysql_res;
    char query_buf[QUERY_STR_LEN] = {0};
    int code;

    snprintf(query_buf, QUERY_STR_LEN,
        "select pu_last_cmsip from %s where pu_id='%s' and pu_domain='%s'",
        PU_RUNNING_TABLE, puid, domain_id
    );
    mysql_res = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!mysql_res);

    if (G_LIKELY(!MYSQL_RESULT_CODE(mysql_res)))  //success:0 fail:!0
        code = nmp_get_cms_ip(mysql_res, cms_ip);
    else
        code = MYSQL_RESULT_CODE(mysql_res);

    nmp_sql_put_res(mysql_res, sizeof(NmpMysqlRes));

    return code;
}


static __inline__ void
nmp_get_div_mode_info(NmpMysqlRes *result,
                   gint row_num1,
                   NmpGetDivModeRes *query_res
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
        for(field_no = 0; field_no < field_num; field_no++)
        {
            name = nmp_sql_get_field_name(mysql_fields,field_no);
            if (!strcmp(name, "div_id"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->scr_div_info[info_no].div_id= atoi(value);
            }
            else if (!strcmp(name, "div_name"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->scr_div_info[info_no].div_name[DIV_NAME_LEN - 1] = 0;
                    strncpy(
                        query_res->scr_div_info[info_no].div_name,
                        value, DIV_NAME_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "description"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->scr_div_info[info_no].description[DESCRIPTION_INFO_LEN - 1] = 0;
                    strncpy(
                        query_res->scr_div_info[info_no].description,
                        value, DESCRIPTION_INFO_LEN - 1
                    );
                }
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }

        info_no++;
    }
}


NmpGetDivModeRes *
nmp_dbs_get_div_mode(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpGetDivModeRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpGetDivModeRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
	 query_res->back_num = 0;
    }
    else
    {
        len = sizeof(NmpGetDivModeRes) + row_num*sizeof(NmpDivMode);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
	 query_res->total_num = row_num;
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);
        nmp_get_div_mode_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


gint
nmp_dbs_get_dev_type_count(NmpAppObj *app_obj, gchar *dev_type)
{
    char query_buf[QUERY_STR_LEN];
    gint num;

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select count(*)  as count from %s where pu_id like '%%-%s-%%'",
        PU_TABLE, dev_type
    );

    num =  nmp_get_record_count(app_obj, query_buf);

    return num;
}


gint
nmp_dbs_get_online_dev_type_count(NmpAppObj *app_obj, gchar *dev_type)
{
    char query_buf[QUERY_STR_LEN];
    gint num;

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select count(*)  as count from %s as t1,%s as t2 where t1.pu_id like '%%-%s-%%' \
         and pu_state=1 and t1.pu_id=t2.pu_id and t1.pu_domain=t2.pu_domain",
        PU_TABLE, PU_RUNNING_TABLE, dev_type
    );

    num =  nmp_get_record_count(app_obj, query_buf);

    return num;
}


gint
nmp_dbs_get_dev_total_count(NmpAppObj *app_obj)
{
    char query_buf[QUERY_STR_LEN];
    gint num;

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select count(*)  as count from %s",
        PU_TABLE
    );

    num =  nmp_get_record_count(app_obj, query_buf);

    return num;
}


gint
nmp_dbs_get_online_dev_total_count(NmpAppObj *app_obj)
{
    char query_buf[QUERY_STR_LEN];
    gint num;

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select count(*)  as count from %s as t1,%s as t2 where \
        t2.pu_state=1 and t1.pu_id=t2.pu_id and t1.pu_domain=t2.pu_domain",
        PU_TABLE,PU_RUNNING_TABLE
    );

    num =  nmp_get_record_count(app_obj, query_buf);

    return num;
}


gint
nmp_dbs_get_gu_type_count(NmpAppObj *app_obj,  gchar *gu_type)
{
    char query_buf[QUERY_STR_LEN];
    gint num;

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select count(*)  as count from %s where gu_id like '%%-%s-%%'",
        GU_TABLE, gu_type
    );

    num =  nmp_get_record_count(app_obj, query_buf);

    return num;
}


gint
nmp_dbs_check_gu_type_count(NmpAppObj *app_obj,
	gint add_count, gint total_num, gchar *gu_type)
{
    char query_buf[QUERY_STR_LEN];
    gint num;

    if (total_num == RESOURCE_NO_LIMITTED)
        return 0;

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select count(*)  as count from %s where gu_id like '%%-%s-%%'",
        GU_TABLE, gu_type
    );

    num =  nmp_get_record_count(app_obj, query_buf);
    if (num < 0)
        return num;
    if ((num + add_count) > total_num)
    {
        return -1;
    }

    return 0;
}


void
nmp_mod_dbs_deliver_out_msg(NmpAppObj *self, NmpSysMsg *msg)
{
    nmp_app_obj_deliver_out(self, msg);
}

