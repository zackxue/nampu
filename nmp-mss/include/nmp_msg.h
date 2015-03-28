#ifndef __NMP_TINY_MSG_H__
#define __NMP_TINY_MSG_H__

#include "nmp_guid.h"

#define EVENT_REC			0
#define EVENT_STOP_REC		1
#define EVENT_POLICY		2
#define EVENT_URI			3
#define EVENT_MDSIP			4


typedef struct __NmpMsg NmpMsg;
typedef void (*NmpMsgFin)(NmpMsg *msg);

#define FREE_MSG(msg) (((NmpMsg*)(msg))->free(msg))
#define FREE_MSG_FUN(msg) (((NmpMsg*)(msg))->free)

struct __NmpMsg
{
	gint	id;
	NmpMsgFin free;
};


typedef struct __NmpRecMsg NmpRecMsg;
struct __NmpRecMsg
{
	NmpMsg	base;
	NmpGuid	guid;
	gint	rec_type;
	gint	hd_grp;
	gint	level;
};


typedef struct __NmpPolicyMsg NmpPolicyMsg;
struct __NmpPolicyMsg
{
	NmpMsg	base;
	NmpGuid	guid;
};


typedef struct __NmpUriMsg NmpUriMsg;
struct __NmpUriMsg
{
	NmpMsg	base;
	NmpGuid	guid;
	gchar	*uri;
};


typedef struct __NmpMdsIpMsg NmpMdsIpMsg;
struct __NmpMdsIpMsg
{
	NmpMsg	base;

	gchar *mds_id;
};


#endif	/* __NMP_TINY_MSG_H__ */
