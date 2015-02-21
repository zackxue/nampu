#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "config.h"
#include "nmp_xmlmsg.h"
#include "nmp_packet.h"

#ifndef __WIN_32__
	#include <pthread.h>
#endif



#define MERGE_REGISTER_REQUEST_PATH				"merge/REGISTER_REQUEST.xml"
#define MERGE_REGISTER_RESPONSE_PATH			"merge/REGISTER_RESPONSE.xml"
#define MERGE_HEART_BEAT_REQUEST_PATH			"merge/HEART_BEAT_REQUEST.xml"
#define MERGE_HEART_BEAT_RESPONSE_PATH			"merge/HEART_BEAT_RESPONSE.xml"
#define MERGE_CHANGE_DISPATCH_REQUEST_PATH		"merge/CHANGE_DISPATCH_REQUEST.xml"
#define MERGE_CHANGE_DISPATCH_RESULT_PATH		"merge/CHANGE_DISPATCH_RESULT.xml"
#define MERGE_GET_DEVICE_INFO_PATH				"merge/GET_DEVICE_INFO.xml"
#define MERGE_DEVICE_INFO_RESPONSE_PATH			"merge/DEVICE_INFO_RESPONSE.xml"
#define MERGE_GET_DEVICE_NTP_INFO_PATH			"merge/GET_DEVICE_NTP_INFO.xml"
#define MERGE_DEVICE_NTP_INFO_RESPONSE_PATH		"merge/DEVICE_NTP_INFO_RESPONSE.xml"
#define MERGE_SET_DEVICE_NTP_INFO_PATH			"merge/SET_DEVICE_NTP_INFO.xml"
#define MERGE_DEVICE_NTP_INFO_RESULT_PATH		"merge/DEVICE_NTP_INFO_RESULT.xml"
#define MERGE_SET_DEVICE_TIME_PATH				"merge/SET_DEVICE_TIME.xml"
#define MERGE_DEVICE_TIME_RESULT_PATH			"merge/DEVICE_TIME_RESULT.xml"
#define MERGE_GET_PLATFORM_INFO_PATH			"merge/GET_PLATFORM_INFO.xml"
#define MERGE_PLATFORM_INFO_RESPONSE_PATH		"merge/PLATFORM_INFO_RESPONSE.xml"
#define MERGE_SET_PLATFORM_INFO_PATH			"merge/SET_PLATFORM_INFO.xml"
#define MERGE_PLATFORM_INFO_RESULT_PATH			"merge/PLATFORM_INFO_RESULT.xml"
#define MERGE_GET_NETWORK_INFO_PATH				"merge/GET_NETWORK_INFO.xml"
#define MERGE_NETWORK_INFO_RESPONSE_PATH		"merge/NETWORK_INFO_RESPONSE.xml"
#define MERGE_SET_NETWORK_INFO_PATH				"merge/SET_NETWORK_INFO.xml"
#define MERGE_NETWORK_INFO_RESULT_PATH			"merge/NETWORK_INFO_RESULT.xml"
#define MERGE_GET_PPPOE_INFO_PATH				"merge/GET_PPPOE_INFO.xml"
#define MERGE_PPPOE_INFO_RESPONSE_PATH			"merge/PPPOE_INFO_RESPONSE.xml"
#define MERGE_SET_PPPOE_INFO_PATH				"merge/SET_PPPOE_INFO.xml"
#define MERGE_PPPOE_INFO_RESULT_PATH			"merge/PPPOE_INFO_RESULT.xml"
#define MERGE_GET_ENCODE_PARAMETER_PATH			"merge/GET_ENCODE_PARAMETER.xml"
#define MERGE_ENCODE_PARAMETER_RESPONSE_PATH	"merge/ENCODE_PARAMETER_RESPONSE.xml"
#define MERGE_SET_ENCODE_PARAMETER_PATH			"merge/SET_ENCODE_PARAMETER.xml"
#define MERGE_ENCODE_PARAMETER_RESULT_PATH		"merge/ENCODE_PARAMETER_RESULT.xml"
#define MERGE_GET_DISPLAY_PARAMETER_PATH		"merge/GET_DISPLAY_PARAMETER.xml"
#define MERGE_DISPLAY_PARAMETER_RESPONSE_PATH	"merge/DISPLAY_PARAMETER_RESPONSE.xml"
#define MERGE_SET_DISPLAY_PARAMETER_PATH		"merge/SET_DISPLAY_PARAMETER.xml"
#define MERGE_DISPLAY_PARAMETER_RESULT_PATH		"merge/DISPLAY_PARAMETER_RESULT.xml"
#define MERGE_GET_RECORD_PARAMETER_PATH			"merge/GET_RECORD_PARAMETER.xml"
#define MERGE_RECORD_PARAMETER_RESPONSE_PATH	"merge/RECORD_PARAMETER_RESPONSE.xml"
#define MERGE_SET_RECORD_PARAMETER_PATH			"merge/SET_RECORD_PARAMETER.xml"
#define MERGE_RECORD_PARAMETER_RESULT_PATH		"merge/RECORD_PARAMETER_RESULT.xml"
#define MERGE_GET_HIDE_PARAMETER_PATH			"merge/GET_HIDE_PARAMETER.xml"
#define MERGE_HIDE_PARAMETER_RESPONSE_PATH		"merge/HIDE_PARAMETER_RESPONSE.xml"
#define MERGE_SET_HIDE_PARAMETER_PATH			"merge/SET_HIDE_PARAMETER.xml"
#define MERGE_HIDE_PARAMETER_RESULT_PATH		"merge/HIDE_PARAMETER_RESULT.xml"
#define MERGE_GET_SERIAL_PARAMETER_PATH			"merge/GET_SERIAL_PARAMETER.xml"
#define MERGE_SERIAL_PARAMETER_RESPONSE_PATH	"merge/SERIAL_PARAMETER_RESPONSE.xml"
#define MERGE_SET_SERIAL_PARAMETER_PATH			"merge/SET_SERIAL_PARAMETER.xml"
#define MERGE_SERIAL_PARAMETER_RESULT_PATH		"merge/SERIAL_PARAMETER_RESULT.xml"
#define MERGE_GET_OSD_PARAMETER_PATH			"merge/GET_OSD_PARAMETER.xml"
#define MERGE_OSD_PARAMETER_RESPONSE_PATH		"merge/OSD_PARAMETER_RESPONSE.xml"
#define MERGE_SET_OSD_PARAMETER_PATH			"merge/SET_OSD_PARAMETER.xml"
#define MERGE_OSD_PARAMETER_RESULT_PATH			"merge/OSD_PARAMETER_RESULT.xml"
#define MERGE_GET_PTZ_PARAMETER_PATH			"merge/GET_PTZ_PARAMETER.xml"
#define MERGE_PTZ_PARAMETER_RESPONSE_PATH		"merge/PTZ_PARAMETER_RESPONSE.xml"
#define MERGE_SET_PTZ_PARAMETER_PATH			"merge/SET_PTZ_PARAMETER.xml"
#define MERGE_PTZ_PARAMETER_RESULT_PATH			"merge/PTZ_PARAMETER_RESULT.xml"
#define MERGE_GET_FTP_PARAMETER_PATH			"merge/GET_FTP_PARAMETER.xml"
#define MERGE_FTP_PARAMETER_RESPONSE_PATH		"merge/FTP_PARAMETER_RESPONSE.xml"
#define MERGE_SET_FTP_PARAMETER_PATH			"merge/SET_FTP_PARAMETER.xml"
#define MERGE_FTP_PARAMETER_RESULT_PATH			"merge/FTP_PARAMETER_RESULT.xml"
#define MERGE_GET_SMTP_PARAMETER_PATH			"merge/GET_SMTP_PARAMETER.xml"
#define MERGE_SMTP_PARAMETER_RESPONSE_PATH		"merge/SMTP_PARAMETER_RESPONSE.xml"
#define MERGE_SET_SMTP_PARAMETER_PATH			"merge/SET_SMTP_PARAMETER.xml"
#define MERGE_SMTP_PARAMETER_RESULT_PATH		"merge/SMTP_PARAMETER_RESULT.xml"
#define MERGE_GET_UPNP_PARAMETER_PATH			"merge/GET_UPNP_PARAMETER.xml"
#define MERGE_UPNP_PARAMETER_RESPONSE_PATH		"merge/UPNP_PARAMETER_RESPONSE.xml"
#define MERGE_SET_UPNP_PARAMETER_PATH			"merge/SET_UPNP_PARAMETER.xml"
#define MERGE_UPNP_PARAMETER_RESULT_PATH		"merge/UPNP_PARAMETER_RESULT.xml"
#define MERGE_GET_DISK_INFO_PATH				"merge/GET_DISK_INFO.xml"
#define MERGE_DISK_INFO_RESPONSE_PATH			"merge/DISK_INFO_RESPONSE.xml"
#define MERGE_FORMAT_DISK_REQUEST_PATH			"merge/FORMAT_DISK_REQUEST.xml"
#define MERGE_FORMAT_DISK_RESULT_PATH			"merge/FORMAT_DISK_RESULT.xml"
#define MERGE_GET_FORMAT_PROGRESS_PATH			"merge/GET_FORMAT_PROGRESS.xml"
#define MERGE_FORMAT_PROGRESS_RESPONSE_PATH		"merge/FORMAT_PROGRESS_RESPONSE.xml"
#define MERGE_GET_MOVE_ALARM_INFO_PATH			"merge/GET_MOVE_ALARM_INFO.xml"
#define MERGE_MOVE_ALARM_INFO_RESPONSE_PATH		"merge/MOVE_ALARM_INFO_RESPONSE.xml"
#define MERGE_SET_MOVE_ALARM_INFO_PATH			"merge/SET_MOVE_ALARM_INFO.xml"
#define MERGE_MOVE_ALARM_INFO_RESULT_PATH		"merge/MOVE_ALARM_INFO_RESULT.xml"
#define MERGE_GET_LOST_ALARM_INFO_PATH			"merge/GET_LOST_ALARM_INFO.xml"
#define MERGE_LOST_ALARM_INFO_RESPONSE_PATH		"merge/LOST_ALARM_INFO_RESPONSE.xml"
#define MERGE_SET_LOST_ALARM_INFO_PATH			"merge/SET_LOST_ALARM_INFO.xml"
#define MERGE_LOST_ALARM_INFO_RESULT_PATH		"merge/LOST_ALARM_INFO_RESULT.xml"
#define MERGE_GET_HIDE_ALARM_INFO_PATH			"merge/GET_HIDE_ALARM_INFO.xml"
#define MERGE_HIDE_ALARM_INFO_RESPONSE_PATH		"merge/HIDE_ALARM_INFO_RESPONSE.xml"
#define MERGE_SET_HIDE_ALARM_INFO_PATH			"merge/SET_HIDE_ALARM_INFO.xml"
#define MERGE_HIDE_ALARM_INFO_RESULT_PATH		"merge/HIDE_ALARM_INFO_RESULT.xml"
#define MERGE_GET_IO_ALARM_INFO_PATH			"merge/GET_IO_ALARM_INFO.xml"
#define MERGE_IO_ALARM_INFO_RESPONSE_PATH		"merge/IO_ALARM_INFO_RESPONSE.xml"
#define MERGE_SET_IO_ALARM_INFO_PATH			"merge/SET_IO_ALARM_INFO.xml"
#define MERGE_IO_ALARM_INFO_RESULT_PATH			"merge/IO_ALARM_INFO_RESULT.xml"
#define MERGE_SUBMIT_ALARM_REQUEST_PATH			"merge/SUBMIT_ALARM_REQUEST.xml"
#define MERGE_SUBMIT_ALARM_RESULT_PATH			"merge/SUBMIT_ALARM_RESULT.xml"




#define MAX_XML_LEN			4096


void *m_malloc(size_t size)
{
	/*static int flags = 0;
	static int total_size = 0;
	flags++;
	total_size += size;
	printf("calloc flags [%d], total_size [%d]\n", flags, total_size);//*/
	
	return calloc(1, size);
	//return malloc(size);
}

void f_free(void *ptr, size_t size)
{
	/*static int flags = 0;
	static int total_size = 0;
	flags++;
	total_size += size;
	printf("free flags [%d], total_size [%d]\n", flags, total_size);//*/
	
	free(ptr);
	ptr = NULL;
}

static void get_local_time(JTime *timer)
{
	struct tm *local;
	time_t t;
	
	t = time(NULL);
	local = localtime(&t);
	
	timer->year   = local->tm_year;
	timer->month  = local->tm_mon + 1;
	timer->date   = local->tm_mday;
	timer->hour   = local->tm_hour;
	timer->minute = local->tm_min;
	timer->second = local->tm_sec;
}
static int get_xml_info_from_file(const char *path, char *buffer, int buf_size)
{
	FILE *fpr = NULL;
	int file_len = 0;
	
	if (NULL == path)
	{
		printf("path NULL.\n");
		return -1;
	}
	if (0 >= buf_size)
	{
		printf("buf_size invalid.\n");
		return -1;
	}
	
	if (NULL == (fpr = fopen(path, "r")))
	{
		perror("fopen error");
		printf("[%s]\n", path);
		return -1;
	}
	else
	{
		fseek(fpr, 0, SEEK_END);
		file_len = ftell(fpr);
		fseek(fpr, 0, SEEK_SET);

		if (file_len >= buf_size)
		{
			printf("file_len >= buf_size");
			return -1;
		}
		else
		{
			fread(buffer, file_len, 1, fpr);
			
			if (0 != fclose(fpr))
				perror("close(fpr) error.\n");

			return 0;
		}
	}
}

static int create_xml_file_from_buffer(const char *buffer, size_t buf_size, const char *path)
{
	FILE *fpw = NULL;
	
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	if (0 >= buf_size)
	{
		printf("buf_size invalid.\n");
		return -1;
	}
	if (NULL == path)
	{
		printf("path NULL.\n");
		return -1;
	}
	
	if (NULL == (fpw = fopen(path, "w")))
	{
		perror("fopen error.\n");
		return -1;
	}
	else
	{
		fwrite(buffer, buf_size, 1, fpw);

		if (0 != fclose(fpw))
			perror("close(fpw) error.\n");
		
		printf("create xml file succ...\n");
		return 0;
	}
}

