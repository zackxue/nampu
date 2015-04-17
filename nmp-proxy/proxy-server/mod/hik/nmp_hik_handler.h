
#ifndef __HIK_HANDLER_H__
#define __HIK_HANDLER_H__


#include "nmp_packet.h"
#include "nmp_xmlmsg.h"
#include "nmp_msg_impl.h"
#include "nmp_proxy_device.h"


#ifdef __cplusplus
extern "C" {
#endif

int hik_get_device_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_get_serial_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_set_serial_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_get_device_time(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_set_device_time(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_get_ntp_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_set_ntp_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_get_network_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_set_network_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_get_pppoe_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_set_pppoe_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_get_ftp_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_set_ftp_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_get_smtp_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_set_smtp_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_get_ddns_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_set_ddns_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_get_upnp_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_set_upnp_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_get_disk_list(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_format_disk(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_control_device(struct service *srv, int channel, int parm_id, void *pvalue);

int hik_get_encode_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_set_encode_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_get_display_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_set_display_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_get_osd_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_set_osd_info(struct service *srv, int channel, int parm_id, void *pvalue);

int hik_get_ptz_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_set_ptz_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_get_record_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_set_record_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_get_hide_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_set_hide_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_get_move_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_set_move_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_get_video_lost_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_set_video_lost_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_get_hide_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_set_hide_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_get_io_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_set_io_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue);

int hik_get_store_log(struct service *srv, int channel, int parm_id, void *pvalue);

int hik_control_ptz(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_set_preset_point(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_get_cruise_way(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_set_cruise_way(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_add_cruise_way(struct service *srv, int channel, int parm_id, void *pvalue);
int hik_modify_cruise_way(struct service *srv, int channel, int parm_id, void *pvalue);










int hik_get_capability_set(struct service *srv, int channel, int parm_id, void *pvalue);


#ifdef __cplusplus
    }
#endif



#endif  //__HIK_HANDLER_H__

