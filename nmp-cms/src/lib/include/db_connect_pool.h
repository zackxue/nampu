/********************************************************************
 * dbcLib  - Database connection pool for application
 * Function：provide database connection pool inition,get a connection,
 *           release a connection
 * Author:yangy
 * Description: To use this lib, User need to invoke init_db_conn_pool
 *              only once, then he/she can use get_db_connection to get
 *              a connection for DB operation. Last, when he/she don't
 *              need connection, he/she can invoking put_db_connection
 *              to release one connection. Pool only be released with
 *              program quit.
 *              For more information, user can refer to folder test.
 * History:
 * 2011.5.23 - Yang Ying, initiate to create;
 * 2011.5.27 - Yang Ying, add macro define
 * 2011.6.8  - Zhang Shiyong, clear up code, make it more artistic
 ********************************************************************/

#ifndef __DB_CONNECT_POOL_H
#define __DB_CONNECT_POOL_H

#include <mysql.h>
#include <string.h>
#include "nmp_list_head.h"

#define db_printf(fmt, arg...) \
                        do{\
							printf("FILE %s FUNC %s (%d)=>|"fmt,\
							__FILE__, __FUNCTION__, __LINE__ , ##arg);\
						}while(0)

#define db_error(fmt, arg...) \
                        do{\
							printf("FILE %s FUNC %s (%d)=>|"fmt,\
							__FILE__, __FUNCTION__, __LINE__ , ##arg);\
						}while(0)

#define db_fprintf fprintf

#define HOST_NAME_LEN             16
#define DB_NAME_LEN                 32
#define ADMIN_NAME_LEN           16
#define PASSWD_LEN                   16
#define FILE_PATH_LEN              256

#define SET_CONN_POOL_HOST(poolinfo, host) \
    do {\
		strncpy(poolinfo, host, ARRAY_LEN - 1);\
      host[ARRAY_LEN - 1] = 0;\
    	}while(0)
#define SET_DB_NAME(poolinfo, name) \
        (strncpy((poolinfo)->db_name, name, ARRAY_LEN - 1)),\
        ((char *)(name))[ARRAY_LEN - 1] = 0)
#define SET_DB_USER(poolinfo, name) \
        (strncpy((poolinfo)->user_name, name, ARRAY_LEN - 1)),\
        ((char *)(name))[ARRAY_LEN - 1] = 0)
#define SET_DB_PASSWORD(poolinfo, password) \
        (strncpy((poolinfo)->db_password, password, ARRAY_LEN - 1)),\
        ((char *)(password))[ARRAY_LEN - 1] = 0)
#define SET_DB_CONN_MIN_NUM(poolinfo, num) \
		((poolinfo)->min_connections = (num))
#define SET_DB_CONN_MAX_NUM(poolinfo, num) \
		((poolinfo)->max_connections = (num))


typedef  struct _db_conn_pool_info      db_conn_pool_info;
typedef  struct _db_conn_status 		db_conn_status;
typedef  struct _db_conn_pool_conf  	db_conn_pool_conf;

/*
 * 连接池的配置
 */
struct _db_conn_pool_conf
{
    char host[HOST_NAME_LEN];
    char db_name[DB_NAME_LEN];        //数据库名
    char user_name[ADMIN_NAME_LEN];
    char user_password[PASSWD_LEN];
    char my_cnf_path[FILE_PATH_LEN];
    int  min_connections;           //连接池连接数最小值，连接池初始大小
    int  max_connections;           //连接池连接数最大值
};

/*
 * 每个连接的信息
 */
struct _db_conn_status
{
	struct  list_head list;
	void	*mysql;
	int  	ref_count; //引用计数，0：未被引用，1：被引用一次
};

/*
 * 连接池信息
 */
struct _db_conn_pool_info
{
    int              total_num;     //连接池中连接总数
    int              used_num;      //被用掉的连接数
    db_conn_status   pool_head;
    pthread_mutex_t  conn_lock;
    pthread_cond_t   cond;
};


int
init_db_conn_pool(db_conn_pool_info *pool_info,db_conn_pool_conf *pool_conf);

db_conn_status *
get_db_connection(db_conn_pool_info *pool_info,db_conn_pool_conf *pool_conf);

void
put_db_connection(db_conn_pool_info *pool_info, db_conn_status *conn);

void
put_db_connection_try(db_conn_pool_info *pool_info, db_conn_status *conn);

void
kill_db_connection(db_conn_pool_info *pool_info,db_conn_status  *conn);


#endif