/*static void test_get_info(int cmd_type, const char *path)
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	Request *request = NULL;
	
	request = (Request*)j_xml_alloc(sizeof(*request));

	snprintf(request->session_id, sizeof(request->session_id), "%s", "JXJ-SESSION-ID-001");
	snprintf(request->domain_id, sizeof(request->domain_id), "%s", "JXJ-DOMAIN-ID-0001");
	snprintf(request->pu_or_gu_id, sizeof(request->pu_or_gu_id), "%s", "JXJ-PU-ID-000000000001");

	msg = jpf_xml_msg_new_2(cmd_type, request, sizeof(*request));
	j_xml_dealloc(request, sizeof(*request));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), path);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_set_result(int cmd_type, const char *path)
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	Result *result = NULL;
	
	result = (Result*)j_xml_alloc(sizeof(*result));

	snprintf(result->session_id, sizeof(result->session_id), "%s", "JXJ-SESSION-ID-001");
	snprintf(result->domain_id, sizeof(result->domain_id), "%s", "JXJ-DOMAIN-ID-0001");
	snprintf(result->pu_or_gu_id, sizeof(result->pu_or_gu_id), "%s", "JXJ-PU-ID-000000001");
	result->result.code = 1;
	
	msg = jpf_xml_msg_new_2(cmd_type, result, sizeof(*result));
	j_xml_dealloc(result, sizeof(*result));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), path);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}*/

#if 0
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
static void test_register_request()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	RegisterRequestPacket *reg_request = NULL;
		
	memset(buf, 0, sizeof(buf));
	reg_request = (RegisterRequestPacket*)j_xml_alloc(sizeof(*reg_request));

	snprintf(reg_request->pu_id, sizeof(reg_request->pu_id), "%s", "JXJ-PU-ID-000000000001");
	snprintf(reg_request->cms_ip, sizeof(reg_request->cms_ip), "%s", "192.168.1.12");
	reg_request->pu_type = 1;
	
	msg = jpf_xml_msg_new_2(REGISTER_REQUEST_ID, reg_request, sizeof(*reg_request));
	j_xml_dealloc(reg_request, sizeof(*reg_request));

	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_REGISTER_REQUEST_PATH);
	//printf("%d.buf: %s\n", ret, buf);
	jpf_xml_msg_destroy(msg);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);
	/*reg_request = (PuRegister*)msg->priv_obj;
	printf("msg->id     : %d<<<<.\n", msg->id);
	printf("msg->flags  : %d<<<<.\n", msg->flags);
	printf("reg->pu_id  : %s<<<<.\n", reg->pu_id);
	printf("reg->cms_ip : %s<<<<.\n", reg->cms_ip);
	printf("reg->pu_type: %d<<<<.\n", reg->pu_type);//*/	

	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}

static void test_register_response()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	RegisterResponsePacket *reg_response = NULL;
	
	reg_response = (RegisterResponsePacket*)j_xml_alloc(sizeof(*reg_response));

	snprintf(reg_response->pu_id, sizeof(reg_response->pu_id), "%s", "JXJ-PU-ID-000000000001");

	reg_response->result.code = 1;
	reg_response->keep_alive = 15;
	snprintf(reg_response->mds_ip, sizeof(reg_response->mds_ip), "%s", "192.168.1.12");
	reg_response->mds_port = 1234;
	
	msg = jpf_xml_msg_new_2(REGISTER_RESPONSE_ID, reg_response, sizeof(*reg_response));
	j_xml_dealloc(reg_response, sizeof(*reg_response));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_REGISTER_RESPONSE_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_heart_beat_request()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	HeartBeatRequestPacket *pu_heart = NULL;
	
	pu_heart = (HeartBeatRequestPacket*)j_xml_alloc(sizeof(*pu_heart));

	snprintf(pu_heart->pu_id, sizeof(pu_heart->pu_id), "%s", "JXJ-PU-ID-000000000001");
	
	msg = jpf_xml_msg_new_2(HEART_BEAT_REQUEST_ID, pu_heart, sizeof(*pu_heart));
	j_xml_dealloc(pu_heart, sizeof(*pu_heart));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_HEART_BEAT_REQUEST_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_heart_beat_response()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	HeartBeatResponsePacket *hrt_beat_resp = NULL;
	
	hrt_beat_resp = (HeartBeatResponsePacket*)j_xml_alloc(sizeof(*hrt_beat_resp));

	hrt_beat_resp->result.code = 1;
	get_local_time(&(hrt_beat_resp->server_time));
	
	msg = jpf_xml_msg_new_2(HEART_BEAT_RESPONSE_ID, hrt_beat_resp, sizeof(*hrt_beat_resp));
	j_xml_dealloc(hrt_beat_resp, sizeof(*hrt_beat_resp));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_HEART_BEAT_RESPONSE_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_change_dispatch_request()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	ChangeDispatchPacket *change_disp = NULL;
	
	change_disp = (ChangeDispatchPacket*)j_xml_alloc(sizeof(*change_disp));

	snprintf(change_disp->session_id, sizeof(change_disp->session_id), "%s", "JXJ-SESSION-ID-001");
	snprintf(change_disp->domain_id, sizeof(change_disp->domain_id), "%s", "JXJ-DOMAIN-ID-0001");
	snprintf(change_disp->pu_id, sizeof(change_disp->pu_id), "%s", "JXJ-PU-ID-000000000001");
	snprintf(change_disp->mds_ip, sizeof(change_disp->mds_ip), "%s", "192.168.1.12");
	change_disp->mds_port = 4321;
	
	msg = jpf_xml_msg_new_2(CHANGE_DISPATCH_REQUEST_ID, change_disp, sizeof(*change_disp));
	j_xml_dealloc(change_disp, sizeof(*change_disp));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_CHANGE_DISPATCH_REQUEST_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_change_dispatch_result()
{
	test_set_result(CHANGE_DISPATCH_RESULT_ID, MERGE_CHANGE_DISPATCH_RESULT_PATH);
}
static void test_get_device_info()
{
	test_get_info(GET_DEVICE_INFO_ID, MERGE_GET_DEVICE_INFO_PATH);
}
static void test_device_info_response()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	Request *request = NULL;
	DeviceInfoPacket *dev_info = NULL;
	
	dev_info = (DeviceInfoPacket*)j_xml_alloc(sizeof(*dev_info));
	
	snprintf(dev_info->session_id, sizeof(dev_info->session_id), "%s", "JXJ-SESSION-ID-001");
	snprintf(dev_info->domain_id, sizeof(dev_info->domain_id), "%s", "JXJ-DOMAIN-ID-0001");
	snprintf(dev_info->pu_id, sizeof(dev_info->pu_id), "%s", "JXJ-PU-ID-000000000001");	
	snprintf(dev_info->manu_info, sizeof(dev_info->manu_info), "%s", "JXJ-PU");
	snprintf(dev_info->release_date, sizeof(dev_info->release_date), "%s", "JXJ-PU-RELEASE-DATE-20111210");
	snprintf(dev_info->dev_version, sizeof(dev_info->dev_version), "%s", "JXJ-PU-SV-001");
	snprintf(dev_info->hw_version, sizeof(dev_info->hw_version), "%s", "JXJ-PU-HV-001");
	
	dev_info->result.code = 1;
	dev_info->pu_type = 1;
	dev_info->sub_type = 1;
	dev_info->di_num = 1;
	dev_info->do_num = 1;
	dev_info->channel_num = 1;
	dev_info->rs485_num = 1;
	dev_info->rs232_num = 1;	
	
	msg = jpf_xml_msg_new_2(DEVICE_INFO_RESPONSE_ID, dev_info, sizeof(*dev_info));
	j_xml_dealloc(dev_info, sizeof(*dev_info));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), "./get_device_info.xml");
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_get_device_ntp_info()
{
	test_get_info(GET_DEVICE_NTP_INFO_ID, MERGE_GET_DEVICE_NTP_INFO_PATH);
}
static void test_device_ntp_info_response()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	DeviceNTPInfo dev_ntp_info;
	DeviceNTPInfoPacket *ntp_info = NULL;
	
	ntp_info = (DeviceNTPInfoPacket*)j_xml_alloc(sizeof(*ntp_info));

	snprintf(ntp_info->session_id, sizeof(ntp_info->session_id), "%s", "JXJ-SESSION-ID-001");
	snprintf(ntp_info->domain_id, sizeof(ntp_info->domain_id), "%s", "JXJ-DOMAIN-ID-0001");
	snprintf(ntp_info->pu_id, sizeof(ntp_info->pu_id), "%s", "JXJ-PU-ID-000000000001");
	snprintf(ntp_info->ntp_server_ip, sizeof(ntp_info->ntp_server_ip), "%s", "192.168.1.12");
	
	ntp_info->result.code = 1;
	ntp_info->time_zone = 8;
	ntp_info->time_interval = 30;
	ntp_info->ntp_enable = 1;
	ntp_info->dst_enable = 1;
	ntp_info->reserve = 0;
	
	msg = jpf_xml_msg_new_2(DEVICE_NTP_INFO_RESPONSE_ID, ntp_info, sizeof(*ntp_info));
	j_xml_dealloc(ntp_info, sizeof(*ntp_info));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_DEVICE_NTP_INFO_RESPONSE_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);
	//ntp_info = (DeviceNTPInfoPacket*)msg->priv_obj;
	//printf("session_id: %s\n", ntp_info->session_id);
	//printf("dst_enable: %d\n", ntp_info->dst_enable);

	//process_device_ntp_info(ntp_info, &dev_ntp_info, 0);
	//process_device_param(DEVICE_NTP_INFO_RESPONSE, 0, &dev_ntp_info);

	/*DevicePlatformInfo dev_pltf_info;
	set_device_platform_info("JXJ-PU-ID-000000000001", "192.168.1.12", 4123);
	get_device_platform_info(&dev_pltf_info);
	printf("session_id: %s\n", dev_pltf_info.pu_id);
	printf("session_id: %s\n", dev_pltf_info.cms_ip);
	printf("session_id: %d\n", dev_pltf_info.cms_port);
	exit(0);//*/
	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_set_device_ntp_info()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	DeviceNTPInfoPacket *ntp_info = NULL;
	
	ntp_info = (DeviceNTPInfoPacket*)j_xml_alloc(sizeof(*ntp_info));

	snprintf(ntp_info->session_id, sizeof(ntp_info->session_id), "%s", "JXJ-SESSION-ID-001");
	snprintf(ntp_info->domain_id, sizeof(ntp_info->domain_id), "%s", "JXJ-DOMAIN-ID-0001");
	snprintf(ntp_info->pu_id, sizeof(ntp_info->pu_id), "%s", "JXJ-PU-ID-000000000001");
	snprintf(ntp_info->ntp_server_ip, sizeof(ntp_info->ntp_server_ip), "%s", "192.168.1.12");
	
	ntp_info->result.code = 1;
	ntp_info->time_zone = 8;
	ntp_info->time_interval = 30;
	ntp_info->ntp_enable = 1;
	ntp_info->dst_enable = 1;
	ntp_info->reserve = 0;
	
	msg = jpf_xml_msg_new_2(SET_DEVICE_NTP_INFO_ID, ntp_info, sizeof(*ntp_info));
	j_xml_dealloc(ntp_info, sizeof(*ntp_info));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_SET_DEVICE_NTP_INFO_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_device_ntp_info_result()
{
	test_set_result(DEVICE_NTP_INFO_RESULT_ID, MERGE_DEVICE_NTP_INFO_RESULT_PATH);
}
static void test_set_device_time()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	DeviceTimePacket *dev_time = NULL;
	
	dev_time = (DeviceTimePacket*)j_xml_alloc(sizeof(*dev_time));

	snprintf(dev_time->session_id, sizeof(dev_time->session_id), "%s", "JXJ-SESSION-ID-001");
	snprintf(dev_time->domain_id, sizeof(dev_time->domain_id), "%s", "JXJ-DOMAIN-ID-0001");
	snprintf(dev_time->pu_id, sizeof(dev_time->pu_id), "%s", "JXJ-PU-ID-000000000001");
	
	get_local_time(&(dev_time->server_time));
	dev_time->time_zone = 8;
	dev_time->sync_enable = 1;
	
	msg = jpf_xml_msg_new_2(SET_DEVICE_TIME_ID, dev_time, sizeof(*dev_time));
	j_xml_dealloc(dev_time, sizeof(*dev_time));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_SET_DEVICE_TIME_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_device_time_result()
{
	test_set_result(DEVICE_TIME_RESULT_ID, MERGE_DEVICE_TIME_RESULT_PATH);
}
static void test_get_platform_info()
{
	test_get_info(GET_PLATFORM_INFO_ID, MERGE_GET_PLATFORM_INFO_PATH);
}
static void test_platform_info_response()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	PlatformInfoPacket *pltf_info = NULL;
	
	pltf_info = (PlatformInfoPacket*)j_xml_alloc(sizeof(*pltf_info));

	snprintf(pltf_info->session_id, sizeof(pltf_info->session_id), "%s", "JXJ-SESSION-ID-001");
	snprintf(pltf_info->domain_id, sizeof(pltf_info->domain_id), "%s", "JXJ-DOMAIN-ID-0001");
	snprintf(pltf_info->pu_id, sizeof(pltf_info->pu_id), "%s", "JXJ-PU-ID-000000000001");
	snprintf(pltf_info->cms_ip, sizeof(pltf_info->cms_ip), "%s", "192.168.1.12");
	snprintf(pltf_info->mds_ip, sizeof(pltf_info->mds_ip), "%s", "192.168.1.12");
	
	pltf_info->result.code = 1;
	pltf_info->cms_port = 4321;
	pltf_info->mds_port = 4321;
	pltf_info->protocol = 1;
	
	msg = jpf_xml_msg_new_2(PLATFORM_INFO_RESPONSE_ID, pltf_info, sizeof(*pltf_info));
	j_xml_dealloc(pltf_info, sizeof(*pltf_info));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_PLATFORM_INFO_RESPONSE_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_set_platform_info()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	PlatformInfoPacket *pltf_info = NULL;
	
	pltf_info = (PlatformInfoPacket*)j_xml_alloc(sizeof(*pltf_info));

	snprintf(pltf_info->session_id, sizeof(pltf_info->session_id), "%s", "JXJ-SESSION-ID-001");
	snprintf(pltf_info->domain_id, sizeof(pltf_info->domain_id), "%s", "JXJ-DOMAIN-ID-0001");
	snprintf(pltf_info->pu_id, sizeof(pltf_info->pu_id), "%s", "JXJ-PU-ID-000000000001");
	snprintf(pltf_info->cms_ip, sizeof(pltf_info->cms_ip), "%s", "192.168.1.12");
	snprintf(pltf_info->mds_ip, sizeof(pltf_info->mds_ip), "%s", "192.168.1.12");
	
	pltf_info->result.code = 1;
	pltf_info->cms_port = 4321;
	pltf_info->mds_port = 4321;
	pltf_info->protocol = 1;
	
	msg = jpf_xml_msg_new_2(SET_PLATFORM_INFO_ID, pltf_info, sizeof(*pltf_info));
	j_xml_dealloc(pltf_info, sizeof(*pltf_info));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_SET_PLATFORM_INFO_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_platform_info_result()
{
	test_set_result(PLATFORM_INFO_RESULT_ID, MERGE_PLATFORM_INFO_RESULT_PATH);
}
static void test_get_network_info()
{
	test_get_info(GET_NETWORK_INFO_ID, MERGE_GET_NETWORK_INFO_PATH);
}
static void test_network_info_response()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	NetworkInfoPacket *net_info = NULL;
	
	net_info = (NetworkInfoPacket*)j_xml_alloc(sizeof(*net_info));

	snprintf(net_info->session_id, sizeof(net_info->session_id), "%s", "JXJ-SESSION-ID-001");
	snprintf(net_info->domain_id, sizeof(net_info->domain_id), "%s", "JXJ-DOMAIN-ID-0001");
	snprintf(net_info->pu_id, sizeof(net_info->pu_id), "%s", "JXJ-PU-ID-000000000001");
	snprintf(net_info->network[J_SDK_ETH0].ip, sizeof(net_info->network[J_SDK_ETH0].ip), "%s", "192.168.1.12");
	snprintf(net_info->network[J_SDK_ETH0].netmask, sizeof(net_info->network[J_SDK_ETH0].netmask), "%s", "255.255.255.255");
	snprintf(net_info->network[J_SDK_ETH0].gateway, sizeof(net_info->network[J_SDK_ETH0].gateway), "%s", "255.255.255.255");
	snprintf(net_info->network[J_SDK_WIFI].ip, sizeof(net_info->network[J_SDK_WIFI].ip), "%s", "192.168.1.12");
	snprintf(net_info->network[J_SDK_WIFI].netmask, sizeof(net_info->network[J_SDK_WIFI].netmask), "%s", "255.255.255.255");
	snprintf(net_info->network[J_SDK_WIFI].gateway, sizeof(net_info->network[J_SDK_WIFI].gateway), "%s", "255.255.255.255");
	snprintf(net_info->network[J_SDK_3G].ip, sizeof(net_info->network[J_SDK_3G].ip), "%s", "192.168.1.12");
	snprintf(net_info->network[J_SDK_3G].netmask, sizeof(net_info->network[J_SDK_3G].netmask), "%s", "255.255.255.255");
	snprintf(net_info->network[J_SDK_3G].gateway, sizeof(net_info->network[J_SDK_3G].gateway), "%s", "255.255.255.255");
	snprintf(net_info->dns, sizeof(net_info->dns), "%s", "255.255.255.255");
	
	net_info->network[J_SDK_ETH0].dhcp_enable = 1;
	net_info->network[J_SDK_WIFI].dhcp_enable = 1;
	net_info->network[J_SDK_3G].dhcp_enable = 1;
	net_info->auto_dns_enable = 1;
	net_info->server_port = 4321;
	net_info->web_port = 4322;
	
	msg = jpf_xml_msg_new_2(NETWORK_INFO_RESPONSE_ID, net_info, sizeof(*net_info));
	j_xml_dealloc(net_info, sizeof(*net_info));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_NETWORK_INFO_RESPONSE_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_set_network_info()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	NetworkInfoPacket *net_info = NULL;
	
	net_info = (NetworkInfoPacket*)j_xml_alloc(sizeof(*net_info));

	snprintf(net_info->session_id, sizeof(net_info->session_id), "%s", "JXJ-SESSION-ID-001");
	snprintf(net_info->domain_id, sizeof(net_info->domain_id), "%s", "JXJ-DOMAIN-ID-0001");
	snprintf(net_info->pu_id, sizeof(net_info->pu_id), "%s", "JXJ-PU-ID-000000000001");
	snprintf(net_info->network[J_SDK_ETH0].ip, sizeof(net_info->network[J_SDK_ETH0].ip), "%s", "192.168.1.12");
	snprintf(net_info->network[J_SDK_ETH0].netmask, sizeof(net_info->network[J_SDK_ETH0].netmask), "%s", "255.255.255.255");
	snprintf(net_info->network[J_SDK_ETH0].gateway, sizeof(net_info->network[J_SDK_ETH0].gateway), "%s", "255.255.255.255");
	snprintf(net_info->network[J_SDK_WIFI].ip, sizeof(net_info->network[J_SDK_WIFI].ip), "%s", "192.168.1.12");
	snprintf(net_info->network[J_SDK_WIFI].netmask, sizeof(net_info->network[J_SDK_WIFI].netmask), "%s", "255.255.255.255");
	snprintf(net_info->network[J_SDK_WIFI].gateway, sizeof(net_info->network[J_SDK_WIFI].gateway), "%s", "255.255.255.255");
	snprintf(net_info->network[J_SDK_3G].ip, sizeof(net_info->network[J_SDK_3G].ip), "%s", "192.168.1.12");
	snprintf(net_info->network[J_SDK_3G].netmask, sizeof(net_info->network[J_SDK_3G].netmask), "%s", "255.255.255.255");
	snprintf(net_info->network[J_SDK_3G].gateway, sizeof(net_info->network[J_SDK_3G].gateway), "%s", "255.255.255.255");
	snprintf(net_info->dns, sizeof(net_info->dns), "%s", "255.255.255.255");

	net_info->network[J_SDK_ETH0].dhcp_enable = 1;
	net_info->network[J_SDK_WIFI].dhcp_enable = 1;
	net_info->network[J_SDK_3G].dhcp_enable = 1;

	net_info->auto_dns_enable = 1;
	net_info->server_port = 4321;
	net_info->web_port = 4322;
	
	msg = jpf_xml_msg_new_2(SET_NETWORK_INFO_ID, net_info, sizeof(*net_info));
	j_xml_dealloc(net_info, sizeof(*net_info));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_SET_NETWORK_INFO_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);

	NetworkInfoPacket *packet = NULL;
	packet = (NetworkInfoPacket*)XML_MSG_DATA(msg);
	
