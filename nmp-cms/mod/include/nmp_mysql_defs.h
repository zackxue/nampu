/*
 *	@author:	zyt
 *	@time:	2013/05/06
 */
#ifndef __NMP_MYSQL_DEFS_H__
#define __NMP_MYSQL_DEFS_H__

#include <mysql.h>


#define mysql_malloc(size)  malloc(size)
#define mysql_free(ptr)     free(ptr)
#define MYSQL_RESULT_CODE(ptr)  ((ptr)->result_code)
#define MYSQL_RESULT(ptr)  ((ptr)->sql_res)

#define QUERY_STR_LEN           20480
#define MAX_STR_LEN           128

typedef MYSQL               JpfMysql;
typedef MYSQL_FIELD         JpfMysqlField;
typedef MYSQL_ROW           JpfMysqlRow;
typedef MYSQL_FIELD_OFFSET  JpfMysqlFieldOffset;

typedef struct _JpfMysqlRes JpfMysqlRes;
//define struct to store the result of query
struct _JpfMysqlRes
{
	uint32_t result_code;   //0:success, -1:failure
	uint32_t is_select;     //0:not select, 1:is select
   	MYSQL_RES *sql_res;
};


#endif	//__NMP_MYSQL_DEFS_H__