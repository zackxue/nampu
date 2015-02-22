/*
 *	author:	zyt
 *	time:	begin in 2012/10/08
 */
#ifndef __NMP_MSG_TW_H__
#define __NMP_MSG_TW_H__

#include "nmp_msg_share.h"
#include "nmp_tw_interface.h"


typedef struct _JpfTwRegInfo JpfTwRegInfo;
struct _JpfTwRegInfo
{
	gchar	puid[MAX_ID_LEN];
	gint		decode_type;
};

typedef struct _JpfTwRegRes JpfTwRegRes;
struct _JpfTwRegRes
{
	JpfMsgErrCode	code;
	gchar			puid[MAX_ID_LEN];
	gint				keep_alive_time;
};


typedef struct _JpfTwHeart JpfTwHeart;
struct _JpfTwHeart
{
	gchar	puid[MAX_ID_LEN];
};

typedef struct _JpfTwHeartResp JpfTwHeartResp;
struct _JpfTwHeartResp
{
	JpfMsgErrCode	code;
	gchar			server_time[TIME_INFO_LEN];
};


typedef struct _JpfTwCmdResp JpfTwCmdResp;
struct _JpfTwCmdResp
{
	JpfMsgErrCode	code;
	char		session_id[TW_ID_LEN];			//cu»á»°id
};


typedef struct _JpfTwOnlineStatusChange JpfTwOnlineStatusChange;
struct _JpfTwOnlineStatusChange
{
	gchar	domain_id[DOMAIN_ID_LEN];
	gchar	puid[MAX_ID_LEN];
	gchar	pu_ip[MAX_IP_LEN];
	gint		new_status;		//pu online status
};

#endif

