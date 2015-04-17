/**********************************************************
 * @file     nmp_mysql_fun.c
 * @author   Yang Ying
 * @section  LICENSE
 *
 * Copyright by Shenzhen NMP Electronic Co.Ltd, 2011.
 * Website: www.sznmp.com
 *
 * @section  DESCRIPTION
 *
 * 1. nmp_mysql_do_query
 *    deal with query operation ,for example select,insert,delete
 * 2. nmp_sql_get_num_rows
 *    get number of rows
 * 3. nmp_sql_get_num_fields
 *    get number of fields
 * 4. nmp_sql_get_res
 *    get results of query
 * 5. nmp_sql_fetch_row
 *    fetch a row of mysql
 * 6. nmp_sql_fetch_field
 *    fetch a field of mysql
 * 7. nmp_sql_fetch_fields
 *    fetch fields of mysql
 * 8. nmp_sql_get_field_name
 *    get field name
 * 9. nmp_sql_get_field_type
 *    get field type
 * 10. nmp_sql_get_field_value
 *    fetch field value
 * 11. nmp_sql_fetch_lengths
 *    fetch field lengths
 * 12. nmp_sql_field_seek
 *    set field seek
 * 13. nmp_sql_put_res
 *    free result of mysql
 * History:
 * 2011.6.23 - Yang Ying, initiate to create;
 *
***********************************************************/

#include <stdint.h>
#include <stdio.h>
#include <mysql.h>
#include <assert.h>
#include <errno.h>
#include <mysqld_error.h>
#include <errmsg.h>
#include "nmp_share_debug.h"
#include "nmp_mysql_fun.h"
#include "db_connect_pool.h"
#include "nmp_share_errno.h"
#include "message/nmp_msg_share.h"
#include "nmp_mods.h"
#include "nmp_mod_dbs.h"
#include "nmp_memory.h"
#include "nmp_dbs_fun.h"

#define ASSERT assert
//执行指定为“以Null终结的字符串”的SQL查询。
#define DB_SERVER_GONE_ERROR 2006

int
nmp_mysql_do_query(NmpMysql *db, const char *query)
{
	ASSERT(db != NULL && query != NULL);
	int ret,error_code;

	ret = mysql_query(db, query);
	if (ret)  //mysql_query()成功返回0，失败返回非0
	{
		error_code =  mysql_errno(db);
		nmp_warning(
			"<NmpModDbs> exec SQL command \"%s\" failed, err:%d.",
			query, error_code
		);

		return -error_code;
	}

	return 0; // query succeeded, process any data returned by it
}


void
nmp_sql_clean_left_result(NmpMysql *db)         // 清除剩余的结果集
{
	ASSERT(db != NULL);
	MYSQL_RES *result = NULL;

	do
	{
        result = mysql_store_result(db);
        if (result)  // there are rows
        {
            mysql_free_result(result);
        }
	}while((!mysql_next_result(db)));
}


NmpMysqlRes*
nmp_process_query_res(NmpMysql *mysql, const char *query_buf)
{
    int ret;
    gint num;
    NmpMysqlRes     *mysql_result;
    G_ASSERT(mysql != NULL && query_buf != NULL);

    ret = nmp_mysql_do_query(mysql, query_buf);
    if (ret)  // query error
    {
   		printf("----query: %s\n",query_buf);
        mysql_result = (NmpMysqlRes*)mysql_malloc(sizeof(NmpMysqlRes));
        if (G_UNLIKELY(!mysql_result))
        	return NULL;

        mysql_result->result_code = ret;
        mysql_result->sql_res = NULL;
        goto query_end;
    }

    mysql_result = nmp_sql_get_res(mysql);
    if ((MYSQL_RESULT_CODE(mysql_result) == 0)&&(MYSQL_RESULT(mysql_result)))
       num = nmp_sql_get_num_rows(mysql_result);
    else
	num = 0;
    printf("----------num=%d\n",num);
query_end:
	nmp_sql_clean_left_result(mysql);
    return mysql_result;
}


int
nmp_process_query_procedure(NmpMysql *mysql, const char *query_buf)
{
    int ret;
    G_ASSERT(mysql != NULL && query_buf != NULL);

    ret = nmp_mysql_do_query(mysql, query_buf);
    nmp_sql_clean_left_result(mysql);

    return ret;
}


