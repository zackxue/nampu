
#ifndef __HBN_HANDLER_H__
#define __HBN_HANDLER_H__

#include "nmp_packet.h"
#include "nmp_xmlmsg.h"
#include "nmp_msg_impl.h"
#include "nmp_proxy_device.h"


#ifdef __cplusplus
extern "C" {
#endif

int hbn_get_device_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_get_serial_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_set_serial_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_get_device_time(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_set_device_time(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_get_ntp_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_set_ntp_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_get_network_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_set_network_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_get_pppoe_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_set_pppoe_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_get_ftp_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_set_ftp_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_get_smtp_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_set_smtp_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_get_ddns_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_set_ddns_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_get_upnp_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_set_upnp_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_get_disk_list(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_format_disk(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_control_device(struct service *srv, int channel, int parm_id, void *pvalue);

int hbn_get_encode_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_set_encode_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_get_display_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_set_display_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_get_osd_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_set_osd_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_get_ptz_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_set_ptz_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_get_record_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_set_record_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_get_hide_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_set_hide_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_get_move_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_set_move_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_get_video_lost_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_set_video_lost_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_get_hide_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_set_hide_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_get_io_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_set_io_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_get_store_log(struct service *srv, int channel, int parm_id, void *pvalue);

int hbn_ptz_control(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_set_preset_point(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_get_cruise_way(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_set_cruise_way(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_add_cruise_way(struct service *srv, int channel, int parm_id, void *pvalue);
int hbn_modify_cruise_way(struct service *srv, int channel, int parm_id, void *pvalue);

int hbn_get_capability_set(struct service *srv, int channel, int parm_id, void *pvalue);


#ifdef __cplusplus
    }
#endif

#endif  //__HBN_HANDLER_H__

