#ifndef __NMP_BSS_STRUCT_H__
#define __NMP_BSS_STRUCT_H__

#include "nmp_mods.h"

typedef enum
{
	STAT_ADMIN_REQUEST,
	STAT_ADMIN_LOGIN,
	STAT_ADMIN_FAILED
}JpfAdminState;

typedef struct _JpfBss JpfBss;
struct _JpfBss
{
	JpfGuestBase	guest_base;

	JpfAdminState 	user_state;
	gint            ttl;      /*time to live*/
	gint            hb_freq;  /*keep alive frequency(sec)*/
};


#endif	//__NMP_BSS_STRUCT_H__