/* 处理操作结果码，不处理结果集 */
void
nmp_dbs_do_query_code(NmpAppObj *app_obj,
                      NmpSysMsg *sys_msg,
                      char *query,
                      NmpMsgErrCode *result,
                      glong *affect)
{
   // G_ASSERT(sys_msg != NULL && result != NULL);

    db_conn_status *conn = NULL;
    NmpModDbs *dbs_obj;
    gint size;
    gint code;
    glong affect_num = 0;

    code = RES_CODE(result);
    if (G_UNLIKELY(code))
        goto END_DBS_QUERY;

    dbs_obj = NMP_MODDBS(app_obj);
redo:
    conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
    if (G_UNLIKELY(!conn))
    {
        nmp_warning("<NmpModDbs> get db connection error,query=%s.",query);
        code = -E_GETDBCONN;
        goto END_DBS_QUERY;
    }

    if (G_UNLIKELY(!conn->mysql))
    {
    	 nmp_warning("<NmpModDbs> get db connection error,query=%s.",query);
        put_db_connection(dbs_obj->pool_info, conn);
        code = -E_GETDBCONN;
        goto END_DBS_QUERY;
    }

    code = nmp_mysql_do_query(conn->mysql, query);
    if (code == -DB_SERVER_GONE_ERROR)
    {
        kill_db_connection(dbs_obj->pool_info, conn);
        goto redo;
    }

    affect_num =  (glong)mysql_affected_rows(conn->mysql);
    put_db_connection(dbs_obj->pool_info, conn);

END_DBS_QUERY:
    if (affect)
        *affect = affect_num;
    size = sizeof(NmpMsgErrCode);
    SET_CODE(result, code);
}


void
nmp_dbs_do_del_code(NmpAppObj *app_obj,
                      NmpSysMsg *sys_msg,
                      char *query,
                      NmpMsgErrCode *result,
                      glong *affect)
{
    nmp_dbs_do_query_code(app_obj, sys_msg, query, result, affect);
    if (*affect == 0)
        SET_CODE(result, -E_NODBENT);
}


/* 处理结果集    */
NmpMysqlRes*
nmp_dbs_do_query_res(NmpAppObj *app_obj, char *query)
{
    db_conn_status  *conn = NULL;
    NmpModDbs        *dbs_obj;
    NmpMysqlRes      *result = NULL;

    dbs_obj = NMP_MODDBS(app_obj);
redo:
    conn = get_db_connection(dbs_obj->pool_info, dbs_obj->pool_conf);
    if (G_UNLIKELY(!conn))
        goto END_QUERY_RESULT;

    if (G_UNLIKELY(!conn->mysql))
    {
        put_db_connection(dbs_obj->pool_info, conn);
        goto END_QUERY_RESULT;
    }

    result = nmp_process_query_res(conn->mysql, query);
    if (result->result_code == -DB_SERVER_GONE_ERROR)
    {
        kill_db_connection(dbs_obj->pool_info, conn);
        goto redo;
    }

    put_db_connection(dbs_obj->pool_info, conn);

END_QUERY_RESULT:
    if (G_UNLIKELY(!result))
    {
        nmp_warning("<NmpModDbs> get db connection error,query=%s.",query);
        result = mysql_malloc(sizeof(NmpMysqlRes));
        if (G_UNLIKELY(!result))
            return NULL;
        result->result_code = -E_GETDBCONN;
        result->sql_res = NULL;
    }

    return result;
}

gint
nmp_get_count_value(NmpMysqlRes *result)
{
    unsigned int row_num;
    unsigned int field_num;
    NmpMysqlRow mysql_row;
    NmpMysqlField* mysql_fields;
    gchar *name;
    gchar *value;
    gint count = 0;
    gint field_no =0;

    row_num = nmp_sql_get_num_rows(result);
    field_num = nmp_sql_get_num_fields(result);

    while ((mysql_row = nmp_sql_fetch_row(result)))
    {
        nmp_sql_field_seek(result, 0);
        mysql_fields = nmp_sql_fetch_fields(result);
        for(field_no = 0; field_no < field_num; field_no++)
        {
            name = nmp_sql_get_field_name(mysql_fields, field_no);
            if (!strcmp(name, "@count"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    count = atoi(value);
		      return   count;
                }
            }
	     else if (!strcmp(name, "count"))
            {
                value = nmp_sql_get_field_value(mysql_row, field_no);
                if(value)
                {
                    count = atoi(value);
                    printf("--------------==========count: %d\n",count);
                    return   count;
                }
            }
            else
                printf("no need mysql name %s ", name);
        }
    }

    return count;
}

gint
nmp_get_record_count(NmpAppObj *app_obj, char *query)
{
	db_conn_status *conn = NULL;
	NmpModDbs *dbs_obj;
	NmpMysqlRes *result = NULL;
	gint count = 0;

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

	result = nmp_process_query_res(conn->mysql, query);
	if (result->result_code == -DB_SERVER_GONE_ERROR)
	{
		kill_db_connection(dbs_obj->pool_info, conn);
		goto redo;
	}

	put_db_connection(dbs_obj->pool_info, conn);
	if (result && result->sql_res)
	{
		count = nmp_get_count_value(result);
	}

	nmp_sql_put_res(result, sizeof(NmpMysqlRes));
	return count;
}


unsigned long long
nmp_sql_get_num_rows(NmpMysqlRes *res)   //获取结果集中的行数
{
	ASSERT(res != NULL && MYSQL_RESULT(res));

    if (!MYSQL_RESULT(res))
        return 0;
    return mysql_num_rows(res->sql_res);
}


