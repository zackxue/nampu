/********************************************************************
 * dbcLib  - Database connection pool for application
 * Function：provide database connection pool inition,get a connection,
 *           release a connection
 * Author:yangy
 * Description: Because using this lib, User need to invoke init_db_conn_pool
 *              only once, then he/she can use get_db_connection to get
 *              a connection for DB operation. Last, when he/she don't
 *              need connection, he/she can invoking put_db_connection
 *              to release one connection. Pool only be released with
 *              program quit.
 *              For more information, user can refer to folder test.
 * History:
 * 2011.5.23 - Yang Ying, initiate to create;
 * 2011.5.27 - Yang Ying, add db_printf,db_error,db_sprint,
 *             modify init_db_conn_pool
 ********************************************************************/

#include <mysql.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#define __USE_UNIX98
#include <pthread.h>
#undef __USE_UNIX98
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <linux/types.h>
#include <sys/stat.h>
#include "db_connect_pool.h"


void release_db_connection(db_conn_status * conn)
{
    mysql_close(conn->mysql);
    free(conn);
}

db_conn_status *
create_db_connection(char *host, char *name, char *user,
    char *passwd, char *my_cnf_path)
{
    assert((host != NULL) && (name != NULL) && (user != NULL));

    db_conn_status  *con = (db_conn_status*)malloc(sizeof(db_conn_status));
    if (con == NULL)
    {
        db_error("db: malloc(): Error: %s\n", strerror(errno));
        return NULL;
    }

	memset(con, 0, sizeof(db_conn_status));
    con->ref_count = 0;
    con->mysql = mysql_init(NULL);
	if (!con->mysql)
	{
		free(con);
		return NULL;
	}

    	printf("host=%s, user=%s, passwd=%s, name=%s,my_cnf_path=%s\n",
				host, user, passwd, name,my_cnf_path);
		 mysql_options(con->mysql, MYSQL_READ_DEFAULT_FILE, my_cnf_path);
        if (!mysql_real_connect(con->mysql, host, user, passwd, name, 0,
					NULL, CLIENT_MULTI_STATEMENTS))
        {
            db_fprintf(stderr, "db: Failed to connect to database: Error: %s\n",
                      mysql_error(con->mysql));
			 release_db_connection(con);
			  return NULL;
        }

    if (!mysql_set_character_set(con->mysql, "utf8"))
    {
        db_printf("new client character set: %s,\n", mysql_character_set_name(con->mysql));
    }
    else
    {
        db_error("set client character error: %d,", mysql_errno(con->mysql));
    }

    return  con;
}


static __inline__ void
__add_conn_to_pool(db_conn_pool_info *pool_info,db_conn_status *con)
{
	list_add_tail(&con->list, &pool_info->pool_head.list);
	pool_info->total_num++;
	pthread_cond_broadcast(&pool_info->cond);
}

void
add_conn_to_pool(db_conn_pool_info *pool_info,db_conn_status *con)
{
	assert((pool_info != NULL) && (con != NULL));
	pthread_mutex_lock(&pool_info->conn_lock);
	__add_conn_to_pool(pool_info, con);
	pthread_mutex_unlock(&pool_info->conn_lock);
}


int
init_db_conn_pool(db_conn_pool_info *pool_info,db_conn_pool_conf *pool_conf)
{
    int i;
    db_conn_status  *con = NULL;
    static pthread_mutexattr_t  m_mutexattr;

    memset(pool_info, 0, sizeof(*pool_info));
    pthread_mutexattr_init(&m_mutexattr);
    pthread_mutexattr_settype(&m_mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);
    INIT_LIST_HEAD(&pool_info->pool_head.list);
    pthread_mutex_init(&pool_info->conn_lock, &m_mutexattr);
    pthread_cond_init(&pool_info->cond, NULL);

    for (i = 0; i < pool_conf->min_connections; i++)
    {
        con = create_db_connection(pool_conf->host, pool_conf->db_name,
               pool_conf->user_name, pool_conf->user_password, pool_conf->my_cnf_path);
        if (con == NULL)
        {
        	  db_error("create connection  error: ");
        		continue;
        }
        add_conn_to_pool(pool_info, con);
    }

    return  0;
}

db_conn_status *
get_db_connection(db_conn_pool_info *pool_info,db_conn_pool_conf *pool_conf)
{
    db_conn_status  *cur_conn = NULL;
    int ret;
    int idle;
    struct timespec timeout;

    pthread_mutex_lock(&pool_info->conn_lock);

    idle = pool_info->total_num - pool_info->used_num;
    db_printf("---idle=%d,pool_info->used_num=%d,pool_info->total_num=%d\n",
          idle,pool_info->used_num,pool_info->total_num);

    if (idle <= 0) //无空闲连接
    {
        if (pool_info->total_num < pool_conf->max_connections)
        {
            cur_conn = create_db_connection(pool_conf->host, pool_conf->db_name,
                pool_conf->user_name, pool_conf->user_password, pool_conf->my_cnf_path);
            if (cur_conn == NULL)
            {
                pthread_mutex_unlock(&pool_info->conn_lock);
                return NULL;
            }
            __add_conn_to_pool(pool_info, cur_conn);
        }
        else
        {
            db_printf("====pthread_cond_wait\n");

             timeout.tv_sec= time(NULL) + 5;
             timeout.tv_nsec = 0;

            ret = pthread_cond_timedwait(&pool_info->cond, &pool_info->conn_lock,&timeout);
            if ((ret < 0)||(ret == 110)) //超时返回
            {
                if ((errno == ETIMEDOUT)||(ret == 110))
                {
                    pthread_mutex_unlock(&pool_info->conn_lock);
                    return NULL;
                }
            }
        }
    }
    if (!list_empty(&pool_info->pool_head.list))
    {
        list_for_each_entry(cur_conn, &pool_info->pool_head.list, list)
        {
            if (cur_conn->ref_count == 0)
            {
                cur_conn->ref_count++;
                pool_info->used_num++;

                list_del(&cur_conn->list);
                list_add_tail(&cur_conn->list, &pool_info->pool_head.list);

                pthread_mutex_unlock(&pool_info->conn_lock);

                return cur_conn;
            }
        }
    }
    pthread_mutex_unlock(&pool_info->conn_lock);
    return NULL;
}

void
put_db_connection(db_conn_pool_info *pool_info,db_conn_status *conn)
{
    assert((pool_info != NULL) && (conn != NULL));

	pthread_mutex_lock(&pool_info->conn_lock);

	if (conn->ref_count == 1)
	{
	     conn->ref_count = 0;
            pool_info->used_num--;
            pthread_cond_broadcast(&pool_info->cond);
	}
	else
	{
            list_del(&conn->list);
            release_db_connection(conn);
            pool_info->used_num--;
            pool_info->total_num--;
	}

   pthread_mutex_unlock(&pool_info->conn_lock);
}


void
put_db_connection_try(db_conn_pool_info *pool_info, db_conn_status *conn)
{
	if (conn == NULL)
		return ;
	put_db_connection(pool_info, conn);
}


void
kill_db_connection(db_conn_pool_info *pool_info,db_conn_status  *conn)
{
    pthread_mutex_lock(&pool_info->conn_lock);
    list_del(&conn->list);
    release_db_connection(conn);
    pool_info->used_num--;
    pool_info->total_num--;
    pthread_mutex_unlock(&pool_info->conn_lock);
}

