#ifndef __MSG_ENGINE_H__
#define __MSG_ENGINE_H__

#include "nmp_object.h"
#include "nmp_sysmsg.h"
#include "nmp_appobj.h"

G_BEGIN_DECLS

#define ENGINE_NAME				32
#define MAX_MSG_ENTRIES			512
#define MAX_MSG_ENGINE_THREADS	8

#define ENTRY_MSG_COUNTS(t) ((t)->counts)
#define ENTRY_MSG_BYINDEX(t, i) ((t)->msg_enties[i].entry_msg)
#define ENTRY_FFUN_BYINDEX(t, i) ((t)->msg_enties[i].fun_forward)
#define ENTRY_BFUN_BYINDEX(t, i) ((t)->msg_enties[i].fun_backward)
#define ENTRY_FLAGS_BYINDEX(t, i) ((t)->msg_enties[i].entry_flags)
#define ENTRY_NSLOT_BYINDEX(t, i) ((t)->msg_enties[i].next_slot)

typedef enum
{
	MFR_ACCEPTED = 0,		//@{msg has been accepted, no delivery
	                        //: needed any more}
	MFR_DELIVER_AHEAD,		//@{deliver, go on ahead}
	MFR_DELIVER_BACK		//@{deliver, go on backward}

}JpfMsgFunRet;


typedef JpfMsgFunRet (*JpfMsgFun)(JpfAppObj *app_obj, JpfSysMsg *msg);

/* 
 * This is POD type
 */

typedef struct _JpfMsgTable JpfMsgTable;
struct _JpfMsgTable
{
	struct _JpfMsgTableEntry {
		JpfMsgID entry_msg;
		guint entry_flags;
		JpfMsgFun fun_forward;
		JpfMsgFun fun_backward;
	}msg_enties[MAX_MSG_ENTRIES];
	guint counts;
};

#define JPF_TYPE_MSGENGINE	(jpf_msg_engine_get_type())
#define JPF_IS_MSGENGINE(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), JPF_TYPE_MSGENGINE))
#define JPF_IS_MSGENGINE_CLASS(c) \
	(G_TYPE_CHECK_CLASS_TYPE((c), JPF_TYPE_MSGENGINE))
#define JPF_MSGENGINE(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), JPF_TYPE_MSGENGINE, JpfMsgEngine))
#define JPF_MSGENGINE_CLASS(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), JPF_TYPE_MSGENGINE, JpfMsgEngineClass))
#define JPF_MSGENGINE_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), JPF_TYPE_MSGENGINE, JpfMsgEngineClass))

typedef struct _JpfMsgEngine JpfMsgEngine;
typedef struct _JpfMsgEngineClass JpfMsgEngineClass;


struct _JpfMsgEngine
{
	JpfObject parent_object;

	gchar		name[ENGINE_NAME];
	JpfAppObj	*owner;
	JpfMsgTable *msg_dict;
	GThreadPool	*th_pool_f;		/* Forward msg threads pool */
	GThreadPool	*th_pool_b;		/* Backward msg threads pool */
};


struct _JpfMsgEngineClass
{
	JpfObjectClass parent_class;
};


GType jpf_msg_engine_get_type( void );
void jpf_msg_engine_set_thread_exclusive(gboolean exclusive);
void jpf_msg_engine_set_threads(guint fthreads, guint bthreads);
void jpf_msg_engine_set_name(JpfMsgEngine *self, const gchar *name);
gint jpf_msg_engine_set_owner(JpfMsgEngine *self, JpfAppObj *owner);
gint jpf_msg_engine_push_msg_f(JpfMsgEngine *self, JpfSysMsg *msg);
gint jpf_msg_engine_push_msg_b(JpfMsgEngine *self, JpfSysMsg *msg);

gint jpf_msg_engine_register_msg(JpfMsgEngine *self, JpfMsgID msg_id,
	JpfMsgFun f_fun, JpfMsgFun b_fun, guint flags);

G_END_DECLS

#endif //__MSG_ENGINE_H__