unsigned int
nmp_sql_get_num_fields(NmpMysqlRes *res) //获取结果集中的列数
{
	ASSERT(res != NULL && MYSQL_RESULT(res));

    if (!MYSQL_RESULT(res))
        return 0;
    return mysql_num_fields(res->sql_res);
}


NmpMysqlRes*
nmp_sql_get_res(NmpMysql *db)         // 获取结果集
{
	ASSERT(db != NULL);
	MYSQL_RES *result = NULL;
	NmpMysqlRes *mysql_res;

	mysql_res = mysql_malloc(sizeof(NmpMysqlRes));
	if (G_UNLIKELY(!mysql_res))
		return NULL;

	result = mysql_store_result(db);
	if (result)  // there are rows
	{
		mysql_res->result_code = 0;
		//mysql_res->is_select = 1;
		mysql_res->sql_res = result;
	}
	else
	{
		if (mysql_errno(db))
		{
			db_fprintf(stderr, "Error: %s\n", mysql_error(db));
		  	mysql_res->result_code = -1;
    		mysql_res->sql_res = NULL;
		}
		else if (mysql_field_count(db) == 0)
		{
		  // query does not return data
		  // (it was not a SELECT)
		  	mysql_res->result_code = 0;
    		mysql_res->sql_res = NULL;
		}
		else
		{
			db_printf("no this result\n");
		}
	}

	return mysql_res;
}


gint nmp_dbs_get_row_num(NmpAppObj *app_obj,
                      NmpSysMsg *sys_msg,
                      char *query)
{
	 NmpMysqlRes *result;
	 gint row_num;
        gint ret;

	 result = nmp_dbs_do_query_res(app_obj, query);

    if (G_UNLIKELY(!result))
   	{
		nmp_error("<NmpModDbs> alloc error");
		ret = -ENOMEM;
		return ret;
	}

    if (G_UNLIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
    {
        row_num = nmp_sql_get_num_rows(result);
        printf("get row num (%d)\n",row_num);
        if(row_num == 0)
        {
        	nmp_sql_put_res(result,sizeof(NmpMysqlRes));
        	return row_num;
        }
    }
    else
    {
    	ret = MYSQL_RESULT_CODE(result);
    	nmp_sql_put_res(result,sizeof(NmpMysqlRes));
    	return ret;
    }
    nmp_sql_put_res(result,sizeof(NmpMysqlRes));
    return row_num;
}


NmpMysqlRow
nmp_sql_fetch_row(NmpMysqlRes *res)
{
	ASSERT(res != NULL);
	if(res->sql_res)
		return mysql_fetch_row(res->sql_res);
	return NULL;
}


NmpMysqlField*
nmp_sql_fetch_field(NmpMysqlRes *res) //返回下一个表字段的类型。
{
	ASSERT(res != NULL);

	if(res->sql_res)
		return mysql_fetch_field(res->sql_res);
	return NULL;
}


NmpMysqlField*
nmp_sql_fetch_fields(NmpMysqlRes *res)  //返回所有字段结构的数组。
{
	ASSERT(res != NULL);

	if(res->sql_res)
	return mysql_fetch_fields(res->sql_res);

	return NULL;
}


char*
nmp_sql_get_field_name(NmpMysqlField *field, int num) //返回下一个表字段的名字。
{
	ASSERT(field != NULL);

	if(field)
		return (field[num]).name;
	return NULL;
}


int32_t
nmp_sql_get_field_type(NmpMysqlField *field, int num) //返回下一个表字段的类型。
{
	ASSERT(field != NULL);

	if (field)
		return (field[num]).type;

	return -E_NOFIELD;
}


char *
nmp_sql_get_field_value(NmpMysqlRow row,int num) //返回下一个表字段的值
{
	ASSERT(row != NULL);

	if(row)
		return row[num];
	return NULL;
}


unsigned long *
nmp_sql_fetch_lengths(NmpMysqlRes *res)  // 返回当前行中所有列的长度
{
	ASSERT(res != NULL);
	if (res->sql_res)
		return mysql_fetch_lengths(res->sql_res);
	return NULL;
}


NmpMysqlFieldOffset
nmp_sql_field_seek(NmpMysqlRes *res, NmpMysqlFieldOffset offset)
{
	ASSERT(res != NULL);
	if (res->sql_res)
		return mysql_field_seek(res->sql_res, offset);
	return -E_NOFIELD;
}

/*
void
nmp_sql_put_res(NmpMysqlRes *res)   // 释放结果集使用的内存
{
	ASSERT(res != NULL);

	if(res->sql_res)
	{printf("mysql_free_result\n");
		mysql_free_result(res->sql_res);
	}
	if(res)
		mysql_free(res);
}*/

void
nmp_sql_put_res(NmpMysqlRes *res, guint size)   // 释放结果集使用的内存
{
	ASSERT(res != NULL);

	if(res->sql_res)
		mysql_free_result(res->sql_res);

	if(res)
		nmp_mem_kfree(res,size);
}