printf("result     : %d\n", packet->result);
printf("auto_dns   : %d\n", packet->auto_dns_enable);
printf("cur_network: %d\n", packet->cur_network);
printf("server_port: %d\n", packet->server_port);
printf("web_port   : %d\n", packet->web_port);
	
printf("dns        : %s\n", packet->dns);

printf("type       : %d\n", packet->network[J_SDK_ETH0].type);
printf("dhcp_enable: %d\n", packet->network[J_SDK_ETH0].dhcp_enable);
printf("ip         : %s\n", packet->network[J_SDK_ETH0].ip);
printf("netmask    : %s\n", packet->network[J_SDK_ETH0].netmask);
printf("gateway    : %s\n", packet->network[J_SDK_ETH0].gateway);
	
printf("type	   : %d\n", packet->network[J_SDK_WIFI].type);
printf("dhcp_enable: %d\n", packet->network[J_SDK_WIFI].dhcp_enable);
printf("ip		   : %s\n", packet->network[J_SDK_WIFI].ip);
printf("netmask    : %s\n", packet->network[J_SDK_WIFI].netmask);
printf("gateway    : %s\n", packet->network[J_SDK_WIFI].gateway);
	
printf("type	   : %d\n", packet->network[J_SDK_3G].type);
printf("dhcp_enable: %d\n", packet->network[J_SDK_3G].dhcp_enable);
printf("ip		   : %s\n", packet->network[J_SDK_3G].ip);
printf("netmask    : %s\n", packet->network[J_SDK_3G].netmask);
printf("gateway    : %s\n", packet->network[J_SDK_3G].gateway);

	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_network_info_result()
{
	test_set_result(NETWORK_INFO_RESULT_ID, MERGE_NETWORK_INFO_RESULT_PATH);
}
static void test_get_pppoe_info()
{
	test_get_info(GET_PPPOE_INFO_ID, MERGE_GET_PPPOE_INFO_PATH);
}
static void test_pppoe_info_response()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	PPPOEInfoPacket *pppoe_info = NULL;
	
	pppoe_info = (PPPOEInfoPacket*)j_xml_alloc(sizeof(*pppoe_info));

	snprintf(pppoe_info->session_id, sizeof(pppoe_info->session_id), "%s", "JXJ-SESSION-ID-001");
	snprintf(pppoe_info->domain_id, sizeof(pppoe_info->domain_id), "%s", "JXJ-DOMAIN-ID-0001");
	snprintf(pppoe_info->pu_id, sizeof(pppoe_info->pu_id), "%s", "JXJ-PU-ID-000000000001");
	snprintf(pppoe_info->account, sizeof(pppoe_info->account), "%s", "admin-001");
	snprintf(pppoe_info->passwd, sizeof(pppoe_info->passwd), "%s", "admin-001");
	pppoe_info->result.code = 1;
	pppoe_info->type = 1;
	
	msg = jpf_xml_msg_new_2(PPPOE_INFO_RESPONSE_ID, pppoe_info, sizeof(*pppoe_info));
	j_xml_dealloc(pppoe_info, sizeof(*pppoe_info));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_PPPOE_INFO_RESPONSE_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_set_pppoe_info()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	PPPOEInfoPacket *pppoe_info = NULL;
	
	pppoe_info = (PPPOEInfoPacket*)j_xml_alloc(sizeof(*pppoe_info));

	snprintf(pppoe_info->session_id, sizeof(pppoe_info->session_id), "%s", "JXJ-SESSION-ID-001");
	snprintf(pppoe_info->domain_id, sizeof(pppoe_info->domain_id), "%s", "JXJ-DOMAIN-ID-0001");
	snprintf(pppoe_info->pu_id, sizeof(pppoe_info->pu_id), "%s", "JXJ-PU-ID-000000000001");
	snprintf(pppoe_info->account, sizeof(pppoe_info->account), "%s", "admin-001");
	snprintf(pppoe_info->passwd, sizeof(pppoe_info->passwd), "%s", "admin-001");
	pppoe_info->result.code = 1;
	pppoe_info->type = 1;
	
	msg = jpf_xml_msg_new_2(SET_PPPOE_INFO_ID, pppoe_info, sizeof(*pppoe_info));
	j_xml_dealloc(pppoe_info, sizeof(*pppoe_info));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_SET_PPPOE_INFO_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_pppoe_info_result()
{
	test_set_result(PPPOE_INFO_RESULT_ID, MERGE_PPPOE_INFO_RESULT_PATH);
}
static void test_get_encode_parameter()
{
	test_get_info(GET_ENCODE_PARAMETER_ID, MERGE_GET_ENCODE_PARAMETER_PATH);
}
static void get_encode_parameter(EncodeParameterPacket *encode_para)
{
	snprintf(encode_para->session_id, sizeof(encode_para->session_id), "%s", "JXJ-SESSION-ID-001");
	snprintf(encode_para->domain_id, sizeof(encode_para->domain_id), "%s", "JXJ-DOMAIN-ID-0001");
	snprintf(encode_para->gu_id, sizeof(encode_para->gu_id), "%s", "JXJ-PU-ID-000000000001");
	encode_para->result.code = 1;
	encode_para->frame_rate = 1;
	encode_para->i_frame_interval = 1;
	encode_para->video_type = 1;
	encode_para->audio_type = 1;
	encode_para->audio_enble = 1;
	encode_para->resolution = 1;
	encode_para->qp_value = 1;
	encode_para->code_rate = 1;
	encode_para->frame_priority = 1;
	encode_para->format = 1;
	encode_para->bit_rate = 1;
}
static void test_encode_parameter_response()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;	
	Request *request = NULL;
	EncodeParameterPacket *encode_para = NULL;
	
	encode_para = (EncodeParameterPacket*)j_xml_alloc(sizeof(*encode_para));	
	get_encode_parameter(encode_para);
	
	msg = jpf_xml_msg_new_2(ENCODE_PARAMETER_RESPONSE_ID, encode_para, sizeof(*encode_para));
	j_xml_dealloc(encode_para, sizeof(*encode_para));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_ENCODE_PARAMETER_RESPONSE_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_set_encode_parameter()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	EncodeParameterPacket *encode_para = NULL;
	
	encode_para = (EncodeParameterPacket*)j_xml_alloc(sizeof(*encode_para));
	get_encode_parameter(encode_para);
	
	msg = jpf_xml_msg_new_2(SET_ENCODE_PARAMETER_ID, encode_para, sizeof(*encode_para));
	j_xml_dealloc(encode_para, sizeof(*encode_para));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_SET_ENCODE_PARAMETER_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_encode_parameter_result()
{
	test_set_result(ENCODE_PARAMETER_RESULT_ID, MERGE_ENCODE_PARAMETER_RESULT_PATH);
}
static void test_get_display_parameter()
{
	test_get_info(GET_DISPLAY_PARAMETER_ID, MERGE_GET_DISPLAY_PARAMETER_PATH);
}
static void get_display_parameter(DisplayParameterPacket *display_para)
{
	snprintf(display_para->session_id, sizeof(display_para->session_id), "%s", "JXJ-SESSION-ID-001");
	snprintf(display_para->domain_id, sizeof(display_para->domain_id), "%s", "JXJ-DOMAIN-ID-0001");
	snprintf(display_para->gu_id, sizeof(display_para->gu_id), "%s", "JXJ-PU-ID-000000000001");
	display_para->result.code = 1;
	display_para->contrast = 1;
	display_para->bright = 1;
	display_para->hue = 1;
	display_para->saturation = 1;
	display_para->sharpness = 1;
}
static void test_display_parameter_response()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	DisplayParameterPacket *display_para = NULL;
	
	display_para = (DisplayParameterPacket*)j_xml_alloc(sizeof(*display_para));
	get_display_parameter(display_para);
	
	msg = jpf_xml_msg_new_2(DISPLAY_PARAMETER_RESPONSE_ID, display_para, sizeof(*display_para));
	j_xml_dealloc(display_para, sizeof(*display_para));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_DISPLAY_PARAMETER_RESPONSE_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_set_display_parameter()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	DisplayParameterPacket *display_para = NULL;
	
	display_para = (DisplayParameterPacket*)j_xml_alloc(sizeof(*display_para));
	get_display_parameter(display_para);
	
	msg = jpf_xml_msg_new_2(SET_DISPLAY_PARAMETER_ID, display_para, sizeof(*display_para));
	j_xml_dealloc(display_para, sizeof(*display_para));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_SET_DISPLAY_PARAMETER_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_display_parameter_result()
{
	test_set_result(DISPLAY_PARAMETER_RESULT_ID, MERGE_DISPLAY_PARAMETER_RESULT_PATH);
}

