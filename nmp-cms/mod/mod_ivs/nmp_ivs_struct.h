#ifndef __NMP_IVS_STRUCT_H__
#define __NMP_IVS_STRUCT_H__

#include "nmp_mods.h"

typedef enum
{
	STAT_IVS_REGISTERING,
	STAT_IVS_ONLINE,
	STAT_IVS_OFFLINE
}JpfIvsState;

typedef struct _JpfIvs JpfIvs;
struct _JpfIvs
{
	JpfGuestBase	guest_base;

	JpfIvsState 	ivs_state;
	gint            ttl;      /*time to live*/
	gint            hb_freq;  /*keep alive frequency(sec)*/
};


#endif	//__NMP_IVS_STRUCT_H__
