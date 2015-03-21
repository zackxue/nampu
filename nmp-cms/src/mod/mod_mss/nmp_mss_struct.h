#ifndef __NMP_MSS_STRUCT_H__
#define __NMP_MSS_STRUCT_H__

#include "nmp_mods.h"

typedef enum
{
	STAT_MSS_REGISTERING,
	STAT_MSS_ONLINE,
	STAT_MSS_OFFLINE
}NmpMssState;

typedef struct _NmpMss NmpMss;
struct _NmpMss
{
	NmpGuestBase	guest_base;

	NmpMssState 	mss_state;
	gint            ttl;      /*time to live*/
	gint            hb_freq;  /*keep alive frequency(sec)*/
};


#endif	//__NMP_MSS_STRUCT_H__
