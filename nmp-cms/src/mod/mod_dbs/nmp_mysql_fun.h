#ifndef __NMP_MYSQLFUN_H__
#define __NMP_MYSQLFUN_H__

#include <stdint.h>
#include <mysql.h>
#include "nmp_mods.h"
#include "nmp_mod_dbs.h"
#include "message/nmp_msg_share.h"
#include "nmp_mysql_defs.h"


int
nmp_mysql_do_query(NmpMysql *db, const char *query);


NmpMysqlRes*
nmp_process_query(NmpMysql *mysql, const char *query_buf);

void
nmp_dbs_do_query_code(NmpAppObj *app_obj,
                      NmpSysMsg *sys_msg,
                      char *query,
                      NmpMsgErrCode *result,
                      glong *affect);

void
nmp_dbs_do_del_code(NmpAppObj *app_obj,
                      NmpSysMsg *sys_msg,
                      char *query,
                      NmpMsgErrCode *result,
                      glong *affect);

NmpMysqlRes*
nmp_dbs_do_query_res(NmpAppObj *app_obj, char *query);

NmpMysqlRes*
nmp_process_query_res(NmpMysql *mysql, const char *query_buf);

int
nmp_process_query_procedure(NmpMysql *mysql, const char *query_buf);

gint
nmp_get_record_count(NmpAppObj *app_obj, char *query);

gint nmp_get_count_value(NmpMysqlRes *result);

gint nmp_dbs_get_row_num(NmpAppObj *app_obj,
                      NmpSysMsg *sys_msg,
                      char *query);
unsigned long long
nmp_sql_get_num_rows(NmpMysqlRes *res);

unsigned int
nmp_sql_get_num_fields(NmpMysqlRes *res);

NmpMysqlRes*
nmp_sql_get_res(NmpMysql *db);

NmpMysqlRow
nmp_sql_fetch_row(NmpMysqlRes *res);

NmpMysqlField*
nmp_sql_fetch_field(NmpMysqlRes *res);

NmpMysqlField*
nmp_sql_fetch_fields(NmpMysqlRes *res) ;

char*
nmp_sql_get_field_name(NmpMysqlField *field, int num);

int32_t
nmp_sql_get_field_type(NmpMysqlField *field, int num);

char *
nmp_sql_get_field_value(NmpMysqlRow row,int num);

NmpMysqlFieldOffset
nmp_sql_field_seek(NmpMysqlRes *res, NmpMysqlFieldOffset offset);

void
nmp_sql_put_res(NmpMysqlRes *res, guint size);

#endif //__NMP_MYSQLFUN_H__
