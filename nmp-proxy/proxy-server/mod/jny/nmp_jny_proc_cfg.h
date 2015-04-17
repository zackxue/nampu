#ifndef __J_JNY_PROC_CFG_H__
#define __J_JNY_PROC_CFG_H__


#include "nmp_resolve_host.h"
#include "nmp_jny_swap.h"
#include "nmp_jny_channel.h"
#include "nmp_jny_srv_impl.h"
#include "nmp_jny_service.h"
#include "nmp_jny_handler.h"


#ifdef __cplusplus
	extern "C" {
#endif

int jny_proc_get_device_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info);
int jny_proc_get_serial_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info);
int jny_proc_set_serial_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info);
int jny_proc_get_dev_time_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info);
int jny_proc_set_dev_time_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info);
int jny_proc_get_ntp_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info);
int jny_proc_set_ntp_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info);
int jny_proc_get_network_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info);
int jny_proc_set_network_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info);

int jny_proc_get_pppoe_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info);
int jny_proc_set_pppoe_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info);
int jny_proc_get_ftp_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info);
int jny_proc_set_ftp_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info);
int jny_proc_get_ddns_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info);
int jny_proc_set_ddns_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info);

int jny_proc_get_smtp_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info);
int jny_proc_set_smtp_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info);
int jny_proc_get_disk_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info);

int jny_proc_ctrl_device(int user_id, void *parm, proxy_sdk_t *sdk_info);
int jny_proc_get_osd_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info);
int jny_proc_set_osd_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info);

int jny_proc_get_ptz_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info);
int jny_proc_set_ptz_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info);

int jny_proc_get_hide_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info);
int jny_proc_set_hide_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info);

int jny_proc_get_move_alarm_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info);
int jny_proc_set_move_alarm_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info);


int jny_proc_get_encode_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info);
int jny_proc_set_encode_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info);
int jny_proc_get_hide_alarm_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info);
int jny_proc_set_hide_alarm_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info);
int jny_proc_get_video_lost_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info);
int jny_proc_set_video_lost_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info);
int jny_proc_get_record_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info);
int jny_proc_set_record_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info);
int jny_proc_add_cruise_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info);

int jny_proc_control_ptz(int user_id, void *parm, proxy_sdk_t *sdk_info);
int jny_proc_set_preset_point_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info);


int jny_proc_get_capability_list(int user_id, void *parm, proxy_sdk_t *sdk_info);



#ifdef __cplusplus
	}
#endif

#endif
