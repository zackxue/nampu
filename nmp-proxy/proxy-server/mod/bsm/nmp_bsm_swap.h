#ifndef __BSM_SWAP_H__
#define __BSM_SWAP_H__

#include "hi_net_dev_sdk.h"
#include "hi_net_dev_errors.h"

#include "nmp_sdk.h"

#include "nmp_proxy_server.h"
#include "nmp_bsm_sdkinfo.h"


#ifdef __cplusplus
extern "C" {
#endif

void bsm_swap_device_info(HI_DEVICE_INFO *dev_cfg, 
        JDeviceInfo *dev_info, int flag);

void bsm_swap_network_info(HI_S_NETINFO *net_cfg, 
        JNetworkInfo *net_info, int flag);

void bsm_swap_ftp_info(HI_S_FTP_PARAM *ftp_cfg, 
        JFTPParameter *ftp_param, int flag);

void bsm_swap_smtp_info(HI_S_EMAIL_PARAM *email_cfg, 
        JSMTPParameter *smtp_info, int flag);

void bsm_swap_ddns_info(HI_S_DNS_PARAM *ddns_cfg, 
        JDdnsConfig *ddns_info, int flag);

void bsm_swap_disk_list(HI_S_DISK_INFO *disk_cfg, 
        JDeviceDiskInfo *dev_disk, int flag);

void bsm_swap_video_info(HI_S_Video *video_cfg, 
        JEncodeParameter *enc_info, int flag);

void bsm_swap_audio_info(HI_S_Audio *audio_cfg, 
        JEncodeParameter *enc_info, int flag);

void bsm_swap_display_info(HI_S_Display *dis_cfg, 
        JDisplayParameter *dis_info, int flag);

void bsm_swap_osd_info(HI_S_OSD *osd_cfg, 
        JOSDParameter *osd_info, int flag);

void bsm_swap_move_alarm_info(HI_S_MD_PARAM *bsm_motion, 
        JMoveAlarm *move_alarm, int flag);

void bsm_swap_ptz_info(HI_S_PTZ *ptz_cfg, 
        JPTZParameter *ptz_info, int flag);

#ifdef __cplusplus
    }
#endif


#endif  //__BSM_SWAP_H__