static void test_get_record_parameter()
{
	test_get_info(GET_RECORD_PARAMETER_ID, MERGE_GET_RECORD_PARAMETER_PATH);
}
static void get_record_parameter(RecordParameterPacket *record_para)
{
	JWeek *week;
	snprintf(record_para->session_id, sizeof(record_para->session_id), "%s", "JXJ-SESSION-ID-001");
	snprintf(record_para->domain_id, sizeof(record_para->domain_id), "%s", "JXJ-DOMAIN-ID-0001");
	snprintf(record_para->gu_id, sizeof(record_para->gu_id), "%s", "JXJ-PU-ID-000000000001");
	record_para->result.code = 1;
	record_para->pre_record = 1;
	record_para->auto_cover = 1;

	week = &record_para->week;
	week->count = 2;
	
	week->days[0].day_id = 1;
	week->days[0].all_day_enable = 1;
	week->days[0].count = 2;
	week->days[0].seg[0].enable = 1;
	week->days[0].seg[1].enable = 1;
	get_local_time(&(week->days[0].seg[0].time_start));
	get_local_time(&(week->days[0].seg[0].time_end));
	get_local_time(&(week->days[0].seg[1].time_start));
	get_local_time(&(week->days[0].seg[1].time_end));
	
	week->days[1].day_id = 2;
	week->days[1].all_day_enable = 1;
	week->days[1].count = 2;
	week->days[1].seg[0].enable = 1;
	week->days[1].seg[1].enable = 1;
	get_local_time(&(week->days[1].seg[0].time_start));
	get_local_time(&(week->days[1].seg[0].time_end));
	get_local_time(&(week->days[1].seg[1].time_start));
	get_local_time(&(week->days[1].seg[1].time_end));
}
static void test_record_parameter_response()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	RecordParameterPacket *record = NULL;
	RecordParameterPacket *record_para = NULL;
	
	record_para = (RecordParameterPacket*)j_xml_alloc(sizeof(*record_para));
	get_record_parameter(record_para);
	
	msg = jpf_xml_msg_new_2(RECORD_PARAMETER_RESPONSE_ID, record_para, sizeof(*record_para));
	j_xml_dealloc(record_para, sizeof(*record_para));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_RECORD_PARAMETER_RESPONSE_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);*/
	
		
	msg = parse_xml(buf, sizeof(buf), &err, 0);
	
	record = XML_MSG_DATA(msg);
	printf("record->result.code : %d\n", record->result.code);
	printf("record->session_id  : %s\n", record->session_id);
	printf("record->domain_id   : %s\n", record->domain_id);
	printf("record->pu_id       : %s\n", record->gu_id);
	printf("record->pre_record  : %d\n", record->pre_record);
	printf("record->auto_cover  : %d\n", record->auto_cover);
	
	printf("day_count      : %d\n", record->week.count);

	int day, seg;
	for (day=0; day<record->week.count; day++)
	{
		printf("day_id        : %d\n", record->week.days[day].day_id);
		printf("    all_day_enable : %d\n", record->week.days[day].all_day_enable);
		printf("    seg_count      : %d\n", record->week.days[day].count);

		for (seg=0; seg<record->week.days[day].count; seg++)
		{
			printf("        day_seg_enable : %d\n", record->week.days[day].seg[seg].enable);
			show_time_part(&(record_para->week.days[day].seg[seg].time_start), 
				&(record_para->week.days[day].seg[seg].time_end));
		}
	}
	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_set_record_parameter()
{return ;
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	RecordParameterPacket *record_para = NULL;
	
	record_para = (RecordParameterPacket*)j_xml_alloc(sizeof(*record_para));
	get_record_parameter(record_para);
	
	msg = jpf_xml_msg_new_2(SET_RECORD_PARAMETER_ID, record_para, sizeof(*record_para));
	j_xml_dealloc(record_para, sizeof(*record_para));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_SET_RECORD_PARAMETER_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_record_parameter_result()
{
	test_set_result(RECORD_PARAMETER_RESULT_ID, MERGE_RECORD_PARAMETER_RESULT_PATH);
}
static void test_get_hide_parameter()
{
	test_get_info(GET_HIDE_PARAMETER_ID, MERGE_GET_HIDE_PARAMETER_PATH);
}
static void get_hide_parameter(HideParameterPacket *hide_para)
{
	snprintf(hide_para->session_id, sizeof(hide_para->session_id), "%s", "JXJ-SESSION-ID-001");
	snprintf(hide_para->domain_id, sizeof(hide_para->domain_id), "%s", "JXJ-DOMAIN-ID-0001");
	snprintf(hide_para->gu_id, sizeof(hide_para->gu_id), "%s", "JXJ-PU-ID-000000000001");
	hide_para->result.code = 1;

	//hide_para->hide_area = 1;
	hide_para->hide_color = 1;
}
static void test_hide_parameter_response()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	HideParameterPacket *hide_para = NULL;
	
	hide_para = (HideParameterPacket*)j_xml_alloc(sizeof(*hide_para));
	get_hide_parameter(hide_para);
	
	msg = jpf_xml_msg_new_2(HIDE_PARAMETER_RESPONSE_ID, hide_para, sizeof(*hide_para));
	j_xml_dealloc(hide_para, sizeof(*hide_para));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_HIDE_PARAMETER_RESPONSE_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_set_hide_parameter()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	HideParameterPacket *hide_para = NULL;
	
	hide_para = (HideParameterPacket*)j_xml_alloc(sizeof(*hide_para));
	get_hide_parameter(hide_para);
	
	msg = jpf_xml_msg_new_2(SET_HIDE_PARAMETER_ID, hide_para, sizeof(*hide_para));
	j_xml_dealloc(hide_para, sizeof(*hide_para));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_SET_HIDE_PARAMETER_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_hide_parameter_result()
{
	test_set_result(HIDE_PARAMETER_RESULT_ID, MERGE_HIDE_PARAMETER_RESULT_PATH);
}
static void test_get_serial_parameter()
{
	test_get_info(GET_SERIAL_PARAMETER_ID, MERGE_GET_SERIAL_PARAMETER_PATH);
}
static void get_serial_parameter(SerialParameterPacket *serial_para)
{
	snprintf(serial_para->session_id, sizeof(serial_para->session_id), "%s", "JXJ-SESSION-ID-001");
	snprintf(serial_para->domain_id, sizeof(serial_para->domain_id), "%s", "JXJ-DOMAIN-ID-0001");
	snprintf(serial_para->pu_id, sizeof(serial_para->pu_id), "%s", "JXJ-PU-ID-000000000001");
	snprintf(serial_para->serial_id, sizeof(serial_para->serial_id), "%s", "JXJ-SERIAL-ID-001");
	serial_para->result.code = 1;
	serial_para->baud_rate = 1;
	serial_para->data_bit = 1;
	serial_para->stop_bit = 1;
	serial_para->verify = 1;
}
static void test_serial_parameter_response()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	SerialParameterPacket *serial_para = NULL;
	
	serial_para = (SerialParameterPacket*)j_xml_alloc(sizeof(*serial_para));
	get_serial_parameter(serial_para);
	
	msg = jpf_xml_msg_new_2(SERIAL_PARAMETER_RESPONSE_ID, serial_para, sizeof(*serial_para));
	j_xml_dealloc(serial_para, sizeof(*serial_para));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_SERIAL_PARAMETER_RESPONSE_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_set_serial_parameter()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	SerialParameterPacket *serial_para = NULL;
	
	serial_para = (SerialParameterPacket*)j_xml_alloc(sizeof(*serial_para));
	get_serial_parameter(serial_para);
	
	msg = jpf_xml_msg_new_2(SET_SERIAL_PARAMETER_ID, serial_para, sizeof(*serial_para));
	j_xml_dealloc(serial_para, sizeof(*serial_para));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_SET_SERIAL_PARAMETER_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_serial_parameter_result()
{
	test_set_result(SERIAL_PARAMETER_RESULT_ID, MERGE_SERIAL_PARAMETER_RESULT_PATH);
}
static void test_get_osd_parameter()
{
	test_get_info(GET_OSD_PARAMETER_ID, MERGE_GET_OSD_PARAMETER_PATH);
}
static void get_osd_parameter(OSDParameterPacket *osd_para)
{
	snprintf(osd_para->session_id, sizeof(osd_para->session_id), "%s", "JXJ-SESSION-ID-001");
	snprintf(osd_para->domain_id, sizeof(osd_para->domain_id), "%s", "JXJ-DOMAIN-ID-0001");
	snprintf(osd_para->gu_id, sizeof(osd_para->gu_id), "%s", "JXJ-PU-ID-000000000001");
	osd_para->result.code = 1;
	osd_para->time_enable = 1;
	osd_para->text_display_x = 1;
	osd_para->text_display_y = 1;
	osd_para->text_display_color = 1;
	osd_para->text_enable = 1;
	osd_para->text_display_x = 1;
	osd_para->text_display_y = 1;
	osd_para->text_display_color = 1;
}
static void test_osd_parameter_response()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	OSDParameterPacket *osd_para = NULL;
	
	osd_para = (OSDParameterPacket*)j_xml_alloc(sizeof(*osd_para));
	get_osd_parameter(osd_para);
	
	msg = jpf_xml_msg_new_2(OSD_PARAMETER_RESPONSE_ID, osd_para, sizeof(*osd_para));
	j_xml_dealloc(osd_para, sizeof(*osd_para));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_OSD_PARAMETER_RESPONSE_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_set_osd_parameter()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	OSDParameterPacket *osd_para = NULL;
	
	osd_para = (OSDParameterPacket*)j_xml_alloc(sizeof(*osd_para));
	get_osd_parameter(osd_para);
	
	msg = jpf_xml_msg_new_2(SET_OSD_PARAMETER_ID, osd_para, sizeof(*osd_para));
	j_xml_dealloc(osd_para, sizeof(*osd_para));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_SET_OSD_PARAMETER_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_osd_parameter_result()
{
	test_set_result(OSD_PARAMETER_RESULT_ID, MERGE_OSD_PARAMETER_RESULT_PATH);
}
static void test_get_ptz_parameter()
{
	test_get_info(GET_PTZ_PARAMETER_ID, MERGE_GET_PTZ_PARAMETER_PATH);
}
static void get_ptz_parameter(PTZParameterPacket *ptz_para)
{
	snprintf(ptz_para->session_id, sizeof(ptz_para->session_id), "%s", "JXJ-SESSION-ID-001");
	snprintf(ptz_para->domain_id, sizeof(ptz_para->domain_id), "%s", "JXJ-DOMAIN-ID-0001");
	snprintf(ptz_para->pu_id, sizeof(ptz_para->pu_id), "%s", "JXJ-PU-ID-000000000001");
	//snprintf(ptz_para->ptz_addr, sizeof(ptz_para->ptz_addr), "%s", "JXJ-SERIAL-ID-001");
	ptz_para->ptz_addr = 1;
	ptz_para->result.code = 1;
	ptz_para->protocol = 1;
}
static void test_ptz_parameter_response()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	PTZParameterPacket *ptz_para = NULL;
	
	ptz_para = (PTZParameterPacket*)j_xml_alloc(sizeof(*ptz_para));
	get_ptz_parameter(ptz_para);
	
	msg = jpf_xml_msg_new_2(PTZ_PARAMETER_RESPONSE_ID, ptz_para, sizeof(*ptz_para));
	j_xml_dealloc(ptz_para, sizeof(*ptz_para));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), "./test.xml");
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_set_ptz_parameter()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	PTZParameterPacket *ptz_para = NULL;
	
	ptz_para = (PTZParameterPacket*)j_xml_alloc(sizeof(*ptz_para));
	get_ptz_parameter(ptz_para);
	
	msg = jpf_xml_msg_new_2(SET_PTZ_PARAMETER_ID, ptz_para, sizeof(*ptz_para));
	j_xml_dealloc(ptz_para, sizeof(*ptz_para));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_SET_PTZ_PARAMETER_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_ptz_parameter_result()
{
	test_set_result(PTZ_PARAMETER_RESULT_ID, MERGE_PTZ_PARAMETER_RESULT_PATH);
}
static void test_get_ftp_parameter()
{
	test_get_info(GET_FTP_PARAMETER_ID, MERGE_GET_FTP_PARAMETER_PATH);
}
static void get_ftp_parameter(FTPParameterPacket *ftp_para)
{
	snprintf(ftp_para->session_id, sizeof(ftp_para->session_id), "%s", "JXJ-SESSION-ID-001");
	snprintf(ftp_para->domain_id, sizeof(ftp_para->domain_id), "%s", "JXJ-DOMAIN-ID-0001");
	snprintf(ftp_para->pu_id, sizeof(ftp_para->pu_id), "%s", "JXJ-PU-ID-000000000001");
	snprintf(ftp_para->ftp_ip, sizeof(ftp_para->ftp_ip), "%s", "JXJ-SERIAL-ID-001");
	snprintf(ftp_para->ftp_usr, sizeof(ftp_para->ftp_usr), "%s", "JXJ-SERIAL-ID-001");
	snprintf(ftp_para->ftp_pwd, sizeof(ftp_para->ftp_pwd), "%s", "JXJ-SERIAL-ID-001");
	snprintf(ftp_para->ftp_path, sizeof(ftp_para->ftp_path), "%s", "JXJ-SERIAL-ID-001");
	ftp_para->result.code = 1;
	ftp_para->ftp_port = 5656;
}
static void test_ftp_parameter_response()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	FTPParameterPacket *ftp_para = NULL;
	
	ftp_para = (FTPParameterPacket*)j_xml_alloc(sizeof(*ftp_para));
	get_ftp_parameter(ftp_para);
	
	msg = jpf_xml_msg_new_2(FTP_PARAMETER_RESPONSE_ID, ftp_para, sizeof(*ftp_para));
	j_xml_dealloc(ftp_para, sizeof(*ftp_para));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_FTP_PARAMETER_RESPONSE_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_set_ftp_parameter()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	FTPParameterPacket *ftp_para = NULL;
	
	ftp_para = (FTPParameterPacket*)j_xml_alloc(sizeof(*ftp_para));
	get_ftp_parameter(ftp_para);
	
	msg = jpf_xml_msg_new_2(SET_FTP_PARAMETER_ID, ftp_para, sizeof(*ftp_para));
	j_xml_dealloc(ftp_para, sizeof(*ftp_para));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_SET_FTP_PARAMETER_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_ftp_parameter_result()
{
	test_set_result(FTP_PARAMETER_RESULT_ID, MERGE_FTP_PARAMETER_RESULT_PATH);
}
static void test_get_smtp_parameter()
{
	test_get_info(GET_SMTP_PARAMETER_ID, MERGE_GET_SMTP_PARAMETER_PATH);
}
static void get_smtp_parameter(SMTPParameterPacket *smtp_para)
{
	snprintf(smtp_para->session_id, sizeof(smtp_para->session_id), "%s", "JXJ-SESSION-ID-001");
	snprintf(smtp_para->domain_id, sizeof(smtp_para->domain_id), "%s", "JXJ-DOMAIN-ID-0001");
	snprintf(smtp_para->pu_id, sizeof(smtp_para->pu_id), "%s", "JXJ-PU-ID-000000000001");
	snprintf(smtp_para->mail_ip, sizeof(smtp_para->mail_ip), "%s", "JXJ-PU-ID-000000000001");
	snprintf(smtp_para->mail_addr, sizeof(smtp_para->mail_addr), "%s", "JXJ-PU-ID-000000000001");
	snprintf(smtp_para->mail_usr, sizeof(smtp_para->mail_usr), "%s", "JXJ-PU-ID-000000000001");
	snprintf(smtp_para->mail_pwd, sizeof(smtp_para->mail_pwd), "%s", "JXJ-PU-ID-000000000001");
	snprintf(smtp_para->mail_rctp1, sizeof(smtp_para->mail_rctp1), "%s", "JXJ-PU-ID-000000000001");
	snprintf(smtp_para->mail_rctp2, sizeof(smtp_para->mail_rctp2), "%s", "JXJ-PU-ID-000000000001");
	snprintf(smtp_para->mail_rctp3, sizeof(smtp_para->mail_rctp3), "%s", "JXJ-PU-ID-000000000001");
	smtp_para->result.code = 1;
	smtp_para->ssl_enable= 5656;
}
static void test_smtp_parameter_response()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	SMTPParameterPacket *smtp_para = NULL;
	
	smtp_para = (SMTPParameterPacket*)j_xml_alloc(sizeof(*smtp_para));
	get_smtp_parameter(smtp_para);
	
	msg = jpf_xml_msg_new_2(SMTP_PARAMETER_RESPONSE_ID, smtp_para, sizeof(*smtp_para));
	j_xml_dealloc(smtp_para, sizeof(*smtp_para));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_SMTP_PARAMETER_RESPONSE_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_set_smtp_parameter()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	SMTPParameterPacket *smtp_para = NULL;
	
	smtp_para = (SMTPParameterPacket*)j_xml_alloc(sizeof(*smtp_para));
	get_smtp_parameter(smtp_para);
	
	msg = jpf_xml_msg_new_2(SET_SMTP_PARAMETER_ID, smtp_para, sizeof(*smtp_para));
	j_xml_dealloc(smtp_para, sizeof(*smtp_para));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_SET_SMTP_PARAMETER_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_smtp_parameter_result()
{
	test_set_result(SMTP_PARAMETER_RESULT_ID, MERGE_SMTP_PARAMETER_RESULT_PATH);
}
static void test_get_upnp_parameter()
{
	test_get_info(GET_UPNP_PARAMETER_ID, MERGE_GET_UPNP_PARAMETER_PATH);
}
static void get_upnp_parameter(UPNPParameterPacket *upnp_para)
{
	snprintf(upnp_para->session_id, sizeof(upnp_para->session_id), "%s", "JXJ-SESSION-ID-001");
	snprintf(upnp_para->domain_id, sizeof(upnp_para->domain_id), "%s", "JXJ-DOMAIN-ID-0001");
	snprintf(upnp_para->pu_id, sizeof(upnp_para->pu_id), "%s", "JXJ-PU-ID-000000000001");
	snprintf(upnp_para->upnp_ip, sizeof(upnp_para->upnp_ip), "%s", "192.168.1.12");
	upnp_para->result.code = 1;
	upnp_para->upnp_enable = 1;
	upnp_para->upnp_eth_no = 1;
	upnp_para->upnp_model  = 1;
	upnp_para->upnp_refresh_time = 1;
	upnp_para->upnp_data_port = 5656;
	upnp_para->upnp_web_port  = 5656;
	upnp_para->upnp_data_port_result = 1;
	upnp_para->upnp_web_port_result  = 1;
}
static void test_upnp_parameter_response()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	UPNPParameterPacket *upnp_para = NULL;
	
	upnp_para = (UPNPParameterPacket*)j_xml_alloc(sizeof(*upnp_para));
	get_upnp_parameter(upnp_para);
	
	msg = jpf_xml_msg_new_2(UPNP_PARAMETER_RESPONSE_ID, upnp_para, sizeof(*upnp_para));
	j_xml_dealloc(upnp_para, sizeof(*upnp_para));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_UPNP_PARAMETER_RESPONSE_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_set_upnp_parameter()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	UPNPParameterPacket *upnp_para = NULL;
	
	upnp_para = (UPNPParameterPacket*)j_xml_alloc(sizeof(*upnp_para));
	get_upnp_parameter(upnp_para);
	
	msg = jpf_xml_msg_new_2(SET_UPNP_PARAMETER_ID, upnp_para, sizeof(*upnp_para));
	j_xml_dealloc(upnp_para, sizeof(*upnp_para));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_SET_UPNP_PARAMETER_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_upnp_parameter_result()
{
	test_set_result(UPNP_PARAMETER_RESULT_ID, MERGE_UPNP_PARAMETER_RESULT_PATH);
}
static void test_get_disk_info()
{
	test_get_info(GET_DEVICE_DISK_INFO_ID, MERGE_GET_DISK_INFO_PATH);
}
static void test_disk_info_response()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	DeviceDiskInfoPacket *disk_info = NULL;
	
	disk_info = (DeviceDiskInfoPacket*)j_xml_alloc(sizeof(*disk_info));
	snprintf(disk_info->session_id, sizeof(disk_info->session_id), "%s", "JXJ-SESSION-ID-001");
	snprintf(disk_info->domain_id, sizeof(disk_info->domain_id), "%s", "JXJ-DOMAIN-ID-0001");
	snprintf(disk_info->pu_id, sizeof(disk_info->pu_id), "%s", "JXJ-PU-ID-000000000001");
	disk_info->result.code = 1;

	disk_info->disk_num = 2;
	disk_info->disk[0].disk_no = 1;
	disk_info->disk[0].total_size = 1;
	disk_info->disk[0].free_size = 1;
	disk_info->disk[0].is_backup = 1;
	disk_info->disk[0].disk_type = 1;
	disk_info->disk[0].status = 1;
	disk_info->disk[1].disk_no = 2;
	disk_info->disk[1].total_size = 2;
	disk_info->disk[1].free_size = 2;
	disk_info->disk[1].is_backup = 2;
	disk_info->disk[1].disk_type = 2;
	disk_info->disk[1].status = 2;
	
	msg = jpf_xml_msg_new_2(DEVICE_DISK_INFO_RESPONSE_ID, disk_info, sizeof(*disk_info));
	j_xml_dealloc(disk_info, sizeof(*disk_info));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_DISK_INFO_RESPONSE_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void get_format_disk_info(FormatDiskPacket *format_disk)
{
	snprintf(format_disk->session_id, sizeof(format_disk->session_id), "%s", "JXJ-SESSION-ID-001");
	snprintf(format_disk->domain_id, sizeof(format_disk->domain_id), "%s", "JXJ-DOMAIN-ID-0001");
	snprintf(format_disk->pu_id, sizeof(format_disk->pu_id), "%s", "JXJ-PU-ID-000000000001");
	format_disk->result.code = 1;
	format_disk->disk_no = 1;
	format_disk->progress = 1;
}
static void test_format_disk_request()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	FormatDiskPacket *format_disk = NULL;
	
	format_disk = (FormatDiskPacket*)j_xml_alloc(sizeof(*format_disk));
	get_format_disk_info(format_disk);
	
	msg = jpf_xml_msg_new_2(FORMAT_DISK_REQUEST_ID, format_disk, sizeof(*format_disk));
	j_xml_dealloc(format_disk, sizeof(*format_disk));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_FORMAT_DISK_REQUEST_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_format_disk_result()
{
	test_set_result(FORMAT_DISK_RESULT_ID, MERGE_FORMAT_DISK_RESULT_PATH);
}
static void test_get_format_progress()
{
	test_get_info(GET_FORMAT_PROGRESS_ID, MERGE_GET_FORMAT_PROGRESS_PATH);
}
static void test_format_progress_response()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	FormatDiskPacket *format_disk = NULL;
	
	format_disk = (FormatDiskPacket*)j_xml_alloc(sizeof(*format_disk));
	get_format_disk_info(format_disk);
	
	msg = jpf_xml_msg_new_2(FORMAT_PROGRESS_RESPONSE_ID, format_disk, sizeof(*format_disk));
	j_xml_dealloc(format_disk, sizeof(*format_disk));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_FORMAT_PROGRESS_RESPONSE_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_get_move_alarm_info()
{
	test_get_info(GET_MOVE_ALARM_INFO_ID, MERGE_GET_MOVE_ALARM_INFO_PATH);
}
static void get_move_alarm_info(MoveAlarmPacket *move_alarm)
{
	snprintf(move_alarm->session_id, sizeof(move_alarm->session_id), "%s", "JXJ-SESSION-ID-001");
	snprintf(move_alarm->domain_id, sizeof(move_alarm->domain_id), "%s", "JXJ-DOMAIN-ID-0001");
	snprintf(move_alarm->gu_id, sizeof(move_alarm->gu_id), "%s", "JXJ-PU-ID-000000000001");
	move_alarm->result.code = 1;
	move_alarm->move_enable = 1;
	move_alarm->sensitive_level = 1;
	move_alarm->detect_interval = 1;
	/*move_alarm->detect_area = 1;
	
	get_local_time(&(move_alarm->time_start));
	get_local_time(&(move_alarm->time_end));//*/
}
static void test_move_alarm_info_response()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	MoveAlarmPacket *move_alarm = NULL;
	
	/*move_alarm = (MoveAlarmPacket*)j_xml_alloc(sizeof(*move_alarm));
	get_move_alarm_info(move_alarm);
	
	msg = jpf_xml_msg_new_2(MOVE_ALARM_INFO_RESPONSE_ID, move_alarm, sizeof(*move_alarm));
	j_xml_dealloc(move_alarm, sizeof(*move_alarm));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_MOVE_ALARM_INFO_RESPONSE_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);*/
	
	get_xml_info_from_file("aaa.xml", buf, sizeof(buf));
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_set_move_alarm_info()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	MoveAlarmPacket *move_alarm = NULL;
	
	move_alarm = (MoveAlarmPacket*)j_xml_alloc(sizeof(*move_alarm));
	get_move_alarm_info(move_alarm);
	
	msg = jpf_xml_msg_new_2(SET_MOVE_ALARM_INFO_ID, move_alarm, sizeof(*move_alarm));
	j_xml_dealloc(move_alarm, sizeof(*move_alarm));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_SET_MOVE_ALARM_INFO_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_move_alarm_info_result()
{
	test_set_result(MOVE_ALARM_INFO_RESULT_ID, MERGE_MOVE_ALARM_INFO_RESULT_PATH);
}
static void test_get_lost_alarm_info()
{
	test_get_info(GET_LOST_ALARM_INFO_ID, MERGE_GET_LOST_ALARM_INFO_PATH);
}
static void get_lost_alarm_info(LostAlarmPacket *lost_alarm)
{
	snprintf(lost_alarm->session_id, sizeof(lost_alarm->session_id), "%s", "JXJ-SESSION-ID-001");
	snprintf(lost_alarm->domain_id, sizeof(lost_alarm->domain_id), "%s", "JXJ-DOMAIN-ID-0001");
	snprintf(lost_alarm->gu_id, sizeof(lost_alarm->gu_id), "%s", "JXJ-PU-ID-000000000001");
	lost_alarm->result.code = 1;
	lost_alarm->lost_enable = 1;
	lost_alarm->detect_interval = 1;
	
	//get_local_time(&(lost_alarm->time_start));
	//get_local_time(&(lost_alarm->time_end));
}
static void test_lost_alarm_info_response()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	LostAlarmPacket *lost_alarm = NULL;
	
	lost_alarm = (LostAlarmPacket*)j_xml_alloc(sizeof(*lost_alarm));
	get_lost_alarm_info(lost_alarm);
	
	msg = jpf_xml_msg_new_2(LOST_ALARM_INFO_RESPONSE_ID, lost_alarm, sizeof(*lost_alarm));
	j_xml_dealloc(lost_alarm, sizeof(*lost_alarm));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_LOST_ALARM_INFO_RESPONSE_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_set_lost_alarm_info()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	LostAlarmPacket *lost_alarm = NULL;
	
	lost_alarm = (LostAlarmPacket*)j_xml_alloc(sizeof(*lost_alarm));
	get_lost_alarm_info(lost_alarm);
	
	msg = jpf_xml_msg_new_2(SET_LOST_ALARM_INFO_ID, lost_alarm, sizeof(*lost_alarm));
	j_xml_dealloc(lost_alarm, sizeof(*lost_alarm));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_SET_LOST_ALARM_INFO_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_lost_alarm_info_result()
{
	test_set_result(LOST_ALARM_INFO_RESULT_ID, MERGE_LOST_ALARM_INFO_RESULT_PATH);
}
static void test_get_hide_alarm_info()
{
	test_get_info(GET_HIDE_ALARM_INFO_ID, MERGE_GET_HIDE_ALARM_INFO_PATH);
}
static void get_hide_alarm_info(HideAlarmPacket *hide_alarm)
{
	snprintf(hide_alarm->session_id, sizeof(hide_alarm->session_id), "%s", "JXJ-SESSION-ID-001");
	snprintf(hide_alarm->domain_id, sizeof(hide_alarm->domain_id), "%s", "JXJ-DOMAIN-ID-0001");
	snprintf(hide_alarm->gu_id, sizeof(hide_alarm->gu_id), "%s", "JXJ-PU-ID-000000000001");
	hide_alarm->result.code = 1;
	/*hide_alarm->hide_enable = 1;
	hide_alarm->hide_x= 1;
	hide_alarm->hide_y = 1;
	hide_alarm->hide_w = 1;
	hide_alarm->hide_h = 1;
	
	get_local_time(&(hide_alarm->time_start));
	get_local_time(&(hide_alarm->time_end));*/
}
static void test_hide_alarm_info_response()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	HideAlarmPacket *hide_alarm = NULL;
	
	hide_alarm = (HideAlarmPacket*)j_xml_alloc(sizeof(*hide_alarm));
	get_hide_alarm_info(hide_alarm);
	
	msg = jpf_xml_msg_new_2(HIDE_ALARM_INFO_RESPONSE_ID, hide_alarm, sizeof(*hide_alarm));
	j_xml_dealloc(hide_alarm, sizeof(*hide_alarm));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_HIDE_ALARM_INFO_RESPONSE_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_set_hide_alarm_info()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	HideAlarmPacket *hide_alarm = NULL;
	
	hide_alarm = (HideAlarmPacket*)j_xml_alloc(sizeof(*hide_alarm));
	get_hide_alarm_info(hide_alarm);
	
	msg = jpf_xml_msg_new_2(SET_HIDE_ALARM_INFO_ID, hide_alarm, sizeof(*hide_alarm));
	j_xml_dealloc(hide_alarm, sizeof(*hide_alarm));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_SET_HIDE_ALARM_INFO_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_hide_alarm_info_result()
{
	test_set_result(HIDE_ALARM_INFO_RESULT_ID, MERGE_HIDE_ALARM_INFO_RESULT_PATH);
}
static void test_get_io_alarm_info()
{
	test_get_info(GET_IO_ALARM_INFO_ID, MERGE_GET_IO_ALARM_INFO_PATH);
}
static void get_io_alarm_info(IoAlarmPacket *io_alarm)
{
	snprintf(io_alarm->session_id, sizeof(io_alarm->session_id), "%s", "JXJ-SESSION-ID-001");
	snprintf(io_alarm->domain_id, sizeof(io_alarm->domain_id), "%s", "JXJ-DOMAIN-ID-0001");
	snprintf(io_alarm->gu_id, sizeof(io_alarm->gu_id), "%s", "JXJ-PU-ID-000000000001");
	io_alarm->result.code = 1;
	io_alarm->alarm_enable = 1;
	io_alarm->detect_interval = 1;
	
	//get_local_time(&(io_alarm->time_start));
	//get_local_time(&(io_alarm->time_end));
}
static void test_io_alarm_info_response()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	IoAlarmPacket *io_alarm = NULL;
	
	io_alarm = (IoAlarmPacket*)j_xml_alloc(sizeof(*io_alarm));
	get_io_alarm_info(io_alarm);
	
	msg = jpf_xml_msg_new_2(IO_ALARM_INFO_RESPONSE_ID, io_alarm, sizeof(*io_alarm));
	j_xml_dealloc(io_alarm, sizeof(*io_alarm));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_IO_ALARM_INFO_RESPONSE_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_set_io_alarm_info()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	IoAlarmPacket *io_alarm = NULL;
	
	io_alarm = (IoAlarmPacket*)j_xml_alloc(sizeof(*io_alarm));
	get_io_alarm_info(io_alarm);
	
	msg = jpf_xml_msg_new_2(SET_IO_ALARM_INFO_ID, io_alarm, sizeof(*io_alarm));
	j_xml_dealloc(io_alarm, sizeof(*io_alarm));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_SET_IO_ALARM_INFO_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_io_alarm_info_result()
{
	test_set_result(IO_ALARM_INFO_RESULT_ID, MERGE_IO_ALARM_INFO_RESULT_PATH);
}
static void test_submit_alarm_request()
{
	/*int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	SubmitAlarmPacket *sub_alarm = NULL;
	
	sub_alarm = (SubmitAlarmPacket*)j_xml_alloc(sizeof(*sub_alarm));

	snprintf(sub_alarm->session_id, sizeof(sub_alarm->session_id), "%s", "JXJ-SESSION-ID-001");
	snprintf(sub_alarm->domain_id, sizeof(sub_alarm->domain_id), "%s", "JXJ-DOMAIN-ID-0001");
	snprintf(sub_alarm->pu_id, sizeof(sub_alarm->pu_id), "%s", "JXJ-PU-ID-000000000001");
	snprintf(sub_alarm->data, sizeof(sub_alarm->data), "%s", "JXJ-PU-ID-000000000001");
	sub_alarm->channel = 1;
	sub_alarm->alarm = 1;
	get_local_time(&(sub_alarm->alarm_time));
	
	msg = jpf_xml_msg_new_2(SUBMIT_ALARM_REQUEST_ID, sub_alarm, sizeof(*sub_alarm));
	j_xml_dealloc(sub_alarm, sizeof(*sub_alarm));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), MERGE_SUBMIT_ALARM_REQUEST_PATH);
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);*/
	printf("***************************************************\n");
}
static void test_submit_alarm_result()
{
	test_set_result(SUBMIT_ALARM_RESULT_ID, MERGE_SUBMIT_ALARM_RESULT_PATH);
}
static void test_get_media_url_request()
{
	test_get_info(GET_MEDIA_URL_REQUEST_ID, "get_media_url_request.xml");
}
static void test_get_media_url_response()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	MediaUrlPacket *media_url;
	
	media_url = (MediaUrlPacket*)j_xml_alloc(sizeof(*media_url));
	snprintf(media_url->session_id, sizeof(media_url->session_id), "%s", "JXJ-SESSION-ID-001");
	snprintf(media_url->domain_id, sizeof(media_url->domain_id), "%s", "JXJ-DOMAIN-ID-0001");
	snprintf(media_url->gu_id, sizeof(media_url->gu_id), "%s", "JXJ-PU-ID-000000000001");
	snprintf(media_url->url, sizeof(media_url->url), "%s", "JXJ-URI-000000000001");
	
	msg = jpf_xml_msg_new_2(GET_MEDIA_URL_RESPONSE_ID, media_url, sizeof(*media_url));
	j_xml_dealloc(media_url, sizeof(*media_url));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), "get_media_url_response.xml");
	jpf_xml_msg_destroy(msg);
	//printf("%d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}


