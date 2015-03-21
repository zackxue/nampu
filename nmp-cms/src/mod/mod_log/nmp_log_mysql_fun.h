#ifndef __NMP_LOG_MYSQLFUN_H__
#define __NMP_LOG_MYSQLFUN_H__

#include <stdint.h>
#include <mysql.h>
#include "nmp_mods.h"
#include "nmp_mod_log.h"
#include "message/nmp_msg_share.h"
#include "nmp_mysql_defs.h"


void
nmp_log_dbs_do_query_code(NmpAppObj *app_obj,
                      NmpSysMsg *sys_msg,
                      char *query,
                      NmpMsgErrCode *result,
                      glong *affect);

void
nmp_log_dbs_do_del_code(NmpAppObj *app_obj,
                      NmpSysMsg *sys_msg,
                      char *query,
                      NmpMsgErrCode *result,
                      glong *affect);

gint
nmp_log_get_record_count(NmpAppObj *app_obj, char *query);

NmpMysqlRes*
nmp_log_dbs_do_query_res(NmpAppObj *app_obj, char *query);

NmpMysqlFieldOffset
nmp_log_sql_field_seek(NmpMysqlRes *res, NmpMysqlFieldOffset offset);

NmpMysqlField*
nmp_log_sql_fetch_fields(NmpMysqlRes *res);

char*
nmp_log_sql_get_field_name(NmpMysqlField *field, int num);

char *
nmp_log_sql_get_field_value(NmpMysqlRow row,int num);

unsigned long long
nmp_log_sql_get_num_rows(NmpMysqlRes *res);

void
nmp_log_sql_put_res(NmpMysqlRes *res, guint size);

unsigned long long
nmp_log_sql_get_num_rows(NmpMysqlRes *res);

unsigned int
nmp_log_sql_get_num_fields(NmpMysqlRes *res);

NmpMysqlRow
nmp_log_sql_fetch_row(NmpMysqlRes *res);


#endif //__NMP_LOG_MYSQLFUN_H__
