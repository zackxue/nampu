#ifndef __NMP_MOD_STREAM_H__
#define __NMP_MOD_STREAM_H__

#include "nmp_afx.h"
#include "nmp_sch.h"

G_BEGIN_DECLS

#define NMP_TYPE_MODSTREAM	(nmp_mod_stream_get_type())
#define NMP_IS_MODSTREAM(o)	(G_TYPE_CHECK_INSTANCE_TYPE((o), NMP_TYPE_MODSTREAM))
#define NMP_IS_MODSTREAM_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), NMP_TYPE_MODSTREAM))
#define NMP_MODSTREAM(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), NMP_TYPE_MODSTREAM, NmpModStream))
#define NmpModDBSClass(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), NMP_TYPE_MODSTREAM, NmpModStreamClass))
#define NMP_MODSTREAM_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), NMP_TYPE_MODSTREAM, NmpModStreamClass))

typedef struct _NmpModStream NmpModStream;
typedef struct _NmpModStreamClass NmpModStreamClass;
struct _NmpModStream
{
	NmpAppMod	  parent_object;

	NmpSchPool	*spool;
};

struct _NmpModStreamClass
{
	NmpAppModClass	parent_class;
};


GType nmp_mod_stream_get_type( void );


G_END_DECLS


#endif	//__NMP_MOD_STREAM_H__