void *test_message()
{
	/*test_register_request();
	test_register_response();
	test_heart_beat_request();
	test_heart_beat_response();
	test_change_dispatch_request();
	test_change_dispatch_result();
	test_get_device_info();
	test_device_info_response();
	test_get_device_ntp_info();
	test_device_ntp_info_response();
	test_set_device_ntp_info();
	test_device_ntp_info_result();
	test_set_device_time();
	test_device_time_result();
	test_get_platform_info();
	test_platform_info_response();
	test_set_platform_info();
	test_platform_info_result();
	test_get_network_info();
	test_network_info_response();
	test_set_network_info();
	test_network_info_result();
	test_get_pppoe_info();
	test_pppoe_info_response();
	test_set_pppoe_info();
	test_pppoe_info_result();
	test_get_encode_parameter();
	test_encode_parameter_response();
	test_set_encode_parameter();
	test_encode_parameter_result();
	test_get_display_parameter();
	test_display_parameter_response();
	test_set_display_parameter();
	test_display_parameter_result();*/
	//test_get_record_parameter();
	//test_record_parameter_response();
	/*test_set_record_parameter();
	test_record_parameter_result();
	test_get_hide_parameter();
	test_hide_parameter_response();
	test_set_hide_parameter();
	test_hide_parameter_result();
	test_get_serial_parameter();
	test_serial_parameter_response();
	test_set_serial_parameter();
	test_serial_parameter_result();
	test_get_osd_parameter();
	test_osd_parameter_response();
	test_set_osd_parameter();
	test_osd_parameter_result();
	test_get_ptz_parameter();
	test_ptz_parameter_response();
	test_set_ptz_parameter();
	test_ptz_parameter_result();
	test_get_ftp_parameter();
	test_ftp_parameter_response();
	test_set_ftp_parameter();
	test_ftp_parameter_result();
	test_get_smtp_parameter();
	test_smtp_parameter_response();
	test_set_smtp_parameter();
	test_smtp_parameter_result();
	test_get_upnp_parameter();
	test_upnp_parameter_response();
	test_set_upnp_parameter();
	test_upnp_parameter_result();
	test_get_disk_info();
	test_disk_info_response();
	test_format_disk_request();
	test_format_disk_result();
	test_get_format_progress();
	test_format_progress_response();
	test_get_move_alarm_info();//*/
	//test_move_alarm_info_response();
	/*test_set_move_alarm_info();
	test_move_alarm_info_result();
	test_get_lost_alarm_info();
	test_lost_alarm_info_response();
	test_set_lost_alarm_info();
	test_lost_alarm_info_result();
	test_get_hide_alarm_info();
	test_hide_alarm_info_response();
	test_set_hide_alarm_info();
	test_hide_alarm_info_result();
	test_get_io_alarm_info();
	test_io_alarm_info_response();
	test_set_io_alarm_info();
	test_io_alarm_info_result();
	test_submit_alarm_request();
	test_submit_alarm_result();
	//*/

	return NULL;
}


