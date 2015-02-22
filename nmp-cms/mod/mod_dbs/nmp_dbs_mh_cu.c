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


static __inline__ gint
jpf_get_user_info(JpfMysqlRes *mysql_result, JpfMsgUserInfoRes *res_info)
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

                if (!strcmp(name,"user_name"))
                {
                    value = jpf_sql_get_field_value(mysql_row, j);
                    res_info->name[USER_NAME_LEN - 1] = 0;
                    strncpy(res_info->name, value, USER_NAME_LEN - 1);
                }
                else if (!strcmp(name,"user_group"))
                {
                    value = jpf_sql_get_field_value(mysql_row, j);
                    res_info->group_id = atoi(value);
                }
                else if (!strcmp(name,"user_password"))
                {
                    value = jpf_sql_get_field_value(mysql_row, j);
                    res_info->passwd[USER_PASSWD_LEN - 1] = 0;
                    strncpy(res_info->passwd, value, USER_PASSWD_LEN - 1);
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
jpf_dbs_get_user_info_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfMsgUserInfo *req_info;
    JpfMsgUserInfoRes res_info;
    JpfMysqlRes *mysql_res;
    char query_buf[QUERY_STR_LEN];
    int code = 0;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    snprintf(
        query_buf, QUERY_STR_LEN,
        "select user_name,user_password,user_group from %s where user_name='%s'",
        USER_TABLE, req_info->name
    );

    mysql_res = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!mysql_res);
    if (G_LIKELY(!MYSQL_RESULT_CODE(mysql_res)))  //success:0 fail:!0
        code = jpf_get_user_info(mysql_res, &res_info);
    else
        code = MYSQL_RESULT_CODE(mysql_res);

    jpf_sql_put_res(mysql_res, sizeof(JpfMysqlRes));
    SET_CODE(&res_info, code);
    jpf_dbs_modify_sysmsg_2(
        msg, &res_info, sizeof(res_info),
        BUSSLOT_POS_DBS, BUSSLOT_POS_CU
    );

    return MFR_DELIVER_BACK;
}


static __inline__ gint
jpf_get_user_group_info(JpfMysqlRes *mysql_result,
                        JpfMsgUserGroupInfoRes *res_info)
{
    unsigned long field_num;
    JpfMysqlField *mysql_fields;    //modify: mysql_fields -> *msql_fields
    JpfMysqlRow   mysql_row;
    char *value, *name;
    int j,row_num;

    row_num = jpf_sql_get_num_rows(mysql_result);
    if (row_num == 0)
    {
        jpf_warning("<GetUserGroupInfo>No such record entry in database");
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
                if (!strcmp(name, "group_id"))
                {
                    value = jpf_sql_get_field_value(mysql_row, j);
                    res_info->group_id = atoi(value);
                }
                else if (!strcmp(name, "group_permissions"))
                {
                    value = jpf_sql_get_field_value(mysql_row, j);
                    res_info->permission = atoi(value);
                    jpf_warning("-----res_info->permission=%d",res_info->permission);
                }
                else if (!strcmp(name, "group_rank"))
                {
                    value = jpf_sql_get_field_value(mysql_row, j);
                    res_info->rank = atoi(value);
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


JpfMsgFunRet
jpf_dbs_get_user_group_info_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfMsgUserGroupInfo *req_info;
    JpfMsgUserGroupInfoRes res_info;
    JpfMysqlRes *mysql_res;
    char query_buf[QUERY_STR_LEN];
    int code;

    memset(&res_info, 0, sizeof(res_info));
    req_info = MSG_GET_DATA(msg);
    snprintf(query_buf, QUERY_STR_LEN,
        "select group_id,group_permissions,group_rank from %s where group_id='%d'",
        USER_GROUP_TABLE, req_info->group_id
    );

    mysql_res = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!mysql_res);

    if (G_UNLIKELY(!MYSQL_RESULT_CODE(mysql_res)))  //success:0 fail:!0
        code = jpf_get_user_group_info(mysql_res, &res_info);
    else
        code = MYSQL_RESULT_CODE(mysql_res);

    jpf_sql_put_res(mysql_res, sizeof(JpfMysqlRes));
    SET_CODE(&res_info, code);

    jpf_dbs_modify_sysmsg_2(msg, &res_info, sizeof(res_info),
                  BUSSLOT_POS_DBS, BUSSLOT_POS_CU);

    return MFR_DELIVER_BACK;
}


static __inline__ gint
jpf_get_user_login_info(JpfMysqlRes *mysql_result,
                        JpfMsgUserLoginInfoRes *res_info)
{
    unsigned long field_num;
    JpfMysqlField *mysql_fields;    //modify: mysql_fields -> *msql_fields
    JpfMysqlRow   mysql_row;
    char *value, *name;
    int j,row_num;

    row_num = jpf_sql_get_num_rows(mysql_result);

    if (row_num == 0)
    {
        jpf_warning("<GetUserGroupInfo>No such record entry in database");
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
                if (!strcmp(name, "area_id"))
                {
                    value = jpf_sql_get_field_value(mysql_row, j);
                    res_info->root_area_id = atoi(value);
                }
                else if (!strcmp(name, "area_name"))
                {
                    value = jpf_sql_get_field_value(mysql_row, j);
                    res_info->root_area_name[AREA_NAME_LEN - 1] = 0;
                    strncpy(res_info->root_area_name, value, AREA_NAME_LEN - 1);
                }
                else if (!strcmp(name, "dm_id"))
                {
                    value = jpf_sql_get_field_value(mysql_row, j);
                    res_info->domain_id[DOMAIN_ID_LEN - 1] = 0;
                    strncpy(res_info->domain_id, value, DOMAIN_ID_LEN - 1);
                }
                else if (!strcmp(name, "dm_name"))
                {
                    value = jpf_sql_get_field_value(mysql_row, j);
                    res_info->domain_name[DOMAIN_NAME_LEN - 1] = 0;
                    strncpy(res_info->domain_name, value, DOMAIN_NAME_LEN - 1);
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


JpfMsgFunRet
jpf_dbs_get_user_login_info_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfMsgUserLoginInfoRes res_info;
    JpfMysqlRes *mysql_res;
    char query_buf[QUERY_STR_LEN] = {0};
    int code;
    JpfModDbs        *dbs_obj;

    dbs_obj = JPF_MODDBS(app_obj);

    if (dbs_obj->wdd_status)
    {
        code = -E_NOWDD;
        goto end_get_user_login_info;
    }

    if (dbs_obj->authorization_expired)
    {
        code = -E_EXPIRED;
        goto end_get_user_login_info;
    }

    if (dbs_obj->time_status)
    {
        code = -E_TIMER;
        goto end_get_user_login_info;
    }

    snprintf(query_buf, QUERY_STR_LEN,
        "select t1.dm_id,t1.dm_name,t2.area_id,t2.area_name from %s as t1,%s as t2 \
        where t1.dm_type=0 and t2.area_parent is NULL",
        DOMAIN_TABLE, AREA_TABLE
    );

    mysql_res = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!mysql_res);

    if (G_LIKELY(!MYSQL_RESULT_CODE(mysql_res)))  //success:0 fail:!0
        code = jpf_get_user_login_info(mysql_res, &res_info);
    else
        code = MYSQL_RESULT_CODE(mysql_res);

    jpf_sql_put_res(mysql_res, sizeof(JpfMysqlRes));
 end_get_user_login_info:
    SET_CODE(&res_info.code, code);
    jpf_dbs_modify_sysmsg_2(msg, &res_info, sizeof(res_info),
                  BUSSLOT_POS_DBS, BUSSLOT_POS_CU);

    return MFR_DELIVER_BACK;
}


static __inline__ void
jpf_get_all_area_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfGetAreaRes *query_res
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
            else
                cms_debug("no need mysql name %s \n", name);
        }

        info_no++;
    }

}


static __inline__ JpfGetAreaRes *
jpf_dbs_get_all_area(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfGetAreaRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfGetAreaRes);
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
        len = sizeof(JpfGetAreaRes) + row_num*sizeof(JpfArea);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
	 query_res->total_num = row_num;
        query_res->req_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);
        jpf_get_all_area_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ gint
jpf_get_area_info(JpfMysqlRes *mysql_result, JpfGetAreaInfoRes *res_info)
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

                if (!strcmp(name, "area_name"))
                {
                    value = jpf_sql_get_field_value(mysql_row, j);
                    if (value)
                    {
                        res_info->area_name[USER_NAME_LEN - 1] = 0;
                        strncpy(res_info->area_name, value, USER_NAME_LEN - 1);
                    }
                }
                else if (!strcmp(name, "user_name"))
                {
                    value = jpf_sql_get_field_value(mysql_row, j);
                    if (value)
                    {
                        res_info->user_name[USER_NAME_LEN - 1] = 0;
                        strncpy(res_info->user_name, value, USER_NAME_LEN - 1);
                    }
                }
                else if (!strcmp(name,"user_phone"))
                {
                    value = jpf_sql_get_field_value(mysql_row, j);
                    if (value)
                    {
                        res_info->user_phone[PHONE_NUM_LEN - 1] = 0;
                        strncpy(res_info->user_phone, value, PHONE_NUM_LEN - 1);
                    }
                }
                else if (!strcmp(name,"user_address"))
                {
                    value = jpf_sql_get_field_value(mysql_row, j);
                    if (value)
                    {
                        res_info->user_address[DESCRIPTION_INFO_LEN - 1] = 0;
                        strncpy(res_info->user_address, value, DESCRIPTION_INFO_LEN - 1);
                    }
                }
                else if (!strcmp(name,"description"))
                {
                    value = jpf_sql_get_field_value(mysql_row, j);
                    if (value)
                    {
                        res_info->description[DESCRIPTION_INFO_LEN - 1] = 0;
                        strncpy(res_info->description, value, DESCRIPTION_INFO_LEN - 1);
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


static __inline__ void
jpf_cu_get_defence_area_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfGetDefenceAreaRes *query_res
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
            else if (!strcmp(name, "defence_enable"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->area_info[info_no].is_defence_area = 1;
                    query_res->area_info[info_no].defence_enable = atoi(value);
                }
                else
                {
                    query_res->area_info[info_no].is_defence_area = 0;
                    query_res->area_info[info_no].defence_enable = 0;
                }
            }
            else if (!strcmp(name, "policy"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->area_info[info_no].policy[POLICY_LEN - 1] = 0;
                    strncpy(
                        query_res->area_info[info_no].policy,
                        value, POLICY_LEN - 1
                    );
                }
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }

        info_no++;
    }

}


static __inline__ JpfGetDefenceAreaRes *
jpf_dbs_cu_get_defence_area(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfGetDefenceAreaRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfGetDefenceAreaRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
	 query_res->back_num = 0;
        jpf_warning("login user is not exist or password not right\n");
    }
    else
    {
        len = sizeof(JpfGetDefenceAreaRes) + row_num*sizeof(JpfDefenceArea);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
	 query_res->total_num = row_num;
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);
        jpf_cu_get_defence_area_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_cu_get_defence_map_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfGetDefenceMapRes *query_res
                  )
{
    G_ASSERT(query_res != NULL);

    gint info_no = 0, field_no =0;
    JpfMysqlRow mysql_row;
    JpfMysqlField* mysql_fields;
    gchar *name;
    gchar *value;
    gchar map_path[FILE_PATH_LEN] = {0};
    gchar map_location[MAP_LOCATION_LEN] = {0};
    unsigned int row_num;
    unsigned int field_num;

    strncpy(map_path, jpf_get_sys_parm_str(SYS_PARM_MAPPATH), FILE_PATH_LEN- 1);
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
                    query_res->map_info[info_no].map_name[MAP_NAME_LEN- 1] = 0;
                    strncpy(
                        query_res->map_info[info_no].map_name,
                        value, MAP_NAME_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "map_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->map_info[info_no].map_id = atoi(value);
            }
            else if (!strcmp(name, "map_location"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    map_location[MAP_LOCATION_LEN - 1] = 0;
                    strncpy(map_location, value, MAP_LOCATION_LEN - 1);
                    query_res->map_info[info_no].map_location[MAP_LOCATION_LEN - 1] = 0;
                    snprintf(
                        query_res->map_info[info_no].map_location,
                         MAP_LOCATION_LEN, "%s", map_location
                    );
                }
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }

        info_no++;
    }

}


