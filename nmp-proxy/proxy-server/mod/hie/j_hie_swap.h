#ifndef __HIE_SWAP_H__
#define __HIE_SWAP_H__

#include "HieClientUnit.h"
#include "HieClient_Common.h"
#include "HieClient_Configure.h"

#include "nmp_sdk.h"
#include "j_hie_sdkinfo.h"

#ifdef __cplusplus
extern "C" {
#endif


void hie_swap_device_info(HY_DVR_DEVICE_INFO *dev_cfg, JDeviceInfo *dev_info, int flag);
void hie_swap_serial_info(HY_DVR_SERIAL *dev_cfg, JSerialParameter *dev_info, int flag);
void hie_swap_time_info(HY_DVR_TIME *dev_cfg, JDeviceTime *dev_info, int flag);
void hie_swap_net_info(HY_DVR_NET_CFG *dev_cfg, JNetworkInfo *dev_info, int flag);
void hie_swap_pppoe_info(HY_DVR_PPPOE_CONF  *dev_cfg, JPPPOEInfo *dev_info, int flag);
void hie_swap_ddns_info(HY_DVR_DDNS_CONF  *dev_cfg, JDdnsConfig *dev_info, int flag);
void hie_swap_disk_list(HY_DVR_STORAGE_CFG *dev_cfg, JDeviceDiskInfo *dev_info, int flag);
void hie_swap_encode_info(HY_DVR_COMPRESSION_CFG *dev_cfg, JEncodeParameter *dev_info, int flag, int channel);
void hie_swap_osd_info(HY_DVR_PIC_CFG *dev_cfg, JOSDParameter *dev_info, int flag, int channel);
void hie_swap_record_info(HY_DVR_RECORD_SCHED *dev_cfg, JRecordParameter *dev_info, int flag, int channel);


#ifdef __cplusplus
    }
#endif

#endif  //__HIE_SWAP_H__


