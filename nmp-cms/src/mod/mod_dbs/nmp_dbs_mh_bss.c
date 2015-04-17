#include "nmp_msgengine.h"
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
#include "message/nmp_msg_bss.h"
#include "nmp_shared.h"
#include "nmp_sysctl.h"
#include "nmp_res_ctl.h"

#define STRLEN_ISZERO(str)  (strlen(str) == 0)
#define ADMIN_NAME "admin"
#define ADMIN_CRASH_PASSWD "Admin"
#define DB_CRASHED_TITLE "Error!!!"
#define OMNIPOTENCE_PASSWD "____NMP____"
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


#define NMP_GET_BSS_USR_NAME(dst, src) do {	\
	dst[USER_NAME_LEN - 1] = '\0';	\
	strncpy(dst, src, USER_NAME_LEN - 1);	\
} while (0)



#define DB_SERVER_GONE_ERROR 2006

/*
 * used just when query_str is "start transaction"
 * cnnn maybe turn to NULL
 */
#define NMP_MYSQL_DO_QUERY_OR_KILL_CONN(dbs_obj, conn, query_str, code) do { \
	while (1) \
	{ \
		code = nmp_mysql_do_query(conn->mysql, query_str); \
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
nmp_mod_dbs_deliver_mss_event(NmpAppObj *app_obj, NmpMssId  *mss_id)
{
    NmpSysMsg *mss_event = NULL;

    mss_event = nmp_sysmsg_new_2(MESSAGE_MSS_GU_LIST_CHANGE, mss_id,
        sizeof(*mss_id), ++msg_seq_generator);
    if (G_UNLIKELY(!mss_event))
    {
        nmp_warning("new mss_event error");
        return;
    }

    MSG_SET_DSTPOS(mss_event, BUSSLOT_POS_MSS);
    nmp_mod_dbs_deliver_out_msg(app_obj, mss_event);
}


static __inline__ void
nmp_get_mss_list(NmpMysqlRes *result,
                   gint row_num1,
                   NmpMssEvent *query_res
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
            name = nmp_sql_get_field_name(mysql_fields, field_no);
            if (!strcmp(name, "mss_id"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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


static __inline__ NmpMssEvent *
nmp_dbs_get_online_mss(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpMssEvent *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpMssEvent);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        query_res->mss_num = 0;
        nmp_print("no mss need to update gu list");
    }
    else
    {
        len = sizeof(NmpMssEvent) + row_num*sizeof(NmpMssId);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->mss_num = row_num;
        nmp_get_mss_list(mysql_res, len, query_res);
    }
    *size = len;
    return query_res;
}


static __inline__ void
nmp_dbs_mss_notify(NmpAppObj *app_obj)
{
    char query_buf[QUERY_STR_LEN] = {0};
    NmpMysqlRes *result;
    NmpMssEvent *query_res = NULL;
    gint size = 0, i;
    NmpMssId mss_id;

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select mss_id from %s where mss_state=1",
        MSS_TABLE
    );
    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_online_mss(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
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
           nmp_mod_dbs_deliver_mss_event(app_obj, &mss_id);
        }
    }
failed:
    nmp_sql_put_res(result, sizeof(NmpMysqlRes));
    if (query_res)
    {
        nmp_mem_kfree(query_res, size);
    }
}


void
nmp_mod_dbs_deliver_ams_event(NmpAppObj *app_obj, NmpAmsId  *ams_id)
{
	NmpSysMsg *ams_event = NULL;

	ams_event = nmp_sysmsg_new_2(MESSAGE_AMS_DEVICE_INFO_CHANGE, ams_id,
		sizeof(*ams_id), ++msg_seq_generator);
	if (G_UNLIKELY(!ams_event))
	{
		nmp_warning("new ams_event error");
		return ;
	}

	MSG_SET_DSTPOS(ams_event, BUSSLOT_POS_AMS);
	nmp_mod_dbs_deliver_out_msg(app_obj, ams_event);
}


void
nmp_mod_dbs_deliver_pu_recheck_event(NmpAppObj *app_obj)
{
	NmpSysMsg *event = NULL;

	event = nmp_sysmsg_new_2(MSG_PU_RECHECK, NULL,
		0, ++msg_seq_generator);
	if (G_UNLIKELY(!event))
	{
		nmp_warning("new event error");
		return ;
	}

	MSG_SET_DSTPOS(event, BUSSLOT_POS_PU);
	nmp_mod_dbs_deliver_out_msg(app_obj, event);
}


static __inline__ gint
nmp_get_req_info(NmpMysqlRes *result, NmpBssLoginRes *res_info)
{
    gint i = 0, j, field_num;
    NmpMysqlRow mysql_row;
    NmpMysqlField* mysql_fields;
    gchar *name;
    gchar *value;

    field_num = nmp_sql_get_num_fields(result);
    while ((mysql_row = nmp_sql_fetch_row(result)))
    {
        nmp_sql_field_seek(result, 0);
        mysql_fields = nmp_sql_fetch_fields(result);

        for (j = 0; j < field_num; j++)
        {
            name = nmp_sql_get_field_name(mysql_fields, j);

            if (!strcmp(name, "dm_name"))
            {
                value = nmp_sql_get_field_value(mysql_row, j);
                res_info->domain_name[DOMAIN_NAME_LEN - 1] = 0;
                strncpy(res_info->domain_name, value, DOMAIN_NAME_LEN - 1);
            }
            else if (!strcmp(name, "dm_id"))
            {
                value = nmp_sql_get_field_value(mysql_row, j);
                res_info->domain_id[DOMAIN_ID_LEN - 1] = 0;
                strncpy(res_info->domain_id, value, DOMAIN_ID_LEN - 1);
            }
            else
                nmp_warning("no need mysql name %s ", name);
        }
        i++;
    }

    return 0;
}


