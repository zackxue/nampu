#ifndef __NMP_MOD_VOD_H__
#define __NMP_MOD_VOD_H__

#include "nmp_afx.h"
#include "nmp_mds.h"
#include "nmp_records.h"


G_BEGIN_DECLS

#define NMP_TYPE_MODVOD	(nmp_mod_vod_get_type())
#define NMP_IS_MODVOD(o)	(G_TYPE_CHECK_INSTANCE_TYPE((o), NMP_TYPE_MODVOD))
#define NMP_IS_MODVOD_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), NMP_TYPE_MODVOD))
#define NMP_MODVOD(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), NMP_TYPE_MODVOD, NmpModVod))
#define NmpModDBSClass(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), NMP_TYPE_MODVOD, NmpModVodClass))
#define NMP_MODVOD_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), NMP_TYPE_MODVOD, NmpModVodClass))

typedef struct _NmpModVod NmpModVod;
typedef struct _NmpModVodClass NmpModVodClass;
struct _NmpModVod
{
	NmpAppMod	  parent_object;

	NmpMdsPool    *mds_pool;
	NmpRecords	  *records;
};

struct _NmpModVodClass
{
	NmpAppModClass	parent_class;
};


GType nmp_mod_vod_get_type( void );


G_END_DECLS


#endif	//__NMP_MOD_VOD_H__












