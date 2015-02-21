/*
 * nmp_appmod.h
 *
 * Copyright(c) by HiMickey, 2010~2014
 * Author:
*/

#ifndef __JPF_APP_MOD_H__
#define __JPF_APP_MOD_H__

#include "nmp_appobj.h"
#include "nmp_modio.h"
#include "nmp_msgengine.h"
#include "nmp_timer.h"
#include "nmp_etable.h"

G_BEGIN_DECLS

#define MAX_MOD_NAME_LEN			32

#define JPF_TYPE_APPMOD	(jpf_app_mod_get_type())
#define JPF_IS_APPMOD(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), JPF_TYPE_APPMOD))
#define JPF_IS_APPMOD_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), JPF_TYPE_APPMOD))
#define JPF_APPMOD(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), JPF_TYPE_APPMOD, JpfAppMod))
#define JPF_APPMOD_CLASS(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), JPF_TYPE_APPMOD, JpfAppModClass))
#define JPF_APPMOD_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), JPF_TYPE_APPMOD, JpfAppModClass))

typedef struct _JpfAppMod JpfAppMod;
typedef struct _JpfAppModClass JpfAppModClass;

struct _JpfAppMod
{
	JpfAppObj parent_object;

	JpfModIO 		*io_mod;
	JpfEventTable	*event_table;		/* sync request record table */
	JpfMsgEngine 	*msg_engine;
	
#ifdef MOD_HAS_TIMER
	JpfTimerEngine	*timer_engine;
#endif
	
	gchar 			name[MAX_MOD_NAME_LEN];	/* mod name */
};


struct _JpfAppModClass
{
	JpfAppObjClass parent_class;

	gint (*setup_mod)(JpfAppMod *self);
	void (*consume_msg)(JpfAppMod *self, JpfSysMsg *msg);
};


GType jpf_app_mod_get_type( void );

void jpf_app_mod_setup(JpfAppMod *self);

void jpf_app_mod_set_name(JpfAppMod *self, const gchar *name);
const gchar *jpf_app_mod_get_name(JpfAppMod *self);

gint jpf_app_mod_rcv_f(JpfAppMod *self, JpfSysMsg *msg);
gint jpf_app_mod_rcv_b(JpfAppMod *self, JpfSysMsg *msg);
gint jpf_app_mod_snd(JpfAppMod *self, JpfSysMsg *msg);

gint jpf_app_mod_register_msg(JpfAppMod *self, JpfMsgID msg_id,
	JpfMsgFun f_fun, JpfMsgFun b_fun, guint flags);

#ifdef MOD_HAS_TIMER
guint jpf_app_mod_set_timer(JpfAppMod *self, guint interval,
	JpfTimerFun fun, gpointer data);
void jpf_app_mod_del_timer(JpfAppMod *self, gint id);
#endif	//MOD_HAS_TIMER

gint jpf_app_mod_sync_request(JpfAppMod *self, JpfSysMsg **msg);

G_END_DECLS

#endif	//__JPF_APP_MOD_H__
