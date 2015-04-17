
#ifndef __BSM_HANDLER_H__
#define __BSM_HANDLER_H__

#include "nmp_packet.h"
#include "nmp_xmlmsg.h"
#include "nmp_msg_impl.h"
#include "nmp_proxy_device.h"


#ifdef __cplusplus
extern "C" {
#endif

int bsm_get_device_info(struct service *srv, int channel, int parm_id, void *pvalue);
int bsm_get_network_info(struct service *srv, int channel, int parm_id, void *pvalue);
int bsm_set_network_info(struct service *srv, int channel, int parm_id, void *pvalue);
int bsm_get_ftp_info(struct service *srv, int channel, int parm_id, void *pvalue);
int bsm_set_ftp_info(struct service *srv, int channel, int parm_id, void *pvalue);
int bsm_get_smtp_info(struct service *srv, int channel, int parm_id, void *pvalue);
int bsm_set_smtp_info(struct service *srv, int channel, int parm_id, void *pvalue);
int bsm_get_ddns_info(struct service *srv, int channel, int parm_id, void *pvalue);
int bsm_set_ddns_info(struct service *srv, int channel, int parm_id, void *pvalue);
int bsm_get_disk_list(struct service *srv, int channel, int parm_id, void *pvalue);
int bsm_format_disk(struct service *srv, int channel, int parm_id, void *pvalue);
int bsm_control_device(struct service *srv, int channel, int parm_id, void *pvalue);
int bsm_get_encode_info(struct service *srv, int channel, int parm_id, void *pvalue);
int bsm_set_encode_info(struct service *srv, int channel, int parm_id, void *pvalue);
int bsm_get_display_info(struct service *srv, int channel, int parm_id, void *pvalue);
int bsm_set_display_info(struct service *srv, int channel, int parm_id, void *pvalue);
int bsm_get_osd_info(struct service *srv, int channel, int parm_id, void *pvalue);
int bsm_set_osd_info(struct service *srv, int channel, int parm_id, void *pvalue);
int bsm_get_ptz_info(struct service *srv, int channel, int parm_id, void *pvalue);
int bsm_set_ptz_info(struct service *srv, int channel, int parm_id, void *pvalue);
int bsm_get_move_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue);
int bsm_set_move_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue);
int bsm_control_ptz(struct service *srv, int channel, int parm_id, void *pvalue);
int bsm_get_capability_set(struct service *srv, int channel, int parm_id, void *pvalue);


#ifdef __cplusplus
    }
#endif



#endif  //__BSM_HANDLER_H__


