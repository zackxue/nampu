/*
 *	@author:	zyt
 *	@time:	2013/05/04
 */
#ifndef __NMP_MOD_LOG_H__
#define __NMP_MOD_LOG_H__

#include "nmp_mods.h"
#include "nmp_appmod.h"
#include "db_connect_pool.h"


G_BEGIN_DECLS

#define LOG_TABLE				"log_table"

typedef struct {
	gint enable;
	guint flag;	//LOG_MSG_REQ | LOG_MSG_RES
} LOG_MSG_ENABLE;

#define LOG_MAX_MESSAGE_NUM	512
#define LOG_MSG_REQ				(1 << 0)
#define LOG_MSG_RES				(1 << 1)
extern LOG_MSG_ENABLE g_log_msg_enable[LOG_MAX_MESSAGE_NUM];


#define DEFAULT_HEART_SLICE		(3)

#define NMP_TYPE_MODLOG	(nmp_mod_log_get_type())
#define NMP_IS_MODLOG(o)	(G_TYPE_CHECK_INSTANCE_TYPE((o), NMP_TYPE_MODLOG))
#define NMP_IS_MODLOG_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), NMP_TYPE_MODLOG))
#define NMP_MODLOG(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), NMP_TYPE_MODLOG, NmpModLog))
#define NMP_MODLOG_CLASS(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), NMP_TYPE_MODLOG, NmpModLogClass))
#define NMP_MODLOG_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), NMP_TYPE_MODLOG, NmpModLogClass))


typedef struct _NmpModLog NmpModLog;
typedef struct _NmpModLogClass NmpModLogClass;

struct _NmpModLog
{
	NmpAppMod	parent_object;
	db_conn_pool_conf *pool_conf;
	db_conn_pool_info *pool_info;
	gint del_log_flag;
};

struct _NmpModLogClass
{
	NmpAppModClass	parent_class;
};


GType nmp_mod_log_get_type(void);


G_END_DECLS


#endif	//__NMP_MOD_LOG_H__
