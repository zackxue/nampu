#ifndef __NMP_MDS_STRUCT_H__
#define __NMP_MDS_STRUCT_H__

#include "nmp_mods.h"

typedef enum
{
	STAT_MDS_REGISTERING,
	STAT_MDS_ONLINE,
	STAT_MDS_OFFLINE
}NmpMdsState;

typedef struct _NmpMds NmpMds;
struct _NmpMds
{
	NmpGuestBase	guest_base;

	NmpMdsState 	mds_state;
	gint            ttl;      /*time to live*/
	gint            hb_freq;  /*keep alive frequency(sec)*/
};


#endif	//__NMP_MDS_STRUCT_H__
