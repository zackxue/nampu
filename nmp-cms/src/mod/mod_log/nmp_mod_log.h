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

#define JPF_TYPE_MODLOG	(jpf_mod_log_get_type())
#define JPF_IS_MODLOG(o)	(G_TYPE_CHECK_INSTANCE_TYPE((o), JPF_TYPE_MODLOG))
#define JPF_IS_MODLOG_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), JPF_TYPE_MODLOG))
#define JPF_MODLOG(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), JPF_TYPE_MODLOG, JpfModLog))
#define JPF_MODLOG_CLASS(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), JPF_TYPE_MODLOG, JpfModLogClass))
#define JPF_MODLOG_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), JPF_TYPE_MODLOG, JpfModLogClass))


typedef struct _JpfModLog JpfModLog;
typedef struct _JpfModLogClass JpfModLogClass;

struct _JpfModLog
{
	JpfAppMod	parent_object;
	db_conn_pool_conf *pool_conf;
	db_conn_pool_info *pool_info;
	gint del_log_flag;
};

struct _JpfModLogClass
{
	JpfAppModClass	parent_class;
};


GType jpf_mod_log_get_type(void);


G_END_DECLS


#endif	//__NMP_MOD_LOG_H__
