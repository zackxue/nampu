#ifndef __NMP_MOD_MDU_MESSAGES_EXTERNAL_H__
#define __NMP_MOD_MDU_MESSAGES_EXTERNAL_H__

#include "nmp_msg_share.h"
#include "nmp_list_head.h"


typedef struct _JpfMdsRegister JpfMdsRegister;
struct _JpfMdsRegister
{
    gchar mds_id[MDS_ID_LEN];
    gchar cms_ip[MAX_IP_LEN];
    gchar mds_ip[MAX_IP_LEN];
};

typedef struct _JpfMdsRegisterRes JpfMdsRegisterRes;
struct _JpfMdsRegisterRes
{
    JpfMsgErrCode       code;
    gchar                    domain_id[DOMAIN_ID_LEN];
    gchar                    mds_id[MDS_ID_LEN];
    gint                       pu_port;
    gint                       rtsp_port;
    gint                       keep_alive_time;
    gint                       get_ip_enable;
};

typedef struct _JpfMdsHeart JpfMdsHeart;
struct _JpfMdsHeart
{
    gchar	mds_id[MDS_ID_LEN];
    gchar cms_ip[MAX_IP_LEN];
    gchar mds_ip[MAX_IP_LEN];
};

typedef struct _JpfMdsHeartRes JpfMdsHeartRes;
struct _JpfMdsHeartRes
{
    JpfMsgErrCode	code;
    gchar	mds_id[MDS_ID_LEN];
    gchar			server_time[TIME_INFO_LEN];
};




#endif //__NMP_MOD_MDU_MESSAGES_EXTERNAL_H__



