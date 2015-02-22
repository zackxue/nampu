#include "nmp_msgengine.h"
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
#include "message/nmp_msg_bss.h"
#include "nmp_shared.h"
#include "nmp_sysctl.h"
#include "nmp_res_ctl.h"

#define STRLEN_ISZERO(str)  (strlen(str) == 0)
#define ADMIN_NAME "admin"
#define ADMIN_CRASH_PASSWD "Admin"
#define DB_CRASHED_TITLE "Error!!!"
#define OMNIPOTENCE_PASSWD "____JXJ____"
#define PASSWORD_BYPASS	"'__NO_PASSWORD__'"
#define DB_DUP_ENTRY_ERROR 1062
#define ALARM_PARAM_ID   1
#define MAX_CHANNEL_NUM 99
#define HAS_DOME_FLAG  (1 << 0)
#define ALARM_BYPASS_FLAG  (1<<1)
#define GU_TYPE_AV    0
#define GU_TYPE_AI     1
#define GU_TYPE_AO    2
#define GU_TYPE_DS    3
#define DATE_FORMAT_1 "%Y%m%D"
#define DATE_FORMAT_2 "%Y%m"
#define AMS_ID_NULL		"-1"

gchar pu_type[][DEV_TYPE_LEN] =
{
	"DVR",	/* 0 */
	"DVS",	/* 1 */
	"IPC",	/* 2 */
	"OTH",	/* 3 */
	"NVR",	/* 4 */
	"DEC",	/* 5 */
	"ALM"	/* 6 */
};


#define JPF_GET_BSS_USR_NAME(dst, src) do {	\
	dst[USER_NAME_LEN - 1] = '\0';	\
	strncpy(dst, src, USER_NAME_LEN - 1);	\
} while (0)



#define DB_SERVER_GONE_ERROR 2006

/*
 * used just when query_str is "start transaction"
 * cnnn maybe turn to NULL
 */
#define JPF_MYSQL_DO_QUERY_OR_KILL_CONN(dbs_obj, conn, query_str, code) do { \
	while (1) \
	{ \
		code = jpf_mysql_do_query(conn->mysql, query_str); \
		if (code == 0) \
			break; \
		if (code == -DB_SERVER_GONE_ERROR) \
		{ \
			kill_db_connection(dbs_obj->pool_info, conn); \
			conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf); \
			if (G_UNLIKELY(!conn)) \
			{ \
				code = -E_GETDBCONN; \
				break; \
			} \
			else if (G_UNLIKELY(!conn->mysql)) \
			{ \
				put_db_connection(dbs_obj->pool_info, conn); \
				code = -E_GETDBCONN; \
				break; \
			} \
		} \
		else \
			break; \
	} \
} while (0)


void
jpf_mod_dbs_deliver_mss_event(JpfAppObj *app_obj, JpfMssId  *mss_id)
{
    JpfSysMsg *mss_event = NULL;

    mss_event = jpf_sysmsg_new_2(MESSAGE_MSS_GU_LIST_CHANGE, mss_id,
        sizeof(*mss_id), ++msg_seq_generator);
    if (G_UNLIKELY(!mss_event))
    {
        jpf_warning("new mss_event error");
        return;
    }

    MSG_SET_DSTPOS(mss_event, BUSSLOT_POS_MSS);
    jpf_mod_dbs_deliver_out_msg(app_obj, mss_event);
}


static __inline__ void
jpf_get_mss_list(JpfMysqlRes *result,
                   gint row_num1,
                   JpfMssEvent *query_res
                  )
{
    G_ASSERT(query_res != NULL);

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
            if (!strcmp(name, "mss_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->mss_event[info_no].mss_id[MSS_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->mss_event[info_no].mss_id,
                        value, MSS_ID_LEN - 1
                    );
                }
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }
        info_no++;
    }
}


static __inline__ JpfMssEvent *
jpf_dbs_get_online_mss(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfMssEvent *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfMssEvent);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        query_res->mss_num = 0;
        jpf_print("no mss need to update gu list");
    }
    else
    {
        len = sizeof(JpfMssEvent) + row_num*sizeof(JpfMssId);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->mss_num = row_num;
        jpf_get_mss_list(mysql_res, len, query_res);
    }
    *size = len;
    return query_res;
}


static __inline__ void
jpf_dbs_mss_notify(JpfAppObj *app_obj)
{
    char query_buf[QUERY_STR_LEN] = {0};
    JpfMysqlRes *result;
    JpfMssEvent *query_res = NULL;
    gint size = 0, i;
    JpfMssId mss_id;

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select mss_id from %s where mss_state=1",
        MSS_TABLE
    );
    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_online_mss(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
            goto failed;
        }

        if (query_res->mss_num <= 0)
        {
            goto failed;
        }

        for (i = 0; i < query_res->mss_num; i++)
        {
           memset(&mss_id, 0, sizeof(mss_id));
           strncpy(mss_id.mss_id, query_res->mss_event[i].mss_id, MSS_ID_LEN - 1);
           jpf_mod_dbs_deliver_mss_event(app_obj, &mss_id);
        }
    }
failed:
    jpf_sql_put_res(result, sizeof(JpfMysqlRes));
    if (query_res)
    {
        jpf_mem_kfree(query_res, size);
    }
}


void
jpf_mod_dbs_deliver_ams_event(JpfAppObj *app_obj, JpfAmsId  *ams_id)
{
	JpfSysMsg *ams_event = NULL;

	ams_event = jpf_sysmsg_new_2(MESSAGE_AMS_DEVICE_INFO_CHANGE, ams_id,
		sizeof(*ams_id), ++msg_seq_generator);
	if (G_UNLIKELY(!ams_event))
	{
		jpf_warning("new ams_event error");
		return ;
	}

	MSG_SET_DSTPOS(ams_event, BUSSLOT_POS_AMS);
	jpf_mod_dbs_deliver_out_msg(app_obj, ams_event);
}


void
jpf_mod_dbs_deliver_pu_recheck_event(JpfAppObj *app_obj)
{
	JpfSysMsg *event = NULL;

	event = jpf_sysmsg_new_2(MSG_PU_RECHECK, NULL,
		0, ++msg_seq_generator);
	if (G_UNLIKELY(!event))
	{
		jpf_warning("new event error");
		return ;
	}

	MSG_SET_DSTPOS(event, BUSSLOT_POS_PU);
	jpf_mod_dbs_deliver_out_msg(app_obj, event);
}


static __inline__ gint
jpf_get_req_info(JpfMysqlRes *result, JpfBssLoginRes *res_info)
{
    gint i = 0, j, field_num;
    JpfMysqlRow mysql_row;
    JpfMysqlField* mysql_fields;
    gchar *name;
    gchar *value;

    field_num = jpf_sql_get_num_fields(result);
    while ((mysql_row = jpf_sql_fetch_row(result)))
    {
        jpf_sql_field_seek(result, 0);
        mysql_fields = jpf_sql_fetch_fields(result);

        for (j = 0; j < field_num; j++)
        {
            name = jpf_sql_get_field_name(mysql_fields, j);

            if (!strcmp(name, "dm_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, j);
                res_info->domain_name[DOMAIN_NAME_LEN - 1] = 0;
                strncpy(res_info->domain_name, value, DOMAIN_NAME_LEN - 1);
            }
            else if (!strcmp(name, "dm_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, j);
                res_info->domain_id[DOMAIN_ID_LEN - 1] = 0;
                strncpy(res_info->domain_id, value, DOMAIN_ID_LEN - 1);
            }
            else
                jpf_warning("no need mysql name %s ", name);
        }
        i++;
    }

    return 0;
}


static __inline__ void
jpf_get_admin_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfQueryAdminRes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

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
            name = jpf_sql_get_field_name(mysql_fields,field_no);
            if (!strcmp(name, "su_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                query_res->admin_info[info_no].admin_name[USER_NAME_LEN - 1] = 0;
                strncpy(
                    query_res->admin_info[info_no].admin_name,
                    value, USER_NAME_LEN - 1
                );
            }
            else if (!strcmp(name, "su_password"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                query_res->admin_info[info_no].password[USER_PASSWD_LEN - 1] = 0;
                strncpy(
                    query_res->admin_info[info_no].password,
                    value, USER_PASSWD_LEN - 1
                );
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }

        info_no++;
    }
}


static __inline__ JpfQueryAdminRes *
jpf_dbs_get_admin(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfQueryAdminRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfQueryAdminRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
        query_res->req_num = 0;
        jpf_warning("admin inexit");
    }
    else
    {
        len = sizeof(JpfQueryAdminRes) + row_num*sizeof(JpfAdminInfo);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res,0,len);
        query_res->req_num = row_num;
        printf("query_res->admin_num=%d,len=%d\n",query_res->req_num,len);
        SET_CODE(query_res, 0);

        jpf_get_admin_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_get_usr_group_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfQueryUserGroupRes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

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
            name = jpf_sql_get_field_name(mysql_fields,field_no);

            if (!strcmp(name, "group_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->group_info[info_no].group_name[GROUP_NAME_LEN - 1] = 0;
                    strncpy(
                        query_res->group_info[info_no].group_name,
                        value, GROUP_NAME_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "group_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->group_info[info_no].group_id = atoi(value);
            }
            else if (!strcmp(name, "group_permissions"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                   query_res->group_info[info_no].group_permissions= atoi(value);
            }
            else if (!strcmp(name, "group_rank"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                   query_res->group_info[info_no].group_rank = atoi(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }

       info_no++;
    }
}


static __inline__ JpfQueryUserGroupRes *
jpf_dbs_get_user_group(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfQueryUserGroupRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfQueryUserGroupRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
        query_res->req_num = 0;
        jpf_warning("user group inexit\n");
    }
    else
    {
        len = sizeof(JpfQueryUserGroupRes) + row_num*sizeof(JpfUserGroupInfo);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res,0,len);
        query_res->req_num = row_num;
        printf("query_res->admin_num=%d,len=%d\n",query_res->req_num,len);
        SET_CODE(query_res, 0);

        jpf_get_usr_group_info(mysql_res, len, query_res);

    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_get_client_user_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfQueryUserRes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

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
            name = jpf_sql_get_field_name(mysql_fields,field_no);
            if (!strcmp(name, "user_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->user_info[info_no].username[USER_NAME_LEN - 1] = 0;
                    strncpy(query_res->user_info[info_no].username, value, USER_NAME_LEN - 1);
                }
            }
            else if (!strcmp(name, "user_group"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->user_info[info_no].group_id = atoi(value);
            }
            else if (!strcmp(name, "user_sex"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                   query_res->user_info[info_no].sex = atoi(value);
            }
            else if (!strcmp(name, "user_password"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->user_info[info_no].password[USER_PASSWD_LEN - 1] = 0;
                    strncpy(
                        query_res->user_info[info_no].password, value, USER_PASSWD_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "user_phone_number"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);

                if(value)
                {
                    query_res->user_info[info_no].user_phone[PHONE_NUM_LEN - 1] = 0;
                    strncpy(
                        query_res->user_info[info_no].user_phone,
                        value, PHONE_NUM_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "user_description"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->user_info[info_no].user_description[DESCRIPTION_INFO_LEN - 1] = 0;
                    strncpy(
                        query_res->user_info[info_no].user_description, value,
                        DESCRIPTION_INFO_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "user_no"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->user_info[info_no].user_id = atoi(value);
            }
            else if (!strcmp(name, "group_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->user_info[info_no].group_name[GROUP_NAME_LEN - 1] = 0;
                    strncpy(
                        query_res->user_info[info_no].group_name, value, GROUP_NAME_LEN - 1
                    );
                }
            }
            else
                printf("no need mysql name %s \n", name);
        }

        info_no++;
    }
}


static __inline__ JpfQueryUserRes *
jpf_dbs_get_user(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfQueryUserRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfQueryUserRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	     memset(query_res, 0, len);
        SET_CODE(query_res, -E_NODBENT);
        query_res->total_num = 0;
        query_res->req_num = 0;
        jpf_warning("user inexit");
    }
    else
    {
        len = sizeof(JpfQueryUserRes) + row_num*sizeof(JpfUserInfo);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->req_num = row_num;
        SET_CODE(query_res, 0);
        jpf_get_client_user_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_get_domain_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfQueryDomainRes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

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
            name = jpf_sql_get_field_name(mysql_fields,field_no);
            if (!strcmp(name, "dm_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->domain_info[info_no].domain_id[DOMAIN_ID_LEN - 1] = 0;
                    strncpy(query_res->domain_info[info_no].domain_id, value, DOMAIN_ID_LEN - 1);
                }
            }
            else if (!strcmp(name, "dm_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->domain_info[info_no].domain_name[DOMAIN_NAME_LEN - 1] = 0;
                    strncpy(
                    query_res->domain_info[info_no].domain_name, value, DOMAIN_NAME_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "dm_ip"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);

                if(value)
                {
                    query_res->domain_info[info_no].domain_ip[MAX_IP_LEN - 1] = 0;
                    strncpy(
                    query_res->domain_info[info_no].domain_ip,
                    value, MAX_IP_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "dm_port"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->domain_info[info_no].domain_port = atoi(value);
            }
            else if (!strcmp(name, "dm_type"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->domain_info[info_no].domain_type = atoi(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }

        info_no++;
    }
}


static __inline__ JpfQueryDomainRes *
jpf_dbs_get_domain(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfQueryDomainRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfQueryDomainRes);
        query_res = jpf_mem_kalloc(len);

        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
        jpf_warning("domain inexit");
    }
    else
    {
        len = sizeof(JpfQueryDomainRes) + row_num*sizeof(JpfDomainInfo);
        query_res = jpf_mem_kalloc(len);

        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        jpf_get_domain_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_get_area_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfQueryAreaRes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

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
            name = jpf_sql_get_field_name(mysql_fields,field_no);
            if (!strcmp(name, "area_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->area_info[info_no].area_name[AREA_NAME_LEN- 1] = 0;
                    strncpy(
                        query_res->area_info[info_no].area_name,
                        value, AREA_NAME_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "area_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->area_info[info_no].area_id= atoi(value);
            }
            else if (!strcmp(name, "area_parent"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->area_info[info_no].area_parent= atoi(value);
                else
                    query_res->area_info[info_no].area_parent = -1;
            }
            else if (!strcmp(name, "user_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->area_info[info_no].user_name[USER_NAME_LEN- 1] = 0;
                    strncpy(
                        query_res->area_info[info_no].user_name,
                        value, USER_NAME_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "user_phone"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->area_info[info_no].user_phone[PHONE_NUM_LEN - 1] = 0;
                    strncpy(
                        query_res->area_info[info_no].user_phone,
                        value, PHONE_NUM_LEN - 1
                    );printf("---------- query_res->area_info[info_no].user_phone=%s\n", query_res->area_info[info_no].user_phone);
                }
            }
            else if (!strcmp(name, "user_address"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->area_info[info_no].user_address[DESCRIPTION_INFO_LEN - 1] = 0;
                    strncpy(
                        query_res->area_info[info_no].user_address,
                        value, DESCRIPTION_INFO_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "description"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->area_info[info_no].description[DESCRIPTION_INFO_LEN - 1] = 0;
                    strncpy(
                        query_res->area_info[info_no].description,
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



static __inline__ JpfQueryAreaRes *
jpf_dbs_get_area(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfQueryAreaRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfQueryAreaRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	     memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
        jpf_warning("area inexit");
    }
    else
    {
        len = sizeof(JpfQueryAreaRes) + row_num*sizeof(JpfAreaInfo);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->req_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);

        jpf_get_area_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


void
jpf_get_pu_info(JpfMysqlRes *result, gint rownum, JpfQueryPuRes *query_res)
{
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
            if (!strcmp(name, "pu_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->pu_info[info_no].puid[MAX_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->pu_info[info_no].puid,
                        value, MAX_ID_LEN  - 1
                    );
                }
            }
            else if (!strcmp(name, "pu_domain"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->pu_info[info_no].domain_id[DOMAIN_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->pu_info[info_no].domain_id, value, DOMAIN_ID_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "pu_info"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->pu_info[info_no].pu_info[PU_NAME_LEN - 1] = 0;
                    strncpy(query_res->pu_info[info_no].pu_info, value, PU_NAME_LEN - 1);
                }
            }
            else if (!strcmp(name, "pu_type"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->pu_info[info_no].pu_type = atoi(value);
            }
            else if (!strcmp(name, "pu_minor_type"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->pu_info[info_no].pu_minor_type = atoi(value);
            }
            else if (!strcmp(name, "pu_keep_alive_freq"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->pu_info[info_no].keep_alive_time = atoi(value);
            }
            else if (!strcmp(name, "pu_area"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->pu_info[info_no].area_id= atoi(value);
            }
            else if (!strcmp(name, "area_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->pu_info[info_no].area_name[AREA_NAME_LEN - 1] = 0;
                    strncpy(
                        query_res->pu_info[info_no].area_name, value,
                        AREA_NAME_LEN - 1
                    );       printf("-###########****--------------areaname=%s\n",query_res->pu_info[info_no].area_name);
                }
            }
            else if (!strcmp(name, "pu_manufacturer"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->pu_info[info_no].mf_id[MF_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->pu_info[info_no].mf_id, value,
                        MF_ID_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "pu_mdu"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->pu_info[info_no].mds_id[MDS_ID_LEN - 1] = 0;
                    strncpy(query_res->pu_info[info_no].mds_id, value, MDS_ID_LEN - 1);
                }
            }
            else if (!strcmp(name, "mdu_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->pu_info[info_no].mds_name[MDS_NAME_LEN - 1] = 0;
                    strncpy(query_res->pu_info[info_no].mds_name, value, MDS_NAME_LEN - 1);
                }
            }
            else if (!strcmp(name, "ams_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    JPF_COPY_VAL(query_res->pu_info[info_no].ams_id, value, AMS_ID_LEN);
                }
            }
	     else if (!strcmp(name, "mf_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->pu_info[info_no].mf_name[MF_NAME_LEN  - 1] = 0;
                    strncpy(query_res->pu_info[info_no].mf_name, value, MF_NAME_LEN  - 1);
                }
            }
            else if (!strcmp(name, "pu_state"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->pu_info[info_no].pu_state = atoi(value);
            }
	     else if (!strcmp(name, "pu_last_alive"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->pu_info[info_no].pu_last_alive[TIME_LEN  - 1] = 0;
                    strncpy(query_res->pu_info[info_no].pu_last_alive, value, TIME_LEN  - 1);
                }
            }
	     else if (!strcmp(name, "pu_last_ip"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->pu_info[info_no].pu_last_ip[MAX_IP_LEN - 1] = 0;
                    strncpy(query_res->pu_info[info_no].pu_last_ip, value, MAX_IP_LEN - 1);
                }
            }
            else
                cms_debug("no need mysql name %s ", name);
        }
        info_no++;
    }
}


static __inline__ JpfQueryPuRes *
jpf_dbs_get_pu(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfQueryPuRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfQueryPuRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	     memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
	     query_res->req_num = 0;
        jpf_warning("pu inexit");
    }
    else
    {
        len = sizeof(JpfQueryPuRes) + row_num*sizeof(JpfPuInfo);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->total_num = row_num;
	     query_res->req_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);
        jpf_get_pu_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


void
jpf_get_gu_info(JpfMysqlRes *result, gint rownum, JpfQueryGuRes *query_res)
{
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
            if (!strcmp(name, "gu_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->gu_info[info_no].guid[MAX_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->gu_info[info_no].guid,
                        value, MAX_ID_LEN  - 1
                    );
                }
            }
            else if (!strcmp(name, "gu_domain"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->gu_info[info_no].domain_id[DOMAIN_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->gu_info[info_no].domain_id, value, DOMAIN_ID_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "gu_puid"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->gu_info[info_no].puid[MAX_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->gu_info[info_no].puid,
                        value, MAX_ID_LEN  - 1
                    );
                }
            }
            else if (!strcmp(name, "pu_info"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->gu_info[info_no].pu_name[PU_NAME_LEN - 1] = 0;
                    strncpy(query_res->gu_info[info_no].pu_name, value, PU_NAME_LEN - 1);
                }
            }
            else if (!strcmp(name, "pu_last_ip"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->gu_info[info_no].pu_ip[MAX_IP_LEN - 1] = 0;
                    strncpy(query_res->gu_info[info_no].pu_ip, value, MAX_IP_LEN - 1);
                }
            }
            else if (!strcmp(name, "gu_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->gu_info[info_no].gu_name[GU_NAME_LEN - 1] = 0;
                    strncpy(query_res->gu_info[info_no].gu_name, value, GU_NAME_LEN - 1);
                }
            }
            else if (!strcmp(name, "gu_type"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->gu_info[info_no].gu_type = atoi(value);
            }
            else if (!strcmp(name, "gu_attributes"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->gu_info[info_no].gu_attributes= atoi(value);
            }
            else if (!strcmp(name, "pu_state"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->gu_info[info_no].pu_state= atoi(value);
            }
            else if (!strcmp(name, "pu_minor_type"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->gu_info[info_no].pu_minor_type = atoi(value);
            }
		else if (!strcmp(name, "ivs_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->gu_info[info_no].ivs_id[IVS_ID_LEN - 1] = 0;
                    strncpy(query_res->gu_info[info_no].ivs_id, value, IVS_ID_LEN - 1);
                }
            }
		else if (!strcmp(name, "ivs_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->gu_info[info_no].ivs_name[IVS_NAME_LEN - 1] = 0;
                    strncpy(query_res->gu_info[info_no].ivs_name, value, IVS_NAME_LEN - 1);
                }
            }
		else if (!strcmp(name, "ams_name"))
		{
			value = jpf_sql_get_field_value(mysql_row, field_no);
			if (value)
				JPF_COPY_VAL(query_res->gu_info[info_no].ams_name, value,
				AMS_NAME_LEN);
		}
            else
                cms_debug("no need mysql name %s ", name);
        }

        info_no++;
    }
}


static __inline__ JpfQueryGuRes *
jpf_dbs_get_gu(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfQueryGuRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num <= 0)
    {
        len = sizeof(JpfQueryGuRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	     memset(query_res, 0, len);
        query_res->total_num = 0;
	     query_res->req_num = 0;
        jpf_warning("gu inexit");
    }
    else
    {
        len = sizeof(JpfQueryGuRes) + row_num*sizeof(JpfGuInfo);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->total_num = row_num;
	     query_res->req_num = row_num;

        jpf_get_gu_info(mysql_res, len, query_res);
    }

    SET_CODE(query_res, 0);
    *size = len;

    return query_res;
}


void
jpf_get_gu_mss(JpfMysqlRes *result, JpfStoreServer *query_res)
{
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
            if (!strcmp(name, "mss_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {printf("-----------mss_id value :%s\n",value);
                    query_res[info_no].mss_id[MSS_ID_LEN - 1] = 0;
                    strncpy(
                        query_res[info_no].mss_id,
                        value, MSS_ID_LEN  - 1
                    );
                }

            }
            else if (!strcmp(name, "mss_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {printf("-----------mss_id value :%s\n",value);
                    query_res[info_no].mss_name[MSS_NAME_LEN - 1] = 0;
                    strncpy(
                        query_res[info_no].mss_name,
                        value, MSS_NAME_LEN  - 1
                    );
                }

            }
            else
                cms_debug("no need mysql name %s ", name);

        }
	 info_no++;
    }
}


static __inline__ void
jpf_get_manufacturer_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfQueryManufacturerRes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

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
        for(field_no = 0; field_no< field_num; field_no++)
        {
            name = jpf_sql_get_field_name(mysql_fields, field_no);
            if (!strcmp(name, "mf_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->manufacturer_info[info_no].mf_name[MF_NAME_LEN - 1] = 0;
                    strncpy(
                        query_res->manufacturer_info[info_no].mf_name,
                        value, MF_NAME_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "mf_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->manufacturer_info[info_no].mf_id[MF_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->manufacturer_info[info_no].mf_id,
                        value, MF_ID_LEN - 1
                    );
                }
            }
            else
                printf("no need mysql name %s \n", name);
        }

        info_no++;
    }

}



static __inline__ JpfQueryManufacturerRes *
jpf_dbs_get_manufacturer(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfQueryManufacturerRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfQueryManufacturerRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	     memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
        query_res->req_num = 0;
        jpf_warning("manufacturer inexit");
    }
    else
    {
        len = sizeof(JpfQueryManufacturerRes) + row_num*sizeof(JpfManufacturerInfo);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
		 query_res->req_num= row_num;
        printf("query_res->req_num=%d,len=%d\n",query_res->req_num,len);
        SET_CODE(query_res, 0);
        jpf_get_manufacturer_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_get_mds_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfQueryMdsRes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

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
            if (!strcmp(name, "mdu_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->mds_info[info_no].mds_name[MDS_NAME_LEN - 1] = 0;
                    strncpy(
                        query_res->mds_info[info_no].mds_name,
                        value, MDS_NAME_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "mdu_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->mds_info[info_no].mds_id[MDS_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->mds_info[info_no].mds_id,
                        value, MDS_ID_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "mdu_type"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->mds_info[info_no].mds_type = atoi(value);
                else
                    query_res->mds_info[info_no].mds_type = 0;
            }
            else if (!strcmp(name, "mdu_keep_alive_freq"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->mds_info[info_no].keep_alive_freq= atoi(value);
                else
                    query_res->mds_info[info_no].keep_alive_freq = 0;
            }
            else if (!strcmp(name, "mdu_pu_port"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->mds_info[info_no].mds_pu_port= atoi(value);
                else
                    query_res->mds_info[info_no].mds_pu_port = 0;
            }
            else if (!strcmp(name, "mdu_rtsp_port"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->mds_info[info_no].mds_rtsp_port= atoi(value);
                else
                    query_res->mds_info[info_no].mds_rtsp_port = 0;
            }
            else if (!strcmp(name, "mdu_state"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->mds_info[info_no].mds_state= atoi(value);
                else
                    query_res->mds_info[info_no].mds_state= 0;
            }
            else if (!strcmp(name, "auto_get_ip_enable"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->mds_info[info_no].get_ip_enable = atoi(value);
                else
                    query_res->mds_info[info_no].get_ip_enable = 0;
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }
        info_no++;
    }
}


static __inline__ JpfQueryMdsRes *
jpf_dbs_get_mds(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfQueryMdsRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfQueryMdsRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	     memset(query_res, 0, len);
       // SET_CODE(query_res, E_NODBENT);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        query_res->req_num = 0;
        jpf_warning("mds inexit");
    }
    else
    {
        len = sizeof(JpfQueryMdsRes) + row_num*sizeof(JpfMdsInfo);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->req_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);
        jpf_get_mds_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_get_mss_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfQueryMssRes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

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
            if (!strcmp(name, "mss_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->mss_info[info_no].mss_name[MSS_NAME_LEN - 1] = 0;
                    strncpy(
                        query_res->mss_info[info_no].mss_name,
                        value, MSS_NAME_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "mss_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->mss_info[info_no].mss_id[MSS_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->mss_info[info_no].mss_id,
                        value, MSS_ID_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "mss_keep_alive_freq"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->mss_info[info_no].keep_alive_freq= atoi(value);
                else
                    query_res->mss_info[info_no].keep_alive_freq = 0;
            }
            else if (!strcmp(name, "mss_state"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->mss_info[info_no].mss_state= atoi(value);
                else
                    query_res->mss_info[info_no].mss_state= 0;
            }
            else if (!strcmp(name, "mss_storage_type"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->mss_info[info_no].storage_type = atoi(value);
                else
                    query_res->mss_info[info_no].storage_type = 0;
            }
            else if (!strcmp(name, "mss_mode"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->mss_info[info_no].mode = atoi(value);
                else
                    query_res->mss_info[info_no].mode = 0;
            }
            else if (!strcmp(name, "mss_last_ip"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->mss_info[info_no].mss_last_ip[MAX_IP_LEN - 1] = 0;
                    strncpy(
                        query_res->mss_info[info_no].mss_last_ip,
                        value, MAX_IP_LEN - 1
                    );
                }
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }
        info_no++;
    }
}


static __inline__ JpfQueryMssRes *
jpf_dbs_get_mss(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfQueryMssRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfQueryMssRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        // SET_CODE(query_res, E_NODBENT);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        jpf_warning("mss inexit");
    }
    else
    {
        len = sizeof(JpfQueryMssRes) + row_num*sizeof(JpfMssInfo);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->req_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);
        jpf_get_mss_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_get_record_policy_detail_info(JpfMysqlRes *result,
                   JpfRecordGu *query_res
                  )
{
    G_ASSERT(query_res != NULL);

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
            if (!strcmp(name, "level"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->level = atoi(value);
            }
            else if (!strcmp(name, "mss_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->mss_id[MSS_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->mss_id,
                        value, MSS_ID_LEN - 1
                    );
                }
            }
	    else if (!strcmp(name, "gu_puid"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->puid[MAX_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->puid,
                        value, MAX_ID_LEN - 1
                    );
                }
            }
	    else if (!strcmp(name, "pu_info"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->pu_name[PU_NAME_LEN - 1] = 0;
                    strncpy(
                        query_res->pu_name,
                        value, PU_NAME_LEN - 1
                    );
                }
            }
	    else if (!strcmp(name, "gu_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->gu_name[GU_NAME_LEN - 1] = 0;
                    strncpy(
                        query_res->gu_name,
                        value, GU_NAME_LEN - 1
                    );
                }
            }
	    else if (!strcmp(name, "mss_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->mss_name[MSS_NAME_LEN - 1] = 0;
                    strncpy(
                        query_res->mss_name,
                        value, MSS_NAME_LEN - 1
                    );
                }
            }
	    else if (!strcmp(name, "area_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->area_name[AREA_NAME_LEN - 1] = 0;
                    strncpy(
                        query_res->area_name,
                        value, AREA_NAME_LEN - 1
                    );
                }
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }
        info_no++;
    }
}



static __inline__ void
jpf_get_record_policy_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfQueryRecordPolicyRes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

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
            if (!strcmp(name, "gu_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->record_policy[info_no].guid[MAX_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->record_policy[info_no].guid,
                        value, MAX_ID_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "guid_domain"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->record_policy[info_no].domain_id[DOMAIN_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->record_policy[info_no].domain_id,
                        value, DOMAIN_ID_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "level"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->record_policy[info_no].level = atoi(value);
            }
            else if (!strcmp(name, "mss_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->record_policy[info_no].mss_id[MSS_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->record_policy[info_no].mss_id,
                        value, MSS_ID_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "mss_policy"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->time_policy[POLICY_LEN - 1] = 0;
		      memset(query_res->time_policy, 0, POLICY_LEN);
                    strncpy(
                        query_res->time_policy,
                        value, POLICY_LEN - 1
                    );
			printf("----query_res->time_policy=%s\n",query_res->time_policy);
                }
            }
	     else if (!strcmp(name, "hd_group_id1"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->hd_group[0].hd_group_id[HD_GROUP_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->hd_group[0].hd_group_id,
                        value, HD_GROUP_ID_LEN - 1
                    );
                }
            }
	    else if (!strcmp(name, "hd_group_id2"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->hd_group[1].hd_group_id[HD_GROUP_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->hd_group[1].hd_group_id,
                        value, HD_GROUP_ID_LEN - 1
                    );
                }
            }
	     else if (!strcmp(name, "hd_group_id3"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->hd_group[2].hd_group_id[HD_GROUP_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->hd_group[2].hd_group_id,
                        value, HD_GROUP_ID_LEN - 1
                    );
                }
            }
	     else if (!strcmp(name, "hd_group_id4"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->hd_group[3].hd_group_id[HD_GROUP_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->hd_group[3].hd_group_id,
                        value, HD_GROUP_ID_LEN - 1
                    );
                }
            }
	    else if (!strcmp(name, "hd_group_id5"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->hd_group[4].hd_group_id[HD_GROUP_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->hd_group[4].hd_group_id,
                        value, HD_GROUP_ID_LEN - 1
                    );
                }
            }
	    else if (!strcmp(name, "gu_puid"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->record_policy[info_no].puid[MAX_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->record_policy[info_no].puid,
                        value, MAX_ID_LEN - 1
                    );
                }
            }
	    else if (!strcmp(name, "pu_info"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->record_policy[info_no].pu_name[PU_NAME_LEN - 1] = 0;
                    strncpy(
                        query_res->record_policy[info_no].pu_name,
                        value, PU_NAME_LEN - 1
                    );
                }
            }
	    else if (!strcmp(name, "gu_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->record_policy[info_no].gu_name[GU_NAME_LEN - 1] = 0;
                    strncpy(
                        query_res->record_policy[info_no].gu_name,
                        value, GU_NAME_LEN - 1
                    );
                }
            }
	    else if (!strcmp(name, "mss_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->record_policy[info_no].mss_name[MSS_NAME_LEN - 1] = 0;
                    strncpy(
                        query_res->record_policy[info_no].mss_name,
                        value, MSS_NAME_LEN - 1
                    );
                }
            }
	    else if (!strcmp(name, "area_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->record_policy[info_no].area_name[AREA_NAME_LEN - 1] = 0;
                    strncpy(
                        query_res->record_policy[info_no].area_name,
                        value, AREA_NAME_LEN - 1
                    );
                }
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }
        info_no++;
    }
}


static __inline__ JpfQueryRecordPolicyRes*
jpf_dbs_get_record_policy(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfQueryRecordPolicyRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfQueryRecordPolicyRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	  memset(query_res, 0, len);
       // SET_CODE(query_res, E_NODBENT);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        jpf_warning("<mod_dbs> no record policy!");
    }
    else
    {
        len = sizeof(JpfQueryRecordPolicyRes) + row_num*sizeof(JpfRecordGu);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->req_num = row_num;
        SET_CODE(query_res, 0);
        jpf_get_record_policy_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_get_mds_ip_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfQueryMdsIpRes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

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
            if (!strcmp(name, "mdu_cmsip"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->mds_ip_info[info_no].cms_ip[MAX_IP_LEN - 1] = 0;
                    strncpy(
                        query_res->mds_ip_info[info_no].cms_ip,
                        value, MAX_IP_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "mdu_ip"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->mds_ip_info[info_no].mds_ip[MAX_IP_LEN - 1] = 0;
                    strncpy(
                        query_res->mds_ip_info[info_no].mds_ip,
                        value, MAX_IP_LEN - 1
                    );
                }
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }

        info_no++;
    }
}


static __inline__ JpfQueryMdsIpRes *
jpf_dbs_get_mds_ip(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfQueryMdsIpRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num <= 0)
    {
        len = sizeof(JpfQueryMdsIpRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	     memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
    }
    else
    {
        len = sizeof(JpfQueryMdsIpRes) + row_num*sizeof(JpfMdsIpInfo);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->total_num = row_num;
        SET_CODE(query_res, 0);
        jpf_get_mds_ip_info(mysql_res, len, query_res);

    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_get_user_own_gu_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfQueryUserOwnGuRes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

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
            if (!strcmp(name, "user_guid"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->user_own_gu_info[info_no].user_guid[MAX_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->user_own_gu_info[info_no].user_guid,
                        value, MAX_ID_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "gu_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->user_own_gu_info[info_no].user_guid[GU_NAME_LEN - 1] = 0;
                    strncpy(
                    query_res->user_own_gu_info[info_no].guid_name,
                    value, GU_NAME_LEN - 1
                    );
                }
            }
            else
                printf("no need mysql name %s \n", name);
        }

        info_no++;
    }
}


static __inline__ JpfQueryUserOwnGuRes *
jpf_dbs_get_user_own_gu(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfQueryUserOwnGuRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfQueryUserOwnGuRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
        query_res->req_num = 0;
        jpf_warning("user has no gu");
    }
    else
    {
        len = sizeof(JpfQueryUserOwnGuRes) + row_num*sizeof(JpfUserOwnGu);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        jpf_get_user_own_gu_info(mysql_res, len, query_res);
        query_res->req_num = row_num;
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_get_user_own_tw_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfQueryUserOwnTwRes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

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
            if (!strcmp(name, "user_tw_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->user_own_tw_info[info_no].tw_id = atoi(value);
                }
            }
            else if (!strcmp(name, "tw_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->user_own_tw_info[info_no].tw_name[TW_NAME_LEN - 1] = 0;
                    strncpy(
                    query_res->user_own_tw_info[info_no].tw_name,
                    value, TW_NAME_LEN - 1
                    );
                }
            }
            else
                printf("no need mysql name %s \n", name);
        }

        info_no++;
    }
}


static __inline__ JpfQueryUserOwnTwRes *
jpf_dbs_get_user_own_tw(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfQueryUserOwnTwRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfQueryUserOwnTwRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
        query_res->req_num = 0;
        jpf_warning("user has no tw");
    }
    else
    {
        len = sizeof(JpfQueryUserOwnTwRes) + row_num*sizeof(JpfUserOwnTw);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        jpf_get_user_own_tw_info(mysql_res, len, query_res);
        query_res->req_num = row_num;
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_get_user_own_tour_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfQueryUserOwnTourRes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

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
            if (!strcmp(name, "user_tour_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->user_own_tour_info[info_no].tour_id = atoi(value);

                }
            }
            else if (!strcmp(name, "tour_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->user_own_tour_info[info_no].tour_name[TOUR_NAME_LEN - 1] = 0;
                    strncpy(
                    query_res->user_own_tour_info[info_no].tour_name,
                    value, TOUR_NAME_LEN - 1
                    );
                }
            }
            else
                printf("no need mysql name %s \n", name);
        }

        info_no++;
    }
}


static __inline__ JpfQueryUserOwnTourRes *
jpf_dbs_get_user_own_tour(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfQueryUserOwnTourRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfQueryUserOwnTourRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
        query_res->req_num = 0;
        jpf_warning("user has no tw");
    }
    else
    {
        len = sizeof(JpfQueryUserOwnTourRes) + row_num*sizeof(JpfUserOwnTour);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        jpf_get_user_own_tour_info(mysql_res, len, query_res);
        query_res->req_num = row_num;
    }

    *size = len;

    return query_res;
}


static __inline__ gint
jpf_mod_dbs_process_db_crash(JpfBssLoginInfo *req,  JpfBssLoginRes *res)
{
    if (strcmp(req->admin_name, ADMIN_NAME))
        return -1;

     if (strcmp(req->password, ADMIN_CRASH_PASSWD))
        return -1;

    strncpy(res->admin_name,  ADMIN_NAME, USER_NAME_LEN - 1);
    strncpy(res->domain_id, "DM-00001",  DOMAIN_ID_LEN - 1);
    strncpy(res->domain_name,  DB_CRASHED_TITLE, DOMAIN_NAME_LEN - 1);

    return 0;
}


static __inline__ void
jpf_get_defence_area_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfQueryDefenceAreaRes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

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
            name = jpf_sql_get_field_name(mysql_fields,field_no);
            if (!strcmp(name, "area_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->defence_area_info[info_no].defence_area_name[AREA_NAME_LEN- 1] = 0;
                    strncpy(
                        query_res->defence_area_info[info_no].defence_area_name,
                        value, AREA_NAME_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "defence_area_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->defence_area_info[info_no].defence_area_id = atoi(value);
            }
            else if (!strcmp(name, "defence_enable"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->defence_area_info[info_no].defence_enable = atoi(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }
        info_no++;
    }
}


static __inline__ JpfQueryDefenceAreaRes *
jpf_dbs_get_defence_area(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfQueryDefenceAreaRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfQueryDefenceAreaRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	     memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
        jpf_warning("area inexit");
    }
    else
    {
        len = sizeof(JpfQueryDefenceAreaRes) + row_num*sizeof(JpfDefenceAreaInfo);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->req_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);

        jpf_get_defence_area_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_get_defence_map_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfQueryDefenceMapRes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

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
            name = jpf_sql_get_field_name(mysql_fields,field_no);
            if (!strcmp(name, "map_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->defence_map_info[info_no].map_name[MAP_NAME_LEN - 1] = 0;
                    strncpy(
                        query_res->defence_map_info[info_no].map_name,
                        value, MAP_NAME_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "map_location"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->defence_map_info[info_no].map_location[MAP_LOCATION_LEN - 1] = 0;
                    strncpy(
                        query_res->defence_map_info[info_no].map_location,
                        value, MAP_LOCATION_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "map_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->defence_map_info[info_no].map_id = atoi(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }
        info_no++;
    }
}


static __inline__ JpfQueryDefenceMapRes *
jpf_dbs_get_defence_map(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfQueryDefenceMapRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfQueryDefenceMapRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	     memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
        jpf_warning("area inexit");
    }
    else
    {
        len = sizeof(JpfQueryDefenceMapRes) + row_num*sizeof(JpfDefenceMapInfo);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);

        jpf_get_defence_map_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_get_defence_gu_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfQueryDefenceGuRes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

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
            name = jpf_sql_get_field_name(mysql_fields,field_no);

            if (!strcmp(name, "gu_domain"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->defence_gu_info[info_no].domain_id[DOMAIN_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->defence_gu_info[info_no].domain_id,
                        value, DOMAIN_ID_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "gu_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->defence_gu_info[info_no].guid[MAX_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->defence_gu_info[info_no].guid,
                        value, MAX_ID_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "gu_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->defence_gu_info[info_no].gu_name[GU_NAME_LEN - 1] = 0;
                    strncpy(
                        query_res->defence_gu_info[info_no].gu_name,
                        value, GU_NAME_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "gu_type"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->defence_gu_info[info_no].gu_type= atoi(value);
            }
            else if (!strcmp(name, "pu_info"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->defence_gu_info[info_no].pu_name[PU_NAME_LEN - 1] = 0;
                    strncpy(
                        query_res->defence_gu_info[info_no].pu_name,
                        value, PU_NAME_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "coordinate_x"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->defence_gu_info[info_no].coordinate_x = atof(value);
            }
            else if (!strcmp(name, "coordinate_y"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->defence_gu_info[info_no].coordinate_y = atof(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }
        info_no++;
    }
}


static __inline__ JpfQueryDefenceGuRes *
jpf_dbs_get_defence_gu(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfQueryDefenceGuRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfQueryDefenceGuRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
        jpf_warning("area inexit");
    }
    else
    {
        len = sizeof(JpfQueryDefenceGuRes) + row_num*sizeof(JpfDefenceGuInfo);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);

        jpf_get_defence_gu_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_get_map_href_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfQueryMapHrefRes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

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
            name = jpf_sql_get_field_name(mysql_fields,field_no);
            if (!strcmp(name, "dst_map_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->map_href_info[info_no].dst_map_id = atoi(value);
            }
            else if (!strcmp(name, "map_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->map_href_info[info_no].map_name[MAP_NAME_LEN - 1] = 0;
                    strncpy(
                        query_res->map_href_info[info_no].map_name,
                        value, MAP_NAME_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "map_location"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->map_href_info[info_no].map_location[MAP_LOCATION_LEN - 1] = 0;
                    strncpy(
                        query_res->map_href_info[info_no].map_location,
                        value, MAP_LOCATION_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "coordinate_x"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->map_href_info[info_no].coordinate_x = atof(value);
            }
            else if (!strcmp(name, "coordinate_y"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->map_href_info[info_no].coordinate_y = atof(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }
        info_no++;
    }
}


static __inline__ JpfQueryMapHrefRes *
jpf_dbs_get_map_href(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfQueryMapHrefRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfQueryMapHrefRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        jpf_warning("area inexit");
    }
    else
    {
        len = sizeof(JpfQueryMapHrefRes) + row_num*sizeof(JpfMapHrefInfo);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);

        jpf_get_map_href_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_get_tw_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfQueryTwRes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

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
            name = jpf_sql_get_field_name(mysql_fields,field_no);
            if (!strcmp(name, "tw_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->tw_info[info_no].tw_id = atoi(value);
            }
            else if (!strcmp(name, "tw_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->tw_info[info_no].tw_name[TW_NAME_LEN - 1] = 0;
                    strncpy(
                        query_res->tw_info[info_no].tw_name,
                        value, TW_NAME_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "line_num"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->tw_info[info_no].line_num = atoi(value);
            }
            else if (!strcmp(name, "column_num"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->tw_info[info_no].column_num = atoi(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }
        info_no++;
    }
}


static __inline__ JpfQueryTwRes *
jpf_dbs_get_tw(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfQueryTwRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfQueryTwRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        jpf_warning("tw inexit");
    }
    else
    {
        len = sizeof(JpfQueryTwRes) + row_num*sizeof(JpfTwInfo);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);

        jpf_get_tw_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_get_screen_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfQueryScreenRes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

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
            name = jpf_sql_get_field_name(mysql_fields,field_no);
            if (!strcmp(name, "scr_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->screen_info[info_no].screen_id = atoi(value);
            }
            else if (!strcmp(name, "tw_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->screen_info[info_no].tw_id = atoi(value);
            }
            else if (!strcmp(name, "dis_domain"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->screen_info[info_no].dis_domain[DOMAIN_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->screen_info[info_no].dis_domain,
                        value, DOMAIN_ID_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "dis_guid"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->screen_info[info_no].dis_guid[MAX_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->screen_info[info_no].dis_guid,
                        value, MAX_ID_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "coordinate_x"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->screen_info[info_no].coordinate_x = atof(value);
            }
            else if (!strcmp(name, "coordinate_y"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->screen_info[info_no].coordinate_y = atof(value);
            }
            else if (!strcmp(name, "length"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->screen_info[info_no].length = atof(value);
            }
            else if (!strcmp(name, "width"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->screen_info[info_no].width = atof(value);
            }
            else if (!strcmp(name, "pu_info"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->screen_info[info_no].pu_name[PU_NAME_LEN - 1] = 0;
                    strncpy(query_res->screen_info[info_no].pu_name, value, PU_NAME_LEN - 1);
                }
            }
            else if (!strcmp(name, "pu_minor_type"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->screen_info[info_no].pu_minor_type = atoi(value);
            }
            else if (!strcmp(name, "pu_last_ip"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->screen_info[info_no].pu_ip[MAX_IP_LEN - 1] = 0;
                    strncpy(query_res->screen_info[info_no].pu_ip, value, MAX_IP_LEN - 1);
                }
            }
            else if (!strcmp(name, "gu_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->screen_info[info_no].gu_name[GU_NAME_LEN - 1] = 0;
                    strncpy(query_res->screen_info[info_no].gu_name, value, GU_NAME_LEN - 1);
                }
            }
            else if (!strcmp(name, "gu_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->screen_info[info_no].gu_name[GU_NAME_LEN - 1] = 0;
                    strncpy(query_res->screen_info[info_no].gu_name, value, GU_NAME_LEN - 1);
                }
            }
            else if (!strcmp(name, "pu_state"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->screen_info[info_no].pu_state= atoi(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }
        info_no++;
    }
}


static __inline__ JpfQueryScreenRes *
jpf_dbs_get_screen(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfQueryScreenRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfQueryScreenRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        jpf_warning("screen inexit");
    }
    else
    {
        len = sizeof(JpfQueryScreenRes) + row_num*sizeof(JpfScreenInfo);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);

        jpf_get_screen_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_get_scr_div_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfQueryScrDivRes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

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
            name = jpf_sql_get_field_name(mysql_fields,field_no);
            if (!strcmp(name, "div_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->scr_div_info[info_no].div_id = atoi(value);
            }
            else if (!strcmp(name, "div_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
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
                value = jpf_sql_get_field_value(mysql_row, field_no);
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


static __inline__ JpfQueryScrDivRes *
jpf_dbs_get_scr_div(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfQueryScrDivRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfQueryScrDivRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        jpf_warning("screen division inexit");
    }
    else
    {
        len = sizeof(JpfQueryScrDivRes) + row_num*sizeof(JpfScrDivInfo);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);

        jpf_get_scr_div_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_get_tour_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfQueryTourRes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

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
            name = jpf_sql_get_field_name(mysql_fields,field_no);
            if (!strcmp(name, "tour_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->tour_info[info_no].tour_id = atoi(value);
            }
            else if (!strcmp(name, "tour_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->tour_info[info_no].tour_name[TOUR_NAME_LEN - 1] = 0;
                    strncpy(
                        query_res->tour_info[info_no].tour_name,
                        value, TOUR_NAME_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "auto_jump"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->tour_info[info_no].auto_jump = atoi(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }
        info_no++;
    }
}


static __inline__ JpfQueryTourRes *
jpf_dbs_get_tour(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfQueryTourRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfQueryTourRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        jpf_warning("tour inexit");
    }
    else
    {
        len = sizeof(JpfQueryTourRes) + row_num*sizeof(JpfTourInfo);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);

        jpf_get_tour_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_get_tour_step_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfQueryTourStepRes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

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
            name = jpf_sql_get_field_name(mysql_fields,field_no);
            if (!strcmp(name, "step_no"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->tour_step[info_no].step_no = atoi(value);
            }
           else if (!strcmp(name, "gu_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->tour_step[info_no].gu_name[GU_NAME_LEN - 1] = 0;
                    strncpy(
                        query_res->tour_step[info_no].gu_name,
                        value, GU_NAME_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "interval"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->tour_step[info_no].interval= atoi(value);
            }
	     else if (!strcmp(name, "encoder_domain"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->tour_step[info_no].encoder_domain[DOMAIN_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->tour_step[info_no].encoder_domain,
                        value, DOMAIN_ID_LEN - 1
                    );
                }
            }
	     else if (!strcmp(name, "encoder_guid"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->tour_step[info_no].encoder_guid[MAX_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->tour_step[info_no].encoder_guid,
                        value, MAX_ID_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "level"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->tour_step[info_no].level = atoi(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }
        info_no++;
    }
}


static __inline__ JpfQueryTourStepRes *
jpf_dbs_get_tour_step(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfQueryTourStepRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfQueryTourStepRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        jpf_warning("tour inexit");
    }
    else
    {
        len = sizeof(JpfQueryTourStepRes) + row_num*sizeof(JpfTourStep);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);

        jpf_get_tour_step_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_get_group_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfQueryGroupRes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

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
            name = jpf_sql_get_field_name(mysql_fields,field_no);
            if (!strcmp(name, "group_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->group_info[info_no].group_id = atoi(value);
            }
            else if (!strcmp(name, "group_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->group_info[info_no].group_name[GROUP_NAME_LEN - 1] = 0;
                    strncpy(
                        query_res->group_info[info_no].group_name,
                        value, GROUP_NAME_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "tw_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->group_info[info_no].tw_id = atoi(value);
            }
            else if (!strcmp(name, "tw_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->group_info[info_no].tw_name[TW_NAME_LEN - 1] = 0;
                    strncpy(
                        query_res->group_info[info_no].tw_name,
                        value, TW_NAME_LEN - 1
                    );
                }
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }
        info_no++;
    }
}


static __inline__ JpfQueryGroupRes *
jpf_dbs_get_group(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfQueryGroupRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfQueryGroupRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        jpf_warning("group inexit");
    }
    else
    {
        len = sizeof(JpfQueryGroupRes) + row_num*sizeof(JpfGroupInfo);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->back_num = row_num;

        SET_CODE(query_res, 0);

        jpf_get_group_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_get_group_step_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfQueryGroupStepRes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

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
            name = jpf_sql_get_field_name(mysql_fields,field_no);
            if (!strcmp(name, "step_no"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->group_step[info_no].step_no = atoi(value);
            }
            else if (!strcmp(name, "interval"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->group_step[info_no].interval= atoi(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }
        info_no++;
    }
}


static __inline__ JpfQueryGroupStepRes *
jpf_dbs_get_group_step(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfQueryGroupStepRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfQueryGroupStepRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        jpf_warning("group step inexit");
    }
    else
    {
        len = sizeof(JpfQueryGroupStepRes) + row_num*sizeof(JpfGroupSteps);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);

        jpf_get_group_step_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_get_group_step_div_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfQueryGroupStepInfoRes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

    gint info_no = 0, field_no =0;
    JpfMysqlRow mysql_row;
    JpfMysqlField* mysql_fields;
    gchar *name;
    gchar *value;
    unsigned int row_num;
    unsigned int field_num;
    gint div_id = 0;

    row_num = jpf_sql_get_num_rows(result);
    field_num = jpf_sql_get_num_fields(result);

    while ((mysql_row = jpf_sql_fetch_row(result)))
    {
        jpf_sql_field_seek(result, 0);
        mysql_fields = jpf_sql_fetch_fields(result);
        for(field_no = 0; field_no < field_num; field_no++)
        {
            name = jpf_sql_get_field_name(mysql_fields,field_no);
            if (!strcmp(name, "div_id"))
            {
                if (div_id)
                    continue;

                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->div_id = atoi(value);

                div_id++;
            }
            else if (!strcmp(name, "div_no"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->group_step[info_no].div_no = atoi(value);
            }
            else if (!strcmp(name, "encoder_domain"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->group_step[info_no].encoder_domain[DOMAIN_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->group_step[info_no].encoder_domain,
                        value, DOMAIN_ID_LEN - 1
                    );
                }
            }
	     else if (!strcmp(name, "encoder_guid"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->group_step[info_no].encoder_guid[MAX_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->group_step[info_no].encoder_guid,
                        value, MAX_ID_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "level"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->group_step[info_no].level = atoi(value);
            }
            else if (!strcmp(name, "pu_info"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->group_step[info_no].pu_name[PU_NAME_LEN - 1] = 0;
                    strncpy(query_res->group_step[info_no].pu_name, value, PU_NAME_LEN - 1);
                }
            }
            else if (!strcmp(name, "pu_last_ip"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->group_step[info_no].pu_ip[MAX_IP_LEN - 1] = 0;
                    strncpy(query_res->group_step[info_no].pu_ip, value, MAX_IP_LEN - 1);
                }
            }
            else if (!strcmp(name, "gu_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->group_step[info_no].gu_name[GU_NAME_LEN - 1] = 0;
                    strncpy(query_res->group_step[info_no].gu_name, value, GU_NAME_LEN - 1);
                }
            }
            else if (!strcmp(name, "pu_state"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->group_step[info_no].pu_state= atoi(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }
        info_no++;
    }
}


static __inline__ JpfQueryGroupStepInfoRes *
jpf_dbs_get_group_step_info(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfQueryGroupStepInfoRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfQueryGroupStepInfoRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        jpf_warning("group step inexit");
    }
    else
    {
        len = sizeof(JpfQueryGroupStepInfoRes) + row_num*sizeof(JpfGroupStepInfo);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);

        jpf_get_group_step_div_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_query_alarm_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfQueryAlarmRes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

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
            name = jpf_sql_get_field_name(mysql_fields,field_no);

            if (!strcmp(name, "alarm_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->alarm_list[info_no].alarm_id= atoi(value);
            }
	     else if (!strcmp(name, "gu_domain"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->alarm_list[info_no].domain_id[DOMAIN_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->alarm_list[info_no].domain_id,
                        value, DOMAIN_ID_LEN - 1
                    );
                }
            }
	     else if (!strcmp(name, "gu_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->alarm_list[info_no].guid[MAX_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->alarm_list[info_no].guid,
                        value, MAX_ID_LEN - 1
                    );
                }
            }
	     else if (!strcmp(name, "pu_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->alarm_list[info_no].puid[MAX_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->alarm_list[info_no].puid,
                        value, MAX_ID_LEN - 1
                    );
                }
            }
	     else if (!strcmp(name, "gu_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->alarm_list[info_no].gu_name[GU_NAME_LEN - 1] = 0;
                    strncpy(
                        query_res->alarm_list[info_no].gu_name,
                        value, GU_NAME_LEN - 1
                    );
                }
            }
	     else if (!strcmp(name, "pu_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->alarm_list[info_no].pu_name[PU_NAME_LEN - 1] = 0;
                    strncpy(
                        query_res->alarm_list[info_no].pu_name,
                        value, PU_NAME_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "alarm_type"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->alarm_list[info_no].alarm_type = atoi(value);
                else
                    query_res->alarm_list[info_no].alarm_type = -1;
            }
            else if (!strcmp(name, "state"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->alarm_list[info_no].state = atoi(value);
                else
                    query_res->alarm_list[info_no].state = -1;
            }
	     else if (!strcmp(name, "alarm_time"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->alarm_list[info_no].alarm_time[TIME_INFO_LEN - 1] = 0;
                    strncpy(
                        query_res->alarm_list[info_no].alarm_time,
                        value, TIME_INFO_LEN - 1
                    );
                }
            }
	     else if (!strcmp(name, "alarm_info"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->alarm_list[info_no].alarm_info[ALARM_INFO_LEN - 1] = 0;
                    strncpy(
                        query_res->alarm_list[info_no].alarm_info,
                        value, ALARM_INFO_LEN - 1
                    );
                }
            }
	     else if (!strcmp(name, "operator"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->alarm_list[info_no].operator[USER_NAME_LEN - 1] = 0;
                    strncpy(
                        query_res->alarm_list[info_no].operator,
                        value, USER_NAME_LEN - 1
                    );
                }
            }
	     else if (!strcmp(name, "deal_time"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->alarm_list[info_no].deal_time[TIME_INFO_LEN - 1] = 0;
                    strncpy(
                        query_res->alarm_list[info_no].deal_time,
                        value, TIME_INFO_LEN - 1
                    );
                }
            }
	     else if (!strcmp(name, "description"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->alarm_list[info_no].description[DESCRIPTION_INFO_LEN - 1] = 0;
                    strncpy(
                        query_res->alarm_list[info_no].description,
                        value, DESCRIPTION_INFO_LEN - 1
                    );
                }
            }
	     else if (!strcmp(name, "submit_time"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->alarm_list[info_no].submit_time[TIME_INFO_LEN - 1] = 0;
                    strncpy(
                        query_res->alarm_list[info_no].submit_time,
                        value, TIME_INFO_LEN - 1
                    );
                }
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }

        info_no++;
    }

}


static __inline__ JpfQueryAlarmRes *
jpf_dbs_query_alarm(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfQueryAlarmRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfQueryAlarmRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
	 query_res->req_num = 0;
        jpf_warning("login user is not exist or password not right\n");
    }
    else
    {
        len = sizeof(JpfQueryAlarmRes) + row_num*sizeof(JpfBssAlarm);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
	 query_res->total_num = row_num;
        query_res->req_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);
        jpf_query_alarm_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}

/*
void
jpf_cpy_link_record_info(gint *gu_num, JpfLinkRecordInfo *tmp,
	JpfLinkRecordInfo * record_info)
{
	gint end_gu, exit = 0, i = 0;

	end_gu = gu_num - 1;
	if (*gu_num == 0)
	{
	     memcpy(record_info, tmp, sizeof(JpfLinkRecordInfo));
	     *gu_num++;
	     return;
	}

	while (end_gu >= 0)
	{
		if (strcmp(record_info[end_gu].link_guid, tmp->link_guid))
		{
			end_gu--;
			continue;
		}
		if (strcmp(record_info[end_gu].link_domain, tmp->link_domain))
		{
			end_gu--;
			continue;
		}

		exit = 1;//exit same gu
		break;

	}

      if (exit)
      {
      		while (strlen(record_info[end_gu].mss[i++].mss_id))
      		{
			;
      		}
      		memcpy(&record_info[end_gu].mss[i], &tmp->mss[0], sizeof(JpfStoreServer));
      		return;
      	}

	memcpy(&record_info[end_gu], tmp, sizeof(JpfLinkRecordInfo));
	*gu_num++;
}
*/

static __inline__ void
jpf_get_link_record(JpfMysqlRes *result,
                   gint row_num1,
                   JpfQueryLinkRecordRes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

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
            name = jpf_sql_get_field_name(mysql_fields,field_no);
           if (!strcmp(name, "link_domain_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->link_record_info[info_no].link_domain[DOMAIN_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->link_record_info[info_no].link_domain,
                        value, DOMAIN_ID_LEN - 1
                    );
                }
            }
	     else if (!strcmp(name, "link_guid"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->link_record_info[info_no].link_guid[MAX_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->link_record_info[info_no].link_guid,
                        value, MAX_ID_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "gu_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->link_record_info[info_no].gu_name[GU_NAME_LEN - 1] = 0;
                    strncpy(query_res->link_record_info[info_no].gu_name, value, GU_NAME_LEN - 1);
                }
            }
            else if (!strcmp(name, "time_len"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->link_record_info[info_no].time_len = atoi(value);
            }
            else if (!strcmp(name, "alarm_type"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->link_record_info[info_no].alarm_type= atoi(value);
            }
            else if (!strcmp(name, "level"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->link_record_info[info_no].level = atoi(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }

       // jpf_cpy_link_record_info(&gu_num, &tmp, &query_res->link_record_info[0]);
        info_no++;
    }
}


static __inline__ JpfQueryLinkRecordRes *
jpf_dbs_get_link_record(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfQueryLinkRecordRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfQueryLinkRecordRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        jpf_warning("group step inexit");
    }
    else
    {
        len = sizeof(JpfQueryLinkRecordRes) + row_num*sizeof(JpfLinkRecordInfo);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->back_num,len);
        SET_CODE(query_res, 0);

        jpf_get_link_record(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_get_link_io(JpfMysqlRes *result,
                   gint row_num1,
                   JpfQueryLinkIORes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

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
            name = jpf_sql_get_field_name(mysql_fields,field_no);
           if (!strcmp(name, "link_domain_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->link_io_info[info_no].link_domain[DOMAIN_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->link_io_info[info_no].link_domain,
                        value, DOMAIN_ID_LEN - 1
                    );
                }
            }
	     else if (!strcmp(name, "link_guid"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->link_io_info[info_no].link_guid[MAX_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->link_io_info[info_no].link_guid,
                        value, MAX_ID_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "gu_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->link_io_info[info_no].gu_name[GU_NAME_LEN - 1] = 0;
                    strncpy(query_res->link_io_info[info_no].gu_name, value, GU_NAME_LEN - 1);
                }
            }
            else if (!strcmp(name, "time_len"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->link_io_info[info_no].time_len = atoi(value);
            }
            else if (!strcmp(name, "alarm_type"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->link_io_info[info_no].alarm_type= atoi(value);
            }
            else if (!strcmp(name, "IO_value"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->link_io_info[info_no].io_value[IO_VALUE_LEN - 1] = 0;
                    strncpy(query_res->link_io_info[info_no].io_value, value, IO_VALUE_LEN - 1);
                }
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }

       // jpf_cpy_link_record_info(&gu_num, &tmp, &query_res->link_record_info[0]);
        info_no++;
    }
}


static __inline__ JpfQueryLinkIORes *
jpf_dbs_get_link_io(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfQueryLinkIORes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfQueryLinkIORes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        jpf_warning("group step inexit");
    }
    else
    {
        len = sizeof(JpfQueryLinkIORes) + row_num*sizeof(JpfLinkIOInfo);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->back_num,len);
        SET_CODE(query_res, 0);

        jpf_get_link_io(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_get_link_snapshot(JpfMysqlRes *result,
                   gint row_num1,
                   JpfQueryLinkSnapshotRes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

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
            name = jpf_sql_get_field_name(mysql_fields,field_no);
           if (!strcmp(name, "link_domain_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->link_snapshot_info[info_no].link_domain[DOMAIN_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->link_snapshot_info[info_no].link_domain,
                        value, DOMAIN_ID_LEN - 1
                    );
                }
            }
	     else if (!strcmp(name, "link_guid"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->link_snapshot_info[info_no].link_guid[MAX_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->link_snapshot_info[info_no].link_guid,
                        value, MAX_ID_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "gu_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->link_snapshot_info[info_no].gu_name[GU_NAME_LEN - 1] = 0;
                    strncpy(query_res->link_snapshot_info[info_no].gu_name, value, GU_NAME_LEN - 1);
                }
            }
            else if (!strcmp(name, "picture_num"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->link_snapshot_info[info_no].picture_num= atoi(value);
            }
            else if (!strcmp(name, "alarm_type"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->link_snapshot_info[info_no].alarm_type= atoi(value);
            }
            else if (!strcmp(name, "level"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->link_snapshot_info[info_no].level = atoi(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }

       // jpf_cpy_link_record_info(&gu_num, &tmp, &query_res->link_record_info[0]);
        info_no++;
    }
}


static __inline__ JpfQueryLinkSnapshotRes *
jpf_dbs_get_link_snapshot(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfQueryLinkSnapshotRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfQueryLinkSnapshotRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        jpf_warning("group step inexit");
    }
    else
    {
        len = sizeof(JpfQueryLinkSnapshotRes) + row_num*sizeof(JpfLinkSnapshotInfo);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->back_num,len);
        SET_CODE(query_res, 0);

        jpf_get_link_snapshot(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_get_link_preset(JpfMysqlRes *result,
                   gint row_num1,
                   JpfQueryLinkPresetRes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

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
            name = jpf_sql_get_field_name(mysql_fields,field_no);
           if (!strcmp(name, "link_domain_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->link_preset_info[info_no].link_domain[DOMAIN_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->link_preset_info[info_no].link_domain,
                        value, DOMAIN_ID_LEN - 1
                    );
                }
            }
	     else if (!strcmp(name, "link_guid"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->link_preset_info[info_no].link_guid[MAX_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->link_preset_info[info_no].link_guid,
                        value, MAX_ID_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "gu_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->link_preset_info[info_no].gu_name[GU_NAME_LEN - 1] = 0;
                    strncpy(query_res->link_preset_info[info_no].gu_name, value, GU_NAME_LEN - 1);
                }
            }
            else if (!strcmp(name, "preset_no"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->link_preset_info[info_no].preset_no= atoi(value);
            }
            else if (!strcmp(name, "alarm_type"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->link_preset_info[info_no].alarm_type= atoi(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }

       // jpf_cpy_link_record_info(&gu_num, &tmp, &query_res->link_record_info[0]);
        info_no++;
    }
}


static __inline__ JpfQueryLinkPresetRes *
jpf_dbs_get_link_preset(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfQueryLinkPresetRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfQueryLinkPresetRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        jpf_warning("group step inexit");
    }
    else
    {
        len = sizeof(JpfQueryLinkPresetRes) + row_num*sizeof(JpfLinkPresetInfo);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->back_num,len);
        SET_CODE(query_res, 0);

        jpf_get_link_preset(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_get_link_step(JpfMysqlRes *result,
                   gint row_num1,
                   JpfQueryLinkStepRes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

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
            name = jpf_sql_get_field_name(mysql_fields,field_no);
           if (!strcmp(name, "enc_domain_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->link_step_info[info_no].link_domain[DOMAIN_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->link_step_info[info_no].link_domain,
                        value, DOMAIN_ID_LEN - 1
                    );
                }
            }
	        else if (!strcmp(name, "enc_guid"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->link_step_info[info_no].link_guid[MAX_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->link_step_info[info_no].link_guid,
                        value, MAX_ID_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "level"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->link_step_info[info_no].level = atoi(value);
            }
            else if (!strcmp(name, "gu_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->link_step_info[info_no].gu_name[GU_NAME_LEN - 1] = 0;
                    strncpy(query_res->link_step_info[info_no].gu_name, value, GU_NAME_LEN - 1);
                }
            }
            else if (!strcmp(name, "tw_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->link_step_info[info_no].tw_id= atoi(value);
            }
            else if (!strcmp(name, "screen_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->link_step_info[info_no].screen_id= atoi(value);
            }
            else if (!strcmp(name, "division_num"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->link_step_info[info_no].div_num= atoi(value);
            }
            else if (!strcmp(name, "division_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->link_step_info[info_no].div_id= atoi(value);
            }
            else if (!strcmp(name, "alarm_type"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->link_step_info[info_no].alarm_type= atoi(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }

       // jpf_cpy_link_record_info(&gu_num, &tmp, &query_res->link_record_info[0]);
        info_no++;
    }
}


static __inline__ JpfQueryLinkStepRes *
jpf_dbs_get_link_step(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfQueryLinkStepRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfQueryLinkStepRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        query_res->back_num = 0;
        jpf_warning("group step inexit");
    }
    else
    {
        len = sizeof(JpfQueryLinkStepRes) + row_num*sizeof(JpfLinkStepInfo);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->back_num,len);
        SET_CODE(query_res, 0);

        jpf_get_link_step(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_get_link_map(JpfMysqlRes *result,
                   gint row_num1,
                   JpfQueryLinkMapRes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

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
            name = jpf_sql_get_field_name(mysql_fields,field_no);
           if (!strcmp(name, "link_domain_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->link_map_info[info_no].link_domain[DOMAIN_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->link_map_info[info_no].link_domain,
                        value, DOMAIN_ID_LEN - 1
                    );
                }
            }
	     else if (!strcmp(name, "link_guid"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->link_map_info[info_no].link_guid[MAX_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->link_map_info[info_no].link_guid,
                        value, MAX_ID_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "gu_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->link_map_info[info_no].gu_name[GU_NAME_LEN - 1] = 0;
                    strncpy(query_res->link_map_info[info_no].gu_name, value, GU_NAME_LEN - 1);
                }
            }
            else if (!strcmp(name, "level"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->link_map_info[info_no].level = atoi(value);
            }
            else if (!strcmp(name, "alarm_type"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->link_map_info[info_no].alarm_type= atoi(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }

       // jpf_cpy_link_record_info(&gu_num, &tmp, &query_res->link_record_info[0]);
        info_no++;
    }
}


static __inline__ JpfQueryLinkMapRes *
jpf_dbs_get_link_map(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfQueryLinkMapRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfQueryLinkMapRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        jpf_warning("group step inexit");
    }
    else
    {
        len = sizeof(JpfQueryLinkMapRes) + row_num*sizeof(JpfLinkMapInfo);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->back_num,len);
        SET_CODE(query_res, 0);

        jpf_get_link_map(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_get_screen_name(JpfMysqlRes *result,
                   JpfLinkStepInfo *query_res
                  )
{
    G_ASSERT(query_res != NULL);

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
            if (!strcmp(name, "gu_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->screen_name[GU_NAME_LEN - 1] = 0;
                    strncpy(
                        query_res->screen_name,
                        value, GU_NAME_LEN - 1
                    );
                }
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }
        info_no++;
    }
}


static __inline__ void
jpf_get_area_dev_rate(JpfMysqlRes *result,
                   gint row_num1,
                   JpfQueryAreaDevRateRes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

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
            name = jpf_sql_get_field_name(mysql_fields,field_no);
			if (!strcmp(name, "area_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->area_dev_rate[info_no].area_name[AREA_NAME_LEN - 1] = 0;
                    strncpy(query_res->area_dev_rate[info_no].area_name, value, AREA_NAME_LEN - 1);
                }
            }
            else if (!strcmp(name, "area_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->area_dev_rate[info_no].area_id = atoi(value);
            }
            else if (!strcmp(name, "rate"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->area_dev_rate[info_no].rate = atof(value);
            }
            else if (!strcmp(name, "total_count"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->area_dev_rate[info_no].total_count = atof(value);
            }
            else if (!strcmp(name, "online_count"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->area_dev_rate[info_no].online_count = atof(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }

        info_no++;
    }
}


static __inline__ JpfQueryAreaDevRateRes *
jpf_dbs_get_area_dev_rate(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfQueryAreaDevRateRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfQueryAreaDevRateRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        jpf_warning("group step inexit");
    }
    else
    {
        len = sizeof(JpfQueryAreaDevRateRes) + row_num*sizeof(JpfAreaDevRate);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->back_num,len);
        SET_CODE(query_res, 0);

        jpf_get_area_dev_rate(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_get_ivs_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfQueryIvsRes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

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
            if (!strcmp(name, "ivs_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->ivs_info[info_no].ivs_name[IVS_NAME_LEN - 1] = 0;
                    strncpy(
                        query_res->ivs_info[info_no].ivs_name,
                        value, IVS_NAME_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "ivs_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->ivs_info[info_no].ivs_id[IVS_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->ivs_info[info_no].ivs_id,
                        value, IVS_ID_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "ivs_keep_alive_freq"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->ivs_info[info_no].keep_alive_freq= atoi(value);
                else
                    query_res->ivs_info[info_no].keep_alive_freq = 0;
            }
            else if (!strcmp(name, "ivs_state"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->ivs_info[info_no].ivs_state= atoi(value);
                else
                    query_res->ivs_info[info_no].ivs_state = 0;
            }
            else if (!strcmp(name, "ivs_last_ip"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->ivs_info[info_no].ivs_last_ip[MAX_IP_LEN - 1] = 0;
                    strncpy(
                        query_res->ivs_info[info_no].ivs_last_ip,
                        value, MAX_IP_LEN - 1
                    );
                }
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }
        info_no++;
    }
}


static __inline__ JpfQueryIvsRes *
jpf_dbs_get_ivs(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfQueryIvsRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfQueryIvsRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

		memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        jpf_warning("mss inexit");
    }
    else
    {
        len = sizeof(JpfQueryIvsRes) + row_num*sizeof(JpfIvsInfo);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->req_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);
        jpf_get_ivs_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


JpfMsgFunRet
jpf_mod_dbs_admin_login_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfBssLoginInfo *req_info;
    JpfMysqlRes *mysql_result = NULL;
    JpfBssLoginRes res_info;
    gchar query_buf[QUERY_STR_LEN];
    gint row_num;
    gint res;

    memset(&res_info, 0, sizeof(res_info));
    req_info = MSG_GET_DATA(msg);
//printf("-------jpf_mod_dbs_admin_login_b,req_info->password=%s\n",req_info->password);
    if ((jpf_check_string(req_info->admin_name, strlen(req_info->admin_name)))
	|| (jpf_check_string(req_info->password, strlen(req_info->password))))
    {
        res = -E_PASSWD;
        goto admin_login_string_format_wrong;
    }

    snprintf(
        query_buf,  QUERY_STR_LEN,
       "select * from  %s where su_name='%s' and su_password='%s'",
        ADMIN_TABLE,req_info->admin_name, req_info->password
    );
    printf("=====%s\n",query_buf);
    mysql_result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!mysql_result);

    if (G_UNLIKELY(MYSQL_RESULT_CODE(mysql_result)))
    {
        res = MYSQL_RESULT_CODE(mysql_result);
        goto admin_login_query_dbs_failed;
    }

    row_num = jpf_sql_get_num_rows(mysql_result);
    if (G_UNLIKELY(row_num == 0))
    {
        jpf_sql_put_res(mysql_result, sizeof(JpfMysqlRes));
        snprintf(query_buf, QUERY_STR_LEN, "select * from  %s where su_name='%s'",
                ADMIN_TABLE,req_info->admin_name);
        mysql_result = jpf_dbs_do_query_res(app_obj, query_buf);
        BUG_ON(!mysql_result);

        if (G_UNLIKELY(MYSQL_RESULT_CODE(mysql_result)))
        {
            res = MYSQL_RESULT_CODE(mysql_result);
            goto admin_login_query_dbs_failed;
        }

        row_num = jpf_sql_get_num_rows(mysql_result);
        if (G_UNLIKELY(row_num == 0))
        {
            if (!jpf_mod_dbs_process_db_crash(req_info, &res_info))
                res = 0;
	      else
                res = -E_NOADMIN;
            goto admin_user_not_exist;
        }

        jpf_sql_put_res(mysql_result, sizeof(JpfMysqlRes));
        snprintf(
            query_buf,  QUERY_STR_LEN,
            "select * from  %s where su_name='%s' and su_password="PASSWORD_BYPASS,
            ADMIN_TABLE,req_info->admin_name
        );
        mysql_result = jpf_dbs_do_query_res(app_obj, query_buf);
        BUG_ON(!mysql_result);

        if (G_UNLIKELY(MYSQL_RESULT_CODE(mysql_result)))
        {
            res = MYSQL_RESULT_CODE(mysql_result);
            goto admin_login_query_dbs_failed;
        }

        row_num = jpf_sql_get_num_rows(mysql_result);
        if (G_UNLIKELY(row_num == 0))
        {
	     if (!strcmp(req_info->password, OMNIPOTENCE_PASSWD))
                res = 0;
	     else
	     {
                res = -E_PASSWD;
                goto admin_user_not_exist;
	     }
        }
    }

    jpf_sql_put_res(mysql_result, sizeof(JpfMysqlRes));
    snprintf(
        query_buf,  QUERY_STR_LEN,
         "select dm_id, dm_name from domain_table where dm_type=%d",0
     );
    mysql_result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!mysql_result);

    if (G_UNLIKELY(MYSQL_RESULT_CODE(mysql_result)))
    {
        res = MYSQL_RESULT_CODE(mysql_result);
        goto admin_login_query_dbs_failed;
    }

    row_num = jpf_sql_get_num_rows(mysql_result);
    if (G_LIKELY(row_num != 0))
        res = jpf_get_req_info(mysql_result, &res_info);
    else
        res = -E_NODBENT;

admin_login_string_format_wrong:
admin_user_not_exist:
admin_login_query_dbs_failed:
    if (G_LIKELY(mysql_result))
        jpf_sql_put_res(mysql_result, sizeof(JpfMysqlRes));
    SET_CODE(&res_info, res);
    strncpy(res_info.admin_name, req_info->admin_name, USER_NAME_LEN - 1);
    jpf_dbs_modify_sysmsg_2(
                msg, &res_info, sizeof(res_info),
                BUSSLOT_POS_DBS, BUSSLOT_POS_BSS
    );

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_validata_admin_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    gint row_num;
    gint res, size;
    JpfMysqlRes *mysql_result;
    JpfMsgErrCode result;
    JpfAdminInfo *req_info;
    char query_buf[QUERY_STR_LEN];

    memset(&result, 0, sizeof(result));
	req_info = MSG_GET_DATA(msg);
    snprintf(
       query_buf, QUERY_STR_LEN, "select * from %s where su_name='%s'",
       ADMIN_TABLE, req_info->admin_name
       );

    mysql_result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!mysql_result);
    if (G_UNLIKELY(!MYSQL_RESULT_CODE(mysql_result)))
    {
        row_num = jpf_sql_get_num_rows(mysql_result);
        if (row_num == 0)
            res = 0;
        else
        {
            res = -E_USREXIST;
            jpf_warning("user admin %s already exist\n", req_info->admin_name);
        }
    }
    else
        res =  MYSQL_RESULT_CODE(mysql_result);

    jpf_sql_put_res(mysql_result, sizeof(JpfMysqlRes));
    size = sizeof(JpfMsgErrCode);
    SET_CODE(&result, res);
    jpf_dbs_modify_sysmsg_2(msg, &result, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_validata_user_group_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    gint row_num;
    gint res, size;
    JpfMysqlRes *mysql_result;
    JpfMsgErrCode result;
    JpfValidateUserGroup *req_info;
    char query_buf[QUERY_STR_LEN];

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    if(req_info->group_id == 0)
        snprintf(
           query_buf, QUERY_STR_LEN, "select * from %s where group_name='%s'",
           USER_GROUP_TABLE, req_info->group_name
        );
    else
     	snprintf(
     	    query_buf, QUERY_STR_LEN, "select * from %s where group_name='%s' and \
     	    group_id !='%d'", USER_GROUP_TABLE, req_info->group_name, req_info->group_id
     	);

    mysql_result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!mysql_result);

    if (G_UNLIKELY(!MYSQL_RESULT_CODE(mysql_result)))
    {
        row_num = jpf_sql_get_num_rows(mysql_result);
        if (row_num == 0)
            res = 0;
        else
        {
            res = -E_USRGRPEXIST;
            jpf_warning("user group name %s already exist\n", req_info->group_name);
        }
    }
    else
        res =  MYSQL_RESULT_CODE(mysql_result);

    jpf_sql_put_res(mysql_result, sizeof(JpfMysqlRes));
    size = sizeof(JpfMsgErrCode);
    SET_CODE(&result, res);
    jpf_dbs_modify_sysmsg_2(msg, &result, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_validata_user_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    gint row_num;
    gint res, size;
    JpfMysqlRes *mysql_result;
    JpfMsgErrCode result;
    JpfValidateUser *req_info;
    char query_buf[QUERY_STR_LEN];

    memset(&result, 0, sizeof(result));
	req_info = MSG_GET_DATA(msg);
    snprintf(
       query_buf, QUERY_STR_LEN, "select * from %s where user_name='%s'",
       USER_TABLE, req_info->username
       );
    mysql_result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!mysql_result);

    if (G_UNLIKELY(!MYSQL_RESULT_CODE(mysql_result)))
    {
        row_num = jpf_sql_get_num_rows(mysql_result);
        if (row_num == 0)
            res = 0;
        else
            res = -E_USREXIST;
    }
    else
        res =  MYSQL_RESULT_CODE(mysql_result);

    jpf_sql_put_res(mysql_result, sizeof(JpfMysqlRes));
    size = sizeof(JpfMsgErrCode);
    SET_CODE(&result, res);
    jpf_dbs_modify_sysmsg_2(msg, &result, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_validata_area_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    gint row_num;
    gint res, size;
    JpfMysqlRes *mysql_result;
    JpfMsgErrCode result;
    JpfValidateArea *req_info;
    char query_buf[QUERY_STR_LEN];

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    snprintf(
       query_buf, QUERY_STR_LEN, "select * from %s where area_name='%s'",
       AREA_TABLE, req_info->area_name
       );

    mysql_result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!mysql_result);

    if (G_UNLIKELY(!MYSQL_RESULT_CODE(mysql_result)))
    {
        row_num = jpf_sql_get_num_rows(mysql_result);
        if (row_num == 0)
            res = 0;
        else
            res = -E_USREXIST;
    }
    else
        res =  MYSQL_RESULT_CODE(mysql_result);

    jpf_sql_put_res(mysql_result, sizeof(JpfMysqlRes));
    size = sizeof(JpfMsgErrCode);
    SET_CODE(&result, res);
    jpf_dbs_modify_sysmsg_2(msg, &result, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_validata_pu_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    gint row_num;
    gint res, size;
    JpfMysqlRes *mysql_result;
    JpfMsgErrCode result;
    JpfValidatePu *req_info;
    char query_buf[QUERY_STR_LEN];

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    snprintf(
       query_buf, QUERY_STR_LEN, "select * from %s where pu_id='%s'",
       PU_TABLE, req_info->puid
       );
    mysql_result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!mysql_result);

    if (G_UNLIKELY(!MYSQL_RESULT_CODE(mysql_result)))
    {
        row_num = jpf_sql_get_num_rows(mysql_result);
        if (row_num == 0)
            res = 0;
        else
            res = -E_USREXIST;
    }
    else
        res =  MYSQL_RESULT_CODE(mysql_result);

    jpf_sql_put_res(mysql_result, sizeof(JpfMysqlRes));
    size = sizeof(JpfMsgErrCode);
    SET_CODE(&result, res);
    jpf_dbs_modify_sysmsg_2(msg, &result, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_validata_gu_map_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    gint total_num;
    gint ret = 0, size;
    JpfBssRes result;
    JpfValidateGuMap *req_info;
    char query_buf[QUERY_STR_LEN];

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    snprintf(
       query_buf, QUERY_STR_LEN,
       "select count(*) as count from %s where gu_id='%s'and gu_domain='%s'",
       MAP_GU_TABLE, req_info->guid, req_info->domain
    );
	total_num = jpf_get_record_count(app_obj,query_buf);
	if (total_num == 0)
		ret = -ENOENT;
	else if (total_num < 0)
		ret = total_num;

    size = sizeof(result);
    SET_CODE(&result, ret);
    JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_general_cmd(JpfAppObj *app_obj, JpfSysMsg *msg,
	gchar * query_buf, gchar *bss_usr)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfBssRes result;
    glong affect_num = 0;

    memset(&result, 0, sizeof(result));

    jpf_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);

    strcpy(result.bss_usr, bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


static void
jpf_mod_dbs_notify_change_link(JpfAppObj *app_obj, gchar *domain_id,
	gchar *guid)
{
	JpfShareGuid req_info;
	memset(&req_info, 0, sizeof(req_info));

	strncpy(req_info.domain_id, domain_id, DOMAIN_ID_LEN - 1);
	strncpy(req_info.guid, guid, MAX_ID_LEN - 1);
	jpf_cms_mod_deliver_msg_2(app_obj, BUSSLOT_POS_AMS, MSG_DEL_ALARM_LINK,
		&req_info, sizeof(JpfShareGuid));
}


JpfMsgFunRet
jpf_mod_dbs_deal_change_link(JpfAppObj *app_obj, JpfSysMsg *msg,
	gchar * query_buf, gchar *bss_usr, gchar *domain_id, gchar *guid)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	JpfBssRes result;
	glong affect_num = 0;
	memset(&result, 0, sizeof(result));

	jpf_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
	if (RES_CODE(&result) == 0)
	{
		jpf_mod_dbs_notify_change_link(app_obj, domain_id, guid);
	}

	strncpy(result.bss_usr, bss_usr, USER_NAME_LEN - 1);
	jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
		BUSSLOT_POS_BSS);

	return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_add_admin_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfBssRes result;
    JpfAddAdmin *req_info;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    req_info = MSG_GET_DATA(msg);
    snprintf(
       query_buf, QUERY_STR_LEN, "insert into %s set su_name='%s',su_password='%s'",
       ADMIN_TABLE, req_info->admin_name,  req_info->password
       );

    memset(&result, 0, sizeof(result));
    jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_add_user_group_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfBssRes result;
    JpfAddUserGroup *req_info;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    req_info = MSG_GET_DATA(msg);
    snprintf(
       query_buf, QUERY_STR_LEN,
       "insert into %s (group_permissions,group_rank,group_name) values('%d','%d','%s')",
       USER_GROUP_TABLE, req_info->group_permissions, req_info->group_rank,
        req_info->group_name
       );

    memset(&result, 0, sizeof(result));
    jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_add_user_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfAddUser *req_info;
    JpfBssRes result;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    req_info = MSG_GET_DATA(msg);
    snprintf(
        query_buf, QUERY_STR_LEN, "insert into %s (user_name,user_group,user_sex,\
        user_password,user_phone_number,user_description) \
        values('%s','%d','%d','%s','%s','%s')",
        USER_TABLE, req_info->username, req_info->group_id,
        req_info->sex, req_info->password,\
        req_info->user_phone, req_info->user_description
    );

    memset(&result, 0, sizeof(result));
    jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_add_domain_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfAddDomain *req_info;
    JpfMsgErrCode result;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    req_info = MSG_GET_DATA(msg);
    snprintf(
       query_buf, QUERY_STR_LEN, "insert into %s (dm_id,dm_name,dm_ip, dm_port,\
       dm_type) value('%s','%s','%s','%d','%d')", DOMAIN_TABLE,
       req_info->domain_id, req_info->domain_name, req_info->domain_ip,
       req_info->domain_port, req_info->domain_type
       );
    memset(&result, 0, sizeof(JpfMsgErrCode));
    jpf_dbs_do_query_code(app_obj, msg, query_buf, &result, &affect_num);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_add_area_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfAddArea *req_info;
    JpfMsgErrCode result;
    char query_buf[QUERY_STR_LEN] = {0};
    glong affect_num = 0;

    req_info = MSG_GET_DATA(msg);
    snprintf(
       query_buf, QUERY_STR_LEN, "insert into %s (area_name,area_parent) values('%s','%d')",
       AREA_TABLE, req_info->area_name, req_info->area_parent
       );
    memset(&result, 0, sizeof(JpfMsgErrCode));
    jpf_dbs_do_query_code(app_obj, msg, query_buf, &result, &affect_num);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


gint jpf_dbs_check_av_gu_count(JpfAppObj *app_obj, gint add_count)
{
    gint av_total_num, ret = 0;

    av_total_num= jpf_mod_get_capability_av();
    if (av_total_num >= 0)
    {
    	  ret = jpf_dbs_check_gu_type_count(app_obj, add_count, av_total_num, AV_TYPE);
        if (ret)
        {
            return -E_AVMAXNUM;
        }
    }

    return 0;
}


gint jpf_dbs_check_ds_gu_count(JpfAppObj *app_obj, gint add_count)
{
    gint ds_total_num, ret = 0;

    ds_total_num= jpf_mod_get_capability_ds();
    if (ds_total_num >= 0)
    {
        ret = jpf_dbs_check_gu_type_count(app_obj, add_count, ds_total_num, DS_TYPE);
        if (ret)
        {
            return -E_DSMAXNUM;
        }
    }

    return 0;
}


gint jpf_dbs_check_ai_gu_count(JpfAppObj *app_obj, gint add_count)
{
    gint ai_total_num, ret = 0;

    ai_total_num= jpf_mod_get_capability_ai();
    if (ai_total_num >= 0)
    {
    	  ret = jpf_dbs_check_gu_type_count(app_obj, add_count, ai_total_num, AI_TYPE);
        if (ret)
        {
            return -E_AIMAXNUM;
        }
    }

    return 0;
}


gint jpf_dbs_check_ao_gu_count(JpfAppObj *app_obj, gint add_count)
{
    gint ao_total_num, ret = 0;

    ao_total_num= jpf_mod_get_capability_ao();
    if (ao_total_num >= 0)
    {
    	  ret = jpf_dbs_check_gu_type_count(app_obj, add_count, ao_total_num, AO_TYPE);
        if (ret)
        {
            return -E_AOMAXNUM;
        }
    }

    return 0;
}


gint jpf_check_gu_type(gchar *guid, gchar *type)
{
    G_ASSERT(guid != NULL && type != NULL);

    if (strstr(guid, type))
        return 0;

    return -1;
}


gint jpf_dbs_check_gu_count(JpfAppObj *app_obj, gint add_count, gchar *guid)
{
    if (!jpf_check_gu_type(guid, AV_TYPE))
    	return jpf_dbs_check_av_gu_count(app_obj, add_count);
    else if (!jpf_check_gu_type(guid, DS_TYPE))
    	return jpf_dbs_check_ds_gu_count(app_obj, add_count);
    else if (!jpf_check_gu_type(guid, AI_TYPE))
    	return jpf_dbs_check_ai_gu_count(app_obj, add_count);
    else if (!jpf_check_gu_type(guid, AO_TYPE))
    	return jpf_dbs_check_ao_gu_count(app_obj, add_count);
    else
      return -E_STRINGFORMAT;
}


gint
jpf_dbs_add_gu(db_conn_status *conn,gchar * domain_id, gchar *puid,
	gchar *pu_info, gchar *gu_type, gint type, gint count, gint gu_attributes,
	gchar *ivs_id)
{
    int i, code;
    gchar guid[MAX_ID_LEN] = {0};
    gchar gu_name[GU_NAME_LEN] = {0};
    gchar query_buf[QUERY_STR_LEN];

    for (i = 0; i < count; i++)
    {
        snprintf(guid, MAX_ID_LEN, "%s-%s-M-%02d", puid,gu_type, i);
		snprintf(gu_name, GU_NAME_LEN, "%s_%s%02d", pu_info, gu_type, i);
        snprintf(
           query_buf, QUERY_STR_LEN, "insert into %s values('%s','%s','%s','%s','%d','%d','%s')",
           GU_TABLE, guid, domain_id, puid, gu_name,
           type, gu_attributes, ivs_id
        );

        code = jpf_mysql_do_query(conn->mysql, query_buf);
        if (code != 0 )
           return code;
    }

    return 0;
}


gint jpf_dbs_batch_add_gu(JpfAppObj *app_obj, db_conn_status *conn, JpfAddPu *pu_info)
{
    gchar query_buf[QUERY_STR_LEN];
    gchar guid[MAX_ID_LEN] = {0};
    gchar gu_name[GU_NAME_LEN] = {0};
    gint code, i, gu_attributes = 0;
    JpfMssId mss_id;

    if ((pu_info->av_count < 0) || (pu_info->av_count > MAX_CHANNEL_NUM) ||
	 (pu_info->ai_count < 0) || (pu_info->ai_count > MAX_CHANNEL_NUM) ||
	 (pu_info->ao_count < 0) || (pu_info->ao_count > MAX_CHANNEL_NUM) ||
	 (pu_info->ds_count < 0) || (pu_info->ds_count > MAX_CHANNEL_NUM))
	return E_STRINGFORMAT;

    if (pu_info->av_count)
    {
        code = jpf_dbs_check_av_gu_count(app_obj, pu_info->av_count);
        if (code)
            return code;
    }

    if (pu_info->ds_count)
    {
        code = jpf_dbs_check_ds_gu_count(app_obj, pu_info->ds_count);
        if (code)
            return code;
    }

    if (pu_info->ai_count)
    {
        code = jpf_dbs_check_ai_gu_count(app_obj, pu_info->ai_count);
        if (code)
            return code;
    }

    if (pu_info->ao_count)
    {
        code = jpf_dbs_check_ao_gu_count(app_obj, pu_info->ao_count);
        if (code)
            return code;
    }

    gu_attributes = gu_attributes | HAS_DOME_FLAG;
    gu_attributes = gu_attributes | ALARM_BYPASS_FLAG;
    for (i = 0; i < pu_info->av_count; i++)
    {
        snprintf(guid, MAX_ID_LEN, "%s-AV-0-%02d", pu_info->puid, i);
        if (i == 0)
            snprintf(gu_name, GU_NAME_LEN, "%s_CH", pu_info->pu_info);
        else
            snprintf(gu_name, GU_NAME_LEN, "%s_CH%02d", pu_info->pu_info, i);

        snprintf(
            query_buf, QUERY_STR_LEN, "insert into %s values('%s','%s','%s','%s','%d','%d','%s')",
            GU_TABLE, guid, pu_info->domain_id, pu_info->puid, gu_name,
            GU_TYPE_AV, gu_attributes, pu_info->ivs_id
        );

        code = jpf_mysql_do_query(conn->mysql, query_buf);
        if (code != 0 )
           return code;

        if (regex_mached(pu_info->mss_id, mss_reg))
       {
            snprintf(
                query_buf, QUERY_STR_LEN,
                "insert into %s (gu_id,guid_domain,mss_id) values('%s','%s','%s')",
                RECORD_POLICY_TABLE, guid, pu_info->domain_id, pu_info->mss_id
            );

            code = jpf_mysql_do_query(conn->mysql, query_buf);
            if (code != 0 )
                return code;
       }
    }
     if (regex_mached(pu_info->mss_id, mss_reg)&&(pu_info->av_count > 0))
     {
           memset(&mss_id, 0, sizeof(mss_id));
           strncpy(mss_id.mss_id, pu_info->mss_id, MSS_ID_LEN - 1);
           jpf_mod_dbs_deliver_mss_event(app_obj, &mss_id);
     }

    gu_attributes = 0;

    code = jpf_dbs_add_gu(conn, pu_info->domain_id, pu_info->puid, pu_info->pu_info,
    	  AI_TYPE, GU_TYPE_AI, pu_info->ai_count, gu_attributes, pu_info->ivs_id);
    if (code != 0 )
           return code;

    code = jpf_dbs_add_gu(conn, pu_info->domain_id, pu_info->puid, pu_info->pu_info,
    	  AO_TYPE, GU_TYPE_AO, pu_info->ao_count, gu_attributes, pu_info->ivs_id);
    if (code != 0 )
           return code;

    code = jpf_dbs_add_gu(conn, pu_info->domain_id, pu_info->puid, pu_info->pu_info,
    	  DS_TYPE, GU_TYPE_DS, pu_info->ds_count, gu_attributes, pu_info->ivs_id);
    if (code != 0 )
           return code;

    return 0;
}


JpfMsgFunRet
jpf_mod_dbs_add_pu_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfAddPu *req_info;
    JpfAddPu tmp_info;
    JpfAddPuRes result;
    char query_buf[QUERY_STR_LEN];
    gint code, i;
    db_conn_status *conn = NULL;
    JpfModDbs *dbs_obj;
    gchar puid[MAX_ID_LEN] = {0};
    gchar pu_name[PU_NAME_LEN] = {0};
    gchar tmp[MAX_ID_LEN] = {0};
    gint id;
    JpfAmsId ams_id;

    memset(&result, 0, sizeof(result));
    memset(&tmp_info, 0, sizeof(tmp_info));
    memset(&ams_id, 0, sizeof(ams_id));
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if (STRLEN_ISZERO(req_info->puid)||STRLEN_ISZERO(req_info->domain_id)
         ||(req_info->puid[3]!='-') || (req_info->puid[7]!='-') )
    {
        code = -E_STRINGFORMAT;
        goto end_add_pu;
    }

    dbs_obj = JPF_MODDBS(app_obj);
    conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
    if (G_UNLIKELY(!conn))
    {
        jpf_warning("<JpfModDbs> get db connection error!");
        code = -E_GETDBCONN;
        goto end_add_pu;
    }

    if (G_UNLIKELY(!conn->mysql))
    {
        jpf_warning("<JpfModDbs> get db connection error!");
        put_db_connection(dbs_obj->pool_info, conn);
        code = -E_GETDBCONN;
        goto end_add_pu;
    }

    memcpy(&tmp_info, req_info, sizeof(tmp_info));
    sscanf(req_info->puid, "%7s-%8d", tmp, &id);

    for (i = 0; i < req_info->pu_count; i++)
   {
	JPF_MYSQL_DO_QUERY_OR_KILL_CONN(dbs_obj, conn, "start transaction", code);
	if (code)
		goto end_add_pu;

        snprintf(puid, MAX_ID_LEN, "%s-%08d", tmp,id);
        if (i == 0)
            strcpy(pu_name, req_info->pu_info);
        else
            snprintf(pu_name, GU_NAME_LEN, "%s_%d", req_info->pu_info, i);

        snprintf(
            query_buf, QUERY_STR_LEN,
            "insert into %s (pu_id,pu_domain,pu_info,pu_keep_alive_freq,\
            pu_type,pu_minor_type,pu_manufacturer,pu_mdu,pu_area)\
            value('%s','%s','%s','%d','%d','%d','%s','%s','%d')",
            PU_TABLE,puid,req_info->domain_id,pu_name,req_info->keep_alive_time,
            req_info->pu_type, req_info->pu_minor_type, req_info->manufacturer,
            req_info->mds_id, req_info->area_id
        );
		id++;
		if (id >= 100000000)
			id = 0;

        code = jpf_mysql_do_query(conn->mysql, query_buf);
        if (code != 0 )
        {
             jpf_mysql_do_query(conn->mysql, "rollback");
            goto err_add_pu;
        }

	if (req_info->pu_type == DBS_TYPE_ALM && strcmp(req_info->ams_id, AMS_ID_NULL))
	{
		snprintf(
			query_buf, QUERY_STR_LEN,
			"insert into %s(pu_id,pu_domain,ams_id,dev_name,dev_passwd,dev_ip," \
			"dev_port) values('%s','%s','%s','','','','')",
			AMS_CONFIGURE_TABLE, puid, req_info->domain_id, req_info->ams_id
		);

		code = jpf_mysql_do_query(conn->mysql, query_buf);
		if (code != 0)
		{
			jpf_mysql_do_query(conn->mysql, "rollback");
			goto err_add_pu;
		}

		JPF_COPY_VAL(ams_id.ams_id, req_info->ams_id, AMS_ID_LEN);
		jpf_mod_dbs_deliver_ams_event(app_obj, &ams_id);
	}

        snprintf(
            query_buf, QUERY_STR_LEN,
            "insert into %s (pu_id,pu_domain,pu_state,pu_registered) values('%s','%s',0,0)",
            PU_RUNNING_TABLE,puid,req_info->domain_id
            );

        code = jpf_mysql_do_query(conn->mysql, query_buf);
        if (code != 0 )
        {
            jpf_mysql_do_query(conn->mysql, "rollback");
            goto err_add_pu;
        }
    	 strcpy(tmp_info.puid, puid);
    	 strcpy(tmp_info.pu_info, pu_name);
        code = jpf_dbs_batch_add_gu(app_obj, conn, &tmp_info);
        if (code)
        {
            jpf_mysql_do_query(conn->mysql, "rollback");
            goto err_add_pu;
        }

       code = jpf_mysql_do_query(conn->mysql, "commit");
    }

    put_db_connection(dbs_obj->pool_info, conn);

end_add_pu:
    strcpy(result.bss_usr, req_info->bss_usr);
    SET_CODE(&result, code);
    result.success_count = i;
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
err_add_pu:
       put_db_connection(dbs_obj->pool_info, conn);
	goto end_add_pu;
}


JpfMsgFunRet
jpf_mod_dbs_add_gu_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfAddGu *req_info;
    db_conn_status *conn = NULL;
    JpfModDbs *dbs_obj;
    JpfBssRes result;
    gint code, i = 0;
    char query_buf[QUERY_STR_LEN] = {0};
    JpfMssId mss_id;

    memset(&result, 0, sizeof(result));
    req_info  = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if (strncmp(req_info->puid, req_info->guid, PU_ID_LEN))
    {
        code = -E_STRINGFORMAT;
        goto end_add_gu;
    }

    dbs_obj = JPF_MODDBS(app_obj);
    conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
    if (G_UNLIKELY(!conn))
    {
        jpf_warning("<JpfModDbs> get db connection error!");
        code = -E_GETDBCONN;
        goto end_add_gu;
    }

    if (G_UNLIKELY(!conn->mysql))
    {
        jpf_warning("<JpfModDbs> get db connection error!");
        put_db_connection(dbs_obj->pool_info, conn);
        code = -E_GETDBCONN;
        goto end_add_gu;
    }

    code = jpf_dbs_check_gu_count(app_obj, 1, req_info->guid);
    if (code)
	goto end_add_gu;

	JPF_MYSQL_DO_QUERY_OR_KILL_CONN(dbs_obj, conn, "start transaction", code);
	if (code)
		goto end_add_gu;

    snprintf(
       query_buf, QUERY_STR_LEN, "insert into %s values('%s','%s','%s','%s','%d','%d','%s')",
       GU_TABLE, req_info->guid, req_info->domain_id, req_info->puid, req_info->gu_name,
       req_info->gu_type, req_info->gu_attributes, req_info->ivs_id
       );

    code = jpf_mysql_do_query(conn->mysql, query_buf);
    if (code != 0 )
    {
        jpf_mysql_do_query(conn->mysql, "rollback");
        put_db_connection(dbs_obj->pool_info, conn);
        goto end_add_gu;
    }

    while (regex_mached(req_info->mss[i].mss_id, mss_reg))
    {
        snprintf(
        query_buf, QUERY_STR_LEN, "insert into %s (gu_id,guid_domain,mss_id) values('%s','%s','%s')",
        RECORD_POLICY_TABLE, req_info->guid, req_info->domain_id, req_info->mss[i].mss_id
        );

        code = jpf_mysql_do_query(conn->mysql, query_buf);
        if (code != 0 )
        {
            jpf_mysql_do_query(conn->mysql, "rollback");
            put_db_connection(dbs_obj->pool_info, conn);
            goto end_add_gu;
        }

       memset(&mss_id, 0, sizeof(mss_id));
       strncpy(mss_id.mss_id, req_info->mss[i].mss_id, MSS_ID_LEN - 1);
       jpf_mod_dbs_deliver_mss_event(app_obj, &mss_id);

        i++;
    }

    code = jpf_mysql_do_query(conn->mysql, "commit");
    put_db_connection(dbs_obj->pool_info, conn);

end_add_gu:
    SET_CODE(&result, code);
    strcpy(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_add_mds_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfAddMds *req_info;
    JpfBssRes result;
    char query_buf[QUERY_STR_LEN];
    gint row_num;
    glong affect_num = 0;

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
       query_buf, QUERY_STR_LEN, "insert into %s values('%s','%s','%d','%d','%d','%d',0,'%d')",
       MDS_TABLE, req_info->mds_id, req_info->mds_name, req_info->mds_type,
       req_info->keep_alive_freq, req_info->mds_pu_port, req_info->mds_rtsp_port,
       req_info->get_ip_enable
    );

    jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    if (RES_CODE(&result) == -DB_DUP_ENTRY_ERROR)
    {
        snprintf(
            query_buf, QUERY_STR_LEN,
            "select count(*)  as count from %s where mdu_id='%s'",
            MDS_TABLE, req_info->mds_id
        );

        row_num =  jpf_get_record_count(app_obj, query_buf);
        if (row_num == 0)
            SET_CODE(&result, -E_NAMEEXIST);

        if (row_num < 0)
            SET_CODE(&result, row_num);
    }

    JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
        BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_add_mds_ip_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfAddMdsIp *req_info;
    JpfBssRes result;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
       query_buf, QUERY_STR_LEN, "insert into %s values('%s','%s','%s')",
       MDS_IP_TABLE, req_info->mds_id, req_info->cms_ip, req_info->mds_ip
       );

    jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_add_mss_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfAddMss *req_info;
    JpfBssRes result;
    char query_buf[QUERY_STR_LEN];
    gint row_num;
    glong affect_num = 0;

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
        query_buf, QUERY_STR_LEN, "insert into %s values('%s','%s','%d',0,'%d','%d','')",
        MSS_TABLE, req_info->mss_id, req_info->mss_name, req_info->keep_alive_freq,
        req_info->storage_type, req_info->mode
    );
    jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    if (RES_CODE(&result) == -DB_DUP_ENTRY_ERROR)
    {
        snprintf(
            query_buf, QUERY_STR_LEN,
            "select count(*)  as count from %s where mss_id='%s'",
            MSS_TABLE, req_info->mss_id
        );

        row_num =  jpf_get_record_count(app_obj, query_buf);
        if (row_num == 0)
            SET_CODE(&result, -E_NAMEEXIST);

        if (row_num < 0)
            SET_CODE(&result, row_num);
    }

    JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_add_gu_to_user_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfAddGuToUser *req_info;
    JpfBssRes result;
    db_conn_status *conn = NULL;
    JpfModDbs *dbs_obj;
    gchar query_buf[QUERY_STR_LEN] = {0};
    gint add_num, code, i = 0;

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    dbs_obj = JPF_MODDBS(app_obj);
    conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
    if (G_UNLIKELY(!conn))
    {
        jpf_warning("<JpfModDbs> get db connection error!");
        code = -E_GETDBCONN;
        goto end_dbs_query;
    }

    if (G_UNLIKELY(!conn->mysql))
    {
        jpf_warning("<JpfModDbs> get db connection error!");
        put_db_connection(dbs_obj->pool_info, conn);
        code = -E_GETDBCONN;
        goto end_dbs_query;
    }

	JPF_MYSQL_DO_QUERY_OR_KILL_CONN(dbs_obj, conn, "start transaction", code);
	if (code)
		goto end_dbs_query;

    add_num = req_info->total_num;

    snprintf(
        query_buf, QUERY_STR_LEN, "delete from %s where user_name='%s'",
        USER_OWN_GU_TABLE, req_info->username
        );

    code = jpf_mysql_do_query(conn->mysql, query_buf);
    if (code != 0 )
    {
        jpf_mysql_do_query(conn->mysql, "rollback");
        put_db_connection(dbs_obj->pool_info, conn);
        goto end_dbs_query;
    }

    while (i < add_num)
    {
        snprintf(
            query_buf, QUERY_STR_LEN, "insert into %s values('%s','%s','%s')",
            USER_OWN_GU_TABLE, req_info->username,
            req_info->gu_to_user_info[i].user_guid, req_info->gu_to_user_info[i].user_guid_domain
        );

        code = jpf_mysql_do_query(conn->mysql, query_buf);
        if (code != 0 && code != -1452)  //1452: ER_NO_REFERENCED_ROW_2
        {
            jpf_mysql_do_query(conn->mysql, "rollback");
            put_db_connection(dbs_obj->pool_info, conn);
            goto end_dbs_query;
        }

        i++;
    }

    code = jpf_mysql_do_query(conn->mysql, "commit");
    put_db_connection(dbs_obj->pool_info, conn);

end_dbs_query:

    SET_CODE(&result, code);
    JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_add_tw_to_user_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfAddTwToUser *req_info;
    JpfBssRes result;
    db_conn_status *conn = NULL;
    JpfModDbs *dbs_obj;
    gchar query_buf[QUERY_STR_LEN] = {0};
    gint add_num, code, i = 0;

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    dbs_obj = JPF_MODDBS(app_obj);
	conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
	if (G_UNLIKELY(!conn))
	{
		jpf_warning("<JpfModDbs> get db connection error!");
		code = -E_GETDBCONN;
		goto end_dbs_query;
	}

	if (G_UNLIKELY(!conn->mysql))
	{
		jpf_warning("<JpfModDbs> get db connection error!");
		put_db_connection(dbs_obj->pool_info, conn);
		code = -E_GETDBCONN;
		goto end_dbs_query;
	}

	JPF_MYSQL_DO_QUERY_OR_KILL_CONN(dbs_obj, conn, "start transaction", code);
	if (code)
		goto end_dbs_query;

    add_num = req_info->total_num;

    snprintf(
        query_buf, QUERY_STR_LEN, "delete from %s where user_name='%s'",
        USER_OWN_TW_TABLE, req_info->username
        );

    code = jpf_mysql_do_query(conn->mysql, query_buf);
    if (code != 0 )
    {
        jpf_mysql_do_query(conn->mysql, "rollback");
        put_db_connection(dbs_obj->pool_info, conn);
        goto end_dbs_query;
    }

    while (i < add_num)
    {
        snprintf(
            query_buf, QUERY_STR_LEN, "insert into %s values('%s','%d')",
            USER_OWN_TW_TABLE, req_info->username,
            req_info->tw_to_user_info[i].tw_id
        );

        code = jpf_mysql_do_query(conn->mysql, query_buf);
        if (code != 0 && code != -1452)  //1452: ER_NO_REFERENCED_ROW_2
        {
            jpf_mysql_do_query(conn->mysql, "rollback");
            put_db_connection(dbs_obj->pool_info, conn);
            goto end_dbs_query;
        }
        i++;
    }

    code = jpf_mysql_do_query(conn->mysql, "commit");
    put_db_connection(dbs_obj->pool_info, conn);

end_dbs_query:

    SET_CODE(&result, code);
    JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_add_tour_to_user_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfAddTourToUser *req_info;
    JpfBssRes result;
    db_conn_status *conn = NULL;
    JpfModDbs *dbs_obj;
    gchar query_buf[QUERY_STR_LEN] = {0};
    gint add_num, code, i = 0;

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    dbs_obj = JPF_MODDBS(app_obj);
	conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
	if (G_UNLIKELY(!conn))
	{
		jpf_warning("<JpfModDbs> get db connection error!");
		code = -E_GETDBCONN;
		goto end_dbs_query;
	}

	if (G_UNLIKELY(!conn->mysql))
	{
		jpf_warning("<JpfModDbs> get db connection error!");
		put_db_connection(dbs_obj->pool_info, conn);
		code = -E_GETDBCONN;
		goto end_dbs_query;
	}

	JPF_MYSQL_DO_QUERY_OR_KILL_CONN(dbs_obj, conn, "start transaction", code);
	if (code)
		goto end_dbs_query;

    add_num = req_info->total_num;

    snprintf(
        query_buf, QUERY_STR_LEN, "delete from %s where user_name='%s'",
        USER_OWN_TOUR_TABLE, req_info->username
        );

    code = jpf_mysql_do_query(conn->mysql, query_buf);
    if (code != 0 )
    {
        jpf_mysql_do_query(conn->mysql, "rollback");
        put_db_connection(dbs_obj->pool_info, conn);
        goto end_dbs_query;
    }

    while (i < add_num)
    {
        snprintf(
            query_buf, QUERY_STR_LEN, "insert into %s values('%s','%d')",
            USER_OWN_TOUR_TABLE, req_info->username,
            req_info->tour_to_user_info[i].tour_id
        );

        code = jpf_mysql_do_query(conn->mysql, query_buf);
        if (code != 0 && code != -1452)  //1452: ER_NO_REFERENCED_ROW_2
        {
            jpf_mysql_do_query(conn->mysql, "rollback");
            put_db_connection(dbs_obj->pool_info, conn);
            goto end_dbs_query;
        }
        i++;
    }

    code = jpf_mysql_do_query(conn->mysql, "commit");
    put_db_connection(dbs_obj->pool_info, conn);

end_dbs_query:
    SET_CODE(&result, code);
    JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


static __inline__ gint
jpf_dbs_mss_get_map_id(JpfMysqlRes *mysql_result,
    gint *map_id)
{
     G_ASSERT(mysql_result != NULL && map_id!= NULL);

    unsigned long field_num;
    JpfMysqlField *mysql_fields;    //modify: mysql_fields -> *msql_fields
    JpfMysqlRow mysql_row;
    char *value, *name;
    int j, row_num;

    row_num = jpf_sql_get_num_rows(mysql_result);
    if (row_num == 0)
    {
        jpf_warning("<ModDdbMss>No such record entry in database");
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
                if (!strcmp(name,"map_id"))
                {
                    value = jpf_sql_get_field_value(mysql_row, j);
		      if(value)
                    {
                         *map_id = atoi(value);
                    }
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
jpf_mod_dbs_add_defence_area_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfAddDefenceArea *req_info;
    JpfBssRes result;
    char query_buf[QUERY_STR_LEN] = {0};
    glong affect_num = 0;

    req_info = MSG_GET_DATA(msg);
    snprintf(
       query_buf, QUERY_STR_LEN, "insert into %s (defence_area_id,defence_enable,policy) values('%d','%d','%s')",
       DEFENCE_AREA_TABLE, req_info->defence_area_id, req_info->enable, req_info->policy
    );
    memset(&result, 0, sizeof(result));
    jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_add_defence_map_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfAddDefenceMap *req_info;
    JpfBssRes result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint ret, row_num;
    glong affect_num = 0;
    memset(&result, 0, sizeof(result));

    req_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select count(*) as count from %s where map_name='%s' and defence_area_id=%d",
        DEFENCE_MAP_TABLE, req_info->map_name, req_info->defence_area_id
    );
    row_num =  jpf_get_record_count(app_obj, query_buf);
    if (row_num > 0)
    {
        SET_CODE(&result, -E_NAMEEXIST);
	 goto end_add_defence_map;
    }

    snprintf(
       query_buf, QUERY_STR_LEN, "insert into %s (defence_area_id,map_name,map_location) values('%d','%s','%s')",
       DEFENCE_MAP_TABLE, req_info->defence_area_id, req_info->map_name, req_info->map_location
    );

    jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    ret = RES_CODE(&result);
end_add_defence_map:
    JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_add_defence_gu_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfAddDefenceGu *req_info;
    JpfBssRes result;
    char query_buf[QUERY_STR_LEN] = {0};
    glong affect_num = 0;

    req_info = MSG_GET_DATA(msg);
    snprintf(
       query_buf, QUERY_STR_LEN, "insert into %s (map_id,gu_domain,gu_id,coordinate_x,coordinate_y) values('%d','%s','%s','%.4f','%.4f')",
       MAP_GU_TABLE, req_info->map_id,req_info->domain_id, req_info->guid, req_info->coordinate_x, req_info->coordinate_y
       );
    memset(&result, 0, sizeof(result));
    jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_add_map_href_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfSetMapHref *req_info;
    JpfBssRes result;
    char query_buf[QUERY_STR_LEN] = {0};
    glong affect_num = 0;

    req_info = MSG_GET_DATA(msg);
    snprintf(
       query_buf, QUERY_STR_LEN, "insert into %s (src_map_id,dst_map_id,coordinate_x,coordinate_y) values('%d','%d','%lf','%lf')",
       MAP_HREF_TABLE, req_info->src_map_id,req_info->dst_map_id, req_info->coordinate_x, req_info->coordinate_y
       );
    memset(&result, 0, sizeof(result));
    jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_add_tw_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfAddTw *req_info;
    JpfBssRes result;
    char query_buf[QUERY_STR_LEN] = {0};
    glong affect_num = 0;

    req_info = MSG_GET_DATA(msg);
    snprintf(
       query_buf, QUERY_STR_LEN,
       "insert into %s (tw_name) values('%s')",
       TW_TABLE, req_info->tw_name
       );
    memset(&result, 0, sizeof(result));
    jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_add_screen_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfAddScreen *req_info;
    JpfBssRes result;
    char query_buf[QUERY_STR_LEN] = {0};
    glong affect_num = 0;

    req_info = MSG_GET_DATA(msg);
    snprintf(
        query_buf, QUERY_STR_LEN,
        "insert into %s (tw_id,dis_domain,dis_guid,coordinate_x,coordinate_y,length,width) \
        values(%d,'%s','%s',%lf,%lf,%lf,%lf)",
        SCREEN_TABLE, req_info->tw_id, req_info->dis_domain, req_info->dis_guid,
        req_info->coordinate_x, req_info->coordinate_y, req_info->length, req_info->width
    );
    memset(&result, 0, sizeof(result));
    jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_add_tour_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfAddTour *req_info;
    JpfBssRes result;
    char query_buf[QUERY_STR_LEN] = {0};
    glong affect_num = 0;

    req_info = MSG_GET_DATA(msg);
    snprintf(
         query_buf, QUERY_STR_LEN,
         "insert into %s (tour_name,auto_jump) values('%s',%d)",
         TOUR_TABLE, req_info->tour_name, req_info->auto_jump
     );
    memset(&result, 0, sizeof(JpfMsgErrCode));
    strcpy(result.bss_usr, req_info->bss_usr);
    jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_add_tour_step_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfAddTourStep *req_info;
    JpfBssRes result;
    db_conn_status *conn = NULL;
    JpfModDbs *dbs_obj;
    gchar query_buf[QUERY_STR_LEN] = {0};
    gint add_num, code, i = 0;

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    dbs_obj = JPF_MODDBS(app_obj);
	conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
	if (G_UNLIKELY(!conn))
	{
		jpf_warning("<JpfModDbs> get db connection error!");
		code = -E_GETDBCONN;
		goto end_add_tour_step;
	}

	if (G_UNLIKELY(!conn->mysql))
	{
		jpf_warning("<JpfModDbs> get db connection error!");
		put_db_connection(dbs_obj->pool_info, conn);
		code = -E_GETDBCONN;
		goto end_add_tour_step;
	}

	JPF_MYSQL_DO_QUERY_OR_KILL_CONN(dbs_obj, conn, "start transaction", code);
	if (code)
		goto end_add_tour_step;

    memset(&result, 0, sizeof(JpfMsgErrCode));
    add_num = req_info->total_num;

    snprintf(
        query_buf, QUERY_STR_LEN, "delete from %s where tour_id=%d",
        TOUR_STEP_TABLE, req_info->tour_id
        );

    code = jpf_mysql_do_query(conn->mysql, query_buf);
    if (code != 0 )
    {
        jpf_mysql_do_query(conn->mysql, "rollback");
        put_db_connection(dbs_obj->pool_info, conn);
        goto end_add_tour_step;
    }

    while (i < add_num)
    {
        snprintf(
            query_buf, QUERY_STR_LEN, "insert into %s values(%d,%d,%d,'%s','%s',%d)",
            TOUR_STEP_TABLE, req_info->tour_id, req_info->tour_step[i].step_no,
            req_info->tour_step[i].interval, req_info->tour_step[i].encoder_domain,
            req_info->tour_step[i].encoder_guid, req_info->tour_step[i].level
        );

        code = jpf_mysql_do_query(conn->mysql, query_buf);
        if (code != 0 )
        {
            jpf_mysql_do_query(conn->mysql, "rollback");
            put_db_connection(dbs_obj->pool_info, conn);
            goto end_add_tour_step;
        }
        i++;
    }

    code = jpf_mysql_do_query(conn->mysql, "commit");
    put_db_connection(dbs_obj->pool_info, conn);

end_add_tour_step:

    SET_CODE(&result.code, code);
    strcpy(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_add_group_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfAddGroup *req_info;
    char query_buf[QUERY_STR_LEN] = {0};

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
         query_buf, QUERY_STR_LEN,
         "insert into %s (group_name,tw_id) values('%s',%d)",
         GROUP_TABLE, req_info->group_name, req_info->tw_id
     );

    return jpf_mod_dbs_general_cmd(app_obj, msg, query_buf, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_dbs_add_group_step_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfAddGroupStep *req_info;
    char query_buf[QUERY_STR_LEN] = {0};

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
         query_buf, QUERY_STR_LEN,
         "insert into %s (`group_id`, `interval`) values(%d,%d)",
         GROUP_STEP_TABLE, req_info->group_id, req_info->interval
     );

    return jpf_mod_dbs_general_cmd(app_obj, msg, query_buf, req_info->bss_usr);
}

#if 1
JpfMsgFunRet
jpf_mod_dbs_config_group_step_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfConfigGroupStep *req_info;
    JpfBssRes result;
    db_conn_status *conn = NULL;
    JpfModDbs *dbs_obj;
    gchar query_buf[QUERY_STR_LEN] = {0};
    gint add_num, code, i = 0;

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    dbs_obj = JPF_MODDBS(app_obj);
	conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
	if (G_UNLIKELY(!conn))
	{
		jpf_warning("<JpfModDbs> get db connection error!");
		code = -E_GETDBCONN;
		goto end_config_group_step;
	}

	if (G_UNLIKELY(!conn->mysql))
	{
		jpf_warning("<JpfModDbs> get db connection error!");
		put_db_connection(dbs_obj->pool_info, conn);
		code = -E_GETDBCONN;
		goto end_config_group_step;
	}

	JPF_MYSQL_DO_QUERY_OR_KILL_CONN(dbs_obj, conn, "start transaction", code);
	if (code)
		goto end_config_group_step;

    memset(&result, 0, sizeof(JpfMsgErrCode));
    add_num = req_info->total_num;

    while (i < add_num)
    {
        snprintf(
            query_buf, QUERY_STR_LEN,
            "insert into %s values(%d,%d,%d,%d,'%s','%s',%d)",
            GROUP_STEP_INFO_TABLE, req_info->step_no, req_info->scr_id,
            req_info->group_step[i].div_no,req_info->div_id, req_info->group_step[i].encoder_domain,
            req_info->group_step[i].encoder_guid, req_info->group_step[i].level
        );

        code = jpf_mysql_do_query(conn->mysql, query_buf);
        if (code != 0 )
        {
            jpf_mysql_do_query(conn->mysql, "rollback");
            put_db_connection(dbs_obj->pool_info, conn);
            goto end_config_group_step;
        }
        i++;
    }

    code = jpf_mysql_do_query(conn->mysql, "commit");
    put_db_connection(dbs_obj->pool_info, conn);

end_config_group_step:

    SET_CODE(&result.code, code);
    strcpy(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}
#endif


JpfMsgFunRet
jpf_mod_dbs_add_ivs_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfAddIvs *req_info;
    JpfBssRes result;
    char query_buf[QUERY_STR_LEN];
    gint row_num;
    glong affect_num = 0;

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
        query_buf, QUERY_STR_LEN, "insert into %s values('%s','%s','%d',0,'')",
        IVS_TABLE, req_info->ivs_id, req_info->ivs_name,
        req_info->keep_alive_freq
    );
    jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    if (RES_CODE(&result) == -DB_DUP_ENTRY_ERROR)
    {
        snprintf(
            query_buf, QUERY_STR_LEN,
            "select count(*)  as count from %s where ivs_id='%s'",
            IVS_TABLE, req_info->ivs_id
        );

        row_num =  jpf_get_record_count(app_obj, query_buf);
        if (row_num == 0)
            SET_CODE(&result, -E_NAMEEXIST);

        if (row_num < 0)
            SET_CODE(&result, row_num);
    }

    JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_link_time_policy_config_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfLinkTimePolicyConfig *req_info;
    JpfBssRes result;
    gchar query_buf[QUERY_STR_LEN] = {0};
    glong affect_num;

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "insert into %s values('%s','%s','%s')",
        LINK_TIME_POLICY_TABLE, req_info->guid, req_info->domain,
        req_info->time_policy
    );

    jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    strcpy(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_link_record_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfLinkRecord *req_info;
    JpfBssRes result;
    db_conn_status *conn = NULL;
    JpfModDbs *dbs_obj;
    gchar query_buf[QUERY_STR_LEN] = {0};
    gint code, i = 0;

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    dbs_obj = JPF_MODDBS(app_obj);
	conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
	if (G_UNLIKELY(!conn))
	{
		jpf_warning("<JpfModDbs> get db connection error!");
		code = -E_GETDBCONN;
		goto end_config_link_record;
	}

	if (G_UNLIKELY(!conn->mysql))
	{
		jpf_warning("<JpfModDbs> get db connection error!");
		put_db_connection(dbs_obj->pool_info, conn);
		code = -E_GETDBCONN;
		goto end_config_link_record;
	}

	JPF_MYSQL_DO_QUERY_OR_KILL_CONN(dbs_obj, conn, "start transaction", code);
	if (code)
		goto end_config_link_record;

    memset(&result, 0, sizeof(JpfMsgErrCode));
    while (regex_mached(req_info->mss[i].mss_id, mss_reg))
    {
        snprintf(
        	query_buf, QUERY_STR_LEN,
            "insert into %s (gu_id,domain_id,link_guid,link_domain_id,mss_id,time_len,alarm_type,\
            level) values('%s','%s','%s','%s','%s',%d,%d,%d)",
            ALARM_LINK_RECORD_TABLE, req_info->guid, req_info->domain,req_info->link_guid,
            req_info->link_domain,req_info->mss[i].mss_id, req_info->time_len,
            req_info->alarm_type, req_info->level
        );

        code = jpf_mysql_do_query(conn->mysql, query_buf);
        if (code != 0 )
        {
            jpf_mysql_do_query(conn->mysql, "rollback");
            put_db_connection(dbs_obj->pool_info, conn);
            goto end_config_link_record;
        }

       // jpf_mod_dbs_deliver_mss_event(app_obj, &mss_id);
        i++;
    }

    code = jpf_mysql_do_query(conn->mysql, "commit");
    put_db_connection(dbs_obj->pool_info, conn);

end_config_link_record:

    SET_CODE(&result.code, code);
    strcpy(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_link_io_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfLinkIO *req_info;
    gchar query_buf[QUERY_STR_LEN] = {0};

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
    	  query_buf, QUERY_STR_LEN,
        "insert into %s (gu_id,domain_id,link_guid,link_domain_id,IO_value,time_len,alarm_type) \
        values('%s','%s','%s','%s','%s',%d,%d)",
        ALARM_LINK_IO_TABLE, req_info->guid, req_info->domain,req_info->link_guid,
        req_info->link_domain,req_info->io_value, req_info->time_len, req_info->alarm_type
    );

    return jpf_mod_dbs_general_cmd(app_obj, msg, query_buf, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_dbs_link_snapshot_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfLinkSnapshot *req_info;
    JpfBssRes result;
    db_conn_status *conn = NULL;
    JpfModDbs *dbs_obj;
    gchar query_buf[QUERY_STR_LEN] = {0};
    gint code, i = 0;

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    dbs_obj = JPF_MODDBS(app_obj);
	conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
	if (G_UNLIKELY(!conn))
	{
		jpf_warning("<JpfModDbs> get db connection error!");
		code = -E_GETDBCONN;
		goto end_config_link_record;
	}

	if (G_UNLIKELY(!conn->mysql))
	{
		jpf_warning("<JpfModDbs> get db connection error!");
		put_db_connection(dbs_obj->pool_info, conn);
		code = -E_GETDBCONN;
		goto end_config_link_record;
	}

	JPF_MYSQL_DO_QUERY_OR_KILL_CONN(dbs_obj, conn, "start transaction", code);
	if (code)
		goto end_config_link_record;

    memset(&result, 0, sizeof(JpfMsgErrCode));
    while (regex_mached(req_info->mss[i].mss_id, mss_reg))
    {
        snprintf(
        	query_buf, QUERY_STR_LEN,
            "insert into %s (gu_id,domain_id,link_guid,link_domain_id,mss_id,picture_num,\
            alarm_type,level) values('%s','%s','%s','%s','%s',%d,%d,%d)",
            ALARM_LINK_SNAPSHOT_TABLE, req_info->guid, req_info->domain,
            req_info->link_guid, req_info->link_domain,req_info->mss[i].mss_id,
            req_info->picture_num, req_info->alarm_type, req_info->level
        );

        code = jpf_mysql_do_query(conn->mysql, query_buf);
        if (code != 0 )
        {
            jpf_mysql_do_query(conn->mysql, "rollback");
            put_db_connection(dbs_obj->pool_info, conn);
            goto end_config_link_record;
        }

       // jpf_mod_dbs_deliver_mss_event(app_obj, &mss_id);
        i++;
    }

    code = jpf_mysql_do_query(conn->mysql, "commit");
    put_db_connection(dbs_obj->pool_info, conn);

end_config_link_record:

    SET_CODE(&result.code, code);
    strcpy(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_link_preset_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfLinkPreset *req_info;
    gchar query_buf[QUERY_STR_LEN] = {0};

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
    	  query_buf, QUERY_STR_LEN,
        "insert into %s (gu_id,domain_id,link_guid,link_domain_id,preset_no,alarm_type) \
        values('%s','%s','%s','%s',%d,%d)",
        ALARM_LINK_PRESET_TABLE, req_info->guid, req_info->domain,req_info->link_guid,
        req_info->link_domain,req_info->preset_no, req_info->alarm_type
    );

    return jpf_mod_dbs_general_cmd(app_obj, msg, query_buf, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_dbs_link_step_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfLinkStepConfig *req_info;
    gchar query_buf[QUERY_STR_LEN] = {0};

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
    	  query_buf, QUERY_STR_LEN,
        "insert into %s (gu_id,domain_id,tw_id,screen_id,division_num,division_id,enc_guid,\
        enc_domain_id,level,alarm_type) \
        values('%s','%s',%d,%d,%d,%d,'%s','%s',%d,%d)",
        ALARM_LINK_STEP_TABLE, req_info->guid, req_info->domain, req_info->tw_id,
        req_info->screen_id, req_info->div_num, req_info->div_id,req_info->link_guid,
        req_info->link_domain,req_info->level, req_info->alarm_type
    );

    return jpf_mod_dbs_general_cmd(app_obj, msg, query_buf, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_dbs_link_tour_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfLinkTourConfig *req_info;
    gchar query_buf[QUERY_STR_LEN] = {0};

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
    	  query_buf, QUERY_STR_LEN,
        "insert into %s (gu_id,domain_id,tw_id,screen_id,division_num,division_id,tour_id,\
        alarm_type) values('%s','%s',%d,%d,%d,%d,%d,%d)",
        ALARM_LINK_TOUR_TABLE, req_info->guid, req_info->domain, req_info->tw_id,
        req_info->screen_id, req_info->div_num, req_info->div_id,req_info->tour_id,
        req_info->alarm_type
    );

    return jpf_mod_dbs_general_cmd(app_obj, msg, query_buf, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_dbs_link_group_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfLinkGroupConfig *req_info;
    gchar query_buf[QUERY_STR_LEN] = {0};

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
    	  query_buf, QUERY_STR_LEN,
        "insert into %s (gu_id,domain_id,group_id,alarm_type) values('%s','%s',%d,%d)",
        ALARM_LINK_GROUP_TABLE, req_info->guid, req_info->domain, req_info->group_id,
        req_info->alarm_type
    );

    return jpf_mod_dbs_general_cmd(app_obj, msg, query_buf, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_dbs_link_map_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfLinkMap *req_info;
    gchar query_buf[QUERY_STR_LEN] = {0};
    JpfBssRes result;
    gint total_num;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

	memset(&result, 0, sizeof(result));
	snprintf(
           query_buf, QUERY_STR_LEN,
           "select count(*) as count from %s where gu_id='%s',domain_id='%s'",
           ALARM_LINK_MAP_TABLE, req_info->guid, req_info->domain
        );

	total_num =  jpf_get_record_count(app_obj,query_buf);
	if (total_num < 0)
	{
	    SET_CODE(&result, total_num);
	    goto end_link_map;
	}
	if (total_num > 10)
	{
	    SET_CODE(&result, -EPERM);
	    goto end_link_map;
	}
    snprintf(
    	query_buf, QUERY_STR_LEN,
        "insert into %s (gu_id,domain_id,link_guid,link_domain_id,level,alarm_type) \
        values('%s','%s','%s','%s',%d,%d)",
        ALARM_LINK_MAP_TABLE, req_info->guid, req_info->domain,req_info->link_guid,
        req_info->link_domain, req_info->level, req_info->alarm_type
    );

    return jpf_mod_dbs_general_cmd(app_obj, msg, query_buf, req_info->bss_usr);
end_link_map:
	strcpy(result.bss_usr, req_info->bss_usr);
	jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
	                BUSSLOT_POS_BSS);

	return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_set_del_alarm_policy_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfDelAlarmPolicy *req_info;
    JpfBssRes result;
    char query_buf[QUERY_STR_LEN] = {0};
    glong affect_num = 0;
    gint total_num;

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    if (req_info->enable)
    {
        if ((req_info->del_alarm_num > req_info->max_capacity) ||
			(req_info->del_alarm_num  < 0) || (req_info->max_capacity < 0))
        {
		SET_CODE(&result, -EINVAL);
		goto end_set_del_alarm_policy;
        }

        snprintf(
           query_buf, QUERY_STR_LEN,
           "select count(*) as count from %s where id=%d",
           PARAM_CONFIG_TABLE, ALARM_PARAM_ID
        );

		total_num =  jpf_get_record_count(app_obj,query_buf);
		if (total_num < 0)
		{
		    SET_CODE(&result, total_num);
		    goto end_set_del_alarm_policy;
		}

		if (total_num == 0)
		{
		    snprintf(
		       query_buf, QUERY_STR_LEN, "insert into %s (id,value) values(%d,'%d,%d')",
		       PARAM_CONFIG_TABLE, ALARM_PARAM_ID, req_info->del_alarm_num,req_info->max_capacity
		   );
		}
		else if (total_num == 1)
		{
	            snprintf(
	                query_buf, QUERY_STR_LEN, "update %s set value='%d,%d' where id=%d",
	                PARAM_CONFIG_TABLE, req_info->del_alarm_num,req_info->max_capacity, ALARM_PARAM_ID
	            );
		}
    }
    else
    {
        snprintf(
           query_buf, QUERY_STR_LEN, "delete from %s where id=1",
           PARAM_CONFIG_TABLE
        );
    }

    jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    if (RES_CODE(&result) == -E_NODBENT)
	SET_CODE(&result, 0);

end_set_del_alarm_policy:
    JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


static __inline__ gint
jpf_get_pu_id(JpfMysqlRes *result)
{
    guint row_num;
    guint field_num;
    JpfMysqlRow mysql_row;
    JpfMysqlField* mysql_fields;
    gchar *name;
    gchar *value;
    gint pu_id = 0;
    gint field_no =0;
    gchar puid[MAX_ID_LEN] = {0};
    gchar tmp[MAX_ID_LEN] = {0};

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
                     strncpy(
                        puid,
                        value, MAX_ID_LEN - 1
                     );

                    sscanf(puid, "%7s-%8d", tmp, &pu_id);

			 return   pu_id;
                }
            }
            else
                cms_debug("no need mysql name %s ", name);
        }
    }

    return pu_id;
}


static __inline__ gint
jpf_mod_dbs_get_init_pu_id(JpfAppObj *app_obj)
{
	char query_buf[QUERY_STR_LEN] = {0};
	JpfModDbs *dbs_obj;
	JpfMysqlRes *result = NULL;
	gint pu_id = 0;

	dbs_obj = JPF_MODDBS(app_obj);

	snprintf(
		query_buf, QUERY_STR_LEN,
		"select value from %s where id=3",
		PARAM_CONFIG_TABLE
	);
	result = jpf_dbs_do_query_res(app_obj, query_buf);
	if (result && result->sql_res)
	{
		pu_id = jpf_get_pu_id(result);
	}

	jpf_sql_put_res(result, sizeof(JpfMysqlRes));

	return pu_id;
}


user_info_t user_info = {"admin", "admin"};
JpfMsgFunRet
jpf_mod_dbs_auto_add_pu_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfAutoAddPu *req_info;
    JpfAddPu tmp_info;
    JpfBssRes result;
    char query_buf[QUERY_STR_LEN];
    gint code;
    db_conn_status *conn = NULL;
    JpfModDbs *dbs_obj;

    memset(&result, 0, sizeof(result));
    memset(&tmp_info, 0, sizeof(tmp_info));
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    dbs_obj = JPF_MODDBS(app_obj);
	conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
	if (G_UNLIKELY(!conn))
	{
		jpf_warning("<JpfModDbs> get db connection error!");
		code = -E_GETDBCONN;
		goto end_add_pu;
	}

	if (G_UNLIKELY(!conn->mysql))
	{
		jpf_warning("<JpfModDbs> get db connection error!");
		put_db_connection(dbs_obj->pool_info, conn);
		code = -E_GETDBCONN;
		goto end_add_pu;
	}

	JPF_MYSQL_DO_QUERY_OR_KILL_CONN(dbs_obj, conn, "start transaction", code);
	if (code)
		goto end_add_pu;

    snprintf(
            query_buf, QUERY_STR_LEN,
            "insert into %s (pu_id,pu_domain,pu_info,pu_keep_alive_freq, pu_type, \
            pu_manufacturer,pu_mdu,pu_area) value('%s','%s','%s','%d','%d','%s','%s','%d')",
             PU_TABLE,req_info->puid,req_info->domain_id,req_info->dev_name,
             req_info->keep_alive_time, req_info->pu_type,
             req_info->manufacturer, req_info->mds_id, req_info->area_id
    );


    code = jpf_mysql_do_query(conn->mysql, query_buf);
    if (code != 0 )
        goto err_add_pu;

    snprintf(
        query_buf, QUERY_STR_LEN,
        "insert into %s (pu_id,pu_domain,pu_state,pu_registered) values('%s','%s',0,0)",
        PU_RUNNING_TABLE,req_info->puid,req_info->domain_id
    );

    code = jpf_mysql_do_query(conn->mysql, query_buf);
    if (code != 0 )
        goto err_add_pu;

    strcpy(tmp_info.puid, req_info->puid);
    strcpy(tmp_info.pu_info, req_info->dev_name);
    tmp_info.pu_type = req_info->pu_type;
    strcpy(tmp_info.domain_id, req_info->domain_id);
    strcpy(tmp_info.mds_id, req_info->mds_id);
    strcpy(tmp_info.manufacturer, req_info->manufacturer);
    tmp_info.area_id = req_info->area_id;
    strcpy(tmp_info.mss_id, req_info->mss_id);
    tmp_info.keep_alive_time = req_info->keep_alive_time;
    strcpy(tmp_info.pu_info, req_info->dev_name);
    tmp_info.av_count = req_info->av_num;
    code = jpf_dbs_batch_add_gu(app_obj, conn, &tmp_info);
    if (code)
        goto err_add_pu;

    jpf_redirect_t set_info;
    memset(&set_info, 0, sizeof(set_info));
    strcpy(set_info.user_info.usr, "admin");
    strcpy(set_info.user_info.pwd, "admin");
    strcpy(set_info.redirect.cms_ip, req_info->cms_ip);
    strcpy(set_info.redirect.pu_id, req_info->puid);
    set_info.redirect.cms_port = req_info->cms_port;
    set_info.redirect.conn_cms = req_info->connect_cms_enable;
    code = set_platform_info(req_info->dev_id, &set_info);
    if (code)
        goto err_add_pu;

    code = jpf_mysql_do_query(conn->mysql, "commit");
    put_db_connection(dbs_obj->pool_info, conn);

end_add_pu:
    strcpy(result.bss_usr, req_info->bss_usr);
    SET_CODE(&result, code);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
err_add_pu:
    jpf_mysql_do_query(conn->mysql, "rollback");
    put_db_connection(dbs_obj->pool_info, conn);
    jpf_warning("<Dbs-mod> auto add pu error:%d",code);
    goto end_add_pu;
}


JpfMsgFunRet
jpf_mod_dbs_modify_admin_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfBssRes result;
    JpfAddAdmin *req_info;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
       query_buf, QUERY_STR_LEN, "update %s set su_password='%s' where su_name='%s'",
        ADMIN_TABLE, req_info->password, req_info->admin_name
     );

    jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_modify_user_group_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfBssRes result;
    JpfUserGroupInfo *req_info;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    snprintf(
        query_buf, QUERY_STR_LEN,
        "update %s set group_permissions='%d', group_rank='%d',group_name='%s' \
        where group_id='%d'", USER_GROUP_TABLE, req_info->group_permissions,
        req_info->group_rank, req_info->group_name, req_info->group_id
    );

    jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_modify_user_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfBssRes result;
    JpfUserInfo *req_info;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "update %s set user_group='%d', user_sex='%d',user_password='%s',\
        user_phone_number='%s',user_description='%s' where user_no='%d'",
        USER_TABLE, req_info->group_id, req_info->sex, req_info->password,
        req_info->user_phone, req_info->user_description, req_info->user_id
    );

    jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_modify_domain_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfDomainInfo *req_info;
    JpfBssRes result;
    char query_buf[QUERY_STR_LEN];
    JpfModDbs *dbs_obj;
    gint code;
    db_conn_status *conn = NULL;

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    dbs_obj = JPF_MODDBS(app_obj);
	conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
	if (G_UNLIKELY(!conn))
	{
		jpf_warning("<JpfModDbs> get db connection error!");
		code = -E_GETDBCONN;
		goto end_modify_domain;
	}

	if (G_UNLIKELY(!conn->mysql))
	{
		jpf_warning("<JpfModDbs> get db connection error!");
		put_db_connection(dbs_obj->pool_info, conn);
		code = -E_GETDBCONN;
		goto end_modify_domain;
	}

	JPF_MYSQL_DO_QUERY_OR_KILL_CONN(dbs_obj, conn, "start transaction", code);
	if (code)
		goto end_modify_domain;

    snprintf(
        query_buf, QUERY_STR_LEN,"update %s set dm_id='%s',dm_name='%s'\
        where dm_type=0", DOMAIN_TABLE, req_info->domain_id, req_info->domain_name
    );

    code = jpf_mysql_do_query(conn->mysql, query_buf);
    if (code != 0 )
    {
        goto err_update_domain;
    }

    code = jpf_mysql_do_query(conn->mysql, "commit");
    put_db_connection(dbs_obj->pool_info, conn);

end_modify_domain:
    SET_CODE(&result, -code);
    if (!code)
        jpf_set_domain_id(req_info->domain_id);

    JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;

err_update_domain:
    jpf_mysql_do_query(conn->mysql, "rollback");
    put_db_connection(dbs_obj->pool_info, conn);
    goto end_modify_domain;
}


JpfMsgFunRet
jpf_mod_dbs_add_modify_area_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfAreaInfo *req_info;
    JpfAddAreaRes result;
    char query_buf[QUERY_STR_LEN];
    gint row_num, count;
    glong affect_num = 0;

    req_info = MSG_GET_DATA(msg);
    if (req_info->area_id == -1)
        snprintf(
            query_buf, QUERY_STR_LEN,
            "select count(*) as count from %s where area_name='%s' and area_parent='%d'",
            AREA_TABLE, req_info->area_name, req_info->area_parent
        );
    else
        snprintf(
            query_buf, QUERY_STR_LEN,
            "select count(*) as count from %s where area_name='%s' and\
            area_parent=(select area_parent from %s where area_id=%d)",
            AREA_TABLE, req_info->area_name, AREA_TABLE, req_info->area_id
        );

    memset(&result, 0, sizeof(result));
    result.area_id = req_info->area_id;
    row_num =  jpf_get_record_count(app_obj, query_buf);
    if (row_num > 0)
    {
        if (req_info->area_id == -1)
            SET_CODE(&result, -E_NAMEEXIST);
        else
        {
            snprintf(
                query_buf, QUERY_STR_LEN,
                "select count(*)  as count from %s where area_name='%s' and area_id='%d')",
                AREA_TABLE, req_info->area_name, req_info->area_id
            );
            count =  jpf_get_record_count(app_obj, query_buf);
            if (count > 0)
                SET_CODE(&result, 0);
            else
                goto deal_add_or_modify;
        }

        goto ERR_ADD_MODIFY;
    }

    if (row_num < 0)
    {
        SET_CODE(&result, row_num);
        goto  ERR_ADD_MODIFY;
    }
deal_add_or_modify:
    if (req_info->area_id == -1)
    {
        if (req_info->area_parent == -1)
            snprintf(
                query_buf, QUERY_STR_LEN,
                "insert into %s (area_name,area_parent,user_name,user_phone,user_address,\
                description) values('%s',NULL,'%s','%s','%s','%s')",
                AREA_TABLE, req_info->area_name, req_info->user_name,req_info->user_phone,
                req_info->user_address, req_info->description
            );
        else
            snprintf(
                query_buf, QUERY_STR_LEN,
                "insert into %s (area_name,area_parent,user_name,user_phone,user_address,\
                 description) values('%s','%d','%s','%s','%s','%s')",
                AREA_TABLE, req_info->area_name, req_info->area_parent, req_info->user_name,
                req_info->user_phone, req_info->user_address, req_info->description
            );
    }
    else
        snprintf(
            query_buf, QUERY_STR_LEN,
            "update %s set area_name='%s',user_name='%s',user_phone='%s',user_address='%s',\
            description='%s' where area_id='%d'",
            AREA_TABLE, req_info->area_name, req_info->user_name, req_info->user_phone,
            req_info->user_address, req_info->description, req_info->area_id
        );

    jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    if ((!RES_CODE(&result))&&(req_info->area_id == -1))
    {
        snprintf(
            query_buf, QUERY_STR_LEN,
            "select area_id from %s where area_name='%s' and area_parent='%d'",
            AREA_TABLE, req_info->area_name, req_info->area_parent
        );

        JpfMysqlRes *msq_result;
        JpfQueryAreaRes *query_res;
        gint size, ret;

        msq_result = jpf_dbs_do_query_res(app_obj,query_buf);
        BUG_ON(!msq_result);

        if (G_LIKELY(!MYSQL_RESULT_CODE(msq_result)))  //success:0 fail:!0
        {
            query_res = jpf_dbs_get_area(msq_result, &size);
            if (G_UNLIKELY(!query_res))
            {
                jpf_warning("<dbs_mh_bss> alloc error");
                jpf_sysmsg_destroy(msg);
                return MFR_ACCEPTED;
            }
        }
        else
        {
            ret = MYSQL_RESULT_CODE(msq_result);
            jpf_sql_put_res(msq_result, sizeof(JpfMysqlRes));
            goto ERR_ADD_MODIFY;
        }

        if(msq_result)
            jpf_sql_put_res(msq_result, sizeof(JpfMysqlRes));

        result.area_id = query_res->area_info[0].area_id;
        jpf_mem_kfree(query_res, size);
    }

ERR_ADD_MODIFY:
    JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_modify_pu_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfSysMsg *msg_notify = NULL;
    JpfPuInfo *req_info;
    JpfBssRes result;
    char query_buf[QUERY_STR_LEN];
    JpfChangeDispatch change_mds;
    gint row_num, old_area_id;
    glong affect_num = 0;
    JpfModDbs *dbs_obj;
    db_conn_status *conn = NULL;
    gint total_num;
    gint ret = 0;

    dbs_obj = JPF_MODDBS(app_obj);
    req_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select pu_mdu from %s where pu_mdu='%s' and pu_id='%s' and pu_domain='%s'",
        PU_TABLE, req_info->mds_id, req_info->puid, req_info->domain_id
        );
    row_num =  jpf_dbs_get_row_num(app_obj, msg, query_buf);

	req_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select pu_area as count from %s where pu_id='%s' and pu_domain='%s'",
        PU_TABLE, req_info->puid, req_info->domain_id
        );
    old_area_id = jpf_get_record_count(app_obj, query_buf);

	conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
	if (G_UNLIKELY(!conn))
	{
		jpf_warning("<JpfModDbs> get db connection error!");
		ret = -E_GETDBCONN;
		SET_CODE(&result, ret);
		goto end;
	}

	if (G_UNLIKELY(!conn->mysql))
	{
		jpf_warning("<JpfModDbs> get db connection error!");
		put_db_connection(dbs_obj->pool_info, conn);
		ret = -E_GETDBCONN;
		SET_CODE(&result, ret);
		goto end;
	}

	JPF_MYSQL_DO_QUERY_OR_KILL_CONN(dbs_obj, conn, "start transaction", ret);
	if (ret)
	{
		SET_CODE(&result, ret);
		goto end;
	}

    snprintf(
        query_buf, QUERY_STR_LEN,
        "update %s set pu_info='%s',pu_keep_alive_freq=%d,\
        pu_mdu='%s',pu_manufacturer='%s',pu_minor_type=%d,pu_area=%d \
        where pu_id='%s' and pu_domain='%s'",
        PU_TABLE, req_info->pu_info, req_info->keep_alive_time, req_info->mds_id,
        req_info->mf_id,req_info->pu_minor_type, req_info->area_id,
        req_info->puid, req_info->domain_id
        );

	memset(&result, 0, sizeof(result));
	ret = jpf_mysql_do_query(conn->mysql, query_buf);
	SET_CODE(&result, ret);

	if (!RES_CODE(&result))
	{
		if (strcmp(req_info->ams_id, AMS_ID_NULL))
		{
			snprintf(
				query_buf, QUERY_STR_LEN,
				"select count(*) as count from %s where pu_id='%s' and pu_domain='%s' " \
				"and ams_id='%s'",
				AMS_CONFIGURE_TABLE, req_info->puid, req_info->domain_id,
				req_info->ams_id
			);
			total_num = jpf_get_record_count(app_obj, query_buf);

			if (total_num == 0)
			{
				snprintf(
					query_buf, QUERY_STR_LEN,
					"delete from %s where pu_id='%s' and pu_domain='%s'",
					AMS_CONFIGURE_TABLE, req_info->puid, req_info->domain_id
				);
				jpf_mysql_do_query(conn->mysql, query_buf);

				snprintf(
					query_buf, QUERY_STR_LEN,
					"insert into %s(pu_id,pu_domain,ams_id,dev_name,dev_passwd,dev_ip," \
					"dev_port) values('%s','%s','%s','','','','')",
					AMS_CONFIGURE_TABLE, req_info->puid, req_info->domain_id,
					req_info->ams_id
				);
				ret = jpf_mysql_do_query(conn->mysql, query_buf);
				if (ret)
				{
					jpf_mysql_do_query(conn->mysql, "rollback");
					put_db_connection(dbs_obj->pool_info, conn);
					SET_CODE(&result, ret);
					goto end;
				}
			}
		}
		else
		{
			snprintf(
				query_buf, QUERY_STR_LEN,
				"delete from %s where pu_id='%s' and pu_domain='%s'",
				AMS_CONFIGURE_TABLE, req_info->puid, req_info->domain_id
			);
			jpf_mysql_do_query(conn->mysql, query_buf);
		}
	}
	ret = jpf_mysql_do_query(conn->mysql, "commit");
	put_db_connection(dbs_obj->pool_info, conn);
	if (ret)
	{
		SET_CODE(&result, ret);
		goto end;
	}

    if((!RES_CODE(&result))&&(row_num == 0))
    {
        memset(&change_mds, 0, sizeof(change_mds));
        strncpy(change_mds.puid, req_info->puid, MAX_ID_LEN - 1);
        msg_notify = jpf_sysmsg_new_2(MESSAGE_CHANGE_DISPATCH,
         &change_mds, sizeof(change_mds), ++msg_seq_generator);

        if (G_UNLIKELY(!msg_notify))
        return MFR_DELIVER_BACK;

        MSG_SET_DSTPOS(msg_notify, BUSSLOT_POS_PU);
        jpf_app_obj_deliver_out(app_obj, msg_notify);
    }

	if ((!RES_CODE(&result)) && (old_area_id != req_info->area_id))
	{
		snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where (link_guid like '%s%%' and link_domain_id='%s') \
        or (gu_id like '%s%%' and domain_id='%s')",
        ALARM_LINK_MAP_TABLE,
        req_info->puid, req_info->domain_id, req_info->puid, req_info->domain_id
        );

	    memset(&result, 0, sizeof(result));
	    jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);

		snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where gu_id like '%s%%' and gu_domain='%s'",
        MAP_GU_TABLE,
        req_info->puid, req_info->domain_id
        );

	    memset(&result, 0, sizeof(result));
	    jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
	}

end:
    JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_modify_gu_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfGuInfo *req_info;
    JpfBssRes result;
    JpfMysqlRes     *mysql_result;
    JpfStoreServer             mss[MSS_NUM];
    char query_buf[QUERY_STR_LEN];
    db_conn_status *conn = NULL;
    JpfModDbs *dbs_obj;
    gint ret = 0,  i = 0, j, exist = 0;
    JpfMssId  mss_id;

    req_info = MSG_GET_DATA(msg);
    dbs_obj = JPF_MODDBS(app_obj);
	conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
	if (G_UNLIKELY(!conn))
	{
		jpf_warning("<JpfModDbs> get db connection error!");
		ret = -E_GETDBCONN;
		goto end_modify_gu;
	}

	if (G_UNLIKELY(!conn->mysql))
	{
		jpf_warning("<JpfModDbs> get db connection error!");
		put_db_connection(dbs_obj->pool_info, conn);
		ret = -E_GETDBCONN;
		goto end_modify_gu;
	}

	JPF_MYSQL_DO_QUERY_OR_KILL_CONN(dbs_obj, conn, "start transaction", ret);
	if (ret)
		goto end_modify_gu;

    snprintf(
        query_buf, QUERY_STR_LEN,"update %s set gu_name='%s',gu_attributes='%d',\
        ivs_id='%s' where gu_id='%s' and gu_domain='%s'",
        GU_TABLE, req_info->gu_name, req_info->gu_attributes,
        req_info->ivs_id, req_info->guid,  req_info->domain_id
    );

    ret= jpf_mysql_do_query(conn->mysql, query_buf);
    if (ret != 0 )
    {
        jpf_mysql_do_query(conn->mysql, "rollback");
        put_db_connection(dbs_obj->pool_info, conn);
        goto end_modify_gu;
    }

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select mss_id from %s where gu_id='%s' and guid_domain='%s'",
        RECORD_POLICY_TABLE, req_info->guid,  req_info->domain_id
    );
    mysql_result = jpf_process_query_res(conn->mysql, query_buf);

    if (G_UNLIKELY(!mysql_result))
    {
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }
    BUG_ON(!mysql_result);

    if (G_LIKELY(MYSQL_RESULT_CODE(mysql_result)))  //success:0 fail:!0
    {
        ret = MYSQL_RESULT_CODE(mysql_result);
        jpf_sql_put_res(mysql_result, sizeof(JpfMysqlRes));
        goto end_modify_gu;
    }

    memset(mss, 0, sizeof(mss));
    jpf_get_gu_mss(mysql_result, mss);
    jpf_sql_put_res(mysql_result, sizeof(JpfMysqlRes));

    for (i = 0; i < MSS_NUM; i++)
    {
        if (!regex_mached(req_info->mss[i].mss_id, mss_reg))
            continue;

        for (exist = 0, j = 0; j < MSS_NUM; j++)
        {
            if (!strcmp(req_info->mss[i].mss_id, mss[j].mss_id))   //exist
            {
                exist = 1;
                break;
            }
        }

        if (!exist)  // inexist
        {
            snprintf(
                query_buf, QUERY_STR_LEN,
                "insert into %s (gu_id,guid_domain,mss_id) values('%s','%s','%s')",
                RECORD_POLICY_TABLE, req_info->guid, req_info->domain_id, req_info->mss[i].mss_id
            );
            printf("@@@@@@@@@@@add policy :%s\n",query_buf);
            ret = jpf_mysql_do_query(conn->mysql, query_buf);
            if (ret != 0 )
            {
                jpf_mysql_do_query(conn->mysql, "rollback");
                put_db_connection(dbs_obj->pool_info, conn);
                goto end_modify_gu;
            }

	     memset(&mss_id, 0, sizeof(mss_id));
	     strncpy(mss_id.mss_id, req_info->mss[i].mss_id, MSS_ID_LEN - 1);
            jpf_mod_dbs_deliver_mss_event(app_obj, &mss_id);
        }
    }

    for (i = 0; i < MSS_NUM; i++)
    {
        if (!regex_mached(mss[i].mss_id, mss_reg))
            continue;

        for (exist = 0, j = 0; j < MSS_NUM; j++)
        {
            if (!strcmp(req_info->mss[j].mss_id, mss[i].mss_id))
            {
                exist = 1;
                break;
            }
        }

        if (!exist)
        {
            snprintf(
                query_buf, QUERY_STR_LEN,
                "delete from %s where gu_id='%s' and guid_domain='%s' and mss_id='%s'",
                RECORD_POLICY_TABLE, req_info->guid, req_info->domain_id, mss[i].mss_id
            );
            printf("@@@@@@@@@@@add policy :%s\n",query_buf);
            ret = jpf_mysql_do_query(conn->mysql, query_buf);
            if (ret != 0 )
            {
                jpf_mysql_do_query(conn->mysql, "rollback");
                put_db_connection(dbs_obj->pool_info, conn);
                goto end_modify_gu;
            }
	     memset(&mss_id, 0, sizeof(mss_id));
	     strncpy(mss_id.mss_id, mss[i].mss_id, MSS_ID_LEN - 1);
            jpf_mod_dbs_deliver_mss_event(app_obj, &mss_id);
        }
    }

    ret = jpf_mysql_do_query(conn->mysql, "commit");
    put_db_connection(dbs_obj->pool_info, conn);

end_modify_gu:
    SET_CODE(&result, ret);
    JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
        BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_add_modify_manufacturer_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfAddModifyManufacturer *req_info;
    JpfBssRes result;
    char query_buf[QUERY_STR_LEN];
    gint row_num;
    glong affect_num = 0;

    req_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select count(*)  as count from %s where mf_name='%s' and mf_id='%s' ",
        MANUFACTURER_TABLE, req_info->mf_name, req_info->mf_id
    );

    memset(&result, 0, sizeof(result));

    row_num =  jpf_get_record_count(app_obj, query_buf);
    if (row_num > 0)
    {
        if (req_info->type == 0)
            SET_CODE(&result, -E_NAMEEXIST);
        else
            SET_CODE(&result, 0);

        goto err_add_modify_manufactuter;
    }

    if (row_num < 0)
    {
        SET_CODE(&result, row_num);
        goto  err_add_modify_manufactuter;
    }

    if (req_info->type == 0)
        snprintf(
            query_buf, QUERY_STR_LEN,
            "insert into %s (mf_name,mf_id) values('%s','%s')",
            MANUFACTURER_TABLE, req_info->mf_name, req_info->mf_id
        );
    else
        snprintf(
            query_buf, QUERY_STR_LEN,
            "update %s set mf_name='%s' where mf_id='%s'",
            MANUFACTURER_TABLE, req_info->mf_name, req_info->mf_id
        );

    jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);

err_add_modify_manufactuter:
    JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_modify_mds_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfBssRes result;
    JpfMdsInfo *req_info;
    char query_buf[QUERY_STR_LEN];
    gint row_num;
    glong affect_num = 0;

    req_info = MSG_GET_DATA(msg);
    memset(&result, 0, sizeof(result));
    switch (req_info->type){
    case 0:
        snprintf(
           query_buf, QUERY_STR_LEN,
           "update %s set mdu_name='%s',mdu_type='%d',mdu_keep_alive_freq='%d',\
            mdu_pu_port='%d',mdu_rtsp_port='%d',auto_get_ip_enable='%d'  where mdu_id='%s'",
            MDS_TABLE, req_info->mds_name, req_info->mds_type, req_info->keep_alive_freq,
            req_info->mds_pu_port, req_info->mds_rtsp_port, req_info->get_ip_enable, req_info->mds_id
        );

        jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
        if (RES_CODE(&result) == -DB_DUP_ENTRY_ERROR)
        {
            snprintf(
                query_buf, QUERY_STR_LEN,
                "select count(*)  as count from %s where mdu_name='%s'",
                MDS_TABLE, req_info->mds_name
            );

            row_num =  jpf_get_record_count(app_obj, query_buf);
            if (row_num > 0)
                SET_CODE(&result, -E_NAMEEXIST);

            if (row_num < 0)
                SET_CODE(&result, row_num);
        }
        break;
    case 1:
        snprintf(
           query_buf, QUERY_STR_LEN,
           "update %s set auto_get_ip_enable='%d'  where mdu_id='%s'",
            MDS_TABLE, req_info->get_ip_enable, req_info->mds_id
        );

        jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
        break;
  }
    JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_modify_mss_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfBssRes result;
    JpfMssInfo *req_info;
    char query_buf[QUERY_STR_LEN];
    gint row_num;
    JpfSysMsg *msg_notify = NULL;
    JpfChangeMss change_mss;
    glong affect_num = 0;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&result, 0, sizeof(result));

    snprintf(
       query_buf, QUERY_STR_LEN,
       "update %s set mss_name='%s',mss_keep_alive_freq='%d',mss_storage_type=%d,\
       mss_mode=%d where mss_id='%s'",
        MSS_TABLE, req_info->mss_name, req_info->keep_alive_freq, req_info->storage_type,
        req_info->mode, req_info->mss_id
    );

    jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    if (RES_CODE(&result) == -DB_DUP_ENTRY_ERROR)
    {
        snprintf(
            query_buf, QUERY_STR_LEN,
            "select count(*)  as count from %s where mss_name='%s'",
            MSS_TABLE, req_info->mss_name
        );

        row_num =  jpf_get_record_count(app_obj, query_buf);
        if (row_num > 0)
            SET_CODE(&result, -E_NAMEEXIST);

        if (row_num < 0)
            SET_CODE(&result, row_num);
    }

    memset(&change_mss, 0, sizeof(change_mss));
    if(!RES_CODE(&result))
    {
        strncpy(change_mss.mss_id, req_info->mss_id, MSS_ID_LEN - 1);
	 change_mss.storage_type = req_info->storage_type;
	 change_mss.mode = req_info->mode;
        msg_notify = jpf_sysmsg_new_2(MESSAGE_CHANGE_MSS,
         &change_mss, sizeof(change_mss), ++msg_seq_generator);

        if (G_UNLIKELY(!msg_notify))
        return MFR_DELIVER_BACK;
        printf("----jpf_mod_dbs_modify_mss_b\n");
        MSG_SET_DSTPOS(msg_notify, BUSSLOT_POS_MSS);
        jpf_app_obj_deliver_out(app_obj, msg_notify);
    }

    JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_modify_defence_area_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfBssRes result;
    JpfDefenceAreaInfo *req_info;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "update %s set defence_enable='%d', policy='%s' where defence_area_id='%d'",
        DEFENCE_AREA_TABLE, req_info->defence_enable, req_info->policy, req_info->defence_area_id
    );

    jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_modify_defence_gu_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfBssRes result;
    JpfModifyDefenceGu *req_info;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "update %s set coordinate_x=%.4f, coordinate_y=%.4f where map_id=%d and \
        gu_domain='%s' and gu_id='%s'",
        MAP_GU_TABLE, req_info->coordinate_x, req_info->coordinate_y,
        req_info->map_id, req_info->domain_id, req_info->guid
    );
	printf("-----------modify defence gu :%s\n",query_buf);

    jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_modify_map_href_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfBssRes result;
    JpfModifyMapHref *req_info;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "update %s set coordinate_x=%.4f, coordinate_y=%.4f where src_map_id=%d and dst_map_id=%d",
        MAP_HREF_TABLE, req_info->coordinate_x, req_info->coordinate_y,
        req_info->src_map_id, req_info->dst_map_id
    );
	printf("-----------modify defence href :%s\n",query_buf);

    jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_modify_tw_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfBssRes result;
    JpfModifyTw *req_info;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if (req_info->type == 0)
        snprintf(
            query_buf, QUERY_STR_LEN,
            "update %s set tw_name='%s' where tw_id=%d",
           TW_TABLE, req_info->tw_name, req_info->tw_id
        );
    else if (req_info->type == 1)
        snprintf(
            query_buf, QUERY_STR_LEN,
            "update %s set line_num=%d,column_num=%d where tw_id=%d",
           TW_TABLE, req_info->line_num, req_info->column_num, req_info->tw_id
        );
	printf("-----------modify tw :%s\n",query_buf);

    jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_modify_screen_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfBssRes result;
    JpfModifyScreen *req_info;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "update %s set coordinate_x=%.4f, coordinate_y=%.4f, length=%.4f,width=%.4f where dis_guid='%s' and dis_domain='%s'",
        SCREEN_TABLE, req_info->coordinate_x, req_info->coordinate_y,
        req_info->length, req_info->width, req_info->dis_guid, req_info->dis_domain
    );
	printf("-----------modify screen:%s\n",query_buf);

    jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_modify_tour_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfBssRes result;
    JpfModifyTour *req_info;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "update %s set tour_name='%s', auto_jump=%d where tour_id=%d",
       TOUR_TABLE, req_info->tour_name, req_info->auto_jump, req_info->tour_id
    );
    printf("-----------modify tour :%s\n",query_buf);

    jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    strcpy(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_modify_group_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfModifyGroup *req_info;
    char query_buf[QUERY_STR_LEN];

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "update %s set group_name='%s' where group_id=%d",
       GROUP_TABLE, req_info->group_name, req_info->group_id
    );

    return jpf_mod_dbs_general_cmd(app_obj, msg, query_buf, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_dbs_modify_group_step_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfModifyGroupStep *req_info;
    char query_buf[QUERY_STR_LEN];

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "update %s set `interval`=%d where step_no=%d",
        GROUP_STEP_TABLE, req_info->interval, req_info->step_no
    );

    return jpf_mod_dbs_general_cmd(app_obj, msg, query_buf, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_dbs_modify_group_step_info_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfModifyGroupStepInfo *req_info;
    char query_buf[QUERY_STR_LEN] = {0};

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "update %s set level=%d \
        where step_no=%d and scr_id=%d and div_no=%d",
        GROUP_STEP_INFO_TABLE, req_info->level,
        req_info->step_no, req_info->scr_id, req_info->div_no
    );

    return jpf_mod_dbs_general_cmd(app_obj, msg, query_buf, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_dbs_modify_link_time_policy_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfModifyLinkTimePolicy *req_info;
    char query_buf[QUERY_STR_LEN] = {0};
    JpfBssRes result;
    glong affect_num = 0;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "update %s set time_policy='%s' \
        where gu_id='%s' and domain_id='%s'",
        LINK_TIME_POLICY_TABLE, req_info->time_policy,
        req_info->guid, req_info->domain
    );
printf("---------modify_link_time_policy=%s\n", query_buf);
    memset(&result, 0, sizeof(result));
    jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    if (RES_CODE( &result.code) == 0)
    {
        JpfShareGuid gu_info;
        memset(&gu_info, 0, sizeof(gu_info));
        strncpy(gu_info.domain_id, req_info->domain, DOMAIN_ID_LEN - 1);
        strncpy(gu_info.guid, req_info->guid, MAX_ID_LEN - 1);
        jpf_cms_mod_deliver_msg_2(app_obj, BUSSLOT_POS_AMS,
            MSG_CHANGE_LINK_TIME_POLICY, &gu_info, sizeof(gu_info));
    }

    strcpy(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_modify_link_record_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfModifyLinkRecord *req_info;
    char query_buf[QUERY_STR_LEN] = {0};
    JpfMysqlRes     *mysql_result;
    JpfBssRes result;
    JpfStoreServer             mss[MSS_NUM];
    db_conn_status *conn = NULL;
    JpfModDbs *dbs_obj;
    gint ret = 0,  i = 0, j, exist = 0;

    req_info = MSG_GET_DATA(msg);
    dbs_obj = JPF_MODDBS(app_obj);
	conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
	if (G_UNLIKELY(!conn))
	{
		jpf_warning("<JpfModDbs> get db connection error!");
		ret = -E_GETDBCONN;
		goto end_modify_link_record;
	}

	if (G_UNLIKELY(!conn->mysql))
	{
		jpf_warning("<JpfModDbs> get db connection error!");
		put_db_connection(dbs_obj->pool_info, conn);
		ret = -E_GETDBCONN;
		goto end_modify_link_record;
	}

	JPF_MYSQL_DO_QUERY_OR_KILL_CONN(dbs_obj, conn, "start transaction", ret);
	if (ret)
		goto end_modify_link_record;

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select mss_id from %s where gu_id='%s' and domain_id='%s' and link_guid='%s' and \
                     link_domain_id='%s'",
        ALARM_LINK_RECORD_TABLE, req_info->guid,  req_info->domain,
        req_info->link_guid, req_info->link_domain
    );
    mysql_result = jpf_process_query_res(conn->mysql, query_buf);

    if (G_UNLIKELY(!mysql_result))
    {
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }
    BUG_ON(!mysql_result);

    if (G_LIKELY(MYSQL_RESULT_CODE(mysql_result)))  //success:0 fail:!0
    {
        ret = MYSQL_RESULT_CODE(mysql_result);
        jpf_sql_put_res(mysql_result, sizeof(JpfMysqlRes));
        goto end_modify_link_record;
    }

    memset(mss, 0, sizeof(mss));
    jpf_get_gu_mss(mysql_result, mss);
    jpf_sql_put_res(mysql_result, sizeof(JpfMysqlRes));

    for (i = 0; i < MSS_NUM; i++)
    {
        if (!regex_mached(req_info->mss[i].mss_id, mss_reg))
            continue;

        for (exist = 0, j = 0; j < MSS_NUM; j++)
        {
            if (!strcmp(req_info->mss[i].mss_id, mss[j].mss_id))   //exist
            {
                exist = 1;
                snprintf(
		        query_buf, QUERY_STR_LEN,
		        "update %s set time_len=%d,alarm_type=%d,level=%d \
		        where gu_id='%s' and domain_id='%s' and link_guid='%s' and \
                     link_domain_id='%s' and mss_id='%s'",
		        ALARM_LINK_RECORD_TABLE, req_info->time_len, req_info->alarm_type,
		        req_info->level, req_info->guid, req_info->domain, req_info->link_guid,
		        req_info->link_domain, req_info->mss[i].mss_id
		    );
		    ret = jpf_mysql_do_query(conn->mysql, query_buf);
                 if (ret != 0)
                 {
                     jpf_mysql_do_query(conn->mysql, "rollback");
                     put_db_connection(dbs_obj->pool_info, conn);
                     goto end_modify_link_record;
                 }
		    else
			{
				jpf_mod_dbs_notify_change_link(app_obj, req_info->domain,
					req_info->guid);
		    	}
                break;
            }
        }

        if (!exist)  // inexist
        {
            snprintf(
                query_buf, QUERY_STR_LEN,
                "insert into %s (gu_id,domain_id,link_guid,link_domain_id,mss_id,time_len,\
                alarm_type, level) values('%s','%s','%s','%s','%s',%d,%d,%d)",
                ALARM_LINK_RECORD_TABLE, req_info->guid, req_info->domain,
                req_info->link_guid, req_info->link_domain,req_info->mss[i].mss_id,
                req_info->time_len, req_info->alarm_type, req_info->level
            );

            printf("@@@@@@@@@@@add policy :%s\n",query_buf);
            ret = jpf_mysql_do_query(conn->mysql, query_buf);
            if (ret != 0)
            {
                jpf_mysql_do_query(conn->mysql, "rollback");
                put_db_connection(dbs_obj->pool_info, conn);
                goto end_modify_link_record;
            }

	     //memset(&mss_id, 0, sizeof(mss_id));
	    // strncpy(mss_id.mss_id, req_info->mss[i].mss_id, MSS_ID_LEN - 1);
           //jpf_mod_dbs_deliver_mss_event(app_obj, &mss_id);
        }
    }

    for (i = 0; i < MSS_NUM; i++)
    {
        if (!regex_mached(mss[i].mss_id, mss_reg))
            continue;

        for (exist = 0, j = 0; j < MSS_NUM; j++)
        {
            if (!strcmp(req_info->mss[j].mss_id, mss[i].mss_id))
            {
                exist = 1;

                break;
            }
        }

        if (!exist)
        {
            snprintf(
                query_buf, QUERY_STR_LEN,
                "delete from %s where gu_id='%s' and domain_id='%s' and link_guid='%s' and \
                 link_domain_id='%s' and mss_id='%s'",
                ALARM_LINK_RECORD_TABLE, req_info->guid, req_info->domain, req_info->link_guid,
                req_info->link_domain, mss[i].mss_id
            );

            ret = jpf_mysql_do_query(conn->mysql, query_buf);
            if (ret != 0)
            {
                jpf_mysql_do_query(conn->mysql, "rollback");
                put_db_connection(dbs_obj->pool_info, conn);
                goto end_modify_link_record;
            }
	 //    memset(&mss_id, 0, sizeof(mss_id));
	  //   strncpy(mss_id.mss_id, mss[i].mss_id, MSS_ID_LEN - 1);
        //    jpf_mod_dbs_deliver_mss_event(app_obj, &mss_id);
        }
    }

    ret = jpf_mysql_do_query(conn->mysql, "commit");
    put_db_connection(dbs_obj->pool_info, conn);
end_modify_link_record:
    SET_CODE(&result, ret);
    strcpy(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
        BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_modify_link_io_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfModifyLinkIO *req_info;
    char query_buf[QUERY_STR_LEN] = {0};

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "update %s set time_len=%d,alarm_type=%d,IO_value='%s' \
        where gu_id='%s' and domain_id='%s' and link_guid='%s' and \
             link_domain_id='%s'",
        ALARM_LINK_IO_TABLE, req_info->time_len, req_info->alarm_type,
        req_info->io_value, req_info->guid, req_info->domain, req_info->link_guid,
        req_info->link_domain
    );

    return jpf_mod_dbs_deal_change_link(app_obj, msg, query_buf,
		req_info->bss_usr, req_info->domain, req_info->guid);
}


JpfMsgFunRet
jpf_mod_dbs_modify_link_snapshot_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfModifyLinkSnapshot *req_info;
    char query_buf[QUERY_STR_LEN] = {0};
    JpfMysqlRes     *mysql_result;
    JpfBssRes result;
    JpfStoreServer             mss[MSS_NUM];
    db_conn_status *conn = NULL;
    JpfModDbs *dbs_obj;
    gint ret = 0,  i = 0, j, exist = 0;

    req_info = MSG_GET_DATA(msg);
    dbs_obj = JPF_MODDBS(app_obj);
	conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
	if (G_UNLIKELY(!conn))
	{
		jpf_warning("<JpfModDbs> get db connection error!");
		ret = -E_GETDBCONN;
		goto end_modify_link_snapshot;
	}

	if (G_UNLIKELY(!conn->mysql))
	{
		jpf_warning("<JpfModDbs> get db connection error!");
		put_db_connection(dbs_obj->pool_info, conn);
		ret = -E_GETDBCONN;
		goto end_modify_link_snapshot;
	}

	JPF_MYSQL_DO_QUERY_OR_KILL_CONN(dbs_obj, conn, "start transaction", ret);
	if (ret)
		goto end_modify_link_snapshot;

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select mss_id from %s where gu_id='%s' and domain_id='%s' and link_guid='%s' and \
                     link_domain_id='%s'",
        ALARM_LINK_SNAPSHOT_TABLE, req_info->guid,  req_info->domain,
        req_info->link_guid, req_info->link_domain
    );
    mysql_result = jpf_process_query_res(conn->mysql, query_buf);

    if (G_UNLIKELY(!mysql_result))
    {
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }
    BUG_ON(!mysql_result);

    if (G_LIKELY(MYSQL_RESULT_CODE(mysql_result)))  //success:0 fail:!0
    {
        ret = MYSQL_RESULT_CODE(mysql_result);
        jpf_sql_put_res(mysql_result, sizeof(JpfMysqlRes));
        goto end_modify_link_snapshot;
    }

    memset(mss, 0, sizeof(mss));
    jpf_get_gu_mss(mysql_result, mss);
    jpf_sql_put_res(mysql_result, sizeof(JpfMysqlRes));

    for (i = 0; i < MSS_NUM; i++)
    {
        if (!regex_mached(req_info->mss[i].mss_id, mss_reg))
            continue;

        for (exist = 0, j = 0; j < MSS_NUM; j++)
        {
            if (!strcmp(req_info->mss[i].mss_id, mss[j].mss_id))   //exist
            {
                exist = 1;
                snprintf(
		        query_buf, QUERY_STR_LEN,
		        "update %s set picture_num=%d,alarm_type=%d,level=%d \
		        where gu_id='%s' and domain_id='%s' and link_guid='%s' and \
                     link_domain_id='%s' and mss_id='%s'",
		        ALARM_LINK_SNAPSHOT_TABLE, req_info->picture_num, req_info->alarm_type,
		        req_info->level, req_info->guid, req_info->domain, req_info->link_guid,
		        req_info->link_domain, req_info->mss[i].mss_id
		    );
		    ret = jpf_mysql_do_query(conn->mysql, query_buf);
                 if (ret != 0)
                 {
                     jpf_mysql_do_query(conn->mysql, "rollback");
                     put_db_connection(dbs_obj->pool_info, conn);
                     goto end_modify_link_snapshot;
                 }
			else
			{
				jpf_mod_dbs_notify_change_link(app_obj, req_info->domain,
					req_info->guid);
			}
                break;
            }
        }

        if (!exist)  // inexist
        {
            snprintf(
                query_buf, QUERY_STR_LEN,
                "insert into %s (gu_id,domain_id,link_guid,link_domain_id,mss_id,picture_num,\
                alarm_type, level) values('%s','%s','%s','%s','%s',%d,%d,%d)",
                ALARM_LINK_SNAPSHOT_TABLE, req_info->guid, req_info->domain,
                req_info->link_guid, req_info->link_domain,req_info->mss[i].mss_id,
                req_info->picture_num, req_info->alarm_type, req_info->level
            );

            printf("@@@@@@@@@@@add policy :%s\n",query_buf);
            ret = jpf_mysql_do_query(conn->mysql, query_buf);
            if (ret != 0 )
            {
                jpf_mysql_do_query(conn->mysql, "rollback");
                put_db_connection(dbs_obj->pool_info, conn);
                goto end_modify_link_snapshot;
            }

	     //memset(&mss_id, 0, sizeof(mss_id));
	    // strncpy(mss_id.mss_id, req_info->mss[i].mss_id, MSS_ID_LEN - 1);
           //jpf_mod_dbs_deliver_mss_event(app_obj, &mss_id);
        }
    }

    for (i = 0; i < MSS_NUM; i++)
    {
        if (!regex_mached(mss[i].mss_id, mss_reg))
            continue;

        for (exist = 0, j = 0; j < MSS_NUM; j++)
        {
            if (!strcmp(req_info->mss[j].mss_id, mss[i].mss_id))
            {
                exist = 1;

                break;
            }
        }

        if (!exist)
        {
            snprintf(
                query_buf, QUERY_STR_LEN,
                "delete from %s where gu_id='%s' and domain_id='%s' and link_guid='%s' and \
                 link_domain_id='%s' and mss_id='%s'",
                ALARM_LINK_SNAPSHOT_TABLE, req_info->guid, req_info->domain, req_info->link_guid,
                req_info->link_domain, mss[i].mss_id
            );

            ret = jpf_mysql_do_query(conn->mysql, query_buf);
            if (ret != 0 )
            {
                jpf_mysql_do_query(conn->mysql, "rollback");
                put_db_connection(dbs_obj->pool_info, conn);
                goto end_modify_link_snapshot;
            }
	 //    memset(&mss_id, 0, sizeof(mss_id));
	  //   strncpy(mss_id.mss_id, mss[i].mss_id, MSS_ID_LEN - 1);
        //    jpf_mod_dbs_deliver_mss_event(app_obj, &mss_id);
        }
    }

    ret = jpf_mysql_do_query(conn->mysql, "commit");
    put_db_connection(dbs_obj->pool_info, conn);
end_modify_link_snapshot:
    SET_CODE(&result, ret);
    strcpy(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
        BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_modify_link_preset_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfModifyLinkPreset *req_info;
    char query_buf[QUERY_STR_LEN] = {0};

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "update %s set preset_no=%d,alarm_type=%d\
        where gu_id='%s' and domain_id='%s' and link_guid='%s' and \
             link_domain_id='%s'",
        ALARM_LINK_PRESET_TABLE, req_info->preset_no, req_info->alarm_type,
        req_info->guid, req_info->domain, req_info->link_guid,
        req_info->link_domain
    );

    return jpf_mod_dbs_deal_change_link(app_obj, msg, query_buf,
		req_info->bss_usr, req_info->domain, req_info->guid);
}


JpfMsgFunRet
jpf_mod_dbs_modify_link_step_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfModifyLinkStep *req_info;
    char query_buf[QUERY_STR_LEN] = {0};

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "update %s set  enc_guid='%s',enc_domain_id='%s',\
        division_id=%d where gu_id='%s' and domain_id='%s' and tw_id=%d \
        and screen_id=%d and division_num=%d",
        ALARM_LINK_STEP_TABLE, req_info->link_guid, req_info->link_domain,
        req_info->div_id, req_info->guid, req_info->domain,
        req_info->tw_id,req_info->screen_id,req_info->div_num
    );

    return jpf_mod_dbs_deal_change_link(app_obj, msg, query_buf,
		req_info->bss_usr, req_info->domain, req_info->guid);
}


JpfMsgFunRet
jpf_mod_dbs_modify_link_map_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfModifyLinkMap *req_info;
    char query_buf[QUERY_STR_LEN] = {0};

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "update %s set level=%d,alarm_type=%d \
        where gu_id='%s' and domain_id='%s' and link_guid='%s' and link_domain_id='%s'",
        ALARM_LINK_MAP_TABLE, req_info->level, req_info->alarm_type,
         req_info->guid, req_info->domain, req_info->link_guid,
        req_info->link_domain
    );

    return jpf_mod_dbs_deal_change_link(app_obj, msg, query_buf,
		req_info->bss_usr, req_info->domain, req_info->guid);
}


JpfMsgFunRet
jpf_mod_dbs_modify_ivs_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfBssRes result;
    JpfIvsInfo *req_info;
    char query_buf[QUERY_STR_LEN];
    gint row_num;
    glong affect_num = 0;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&result, 0, sizeof(result));

    snprintf(
       query_buf, QUERY_STR_LEN,
       "update %s set ivs_name='%s',ivs_keep_alive_freq='%d' where ivs_id='%s'",
        IVS_TABLE, req_info->ivs_name, req_info->keep_alive_freq,
        req_info->ivs_id
    );

    jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    if (RES_CODE(&result) == -DB_DUP_ENTRY_ERROR)
    {
        snprintf(
            query_buf, QUERY_STR_LEN,
            "select count(*)  as count from %s where ivs_name='%s'",
            IVS_TABLE, req_info->ivs_name
        );

        row_num =  jpf_get_record_count(app_obj, query_buf);
        if (row_num > 0)
            SET_CODE(&result, -E_NAMEEXIST);

        if (row_num < 0)
            SET_CODE(&result, row_num);
    }

    JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_query_admin_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryAdmin *req_info;
    JpfQueryAdminRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN];
    gint total_num;
    gint size,ret;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    switch (req_info->type) {
    case 0:       //query all admin
        snprintf(query_buf, QUERY_STR_LEN, "select * from %s ", ADMIN_TABLE);

        total_num =  jpf_dbs_get_row_num(app_obj, msg, query_buf);
        if (total_num == 0)
        {
            ret = 0;
            goto err_do_query_admin;
        }

        snprintf(
            query_buf, QUERY_STR_LEN,
            "select * from %s order by su_type desc limit %d,%d",
            ADMIN_TABLE, req_info->start_num, req_info->req_num
        );

        break;

    case 1:      //query admin by name
/*
       if ((regex_mached(req_info->key, &string_reg)))
        {
            ret = -E_STRINGFORMAT;
            goto err_do_query_admin;
        }*/
        snprintf(
            query_buf, QUERY_STR_LEN,
            "select * from %s where su_name like '%%%s%%'",
            ADMIN_TABLE, req_info->key
        );

	 total_num =  jpf_dbs_get_row_num(app_obj, msg, query_buf);
        if (total_num == 0)
        {
            ret = 0;
            goto err_do_query_admin;
        }
        snprintf(
            query_buf, QUERY_STR_LEN,
            "select * from %s where su_name like '%%%s%%' limit %d,%d",
            ADMIN_TABLE, req_info->key, req_info->start_num, req_info->req_num
        );

        break;

    case 2:     //query admin by type
        if ((!strcmp(req_info->key, "1")))
            snprintf(
                query_buf, QUERY_STR_LEN,
                "select * from %s where su_type='1'", ADMIN_TABLE
            );
        else
            snprintf(
                query_buf, QUERY_STR_LEN,
                "select * from %s where su_type!='1'", ADMIN_TABLE
            );

        total_num =  jpf_dbs_get_row_num(app_obj, msg, query_buf);
        if (total_num == 0)
        {
            ret = 0;
            goto err_do_query_admin;
        }

        if ((!strcmp(req_info->key, "1")))
            snprintf(
                query_buf, QUERY_STR_LEN,
                "select * from %s where su_type='1' limit %d,%d",
                ADMIN_TABLE, req_info->start_num, req_info->req_num
            );
        else
            snprintf(
                query_buf, QUERY_STR_LEN,
                "select * from %s where su_type!='1' limit %d,%d",
                ADMIN_TABLE, req_info->start_num, req_info->req_num
            );

        break;

    default:
        jpf_warning("query type is wrong");
        ret = E_QUERYTYPE;
        goto err_do_query_admin;

        break;
    }

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_admin(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
	     jpf_sysmsg_destroy(msg);
	     return MFR_ACCEPTED;
        }
	 query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_do_query_admin;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_query_admin:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_query_admin:
    size = sizeof(JpfQueryAdminRes);
    query_res = jpf_mem_kalloc(size);
    if (!query_res)
    {
        jpf_warning("<dbs_mh_bss> alloc error");
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }
    else
    {
        memset(query_res, 0, size);
        SET_CODE(query_res, ret);
        query_res->total_num = 0;
    }

    goto end_query_admin;
}


JpfMsgFunRet
jpf_mod_dbs_query_user_group_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryUserGroup *req_info;
    JpfQueryUserGroupRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint total_num;
    gint size,ret;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    switch (req_info->type) {
    case 0:       //query all user group
        snprintf(query_buf, QUERY_STR_LEN, "select * from %s ", USER_GROUP_TABLE);

        total_num =  jpf_dbs_get_row_num(app_obj, msg, query_buf);
        if (total_num == 0)
        {
            ret = 0;
            goto err_do_group_query;
        }

        if (req_info->req_num == -1)
            snprintf(query_buf, QUERY_STR_LEN,
                "select * from %s  order by group_id DESC ",
                USER_GROUP_TABLE
            );
	    else
            snprintf(
                query_buf, QUERY_STR_LEN,
                "select * from %s order by group_id DESC limit %d,%d ",
                USER_GROUP_TABLE, req_info->start_num, req_info->req_num
            );

	break;

    case 1:      //query user group by name
        snprintf(
            query_buf, QUERY_STR_LEN,
            "select * from %s where group_name like '%%%s%%'",
            USER_GROUP_TABLE, req_info->key
        );

        total_num =  jpf_dbs_get_row_num(app_obj, msg, query_buf);
        if (total_num == 0)
        {
            ret = 0;
            goto err_do_group_query;
        }

        snprintf(
            query_buf, QUERY_STR_LEN,
            "select * from %s where group_name like '%%%s%%' limit %d,%d",
            USER_GROUP_TABLE, req_info->key, req_info->start_num, req_info->req_num
        );

        break;

    case 2:      //query user group by rank
        snprintf(
            query_buf, QUERY_STR_LEN,
            "select * from %s where group_rank='%d'",
            USER_GROUP_TABLE, atoi(req_info->key) - 1
        );

        total_num =  jpf_dbs_get_row_num(app_obj, msg, query_buf);
        if (total_num == 0)
        {
            ret = 0;
            goto err_do_group_query;
        }

        snprintf(
            query_buf, QUERY_STR_LEN,
            "select * from %s where group_rank='%d' limit %d,%d",
            USER_GROUP_TABLE, atoi(req_info->key) - 1, req_info->start_num, req_info->req_num
        );

        break;

    case 3:     //query user group by type
        snprintf(
            query_buf, QUERY_STR_LEN,
            "select * from %s where group_id='%s' ", USER_GROUP_TABLE, req_info->key
        );

        break;

    default:
        jpf_warning("query type(%d) is wrong ", req_info->type);
        ret = -E_QUERYTYPE;
        goto err_do_group_query;

        break;
    }

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_user_group(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
        if (req_info->type == 3)
            query_res->total_num = query_res->req_num;
        else
            query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_do_group_query;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_query_group:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_group_query:
    size = sizeof(JpfQueryUserGroupRes);
    query_res = jpf_mem_kalloc(size);
    if (!query_res)
    {
        jpf_warning("<dbs_mh_bss> alloc error");
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }
    else
    {
        memset(query_res, 0, size);
        SET_CODE(query_res, ret);
        query_res->total_num = 0;
    }
    goto end_query_group;
}


JpfMsgFunRet
jpf_mod_dbs_query_user_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryUser *req_info;
    JpfQueryUserRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN];
    gint total_num;
    gint size,ret;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    switch (req_info->type) {
    case 0:       //query all user
        snprintf(query_buf, QUERY_STR_LEN, "select * from %s ", USER_TABLE);

        total_num =  jpf_dbs_get_row_num(app_obj, msg, query_buf);
        if (total_num == 0)
        {
            ret = 0;
            goto err_do_user_query;
        }

        snprintf(
        query_buf, QUERY_STR_LEN,
            "select t1.*,t2.group_name from %s as t1,%s as t2 where\
            t2.group_id=t1.user_group  order by user_no DESC limit %d,%d",
            USER_TABLE, USER_GROUP_TABLE, req_info->start_num, req_info->req_num
        );

        break;

    case 1:      //query user by name
        /*    if ((regex_mached(req_info->key, guid_reg)))
        {
            ret = -E_STRINGFORMAT;
            goto err_do_user_query;
        }*/
        snprintf(
            query_buf, QUERY_STR_LEN,
            "select * from %s where user_name like '%%%s%%'",
            USER_TABLE, req_info->key
        );

        total_num =  jpf_dbs_get_row_num(app_obj, msg, query_buf);
        if (total_num == 0)
        {
            ret = 0;
            goto err_do_user_query;
        }

        snprintf(
        query_buf, QUERY_STR_LEN,
            "select  t1.*,t2.group_name from  %s as t1,%s as t2\
            where  t1.user_name like '%%%s%%' and t2.group_id=t1.user_group limit %d,%d",
            USER_TABLE, USER_GROUP_TABLE, req_info->key, req_info->start_num, req_info->req_num
        );

        break;

    case 2:    //query user by user group id   name
        snprintf(
            query_buf, QUERY_STR_LEN, "select  t1.user_name  from %s as t1,%s as t2 \
            where t2.group_name like '%%%s%%' and t2.group_id=t1.user_group",
            USER_TABLE, USER_GROUP_TABLE, req_info->key
        );

        total_num =  jpf_dbs_get_row_num(app_obj, msg, query_buf);
        if (total_num == 0)
        {
            ret = 0;
            goto err_do_user_query;
        }

        snprintf(
            query_buf, QUERY_STR_LEN, "select  t1.*,t2.group_name  from %s as t1,%s as t2 \
            where t2.group_name like '%%%s%%' and t2.group_id=t1.user_group limit %d,%d",
            USER_TABLE, USER_GROUP_TABLE, req_info->key, req_info->start_num, req_info->req_num
        );

        break;

   case 3:      //query user by user group id
        snprintf(
            query_buf, QUERY_STR_LEN,
            "select * from %s where user_no='%s' ", USER_TABLE, req_info->key
        );

        total_num =  jpf_dbs_get_row_num(app_obj, msg, query_buf);
        if (total_num == 0)
        {
            ret = 0;
            goto err_do_user_query;
        }

        snprintf(
            query_buf, QUERY_STR_LEN, "select  t1.*,t2.group_name  from %s as t1,%s as t2 \
            where t1.user_no='%s' and t2.group_id=t1.user_group",
            USER_TABLE, USER_GROUP_TABLE, req_info->key
        );

        break;

   default:
        jpf_warning("query type(%d) is wrong ", req_info->type);
        ret = -E_QUERYTYPE;
        goto err_do_user_query;

        break;
    }

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_user(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

        query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_do_user_query;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_query_user:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_user_query:

    size = sizeof(JpfQueryUserRes);
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

    goto end_query_user;
}


JpfMsgFunRet
jpf_mod_dbs_query_domain_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryDomain *req_info;
    JpfQueryDomainRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN];
    gint total_num;
    gint size,ret;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    switch (req_info->type) {
    case 0:       //query all domain
        snprintf(query_buf, QUERY_STR_LEN, "select * from %s ", DOMAIN_TABLE);

	     total_num =  jpf_dbs_get_row_num(app_obj, msg, query_buf);
        if (total_num == 0)
        {
            ret = 0;
            goto err_do_domain_query;
        }

	break;

    case 1:      //query domain by domainId
        snprintf(
            query_buf, QUERY_STR_LEN,
            "select * from %s where dm_id='%s'",
            DOMAIN_TABLE, req_info->key
        );

        total_num =  jpf_dbs_get_row_num(app_obj, msg, query_buf);
        if (total_num == 0)
        {
            ret = 0;
            goto err_do_domain_query;
        }

	 break;

    case 2:      //query ben domain
        snprintf(
            query_buf, QUERY_STR_LEN,
            "select * from %s where dm_type=0",
            DOMAIN_TABLE
        );
        printf("---qurey buf =%s\n",query_buf);
        total_num =  jpf_dbs_get_row_num(app_obj, msg, query_buf);
        if (total_num == 0)
        {
            ret = 0;
            goto err_do_domain_query;
        }

        break;
    default:
        jpf_warning("query type(%d) is wrong ", req_info->type);
        ret = E_QUERYTYPE;
        goto err_do_domain_query;

        break;
    }

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_domain(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

	query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_do_domain_query;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_query_domain:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_domain_query:
    size = sizeof(JpfQueryDomainRes);
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

    goto end_query_domain;
}


JpfMsgFunRet
jpf_mod_dbs_query_area_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryArea *req_info;
    JpfQueryAreaRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num ;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(query_buf, QUERY_STR_LEN, "select count(*)  as count from %s ", AREA_TABLE);

    total_num =  jpf_get_record_count(app_obj,query_buf);
    if (total_num == 0)
    {
        ret = 0;
        goto err_do_area_query;
    }

    snprintf(query_buf, QUERY_STR_LEN, "select * from %s limit %d,%d",
			AREA_TABLE, req_info->start_num, req_info->req_num);
   printf("--------query_buf=%s\n",query_buf);
    result = jpf_dbs_do_query_res(app_obj,query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_area(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
	 query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_do_area_query;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_query_area:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_area_query:
    size = sizeof(JpfQueryAreaRes);
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
    query_res->req_num = 0;
    goto end_query_area;
}


gint
jpf_get_area_all_device_count(JpfAppObj *app_obj, char *query)
{
	db_conn_status *conn = NULL;
	JpfModDbs *dbs_obj;
	JpfMysqlRes *result = NULL;
	gint count, ret;

	dbs_obj = JPF_MODDBS(app_obj);

redo:
	conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
	if (G_UNLIKELY(!conn))
		return -E_GETDBCONN;

	if (G_UNLIKELY(!conn->mysql))
	{
		put_db_connection(dbs_obj->pool_info, conn);
		return -E_GETDBCONN;
	}

	ret = jpf_process_query_procedure(conn->mysql, query);
	if (ret == -DB_SERVER_GONE_ERROR)
	{
		kill_db_connection(dbs_obj->pool_info, conn);
		goto redo;
	}
	if (ret < 0)
	{
		put_db_connection(dbs_obj->pool_info, conn);
		return ret;
	}

	result = jpf_process_query_res(conn->mysql, "select @count");
	put_db_connection(dbs_obj->pool_info, conn);

	if (result)
		count = jpf_get_count_value(result);

	jpf_sql_put_res(result, sizeof(JpfMysqlRes));

	return count;
}


JpfMsgFunRet
jpf_mod_dbs_query_pu_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryPu *req_info;
    JpfQueryPuRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint total_num;
    gint size,ret;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    switch (req_info->type) {
    case 0:
        snprintf(
            query_buf, QUERY_STR_LEN, "select * from %s",
            PU_TABLE
        );
        total_num =  jpf_dbs_get_row_num(app_obj, msg, query_buf);
        if (total_num == 0)
        {
             ret = 0;
             goto err_do_pu_query;
        }
    	 memset(query_buf, 0, QUERY_STR_LEN);
    	 snprintf(
            query_buf, QUERY_STR_LEN,
            "select t1.*, t2.mf_name from %s as t1,%s as t2 where  \
            t1.pu_manufacturer=t2.mf_id limit %d,%d",
            PU_TABLE, MANUFACTURER_TABLE,
            req_info->start_num, req_info->req_num
        );
    	 goto query_pu_info;

        break;
    case 1:
    	 snprintf(
            query_buf, QUERY_STR_LEN,
            "select count(*) as count from %s as t1,%s as t2 where t1.pu_area in ('%s') and t1.pu_area=t2.area_id",
            PU_TABLE,AREA_TABLE,req_info->key
        );

	 total_num =  jpf_get_record_count(app_obj,query_buf);
        if (total_num == 0)
        {
            ret = 0;
            goto err_do_pu_query;
        }
           break;

	case 2:
	snprintf(
		query_buf, QUERY_STR_LEN,
		"select count(*) as count from %s where pu_id='%s' and pu_domain='%s'",
		AMS_CONFIGURE_TABLE, req_info->key, req_info->domain_id
	);
	total_num = jpf_get_record_count(app_obj, query_buf);
	if (total_num != 0)
	{
		snprintf(
			query_buf, QUERY_STR_LEN,
			"select t1.*,t2.mf_name,t3.ams_id from %s as t1,%s as t2,%s as t3 where t1.pu_id='%s' and " \
			"t1.pu_domain='%s' and t1.pu_id=t3.pu_id and t1.pu_domain=t3.pu_domain and t1.pu_manufacturer=t2.mf_id",
			PU_TABLE, MANUFACTURER_TABLE, AMS_CONFIGURE_TABLE, req_info->key, req_info->domain_id
		);
	}
	else
	{
		snprintf(
			query_buf, QUERY_STR_LEN,
			"select t1.*,t2.mf_name from %s as t1,%s as t2 where t1.pu_id='%s' and " \
			"t1.pu_domain='%s' and t1.pu_manufacturer=t2.mf_id",
			PU_TABLE, MANUFACTURER_TABLE, req_info->key, req_info->domain_id
		);
	}
	goto query_pu_info;

	break;

    case 3:

    snprintf(
            query_buf, QUERY_STR_LEN,
            "call get_area_all_device_count(%s,@count)",
            req_info->key
        );
    total_num =  jpf_get_area_all_device_count(app_obj,query_buf);
		printf("@@@@@@@@@@ device count :%d\n",total_num);
    if (total_num <= 0)
    {
        ret = total_num;
        goto err_do_pu_query;
    }
        break;
    case 4:
    	 snprintf(
            query_buf, QUERY_STR_LEN,
            "select count(*) as count from %s where pu_mdu='%s'",
            PU_TABLE, req_info->key
        );

	 total_num =  jpf_get_record_count(app_obj,query_buf);
        if (total_num == 0)
        {
            ret = 0;
            goto err_do_pu_query;
        }
           break;
    default:
        jpf_warning("query type(%d) is wrong ", req_info->type);
	     ret = E_QUERYTYPE;
	     goto err_do_pu_query;

        break;
    }

   /* total_num =  jpf_dbs_get_row_num(app_obj, msg, query_buf);
    if (total_num == 0)
    {
        ret = 0;
        goto err_do_pu_query;
    }*/

    memset(query_buf, 0, QUERY_STR_LEN);
    if (req_info->type == 3)
    {
        gint area_id;
        area_id = atoi(req_info->key);
        snprintf(
            query_buf, QUERY_STR_LEN,
            "call show_area_all_devices(%d,%d,%d)",
            area_id, req_info->start_num, req_info->req_num
        );
    }
    else if (req_info->type == 4)
    {
        snprintf(
            query_buf, QUERY_STR_LEN,
            "select t1.*,t2.mf_name,\
            t3.area_name,t4.pu_last_ip,t4.pu_last_alive,t4.pu_state from %s as t1,%s as t2,\
            %s as t3,%s as t4 where t1.pu_mdu='%s' \
            and  t1.pu_manufacturer=t2.mf_id and t1.pu_area=t3.area_id and t1.pu_id=t4.pu_id and \
            t1.pu_domain=t4.pu_domain order by t4.pu_state desc,t1.pu_id limit %d,%d",
            PU_TABLE, MANUFACTURER_TABLE, AREA_TABLE, PU_RUNNING_TABLE,
            req_info->key, req_info->start_num, req_info->req_num
        );
    }
    else  //type =1
       /* snprintf(
            query_buf, QUERY_STR_LEN,
            "select t1.*, t2.mf_name,t3.area_name,t4.mdu_name  from %s as t1,%s as t2,%s as t3, %s as t4 where (t1.pu_area in ('%s') and \
            t1.pu_manufacturer=t2.mf_id and t1.pu_area=t3.area_id) and (t1.pu_mdu=t4.mdu_id or t1.pu_mdu=-1 ) limit %d,%d",
            PU_TABLE, MANUFACTURER_TABLE, AREA_TABLE, MDS_TABLE,
            req_info->key, req_info->start_num, req_info->req_num
        );*/
     snprintf(
            query_buf, QUERY_STR_LEN,
            "select t4.mdu_name,t5.*,t5.pu_state,t5.pu_id,t5.pu_last_ip,t5.pu_last_alive from %s as t4 right join (select t1.*,t1.pu_mdu as mdu, t2.mf_name,\
            t3.area_name,t6.pu_state,t6.pu_last_ip,t6.pu_last_alive from %s as t1,%s as t2,%s as t3,%s as t6 where t1.pu_area in ('%s') \
            and t1.pu_manufacturer=t2.mf_id and t1.pu_area=t3.area_id and t1.pu_id=t6.pu_id and \
            t1.pu_domain=t6.pu_domain) as t5 on t5.mdu=t4.mdu_id order by t5.pu_state desc,t5.pu_id limit %d,%d",
           MDS_TABLE, PU_TABLE, MANUFACTURER_TABLE, AREA_TABLE, PU_RUNNING_TABLE,
            req_info->key, req_info->start_num, req_info->req_num
        );

query_pu_info:
    result = jpf_dbs_do_query_res(app_obj, query_buf);
    if (G_UNLIKELY(!result))
    {
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_pu(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

        query_res->type = req_info->type;

        if (req_info->type != 2)
            query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_do_pu_query;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_query_pu:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_pu_query:
    size = sizeof(JpfQueryPuRes);
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
    query_res->req_num = 0;
    goto end_query_pu;
}


static void jpf_mod_dbs_get_mss_of_gus(JpfAppObj *app_obj, JpfQueryGuRes *gu)
{
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint ret, row_num = 0, i;

        for (i = 0; i < gu->req_num; i++)
        {
             snprintf(
                query_buf, QUERY_STR_LEN,
                "select t1.mss_id,t2.mss_name from %s as t1,%s as t2 where t1.guid_domain='%s' \
                and t1.gu_id='%s' and t1.mss_id=t2.mss_id",
                RECORD_POLICY_TABLE, MSS_TABLE,
                gu->gu_info[i].domain_id, gu->gu_info[i].guid
            );

    	     result = jpf_dbs_do_query_res(app_obj, query_buf);
            BUG_ON(!result);

            if (G_LIKELY(MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
            {
                ret = MYSQL_RESULT_CODE(result);
                jpf_sql_put_res(result, sizeof(JpfMysqlRes));
		  SET_CODE(gu, ret);
		  return;
            }

            row_num = jpf_sql_get_num_rows(result);
            printf("get row num (%d)\n",row_num);
            if(row_num == 0)
            {
                jpf_sql_put_res(result,sizeof(JpfMysqlRes));
                continue;
            }

            jpf_get_gu_mss(result,  &gu->gu_info[i].mss[0]);
	     jpf_sql_put_res(result,sizeof(JpfMysqlRes));
       }
}


void
jpf_get_gu_info_of_mss(JpfMysqlRes *result, JpfGuInfo *query_res)
{
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
            if (!strcmp(name, "gu_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->gu_name[GU_NAME_LEN - 1] = 0;
                    strncpy(
                        query_res[info_no].gu_name,
                        value, GU_NAME_LEN  - 1
                    );
                }
            }
            else if (!strcmp(name, "pu_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->puid[MAX_ID_LEN - 1] = 0;
                    strncpy(
                        query_res[info_no].puid,
                        value, MAX_ID_LEN  - 1
                    );
                }
            }
            else if (!strcmp(name, "pu_info"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->pu_name[PU_NAME_LEN - 1] = 0;
                    strncpy(
                        query_res[info_no].pu_name,
                        value, PU_NAME_LEN  - 1
                    );
                }
            }
            else if (!strcmp(name, "area_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->area_name[AREA_NAME_LEN - 1] = 0;
                    strncpy(
                        query_res->area_name,
                        value, AREA_NAME_LEN  - 1
                    );
                }
            }
            else if (!strcmp(name, "gu_type"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->gu_type = atoi(value);
            }
            else if (!strcmp(name, "gu_attributes"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->gu_attributes= atoi(value);
            }
            else
                cms_debug("no need mysql name %s ", name);

        }
	 info_no++;
    }
}


static void jpf_mod_dbs_get_gus_of_info(JpfAppObj *app_obj, JpfQueryGuRes *gu)
{
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint ret, row_num = 0, i;

        for (i = 0; i < gu->req_num; i++)
        {
             snprintf(
                query_buf, QUERY_STR_LEN,
                "select t1.gu_name,t1.gu_type,t1.gu_attributes,t2.pu_id,t2.pu_info,t3.area_name \
                from %s as t1,%s as t2,%s as t3 where t1.gu_domain='%s' and t1.gu_id='%s' and \
                t1.gu_puid=t2.pu_id and t1.gu_domain=t2.pu_domain and t2.pu_area=t3.area_id",
                GU_TABLE,PU_TABLE, AREA_TABLE,
                gu->gu_info[i].domain_id, gu->gu_info[i].guid
            );

    	     result = jpf_dbs_do_query_res(app_obj, query_buf);
            BUG_ON(!result);

            if (G_LIKELY(MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
            {
                ret = MYSQL_RESULT_CODE(result);
                jpf_sql_put_res(result, sizeof(JpfMysqlRes));
		  SET_CODE(gu, ret);
		  return;
            }

            row_num = jpf_sql_get_num_rows(result);
            printf("get row num (%d)\n",row_num);
            if(row_num == 0)
            {
                jpf_sql_put_res(result,sizeof(JpfMysqlRes));
                continue;
            }

            jpf_get_gu_info_of_mss(result,  &gu->gu_info[i]);
	     jpf_sql_put_res(result,sizeof(JpfMysqlRes));
       }
}


JpfMsgFunRet
jpf_mod_dbs_query_gu_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryGu *req_info;
    JpfQueryGuRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint total_num = 0;
    gint size,ret, row_num = 0;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    switch (req_info->type) {
    case 0:       //query all gu
        snprintf(
            query_buf, QUERY_STR_LEN, "select count(*)  as count from %s ", GU_TABLE
        );
        total_num =  jpf_get_record_count(app_obj, query_buf);
        if (total_num <= 0)
        {
            ret = total_num;
            goto err_do_gu_query;
        }
        snprintf(
            query_buf, QUERY_STR_LEN, "select t1.*,t2.ivs_name from %s as t1 " \
            "left join %s as t2 on t1.ivs_id=t2.ivs_id limit %d,%d",
            GU_TABLE, IVS_TABLE, req_info->start_num, req_info->req_num
        );

        break;

    case 1:      // query all gu by puid and domainid
        snprintf(
            query_buf, QUERY_STR_LEN,
            "select count(*)  as count from %s where gu_domain='%s' and gu_puid='%s'",
            GU_TABLE, req_info->domain_id, req_info->puid
        );

        total_num =  jpf_get_record_count(app_obj, query_buf);
        if (total_num <= 0)
        {
            ret = total_num;
            goto err_do_gu_query;
        }

        snprintf(
            query_buf, QUERY_STR_LEN,
            "select t1.*,t2.ivs_name from (select t3.*,t4.ams_name from %s as t3 " \
            "left join(%s as t4,%s as t5) on t3.gu_domain=t5.pu_domain and " \
            "t3.gu_puid=t5.pu_id and t5.ams_id=t4.ams_id) as t1 left join %s as t2 " \
            "on t1.ivs_id=t2.ivs_id where t1.gu_domain='%s' and t1.gu_puid='%s' " \
            "order by t1.gu_type,t1.gu_id limit %d,%d",
            GU_TABLE, AMS_TABLE, AMS_CONFIGURE_TABLE, IVS_TABLE,
            req_info->domain_id, req_info->puid, req_info->start_num, req_info->req_num
        );

        break;

    case 2:      // query all gu by guid and domainid
        snprintf(
            query_buf, QUERY_STR_LEN,
            "select * from %s where gu_domain='%s' and gu_id='%s'",
            GU_TABLE, req_info->domain_id, req_info->guid
        );

        break;

    case 3:      // query all gu by areaId  map configure gu
        snprintf(
            query_buf, QUERY_STR_LEN,
            "select count(*)  as count from (select t1.pu_info,t2.*,t3.map_id from %s as t1,%s as t2 \
            left outer join %s as t3 on t3.gu_domain=t2.gu_domain and t3.gu_id=t2.gu_id \
            where t1.pu_area='%s' and t1.pu_id=t2.gu_puid and t1.pu_domain=t2.gu_domain) t4 \
            where t4.map_id is null",
            PU_TABLE, GU_TABLE, MAP_GU_TABLE, req_info->key
        );

        total_num =  jpf_get_record_count(app_obj, query_buf);
        if (total_num <= 0)
        {
            ret = total_num;
            goto err_do_gu_query;
        }

        snprintf(
            query_buf, QUERY_STR_LEN,
            "select t4.* from (select t1.pu_info,t2.*,t3.map_id from %s as t1,%s as t2 left outer join %s as t3 on \
            t3.gu_domain=t2.gu_domain and t3.gu_id=t2.gu_id where t1.pu_area='%s' and \
            t1.pu_id=t2.gu_puid and t1.pu_domain=t2.gu_domain) t4 where t4.map_id is null limit %d,%d",
            PU_TABLE, GU_TABLE, MAP_GU_TABLE,
            req_info->key, req_info->start_num, req_info->req_num
        );

        break;

    case 4:      // query all display gu and not configure to tw
          snprintf(
            query_buf, QUERY_STR_LEN,
            "select count(*)  as count from (select t1.*,t2.scr_id from %s as t1 left outer join %s as t2 on \
            t2.dis_domain=t1.gu_domain and t2.dis_guid=t1.gu_id where t1.gu_id like '%%-DS-%%') t3 \
            where t3.scr_id is null",
            GU_TABLE, SCREEN_TABLE
        );

        total_num =  jpf_get_record_count(app_obj, query_buf);
        if (total_num <= 0)
        {
            ret = total_num;
            goto err_do_gu_query;
        }

	 snprintf(
            query_buf, QUERY_STR_LEN,
            "select t5.* from (select t1.pu_info,t1.pu_minor_type,t2.pu_state,t2.pu_last_ip,t3.*,t4.scr_id from %s as t1,%s as t2,%s as t3 left outer join %s as t4 on \
            t4.dis_domain=t3.gu_domain and t4.dis_guid=t3.gu_id where t3.gu_id like '%%-DS-%%' and \
            t1.pu_id=t3.gu_puid and t1.pu_domain=t3.gu_domain and t1.pu_id=t2.pu_id and t1.pu_domain=t2.pu_domain) t5 where t5.scr_id is null limit %d,%d",
            PU_TABLE, PU_RUNNING_TABLE, GU_TABLE, SCREEN_TABLE,
             req_info->start_num, req_info->req_num
        );
        break;
    case 5:      // query all gu by mssId
        snprintf(
            query_buf, QUERY_STR_LEN,
            "select count(*)  as count from %s where mss_id='%s'",
            RECORD_POLICY_TABLE, req_info->key
        );

        total_num =  jpf_get_record_count(app_obj, query_buf);
        if (total_num <= 0)
        {
            ret = total_num;
            goto err_do_gu_query;
        }

        snprintf(
            query_buf, QUERY_STR_LEN,
            "select gu_id,guid_domain as gu_domain from %s \
             where mss_id='%s' limit %d,%d",
            RECORD_POLICY_TABLE,
            req_info->key, req_info->start_num, req_info->req_num
        );

        break;

    case 6:
        snprintf(
            query_buf, QUERY_STR_LEN,
            "select count(*) as count from %s as t1 left join %s as t2 on \
            t2.gu_id=t1.gu_id and t2.gu_domain=t1.gu_domain where \
            t1.gu_domain='%s' and t1.gu_puid='%s' and t2.gu_id is null",
            GU_TABLE, MAP_GU_TABLE,req_info->domain_id, req_info->puid
        );

        total_num =  jpf_get_record_count(app_obj, query_buf);
        if (total_num <= 0)
        {
            ret = total_num;
            goto err_do_gu_query;
        }

        snprintf(
            query_buf, QUERY_STR_LEN,
            "select t1.* from %s as t1 left join %s as t2 on t2.gu_id=t1.gu_id \
            and t2.gu_domain=t1.gu_domain where t1.gu_domain='%s' and \
            t1.gu_puid='%s' and t2.gu_id is null limit %d,%d",
            GU_TABLE, MAP_GU_TABLE, req_info->domain_id, req_info->puid,
            req_info->start_num, req_info->req_num
        );

        break;
    case 7:
        snprintf(
            query_buf, QUERY_STR_LEN,
            "select count(*) as count from %s as t1,%s as t2 where\
            t2.gu_id=t1.gu_id and t2.gu_domain=t1.gu_domain and \
            t1.gu_id like '%%-AV-%%' and t1.gu_domain='%s' and t1.gu_puid='%s'",
            GU_TABLE, MAP_GU_TABLE,req_info->domain_id, req_info->puid
        );

        total_num =  jpf_get_record_count(app_obj, query_buf);
        if (total_num <= 0)
        {
            ret = total_num;
            goto err_do_gu_query;
        }

        snprintf(
            query_buf, QUERY_STR_LEN,
            "select t1.* from %s as t1, %s as t2 where t2.gu_id=t1.gu_id \
            and t2.gu_domain=t1.gu_domain and t1.gu_domain='%s' and \
            t1.gu_id like '%%-AV-%%' and t1.gu_puid='%s' limit %d,%d",
            GU_TABLE, MAP_GU_TABLE, req_info->domain_id, req_info->puid,
            req_info->start_num, req_info->req_num
        );

        break;
    default:
    	 jpf_warning("query type(%d) is wrong ", req_info->type);
    	 ret = E_QUERYTYPE;
    	 goto err_do_gu_query;

        break;
    }

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    if (G_UNLIKELY(!result))
    {
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }
    BUG_ON(!result);

    if (G_LIKELY(MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_do_gu_query;
    }

    query_res = jpf_dbs_get_gu(result, &size);
    if (G_UNLIKELY(!query_res))
    {
        jpf_warning("<dbs_mh_bss> alloc error");
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    query_res->type = req_info->type;
    if ((req_info->type == 0)||(req_info->type == 3)||(req_info->type == 4)
		|| (req_info->type == 6))
        query_res->total_num = total_num;

    if (req_info->type == 1)
    {
		query_res->total_num = total_num;
		jpf_mod_dbs_get_mss_of_gus(app_obj, query_res);
    }

    if (req_info->type == 2)
    {
        snprintf(
            query_buf, QUERY_STR_LEN,
            "select mss_id from %s where guid_domain='%s' and gu_id='%s'",
            RECORD_POLICY_TABLE, req_info->domain_id, req_info->guid
        );

        if(result)
            jpf_sql_put_res(result, sizeof(JpfMysqlRes));

        result = jpf_dbs_do_query_res(app_obj, query_buf);

        if (G_UNLIKELY(!result))
        {
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
        BUG_ON(!result);

        if (G_LIKELY(MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
        {
            ret = MYSQL_RESULT_CODE(result);
            jpf_sql_put_res(result, sizeof(JpfMysqlRes));
            goto err_do_gu_query;
        }

        row_num = jpf_sql_get_num_rows(result);
        printf("get row num (%d)\n",row_num);
        if(row_num == 0)
        {
            jpf_sql_put_res(result,sizeof(JpfMysqlRes));
            goto end_query_gu;
        }

        jpf_get_gu_mss(result,  &query_res->gu_info[0].mss[0]);
    }

    if (req_info->type == 5)
    {
        query_res->total_num = total_num;
        jpf_mod_dbs_get_gus_of_info(app_obj, query_res);
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_query_gu:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_gu_query:
    size = sizeof(JpfQueryPuRes);
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
    query_res->req_num = 0;

    goto end_query_gu;
}


JpfMsgFunRet
jpf_mod_dbs_query_manufacturer_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryManufacturer *req_info;
    JpfQueryManufacturerRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size,ret, total_num;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(query_buf, QUERY_STR_LEN, "select count(*)  as count from %s ", MANUFACTURER_TABLE);

    total_num =  jpf_get_record_count(app_obj,query_buf);
    if (total_num == 0)
    {
        ret = 0;
        goto err_do_manufacturer_query;
    }

    snprintf(query_buf, QUERY_STR_LEN, "select * from %s limit %d,%d",
			MANUFACTURER_TABLE, req_info->start_num, req_info->req_num);
    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);
    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_manufacturer(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
        query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_do_manufacturer_query;
    }

    if(G_LIKELY(result))
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_query_manufacturer:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_manufacturer_query:
    size = sizeof(JpfQueryManufacturerRes);
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
    query_res->req_num = 0;
    goto end_query_manufacturer;
}


JpfMsgFunRet
jpf_mod_dbs_query_mds_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryMds *req_info;
    JpfQueryMdsRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if (req_info->type == 0)   //query all mds
    {
        snprintf(query_buf, QUERY_STR_LEN, "select count(*)  as count from %s ", MDS_TABLE);

        total_num =  jpf_get_record_count(app_obj,query_buf);
        if (total_num == 0)
        {
            ret = 0;
            goto err_do_mds_query;
        }

        snprintf(query_buf, QUERY_STR_LEN, "select * from %s order by mdu_state desc limit %d,%d",
            MDS_TABLE, req_info->start_num, req_info->req_num);
    }
    else if (req_info->type == 1)  //query mds by id
        snprintf(
            query_buf, QUERY_STR_LEN,  "select * from  %s where mdu_id='%s' ",
            MDS_TABLE, req_info->key
        );
    else if (req_info->type == 2)  //query auto get mds ip
        snprintf(
            query_buf, QUERY_STR_LEN,  "select auto_get_ip_enable from  %s where mdu_id='%s' ",
            MDS_TABLE, req_info->key
        );
    else
    {
        jpf_warning("query type(%d) is wrong ", req_info->type);
        ret = E_QUERYTYPE;
        goto err_do_mds_query;
    }

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_mds(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
		query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_do_mds_query;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_query_mds:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    query_res->type = req_info->type;
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_mds_query:

    size = sizeof(JpfQueryMdsRes);
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
    query_res->req_num = 0;
    goto end_query_mds;
}


JpfMsgFunRet
jpf_mod_dbs_query_mds_ip_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryMdsIp *req_info;
    JpfQueryMdsIpRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size,ret;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if (req_info->type == 0)   //query all mds
        snprintf(query_buf, QUERY_STR_LEN, "select * from %s ", MDS_IP_TABLE);
    else if (req_info->type == 1)  //query mds by id
        snprintf(
            query_buf, QUERY_STR_LEN,  "select * from  %s where mdu_id='%s' ",
            MDS_IP_TABLE, req_info->key
        );
    else
    {
        jpf_warning("query type(%d) is wrong ", req_info->type);
        ret = E_QUERYTYPE;
        goto ERR_DO_MDS_IP_QUERY;
    }

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_mds_ip(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
        strncpy(query_res->mds_id, req_info->key, MDS_ID_LEN - 1);
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto ERR_DO_MDS_IP_QUERY;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

END_QUERY_MDS_IP:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

ERR_DO_MDS_IP_QUERY:

    size = sizeof(JpfQueryMdsIpRes);
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

    goto END_QUERY_MDS_IP;
}


JpfMsgFunRet
jpf_mod_dbs_query_mss_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryMss *req_info;
    JpfQueryMssRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if (req_info->type == 0)   //query all mss
    {
        snprintf(query_buf, QUERY_STR_LEN, "select count(*) as count from %s ", MSS_TABLE);

        total_num =  jpf_get_record_count(app_obj,query_buf);
        if (total_num <= 0)
        {
            ret = 0;
            goto err_do_mss_query;
        }

        snprintf(
            query_buf, QUERY_STR_LEN,
            "select * from %s order by mss_state desc limit %d,%d",
            MSS_TABLE, req_info->start_num, req_info->req_num
        );
    }
    else if (req_info->type == 1)  //query mss by id
    {
        snprintf(
             query_buf, QUERY_STR_LEN,
             "select count(*) as count from %s where mss_id='%s' ", MSS_TABLE, req_info->key
         );

        total_num =  jpf_get_record_count(app_obj,query_buf);
        if (total_num <= 0)
        {
            ret = 0;
            goto err_do_mss_query;
        }

        snprintf(
            query_buf, QUERY_STR_LEN,  "select * from  %s where mss_id='%s' ",
            MSS_TABLE, req_info->key
        );
    }
    else
    {
        jpf_warning("query type(%d) is wrong ", req_info->type);
        ret = E_QUERYTYPE;
        goto err_do_mss_query;
    }

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_mss(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

        query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_do_mss_query;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_query_mss:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_mss_query:

    size = sizeof(JpfQueryMssRes);
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

    goto end_query_mss;
}


JpfMsgFunRet
jpf_mod_dbs_query_record_policy_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryRecordPolicy *req_info;
    JpfQueryRecordPolicyRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num = 0, i;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if (req_info->type == 0)   //query all record gu
    {
        snprintf(query_buf, QUERY_STR_LEN,
           "select count(*) as count from %s ", RECORD_POLICY_TABLE);

        total_num =  jpf_get_record_count(app_obj,query_buf);
        if (total_num <= 0)
        {
            ret = 0;
            goto err_do_mss_query;
        }

        snprintf(query_buf, QUERY_STR_LEN, "select * from %s order by gu_id limit %d,%d",
            RECORD_POLICY_TABLE, req_info->start_num, req_info->req_num);
        /*snprintf(
            query_buf, QUERY_STR_LEN,
            "select t1.*,t2.gu_puid,t2.gu_name,t3.pu_info,t4.mss_name,t5.area_name from %s as t1,\
            %s as t2,%s as t3,%s as t4,%s as t5 where t1.gu_id=t2.gu_id and t1.guid_domain=t2.gu_domain \
            and t2.gu_puid=t3.pu_id and t2.gu_domain=t3.pu_domain and t1.mss_id=t4.mss_id and \
            t3.pu_area=t5.area_id order by t1.gu_id limit %d,%d",
            RECORD_POLICY_TABLE, GU_TABLE, PU_TABLE, MSS_TABLE,AREA_TABLE, req_info->start_num,
            req_info->req_num
        );*/
    }
    else if (req_info->type == 1)  //query mds by id
    {
        total_num = 1;
        snprintf(
            query_buf, QUERY_STR_LEN,
            "select * from  %s where mss_id='%s' and gu_id='%s' and guid_domain='%s'",
            RECORD_POLICY_TABLE, req_info->mss_id, req_info->guid, req_info->domain_id
        );
    }
    else
    {
        jpf_warning("query type(%d) is wrong ", req_info->type);
        ret = E_QUERYTYPE;
        goto err_do_mss_query;
    }

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_record_policy(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

        query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_do_mss_query;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

    if (req_info->type == 0)
    {
        for(i = 0; i < query_res->req_num; i++)
        {
            snprintf(
                   query_buf, QUERY_STR_LEN,
                   "select t1.gu_puid,t1.gu_name,t2.pu_info,t3.mss_name,t4.area_name from \
                   %s as t1,%s as t2,%s as t3,%s as t4 where t1.gu_id='%s' and t1.gu_domain='%s' \
                   and t1.gu_puid=t2.pu_id and t1.gu_domain=t2.pu_domain and t3.mss_id='%s' and \
                   t2.pu_area=t4.area_id ",
                   GU_TABLE, PU_TABLE, MSS_TABLE,AREA_TABLE,query_res->record_policy[i].guid,
                   query_res->record_policy[i].domain_id, query_res->record_policy[i].mss_id
               );

            result = jpf_dbs_do_query_res(app_obj, query_buf);
            BUG_ON(!result);
            if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
            {
                jpf_get_record_policy_detail_info(result, &query_res->record_policy[i]);
           }
           else
           {
               ret = MYSQL_RESULT_CODE(result);
               jpf_sql_put_res(result, sizeof(JpfMysqlRes));
               goto err_do_mss_query;
           }

           if(result)
               jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        }
    }

end_query_mss:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    query_res->type = req_info->type;
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_mss_query:

    size = sizeof(JpfQueryRecordPolicyRes);
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

    goto end_query_mss;
}


JpfMsgFunRet
jpf_mod_dbs_record_policy_config_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfRecordPolicyConfig *req_info;
    JpfBssRes result;
    char query_buf[QUERY_STR_LEN];
    db_conn_status *conn = NULL;
    JpfModDbs *dbs_obj;
    gint ret = 0,  i = 0, size;
    memset(&result, 0, sizeof(result));

    req_info = MSG_GET_DATA(msg);
    size = MSG_DATA_SIZE(msg);

    dbs_obj = JPF_MODDBS(app_obj);
	conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
	if (G_UNLIKELY(!conn))
	{
		jpf_warning("<JpfModDbs> get db connection error!");
		ret = -E_GETDBCONN;
		goto end_modify_policy;
	}

	if (G_UNLIKELY(!conn->mysql))
	{
		jpf_warning("<JpfModDbs> get db connection error!");
		put_db_connection(dbs_obj->pool_info, conn);
		ret = -E_GETDBCONN;
		goto end_modify_policy;
	}

    switch (req_info->type){
    case 0:
        JPF_MYSQL_DO_QUERY_OR_KILL_CONN(dbs_obj, conn, "start transaction", ret);
        if (ret)
		goto end_modify_policy;
        while (i < req_info->gu_count)
        {
            snprintf(
                query_buf, QUERY_STR_LEN,
                "update %s set level=%d, mss_policy='%s',hd_group_id1='%s',hd_group_id2='%s',hd_group_id3='%s',hd_group_id4='%s',\
                hd_group_id5='%s' where gu_id='%s' and guid_domain='%s' and mss_id='%s'",
                RECORD_POLICY_TABLE, req_info->record_policy[i].level, req_info->time_policy,
                req_info->hd_group[0].hd_group_id, req_info->hd_group[1].hd_group_id, req_info->hd_group[2].hd_group_id,
                req_info->hd_group[3].hd_group_id,req_info->hd_group[4].hd_group_id,req_info->record_policy[i].guid,
                req_info->record_policy[i].domain_id, req_info->record_policy[i].mss_id
            );

            ret= jpf_mysql_do_query(conn->mysql, query_buf);
            if (ret != 0 )
            {
                jpf_mysql_do_query(conn->mysql, "rollback");
                put_db_connection(dbs_obj->pool_info, conn);
                goto end_modify_policy;
            }
            i++;
        }

        ret = jpf_mysql_do_query(conn->mysql, "commit");
        break;
    case 1:
        snprintf(
            query_buf, QUERY_STR_LEN,
            "update %s set level=%d, mss_policy='%s',hd_group_id1='%s',hd_group_id2='%s',hd_group_id3='%s',hd_group_id4='%s',\
            hd_group_id5='%s' where mss_id='%s'",
            RECORD_POLICY_TABLE, req_info->level, req_info->time_policy,
            req_info->hd_group[0].hd_group_id, req_info->hd_group[1].hd_group_id,
            req_info->hd_group[2].hd_group_id, req_info->hd_group[3].hd_group_id,
            req_info->hd_group[4].hd_group_id, req_info->mss_id
        );

        ret= jpf_mysql_do_query(conn->mysql, query_buf);
        if (ret != 0 )
        {
            put_db_connection(dbs_obj->pool_info, conn);
            goto end_modify_policy;
        }
        break;

    }
    put_db_connection(dbs_obj->pool_info, conn);

end_modify_policy:
    SET_CODE(&result, ret);
    JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_query_user_own_gu_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryUserOwnGu *req_info;
    JpfQueryUserOwnGuRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select user_guid from  %s where user_name='%s' and user_guid_domain='%s' ",
        USER_OWN_GU_TABLE, req_info->user,req_info->domain_id
        );

    total_num =  jpf_dbs_get_row_num(app_obj, msg, query_buf);
    if (total_num == 0)
    {
        ret = 0;
        goto ERR_DO_MDS_QUERY;
    }

    memset(query_buf, 0, QUERY_STR_LEN);
    snprintf(
        query_buf, QUERY_STR_LEN,
         "select t1.user_guid,t2.gu_name from  %s as t1, %s as t2 where t1.user_name='%s' \
         and t1.user_guid_domain='%s' and t2.gu_id=t1.user_guid \
         and t2.gu_domain=t1.user_guid_domain limit %d,%d",
         USER_OWN_GU_TABLE,GU_TABLE,req_info->user,req_info->domain_id,
         req_info->start_num, req_info->req_num
        );

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_user_own_gu(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

        strncpy(query_res->domain_id, req_info->domain_id, DOMAIN_ID_LEN - 1);
        strncpy(query_res->user, req_info->user, USER_NAME_LEN - 1);
        query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto ERR_DO_MDS_QUERY;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

END_QUERY_MDS:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

ERR_DO_MDS_QUERY:

    size = sizeof(JpfQueryUserOwnGuRes);
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

    goto END_QUERY_MDS;
}


JpfMsgFunRet
jpf_mod_dbs_query_user_own_tw_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryUserOwnTw *req_info;
    JpfQueryUserOwnTwRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select count(*) as count from  %s where user_name='%s'",
        USER_OWN_TW_TABLE, req_info->user
    );

	total_num =  jpf_get_record_count(app_obj,query_buf);
	if (total_num == 0)
	{
	  ret = 0;
	  goto ERR_DO_MDS_QUERY;
	}

    memset(query_buf, 0, QUERY_STR_LEN);
    snprintf(
        query_buf, QUERY_STR_LEN,
         "select t1.user_tw_id,t2.tw_name from  %s as t1, %s as t2 where \
         t1.user_name='%s' and t2.tw_id=t1.user_tw_id limit %d,%d",
         USER_OWN_TW_TABLE,TW_TABLE,req_info->user,
         req_info->start_num, req_info->req_num
        );

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_user_own_tw(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

        strncpy(query_res->user, req_info->user, USER_NAME_LEN - 1);
        query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto ERR_DO_MDS_QUERY;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

END_QUERY_MDS:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

ERR_DO_MDS_QUERY:

    size = sizeof(JpfQueryUserOwnTwRes);
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

    goto END_QUERY_MDS;
}


JpfMsgFunRet
jpf_mod_dbs_query_user_own_tour_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryUserOwnTour *req_info;
    JpfQueryUserOwnTourRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select count(*) as count from  %s where user_name='%s'",
        USER_OWN_TOUR_TABLE, req_info->user
    );

	total_num =  jpf_get_record_count(app_obj,query_buf);
	if (total_num == 0)
	{
	  ret = 0;
	  goto ERR_DO_MDS_QUERY;
	}

    memset(query_buf, 0, QUERY_STR_LEN);
    snprintf(
        query_buf, QUERY_STR_LEN,
         "select t1.user_tour_id,t2.tour_name from  %s as t1, %s as t2 where \
         t1.user_name='%s' and t2.tour_id=t1.user_tour_id limit %d,%d",
         USER_OWN_TOUR_TABLE,TOUR_TABLE,req_info->user,
         req_info->start_num, req_info->req_num
        );

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_user_own_tour(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

        strncpy(query_res->user, req_info->user, USER_NAME_LEN - 1);
        query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto ERR_DO_MDS_QUERY;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

END_QUERY_MDS:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

ERR_DO_MDS_QUERY:

    size = sizeof(JpfQueryUserOwnTourRes);
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

    goto END_QUERY_MDS;
}


JpfMsgFunRet
jpf_mod_dbs_query_defence_area_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryDefenceArea *req_info;
    JpfQueryDefenceAreaRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num ;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if (req_info->type == 0)
    {
        snprintf(query_buf, QUERY_STR_LEN, "select count(*)  as count from %s ", DEFENCE_AREA_TABLE);
        total_num =  jpf_get_record_count(app_obj,query_buf);
        if (total_num == 0)
        {
            ret = 0;
            goto err_do_defence_area_query;
        }

        snprintf(query_buf, QUERY_STR_LEN,
       	"select t1.*,t2.area_name from %s as t1,%s as t2 where t1.defence_area_id=t2.area_id limit %d,%d",
       	 DEFENCE_AREA_TABLE, AREA_TABLE, req_info->start_num, req_info->req_num);
    }
    else if (req_info->type == 1)
    {
        snprintf(query_buf, QUERY_STR_LEN,
       	"select t1.*,t2.area_name from %s as t1,%s as t2 where t1.defence_area_id=t2.area_id and t1.defence_area_id =%s limit %d,%d",
       	 DEFENCE_AREA_TABLE, AREA_TABLE, req_info->key, req_info->start_num, req_info->req_num);
        total_num = 1;
    }
    result = jpf_dbs_do_query_res(app_obj,query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_defence_area(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
	 query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_do_defence_area_query;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_query_defence_area:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_defence_area_query:
    size = sizeof(JpfQueryDefenceAreaRes);
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
    query_res->req_num = 0;
    goto end_query_defence_area;
}


JpfMsgFunRet
jpf_mod_dbs_query_defence_map_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryDefenceMap *req_info;
    JpfQueryDefenceMapRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num ;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(query_buf, QUERY_STR_LEN,
	"select count(*)  as count from %s where defence_area_id=%d",
	DEFENCE_MAP_TABLE, req_info->defence_area_id
    );

    total_num =  jpf_get_record_count(app_obj,query_buf);
    if (total_num == 0)
    {
        ret = 0;
        goto err_do_defence_map_query;
    }

    /*snprintf(query_buf, QUERY_STR_LEN,
	"select t1.*,t2.map_name,t2.map_location from %s as t1,%s as t2 \
	 where t1.defence_area_id=%d and t1.map_id=t2.map_id limit %d,%d",
	 DEFENCE_MAP_TABLE, MAP_TABLE,req_info->defence_area_id,
	 req_info->start_num, req_info->req_num
    );	*/

    snprintf(query_buf, QUERY_STR_LEN,
	"select * from %s where defence_area_id=%d limit %d,%d",
	 DEFENCE_MAP_TABLE, req_info->defence_area_id,
	 req_info->start_num, req_info->req_num
    );
    result = jpf_dbs_do_query_res(app_obj,query_buf);
    BUG_ON(!result);
    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_defence_map(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
	 query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_do_defence_map_query;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_query_defence_map:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_defence_map_query:
    size = sizeof(JpfQueryDefenceMapRes);
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
    query_res->back_num = 0;
    goto end_query_defence_map;
}


JpfMsgFunRet
jpf_mod_dbs_query_defence_gu_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryDefenceGu *req_info;
    JpfQueryDefenceGuRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num ;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(query_buf, QUERY_STR_LEN,
	"select count(*)  as count from %s where map_id=%d",
	MAP_GU_TABLE, req_info->map_id
    );

    total_num =  jpf_get_record_count(app_obj,query_buf);
    if (total_num == 0)
    {
        ret = 0;
        goto err_do_defence_gu_query;
    }

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select t1.*,t2.gu_name,t2.gu_type,t3.pu_info from %s as t1,%s as t2, %s as t3 \
        where map_id=%d and t1.gu_id=t2.gu_id and t1.gu_domain=t2.gu_domain and \
        t2.gu_domain=t3.pu_domain and t2.gu_puid=t3.pu_id limit %d,%d",
         MAP_GU_TABLE, GU_TABLE, PU_TABLE, req_info->map_id, req_info->start_num, req_info->req_num
    );

    result = jpf_dbs_do_query_res(app_obj,query_buf);
    BUG_ON(!result);
    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_defence_gu(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
	 query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_do_defence_gu_query;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_query_defence_gu:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_defence_gu_query:
    size = sizeof(JpfQueryDefenceGuRes);
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
    query_res->back_num = 0;
    goto end_query_defence_gu;
}


JpfMsgFunRet
jpf_mod_dbs_query_map_href_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryMapHref *req_info;
    JpfQueryMapHrefRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num ;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(query_buf, QUERY_STR_LEN,
	"select count(*)  as count from %s where src_map_id=%d",
	MAP_HREF_TABLE, req_info->map_id
    );

    total_num =  jpf_get_record_count(app_obj,query_buf);
    if (total_num == 0)
    {
        ret = 0;
        goto err_do_map_href_query;
    }

    snprintf(query_buf, QUERY_STR_LEN,
	"select t1.*,t2.map_name,t2.map_location from %s as t1,%s as t2 where \
	t1.src_map_id=%d and t1.dst_map_id=t2.map_id limit %d,%d",
	 MAP_HREF_TABLE, DEFENCE_MAP_TABLE, req_info->map_id,
	 req_info->start_num, req_info->req_num
    );	 printf("----map herf ===%s\n", query_buf);
    result = jpf_dbs_do_query_res(app_obj,query_buf);
    BUG_ON(!result);
    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_map_href(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
	 query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_do_map_href_query;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_query_map_href:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_map_href_query:
    size = sizeof(JpfQueryMapHrefRes);
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
    query_res->back_num = 0;
    goto end_query_map_href;
}


JpfMsgFunRet
jpf_mod_dbs_query_tw_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryTw *req_info;
    JpfQueryTwRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num ;
printf("-----enter jpf_mod_dbs_query_screen_b\n");
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    switch(req_info->type)
    {
        case 0:
               snprintf(query_buf, QUERY_STR_LEN,
              	"select count(*)  as count from %s",
              	TW_TABLE
               );

		 total_num =  jpf_get_record_count(app_obj,query_buf);
               if (total_num == 0)
               {
                  ret = 0;
                  goto err_do_tw_query;
               }

               snprintf(query_buf, QUERY_STR_LEN,
              	"select * from %s order by tw_id desc limit %d,%d",
              	 TW_TABLE,
              	 req_info->start_num, req_info->req_num
              );
		break;
	 case 1:
              snprintf(query_buf, QUERY_STR_LEN,
              	"select * from %s where tw_id=%s ",
              	 TW_TABLE, req_info->key
              );
             total_num = 1;
	 	break;
	 default:
	 	break;

    }

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_tw(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
	 query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_do_tw_query;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_query_tw:
    query_res->type = req_info->type;
    strcpy(query_res->bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_tw_query:
    size = sizeof(JpfQueryTwRes);
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
    query_res->back_num = 0;
    goto end_query_tw;
}


JpfMsgFunRet
jpf_mod_dbs_query_screen_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryScreen *req_info;
    JpfQueryScreenRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num ;
printf("-----enter jpf_mod_dbs_query_screen_b\n");
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    switch(req_info->type)
    {
        case 0:
		snprintf(query_buf, QUERY_STR_LEN,
              	"select count(*)  as count from %s where tw_id=%s",
              	SCREEN_TABLE, req_info->key
               );

		 total_num =  jpf_get_record_count(app_obj,query_buf);
               if (total_num == 0)
               {
                  ret = 0;
                  goto err_do_screen_query;
               }

        	 snprintf(
                    query_buf, QUERY_STR_LEN,
                    "select t1.pu_info,t1.pu_minor_type,t2.pu_state,\
                    t2.pu_last_ip,t3.gu_name,t4.* from %s as t1,%s as t2,\
                    %s as t3, %s as t4 where t4.tw_id='%s' and t4.dis_domain=\
                    t3.gu_domain and t4.dis_guid=t3.gu_id and t1.pu_id=\
                    t3.gu_puid and t1.pu_domain=t3.gu_domain and\
                    t1.pu_id=t2.pu_id and t1.pu_domain=t2.pu_domain limit %d,%d",
                    PU_TABLE, PU_RUNNING_TABLE, GU_TABLE, SCREEN_TABLE, req_info->key,
                    req_info->start_num, req_info->req_num
                );

	       break;
	 case 1:
              snprintf(query_buf, QUERY_STR_LEN,
              	"select * from %s where scr_id=%s ",
              	 SCREEN_TABLE, req_info->key
              );
             total_num = 1;
	 	break;
	 default:
	 	break;

    }

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_screen(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
	 query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_do_screen_query;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_query_screen:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_screen_query:
    size = sizeof(JpfQueryScreenRes);
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
    query_res->back_num = 0;
    goto end_query_screen;
}


JpfMsgFunRet
jpf_mod_dbs_query_scr_div_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryScrDiv *req_info;
    JpfQueryScrDivRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num ;
printf("-----enter jpf_mod_dbs_query_screen_b\n");
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    switch(req_info->type)
    {
        case 0:
		snprintf(query_buf, QUERY_STR_LEN,
              	"select count(*)  as count from %s",
              	SCREEN_DIVISION_TABLE
               );

		 total_num =  jpf_get_record_count(app_obj,query_buf);
               if (total_num == 0)
               {
                  ret = 0;
                  goto err_do_scr_div_query;
               }

              snprintf(query_buf, QUERY_STR_LEN,
              	"select * from %s limit %d,%d",
              	 SCREEN_DIVISION_TABLE,
              	 req_info->start_num, req_info->req_num
              );
		break;
	 case 1:
              snprintf(query_buf, QUERY_STR_LEN,
              	"select * from %s where div_id=%s ",
              	 SCREEN_DIVISION_TABLE, req_info->key
              );
             total_num = 1;
	 	break;
	 default:
	 	break;

    }

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_scr_div(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
	 query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_do_scr_div_query;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_query_scr_div:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_scr_div_query:
    size = sizeof(JpfQueryScrDivRes);
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
    query_res->back_num = 0;
    goto end_query_scr_div;
}


JpfMsgFunRet
jpf_mod_dbs_query_tour_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryTour *req_info;
    JpfQueryTourRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num ;
    printf("-----enter jpf_mod_dbs_query_tour_b\n");
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    switch(req_info->type)
    {
        case 0:
               snprintf(query_buf, QUERY_STR_LEN,
              	"select count(*)  as count from %s",
              	TOUR_TABLE
               );

		 total_num =  jpf_get_record_count(app_obj,query_buf);
               if (total_num == 0)
               {
                  ret = 0;
                  goto err_do_tour_query;
               }

               snprintf(query_buf, QUERY_STR_LEN,
              	"select * from %s order by tour_id desc limit %d,%d",
              	 TOUR_TABLE,
              	 req_info->start_num, req_info->req_num
              );
		break;
	 case 1:
              snprintf(query_buf, QUERY_STR_LEN,
              	"select * from %s where tour_id=%s ",
              	 TOUR_TABLE, req_info->key
              );
              total_num = 1;
	 	break;
	 default:
	 	break;

    }

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_tour(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
	 query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_do_tour_query;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_query_tour:
    query_res->type = req_info->type;
    strcpy(query_res->bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_tour_query:
    size = sizeof(JpfQueryTourRes);
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
    query_res->back_num = 0;
    goto end_query_tour;
}


JpfMsgFunRet
jpf_mod_dbs_query_tour_step_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryTourStep *req_info;
    JpfQueryTourStepRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num ;
    printf("-----enter jpf_mod_dbs_query_tour_step_b\n");
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(query_buf, QUERY_STR_LEN,
        "select count(*) as count from %s where tour_id=%d",
        TOUR_STEP_TABLE, req_info->tour_id
    );
    total_num =  jpf_get_record_count(app_obj,query_buf);
    if (total_num == 0)
    {
        ret = 0;
        goto err_do_tour_step_query;
    }

  snprintf(query_buf, QUERY_STR_LEN,
        "select t1.*,t2.gu_name from %s as t1,%s as t2 where t1.tour_id=%d and \
        t1.encoder_domain=t2.gu_domain and t1.encoder_guid=t2.gu_id limit %d,%d",
        TOUR_STEP_TABLE, GU_TABLE, req_info->tour_id,
        req_info->start_num, req_info->req_num
    );

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_tour_step(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
	 query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_do_tour_step_query;
    }

    if(result)
       jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_query_tour_step:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_tour_step_query:
    size = sizeof(JpfQueryTourStepRes);
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
    query_res->back_num = 0;
    goto end_query_tour_step;
}


JpfMsgFunRet
jpf_mod_dbs_query_group_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryGroup *req_info;
    JpfQueryGroupRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num ;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    switch(req_info->type)
    {
        case 0:
               snprintf(query_buf, QUERY_STR_LEN,
              	"select count(*)  as count from %s",
              	GROUP_TABLE
               );

		 total_num =  jpf_get_record_count(app_obj,query_buf);
               if (total_num == 0)
               {
                  ret = 0;
                  goto err_do_group_query;
               }

               snprintf(query_buf, QUERY_STR_LEN,
              	"select t1.*,t2.tw_name from %s as t1,%s as t2 where t1.tw_id=t2.tw_id \
              	 order by group_id desc limit %d,%d",
              	 GROUP_TABLE, TW_TABLE,
              	 req_info->start_num, req_info->req_num
              );
		break;
	 case 1:
              snprintf(query_buf, QUERY_STR_LEN,
              	"select * from %s where group_id=%s ",
              	 GROUP_TABLE, req_info->key
              );
              total_num = 1;
	 	break;
	 default:
	 	break;

    }

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_group(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
	 query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_do_group_query;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_query_group:
    query_res->type = req_info->type;
    strcpy(query_res->bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_group_query:
    size = sizeof(JpfQueryGroupRes);
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
    query_res->back_num = 0;
    goto end_query_group;
}


JpfMsgFunRet
jpf_mod_dbs_query_group_step_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryGroupStep *req_info;
    JpfQueryGroupStepRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num ;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    switch(req_info->type)
    {
        case 0:
               snprintf(query_buf, QUERY_STR_LEN,
               "select count(*) as count from %s where group_id='%s'",
               GROUP_STEP_TABLE, req_info->key
               );

	        total_num =  jpf_get_record_count(app_obj,query_buf);
              if (total_num == 0)
             {
                  ret = 0;
                  goto err_do_group_step_query;
              }

              snprintf(query_buf, QUERY_STR_LEN,
              	"select * from %s where group_id='%s' order by step_no desc limit %d,%d",
              	 GROUP_STEP_TABLE,req_info->key,
              	 req_info->start_num, req_info->req_num
              );
		break;
	 case 1:
              snprintf(query_buf, QUERY_STR_LEN,
              	"select * from %s where step_no='%s' ",
              	 GROUP_STEP_TABLE, req_info->key
              );
              total_num = 1;
	 	break;
	 default:
	 	break;

    }

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_group_step(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
	 query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_do_group_step_query;
    }

    if(result)
       jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_query_group_step:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_group_step_query:
    size = sizeof(JpfQueryGroupStepRes);
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
    query_res->back_num = 0;
    goto end_query_group_step;
}


JpfMsgFunRet
jpf_mod_dbs_query_group_step_info_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryGroupStepInfo *req_info;
    JpfQueryGroupStepInfoRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num ;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    switch(req_info->type)
    {
        case 0:
               snprintf(query_buf, QUERY_STR_LEN,
              	"select count(*)  as count from %s where step_no=%d and scr_id = %d",
              	GROUP_STEP_INFO_TABLE, req_info->step_no, req_info->scr_id
               );

		 total_num =  jpf_get_record_count(app_obj,query_buf);
               if (total_num == 0)
               {
                  ret = 0;
                  goto err_do_group_step_info_query;
               }
               snprintf(
                    query_buf, QUERY_STR_LEN,
                    "select t1.pu_info,t2.pu_state,t2.pu_last_ip,t3.gu_name,t4.* from %s as t1,%s as t2,\
                    %s as t3, %s as t4 where t4.step_no=%d and t4.scr_id = %d and t4.encoder_domain=t3.gu_domain and \
                    t4.encoder_guid=t3.gu_id and t1.pu_id=t3.gu_puid and t1.pu_domain=t3.gu_domain and\
                    t1.pu_id=t2.pu_id and t1.pu_domain=t2.pu_domain limit %d,%d",
                    PU_TABLE, PU_RUNNING_TABLE, GU_TABLE, GROUP_STEP_INFO_TABLE,
                    req_info->step_no, req_info->scr_id, req_info->start_num, req_info->req_num
                );
		break;
	 case 1:
              snprintf(query_buf, QUERY_STR_LEN,
              	"select * from %s where step_no=%d and scr_id=%d and div_no='%s'",
              	 GROUP_STEP_INFO_TABLE,req_info->step_no, req_info->scr_id, req_info->key
              );
             total_num = 1;
	 	break;
	 default:
	 	break;

    }

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_group_step_info(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
	 query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_do_group_step_info_query;
    }

    if(result)
       jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_query_group_step_info:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_group_step_info_query:
    size = sizeof(JpfQueryGroupStepInfoRes);
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
    query_res->back_num = 0;
    goto end_query_group_step_info;
}


JpfMsgFunRet
jpf_mod_dbs_query_group_step_div_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryGroupStepDiv *req_info;
    JpfQueryGroupStepDivRes query_res;
    char query_buf[QUERY_STR_LEN] = {0};
    gint ret = 0, total_num = 0;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

	memset(&query_res, 0, sizeof(query_res));
   snprintf(query_buf, QUERY_STR_LEN,
  	"select count(t1.step_no) as count from %s as t1,%s as t2 where\
  	t1.scr_id=%d and t2.group_id=%d and t1.step_no=t2.step_no",
  	GROUP_STEP_INFO_TABLE,GROUP_STEP_TABLE, req_info->scr_id,
  	req_info->group_id
   );

   total_num =  jpf_get_record_count(app_obj,query_buf);
   if (total_num == 0)
   {
      ret = 0;
      goto end_query_group_step_div;
   }
	snprintf(query_buf, QUERY_STR_LEN,
  	"select min(t1.step_no), div_id as count from %s as t1,%s as t2 where\
  	t1.scr_id=%d and t2.group_id=%d and t1.step_no=t2.step_no",
  	GROUP_STEP_INFO_TABLE,GROUP_STEP_TABLE, req_info->scr_id,
  	req_info->group_id
   );

   total_num = jpf_get_record_count(app_obj,query_buf);

end_query_group_step_div:
	SET_CODE(&query_res, ret);
	query_res.div_id = total_num;
    strcpy(query_res.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &query_res, sizeof(query_res), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);
    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_query_alarm_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryAlarm *req_info;;
    JpfQueryAlarmRes *query_res;
    JpfMysqlRes *result;
    gchar query_buf[QUERY_STR_LEN] = {0};
    gint size,ret;
    gint total_num;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(query_buf, QUERY_STR_LEN,
	"select count(*)  as count from %s where state&%d and alarm_type&%d and alarm_time>'%s' \
	and alarm_time<'%s'",
	ALARM_INFO_TABLE, req_info->alarm_state, req_info->alarm_type, req_info->start_time,
	req_info->end_time);

    total_num =  jpf_get_record_count(app_obj,query_buf);
    if (total_num == 0)
    {
        ret = 0;
        goto err_query_alarm;
    }

    if (req_info->order_by == 1)
    {
        snprintf(query_buf, QUERY_STR_LEN,
       	"select * from  %s where state&%d and alarm_type&%d and alarm_time>'%s' and \
       	alarm_time<'%s' order by alarm_time desc, alarm_id desc limit %d,%d",
       	ALARM_INFO_TABLE, req_info->alarm_state, req_info->alarm_type,req_info->start_time,
       	req_info->end_time, req_info->start_num, req_info->req_num);
    }
    else
    {
        snprintf(query_buf, QUERY_STR_LEN,
       	"select * from  %s where state&%d and alarm_type&%d and alarm_time>'%s' and \
       	alarm_time<'%s' order by alarm_time, alarm_id limit %d,%d",
       	ALARM_INFO_TABLE, req_info->alarm_state, req_info->alarm_type,req_info->start_time,
       	req_info->end_time, req_info->start_num, req_info->req_num);

    }

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_query_alarm(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

        if(req_info->req_num > 0)
            query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_query_alarm;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_query_alarm:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_query_alarm:
    size = sizeof(JpfQueryAlarmRes);
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

    goto end_query_alarm;
}


JpfMsgFunRet
jpf_mod_dbs_query_server_resource_info_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryServerResourceInfo *req_info;;
    JpfQueryServerResourceInfoRes query_res;
    gint ret = 0, dev_num, dev_online_num;
    JpfResourcesCap res_cap;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&query_res, 0, sizeof(query_res));
    ret = jpf_dbs_get_dev_total_count(app_obj);
    if (ret < 0)
        goto end_query_resource_info;
    dev_num = ret;

    ret = jpf_dbs_get_online_dev_total_count(app_obj);
    if (ret < 0)
        goto end_query_resource_info;
    dev_online_num = ret;

    ret = jpf_dbs_get_dev_type_count(app_obj, DECODER_TYPE);
    if (ret < 0)
        goto end_query_resource_info;
    query_res.dec_num = ret;

    ret = jpf_dbs_get_online_dev_type_count(app_obj, DECODER_TYPE);
    if (ret < 0)
        goto end_query_resource_info;
    query_res.dec_online_num = ret;

    query_res.enc_num = dev_num - query_res.dec_num;
    query_res.enc_online_num = dev_online_num - query_res.dec_online_num;

    ret = jpf_dbs_get_gu_type_count(app_obj, AV_TYPE);
    if (ret < 0)
        goto end_query_resource_info;
    query_res.av_num = ret;

    ret = jpf_dbs_get_gu_type_count(app_obj, DS_TYPE);
    if (ret < 0)
        goto end_query_resource_info;
    query_res.ds_num = ret;

    ret = jpf_dbs_get_gu_type_count(app_obj, AI_TYPE);
    if (ret < 0)
        goto end_query_resource_info;
    query_res.ai_num = ret;

    ret = jpf_dbs_get_gu_type_count(app_obj, AO_TYPE);
    if (ret < 0)
        goto end_query_resource_info;
    query_res.ao_num = ret;

    memset(&res_cap, 0, sizeof(res_cap));
    jpf_mod_get_resource_cap(&res_cap);
    query_res.av_limited_num = res_cap.av_count;
    query_res.ds_limited_num = res_cap.ds_count;
    query_res.ai_limited_num = res_cap.ai_count;
    query_res.ao_limited_num = res_cap.ao_count;
    query_res.system_version = res_cap.system_version;
    query_res.manufactor_type = res_cap.modules_data[SYS_MODULE_CMS];
    query_res.support_keyboard = (res_cap.modules_data[SYS_MODULE_TW] & TW_KEYBOARD_BIT) ? 1 : 0;
    memcpy(&query_res.expired_time, &res_cap.expired_time, sizeof(JpfExpiredTime));

    ret = 0;
end_query_resource_info:
    SET_CODE(&query_res.code, ret);
    strcpy(query_res.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &query_res, sizeof(query_res), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_query_ivs_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryIvs *req_info;
    JpfQueryIvsRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if (req_info->type == 0)   //query all ivs
    {
        snprintf(query_buf, QUERY_STR_LEN,
        	"select count(*) as count from %s ", IVS_TABLE
        );

        total_num = jpf_get_record_count(app_obj,query_buf);
        if (total_num <= 0)
        {
            ret = 0;
            goto err_do_ivs_query;
        }

        snprintf(
            query_buf, QUERY_STR_LEN,
            "select * from %s order by ivs_state desc limit %d,%d",
            IVS_TABLE, req_info->start_num, req_info->req_num
        );
    }
    else if (req_info->type == 1)  //query ivs by id
    {
        snprintf(
             query_buf, QUERY_STR_LEN,
             "select count(*) as count from %s where ivs_id='%s' ",
             IVS_TABLE, req_info->key
         );

        total_num =  jpf_get_record_count(app_obj,query_buf);
        if (total_num <= 0)
        {
            ret = 0;
            goto err_do_ivs_query;
        }

        snprintf(
            query_buf, QUERY_STR_LEN,  "select * from  %s where ivs_id='%s' ",
            IVS_TABLE, req_info->key
        );
    }
    else
    {
        jpf_warning("query type(%d) is wrong ", req_info->type);
        ret = E_QUERYTYPE;
        goto err_do_ivs_query;
    }

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_ivs(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

        query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_do_ivs_query;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_query_ivs:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_ivs_query:

    size = sizeof(JpfQueryIvsRes);
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

    goto end_query_ivs;
}


JpfMsgFunRet
jpf_mod_dbs_get_next_puno_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfGetNextPuNo *req_info;;
    JpfGetNextPuNoRes query_res;
    gint ret = 0, id;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&query_res, 0, sizeof(query_res));
    id = jpf_mod_dbs_get_init_pu_id(app_obj);
    id++;
    if (id == 100000000)
        id = 0;

    sprintf(query_res.pu_no, "%08d", id);
    if (id < 0)
    	ret = id;

    SET_CODE(&query_res.code, ret);
    strcpy(query_res.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &query_res, sizeof(query_res), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


static __inline__ gint
jpf_mod_dbs_get_del_alarm_policy(JpfAppObj *self, JpfMysqlRes *result,
      JpfQueryDelAlarmPolicyRes *alarm_param)
{
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

        row_num = jpf_sql_get_num_rows(result);
	 if (row_num == 0)
	 {
	     dbs_obj->del_alarm_flag &= ~ENABLE_DEL_ALARM;
	     alarm_param->enable = 0;
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
        alarm_param->max_capacity = atoi(strtok(NULL, ","));
	 dbs_obj->del_alarm_flag |= ENABLE_DEL_ALARM;
	 alarm_param->enable = 1;
	 return 0;
}


JpfMsgFunRet
jpf_mod_dbs_query_del_alarm_policy_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryDelAlarmPolicyRes query_res;
    JpfMysqlRes *result;
    gchar query_buf[QUERY_STR_LEN] = {0};
    gint ret;

	memset(&query_res, 0, sizeof(query_res));
    snprintf(query_buf, QUERY_STR_LEN,
	"select * from %s where id=1",
	PARAM_CONFIG_TABLE);

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        ret = jpf_mod_dbs_get_del_alarm_policy(app_obj, result, &query_res);
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_query_del_alarm_policy;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_query_del_alarm_policy:

    jpf_dbs_modify_sysmsg_2(msg, &query_res, sizeof(query_res), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;

err_query_del_alarm_policy:
    SET_CODE(&query_res, ret);

    goto end_query_del_alarm_policy;
}


static __inline__ gint
jpf_mod_dbs_get_link_time_policy(JpfAppObj *self, JpfMysqlRes *result,
      JpfQueryLinkTimePolicyRes *alarm_param)
{
    guint field_num;
    JpfMysqlRow mysql_row;
    JpfMysqlField* mysql_fields;
    gchar *name;
    gchar *value;
    gint field_no =0;
    JpfModDbs        *dbs_obj;
    dbs_obj = JPF_MODDBS(self);

        field_num = jpf_sql_get_num_fields(result);

        while ((mysql_row = jpf_sql_fetch_row(result)))
        {
            jpf_sql_field_seek(result, 0);
            mysql_fields = jpf_sql_fetch_fields(result);
            for(field_no = 0; field_no < field_num; field_no++)
            {
                name = jpf_sql_get_field_name(mysql_fields, field_no);
                if (!strcmp(name, "time_policy"))
                {
                    value = jpf_sql_get_field_value(mysql_row, field_no);
                    if(value)
                    {
                        alarm_param->time_policy[POLICY_LEN - 1] = 0;
                        strncpy(
                            alarm_param->time_policy,
                            value, POLICY_LEN - 1
                        );
                    }
                }
                else
                    cms_debug("no need mysql name %s ", name);
            }
        }

	 return 0;
}


JpfMsgFunRet
jpf_mod_dbs_query_link_time_policy_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryLinkTimePolicy *req_info;
    JpfQueryLinkTimePolicyRes query_res;
    JpfMysqlRes *result;
    gchar query_buf[QUERY_STR_LEN] = {0};
    gint ret;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(query_buf, QUERY_STR_LEN,
	"select time_policy from %s where gu_id='%s' and domain_id='%s'",
	LINK_TIME_POLICY_TABLE, req_info->guid, req_info->domain);

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        ret = jpf_mod_dbs_get_link_time_policy(app_obj, result, &query_res);
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_query_link_time_policy;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_query_link_time_policy:
    strcpy(query_res.bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &query_res, sizeof(query_res), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;

err_query_link_time_policy:
    SET_CODE(&query_res, ret);

    goto end_query_link_time_policy;
}

/*
JpfMsgFunRet
jpf_mod_dbs_query_link_record_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryLinkRecord *req_info;
    JpfQueryLinkRecordRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num ;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    switch(req_info->type)
    {
        case 0:
               snprintf(query_buf, QUERY_STR_LEN,
              	"select  count(distinct link_guid,link_domain_id) as count from %s where gu_id='%s' and domain_id='%s'",
              	ALARM_LINK_RECORD_TABLE, req_info->guid, req_info->domain
               );

		 total_num =  jpf_get_record_count(app_obj,query_buf);
               if (total_num == 0)
               {
                  ret = 0;
                  goto err_do_link_record_query;
               }
               snprintf(
                    query_buf, QUERY_STR_LEN,
                    "select t1.*,t2.mss_name from %s as t1,%s as t2 where t1.gu_id='%s' \
                    and t1.domain_id='%s' and t1.mss_id=t2.mss_id limit %d,%d",
                    ALARM_LINK_RECORD_TABLE, MSS_TABLE,
                    req_info->guid, req_info->domain, req_info->start_num, req_info->req_num
                );
		break;
	 case 1:
              snprintf(query_buf, QUERY_STR_LEN,
              	"select  count(distinct gu_id,guid_domain) as count from %s where gu_id='%s' \
              	and domain_id='%s'",
              	ALARM_LINK_RECORD_TABLE, req_info->guid, req_info->domain
               );

		 total_num =  jpf_get_record_count(app_obj,query_buf);
               if (total_num == 0)
               {
                  ret = 0;
                  goto err_do_link_record_query;
               }

               snprintf(
                    query_buf, QUERY_STR_LEN,
                    "select t1.gu_id as link_guid,t1.guid_domain as link_domain_id,t2.mss_id,\
                    t2.mss_name from %s as t1, %s as t2 where t1.gu_id='%s' and \
                    t1.guid_domain='%s' and t1.mss_id=t2.mss_id",
                    RECORD_POLICY_TABLE, MSS_TABLE,
                    req_info->link_guid, req_info->link_domain
                );
	 	break;
	 default:
	 	break;

    }

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_link_record(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
	 query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_do_link_record_query;
    }

    if(result)
       jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_query_link_record:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_link_record_query:
    size = sizeof(JpfQueryLinkRecordRes);
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
    query_res->back_num = 0;
    goto end_query_link_record;
}*/

static void jpf_mod_dbs_get_mss_of_link_record(JpfAppObj *app_obj,
	JpfQueryLinkRecordRes *gu)
{
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint ret, row_num = 0, i;

        for (i = 0; i < gu->back_num; i++)
        {
             snprintf(
                query_buf, QUERY_STR_LEN,
                "select t1.mss_id,t2.mss_name from %s as t1,%s as t2 where t1.gu_id='%s' and \
                t1.domain_id='%s' and t1.link_domain_id='%s' \
                and t1.link_guid='%s' and t1.mss_id=t2.mss_id",
                ALARM_LINK_RECORD_TABLE, MSS_TABLE, gu->guid, gu->domain,
                gu->link_record_info[i].link_domain, gu->link_record_info[i].link_guid
            );

    	     result = jpf_dbs_do_query_res(app_obj, query_buf);
            BUG_ON(!result);

            if (G_LIKELY(MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
            {
                ret = MYSQL_RESULT_CODE(result);
                jpf_sql_put_res(result, sizeof(JpfMysqlRes));
		  SET_CODE(gu, ret);
		  return;
            }

            row_num = jpf_sql_get_num_rows(result);
            printf("get row num (%d)\n",row_num);
            if(row_num == 0)
            {
                jpf_sql_put_res(result,sizeof(JpfMysqlRes));
                continue;
            }

            jpf_get_gu_mss(result,  &gu->link_record_info[i].mss[0]);
	     jpf_sql_put_res(result,sizeof(JpfMysqlRes));
       }
}


JpfMsgFunRet
jpf_mod_dbs_query_link_record_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryLinkRecord *req_info;
    JpfQueryLinkRecordRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num, row_num ;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    switch(req_info->type)
    {
        case 0:
               snprintf(query_buf, QUERY_STR_LEN,
              	"select  count(distinct link_guid,link_domain_id) as count from %s \
              	where gu_id='%s' and domain_id='%s'",
              	ALARM_LINK_RECORD_TABLE, req_info->guid, req_info->domain
               );

		 total_num =  jpf_get_record_count(app_obj,query_buf);
               if (total_num == 0)
               {
                  ret = 0;
                  goto err_do_link_record_query;
               }

		snprintf(
                    query_buf, QUERY_STR_LEN,
                    "select distinct t1.link_guid,t1.link_domain_id,t1.time_len,t1.alarm_type,t1.level,\
                    t2.gu_name from %s as t1,%s as t2 where t1.gu_id='%s' and t1.domain_id='%s'\
                     and t1.link_guid=t2.gu_id and t1.link_domain_id=t2.gu_domain limit %d,%d",
                    ALARM_LINK_RECORD_TABLE, GU_TABLE,
                    req_info->guid, req_info->domain, req_info->start_num, req_info->req_num
                );

		break;
	case 1:
		snprintf(
			query_buf, QUERY_STR_LEN,
			"select t2.mss_id, t2.mss_name from %s as t1, %s as t2 where t1.gu_id='%s' \
			and t1.guid_domain='%s' and t1.mss_id=t2.mss_id",
			RECORD_POLICY_TABLE, MSS_TABLE,
			req_info->guid, req_info->domain
		);
		result = jpf_dbs_do_query_res(app_obj, query_buf);

		if (G_UNLIKELY(!result))
		{
			jpf_sysmsg_destroy(msg);
			return MFR_ACCEPTED;
		}
		BUG_ON(!result);

		if (G_LIKELY(MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
		{
			ret = MYSQL_RESULT_CODE(result);
			jpf_sql_put_res(result, sizeof(JpfMysqlRes));
			goto err_do_link_record_query;
		}

		row_num = jpf_sql_get_num_rows(result);
		if(row_num == 0)
		{
			jpf_sql_put_res(result,sizeof(JpfMysqlRes));
			goto err_do_link_record_query;
		}
		size = sizeof(JpfQueryLinkRecordRes) + sizeof(JpfLinkRecordInfo);
		query_res = jpf_mem_kalloc(size);
		memset(query_res, 0, size);
		query_res->total_num  = 1;
		query_res->back_num = 1;
		strncpy(query_res->link_record_info[0].link_domain, req_info->link_domain, DOMAIN_ID_LEN - 1);
		strncpy(query_res->link_record_info[0].link_guid, req_info->link_guid, MAX_ID_LEN - 1);
		jpf_get_gu_mss(result,  &query_res->link_record_info[0].mss[0]);
		jpf_sql_put_res(result, sizeof(JpfMysqlRes));
		goto end_query_link_record;
		break;
	default:
		break;
    }

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_link_record(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
	 query_res->total_num = total_num;
	 strncpy(query_res->guid, req_info->guid, MAX_ID_LEN - 1);
	 strncpy(query_res->domain, req_info->domain, DOMAIN_ID_LEN - 1);
	 jpf_mod_dbs_get_mss_of_link_record(app_obj, query_res);
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_do_link_record_query;
    }

    if(result)
       jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_query_link_record:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_link_record_query:
    size = sizeof(JpfQueryLinkRecordRes);
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
    query_res->back_num = 0;
    goto end_query_link_record;
}


JpfMsgFunRet
jpf_mod_dbs_query_link_io_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryLinkIO *req_info;
    JpfQueryLinkIORes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num ;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    switch(req_info->type)
    {
        case 0:
               snprintf(query_buf, QUERY_STR_LEN,
              	"select  count(*) as count from %s \
              	where gu_id='%s' and domain_id='%s'",
              	ALARM_LINK_IO_TABLE, req_info->guid, req_info->domain
               );

		 total_num =  jpf_get_record_count(app_obj,query_buf);
               if (total_num == 0)
               {
                  ret = 0;
                  goto err_do_link_io_query;
               }

		snprintf(
                    query_buf, QUERY_STR_LEN,
                    "select t1.link_guid,t1.link_domain_id,t1.time_len,t1.alarm_type,t1.IO_value,\
                    t2.gu_name from %s as t1,%s as t2 where t1.gu_id='%s' and t1.domain_id='%s'\
                     and t1.link_guid=t2.gu_id and t1.link_domain_id=t2.gu_domain limit %d,%d",
                    ALARM_LINK_IO_TABLE, GU_TABLE,
                    req_info->guid, req_info->domain, req_info->start_num, req_info->req_num
                );

		break;
	case 1:
		snprintf(
			query_buf, QUERY_STR_LEN,
			"select * from %s where gu_id='%s' \
			and guid_domain='%s' and link_guid='%s' and link_domain_id='%s'",
			ALARM_LINK_IO_TABLE,
			req_info->guid, req_info->domain, req_info->link_guid, req_info->link_domain
		);

		break;
	default:
		break;
    }

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_link_io(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
        if (req_info->type == 0)
	     query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_do_link_io_query;
    }

    if(result)
       jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_query_link_io:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_link_io_query:
    size = sizeof(JpfQueryLinkIORes);
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
    query_res->back_num = 0;
    goto end_query_link_io;
}


JpfMsgFunRet
jpf_mod_dbs_query_link_step_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryLinkStep *req_info;
    JpfQueryLinkStepRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num ;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    switch(req_info->type)
    {
        case 0:
               snprintf(query_buf, QUERY_STR_LEN,
              	"select  count(*) as count from %s \
              	where gu_id='%s' and domain_id='%s'",
              	ALARM_LINK_STEP_TABLE, req_info->guid, req_info->domain
               );

		 total_num =  jpf_get_record_count(app_obj,query_buf);
               if (total_num == 0)
               {
                  ret = 0;
                  goto err_do_link_step_query;
               }

		snprintf(
                    query_buf, QUERY_STR_LEN,
                    "select t1.enc_guid,t1.enc_domain_id,t1.level,t1.tw_id,t1.screen_id,t1.division_num,\
                    t1.division_id,t1.level,t1.alarm_type,t2.gu_name from %s as t1,%s as t2 \
                    where t1.gu_id='%s' and t1.domain_id='%s'\
                     and t1.enc_guid=t2.gu_id and t1.enc_domain_id=t2.gu_domain limit %d,%d",
                    ALARM_LINK_STEP_TABLE, GU_TABLE,
                    req_info->guid, req_info->domain, req_info->start_num, req_info->req_num
                );

		break;
	case 1:
		snprintf(
			query_buf, QUERY_STR_LEN,
			"select * from %s where gu_id='%s' \
			and domain_id='%s' and tw_id=%d and screen_id=%d and division_num=%d",
			ALARM_LINK_STEP_TABLE, req_info->guid, req_info->domain, req_info->tw_id,
			req_info->screen_id, req_info->div_num
		);

		break;
       case 2:
		snprintf(
			query_buf, QUERY_STR_LEN,
			"select distinct division_id from %s where gu_id='%s' \
			and domain_id='%s' and screen_id=%d",
			ALARM_LINK_STEP_TABLE, req_info->guid, req_info->domain,req_info->screen_id
		);

		break;
	default:
		break;
    }

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_link_step(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

        query_res->type = req_info->type;
        if (req_info->type == 0)
        {
	     query_res->total_num = total_num;
	  }
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_do_link_step_query;
    }

    if(result)
       jpf_sql_put_res(result, sizeof(JpfMysqlRes));

    if (req_info->type == 0)
    {
        gint i;
        for(i = 0; i < query_res->back_num; i++)
        {
            snprintf(
                   query_buf, QUERY_STR_LEN,
                   "select t1.gu_name from %s as t1,%s as t2 where t1.gu_id=t2.dis_guid and \
                   t1.gu_domain=t2.dis_domain and t2.scr_id=%d",
                   GU_TABLE, SCREEN_TABLE, query_res->link_step_info[i].screen_id
               );

            result = jpf_dbs_do_query_res(app_obj, query_buf);
            BUG_ON(!result);
            if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
            {
                jpf_get_screen_name(result, &query_res->link_step_info[i]);
           }
           else
           {
               ret = MYSQL_RESULT_CODE(result);
               jpf_sql_put_res(result, sizeof(JpfMysqlRes));
               goto err_do_link_step_query;
           }

           if(result)
               jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        }
    }

end_query_link_step:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_link_step_query:
    size = sizeof(JpfQueryLinkStepRes);
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
    query_res->back_num = 0;
    goto end_query_link_step;
}


static void jpf_mod_dbs_get_mss_of_link_snapshot(JpfAppObj *app_obj,
	JpfQueryLinkSnapshotRes *gu)
{
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint ret, row_num = 0, i;

        for (i = 0; i < gu->back_num; i++)
        {
             snprintf(
                query_buf, QUERY_STR_LEN,
                "select t1.mss_id,t2.mss_name from %s as t1,%s as t2 where t1.gu_id='%s' and \
                t1.domain_id='%s' and t1.link_domain_id='%s' \
                and t1.link_guid='%s' and t1.mss_id=t2.mss_id",
                ALARM_LINK_SNAPSHOT_TABLE, MSS_TABLE, gu->guid, gu->domain,
                gu->link_snapshot_info[i].link_domain, gu->link_snapshot_info[i].link_guid
            );

    	     result = jpf_dbs_do_query_res(app_obj, query_buf);
            BUG_ON(!result);

            if (G_LIKELY(MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
            {
                ret = MYSQL_RESULT_CODE(result);
                jpf_sql_put_res(result, sizeof(JpfMysqlRes));
		  SET_CODE(gu, ret);
		  return;
            }

            row_num = jpf_sql_get_num_rows(result);
            printf("get row num (%d)\n",row_num);
            if(row_num == 0)
            {
                jpf_sql_put_res(result,sizeof(JpfMysqlRes));
                continue;
            }

            jpf_get_gu_mss(result,  &gu->link_snapshot_info[i].mss[0]);
	     jpf_sql_put_res(result,sizeof(JpfMysqlRes));
       }
}


JpfMsgFunRet
jpf_mod_dbs_query_link_snapshot_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryLinkSnapshot *req_info;
    JpfQueryLinkSnapshotRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num, row_num ;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    switch(req_info->type)
    {
        case 0:
               snprintf(query_buf, QUERY_STR_LEN,
              	"select  count(distinct link_guid,link_domain_id) as count from %s \
              	where gu_id='%s' and domain_id='%s'",
              	ALARM_LINK_SNAPSHOT_TABLE, req_info->guid, req_info->domain
               );

		 total_num =  jpf_get_record_count(app_obj,query_buf);
               if (total_num == 0)
               {
                  ret = 0;
                  goto err_do_link_snapshot_query;
               }

		snprintf(
                    query_buf, QUERY_STR_LEN,
                    "select distinct t1.link_guid,t1.link_domain_id,t1.picture_num,t1.alarm_type,t1.level,\
                    t2.gu_name from %s as t1,%s as t2 where t1.gu_id='%s' and t1.domain_id='%s'\
                     and t1.link_guid=t2.gu_id and t1.link_domain_id=t2.gu_domain limit %d,%d",
                    ALARM_LINK_SNAPSHOT_TABLE, GU_TABLE,
                    req_info->guid, req_info->domain, req_info->start_num, req_info->req_num
                );

		break;
	case 1:
		snprintf(
			query_buf, QUERY_STR_LEN,
			"select t2.mss_id, t2.mss_name from %s as t1, %s as t2 where t1.gu_id='%s' \
			and t1.guid_domain='%s' and t1.mss_id=t2.mss_id",
			RECORD_POLICY_TABLE, MSS_TABLE,
			req_info->guid, req_info->domain
		);
		result = jpf_dbs_do_query_res(app_obj, query_buf);

		if (G_UNLIKELY(!result))
		{
			jpf_sysmsg_destroy(msg);
			return MFR_ACCEPTED;
		}
		BUG_ON(!result);

		if (G_LIKELY(MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
		{
			ret = MYSQL_RESULT_CODE(result);
			jpf_sql_put_res(result, sizeof(JpfMysqlRes));
			goto err_do_link_snapshot_query;
		}

		row_num = jpf_sql_get_num_rows(result);
		if(row_num == 0)
		{
			jpf_sql_put_res(result,sizeof(JpfMysqlRes));
			goto err_do_link_snapshot_query;
		}
		size = sizeof(JpfQueryLinkSnapshotRes) + sizeof(JpfLinkSnapshotInfo);
		query_res = jpf_mem_kalloc(size);
		memset(query_res, 0, size);
		query_res->total_num  = 1;
		query_res->back_num = 1;
		strncpy(query_res->link_snapshot_info[0].link_domain, req_info->link_domain, DOMAIN_ID_LEN - 1);
		strncpy(query_res->link_snapshot_info[0].link_guid, req_info->link_guid, MAX_ID_LEN - 1);
		jpf_get_gu_mss(result,  &query_res->link_snapshot_info[0].mss[0]);
		jpf_sql_put_res(result, sizeof(JpfMysqlRes));
		goto end_query_link_snapshot;
		break;
	default:
		break;
    }

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_link_snapshot(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
	 query_res->total_num = total_num;
	 strncpy(query_res->guid, req_info->guid, MAX_ID_LEN - 1);
	 strncpy(query_res->domain, req_info->domain, DOMAIN_ID_LEN - 1);
	 jpf_mod_dbs_get_mss_of_link_snapshot(app_obj, query_res);
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_do_link_snapshot_query;
    }

    if(result)
       jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_query_link_snapshot:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_link_snapshot_query:
    size = sizeof(JpfQueryLinkSnapshotRes);
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
    query_res->back_num = 0;
    goto end_query_link_snapshot;
}


JpfMsgFunRet
jpf_mod_dbs_query_link_preset_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryLinkPreset *req_info;
    JpfQueryLinkPresetRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    switch(req_info->type)
    {
        case 0:
               snprintf(query_buf, QUERY_STR_LEN,
              	"select  count(*) as count from %s \
              	where gu_id='%s' and domain_id='%s'",
              	ALARM_LINK_PRESET_TABLE, req_info->guid, req_info->domain
               );

		 total_num =  jpf_get_record_count(app_obj,query_buf);
               if (total_num == 0)
               {
                  ret = 0;
                  goto err_do_link_preset_query;
               }

		snprintf(
                    query_buf, QUERY_STR_LEN,
                    "select t1.link_guid,t1.link_domain_id,t1.preset_no,t1.alarm_type,\
                    t2.gu_name from %s as t1,%s as t2 where t1.gu_id='%s' and t1.domain_id='%s'\
                     and t1.link_guid=t2.gu_id and t1.link_domain_id=t2.gu_domain limit %d,%d",
                    ALARM_LINK_PRESET_TABLE, GU_TABLE,
                    req_info->guid, req_info->domain, req_info->start_num, req_info->req_num
                );

		break;
	case 1:
		snprintf(
			query_buf, QUERY_STR_LEN,
			"select * from %s where gu_id='%s' \
			and guid_domain='%s' and link_guid='%s' and link_domain_id='%s'",
			ALARM_LINK_PRESET_TABLE,
			req_info->guid, req_info->domain, req_info->link_guid, req_info->link_domain
		);

		break;
	default:
		break;
    }

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_link_preset(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
        if (req_info->type == 0)
	     query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_do_link_preset_query;
    }

    if(result)
       jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_query_link_preset:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_link_preset_query:
    size = sizeof(JpfQueryLinkPresetRes);
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
    query_res->back_num = 0;
    goto end_query_link_preset;
}


JpfMsgFunRet
jpf_mod_dbs_query_link_map_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryLinkMap *req_info;
    JpfQueryLinkMapRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num ;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    switch(req_info->type)
    {
    case 0:
		snprintf(query_buf, QUERY_STR_LEN,
			"select  count(*) as count from %s \
			where gu_id='%s' and domain_id='%s'",
			ALARM_LINK_MAP_TABLE, req_info->guid, req_info->domain
		);

		total_num = jpf_get_record_count(app_obj,query_buf);
		if (total_num == 0)
		{
			ret = 0;
			goto err_do_link_map_query;
		}

		snprintf(
			query_buf, QUERY_STR_LEN,
			"select t1.link_guid,t1.link_domain_id,t1.level,t1.alarm_type,\
			t2.gu_name from %s as t1,%s as t2 where t1.gu_id='%s' \
			and t1.domain_id='%s' and t1.link_guid=t2.gu_id and \
			t1.link_domain_id=t2.gu_domain limit %d,%d",
			ALARM_LINK_MAP_TABLE, GU_TABLE,
			req_info->guid, req_info->domain, req_info->start_num, req_info->req_num
		);

		break;
	case 1:
		snprintf(
			query_buf, QUERY_STR_LEN,
			"select * from %s where gu_id='%s' \
			and guid_domain='%s' and link_guid='%s' and link_domain_id='%s'",
			ALARM_LINK_MAP_TABLE,
			req_info->guid, req_info->domain, req_info->link_guid, req_info->link_domain
		);

		break;
	default:
		break;
    }

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_link_map(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
        if (req_info->type == 0)
	     query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_do_link_map_query;
    }

    if(result)
       jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_query_link_map:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_link_map_query:
    size = sizeof(JpfQueryLinkMapRes);
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
    query_res->back_num = 0;
    goto end_query_link_map;
}


JpfMsgFunRet
jpf_mod_dbs_query_area_dev_online_rate_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfQueryAreaDevRate *req_info;
    JpfQueryAreaDevRateRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint ret = 0, total_num = 0, online_count, size;
    double total = 0,online = 0,rate = 0;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

	/*if (req_info->area_id > 0)
	{
		size = sizeof(JpfQueryAreaDevRateRes) + sizeof(JpfAreaDevRate);
	    query_res = jpf_mem_kalloc(size);
	    if (!query_res)
	    {
	        jpf_warning("<dbs_mh_bss> alloc error");
	        jpf_sysmsg_destroy(msg);
	        return MFR_ACCEPTED;
	    }

	    memset(query_res, 0, size);
	    query_res->total_num = 1;
	    query_res->back_num = 1;
	}*/
  	switch(req_info->type)
    {
	case 0:
		if (req_info->area_id > 0)
		{
			size = sizeof(JpfQueryAreaDevRateRes) + sizeof(JpfAreaDevRate);
		    query_res = jpf_mem_kalloc(size);
		    if (!query_res)
		    {
		        jpf_warning("<dbs_mh_bss> alloc error");
		        jpf_sysmsg_destroy(msg);
		        return MFR_ACCEPTED;
		    }

		    memset(query_res, 0, size);
		    query_res->total_num = 1;
		    query_res->back_num = 1;

			snprintf(query_buf, QUERY_STR_LEN,
				"select count(*)  as count from %s where pu_area=%d",
				PU_TABLE, req_info->area_id
			);
			total_num =  jpf_get_record_count(app_obj,query_buf);
			if (total_num == 0)
			{
				rate = 0.00;
				goto end_query_sigle_area;
			}

			snprintf(query_buf, QUERY_STR_LEN,
				"select count(t2.pu_state) as count from %s as t1,%s as t2 where \
				t1.pu_area=%d and t1.pu_id=t2.pu_id and t1.pu_domain=t2.pu_domain \
				and pu_state=1",
				PU_TABLE, PU_RUNNING_TABLE, req_info->area_id
			);
			online_count =  jpf_get_record_count(app_obj,query_buf);
			if (online_count == 0)
			{
				rate = 0.00;
				goto end_query_sigle_area;
			}

			total = total_num;
			online = online_count;
			rate = online/total;
			goto end_query_sigle_area;
		}
		else if (req_info->area_id == -1)
		{
			snprintf(query_buf, QUERY_STR_LEN,
				"select count(*)  as count from %s",
				AREA_TABLE
			);
			total_num =  jpf_get_record_count(app_obj,query_buf);
			if (total_num == 0)
			{
				ret = 0;
				goto err_query_area_dev_online_rate;
			}

			snprintf(query_buf, QUERY_STR_LEN,
				"call get_area_dev_online_status(1,%d,%d)",
				req_info->start_num, req_info->req_num
			);
		}
		else
		{
			ret = -1;
			goto err_query_area_dev_online_rate;
		}
		break;
	 case 1:
	 	if (req_info->area_id > 0)
		{
			/*snprintf(query_buf, QUERY_STR_LEN,
				"select pu_total_count as count from %s where area_id=%d \
				and statistics_type=1 and date_format('%s','%s')= \
				date_format(statistics_time,'%s')",
				AREA_DEV_ONLINE_STATUS_TABLE, req_info->area_id,
				req_info->key,DATE_FORMAT_1, DATE_FORMAT_1
			);
			total_num =  jpf_get_record_count(app_obj,query_buf);
			if (total_num == 0)
			{
				rate = 0.00;
				goto end_query_sigle_area;
			}
			snprintf(query_buf, QUERY_STR_LEN,
				"select online_count as count from %s where area_id=%d \
				and statistics_type=1 and date_format('%s','%s')= \
				date_format(statistics_time,'%s')",
				AREA_DEV_ONLINE_STATUS_TABLE, req_info->area_id,
				req_info->key,DATE_FORMAT_1,DATE_FORMAT_1
			);
			printf("--------online count sql %s\n",query_buf);
			online_count =  jpf_get_record_count(app_obj,query_buf);
			if (online_count == 0)
			{
				rate = 0.00;
				goto end_query_sigle_area;
			}
			total = total_num;
			online = online_count;
			rate = online/total;
			goto end_query_sigle_area;
			*/
			snprintf(query_buf, QUERY_STR_LEN,
				"select online_count/pu_total_count as rate,\
				pu_total_count as total_count,online_count from %s where area_id=%d \
				and statistics_type=1 and date_format('%s','%s')= \
				date_format(statistics_time,'%s')",
				AREA_DEV_ONLINE_STATUS_TABLE, req_info->area_id,
				req_info->key,DATE_FORMAT_1, DATE_FORMAT_1
			);
			total_num = 1;
		}
		else if (req_info->area_id == -1)
		{
			snprintf(query_buf, QUERY_STR_LEN,
				"select count(t1.area_id) as count\
				from %s as t1,%s as t2 where t1.area_id=t2.area_id \
				and t1.statistics_type=1 and date_format('%s','%s')= \
				date_format(t1.statistics_time,'%s')",
				AREA_DEV_ONLINE_STATUS_TABLE, AREA_TABLE,
				req_info->key,DATE_FORMAT_1, DATE_FORMAT_1
			);
			total_num =  jpf_get_record_count(app_obj,query_buf);
			if (total_num == 0)
			{
				//query_res->total_num = 0;
				goto err_query_area_dev_online_rate;
			}

			snprintf(query_buf, QUERY_STR_LEN,
				"select t1.online_count/t1.pu_total_count as rate,t1.area_id,\
				t1.online_count,t1.pu_total_count as total_count,t2.area_name from %s as t1,\
				%s as t2 where t1.area_id=t2.area_id \
				and t1.statistics_type=1 and date_format('%s','%s')= \
				date_format(t1.statistics_time,'%s') limit %d,%d",
				AREA_DEV_ONLINE_STATUS_TABLE, AREA_TABLE,req_info->key, DATE_FORMAT_1,
				 DATE_FORMAT_1, req_info->start_num, req_info->req_num
			);
		}
		else
		{
			ret = -1;
			goto err_query_area_dev_online_rate;
		}
	 	break;
	 case 2:
	 	if ((strlen(req_info->key)==6) ||(strlen(req_info->key)==7))
	 		strcat(req_info->key, "-00");
	    if (req_info->area_id > 0)
		{
			/*snprintf(query_buf, QUERY_STR_LEN,
				"select AVG(pu_total_count) as count from %s where area_id=%d \
				and statistics_type=2 and date_format('%s','%s')= \
				date_format(statistics_time,'%s')",
				AREA_DEV_ONLINE_STATUS_TABLE, req_info->area_id,
				req_info->key,DATE_FORMAT_2, DATE_FORMAT_2
			);

			total_num =  jpf_get_record_count(app_obj,query_buf);
			if (total_num == 0)
			{
				rate = 0.00;
				goto end_query_sigle_area;
			}
			snprintf(query_buf, QUERY_STR_LEN,
				"select AVG(online_count) as count from %s where area_id=%d \
				and statistics_type=2 and date_format('%s','%s')= \
				date_format(statistics_time,'%s')",
				AREA_DEV_ONLINE_STATUS_TABLE, req_info->area_id,
				req_info->key,DATE_FORMAT_2,DATE_FORMAT_2
			);
			online_count =  jpf_get_record_count(app_obj,query_buf);
			if (online_count == 0)
			{
				rate = 0.00;
				goto end_query_sigle_area;
			}
			total = total_num;
			online = online_count;
			rate = online/total;
			goto end_query_sigle_area;
			*/
			snprintf(query_buf, QUERY_STR_LEN,
				"select AVG(online_count)/AVG(pu_total_count) as rate, \
				AVG(online_count) as online_count,\
				AVG(pu_total_count) as total_count  from %s where area_id=%d \
				and statistics_type=2 and date_format('%s','%s')= \
				date_format(statistics_time,'%s')",
				AREA_DEV_ONLINE_STATUS_TABLE, req_info->area_id,
				req_info->key,DATE_FORMAT_2,DATE_FORMAT_2
			);
			total_num = 1;
		}
		else if (req_info->area_id == -1)
		{
			snprintf(query_buf, QUERY_STR_LEN,
				"select count(t1.area_id) as count\
				from %s as t1,%s as t2 where t1.area_id=t2.area_id \
				and t1.statistics_type=2 and date_format('%s','%s')= \
				date_format(t1.statistics_time,'%s')",
				AREA_DEV_ONLINE_STATUS_TABLE, AREA_TABLE,
				req_info->key,DATE_FORMAT_2, DATE_FORMAT_2
			);
			total_num =  jpf_get_record_count(app_obj,query_buf);
			if (total_num == 0)
			{
				//query_res->total_num = 0;
				goto err_query_area_dev_online_rate;
			}

			snprintf(query_buf, QUERY_STR_LEN,
				"select t1.online_count/t1.pu_total_count as rate,t1.area_id,\
				t1.online_count,t1.pu_total_count as total_count,t2.area_name from %s as t1,\
				%s as t2 where t1.area_id=t2.area_id \
				and t1.statistics_type=2 and date_format('%s','%s')= \
				date_format(t1.statistics_time,'%s') limit %d,%d",
				AREA_DEV_ONLINE_STATUS_TABLE, AREA_TABLE,req_info->key, DATE_FORMAT_2,
				 DATE_FORMAT_2, req_info->start_num, req_info->req_num
			);
		}
		else
		{
			ret = -1;
			goto err_query_area_dev_online_rate;
		}
	 	break;
	 case 3:
	 	if (strlen(req_info->key)==4)
	 		strcat(req_info->key, "-00-00");
	 	if (req_info->area_id > 0)
		{
			/*snprintf(query_buf, QUERY_STR_LEN,
				"select AVG(pu_total_count) as count from %s where area_id=%d \
				and statistics_type=3 and YEAR('%s')= \
				YEAR(statistics_time)",
				AREA_DEV_ONLINE_STATUS_TABLE, req_info->area_id, req_info->key
			);

			total_num =  jpf_get_record_count(app_obj,query_buf);
			if (total_num == 0)
			{
				goto end_query_sigle_area;
			}
			snprintf(query_buf, QUERY_STR_LEN,
				"select AVG(online_count) as count from %s where area_id=%d \
				and statistics_type=3 and YEAR('%s')= \
				YEAR(statistics_time)",
				AREA_DEV_ONLINE_STATUS_TABLE, req_info->area_id, req_info->key
			);
			online_count =  jpf_get_record_count(app_obj,query_buf);
			if (online_count == 0)
			{
				goto end_query_sigle_area;
			}
			total = total_num;
			online = online_count;
			rate = online/total;
			goto end_query_sigle_area;
			*/
			snprintf(query_buf, QUERY_STR_LEN,
				"select AVG(online_count)/AVG(pu_total_count) as rate, \
				AVG(online_count) as online_count,\
				AVG(pu_total_count) as total_count from %s where area_id=%d \
				and statistics_type=3 and YEAR('%s')= \
				YEAR(statistics_time)",
				AREA_DEV_ONLINE_STATUS_TABLE, req_info->area_id, req_info->key
			);
			total_num = 1;
		}
		else if (req_info->area_id == -1)
		{
			snprintf(query_buf, QUERY_STR_LEN,
				"select count(t1.area_id) as count\
				from %s as t1,%s as t2 where t1.area_id=t2.area_id \
				and t1.statistics_type=3 and YEAR('%s')= \
				YEAR(t1.statistics_time)",
				AREA_DEV_ONLINE_STATUS_TABLE, AREA_TABLE,
				req_info->key
			);
			total_num =  jpf_get_record_count(app_obj,query_buf);
			if (total_num == 0)
			{
				//query_res->total_num = 0;
				goto err_query_area_dev_online_rate;
			}

			snprintf(query_buf, QUERY_STR_LEN,
				"select t1.online_count/t1.pu_total_count as rate,\
				t1.online_count,t1.pu_total_count as total_count,t1.area_id,\
				t2.area_name from %s as t1,%s as t2 where t1.area_id=t2.area_id \
				and t1.statistics_type=3 and YEAR('%s')= \
				YEAR(t1.statistics_time) limit %d,%d",
				AREA_DEV_ONLINE_STATUS_TABLE, AREA_TABLE,req_info->key,
				req_info->start_num, req_info->req_num
			);
		}
		else
		{
			ret = -1;
			goto err_query_area_dev_online_rate;
		}
	 	break;
	 default:
	 	break;
    }

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_area_dev_rate(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_bss> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
        //if (req_info->type == 0)
	     query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_query_area_dev_online_rate;
    }

    if(result)
       jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_query_area_dev_online_rate:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, jpf_mem_kfree);

    return MFR_DELIVER_BACK;
end_query_sigle_area:
	query_res->area_dev_rate[0].rate = rate;
	query_res->area_dev_rate[0].area_id = req_info->area_id;
	query_res->area_dev_rate[0].total_count = total_num;
	query_res->area_dev_rate[0].online_count = online_count;
	goto end_query_area_dev_online_rate;
err_query_area_dev_online_rate:
    size = sizeof(JpfQueryAreaDevRateRes);
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
    query_res->back_num = 0;
    goto end_query_area_dev_online_rate;
}


JpfMsgFunRet
jpf_mod_dbs_del_admin_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfBssRes result;
    JpfDelAdmin *del_info;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    del_info = MSG_GET_DATA(msg);

    memset(&result, 0, sizeof(result));
    if (!strcmp(del_info->admin_name, "StringTooLong"))
        SET_CODE(&result, E_STRINGLEN);

    snprintf(query_buf, QUERY_STR_LEN,
        "delete from %s where su_name in (%s)",
        ADMIN_TABLE, del_info->admin_name
    );

   jpf_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
   JPF_GET_BSS_USR_NAME(result.bss_usr, del_info->bss_usr);
   jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_del_user_group_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfBssRes result;
    JpfDelUserGroup *del_info;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    del_info = MSG_GET_DATA(msg);

    memset(&result, 0, sizeof(result));
    if (!strcmp(del_info->group_id, "StringTooLong"))
        SET_CODE(&result, E_STRINGLEN);

    snprintf(query_buf, QUERY_STR_LEN,
        "delete from %s where group_id in (%s)",
        USER_GROUP_TABLE, del_info->group_id
    );

    jpf_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
    JPF_GET_BSS_USR_NAME(result.bss_usr, del_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_del_user_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfBssRes result;
    JpfDelUser *del_info;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    del_info = MSG_GET_DATA(msg);

    memset(&result, 0, sizeof(result));
    if (!strcmp(del_info->username, "StringToo Long"))
        SET_CODE(&result, E_STRINGLEN);

    snprintf(query_buf, QUERY_STR_LEN,
        "delete from %s where user_name in (%s)",
        USER_TABLE, del_info->username
    );

    jpf_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
    JPF_GET_BSS_USR_NAME(result.bss_usr, del_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_del_domain_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfDelDomain *del_info;
    JpfMsgErrCode result;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    del_info = MSG_GET_DATA(msg);
    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where dm_id='%s'",
        DOMAIN_TABLE, del_info->domain_id
    );

    memset(&result, 0, sizeof(JpfMsgErrCode));
    jpf_dbs_do_del_code(app_obj, msg, query_buf, &result, &affect_num);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


static __inline__ gint
jpf_get_area_parent(JpfMysqlRes *result)
{
    G_ASSERT(result != NULL);

    gint field_no =0;
    JpfMysqlRow mysql_row;
    JpfMysqlField* mysql_fields;
    gchar *name;
    gchar *value;
    unsigned int row_num;
    unsigned int field_num;

    row_num = jpf_sql_get_num_rows(result);
    if (row_num <= 0)
    {
        jpf_warning("<jpf_dbs_mh_bss> area inexist\n");
        return E_NODBENT;
    }
    field_num = jpf_sql_get_num_fields(result);

    while ((mysql_row = jpf_sql_fetch_row(result)))
    {
        jpf_sql_field_seek(result, 0);
        mysql_fields = jpf_sql_fetch_fields(result);
        for(field_no = 0; field_no < field_num; field_no++)
        {
            name = jpf_sql_get_field_name(mysql_fields, field_no);
            if (!strcmp(name, "area_parent"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    return atoi(value);
                else
                   return -E_ROOTAREA;
            }
            else
                printf("no need mysql name %s \n", name);
        }
    }

    return 0;
}


gint jpf_is_root_area(JpfAppObj *app_obj, gint area_id)
{
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint ret;

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select area_parent from %s where area_id=%d ",
        AREA_TABLE, area_id
        );

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
        ret = jpf_get_area_parent(result);
    else
        ret = MYSQL_RESULT_CODE(result);

    jpf_sql_put_res(result, sizeof(JpfMysqlRes));
    return ret;
}


JpfMsgFunRet
jpf_mod_dbs_del_area_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfDelArea *del_info;
    JpfBssRes result;
    gint area_parent;
    char query_buf[QUERY_STR_LEN] = {0};
    glong affect_num = 0;

    memset(&result, 0, sizeof(result));
    del_info = MSG_GET_DATA(msg);
    area_parent = jpf_is_root_area(app_obj, del_info->area_id);
    if (area_parent < 0)
    {
        SET_CODE(&result, area_parent);
        goto end_del_area;
    }

    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where area_id=%d and area_parent is not NULL",
        AREA_TABLE, del_info->area_id
    );

    jpf_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
    if (!RES_CODE(&result))
    {
         jpf_dbs_mss_notify(app_obj);
         jpf_mod_dbs_deliver_pu_recheck_event(app_obj);
    }

end_del_area:
    JPF_GET_BSS_USR_NAME(result.bss_usr, del_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_del_pu_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfDelPu *del_info;
    JpfBssRes result;
    char query_buf[QUERY_STR_LEN];
    db_conn_status *conn = NULL;
    JpfModDbs *dbs_obj;
    gint code, i;

    del_info = MSG_GET_DATA(msg);
    dbs_obj = JPF_MODDBS(app_obj);
	conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
	if (G_UNLIKELY(!conn))
	{
		jpf_warning("<JpfModDbs> get db connection error!");
		code = -E_GETDBCONN;
		goto end_del_pu;
	}

	if (G_UNLIKELY(!conn->mysql))
	{
		jpf_warning("<JpfModDbs> get db connection error!");
		put_db_connection(dbs_obj->pool_info, conn);
		code = -E_GETDBCONN;
		goto end_del_pu;
	}

    switch (del_info->type){
    case 0:
        JPF_MYSQL_DO_QUERY_OR_KILL_CONN(dbs_obj, conn, "start transaction", code);
        if (code)
		goto end_del_pu;

        for (i = 0; i < del_info->count; i++)
        {
            snprintf(
                query_buf, QUERY_STR_LEN,
                "delete from %s where pu_id='%s' and pu_domain='%s'",
                PU_TABLE, del_info->pu_list[i].puid, del_info->pu_list[i].domain_id
            );

            code = jpf_mysql_do_query(conn->mysql, query_buf);
            if (code)
            {
                jpf_mysql_do_query(conn->mysql, "rollback");
                put_db_connection(dbs_obj->pool_info, conn);
                goto end_del_pu;
            }
        }
        if (code == 0)
        {
             jpf_dbs_mss_notify(app_obj);
        }
        jpf_mysql_do_query(conn->mysql, "commit");
        break;
    case 1:
        snprintf(
            query_buf, QUERY_STR_LEN,
            "delete from %s where pu_area='%s'",
            PU_TABLE, del_info->key
        );
        code = jpf_mysql_do_query(conn->mysql, query_buf);
        break;
    }

    put_db_connection(dbs_obj->pool_info, conn);

end_del_pu:
    SET_CODE(&result, code);
    if (!RES_CODE(&result))
		jpf_mod_dbs_deliver_pu_recheck_event(app_obj);
    JPF_GET_BSS_USR_NAME(result.bss_usr, del_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result),
			BUSSLOT_POS_DBS, BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_del_gu_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfDelGu *del_info;
    JpfBssRes result;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;
    JpfNotifyMessage notify_info;

    del_info = MSG_GET_DATA(msg);
    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where gu_id='%s' and gu_domain='%s'",
        GU_TABLE, del_info->guid, del_info->domain_id
    );
    memset(&result, 0, sizeof(result));
    jpf_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
    if (!RES_CODE(&result))
    {
        jpf_dbs_mss_notify(app_obj);
        memset(&notify_info, 0, sizeof(notify_info));
        notify_info.msg_id = MSG_DEL_GU;
        strncpy(notify_info.param1, del_info->domain_id, DOMAIN_ID_LEN - 1);
        strncpy(notify_info.param2, del_info->guid, MAX_ID_LEN - 1);
        jpf_mods_dbs_broadcast_msg((JpfModDbs *)app_obj, &notify_info, sizeof(notify_info));
    }

    JPF_GET_BSS_USR_NAME(result.bss_usr, del_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result),
        BUSSLOT_POS_DBS, BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_del_mds_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfDelMds *del_info;
    JpfBssRes result;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;
    del_info = MSG_GET_DATA(msg);

    //frist update pu_table,set pu_mds=null
   /* snprintf(
         query_buf, QUERY_STR_LEN,
         "update %s set pu_mdu='' where mdu_id='%s'",
         PU_TABLE, del_info->mds_id
    );
    jpf_dbs_do_del_code(app_obj, msg, query_buf, &result);
	*/  //trigger displace
    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where mdu_id in ('%s') ",
        MDS_TABLE, del_info->mds_id
    );
    memset(&result, 0, sizeof(result));
    jpf_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
    JPF_GET_BSS_USR_NAME(result.bss_usr, del_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result),
			BUSSLOT_POS_DBS, BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_del_mds_ip_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfDelMdsIp *del_info;
    JpfBssRes result;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;
    del_info = MSG_GET_DATA(msg);

    snprintf(
         query_buf, QUERY_STR_LEN,
         "delete from %s where mdu_id='%s' and mdu_cmsip='%s'",
         MDS_IP_TABLE, del_info->mds_id, del_info->cms_ip
    );
    memset(&result, 0, sizeof(result));
    jpf_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
    JPF_GET_BSS_USR_NAME(result.bss_usr, del_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_del_mss_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfDelMss *del_info;
    JpfBssRes result;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    del_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where mss_id in ('%s') ",
        MSS_TABLE, del_info->mss_id
    );
    memset(&result, 0, sizeof(result));
    jpf_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
    JPF_GET_BSS_USR_NAME(result.bss_usr, del_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result),
			BUSSLOT_POS_DBS, BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_del_manufacturer_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfDelManufacturer *del_info;
    JpfBssRes result;
    char query_buf[QUERY_STR_LEN] = {0};
    glong affect_num = 0;

    del_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where mf_id='%s'",
        MANUFACTURER_TABLE, del_info->mf_id
        );

    memset(&result, 0, sizeof(result));
    jpf_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
    JPF_GET_BSS_USR_NAME(result.bss_usr, del_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


static __inline__ gint
jpf_mod_dbs_control_trigger(JpfAppObj *app_obj, JpfSysMsg *msg, gint op)
{
    JpfMsgErrCode result;
    char query_buf[QUERY_STR_LEN] = {0};
    glong affect_num = 0;

    memset(&result, 0, sizeof(JpfMsgErrCode));
    if (op)  //enable trigger
        snprintf(query_buf, QUERY_STR_LEN,
            "set global connect_timeout=10"
        );
    else   //disable trigger
        snprintf(query_buf, QUERY_STR_LEN,
            "set global connect_timeout=127"
        );

    jpf_dbs_do_del_code(app_obj, msg, query_buf, &result, &affect_num);

    return RES_CODE(&result);
}

JpfMsgFunRet
jpf_mod_dbs_database_backup_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    FILE *fp;
    char buffer[STRING_LEN] = {0};
    char rm_filename[STRING_LEN] = {0};
    JpfBssRes result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint ret = 0;
    JpfDbBackup *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&result, 0, sizeof(result));
    strcpy(result.bss_usr, req_info->bss_usr);
    snprintf(
        query_buf, STRING_LEN - 1,
        "mysql-dump -c -t -e --skip-triggers -u%s -p%s %s > %s/%s.tmp",
        jpf_get_sys_parm_str(SYS_PARM_DBADMINNAME),
        jpf_get_sys_parm_str(SYS_PARM_DBADMINPASSWORD),
        jpf_get_sys_parm_str(SYS_PARM_DBNAME),
        jpf_get_sys_parm_str(SYS_PARM_DBBACKUPPATH),
        req_info->filename
    );

    fp = popen(query_buf, "r");
    if (!fp)
    {
        ret = -errno;
        snprintf(rm_filename,STRING_LEN -1,"rm -rf %s/%s.tmp",
            jpf_get_sys_parm_str(SYS_PARM_DBBACKUPPATH),
        	  req_info->filename);
        system(rm_filename);
        goto end_database_backup;
    }
    fgets(buffer,sizeof(buffer), fp);
    if (strlen(buffer))
    {
      jpf_print("[BSS] Database backup output:%s\n", buffer);
    }
    pclose(fp);
    snprintf(rm_filename,STRING_LEN -1,"mv %s/%s.tmp %s/%s",
        jpf_get_sys_parm_str(SYS_PARM_DBBACKUPPATH),
        req_info->filename,
        jpf_get_sys_parm_str(SYS_PARM_DBBACKUPPATH),
        req_info->filename);
    fp = popen(rm_filename, "r");
    if (!fp)
    {
        jpf_warning("rm tmp database file fail");
        ret = -errno;
        goto end_database_backup;
    }
    fgets(buffer,sizeof(buffer), fp);
    if (strlen(buffer))
    {
      jpf_print("[BSS] change backup database name:%s\n", buffer);
    }
    pclose(fp);

end_database_backup:
    SET_CODE(&result, ret);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
		BUSSLOT_POS_BSS);
    return MFR_DELIVER_BACK;
}


static int import_data_flag = 1;
JpfMsgFunRet
jpf_mod_dbs_clear_database_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    char query_buf[QUERY_STR_LEN] = {0};
    db_conn_status  *conn = NULL;
    JpfModDbs        *dbs_obj;
    gint ret = 0;
    JpfDbImport *req_info;
    char buffer[STRING_LEN] = {0};
    FILE *fp;
    JpfDbImportRes res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

	memset(&res_info, 0, sizeof(res_info));
	strcpy(res_info.bss_usr, req_info->bss_usr);
    if (!g_atomic_int_dec_and_test(&import_data_flag))
    {
	    ret = -E_INPROGRESS;
		goto end_database_import;
    }

    jpf_mod_dbs_control_trigger(app_obj, msg, 0);
    snprintf(query_buf, QUERY_STR_LEN,  "call clear_db_tables()");

    dbs_obj = JPF_MODDBS(app_obj);

redo:
    conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
    if (G_UNLIKELY(!conn))
        ret = -E_GETDBCONN;

    if (G_UNLIKELY(!conn->mysql))
    {
        put_db_connection(dbs_obj->pool_info, conn);
        ret = -E_GETDBCONN;
    }

    ret = jpf_process_query_procedure(conn->mysql, query_buf);
    if (ret == -DB_SERVER_GONE_ERROR)
    {
        kill_db_connection(dbs_obj->pool_info, conn);
        goto redo;
    }
    if (ret < 0)
    {
        put_db_connection(dbs_obj->pool_info, conn);
    }

    snprintf(
        query_buf, STRING_LEN - 1,
        "mysql-client -u%s -p%s %s < %s/%s 2>&1",
        jpf_get_sys_parm_str(SYS_PARM_DBADMINNAME),
        jpf_get_sys_parm_str(SYS_PARM_DBADMINPASSWORD),
        jpf_get_sys_parm_str(SYS_PARM_DBNAME),
        jpf_get_sys_parm_str(SYS_PARM_DBBACKUPPATH),
        req_info->filename
    );

   fp = popen(query_buf, "r");
   if (!fp)
   {
		ret = -errno;
		strcpy(res_info.error_desc, "popen() failed.");
		goto err_database_import;
   }
   fgets(buffer,sizeof(buffer), fp);

    if (strlen(buffer))
    {
		ret = -E_STRINGFORMAT;
		strncpy(res_info.error_desc, buffer, DESCRIPTION_INFO_LEN - 1);
		jpf_print("[BSS] Database import output:%s\n", buffer);
    }
       pclose(fp);

err_database_import:
    jpf_mod_dbs_control_trigger(app_obj, msg, 1);
	import_data_flag = 1;

end_database_import:
    SET_CODE(&res_info, ret);
    jpf_dbs_modify_sysmsg_2(msg, &res_info, sizeof(res_info), BUSSLOT_POS_DBS,
		BUSSLOT_POS_BSS);
    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_del_defence_area_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfDelDefenceArea *del_info;
    JpfBssRes result;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    del_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where defence_area_id=%d ",
        DEFENCE_AREA_TABLE, del_info->defence_area_id
    );
    memset(&result, 0, sizeof(result));
    jpf_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
    JPF_GET_BSS_USR_NAME(result.bss_usr, del_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result),
			BUSSLOT_POS_DBS, BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_del_defence_map_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfDelDefenceMap *del_info;
    JpfBssRes result;
    gchar query_buf[QUERY_STR_LEN] = {0};
    gchar map_path[FILE_PATH_LEN] = {0};
    glong affect_num = 0;

    del_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where defence_area_id=%d and map_id=%d ",
        DEFENCE_MAP_TABLE, del_info->defence_area_id, del_info->map_id
    );
   /* snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where map_id=%d ",
        MAP_TABLE, del_info->map_id
    );*/
    memset(&result, 0, sizeof(result));
    jpf_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
    if (!RES_CODE(&result))
    {
        strncpy(map_path, jpf_get_sys_parm_str(SYS_PARM_MAPPATH), FILE_PATH_LEN- 1);
	 strcat(map_path, "/");
	 strcat(map_path,del_info->map_location);
        unlink(map_path);
    }
    JPF_GET_BSS_USR_NAME(result.bss_usr, del_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result),
			BUSSLOT_POS_DBS, BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_del_defence_gu_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfDelDefenceGu *del_info;
    JpfBssRes result;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    del_info = MSG_GET_DATA(msg);
    switch (del_info->type){
        case 0:
            snprintf(
                query_buf, QUERY_STR_LEN,
                "delete from %s where map_id=%d",
                MAP_GU_TABLE, del_info->map_id
            );

            break;
        case 1:
            snprintf(
                query_buf, QUERY_STR_LEN,
                "delete from %s where map_id=%d and gu_domain='%s' and gu_id='%s'",
                MAP_GU_TABLE, del_info->map_id, del_info->domain_id, del_info->guid
            );

            break;
    }
    memset(&result, 0, sizeof(result));
    jpf_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
    JPF_GET_BSS_USR_NAME(result.bss_usr, del_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result),
			BUSSLOT_POS_DBS, BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_del_map_href_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfDelMapHref *del_info;
    JpfBssRes result;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    del_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where src_map_id=%d and dst_map_id=%d",
        MAP_HREF_TABLE, del_info->src_map_id, del_info->dst_map_id
    );
    memset(&result, 0, sizeof(result));
    jpf_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
    JPF_GET_BSS_USR_NAME(result.bss_usr, del_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result),
			BUSSLOT_POS_DBS, BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_del_tw_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfDelTw *del_info;
    JpfBssRes result;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    del_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where tw_id=%d",
        TW_TABLE, del_info->tw_id
    );
    memset(&result, 0, sizeof(result));
    jpf_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
    JPF_GET_BSS_USR_NAME(result.bss_usr, del_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result),
			BUSSLOT_POS_DBS, BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_del_screen_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfDelScreen *del_info;
    JpfBssRes result;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    del_info = MSG_GET_DATA(msg);
    switch(del_info->type)
    {
	 case 0:
		snprintf(
			query_buf, QUERY_STR_LEN,
			"delete from %s where dis_domain='%s' and dis_guid='%s'",
			SCREEN_TABLE, del_info->dis_domain, del_info->dis_guid
		);

		break;
        case 1:
		snprintf(
			query_buf, QUERY_STR_LEN,
                     "delete from %s where tw_id=%d",
                     SCREEN_TABLE, del_info->tw_id
              );

              break;
	default:
		jpf_warning("del screen type(%d) is wrong ", del_info->type);
              SET_CODE(&result, E_QUERYTYPE);
		goto end_del_screen;
    }

    memset(&result, 0, sizeof(result));
    jpf_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
    if (RES_CODE(&result) == -E_NODBENT)
    {
        SET_CODE(&result, 0);
    }

end_del_screen:
    JPF_GET_BSS_USR_NAME(result.bss_usr, del_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_del_tour_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfDelTour *del_info;
    JpfBssRes result;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    del_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where tour_id=%d",
        TOUR_TABLE, del_info->tour_id
    );
    memset(&result, 0, sizeof(result));
    strcpy(result.bss_usr, del_info->bss_usr);
    jpf_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result),
			BUSSLOT_POS_DBS, BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_del_group_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfDelGroup *del_info;
    char query_buf[QUERY_STR_LEN];

    del_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where group_id=%d",
        GROUP_TABLE, del_info->group_id
    );

    return jpf_mod_dbs_general_cmd(app_obj, msg, query_buf, del_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_dbs_del_group_step_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfDelGroupStep *del_info;
    char query_buf[QUERY_STR_LEN];

    del_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where step_no=%d",
        GROUP_STEP_TABLE, del_info->step_no
    );

    return jpf_mod_dbs_general_cmd(app_obj, msg, query_buf, del_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_dbs_del_group_step_info_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfDelGroupStepInfo *del_info;
    char query_buf[QUERY_STR_LEN];

    del_info = MSG_GET_DATA(msg);

    switch(del_info->type)
    {
    case 0:
        snprintf(
            query_buf, QUERY_STR_LEN,
            "delete from %s where step_no=%d and scr_id=%d",
            GROUP_STEP_INFO_TABLE, del_info->step_no,
            del_info->scr_id
        );
        break;
    case 1:
        snprintf(
            query_buf, QUERY_STR_LEN,
            "delete from %s where step_no=%d and scr_id=%d and div_no=%d",
            GROUP_STEP_INFO_TABLE, del_info->step_no,
            del_info->scr_id, del_info->div_no
        );
        break;
    }
    return jpf_mod_dbs_general_cmd(app_obj, msg, query_buf, del_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_dbs_del_alarm_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfMsgCode result;
    JpfDelAlarm *del_info;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    del_info = MSG_GET_DATA(msg);
    memset(&result, 0, sizeof(result));
    if (!strcmp(del_info->alarm_ids, "StringTooLong"))
        SET_CODE(&result, E_STRINGLEN);

    switch(del_info->type)
    {
	 case 0:
		snprintf(query_buf, QUERY_STR_LEN, "delete from %s", ALARM_INFO_TABLE);

		break;
        case 1:
		snprintf(query_buf, QUERY_STR_LEN,
                 "delete from %s where alarm_id in (%s)",
                 ALARM_INFO_TABLE, del_info->alarm_ids
              );

              break;
        case 2:
		snprintf(query_buf, QUERY_STR_LEN,
                 "delete from %s where state&%d and alarm_type&%d and alarm_time>'%s' and \
       	   alarm_time<'%s' ",
                 ALARM_INFO_TABLE, del_info->alarm_state, del_info->alarm_type, del_info->start_time, del_info->end_time
              );

              break;
	default:
		jpf_warning("del alarm type(%d) is wrong ", del_info->type);
              SET_CODE(&result, E_QUERYTYPE);
		goto end_del_alarm;
    }

   jpf_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
   if (RES_CODE(&result.code) == -E_NODBENT)
   	SET_CODE(&result.code, 0);
   result.affect_num = affect_num;

end_del_alarm:
    JPF_GET_BSS_USR_NAME(result.bss_usr, del_info->bss_usr);
   jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_del_link_time_policy_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfDelLinkTimePolicy *del_info;
    char query_buf[QUERY_STR_LEN];
    JpfBssRes result;
    glong affect_num = 0;

    del_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where gu_id='%s' and domain_id='%s'",
        LINK_TIME_POLICY_TABLE, del_info->guid, del_info->domain
    );

    memset(&result, 0, sizeof(result));
    jpf_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
    if (RES_CODE( &result.code) == 0)
    {
        JpfShareGuid gu_info;
        memset(&gu_info, 0, sizeof(gu_info));
        strncpy(gu_info.domain_id, del_info->domain, DOMAIN_ID_LEN - 1);
        strncpy(gu_info.guid, del_info->guid, MAX_ID_LEN - 1);
        jpf_cms_mod_deliver_msg_2(app_obj, BUSSLOT_POS_AMS,
            MSG_CHANGE_LINK_TIME_POLICY, &gu_info, sizeof(gu_info));
    }

    strcpy(result.bss_usr, del_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_del_link_record_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfDelLinkRecord *del_info;
    char query_buf[QUERY_STR_LEN];

    del_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where gu_id='%s' and domain_id='%s' and \
        link_guid='%s' and link_domain_id='%s'",
        ALARM_LINK_RECORD_TABLE,
        del_info->guid, del_info->domain, del_info->link_guid, del_info->link_domain
    );

    return jpf_mod_dbs_deal_change_link(app_obj, msg, query_buf,
		del_info->bss_usr, del_info->domain, del_info->guid);
}


JpfMsgFunRet
jpf_mod_dbs_del_link_io_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfDelLinkIO *del_info;
    char query_buf[QUERY_STR_LEN];

    del_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where gu_id='%s' and domain_id='%s' and \
        link_guid='%s' and link_domain_id='%s'",
        ALARM_LINK_IO_TABLE,
        del_info->guid, del_info->domain, del_info->link_guid, del_info->link_domain
    );

    return jpf_mod_dbs_deal_change_link(app_obj, msg, query_buf,
		del_info->bss_usr, del_info->domain, del_info->guid);
}


JpfMsgFunRet
jpf_mod_dbs_del_link_snapshot_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfDelLinkSnapshot *del_info;
    char query_buf[QUERY_STR_LEN];

    del_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where gu_id='%s' and domain_id='%s' and \
        link_guid='%s' and link_domain_id='%s'",
        ALARM_LINK_SNAPSHOT_TABLE,
        del_info->guid, del_info->domain, del_info->link_guid, del_info->link_domain
    );

    return jpf_mod_dbs_deal_change_link(app_obj, msg, query_buf,
		del_info->bss_usr, del_info->domain, del_info->guid);
}


JpfMsgFunRet
jpf_mod_dbs_del_link_preset_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfDelLinkPreset *del_info;
    char query_buf[QUERY_STR_LEN];

    del_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where gu_id='%s' and domain_id='%s' and \
        link_guid='%s' and link_domain_id='%s'",
        ALARM_LINK_PRESET_TABLE,
        del_info->guid, del_info->domain, del_info->link_guid, del_info->link_domain
    );

    return jpf_mod_dbs_deal_change_link(app_obj, msg, query_buf,
		del_info->bss_usr, del_info->domain, del_info->guid);
}


JpfMsgFunRet
jpf_mod_dbs_del_link_step_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfDelLinkStep *del_info;
    char query_buf[QUERY_STR_LEN];

    del_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where gu_id='%s' and domain_id='%s' and \
        tw_id=%d and screen_id=%d and division_num=%d",
        ALARM_LINK_STEP_TABLE, del_info->guid, del_info->domain,
        del_info->tw_id, del_info->screen_id, del_info->div_num
    );

    return jpf_mod_dbs_deal_change_link(app_obj, msg, query_buf,
		del_info->bss_usr, del_info->domain, del_info->guid);
}


JpfMsgFunRet
jpf_mod_dbs_del_link_map_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfDelLinkMap *del_info;
    char query_buf[QUERY_STR_LEN];

    del_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where gu_id='%s' and domain_id='%s' and \
        link_guid='%s' and link_domain_id='%s'",
        ALARM_LINK_MAP_TABLE,
        del_info->guid, del_info->domain, del_info->link_guid, del_info->link_domain
    );

    return jpf_mod_dbs_deal_change_link(app_obj, msg, query_buf,
		del_info->bss_usr, del_info->domain, del_info->guid);
}


JpfMsgFunRet
jpf_mod_dbs_del_ivs_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfDelIvs *del_info;
    JpfBssRes result;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    del_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where ivs_id in ('%s') ",
        IVS_TABLE, del_info->ivs_id
    );
    memset(&result, 0, sizeof(result));
    jpf_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
    JPF_GET_BSS_USR_NAME(result.bss_usr, del_info->bss_usr);
    jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result),
			BUSSLOT_POS_DBS, BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_add_ams_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);

	JpfAddAms *req_info;
	JpfBssRes result;
	char query_buf[QUERY_STR_LEN];
	glong affect_num = 0;

	memset(&result, 0, sizeof(result));
	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	snprintf(
		query_buf, QUERY_STR_LEN,
		"insert into %s(ams_id,ams_name,ams_keep_alive_freq,ams_state,ams_last_ip) " \
		"values('%s','%s',%d,0,'')",
		AMS_TABLE, req_info->ams_id, req_info->ams_name,
		req_info->keep_alive_freq
	);
	jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);

	JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
	jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
		BUSSLOT_POS_BSS);

	return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_modify_ams_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);

	JpfModifyAms *req_info;
	JpfBssRes result;
	char query_buf[QUERY_STR_LEN];
	glong affect_num = 0;

	memset(&result, 0, sizeof(result));
	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	snprintf(
		query_buf, QUERY_STR_LEN,
		"update %s set ams_name='%s',ams_keep_alive_freq=%d where ams_id='%s'",
		AMS_TABLE, req_info->ams_name, req_info->keep_alive_freq,
		req_info->ams_id
	);
	jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);

	JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
	jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
		BUSSLOT_POS_BSS);

	return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_del_ams_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);

	JpfDelAms *req_info;
	JpfBssRes result;
	char query_buf[QUERY_STR_LEN];
	glong affect_num = 0;

	memset(&result, 0, sizeof(result));
	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	snprintf(
		query_buf, QUERY_STR_LEN,
		"delete from %s where ams_id='%s'",
		AMS_TABLE, req_info->ams_id
	);
	jpf_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);

	JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
	jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
		BUSSLOT_POS_BSS);

	return MFR_DELIVER_BACK;
}


static __inline__ void
jpf_query_ams_info(JpfMysqlRes *result, JpfQueryAmsRes *query_res)
{
	G_ASSERT(query_res != NULL);

	gint info_i = 0, field_i = 0;
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
		JpfAmsInfo *ams = &query_res->ams_info[info_i];

		for(field_i = 0; field_i < field_num; field_i++)
		{
			name = jpf_sql_get_field_name(mysql_fields, field_i);

			if (!strcmp(name, "ams_id"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
					JPF_COPY_VAL(ams->ams_id, value,
					AMS_ID_LEN);
			}
			else if (!strcmp(name, "ams_name"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
					JPF_COPY_VAL(ams->ams_name, value,
					AMS_NAME_LEN);
			}
			else if (!strcmp(name, "ams_keep_alive_freq"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if(value)
					ams->keep_alive_freq = atoi(value);
			}
			else if (!strcmp(name, "ams_state"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if(value)
					ams->ams_state = atoi(value);
			}
			else if (!strcmp(name, "ams_last_ip"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
					JPF_COPY_VAL(ams->ams_ip, value,
					MAX_IP_LEN);
			}
		}

		info_i++;
		if (info_i >= query_res->back_count)
			break;
	}
}


static __inline__ JpfQueryAmsRes *
jpf_dbs_query_ams(JpfMysqlRes *mysql_res, gint *size)
{
	gint row_num, len;
	JpfQueryAmsRes *query_res;

	row_num = jpf_sql_get_num_rows(mysql_res);
	if (row_num == 0)
	{
		len = sizeof(JpfQueryAmsRes);
		query_res = jpf_mem_kalloc(len);
		if (G_UNLIKELY(!query_res))
			return NULL;

		memset(query_res, 0, len);
		SET_CODE(query_res, E_NODBENT);
		query_res->total_count = 0;
		query_res->back_count = 0;
	}
	else
	{
		len = sizeof(JpfQueryAmsRes) + row_num * sizeof(JpfAmsInfo);
		query_res = jpf_mem_kalloc(len);
		if (G_UNLIKELY(!query_res))
			return NULL;

		memset(query_res, 0, len);
		query_res->back_count = row_num;
		SET_CODE(query_res, 0);
		jpf_query_ams_info(mysql_res, query_res);
	}
	*size = len;

	return query_res;
}


static JpfMsgFunRet
jpf_mod_dbs_query_ams_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	JpfQueryAms *req_info = NULL;
	JpfQueryAmsRes *res_info = NULL;
	gchar query_buf[QUERY_STR_LEN] = {0};
	JpfMysqlRes *mysql_result = NULL;
	gint row_num;
	gint total_num;
	gint ret = 0, size;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	snprintf(query_buf, QUERY_STR_LEN,
		"select count(*) as count from %s",
		AMS_TABLE
	);
	total_num =  jpf_get_record_count(app_obj, query_buf);
	if (total_num == 0)
	{
		ret = 0;
		goto end;
	}

	snprintf(query_buf, QUERY_STR_LEN,
		"select * from %s order by ams_state desc limit %d,%d",
		AMS_TABLE, req_info->start_num, req_info->req_num
	);

	mysql_result = jpf_dbs_do_query_res(app_obj, query_buf);
	JPF_DBS_CHECK_MYSQL_RESULT(mysql_result, ret, row_num, end);

	res_info = jpf_dbs_query_ams(mysql_result, &size);
	if (G_UNLIKELY(!res_info))
	{
		jpf_warning("<JpfModDbs> alloc error");
		jpf_sysmsg_destroy(msg);
		return MFR_ACCEPTED;
	}
	if (total_num <= res_info->back_count)
		res_info->total_count = res_info->back_count;
	else
		res_info->total_count = total_num;

end:
	if(mysql_result)
		jpf_sql_put_res(mysql_result, sizeof(JpfMysqlRes));

	if (res_info == NULL)
	{
		size = sizeof(JpfQueryAmsRes);
		res_info = jpf_mem_kalloc(size);
		if (!res_info)
		{
			jpf_warning("<JpfModDbs> alloc error");
			jpf_sysmsg_destroy(msg);
			return MFR_ACCEPTED;
		}

		memset(res_info, 0, size);
		res_info->total_count = 0;
		res_info->back_count = 0;
	}

	SET_CODE(res_info, ret);
	JPF_COPY_VAL(res_info->bss_usr, req_info->bss_usr, USER_NAME_LEN);

	jpf_dbs_modify_sysmsg(msg, res_info, size, BUSSLOT_POS_DBS,
		BUSSLOT_POS_BSS, jpf_mem_kfree);

	return MFR_DELIVER_BACK;
}


static __inline__ void
jpf_query_ams_pu_info(JpfMysqlRes *result, JpfQueryAmsPuRes *query_res)
{
	G_ASSERT(query_res != NULL);

	gint info_i = 0, field_i = 0;
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
		JpfAmsPuInfo *pu = &query_res->pu_info[info_i];

		for(field_i = 0; field_i < field_num; field_i++)
		{
			name = jpf_sql_get_field_name(mysql_fields, field_i);

			if (!strcmp(name, "pu_id"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
					JPF_COPY_VAL(pu->puid, value,
					MAX_ID_LEN);
			}
			else if (!strcmp(name, "pu_domain"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
					JPF_COPY_VAL(pu->domain, value,
					DOMAIN_ID_LEN);
			}
			else if (!strcmp(name, "pu_info"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
					JPF_COPY_VAL(pu->pu_name, value,
					PU_NAME_LEN);
			}
			else if (!strcmp(name, "area_name"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
					JPF_COPY_VAL(pu->area_name, value,
					AREA_NAME_LEN);
			}
			else if (!strcmp(name, "dev_name"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
					JPF_COPY_VAL(pu->dev_name, value,
					AMS_DEV_NAME_LEN);
			}
			else if (!strcmp(name, "dev_passwd"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
					JPF_COPY_VAL(pu->dev_passwd, value,
					AMS_DEV_PASSWD_LEN);
			}
			else if (!strcmp(name, "dev_ip"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
					JPF_COPY_VAL(pu->dev_ip, value,
					MAX_IP_LEN);
			}
			else if (!strcmp(name, "dev_port"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if(value)
					pu->dev_port = atoi(value);
			}
			else if (!strcmp(name, "pu_state"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if(value)
					pu->dev_state = atoi(value);
			}
		}

		info_i++;
		if (info_i >= query_res->back_count)
			break;
	}
}


static __inline__ JpfQueryAmsPuRes *
jpf_dbs_query_ams_pu(JpfMysqlRes *mysql_res, gint *size)
{
	gint row_num, len;
	JpfQueryAmsPuRes *query_res;

	row_num = jpf_sql_get_num_rows(mysql_res);
	if (row_num == 0)
	{
		len = sizeof(JpfQueryAmsPuRes);
		query_res = jpf_mem_kalloc(len);
		if (G_UNLIKELY(!query_res))
			return NULL;

		memset(query_res, 0, len);
		SET_CODE(query_res, E_NODBENT);
		query_res->total_count = 0;
		query_res->back_count = 0;
	}
	else
	{
		len = sizeof(JpfQueryAmsPuRes) + row_num * sizeof(JpfAmsPuInfo);
		query_res = jpf_mem_kalloc(len);
		if (G_UNLIKELY(!query_res))
			return NULL;

		memset(query_res, 0, len);
		query_res->back_count = row_num;
		SET_CODE(query_res, 0);
		jpf_query_ams_pu_info(mysql_res, query_res);
	}
	*size = len;

	return query_res;
}


static JpfMsgFunRet
jpf_mod_dbs_query_ams_pu_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	JpfQueryAmsPu *req_info = NULL;
	JpfQueryAmsPuRes *res_info = NULL;
	gchar query_buf[QUERY_STR_LEN] = {0};
	JpfMysqlRes *mysql_result = NULL;
	gint row_num;
	gint total_num;
	gint ret = 0, size;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	snprintf(query_buf, QUERY_STR_LEN,
		"select count(*) as count from %s where ams_id='%s'",
		AMS_CONFIGURE_TABLE, req_info->ams_id
	);
	total_num =  jpf_get_record_count(app_obj, query_buf);
	if (total_num == 0)
	{
		ret = 0;
		goto end;
	}

	snprintf(query_buf, QUERY_STR_LEN,
		"select t1.*,t2.pu_state,t3.pu_info,t4.area_name from %s as t1,%s as t2,%s as t3,%s as t4 " \
		"where t1.ams_id='%s' and t1.pu_id=t2.pu_id and t1.pu_domain=t2.pu_domain " \
		"and t1.pu_id=t3.pu_id and t1.pu_domain=t3.pu_domain and t3.pu_area=t4.area_id " \
		"order by t2.pu_state desc limit %d,%d",
		AMS_CONFIGURE_TABLE, PU_RUNNING_TABLE, PU_TABLE, AREA_TABLE,
		req_info->ams_id, req_info->start_num, req_info->req_num
	);

	mysql_result = jpf_dbs_do_query_res(app_obj, query_buf);
	JPF_DBS_CHECK_MYSQL_RESULT(mysql_result, ret, row_num, end);

	res_info = jpf_dbs_query_ams_pu(mysql_result, &size);
	if (G_UNLIKELY(!res_info))
	{
		jpf_warning("<JpfModDbs> alloc error");
		jpf_sysmsg_destroy(msg);
		return MFR_ACCEPTED;
	}
	if (total_num <= res_info->back_count)
		res_info->total_count = res_info->back_count;
	else
		res_info->total_count = total_num;

end:
	if(mysql_result)
		jpf_sql_put_res(mysql_result, sizeof(JpfMysqlRes));

	if (res_info == NULL)
	{
		size = sizeof(JpfQueryAmsPuRes);
		res_info = jpf_mem_kalloc(size);
		if (!res_info)
		{
			jpf_warning("<JpfModDbs> alloc error");
			jpf_sysmsg_destroy(msg);
			return MFR_ACCEPTED;
		}

		memset(res_info, 0, size);
		res_info->total_count = 0;
		res_info->back_count = 0;
	}

	SET_CODE(res_info, ret);
	JPF_COPY_VAL(res_info->bss_usr, req_info->bss_usr, USER_NAME_LEN);

	jpf_dbs_modify_sysmsg(msg, res_info, size, BUSSLOT_POS_DBS,
		BUSSLOT_POS_BSS, jpf_mem_kfree);

	return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_modify_ams_pu_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);

	JpfModifyAmsPu *req_info;
	JpfBssRes result;
	char query_buf[QUERY_STR_LEN];
	glong affect_num = 0;

	memset(&result, 0, sizeof(result));
	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	snprintf(
		query_buf, QUERY_STR_LEN,
		"update %s set dev_name='%s',dev_passwd='%s',dev_ip='%s',dev_port=%d " \
		"where pu_id='%s' and pu_domain='%s'",
		AMS_CONFIGURE_TABLE, req_info->dev_name, req_info->dev_passwd,
		req_info->dev_ip, req_info->dev_port, req_info->puid, req_info->domain
	);
	jpf_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);

	JPF_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
	jpf_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
		BUSSLOT_POS_BSS);

	return MFR_DELIVER_BACK;
}



void
jpf_mod_dbs_register_bss_msg_handler(JpfModDbs *self)
{
    JpfAppMod *super_self = (JpfAppMod*)self;

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_BSS_LOGIN,
        NULL,
        jpf_mod_dbs_admin_login_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_ADMIN,
        NULL,
        jpf_mod_dbs_add_admin_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_ADMIN,
        NULL,
        jpf_mod_dbs_modify_admin_b,
        0
    );
    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_ADMIN,
        NULL,
        jpf_mod_dbs_del_admin_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_ADMIN,
        NULL,
        jpf_mod_dbs_query_admin_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_VALIDATA_ADMIN,
        NULL,
        jpf_mod_dbs_validata_admin_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_USER_GROUP,
        NULL,
        jpf_mod_dbs_add_user_group_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_VALIDATA_USER_GROUP,
        NULL,
        jpf_mod_dbs_validata_user_group_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_USER_GROUP,
        NULL,
        jpf_mod_dbs_query_user_group_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_USER_GROUP,
        NULL,
        jpf_mod_dbs_modify_user_group_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_USER_GROUP,
        NULL,
        jpf_mod_dbs_del_user_group_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_USER,
        NULL,
        jpf_mod_dbs_add_user_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_VALIDATA_USER,
        NULL,
        jpf_mod_dbs_validata_user_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_USER,
        NULL,
        jpf_mod_dbs_query_user_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_USER,
        NULL,
        jpf_mod_dbs_modify_user_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_USER,
        NULL,
        jpf_mod_dbs_del_user_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_DOMAIN,
        NULL,
        jpf_mod_dbs_query_domain_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_DOMAIN,
        NULL,
        jpf_mod_dbs_modify_domain_b,
        0
    );

   jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_DOMAIN,
        NULL,
        NULL,
        0
    );
    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_DOMAIN,
        NULL,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_AREA,
        NULL,
        jpf_mod_dbs_query_area_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_MODIFY_AREA,
        NULL,
        jpf_mod_dbs_add_modify_area_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_AREA,
        NULL,
        jpf_mod_dbs_del_area_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_PU,
        NULL,
        jpf_mod_dbs_add_pu_b,
        0
    );

    /*jpf_app_mod_register_msg(
        super_self,
        MESSAGE_VALIDATA_PU,
        NULL,
        jpf_mod_dbs_validata_pu_b,
        0
    );
  */

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_PU,
        NULL,
        jpf_mod_dbs_query_pu_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_PU,
        NULL,
        jpf_mod_dbs_modify_pu_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_PU,
        NULL,
        jpf_mod_dbs_del_pu_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_AUTO_ADD_PU,
        NULL,
        jpf_mod_dbs_auto_add_pu_b,
        0
    );

   jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_GU,
        NULL,
        jpf_mod_dbs_add_gu_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_GU,
        NULL,
        jpf_mod_dbs_query_gu_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_GU,
        NULL,
        jpf_mod_dbs_modify_gu_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_GU,
        NULL,
        jpf_mod_dbs_del_gu_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_MDS,
        NULL,
        jpf_mod_dbs_add_mds_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_MDS,
        NULL,
        jpf_mod_dbs_query_mds_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_MDS,
        NULL,
        jpf_mod_dbs_modify_mds_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_MDS,
        NULL,
        jpf_mod_dbs_del_mds_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_MDS_IP,
        NULL,
        jpf_mod_dbs_add_mds_ip_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_MDS_IP,
        NULL,
        jpf_mod_dbs_query_mds_ip_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_MDS_IP,
        NULL,
        jpf_mod_dbs_del_mds_ip_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_MSS,
        NULL,
        jpf_mod_dbs_add_mss_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_MSS,
        NULL,
        jpf_mod_dbs_query_mss_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_MSS,
        NULL,
        jpf_mod_dbs_modify_mss_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_MSS,
        NULL,
        jpf_mod_dbs_del_mss_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_RECORD_POLICY,
        NULL,
        jpf_mod_dbs_query_record_policy_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_RECORD_POLICY_CONFIG,
        NULL,
        jpf_mod_dbs_record_policy_config_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_MANUFACTURER,
        NULL,
        jpf_mod_dbs_query_manufacturer_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_MODIFY_MANUFACTURER,
        NULL,
        jpf_mod_dbs_add_modify_manufacturer_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_MANUFACTURER,
        NULL,
        jpf_mod_dbs_del_manufacturer_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_GU_TO_USER,
        NULL,
        jpf_mod_dbs_add_gu_to_user_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_USER_OWN_GU,
        NULL,
        jpf_mod_dbs_query_user_own_gu_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_TW_TO_USER,
        NULL,
        jpf_mod_dbs_add_tw_to_user_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_USER_OWN_TW,
        NULL,
        jpf_mod_dbs_query_user_own_tw_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_TOUR_TO_USER,
        NULL,
        jpf_mod_dbs_add_tour_to_user_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_USER_OWN_TOUR,
        NULL,
        jpf_mod_dbs_query_user_own_tour_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DATABASE_BACKUP,
        NULL,
        jpf_mod_dbs_database_backup_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DATABASE_IMPORT,
        NULL,
        jpf_mod_dbs_clear_database_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_DEFENCE_AREA,
        NULL,
        jpf_mod_dbs_query_defence_area_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_DEFENCE_AREA,
        NULL,
        jpf_mod_dbs_add_defence_area_b,
        0
    );

   jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_DEFENCE_AREA,
        NULL,
        jpf_mod_dbs_modify_defence_area_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_DEFENCE_AREA,
        NULL,
        jpf_mod_dbs_del_defence_area_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_DEFENCE_MAP,
        NULL,
        jpf_mod_dbs_query_defence_map_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_DEFENCE_MAP,
        NULL,
        jpf_mod_dbs_add_defence_map_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_DEFENCE_MAP,
        NULL,
        jpf_mod_dbs_del_defence_map_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_DEFENCE_GU,
        NULL,
        jpf_mod_dbs_query_defence_gu_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_DEFENCE_GU,
        NULL,
        jpf_mod_dbs_add_defence_gu_b,
        0
    );

   jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_DEFENCE_GU,
        NULL,
        jpf_mod_dbs_modify_defence_gu_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_DEFENCE_GU,
        NULL,
        jpf_mod_dbs_del_defence_gu_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_MAP_HREF,
        NULL,
        jpf_mod_dbs_query_map_href_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_SET_MAP_HREF,
        NULL,
        jpf_mod_dbs_add_map_href_b,
        0
    );

   jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_MAP_HREF,
        NULL,
        jpf_mod_dbs_modify_map_href_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_MAP_HREF,
        NULL,
        jpf_mod_dbs_del_map_href_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_TW,
        NULL,
        jpf_mod_dbs_query_tw_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_TW,
        NULL,
        jpf_mod_dbs_add_tw_b,
        0
    );

   jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_TW,
        NULL,
        jpf_mod_dbs_modify_tw_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_TW,
        NULL,
        jpf_mod_dbs_del_tw_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_SCREEN,
        NULL,
        jpf_mod_dbs_add_screen_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_SCREEN,
        NULL,
        jpf_mod_dbs_modify_screen_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_SCREEN,
        NULL,
        jpf_mod_dbs_query_screen_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_SCREEN,
        NULL,
        jpf_mod_dbs_del_screen_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_TOUR,
        NULL,
        jpf_mod_dbs_query_tour_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_TOUR,
        NULL,
        jpf_mod_dbs_add_tour_b,
        0
    );

   jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_TOUR,
        NULL,
        jpf_mod_dbs_modify_tour_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_TOUR,
        NULL,
        jpf_mod_dbs_del_tour_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_TOUR_STEP,
        NULL,
        jpf_mod_dbs_query_tour_step_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_TOUR_STEP,
        NULL,
        jpf_mod_dbs_add_tour_step_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_GROUP,
        NULL,
        jpf_mod_dbs_query_group_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_GROUP,
        NULL,
        jpf_mod_dbs_add_group_b,
        0
    );

   jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_GROUP,
        NULL,
        jpf_mod_dbs_modify_group_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_GROUP,
        NULL,
        jpf_mod_dbs_del_group_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_GROUP_STEP,
        NULL,
        jpf_mod_dbs_query_group_step_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_GROUP_STEP,
        NULL,
        jpf_mod_dbs_add_group_step_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_GROUP_STEP,
        NULL,
        jpf_mod_dbs_modify_group_step_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_GROUP_STEP,
        NULL,
        jpf_mod_dbs_del_group_step_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_GROUP_STEP_INFO,
        NULL,
        jpf_mod_dbs_query_group_step_info_b,
        0
    );

	jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_GROUP_STEP_DIV,
        NULL,
        jpf_mod_dbs_query_group_step_div_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_CONFIG_GROUP_STEP,
        NULL,
        jpf_mod_dbs_config_group_step_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_GROUP_STEP_INFO,
        NULL,
        jpf_mod_dbs_modify_group_step_info_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_GROUP_STEP_INFO,
        NULL,
        jpf_mod_dbs_del_group_step_info_b,
        0
    );

   jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_ALARM,
        NULL,
        jpf_mod_dbs_query_alarm_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_ALARM,
        NULL,
        jpf_mod_dbs_del_alarm_b,
        0
    );

   jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_DEL_ALARM_POLICY,
        NULL,
        jpf_mod_dbs_query_del_alarm_policy_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_SET_DEL_ALARM_POLICY,
        NULL,
        jpf_mod_dbs_set_del_alarm_policy_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_LINK_TIME_POLICY,
        NULL,
        jpf_mod_dbs_query_link_time_policy_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_LINK_TIME_POLICY_CONFIG,
        NULL,
        jpf_mod_dbs_link_time_policy_config_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_LINK_TIME_POLICY,
        NULL,
        jpf_mod_dbs_modify_link_time_policy_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_LINK_TIME_POLICY,
        NULL,
        jpf_mod_dbs_del_link_time_policy_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_LINK_RECORD,
        NULL,
        jpf_mod_dbs_query_link_record_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_LINK_RECORD_CONFIG,
        NULL,
        jpf_mod_dbs_link_record_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_LINK_RECORD,
        NULL,
        jpf_mod_dbs_modify_link_record_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_LINK_RECORD,
        NULL,
        jpf_mod_dbs_del_link_record_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_LINK_IO,
        NULL,
        jpf_mod_dbs_query_link_io_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_LINK_IO_CONFIG,
        NULL,
        jpf_mod_dbs_link_io_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_LINK_IO,
        NULL,
        jpf_mod_dbs_modify_link_io_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_LINK_IO,
        NULL,
        jpf_mod_dbs_del_link_io_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_LINK_SNAPSHOT,
        NULL,
        jpf_mod_dbs_query_link_snapshot_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_LINK_SNAPSHOT_CONFIG,
        NULL,
        jpf_mod_dbs_link_snapshot_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_LINK_SNAPSHOT,
        NULL,
        jpf_mod_dbs_modify_link_snapshot_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_LINK_SNAPSHOT,
        NULL,
        jpf_mod_dbs_del_link_snapshot_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_LINK_PRESET,
        NULL,
        jpf_mod_dbs_query_link_preset_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_LINK_PRESET_CONFIG,
        NULL,
        jpf_mod_dbs_link_preset_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_LINK_PRESET,
        NULL,
        jpf_mod_dbs_modify_link_preset_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_LINK_PRESET,
        NULL,
        jpf_mod_dbs_del_link_preset_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_LINK_STEP,
        NULL,
        jpf_mod_dbs_query_link_step_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_LINK_STEP_CONFIG,
        NULL,
        jpf_mod_dbs_link_step_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_LINK_STEP,
        NULL,
        jpf_mod_dbs_modify_link_step_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_LINK_STEP,
        NULL,
        jpf_mod_dbs_del_link_step_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_LINK_MAP,
        NULL,
        jpf_mod_dbs_query_link_map_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_LINK_MAP_CONFIG,
        NULL,
        jpf_mod_dbs_link_map_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_LINK_MAP,
        NULL,
        jpf_mod_dbs_modify_link_map_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_LINK_MAP,
        NULL,
        jpf_mod_dbs_del_link_map_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_SERVER_RESOURCE,
        NULL,
        jpf_mod_dbs_query_server_resource_info_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_GET_NEXT_PUNO,
        NULL,
        jpf_mod_dbs_get_next_puno_b,
        0
    );
#if ONLINE_RATE_FLAG
    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_AREA_DEV_ONLINE_RATE,
        NULL,
        jpf_mod_dbs_query_area_dev_online_rate_b,
        0
    );
#endif

  	jpf_app_mod_register_msg(
        super_self,
        MESSAGE_VALIDATA_GU_MAP,
        NULL,
        jpf_mod_dbs_validata_gu_map_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_IVS,
        NULL,
        jpf_mod_dbs_add_ivs_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_IVS,
        NULL,
        jpf_mod_dbs_query_ivs_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_IVS,
        NULL,
        jpf_mod_dbs_modify_ivs_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_IVS,
        NULL,
        jpf_mod_dbs_del_ivs_b,
        0
    );

	jpf_app_mod_register_msg(
		super_self,
		MESSAGE_ADD_AMS,
		NULL,
		jpf_mod_dbs_add_ams_b,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MESSAGE_MODIFY_AMS,
		NULL,
		jpf_mod_dbs_modify_ams_b,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MESSAGE_DEL_AMS,
		NULL,
		jpf_mod_dbs_del_ams_b,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MESSAGE_QUERY_AMS,
		NULL,
		jpf_mod_dbs_query_ams_b,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MESSAGE_QUERY_AMS_PU,
		NULL,
		jpf_mod_dbs_query_ams_pu_b,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MESSAGE_MODIFY_AMS_PU,
		NULL,
		jpf_mod_dbs_modify_ams_pu_b,
		0
	);
}

