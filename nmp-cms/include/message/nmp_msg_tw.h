/*
 *	author:	zyt
 *	time:	begin in 2012/10/08
 */
#ifndef __NMP_MSG_TW_H__
#define __NMP_MSG_TW_H__

#include "nmp_msg_share.h"
#include "nmp_tw_interface.h"


typedef struct _NmpTwRegInfo NmpTwRegInfo;
struct _NmpTwRegInfo
{
	gchar	puid[MAX_ID_LEN];
	gint		decode_type;
};

typedef struct _NmpTwRegRes NmpTwRegRes;
struct _NmpTwRegRes
{
	NmpMsgErrCode	code;
	gchar			puid[MAX_ID_LEN];
	gint				keep_alive_time;
};


typedef struct _NmpTwHeart NmpTwHeart;
struct _NmpTwHeart
{
	gchar	puid[MAX_ID_LEN];
};

typedef struct _NmpTwHeartResp NmpTwHeartResp;
struct _NmpTwHeartResp
{
	NmpMsgErrCode	code;
	gchar			server_time[TIME_INFO_LEN];
};


typedef struct _NmpTwCmdResp NmpTwCmdResp;
struct _NmpTwCmdResp
{
	NmpMsgErrCode	code;
	char		session_id[TW_ID_LEN];			//cu»á»°id
};


typedef struct _NmpTwOnlineStatusChange NmpTwOnlineStatusChange;
struct _NmpTwOnlineStatusChange
{
	gchar	domain_id[DOMAIN_ID_LEN];
	gchar	puid[MAX_ID_LEN];
	gchar	pu_ip[MAX_IP_LEN];
	gint		new_status;		//pu online status
};

#endif