#ifdef HAVE_PROXY_INFO
#include "proxy_info.h"

static void test_user_login_request()
{
	int err = 0;
	int ret = -1;

	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	struct __UserInfo login_info;

	snprintf(login_info.username, sizeof(login_info.username), "%s", "JXJ-USER-NAME");
	snprintf(login_info.password, sizeof(login_info.password), "%s", "JXJ-PASSWORD");
	
	msg = jpf_xml_msg_new_2(USER_LONGI_REQUEST_ID, &login_info, sizeof(struct __UserInfo));	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), "user_login_request.xml");
	jpf_xml_msg_destroy_2(msg);
	printf("return: %d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}

static void test_user_login_result()
{
	int err = 0;
	int ret = -1;

	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	int result = 1;
	
	msg = jpf_xml_msg_new_2(USER_LONGI_RESULT_ID, (void*)&result, sizeof(result));	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), "user_login_result.xml");
	jpf_xml_msg_destroy_2(msg);
	printf("return: %d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);
	void *priv = XML_MSG_DATA(msg);
	printf("priv: %d\n", *((int*)priv));
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}

static void test_add_user_info_result()
{
	int err = 0;
	int ret = -1;

	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	int result = 1;
	
	msg = jpf_xml_msg_new_2(ADD_USER_RESULT_ID, (void*)&result, sizeof(result));	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), "add_user_result.xml");
	jpf_xml_msg_destroy_2(msg);
	printf("return: %d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);
	void *priv = XML_MSG_DATA(msg);
	printf("priv: %d\n", *((int*)priv));
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}

static void test_proxy_heart_request()
{
	int err = 0;
	int ret = -1;

	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	struct __UserHeart heart;
	
	msg = jpf_xml_msg_new_2(USER_HEART_REQUEST_ID, &heart, sizeof(struct __UserHeart));	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), "proxy_heart_request.xml");
	jpf_xml_msg_destroy_2(msg);
	printf("return: %d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}

static void test_proxy_heart_result()
{
	int err = 0;
	int ret = -1;

	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	struct __UserHeart heart;
	
	get_local_time(&(heart.server_time));
	
	msg = jpf_xml_msg_new_2(USER_HEART_RESPONSE_ID, (void*)&heart, sizeof(struct __UserHeart));	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), "proxy_heart_result.xml");
	jpf_xml_msg_destroy_2(msg);
	printf("return: %d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);
	void *priv = XML_MSG_DATA(msg);
	printf("priv: %d\n", *((int*)priv));
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}

static void test_user_list_info()
{
	int err = 0;
	int ret = -1;

	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;

	struct _prx_user_st user_list;
	struct __UserInfo *user_info;

	user_list.count = 2;
	user_list.user_info = (struct __UserInfo *)j_xml_alloc(user_list.count * sizeof(struct __UserInfo));
	
	user_info = &user_list.user_info[0];
	snprintf(user_info->username, sizeof(user_info->username), 
		"%s", "JXJ-111");
	snprintf(user_info->password, sizeof(user_info->password), 
		"%s", "JXJ-111");

	user_info = &user_list.user_info[1];
	snprintf(user_info->username, sizeof(user_info->username), 
		"%s", "JXJ-222");
	snprintf(user_info->password, sizeof(user_info->password), 
		"%s", "JXJ-222");
	
	msg = jpf_xml_msg_new_2(USER_LIST_INFO_ID, 
			(void*)&user_list, sizeof(struct _prx_user_st));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), "user_list_info.xml");
	
	j_xml_dealloc(user_list.user_info, user_list.count * sizeof(struct __UserInfo));
	jpf_xml_msg_destroy_2(msg);
	printf("return: %d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);
	struct _prx_user_st *user = XML_MSG_DATA(msg);
	printf("%d\n", user->count);
	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}

static void test_device_list_info()
{
	int err = 0;
	int ret = -1;

	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;

	struct _prx_device_st device_list;
	struct _prx_device_info *dev_info;

	device_list.count = 2;
	device_list.device_info = (struct _prx_device_info *)j_xml_alloc(device_list.count * sizeof(struct _prx_device_info));

	dev_info = &device_list.device_info[0];
	dev_info->device_id = 1;
	dev_info->device_port = 3030;
	dev_info->platform_port = 3040;
	dev_info->type = 1;
	snprintf(dev_info->device_ip, sizeof(dev_info->device_ip), "%s", "192.168.1.179");
	snprintf(dev_info->pu_id, sizeof(dev_info->pu_id), "%s", "JXJ-PU-ID");
	snprintf(dev_info->factory, sizeof(dev_info->factory), "%s", "sz-jxj");
	snprintf(dev_info->sdk_version, sizeof(dev_info->sdk_version), "%s", "2.0.1");
	snprintf(dev_info->platform_ip, sizeof(dev_info->platform_ip), "%s", "192.168.1.12");
	snprintf(dev_info->username, sizeof(dev_info->username), "%s", "username");
	snprintf(dev_info->password, sizeof(dev_info->password), "%s", "password");

	dev_info = &device_list.device_info[1];
	dev_info->device_id = 2;
	dev_info->device_port = 3030;
	dev_info->platform_port = 3040;
	dev_info->type = 1;
	snprintf(dev_info->device_ip, sizeof(dev_info->device_ip), "%s", "192.168.1.179");
	snprintf(dev_info->pu_id, sizeof(dev_info->pu_id), "%s", "JXJ-PU-ID");
	snprintf(dev_info->factory, sizeof(dev_info->factory), "%s", "sz-jxj");
	snprintf(dev_info->sdk_version, sizeof(dev_info->sdk_version), "%s", "2.0.1");
	snprintf(dev_info->platform_ip, sizeof(dev_info->platform_ip), "%s", "192.168.1.12");
	snprintf(dev_info->username, sizeof(dev_info->username), "%s", "username");
	snprintf(dev_info->password, sizeof(dev_info->password), "%s", "password");
	
	msg = jpf_xml_msg_new_2(DEVICE_LIST_INFO_ID, 
			(void*)&device_list, sizeof(struct _prx_device_st));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), "device_list_info.xml");
	
	j_xml_dealloc(device_list.device_info, device_list.count * sizeof(struct _prx_device_info));
	jpf_xml_msg_destroy_2(msg);
	printf("return: %d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);
	struct _prx_device_st *device = XML_MSG_DATA(msg);
	printf("%d\n", device->count);
	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}

static void test_factory_list_info()
{
	int err = 0;
	int ret = -1;

	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;

	struct _prx_factory_list fct_list;
	struct _prx_factory_info *fct_info;

	/*fct_list.count = 2;
	fct_list.factory = (struct _prx_factory_info *)j_xml_alloc(fct_list.count * sizeof(struct _prx_factory_info));

	fct_info = &fct_list.factory[0];
	fct_info->ver_count = 2;
	fct_info->type_count = 2;
	fct_info->type[0] = 1;
	fct_info->type[1] = 2;
	snprintf(fct_info->sdk_version[0], sizeof(fct_info->sdk_version[0]), "%s", "2.0.0");
	snprintf(fct_info->sdk_version[1], sizeof(fct_info->sdk_version[1]), "%s", "2.0.1");
	snprintf(fct_info->factory_name, sizeof(fct_info->factory_name), "%s", "szjxj");
	
	fct_info = &fct_list.factory[1];
	fct_info->ver_count = 2;
	fct_info->type_count = 2;
	fct_info->type[0] = 1;
	fct_info->type[1] = 2;
	snprintf(fct_info->sdk_version[0], sizeof(fct_info->sdk_version[0]), "%s", "2.0.0");
	snprintf(fct_info->sdk_version[1], sizeof(fct_info->sdk_version[1]), "%s", "2.0.1");
	snprintf(fct_info->factory_name, sizeof(fct_info->factory_name), "%s", "szjxj");
	
	msg = jpf_xml_msg_new_2(FACTORY_LIST_INFO_ID, 
			(void*)&fct_list, sizeof(struct _prx_factory_list));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), "factory_list_info.xml");
	
	j_xml_dealloc(fct_list.factory, fct_list.count * sizeof(struct _prx_factory_info));
	jpf_xml_msg_destroy_2(msg);
	printf("return: %d.buf: %s\n", ret, buf);*/
	
	get_xml_info_from_file("factory_info.xml", buf, MAX_XML_LEN);
	msg = parse_xml(buf, sizeof(buf), &err, 0);

	struct _prx_factory_list *list;
	list = XML_MSG_DATA(msg);

	int i;
	for (i=0; i<list->count; i++)
	{
		printf("factory info [%d]\n", i);
		printf("    name    : %s\n", list->factory[i].factory_name);
		int n, m;
		for (n=0; n<list->factory[i].type_count; n++)
		{
			printf("    machine : %d\n", list->factory[i].type[n]);
			for (m=0; m<list->factory[i].ver_count[n]; m++)
			{
				printf("        sdkVersion : %s\n", list->factory[i].sdk_version[n][m]);
			}
		}
	}

	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}

