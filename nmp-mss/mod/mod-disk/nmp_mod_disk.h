#ifndef __NMP_MOD_DISK_H__
#define __NMP_MOD_DISK_H__

#include "nmp_afx.h"
#include "nmp_storage.h"

G_BEGIN_DECLS

#define NMP_TYPE_MODDISK	(nmp_mod_disk_get_type())
#define NMP_IS_MODDISK(o)	(G_TYPE_CHECK_INSTANCE_TYPE((o), NMP_TYPE_MODDISK))
#define NMP_IS_MODDISK_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), NMP_TYPE_MODDISK))
#define NMP_MODDISK(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), NMP_TYPE_MODDISK, NmpModDisk))
#define NmpModDBSClass(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), NMP_TYPE_MODDISK, NmpModDiskClass))
#define NMP_MODDISK_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), NMP_TYPE_MODDISK, NmpModDiskClass))

typedef struct _NmpModDisk NmpModDisk;
typedef struct _NmpModDiskClass NmpModDiskClass;
struct _NmpModDisk
{
	NmpAppMod	  parent_object;
	
	NmpStorage	*storage;
};

struct _NmpModDiskClass
{
	NmpAppModClass	parent_class;
};


GType nmp_mod_disk_get_type( void );


G_END_DECLS


#endif	//__NMP_MOD_DISK_H__












