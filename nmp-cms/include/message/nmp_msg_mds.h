#ifndef __NMP_MOD_MDU_MESSAGES_EXTERNAL_H__
#define __NMP_MOD_MDU_MESSAGES_EXTERNAL_H__

#include "nmp_msg_share.h"
#include "nmp_list_head.h"


typedef struct _NmpMdsRegister NmpMdsRegister;
struct _NmpMdsRegister
{
    gchar mds_id[MDS_ID_LEN];
    gchar cms_ip[MAX_IP_LEN];
    gchar mds_ip[MAX_IP_LEN];
};

typedef struct _NmpMdsRegisterRes NmpMdsRegisterRes;
struct _NmpMdsRegisterRes
{
    NmpMsgErrCode       code;
    gchar                    domain_id[DOMAIN_ID_LEN];
    gchar                    mds_id[MDS_ID_LEN];
    gint                       pu_port;
    gint                       rtsp_port;
    gint                       keep_alive_time;
    gint                       get_ip_enable;
};

typedef struct _NmpMdsHeart NmpMdsHeart;
struct _NmpMdsHeart
{
    gchar	mds_id[MDS_ID_LEN];
    gchar cms_ip[MAX_IP_LEN];
    gchar mds_ip[MAX_IP_LEN];
};

typedef struct _NmpMdsHeartRes NmpMdsHeartRes;
struct _NmpMdsHeartRes
{
    NmpMsgErrCode	code;
    gchar	mds_id[MDS_ID_LEN];
    gchar			server_time[TIME_INFO_LEN];
};




#endif //__NMP_MOD_MDU_MESSAGES_EXTERNAL_H__