static void test_fuzzy_find_user_request()
{
	int err = 0;
	int ret = -1;

	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	
	struct __UserInfo user_info;

	memset(&user_info, 0, sizeof(user_info));
	snprintf(user_info.username, sizeof(user_info.username), 
		"%s", "JXJ-111");
	
	msg = jpf_xml_msg_new_2(FUZZY_FIND_USER_REQUEST_ID, 
			(void*)&user_info, sizeof(struct __UserInfo));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), "fuzzy_find_user_request.xml");
	jpf_xml_msg_destroy_2(msg);
	printf("return: %d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	void *priv = XML_MSG_DATA(msg);
	printf("priv: %d\n", *((int*)priv));
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}

static void test_fuzzy_find_user_result()
{
	int err = 0;
	int ret = -1;

	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	
	struct _prx_user_st user_list;
	struct __UserInfo *user_info;

	user_list.count = 2;
	user_list.user_info = (struct __UserInfo *)j_xml_alloc(user_list.count * sizeof(struct __UserInfo));
	
	user_info = &user_list.user_info[0];
	snprintf(user_info->username, sizeof(user_info->username), 
		"%s", "JXJ-111");

	user_info = &user_list.user_info[1];
	snprintf(user_info->username, sizeof(user_info->username), 
		"%s", "JXJ-222");

	msg = jpf_xml_msg_new_2(FUZZY_FIND_USER_RESULT_ID, 
			(void*)&user_list, sizeof(struct _prx_user_st));

	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), "fuzzy_find_user_result.xml");
	
	j_xml_dealloc(user_list.user_info, user_list.count * sizeof(struct __UserInfo));
	jpf_xml_msg_destroy_2(msg);
	printf("return: %d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);
	void *priv = XML_MSG_DATA(msg);
	printf("priv: %d\n", *((int*)priv));
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}

static void test_modify_password_request()
{
	int err = 0;
	int ret = -1;

	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	
	struct _prx_modify_pwd modify_pwd;

	memset(&modify_pwd, 0, sizeof(modify_pwd));
	snprintf(modify_pwd.username, sizeof(modify_pwd.username), 
		"%s", "JXJ-111");
	snprintf(modify_pwd.old_pwd, sizeof(modify_pwd.old_pwd), 
		"%s", "JXJ-222");
	snprintf(modify_pwd.new_pwd, sizeof(modify_pwd.new_pwd), 
		"%s", "JXJ-333");
	
	msg = jpf_xml_msg_new_2(MODIFY_PASSWORD_REQUEST_ID, 
			(void*)&modify_pwd, sizeof(struct _prx_modify_pwd));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), "modify_password_request.xml");
	jpf_xml_msg_destroy_2(msg);
	printf("return: %d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	void *priv = XML_MSG_DATA(msg);
	printf("priv: %d\n", *((int*)priv));
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}

static void test_modify_password_result()
{
	int err = 0;
	int ret = -1;

	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	int result = 1;
	
	msg = jpf_xml_msg_new_2(MODIFY_PASSWORD_RESULT_ID, (void*)&result, sizeof(int));	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), "modify_password_result.xml");
	jpf_xml_msg_destroy_2(msg);
	printf("return: %d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	void *priv = XML_MSG_DATA(msg);
	printf("priv: %d\n", *((int*)priv));
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}

static void test_add_device_request()
{
	int err = 0;
	int ret = -1;

	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	prx_device_info dev_info;

	dev_info.device_id = 0;
	dev_info.type = 1;
	dev_info.device_port = 5050;
	dev_info.platform_port = 5050;
	snprintf(dev_info.pu_id, sizeof(dev_info.pu_id), "%s", "JXJ-PU-ID");
	snprintf(dev_info.factory, sizeof(dev_info.factory), "%s", "JXJ");
	snprintf(dev_info.sdk_version, sizeof(dev_info.sdk_version), "%s", "0.0.1");
	snprintf(dev_info.username, sizeof(dev_info.username), "%s", "JXJ-USER-NAME");
	snprintf(dev_info.password, sizeof(dev_info.password), "%s", "JXJ-PASSWORD");
	snprintf(dev_info.device_ip, sizeof(dev_info.device_ip), "%s", "192.168.1.12");
	snprintf(dev_info.platform_ip, sizeof(dev_info.platform_ip), "%s", "192.168.1.12");

	
	msg = jpf_xml_msg_new_2(ADD_DEVICE_REQUEST_ID, &dev_info, sizeof(prx_device_info));	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), "add_device_request.xml");
	jpf_xml_msg_destroy_2(msg);
	printf("return: %d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}

static void test_add_device_result()
{
	int err = 0;
	int ret = -1;

	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	int result = 1;
	
	msg = jpf_xml_msg_new_2(ADD_DEVICE_RESULT_ID, (void*)&result, sizeof(int));	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), "add_device_result.xml");
	jpf_xml_msg_destroy_2(msg);
	printf("return: %d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	void *priv = XML_MSG_DATA(msg);
	printf("priv: %d\n", *((int*)priv));
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}

static void test_del_device_request()
{
	int err = 0;
	int ret = -1;

	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	int device_id = 1;
	
	msg = jpf_xml_msg_new_2(DEL_DEVICE_REQUEST_ID, (void*)&device_id, sizeof(int));	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), "del_device_request.xml");
	jpf_xml_msg_destroy_2(msg);
	printf("return: %d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	void *priv = XML_MSG_DATA(msg);
	printf("priv: %d\n", *((int*)priv));
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}

static void test_del_device_result()
{
	int err = 0;
	int ret = -1;

	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	int result = 1;
	
	msg = jpf_xml_msg_new_2(DEL_DEVICE_RESULT_ID, (void*)&result, sizeof(int));	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), "del_device_result.xml");
	jpf_xml_msg_destroy_2(msg);
	printf("return: %d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	void *priv = XML_MSG_DATA(msg);
	printf("priv: %d\n", *((int*)priv));
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}

static void test_get_device_info_request()
{
	int err = 0;
	int ret = -1;

	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	/*int device_id = 1;
	
	msg = jpf_xml_msg_new_2(GET_DEVICE_INFO_REQUEST_ID, (void*)&device_id, sizeof(int));	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), "get_device_info_request.xml");
	jpf_xml_msg_destroy_2(msg);
	printf("return: %d.buf: %s\n", ret, buf);*/
	
	get_xml_info_from_file("get_page_device_request.xml", buf, MAX_XML_LEN);
	msg = parse_xml(buf, sizeof(buf), &err, 0);

	struct _prx_page_device *page_dev = XML_MSG_DATA(msg);
	printf("%s\n", page_dev->factory_name);
	printf("%s\n", page_dev->sdk_version);
	printf("%d\n", page_dev->machine_type);
	printf("%d\n", page_dev->count);
	printf("%d\n", page_dev->offset);

	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_get_device_info_result()
{
	int err = 0;
	int ret = -1;

	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;

	struct _prx_page_device page_dev;
	struct _prx_device_info *dev_info;

	page_dev.count = 2;
	page_dev.total = 10;
	page_dev.offset= 10;
	page_dev.machine_type= 1;
	snprintf(page_dev.factory_name, sizeof(page_dev.factory_name), "%s", "JXJ");
	snprintf(page_dev.sdk_version, sizeof(page_dev.sdk_version), "%s", "2.0.0.0");
	
	page_dev.dev_info= (struct _prx_device_info*)
		j_xml_alloc(sizeof(struct _prx_device_info)*2);

	dev_info = &page_dev.dev_info[0];
	dev_info->device_id = 1;
	dev_info->type = 1;
	dev_info->result = 1;
	dev_info->device_port = 5050;
	dev_info->platform_port = 5050;
	snprintf(dev_info->pu_id, sizeof(dev_info->pu_id), "%s", "JXJ-PU-ID");
	snprintf(dev_info->factory, sizeof(dev_info->factory), "%s", "JXJ");
	snprintf(dev_info->sdk_version, sizeof(dev_info->sdk_version), "%s", "0.0.1");
	snprintf(dev_info->username, sizeof(dev_info->username), "%s", "JXJ-USER-NAME");
	snprintf(dev_info->password, sizeof(dev_info->password), "%s", "JXJ-PASSWORD");
	snprintf(dev_info->device_ip, sizeof(dev_info->device_ip), "%s", "192.168.1.12");
	snprintf(dev_info->platform_ip, sizeof(dev_info->platform_ip), "%s", "192.168.1.12");

	dev_info = &page_dev.dev_info[1];
	dev_info->device_id = 2;
	dev_info->type = 2;
	dev_info->result = 2;
	dev_info->device_port = 5052;
	dev_info->platform_port = 5052;
	snprintf(dev_info->pu_id, sizeof(dev_info->pu_id), "%s", "JXJ-PU-ID2");
	snprintf(dev_info->factory, sizeof(dev_info->factory), "%s", "JXJ2");
	snprintf(dev_info->sdk_version, sizeof(dev_info->sdk_version), "%s", "0.0.2");
	snprintf(dev_info->username, sizeof(dev_info->username), "%s", "JXJ-USER-NAME2");
	snprintf(dev_info->password, sizeof(dev_info->password), "%s", "JXJ-PASSWORD2");
	snprintf(dev_info->device_ip, sizeof(dev_info->device_ip), "%s", "192.168.1.12");
	snprintf(dev_info->platform_ip, sizeof(dev_info->platform_ip), "%s", "192.168.1.12");

	msg = jpf_xml_msg_new_2(GET_DEVICE_INFO_RESULT_ID, &page_dev, sizeof(prx_page_device));	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	
	create_xml_file_from_buffer(buf, strlen(buf), "get_device_info_result.xml");
	j_xml_dealloc(page_dev.dev_info, sizeof(struct _prx_device_info)*2);
	jpf_xml_msg_destroy_2(msg);
	printf("return: %d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_set_device_info_request()
{
	int err = 0;
	int ret = -1;

	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	prx_device_info dev_info;

	dev_info.device_id = 1;
	dev_info.type = 1;
	dev_info.device_port = 5050;
	dev_info.platform_port = 5050;
	snprintf(dev_info.pu_id, sizeof(dev_info.pu_id), "%s", "JXJ-PU-ID");
	snprintf(dev_info.factory, sizeof(dev_info.factory), "%s", "JXJ");
	snprintf(dev_info.sdk_version, sizeof(dev_info.sdk_version), "%s", "0.0.1");
	snprintf(dev_info.username, sizeof(dev_info.username), "%s", "JXJ-USER-NAME");
	snprintf(dev_info.password, sizeof(dev_info.password), "%s", "JXJ-PASSWORD");
	snprintf(dev_info.device_ip, sizeof(dev_info.device_ip), "%s", "192.168.1.12");
	snprintf(dev_info.platform_ip, sizeof(dev_info.platform_ip), "%s", "192.168.1.12");
	
	msg = jpf_xml_msg_new_2(SET_DEVICE_INFO_REQUEST_ID, &dev_info, sizeof(prx_device_info));	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), "set_device_info_request.xml");
	jpf_xml_msg_destroy_2(msg);
	printf("return: %d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}

static void test_set_device_info_result()
{
	int err = 0;
	int ret = -1;

	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	int device_id = 1;
	
	msg = jpf_xml_msg_new_2(SET_DEVICE_INFO_RESULT_ID, (void*)&device_id, sizeof(int));	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), "set_device_info_result.xml");
	jpf_xml_msg_destroy_2(msg);
	printf("return: %d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);	
	void *priv = XML_MSG_DATA(msg);
	printf("priv: %d\n", *((int*)priv));
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}

static void test_get_all_device_id_request()
{
	int err = 0;
	int ret = -1;

	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	int device_id = 1;
	
	msg = jpf_xml_msg_new_2(GET_ALL_DEVICE_ID_REQUEST_ID, (void*)&device_id, sizeof(int));	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), "get_all_device_id_request.xml");
	jpf_xml_msg_destroy_2(msg);
	printf("return: %d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);
	if (!msg)
		printf("parse_xml() failer!\n");
	void *priv = XML_MSG_DATA(msg);
	printf("priv: %d\n", *((int*)priv));
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}
static void test_get_all_device_id_result()
{
	int err = 0;
	int ret = -1;

	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	struct _prx_device_id_st id_list;
	
	id_list.count = 2;
	id_list.device_id = (struct _prx_device_id_st *)j_xml_alloc(id_list.count * sizeof(int));
	id_list.device_id[0] = 1;
	id_list.device_id[1] = 2;
	
	msg = jpf_xml_msg_new_2(GET_ALL_DEVICE_ID_RESULT_ID, &id_list, sizeof(prx_device_id_st));
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), "get_all_device_id_result.xml");
	j_xml_dealloc(id_list.device_id, id_list.count * sizeof(int));
	jpf_xml_msg_destroy_2(msg);
	printf("return: %d.buf: %s\n", ret, buf);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}


static void test_get_page_user_request()
{
	int err = 0;
	int ret = -1;

	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	//struct _prx_page_user page_user;
	struct _prx_page_user *one_page_user;

	/*snprintf(page_user.username, sizeof(page_user.username), "%s", "JXJ-USER-NAME");
	page_user.offset = 0;
	page_user.count = 5;

	msg = jpf_xml_msg_new_2(GET_USER_INFO_REQUEST_ID, &page_user, sizeof(struct _prx_page_user));	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), "get_page_user_request.xml");
	jpf_xml_msg_destroy_2(msg);
	printf("return: %d.buf: %s\n", ret, buf);*/

	get_xml_info_from_file("get_page_user_request.xml", buf, MAX_XML_LEN);
	msg = parse_xml(buf, sizeof(buf), &err, 0);	

	one_page_user = XML_MSG_DATA(msg);
	printf("%s\n", one_page_user->username);
	printf("%d\n", one_page_user->offset);
	printf("%d\n", one_page_user->count);
	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}

