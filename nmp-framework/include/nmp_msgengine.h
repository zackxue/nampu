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

}NmpMsgFunRet;


typedef NmpMsgFunRet (*NmpMsgFun)(NmpAppObj *app_obj, NmpSysMsg *msg);

/* 
 * This is POD type
 */

typedef struct _JpfMsgTable JpfMsgTable;
struct _JpfMsgTable
{
	struct _JpfMsgTableEntry {
		NmpMsgID entry_msg;
		guint entry_flags;
		NmpMsgFun fun_forward;
		NmpMsgFun fun_backward;
	}msg_enties[MAX_MSG_ENTRIES];
	guint counts;
};

#define NMP_TYPE_MSGENGINE	(jpf_msg_engine_get_type())
#define NMP_IS_MSGENGINE(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), NMP_TYPE_MSGENGINE))
#define NMP_IS_MSGENGINE_CLASS(c) \
	(G_TYPE_CHECK_CLASS_TYPE((c), NMP_TYPE_MSGENGINE))
#define NMP_MSGENGINE(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), NMP_TYPE_MSGENGINE, NmpMsgEngine))
#define NMP_MSGENGINE_CLASS(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), NMP_TYPE_MSGENGINE, NmpMsgEngineClass))
#define NMP_MSGENGINE_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), NMP_TYPE_MSGENGINE, NmpMsgEngineClass))

typedef struct _NmpMsgEngine NmpMsgEngine;
typedef struct _NmpMsgEngineClass NmpMsgEngineClass;


struct _NmpMsgEngine
{
	JpfObject parent_object;

	gchar		name[ENGINE_NAME];
	NmpAppObj	*owner;
	JpfMsgTable *msg_dict;
	GThreadPool	*th_pool_f;		/* Forward msg threads pool */
	GThreadPool	*th_pool_b;		/* Backward msg threads pool */
};


struct _NmpMsgEngineClass
{
	JpfObjectClass parent_class;
};


GType jpf_msg_engine_get_type( void );
void jpf_msg_engine_set_thread_exclusive(gboolean exclusive);
void jpf_msg_engine_set_threads(guint fthreads, guint bthreads);
void jpf_msg_engine_set_name(NmpMsgEngine *self, const gchar *name);
gint jpf_msg_engine_set_owner(NmpMsgEngine *self, NmpAppObj *owner);
gint jpf_msg_engine_push_msg_f(NmpMsgEngine *self, NmpSysMsg *msg);
gint jpf_msg_engine_push_msg_b(NmpMsgEngine *self, NmpSysMsg *msg);

gint jpf_msg_engine_register_msg(NmpMsgEngine *self, NmpMsgID msg_id,
	NmpMsgFun f_fun, NmpMsgFun b_fun, guint flags);

G_END_DECLS

#endif //__MSG_ENGINE_H__
