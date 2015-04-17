/*
 * nmp_appmod.h
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#ifndef __NMP_APP_MOD_H__
#define __NMP_APP_MOD_H__

#include "nmp_appobj.h"
#include "nmp_modio.h"
#include "nmp_msgengine.h"
#include "nmp_share_timer.h"
#include "nmp_etable.h"

G_BEGIN_DECLS

#define MAX_MOD_NAME_LEN			32

#define NMP_TYPE_APPMOD	(nmp_app_mod_get_type())
#define NMP_IS_APPMOD(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), NMP_TYPE_APPMOD))
#define NMP_IS_APPMOD_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), NMP_TYPE_APPMOD))
#define NMP_APPMOD(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), NMP_TYPE_APPMOD, NmpAppMod))
#define NMP_APPMOD_CLASS(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), NMP_TYPE_APPMOD, NmpAppModClass))
#define NMP_APPMOD_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), NMP_TYPE_APPMOD, NmpAppModClass))

typedef struct _NmpAppMod NmpAppMod;
typedef struct _NmpAppModClass NmpAppModClass;

struct _NmpAppMod
{
	NmpAppObj parent_object;

	NmpModIO 		*io_mod;
	NmpEventTable	*event_table;		/* sync request record table */
	NmpMsgEngine 	*msg_engine;
	
#ifdef MOD_HAS_TIMER
	NmpTimerEngine	*timer_engine;
#endif
	
	gchar 			name[MAX_MOD_NAME_LEN];	/* mod name */
};


struct _NmpAppModClass
{
	NmpAppObjClass parent_class;

	gint (*setup_mod)(NmpAppMod *self);
	void (*consume_msg)(NmpAppMod *self, NmpSysMsg *msg);
};


GType nmp_app_mod_get_type( void );

void nmp_app_mod_setup(NmpAppMod *self);

void nmp_app_mod_set_name(NmpAppMod *self, const gchar *name);
const gchar *nmp_app_mod_get_name(NmpAppMod *self);

gint nmp_app_mod_rcv_f(NmpAppMod *self, NmpSysMsg *msg);
gint nmp_app_mod_rcv_b(NmpAppMod *self, NmpSysMsg *msg);
gint nmp_app_mod_snd(NmpAppMod *self, NmpSysMsg *msg);

gint nmp_app_mod_register_msg(NmpAppMod *self, NmpMsgID msg_id,
	NmpMsgFun f_fun, NmpMsgFun b_fun, guint flags);

#ifdef MOD_HAS_TIMER
guint nmp_app_mod_set_timer(NmpAppMod *self, guint interval,
	NmpTimerFun fun, gpointer data);
void nmp_app_mod_del_timer(NmpAppMod *self, gint id);
#endif	//MOD_HAS_TIMER

gint nmp_app_mod_sync_request(NmpAppMod *self, NmpSysMsg **msg);

G_END_DECLS

#endif	//__NMP_APP_MOD_H__
