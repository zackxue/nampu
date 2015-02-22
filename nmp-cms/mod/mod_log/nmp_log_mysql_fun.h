#ifndef __NMP_LOG_MYSQLFUN_H__
#define __NMP_LOG_MYSQLFUN_H__

#include <stdint.h>
#include <mysql.h>
#include "nmp_mods.h"
#include "nmp_mod_log.h"
#include "message/nmp_msg_share.h"
#include "nmp_mysql_defs.h"


void
jpf_log_dbs_do_query_code(JpfAppObj *app_obj,
                      JpfSysMsg *sys_msg,
                      char *query,
                      JpfMsgErrCode *result,
                      glong *affect);

void
jpf_log_dbs_do_del_code(JpfAppObj *app_obj,
                      JpfSysMsg *sys_msg,
                      char *query,
                      JpfMsgErrCode *result,
                      glong *affect);

gint
jpf_log_get_record_count(JpfAppObj *app_obj, char *query);

JpfMysqlRes*
jpf_log_dbs_do_query_res(JpfAppObj *app_obj, char *query);

JpfMysqlFieldOffset
jpf_log_sql_field_seek(JpfMysqlRes *res, JpfMysqlFieldOffset offset);

JpfMysqlField*
jpf_log_sql_fetch_fields(JpfMysqlRes *res);

char*
jpf_log_sql_get_field_name(JpfMysqlField *field, int num);

char *
jpf_log_sql_get_field_value(JpfMysqlRow row,int num);

unsigned long long
jpf_log_sql_get_num_rows(JpfMysqlRes *res);

void
jpf_log_sql_put_res(JpfMysqlRes *res, guint size);

unsigned long long
jpf_log_sql_get_num_rows(JpfMysqlRes *res);

unsigned int
jpf_log_sql_get_num_fields(JpfMysqlRes *res);

JpfMysqlRow
jpf_log_sql_fetch_row(JpfMysqlRes *res);


#endif //__NMP_LOG_MYSQLFUN_H__
