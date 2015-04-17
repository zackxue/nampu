
#ifndef __XMT_HANDLER_H__
#define __XMT_HANDLER_H__

#include "nmp_packet.h"
#include "nmp_xmlmsg.h"
#include "nmp_msg_impl.h"
#include "nmp_proxy_device.h"


#ifdef __cplusplus
extern "C" {
#endif

int xmt_get_device_info(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_get_serial_info(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_set_serial_info(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_get_device_time(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_set_device_time(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_get_ntp_info(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_set_ntp_info(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_get_network_info(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_set_network_info(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_get_pppoe_info(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_set_pppoe_info(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_get_ftp_info(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_set_ftp_info(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_get_smtp_info(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_set_smtp_info(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_get_ddns_info(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_set_ddns_info(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_get_upnp_info(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_set_upnp_info(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_get_disk_list(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_format_disk(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_control_device(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_get_encode_info(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_set_encode_info(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_get_display_info(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_set_display_info(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_get_osd_info(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_set_osd_info(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_get_ptz_info(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_set_ptz_info(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_get_record_info(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_set_record_info(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_get_hide_info(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_set_hide_info(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_get_move_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_set_move_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_get_video_lost_info(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_set_video_lost_info(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_get_hide_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_set_hide_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_get_io_alarm_info(struct service *srv, int channel, int type, void *pvalue);
int xmt_set_io_alarm_info(struct service *srv, int channel, int type, void *pvalue);
int xmt_get_store_log(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_control_ptz(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_set_preset_point(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_get_cruise_way(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_set_cruise_way(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_add_cruise_way(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_modify_cruise_way(struct service *srv, int channel, int parm_id, void *pvalue);
int xmt_get_capability_set(struct service *srv, int channel, int parm_id, void *pvalue);


#ifdef __cplusplus
    }
#endif



#endif  //__XMT_HANDLER_H__