static __inline__ JpfGetDefenceMapRes *
jpf_dbs_cu_get_defence_map(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfGetDefenceMapRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfGetDefenceMapRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
	 query_res->back_num = 0;
        jpf_warning("login user is not exist or password not right\n");
    }
    else
    {
        len = sizeof(JpfGetDefenceMapRes) + row_num*sizeof(JpfDefenceMap);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
	 query_res->total_num = row_num;
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);
        jpf_cu_get_defence_map_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_cu_get_defence_gu_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfGetDefenceGuRes *query_res
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
            if (!strcmp(name, "gu_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->defence_gu[info_no].guid[MAX_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->defence_gu[info_no].guid,
                        value, MAX_ID_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "gu_domain"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->defence_gu[info_no].domain_id[DOMAIN_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->defence_gu[info_no].domain_id,
                        value, DOMAIN_ID_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "gu_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->defence_gu[info_no].gu_name[GU_NAME_LEN - 1] = 0;
                    strncpy(
                        query_res->defence_gu[info_no].gu_name,
                        value, GU_NAME_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "gu_type"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->defence_gu[info_no].gu_type= atoi(value);
            }
            else if (!strcmp(name, "coordinate_x"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->defence_gu[info_no].coordinate_x = atof(value);
            }
            else if (!strcmp(name, "coordinate_y"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->defence_gu[info_no].coordinate_y = atof(value);
            }
            else if (!strcmp(name, "pu_state"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->defence_gu[info_no].pu_online_state = atoi(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }

        info_no++;
    }

}


static __inline__ JpfGetDefenceGuRes *
jpf_dbs_cu_get_defence_gu(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfGetDefenceGuRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfGetDefenceGuRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
		query_res->back_num = 0;
        jpf_warning("login user is not exist or password not right\n");
    }
    else
    {
        len = sizeof(JpfGetDefenceGuRes) + row_num*sizeof(JpfDefenceGu);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
		query_res->total_num = row_num;
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);
        jpf_cu_get_defence_gu_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_cu_get_map_href_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfGetMapHrefRes *query_res
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
                    query_res->map_href[info_no].map_name[MAP_NAME_LEN- 1] = 0;
                    strncpy(
                        query_res->map_href[info_no].map_name,
                        value, MAP_NAME_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "dst_map_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->map_href[info_no].dst_map_id = atoi(value);
            }
            else if (!strcmp(name, "map_location"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->map_href[info_no].map_location[MAP_LOCATION_LEN - 1] = 0;
                    strncpy(
                        query_res->map_href[info_no].map_location,
                        value, MAP_LOCATION_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "coordinate_x"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->map_href[info_no].coordinate_x = atof(value);
            }
            else if (!strcmp(name, "coordinate_y"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->map_href[info_no].coordinate_y = atof(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }

        info_no++;
    }

}


static __inline__ JpfGetMapHrefRes *
jpf_dbs_cu_get_map_href(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfGetMapHrefRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfGetMapHrefRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
	 query_res->back_num = 0;
        jpf_warning("login user is not exist or password not right\n");
    }
    else
    {
        len = sizeof(JpfGetMapHrefRes) + row_num*sizeof(JpfMapHref);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
	 query_res->total_num = row_num;
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);
        jpf_cu_get_map_href_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_cu_get_tw_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfGetTwRes *query_res
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
                    query_res->tw_info[info_no].tw_id= atoi(value);
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
            else
                cms_debug("no need mysql name %s \n", name);
        }

        info_no++;
    }

}


static __inline__ JpfGetTwRes *
jpf_dbs_cu_get_tw(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfGetTwRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfGetTwRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
	 query_res->back_num = 0;
    }
    else
    {
        len = sizeof(JpfGetTwRes) + row_num * sizeof(JpfCuTwInfo);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
	 query_res->total_num = row_num;
        query_res->back_num = row_num;
        printf("query_res->total_num = %d, len = %d\n", query_res->total_num, len);
        SET_CODE(query_res, 0);
        jpf_cu_get_tw_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_cu_get_screen_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfGetScreenRes *query_res
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
                    query_res->screen_info[info_no].scr_id = atoi(value);
            }
            else if (!strcmp(name, "user_screen_num"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->screen_info[info_no].screen_num = atoi(value);
            }
            else if (!strcmp(name, "gu_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->screen_info[info_no].screen_name[SCREEN_NAME_LEN - 1] = 0;
                    strncpy(
                        query_res->screen_info[info_no].screen_name,
                        value, SCREEN_NAME_LEN - 1
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
            else if (!strcmp(name, "pu_state"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->screen_info[info_no].online_state = atoi(value);
            }
            else if (!strcmp(name, "pu_minor_type"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->screen_info[info_no].pu_minor_type = atoi(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }

        info_no++;
    }
}


static __inline__ JpfGetScreenRes *
jpf_dbs_cu_get_screen(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfGetScreenRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfGetScreenRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
	 query_res->back_num = 0;
    }
    else
    {
        len = sizeof(JpfGetScreenRes) + row_num*sizeof(JpfScreen);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
	 query_res->total_num = row_num;
        query_res->back_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);
        jpf_cu_get_screen_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_cu_get_tour_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfGetTourRes *query_res
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
            if (!strcmp(name, "user_tour_num"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->tour_info[info_no].tour_num = atoi(value);
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


static __inline__ JpfGetTourRes *
jpf_dbs_cu_get_tour(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfGetTourRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfGetTourRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
	 query_res->back_num = 0;
    }
    else
    {
        len = sizeof(JpfGetTourRes) + row_num * sizeof(JpfCuTourInfo);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
	 query_res->total_num = row_num;
        query_res->back_num = row_num;
        printf("query_res->total_num = %d, len = %d\n", query_res->total_num, len);
        SET_CODE(query_res, 0);
        jpf_cu_get_tour_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_cu_get_tour_step_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfGetTourStepRes *query_res
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
                    query_res->step_info[info_no].step_no = atoi(value);
            }
             else if (!strcmp(name, "interval"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->step_info[info_no].interval = atoi(value);
            }
            else if (!strcmp(name, "encoder_domain"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->step_info[info_no].encoder_domain[DOMAIN_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->step_info[info_no].encoder_domain,
                        value, DOMAIN_ID_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "encoder_guid"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->step_info[info_no].encoder_guid[MAX_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->step_info[info_no].encoder_guid,
                        value, MAX_ID_LEN - 1
                    );
                }
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }

        info_no++;
    }
}


static __inline__ JpfGetTourStepRes *
jpf_dbs_cu_get_tour_step(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfGetTourStepRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfGetTourStepRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
	 query_res->back_num = 0;
    }
    else
    {
        len = sizeof(JpfGetTourStepRes) + row_num * sizeof(JpfCuTourStepInfo);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
	 query_res->total_num = row_num;
        query_res->back_num = row_num;
        printf("query_res->total_num = %d, len = %d\n", query_res->total_num, len);
        SET_CODE(query_res, 0);
        jpf_cu_get_tour_step_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_cu_get_group_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfGetGroupRes *query_res
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
            if (!strcmp(name, "user_group_num"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->group_info[info_no].group_num = atoi(value);
            }
            else if (!strcmp(name, "tw_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->group_info[info_no].tw_id = atoi(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }

        info_no++;
    }
}


static __inline__ JpfGetGroupRes *
jpf_dbs_cu_get_group(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfGetGroupRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfGetGroupRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
	 query_res->back_num = 0;
    }
    else
    {
        len = sizeof(JpfGetGroupRes) + row_num * sizeof(JpfCuGroupInfo);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
	 query_res->total_num = row_num;
        query_res->back_num = row_num;
        printf("query_res->total_num = %d, len = %d\n", query_res->total_num, len);
        SET_CODE(query_res, 0);
        jpf_cu_get_group_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


void jpf_put_device_list(JpfGetDeviceRes *head)
{
    JpfGu  *pos_gu;
    JpfDevice *pos_device;
    int count = 0;
    struct list_head *cur_device = NULL,*tmp_device = NULL, *cur_gu = NULL,*tmp_gu = NULL;

    if (head->device_list)
    {
        list_for_each_safe(cur_device, tmp_device,  &head->device_list->list)
        {
            pos_device = list_entry(cur_device, JpfDevice, list);
            list_for_each_safe(cur_gu, tmp_gu, &pos_device->gu_list->list)
            {
                pos_gu = list_entry(cur_gu, JpfGu, list);
                list_del(&pos_gu->list);
                g_free(pos_gu);
            }

            g_free(pos_device->gu_list);
            count++;

            list_del(&pos_device->list);
            g_free(pos_device);
        }

        list_del(&head->device_list->list);
        g_free(head->device_list);
    }

    g_free(head);
}


void jpf_destroy_device_list(gpointer priv, gsize size)
{
    JpfGetDeviceRes *device;

    device = (JpfGetDeviceRes *)priv;
    jpf_put_device_list(device);
}


static __inline__ JpfDevice *
jpf_cu_find_and_get_device(JpfGetDeviceRes *head, gchar *puid)
{
    G_ASSERT(head != NULL);
    JpfDevice *cur = NULL;
    gint find_flag = 0;

    g_static_mutex_lock(&head->list_lock);
    list_for_each_entry(cur, &head->device_list->list, list)
    {
        if (!strcmp(cur->puid, puid))
        {
            find_flag = 1;
            break;
        }
    }
    g_static_mutex_unlock(&head->list_lock);

    if (find_flag == 0)
        return NULL;
    else
        return cur;
}


static __inline__ gint
jpf_get_device_info(JpfMysqlRes *result, JpfGetDeviceRes *query_res)
{
    G_ASSERT(query_res != NULL);

    gint field_no =0;
    JpfMysqlRow mysql_row;
    JpfMysqlField* mysql_fields;
    JpfGu  *new_gu;
    JpfDevice *new_device, *old_device = NULL;
    gchar *name;
    gchar *value;
    guint row_num;
    guint field_num;
    gint find_flag = 0;

    row_num = jpf_sql_get_num_rows(result);
    field_num = jpf_sql_get_num_fields(result);
    while ((mysql_row = jpf_sql_fetch_row(result)))
    {
        jpf_sql_field_seek(result, 0);
        mysql_fields = jpf_sql_fetch_fields(result);
        new_device = jpf_mem_kalloc(sizeof(JpfDevice));
        if (G_UNLIKELY(!new_device))
            return -E_NOMEM;

        new_gu = jpf_mem_kalloc(sizeof(JpfGu));
        if (G_UNLIKELY(!new_gu))
            return -E_NOMEM;

        memset(new_device,0,sizeof(JpfDevice));
        memset(new_gu,0,sizeof(JpfGu));
        for(field_no = 0; field_no < field_num; field_no++)
        {
            name = jpf_sql_get_field_name(mysql_fields,field_no);
            if (!strcmp(name, "pu_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if (value)
                {
                    new_device->puid[MAX_ID_LEN - 1] = 0;
                    strncpy(
                        new_device->puid,
                        value, MAX_ID_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "pu_info"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if (value)
                {
                    new_device->pu_name[PU_NAME_LEN- 1] = 0;
                    strncpy(
                        new_device->pu_name,
                        value, PU_NAME_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "pu_domain"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if (value)
                {
                    new_device->domain_id[DOMAIN_ID_LEN - 1] = 0;
                    strncpy(
                        new_device->domain_id,
                        value, DOMAIN_ID_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "gu_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if (value)
                {
                    new_gu->guid[MAX_ID_LEN - 1] = 0;
                    strncpy(
                        new_gu->guid,
                        value, MAX_ID_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "gu_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if (value)
                {
                    new_gu->gu_name[GU_NAME_LEN - 1] = 0;
                    strncpy(
                        new_gu->gu_name,
                        value, GU_NAME_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "user_gu_num"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if (value)
                    new_gu->gu_num = atoi(value);
            }
            else if (!strcmp(name, "gu_type"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if (value)
                    new_gu->gu_type = atoi(value);
            }
            else if (!strcmp(name, "gu_attributes"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if (value)
                    new_gu->gu_attribute = atoi(value);
            }
            else if (!strcmp(name, "pu_state"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if (value)
                    new_device->pu_state = atoi(value);
            }
            else if (!strcmp(name, "pu_type"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if (value)
                    new_device->pu_type = atoi(value);
            }
            else
                cms_debug("no need mysql name %s \n", name);

        }

        old_device = jpf_cu_find_and_get_device(query_res, new_device->puid);
        if (old_device)
        {
            list_add_tail(&new_gu->list, &old_device->gu_list->list);
            ++query_res->gu_count;
            jpf_mem_kfree(new_device, sizeof(JpfDevice));
            find_flag = 0;
        }
        else
        {
            new_device->gu_list = jpf_mem_kalloc(sizeof(JpfGu));
            INIT_LIST_HEAD(&new_device->gu_list->list);
            list_add_tail(&new_gu->list, &new_device->gu_list->list);
            list_add_tail(&new_device->list, &query_res->device_list->list);
            ++query_res->gu_count;
            ++query_res->device_count;
        }
    }

	return query_res->device_count;
}


static __inline__ JpfGetDeviceRes *
jpf_dbs_get_device(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len,device_num;
    JpfGetDeviceRes *query_res;

    len = sizeof(JpfGetDeviceRes);
    query_res = jpf_mem_kalloc(len);
    if (G_UNLIKELY(!query_res))
	    return NULL;

    memset(query_res, 0, len);
    query_res->device_list = jpf_mem_kalloc(sizeof(JpfDevice));
    INIT_LIST_HEAD(&query_res->device_list->list);
    query_res->device_list->gu_list = NULL;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        SET_CODE(query_res, 0);
        query_res->device_count = 0;
        query_res->gu_count= 0;
        jpf_warning("login user is not exist or password not right\n");
    }
    else
    {
        device_num =  jpf_get_device_info(mysql_res, query_res);
        if (device_num < 0)
            SET_CODE(query_res, device_num);
        else
        {
            query_res->gu_count= row_num;
            SET_CODE(query_res, 0);
        }
    }

    *size = len;

    return query_res;
}


static __inline__ void
jpf_get_area_device_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfGetAreaDeviceRes *query_res
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
            if (!strcmp(name, "pu_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->device_list[info_no].puid[MAX_ID_LEN- 1] = 0;
                    strncpy(
                        query_res->device_list[info_no].puid,
                        value, MAX_ID_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "pu_domain"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->device_list[info_no].domain_id[DOMAIN_ID_LEN- 1] = 0;
                    strncpy(
                        query_res->device_list[info_no].domain_id,
                        value, DOMAIN_ID_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "pu_info"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->device_list[info_no].pu_name[PU_NAME_LEN- 1] = 0;
                    strncpy(
                        query_res->device_list[info_no].pu_name,
                        value, PU_NAME_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "pu_type"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->device_list[info_no].pu_type = atoi(value);
            }
            else if (!strcmp(name, "pu_state"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                    query_res->device_list[info_no].pu_state = atoi(value);
            }
            else if (!strcmp(name, "pu_last_ip"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->device_list[info_no].pu_ip[MAX_IP_LEN - 1] = 0;
                    strncpy(
                        query_res->device_list[info_no].pu_ip,
                        value, MAX_IP_LEN - 1
                    );
                }
            }
            else if (!strcmp(name, "pu_last_alive"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->device_list[info_no].pu_last_alive_time[TIME_INFO_LEN - 1] = 0;
                    strncpy(
                        query_res->device_list[info_no].pu_last_alive_time,
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


static __inline__ JpfGetAreaDeviceRes *
jpf_dbs_get_area_device(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfGetAreaDeviceRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfGetAreaDeviceRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->device_count= 0;
        query_res->req_num = 0;
    }
    else
    {
        len = sizeof(JpfGetAreaDeviceRes) + row_num*sizeof(JpfDeviceInfo);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
	 query_res->device_count= row_num;
        query_res->req_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->device_count,len);
        SET_CODE(query_res, 0);
        jpf_get_area_device_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}




JpfMsgFunRet
jpf_dbs_get_all_area_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfGetArea *req_info;;
    JpfGetAreaRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size,ret;
    gint total_num;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if (req_info->area_id < 0)
    {
        snprintf(query_buf, QUERY_STR_LEN, "select * from %s where area_parent is null",
			  AREA_TABLE);
        total_num =  jpf_dbs_get_row_num(app_obj, msg, query_buf);
        if (total_num == 0)
        {
            ret = 0;
            goto err_get_area;
        }
    }
    else
    {
	snprintf(query_buf, QUERY_STR_LEN, "select * from %s where area_parent=%d",
			  AREA_TABLE, req_info->area_id);
        total_num =  jpf_dbs_get_row_num(app_obj, msg, query_buf);
        if (total_num == 0)
        {
            ret = 0;
            goto err_get_area;
        }

        snprintf(
            query_buf, QUERY_STR_LEN,
            "select * from %s where area_parent=%d order by area_name limit %d,%d",
            AREA_TABLE, req_info->area_id, req_info->start_num, req_info->req_num
        );
    }

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_all_area(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_cu> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

        //if(req_info->req_num > 0)
        query_res->total_num = total_num;

        strcpy(query_res->session, req_info->session);
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_get_area;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_get_area:

    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_CU, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_get_area:
    size = sizeof(JpfGetAreaRes);
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
    strcpy(query_res->session, req_info->session);

    goto end_get_area;
}


JpfMsgFunRet
jpf_dbs_get_area_info_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfGetAreaInfo *req_info;;
    JpfGetAreaInfoRes res_info;
    JpfMysqlRes *mysql_res = NULL;
    char query_buf[QUERY_STR_LEN];
    int code = 0, total_num;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    snprintf(
        query_buf, QUERY_STR_LEN,
        "select * from %s where area_id=%d",
        AREA_TABLE, req_info->area_id
    );
    printf("========querybuf=%s\n",query_buf);
    mysql_res = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!mysql_res);
    if (G_LIKELY(!MYSQL_RESULT_CODE(mysql_res)))  //success:0 fail:!0
        code = jpf_get_area_info(mysql_res, &res_info);
    else
        code = MYSQL_RESULT_CODE(mysql_res);

    jpf_sql_put_res(mysql_res, sizeof(JpfMysqlRes));

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select count(t2.gu_id) as count from %s as t1,%s as t2,%s as t3 where t1.pu_area=%d and \
        t2.gu_puid = t1.pu_id and t2.gu_domain=t1.pu_domain and t2.gu_id like '%%-%s-%%' \
        and t3.user_name='%s' and t3.user_guid=t2.gu_id and t3.user_guid_domain=t2.gu_domain",
        PU_TABLE, GU_TABLE, USER_OWN_GU_TABLE, req_info->area_id,AV_TYPE, req_info->username
    );

    total_num =  jpf_get_record_count(app_obj,query_buf);
    if (total_num >= 0)
        res_info.gu_count = total_num;

    strncpy(res_info.session, req_info->session, SESSION_ID_LEN - 1);
    SET_CODE(&res_info, code);
    jpf_dbs_modify_sysmsg_2(
        msg, &res_info, sizeof(res_info), BUSSLOT_POS_DBS, BUSSLOT_POS_CU
    );

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_dbs_get_defence_area_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfGetDefenceArea *req_info;;
    JpfGetDefenceAreaRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size,ret;
    gint total_num;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if (req_info->area_id < 0)
    {
        snprintf(query_buf, QUERY_STR_LEN, "select * from %s where area_parent is null",
			  AREA_TABLE);
        total_num =  jpf_dbs_get_row_num(app_obj, msg, query_buf);
        if (total_num == 0)
        {
            ret = 0;
            goto err_get_defence_area;
        }

        snprintf(
            query_buf, QUERY_STR_LEN,
            "select t1.area_id,t1.area_name,t2.defence_enable,t2.policy from %s as t1 \
            left outer join %s as t2 on t1.area_id=t2.defence_area_id where t1.area_parent is null",
            AREA_TABLE, DEFENCE_AREA_TABLE
        );			printf("----get defence area=%s\n", query_buf);
    }
    else
    {
        snprintf(query_buf, QUERY_STR_LEN, "select * from %s where area_parent=%d",
			  AREA_TABLE, req_info->area_id);
        total_num =  jpf_dbs_get_row_num(app_obj, msg, query_buf);
        if (total_num == 0)
        {
            ret = 0;
            goto err_get_defence_area;
        }

        snprintf(
            query_buf, QUERY_STR_LEN,
            "select t1.area_id,t1.area_name,t2.defence_enable,t2.policy from %s as t1 \
            left outer join %s as t2 on t1.area_id=t2.defence_area_id \
            where t1.area_parent=%d order by t1.area_name limit %d,%d",
            AREA_TABLE, DEFENCE_AREA_TABLE, req_info->area_id, req_info->start_num, req_info->req_num
        );
    }

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_cu_get_defence_area(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_cu> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

        if(req_info->req_num > 0)
            query_res->total_num = total_num;

        strcpy(query_res->session, req_info->session);
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_get_defence_area;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_get_defence_area:

    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_CU, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_get_defence_area:
    size = sizeof(JpfGetDefenceAreaRes);
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
    strcpy(query_res->session, req_info->session);

    goto end_get_defence_area;
}


JpfMsgFunRet
jpf_dbs_get_defence_map_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfGetDefenceMap *req_info;;
    JpfGetDefenceMapRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size,ret;
    gint total_num;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(query_buf, QUERY_STR_LEN, "select * from %s where defence_area_id=%d",
			 DEFENCE_MAP_TABLE, req_info->defence_area_id);

    total_num =  jpf_dbs_get_row_num(app_obj, msg, query_buf);
    if (total_num == 0)
    {
        ret = 0;
        goto err_get_defence_map;
    }

    /*snprintf(
        query_buf, QUERY_STR_LEN,
        "select t2.* from %s as t1,%s as t2 where t1.defence_area_id=%d and t1.map_id=t2.map_id limit %d,%d",
        DEFENCE_MAP_TABLE, MAP_TABLE, req_info->defence_area_id, req_info->start_num, req_info->req_num
    );	*/
   snprintf(
        query_buf, QUERY_STR_LEN,
        "select * from %s where defence_area_id=%d limit %d,%d",
        DEFENCE_MAP_TABLE, req_info->defence_area_id, req_info->start_num, req_info->req_num
    );

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_cu_get_defence_map(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_cu> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

        if(req_info->req_num > 0)
            query_res->total_num = total_num;

        strcpy(query_res->session, req_info->session);
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_get_defence_map;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_get_defence_map:

    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_CU, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_get_defence_map:
    size = sizeof(JpfGetDefenceMapRes);
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
    strcpy(query_res->session, req_info->session);

    goto end_get_defence_map;
}


JpfMsgFunRet
jpf_dbs_get_defence_gu_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfGetDefenceGu *req_info;;
    JpfGetDefenceGuRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size,ret;
    gint total_num;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(query_buf, QUERY_STR_LEN, "select t1.* from %s as t1,%s as t2 where t1.map_id=%d and t2.user_guid=t1.gu_id and t2.user_guid_domain=t1.gu_domain and t2.user_name='%s'",
		MAP_GU_TABLE, USER_OWN_GU_TABLE, req_info->map_id, req_info->username);
    total_num =  jpf_dbs_get_row_num(app_obj, msg, query_buf);
    if (total_num == 0)
    {
        ret = 0;
        goto err_get_defence_gu;
    }

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select t1.*,t2.gu_name,t2.gu_type,t4.pu_state from %s as t1,%s as t2, %s as t3,%s as t4 \
        where t1.map_id=%d and t1.gu_id=t2.gu_id and t1.gu_domain=t2.gu_domain \
        and t3.user_guid=t1.gu_id and t3.user_guid_domain=t1.gu_domain and \
        t3.user_name='%s' and t2.gu_puid=t4.pu_id limit %d,%d",
         MAP_GU_TABLE, GU_TABLE, USER_OWN_GU_TABLE, PU_RUNNING_TABLE,req_info->map_id, req_info->username,
         req_info->start_num, req_info->req_num
    );

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_cu_get_defence_gu(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_cu> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

        if(req_info->req_num > 0)
            query_res->total_num = total_num;

        strcpy(query_res->session, req_info->session);
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_get_defence_gu;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_get_defence_gu:

    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_CU, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_get_defence_gu:
    size = sizeof(JpfGetDefenceGuRes);
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
    strcpy(query_res->session, req_info->session);

    goto end_get_defence_gu;
}


JpfMsgFunRet
jpf_dbs_get_map_href_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfGetMapHref *req_info;;
    JpfGetMapHrefRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size,ret;
    gint total_num;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(query_buf, QUERY_STR_LEN, "select * from %s where src_map_id=%d",
			 MAP_HREF_TABLE, req_info->map_id);

    total_num =  jpf_dbs_get_row_num(app_obj, msg, query_buf);
    if (total_num == 0)
    {
        ret = 0;
        goto err_get_map_href;
    }

    /*snprintf(
        query_buf, QUERY_STR_LEN,
        "select t1.*,t2.* from %s as t1,%s as t2 where t1.src_map_id=%d and t1.dst_map_id=t2.map_id limit %d,%d",
        MAP_HREF_TABLE, MAP_TABLE, req_info->map_id, req_info->start_num, req_info->req_num
    );	*/

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select t1.*,t2.* from %s as t1,%s as t2 where t1.src_map_id=%d and \
        t1.dst_map_id=t2.map_id limit %d,%d",
        MAP_HREF_TABLE, DEFENCE_MAP_TABLE,
        req_info->map_id, req_info->start_num, req_info->req_num
    );
    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_cu_get_map_href(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_cu> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

        if(req_info->req_num > 0)
            query_res->total_num = total_num;

        strcpy(query_res->session, req_info->session);
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_get_map_href;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_get_map_href:

    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_CU, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_get_map_href:
    size = sizeof(JpfGetMapHrefRes);
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
    strcpy(query_res->session, req_info->session);

    goto end_get_map_href;
}


static __inline__ gint
jpf_get_gu_map_location(JpfMysqlRes *mysql_result, JpfGetGuMapLocationRes *res_info)
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

                if (!strcmp(name, "defence_area_id"))
                {
                    value = jpf_sql_get_field_value(mysql_row, j);
                    if(value)
                        res_info->defence_area_id = atoi(value);
                }
                else if (!strcmp(name, "map_id"))
                {
                    value = jpf_sql_get_field_value(mysql_row, j);
                    if(value)
                        res_info->map_id = atoi(value);
                }
                else if (!strcmp(name,"map_name"))
                {
                    value = jpf_sql_get_field_value(mysql_row, j);
                    res_info->map_name[MAP_NAME_LEN - 1] = 0;
                    strncpy(res_info->map_name, value, MAP_NAME_LEN - 1);
                }
                else if (!strcmp(name,"map_location"))
                {
                    value = jpf_sql_get_field_value(mysql_row, j);
                    res_info->map_location[MAP_LOCATION_LEN - 1] = 0;
                    strncpy(res_info->map_location, value, MAP_LOCATION_LEN - 1);
                }
                else if (!strcmp(name, "coordinate_x"))
                {
                    value = jpf_sql_get_field_value(mysql_row, j);
                    if(value)
                        res_info->coordinate_x = atof(value);
                }
                else if (!strcmp(name, "coordinate_y"))
                {
                    value = jpf_sql_get_field_value(mysql_row, j);
                    if(value)
                        res_info->coordinate_y = atof(value);
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
jpf_dbs_get_gu_map_location_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfGetGuMapLocation *req_info;;
    JpfGetGuMapLocationRes res_info;
    JpfMysqlRes *mysql_res = NULL;
    char query_buf[QUERY_STR_LEN];
    int code = 0;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select t1.coordinate_x,t1.coordinate_y,t2.* from %s as t1,%s as t2 where \
        t1.gu_domain='%s' and t1.gu_id='%s' and t1.map_id=t2.map_id",
        MAP_GU_TABLE, DEFENCE_MAP_TABLE, req_info->domain_id, req_info->guid
    );
    printf("========querybuf=%s\n",query_buf);
    mysql_res = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!mysql_res);
    if (G_LIKELY(!MYSQL_RESULT_CODE(mysql_res)))  //success:0 fail:!0
        code = jpf_get_gu_map_location(mysql_res, &res_info);
    else
        code = MYSQL_RESULT_CODE(mysql_res);

    jpf_sql_put_res(mysql_res, sizeof(JpfMysqlRes));
    strncpy(res_info.session, req_info->session, SESSION_ID_LEN - 1);
    SET_CODE(&res_info, code);
    jpf_dbs_modify_sysmsg_2(
        msg, &res_info, sizeof(res_info), BUSSLOT_POS_DBS, BUSSLOT_POS_CU
    );

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_dbs_get_tw_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfGetTw *req_info;;
    JpfGetTwRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size,ret;
    gint total_num;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(query_buf, QUERY_STR_LEN,
	"select count(*)  as count from %s where user_name='%s'",
	USER_OWN_TW_TABLE, req_info->username
    );
    total_num =  jpf_get_record_count(app_obj,query_buf);
    if (total_num == 0)
    {
        ret = 0;
        goto err_get_tw;
    }

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select t1.* from %s as t1,%s as t2 where t2.user_name='%s' and \
        t1.tw_id=t2.user_tw_id limit %d,%d",
         TW_TABLE, USER_OWN_TW_TABLE, req_info->username, req_info->start_num,
         req_info->req_num
    );

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_cu_get_tw(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_cu> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

        if(req_info->req_num > 0)
            query_res->total_num = total_num;

        strcpy(query_res->session, req_info->session);
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_get_tw;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_get_tw:

    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_CU, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_get_tw:
    size = sizeof(JpfGetTwRes);
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
    strcpy(query_res->session, req_info->session);

    goto end_get_tw;
}


JpfMsgFunRet
jpf_dbs_get_screen_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfGetScreen *req_info;
    JpfGetScreenRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size,ret;
    gint total_num;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(query_buf, QUERY_STR_LEN,
	"select count(*)  as count from %s where tw_id=%d",
	SCREEN_TABLE, req_info->tw_id
    );
    total_num =  jpf_get_record_count(app_obj,query_buf);
    if (total_num == 0)
    {
        ret = 0;
        goto err_get_screen;
    }

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select t5.*,t6.user_screen_num from (select t1.*,t2.gu_name,t3.pu_state,t4.pu_minor_type from %s as t1,\
        %s as t2,%s as t3,%s as t4 where tw_id=%d and t1.dis_guid=t2.gu_id and \
        t1.dis_domain=t2.gu_domain and t2.gu_puid=t3.pu_id and \
        t2.gu_domain=t3.pu_domain and t2.gu_puid=t4.pu_id and \
        t2.gu_domain=t4.pu_domain limit %d,%d) t5 left join %s as t6 on \
        t6.user_name='%s' and t5.scr_id=t6.screen_id",
         SCREEN_TABLE, GU_TABLE, PU_RUNNING_TABLE, PU_TABLE,
         req_info->tw_id, req_info->start_num, req_info->req_num,
         USER_SCREEN_NUMBER_TABLE, req_info->user
    );

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_cu_get_screen(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_cu> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

        if(req_info->req_num > 0)
            query_res->total_num = total_num;

        strcpy(query_res->session, req_info->session);
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_get_screen;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_get_screen:

    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_CU, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_get_screen:
    size = sizeof(JpfGetScreenRes);
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
    strcpy(query_res->session, req_info->session);

    goto end_get_screen;
}


JpfMsgFunRet
jpf_dbs_get_scr_div_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfGetDivMode *req_info;;
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
        "select * from %s limit %d,%d",
         SCREEN_DIVISION_TABLE, req_info->start_num, req_info->req_num
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

        if(req_info->req_num > 0)
            query_res->total_num = total_num;

        strcpy(query_res->session, req_info->session);
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
                            BUSSLOT_POS_CU, jpf_mem_kfree);

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
    strcpy(query_res->session, req_info->session);

    goto end_get_scr_div;
}


JpfMsgFunRet
jpf_dbs_get_scr_state_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfGetScrState *req_info;
    int code;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    code = jpf_get_scr_display_guid(app_obj, &req_info->guid,
        req_info->screen_id, req_info->tw_id);
    MSG_SET_DSTPOS(msg, BUSSLOT_POS_PU);

    return MFR_DELIVER_BACK;
}

/*
JpfMsgFunRet
jpf_dbs_change_div_mode_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfChangeDivMode *req_info;
    int code;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    code = jpf_get_scr_display_guid(app_obj, &req_info->guid,
        req_info->screen_id, req_info->tw_id);

    MSG_SET_DSTPOS(msg, BUSSLOT_POS_PU);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_dbs_full_screen_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfFullScreen *req_info;
    int code;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    code = jpf_get_scr_display_guid(app_obj, &req_info->guid,
        req_info->screen_id, req_info->tw_id);

    MSG_SET_DSTPOS(msg, BUSSLOT_POS_PU);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_dbs_exit_full_screen_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfExitFullScreen *req_info;
    int code;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    code = jpf_get_scr_display_guid(app_obj, &req_info->guid,
        req_info->screen_id, req_info->tw_id);

    MSG_SET_DSTPOS(msg, BUSSLOT_POS_PU);

    return MFR_DELIVER_BACK;
}
*/

JpfMsgFunRet
jpf_dbs_get_tour_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfGetTour *req_info;
    JpfGetTourRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size,ret;
    gint total_num;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(query_buf, QUERY_STR_LEN,
	"select count(*) as count from %s where user_name='%s'",
	USER_OWN_TOUR_TABLE, req_info->username
    );
    total_num =  jpf_get_record_count(app_obj,query_buf);
    if (total_num == 0)
    {
        ret = 0;
        goto err_get_tour;
    }

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select t3.*,t4.user_tour_num from (select t1.* from %s as t1,%s as t2 where user_name='%s' and \
        t1.tour_id=t2.user_tour_id limit %d,%d) t3 left join %s as t4 on t4.user_name='%s' and t3.tour_id=t4.tour_id",
         TOUR_TABLE, USER_OWN_TOUR_TABLE, req_info->username,
         req_info->start_num, req_info->req_num,
         USER_TOUR_NUMBER_TABLE, req_info->username
    );

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_cu_get_tour(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_cu> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

        if(req_info->req_num > 0)
            query_res->total_num = total_num;

        strcpy(query_res->session, req_info->session);
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_get_tour;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_get_tour:

    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_CU, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_get_tour:
    size = sizeof(JpfGetTourRes);
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
    strcpy(query_res->session, req_info->session);

    goto end_get_tour;
}


JpfMsgFunRet
jpf_dbs_get_tour_step_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfGetTourStep *req_info;;
    JpfGetTourStepRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size,ret;
    gint total_num;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(query_buf, QUERY_STR_LEN,
	"select count(*)  as count from %s where tour_id=%d",
	TOUR_STEP_TABLE, req_info->tour_id
    );
    total_num =  jpf_get_record_count(app_obj,query_buf);
    if (total_num == 0)
    {
        ret = 0;
        goto err_get_tour_step;
    }

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select * from %s where tour_id=%d limit %d,%d",
         TOUR_STEP_TABLE, req_info->tour_id, req_info->start_num, req_info->req_num
    );

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_cu_get_tour_step(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_cu> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

        if(req_info->req_num > 0)
            query_res->total_num = total_num;

        strcpy(query_res->session, req_info->session);
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_get_tour_step;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_get_tour_step:

    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_CU, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_get_tour_step:
    size = sizeof(JpfGetTourStepRes);
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
    strcpy(query_res->session, req_info->session);

    goto end_get_tour_step;
}


JpfMsgFunRet
jpf_dbs_get_group_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfGetGroup *req_info;
    JpfGetGroupRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size,ret;
    gint total_num;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(query_buf, QUERY_STR_LEN,
	"select count(*) as count from %s",
	GROUP_TABLE
    );
    total_num =  jpf_get_record_count(app_obj,query_buf);
    if (total_num == 0)
    {
        ret = 0;
        goto err_get_group;
    }

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select t1.*,t2.user_group_num from %s as t1 left join %s as t2 on " \
        "t2.user_name='%s' and t1.group_id=t2.group_id limit %d,%d",
         GROUP_TABLE, USER_GROUP_NUMBER_TABLE,
         req_info->user, req_info->start_num, req_info->req_num
    );

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_cu_get_group(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_cu> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

        if(req_info->req_num > 0)
            query_res->total_num = total_num;

        strcpy(query_res->session, req_info->session);
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_get_group;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_get_group:

    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_CU, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_get_group:
    size = sizeof(JpfGetGroupRes);
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
    strcpy(query_res->session, req_info->session);

    goto end_get_group;
}


JpfMsgFunRet
jpf_dbs_get_device_list_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfGetDevice *req_info;;
    JpfGetDeviceRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size,ret;
    gint total_num;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select t1.pu_id,t1.pu_domain,t3.pu_state,t1.pu_info,t1.pu_type, \
        t2.gu_id,t2.gu_name,t2.gu_type,t2.gu_attributes from %s as t1,%s as t2,\
        %s as t3, %s where t1.pu_area=%d and t1.pu_id=t2.gu_puid and \
        t1.pu_domain=t2.gu_domain and t1.pu_id=t3.pu_id and \
        t1.pu_domain=t3.pu_domain and %s.user_name='%s' and \
        %s.user_guid=t2.gu_id and %s.user_guid_domain=t2.gu_domain",
        PU_TABLE, GU_TABLE, PU_RUNNING_TABLE, USER_OWN_GU_TABLE,
        req_info->area_id, USER_OWN_GU_TABLE, req_info->username,
        USER_OWN_GU_TABLE, USER_OWN_GU_TABLE
    );
    if(req_info->req_num > 0)
    {
        total_num =  jpf_dbs_get_row_num(app_obj, msg, query_buf);
        if (total_num == 0)
        {
            ret = 0;
            goto err_get_device_list;
        }

       snprintf(
       	query_buf, QUERY_STR_LEN,
       	"select t4.*,t5.user_gu_num from (select t1.pu_id,t1.pu_domain,t3.pu_state,t1.pu_info,t1.pu_type,\
       	t2.gu_id,t2.gu_name,t2.gu_type,t2.gu_attributes from %s as t1,%s as t2,\
       	%s as t3 ,%s where t1.pu_area=%d and t1.pu_id=t2.gu_puid and \
       	t1.pu_domain=t2.gu_domain and t1.pu_id=t3.pu_id and \
       	t1.pu_domain=t3.pu_domain and %s.user_name='%s' and \
       	%s.user_guid=t2.gu_id and %s.user_guid_domain=t2.gu_domain \
       	order by t1.pu_info limit %d,%d) t4 left join %s as t5 on \
       	t4.gu_id=t5.user_guid and t4.pu_domain=t5.user_guid_domain \
       	and t5.user_name='%s'",
       	PU_TABLE, GU_TABLE, PU_RUNNING_TABLE, USER_OWN_GU_TABLE, 
       	req_info->area_id, USER_OWN_GU_TABLE, req_info->username,
       	USER_OWN_GU_TABLE, USER_OWN_GU_TABLE,
       	req_info->start_num, req_info->req_num,
       	USER_GU_NUMBER_TABLE, req_info->username
       	);
   }

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_device(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_cu> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

        if(req_info->req_num > 0)
            query_res->gu_count = total_num;

        strcpy(query_res->session, req_info->session);
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_get_device_list;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_get_device_list:
    MSG_SET_DSTPOS(msg, BUSSLOT_POS_CU);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private(msg, query_res, size, jpf_destroy_device_list);

    return MFR_DELIVER_BACK;

err_get_device_list:
    query_res = jpf_mem_kalloc(sizeof(JpfGetDeviceRes));
    if (!query_res)
    {
        strcpy(query_res->session, req_info->session);
        jpf_warning("<dbs_mh_cu> alloc error");
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

	strcpy(query_res->session, req_info->session);
    size =  sizeof(JpfGetDeviceRes);
    SET_CODE(query_res, ret);
    query_res->gu_count = 0;
    query_res->device_list = NULL;
    goto end_get_device_list;
}


JpfMsgFunRet
jpf_dbs_get_area_device_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfGetAreaDevice *req_info;;
    JpfGetAreaDeviceRes *query_res;
    JpfMysqlRes *result;
    char query_buf[QUERY_STR_LEN] = {0};
    gint size,ret;
    gint total_num;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select count(distinct t1.pu_id,t1.pu_domain) as count from %s as t1,%s as t2,\
        %s as t3,%s where t1.pu_area=%d and t1.pu_id=t2.pu_id and \
        t1.pu_domain=t2.pu_domain and t1.pu_id=t3.gu_puid and \
        t1.pu_domain=t3.gu_domain   and %s.user_name='%s' \
        and %s.user_guid=t3.gu_id and %s.user_guid_domain=t3.gu_domain",
        PU_TABLE, PU_RUNNING_TABLE,GU_TABLE,USER_OWN_GU_TABLE,
        req_info->area_id, USER_OWN_GU_TABLE, req_info->username,
        USER_OWN_GU_TABLE, USER_OWN_GU_TABLE
    );
    printf("1111-------get area device :%s\n",query_buf);

    total_num =  jpf_get_record_count(app_obj,query_buf);  printf("---total_num=%d\n",total_num);
    if (total_num == 0)
    {
        ret = 0;
        goto err_get_area_device;
    }

   snprintf(
   	query_buf, QUERY_STR_LEN,
   	"select distinct t1.pu_id,t1.pu_domain,t2.pu_state,t1.pu_info,t1.pu_type, t2.pu_last_ip,\
         t2.pu_last_alive from %s as t1,%s as t2,%s as t3,%s where t1.pu_area=%d and \
         t1.pu_id=t2.pu_id and t1.pu_domain=t2.pu_domain and t1.pu_id=t3.gu_puid \
         and t1.pu_domain=t3.gu_domain  and %s.user_name='%s' \
         and %s.user_guid=t3.gu_id and %s.user_guid_domain=t3.gu_domain limit %d,%d",
   	PU_TABLE, PU_RUNNING_TABLE,GU_TABLE, USER_OWN_GU_TABLE, req_info->area_id,
   	USER_OWN_GU_TABLE, req_info->username,USER_OWN_GU_TABLE,
   	USER_OWN_GU_TABLE, req_info->start_num, req_info->req_num
   	);
   printf("-------get  area device :%s\n",query_buf);

    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_area_device(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_cu> alloc error");
            jpf_sql_put_res(result, sizeof(JpfMysqlRes));
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

        if(req_info->req_num > 0)
            query_res->device_count = total_num;

        strcpy(query_res->session, req_info->session);
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_get_area_device;
    }

    if(result)
       jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_get_area_device:
    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_CU, jpf_mem_kfree);
    return MFR_DELIVER_BACK;

err_get_area_device:
    size =  sizeof(JpfGetAreaDeviceRes);
    query_res = jpf_mem_kalloc(size);
    if (!query_res)
    {
        strcpy(query_res->session, req_info->session);
        jpf_warning("<dbs_mh_cu> alloc error");
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    memset(query_res, 0, size);
    strcpy(query_res->session, req_info->session);
    query_res->device_count = 0;
    SET_CODE(query_res, ret);
    goto end_get_area_device;
}


JpfMsgFunRet
jpf_dbs_get_mds_ip_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfMsgGetMdsIp *req_info;
    JpfMsgGetMdsIpRes res_info;
    JpfMysqlRes *mysql_res;
    char query_buf[QUERY_STR_LEN];
    int code = 0;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select mdu_ip from %s where mdu_cmsip='%s' and \
        mdu_id=(select pu_mdu from %s where pu_id='%s' and pu_domain='%s' )",
        MDS_IP_TABLE, req_info->cms_ip, PU_TABLE, req_info->puid, req_info->domain_id
	 );
    printf("========querybuf=%s\n",query_buf);
    mysql_res = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!mysql_res);
    if (G_LIKELY(!MYSQL_RESULT_CODE(mysql_res)))  //success:0 fail:!0
        code = jpf_get_mds_ip(mysql_res, &res_info);
    else
        code = MYSQL_RESULT_CODE(mysql_res);

    jpf_sql_put_res(mysql_res, sizeof(JpfMysqlRes));
    SET_CODE(&res_info, code);
    jpf_dbs_modify_sysmsg_2(
        msg, &res_info, sizeof(res_info), BUSSLOT_POS_DBS, BUSSLOT_POS_MDS
    );

    return MFR_DELIVER_BACK;
}


static __inline__ gint jpf_dbs_check_user_own_gu(JpfAppObj *app_obj,
    gchar *guid, gchar *domain_id, gchar *username)
{
    gchar query_buf[QUERY_STR_LEN] = {0};
    gint total_num;
    gchar own_guid[MAX_ID_LEN] = {0};

    strncpy(own_guid, guid, MAX_ID_LEN - 1);
    jpf_set_guid_level(own_guid);
    snprintf(
        query_buf, QUERY_STR_LEN,
        "select count(*) as count from %s where user_name='%s' and user_guid='%s' and \
        user_guid_domain='%s'",
        USER_OWN_GU_TABLE, username, own_guid, domain_id
    );

    total_num = jpf_get_record_count(app_obj,query_buf);
    if (total_num < 0)
        return total_num;
    else if (total_num == 0)
        return  -EPERM;
    else
        return 0;
}


JpfMsgFunRet
jpf_dbs_get_media_url_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfGetMediaUrl *req_info;
    JpfMsgGetUrl res_info;
    JpfGetMediaUrlRes err_res;
    JpfMsgGetMdsIpRes mds_ip;
    gchar puid[MAX_ID_LEN] = {0};
    gint code = 0;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    puid[MAX_ID_LEN - 1] = '\0';

    get_mached_string(req_info->guid, puid, PU_ID_LEN, puid_reg);
    code = jpf_dbs_check_user_own_gu(app_obj,req_info->guid, req_info->domain_id,
        req_info->username);
    if (code)
        goto err_get_media_url;

    code = jpf_dbs_get_mds_info(app_obj, req_info->ip, puid, req_info->domain_id, &mds_ip);
    strcpy(res_info.session, req_info->session);
    if (G_LIKELY(!code))
    {
        strcpy(res_info.mds_ip, mds_ip.mds_ip);
        strcpy(res_info.mds_id, mds_ip.mds_id);
        res_info.rtsp_port = mds_ip.rtsp_port;
        SET_CODE(&res_info, code);
        strcpy(res_info.domain_id, req_info->domain_id);
        strcpy(res_info.guid, req_info->guid);
        strcpy(res_info.cms_ip, req_info->ip);
	 strcpy(res_info.mss_id, req_info->mss_id);
	 strcpy(res_info.username, req_info->username);
        res_info.media = req_info->media;
        jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
        MSG_SET_DSTPOS(msg, BUSSLOT_POS_MDS);

        return MFR_DELIVER_BACK;
    }

err_get_media_url:
    jpf_warning("<GetMediaUrl> get media url error:%d", code);
    memset(&err_res, 0, sizeof(err_res));
    strcpy(err_res.session, req_info->session);
    SET_CODE(&err_res, -code);
    jpf_dbs_modify_sysmsg_2(
        msg, &err_res, sizeof(err_res),
        BUSSLOT_POS_DBS, BUSSLOT_POS_CU
    );

    return MFR_DELIVER_BACK;
}


static __inline__ void
jpf_get_alarm_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfGetAlarmRes *query_res
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
            else
                cms_debug("no need mysql name %s \n", name);
        }

        info_no++;
    }

}


static __inline__ JpfGetAlarmRes *
jpf_dbs_get_alarm(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfGetAlarmRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfGetAlarmRes);
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
        len = sizeof(JpfGetAlarmRes) + row_num*sizeof(JpfAlarm);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
	 query_res->total_num = row_num;
        query_res->req_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);
        jpf_get_alarm_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


JpfMsgFunRet
jpf_dbs_get_alarm_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfGetAlarm *req_info;;
    JpfGetAlarmRes *query_res;
    JpfMysqlRes *result;
    gchar query_buf[QUERY_STR_LEN] = {0};
    gint size,ret;
    gint total_num;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(query_buf, QUERY_STR_LEN,
	"select count(*)  as count from %s as t1,%s as t2 where t1.state&%d and t1.alarm_type&%d and t1.alarm_time>'%s' \
	and t1.alarm_time<'%s' and t2.user_guid=t1.gu_id and t2.user_guid_domain=t1.gu_domain and t2.user_name='%s'",
	ALARM_INFO_TABLE, USER_OWN_GU_TABLE, req_info->alarm_state, req_info->alarm_type, req_info->start_time,
	req_info->end_time, req_info->username);
    printf("-----------get alarm count:%s\n",query_buf);
    total_num = jpf_get_record_count(app_obj,query_buf);
    if (total_num == 0)
    {
        ret = 0;
        goto err_get_alarm;
    }

    snprintf(query_buf, QUERY_STR_LEN,
	"select t1.* from  %s as t1,%s as t2 where t1.state&%d and t1.alarm_type&%d and t1.alarm_time>'%s' and \
	t1.alarm_time<'%s' and t2.user_guid=t1.gu_id and t2.user_guid_domain=t1.gu_domain and t2.user_name='%s'\
	order by t1.alarm_time desc limit %d,%d",
	ALARM_INFO_TABLE, USER_OWN_GU_TABLE, req_info->alarm_state, req_info->alarm_type,req_info->start_time,
	req_info->end_time, req_info->username, req_info->start_num, req_info->req_num);
    printf("11111111-----------get alarm :%s\n",query_buf);
    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_alarm(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_cu> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

        if(req_info->req_num > 0)
            query_res->total_num = total_num;

        strcpy(query_res->session, req_info->session);
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_get_alarm;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_get_alarm:

    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_CU, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_get_alarm:
    size = sizeof(JpfGetAlarmRes);
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
    strcpy(query_res->session, req_info->session);

    goto end_get_alarm;
}


static __inline__ gint
jpf_get_alarm_state(JpfMysqlRes *mysql_result, JpfGetAlarmStateRes *res_info)
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

                if (!strcmp(name,"operator"))
                {
                    value = jpf_sql_get_field_value(mysql_row, j);
                    res_info->operator[DESCRIPTION_INFO_LEN- 1] = 0;
		  //    if (value)
                        strncpy(res_info->operator, value, DESCRIPTION_INFO_LEN - 1);
		      //else
			 //  strcpy(res_info->operator, "");
                }
                else if (!strcmp(name,"deal_time"))
                {
                    value = jpf_sql_get_field_value(mysql_row, j);
                    res_info->deal_time[TIME_INFO_LEN - 1] = 0;
                    strncpy(res_info->deal_time, value, TIME_INFO_LEN - 1);
                }
                else if (!strcmp(name,"description"))
                {
                    value = jpf_sql_get_field_value(mysql_row, j);
                    res_info->description[DESCRIPTION_INFO_LEN - 1] = 0;
                    strncpy(res_info->description, value, DESCRIPTION_INFO_LEN - 1);
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
jpf_dbs_get_alarm_state_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfGetAlarmState *req_info;;
    JpfGetAlarmStateRes res_info;
    JpfMysqlRes *mysql_res = NULL;
    char query_buf[QUERY_STR_LEN];
    int code = 0;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(
        query_buf, QUERY_STR_LEN,
        "select operator,deal_time,description from %s where alarm_id=%d",
        ALARM_INFO_TABLE, req_info->alarm_id
	 );
    printf("========querybuf=%s\n",query_buf);
    mysql_res = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!mysql_res);
    if (G_LIKELY(!MYSQL_RESULT_CODE(mysql_res)))  //success:0 fail:!0
        code = jpf_get_alarm_state(mysql_res, &res_info);
    else
        code = MYSQL_RESULT_CODE(mysql_res);

    jpf_sql_put_res(mysql_res, sizeof(JpfMysqlRes));
    strncpy(res_info.session, req_info->session, SESSION_ID_LEN - 1);
    SET_CODE(&res_info, code);
    jpf_dbs_modify_sysmsg_2(
        msg, &res_info, sizeof(res_info), BUSSLOT_POS_DBS, BUSSLOT_POS_CU
    );

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_dbs_deal_alarm_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfDealAlarm *req_info;;
    JpfDealAlarmRes res_info;
    gchar query_buf[QUERY_STR_LEN] = {0};
    gchar deal_time[TIME_STRING_LEN] = {0};
    gint state = 1<<1;
    glong affect_num = 0;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    jpf_get_current_zone_time(deal_time);
    snprintf(
        query_buf, QUERY_STR_LEN,
        "update %s set operator='%s',deal_time='%s',description='%s',state=%d where alarm_id=%d",
        ALARM_INFO_TABLE, req_info->operator, deal_time, req_info->description, state, req_info->alarm_id
	 );
    printf("========querybuf=%s\n",query_buf);
    memset(&res_info, 0, sizeof(res_info));
    jpf_dbs_do_query_code(app_obj, msg, query_buf, &res_info.code, &affect_num);
    strncpy(res_info.session, req_info->session, SESSION_ID_LEN - 1);
    jpf_dbs_modify_sysmsg_2(msg, &res_info, sizeof(res_info), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_CU);

    return MFR_DELIVER_BACK;
}


static __inline__ void
jpf_get_gu_mss_info(JpfMysqlRes *result,
                   gint row_num1,
                   JpfGetGuMssRes *query_res
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

            if (!strcmp(name, "mss_id"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->mss_list[info_no].mss_id[MSS_ID_LEN - 1] = 0;
                    strncpy(
                        query_res->mss_list[info_no].mss_id,
                        value, MSS_ID_LEN - 1
                    );
                }
            }
	     else if (!strcmp(name, "mss_name"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->mss_list[info_no].mss_name[MSS_NAME_LEN - 1] = 0;
                    strncpy(
                        query_res->mss_list[info_no].mss_name,
                        value, MSS_NAME_LEN - 1
                    );
                }
            }
	     else if (!strcmp(name, "hd_group_id1"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->mss_list[info_no].hd_group[0].hd_group_id[HD_GROUP_ID_LEN - 1]= 0;
                    strncpy(
                        query_res->mss_list[info_no].hd_group[0].hd_group_id,
                        value, HD_GROUP_ID_LEN - 1
                    );
                }
            }
	     else if (!strcmp(name, "hd_group_id2"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->mss_list[info_no].hd_group[1].hd_group_id[HD_GROUP_ID_LEN - 1]= 0;
                    strncpy(
                        query_res->mss_list[info_no].hd_group[1].hd_group_id,
                        value, HD_GROUP_ID_LEN - 1
                    );
                }
            }
	     else if (!strcmp(name, "hd_group_id3"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->mss_list[info_no].hd_group[2].hd_group_id[HD_GROUP_ID_LEN - 1]= 0;
                    strncpy(
                        query_res->mss_list[info_no].hd_group[2].hd_group_id,
                        value, HD_GROUP_ID_LEN - 1
                    );
                }
            }
	     else if (!strcmp(name, "hd_group_id4"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->mss_list[info_no].hd_group[3].hd_group_id[HD_GROUP_ID_LEN - 1]= 0;
                    strncpy(
                        query_res->mss_list[info_no].hd_group[3].hd_group_id,
                        value, HD_GROUP_ID_LEN - 1
                    );
                }
            }
	     else if (!strcmp(name, "hd_group_id5"))
            {
                value = jpf_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    query_res->mss_list[info_no].hd_group[4].hd_group_id[HD_GROUP_ID_LEN - 1]= 0;
                    strncpy(
                        query_res->mss_list[info_no].hd_group[4].hd_group_id,
                        value, HD_GROUP_ID_LEN - 1
                    );
                }
            }
            else
                cms_debug("no need mysql name %s \n", name);
        }

        info_no++;
    }
}


static __inline__ JpfGetGuMssRes *
jpf_dbs_get_gu_mss(JpfMysqlRes *mysql_res, gint *size)
{
    gint row_num, len;
    JpfGetGuMssRes *query_res;

    row_num = jpf_sql_get_num_rows(mysql_res);
    if (row_num == 0)
    {
        len = sizeof(JpfGetGuMssRes);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
        SET_CODE(query_res, E_NODBENT);
        query_res->total_num = 0;
        jpf_warning("login user is not exist or password not right\n");
    }
    else
    {
        len = sizeof(JpfGetGuMssRes) + row_num*sizeof(JpfGuMss);
        query_res = jpf_mem_kalloc(len);
        if (G_UNLIKELY(!query_res))
            return NULL;

        memset(query_res, 0, len);
	 query_res->total_num = row_num;
        printf("query_res->total_num=%d,len=%d\n",query_res->total_num,len);
        SET_CODE(query_res, 0);
        jpf_get_gu_mss_info(mysql_res, len, query_res);
    }

    *size = len;

    return query_res;
}


JpfMsgFunRet
jpf_dbs_get_gu_mss_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfGetGuMss *req_info;;
    JpfGetGuMssRes *query_res;
    JpfMysqlRes *result;
    gchar query_buf[QUERY_STR_LEN] = {0};
    gint size,ret;
    gint total_num;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    snprintf(query_buf, QUERY_STR_LEN,
	"select count(*)  as count from %s as t1,%s as t2 where t1.gu_id='%s' and t1.guid_domain='%s' and t1.mss_id=t2.mss_id",
	RECORD_POLICY_TABLE, MSS_TABLE, req_info->guid, req_info->domain_id
    );
    printf("-----------get gu mss count:%s\n",query_buf);
    total_num = jpf_get_record_count(app_obj,query_buf);
    if (total_num == 0)
    {
        ret = 0;
        goto err_get_gu_mss;
    }

    snprintf(query_buf, QUERY_STR_LEN,
	"select t1.mss_id,t2.mss_name,t1.hd_group_id1,t1.hd_group_id2,t1.hd_group_id3,t1.hd_group_id4,t1.hd_group_id5 from  %s as t1,%s as t2 where t1.gu_id='%s' \
	 and t1.guid_domain='%s' and t1.mss_id=t2.mss_id",
	RECORD_POLICY_TABLE, MSS_TABLE, req_info->guid, req_info->domain_id);
    printf("11111111-----------get gu mss :%s\n",query_buf);
    result = jpf_dbs_do_query_res(app_obj, query_buf);
    BUG_ON(!result);

    if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        query_res = jpf_dbs_get_gu_mss(result, &size);
        if (G_UNLIKELY(!query_res))
        {
            jpf_warning("<dbs_mh_cu> alloc error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

        strcpy(query_res->session, req_info->session);
    }
    else
    {
        ret = MYSQL_RESULT_CODE(result);
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));
        goto err_get_gu_mss;
    }

    if(result)
        jpf_sql_put_res(result, sizeof(JpfMysqlRes));

end_get_gu_mss:

    jpf_dbs_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
                            BUSSLOT_POS_CU, jpf_mem_kfree);

    return MFR_DELIVER_BACK;

err_get_gu_mss:
    size = sizeof(JpfGetGuMssRes);
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
    strcpy(query_res->session, req_info->session);

    goto end_get_gu_mss;
}


JpfMsgFunRet
jpf_dbs_cu_modify_user_pwd_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfCuModifyUserPwd *req_info;;
    JpfCuModifyUserPwdRes res_info;
    gchar query_buf[QUERY_STR_LEN] = {0};
    gint total_num;
    glong affect_num = 0;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

	memset(&res_info, 0, sizeof(res_info));
    snprintf(
        query_buf, QUERY_STR_LEN,
        "select count(*)  as count from %s where user_name='%s' and user_password='%s'",
        USER_TABLE, req_info->username, req_info->old_password
	);
	total_num = jpf_get_record_count(app_obj,query_buf);
    if (total_num == 0)
    {
        SET_CODE(&res_info, -E_PASSWD);
        goto err_modify_user_pwd;
    }
    snprintf(
        query_buf, QUERY_STR_LEN,
        "update %s set user_password='%s' where user_name='%s'",
        USER_TABLE, req_info->new_password, req_info->username
	);
    printf("========querybuf=%s\n",query_buf);

    jpf_dbs_do_query_code(app_obj, msg, query_buf, &res_info.code, &affect_num);
    strncpy(res_info.new_password, req_info->new_password, USER_PASSWD_LEN - 1);
err_modify_user_pwd:
    strncpy(res_info.session, req_info->session, SESSION_ID_LEN - 1);
    jpf_dbs_modify_sysmsg_2(msg, &res_info, sizeof(res_info), BUSSLOT_POS_DBS,
                            BUSSLOT_POS_CU);

    return MFR_DELIVER_BACK;
}


static __inline__ gint
jpf_dbs_cu_query_guid(JpfMysqlRes *result, JpfCuQueryGuidRes *res_info)
{
	gint field_num, field_i;
	JpfMysqlRow mysql_row;
	JpfMysqlField* mysql_fields;
	gchar *name;
	gchar *value;

	field_num = jpf_sql_get_num_fields(result);
	while ((mysql_row = jpf_sql_fetch_row(result)))
	{
		jpf_sql_field_seek(result, 0);
		mysql_fields = jpf_sql_fetch_fields(result);

		for (field_i = 0; field_i < field_num; field_i++)
		{
			name = jpf_sql_get_field_name(mysql_fields, field_i);

			if (!strcmp(name, "user_guid"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
					JPF_COPY_VAL(res_info->guid, value,
					MAX_ID_LEN);
			}
			else if (!strcmp(name, "user_guid_domain"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
					JPF_COPY_VAL(res_info->domain, value,
					DOMAIN_ID_LEN);
			}
			else if (!strcmp(name, "gu_name"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
					JPF_COPY_VAL(res_info->gu_name, value,
					GU_NAME_LEN);
			}
			else if (!strcmp(name, "user_guid_level"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
					res_info->level = atoi(value);
			}
			else if (!strcmp(name, "gu_attributes"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
					res_info->gu_attributes = atoi(value);
			}
		}
	}

	return 0;
}


static JpfMsgFunRet
jpf_dbs_cu_query_guid_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	JpfCuQueryGuid *req_info = NULL;
	JpfCuQueryGuidRes res_info;
	gchar query_buf[QUERY_STR_LEN] = {0};
	JpfMysqlRes *mysql_result = NULL;
	gint ret = 0;
	memset(&res_info, 0, sizeof(res_info));

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	snprintf(query_buf, QUERY_STR_LEN,
		"select t1.*,t2.gu_attributes,t2.gu_name from %s as t1, %s as t2 where t1.user_name='%s' and " \
		"t1.user_gu_num=%d and t1.user_guid=t2.gu_id and t1.user_guid_domain=t2.gu_domain",
		USER_GU_NUMBER_TABLE, GU_TABLE, req_info->user, req_info->gu_num
	);

	mysql_result = jpf_dbs_do_query_res(app_obj, query_buf);
	BUG_ON(!mysql_result);
	if (G_UNLIKELY(MYSQL_RESULT_CODE(mysql_result))) {
		ret = MYSQL_RESULT_CODE(mysql_result);
		goto end;
	}

	jpf_dbs_cu_query_guid(mysql_result, &res_info);

end:
	if(mysql_result)
		jpf_sql_put_res(mysql_result, sizeof(JpfMysqlRes));

	SET_CODE(&res_info, ret);
	JPF_COPY_VAL(res_info.session, req_info->session, SESSION_ID_LEN);

	jpf_dbs_modify_sysmsg_2(msg, &res_info, sizeof(res_info), BUSSLOT_POS_DBS,
		BUSSLOT_POS_CU);
	return MFR_DELIVER_BACK;
}


static __inline__ gint
jpf_dbs_cu_query_screen_id(JpfMysqlRes *result, JpfCuQueryScreenIDRes *res_info)
{
	gint field_num, field_i;
	JpfMysqlRow mysql_row;
	JpfMysqlField* mysql_fields;
	gchar *name;
	gchar *value;

	field_num = jpf_sql_get_num_fields(result);
	while ((mysql_row = jpf_sql_fetch_row(result)))
	{
		jpf_sql_field_seek(result, 0);
		mysql_fields = jpf_sql_fetch_fields(result);

		for (field_i = 0; field_i < field_num; field_i++)
		{
			name = jpf_sql_get_field_name(mysql_fields, field_i);

			if (!strcmp(name, "screen_id"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
					res_info->screen_id = atoi(value);
			}
			else if (!strcmp(name, "tw_id"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
					res_info->tw_id = atoi(value);
			}
		}
	}

	return 0;
}


static JpfMsgFunRet
jpf_dbs_cu_query_screen_id_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	JpfCuQueryScreenID *req_info = NULL;
	JpfCuQueryScreenIDRes res_info;
	gchar query_buf[QUERY_STR_LEN] = {0};
	JpfMysqlRes *mysql_result = NULL;
	gint ret = 0;
	memset(&res_info, 0, sizeof(res_info));

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	snprintf(query_buf, QUERY_STR_LEN,
		"select t1.*,t2.tw_id from %s as t1,%s as t2 where t1.user_name='%s' and " \
		"t1.user_screen_num=%d and t1.screen_id=t2.scr_id",
		USER_SCREEN_NUMBER_TABLE, SCREEN_TABLE, req_info->user,
		req_info->screen_num
	);

	mysql_result = jpf_dbs_do_query_res(app_obj, query_buf);
	BUG_ON(!mysql_result);
	if (G_UNLIKELY(MYSQL_RESULT_CODE(mysql_result))) {
		ret = MYSQL_RESULT_CODE(mysql_result);
		goto end;
	}

	jpf_dbs_cu_query_screen_id(mysql_result, &res_info);

end:
	if(mysql_result)
		jpf_sql_put_res(mysql_result, sizeof(JpfMysqlRes));

	SET_CODE(&res_info, ret);
	JPF_COPY_VAL(res_info.session, req_info->session, SESSION_ID_LEN);

	jpf_dbs_modify_sysmsg_2(msg, &res_info, sizeof(res_info), BUSSLOT_POS_DBS,
		BUSSLOT_POS_CU);
	return MFR_DELIVER_BACK;
}


static __inline__ void
jpf_dbs_cu_query_guid_info(JpfMysqlRes *result, JpfCuQueryUserGuidsRes *query_res)
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
		JpfUserGuidInfo *gu = &query_res->guid_info[info_i];

		for(field_i = 0; field_i < field_num; field_i++)
		{
			name = jpf_sql_get_field_name(mysql_fields, field_i);

			if (!strcmp(name, "user_guid"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
					JPF_COPY_VAL(gu->guid, value,
					MAX_ID_LEN);
			}
			else if (!strcmp(name, "user_guid_domain"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
					JPF_COPY_VAL(gu->domain, value,
					DOMAIN_ID_LEN);
			}
			else if (!strcmp(name, "user_guid_level"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if(value)
					gu->level = atoi(value);
			}
			else if (!strcmp(name, "gu_name"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
					JPF_COPY_VAL(gu->gu_name, value,
					GU_NAME_LEN);
			}
			else if (!strcmp(name, "user_gu_num"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if(value)
					gu->gu_num = atoi(value);
			}
		}

		info_i++;
		if (info_i >= query_res->back_count)
			break;
	}
}


static __inline__ JpfCuQueryUserGuidsRes *
jpf_dbs_cu_query_user_guids(JpfMysqlRes *mysql_res, gint *size)
{
	gint row_num, len;
	JpfCuQueryUserGuidsRes *query_res;

	row_num = jpf_sql_get_num_rows(mysql_res);
	if (row_num == 0)
	{
		len = sizeof(JpfCuQueryUserGuidsRes);
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
		len = sizeof(JpfCuQueryUserGuidsRes) + row_num * sizeof(JpfUserGuidInfo);
		query_res = jpf_mem_kalloc(len);
		if (G_UNLIKELY(!query_res))
			return NULL;

		memset(query_res, 0, len);
		query_res->back_count = row_num;
		SET_CODE(query_res, 0);
		jpf_dbs_cu_query_guid_info(mysql_res, query_res);
	}
	*size = len;

	return query_res;
}


static JpfMsgFunRet
jpf_dbs_cu_query_user_guids_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	JpfCuQueryUserGuids *req_info = NULL;
	JpfCuQueryUserGuidsRes *res_info = NULL;
	gchar query_buf[QUERY_STR_LEN] = {0};
	JpfMysqlRes *mysql_result = NULL;
	gint row_num;
	gint total_num;
	gint ret = 0, size;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	snprintf(query_buf, QUERY_STR_LEN,
		"select count(*) as count from %s where user_name='%s'",
		USER_GU_NUMBER_TABLE, req_info->user
	);
	total_num = jpf_get_record_count(app_obj, query_buf);
	if (total_num == 0)
	{
		ret = 0;
		goto end;
	}

	snprintf(query_buf, QUERY_STR_LEN,
		"select t1.*,t2.gu_name from %s as t1,%s as t2 where user_name='%s' " \
		"and t1.user_guid=t2.gu_id order by t1.user_gu_num limit %d,%d",
		USER_GU_NUMBER_TABLE, GU_TABLE, req_info->user,
		req_info->start_num, req_info->req_num
	);

	mysql_result = jpf_dbs_do_query_res(app_obj, query_buf);
	JPF_DBS_CHECK_MYSQL_RESULT(mysql_result, ret, row_num, end);

	res_info = jpf_dbs_cu_query_user_guids(mysql_result, &size);
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
		size = sizeof(JpfCuQueryUserGuidsRes);
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
	JPF_COPY_VAL(res_info->session, req_info->session, SESSION_ID_LEN);

	jpf_dbs_modify_sysmsg(msg, res_info, size, BUSSLOT_POS_DBS,
		BUSSLOT_POS_CU, jpf_mem_kfree);

	return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_dbs_cu_set_user_guids_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	JpfCuSetUserGuids *req_info = NULL;
	JpfUserGuidInfo *guid_info = NULL;
	JpfResult res_info;
	gchar query_buf[QUERY_STR_LEN] = {0};
	glong affect_num = 0;
	gint i;
	memset(&res_info, 0, sizeof(res_info));

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	if (req_info->first_req)
	{
		snprintf(query_buf, QUERY_STR_LEN,
			"delete from %s where user_name='%s'",
			USER_GU_NUMBER_TABLE, req_info->user
		);
		jpf_dbs_do_del_code(app_obj, msg, query_buf, &res_info.code, &affect_num);
		memset(&res_info, 0, sizeof(res_info));
	}

	for (i = 0; i < req_info->count; i++)
	{
		guid_info = &req_info->guid_info[i];

		snprintf(query_buf, QUERY_STR_LEN,
			"insert into %s(user_name,user_gu_num,user_guid,user_guid_domain," \
			"user_guid_level) values('%s',%d,'%s','%s',%d)",
			USER_GU_NUMBER_TABLE, req_info->user, guid_info->gu_num,
			guid_info->guid, guid_info->domain, guid_info->level
		);

		jpf_dbs_do_query_code(app_obj, msg, query_buf, &res_info.code, &affect_num);
		if (RES_CODE(&res_info))
			break;
	}

	JPF_COPY_VAL(res_info.session, req_info->session, SESSION_ID_LEN);

	jpf_dbs_modify_sysmsg_2(msg, &res_info, sizeof(res_info), BUSSLOT_POS_DBS,
		BUSSLOT_POS_CU);

	return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_dbs_cu_set_screen_num_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	JpfCuSetScreenNum *req_info = NULL;
	JpfResult res_info;
	gchar query_buf[QUERY_STR_LEN] = {0};
	glong affect_num = 0;
	gint total_num;
	memset(&res_info, 0, sizeof(res_info));

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	if (req_info->screen_num == 0)
	{
		snprintf(query_buf, QUERY_STR_LEN,
			"delete from %s where user_name='%s' and screen_id=%d",
			USER_SCREEN_NUMBER_TABLE, req_info->user, req_info->screen_id
		);
		jpf_dbs_do_del_code(app_obj, msg, query_buf, &res_info.code, &affect_num);
		memset(&res_info, 0, sizeof(res_info));
		goto end;
	}

	snprintf(query_buf, QUERY_STR_LEN,
		"select count(*) as count from %s where user_name='%s' and user_screen_num=%d",
		USER_SCREEN_NUMBER_TABLE, req_info->user, req_info->screen_num
	);

	total_num = jpf_get_record_count(app_obj, query_buf);
	if (total_num != 0)
	{
		snprintf(query_buf, QUERY_STR_LEN,
			"select count(*) as count from %s where user_name='%s' and " \
			"user_screen_num=%d and screen_id=%d",
			USER_SCREEN_NUMBER_TABLE, req_info->user,
			req_info->screen_num, req_info->screen_id
		);

		total_num = jpf_get_record_count(app_obj, query_buf);
		if (total_num == 0)
			SET_CODE(&res_info, EEXIST);
		goto end;
	}

	snprintf(query_buf, QUERY_STR_LEN,
		"select count(*) as count from %s where user_name='%s' and screen_id=%d",
		USER_SCREEN_NUMBER_TABLE, req_info->user, req_info->screen_id
	);
	total_num = jpf_get_record_count(app_obj, query_buf);
	if (total_num == 0)
		snprintf(query_buf, QUERY_STR_LEN,
			"insert into %s(user_name,user_screen_num,screen_id) values('%s',%d,%d)",
			USER_SCREEN_NUMBER_TABLE, req_info->user, req_info->screen_num,
			req_info->screen_id
		);
	else
		snprintf(query_buf, QUERY_STR_LEN,
			"update %s set user_screen_num=%d where user_name='%s' and screen_id=%d",
			USER_SCREEN_NUMBER_TABLE, req_info->screen_num, req_info->user,
			req_info->screen_id
		);

	jpf_dbs_do_query_code(app_obj, msg, query_buf, &res_info.code, &affect_num);

end:
	JPF_COPY_VAL(res_info.session, req_info->session, SESSION_ID_LEN);

	jpf_dbs_modify_sysmsg_2(msg, &res_info, sizeof(res_info), BUSSLOT_POS_DBS,
		BUSSLOT_POS_CU);

	return MFR_DELIVER_BACK;
}


static __inline__ gint
jpf_dbs_cu_query_tour_id(JpfMysqlRes *result, JpfCuQueryTourIDRes *res_info)
{
	gint field_num, field_i;
	JpfMysqlRow mysql_row;
	JpfMysqlField* mysql_fields;
	gchar *name;
	gchar *value;

	field_num = jpf_sql_get_num_fields(result);
	while ((mysql_row = jpf_sql_fetch_row(result)))
	{
		jpf_sql_field_seek(result, 0);
		mysql_fields = jpf_sql_fetch_fields(result);

		for (field_i = 0; field_i < field_num; field_i++)
		{
			name = jpf_sql_get_field_name(mysql_fields, field_i);

			if (!strcmp(name, "tour_id"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
					res_info->tour_id = atoi(value);
			}
		}
	}

	return 0;
}


static JpfMsgFunRet
jpf_dbs_cu_query_tour_id_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	JpfCuQueryTourID *req_info = NULL;
	JpfCuQueryTourIDRes res_info;
	gchar query_buf[QUERY_STR_LEN] = {0};
	JpfMysqlRes *mysql_result = NULL;
	gint ret = 0;
	memset(&res_info, 0, sizeof(res_info));

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	snprintf(query_buf, QUERY_STR_LEN,
		"select * from %s where user_name='%s' and user_tour_num=%d",
		USER_TOUR_NUMBER_TABLE, req_info->user, req_info->tour_num
	);

	mysql_result = jpf_dbs_do_query_res(app_obj, query_buf);
	BUG_ON(!mysql_result);
	if (G_UNLIKELY(MYSQL_RESULT_CODE(mysql_result))) {
		ret = MYSQL_RESULT_CODE(mysql_result);
		goto end;
	}

	jpf_dbs_cu_query_tour_id(mysql_result, &res_info);

end:
	if(mysql_result)
		jpf_sql_put_res(mysql_result, sizeof(JpfMysqlRes));

	SET_CODE(&res_info, ret);
	JPF_COPY_VAL(res_info.session, req_info->session, SESSION_ID_LEN);

	jpf_dbs_modify_sysmsg_2(msg, &res_info, sizeof(res_info), BUSSLOT_POS_DBS,
		BUSSLOT_POS_CU);
	return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_dbs_cu_set_tour_num_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	JpfCuSetTourNum *req_info = NULL;
	JpfResult res_info;
	gchar query_buf[QUERY_STR_LEN] = {0};
	glong affect_num = 0;
	gint total_num;
	memset(&res_info, 0, sizeof(res_info));

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	if (req_info->tour_num == 0)
	{
		snprintf(query_buf, QUERY_STR_LEN,
			"delete from %s where user_name='%s' and tour_id=%d",
			USER_TOUR_NUMBER_TABLE, req_info->user, req_info->tour_id
		);
		jpf_dbs_do_del_code(app_obj, msg, query_buf, &res_info.code, &affect_num);
		memset(&res_info, 0, sizeof(res_info));
		goto end;
	}

	snprintf(query_buf, QUERY_STR_LEN,
		"select count(*) as count from %s where user_name='%s' and user_tour_num=%d",
		USER_TOUR_NUMBER_TABLE, req_info->user, req_info->tour_num
	);

	total_num = jpf_get_record_count(app_obj, query_buf);
	if (total_num != 0)
	{
		snprintf(query_buf, QUERY_STR_LEN,
			"select count(*) as count from %s where user_name='%s' and " \
			"user_tour_num=%d and tour_id=%d",
			USER_TOUR_NUMBER_TABLE, req_info->user,
			req_info->tour_num, req_info->tour_id
		);

		total_num = jpf_get_record_count(app_obj, query_buf);
		if (total_num == 0)
			SET_CODE(&res_info, EEXIST);
		goto end;
	}

	snprintf(query_buf, QUERY_STR_LEN,
		"select count(*) as count from %s where user_name='%s' and tour_id=%d",
		USER_TOUR_NUMBER_TABLE, req_info->user, req_info->tour_id
	);
	total_num = jpf_get_record_count(app_obj, query_buf);
	if (total_num == 0)
		snprintf(query_buf, QUERY_STR_LEN,
			"insert into %s(user_name,user_tour_num,tour_id) values('%s',%d,%d)",
			USER_TOUR_NUMBER_TABLE, req_info->user, req_info->tour_num,
			req_info->tour_id
		);
	else
		snprintf(query_buf, QUERY_STR_LEN,
			"update %s set user_tour_num=%d where user_name='%s' and tour_id=%d",
			USER_TOUR_NUMBER_TABLE, req_info->tour_num, req_info->user,
			req_info->tour_id
		);

	jpf_dbs_do_query_code(app_obj, msg, query_buf, &res_info.code, &affect_num);

end:
	JPF_COPY_VAL(res_info.session, req_info->session, SESSION_ID_LEN);

	jpf_dbs_modify_sysmsg_2(msg, &res_info, sizeof(res_info), BUSSLOT_POS_DBS,
		BUSSLOT_POS_CU);

	return MFR_DELIVER_BACK;
}


static __inline__ gint
jpf_dbs_cu_query_group_id(JpfMysqlRes *result, JpfCuQueryGroupIDRes *res_info)
{
	gint field_num, field_i;
	JpfMysqlRow mysql_row;
	JpfMysqlField* mysql_fields;
	gchar *name;
	gchar *value;

	field_num = jpf_sql_get_num_fields(result);
	while ((mysql_row = jpf_sql_fetch_row(result)))
	{
		jpf_sql_field_seek(result, 0);
		mysql_fields = jpf_sql_fetch_fields(result);

		for (field_i = 0; field_i < field_num; field_i++)
		{
			name = jpf_sql_get_field_name(mysql_fields, field_i);

			if (!strcmp(name, "group_id"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
					res_info->group_id = atoi(value);
			}
		}
	}

	return 0;
}


static JpfMsgFunRet
jpf_dbs_cu_query_group_id_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	JpfCuQueryGroupID *req_info = NULL;
	JpfCuQueryGroupIDRes res_info;
	gchar query_buf[QUERY_STR_LEN] = {0};
	JpfMysqlRes *mysql_result = NULL;
	gint ret = 0;
	memset(&res_info, 0, sizeof(res_info));

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	snprintf(query_buf, QUERY_STR_LEN,
		"select * from %s where user_name='%s' and user_group_num=%d",
		USER_GROUP_NUMBER_TABLE, req_info->user, req_info->group_num
	);

	mysql_result = jpf_dbs_do_query_res(app_obj, query_buf);
	BUG_ON(!mysql_result);
	if (G_UNLIKELY(MYSQL_RESULT_CODE(mysql_result))) {
		ret = MYSQL_RESULT_CODE(mysql_result);
		goto end;
	}

	jpf_dbs_cu_query_group_id(mysql_result, &res_info);

end:
	if(mysql_result)
		jpf_sql_put_res(mysql_result, sizeof(JpfMysqlRes));

	SET_CODE(&res_info, ret);
	JPF_COPY_VAL(res_info.session, req_info->session, SESSION_ID_LEN);

	jpf_dbs_modify_sysmsg_2(msg, &res_info, sizeof(res_info), BUSSLOT_POS_DBS,
		BUSSLOT_POS_CU);
	return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_dbs_cu_set_group_num_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	JpfCuSetGroupNum *req_info = NULL;
	JpfResult res_info;
	gchar query_buf[QUERY_STR_LEN] = {0};
	glong affect_num = 0;
	gint total_num;
	memset(&res_info, 0, sizeof(res_info));

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	if (req_info->group_num == 0)
	{
		snprintf(query_buf, QUERY_STR_LEN,
			"delete from %s where user_name='%s' and group_id=%d",
			USER_GROUP_NUMBER_TABLE, req_info->user, req_info->group_id
		);
		jpf_dbs_do_del_code(app_obj, msg, query_buf, &res_info.code, &affect_num);
		memset(&res_info, 0, sizeof(res_info));
		goto end;
	}

	snprintf(query_buf, QUERY_STR_LEN,
		"select count(*) as count from %s where user_name='%s' and user_group_num=%d",
		USER_GROUP_NUMBER_TABLE, req_info->user, req_info->group_num
	);

	total_num = jpf_get_record_count(app_obj, query_buf);
	if (total_num != 0)
	{
		snprintf(query_buf, QUERY_STR_LEN,
			"select count(*) as count from %s where user_name='%s' and " \
			"user_group_num=%d and group_id=%d",
			USER_GROUP_NUMBER_TABLE, req_info->user,
			req_info->group_num, req_info->group_id
		);

		total_num = jpf_get_record_count(app_obj, query_buf);
		if (total_num == 0)
			SET_CODE(&res_info, EEXIST);
		goto end;
	}

	snprintf(query_buf, QUERY_STR_LEN,
		"select count(*) as count from %s where user_name='%s' and group_id=%d",
		USER_GROUP_NUMBER_TABLE, req_info->user, req_info->group_id
	);
	total_num = jpf_get_record_count(app_obj, query_buf);
	if (total_num == 0)
		snprintf(query_buf, QUERY_STR_LEN,
			"insert into %s(user_name,user_group_num,group_id) values('%s',%d,%d)",
			USER_GROUP_NUMBER_TABLE, req_info->user, req_info->group_num,
			req_info->group_id
		);
	else
		snprintf(query_buf, QUERY_STR_LEN,
			"update %s set user_group_num=%d where user_name='%s' and group_id=%d",
			USER_GROUP_NUMBER_TABLE, req_info->group_num, req_info->user,
			req_info->group_id
		);

	jpf_dbs_do_query_code(app_obj, msg, query_buf, &res_info.code, &affect_num);

end:
	JPF_COPY_VAL(res_info.session, req_info->session, SESSION_ID_LEN);

	jpf_dbs_modify_sysmsg_2(msg, &res_info, sizeof(res_info), BUSSLOT_POS_DBS,
		BUSSLOT_POS_CU);

	return MFR_DELIVER_BACK;
}



void
jpf_mod_dbs_register_cu_msg_handler(JpfModDbs *self)
{
    JpfAppMod *super_self = (JpfAppMod*)self;

    jpf_app_mod_register_msg(
        super_self,
        MSG_GET_USER_INFO,
        NULL,
        jpf_dbs_get_user_info_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MSG_GET_USER_GROUP_INFO,
        NULL,
        jpf_dbs_get_user_group_info_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MSG_GET_USER_LOGIN_INFO,
        NULL,
        jpf_dbs_get_user_login_info_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_GET_ALL_AREA,
        NULL,
        jpf_dbs_get_all_area_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_GET_AREA_INFO,
        NULL,
        jpf_dbs_get_area_info_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_GET_DEVICE_LIST,
        NULL,
        jpf_dbs_get_device_list_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_GET_AREA_DEVICE_INFO,
        NULL,
        jpf_dbs_get_area_device_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_GET_MEDIA_URL,
        NULL,
        jpf_dbs_get_media_url_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_GET_ALARM,
        NULL,
        jpf_dbs_get_alarm_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_GET_ALARM_STATE,
        NULL,
        jpf_dbs_get_alarm_state_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEAL_ALARM,
        NULL,
        jpf_dbs_deal_alarm_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_GET_GU_MSS,
        NULL,
        jpf_dbs_get_gu_mss_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_GET_DEFENCE_AREA,
        NULL,
        jpf_dbs_get_defence_area_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_GET_DEFENCE_MAP,
        NULL,
        jpf_dbs_get_defence_map_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_GET_DEFENCE_GU,
        NULL,
        jpf_dbs_get_defence_gu_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_GET_MAP_HREF,
        NULL,
        jpf_dbs_get_map_href_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_GET_GU_MAP_LOCATION,
        NULL,
        jpf_dbs_get_gu_map_location_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_GET_TW,
        NULL,
        jpf_dbs_get_tw_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_GET_SCREEN,
        NULL,
        jpf_dbs_get_screen_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_GET_SCR_DIV,
        NULL,
        jpf_dbs_get_scr_div_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_GET_SCR_STATE,
        NULL,
        jpf_dbs_get_scr_state_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_GET_TOUR,
        NULL,
        jpf_dbs_get_tour_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_GET_TOUR_STEP,
        NULL,
        jpf_dbs_get_tour_step_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_GET_GROUP,
        NULL,
        jpf_dbs_get_group_b,
        0
    );

	jpf_app_mod_register_msg(
        super_self,
        MESSAGE_CU_MODIFY_USER_PWD,
        NULL,
        jpf_dbs_cu_modify_user_pwd_b,
        0
    );

	jpf_app_mod_register_msg(
		super_self,
		MESSAGE_QUERY_GUID,
		NULL,
		jpf_dbs_cu_query_guid_b,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MESSAGE_QUERY_SCREEN_ID,
		NULL,
		jpf_dbs_cu_query_screen_id_b,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MESSAGE_QUERY_USER_GUIDS,
		NULL,
		jpf_dbs_cu_query_user_guids_b,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MESSAGE_SET_USER_GUIDS,
		NULL,
		jpf_dbs_cu_set_user_guids_b,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MESSAGE_SET_SCREEN_NUM,
		NULL,
		jpf_dbs_cu_set_screen_num_b,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MESSAGE_QUERY_TOUR_ID,
		NULL,
		jpf_dbs_cu_query_tour_id_b,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MESSAGE_SET_TOUR_NUM,
		NULL,
		jpf_dbs_cu_set_tour_num_b,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MESSAGE_QUERY_GROUP_ID,
		NULL,
		jpf_dbs_cu_query_group_id_b,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MESSAGE_SET_GROUP_NUM,
		NULL,
		jpf_dbs_cu_set_group_num_b,
		0
	);
}


//:~ End
