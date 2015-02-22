#ifndef __NMP_MSS_STRUCT_H__
#define __NMP_MSS_STRUCT_H__

#include "nmp_mods.h"

typedef enum
{
	STAT_MSS_REGISTERING,
	STAT_MSS_ONLINE,
	STAT_MSS_OFFLINE
}JpfMssState;

typedef struct _JpfMss JpfMss;
struct _JpfMss
{
	JpfGuestBase	guest_base;

	JpfMssState 	mss_state;
	gint            ttl;      /*time to live*/
	gint            hb_freq;  /*keep alive frequency(sec)*/
};


#endif	//__NMP_MSS_STRUCT_H__
