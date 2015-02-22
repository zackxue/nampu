#ifndef __NMP_MYSQLFUN_H__
#define __NMP_MYSQLFUN_H__

#include <stdint.h>
#include <mysql.h>
#include "nmp_mods.h"
#include "nmp_mod_dbs.h"
#include "message/nmp_msg_share.h"
#include "nmp_mysql_defs.h"


int
jpf_mysql_do_query(JpfMysql *db, const char *query);


JpfMysqlRes*
jpf_process_query(JpfMysql *mysql, const char *query_buf);

void
jpf_dbs_do_query_code(JpfAppObj *app_obj,
                      JpfSysMsg *sys_msg,
                      char *query,
                      JpfMsgErrCode *result,
                      glong *affect);

void
jpf_dbs_do_del_code(JpfAppObj *app_obj,
                      JpfSysMsg *sys_msg,
                      char *query,
                      JpfMsgErrCode *result,
                      glong *affect);

JpfMysqlRes*
jpf_dbs_do_query_res(JpfAppObj *app_obj, char *query);

JpfMysqlRes*
jpf_process_query_res(JpfMysql *mysql, const char *query_buf);

int
jpf_process_query_procedure(JpfMysql *mysql, const char *query_buf);

gint
jpf_get_record_count(JpfAppObj *app_obj, char *query);

gint jpf_get_count_value(JpfMysqlRes *result);

gint jpf_dbs_get_row_num(JpfAppObj *app_obj,
                      JpfSysMsg *sys_msg,
                      char *query);
unsigned long long
jpf_sql_get_num_rows(JpfMysqlRes *res);

unsigned int
jpf_sql_get_num_fields(JpfMysqlRes *res);

JpfMysqlRes*
jpf_sql_get_res(JpfMysql *db);

JpfMysqlRow
jpf_sql_fetch_row(JpfMysqlRes *res);

JpfMysqlField*
jpf_sql_fetch_field(JpfMysqlRes *res);

JpfMysqlField*
jpf_sql_fetch_fields(JpfMysqlRes *res) ;

char*
jpf_sql_get_field_name(JpfMysqlField *field, int num);

int32_t
jpf_sql_get_field_type(JpfMysqlField *field, int num);

char *
jpf_sql_get_field_value(JpfMysqlRow row,int num);

JpfMysqlFieldOffset
jpf_sql_field_seek(JpfMysqlRes *res, JpfMysqlFieldOffset offset);

void
jpf_sql_put_res(JpfMysqlRes *res, guint size);

#endif //__NMP_MYSQLFUN_H__