static void test_get_page_user_response()
{
	int err = 0;
	int ret = -1;

	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	struct _prx_page_user page_user;
	struct __UserInfo *user_info;

	page_user.user_info = (struct __UserInfo*)j_xml_alloc(sizeof(struct __UserInfo)*2);

	snprintf(page_user.username, sizeof(page_user.username), "%s", "aaa");
	page_user.total = 10;
	page_user.offset = 0;
	page_user.count = 2;

	user_info = &page_user.user_info[0];
	snprintf(user_info->username, sizeof(user_info->username), "%s", "JXJ-USER-NAME-0");
	
	user_info = &page_user.user_info[1];
	snprintf(user_info->username, sizeof(user_info->username), "%s", "JXJ-USER-NAME-1");
	
	msg = jpf_xml_msg_new_2(GET_USER_INFO_RESPONSE_ID, &page_user, sizeof(struct _prx_page_user));	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	j_xml_dealloc(page_user.user_info, sizeof(struct __UserInfo)*2);
	
	create_xml_file_from_buffer(buf, strlen(buf), "get_page_user_response.xml");
	jpf_xml_msg_destroy_2(msg);
	printf("return: %d.buf: %s\n", ret, buf);
	
	//msg = parse_xml(buf, sizeof(buf), &err, 0);
	//jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}



#endif
#endif

#ifndef __WIN_32__

#define _snprintf(str, size, fmt, ...) \
				{ \
					snprintf(str, size-1, fmt, ##__VA_ARGS__); \
					str[size-1] = '\0'; \
				}
#endif

#if 0
#include "proxy_info.h"
static int test_store_log_request()
{
  int err = 0;
  int ret = -1;
  
  char buf[MAX_XML_LEN];
  JpfXmlMsg *msg = NULL;
  char time_buffer[J_SDK_MAX_TIME_LEN];
  struct __StoreLogPacket *store_log_packet;
  
  store_log_packet = (struct __StoreLogPacket *)j_xml_alloc(sizeof(struct __StoreLogPacket));   
  
  snprintf(store_log_packet->session_id, sizeof(store_log_packet->session_id), "%s", "JXJ-SESSION-ID-001");
  snprintf(store_log_packet->domain_id, sizeof(store_log_packet->domain_id), "%s", "JXJ-DOMAIN-ID-0001");
  snprintf(store_log_packet->gu_id, sizeof(store_log_packet->gu_id), "%s", "JXJ-PU-ID-000000000001");
  
  store_log_packet->rec_type = 0xFFFFFFFF; 
  get_local_time(&(store_log_packet->beg_time));
  get_local_time(&(store_log_packet->end_time));
  store_log_packet->beg_node = 1;
  store_log_packet->end_node = 10;
    
  msg = jpf_xml_msg_new_2(GET_STORE_LOG_REQUEST_ID, store_log_packet, sizeof(struct __StoreLogPacket));
  j_xml_dealloc(store_log_packet, sizeof(struct __StoreLogPacket));
   
  ret = create_xml(msg, buf, sizeof(buf), 0);
  
  if (create_xml_file_from_buffer(buf, strlen(buf), "./GetStoreLog.xml") < 0)
  {
    return ret;
  }  
  
  jpf_xml_msg_destroy_2(msg);
  printf("%d.buf: %s\n", ret, buf);
  
  msg = parse_xml(buf, sizeof(buf), &err, 0);

  void *priv = XML_MSG_DATA(msg);

  printf("--------------------------------------------------\n");
  store_log_packet = (struct __StoreLogPacket *)priv;
  memcpy(store_log_packet, (struct __StoreLogPacket *)priv, sizeof(struct __StoreLogPacket));

  printf("sessionId=%s\n", store_log_packet->session_id);
  printf("domainId=%s\n", store_log_packet->domain_id);
  printf("guId=%s\n", store_log_packet->gu_id);
  printf("recType=%d\n", store_log_packet->rec_type);

  memset(time_buffer, 0, sizeof(time_buffer));
  merge_time_string(time_buffer, sizeof(store_log_packet->beg_time), 
                  &(store_log_packet->beg_time));
  printf("%s\n", time_buffer);

  memset(time_buffer, 0, sizeof(time_buffer));
  merge_time_string(time_buffer, sizeof(store_log_packet->end_time), 
                  &(store_log_packet->end_time));
  printf("%s\n", time_buffer);
  
  printf("begNode=%d\n", store_log_packet->beg_node);
  printf("endNode=%d\n", store_log_packet->end_node);

  jpf_xml_msg_destroy(msg);
  printf("***************************************************\n");
  
}

static int test_store_log_result()
{
	int err = 0;
	int ret = -1;
 
	char buf[MAX_XML_LEN];
	char time_buffer[J_SDK_MAX_TIME_LEN];
	JpfXmlMsg *msg = NULL;

	struct __StoreLogPacket *store_log_info;    
    store_log_info = (struct __StoreLogPacket *)j_xml_alloc(sizeof(struct __StoreLogPacket));   
   
    snprintf(store_log_info->session_id, sizeof(store_log_info->session_id), "%s", "DM-00001-00003");
    snprintf(store_log_info->domain_id, sizeof(store_log_info->domain_id), "%s", "JXJ-DOMAIN-ID-0001");
    snprintf(store_log_info->gu_id, sizeof(store_log_info->gu_id), "%s", "JXJ-PU-ID-000000000001");

	store_log_info->result.code  = 1;
	store_log_info->node_count = 2;

	store_log_info->store[0].rec_type = 4; 
	get_local_time(&(store_log_info->store[0].beg_time));
	get_local_time(&(store_log_info->store[0].end_time));
	store_log_info->store[0].file_size = 1222;

#if 1
	store_log_info->store[1].rec_type = 2;
	get_local_time(&(store_log_info->store[1].beg_time));
	get_local_time(&(store_log_info->store[1].end_time));
	store_log_info->store[1].file_size = 444;
#endif
    
	msg = jpf_xml_msg_new_2(GET_STORE_LOG_RESPONSE_ID, store_log_info, sizeof(struct __StoreLogPacket));
    j_xml_dealloc(store_log_info, sizeof(struct __StoreLogPacket));

    printf("buf:%s\n", buf);
    
    ret = create_xml(msg, buf, sizeof(buf), 0); 
    printf("buf:%s\n", buf);
    if (create_xml_file_from_buffer(buf, strlen(buf), "./GetStoreLogResponse.xml") < 0)
	{
		return ret;
	}  

    printf("ret:%d\n", ret);
    
	jpf_xml_msg_destroy_2(msg);
	
	msg = parse_xml(buf, sizeof(buf), &err, 0);
	void *priv = XML_MSG_DATA(msg);
        
    printf("sessionId=%s\n", ((struct __StoreLogPacket *)(priv))->session_id);
    printf("domainId=%s\n", ((struct __StoreLogPacket *)(priv))->domain_id);
    printf("guId=%s\n", ((struct __StoreLogPacket *)(priv))->gu_id);
	printf("resultCode=%d\n", ((struct __StoreLogPacket *)(priv))->result.code);  
	printf("logCount=%d\n", ((struct __StoreLogPacket *)(priv))->node_count); 
	printf("videoType=%d\n", ((struct __StoreLogPacket *)(priv))->store[0].rec_type);
	printf("fileSize=%d\n", ((struct __StoreLogPacket *)(priv))->store[0].file_size); 

	memset(time_buffer, 0, sizeof(time_buffer));
	merge_time_string(time_buffer, sizeof(time_buffer), (&((struct __StoreLogPacket *)(priv))->store[0].beg_time));
	printf("%s\n", time_buffer);

	memset(time_buffer, 0, sizeof(time_buffer));
	merge_time_string(time_buffer, sizeof(time_buffer), 
	              (&((struct __StoreLogPacket *)(priv))->store[0].end_time));
	printf("%s\n", time_buffer);

	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");

}

static int test_firmware_upgrade_request()
{
	int err = 0;
	int ret = -1;

	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;

	FirmwareUpgradePacket *upgrade_info;

	upgrade_info = (FirmwareUpgradePacket*)j_xml_alloc(sizeof(FirmwareUpgradePacket));   

	snprintf(upgrade_info->session_id, sizeof(upgrade_info->session_id), "%s", "JXJ-SESSION-ID-001");
	snprintf(upgrade_info->domain_id, sizeof(upgrade_info->domain_id), "%s", "JXJ-DOMAIN-ID-0001");
	snprintf(upgrade_info->pu_id, sizeof(upgrade_info->pu_id), "%s", "JXJ-PU-ID-000000000001");

	upgrade_info->data_len = 128;

	msg = jpf_xml_msg_new_2(FIRMWARE_UPGRADE_REQUEST_ID, upgrade_info, sizeof(FirmwareUpgradePacket));
	j_xml_dealloc(upgrade_info, sizeof(FirmwareUpgradePacket));

	ret = create_xml(msg, buf, sizeof(buf), 0);

	if (create_xml_file_from_buffer(buf, strlen(buf), "./upgrade_info.xml") < 0)
	{
		return ret;
	}  

	jpf_xml_msg_destroy_2(msg);
	printf("%d.buf: %s\n", ret, buf);

	msg = parse_xml(buf, strlen(buf), &err, 0);

	/*upgrade_info = (FirmwareUpgradePacket*)XML_MSG_DATA(msg);

	printf("--------------------------------------------------\n");
	//upgrade_info = priv;
	//memcpy(upgrade_info, (struct __StoreLogPacket *)priv, sizeof(FirmwareUpgradePacket));

	printf("sessionId=%s\n", upgrade_info->session_id);
	printf("domainId=%s\n", upgrade_info->domain_id);
	printf("puId=%s\n", upgrade_info->pu_id);
	printf("data_len=%d\n", upgrade_info->data_len);*/

	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}

#ifdef HAVE_PROXY_INFO

static void test_factory_list_info()
{
	int err = 0;
	int ret = -1;
	
	char buf[MAX_XML_LEN];
	JpfXmlMsg *msg = NULL;
	
	struct _prx_factory_list fct_list;
	struct _prx_factory_info *fct_info;

	/*fct_list.count = 2;
	fct_list.factory = (struct _prx_factory_info *)j_xml_alloc(fct_list.count * sizeof(struct _prx_factory_info));

	fct_info = &fct_list.factory[0];
	fct_info->ver_count = 2;
	fct_info->type_count = 2;
	fct_info->type[0] = 1;
	fct_info->type[1] = 2;
	snprintf(fct_info->sdk_version[0], sizeof(fct_info->sdk_version[0]), "%s", "2.0.0");
	snprintf(fct_info->sdk_version[1], sizeof(fct_info->sdk_version[1]), "%s", "2.0.1");
	snprintf(fct_info->factory_name, sizeof(fct_info->factory_name), "%s", "szjxj");
	
	fct_info = &fct_list.factory[1];
	fct_info->ver_count = 2;
	fct_info->type_count = 2;
	fct_info->type[0] = 1;
	fct_info->type[1] = 2;
	snprintf(fct_info->sdk_version[0], sizeof(fct_info->sdk_version[0]), "%s", "2.0.0");
	snprintf(fct_info->sdk_version[1], sizeof(fct_info->sdk_version[1]), "%s", "2.0.1");
	snprintf(fct_info->factory_name, sizeof(fct_info->factory_name), "%s", "szjxj");
	
	msg = jpf_xml_msg_new_2(FACTORY_LIST_INFO_ID, 
			(void*)&fct_list, sizeof(struct _prx_factory_list));
	
	ret = create_xml(msg, buf, sizeof(buf), 0);
	create_xml_file_from_buffer(buf, strlen(buf), "factory_list_info.xml");
	
	j_xml_dealloc(fct_list.factory, fct_list.count * sizeof(struct _prx_factory_info));
	jpf_xml_msg_destroy_2(msg);
	printf("return: %d.buf: %s\n", ret, buf);*/
	
	get_xml_info_from_file("factory_info.xml", buf, MAX_XML_LEN);
	msg = parse_xml(buf, sizeof(buf), &err, 0);

	struct _prx_factory_list *list;
	list = XML_MSG_DATA(msg);

	int i;
	for (i=0; i<list->count; i++)
	{
		printf("factory info [%d]\n", i);
		printf("    name    : %s\n", list->factory[i].factory_name);
		int n, m;
		for (n=0; n<list->factory[i].type_count; n++)
		{
			printf("    machine : %d\n", list->factory[i].type[n]);
			for (m=0; m<list->factory[i].ver_count[n]; m++)
			{
				printf("        sdkVersion : %s\n", list->factory[i].sdk_version[n][m]);
			}
		}
	}

	
	jpf_xml_msg_destroy(msg);
	printf("***************************************************\n");
}

#endif  //HAVE_PROXY_INFO
#endif

int main(int argc, char *argv[])
{
	init_jpf_xml_msg();

	///////////////////////////////////////////////////////////////////////////
	/*int err;
	JpfXmlMsg *msg = NULL;
	MoveAlarmPacket *move_alarm;

	char buffer[4096*2];
	memset(buffer, 0, sizeof(buffer));
	get_xml_info_from_file("./3.xml", buffer, sizeof(buffer));
	printf("%s\n", buffer);
	
	msg = parse_xml(buffer, strlen(buffer), &err, 0);
	if (msg)
	{
	printf("*************************************************************************\n");
		int day, seg;
		JWeek *week;
		move_alarm = (MoveAlarmPacket*)msg->priv_obj;
		week = &move_alarm->week;
		for (day=0; day<week->count; day++)
		{
			printf("count: %d\n", week->days[day].count);
			for (seg=0; seg<week->days[day].count; seg++)
			{
				printf("day_id: %d\n", week->days[day].day_id);
			}
		}
	}

	printf("*************************************************************************\n");
	create_xml(msg, buffer, sizeof(buffer), 0);

	jpf_xml_msg_destroy(msg);
	//////////////////////////////////////////////////////////////////////////*/
	
	//set_mem_handler(m_malloc, f_free);
	
	
#ifdef HAVE_PROXY_INFO

	//test_user_login_request();
	//test_user_login_result();

	//test_add_user_info_result();

	//test_get_page_user_request();
	//test_get_page_user_response();

	//test_user_list_info();
	//test_device_list_info();
//	test_factory_list_info();
	
	//test_fuzzy_find_user_request();
	//test_fuzzy_find_user_result();

	//test_modify_password_request();
	//test_modify_password_result();

	//test_add_device_request();
	//test_add_device_result();
	//test_del_device_request();
	//test_del_device_result();
	
	//test_get_device_info_request();
	//test_get_device_info_result();
	//test_set_device_info_request();
	//test_set_device_info_result();

	//test_get_all_device_id_request();
	//test_get_all_device_id_result();


#endif


		

//#ifndef __WIN_32__
//	_snprintf(buffer, sizeof(buffer), "%s%d", "abcdefg", 12345);
//	printf("buffer: %s\n", buffer);
//#endif

	return 0;
}


