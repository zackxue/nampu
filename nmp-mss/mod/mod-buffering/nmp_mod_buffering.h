#ifndef __NMP_MOD_BUFFERING_H__
#define __NMP_MOD_BUFFERING_H__

#include "nmp_afx.h"
#include "nmp_sbuff.h"

G_BEGIN_DECLS

#define NMP_TYPE_MODBUFFERING	(nmp_mod_buffering_get_type())
#define NMP_IS_MODBUFFERING(o)	(G_TYPE_CHECK_INSTANCE_TYPE((o), NMP_TYPE_MODBUFFERING))
#define NMP_IS_MODBUFFERING_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), NMP_TYPE_MODBUFFERING))
#define NMP_MODBUFFERING(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), NMP_TYPE_MODBUFFERING, NmpModBuffering))
#define NmpModDBSClass(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), NMP_TYPE_MODBUFFERING, NmpModBufferingClass))
#define NMP_MODBUFFERING_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), NMP_TYPE_MODBUFFERING, NmpModBufferingClass))

typedef struct _NmpModBuffering NmpModBuffering;
typedef struct _NmpModBufferingClass NmpModBufferingClass;
struct _NmpModBuffering
{
	NmpAppMod	  parent_object;

	NmpSBuffOps   *ops;
	GList         *sb_list;
	GMutex        *mutex;
	GThreadPool   *tp_flush;
};


struct _NmpModBufferingClass
{
	NmpAppModClass	parent_class;
};


GType nmp_mod_buffering_get_type( void );
NmpModBuffering *nmp_mod_buffering_get( void );

void nmp_mod_buffering_register_sb_ops(NmpModBuffering *mb, NmpSBuffOps *ops);

NmpSBuff *nmp_mod_buffering_new_sbuff(NmpModBuffering *mb, NmpGuid *guid,
	guint flags, guint local_tags);
void nmp_mod_buffering_del_sbuff(NmpModBuffering *mb, NmpSBuff *sb);
void nmp_mod_buffering_flush_pending(NmpModBuffering *mb, NmpSBuff *sb);

G_END_DECLS

#endif	//__NMP_MOD_BUFFERING_H__












