#ifndef __J_JNY_HANDLER_H__
#define __J_JNY_HANDLER_H__

typedef struct _jny_device_time
{
	unsigned short	nYearGet;
	unsigned short	nMonthGet;
	unsigned short	nDayGet;
	unsigned short	nHourGet;
	unsigned short	nMinuteGet;
	unsigned short	nSecondGet;
}jny_device_time;

typedef struct _jny_parm_info
{
	int channel;
	void *parm_ptr;
}jny_parm_info;


#ifdef __cplusplus
	extern "C" {
#endif
int jny_get_ddns_info(struct service *srv, int channel, int parm_id, void *pvalue);
int jny_set_ddns_info(struct service *srv, int channel, int parm_id, void *pvalue);
int jny_get_device_info(struct service *srv, int channel, int parm_id, void *pvalue);
int jny_get_serial_info(struct service *srv, int channel, int parm_id, void *pvalue);
int jny_set_serial_info(struct service *srv, int channel, int parm_id, void *pvalue);
int jny_get_device_time(struct service *srv, int channel, int parm_id, void *pvalue);
int jny_set_device_time(struct service *srv, int channel, int parm_id, void *pvalue);
int jny_get_ntp_info(struct service *srv, int channel, int parm_id, void *pvalue);
int jny_set_ntp_info(struct service *srv, int channel, int parm_id, void *pvalue);
int jny_get_network_info(struct service *srv, int channel, int parm_id, void *pvalue);
int jny_set_network_info(struct service *srv, int channel, int parm_id, void *pvalue);
int jny_get_pppoe_info(struct service *srv, int channel, int parm_id, void *pvalue);
int jny_set_pppoe_info(struct service *srv, int channel, int parm_id, void *pvalue);

int jny_get_upnp_info(struct service *srv, int channel, int parm_id, void *pvalue);

int jny_format_disk(struct service *srv, int channel, int parm_id, void *pvalue);
int jny_get_encode_info(struct service *srv, int channel, int parm_id, void *pvalue);
int jny_set_encode_info(struct service *srv, int channel, int parm_id, void *pvalue);
int jny_set_display_info(struct service *srv, int channel, int parm_id, void *pvalue);
int jny_get_record_info(struct service *srv, int channel, int parm_id, void *pvalue);

int jny_get_ftp_info(struct service *srv, int channel, int parm_id, void *pvalue);
int jny_set_ftp_info(struct service *srv, int channel, int parm_id, void *pvalue);
int jny_get_smtp_info(struct service *srv, int channel, int parm_id, void *pvalue);
int jny_set_smtp_info(struct service *srv, int channel, int parm_id, void *pvalue);
int jny_get_disk_list(struct service *srv, int channel, int parm_id, void *pvalue);

int jny_control_device(struct service *srv, int channel, int parm_id, void *pvalue);

int jny_get_osd_info(struct service *srv, int channel, int parm_id, void *pvalue);
int jny_set_osd_info(struct service *srv, int channel, int parm_id, void *pvalue);
int jny_get_ptz_info(struct service *srv, int channel, int parm_id, void *pvalue);
int jny_set_ptz_info(struct service *srv, int channel, int parm_id, void *pvalue);
int jny_control_ptz(struct service *srv, int channel, int parm_id, void *pvalue);
int jny_set_preset_point(struct service *srv, int channel, int parm_id, void *pvalue);

int jny_get_hide_info(struct service *srv, int channel, int parm_id, void *pvalue);
int jny_set_hide_info(struct service *srv, int channel, int parm_id, void *pvalue);
int jny_get_move_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue);
int jny_set_move_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue);
int jny_get_video_lost_info(struct service *srv, int channel, int parm_id, void *pvalue);
int jny_set_video_lost_info(struct service *srv, int channel, int parm_id, void *pvalue);
int jny_get_hide_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue);
int jny_set_hide_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue);
int jny_get_record_info(struct service *srv, int channel, int parm_id, void *pvalue);
int jny_set_record_info(struct service *srv, int channel, int parm_id, void *pvalue);
int jny_get_capability_list(struct service *srv, int channel, int parm_id, void *pvalue);
#ifdef __cplusplus
	}
#endif

#endif