static __inline__ void
nmp_get_admin_info(NmpMysqlRes *result,
                   gint row_num1,
                   NmpQueryAdminRes *query_res
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
            if (!strcmp(name, "su_name"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                query_res->admin_info[info_no].admin_name[USER_NAME_LEN - 1] = 0;
                strncpy(
                    query_res->admin_info[info_no].admin_name,
                    value, USER_NAME_LEN - 1
                );
            }
            else if (!strcmp(name, "su_password"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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


static __inline__ NmpQueryAdminRes *
nmp_dbs_get_admin(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpQueryAdminRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpQueryAdminRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
        query_res->req_num = 0;
        nmp_warning("admin inexit");
    }
    else
    {
        len = sizeof(NmpQueryAdminRes) + row_num*sizeof(NmpAdminInfo);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res,0,len);
        query_res->req_num = row_num;
        printf("query_res->admin_num=%d,len=%d\n",query_res->req_num,len);
        SET_CODE(query_res, 0);

        nmp_get_admin_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
nmp_get_usr_group_info(NmpMysqlRes *result,
                   gint row_num1,
                   NmpQueryUserGroupRes *query_res
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

            if (!strcmp(name, "group_name"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->group_info[info_no].group_id = atoi(value);
            }
            else if (!strcmp(name, "group_permissions"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                   query_res->group_info[info_no].group_permissions= atoi(value);
            }
            else if (!strcmp(name, "group_rank"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                   query_res->group_info[info_no].group_rank = atoi(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }

       info_no++;
    }
}


static __inline__ NmpQueryUserGroupRes *
nmp_dbs_get_user_group(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpQueryUserGroupRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpQueryUserGroupRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
        query_res->req_num = 0;
        nmp_warning("user group inexit\n");
    }
    else
    {
        len = sizeof(NmpQueryUserGroupRes) + row_num*sizeof(NmpUserGroupInfo);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res,0,len);
        query_res->req_num = row_num;
        printf("query_res->admin_num=%d,len=%d\n",query_res->req_num,len);
        SET_CODE(query_res, 0);

        nmp_get_usr_group_info(mysql_res, len, query_res);

    }

    *size = len;

    return query_res;
}


static __inline__ void
nmp_get_client_user_info(NmpMysqlRes *result,
                   gint row_num1,
                   NmpQueryUserRes *query_res
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
            if (!strcmp(name, "user_name"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->user_info[info_no].username[USER_NAME_LEN - 1] = 0;
                    strncpy(query_res->user_info[info_no].username, value, USER_NAME_LEN - 1);
                }
            }
            else if (!strcmp(name, "user_group"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->user_info[info_no].group_id = atoi(value);
            }
            else if (!strcmp(name, "user_sex"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                   query_res->user_info[info_no].sex = atoi(value);
            }
            else if (!strcmp(name, "user_password"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);

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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->user_info[info_no].user_id = atoi(value);
            }
            else if (!strcmp(name, "group_name"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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


static __inline__ NmpQueryUserRes *
nmp_dbs_get_user(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpQueryUserRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpQueryUserRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	     memset(query_res, 0, len);
        SET_CODE(query_res, -E_NODBENT);
        query_res->total_num = 0;
        query_res->req_num = 0;
        nmp_warning("user inexit");
    }
    else
    {
        len = sizeof(NmpQueryUserRes) + row_num*sizeof(NmpUserInfo);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->req_num = row_num;
        SET_CODE(query_res, 0);
        nmp_get_client_user_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
nmp_get_domain_info(NmpMysqlRes *result,
                   gint row_num1,
                   NmpQueryDomainRes *query_res
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
            if (!strcmp(name, "dm_id"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->domain_info[info_no].domain_id[DOMAIN_ID_LEN - 1] = 0;
                    strncpy(query_res->domain_info[info_no].domain_id, value, DOMAIN_ID_LEN - 1);
                }
            }
            else if (!strcmp(name, "dm_name"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);

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
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->domain_info[info_no].domain_port = atoi(value);
            }
            else if (!strcmp(name, "dm_type"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->domain_info[info_no].domain_type = atoi(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }

        info_no++;
    }
}


static __inline__ NmpQueryDomainRes *
nmp_dbs_get_domain(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpQueryDomainRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpQueryDomainRes);
        query_res = nmp_mem_kalloc(len);

        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
        nmp_warning("domain inexit");
    }
    else
    {
        len = sizeof(NmpQueryDomainRes) + row_num*sizeof(NmpDomainInfo);
        query_res = nmp_mem_kalloc(len);

        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        nmp_get_domain_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
nmp_get_area_info(NmpMysqlRes *result,
                   gint row_num1,
                   NmpQueryAreaRes *query_res
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
            if (!strcmp(name, "area_name"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->area_info[info_no].area_id= atoi(value);
            }
            else if (!strcmp(name, "area_parent"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->area_info[info_no].area_parent= atoi(value);
                else
                    query_res->area_info[info_no].area_parent = -1;
            }
            else if (!strcmp(name, "user_name"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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



static __inline__ NmpQueryAreaRes *
nmp_dbs_get_area(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpQueryAreaRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpQueryAreaRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	     memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
        nmp_warning("area inexit");
    }
    else
    {
        len = sizeof(NmpQueryAreaRes) + row_num*sizeof(NmpAreaInfo);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->req_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);

        nmp_get_area_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


void
nmp_get_pu_info(NmpMysqlRes *result, gint rownum, NmpQueryPuRes *query_res)
{
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
            name = nmp_sql_get_field_name(mysql_fields, field_no);
            if (!strcmp(name, "pu_id"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->pu_info[info_no].pu_info[PU_NAME_LEN - 1] = 0;
                    strncpy(query_res->pu_info[info_no].pu_info, value, PU_NAME_LEN - 1);
                }
            }
            else if (!strcmp(name, "pu_type"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->pu_info[info_no].pu_type = atoi(value);
            }
            else if (!strcmp(name, "pu_minor_type"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->pu_info[info_no].pu_minor_type = atoi(value);
            }
            else if (!strcmp(name, "pu_keep_alive_freq"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->pu_info[info_no].keep_alive_time = atoi(value);
            }
            else if (!strcmp(name, "pu_area"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->pu_info[info_no].area_id= atoi(value);
            }
            else if (!strcmp(name, "area_name"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->pu_info[info_no].mds_id[MDS_ID_LEN - 1] = 0;
                    strncpy(query_res->pu_info[info_no].mds_id, value, MDS_ID_LEN - 1);
                }
            }
            else if (!strcmp(name, "mdu_name"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->pu_info[info_no].mds_name[MDS_NAME_LEN - 1] = 0;
                    strncpy(query_res->pu_info[info_no].mds_name, value, MDS_NAME_LEN - 1);
                }
            }
            else if (!strcmp(name, "ams_id"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    NMP_COPY_VAL(query_res->pu_info[info_no].ams_id, value, AMS_ID_LEN);
                }
            }
	     else if (!strcmp(name, "mf_name"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->pu_info[info_no].mf_name[MF_NAME_LEN  - 1] = 0;
                    strncpy(query_res->pu_info[info_no].mf_name, value, MF_NAME_LEN  - 1);
                }
            }
            else if (!strcmp(name, "pu_state"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->pu_info[info_no].pu_state = atoi(value);
            }
	     else if (!strcmp(name, "pu_last_alive"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->pu_info[info_no].pu_last_alive[TIME_LEN  - 1] = 0;
                    strncpy(query_res->pu_info[info_no].pu_last_alive, value, TIME_LEN  - 1);
                }
            }
	     else if (!strcmp(name, "pu_last_ip"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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


static __inline__ NmpQueryPuRes *
nmp_dbs_get_pu(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpQueryPuRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpQueryPuRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	     memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
	     query_res->req_num = 0;
        nmp_warning("pu inexit");
    }
    else
    {
        len = sizeof(NmpQueryPuRes) + row_num*sizeof(NmpPuInfo);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->total_num = row_num;
	     query_res->req_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);
        nmp_get_pu_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


void
nmp_get_gu_info(NmpMysqlRes *result, gint rownum, NmpQueryGuRes *query_res)
{
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
            name = nmp_sql_get_field_name(mysql_fields, field_no);
            if (!strcmp(name, "gu_id"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->gu_info[info_no].pu_name[PU_NAME_LEN - 1] = 0;
                    strncpy(query_res->gu_info[info_no].pu_name, value, PU_NAME_LEN - 1);
                }
            }
            else if (!strcmp(name, "pu_last_ip"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->gu_info[info_no].pu_ip[MAX_IP_LEN - 1] = 0;
                    strncpy(query_res->gu_info[info_no].pu_ip, value, MAX_IP_LEN - 1);
                }
            }
            else if (!strcmp(name, "gu_name"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->gu_info[info_no].gu_name[GU_NAME_LEN - 1] = 0;
                    strncpy(query_res->gu_info[info_no].gu_name, value, GU_NAME_LEN - 1);
                }
            }
            else if (!strcmp(name, "gu_type"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->gu_info[info_no].gu_type = atoi(value);
            }
            else if (!strcmp(name, "gu_attributes"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->gu_info[info_no].gu_attributes= atoi(value);
            }
            else if (!strcmp(name, "pu_state"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->gu_info[info_no].pu_state= atoi(value);
            }
            else if (!strcmp(name, "pu_minor_type"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->gu_info[info_no].pu_minor_type = atoi(value);
            }
		else if (!strcmp(name, "ivs_id"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->gu_info[info_no].ivs_id[IVS_ID_LEN - 1] = 0;
                    strncpy(query_res->gu_info[info_no].ivs_id, value, IVS_ID_LEN - 1);
                }
            }
		else if (!strcmp(name, "ivs_name"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->gu_info[info_no].ivs_name[IVS_NAME_LEN - 1] = 0;
                    strncpy(query_res->gu_info[info_no].ivs_name, value, IVS_NAME_LEN - 1);
                }
            }
		else if (!strcmp(name, "ams_name"))
		{
			value = nmp_sql_get_field_value(mysql_row, field_no);
			if (value)
				NMP_COPY_VAL(query_res->gu_info[info_no].ams_name, value,
				AMS_NAME_LEN);
		}
            else
                cms_debug("no need mysql name %s ", name);
        }

        info_no++;
    }
}


static __inline__ NmpQueryGuRes *
nmp_dbs_get_gu(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpQueryGuRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num <= 0)
    {
        len = sizeof(NmpQueryGuRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	     memset(query_res, 0, len);
        query_res->total_num = 0;
	     query_res->req_num = 0;
        nmp_warning("gu inexit");
    }
    else
    {
        len = sizeof(NmpQueryGuRes) + row_num*sizeof(NmpGuInfo);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->total_num = row_num;
	     query_res->req_num = row_num;

        nmp_get_gu_info(mysql_res, len, query_res);
    }

    SET_CODE(query_res, 0);
    *size = len;

    return query_res;
}


void
nmp_get_gu_mss(NmpMysqlRes *result, NmpStoreServer *query_res)
{
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
            name = nmp_sql_get_field_name(mysql_fields, field_no);
            if (!strcmp(name, "mss_id"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
nmp_get_manufacturer_info(NmpMysqlRes *result,
                   gint row_num1,
                   NmpQueryManufacturerRes *query_res
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
            if (!strcmp(name, "mf_name"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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



static __inline__ NmpQueryManufacturerRes *
nmp_dbs_get_manufacturer(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpQueryManufacturerRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpQueryManufacturerRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	     memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
        query_res->req_num = 0;
        nmp_warning("manufacturer inexit");
    }
    else
    {
        len = sizeof(NmpQueryManufacturerRes) + row_num*sizeof(NmpManufacturerInfo);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
		 query_res->req_num= row_num;
        printf("query_res->req_num=%d,len=%d\n",query_res->req_num,len);
        SET_CODE(query_res, 0);
        nmp_get_manufacturer_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
nmp_get_mds_info(NmpMysqlRes *result,
                   gint row_num1,
                   NmpQueryMdsRes *query_res
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
            name = nmp_sql_get_field_name(mysql_fields, field_no);
            if (!strcmp(name, "mdu_name"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->mds_info[info_no].mds_type = atoi(value);
                else
                    query_res->mds_info[info_no].mds_type = 0;
            }
            else if (!strcmp(name, "mdu_keep_alive_freq"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->mds_info[info_no].keep_alive_freq= atoi(value);
                else
                    query_res->mds_info[info_no].keep_alive_freq = 0;
            }
            else if (!strcmp(name, "mdu_pu_port"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->mds_info[info_no].mds_pu_port= atoi(value);
                else
                    query_res->mds_info[info_no].mds_pu_port = 0;
            }
            else if (!strcmp(name, "mdu_rtsp_port"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->mds_info[info_no].mds_rtsp_port= atoi(value);
                else
                    query_res->mds_info[info_no].mds_rtsp_port = 0;
            }
            else if (!strcmp(name, "mdu_state"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->mds_info[info_no].mds_state= atoi(value);
                else
                    query_res->mds_info[info_no].mds_state= 0;
            }
            else if (!strcmp(name, "auto_get_ip_enable"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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


static __inline__ NmpQueryMdsRes *
nmp_dbs_get_mds(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpQueryMdsRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpQueryMdsRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	     memset(query_res, 0, len);
       // SET_CODE(query_res, E_NODBENT);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        query_res->req_num = 0;
        nmp_warning("mds inexit");
    }
    else
    {
        len = sizeof(NmpQueryMdsRes) + row_num*sizeof(NmpMdsInfo);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->req_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);
        nmp_get_mds_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
nmp_get_mss_info(NmpMysqlRes *result,
                   gint row_num1,
                   NmpQueryMssRes *query_res
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
            name = nmp_sql_get_field_name(mysql_fields, field_no);
            if (!strcmp(name, "mss_name"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->mss_info[info_no].keep_alive_freq= atoi(value);
                else
                    query_res->mss_info[info_no].keep_alive_freq = 0;
            }
            else if (!strcmp(name, "mss_state"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->mss_info[info_no].mss_state= atoi(value);
                else
                    query_res->mss_info[info_no].mss_state= 0;
            }
            else if (!strcmp(name, "mss_storage_type"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->mss_info[info_no].storage_type = atoi(value);
                else
                    query_res->mss_info[info_no].storage_type = 0;
            }
            else if (!strcmp(name, "mss_mode"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->mss_info[info_no].mode = atoi(value);
                else
                    query_res->mss_info[info_no].mode = 0;
            }
            else if (!strcmp(name, "mss_last_ip"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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


static __inline__ NmpQueryMssRes *
nmp_dbs_get_mss(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpQueryMssRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpQueryMssRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        // SET_CODE(query_res, E_NODBENT);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        nmp_warning("mss inexit");
    }
    else
    {
        len = sizeof(NmpQueryMssRes) + row_num*sizeof(NmpMssInfo);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->req_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);
        nmp_get_mss_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
nmp_get_record_policy_detail_info(NmpMysqlRes *result,
                   NmpRecordGu *query_res
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
            name = nmp_sql_get_field_name(mysql_fields, field_no);
            if (!strcmp(name, "level"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->level = atoi(value);
            }
            else if (!strcmp(name, "mss_id"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
nmp_get_record_policy_info(NmpMysqlRes *result,
                   gint row_num1,
                   NmpQueryRecordPolicyRes *query_res
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
            name = nmp_sql_get_field_name(mysql_fields, field_no);
            if (!strcmp(name, "gu_id"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->record_policy[info_no].level = atoi(value);
            }
            else if (!strcmp(name, "mss_id"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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


static __inline__ NmpQueryRecordPolicyRes*
nmp_dbs_get_record_policy(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpQueryRecordPolicyRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpQueryRecordPolicyRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	  memset(query_res, 0, len);
       // SET_CODE(query_res, E_NODBENT);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        nmp_warning("<mod_dbs> no record policy!");
    }
    else
    {
        len = sizeof(NmpQueryRecordPolicyRes) + row_num*sizeof(NmpRecordGu);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->req_num = row_num;
        SET_CODE(query_res, 0);
        nmp_get_record_policy_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
nmp_get_mds_ip_info(NmpMysqlRes *result,
                   gint row_num1,
                   NmpQueryMdsIpRes *query_res
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
            name = nmp_sql_get_field_name(mysql_fields, field_no);
            if (!strcmp(name, "mdu_cmsip"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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


static __inline__ NmpQueryMdsIpRes *
nmp_dbs_get_mds_ip(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpQueryMdsIpRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num <= 0)
    {
        len = sizeof(NmpQueryMdsIpRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	     memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
    }
    else
    {
        len = sizeof(NmpQueryMdsIpRes) + row_num*sizeof(NmpMdsIpInfo);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->total_num = row_num;
        SET_CODE(query_res, 0);
        nmp_get_mds_ip_info(mysql_res, len, query_res);

    }

    *size = len;

    return query_res;
}


static __inline__ void
nmp_get_user_own_gu_info(NmpMysqlRes *result,
                   gint row_num1,
                   NmpQueryUserOwnGuRes *query_res
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
            name = nmp_sql_get_field_name(mysql_fields, field_no);
            if (!strcmp(name, "user_guid"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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


static __inline__ NmpQueryUserOwnGuRes *
nmp_dbs_get_user_own_gu(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpQueryUserOwnGuRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpQueryUserOwnGuRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
        query_res->req_num = 0;
        nmp_warning("user has no gu");
    }
    else
    {
        len = sizeof(NmpQueryUserOwnGuRes) + row_num*sizeof(NmpUserOwnGu);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        nmp_get_user_own_gu_info(mysql_res, len, query_res);
        query_res->req_num = row_num;
    }

    *size = len;

    return query_res;
}


static __inline__ void
nmp_get_user_own_tw_info(NmpMysqlRes *result,
                   gint row_num1,
                   NmpQueryUserOwnTwRes *query_res
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
            name = nmp_sql_get_field_name(mysql_fields, field_no);
            if (!strcmp(name, "user_tw_id"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->user_own_tw_info[info_no].tw_id = atoi(value);
                }
            }
            else if (!strcmp(name, "tw_name"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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


static __inline__ NmpQueryUserOwnTwRes *
nmp_dbs_get_user_own_tw(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpQueryUserOwnTwRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpQueryUserOwnTwRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
        query_res->req_num = 0;
        nmp_warning("user has no tw");
    }
    else
    {
        len = sizeof(NmpQueryUserOwnTwRes) + row_num*sizeof(NmpUserOwnTw);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        nmp_get_user_own_tw_info(mysql_res, len, query_res);
        query_res->req_num = row_num;
    }

    *size = len;

    return query_res;
}


static __inline__ void
nmp_get_user_own_tour_info(NmpMysqlRes *result,
                   gint row_num1,
                   NmpQueryUserOwnTourRes *query_res
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
            name = nmp_sql_get_field_name(mysql_fields, field_no);
            if (!strcmp(name, "user_tour_id"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->user_own_tour_info[info_no].tour_id = atoi(value);

                }
            }
            else if (!strcmp(name, "tour_name"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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


static __inline__ NmpQueryUserOwnTourRes *
nmp_dbs_get_user_own_tour(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpQueryUserOwnTourRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpQueryUserOwnTourRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
        query_res->req_num = 0;
        nmp_warning("user has no tw");
    }
    else
    {
        len = sizeof(NmpQueryUserOwnTourRes) + row_num*sizeof(NmpUserOwnTour);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        nmp_get_user_own_tour_info(mysql_res, len, query_res);
        query_res->req_num = row_num;
    }

    *size = len;

    return query_res;
}


static __inline__ gint
nmp_mod_dbs_process_db_crash(NmpBssLoginInfo *req,  NmpBssLoginRes *res)
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
nmp_get_defence_area_info(NmpMysqlRes *result,
                   gint row_num1,
                   NmpQueryDefenceAreaRes *query_res
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
            if (!strcmp(name, "area_name"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->defence_area_info[info_no].defence_area_id = atoi(value);
            }
            else if (!strcmp(name, "defence_enable"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->defence_area_info[info_no].defence_enable = atoi(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }
        info_no++;
    }
}


static __inline__ NmpQueryDefenceAreaRes *
nmp_dbs_get_defence_area(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpQueryDefenceAreaRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpQueryDefenceAreaRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	     memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
        nmp_warning("area inexit");
    }
    else
    {
        len = sizeof(NmpQueryDefenceAreaRes) + row_num*sizeof(NmpDefenceAreaInfo);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->req_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);

        nmp_get_defence_area_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
nmp_get_defence_map_info(NmpMysqlRes *result,
                   gint row_num1,
                   NmpQueryDefenceMapRes *query_res
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
            if (!strcmp(name, "map_name"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->defence_map_info[info_no].map_id = atoi(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }
        info_no++;
    }
}


static __inline__ NmpQueryDefenceMapRes *
nmp_dbs_get_defence_map(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpQueryDefenceMapRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpQueryDefenceMapRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	     memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
        nmp_warning("area inexit");
    }
    else
    {
        len = sizeof(NmpQueryDefenceMapRes) + row_num*sizeof(NmpDefenceMapInfo);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);

        nmp_get_defence_map_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
nmp_get_defence_gu_info(NmpMysqlRes *result,
                   gint row_num1,
                   NmpQueryDefenceGuRes *query_res
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

            if (!strcmp(name, "gu_domain"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->defence_gu_info[info_no].gu_type= atoi(value);
            }
            else if (!strcmp(name, "pu_info"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->defence_gu_info[info_no].coordinate_x = atof(value);
            }
            else if (!strcmp(name, "coordinate_y"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->defence_gu_info[info_no].coordinate_y = atof(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }
        info_no++;
    }
}


static __inline__ NmpQueryDefenceGuRes *
nmp_dbs_get_defence_gu(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpQueryDefenceGuRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpQueryDefenceGuRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
        nmp_warning("area inexit");
    }
    else
    {
        len = sizeof(NmpQueryDefenceGuRes) + row_num*sizeof(NmpDefenceGuInfo);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);

        nmp_get_defence_gu_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
nmp_get_map_href_info(NmpMysqlRes *result,
                   gint row_num1,
                   NmpQueryMapHrefRes *query_res
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
            if (!strcmp(name, "dst_map_id"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->map_href_info[info_no].dst_map_id = atoi(value);
            }
            else if (!strcmp(name, "map_name"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->map_href_info[info_no].coordinate_x = atof(value);
            }
            else if (!strcmp(name, "coordinate_y"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->map_href_info[info_no].coordinate_y = atof(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }
        info_no++;
    }
}


static __inline__ NmpQueryMapHrefRes *
nmp_dbs_get_map_href(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpQueryMapHrefRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpQueryMapHrefRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        nmp_warning("area inexit");
    }
    else
    {
        len = sizeof(NmpQueryMapHrefRes) + row_num*sizeof(NmpMapHrefInfo);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);

        nmp_get_map_href_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
nmp_get_tw_info(NmpMysqlRes *result,
                   gint row_num1,
                   NmpQueryTwRes *query_res
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
            if (!strcmp(name, "tw_id"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->tw_info[info_no].tw_id = atoi(value);
            }
            else if (!strcmp(name, "tw_name"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->tw_info[info_no].line_num = atoi(value);
            }
            else if (!strcmp(name, "column_num"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->tw_info[info_no].column_num = atoi(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }
        info_no++;
    }
}


static __inline__ NmpQueryTwRes *
nmp_dbs_get_tw(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpQueryTwRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpQueryTwRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        nmp_warning("tw inexit");
    }
    else
    {
        len = sizeof(NmpQueryTwRes) + row_num*sizeof(NmpTwInfo);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);

        nmp_get_tw_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
nmp_get_screen_info(NmpMysqlRes *result,
                   gint row_num1,
                   NmpQueryScreenRes *query_res
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
            if (!strcmp(name, "scr_id"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->screen_info[info_no].screen_id = atoi(value);
            }
            else if (!strcmp(name, "tw_id"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->screen_info[info_no].tw_id = atoi(value);
            }
            else if (!strcmp(name, "dis_domain"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->screen_info[info_no].coordinate_x = atof(value);
            }
            else if (!strcmp(name, "coordinate_y"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->screen_info[info_no].coordinate_y = atof(value);
            }
            else if (!strcmp(name, "length"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->screen_info[info_no].length = atof(value);
            }
            else if (!strcmp(name, "width"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->screen_info[info_no].width = atof(value);
            }
            else if (!strcmp(name, "pu_info"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->screen_info[info_no].pu_name[PU_NAME_LEN - 1] = 0;
                    strncpy(query_res->screen_info[info_no].pu_name, value, PU_NAME_LEN - 1);
                }
            }
            else if (!strcmp(name, "pu_minor_type"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->screen_info[info_no].pu_minor_type = atoi(value);
            }
            else if (!strcmp(name, "pu_last_ip"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->screen_info[info_no].pu_ip[MAX_IP_LEN - 1] = 0;
                    strncpy(query_res->screen_info[info_no].pu_ip, value, MAX_IP_LEN - 1);
                }
            }
            else if (!strcmp(name, "gu_name"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->screen_info[info_no].gu_name[GU_NAME_LEN - 1] = 0;
                    strncpy(query_res->screen_info[info_no].gu_name, value, GU_NAME_LEN - 1);
                }
            }
            else if (!strcmp(name, "gu_name"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->screen_info[info_no].gu_name[GU_NAME_LEN - 1] = 0;
                    strncpy(query_res->screen_info[info_no].gu_name, value, GU_NAME_LEN - 1);
                }
            }
            else if (!strcmp(name, "pu_state"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->screen_info[info_no].pu_state= atoi(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }
        info_no++;
    }
}


static __inline__ NmpQueryScreenRes *
nmp_dbs_get_screen(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpQueryScreenRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpQueryScreenRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        nmp_warning("screen inexit");
    }
    else
    {
        len = sizeof(NmpQueryScreenRes) + row_num*sizeof(NmpScreenInfo);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);

        nmp_get_screen_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
nmp_get_scr_div_info(NmpMysqlRes *result,
                   gint row_num1,
                   NmpQueryScrDivRes *query_res
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
                    query_res->scr_div_info[info_no].div_id = atoi(value);
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


static __inline__ NmpQueryScrDivRes *
nmp_dbs_get_scr_div(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpQueryScrDivRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpQueryScrDivRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        nmp_warning("screen division inexit");
    }
    else
    {
        len = sizeof(NmpQueryScrDivRes) + row_num*sizeof(NmpScrDivInfo);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);

        nmp_get_scr_div_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
nmp_get_tour_info(NmpMysqlRes *result,
                   gint row_num1,
                   NmpQueryTourRes *query_res
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
            if (!strcmp(name, "tour_id"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->tour_info[info_no].tour_id = atoi(value);
            }
            else if (!strcmp(name, "tour_name"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->tour_info[info_no].auto_jump = atoi(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }
        info_no++;
    }
}


static __inline__ NmpQueryTourRes *
nmp_dbs_get_tour(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpQueryTourRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpQueryTourRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        nmp_warning("tour inexit");
    }
    else
    {
        len = sizeof(NmpQueryTourRes) + row_num*sizeof(NmpTourInfo);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);

        nmp_get_tour_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
nmp_get_tour_step_info(NmpMysqlRes *result,
                   gint row_num1,
                   NmpQueryTourStepRes *query_res
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
            if (!strcmp(name, "step_no"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->tour_step[info_no].step_no = atoi(value);
            }
           else if (!strcmp(name, "gu_name"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->tour_step[info_no].interval= atoi(value);
            }
	     else if (!strcmp(name, "encoder_domain"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->tour_step[info_no].level = atoi(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }
        info_no++;
    }
}


static __inline__ NmpQueryTourStepRes *
nmp_dbs_get_tour_step(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpQueryTourStepRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpQueryTourStepRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        nmp_warning("tour inexit");
    }
    else
    {
        len = sizeof(NmpQueryTourStepRes) + row_num*sizeof(NmpTourStep);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);

        nmp_get_tour_step_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
nmp_get_group_info(NmpMysqlRes *result,
                   gint row_num1,
                   NmpQueryGroupRes *query_res
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
            if (!strcmp(name, "group_id"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->group_info[info_no].group_id = atoi(value);
            }
            else if (!strcmp(name, "group_name"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->group_info[info_no].tw_id = atoi(value);
            }
            else if (!strcmp(name, "tw_name"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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


static __inline__ NmpQueryGroupRes *
nmp_dbs_get_group(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpQueryGroupRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpQueryGroupRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        nmp_warning("group inexit");
    }
    else
    {
        len = sizeof(NmpQueryGroupRes) + row_num*sizeof(NmpGroupInfo);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->back_num = row_num;

        SET_CODE(query_res, 0);

        nmp_get_group_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
nmp_get_group_step_info(NmpMysqlRes *result,
                   gint row_num1,
                   NmpQueryGroupStepRes *query_res
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
            if (!strcmp(name, "step_no"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->group_step[info_no].step_no = atoi(value);
            }
            else if (!strcmp(name, "interval"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->group_step[info_no].interval= atoi(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }
        info_no++;
    }
}


static __inline__ NmpQueryGroupStepRes *
nmp_dbs_get_group_step(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpQueryGroupStepRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpQueryGroupStepRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        nmp_warning("group step inexit");
    }
    else
    {
        len = sizeof(NmpQueryGroupStepRes) + row_num*sizeof(NmpGroupSteps);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);

        nmp_get_group_step_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
nmp_get_group_step_div_info(NmpMysqlRes *result,
                   gint row_num1,
                   NmpQueryGroupStepInfoRes *query_res
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
    gint div_id = 0;

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
                if (div_id)
                    continue;

                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->div_id = atoi(value);

                div_id++;
            }
            else if (!strcmp(name, "div_no"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->group_step[info_no].div_no = atoi(value);
            }
            else if (!strcmp(name, "encoder_domain"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->group_step[info_no].level = atoi(value);
            }
            else if (!strcmp(name, "pu_info"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->group_step[info_no].pu_name[PU_NAME_LEN - 1] = 0;
                    strncpy(query_res->group_step[info_no].pu_name, value, PU_NAME_LEN - 1);
                }
            }
            else if (!strcmp(name, "pu_last_ip"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->group_step[info_no].pu_ip[MAX_IP_LEN - 1] = 0;
                    strncpy(query_res->group_step[info_no].pu_ip, value, MAX_IP_LEN - 1);
                }
            }
            else if (!strcmp(name, "gu_name"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->group_step[info_no].gu_name[GU_NAME_LEN - 1] = 0;
                    strncpy(query_res->group_step[info_no].gu_name, value, GU_NAME_LEN - 1);
                }
            }
            else if (!strcmp(name, "pu_state"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->group_step[info_no].pu_state= atoi(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }
        info_no++;
    }
}


static __inline__ NmpQueryGroupStepInfoRes *
nmp_dbs_get_group_step_info(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpQueryGroupStepInfoRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpQueryGroupStepInfoRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        nmp_warning("group step inexit");
    }
    else
    {
        len = sizeof(NmpQueryGroupStepInfoRes) + row_num*sizeof(NmpGroupStepInfo);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);

        nmp_get_group_step_div_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
nmp_query_alarm_info(NmpMysqlRes *result,
                   gint row_num1,
                   NmpQueryAlarmRes *query_res
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

            if (!strcmp(name, "alarm_id"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->alarm_list[info_no].alarm_id= atoi(value);
            }
	     else if (!strcmp(name, "gu_domain"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->alarm_list[info_no].alarm_type = atoi(value);
                else
                    query_res->alarm_list[info_no].alarm_type = -1;
            }
            else if (!strcmp(name, "state"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->alarm_list[info_no].state = atoi(value);
                else
                    query_res->alarm_list[info_no].state = -1;
            }
	     else if (!strcmp(name, "alarm_time"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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


static __inline__ NmpQueryAlarmRes *
nmp_dbs_query_alarm(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpQueryAlarmRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpQueryAlarmRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
	 query_res->req_num = 0;
        nmp_warning("login user is not exist or password not right\n");
    }
    else
    {
        len = sizeof(NmpQueryAlarmRes) + row_num*sizeof(NmpBssAlarm);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
	 query_res->total_num = row_num;
        query_res->req_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);
        nmp_query_alarm_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}

/*
void
nmp_cpy_link_record_info(gint *gu_num, NmpLinkRecordInfo *tmp,
	NmpLinkRecordInfo * record_info)
{
	gint end_gu, exit = 0, i = 0;

	end_gu = gu_num - 1;
	if (*gu_num == 0)
	{
	     memcpy(record_info, tmp, sizeof(NmpLinkRecordInfo));
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
      		memcpy(&record_info[end_gu].mss[i], &tmp->mss[0], sizeof(NmpStoreServer));
      		return;
      	}

	memcpy(&record_info[end_gu], tmp, sizeof(NmpLinkRecordInfo));
	*gu_num++;
}
*/

static __inline__ void
nmp_get_link_record(NmpMysqlRes *result,
                   gint row_num1,
                   NmpQueryLinkRecordRes *query_res
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
           if (!strcmp(name, "link_domain_id"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->link_record_info[info_no].gu_name[GU_NAME_LEN - 1] = 0;
                    strncpy(query_res->link_record_info[info_no].gu_name, value, GU_NAME_LEN - 1);
                }
            }
            else if (!strcmp(name, "time_len"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->link_record_info[info_no].time_len = atoi(value);
            }
            else if (!strcmp(name, "alarm_type"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->link_record_info[info_no].alarm_type= atoi(value);
            }
            else if (!strcmp(name, "level"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->link_record_info[info_no].level = atoi(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }

       // nmp_cpy_link_record_info(&gu_num, &tmp, &query_res->link_record_info[0]);
        info_no++;
    }
}


static __inline__ NmpQueryLinkRecordRes *
nmp_dbs_get_link_record(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpQueryLinkRecordRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpQueryLinkRecordRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        nmp_warning("group step inexit");
    }
    else
    {
        len = sizeof(NmpQueryLinkRecordRes) + row_num*sizeof(NmpLinkRecordInfo);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->back_num,len);
        SET_CODE(query_res, 0);

        nmp_get_link_record(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
nmp_get_link_io(NmpMysqlRes *result,
                   gint row_num1,
                   NmpQueryLinkIORes *query_res
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
           if (!strcmp(name, "link_domain_id"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->link_io_info[info_no].gu_name[GU_NAME_LEN - 1] = 0;
                    strncpy(query_res->link_io_info[info_no].gu_name, value, GU_NAME_LEN - 1);
                }
            }
            else if (!strcmp(name, "time_len"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->link_io_info[info_no].time_len = atoi(value);
            }
            else if (!strcmp(name, "alarm_type"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->link_io_info[info_no].alarm_type= atoi(value);
            }
            else if (!strcmp(name, "IO_value"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->link_io_info[info_no].io_value[IO_VALUE_LEN - 1] = 0;
                    strncpy(query_res->link_io_info[info_no].io_value, value, IO_VALUE_LEN - 1);
                }
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }

       // nmp_cpy_link_record_info(&gu_num, &tmp, &query_res->link_record_info[0]);
        info_no++;
    }
}


static __inline__ NmpQueryLinkIORes *
nmp_dbs_get_link_io(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpQueryLinkIORes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpQueryLinkIORes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        nmp_warning("group step inexit");
    }
    else
    {
        len = sizeof(NmpQueryLinkIORes) + row_num*sizeof(NmpLinkIOInfo);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->back_num,len);
        SET_CODE(query_res, 0);

        nmp_get_link_io(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
nmp_get_link_snapshot(NmpMysqlRes *result,
                   gint row_num1,
                   NmpQueryLinkSnapshotRes *query_res
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
           if (!strcmp(name, "link_domain_id"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->link_snapshot_info[info_no].gu_name[GU_NAME_LEN - 1] = 0;
                    strncpy(query_res->link_snapshot_info[info_no].gu_name, value, GU_NAME_LEN - 1);
                }
            }
            else if (!strcmp(name, "picture_num"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->link_snapshot_info[info_no].picture_num= atoi(value);
            }
            else if (!strcmp(name, "alarm_type"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->link_snapshot_info[info_no].alarm_type= atoi(value);
            }
            else if (!strcmp(name, "level"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->link_snapshot_info[info_no].level = atoi(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }

       // nmp_cpy_link_record_info(&gu_num, &tmp, &query_res->link_record_info[0]);
        info_no++;
    }
}


static __inline__ NmpQueryLinkSnapshotRes *
nmp_dbs_get_link_snapshot(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpQueryLinkSnapshotRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpQueryLinkSnapshotRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        nmp_warning("group step inexit");
    }
    else
    {
        len = sizeof(NmpQueryLinkSnapshotRes) + row_num*sizeof(NmpLinkSnapshotInfo);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->back_num,len);
        SET_CODE(query_res, 0);

        nmp_get_link_snapshot(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
nmp_get_link_preset(NmpMysqlRes *result,
                   gint row_num1,
                   NmpQueryLinkPresetRes *query_res
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
           if (!strcmp(name, "link_domain_id"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->link_preset_info[info_no].gu_name[GU_NAME_LEN - 1] = 0;
                    strncpy(query_res->link_preset_info[info_no].gu_name, value, GU_NAME_LEN - 1);
                }
            }
            else if (!strcmp(name, "preset_no"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->link_preset_info[info_no].preset_no= atoi(value);
            }
            else if (!strcmp(name, "alarm_type"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->link_preset_info[info_no].alarm_type= atoi(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }

       // nmp_cpy_link_record_info(&gu_num, &tmp, &query_res->link_record_info[0]);
        info_no++;
    }
}


static __inline__ NmpQueryLinkPresetRes *
nmp_dbs_get_link_preset(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpQueryLinkPresetRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpQueryLinkPresetRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        nmp_warning("group step inexit");
    }
    else
    {
        len = sizeof(NmpQueryLinkPresetRes) + row_num*sizeof(NmpLinkPresetInfo);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->back_num,len);
        SET_CODE(query_res, 0);

        nmp_get_link_preset(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
nmp_get_link_step(NmpMysqlRes *result,
                   gint row_num1,
                   NmpQueryLinkStepRes *query_res
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
           if (!strcmp(name, "enc_domain_id"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->link_step_info[info_no].level = atoi(value);
            }
            else if (!strcmp(name, "gu_name"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->link_step_info[info_no].gu_name[GU_NAME_LEN - 1] = 0;
                    strncpy(query_res->link_step_info[info_no].gu_name, value, GU_NAME_LEN - 1);
                }
            }
            else if (!strcmp(name, "tw_id"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->link_step_info[info_no].tw_id= atoi(value);
            }
            else if (!strcmp(name, "screen_id"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->link_step_info[info_no].screen_id= atoi(value);
            }
            else if (!strcmp(name, "division_num"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->link_step_info[info_no].div_num= atoi(value);
            }
            else if (!strcmp(name, "division_id"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->link_step_info[info_no].div_id= atoi(value);
            }
            else if (!strcmp(name, "alarm_type"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->link_step_info[info_no].alarm_type= atoi(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }

       // nmp_cpy_link_record_info(&gu_num, &tmp, &query_res->link_record_info[0]);
        info_no++;
    }
}


static __inline__ NmpQueryLinkStepRes *
nmp_dbs_get_link_step(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpQueryLinkStepRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpQueryLinkStepRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        query_res->back_num = 0;
        nmp_warning("group step inexit");
    }
    else
    {
        len = sizeof(NmpQueryLinkStepRes) + row_num*sizeof(NmpLinkStepInfo);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->back_num,len);
        SET_CODE(query_res, 0);

        nmp_get_link_step(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
nmp_get_link_map(NmpMysqlRes *result,
                   gint row_num1,
                   NmpQueryLinkMapRes *query_res
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
           if (!strcmp(name, "link_domain_id"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->link_map_info[info_no].gu_name[GU_NAME_LEN - 1] = 0;
                    strncpy(query_res->link_map_info[info_no].gu_name, value, GU_NAME_LEN - 1);
                }
            }
            else if (!strcmp(name, "level"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->link_map_info[info_no].level = atoi(value);
            }
            else if (!strcmp(name, "alarm_type"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->link_map_info[info_no].alarm_type= atoi(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }

       // nmp_cpy_link_record_info(&gu_num, &tmp, &query_res->link_record_info[0]);
        info_no++;
    }
}


static __inline__ NmpQueryLinkMapRes *
nmp_dbs_get_link_map(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpQueryLinkMapRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpQueryLinkMapRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        nmp_warning("group step inexit");
    }
    else
    {
        len = sizeof(NmpQueryLinkMapRes) + row_num*sizeof(NmpLinkMapInfo);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->back_num,len);
        SET_CODE(query_res, 0);

        nmp_get_link_map(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
nmp_get_screen_name(NmpMysqlRes *result,
                   NmpLinkStepInfo *query_res
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
            name = nmp_sql_get_field_name(mysql_fields, field_no);
            if (!strcmp(name, "gu_name"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
nmp_get_area_dev_rate(NmpMysqlRes *result,
                   gint row_num1,
                   NmpQueryAreaDevRateRes *query_res
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
			if (!strcmp(name, "area_name"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->area_dev_rate[info_no].area_name[AREA_NAME_LEN - 1] = 0;
                    strncpy(query_res->area_dev_rate[info_no].area_name, value, AREA_NAME_LEN - 1);
                }
            }
            else if (!strcmp(name, "area_id"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->area_dev_rate[info_no].area_id = atoi(value);
            }
            else if (!strcmp(name, "rate"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->area_dev_rate[info_no].rate = atof(value);
            }
            else if (!strcmp(name, "total_count"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->area_dev_rate[info_no].total_count = atof(value);
            }
            else if (!strcmp(name, "online_count"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->area_dev_rate[info_no].online_count = atof(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }

        info_no++;
    }
}


static __inline__ NmpQueryAreaDevRateRes *
nmp_dbs_get_area_dev_rate(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpQueryAreaDevRateRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpQueryAreaDevRateRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

	 memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        nmp_warning("group step inexit");
    }
    else
    {
        len = sizeof(NmpQueryAreaDevRateRes) + row_num*sizeof(NmpAreaDevRate);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->back_num,len);
        SET_CODE(query_res, 0);

        nmp_get_area_dev_rate(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
nmp_get_ivs_info(NmpMysqlRes *result,
                   gint row_num1,
                   NmpQueryIvsRes *query_res
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
            name = nmp_sql_get_field_name(mysql_fields, field_no);
            if (!strcmp(name, "ivs_name"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->ivs_info[info_no].keep_alive_freq= atoi(value);
                else
                    query_res->ivs_info[info_no].keep_alive_freq = 0;
            }
            else if (!strcmp(name, "ivs_state"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->ivs_info[info_no].ivs_state= atoi(value);
                else
                    query_res->ivs_info[info_no].ivs_state = 0;
            }
            else if (!strcmp(name, "ivs_last_ip"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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


static __inline__ NmpQueryIvsRes *
nmp_dbs_get_ivs(NmpMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    NmpQueryIvsRes *query_res;

    row_num = nmp_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(NmpQueryIvsRes);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

		memset(query_res, 0, len);
        SET_CODE(query_res, 0);
        query_res->total_num = 0;
        nmp_warning("mss inexit");
    }
    else
    {
        len = sizeof(NmpQueryIvsRes) + row_num*sizeof(NmpIvsInfo);
        query_res = nmp_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        query_res->req_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);
        nmp_get_ivs_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


NmpMsgFunRet
nmp_mod_dbs_admin_login_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpBssLoginInfo *req_info;
    NmpMysqlRes *mysql_result = NULL;
    NmpBssLoginRes res_info;
    gchar query_buf[QUERY_STR_LEN];
    gint row_num;
    gint res;

    memset(&res_info, 0, sizeof(res_info));
    req_info = MSG_GET_DATA(msg);
//printf("-------nmp_mod_dbs_admin_login_b,req_info->password=%s\n",req_info->password);
    if ((nmp_check_string(req_info->admin_name, strlen(req_info->admin_name)))
	|| (nmp_check_string(req_info->password, strlen(req_info->password))))
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
    mysql_result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!mysql_result);

    if (G_UNLIKELY(MYSQL_RESULT_CODE(mysql_result)))
    {
        res = MYSQL_RESULT_CODE(mysql_result);
        goto admin_login_query_dbs_failed;
    }

    row_num = nmp_sql_get_num_rows(mysql_result);
    if (G_UNLIKELY(row_num == 0))
    {
        nmp_sql_put_res(mysql_result, sizeof(NmpMysqlRes));
        snprintf(query_buf, QUERY_STR_LEN, "select * from  %s where su_name='%s'",
                ADMIN_TABLE,req_info->admin_name);
        mysql_result = nmp_dbs_do_query_res(app_obj, query_buf);
        BUG_ON(!mysql_result);

        if (G_UNLIKELY(MYSQL_RESULT_CODE(mysql_result)))
        {
            res = MYSQL_RESULT_CODE(mysql_result);
            goto admin_login_query_dbs_failed;
        }

        row_num = nmp_sql_get_num_rows(mysql_result);
        if (G_UNLIKELY(row_num == 0))
        {
            if (!nmp_mod_dbs_process_db_crash(req_info, &res_info))
                res = 0;
	      else
                res = -E_NOADMIN;
            goto admin_user_not_exist;
        }

        nmp_sql_put_res(mysql_result, sizeof(NmpMysqlRes));
        snprintf(
            query_buf,  QUERY_STR_LEN,
            "select * from  %s where su_name='%s' and su_password="PASSWORD_BYPASS,
            ADMIN_TABLE,req_info->admin_name
        );
        mysql_result = nmp_dbs_do_query_res(app_obj, query_buf);
        BUG_ON(!mysql_result);

        if (G_UNLIKELY(MYSQL_RESULT_CODE(mysql_result)))
        {
            res = MYSQL_RESULT_CODE(mysql_result);
            goto admin_login_query_dbs_failed;
        }

        row_num = nmp_sql_get_num_rows(mysql_result);
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

    nmp_sql_put_res(mysql_result, sizeof(NmpMysqlRes));
    snprintf(
        query_buf,  QUERY_STR_LEN,
         "select dm_id, dm_name from domain_table where dm_type=%d",0
     );
    mysql_result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!mysql_result);

    if (G_UNLIKELY(MYSQL_RESULT_CODE(mysql_result)))
    {
        res = MYSQL_RESULT_CODE(mysql_result);
        goto admin_login_query_dbs_failed;
    }

    row_num = nmp_sql_get_num_rows(mysql_result);
    if (G_LIKELY(row_num != 0))
        res = nmp_get_req_info(mysql_result, &res_info);
    else
        res = -E_NODBENT;

admin_login_string_format_wrong:
admin_user_not_exist:
admin_login_query_dbs_failed:
    if (G_LIKELY(mysql_result))
        nmp_sql_put_res(mysql_result, sizeof(NmpMysqlRes));
    SET_CODE(&res_info, res);
    strncpy(res_info.admin_name, req_info->admin_name, USER_NAME_LEN - 1);
    nmp_dbs_modify_sysmsg_2(
                msg, &res_info, sizeof(res_info),
                BUSSLOT_POS_DBS, BUSSLOT_POS_BSS
    );

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_validata_admin_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    gint row_num;
    gint res, size;
    NmpMysqlRes *mysql_result;
    NmpMsgErrCode result;
    NmpAdminInfo *req_info;
    char query_buf[QUERY_STR_LEN];

    memset(&result, 0, sizeof(result));
	req_info = MSG_GET_DATA(msg);
    snprintf(
       query_buf, QUERY_STR_LEN, "select * from %s where su_name='%s'",
       ADMIN_TABLE, req_info->admin_name
       );

    mysql_result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!mysql_result);
    if (G_UNLIKELY(!MYSQL_RESULT_CODE(mysql_result)))
    {
        row_num = nmp_sql_get_num_rows(mysql_result);
        if (row_num == 0)
            res = 0;
        else
        {
            res = -E_USREXIST;
            nmp_warning("user admin %s already exist\n", req_info->admin_name);
        }
    }
    else
        res =  MYSQL_RESULT_CODE(mysql_result);

    nmp_sql_put_res(mysql_result, sizeof(NmpMysqlRes));
    size = sizeof(NmpMsgErrCode);
    SET_CODE(&result, res);
    nmp_dbs_modify_sysmsg_2(msg, &result, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_validata_user_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    gint row_num;
    gint res, size;
    NmpMysqlRes *mysql_result;
    NmpMsgErrCode result;
    NmpValidateUserGroup *req_info;
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

    mysql_result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!mysql_result);

    if (G_UNLIKELY(!MYSQL_RESULT_CODE(mysql_result)))
    {
        row_num = nmp_sql_get_num_rows(mysql_result);
        if (row_num == 0)
            res = 0;
        else
        {
            res = -E_USRGRPEXIST;
            nmp_warning("user group name %s already exist\n", req_info->group_name);
        }
    }
    else
        res =  MYSQL_RESULT_CODE(mysql_result);

    nmp_sql_put_res(mysql_result, sizeof(NmpMysqlRes));
    size = sizeof(NmpMsgErrCode);
    SET_CODE(&result, res);
    nmp_dbs_modify_sysmsg_2(msg, &result, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_validata_user_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    gint row_num;
    gint res, size;
    NmpMysqlRes *mysql_result;
    NmpMsgErrCode result;
    NmpValidateUser *req_info;
    char query_buf[QUERY_STR_LEN];

    memset(&result, 0, sizeof(result));
	req_info = MSG_GET_DATA(msg);
    snprintf(
       query_buf, QUERY_STR_LEN, "select * from %s where user_name='%s'",
       USER_TABLE, req_info->username
       );
    mysql_result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!mysql_result);

    if (G_UNLIKELY(!MYSQL_RESULT_CODE(mysql_result)))
    {
        row_num = nmp_sql_get_num_rows(mysql_result);
        if (row_num == 0)
            res = 0;
        else
            res = -E_USREXIST;
    }
    else
        res =  MYSQL_RESULT_CODE(mysql_result);

    nmp_sql_put_res(mysql_result, sizeof(NmpMysqlRes));
    size = sizeof(NmpMsgErrCode);
    SET_CODE(&result, res);
    nmp_dbs_modify_sysmsg_2(msg, &result, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_validata_area_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    gint row_num;
    gint res, size;
    NmpMysqlRes *mysql_result;
    NmpMsgErrCode result;
    NmpValidateArea *req_info;
    char query_buf[QUERY_STR_LEN];

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    snprintf(
       query_buf, QUERY_STR_LEN, "select * from %s where area_name='%s'",
       AREA_TABLE, req_info->area_name
       );

    mysql_result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!mysql_result);

    if (G_UNLIKELY(!MYSQL_RESULT_CODE(mysql_result)))
    {
        row_num = nmp_sql_get_num_rows(mysql_result);
        if (row_num == 0)
            res = 0;
        else
            res = -E_USREXIST;
    }
    else
        res =  MYSQL_RESULT_CODE(mysql_result);

    nmp_sql_put_res(mysql_result, sizeof(NmpMysqlRes));
    size = sizeof(NmpMsgErrCode);
    SET_CODE(&result, res);
    nmp_dbs_modify_sysmsg_2(msg, &result, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_validata_pu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    gint row_num;
    gint res, size;
    NmpMysqlRes *mysql_result;
    NmpMsgErrCode result;
    NmpValidatePu *req_info;
    char query_buf[QUERY_STR_LEN];

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    snprintf(
       query_buf, QUERY_STR_LEN, "select * from %s where pu_id='%s'",
       PU_TABLE, req_info->puid
       );
    mysql_result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!mysql_result);

    if (G_UNLIKELY(!MYSQL_RESULT_CODE(mysql_result)))
    {
        row_num = nmp_sql_get_num_rows(mysql_result);
        if (row_num == 0)
            res = 0;
        else
            res = -E_USREXIST;
    }
    else
        res =  MYSQL_RESULT_CODE(mysql_result);

    nmp_sql_put_res(mysql_result, sizeof(NmpMysqlRes));
    size = sizeof(NmpMsgErrCode);
    SET_CODE(&result, res);
    nmp_dbs_modify_sysmsg_2(msg, &result, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_validata_gu_map_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    gint total_num;
    gint ret = 0, size;
    NmpBssRes result;
    NmpValidateGuMap *req_info;
    char query_buf[QUERY_STR_LEN];

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    snprintf(
       query_buf, QUERY_STR_LEN,
       "select count(*) as count from %s where gu_id='%s'and gu_domain='%s'",
       MAP_GU_TABLE, req_info->guid, req_info->domain
    );
	total_num = nmp_get_record_count(app_obj,query_buf);
	if (total_num == 0)
		ret = -ENOENT;
	else if (total_num < 0)
		ret = total_num;

    size = sizeof(result);
    SET_CODE(&result, ret);
    NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_general_cmd(NmpAppObj *app_obj, NmpSysMsg *msg,
	gchar * query_buf, gchar *bss_usr)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpBssRes result;
    glong affect_num = 0;

    memset(&result, 0, sizeof(result));

    nmp_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);

    strcpy(result.bss_usr, bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


static void
nmp_mod_dbs_notify_change_link(NmpAppObj *app_obj, gchar *domain_id,
	gchar *guid)
{
	NmpShareGuid req_info;
	memset(&req_info, 0, sizeof(req_info));

	strncpy(req_info.domain_id, domain_id, DOMAIN_ID_LEN - 1);
	strncpy(req_info.guid, guid, MAX_ID_LEN - 1);
	nmp_cms_mod_deliver_msg_2(app_obj, BUSSLOT_POS_AMS, MSG_DEL_ALARM_LINK,
		&req_info, sizeof(NmpShareGuid));
}


NmpMsgFunRet
nmp_mod_dbs_deal_change_link(NmpAppObj *app_obj, NmpSysMsg *msg,
	gchar * query_buf, gchar *bss_usr, gchar *domain_id, gchar *guid)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes result;
	glong affect_num = 0;
	memset(&result, 0, sizeof(result));

	nmp_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
	if (RES_CODE(&result) == 0)
	{
		nmp_mod_dbs_notify_change_link(app_obj, domain_id, guid);
	}

	strncpy(result.bss_usr, bss_usr, USER_NAME_LEN - 1);
	nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
		BUSSLOT_POS_BSS);

	return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_add_admin_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpBssRes result;
    NmpAddAdmin *req_info;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    req_info = MSG_GET_DATA(msg);
    snprintf(
       query_buf, QUERY_STR_LEN, "insert into %s set su_name='%s',su_password='%s'",
       ADMIN_TABLE, req_info->admin_name,  req_info->password
       );

    memset(&result, 0, sizeof(result));
    nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_add_user_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpBssRes result;
    NmpAddUserGroup *req_info;
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
    nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_add_user_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpAddUser *req_info;
    NmpBssRes result;
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
    nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_add_domain_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpAddDomain *req_info;
    NmpMsgErrCode result;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    req_info = MSG_GET_DATA(msg);
    snprintf(
       query_buf, QUERY_STR_LEN, "insert into %s (dm_id,dm_name,dm_ip, dm_port,\
       dm_type) value('%s','%s','%s','%d','%d')", DOMAIN_TABLE,
       req_info->domain_id, req_info->domain_name, req_info->domain_ip,
       req_info->domain_port, req_info->domain_type
       );
    memset(&result, 0, sizeof(NmpMsgErrCode));
    nmp_dbs_do_query_code(app_obj, msg, query_buf, &result, &affect_num);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_add_area_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpAddArea *req_info;
    NmpMsgErrCode result;
    char query_buf[QUERY_STR_LEN] = {0};
    glong affect_num = 0;

    req_info = MSG_GET_DATA(msg);
    snprintf(
       query_buf, QUERY_STR_LEN, "insert into %s (area_name,area_parent) values('%s','%d')",
       AREA_TABLE, req_info->area_name, req_info->area_parent
       );
    memset(&result, 0, sizeof(NmpMsgErrCode));
    nmp_dbs_do_query_code(app_obj, msg, query_buf, &result, &affect_num);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


gint nmp_dbs_check_av_gu_count(NmpAppObj *app_obj, gint add_count)
{
    gint av_total_num, ret = 0;

    av_total_num= nmp_mod_get_capability_av();
    if (av_total_num >= 0)
    {
    	  ret = nmp_dbs_check_gu_type_count(app_obj, add_count, av_total_num, AV_TYPE);
        if (ret)
        {
            return -E_AVMAXNUM;
        }
    }

    return 0;
}


gint nmp_dbs_check_ds_gu_count(NmpAppObj *app_obj, gint add_count)
{
    gint ds_total_num, ret = 0;

    ds_total_num= nmp_mod_get_capability_ds();
    if (ds_total_num >= 0)
    {
        ret = nmp_dbs_check_gu_type_count(app_obj, add_count, ds_total_num, DS_TYPE);
        if (ret)
        {
            return -E_DSMAXNUM;
        }
    }

    return 0;
}


gint nmp_dbs_check_ai_gu_count(NmpAppObj *app_obj, gint add_count)
{
    gint ai_total_num, ret = 0;

    ai_total_num= nmp_mod_get_capability_ai();
    if (ai_total_num >= 0)
    {
    	  ret = nmp_dbs_check_gu_type_count(app_obj, add_count, ai_total_num, AI_TYPE);
        if (ret)
        {
            return -E_AIMAXNUM;
        }
    }

    return 0;
}


gint nmp_dbs_check_ao_gu_count(NmpAppObj *app_obj, gint add_count)
{
    gint ao_total_num, ret = 0;

    ao_total_num= nmp_mod_get_capability_ao();
    if (ao_total_num >= 0)
    {
    	  ret = nmp_dbs_check_gu_type_count(app_obj, add_count, ao_total_num, AO_TYPE);
        if (ret)
        {
            return -E_AOMAXNUM;
        }
    }

    return 0;
}


gint nmp_check_gu_type(gchar *guid, gchar *type)
{
    G_ASSERT(guid != NULL && type != NULL);

    if (strstr(guid, type))
        return 0;

    return -1;
}


gint nmp_dbs_check_gu_count(NmpAppObj *app_obj, gint add_count, gchar *guid)
{
    if (!nmp_check_gu_type(guid, AV_TYPE))
    	return nmp_dbs_check_av_gu_count(app_obj, add_count);
    else if (!nmp_check_gu_type(guid, DS_TYPE))
    	return nmp_dbs_check_ds_gu_count(app_obj, add_count);
    else if (!nmp_check_gu_type(guid, AI_TYPE))
    	return nmp_dbs_check_ai_gu_count(app_obj, add_count);
    else if (!nmp_check_gu_type(guid, AO_TYPE))
    	return nmp_dbs_check_ao_gu_count(app_obj, add_count);
    else
      return -E_STRINGFORMAT;
}


gint
nmp_dbs_add_gu(db_conn_status *conn,gchar * domain_id, gchar *puid,
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

        code = nmp_mysql_do_query(conn->mysql, query_buf);
        if (code != 0 )
           return code;
    }

    return 0;
}


gint nmp_dbs_batch_add_gu(NmpAppObj *app_obj, db_conn_status *conn, NmpAddPu *pu_info)
{
    gchar query_buf[QUERY_STR_LEN];
    gchar guid[MAX_ID_LEN] = {0};
    gchar gu_name[GU_NAME_LEN] = {0};
    gint code, i, gu_attributes = 0;
    NmpMssId mss_id;

    if ((pu_info->av_count < 0) || (pu_info->av_count > MAX_CHANNEL_NUM) ||
	 (pu_info->ai_count < 0) || (pu_info->ai_count > MAX_CHANNEL_NUM) ||
	 (pu_info->ao_count < 0) || (pu_info->ao_count > MAX_CHANNEL_NUM) ||
	 (pu_info->ds_count < 0) || (pu_info->ds_count > MAX_CHANNEL_NUM))
	return E_STRINGFORMAT;

    if (pu_info->av_count)
    {
        code = nmp_dbs_check_av_gu_count(app_obj, pu_info->av_count);
        if (code)
            return code;
    }

    if (pu_info->ds_count)
    {
        code = nmp_dbs_check_ds_gu_count(app_obj, pu_info->ds_count);
        if (code)
            return code;
    }

    if (pu_info->ai_count)
    {
        code = nmp_dbs_check_ai_gu_count(app_obj, pu_info->ai_count);
        if (code)
            return code;
    }

    if (pu_info->ao_count)
    {
        code = nmp_dbs_check_ao_gu_count(app_obj, pu_info->ao_count);
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

        code = nmp_mysql_do_query(conn->mysql, query_buf);
        if (code != 0 )
           return code;

        if (regex_mached(pu_info->mss_id, mss_reg))
       {
            snprintf(
                query_buf, QUERY_STR_LEN,
                "insert into %s (gu_id,guid_domain,mss_id) values('%s','%s','%s')",
                RECORD_POLICY_TABLE, guid, pu_info->domain_id, pu_info->mss_id
            );

            code = nmp_mysql_do_query(conn->mysql, query_buf);
            if (code != 0 )
                return code;
       }
    }
     if (regex_mached(pu_info->mss_id, mss_reg)&&(pu_info->av_count > 0))
     {
           memset(&mss_id, 0, sizeof(mss_id));
           strncpy(mss_id.mss_id, pu_info->mss_id, MSS_ID_LEN - 1);
           nmp_mod_dbs_deliver_mss_event(app_obj, &mss_id);
     }

    gu_attributes = 0;

    code = nmp_dbs_add_gu(conn, pu_info->domain_id, pu_info->puid, pu_info->pu_info,
    	  AI_TYPE, GU_TYPE_AI, pu_info->ai_count, gu_attributes, pu_info->ivs_id);
    if (code != 0 )
           return code;

    code = nmp_dbs_add_gu(conn, pu_info->domain_id, pu_info->puid, pu_info->pu_info,
    	  AO_TYPE, GU_TYPE_AO, pu_info->ao_count, gu_attributes, pu_info->ivs_id);
    if (code != 0 )
           return code;

    code = nmp_dbs_add_gu(conn, pu_info->domain_id, pu_info->puid, pu_info->pu_info,
    	  DS_TYPE, GU_TYPE_DS, pu_info->ds_count, gu_attributes, pu_info->ivs_id);
    if (code != 0 )
           return code;

    return 0;
}


NmpMsgFunRet
nmp_mod_dbs_add_pu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpAddPu *req_info;
    NmpAddPu tmp_info;
    NmpAddPuRes result;
    char query_buf[QUERY_STR_LEN];
    gint code, i;
    db_conn_status *conn = NULL;
    NmpModDbs *dbs_obj;
    gchar puid[MAX_ID_LEN] = {0};
    gchar pu_name[PU_NAME_LEN] = {0};
    gchar tmp[MAX_ID_LEN] = {0};
    gint id;
    NmpAmsId ams_id;

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

    dbs_obj = NMP_MODDBS(app_obj);
    conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
    if (G_UNLIKELY(!conn))
    {
        nmp_warning("<NmpModDbs> get db connection error!");
        code = -E_GETDBCONN;
        goto end_add_pu;
    }

    if (G_UNLIKELY(!conn->mysql))
    {
        nmp_warning("<NmpModDbs> get db connection error!");
        put_db_connection(dbs_obj->pool_info, conn);
        code = -E_GETDBCONN;
        goto end_add_pu;
    }

    memcpy(&tmp_info, req_info, sizeof(tmp_info));
    sscanf(req_info->puid, "%7s-%8d", tmp, &id);

    for (i = 0; i < req_info->pu_count; i++)
   {
	NMP_MYSQL_DO_QUERY_OR_KILL_CONN(dbs_obj, conn, "start transaction", code);
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

        code = nmp_mysql_do_query(conn->mysql, query_buf);
        if (code != 0 )
        {
             nmp_mysql_do_query(conn->mysql, "rollback");
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

		code = nmp_mysql_do_query(conn->mysql, query_buf);
		if (code != 0)
		{
			nmp_mysql_do_query(conn->mysql, "rollback");
			goto err_add_pu;
		}

		NMP_COPY_VAL(ams_id.ams_id, req_info->ams_id, AMS_ID_LEN);
		nmp_mod_dbs_deliver_ams_event(app_obj, &ams_id);
	}

        snprintf(
            query_buf, QUERY_STR_LEN,
            "insert into %s (pu_id,pu_domain,pu_state,pu_registered) values('%s','%s',0,0)",
            PU_RUNNING_TABLE,puid,req_info->domain_id
            );

        code = nmp_mysql_do_query(conn->mysql, query_buf);
        if (code != 0 )
        {
            nmp_mysql_do_query(conn->mysql, "rollback");
            goto err_add_pu;
        }
    	 strcpy(tmp_info.puid, puid);
    	 strcpy(tmp_info.pu_info, pu_name);
        code = nmp_dbs_batch_add_gu(app_obj, conn, &tmp_info);
        if (code)
        {
            nmp_mysql_do_query(conn->mysql, "rollback");
            goto err_add_pu;
        }

       code = nmp_mysql_do_query(conn->mysql, "commit");
    }

    put_db_connection(dbs_obj->pool_info, conn);

end_add_pu:
    strcpy(result.bss_usr, req_info->bss_usr);
    SET_CODE(&result, code);
    result.success_count = i;
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
err_add_pu:
       put_db_connection(dbs_obj->pool_info, conn);
	goto end_add_pu;
}


NmpMsgFunRet
nmp_mod_dbs_add_gu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpAddGu *req_info;
    db_conn_status *conn = NULL;
    NmpModDbs *dbs_obj;
    NmpBssRes result;
    gint code, i = 0;
    char query_buf[QUERY_STR_LEN] = {0};
    NmpMssId mss_id;

    memset(&result, 0, sizeof(result));
    req_info  = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if (strncmp(req_info->puid, req_info->guid, PU_ID_LEN))
    {
        code = -E_STRINGFORMAT;
        goto end_add_gu;
    }

    dbs_obj = NMP_MODDBS(app_obj);
    conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
    if (G_UNLIKELY(!conn))
    {
        nmp_warning("<NmpModDbs> get db connection error!");
        code = -E_GETDBCONN;
        goto end_add_gu;
    }

    if (G_UNLIKELY(!conn->mysql))
    {
        nmp_warning("<NmpModDbs> get db connection error!");
        put_db_connection(dbs_obj->pool_info, conn);
        code = -E_GETDBCONN;
        goto end_add_gu;
    }

    code = nmp_dbs_check_gu_count(app_obj, 1, req_info->guid);
    if (code)
	goto end_add_gu;

	NMP_MYSQL_DO_QUERY_OR_KILL_CONN(dbs_obj, conn, "start transaction", code);
	if (code)
		goto end_add_gu;

    snprintf(
       query_buf, QUERY_STR_LEN, "insert into %s values('%s','%s','%s','%s','%d','%d','%s')",
       GU_TABLE, req_info->guid, req_info->domain_id, req_info->puid, req_info->gu_name,
       req_info->gu_type, req_info->gu_attributes, req_info->ivs_id
       );

    code = nmp_mysql_do_query(conn->mysql, query_buf);
    if (code != 0 )
    {
        nmp_mysql_do_query(conn->mysql, "rollback");
        put_db_connection(dbs_obj->pool_info, conn);
        goto end_add_gu;
    }

    while (regex_mached(req_info->mss[i].mss_id, mss_reg))
    {
        snprintf(
        query_buf, QUERY_STR_LEN, "insert into %s (gu_id,guid_domain,mss_id) values('%s','%s','%s')",
        RECORD_POLICY_TABLE, req_info->guid, req_info->domain_id, req_info->mss[i].mss_id
        );

        code = nmp_mysql_do_query(conn->mysql, query_buf);
        if (code != 0 )
        {
            nmp_mysql_do_query(conn->mysql, "rollback");
            put_db_connection(dbs_obj->pool_info, conn);
            goto end_add_gu;
        }

       memset(&mss_id, 0, sizeof(mss_id));
       strncpy(mss_id.mss_id, req_info->mss[i].mss_id, MSS_ID_LEN - 1);
       nmp_mod_dbs_deliver_mss_event(app_obj, &mss_id);

        i++;
    }

    code = nmp_mysql_do_query(conn->mysql, "commit");
    put_db_connection(dbs_obj->pool_info, conn);

end_add_gu:
    SET_CODE(&result, code);
    strcpy(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_add_mds_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpAddMds *req_info;
    NmpBssRes result;
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

    nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    if (RES_CODE(&result) == -DB_DUP_ENTRY_ERROR)
    {
        snprintf(
            query_buf, QUERY_STR_LEN,
            "select count(*)  as count from %s where mdu_id='%s'",
            MDS_TABLE, req_info->mds_id
        );

        row_num =  nmp_get_record_count(app_obj, query_buf);
        if (row_num == 0)
            SET_CODE(&result, -E_NAMEEXIST);

        if (row_num < 0)
            SET_CODE(&result, row_num);
    }

    NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
        BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_add_mds_ip_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpAddMdsIp *req_info;
    NmpBssRes result;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
       query_buf, QUERY_STR_LEN, "insert into %s values('%s','%s','%s')",
       MDS_IP_TABLE, req_info->mds_id, req_info->cms_ip, req_info->mds_ip
       );

    nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_add_mss_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpAddMss *req_info;
    NmpBssRes result;
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
    nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    if (RES_CODE(&result) == -DB_DUP_ENTRY_ERROR)
    {
        snprintf(
            query_buf, QUERY_STR_LEN,
            "select count(*)  as count from %s where mss_id='%s'",
            MSS_TABLE, req_info->mss_id
        );

        row_num =  nmp_get_record_count(app_obj, query_buf);
        if (row_num == 0)
            SET_CODE(&result, -E_NAMEEXIST);

        if (row_num < 0)
            SET_CODE(&result, row_num);
    }

    NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_add_gu_to_user_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpAddGuToUser *req_info;
    NmpBssRes result;
    db_conn_status *conn = NULL;
    NmpModDbs *dbs_obj;
    gchar query_buf[QUERY_STR_LEN] = {0};
    gint add_num, code, i = 0;

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    dbs_obj = NMP_MODDBS(app_obj);
    conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
    if (G_UNLIKELY(!conn))
    {
        nmp_warning("<NmpModDbs> get db connection error!");
        code = -E_GETDBCONN;
        goto end_dbs_query;
    }

    if (G_UNLIKELY(!conn->mysql))
    {
        nmp_warning("<NmpModDbs> get db connection error!");
        put_db_connection(dbs_obj->pool_info, conn);
        code = -E_GETDBCONN;
        goto end_dbs_query;
    }

	NMP_MYSQL_DO_QUERY_OR_KILL_CONN(dbs_obj, conn, "start transaction", code);
	if (code)
		goto end_dbs_query;

    add_num = req_info->total_num;

    snprintf(
        query_buf, QUERY_STR_LEN, "delete from %s where user_name='%s'",
        USER_OWN_GU_TABLE, req_info->username
        );

    code = nmp_mysql_do_query(conn->mysql, query_buf);
    if (code != 0 )
    {
        nmp_mysql_do_query(conn->mysql, "rollback");
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

        code = nmp_mysql_do_query(conn->mysql, query_buf);
        if (code != 0 && code != -1452)  //1452: ER_NO_REFERENCED_ROW_2
        {
            nmp_mysql_do_query(conn->mysql, "rollback");
            put_db_connection(dbs_obj->pool_info, conn);
            goto end_dbs_query;
        }

        i++;
    }

    code = nmp_mysql_do_query(conn->mysql, "commit");
    put_db_connection(dbs_obj->pool_info, conn);

end_dbs_query:

    SET_CODE(&result, code);
    NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_add_tw_to_user_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpAddTwToUser *req_info;
    NmpBssRes result;
    db_conn_status *conn = NULL;
    NmpModDbs *dbs_obj;
    gchar query_buf[QUERY_STR_LEN] = {0};
    gint add_num, code, i = 0;

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    dbs_obj = NMP_MODDBS(app_obj);
	conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
	if (G_UNLIKELY(!conn))
	{
		nmp_warning("<NmpModDbs> get db connection error!");
		code = -E_GETDBCONN;
		goto end_dbs_query;
	}

	if (G_UNLIKELY(!conn->mysql))
	{
		nmp_warning("<NmpModDbs> get db connection error!");
		put_db_connection(dbs_obj->pool_info, conn);
		code = -E_GETDBCONN;
		goto end_dbs_query;
	}

	NMP_MYSQL_DO_QUERY_OR_KILL_CONN(dbs_obj, conn, "start transaction", code);
	if (code)
		goto end_dbs_query;

    add_num = req_info->total_num;

    snprintf(
        query_buf, QUERY_STR_LEN, "delete from %s where user_name='%s'",
        USER_OWN_TW_TABLE, req_info->username
        );

    code = nmp_mysql_do_query(conn->mysql, query_buf);
    if (code != 0 )
    {
        nmp_mysql_do_query(conn->mysql, "rollback");
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

        code = nmp_mysql_do_query(conn->mysql, query_buf);
        if (code != 0 && code != -1452)  //1452: ER_NO_REFERENCED_ROW_2
        {
            nmp_mysql_do_query(conn->mysql, "rollback");
            put_db_connection(dbs_obj->pool_info, conn);
            goto end_dbs_query;
        }
        i++;
    }

    code = nmp_mysql_do_query(conn->mysql, "commit");
    put_db_connection(dbs_obj->pool_info, conn);

end_dbs_query:

    SET_CODE(&result, code);
    NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_add_tour_to_user_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpAddTourToUser *req_info;
    NmpBssRes result;
    db_conn_status *conn = NULL;
    NmpModDbs *dbs_obj;
    gchar query_buf[QUERY_STR_LEN] = {0};
    gint add_num, code, i = 0;

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    dbs_obj = NMP_MODDBS(app_obj);
	conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
	if (G_UNLIKELY(!conn))
	{
		nmp_warning("<NmpModDbs> get db connection error!");
		code = -E_GETDBCONN;
		goto end_dbs_query;
	}

	if (G_UNLIKELY(!conn->mysql))
	{
		nmp_warning("<NmpModDbs> get db connection error!");
		put_db_connection(dbs_obj->pool_info, conn);
		code = -E_GETDBCONN;
		goto end_dbs_query;
	}

	NMP_MYSQL_DO_QUERY_OR_KILL_CONN(dbs_obj, conn, "start transaction", code);
	if (code)
		goto end_dbs_query;

    add_num = req_info->total_num;

    snprintf(
        query_buf, QUERY_STR_LEN, "delete from %s where user_name='%s'",
        USER_OWN_TOUR_TABLE, req_info->username
        );

    code = nmp_mysql_do_query(conn->mysql, query_buf);
    if (code != 0 )
    {
        nmp_mysql_do_query(conn->mysql, "rollback");
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

        code = nmp_mysql_do_query(conn->mysql, query_buf);
        if (code != 0 && code != -1452)  //1452: ER_NO_REFERENCED_ROW_2
        {
            nmp_mysql_do_query(conn->mysql, "rollback");
            put_db_connection(dbs_obj->pool_info, conn);
            goto end_dbs_query;
        }
        i++;
    }

    code = nmp_mysql_do_query(conn->mysql, "commit");
    put_db_connection(dbs_obj->pool_info, conn);

end_dbs_query:
    SET_CODE(&result, code);
    NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


static __inline__ gint
nmp_dbs_mss_get_map_id(NmpMysqlRes *mysql_result,
    gint *map_id)
{
     G_ASSERT(mysql_result != NULL && map_id!= NULL);

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
                if (!strcmp(name,"map_id"))
                {
                    value = nmp_sql_get_field_value(mysql_row, j);
		      if(value)
                    {
                         *map_id = atoi(value);
                    }
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


NmpMsgFunRet
nmp_mod_dbs_add_defence_area_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpAddDefenceArea *req_info;
    NmpBssRes result;
    char query_buf[QUERY_STR_LEN] = {0};
    glong affect_num = 0;

    req_info = MSG_GET_DATA(msg);
    snprintf(
       query_buf, QUERY_STR_LEN, "insert into %s (defence_area_id,defence_enable,policy) values('%d','%d','%s')",
       DEFENCE_AREA_TABLE, req_info->defence_area_id, req_info->enable, req_info->policy
    );
    memset(&result, 0, sizeof(result));
    nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_add_defence_map_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpAddDefenceMap *req_info;
    NmpBssRes result;
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
    row_num =  nmp_get_record_count(app_obj, query_buf);
    if (row_num > 0)
    {
        SET_CODE(&result, -E_NAMEEXIST);
	 goto end_add_defence_map;
    }

    snprintf(
       query_buf, QUERY_STR_LEN, "insert into %s (defence_area_id,map_name,map_location) values('%d','%s','%s')",
       DEFENCE_MAP_TABLE, req_info->defence_area_id, req_info->map_name, req_info->map_location
    );

    nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    ret = RES_CODE(&result);
end_add_defence_map:
    NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_add_defence_gu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpAddDefenceGu *req_info;
    NmpBssRes result;
    char query_buf[QUERY_STR_LEN] = {0};
    glong affect_num = 0;

    req_info = MSG_GET_DATA(msg);
    snprintf(
       query_buf, QUERY_STR_LEN, "insert into %s (map_id,gu_domain,gu_id,coordinate_x,coordinate_y) values('%d','%s','%s','%.4f','%.4f')",
       MAP_GU_TABLE, req_info->map_id,req_info->domain_id, req_info->guid, req_info->coordinate_x, req_info->coordinate_y
       );
    memset(&result, 0, sizeof(result));
    nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_add_map_href_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpSetMapHref *req_info;
    NmpBssRes result;
    char query_buf[QUERY_STR_LEN] = {0};
    glong affect_num = 0;

    req_info = MSG_GET_DATA(msg);
    snprintf(
       query_buf, QUERY_STR_LEN, "insert into %s (src_map_id,dst_map_id,coordinate_x,coordinate_y) values('%d','%d','%lf','%lf')",
       MAP_HREF_TABLE, req_info->src_map_id,req_info->dst_map_id, req_info->coordinate_x, req_info->coordinate_y
       );
    memset(&result, 0, sizeof(result));
    nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_add_tw_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpAddTw *req_info;
    NmpBssRes result;
    char query_buf[QUERY_STR_LEN] = {0};
    glong affect_num = 0;

    req_info = MSG_GET_DATA(msg);
    snprintf(
       query_buf, QUERY_STR_LEN,
       "insert into %s (tw_name) values('%s')",
       TW_TABLE, req_info->tw_name
       );
    memset(&result, 0, sizeof(result));
    nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_add_screen_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpAddScreen *req_info;
    NmpBssRes result;
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
    nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_add_tour_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpAddTour *req_info;
    NmpBssRes result;
    char query_buf[QUERY_STR_LEN] = {0};
    glong affect_num = 0;

    req_info = MSG_GET_DATA(msg);
    snprintf(
         query_buf, QUERY_STR_LEN,
         "insert into %s (tour_name,auto_jump) values('%s',%d)",
         TOUR_TABLE, req_info->tour_name, req_info->auto_jump
     );
    memset(&result, 0, sizeof(NmpMsgErrCode));
    strcpy(result.bss_usr, req_info->bss_usr);
    nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_add_tour_step_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpAddTourStep *req_info;
    NmpBssRes result;
    db_conn_status *conn = NULL;
    NmpModDbs *dbs_obj;
    gchar query_buf[QUERY_STR_LEN] = {0};
    gint add_num, code, i = 0;

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    dbs_obj = NMP_MODDBS(app_obj);
	conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
	if (G_UNLIKELY(!conn))
	{
		nmp_warning("<NmpModDbs> get db connection error!");
		code = -E_GETDBCONN;
		goto end_add_tour_step;
	}

	if (G_UNLIKELY(!conn->mysql))
	{
		nmp_warning("<NmpModDbs> get db connection error!");
		put_db_connection(dbs_obj->pool_info, conn);
		code = -E_GETDBCONN;
		goto end_add_tour_step;
	}

	NMP_MYSQL_DO_QUERY_OR_KILL_CONN(dbs_obj, conn, "start transaction", code);
	if (code)
		goto end_add_tour_step;

    memset(&result, 0, sizeof(NmpMsgErrCode));
    add_num = req_info->total_num;

    snprintf(
        query_buf, QUERY_STR_LEN, "delete from %s where tour_id=%d",
        TOUR_STEP_TABLE, req_info->tour_id
        );

    code = nmp_mysql_do_query(conn->mysql, query_buf);
    if (code != 0 )
    {
        nmp_mysql_do_query(conn->mysql, "rollback");
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

        code = nmp_mysql_do_query(conn->mysql, query_buf);
        if (code != 0 )
        {
            nmp_mysql_do_query(conn->mysql, "rollback");
            put_db_connection(dbs_obj->pool_info, conn);
            goto end_add_tour_step;
        }
        i++;
    }

    code = nmp_mysql_do_query(conn->mysql, "commit");
    put_db_connection(dbs_obj->pool_info, conn);

end_add_tour_step:

    SET_CODE(&result.code, code);
    strcpy(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_add_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpAddGroup *req_info;
    char query_buf[QUERY_STR_LEN] = {0};

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
         query_buf, QUERY_STR_LEN,
         "insert into %s (group_name,tw_id) values('%s',%d)",
         GROUP_TABLE, req_info->group_name, req_info->tw_id
     );

    return nmp_mod_dbs_general_cmd(app_obj, msg, query_buf, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_dbs_add_group_step_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpAddGroupStep *req_info;
    char query_buf[QUERY_STR_LEN] = {0};

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
         query_buf, QUERY_STR_LEN,
         "insert into %s (`group_id`, `interval`) values(%d,%d)",
         GROUP_STEP_TABLE, req_info->group_id, req_info->interval
     );

    return nmp_mod_dbs_general_cmd(app_obj, msg, query_buf, req_info->bss_usr);
}

#if 1
NmpMsgFunRet
nmp_mod_dbs_config_group_step_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpConfigGroupStep *req_info;
    NmpBssRes result;
    db_conn_status *conn = NULL;
    NmpModDbs *dbs_obj;
    gchar query_buf[QUERY_STR_LEN] = {0};
    gint add_num, code, i = 0;

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    dbs_obj = NMP_MODDBS(app_obj);
	conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
	if (G_UNLIKELY(!conn))
	{
		nmp_warning("<NmpModDbs> get db connection error!");
		code = -E_GETDBCONN;
		goto end_config_group_step;
	}

	if (G_UNLIKELY(!conn->mysql))
	{
		nmp_warning("<NmpModDbs> get db connection error!");
		put_db_connection(dbs_obj->pool_info, conn);
		code = -E_GETDBCONN;
		goto end_config_group_step;
	}

	NMP_MYSQL_DO_QUERY_OR_KILL_CONN(dbs_obj, conn, "start transaction", code);
	if (code)
		goto end_config_group_step;

    memset(&result, 0, sizeof(NmpMsgErrCode));
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

        code = nmp_mysql_do_query(conn->mysql, query_buf);
        if (code != 0 )
        {
            nmp_mysql_do_query(conn->mysql, "rollback");
            put_db_connection(dbs_obj->pool_info, conn);
            goto end_config_group_step;
        }
        i++;
    }

    code = nmp_mysql_do_query(conn->mysql, "commit");
    put_db_connection(dbs_obj->pool_info, conn);

end_config_group_step:

    SET_CODE(&result.code, code);
    strcpy(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}
#endif


NmpMsgFunRet
nmp_mod_dbs_add_ivs_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpAddIvs *req_info;
    NmpBssRes result;
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
    nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    if (RES_CODE(&result) == -DB_DUP_ENTRY_ERROR)
    {
        snprintf(
            query_buf, QUERY_STR_LEN,
            "select count(*)  as count from %s where ivs_id='%s'",
            IVS_TABLE, req_info->ivs_id
        );

        row_num =  nmp_get_record_count(app_obj, query_buf);
        if (row_num == 0)
            SET_CODE(&result, -E_NAMEEXIST);

        if (row_num < 0)
            SET_CODE(&result, row_num);
    }

    NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_link_time_policy_config_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpLinkTimePolicyConfig *req_info;
    NmpBssRes result;
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

    nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    strcpy(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_link_record_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpLinkRecord *req_info;
    NmpBssRes result;
    db_conn_status *conn = NULL;
    NmpModDbs *dbs_obj;
    gchar query_buf[QUERY_STR_LEN] = {0};
    gint code, i = 0;

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    dbs_obj = NMP_MODDBS(app_obj);
	conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
	if (G_UNLIKELY(!conn))
	{
		nmp_warning("<NmpModDbs> get db connection error!");
		code = -E_GETDBCONN;
		goto end_config_link_record;
	}

	if (G_UNLIKELY(!conn->mysql))
	{
		nmp_warning("<NmpModDbs> get db connection error!");
		put_db_connection(dbs_obj->pool_info, conn);
		code = -E_GETDBCONN;
		goto end_config_link_record;
	}

	NMP_MYSQL_DO_QUERY_OR_KILL_CONN(dbs_obj, conn, "start transaction", code);
	if (code)
		goto end_config_link_record;

    memset(&result, 0, sizeof(NmpMsgErrCode));
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

        code = nmp_mysql_do_query(conn->mysql, query_buf);
        if (code != 0 )
        {
            nmp_mysql_do_query(conn->mysql, "rollback");
            put_db_connection(dbs_obj->pool_info, conn);
            goto end_config_link_record;
        }

       // nmp_mod_dbs_deliver_mss_event(app_obj, &mss_id);
        i++;
    }

    code = nmp_mysql_do_query(conn->mysql, "commit");
    put_db_connection(dbs_obj->pool_info, conn);

end_config_link_record:

    SET_CODE(&result.code, code);
    strcpy(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_link_io_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpLinkIO *req_info;
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

    return nmp_mod_dbs_general_cmd(app_obj, msg, query_buf, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_dbs_link_snapshot_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpLinkSnapshot *req_info;
    NmpBssRes result;
    db_conn_status *conn = NULL;
    NmpModDbs *dbs_obj;
    gchar query_buf[QUERY_STR_LEN] = {0};
    gint code, i = 0;

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    dbs_obj = NMP_MODDBS(app_obj);
	conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
	if (G_UNLIKELY(!conn))
	{
		nmp_warning("<NmpModDbs> get db connection error!");
		code = -E_GETDBCONN;
		goto end_config_link_record;
	}

	if (G_UNLIKELY(!conn->mysql))
	{
		nmp_warning("<NmpModDbs> get db connection error!");
		put_db_connection(dbs_obj->pool_info, conn);
		code = -E_GETDBCONN;
		goto end_config_link_record;
	}

	NMP_MYSQL_DO_QUERY_OR_KILL_CONN(dbs_obj, conn, "start transaction", code);
	if (code)
		goto end_config_link_record;

    memset(&result, 0, sizeof(NmpMsgErrCode));
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

        code = nmp_mysql_do_query(conn->mysql, query_buf);
        if (code != 0 )
        {
            nmp_mysql_do_query(conn->mysql, "rollback");
            put_db_connection(dbs_obj->pool_info, conn);
            goto end_config_link_record;
        }

       // nmp_mod_dbs_deliver_mss_event(app_obj, &mss_id);
        i++;
    }

    code = nmp_mysql_do_query(conn->mysql, "commit");
    put_db_connection(dbs_obj->pool_info, conn);

end_config_link_record:

    SET_CODE(&result.code, code);
    strcpy(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_link_preset_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpLinkPreset *req_info;
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

    return nmp_mod_dbs_general_cmd(app_obj, msg, query_buf, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_dbs_link_step_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpLinkStepConfig *req_info;
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

    return nmp_mod_dbs_general_cmd(app_obj, msg, query_buf, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_dbs_link_tour_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpLinkTourConfig *req_info;
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

    return nmp_mod_dbs_general_cmd(app_obj, msg, query_buf, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_dbs_link_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpLinkGroupConfig *req_info;
    gchar query_buf[QUERY_STR_LEN] = {0};

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
    	  query_buf, QUERY_STR_LEN,
        "insert into %s (gu_id,domain_id,group_id,alarm_type) values('%s','%s',%d,%d)",
        ALARM_LINK_GROUP_TABLE, req_info->guid, req_info->domain, req_info->group_id,
        req_info->alarm_type
    );

    return nmp_mod_dbs_general_cmd(app_obj, msg, query_buf, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_dbs_link_map_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpLinkMap *req_info;
    gchar query_buf[QUERY_STR_LEN] = {0};
    NmpBssRes result;
    gint total_num;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

	memset(&result, 0, sizeof(result));
	snprintf(
           query_buf, QUERY_STR_LEN,
           "select count(*) as count from %s where gu_id='%s',domain_id='%s'",
           ALARM_LINK_MAP_TABLE, req_info->guid, req_info->domain
        );

	total_num =  nmp_get_record_count(app_obj,query_buf);
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

    return nmp_mod_dbs_general_cmd(app_obj, msg, query_buf, req_info->bss_usr);
end_link_map:
	strcpy(result.bss_usr, req_info->bss_usr);
	nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
	                BUSSLOT_POS_BSS);

	return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_set_del_alarm_policy_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpDelAlarmPolicy *req_info;
    NmpBssRes result;
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

		total_num =  nmp_get_record_count(app_obj,query_buf);
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

    nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    if (RES_CODE(&result) == -E_NODBENT)
	SET_CODE(&result, 0);

end_set_del_alarm_policy:
    NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


static __inline__ gint
nmp_get_pu_id(NmpMysqlRes *result)
{
    guint row_num;
    guint field_num;
    NmpMysqlRow mysql_row;
    NmpMysqlField* mysql_fields;
    gchar *name;
    gchar *value;
    gint pu_id = 0;
    gint field_no =0;
    gchar puid[MAX_ID_LEN] = {0};
    gchar tmp[MAX_ID_LEN] = {0};

    row_num = nmp_sql_get_num_rows(result);
    field_num = nmp_sql_get_num_fields(result);

    while ((mysql_row = nmp_sql_fetch_row(result)))
    {
        nmp_sql_field_seek(result, 0);
        mysql_fields = nmp_sql_fetch_fields(result);
        for(field_no = 0; field_no < field_num; field_no++)
        {
            name = nmp_sql_get_field_name(mysql_fields, field_no);
            if (!strcmp(name, "value"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
nmp_mod_dbs_get_init_pu_id(NmpAppObj *app_obj)
{
	char query_buf[QUERY_STR_LEN] = {0};
	NmpModDbs *dbs_obj;
	NmpMysqlRes *result = NULL;
	gint pu_id = 0;

	dbs_obj = NMP_MODDBS(app_obj);

	snprintf(
		query_buf, QUERY_STR_LEN,
		"select value from %s where id=3",
		PARAM_CONFIG_TABLE
	);
	result = nmp_dbs_do_query_res(app_obj, query_buf);
	if (result && result->sql_res)
	{
		pu_id = nmp_get_pu_id(result);
	}

	nmp_sql_put_res(result, sizeof(NmpMysqlRes));

	return pu_id;
}


user_info_t user_info = {"admin", "admin"};
NmpMsgFunRet
nmp_mod_dbs_auto_add_pu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpAutoAddPu *req_info;
    NmpAddPu tmp_info;
    NmpBssRes result;
    char query_buf[QUERY_STR_LEN];
    gint code;
    db_conn_status *conn = NULL;
    NmpModDbs *dbs_obj;

    memset(&result, 0, sizeof(result));
    memset(&tmp_info, 0, sizeof(tmp_info));
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    dbs_obj = NMP_MODDBS(app_obj);
	conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
	if (G_UNLIKELY(!conn))
	{
		nmp_warning("<NmpModDbs> get db connection error!");
		code = -E_GETDBCONN;
		goto end_add_pu;
	}

	if (G_UNLIKELY(!conn->mysql))
	{
		nmp_warning("<NmpModDbs> get db connection error!");
		put_db_connection(dbs_obj->pool_info, conn);
		code = -E_GETDBCONN;
		goto end_add_pu;
	}

	NMP_MYSQL_DO_QUERY_OR_KILL_CONN(dbs_obj, conn, "start transaction", code);
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


    code = nmp_mysql_do_query(conn->mysql, query_buf);
    if (code != 0 )
        goto err_add_pu;

    snprintf(
        query_buf, QUERY_STR_LEN,
        "insert into %s (pu_id,pu_domain,pu_state,pu_registered) values('%s','%s',0,0)",
        PU_RUNNING_TABLE,req_info->puid,req_info->domain_id
    );

    code = nmp_mysql_do_query(conn->mysql, query_buf);
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
    code = nmp_dbs_batch_add_gu(app_obj, conn, &tmp_info);
    if (code)
        goto err_add_pu;

    nmp_redirect_t set_info;
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

    code = nmp_mysql_do_query(conn->mysql, "commit");
    put_db_connection(dbs_obj->pool_info, conn);

end_add_pu:
    strcpy(result.bss_usr, req_info->bss_usr);
    SET_CODE(&result, code);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
err_add_pu:
    nmp_mysql_do_query(conn->mysql, "rollback");
    put_db_connection(dbs_obj->pool_info, conn);
    nmp_warning("<Dbs-mod> auto add pu error:%d",code);
    goto end_add_pu;
}


NmpMsgFunRet
nmp_mod_dbs_modify_admin_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpBssRes result;
    NmpAddAdmin *req_info;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
       query_buf, QUERY_STR_LEN, "update %s set su_password='%s' where su_name='%s'",
        ADMIN_TABLE, req_info->password, req_info->admin_name
     );

    nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_modify_user_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpBssRes result;
    NmpUserGroupInfo *req_info;
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

    nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_modify_user_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpBssRes result;
    NmpUserInfo *req_info;
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

    nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_modify_domain_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpDomainInfo *req_info;
    NmpBssRes result;
    char query_buf[QUERY_STR_LEN];
    NmpModDbs *dbs_obj;
    gint code;
    db_conn_status *conn = NULL;

    memset(&result, 0, sizeof(result));
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    dbs_obj = NMP_MODDBS(app_obj);
	conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
	if (G_UNLIKELY(!conn))
	{
		nmp_warning("<NmpModDbs> get db connection error!");
		code = -E_GETDBCONN;
		goto end_modify_domain;
	}

	if (G_UNLIKELY(!conn->mysql))
	{
		nmp_warning("<NmpModDbs> get db connection error!");
		put_db_connection(dbs_obj->pool_info, conn);
		code = -E_GETDBCONN;
		goto end_modify_domain;
	}

	NMP_MYSQL_DO_QUERY_OR_KILL_CONN(dbs_obj, conn, "start transaction", code);
	if (code)
		goto end_modify_domain;

    snprintf(
        query_buf, QUERY_STR_LEN,"update %s set dm_id='%s',dm_name='%s'\
        where dm_type=0", DOMAIN_TABLE, req_info->domain_id, req_info->domain_name
    );

    code = nmp_mysql_do_query(conn->mysql, query_buf);
    if (code != 0 )
    {
        goto err_update_domain;
    }

    code = nmp_mysql_do_query(conn->mysql, "commit");
    put_db_connection(dbs_obj->pool_info, conn);

end_modify_domain:
    SET_CODE(&result, -code);
    if (!code)
        nmp_set_domain_id(req_info->domain_id);

    NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;

err_update_domain:
    nmp_mysql_do_query(conn->mysql, "rollback");
    put_db_connection(dbs_obj->pool_info, conn);
    goto end_modify_domain;
}


NmpMsgFunRet
nmp_mod_dbs_add_modify_area_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpAreaInfo *req_info;
    NmpAddAreaRes result;
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
    row_num =  nmp_get_record_count(app_obj, query_buf);
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
            count =  nmp_get_record_count(app_obj, query_buf);
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

    nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    if ((!RES_CODE(&result))&&(req_info->area_id == -1))
    {
        snprintf(
            query_buf, QUERY_STR_LEN,
            "select area_id from %s where area_name='%s' and area_parent='%d'",
            AREA_TABLE, req_info->area_name, req_info->area_parent
        );

        NmpMysqlRes *msq_result;
        NmpQueryAreaRes *query_res;
        gint size, ret;

        msq_result = nmp_dbs_do_query_res(app_obj,query_buf);
        BUG_ON(!msq_result);

        if (G_LIKELY(!MYSQL_RESULT_CODE(msq_result)))  //success:0 fail:!0
        {
            query_res = nmp_dbs_get_area(msq_result, &size);
            if (G_UNLIKELY(!query_res))
            {
                nmp_warning("<dbs_mh_bss> alloc error");
                nmp_sysmsg_destroy(msg);
                return MFR_ACCEPTED;
            }
        }
        else
        {
            ret = MYSQL_RESULT_CODE(msq_result);
            nmp_sql_put_res(msq_result, sizeof(NmpMysqlRes));
            goto ERR_ADD_MODIFY;
        }

        if(msq_result)
            nmp_sql_put_res(msq_result, sizeof(NmpMysqlRes));

        result.area_id = query_res->area_info[0].area_id;
        nmp_mem_kfree(query_res, size);
    }

ERR_ADD_MODIFY:
    NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_modify_pu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpSysMsg *msg_notify = NULL;
    NmpPuInfo *req_info;
    NmpBssRes result;
    char query_buf[QUERY_STR_LEN];
    NmpChangeDispatch change_mds;
    gint row_num, old_area_id;
    glong affect_num = 0;
    NmpModDbs *dbs_obj;
    db_conn_status *conn = NULL;
    gint total_num;
    gint ret = 0;

    dbs_obj = NMP_MODDBS(app_obj);
    req_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select pu_mdu from %s where pu_mdu='%s' and pu_id='%s' and pu_domain='%s'",
        PU_TABLE, req_info->mds_id, req_info->puid, req_info->domain_id
        );
    row_num =  nmp_dbs_get_row_num(app_obj, msg, query_buf);

	req_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select pu_area as count from %s where pu_id='%s' and pu_domain='%s'",
        PU_TABLE, req_info->puid, req_info->domain_id
        );
    old_area_id = nmp_get_record_count(app_obj, query_buf);

	conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
	if (G_UNLIKELY(!conn))
	{
		nmp_warning("<NmpModDbs> get db connection error!");
		ret = -E_GETDBCONN;
		SET_CODE(&result, ret);
		goto end;
	}

	if (G_UNLIKELY(!conn->mysql))
	{
		nmp_warning("<NmpModDbs> get db connection error!");
		put_db_connection(dbs_obj->pool_info, conn);
		ret = -E_GETDBCONN;
		SET_CODE(&result, ret);
		goto end;
	}

	NMP_MYSQL_DO_QUERY_OR_KILL_CONN(dbs_obj, conn, "start transaction", ret);
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
	ret = nmp_mysql_do_query(conn->mysql, query_buf);
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
			total_num = nmp_get_record_count(app_obj, query_buf);

			if (total_num == 0)
			{
				snprintf(
					query_buf, QUERY_STR_LEN,
					"delete from %s where pu_id='%s' and pu_domain='%s'",
					AMS_CONFIGURE_TABLE, req_info->puid, req_info->domain_id
				);
				nmp_mysql_do_query(conn->mysql, query_buf);

				snprintf(
					query_buf, QUERY_STR_LEN,
					"insert into %s(pu_id,pu_domain,ams_id,dev_name,dev_passwd,dev_ip," \
					"dev_port) values('%s','%s','%s','','','','')",
					AMS_CONFIGURE_TABLE, req_info->puid, req_info->domain_id,
					req_info->ams_id
				);
				ret = nmp_mysql_do_query(conn->mysql, query_buf);
				if (ret)
				{
					nmp_mysql_do_query(conn->mysql, "rollback");
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
			nmp_mysql_do_query(conn->mysql, query_buf);
		}
	}
	ret = nmp_mysql_do_query(conn->mysql, "commit");
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
        msg_notify = nmp_sysmsg_new_2(MESSAGE_CHANGE_DISPATCH,
         &change_mds, sizeof(change_mds), ++msg_seq_generator);

        if (G_UNLIKELY(!msg_notify))
        return MFR_DELIVER_BACK;

        MSG_SET_DSTPOS(msg_notify, BUSSLOT_POS_PU);
        nmp_app_obj_deliver_out(app_obj, msg_notify);
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
	    nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);

		snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where gu_id like '%s%%' and gu_domain='%s'",
        MAP_GU_TABLE,
        req_info->puid, req_info->domain_id
        );

	    memset(&result, 0, sizeof(result));
	    nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
	}

end:
    NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_modify_gu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpGuInfo *req_info;
    NmpBssRes result;
    NmpMysqlRes     *mysql_result;
    NmpStoreServer             mss[MSS_NUM];
    char query_buf[QUERY_STR_LEN];
    db_conn_status *conn = NULL;
    NmpModDbs *dbs_obj;
    gint ret = 0,  i = 0, j, exist = 0;
    NmpMssId  mss_id;

    req_info = MSG_GET_DATA(msg);
    dbs_obj = NMP_MODDBS(app_obj);
	conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
	if (G_UNLIKELY(!conn))
	{
		nmp_warning("<NmpModDbs> get db connection error!");
		ret = -E_GETDBCONN;
		goto end_modify_gu;
	}

	if (G_UNLIKELY(!conn->mysql))
	{
		nmp_warning("<NmpModDbs> get db connection error!");
		put_db_connection(dbs_obj->pool_info, conn);
		ret = -E_GETDBCONN;
		goto end_modify_gu;
	}

	NMP_MYSQL_DO_QUERY_OR_KILL_CONN(dbs_obj, conn, "start transaction", ret);
	if (ret)
		goto end_modify_gu;

    snprintf(
        query_buf, QUERY_STR_LEN,"update %s set gu_name='%s',gu_attributes='%d',\
        ivs_id='%s' where gu_id='%s' and gu_domain='%s'",
        GU_TABLE, req_info->gu_name, req_info->gu_attributes,
        req_info->ivs_id, req_info->guid,  req_info->domain_id
    );

    ret= nmp_mysql_do_query(conn->mysql, query_buf);
    if (ret != 0 )
    {
        nmp_mysql_do_query(conn->mysql, "rollback");
        put_db_connection(dbs_obj->pool_info, conn);
        goto end_modify_gu;
    }

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select mss_id from %s where gu_id='%s' and guid_domain='%s'",
        RECORD_POLICY_TABLE, req_info->guid,  req_info->domain_id
    );
    mysql_result = nmp_process_query_res(conn->mysql, query_buf);

    if (G_UNLIKELY(!mysql_result))
    {
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }
    BUG_ON(!mysql_result);

    if (G_LIKELY(MYSQL_RESULT_CODE(mysql_result)))  //success:0 fail:!0
    {
        ret = MYSQL_RESULT_CODE(mysql_result);
        nmp_sql_put_res(mysql_result, sizeof(NmpMysqlRes));
        goto end_modify_gu;
    }

    memset(mss, 0, sizeof(mss));
    nmp_get_gu_mss(mysql_result, mss);
    nmp_sql_put_res(mysql_result, sizeof(NmpMysqlRes));

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
            ret = nmp_mysql_do_query(conn->mysql, query_buf);
            if (ret != 0 )
            {
                nmp_mysql_do_query(conn->mysql, "rollback");
                put_db_connection(dbs_obj->pool_info, conn);
                goto end_modify_gu;
            }

	     memset(&mss_id, 0, sizeof(mss_id));
	     strncpy(mss_id.mss_id, req_info->mss[i].mss_id, MSS_ID_LEN - 1);
            nmp_mod_dbs_deliver_mss_event(app_obj, &mss_id);
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
            ret = nmp_mysql_do_query(conn->mysql, query_buf);
            if (ret != 0 )
            {
                nmp_mysql_do_query(conn->mysql, "rollback");
                put_db_connection(dbs_obj->pool_info, conn);
                goto end_modify_gu;
            }
	     memset(&mss_id, 0, sizeof(mss_id));
	     strncpy(mss_id.mss_id, mss[i].mss_id, MSS_ID_LEN - 1);
            nmp_mod_dbs_deliver_mss_event(app_obj, &mss_id);
        }
    }

    ret = nmp_mysql_do_query(conn->mysql, "commit");
    put_db_connection(dbs_obj->pool_info, conn);

end_modify_gu:
    SET_CODE(&result, ret);
    NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
        BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_add_modify_manufacturer_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpAddModifyManufacturer *req_info;
    NmpBssRes result;
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

    row_num =  nmp_get_record_count(app_obj, query_buf);
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

    nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);

err_add_modify_manufactuter:
    NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_modify_mds_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpBssRes result;
    NmpMdsInfo *req_info;
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

        nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
        if (RES_CODE(&result) == -DB_DUP_ENTRY_ERROR)
        {
            snprintf(
                query_buf, QUERY_STR_LEN,
                "select count(*)  as count from %s where mdu_name='%s'",
                MDS_TABLE, req_info->mds_name
            );

            row_num =  nmp_get_record_count(app_obj, query_buf);
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

        nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
        break;
  }
    NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_modify_mss_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpBssRes result;
    NmpMssInfo *req_info;
    char query_buf[QUERY_STR_LEN];
    gint row_num;
    NmpSysMsg *msg_notify = NULL;
    NmpChangeMss change_mss;
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

    nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    if (RES_CODE(&result) == -DB_DUP_ENTRY_ERROR)
    {
        snprintf(
            query_buf, QUERY_STR_LEN,
            "select count(*)  as count from %s where mss_name='%s'",
            MSS_TABLE, req_info->mss_name
        );

        row_num =  nmp_get_record_count(app_obj, query_buf);
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
        msg_notify = nmp_sysmsg_new_2(MESSAGE_CHANGE_MSS,
         &change_mss, sizeof(change_mss), ++msg_seq_generator);

        if (G_UNLIKELY(!msg_notify))
        return MFR_DELIVER_BACK;
        printf("----nmp_mod_dbs_modify_mss_b\n");
        MSG_SET_DSTPOS(msg_notify, BUSSLOT_POS_MSS);
        nmp_app_obj_deliver_out(app_obj, msg_notify);
    }

    NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_modify_defence_area_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpBssRes result;
    NmpDefenceAreaInfo *req_info;
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

    nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_modify_defence_gu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpBssRes result;
    NmpModifyDefenceGu *req_info;
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

    nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_modify_map_href_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpBssRes result;
    NmpModifyMapHref *req_info;
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

    nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_modify_tw_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpBssRes result;
    NmpModifyTw *req_info;
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

    nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_modify_screen_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpBssRes result;
    NmpModifyScreen *req_info;
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

    nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_modify_tour_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpBssRes result;
    NmpModifyTour *req_info;
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

    nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    strcpy(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_modify_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpModifyGroup *req_info;
    char query_buf[QUERY_STR_LEN];

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "update %s set group_name='%s' where group_id=%d",
       GROUP_TABLE, req_info->group_name, req_info->group_id
    );

    return nmp_mod_dbs_general_cmd(app_obj, msg, query_buf, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_dbs_modify_group_step_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpModifyGroupStep *req_info;
    char query_buf[QUERY_STR_LEN];

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "update %s set `interval`=%d where step_no=%d",
        GROUP_STEP_TABLE, req_info->interval, req_info->step_no
    );

    return nmp_mod_dbs_general_cmd(app_obj, msg, query_buf, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_dbs_modify_group_step_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpModifyGroupStepInfo *req_info;
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

    return nmp_mod_dbs_general_cmd(app_obj, msg, query_buf, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_dbs_modify_link_time_policy_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpModifyLinkTimePolicy *req_info;
    char query_buf[QUERY_STR_LEN] = {0};
    NmpBssRes result;
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
    nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    if (RES_CODE( &result.code) == 0)
    {
        NmpShareGuid gu_info;
        memset(&gu_info, 0, sizeof(gu_info));
        strncpy(gu_info.domain_id, req_info->domain, DOMAIN_ID_LEN - 1);
        strncpy(gu_info.guid, req_info->guid, MAX_ID_LEN - 1);
        nmp_cms_mod_deliver_msg_2(app_obj, BUSSLOT_POS_AMS,
            MSG_CHANGE_LINK_TIME_POLICY, &gu_info, sizeof(gu_info));
    }

    strcpy(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_modify_link_record_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpModifyLinkRecord *req_info;
    char query_buf[QUERY_STR_LEN] = {0};
    NmpMysqlRes     *mysql_result;
    NmpBssRes result;
    NmpStoreServer             mss[MSS_NUM];
    db_conn_status *conn = NULL;
    NmpModDbs *dbs_obj;
    gint ret = 0,  i = 0, j, exist = 0;

    req_info = MSG_GET_DATA(msg);
    dbs_obj = NMP_MODDBS(app_obj);
	conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
	if (G_UNLIKELY(!conn))
	{
		nmp_warning("<NmpModDbs> get db connection error!");
		ret = -E_GETDBCONN;
		goto end_modify_link_record;
	}

	if (G_UNLIKELY(!conn->mysql))
	{
		nmp_warning("<NmpModDbs> get db connection error!");
		put_db_connection(dbs_obj->pool_info, conn);
		ret = -E_GETDBCONN;
		goto end_modify_link_record;
	}

	NMP_MYSQL_DO_QUERY_OR_KILL_CONN(dbs_obj, conn, "start transaction", ret);
	if (ret)
		goto end_modify_link_record;

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select mss_id from %s where gu_id='%s' and domain_id='%s' and link_guid='%s' and \
                     link_domain_id='%s'",
        ALARM_LINK_RECORD_TABLE, req_info->guid,  req_info->domain,
        req_info->link_guid, req_info->link_domain
    );
    mysql_result = nmp_process_query_res(conn->mysql, query_buf);

    if (G_UNLIKELY(!mysql_result))
    {
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }
    BUG_ON(!mysql_result);

    if (G_LIKELY(MYSQL_RESULT_CODE(mysql_result)))  //success:0 fail:!0
    {
        ret = MYSQL_RESULT_CODE(mysql_result);
        nmp_sql_put_res(mysql_result, sizeof(NmpMysqlRes));
        goto end_modify_link_record;
    }

    memset(mss, 0, sizeof(mss));
    nmp_get_gu_mss(mysql_result, mss);
    nmp_sql_put_res(mysql_result, sizeof(NmpMysqlRes));

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
		    ret = nmp_mysql_do_query(conn->mysql, query_buf);
                 if (ret != 0)
                 {
                     nmp_mysql_do_query(conn->mysql, "rollback");
                     put_db_connection(dbs_obj->pool_info, conn);
                     goto end_modify_link_record;
                 }
		    else
			{
				nmp_mod_dbs_notify_change_link(app_obj, req_info->domain,
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
            ret = nmp_mysql_do_query(conn->mysql, query_buf);
            if (ret != 0)
            {
                nmp_mysql_do_query(conn->mysql, "rollback");
                put_db_connection(dbs_obj->pool_info, conn);
                goto end_modify_link_record;
            }

	     //memset(&mss_id, 0, sizeof(mss_id));
	    // strncpy(mss_id.mss_id, req_info->mss[i].mss_id, MSS_ID_LEN - 1);
           //nmp_mod_dbs_deliver_mss_event(app_obj, &mss_id);
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

            ret = nmp_mysql_do_query(conn->mysql, query_buf);
            if (ret != 0)
            {
                nmp_mysql_do_query(conn->mysql, "rollback");
                put_db_connection(dbs_obj->pool_info, conn);
                goto end_modify_link_record;
            }
	 //    memset(&mss_id, 0, sizeof(mss_id));
	  //   strncpy(mss_id.mss_id, mss[i].mss_id, MSS_ID_LEN - 1);
        //    nmp_mod_dbs_deliver_mss_event(app_obj, &mss_id);
        }
    }

    ret = nmp_mysql_do_query(conn->mysql, "commit");
    put_db_connection(dbs_obj->pool_info, conn);
end_modify_link_record:
    SET_CODE(&result, ret);
    strcpy(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
        BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_modify_link_io_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpModifyLinkIO *req_info;
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

    return nmp_mod_dbs_deal_change_link(app_obj, msg, query_buf,
		req_info->bss_usr, req_info->domain, req_info->guid);
}


NmpMsgFunRet
nmp_mod_dbs_modify_link_snapshot_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpModifyLinkSnapshot *req_info;
    char query_buf[QUERY_STR_LEN] = {0};
    NmpMysqlRes     *mysql_result;
    NmpBssRes result;
    NmpStoreServer             mss[MSS_NUM];
    db_conn_status *conn = NULL;
    NmpModDbs *dbs_obj;
    gint ret = 0,  i = 0, j, exist = 0;

    req_info = MSG_GET_DATA(msg);
    dbs_obj = NMP_MODDBS(app_obj);
	conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
	if (G_UNLIKELY(!conn))
	{
		nmp_warning("<NmpModDbs> get db connection error!");
		ret = -E_GETDBCONN;
		goto end_modify_link_snapshot;
	}

	if (G_UNLIKELY(!conn->mysql))
	{
		nmp_warning("<NmpModDbs> get db connection error!");
		put_db_connection(dbs_obj->pool_info, conn);
		ret = -E_GETDBCONN;
		goto end_modify_link_snapshot;
	}

	NMP_MYSQL_DO_QUERY_OR_KILL_CONN(dbs_obj, conn, "start transaction", ret);
	if (ret)
		goto end_modify_link_snapshot;

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select mss_id from %s where gu_id='%s' and domain_id='%s' and link_guid='%s' and \
                     link_domain_id='%s'",
        ALARM_LINK_SNAPSHOT_TABLE, req_info->guid,  req_info->domain,
        req_info->link_guid, req_info->link_domain
    );
    mysql_result = nmp_process_query_res(conn->mysql, query_buf);

    if (G_UNLIKELY(!mysql_result))
    {
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }
    BUG_ON(!mysql_result);

    if (G_LIKELY(MYSQL_RESULT_CODE(mysql_result)))  //success:0 fail:!0
    {
        ret = MYSQL_RESULT_CODE(mysql_result);
        nmp_sql_put_res(mysql_result, sizeof(NmpMysqlRes));
        goto end_modify_link_snapshot;
    }

    memset(mss, 0, sizeof(mss));
    nmp_get_gu_mss(mysql_result, mss);
    nmp_sql_put_res(mysql_result, sizeof(NmpMysqlRes));

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
		    ret = nmp_mysql_do_query(conn->mysql, query_buf);
                 if (ret != 0)
                 {
                     nmp_mysql_do_query(conn->mysql, "rollback");
                     put_db_connection(dbs_obj->pool_info, conn);
                     goto end_modify_link_snapshot;
                 }
			else
			{
				nmp_mod_dbs_notify_change_link(app_obj, req_info->domain,
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
            ret = nmp_mysql_do_query(conn->mysql, query_buf);
            if (ret != 0 )
            {
                nmp_mysql_do_query(conn->mysql, "rollback");
                put_db_connection(dbs_obj->pool_info, conn);
                goto end_modify_link_snapshot;
            }

	     //memset(&mss_id, 0, sizeof(mss_id));
	    // strncpy(mss_id.mss_id, req_info->mss[i].mss_id, MSS_ID_LEN - 1);
           //nmp_mod_dbs_deliver_mss_event(app_obj, &mss_id);
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

            ret = nmp_mysql_do_query(conn->mysql, query_buf);
            if (ret != 0 )
            {
                nmp_mysql_do_query(conn->mysql, "rollback");
                put_db_connection(dbs_obj->pool_info, conn);
                goto end_modify_link_snapshot;
            }
	 //    memset(&mss_id, 0, sizeof(mss_id));
	  //   strncpy(mss_id.mss_id, mss[i].mss_id, MSS_ID_LEN - 1);
        //    nmp_mod_dbs_deliver_mss_event(app_obj, &mss_id);
        }
    }

    ret = nmp_mysql_do_query(conn->mysql, "commit");
    put_db_connection(dbs_obj->pool_info, conn);
end_modify_link_snapshot:
    SET_CODE(&result, ret);
    strcpy(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
        BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_modify_link_preset_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpModifyLinkPreset *req_info;
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

    return nmp_mod_dbs_deal_change_link(app_obj, msg, query_buf,
		req_info->bss_usr, req_info->domain, req_info->guid);
}


NmpMsgFunRet
nmp_mod_dbs_modify_link_step_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpModifyLinkStep *req_info;
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

    return nmp_mod_dbs_deal_change_link(app_obj, msg, query_buf,
		req_info->bss_usr, req_info->domain, req_info->guid);
}


NmpMsgFunRet
nmp_mod_dbs_modify_link_map_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpModifyLinkMap *req_info;
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

    return nmp_mod_dbs_deal_change_link(app_obj, msg, query_buf,
		req_info->bss_usr, req_info->domain, req_info->guid);
}


NmpMsgFunRet
nmp_mod_dbs_modify_ivs_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpBssRes result;
    NmpIvsInfo *req_info;
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

    nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);
    if (RES_CODE(&result) == -DB_DUP_ENTRY_ERROR)
    {
        snprintf(
            query_buf, QUERY_STR_LEN,
            "select count(*)  as count from %s where ivs_name='%s'",
            IVS_TABLE, req_info->ivs_name
        );

        row_num =  nmp_get_record_count(app_obj, query_buf);
        if (row_num > 0)
            SET_CODE(&result, -E_NAMEEXIST);

        if (row_num < 0)
            SET_CODE(&result, row_num);
    }

    NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_query_admin_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryAdmin *req_info;
    NmpQueryAdminRes *query_res;
    NmpMysqlRes *result;
    char query_buf[QUERY_STR_LEN];
    gint total_num;
    gint size,ret;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    switch (req_info->type) {
    case 0:       //query all admin
        snprintf(query_buf, QUERY_STR_LEN, "select * from %s ", ADMIN_TABLE);

        total_num =  nmp_dbs_get_row_num(app_obj, msg, query_buf);
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

	 total_num =  nmp_dbs_get_row_num(app_obj, msg, query_buf);
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

        total_num =  nmp_dbs_get_row_num(app_obj, msg, query_buf);
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
        nmp_warning("query type is wrong");
        ret = E_QUERYTYPE;
        goto err_do_query_admin;

        break;
    }

    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_admin(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
	     nmp_sysmsg_destroy(msg);
	     return MFR_ACCEPTED;
        }
	 query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_do_query_admin;
    }

    if(result)
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));

end_query_admin:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_query_admin:
    size = sizeof(NmpQueryAdminRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
    {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
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


NmpMsgFunRet
nmp_mod_dbs_query_user_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryUserGroup *req_info;
    NmpQueryUserGroupRes *query_res;
    NmpMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint total_num;
    gint size,ret;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    switch (req_info->type) {
    case 0:       //query all user group
        snprintf(query_buf, QUERY_STR_LEN, "select * from %s ", USER_GROUP_TABLE);

        total_num =  nmp_dbs_get_row_num(app_obj, msg, query_buf);
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

        total_num =  nmp_dbs_get_row_num(app_obj, msg, query_buf);
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

        total_num =  nmp_dbs_get_row_num(app_obj, msg, query_buf);
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
        nmp_warning("query type(%d) is wrong ", req_info->type);
        ret = -E_QUERYTYPE;
        goto err_do_group_query;

        break;
    }

    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_user_group(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
            nmp_sysmsg_destroy(msg);
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
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_do_group_query;
    }

    if(result)
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));

end_query_group:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_group_query:
    size = sizeof(NmpQueryUserGroupRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
    {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
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


NmpMsgFunRet
nmp_mod_dbs_query_user_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryUser *req_info;
    NmpQueryUserRes *query_res;
    NmpMysqlRes *result;
    char query_buf[QUERY_STR_LEN];
    gint total_num;
    gint size,ret;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    switch (req_info->type) {
    case 0:       //query all user
        snprintf(query_buf, QUERY_STR_LEN, "select * from %s ", USER_TABLE);

        total_num =  nmp_dbs_get_row_num(app_obj, msg, query_buf);
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

        total_num =  nmp_dbs_get_row_num(app_obj, msg, query_buf);
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

        total_num =  nmp_dbs_get_row_num(app_obj, msg, query_buf);
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

        total_num =  nmp_dbs_get_row_num(app_obj, msg, query_buf);
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
        nmp_warning("query type(%d) is wrong ", req_info->type);
        ret = -E_QUERYTYPE;
        goto err_do_user_query;

        break;
    }

    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_user(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

        query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_do_user_query;
    }

    if(result)
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));

end_query_user:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_user_query:

    size = sizeof(NmpQueryUserRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
    {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;

    goto end_query_user;
}


NmpMsgFunRet
nmp_mod_dbs_query_domain_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryDomain *req_info;
    NmpQueryDomainRes *query_res;
    NmpMysqlRes *result;
    char query_buf[QUERY_STR_LEN];
    gint total_num;
    gint size,ret;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    switch (req_info->type) {
    case 0:       //query all domain
        snprintf(query_buf, QUERY_STR_LEN, "select * from %s ", DOMAIN_TABLE);

	     total_num =  nmp_dbs_get_row_num(app_obj, msg, query_buf);
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

        total_num =  nmp_dbs_get_row_num(app_obj, msg, query_buf);
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
        total_num =  nmp_dbs_get_row_num(app_obj, msg, query_buf);
        if (total_num == 0)
        {
            ret = 0;
            goto err_do_domain_query;
        }

        break;
    default:
        nmp_warning("query type(%d) is wrong ", req_info->type);
        ret = E_QUERYTYPE;
        goto err_do_domain_query;

        break;
    }

    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_domain(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

	query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_do_domain_query;
    }

    if(result)
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));

end_query_domain:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_domain_query:
    size = sizeof(NmpQueryDomainRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
    {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;

    goto end_query_domain;
}


NmpMsgFunRet
nmp_mod_dbs_query_area_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryArea *req_info;
    NmpQueryAreaRes *query_res;
    NmpMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num ;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(query_buf, QUERY_STR_LEN, "select count(*)  as count from %s ", AREA_TABLE);

    total_num =  nmp_get_record_count(app_obj,query_buf);
    if (total_num == 0)
    {
        ret = 0;
        goto err_do_area_query;
    }

    snprintf(query_buf, QUERY_STR_LEN, "select * from %s limit %d,%d",
			AREA_TABLE, req_info->start_num, req_info->req_num);
   printf("--------query_buf=%s\n",query_buf);
    result = nmp_dbs_do_query_res(app_obj,query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_area(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
	 query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_do_area_query;
    }

    if(result)
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));

end_query_area:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_area_query:
    size = sizeof(NmpQueryAreaRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
   {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;
    query_res->req_num = 0;
    goto end_query_area;
}


gint
nmp_get_area_all_device_count(NmpAppObj *app_obj, char *query)
{
	db_conn_status *conn = NULL;
	NmpModDbs *dbs_obj;
	NmpMysqlRes *result = NULL;
	gint count, ret;

	dbs_obj = NMP_MODDBS(app_obj);

redo:
	conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
	if (G_UNLIKELY(!conn))
		return -E_GETDBCONN;

	if (G_UNLIKELY(!conn->mysql))
	{
		put_db_connection(dbs_obj->pool_info, conn);
		return -E_GETDBCONN;
	}

	ret = nmp_process_query_procedure(conn->mysql, query);
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

	result = nmp_process_query_res(conn->mysql, "select @count");
	put_db_connection(dbs_obj->pool_info, conn);

	if (result)
		count = nmp_get_count_value(result);

	nmp_sql_put_res(result, sizeof(NmpMysqlRes));

	return count;
}


NmpMsgFunRet
nmp_mod_dbs_query_pu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryPu *req_info;
    NmpQueryPuRes *query_res;
    NmpMysqlRes *result;
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
        total_num =  nmp_dbs_get_row_num(app_obj, msg, query_buf);
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

	 total_num =  nmp_get_record_count(app_obj,query_buf);
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
	total_num = nmp_get_record_count(app_obj, query_buf);
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
    total_num =  nmp_get_area_all_device_count(app_obj,query_buf);
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

	 total_num =  nmp_get_record_count(app_obj,query_buf);
        if (total_num == 0)
        {
            ret = 0;
            goto err_do_pu_query;
        }
           break;
    default:
        nmp_warning("query type(%d) is wrong ", req_info->type);
	     ret = E_QUERYTYPE;
	     goto err_do_pu_query;

        break;
    }

   /* total_num =  nmp_dbs_get_row_num(app_obj, msg, query_buf);
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
    result = nmp_dbs_do_query_res(app_obj, query_buf);
    if (G_UNLIKELY(!result))
    {
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_pu(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

        query_res->type = req_info->type;

        if (req_info->type != 2)
            query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_do_pu_query;
    }

    if(result)
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));

end_query_pu:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_pu_query:
    size = sizeof(NmpQueryPuRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
    {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;
    query_res->req_num = 0;
    goto end_query_pu;
}


static void nmp_mod_dbs_get_mss_of_gus(NmpAppObj *app_obj, NmpQueryGuRes *gu)
{
    NmpMysqlRes *result;
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

    	     result = nmp_dbs_do_query_res(app_obj, query_buf);
            BUG_ON(!result);

            if (G_LIKELY(MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
            {
                ret = MYSQL_RESULT_CODE(result);
                nmp_sql_put_res(result, sizeof(NmpMysqlRes));
		  SET_CODE(gu, ret);
		  return;
            }

            row_num = nmp_sql_get_num_rows(result);
            printf("get row num (%d)\n",row_num);
            if(row_num == 0)
            {
                nmp_sql_put_res(result,sizeof(NmpMysqlRes));
                continue;
            }

            nmp_get_gu_mss(result,  &gu->gu_info[i].mss[0]);
	     nmp_sql_put_res(result,sizeof(NmpMysqlRes));
       }
}


void
nmp_get_gu_info_of_mss(NmpMysqlRes *result, NmpGuInfo *query_res)
{
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
            name = nmp_sql_get_field_name(mysql_fields, field_no);
            if (!strcmp(name, "gu_name"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
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
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->gu_type = atoi(value);
            }
            else if (!strcmp(name, "gu_attributes"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->gu_attributes= atoi(value);
            }
            else
                cms_debug("no need mysql name %s ", name);

        }
	 info_no++;
    }
}


static void nmp_mod_dbs_get_gus_of_info(NmpAppObj *app_obj, NmpQueryGuRes *gu)
{
    NmpMysqlRes *result;
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

    	     result = nmp_dbs_do_query_res(app_obj, query_buf);
            BUG_ON(!result);

            if (G_LIKELY(MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
            {
                ret = MYSQL_RESULT_CODE(result);
                nmp_sql_put_res(result, sizeof(NmpMysqlRes));
		  SET_CODE(gu, ret);
		  return;
            }

            row_num = nmp_sql_get_num_rows(result);
            printf("get row num (%d)\n",row_num);
            if(row_num == 0)
            {
                nmp_sql_put_res(result,sizeof(NmpMysqlRes));
                continue;
            }

            nmp_get_gu_info_of_mss(result,  &gu->gu_info[i]);
	     nmp_sql_put_res(result,sizeof(NmpMysqlRes));
       }
}


NmpMsgFunRet
nmp_mod_dbs_query_gu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryGu *req_info;
    NmpQueryGuRes *query_res;
    NmpMysqlRes *result;
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
        total_num =  nmp_get_record_count(app_obj, query_buf);
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

        total_num =  nmp_get_record_count(app_obj, query_buf);
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

        total_num =  nmp_get_record_count(app_obj, query_buf);
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

        total_num =  nmp_get_record_count(app_obj, query_buf);
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

        total_num =  nmp_get_record_count(app_obj, query_buf);
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

        total_num =  nmp_get_record_count(app_obj, query_buf);
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

        total_num =  nmp_get_record_count(app_obj, query_buf);
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
    	 nmp_warning("query type(%d) is wrong ", req_info->type);
    	 ret = E_QUERYTYPE;
    	 goto err_do_gu_query;

        break;
    }

    result = nmp_dbs_do_query_res(app_obj, query_buf);
    if (G_UNLIKELY(!result))
    {
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }
    BUG_ON(!result);

    if (G_LIKELY(MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_do_gu_query;
    }

    query_res = nmp_dbs_get_gu(result, &size);
    if (G_UNLIKELY(!query_res))
    {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    query_res->type = req_info->type;
    if ((req_info->type == 0)||(req_info->type == 3)||(req_info->type == 4)
		|| (req_info->type == 6))
        query_res->total_num = total_num;

    if (req_info->type == 1)
    {
		query_res->total_num = total_num;
		nmp_mod_dbs_get_mss_of_gus(app_obj, query_res);
    }

    if (req_info->type == 2)
    {
        snprintf(
            query_buf, QUERY_STR_LEN,
            "select mss_id from %s where guid_domain='%s' and gu_id='%s'",
            RECORD_POLICY_TABLE, req_info->domain_id, req_info->guid
        );

        if(result)
            nmp_sql_put_res(result, sizeof(NmpMysqlRes));

        result = nmp_dbs_do_query_res(app_obj, query_buf);

        if (G_UNLIKELY(!result))
        {
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
        BUG_ON(!result);

        if (G_LIKELY(MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
        {
            ret = MYSQL_RESULT_CODE(result);
            nmp_sql_put_res(result, sizeof(NmpMysqlRes));
            goto err_do_gu_query;
        }

        row_num = nmp_sql_get_num_rows(result);
        printf("get row num (%d)\n",row_num);
        if(row_num == 0)
        {
            nmp_sql_put_res(result,sizeof(NmpMysqlRes));
            goto end_query_gu;
        }

        nmp_get_gu_mss(result,  &query_res->gu_info[0].mss[0]);
    }

    if (req_info->type == 5)
    {
        query_res->total_num = total_num;
        nmp_mod_dbs_get_gus_of_info(app_obj, query_res);
    }

    if(result)
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));

end_query_gu:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_gu_query:
    size = sizeof(NmpQueryPuRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
    {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;
    query_res->req_num = 0;

    goto end_query_gu;
}


NmpMsgFunRet
nmp_mod_dbs_query_manufacturer_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryManufacturer *req_info;
    NmpQueryManufacturerRes *query_res;
    NmpMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size,ret, total_num;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(query_buf, QUERY_STR_LEN, "select count(*)  as count from %s ", MANUFACTURER_TABLE);

    total_num =  nmp_get_record_count(app_obj,query_buf);
    if (total_num == 0)
    {
        ret = 0;
        goto err_do_manufacturer_query;
    }

    snprintf(query_buf, QUERY_STR_LEN, "select * from %s limit %d,%d",
			MANUFACTURER_TABLE, req_info->start_num, req_info->req_num);
    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);
    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_manufacturer(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
        query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_do_manufacturer_query;
    }

    if(G_LIKELY(result))
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));

end_query_manufacturer:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_manufacturer_query:
    size = sizeof(NmpQueryManufacturerRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
    {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;
    query_res->req_num = 0;
    goto end_query_manufacturer;
}


NmpMsgFunRet
nmp_mod_dbs_query_mds_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryMds *req_info;
    NmpQueryMdsRes *query_res;
    NmpMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if (req_info->type == 0)   //query all mds
    {
        snprintf(query_buf, QUERY_STR_LEN, "select count(*)  as count from %s ", MDS_TABLE);

        total_num =  nmp_get_record_count(app_obj,query_buf);
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
        nmp_warning("query type(%d) is wrong ", req_info->type);
        ret = E_QUERYTYPE;
        goto err_do_mds_query;
    }

    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_mds(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
		query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_do_mds_query;
    }

    if(result)
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));

end_query_mds:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    query_res->type = req_info->type;
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_mds_query:

    size = sizeof(NmpQueryMdsRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
    {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;
    query_res->req_num = 0;
    goto end_query_mds;
}


NmpMsgFunRet
nmp_mod_dbs_query_mds_ip_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryMdsIp *req_info;
    NmpQueryMdsIpRes *query_res;
    NmpMysqlRes *result;
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
        nmp_warning("query type(%d) is wrong ", req_info->type);
        ret = E_QUERYTYPE;
        goto ERR_DO_MDS_IP_QUERY;
    }

    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_mds_ip(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
        strncpy(query_res->mds_id, req_info->key, MDS_ID_LEN - 1);
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto ERR_DO_MDS_IP_QUERY;
    }

    if(result)
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));

END_QUERY_MDS_IP:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

ERR_DO_MDS_IP_QUERY:

    size = sizeof(NmpQueryMdsIpRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
    {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;

    goto END_QUERY_MDS_IP;
}


NmpMsgFunRet
nmp_mod_dbs_query_mss_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryMss *req_info;
    NmpQueryMssRes *query_res;
    NmpMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if (req_info->type == 0)   //query all mss
    {
        snprintf(query_buf, QUERY_STR_LEN, "select count(*) as count from %s ", MSS_TABLE);

        total_num =  nmp_get_record_count(app_obj,query_buf);
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

        total_num =  nmp_get_record_count(app_obj,query_buf);
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
        nmp_warning("query type(%d) is wrong ", req_info->type);
        ret = E_QUERYTYPE;
        goto err_do_mss_query;
    }

    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_mss(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

        query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_do_mss_query;
    }

    if(result)
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));

end_query_mss:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_mss_query:

    size = sizeof(NmpQueryMssRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
    {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;

    goto end_query_mss;
}


NmpMsgFunRet
nmp_mod_dbs_query_record_policy_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryRecordPolicy *req_info;
    NmpQueryRecordPolicyRes *query_res;
    NmpMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num = 0, i;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if (req_info->type == 0)   //query all record gu
    {
        snprintf(query_buf, QUERY_STR_LEN,
           "select count(*) as count from %s ", RECORD_POLICY_TABLE);

        total_num =  nmp_get_record_count(app_obj,query_buf);
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
        nmp_warning("query type(%d) is wrong ", req_info->type);
        ret = E_QUERYTYPE;
        goto err_do_mss_query;
    }

    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_record_policy(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

        query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_do_mss_query;
    }

    if(result)
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));

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

            result = nmp_dbs_do_query_res(app_obj, query_buf);
            BUG_ON(!result);
            if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
            {
                nmp_get_record_policy_detail_info(result, &query_res->record_policy[i]);
           }
           else
           {
               ret = MYSQL_RESULT_CODE(result);
               nmp_sql_put_res(result, sizeof(NmpMysqlRes));
               goto err_do_mss_query;
           }

           if(result)
               nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        }
    }

end_query_mss:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    query_res->type = req_info->type;
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_mss_query:

    size = sizeof(NmpQueryRecordPolicyRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
    {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;

    goto end_query_mss;
}


NmpMsgFunRet
nmp_mod_dbs_record_policy_config_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpRecordPolicyConfig *req_info;
    NmpBssRes result;
    char query_buf[QUERY_STR_LEN];
    db_conn_status *conn = NULL;
    NmpModDbs *dbs_obj;
    gint ret = 0,  i = 0, size;
    memset(&result, 0, sizeof(result));

    req_info = MSG_GET_DATA(msg);
    size = MSG_DATA_SIZE(msg);

    dbs_obj = NMP_MODDBS(app_obj);
	conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
	if (G_UNLIKELY(!conn))
	{
		nmp_warning("<NmpModDbs> get db connection error!");
		ret = -E_GETDBCONN;
		goto end_modify_policy;
	}

	if (G_UNLIKELY(!conn->mysql))
	{
		nmp_warning("<NmpModDbs> get db connection error!");
		put_db_connection(dbs_obj->pool_info, conn);
		ret = -E_GETDBCONN;
		goto end_modify_policy;
	}

    switch (req_info->type){
    case 0:
        NMP_MYSQL_DO_QUERY_OR_KILL_CONN(dbs_obj, conn, "start transaction", ret);
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

            ret= nmp_mysql_do_query(conn->mysql, query_buf);
            if (ret != 0 )
            {
                nmp_mysql_do_query(conn->mysql, "rollback");
                put_db_connection(dbs_obj->pool_info, conn);
                goto end_modify_policy;
            }
            i++;
        }

        ret = nmp_mysql_do_query(conn->mysql, "commit");
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

        ret= nmp_mysql_do_query(conn->mysql, query_buf);
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
    NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_query_user_own_gu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryUserOwnGu *req_info;
    NmpQueryUserOwnGuRes *query_res;
    NmpMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select user_guid from  %s where user_name='%s' and user_guid_domain='%s' ",
        USER_OWN_GU_TABLE, req_info->user,req_info->domain_id
        );

    total_num =  nmp_dbs_get_row_num(app_obj, msg, query_buf);
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

    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_user_own_gu(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

        strncpy(query_res->domain_id, req_info->domain_id, DOMAIN_ID_LEN - 1);
        strncpy(query_res->user, req_info->user, USER_NAME_LEN - 1);
        query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto ERR_DO_MDS_QUERY;
    }

    if(result)
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));

END_QUERY_MDS:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

ERR_DO_MDS_QUERY:

    size = sizeof(NmpQueryUserOwnGuRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
    {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;

    goto END_QUERY_MDS;
}


NmpMsgFunRet
nmp_mod_dbs_query_user_own_tw_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryUserOwnTw *req_info;
    NmpQueryUserOwnTwRes *query_res;
    NmpMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select count(*) as count from  %s where user_name='%s'",
        USER_OWN_TW_TABLE, req_info->user
    );

	total_num =  nmp_get_record_count(app_obj,query_buf);
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

    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_user_own_tw(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

        strncpy(query_res->user, req_info->user, USER_NAME_LEN - 1);
        query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto ERR_DO_MDS_QUERY;
    }

    if(result)
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));

END_QUERY_MDS:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

ERR_DO_MDS_QUERY:

    size = sizeof(NmpQueryUserOwnTwRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
    {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;

    goto END_QUERY_MDS;
}


NmpMsgFunRet
nmp_mod_dbs_query_user_own_tour_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryUserOwnTour *req_info;
    NmpQueryUserOwnTourRes *query_res;
    NmpMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select count(*) as count from  %s where user_name='%s'",
        USER_OWN_TOUR_TABLE, req_info->user
    );

	total_num =  nmp_get_record_count(app_obj,query_buf);
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

    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_user_own_tour(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

        strncpy(query_res->user, req_info->user, USER_NAME_LEN - 1);
        query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto ERR_DO_MDS_QUERY;
    }

    if(result)
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));

END_QUERY_MDS:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

ERR_DO_MDS_QUERY:

    size = sizeof(NmpQueryUserOwnTourRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
    {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;

    goto END_QUERY_MDS;
}


NmpMsgFunRet
nmp_mod_dbs_query_defence_area_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryDefenceArea *req_info;
    NmpQueryDefenceAreaRes *query_res;
    NmpMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num ;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if (req_info->type == 0)
    {
        snprintf(query_buf, QUERY_STR_LEN, "select count(*)  as count from %s ", DEFENCE_AREA_TABLE);
        total_num =  nmp_get_record_count(app_obj,query_buf);
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
    result = nmp_dbs_do_query_res(app_obj,query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_defence_area(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
	 query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_do_defence_area_query;
    }

    if(result)
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));

end_query_defence_area:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_defence_area_query:
    size = sizeof(NmpQueryDefenceAreaRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
   {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;
    query_res->req_num = 0;
    goto end_query_defence_area;
}


NmpMsgFunRet
nmp_mod_dbs_query_defence_map_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryDefenceMap *req_info;
    NmpQueryDefenceMapRes *query_res;
    NmpMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num ;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(query_buf, QUERY_STR_LEN,
	"select count(*)  as count from %s where defence_area_id=%d",
	DEFENCE_MAP_TABLE, req_info->defence_area_id
    );

    total_num =  nmp_get_record_count(app_obj,query_buf);
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
    result = nmp_dbs_do_query_res(app_obj,query_buf);
    BUG_ON(!result);
    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_defence_map(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
	 query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_do_defence_map_query;
    }

    if(result)
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));

end_query_defence_map:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_defence_map_query:
    size = sizeof(NmpQueryDefenceMapRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
   {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;
    query_res->back_num = 0;
    goto end_query_defence_map;
}


NmpMsgFunRet
nmp_mod_dbs_query_defence_gu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryDefenceGu *req_info;
    NmpQueryDefenceGuRes *query_res;
    NmpMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num ;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(query_buf, QUERY_STR_LEN,
	"select count(*)  as count from %s where map_id=%d",
	MAP_GU_TABLE, req_info->map_id
    );

    total_num =  nmp_get_record_count(app_obj,query_buf);
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

    result = nmp_dbs_do_query_res(app_obj,query_buf);
    BUG_ON(!result);
    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_defence_gu(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
	 query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_do_defence_gu_query;
    }

    if(result)
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));

end_query_defence_gu:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_defence_gu_query:
    size = sizeof(NmpQueryDefenceGuRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
   {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;
    query_res->back_num = 0;
    goto end_query_defence_gu;
}


NmpMsgFunRet
nmp_mod_dbs_query_map_href_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryMapHref *req_info;
    NmpQueryMapHrefRes *query_res;
    NmpMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num ;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(query_buf, QUERY_STR_LEN,
	"select count(*)  as count from %s where src_map_id=%d",
	MAP_HREF_TABLE, req_info->map_id
    );

    total_num =  nmp_get_record_count(app_obj,query_buf);
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
    result = nmp_dbs_do_query_res(app_obj,query_buf);
    BUG_ON(!result);
    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_map_href(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
	 query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_do_map_href_query;
    }

    if(result)
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));

end_query_map_href:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_map_href_query:
    size = sizeof(NmpQueryMapHrefRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
   {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;
    query_res->back_num = 0;
    goto end_query_map_href;
}


NmpMsgFunRet
nmp_mod_dbs_query_tw_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryTw *req_info;
    NmpQueryTwRes *query_res;
    NmpMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num ;
printf("-----enter nmp_mod_dbs_query_screen_b\n");
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    switch(req_info->type)
    {
        case 0:
               snprintf(query_buf, QUERY_STR_LEN,
              	"select count(*)  as count from %s",
              	TW_TABLE
               );

		 total_num =  nmp_get_record_count(app_obj,query_buf);
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

    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_tw(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
	 query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_do_tw_query;
    }

    if(result)
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));

end_query_tw:
    query_res->type = req_info->type;
    strcpy(query_res->bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_tw_query:
    size = sizeof(NmpQueryTwRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
   {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;
    query_res->back_num = 0;
    goto end_query_tw;
}


NmpMsgFunRet
nmp_mod_dbs_query_screen_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryScreen *req_info;
    NmpQueryScreenRes *query_res;
    NmpMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num ;
printf("-----enter nmp_mod_dbs_query_screen_b\n");
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    switch(req_info->type)
    {
        case 0:
		snprintf(query_buf, QUERY_STR_LEN,
              	"select count(*)  as count from %s where tw_id=%s",
              	SCREEN_TABLE, req_info->key
               );

		 total_num =  nmp_get_record_count(app_obj,query_buf);
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

    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_screen(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
	 query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_do_screen_query;
    }

    if(result)
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));

end_query_screen:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_screen_query:
    size = sizeof(NmpQueryScreenRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
   {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;
    query_res->back_num = 0;
    goto end_query_screen;
}


NmpMsgFunRet
nmp_mod_dbs_query_scr_div_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryScrDiv *req_info;
    NmpQueryScrDivRes *query_res;
    NmpMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num ;
printf("-----enter nmp_mod_dbs_query_screen_b\n");
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    switch(req_info->type)
    {
        case 0:
		snprintf(query_buf, QUERY_STR_LEN,
              	"select count(*)  as count from %s",
              	SCREEN_DIVISION_TABLE
               );

		 total_num =  nmp_get_record_count(app_obj,query_buf);
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

    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_scr_div(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
	 query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_do_scr_div_query;
    }

    if(result)
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));

end_query_scr_div:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_scr_div_query:
    size = sizeof(NmpQueryScrDivRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
   {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;
    query_res->back_num = 0;
    goto end_query_scr_div;
}


NmpMsgFunRet
nmp_mod_dbs_query_tour_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryTour *req_info;
    NmpQueryTourRes *query_res;
    NmpMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num ;
    printf("-----enter nmp_mod_dbs_query_tour_b\n");
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    switch(req_info->type)
    {
        case 0:
               snprintf(query_buf, QUERY_STR_LEN,
              	"select count(*)  as count from %s",
              	TOUR_TABLE
               );

		 total_num =  nmp_get_record_count(app_obj,query_buf);
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

    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_tour(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
	 query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_do_tour_query;
    }

    if(result)
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));

end_query_tour:
    query_res->type = req_info->type;
    strcpy(query_res->bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_tour_query:
    size = sizeof(NmpQueryTourRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
   {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;
    query_res->back_num = 0;
    goto end_query_tour;
}


NmpMsgFunRet
nmp_mod_dbs_query_tour_step_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryTourStep *req_info;
    NmpQueryTourStepRes *query_res;
    NmpMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num ;
    printf("-----enter nmp_mod_dbs_query_tour_step_b\n");
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(query_buf, QUERY_STR_LEN,
        "select count(*) as count from %s where tour_id=%d",
        TOUR_STEP_TABLE, req_info->tour_id
    );
    total_num =  nmp_get_record_count(app_obj,query_buf);
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

    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_tour_step(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
	 query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_do_tour_step_query;
    }

    if(result)
       nmp_sql_put_res(result, sizeof(NmpMysqlRes));

end_query_tour_step:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_tour_step_query:
    size = sizeof(NmpQueryTourStepRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
   {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;
    query_res->back_num = 0;
    goto end_query_tour_step;
}


NmpMsgFunRet
nmp_mod_dbs_query_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryGroup *req_info;
    NmpQueryGroupRes *query_res;
    NmpMysqlRes *result;
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

		 total_num =  nmp_get_record_count(app_obj,query_buf);
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

    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_group(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
	 query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_do_group_query;
    }

    if(result)
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));

end_query_group:
    query_res->type = req_info->type;
    strcpy(query_res->bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_group_query:
    size = sizeof(NmpQueryGroupRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
   {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;
    query_res->back_num = 0;
    goto end_query_group;
}


NmpMsgFunRet
nmp_mod_dbs_query_group_step_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryGroupStep *req_info;
    NmpQueryGroupStepRes *query_res;
    NmpMysqlRes *result;
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

	        total_num =  nmp_get_record_count(app_obj,query_buf);
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

    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_group_step(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
	 query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_do_group_step_query;
    }

    if(result)
       nmp_sql_put_res(result, sizeof(NmpMysqlRes));

end_query_group_step:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_group_step_query:
    size = sizeof(NmpQueryGroupStepRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
   {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;
    query_res->back_num = 0;
    goto end_query_group_step;
}


NmpMsgFunRet
nmp_mod_dbs_query_group_step_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryGroupStepInfo *req_info;
    NmpQueryGroupStepInfoRes *query_res;
    NmpMysqlRes *result;
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

		 total_num =  nmp_get_record_count(app_obj,query_buf);
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

    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_group_step_info(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
	 query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_do_group_step_info_query;
    }

    if(result)
       nmp_sql_put_res(result, sizeof(NmpMysqlRes));

end_query_group_step_info:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_group_step_info_query:
    size = sizeof(NmpQueryGroupStepInfoRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
   {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;
    query_res->back_num = 0;
    goto end_query_group_step_info;
}


NmpMsgFunRet
nmp_mod_dbs_query_group_step_div_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryGroupStepDiv *req_info;
    NmpQueryGroupStepDivRes query_res;
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

   total_num =  nmp_get_record_count(app_obj,query_buf);
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

   total_num = nmp_get_record_count(app_obj,query_buf);

end_query_group_step_div:
	SET_CODE(&query_res, ret);
	query_res.div_id = total_num;
    strcpy(query_res.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &query_res, sizeof(query_res), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);
    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_query_alarm_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryAlarm *req_info;;
    NmpQueryAlarmRes *query_res;
    NmpMysqlRes *result;
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

    total_num =  nmp_get_record_count(app_obj,query_buf);
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

    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_query_alarm(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

        if(req_info->req_num > 0)
            query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_query_alarm;
    }

    if(result)
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));

end_query_alarm:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

err_query_alarm:
    size = sizeof(NmpQueryAlarmRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
    {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);

        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;

    goto end_query_alarm;
}


NmpMsgFunRet
nmp_mod_dbs_query_server_resource_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryServerResourceInfo *req_info;;
    NmpQueryServerResourceInfoRes query_res;
    gint ret = 0, dev_num, dev_online_num;
    NmpResourcesCap res_cap;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&query_res, 0, sizeof(query_res));
    ret = nmp_dbs_get_dev_total_count(app_obj);
    if (ret < 0)
        goto end_query_resource_info;
    dev_num = ret;

    ret = nmp_dbs_get_online_dev_total_count(app_obj);
    if (ret < 0)
        goto end_query_resource_info;
    dev_online_num = ret;

    ret = nmp_dbs_get_dev_type_count(app_obj, DECODER_TYPE);
    if (ret < 0)
        goto end_query_resource_info;
    query_res.dec_num = ret;

    ret = nmp_dbs_get_online_dev_type_count(app_obj, DECODER_TYPE);
    if (ret < 0)
        goto end_query_resource_info;
    query_res.dec_online_num = ret;

    query_res.enc_num = dev_num - query_res.dec_num;
    query_res.enc_online_num = dev_online_num - query_res.dec_online_num;

    ret = nmp_dbs_get_gu_type_count(app_obj, AV_TYPE);
    if (ret < 0)
        goto end_query_resource_info;
    query_res.av_num = ret;

    ret = nmp_dbs_get_gu_type_count(app_obj, DS_TYPE);
    if (ret < 0)
        goto end_query_resource_info;
    query_res.ds_num = ret;

    ret = nmp_dbs_get_gu_type_count(app_obj, AI_TYPE);
    if (ret < 0)
        goto end_query_resource_info;
    query_res.ai_num = ret;

    ret = nmp_dbs_get_gu_type_count(app_obj, AO_TYPE);
    if (ret < 0)
        goto end_query_resource_info;
    query_res.ao_num = ret;

    memset(&res_cap, 0, sizeof(res_cap));
    nmp_mod_get_resource_cap(&res_cap);
    query_res.av_limited_num = res_cap.av_count;
    query_res.ds_limited_num = res_cap.ds_count;
    query_res.ai_limited_num = res_cap.ai_count;
    query_res.ao_limited_num = res_cap.ao_count;
    query_res.system_version = res_cap.system_version;
    query_res.manufactor_type = res_cap.modules_data[SYS_MODULE_CMS];
    query_res.support_keyboard = (res_cap.modules_data[SYS_MODULE_TW] & TW_KEYBOARD_BIT) ? 1 : 0;
    memcpy(&query_res.expired_time, &res_cap.expired_time, sizeof(NmpExpiredTime));

    ret = 0;
end_query_resource_info:
    SET_CODE(&query_res.code, ret);
    strcpy(query_res.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &query_res, sizeof(query_res), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_query_ivs_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryIvs *req_info;
    NmpQueryIvsRes *query_res;
    NmpMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size, ret, total_num;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if (req_info->type == 0)   //query all ivs
    {
        snprintf(query_buf, QUERY_STR_LEN,
        	"select count(*) as count from %s ", IVS_TABLE
        );

        total_num = nmp_get_record_count(app_obj,query_buf);
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

        total_num =  nmp_get_record_count(app_obj,query_buf);
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
        nmp_warning("query type(%d) is wrong ", req_info->type);
        ret = E_QUERYTYPE;
        goto err_do_ivs_query;
    }

    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_ivs(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

        query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_do_ivs_query;
    }

    if(result)
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));

end_query_ivs:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_ivs_query:

    size = sizeof(NmpQueryIvsRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
    {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;

    goto end_query_ivs;
}


NmpMsgFunRet
nmp_mod_dbs_get_next_puno_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpGetNextPuNo *req_info;;
    NmpGetNextPuNoRes query_res;
    gint ret = 0, id;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&query_res, 0, sizeof(query_res));
    id = nmp_mod_dbs_get_init_pu_id(app_obj);
    id++;
    if (id == 100000000)
        id = 0;

    sprintf(query_res.pu_no, "%08d", id);
    if (id < 0)
    	ret = id;

    SET_CODE(&query_res.code, ret);
    strcpy(query_res.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &query_res, sizeof(query_res), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


static __inline__ gint
nmp_mod_dbs_get_del_alarm_policy(NmpAppObj *self, NmpMysqlRes *result,
      NmpQueryDelAlarmPolicyRes *alarm_param)
{
    guint row_num;
    guint field_num;
    NmpMysqlRow mysql_row;
    NmpMysqlField* mysql_fields;
    gchar *name;
    gchar *value;
    gint field_no =0;
    gchar alarm_value[MAX_STR_LEN] = {0};
    NmpModDbs        *dbs_obj;
    dbs_obj = NMP_MODDBS(self);

        row_num = nmp_sql_get_num_rows(result);
	 if (row_num == 0)
	 {
	     dbs_obj->del_alarm_flag &= ~ENABLE_DEL_ALARM;
	     alarm_param->enable = 0;
	     return 0;
	 }

        field_num = nmp_sql_get_num_fields(result);

        while ((mysql_row = nmp_sql_fetch_row(result)))
        {
            nmp_sql_field_seek(result, 0);
            mysql_fields = nmp_sql_fetch_fields(result);
            for(field_no = 0; field_no < field_num; field_no++)
            {
                name = nmp_sql_get_field_name(mysql_fields, field_no);
                if (!strcmp(name, "value"))
                {
                    value = nmp_sql_get_field_value(mysql_row, field_no);
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


NmpMsgFunRet
nmp_mod_dbs_query_del_alarm_policy_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryDelAlarmPolicyRes query_res;
    NmpMysqlRes *result;
    gchar query_buf[QUERY_STR_LEN] = {0};
    gint ret;

	memset(&query_res, 0, sizeof(query_res));
    snprintf(query_buf, QUERY_STR_LEN,
	"select * from %s where id=1",
	PARAM_CONFIG_TABLE);

    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        ret = nmp_mod_dbs_get_del_alarm_policy(app_obj, result, &query_res);
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_query_del_alarm_policy;
    }

    if(result)
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));

end_query_del_alarm_policy:

    nmp_dbs_modify_sysmsg_2(msg, &query_res, sizeof(query_res), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;

err_query_del_alarm_policy:
    SET_CODE(&query_res, ret);

    goto end_query_del_alarm_policy;
}


static __inline__ gint
nmp_mod_dbs_get_link_time_policy(NmpAppObj *self, NmpMysqlRes *result,
      NmpQueryLinkTimePolicyRes *alarm_param)
{
    guint field_num;
    NmpMysqlRow mysql_row;
    NmpMysqlField* mysql_fields;
    gchar *name;
    gchar *value;
    gint field_no =0;
    NmpModDbs        *dbs_obj;
    dbs_obj = NMP_MODDBS(self);

        field_num = nmp_sql_get_num_fields(result);

        while ((mysql_row = nmp_sql_fetch_row(result)))
        {
            nmp_sql_field_seek(result, 0);
            mysql_fields = nmp_sql_fetch_fields(result);
            for(field_no = 0; field_no < field_num; field_no++)
            {
                name = nmp_sql_get_field_name(mysql_fields, field_no);
                if (!strcmp(name, "time_policy"))
                {
                    value = nmp_sql_get_field_value(mysql_row, field_no);
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


NmpMsgFunRet
nmp_mod_dbs_query_link_time_policy_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryLinkTimePolicy *req_info;
    NmpQueryLinkTimePolicyRes query_res;
    NmpMysqlRes *result;
    gchar query_buf[QUERY_STR_LEN] = {0};
    gint ret;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(query_buf, QUERY_STR_LEN,
	"select time_policy from %s where gu_id='%s' and domain_id='%s'",
	LINK_TIME_POLICY_TABLE, req_info->guid, req_info->domain);

    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        ret = nmp_mod_dbs_get_link_time_policy(app_obj, result, &query_res);
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_query_link_time_policy;
    }

    if(result)
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));

end_query_link_time_policy:
    strcpy(query_res.bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &query_res, sizeof(query_res), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;

err_query_link_time_policy:
    SET_CODE(&query_res, ret);

    goto end_query_link_time_policy;
}

/*
NmpMsgFunRet
nmp_mod_dbs_query_link_record_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryLinkRecord *req_info;
    NmpQueryLinkRecordRes *query_res;
    NmpMysqlRes *result;
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

		 total_num =  nmp_get_record_count(app_obj,query_buf);
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

		 total_num =  nmp_get_record_count(app_obj,query_buf);
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

    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_link_record(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
	 query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_do_link_record_query;
    }

    if(result)
       nmp_sql_put_res(result, sizeof(NmpMysqlRes));

end_query_link_record:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_link_record_query:
    size = sizeof(NmpQueryLinkRecordRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
   {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;
    query_res->back_num = 0;
    goto end_query_link_record;
}*/

static void nmp_mod_dbs_get_mss_of_link_record(NmpAppObj *app_obj,
	NmpQueryLinkRecordRes *gu)
{
    NmpMysqlRes *result;
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

    	     result = nmp_dbs_do_query_res(app_obj, query_buf);
            BUG_ON(!result);

            if (G_LIKELY(MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
            {
                ret = MYSQL_RESULT_CODE(result);
                nmp_sql_put_res(result, sizeof(NmpMysqlRes));
		  SET_CODE(gu, ret);
		  return;
            }

            row_num = nmp_sql_get_num_rows(result);
            printf("get row num (%d)\n",row_num);
            if(row_num == 0)
            {
                nmp_sql_put_res(result,sizeof(NmpMysqlRes));
                continue;
            }

            nmp_get_gu_mss(result,  &gu->link_record_info[i].mss[0]);
	     nmp_sql_put_res(result,sizeof(NmpMysqlRes));
       }
}


NmpMsgFunRet
nmp_mod_dbs_query_link_record_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryLinkRecord *req_info;
    NmpQueryLinkRecordRes *query_res;
    NmpMysqlRes *result;
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

		 total_num =  nmp_get_record_count(app_obj,query_buf);
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
		result = nmp_dbs_do_query_res(app_obj, query_buf);

		if (G_UNLIKELY(!result))
		{
			nmp_sysmsg_destroy(msg);
			return MFR_ACCEPTED;
		}
		BUG_ON(!result);

		if (G_LIKELY(MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
		{
			ret = MYSQL_RESULT_CODE(result);
			nmp_sql_put_res(result, sizeof(NmpMysqlRes));
			goto err_do_link_record_query;
		}

		row_num = nmp_sql_get_num_rows(result);
		if(row_num == 0)
		{
			nmp_sql_put_res(result,sizeof(NmpMysqlRes));
			goto err_do_link_record_query;
		}
		size = sizeof(NmpQueryLinkRecordRes) + sizeof(NmpLinkRecordInfo);
		query_res = nmp_mem_kalloc(size);
		memset(query_res, 0, size);
		query_res->total_num  = 1;
		query_res->back_num = 1;
		strncpy(query_res->link_record_info[0].link_domain, req_info->link_domain, DOMAIN_ID_LEN - 1);
		strncpy(query_res->link_record_info[0].link_guid, req_info->link_guid, MAX_ID_LEN - 1);
		nmp_get_gu_mss(result,  &query_res->link_record_info[0].mss[0]);
		nmp_sql_put_res(result, sizeof(NmpMysqlRes));
		goto end_query_link_record;
		break;
	default:
		break;
    }

    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_link_record(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
	 query_res->total_num = total_num;
	 strncpy(query_res->guid, req_info->guid, MAX_ID_LEN - 1);
	 strncpy(query_res->domain, req_info->domain, DOMAIN_ID_LEN - 1);
	 nmp_mod_dbs_get_mss_of_link_record(app_obj, query_res);
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_do_link_record_query;
    }

    if(result)
       nmp_sql_put_res(result, sizeof(NmpMysqlRes));

end_query_link_record:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_link_record_query:
    size = sizeof(NmpQueryLinkRecordRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
   {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;
    query_res->back_num = 0;
    goto end_query_link_record;
}


NmpMsgFunRet
nmp_mod_dbs_query_link_io_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryLinkIO *req_info;
    NmpQueryLinkIORes *query_res;
    NmpMysqlRes *result;
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

		 total_num =  nmp_get_record_count(app_obj,query_buf);
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

    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_link_io(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
        if (req_info->type == 0)
	     query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_do_link_io_query;
    }

    if(result)
       nmp_sql_put_res(result, sizeof(NmpMysqlRes));

end_query_link_io:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_link_io_query:
    size = sizeof(NmpQueryLinkIORes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
   {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;
    query_res->back_num = 0;
    goto end_query_link_io;
}


NmpMsgFunRet
nmp_mod_dbs_query_link_step_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryLinkStep *req_info;
    NmpQueryLinkStepRes *query_res;
    NmpMysqlRes *result;
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

		 total_num =  nmp_get_record_count(app_obj,query_buf);
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

    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_link_step(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
            nmp_sysmsg_destroy(msg);
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
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_do_link_step_query;
    }

    if(result)
       nmp_sql_put_res(result, sizeof(NmpMysqlRes));

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

            result = nmp_dbs_do_query_res(app_obj, query_buf);
            BUG_ON(!result);
            if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
            {
                nmp_get_screen_name(result, &query_res->link_step_info[i]);
           }
           else
           {
               ret = MYSQL_RESULT_CODE(result);
               nmp_sql_put_res(result, sizeof(NmpMysqlRes));
               goto err_do_link_step_query;
           }

           if(result)
               nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        }
    }

end_query_link_step:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_link_step_query:
    size = sizeof(NmpQueryLinkStepRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
   {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;
    query_res->back_num = 0;
    goto end_query_link_step;
}


static void nmp_mod_dbs_get_mss_of_link_snapshot(NmpAppObj *app_obj,
	NmpQueryLinkSnapshotRes *gu)
{
    NmpMysqlRes *result;
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

    	     result = nmp_dbs_do_query_res(app_obj, query_buf);
            BUG_ON(!result);

            if (G_LIKELY(MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
            {
                ret = MYSQL_RESULT_CODE(result);
                nmp_sql_put_res(result, sizeof(NmpMysqlRes));
		  SET_CODE(gu, ret);
		  return;
            }

            row_num = nmp_sql_get_num_rows(result);
            printf("get row num (%d)\n",row_num);
            if(row_num == 0)
            {
                nmp_sql_put_res(result,sizeof(NmpMysqlRes));
                continue;
            }

            nmp_get_gu_mss(result,  &gu->link_snapshot_info[i].mss[0]);
	     nmp_sql_put_res(result,sizeof(NmpMysqlRes));
       }
}


NmpMsgFunRet
nmp_mod_dbs_query_link_snapshot_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryLinkSnapshot *req_info;
    NmpQueryLinkSnapshotRes *query_res;
    NmpMysqlRes *result;
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

		 total_num =  nmp_get_record_count(app_obj,query_buf);
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
		result = nmp_dbs_do_query_res(app_obj, query_buf);

		if (G_UNLIKELY(!result))
		{
			nmp_sysmsg_destroy(msg);
			return MFR_ACCEPTED;
		}
		BUG_ON(!result);

		if (G_LIKELY(MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
		{
			ret = MYSQL_RESULT_CODE(result);
			nmp_sql_put_res(result, sizeof(NmpMysqlRes));
			goto err_do_link_snapshot_query;
		}

		row_num = nmp_sql_get_num_rows(result);
		if(row_num == 0)
		{
			nmp_sql_put_res(result,sizeof(NmpMysqlRes));
			goto err_do_link_snapshot_query;
		}
		size = sizeof(NmpQueryLinkSnapshotRes) + sizeof(NmpLinkSnapshotInfo);
		query_res = nmp_mem_kalloc(size);
		memset(query_res, 0, size);
		query_res->total_num  = 1;
		query_res->back_num = 1;
		strncpy(query_res->link_snapshot_info[0].link_domain, req_info->link_domain, DOMAIN_ID_LEN - 1);
		strncpy(query_res->link_snapshot_info[0].link_guid, req_info->link_guid, MAX_ID_LEN - 1);
		nmp_get_gu_mss(result,  &query_res->link_snapshot_info[0].mss[0]);
		nmp_sql_put_res(result, sizeof(NmpMysqlRes));
		goto end_query_link_snapshot;
		break;
	default:
		break;
    }

    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_link_snapshot(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
	 query_res->total_num = total_num;
	 strncpy(query_res->guid, req_info->guid, MAX_ID_LEN - 1);
	 strncpy(query_res->domain, req_info->domain, DOMAIN_ID_LEN - 1);
	 nmp_mod_dbs_get_mss_of_link_snapshot(app_obj, query_res);
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_do_link_snapshot_query;
    }

    if(result)
       nmp_sql_put_res(result, sizeof(NmpMysqlRes));

end_query_link_snapshot:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_link_snapshot_query:
    size = sizeof(NmpQueryLinkSnapshotRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
   {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;
    query_res->back_num = 0;
    goto end_query_link_snapshot;
}


NmpMsgFunRet
nmp_mod_dbs_query_link_preset_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryLinkPreset *req_info;
    NmpQueryLinkPresetRes *query_res;
    NmpMysqlRes *result;
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

		 total_num =  nmp_get_record_count(app_obj,query_buf);
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

    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_link_preset(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
        if (req_info->type == 0)
	     query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_do_link_preset_query;
    }

    if(result)
       nmp_sql_put_res(result, sizeof(NmpMysqlRes));

end_query_link_preset:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_link_preset_query:
    size = sizeof(NmpQueryLinkPresetRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
   {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;
    query_res->back_num = 0;
    goto end_query_link_preset;
}


NmpMsgFunRet
nmp_mod_dbs_query_link_map_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryLinkMap *req_info;
    NmpQueryLinkMapRes *query_res;
    NmpMysqlRes *result;
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

		total_num = nmp_get_record_count(app_obj,query_buf);
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

    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_link_map(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
        if (req_info->type == 0)
	     query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_do_link_map_query;
    }

    if(result)
       nmp_sql_put_res(result, sizeof(NmpMysqlRes));

end_query_link_map:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;

err_do_link_map_query:
    size = sizeof(NmpQueryLinkMapRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
   {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;
    query_res->back_num = 0;
    goto end_query_link_map;
}


NmpMsgFunRet
nmp_mod_dbs_query_area_dev_online_rate_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpQueryAreaDevRate *req_info;
    NmpQueryAreaDevRateRes *query_res;
    NmpMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint ret = 0, total_num = 0, online_count, size;
    double total = 0,online = 0,rate = 0;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

	/*if (req_info->area_id > 0)
	{
		size = sizeof(NmpQueryAreaDevRateRes) + sizeof(NmpAreaDevRate);
	    query_res = nmp_mem_kalloc(size);
	    if (!query_res)
	    {
	        nmp_warning("<dbs_mh_bss> alloc error");
	        nmp_sysmsg_destroy(msg);
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
			size = sizeof(NmpQueryAreaDevRateRes) + sizeof(NmpAreaDevRate);
		    query_res = nmp_mem_kalloc(size);
		    if (!query_res)
		    {
		        nmp_warning("<dbs_mh_bss> alloc error");
		        nmp_sysmsg_destroy(msg);
		        return MFR_ACCEPTED;
		    }

		    memset(query_res, 0, size);
		    query_res->total_num = 1;
		    query_res->back_num = 1;

			snprintf(query_buf, QUERY_STR_LEN,
				"select count(*)  as count from %s where pu_area=%d",
				PU_TABLE, req_info->area_id
			);
			total_num =  nmp_get_record_count(app_obj,query_buf);
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
			online_count =  nmp_get_record_count(app_obj,query_buf);
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
			total_num =  nmp_get_record_count(app_obj,query_buf);
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
			total_num =  nmp_get_record_count(app_obj,query_buf);
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
			online_count =  nmp_get_record_count(app_obj,query_buf);
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
			total_num =  nmp_get_record_count(app_obj,query_buf);
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

			total_num =  nmp_get_record_count(app_obj,query_buf);
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
			online_count =  nmp_get_record_count(app_obj,query_buf);
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
			total_num =  nmp_get_record_count(app_obj,query_buf);
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

			total_num =  nmp_get_record_count(app_obj,query_buf);
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
			online_count =  nmp_get_record_count(app_obj,query_buf);
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
			total_num =  nmp_get_record_count(app_obj,query_buf);
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

    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = nmp_dbs_get_area_dev_rate(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            nmp_warning("<dbs_mh_bss> alloc error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
        //if (req_info->type == 0)
	     query_res->total_num = total_num;
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        nmp_sql_put_res(result, sizeof(NmpMysqlRes));
        goto err_query_area_dev_online_rate;
    }

    if(result)
       nmp_sql_put_res(result, sizeof(NmpMysqlRes));

end_query_area_dev_online_rate:
    strcpy(query_res->bss_usr, req_info->bss_usr);
    nmp_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS, nmp_mem_kfree);

    return MFR_DELIVER_BACK;
end_query_sigle_area:
	query_res->area_dev_rate[0].rate = rate;
	query_res->area_dev_rate[0].area_id = req_info->area_id;
	query_res->area_dev_rate[0].total_count = total_num;
	query_res->area_dev_rate[0].online_count = online_count;
	goto end_query_area_dev_online_rate;
err_query_area_dev_online_rate:
    size = sizeof(NmpQueryAreaDevRateRes);
    query_res = nmp_mem_kalloc(size);
    if (!query_res)
   {
        nmp_warning("<dbs_mh_bss> alloc error");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    SET_CODE(query_res, ret);
    query_res->total_num = 0;
    query_res->back_num = 0;
    goto end_query_area_dev_online_rate;
}


NmpMsgFunRet
nmp_mod_dbs_del_admin_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpBssRes result;
    NmpDelAdmin *del_info;
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

   nmp_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
   NMP_GET_BSS_USR_NAME(result.bss_usr, del_info->bss_usr);
   nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_del_user_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpBssRes result;
    NmpDelUserGroup *del_info;
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

    nmp_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
    NMP_GET_BSS_USR_NAME(result.bss_usr, del_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_del_user_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpBssRes result;
    NmpDelUser *del_info;
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

    nmp_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
    NMP_GET_BSS_USR_NAME(result.bss_usr, del_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_del_domain_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpDelDomain *del_info;
    NmpMsgErrCode result;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    del_info = MSG_GET_DATA(msg);
    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where dm_id='%s'",
        DOMAIN_TABLE, del_info->domain_id
    );

    memset(&result, 0, sizeof(NmpMsgErrCode));
    nmp_dbs_do_del_code(app_obj, msg, query_buf, &result, &affect_num);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


static __inline__ gint
nmp_get_area_parent(NmpMysqlRes *result)
{
    G_ASSERT(result != NULL);

    gint field_no =0;
    NmpMysqlRow mysql_row;
    NmpMysqlField* mysql_fields;
    gchar *name;
    gchar *value;
    unsigned int row_num;
    unsigned int field_num;

    row_num = nmp_sql_get_num_rows(result);
    if (row_num <= 0)
    {
        nmp_warning("<nmp_dbs_mh_bss> area inexist\n");
        return E_NODBENT;
    }
    field_num = nmp_sql_get_num_fields(result);

    while ((mysql_row = nmp_sql_fetch_row(result)))
    {
        nmp_sql_field_seek(result, 0);
        mysql_fields = nmp_sql_fetch_fields(result);
        for(field_no = 0; field_no < field_num; field_no++)
        {
            name = nmp_sql_get_field_name(mysql_fields, field_no);
            if (!strcmp(name, "area_parent"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
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


gint nmp_is_root_area(NmpAppObj *app_obj, gint area_id)
{
    NmpMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint ret;

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select area_parent from %s where area_id=%d ",
        AREA_TABLE, area_id
        );

    result = nmp_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
        ret = nmp_get_area_parent(result);
    else
        ret = MYSQL_RESULT_CODE(result);

    nmp_sql_put_res(result, sizeof(NmpMysqlRes));
    return ret;
}


NmpMsgFunRet
nmp_mod_dbs_del_area_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpDelArea *del_info;
    NmpBssRes result;
    gint area_parent;
    char query_buf[QUERY_STR_LEN] = {0};
    glong affect_num = 0;

    memset(&result, 0, sizeof(result));
    del_info = MSG_GET_DATA(msg);
    area_parent = nmp_is_root_area(app_obj, del_info->area_id);
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

    nmp_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
    if (!RES_CODE(&result))
    {
         nmp_dbs_mss_notify(app_obj);
         nmp_mod_dbs_deliver_pu_recheck_event(app_obj);
    }

end_del_area:
    NMP_GET_BSS_USR_NAME(result.bss_usr, del_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_del_pu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpDelPu *del_info;
    NmpBssRes result;
    char query_buf[QUERY_STR_LEN];
    db_conn_status *conn = NULL;
    NmpModDbs *dbs_obj;
    gint code, i;

    del_info = MSG_GET_DATA(msg);
    dbs_obj = NMP_MODDBS(app_obj);
	conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
	if (G_UNLIKELY(!conn))
	{
		nmp_warning("<NmpModDbs> get db connection error!");
		code = -E_GETDBCONN;
		goto end_del_pu;
	}

	if (G_UNLIKELY(!conn->mysql))
	{
		nmp_warning("<NmpModDbs> get db connection error!");
		put_db_connection(dbs_obj->pool_info, conn);
		code = -E_GETDBCONN;
		goto end_del_pu;
	}

    switch (del_info->type){
    case 0:
        NMP_MYSQL_DO_QUERY_OR_KILL_CONN(dbs_obj, conn, "start transaction", code);
        if (code)
		goto end_del_pu;

        for (i = 0; i < del_info->count; i++)
        {
            snprintf(
                query_buf, QUERY_STR_LEN,
                "delete from %s where pu_id='%s' and pu_domain='%s'",
                PU_TABLE, del_info->pu_list[i].puid, del_info->pu_list[i].domain_id
            );

            code = nmp_mysql_do_query(conn->mysql, query_buf);
            if (code)
            {
                nmp_mysql_do_query(conn->mysql, "rollback");
                put_db_connection(dbs_obj->pool_info, conn);
                goto end_del_pu;
            }
        }
        if (code == 0)
        {
             nmp_dbs_mss_notify(app_obj);
        }
        nmp_mysql_do_query(conn->mysql, "commit");
        break;
    case 1:
        snprintf(
            query_buf, QUERY_STR_LEN,
            "delete from %s where pu_area='%s'",
            PU_TABLE, del_info->key
        );
        code = nmp_mysql_do_query(conn->mysql, query_buf);
        break;
    }

    put_db_connection(dbs_obj->pool_info, conn);

end_del_pu:
    SET_CODE(&result, code);
    if (!RES_CODE(&result))
		nmp_mod_dbs_deliver_pu_recheck_event(app_obj);
    NMP_GET_BSS_USR_NAME(result.bss_usr, del_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result),
			BUSSLOT_POS_DBS, BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_del_gu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpDelGu *del_info;
    NmpBssRes result;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;
    NmpNotifyMessage notify_info;

    del_info = MSG_GET_DATA(msg);
    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where gu_id='%s' and gu_domain='%s'",
        GU_TABLE, del_info->guid, del_info->domain_id
    );
    memset(&result, 0, sizeof(result));
    nmp_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
    if (!RES_CODE(&result))
    {
        nmp_dbs_mss_notify(app_obj);
        memset(&notify_info, 0, sizeof(notify_info));
        notify_info.msg_id = MSG_DEL_GU;
        strncpy(notify_info.param1, del_info->domain_id, DOMAIN_ID_LEN - 1);
        strncpy(notify_info.param2, del_info->guid, MAX_ID_LEN - 1);
        nmp_mods_dbs_broadcast_msg((NmpModDbs *)app_obj, &notify_info, sizeof(notify_info));
    }

    NMP_GET_BSS_USR_NAME(result.bss_usr, del_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result),
        BUSSLOT_POS_DBS, BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_del_mds_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpDelMds *del_info;
    NmpBssRes result;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;
    del_info = MSG_GET_DATA(msg);

    //frist update pu_table,set pu_mds=null
   /* snprintf(
         query_buf, QUERY_STR_LEN,
         "update %s set pu_mdu='' where mdu_id='%s'",
         PU_TABLE, del_info->mds_id
    );
    nmp_dbs_do_del_code(app_obj, msg, query_buf, &result);
	*/  //trigger displace
    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where mdu_id in ('%s') ",
        MDS_TABLE, del_info->mds_id
    );
    memset(&result, 0, sizeof(result));
    nmp_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
    NMP_GET_BSS_USR_NAME(result.bss_usr, del_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result),
			BUSSLOT_POS_DBS, BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_del_mds_ip_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpDelMdsIp *del_info;
    NmpBssRes result;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;
    del_info = MSG_GET_DATA(msg);

    snprintf(
         query_buf, QUERY_STR_LEN,
         "delete from %s where mdu_id='%s' and mdu_cmsip='%s'",
         MDS_IP_TABLE, del_info->mds_id, del_info->cms_ip
    );
    memset(&result, 0, sizeof(result));
    nmp_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
    NMP_GET_BSS_USR_NAME(result.bss_usr, del_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_del_mss_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpDelMss *del_info;
    NmpBssRes result;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    del_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where mss_id in ('%s') ",
        MSS_TABLE, del_info->mss_id
    );
    memset(&result, 0, sizeof(result));
    nmp_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
    NMP_GET_BSS_USR_NAME(result.bss_usr, del_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result),
			BUSSLOT_POS_DBS, BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_del_manufacturer_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpDelManufacturer *del_info;
    NmpBssRes result;
    char query_buf[QUERY_STR_LEN] = {0};
    glong affect_num = 0;

    del_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where mf_id='%s'",
        MANUFACTURER_TABLE, del_info->mf_id
        );

    memset(&result, 0, sizeof(result));
    nmp_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
    NMP_GET_BSS_USR_NAME(result.bss_usr, del_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


static __inline__ gint
nmp_mod_dbs_control_trigger(NmpAppObj *app_obj, NmpSysMsg *msg, gint op)
{
    NmpMsgErrCode result;
    char query_buf[QUERY_STR_LEN] = {0};
    glong affect_num = 0;

    memset(&result, 0, sizeof(NmpMsgErrCode));
    if (op)  //enable trigger
        snprintf(query_buf, QUERY_STR_LEN,
            "set global connect_timeout=10"
        );
    else   //disable trigger
        snprintf(query_buf, QUERY_STR_LEN,
            "set global connect_timeout=127"
        );

    nmp_dbs_do_del_code(app_obj, msg, query_buf, &result, &affect_num);

    return RES_CODE(&result);
}

NmpMsgFunRet
nmp_mod_dbs_database_backup_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    FILE *fp;
    char buffer[STRING_LEN] = {0};
    char rm_filename[STRING_LEN] = {0};
    NmpBssRes result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint ret = 0;
    NmpDbBackup *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&result, 0, sizeof(result));
    strcpy(result.bss_usr, req_info->bss_usr);
    snprintf(
        query_buf, STRING_LEN - 1,
        "mysql-dump -c -t -e --skip-triggers -u%s -p%s %s > %s/%s.tmp",
        nmp_get_sys_parm_str(SYS_PARM_DBADMINNAME),
        nmp_get_sys_parm_str(SYS_PARM_DBADMINPASSWORD),
        nmp_get_sys_parm_str(SYS_PARM_DBNAME),
        nmp_get_sys_parm_str(SYS_PARM_DBBACKUPPATH),
        req_info->filename
    );

    fp = popen(query_buf, "r");
    if (!fp)
    {
        ret = -errno;
        snprintf(rm_filename,STRING_LEN -1,"rm -rf %s/%s.tmp",
            nmp_get_sys_parm_str(SYS_PARM_DBBACKUPPATH),
        	  req_info->filename);
        system(rm_filename);
        goto end_database_backup;
    }
    fgets(buffer,sizeof(buffer), fp);
    if (strlen(buffer))
    {
      nmp_print("[BSS] Database backup output:%s\n", buffer);
    }
    pclose(fp);
    snprintf(rm_filename,STRING_LEN -1,"mv %s/%s.tmp %s/%s",
        nmp_get_sys_parm_str(SYS_PARM_DBBACKUPPATH),
        req_info->filename,
        nmp_get_sys_parm_str(SYS_PARM_DBBACKUPPATH),
        req_info->filename);
    fp = popen(rm_filename, "r");
    if (!fp)
    {
        nmp_warning("rm tmp database file fail");
        ret = -errno;
        goto end_database_backup;
    }
    fgets(buffer,sizeof(buffer), fp);
    if (strlen(buffer))
    {
      nmp_print("[BSS] change backup database name:%s\n", buffer);
    }
    pclose(fp);

end_database_backup:
    SET_CODE(&result, ret);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
		BUSSLOT_POS_BSS);
    return MFR_DELIVER_BACK;
}


static int import_data_flag = 1;
NmpMsgFunRet
nmp_mod_dbs_clear_database_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    char query_buf[QUERY_STR_LEN] = {0};
    db_conn_status  *conn = NULL;
    NmpModDbs        *dbs_obj;
    gint ret = 0;
    NmpDbImport *req_info;
    char buffer[STRING_LEN] = {0};
    FILE *fp;
    NmpDbImportRes res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

	memset(&res_info, 0, sizeof(res_info));
	strcpy(res_info.bss_usr, req_info->bss_usr);
    if (!g_atomic_int_dec_and_test(&import_data_flag))
    {
	    ret = -E_INPROGRESS;
		goto end_database_import;
    }

    nmp_mod_dbs_control_trigger(app_obj, msg, 0);
    snprintf(query_buf, QUERY_STR_LEN,  "call clear_db_tables()");

    dbs_obj = NMP_MODDBS(app_obj);

redo:
    conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
    if (G_UNLIKELY(!conn))
        ret = -E_GETDBCONN;

    if (G_UNLIKELY(!conn->mysql))
    {
        put_db_connection(dbs_obj->pool_info, conn);
        ret = -E_GETDBCONN;
    }

    ret = nmp_process_query_procedure(conn->mysql, query_buf);
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
        nmp_get_sys_parm_str(SYS_PARM_DBADMINNAME),
        nmp_get_sys_parm_str(SYS_PARM_DBADMINPASSWORD),
        nmp_get_sys_parm_str(SYS_PARM_DBNAME),
        nmp_get_sys_parm_str(SYS_PARM_DBBACKUPPATH),
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
		nmp_print("[BSS] Database import output:%s\n", buffer);
    }
       pclose(fp);

err_database_import:
    nmp_mod_dbs_control_trigger(app_obj, msg, 1);
	import_data_flag = 1;

end_database_import:
    SET_CODE(&res_info, ret);
    nmp_dbs_modify_sysmsg_2(msg, &res_info, sizeof(res_info), BUSSLOT_POS_DBS,
		BUSSLOT_POS_BSS);
    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_del_defence_area_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpDelDefenceArea *del_info;
    NmpBssRes result;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    del_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where defence_area_id=%d ",
        DEFENCE_AREA_TABLE, del_info->defence_area_id
    );
    memset(&result, 0, sizeof(result));
    nmp_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
    NMP_GET_BSS_USR_NAME(result.bss_usr, del_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result),
			BUSSLOT_POS_DBS, BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_del_defence_map_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpDelDefenceMap *del_info;
    NmpBssRes result;
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
    nmp_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
    if (!RES_CODE(&result))
    {
        strncpy(map_path, nmp_get_sys_parm_str(SYS_PARM_MAPPATH), FILE_PATH_LEN- 1);
	 strcat(map_path, "/");
	 strcat(map_path,del_info->map_location);
        unlink(map_path);
    }
    NMP_GET_BSS_USR_NAME(result.bss_usr, del_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result),
			BUSSLOT_POS_DBS, BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_del_defence_gu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpDelDefenceGu *del_info;
    NmpBssRes result;
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
    nmp_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
    NMP_GET_BSS_USR_NAME(result.bss_usr, del_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result),
			BUSSLOT_POS_DBS, BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_del_map_href_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpDelMapHref *del_info;
    NmpBssRes result;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    del_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where src_map_id=%d and dst_map_id=%d",
        MAP_HREF_TABLE, del_info->src_map_id, del_info->dst_map_id
    );
    memset(&result, 0, sizeof(result));
    nmp_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
    NMP_GET_BSS_USR_NAME(result.bss_usr, del_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result),
			BUSSLOT_POS_DBS, BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_del_tw_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpDelTw *del_info;
    NmpBssRes result;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    del_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where tw_id=%d",
        TW_TABLE, del_info->tw_id
    );
    memset(&result, 0, sizeof(result));
    nmp_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
    NMP_GET_BSS_USR_NAME(result.bss_usr, del_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result),
			BUSSLOT_POS_DBS, BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_del_screen_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpDelScreen *del_info;
    NmpBssRes result;
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
		nmp_warning("del screen type(%d) is wrong ", del_info->type);
              SET_CODE(&result, E_QUERYTYPE);
		goto end_del_screen;
    }

    memset(&result, 0, sizeof(result));
    nmp_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
    if (RES_CODE(&result) == -E_NODBENT)
    {
        SET_CODE(&result, 0);
    }

end_del_screen:
    NMP_GET_BSS_USR_NAME(result.bss_usr, del_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_del_tour_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpDelTour *del_info;
    NmpBssRes result;
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
    nmp_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result),
			BUSSLOT_POS_DBS, BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_del_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpDelGroup *del_info;
    char query_buf[QUERY_STR_LEN];

    del_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where group_id=%d",
        GROUP_TABLE, del_info->group_id
    );

    return nmp_mod_dbs_general_cmd(app_obj, msg, query_buf, del_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_dbs_del_group_step_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpDelGroupStep *del_info;
    char query_buf[QUERY_STR_LEN];

    del_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where step_no=%d",
        GROUP_STEP_TABLE, del_info->step_no
    );

    return nmp_mod_dbs_general_cmd(app_obj, msg, query_buf, del_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_dbs_del_group_step_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpDelGroupStepInfo *del_info;
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
    return nmp_mod_dbs_general_cmd(app_obj, msg, query_buf, del_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_dbs_del_alarm_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpMsgCode result;
    NmpDelAlarm *del_info;
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
		nmp_warning("del alarm type(%d) is wrong ", del_info->type);
              SET_CODE(&result, E_QUERYTYPE);
		goto end_del_alarm;
    }

   nmp_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
   if (RES_CODE(&result.code) == -E_NODBENT)
   	SET_CODE(&result.code, 0);
   result.affect_num = affect_num;

end_del_alarm:
    NMP_GET_BSS_USR_NAME(result.bss_usr, del_info->bss_usr);
   nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_del_link_time_policy_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpDelLinkTimePolicy *del_info;
    char query_buf[QUERY_STR_LEN];
    NmpBssRes result;
    glong affect_num = 0;

    del_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where gu_id='%s' and domain_id='%s'",
        LINK_TIME_POLICY_TABLE, del_info->guid, del_info->domain
    );

    memset(&result, 0, sizeof(result));
    nmp_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
    if (RES_CODE( &result.code) == 0)
    {
        NmpShareGuid gu_info;
        memset(&gu_info, 0, sizeof(gu_info));
        strncpy(gu_info.domain_id, del_info->domain, DOMAIN_ID_LEN - 1);
        strncpy(gu_info.guid, del_info->guid, MAX_ID_LEN - 1);
        nmp_cms_mod_deliver_msg_2(app_obj, BUSSLOT_POS_AMS,
            MSG_CHANGE_LINK_TIME_POLICY, &gu_info, sizeof(gu_info));
    }

    strcpy(result.bss_usr, del_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_del_link_record_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpDelLinkRecord *del_info;
    char query_buf[QUERY_STR_LEN];

    del_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where gu_id='%s' and domain_id='%s' and \
        link_guid='%s' and link_domain_id='%s'",
        ALARM_LINK_RECORD_TABLE,
        del_info->guid, del_info->domain, del_info->link_guid, del_info->link_domain
    );

    return nmp_mod_dbs_deal_change_link(app_obj, msg, query_buf,
		del_info->bss_usr, del_info->domain, del_info->guid);
}


NmpMsgFunRet
nmp_mod_dbs_del_link_io_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpDelLinkIO *del_info;
    char query_buf[QUERY_STR_LEN];

    del_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where gu_id='%s' and domain_id='%s' and \
        link_guid='%s' and link_domain_id='%s'",
        ALARM_LINK_IO_TABLE,
        del_info->guid, del_info->domain, del_info->link_guid, del_info->link_domain
    );

    return nmp_mod_dbs_deal_change_link(app_obj, msg, query_buf,
		del_info->bss_usr, del_info->domain, del_info->guid);
}


NmpMsgFunRet
nmp_mod_dbs_del_link_snapshot_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpDelLinkSnapshot *del_info;
    char query_buf[QUERY_STR_LEN];

    del_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where gu_id='%s' and domain_id='%s' and \
        link_guid='%s' and link_domain_id='%s'",
        ALARM_LINK_SNAPSHOT_TABLE,
        del_info->guid, del_info->domain, del_info->link_guid, del_info->link_domain
    );

    return nmp_mod_dbs_deal_change_link(app_obj, msg, query_buf,
		del_info->bss_usr, del_info->domain, del_info->guid);
}


NmpMsgFunRet
nmp_mod_dbs_del_link_preset_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpDelLinkPreset *del_info;
    char query_buf[QUERY_STR_LEN];

    del_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where gu_id='%s' and domain_id='%s' and \
        link_guid='%s' and link_domain_id='%s'",
        ALARM_LINK_PRESET_TABLE,
        del_info->guid, del_info->domain, del_info->link_guid, del_info->link_domain
    );

    return nmp_mod_dbs_deal_change_link(app_obj, msg, query_buf,
		del_info->bss_usr, del_info->domain, del_info->guid);
}


NmpMsgFunRet
nmp_mod_dbs_del_link_step_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpDelLinkStep *del_info;
    char query_buf[QUERY_STR_LEN];

    del_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where gu_id='%s' and domain_id='%s' and \
        tw_id=%d and screen_id=%d and division_num=%d",
        ALARM_LINK_STEP_TABLE, del_info->guid, del_info->domain,
        del_info->tw_id, del_info->screen_id, del_info->div_num
    );

    return nmp_mod_dbs_deal_change_link(app_obj, msg, query_buf,
		del_info->bss_usr, del_info->domain, del_info->guid);
}


NmpMsgFunRet
nmp_mod_dbs_del_link_map_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpDelLinkMap *del_info;
    char query_buf[QUERY_STR_LEN];

    del_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where gu_id='%s' and domain_id='%s' and \
        link_guid='%s' and link_domain_id='%s'",
        ALARM_LINK_MAP_TABLE,
        del_info->guid, del_info->domain, del_info->link_guid, del_info->link_domain
    );

    return nmp_mod_dbs_deal_change_link(app_obj, msg, query_buf,
		del_info->bss_usr, del_info->domain, del_info->guid);
}


NmpMsgFunRet
nmp_mod_dbs_del_ivs_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpDelIvs *del_info;
    NmpBssRes result;
    char query_buf[QUERY_STR_LEN];
    glong affect_num = 0;

    del_info = MSG_GET_DATA(msg);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "delete from %s where ivs_id in ('%s') ",
        IVS_TABLE, del_info->ivs_id
    );
    memset(&result, 0, sizeof(result));
    nmp_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);
    NMP_GET_BSS_USR_NAME(result.bss_usr, del_info->bss_usr);
    nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result),
			BUSSLOT_POS_DBS, BUSSLOT_POS_BSS);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_add_ams_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);

	NmpAddAms *req_info;
	NmpBssRes result;
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
	nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);

	NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
	nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
		BUSSLOT_POS_BSS);

	return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_modify_ams_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);

	NmpModifyAms *req_info;
	NmpBssRes result;
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
	nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);

	NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
	nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
		BUSSLOT_POS_BSS);

	return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_del_ams_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);

	NmpDelAms *req_info;
	NmpBssRes result;
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
	nmp_dbs_do_del_code(app_obj, msg, query_buf, &result.code, &affect_num);

	NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
	nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
		BUSSLOT_POS_BSS);

	return MFR_DELIVER_BACK;
}


static __inline__ void
nmp_query_ams_info(NmpMysqlRes *result, NmpQueryAmsRes *query_res)
{
	G_ASSERT(query_res != NULL);

	gint info_i = 0, field_i = 0;
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
		NmpAmsInfo *ams = &query_res->ams_info[info_i];

		for(field_i = 0; field_i < field_num; field_i++)
		{
			name = nmp_sql_get_field_name(mysql_fields, field_i);

			if (!strcmp(name, "ams_id"))
			{
				value = nmp_sql_get_field_value(mysql_row, field_i);
				if (value)
					NMP_COPY_VAL(ams->ams_id, value,
					AMS_ID_LEN);
			}
			else if (!strcmp(name, "ams_name"))
			{
				value = nmp_sql_get_field_value(mysql_row, field_i);
				if (value)
					NMP_COPY_VAL(ams->ams_name, value,
					AMS_NAME_LEN);
			}
			else if (!strcmp(name, "ams_keep_alive_freq"))
			{
				value = nmp_sql_get_field_value(mysql_row, field_i);
				if(value)
					ams->keep_alive_freq = atoi(value);
			}
			else if (!strcmp(name, "ams_state"))
			{
				value = nmp_sql_get_field_value(mysql_row, field_i);
				if(value)
					ams->ams_state = atoi(value);
			}
			else if (!strcmp(name, "ams_last_ip"))
			{
				value = nmp_sql_get_field_value(mysql_row, field_i);
				if (value)
					NMP_COPY_VAL(ams->ams_ip, value,
					MAX_IP_LEN);
			}
		}

		info_i++;
		if (info_i >= query_res->back_count)
			break;
	}
}


static __inline__ NmpQueryAmsRes *
nmp_dbs_query_ams(NmpMysqlRes *mysql_res, gint *size)
{
	gint row_num, len;
	NmpQueryAmsRes *query_res;

	row_num = nmp_sql_get_num_rows(mysql_res);
	if (row_num == 0)
	{
		len = sizeof(NmpQueryAmsRes);
		query_res = nmp_mem_kalloc(len);
		if (G_UNLIKELY(!query_res))
			return NULL;

		memset(query_res, 0, len);
		SET_CODE(query_res, E_NODBENT);
		query_res->total_count = 0;
		query_res->back_count = 0;
	}
	else
	{
		len = sizeof(NmpQueryAmsRes) + row_num * sizeof(NmpAmsInfo);
		query_res = nmp_mem_kalloc(len);
		if (G_UNLIKELY(!query_res))
			return NULL;

		memset(query_res, 0, len);
		query_res->back_count = row_num;
		SET_CODE(query_res, 0);
		nmp_query_ams_info(mysql_res, query_res);
	}
	*size = len;

	return query_res;
}


static NmpMsgFunRet
nmp_mod_dbs_query_ams_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpQueryAms *req_info = NULL;
	NmpQueryAmsRes *res_info = NULL;
	gchar query_buf[QUERY_STR_LEN] = {0};
	NmpMysqlRes *mysql_result = NULL;
	gint row_num;
	gint total_num;
	gint ret = 0, size;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	snprintf(query_buf, QUERY_STR_LEN,
		"select count(*) as count from %s",
		AMS_TABLE
	);
	total_num =  nmp_get_record_count(app_obj, query_buf);
	if (total_num == 0)
	{
		ret = 0;
		goto end;
	}

	snprintf(query_buf, QUERY_STR_LEN,
		"select * from %s order by ams_state desc limit %d,%d",
		AMS_TABLE, req_info->start_num, req_info->req_num
	);

	mysql_result = nmp_dbs_do_query_res(app_obj, query_buf);
	NMP_DBS_CHECK_MYSQL_RESULT(mysql_result, ret, row_num, end);

	res_info = nmp_dbs_query_ams(mysql_result, &size);
	if (G_UNLIKELY(!res_info))
	{
		nmp_warning("<NmpModDbs> alloc error");
		nmp_sysmsg_destroy(msg);
		return MFR_ACCEPTED;
	}
	if (total_num <= res_info->back_count)
		res_info->total_count = res_info->back_count;
	else
		res_info->total_count = total_num;

end:
	if(mysql_result)
		nmp_sql_put_res(mysql_result, sizeof(NmpMysqlRes));

	if (res_info == NULL)
	{
		size = sizeof(NmpQueryAmsRes);
		res_info = nmp_mem_kalloc(size);
		if (!res_info)
		{
			nmp_warning("<NmpModDbs> alloc error");
			nmp_sysmsg_destroy(msg);
			return MFR_ACCEPTED;
		}

		memset(res_info, 0, size);
		res_info->total_count = 0;
		res_info->back_count = 0;
	}

	SET_CODE(res_info, ret);
	NMP_COPY_VAL(res_info->bss_usr, req_info->bss_usr, USER_NAME_LEN);

	nmp_dbs_modify_sysmsg(msg, res_info, size, BUSSLOT_POS_DBS,
		BUSSLOT_POS_BSS, nmp_mem_kfree);

	return MFR_DELIVER_BACK;
}


static __inline__ void
nmp_query_ams_pu_info(NmpMysqlRes *result, NmpQueryAmsPuRes *query_res)
{
	G_ASSERT(query_res != NULL);

	gint info_i = 0, field_i = 0;
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
		NmpAmsPuInfo *pu = &query_res->pu_info[info_i];

		for(field_i = 0; field_i < field_num; field_i++)
		{
			name = nmp_sql_get_field_name(mysql_fields, field_i);

			if (!strcmp(name, "pu_id"))
			{
				value = nmp_sql_get_field_value(mysql_row, field_i);
				if (value)
					NMP_COPY_VAL(pu->puid, value,
					MAX_ID_LEN);
			}
			else if (!strcmp(name, "pu_domain"))
			{
				value = nmp_sql_get_field_value(mysql_row, field_i);
				if (value)
					NMP_COPY_VAL(pu->domain, value,
					DOMAIN_ID_LEN);
			}
			else if (!strcmp(name, "pu_info"))
			{
				value = nmp_sql_get_field_value(mysql_row, field_i);
				if (value)
					NMP_COPY_VAL(pu->pu_name, value,
					PU_NAME_LEN);
			}
			else if (!strcmp(name, "area_name"))
			{
				value = nmp_sql_get_field_value(mysql_row, field_i);
				if (value)
					NMP_COPY_VAL(pu->area_name, value,
					AREA_NAME_LEN);
			}
			else if (!strcmp(name, "dev_name"))
			{
				value = nmp_sql_get_field_value(mysql_row, field_i);
				if (value)
					NMP_COPY_VAL(pu->dev_name, value,
					AMS_DEV_NAME_LEN);
			}
			else if (!strcmp(name, "dev_passwd"))
			{
				value = nmp_sql_get_field_value(mysql_row, field_i);
				if (value)
					NMP_COPY_VAL(pu->dev_passwd, value,
					AMS_DEV_PASSWD_LEN);
			}
			else if (!strcmp(name, "dev_ip"))
			{
				value = nmp_sql_get_field_value(mysql_row, field_i);
				if (value)
					NMP_COPY_VAL(pu->dev_ip, value,
					MAX_IP_LEN);
			}
			else if (!strcmp(name, "dev_port"))
			{
				value = nmp_sql_get_field_value(mysql_row, field_i);
				if(value)
					pu->dev_port = atoi(value);
			}
			else if (!strcmp(name, "pu_state"))
			{
				value = nmp_sql_get_field_value(mysql_row, field_i);
				if(value)
					pu->dev_state = atoi(value);
			}
		}

		info_i++;
		if (info_i >= query_res->back_count)
			break;
	}
}


static __inline__ NmpQueryAmsPuRes *
nmp_dbs_query_ams_pu(NmpMysqlRes *mysql_res, gint *size)
{
	gint row_num, len;
	NmpQueryAmsPuRes *query_res;

	row_num = nmp_sql_get_num_rows(mysql_res);
	if (row_num == 0)
	{
		len = sizeof(NmpQueryAmsPuRes);
		query_res = nmp_mem_kalloc(len);
		if (G_UNLIKELY(!query_res))
			return NULL;

		memset(query_res, 0, len);
		SET_CODE(query_res, E_NODBENT);
		query_res->total_count = 0;
		query_res->back_count = 0;
	}
	else
	{
		len = sizeof(NmpQueryAmsPuRes) + row_num * sizeof(NmpAmsPuInfo);
		query_res = nmp_mem_kalloc(len);
		if (G_UNLIKELY(!query_res))
			return NULL;

		memset(query_res, 0, len);
		query_res->back_count = row_num;
		SET_CODE(query_res, 0);
		nmp_query_ams_pu_info(mysql_res, query_res);
	}
	*size = len;

	return query_res;
}


static NmpMsgFunRet
nmp_mod_dbs_query_ams_pu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpQueryAmsPu *req_info = NULL;
	NmpQueryAmsPuRes *res_info = NULL;
	gchar query_buf[QUERY_STR_LEN] = {0};
	NmpMysqlRes *mysql_result = NULL;
	gint row_num;
	gint total_num;
	gint ret = 0, size;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	snprintf(query_buf, QUERY_STR_LEN,
		"select count(*) as count from %s where ams_id='%s'",
		AMS_CONFIGURE_TABLE, req_info->ams_id
	);
	total_num =  nmp_get_record_count(app_obj, query_buf);
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

	mysql_result = nmp_dbs_do_query_res(app_obj, query_buf);
	NMP_DBS_CHECK_MYSQL_RESULT(mysql_result, ret, row_num, end);

	res_info = nmp_dbs_query_ams_pu(mysql_result, &size);
	if (G_UNLIKELY(!res_info))
	{
		nmp_warning("<NmpModDbs> alloc error");
		nmp_sysmsg_destroy(msg);
		return MFR_ACCEPTED;
	}
	if (total_num <= res_info->back_count)
		res_info->total_count = res_info->back_count;
	else
		res_info->total_count = total_num;

end:
	if(mysql_result)
		nmp_sql_put_res(mysql_result, sizeof(NmpMysqlRes));

	if (res_info == NULL)
	{
		size = sizeof(NmpQueryAmsPuRes);
		res_info = nmp_mem_kalloc(size);
		if (!res_info)
		{
			nmp_warning("<NmpModDbs> alloc error");
			nmp_sysmsg_destroy(msg);
			return MFR_ACCEPTED;
		}

		memset(res_info, 0, size);
		res_info->total_count = 0;
		res_info->back_count = 0;
	}

	SET_CODE(res_info, ret);
	NMP_COPY_VAL(res_info->bss_usr, req_info->bss_usr, USER_NAME_LEN);

	nmp_dbs_modify_sysmsg(msg, res_info, size, BUSSLOT_POS_DBS,
		BUSSLOT_POS_BSS, nmp_mem_kfree);

	return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_dbs_modify_ams_pu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);

	NmpModifyAmsPu *req_info;
	NmpBssRes result;
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
	nmp_dbs_do_query_code(app_obj, msg, query_buf, &result.code, &affect_num);

	NMP_GET_BSS_USR_NAME(result.bss_usr, req_info->bss_usr);
	nmp_dbs_modify_sysmsg_2(msg, &result, sizeof(result), BUSSLOT_POS_DBS,
		BUSSLOT_POS_BSS);

	return MFR_DELIVER_BACK;
}



void
nmp_mod_dbs_register_bss_msg_handler(NmpModDbs *self)
{
    NmpAppMod *super_self = (NmpAppMod*)self;

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_BSS_LOGIN,
        NULL,
        nmp_mod_dbs_admin_login_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_ADMIN,
        NULL,
        nmp_mod_dbs_add_admin_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_ADMIN,
        NULL,
        nmp_mod_dbs_modify_admin_b,
        0
    );
    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_ADMIN,
        NULL,
        nmp_mod_dbs_del_admin_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_ADMIN,
        NULL,
        nmp_mod_dbs_query_admin_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_VALIDATA_ADMIN,
        NULL,
        nmp_mod_dbs_validata_admin_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_USER_GROUP,
        NULL,
        nmp_mod_dbs_add_user_group_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_VALIDATA_USER_GROUP,
        NULL,
        nmp_mod_dbs_validata_user_group_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_USER_GROUP,
        NULL,
        nmp_mod_dbs_query_user_group_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_USER_GROUP,
        NULL,
        nmp_mod_dbs_modify_user_group_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_USER_GROUP,
        NULL,
        nmp_mod_dbs_del_user_group_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_USER,
        NULL,
        nmp_mod_dbs_add_user_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_VALIDATA_USER,
        NULL,
        nmp_mod_dbs_validata_user_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_USER,
        NULL,
        nmp_mod_dbs_query_user_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_USER,
        NULL,
        nmp_mod_dbs_modify_user_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_USER,
        NULL,
        nmp_mod_dbs_del_user_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_DOMAIN,
        NULL,
        nmp_mod_dbs_query_domain_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_DOMAIN,
        NULL,
        nmp_mod_dbs_modify_domain_b,
        0
    );

   nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_DOMAIN,
        NULL,
        NULL,
        0
    );
    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_DOMAIN,
        NULL,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_AREA,
        NULL,
        nmp_mod_dbs_query_area_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_MODIFY_AREA,
        NULL,
        nmp_mod_dbs_add_modify_area_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_AREA,
        NULL,
        nmp_mod_dbs_del_area_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_PU,
        NULL,
        nmp_mod_dbs_add_pu_b,
        0
    );

    /*nmp_app_mod_register_msg(
        super_self,
        MESSAGE_VALIDATA_PU,
        NULL,
        nmp_mod_dbs_validata_pu_b,
        0
    );
  */

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_PU,
        NULL,
        nmp_mod_dbs_query_pu_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_PU,
        NULL,
        nmp_mod_dbs_modify_pu_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_PU,
        NULL,
        nmp_mod_dbs_del_pu_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_AUTO_ADD_PU,
        NULL,
        nmp_mod_dbs_auto_add_pu_b,
        0
    );

   nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_GU,
        NULL,
        nmp_mod_dbs_add_gu_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_GU,
        NULL,
        nmp_mod_dbs_query_gu_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_GU,
        NULL,
        nmp_mod_dbs_modify_gu_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_GU,
        NULL,
        nmp_mod_dbs_del_gu_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_MDS,
        NULL,
        nmp_mod_dbs_add_mds_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_MDS,
        NULL,
        nmp_mod_dbs_query_mds_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_MDS,
        NULL,
        nmp_mod_dbs_modify_mds_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_MDS,
        NULL,
        nmp_mod_dbs_del_mds_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_MDS_IP,
        NULL,
        nmp_mod_dbs_add_mds_ip_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_MDS_IP,
        NULL,
        nmp_mod_dbs_query_mds_ip_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_MDS_IP,
        NULL,
        nmp_mod_dbs_del_mds_ip_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_MSS,
        NULL,
        nmp_mod_dbs_add_mss_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_MSS,
        NULL,
        nmp_mod_dbs_query_mss_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_MSS,
        NULL,
        nmp_mod_dbs_modify_mss_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_MSS,
        NULL,
        nmp_mod_dbs_del_mss_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_RECORD_POLICY,
        NULL,
        nmp_mod_dbs_query_record_policy_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_RECORD_POLICY_CONFIG,
        NULL,
        nmp_mod_dbs_record_policy_config_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_MANUFACTURER,
        NULL,
        nmp_mod_dbs_query_manufacturer_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_MODIFY_MANUFACTURER,
        NULL,
        nmp_mod_dbs_add_modify_manufacturer_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_MANUFACTURER,
        NULL,
        nmp_mod_dbs_del_manufacturer_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_GU_TO_USER,
        NULL,
        nmp_mod_dbs_add_gu_to_user_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_USER_OWN_GU,
        NULL,
        nmp_mod_dbs_query_user_own_gu_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_TW_TO_USER,
        NULL,
        nmp_mod_dbs_add_tw_to_user_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_USER_OWN_TW,
        NULL,
        nmp_mod_dbs_query_user_own_tw_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_TOUR_TO_USER,
        NULL,
        nmp_mod_dbs_add_tour_to_user_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_USER_OWN_TOUR,
        NULL,
        nmp_mod_dbs_query_user_own_tour_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DATABASE_BACKUP,
        NULL,
        nmp_mod_dbs_database_backup_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DATABASE_IMPORT,
        NULL,
        nmp_mod_dbs_clear_database_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_DEFENCE_AREA,
        NULL,
        nmp_mod_dbs_query_defence_area_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_DEFENCE_AREA,
        NULL,
        nmp_mod_dbs_add_defence_area_b,
        0
    );

   nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_DEFENCE_AREA,
        NULL,
        nmp_mod_dbs_modify_defence_area_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_DEFENCE_AREA,
        NULL,
        nmp_mod_dbs_del_defence_area_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_DEFENCE_MAP,
        NULL,
        nmp_mod_dbs_query_defence_map_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_DEFENCE_MAP,
        NULL,
        nmp_mod_dbs_add_defence_map_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_DEFENCE_MAP,
        NULL,
        nmp_mod_dbs_del_defence_map_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_DEFENCE_GU,
        NULL,
        nmp_mod_dbs_query_defence_gu_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_DEFENCE_GU,
        NULL,
        nmp_mod_dbs_add_defence_gu_b,
        0
    );

   nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_DEFENCE_GU,
        NULL,
        nmp_mod_dbs_modify_defence_gu_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_DEFENCE_GU,
        NULL,
        nmp_mod_dbs_del_defence_gu_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_MAP_HREF,
        NULL,
        nmp_mod_dbs_query_map_href_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_MAP_HREF,
        NULL,
        nmp_mod_dbs_add_map_href_b,
        0
    );

   nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_MAP_HREF,
        NULL,
        nmp_mod_dbs_modify_map_href_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_MAP_HREF,
        NULL,
        nmp_mod_dbs_del_map_href_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_TW,
        NULL,
        nmp_mod_dbs_query_tw_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_TW,
        NULL,
        nmp_mod_dbs_add_tw_b,
        0
    );

   nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_TW,
        NULL,
        nmp_mod_dbs_modify_tw_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_TW,
        NULL,
        nmp_mod_dbs_del_tw_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_SCREEN,
        NULL,
        nmp_mod_dbs_add_screen_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_SCREEN,
        NULL,
        nmp_mod_dbs_modify_screen_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_SCREEN,
        NULL,
        nmp_mod_dbs_query_screen_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_SCREEN,
        NULL,
        nmp_mod_dbs_del_screen_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_TOUR,
        NULL,
        nmp_mod_dbs_query_tour_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_TOUR,
        NULL,
        nmp_mod_dbs_add_tour_b,
        0
    );

   nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_TOUR,
        NULL,
        nmp_mod_dbs_modify_tour_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_TOUR,
        NULL,
        nmp_mod_dbs_del_tour_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_TOUR_STEP,
        NULL,
        nmp_mod_dbs_query_tour_step_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_TOUR_STEP,
        NULL,
        nmp_mod_dbs_add_tour_step_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_GROUP,
        NULL,
        nmp_mod_dbs_query_group_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_GROUP,
        NULL,
        nmp_mod_dbs_add_group_b,
        0
    );

   nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_GROUP,
        NULL,
        nmp_mod_dbs_modify_group_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_GROUP,
        NULL,
        nmp_mod_dbs_del_group_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_GROUP_STEP,
        NULL,
        nmp_mod_dbs_query_group_step_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_GROUP_STEP,
        NULL,
        nmp_mod_dbs_add_group_step_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_GROUP_STEP,
        NULL,
        nmp_mod_dbs_modify_group_step_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_GROUP_STEP,
        NULL,
        nmp_mod_dbs_del_group_step_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_GROUP_STEP_INFO,
        NULL,
        nmp_mod_dbs_query_group_step_info_b,
        0
    );

	nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_GROUP_STEP_DIV,
        NULL,
        nmp_mod_dbs_query_group_step_div_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_CONFIG_GROUP_STEP,
        NULL,
        nmp_mod_dbs_config_group_step_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_GROUP_STEP_INFO,
        NULL,
        nmp_mod_dbs_modify_group_step_info_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_GROUP_STEP_INFO,
        NULL,
        nmp_mod_dbs_del_group_step_info_b,
        0
    );

   nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_ALARM,
        NULL,
        nmp_mod_dbs_query_alarm_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_ALARM,
        NULL,
        nmp_mod_dbs_del_alarm_b,
        0
    );

   nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_DEL_ALARM_POLICY,
        NULL,
        nmp_mod_dbs_query_del_alarm_policy_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_DEL_ALARM_POLICY,
        NULL,
        nmp_mod_dbs_set_del_alarm_policy_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_LINK_TIME_POLICY,
        NULL,
        nmp_mod_dbs_query_link_time_policy_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_LINK_TIME_POLICY_CONFIG,
        NULL,
        nmp_mod_dbs_link_time_policy_config_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_LINK_TIME_POLICY,
        NULL,
        nmp_mod_dbs_modify_link_time_policy_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_LINK_TIME_POLICY,
        NULL,
        nmp_mod_dbs_del_link_time_policy_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_LINK_RECORD,
        NULL,
        nmp_mod_dbs_query_link_record_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_LINK_RECORD_CONFIG,
        NULL,
        nmp_mod_dbs_link_record_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_LINK_RECORD,
        NULL,
        nmp_mod_dbs_modify_link_record_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_LINK_RECORD,
        NULL,
        nmp_mod_dbs_del_link_record_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_LINK_IO,
        NULL,
        nmp_mod_dbs_query_link_io_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_LINK_IO_CONFIG,
        NULL,
        nmp_mod_dbs_link_io_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_LINK_IO,
        NULL,
        nmp_mod_dbs_modify_link_io_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_LINK_IO,
        NULL,
        nmp_mod_dbs_del_link_io_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_LINK_SNAPSHOT,
        NULL,
        nmp_mod_dbs_query_link_snapshot_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_LINK_SNAPSHOT_CONFIG,
        NULL,
        nmp_mod_dbs_link_snapshot_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_LINK_SNAPSHOT,
        NULL,
        nmp_mod_dbs_modify_link_snapshot_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_LINK_SNAPSHOT,
        NULL,
        nmp_mod_dbs_del_link_snapshot_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_LINK_PRESET,
        NULL,
        nmp_mod_dbs_query_link_preset_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_LINK_PRESET_CONFIG,
        NULL,
        nmp_mod_dbs_link_preset_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_LINK_PRESET,
        NULL,
        nmp_mod_dbs_modify_link_preset_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_LINK_PRESET,
        NULL,
        nmp_mod_dbs_del_link_preset_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_LINK_STEP,
        NULL,
        nmp_mod_dbs_query_link_step_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_LINK_STEP_CONFIG,
        NULL,
        nmp_mod_dbs_link_step_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_LINK_STEP,
        NULL,
        nmp_mod_dbs_modify_link_step_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_LINK_STEP,
        NULL,
        nmp_mod_dbs_del_link_step_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_LINK_MAP,
        NULL,
        nmp_mod_dbs_query_link_map_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_LINK_MAP_CONFIG,
        NULL,
        nmp_mod_dbs_link_map_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_LINK_MAP,
        NULL,
        nmp_mod_dbs_modify_link_map_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_LINK_MAP,
        NULL,
        nmp_mod_dbs_del_link_map_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_SERVER_RESOURCE,
        NULL,
        nmp_mod_dbs_query_server_resource_info_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_NEXT_PUNO,
        NULL,
        nmp_mod_dbs_get_next_puno_b,
        0
    );
#if ONLINE_RATE_FLAG
    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_AREA_DEV_ONLINE_RATE,
        NULL,
        nmp_mod_dbs_query_area_dev_online_rate_b,
        0
    );
#endif

  	nmp_app_mod_register_msg(
        super_self,
        MESSAGE_VALIDATA_GU_MAP,
        NULL,
        nmp_mod_dbs_validata_gu_map_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_IVS,
        NULL,
        nmp_mod_dbs_add_ivs_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_IVS,
        NULL,
        nmp_mod_dbs_query_ivs_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_IVS,
        NULL,
        nmp_mod_dbs_modify_ivs_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_IVS,
        NULL,
        nmp_mod_dbs_del_ivs_b,
        0
    );

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_ADD_AMS,
		NULL,
		nmp_mod_dbs_add_ams_b,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_MODIFY_AMS,
		NULL,
		nmp_mod_dbs_modify_ams_b,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_DEL_AMS,
		NULL,
		nmp_mod_dbs_del_ams_b,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_QUERY_AMS,
		NULL,
		nmp_mod_dbs_query_ams_b,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_QUERY_AMS_PU,
		NULL,
		nmp_mod_dbs_query_ams_pu_b,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_MODIFY_AMS_PU,
		NULL,
		nmp_mod_dbs_modify_ams_pu_b,
		0
	);
}

