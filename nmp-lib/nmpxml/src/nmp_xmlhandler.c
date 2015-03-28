
#include "config.h"
#include "nmp_xmlhandler.h"
#include "nmp_xmlmem.h"
#include "xml-api.h"



typedef int (*ParseRequestFunc)(void *pvalue, const char *buffer);
typedef int (*ParseResultFunc)(void *pvalue, const char *buffer);


static XmlTable global_xml_table = {0};
XmlTable *get_xml_table()
{
	return &global_xml_table;
}

int register_xml_table(XmlTable *table, xmlid_t id, char *command, 
		NmpCreateXml func_c, NmpParseXml func_p, unsigned int flags)
{
  
	int cmd_size;
	int capacity = 0;
	XmlEntry *entries = NULL;

	J_ASSERT(table);
	
	if (COMMAND_ID_NONE>=(int)id && MAX_COMMAND_ID_SIZE<=(int)id)
	{
		printf("id invalid.\n");
		return -1;
	}

	if (table->n_entries >= table->n_capacity)
	{
		capacity = table->n_capacity ? (2 * table->n_capacity) : 1;
		entries = j_xml_alloc(capacity * sizeof(XmlEntry));
		if (NULL == entries)
			return -1;
		else
		{
			if (0 != table->n_capacity)
			{
				memcpy(entries, table->entries, 
					table->n_capacity * sizeof(XmlEntry));
				j_xml_dealloc(table->entries, 
					table->n_capacity * sizeof(XmlEntry));
			}
			
			table->entries = entries;
			table->n_capacity = capacity;
		}
	}


	table->entries[table->n_entries].id = id;
	
	cmd_size = sizeof(table->entries[table->n_entries].command);

	strncpy(table->entries[table->n_entries].command, command, cmd_size);

	table->entries[table->n_entries].flags = flags;
	table->entries[table->n_entries].func_c = func_c;
	table->entries[table->n_entries].func_p = func_p;

	table->n_entries++;

	return 0;
}

int xml_table_create(XmlTable *table, NmpXmlMsg *msg, 
		char buf[], size_t size, unsigned int flags)
{
	int index = 0;
	
	J_ASSERT(table);

	for (; index<table->n_entries; index++)
	{
	    if (msg->id == table->entries[index].id)
		{

           return (*table->entries[index].func_c)(msg, buf, size, flags);
		}
	}

	printf("xml_table_create() msg id invalid! Has been initialized?\n");
	return -1;
}

void *xml_table_parse(XmlTable *table, xmlid_t id, 
		char buf[], size_t size, int *err, unsigned int flags)
{
	int index;
	NmpXmlMsg *msg = NULL;
	
	J_ASSERT(table);

	for (index=0; index<table->n_entries; index++)
	{
		if (id == table->entries[index].id)
		{
			msg = (*table->entries[index].func_p)(buf, size, err, flags);
			break;
		}
	}
	
	return (void*)msg;
}


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

static __inline__ XmlData *alloc_one_xml_data(xmlid_t id, size_t size)
{
	XmlData *msg = NULL;
	
	if (NULL != (msg = j_xml_alloc(sizeof(XmlData) + size)))
	{
		msg->id = id;
		msg->size = sizeof(XmlData) + size;
		memset(msg->data, 0, size);
	}

	return msg;
}

static __inline__ void xml_data_destroy(XmlData *xml_data)
{
	J_ASSERT(xml_data);

	j_xml_dealloc(xml_data, xml_data->size);
}

static __inline__ void *get_xml_data(XmlData *msg)
{
	return &msg->data[0];
}

/**********************************************
 *根据返回值判断使用堆或者栈
 *0: 使用栈，无需释放，
 *1: 使用堆，必须手动释放
 **********************************************/
static __inline__ int get_xml_buf(char **ptr, char page[], 
					const char *buf, size_t size)
{
	if (J_SDK_MAX_PAGE_SIZE <= size)
	{
		*ptr = j_xml_alloc(size+1);
		memcpy(*ptr, (const void*)buf, size);
		(*ptr)[size] = 0;

		return 1;
	}
	else
	{
		memcpy(page, (const void*)buf, size);
		page[size] = 0;
		*ptr = page;

		return 0;
	}
}

typedef int (*parse_func)(void *pvalue, const char *buffer);

static __inline__ NmpXmlMsg *
create_xml_msg(char buf[], size_t buf_size, 
	int data_id, size_t data_size, parse_func func)
{
	int ret = -1;
	int flag = -1;
	char *new_buf = NULL;
	char page_buf[J_SDK_MAX_PAGE_SIZE];
	
	NmpXmlMsg *msg = NULL;
	void *pri_data = NULL;
	
	flag = get_xml_buf(&new_buf, page_buf, buf, buf_size);
	
	pri_data = j_xml_alloc(data_size);
	ret = (*func)(pri_data, new_buf);
	
	if (1 == flag)
		j_xml_dealloc(new_buf, buf_size+1);
	
	if (0 == ret)
	{
		msg = jpf_xml_msg_new(data_id, pri_data, data_size);
	}
	j_xml_dealloc(pri_data, data_size);
	
	return msg;
}

static __inline__ NmpXmlMsg *
create_xml_msg_2(char buf[], size_t buf_size, 
	int data_id, size_t data_size, parse_func func, NmpXmlMsgDes fin)
{
	int ret = -1;
	int flag = -1;
	char *new_buf = NULL;
	char page_buf[J_SDK_MAX_PAGE_SIZE];
	
	NmpXmlMsg *msg = NULL;
	void *xml_data = NULL;
	
	flag = get_xml_buf(&new_buf, page_buf, buf, buf_size);
	xml_data = j_xml_alloc(data_size);

	ret = (*func)(xml_data, new_buf);
  
	if (1 == flag)
		j_xml_dealloc(new_buf, buf_size+1);
	
	if (0 == ret)
	{
		msg = jpf_xml_msg_new_2(data_id, xml_data, data_size, fin);
	}	
	
	return msg;
}


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int create_get_css_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_css_request_xml(msg->priv_obj, buf, size);
}
int create_get_css_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_css_response_xml(msg->priv_obj, buf, size);
}
int create_pu_register_css(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_pu_register_css_xml(msg->priv_obj, buf, size);
}
int create_register_css_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_register_response_xml(msg->priv_obj, buf, size);
}


int create_register_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_register_request_xml(msg->priv_obj, buf, size);
}

int create_register_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_register_response_xml(msg->priv_obj, buf, size);
}

int create_heart_beat_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{ 
	J_ASSERT(msg && size);
	
	return merge_heart_beat_request_xml(msg->priv_obj, buf, size);
}

int create_heart_beat_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_heart_beat_response_xml(msg->priv_obj, buf, size);
}

int create_get_mds_info_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_mds_info_request_xml(msg->priv_obj, buf, size);
}

int create_get_mds_info_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_mds_info_response_xml(msg->priv_obj, buf, size);
}

int create_change_dispatch_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_change_dispatch_request_xml(msg->priv_obj, buf, size);
}

int create_change_dispatch_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_change_dispatch_result_xml(msg->priv_obj, buf, size);
}

int create_get_device_info(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_device_info_xml(msg->priv_obj, buf, size);
}

int create_device_info_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_device_info_response_xml(msg->priv_obj, buf, size);
}

int create_get_device_ntp_info(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_device_ntp_info_xml(msg->priv_obj, buf, size);
}

int create_device_ntp_info_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_device_ntp_info_response_xml(msg->priv_obj, buf, size);
}

int create_set_device_ntp_info(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_device_ntp_info_xml(msg->priv_obj, buf, size);
}

int create_device_ntp_info_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_device_ntp_info_result_xml(msg->priv_obj, buf, size);
}

int create_get_device_time(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_device_time_xml(msg->priv_obj, buf, size);
}

int create_device_time_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_device_time_response_xml(msg->priv_obj, buf, size);
}

int create_set_device_time(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_device_time_xml(msg->priv_obj, buf, size);
}

int create_device_time_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_device_time_result_xml(msg->priv_obj, buf, size);
}

int create_get_platform_info(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_platform_info_xml(msg->priv_obj, buf, size);
}

int create_platform_info_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_platform_info_response_xml(msg->priv_obj, buf, size);
}
	
int create_set_platform_info(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_platform_info_xml(msg->priv_obj, buf, size);
}

int create_platform_info_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_platform_info_result_xml(msg->priv_obj, buf, size);
}

//network_info
int create_get_network_info(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_network_info_xml(msg->priv_obj, buf, size);
}										
int create_network_info_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_network_info_response_xml(msg->priv_obj, buf, size);
}
int create_set_network_info(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_network_info_xml(msg->priv_obj, buf, size);
}
int create_network_info_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_network_info_result_xml(msg->priv_obj, buf, size);
}

int create_get_pppoe_info(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_pppoe_info_xml(msg->priv_obj, buf, size);
}
int create_pppoe_info_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_pppoe_info_response_xml(msg->priv_obj, buf, size);
}
int create_set_pppoe_info(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_pppoe_info_xml(msg->priv_obj, buf, size);
}

int create_pppoe_info_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_pppoe_info_result_xml(msg->priv_obj, buf, size);
}

//encode_para
int create_get_encode_parameter(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_encode_parameter_xml(msg->priv_obj, buf, size);
}
int create_encode_parameter_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_encode_parameter_response_xml(msg->priv_obj, buf, size);
}
int create_set_encode_parameter(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_encode_parameter_xml(msg->priv_obj, buf, size);
}
											
int create_encode_parameter_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_encode_parameter_result_xml(msg->priv_obj, buf, size);
}

//display_para
int create_get_display_parameter(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_display_parameter_xml(msg->priv_obj, buf, size);
}
int create_display_parameter_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_display_parameter_response_xml(msg->priv_obj, buf, size);
}
int create_set_display_parameter(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_display_parameter_xml(msg->priv_obj, buf, size);
}
int create_display_parameter_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_display_parameter_result_xml(msg->priv_obj, buf, size);
}

//record_para
int create_get_record_parameter(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_record_parameter_xml(msg->priv_obj, buf, size);
}
int create_record_parameter_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_record_parameter_response_xml(msg->priv_obj, buf, size);
}
int create_set_record_parameter(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_record_parameter_xml(msg->priv_obj, buf, size);
}
int create_record_parameter_result(NmpXmlMsg *msg, char buf[],
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_record_parameter_result_xml(msg->priv_obj, buf, size);
}

//hide_para
int create_get_hide_parameter(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_hide_parameter_xml(msg->priv_obj, buf, size);
}
int create_hide_parameter_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_hide_parameter_response_xml(msg->priv_obj, buf, size);
}
int create_set_hide_parameter(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_hide_parameter_xml(msg->priv_obj, buf, size);
}
int create_hide_parameter_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_hide_parameter_result_xml(msg->priv_obj, buf, size);
}

//serial_para
int create_get_serial_parameter(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_serial_parameter_xml(msg->priv_obj, buf, size);
}
int create_serial_parameter_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_serial_parameter_response_xml(msg->priv_obj, buf, size);
}
int create_set_serial_parameter(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_serial_parameter_xml(msg->priv_obj, buf, size);
}
int create_serial_parameter_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_serial_parameter_result_xml(msg->priv_obj, buf, size);
}

//osd_para
int create_get_osd_parameter(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_osd_parameter_xml(msg->priv_obj, buf, size);
}
int create_osd_parameter_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_osd_parameter_response_xml(msg->priv_obj, buf, size);
}
int create_set_osd_parameter(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_osd_parameter_xml(msg->priv_obj, buf, size);
}
int create_osd_parameter_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_osd_parameter_result_xml(msg->priv_obj, buf, size);
}

//ptz_para
int create_get_ptz_parameter(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_ptz_parameter_xml(msg->priv_obj, buf, size);
}
int create_ptz_parameter_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_ptz_parameter_response_xml(msg->priv_obj, buf, size);
}
int create_set_ptz_parameter(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_ptz_parameter_xml(msg->priv_obj, buf, size);
}
int create_ptz_parameter_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_ptz_parameter_result_xml(msg->priv_obj, buf, size);
}

//ftp_para
int create_get_ftp_parameter(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_ftp_parameter_xml(msg->priv_obj, buf, size);
}
int create_ftp_parameter_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_ftp_parameter_response_xml(msg->priv_obj, buf, size);
}
int create_set_ftp_parameter(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_ftp_parameter_xml(msg->priv_obj, buf, size);
}
int create_ftp_parameter_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_ftp_parameter_result_xml(msg->priv_obj, buf, size);
}

//smtp_para 
int create_get_smtp_parameter(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_smtp_parameter_xml(msg->priv_obj, buf, size);
}
int create_smtp_parameter_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_smtp_parameter_response_xml(msg->priv_obj, buf, size);
}
int create_set_smtp_parameter(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_smtp_parameter_xml(msg->priv_obj, buf, size);
}
int create_smtp_parameter_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_smtp_parameter_result_xml(msg->priv_obj, buf, size);
}

//upnp_para
int create_get_upnp_parameter(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_upnp_parameter_xml(msg->priv_obj, buf, size);
}	
int create_upnp_parameter_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_upnp_parameter_response_xml(msg->priv_obj, buf, size);
}
int create_set_upnp_parameter(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_upnp_parameter_xml(msg->priv_obj, buf, size);
}
int create_upnp_parameter_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_upnp_parameter_result_xml(msg->priv_obj, buf, size);
}

//disk_info
int create_get_device_disk_info(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_device_disk_info_xml(msg->priv_obj, buf, size);
}
int create_device_disk_info_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_device_disk_info_response_xml(msg->priv_obj, buf, size);
}

//format_disk
int create_format_disk_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_format_disk_request_xml(msg->priv_obj, buf, size);
}
int create_format_disk_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_format_disk_result_xml(msg->priv_obj, buf, size);
}
int create_submit_format_progress(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_submit_format_progress_xml(msg->priv_obj, buf, size);
}

//move_alarm
int create_get_move_alarm_info(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_move_alarm_info_xml(msg->priv_obj, buf, size);
}
int create_move_alarm_info_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_move_alarm_info_response_xml(msg->priv_obj, buf, size);
}
											
int create_set_move_alarm_info(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_move_alarm_info_xml(msg->priv_obj, buf, size);
}
											
int create_move_alarm_info_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_move_alarm_info_result_xml(msg->priv_obj, buf, size);
}

//lost_alarm
int create_get_lost_alarm_info(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_lost_alarm_info_xml(msg->priv_obj, buf, size);
}
int create_lost_alarm_info_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_lost_alarm_info_response_xml(msg->priv_obj, buf, size);
}
												
int create_set_lost_alarm_info(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_lost_alarm_info_xml(msg->priv_obj, buf, size);
}
										
int create_lost_alarm_info_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_lost_alarm_info_result_xml(msg->priv_obj, buf, size);
}

//hide_alarm
int create_get_hide_alarm_info(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_hide_alarm_info_xml(msg->priv_obj, buf, size);
}
int create_hide_alarm_info_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_hide_alarm_info_response_xml(msg->priv_obj, buf, size);
}
												
int create_set_hide_alarm_info(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_hide_alarm_info_xml(msg->priv_obj, buf, size);
}
											
int create_hide_alarm_info_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_hide_alarm_info_result_xml(msg->priv_obj, buf, size);
}

//io_alarm
int create_get_io_alarm_info(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_io_alarm_info_xml(msg->priv_obj, buf, size);
}
int create_io_alarm_info_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_io_alarm_info_response_xml(msg->priv_obj, buf, size);
}

int create_set_io_alarm_info(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_io_alarm_info_xml(msg->priv_obj, buf, size);
}

int create_io_alarm_info_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_io_alarm_info_result_xml(msg->priv_obj, buf, size);
}

//joint_action
int create_get_joint_action_info(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_joint_action_info_xml(msg->priv_obj, buf, size);
}
int create_joint_action_info_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_joint_action_info_response_xml(msg->priv_obj, buf, size);
}
int create_set_joint_action_info(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_joint_action_info_xml(msg->priv_obj, buf, size);
}
int create_joint_action_info_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_joint_action_info_result_xml(msg->priv_obj, buf, size);
}

//control_ptz
int create_control_ptz_cmd(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_control_ptz_cmd_xml(msg->priv_obj, buf, size);
}
int create_ptz_cmd_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_ptz_cmd_result_xml(msg->priv_obj, buf, size);
}
int create_submit_alarm_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_submit_alarm_request_xml(msg->priv_obj, buf, size);
}
int create_get_media_url_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_media_url_request_xml(msg->priv_obj, buf, size);
}
int create_get_media_url_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_media_url_response_xml(msg->priv_obj, buf, size);
}
int create_get_store_log_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_store_log_request_xml(msg->priv_obj, buf, size);
}
int create_get_store_log_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);

 	return merge_get_store_log_response_xml(msg->priv_obj, buf, size);
}
int create_user_login_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_user_login_request_xml(msg->priv_obj, buf, size);
}
int create_user_login_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_user_login_result_xml(msg->priv_obj, buf, size);
}
int create_user_heart_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{	
	J_ASSERT(msg && size);
	
	return merge_user_heart_request_xml(msg->priv_obj, buf, size);
}
int create_user_heart_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_user_heart_response_xml(msg->priv_obj, buf, size);
}

int create_firmware_upgrade_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return merge_firmware_upgrade_request_xml(msg->priv_obj, buf, size);
}
int create_firmware_upgrade_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return merge_firmware_upgrade_response_xml(msg->priv_obj, buf, size);
}
int create_submit_upgrade_progress(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return merge_submit_upgrade_progress_xml(msg->priv_obj, buf, size);
}

//################################################################################

static __inline__ NmpXmlMsg *
parse_get_info(ParseRequestFunc request_func, 
	xmlid_t id, char buf[], size_t size, int *err, unsigned int flags)
{
	int ret = -1;
	char *new_buf = NULL;
	char page_buf[J_SDK_MAX_PAGE_SIZE];
	
	NmpXmlMsg *msg = NULL;
	XmlData *xml_data = NULL;
	
	J_ASSERT(buf && size);
	
	ret = get_xml_buf(&new_buf, page_buf, buf, size);
	xml_data = alloc_one_xml_data(id, sizeof(Request));
	
	(*request_func)(get_xml_data(xml_data), new_buf);
	if (1 == ret)
		j_xml_dealloc(new_buf, size+1);
	
	msg = jpf_xml_msg_new(XML_DATA_ID(xml_data), 
			(void*)XML_DATA(xml_data), XML_DATA_SIZE(xml_data));
	
	xml_data_destroy(xml_data);
	return msg;
}

static __inline__ NmpXmlMsg *
	parse_set_result(ParseResultFunc result_func, 
						xmlid_t id, char buf[], 
						size_t size, int *err, unsigned int flags)
{
	int ret = -1;
	char *new_buf = NULL;
	char page_buf[J_SDK_MAX_PAGE_SIZE];
	
	NmpXmlMsg *msg = NULL;
	XmlData *xml_data = NULL;
	
	J_ASSERT(buf && size);
	
	ret = get_xml_buf(&new_buf, page_buf, buf, size);
	xml_data = alloc_one_xml_data(id, sizeof(Result));
	
	(*result_func)(get_xml_data(xml_data), new_buf);
	if (1 == ret)
		j_xml_dealloc(new_buf, size+1);
	
	msg = jpf_xml_msg_new(XML_DATA_ID(xml_data), 
			(void*)XML_DATA(xml_data), XML_DATA_SIZE(xml_data));

	xml_data_destroy(xml_data);
	return msg;
}

NmpXmlMsg *parse_get_css_request(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, PU_GET_CSS_REQUEST_ID, 
			sizeof(PuGetCssPacket), parse_get_css_request_xml);
}
NmpXmlMsg *parse_get_css_response(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, PU_GET_CSS_RESPONSE_ID, 
			sizeof(PuGetCssPacket), parse_get_css_response_xml);
}
NmpXmlMsg *parse_pu_register_css(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, REGISTER_CSS_REQUEST_ID, 
			sizeof(PuRegisterCssPacket), parse_pu_register_css_xml);
}

NmpXmlMsg *parse_register_css_response(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, REGISTER_CSS_RESPONSE_ID, 
			sizeof(RegisterResponsePacket), parse_register_response_xml);
}

NmpXmlMsg *parse_register_request(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, REGISTER_REQUEST_ID, 
			sizeof(RegisterRequestPacket), parse_register_request_xml);
}

NmpXmlMsg *parse_register_response(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, REGISTER_RESPONSE_ID, 
			sizeof(RegisterResponsePacket), parse_register_response_xml);
}

NmpXmlMsg *parse_heart_beat_request(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, HEART_BEAT_REQUEST_ID, 
			sizeof(HeartBeatRequestPacket), parse_heart_beat_request_xml);
}

NmpXmlMsg *parse_heart_beat_response(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, HEART_BEAT_RESPONSE_ID, 
			sizeof(HeartBeatResponsePacket), parse_heart_beat_response_xml);
}

NmpXmlMsg *parse_get_mds_info_request(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, GET_MDS_INFO_REQUEST_ID, 
			sizeof(MdsInfoPacket), parse_get_mds_info_request_xml);
}

NmpXmlMsg *parse_get_mds_info_response(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, GET_MDS_INFO_RESPONSE_ID, 
			sizeof(MdsInfoPacket), parse_get_mds_info_response_xml);
}

NmpXmlMsg *parse_change_dispatch_request(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, CHANGE_DISPATCH_REQUEST_ID, 
			sizeof(ChangeDispatchPacket), parse_change_dispatch_request_xml);
}

NmpXmlMsg *parse_change_dispatch_result(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_set_result(parse_change_dispatch_result_xml, CHANGE_DISPATCH_RESULT_ID, 
							buf, size, err, flags);
}

NmpXmlMsg *parse_get_device_info(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_get_info(parse_get_device_info_xml, GET_DEVICE_INFO_ID, 
							buf, size, err, flags);
}

NmpXmlMsg *parse_device_info_response(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, DEVICE_INFO_RESPONSE_ID, 
			sizeof(DeviceInfoPacket), parse_device_info_response_xml);
}

NmpXmlMsg *parse_get_device_ntp_info(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_get_info(parse_get_device_ntp_info_xml, GET_DEVICE_NTP_INFO_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_device_ntp_info_response(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, DEVICE_NTP_INFO_RESPONSE_ID, 
			sizeof(DeviceNTPInfoPacket), parse_device_ntp_info_response_xml);
}
NmpXmlMsg *parse_set_device_ntp_info(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_DEVICE_NTP_INFO_ID, 
			sizeof(DeviceNTPInfoPacket), parse_set_device_ntp_info_xml);
}

NmpXmlMsg *parse_device_ntp_info_result(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_set_result(parse_device_ntp_info_result_xml, DEVICE_NTP_INFO_RESULT_ID, 
							buf, size, err, flags);
}

NmpXmlMsg *parse_get_device_time(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, GET_DEVICE_TIME_ID, 
			sizeof(DeviceTimePacket), parse_get_device_time_xml);
}
NmpXmlMsg *parse_device_time_response(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_set_result(parse_device_time_response_xml, DEVICE_TIME_RESPONSE_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_set_device_time(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_DEVICE_TIME_ID, 
			sizeof(DeviceTimePacket), parse_set_device_time_xml);
}
NmpXmlMsg *parse_device_time_result(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_set_result(parse_device_time_result_xml, DEVICE_TIME_RESULT_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_get_platform_info(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_get_info(parse_get_platform_info_xml, GET_PLATFORM_INFO_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_platform_info_response(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, PLATFORM_INFO_RESPONSE_ID, 
			sizeof(PlatformInfoPacket), parse_platform_info_response_xml);
}
NmpXmlMsg *parse_set_platform_info(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_PLATFORM_INFO_ID, 
			sizeof(PlatformInfoPacket), parse_set_platform_info_xml);
}
NmpXmlMsg *parse_platform_info_result(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_set_result(parse_platform_info_result_xml, PLATFORM_INFO_RESULT_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_get_network_info(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_get_info(parse_get_network_info_xml, GET_NETWORK_INFO_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_network_info_response(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, NETWORK_INFO_RESPONSE_ID, 
			sizeof(NetworkInfoPacket), parse_network_info_response_xml);
}
NmpXmlMsg *parse_set_network_info(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_NETWORK_INFO_ID, 
			sizeof(NetworkInfoPacket), parse_set_network_info_xml);
}
NmpXmlMsg *parse_network_info_result(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_set_result(parse_network_info_result_xml, NETWORK_INFO_RESULT_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_get_pppoe_info(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_get_info(parse_get_pppoe_info_xml, GET_PPPOE_INFO_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_pppoe_info_response(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, PPPOE_INFO_RESPONSE_ID, 
			sizeof(PPPOEInfoPacket), parse_pppoe_info_response_xml);
}
NmpXmlMsg *parse_set_pppoe_info(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_PPPOE_INFO_ID, 
			sizeof(PPPOEInfoPacket), parse_set_pppoe_info_xml);
}
NmpXmlMsg *parse_pppoe_info_result(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_set_result(parse_pppoe_info_result_xml, PPPOE_INFO_RESULT_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_get_encode_parameter(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_get_info(parse_get_encode_parameter_xml, GET_ENCODE_PARAMETER_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_encode_parameter_response(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, ENCODE_PARAMETER_RESPONSE_ID, 
			sizeof(EncodeParameterPacket), parse_encode_parameter_response_xml);
}
NmpXmlMsg *parse_set_encode_parameter(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_ENCODE_PARAMETER_ID, 
			sizeof(EncodeParameterPacket), parse_set_encode_parameter_xml);
}
NmpXmlMsg *parse_encode_parameter_result(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_set_result(parse_encode_parameter_result_xml, ENCODE_PARAMETER_RESULT_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_get_display_parameter(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_get_info(parse_get_display_parameter_xml, GET_DISPLAY_PARAMETER_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_display_parameter_response(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, DISPLAY_PARAMETER_RESPONSE_ID, 
			sizeof(DisplayParameterPacket), parse_display_parameter_response_xml);
}
NmpXmlMsg *parse_set_display_parameter(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_DISPLAY_PARAMETER_ID, 
			sizeof(DisplayParameterPacket), parse_set_display_parameter_xml);
}
NmpXmlMsg *parse_display_parameter_result(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_set_result(parse_display_parameter_result_xml, DISPLAY_PARAMETER_RESULT_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_get_record_parameter(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_get_info(parse_get_record_parameter_xml, GET_RECORD_PARAMETER_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_record_parameter_response(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, RECORD_PARAMETER_RESPONSE_ID, 
			sizeof(RecordParameterPacket), parse_record_parameter_response_xml);
}
NmpXmlMsg *parse_set_record_parameter(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_RECORD_PARAMETER_ID, 
			sizeof(RecordParameterPacket), parse_set_record_parameter_xml);
}
NmpXmlMsg *parse_record_parameter_result(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_set_result(parse_record_parameter_result_xml, RECORD_PARAMETER_RESULT_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_get_hide_parameter(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_get_info(parse_get_hide_parameter_xml, GET_HIDE_PARAMETER_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_hide_parameter_response(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, HIDE_PARAMETER_RESPONSE_ID, 
			sizeof(HideParameterPacket), parse_hide_parameter_response_xml);
}
NmpXmlMsg *parse_set_hide_parameter(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_HIDE_PARAMETER_ID, 
			sizeof(HideParameterPacket), parse_set_hide_parameter_xml);
}
NmpXmlMsg *parse_hide_parameter_result(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_set_result(parse_hide_parameter_result_xml, HIDE_PARAMETER_RESULT_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_get_serial_parameter(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_get_info(parse_get_serial_parameter_xml, GET_SERIAL_PARAMETER_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_serial_parameter_response(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SERIAL_PARAMETER_RESPONSE_ID, 
			sizeof(SerialParameterPacket), parse_serial_parameter_response_xml);
}
NmpXmlMsg *parse_set_serial_parameter(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_SERIAL_PARAMETER_ID, 
			sizeof(SerialParameterPacket), parse_set_serial_parameter_xml);
}
NmpXmlMsg *parse_serial_parameter_result(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_set_result(parse_serial_parameter_result_xml, SERIAL_PARAMETER_RESULT_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_get_osd_parameter(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_get_info(parse_get_osd_parameter_xml, GET_OSD_PARAMETER_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_osd_parameter_response(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, OSD_PARAMETER_RESPONSE_ID, 
			sizeof(OSDParameterPacket), parse_osd_parameter_response_xml);
}
NmpXmlMsg *parse_set_osd_parameter(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_OSD_PARAMETER_ID, 
			sizeof(OSDParameterPacket), parse_set_osd_parameter_xml);
}
NmpXmlMsg *parse_osd_parameter_result(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_set_result(parse_osd_parameter_result_xml, OSD_PARAMETER_RESULT_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_get_ptz_parameter(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_get_info(parse_get_ptz_parameter_xml, GET_PTZ_PARAMETER_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_ptz_parameter_response(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, PTZ_PARAMETER_RESPONSE_ID, 
			sizeof(PTZParameterPacket), parse_ptz_parameter_response_xml);
}
NmpXmlMsg *parse_set_ptz_parameter(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_PTZ_PARAMETER_ID, 
			sizeof(PTZParameterPacket), parse_set_ptz_parameter_xml);
}
NmpXmlMsg *parse_ptz_parameter_result(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_set_result(parse_ptz_parameter_result_xml, PTZ_PARAMETER_RESULT_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_get_ftp_parameter(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_get_info(parse_get_ftp_parameter_xml, GET_FTP_PARAMETER_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_ftp_parameter_response(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, FTP_PARAMETER_RESPONSE_ID, 
			sizeof(FTPParameterPacket), parse_ftp_parameter_response_xml);
}
NmpXmlMsg *parse_set_ftp_parameter(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_FTP_PARAMETER_ID, 
			sizeof(FTPParameterPacket), parse_set_ftp_parameter_xml);
}
NmpXmlMsg *parse_ftp_parameter_result(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_set_result(parse_ftp_parameter_result_xml, FTP_PARAMETER_RESULT_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_get_smtp_parameter(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_get_info(parse_get_smtp_parameter_xml, GET_SMTP_PARAMETER_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_smtp_parameter_response(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SMTP_PARAMETER_RESPONSE_ID, 
			sizeof(SMTPParameterPacket), parse_smtp_parameter_response_xml);
}
NmpXmlMsg *parse_set_smtp_parameter(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_SMTP_PARAMETER_ID, 
			sizeof(SMTPParameterPacket), parse_set_smtp_parameter_xml);
}
NmpXmlMsg *parse_smtp_parameter_result(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_set_result(parse_smtp_parameter_result_xml, SMTP_PARAMETER_RESULT_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_get_upnp_parameter(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_get_info(parse_get_upnp_parameter_xml, GET_UPNP_PARAMETER_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_upnp_parameter_response(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, UPNP_PARAMETER_RESPONSE_ID, 
			sizeof(UPNPParameterPacket), parse_upnp_parameter_response_xml);
}
NmpXmlMsg *parse_set_upnp_parameter(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_UPNP_PARAMETER_ID, 
			sizeof(UPNPParameterPacket), parse_set_upnp_parameter_xml);
}
NmpXmlMsg *parse_upnp_parameter_result(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_set_result(parse_upnp_parameter_result_xml, UPNP_PARAMETER_RESULT_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_get_device_disk_info(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_get_info(parse_get_device_disk_info_xml, GET_DEVICE_DISK_INFO_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_device_disk_info_response(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, DEVICE_DISK_INFO_RESPONSE_ID, 
			sizeof(DeviceDiskInfoPacket), parse_device_disk_info_response_xml);
}
NmpXmlMsg *parse_format_disk_request(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, FORMAT_DISK_REQUEST_ID, 
			sizeof(FormatDiskPacket), parse_format_disk_request_xml);
}
NmpXmlMsg *parse_format_disk_result(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_set_result(parse_format_disk_result_xml, FORMAT_DISK_RESULT_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_submit_format_progress(char buf[], size_t size, int *err, unsigned int flags)
{
	return create_xml_msg(buf, size, SUBMIT_FORMAT_PROGRESS_ID, 
			sizeof(FormatProgressPacket), parse_submit_format_progress_xml);
}
	
NmpXmlMsg *parse_get_move_alarm_info(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_get_info(parse_get_move_alarm_info_xml, GET_MOVE_ALARM_INFO_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_move_alarm_info_response(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, MOVE_ALARM_INFO_RESPONSE_ID, 
			sizeof(MoveAlarmPacket), parse_move_alarm_info_response_xml);
}
NmpXmlMsg *parse_set_move_alarm_info(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_MOVE_ALARM_INFO_ID, 
			sizeof(MoveAlarmPacket), parse_set_move_alarm_info_xml);
}
NmpXmlMsg *parse_move_alarm_info_result(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_set_result(parse_move_alarm_info_result_xml, MOVE_ALARM_INFO_RESULT_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_get_lost_alarm_info(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_get_info(parse_get_lost_alarm_info_xml, GET_LOST_ALARM_INFO_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_lost_alarm_info_response(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, LOST_ALARM_INFO_RESPONSE_ID, 
			sizeof(LostAlarmPacket), parse_lost_alarm_info_response_xml);
}
NmpXmlMsg *parse_set_lost_alarm_info(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_LOST_ALARM_INFO_ID, 
			sizeof(LostAlarmPacket), parse_set_lost_alarm_info_xml);
}
NmpXmlMsg *parse_lost_alarm_info_result(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_set_result(parse_lost_alarm_info_result_xml, LOST_ALARM_INFO_RESULT_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_get_hide_alarm_info(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_get_info(parse_get_hide_alarm_info_xml, GET_HIDE_ALARM_INFO_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_hide_alarm_info_response(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, HIDE_ALARM_INFO_RESPONSE_ID, 
			sizeof(HideAlarmPacket), parse_hide_alarm_info_response_xml);
}
NmpXmlMsg *parse_set_hide_alarm_info(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_HIDE_ALARM_INFO_ID, 
			sizeof(HideAlarmPacket), parse_set_hide_alarm_info_xml);
}
NmpXmlMsg *parse_hide_alarm_info_result(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_set_result(parse_hide_alarm_info_result_xml, HIDE_ALARM_INFO_RESULT_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_get_io_alarm_info(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_get_info(parse_get_io_alarm_info_xml, GET_IO_ALARM_INFO_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_io_alarm_info_response(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, IO_ALARM_INFO_RESPONSE_ID, 
			sizeof(IoAlarmPacket), parse_io_alarm_info_response_xml);
}
NmpXmlMsg *parse_set_io_alarm_info(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_IO_ALARM_INFO_ID, 
			sizeof(IoAlarmPacket), parse_set_io_alarm_info_xml);
}
NmpXmlMsg *parse_io_alarm_info_result(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_set_result(parse_io_alarm_info_result_xml, IO_ALARM_INFO_RESULT_ID, 
							buf, size, err, flags);
}
//////////////////////////////////////////////////////////////////////////////////////////////
NmpXmlMsg *parse_get_joint_action_info(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_get_info(parse_get_joint_action_info_xml, GET_JOINT_ACTION_INFO_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_joint_action_info_response(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, JOINT_ACTION_INFO_RESPONSE_ID, 
			sizeof(JointActionPacket), parse_joint_action_info_response_xml);
}
NmpXmlMsg *parse_set_joint_action_info(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_JOINT_ACTION_INFO_ID, 
			sizeof(JointActionPacket), parse_set_joint_action_info_xml);
}
NmpXmlMsg *parse_joint_action_info_result(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_set_result(parse_joint_action_info_result_xml, JOINT_ACTION_INFO_RESULT_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_control_ptz_cmd(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, CONTROL_PTZ_COMMAND_ID, 
			sizeof(PTZControlPacket), parse_control_ptz_cmd_xml);
}
NmpXmlMsg *parse_ptz_cmd_result(char buf[], size_t size, int *err, unsigned int flags)
{
	return parse_set_result(parse_ptz_cmd_response_xml, PTZ_COMMAND_RESULT_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_submit_alarm_request(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SUBMIT_ALARM_REQUEST_ID, 
			sizeof(SubmitAlarmPacket), parse_submit_alarm_request_xml);
}
NmpXmlMsg *parse_get_media_url_request(char buf[], size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, GET_MEDIA_URL_REQUEST_ID, 
			sizeof(MediaUrlPacket), parse_get_media_url_request_xml);
}
NmpXmlMsg *parse_get_media_url_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, GET_MEDIA_URL_RESPONSE_ID, 
			sizeof(MediaUrlPacket), parse_get_media_url_response_xml);
}

NmpXmlMsg *parse_get_store_log_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, GET_STORE_LOG_REQUEST_ID, 
			sizeof(struct __StoreLogPacket), parse_get_store_log_request_xml);
}
NmpXmlMsg *parse_get_store_log_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);

	return create_xml_msg(buf, size, GET_STORE_LOG_RESPONSE_ID, 
			sizeof(struct __StoreLogPacket), parse_get_store_log_response_xml);
}
NmpXmlMsg *parse_user_login_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, USER_LONGI_REQUEST_ID, 
			sizeof(struct __UserInfo), parse_user_login_request_xml);
}
NmpXmlMsg *parse_user_login_result(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	return create_xml_msg(buf, size, USER_LONGI_RESULT_ID, 
			sizeof(int), parse_user_login_result_xml);
}
NmpXmlMsg *parse_user_heart_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, USER_HEART_REQUEST_ID, 
			sizeof(struct __UserHeart), parse_user_heart_request_xml);
}
NmpXmlMsg *parse_user_heart_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, USER_HEART_RESPONSE_ID, 
			sizeof(struct __UserHeart), parse_user_heart_response_xml);
}

NmpXmlMsg *parse_firmware_upgrade_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, FIRMWARE_UPGRADE_REQUEST_ID, 
			sizeof(FirmwareUpgradePacket), parse_firmware_upgrade_request_xml);
}
NmpXmlMsg *parse_firmware_upgrade_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, FIRMWARE_UPGRADE_RESPONSE_ID, 
			sizeof(FirmwareUpgradePacket), parse_firmware_upgrade_response_xml);
}
NmpXmlMsg *parse_submit_upgrade_progress(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SUBMIT_UPGRADE_PROGRESS_ID, 
			sizeof(UpgradeProgressPacket), parse_submit_upgrade_progress_xml);
}


//####################################################################
# ifdef HAVE_PROXY_INFO
NmpXmlMsg *parse_add_user_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, ADD_USER_REQUEST_ID, 
			sizeof(struct __UserInfo), parse_add_user_request_xml);
}
NmpXmlMsg *parse_add_user_result(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, ADD_USER_RESULT_ID, 
			sizeof(int), parse_add_user_result_xml);
}
NmpXmlMsg *parse_del_user_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, DEL_USER_REQUEST_ID, 
			sizeof(struct __UserInfo), parse_del_user_request_xml);
}
NmpXmlMsg *parse_del_user_result(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, DEL_USER_RESULT_ID, 
			sizeof(int), parse_del_user_result_xml);
}
static void destory_user_list_info(void *pri_data, size_t pri_size)
{
	struct _prx_user_st *user_list;
	
	J_ASSERT(pri_data && pri_size);
	
	user_list = (struct _prx_user_st*)pri_data;
	if (user_list->count)
	{
		j_xml_dealloc(user_list->user_info, 
			user_list->count * sizeof(JUserInfo));
	}
	j_xml_dealloc(user_list, sizeof(struct _prx_user_st));
	
	return ;
}
NmpXmlMsg *parse_user_list_info(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg_2(buf, size, USER_LIST_INFO_ID, sizeof(prx_user_st), 
			parse_proxy_user_list_xml, destory_user_list_info);
}
static void destory_device_list_info(void *pri_data, size_t pri_size)
{
	struct _prx_device_st *dev_list;
	
	J_ASSERT(pri_data && pri_size);
	
	dev_list = (struct _prx_device_st*)pri_data;
	if (dev_list->count)
	{
		j_xml_dealloc(dev_list->device_info, 
			dev_list->count * sizeof(struct _prx_device_info));
	}
	j_xml_dealloc(dev_list, sizeof(struct _prx_device_info));
	
	return ;
}
NmpXmlMsg *parse_device_list_info(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg_2(buf, size, DEVICE_LIST_INFO_ID, sizeof(prx_device_st), 
			parse_proxy_device_list_xml, destory_device_list_info);
}
static void destory_factory_list_info(void *pri_data, size_t pri_size)
{
	struct _prx_factory_list *factory_list;
	
	J_ASSERT(pri_data && pri_size);
	
	factory_list = (struct _prx_factory_list*)pri_data;
	if (factory_list->count)
	{
		j_xml_dealloc(factory_list->factory, 
			factory_list->count * sizeof(struct _prx_factory_info));
	}
	j_xml_dealloc(factory_list, sizeof(struct _prx_factory_info));
	
	return ;
}
NmpXmlMsg *parse_factory_list_info(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg_2(buf, size, FACTORY_LIST_INFO_ID, sizeof(prx_factory_list), 
			parse_proxy_factory_list_xml, destory_factory_list_info);
}
NmpXmlMsg *parse_get_factory_info_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, GET_FACTORY_REQUEST_ID, 
			sizeof(prx_factory_list), parse_get_factory_info_request_xml);
}
NmpXmlMsg *parse_get_factory_info_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, GET_FACTORY_RESPONSE_ID, 
			sizeof(prx_factory_list), parse_get_factory_info_response_xml);
}
NmpXmlMsg *parse_fuzzy_find_user_request(char buf[], 
			size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, FUZZY_FIND_USER_REQUEST_ID, 
			sizeof(struct __UserInfo), parse_fuzzy_find_user_request_xml);
}
NmpXmlMsg *parse_fuzzy_find_user_result(char buf[], 
			size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, FUZZY_FIND_USER_RESULT_ID, 
			sizeof(prx_user_st), parse_fuzzy_find_user_result_xml);
}
NmpXmlMsg *parse_modify_password_request(char buf[], 
			size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, MODIFY_PASSWORD_REQUEST_ID, 
			sizeof(prx_modify_pwd), parse_modify_password_request_xml);
}
NmpXmlMsg *parse_modify_password_result(char buf[], 
			size_t size, int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, MODIFY_PASSWORD_RESULT_ID, 
			sizeof(int), parse_modify_password_result_xml);
}
NmpXmlMsg *parse_add_device_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, ADD_DEVICE_REQUEST_ID, 
			sizeof(prx_device_info), parse_add_device_request_xml);
}
NmpXmlMsg *parse_add_device_result(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, ADD_DEVICE_RESULT_ID, 
			sizeof(int), parse_add_device_result_xml);
}
NmpXmlMsg *parse_del_device_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, DEL_DEVICE_REQUEST_ID, 
			sizeof(int), parse_del_device_request_xml);
}
NmpXmlMsg *parse_del_device_result(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, DEL_DEVICE_RESULT_ID, 
			sizeof(int), parse_del_device_result_xml);
}
NmpXmlMsg *parse_get_device_info_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, GET_DEVICE_INFO_REQUEST_ID, 
			sizeof(struct _prx_page_device), parse_get_device_info_request_xml);
}
NmpXmlMsg *parse_get_device_info_result(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, GET_DEVICE_INFO_RESULT_ID, 
			sizeof(prx_device_info), parse_get_device_info_result_xml);
}
NmpXmlMsg *parse_set_device_info_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_DEVICE_INFO_REQUEST_ID, 
			sizeof(prx_device_info), parse_set_device_info_request_xml);
}
NmpXmlMsg *parse_set_device_info_result(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_DEVICE_INFO_RESULT_ID, 
			sizeof(int), parse_set_device_info_result_xml);
}
NmpXmlMsg *parse_get_all_device_id_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, GET_ALL_DEVICE_ID_REQUEST_ID, 
			sizeof(int), parse_get_all_device_id_request_xml);
}
NmpXmlMsg *parse_get_all_device_id_result(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, GET_ALL_DEVICE_ID_RESULT_ID, 
			sizeof(prx_device_id_st), parse_get_all_device_id_result_xml);
}
NmpXmlMsg *parse_get_server_config_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, GET_SERVER_CONFIG_REQUEST_ID, 
			sizeof(prx_server_config), parse_get_server_config_request_xml);
}
NmpXmlMsg *parse_get_server_config_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, GET_SERVER_CONFIG_RESPONSE_ID, 
			sizeof(prx_server_config), parse_get_server_config_response_xml);
}
NmpXmlMsg *parse_set_server_config_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_SERVER_CONFIG_REQUEST_ID, 
			sizeof(prx_server_config), parse_set_server_config_request_xml);
}
NmpXmlMsg *parse_set_server_config_result(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_SERVER_CONFIG_RESULT_ID, 
			sizeof(int), parse_set_server_config_result_xml);
}
NmpXmlMsg *parse_download_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, DOWNLOAD_DATA_REQUEST_ID, 
			sizeof(prx_backup), parse_download_request_xml);
}
NmpXmlMsg *parse_download_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, DOWNLOAD_DATA_RESPONSE_ID, 
			sizeof(prx_backup), parse_download_response_xml);
}
NmpXmlMsg *parse_upload_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, UPLOAD_DATA_REQUEST_ID, 
			sizeof(prx_backup), parse_upload_request_xml);
}
NmpXmlMsg *parse_upload_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, UPLOAD_DATA_RESPONSE_ID, 
			sizeof(prx_backup), parse_upload_response_xml);
}
NmpXmlMsg *parse_limit_broadcast_status(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);

	return create_xml_msg(buf, size, LIMIT_BROADCASE_STATUE_ID, 
			sizeof(prx_limit), parse_limit_broadcast_status_xml);
}


//##########################################################################
int create_add_user_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_add_user_request_xml(msg->priv_obj, buf, size);
}
int create_add_user_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_add_user_result_xml(msg->priv_obj, buf, size);
}
int create_del_user_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_del_user_request_xml(msg->priv_obj, buf, size);
}
int create_del_user_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_del_user_result_xml(msg->priv_obj, buf, size);
}
int create_user_list_info(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_proxy_user_list_xml(msg->priv_obj, buf, size);
}
int create_device_list_info(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_proxy_device_list_xml(msg->priv_obj, buf, size);
}
int create_factory_list_info(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_proxy_factory_list_xml(msg->priv_obj, buf, size);
}
int create_get_factory_info_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_factory_info_response_xml(msg->priv_obj, buf, size);
}
int create_fuzzy_find_user_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_fuzzy_find_user_request_xml(msg->priv_obj, buf, size);
}
int create_fuzzy_find_user_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_fuzzy_find_user_result_xml(msg->priv_obj, buf, size);
}
int create_modify_password_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_modify_password_request_xml(msg->priv_obj, buf, size);
}
int create_modify_password_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_modify_password_result_xml(msg->priv_obj, buf, size);
}
int create_add_device_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_add_device_request_xml(msg->priv_obj, buf, size);
}
int create_add_device_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_add_device_result_xml(msg->priv_obj, buf, size);
}
int create_del_device_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_del_device_request_xml(msg->priv_obj, buf, size);
}
int create_del_device_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_del_device_result_xml(msg->priv_obj, buf, size);
}
int create_get_device_info_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_device_info_request_xml(msg->priv_obj, buf, size);
}
int create_get_device_info_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_device_info_result_xml(msg->priv_obj, buf, size);
}
int create_set_device_info_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_device_info_request_xml(msg->priv_obj, buf, size);
}
int create_set_device_info_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_device_info_result_xml(msg->priv_obj, buf, size);
}
int create_get_all_device_id_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_all_device_id_request_xml(msg->priv_obj, buf, size);
}
int create_get_all_device_id_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_all_device_id_result_xml(msg->priv_obj, buf, size);
}

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
NmpXmlMsg *parse_proxy_page_user_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, GET_USER_INFO_REQUEST_ID, 
			sizeof(struct _prx_page_user), parse_proxy_page_user_request_xml);
}
int create_proxy_page_user_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{  
	J_ASSERT(msg && size);

 	return merge_proxy_page_user_response_xml(msg->priv_obj, buf, size);
}

int create_broadcast_add_user(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return merge_broadcast_add_user_xml(msg->priv_obj, buf, size);
}
int create_broadcast_del_user(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return merge_broadcast_del_user_xml(msg->priv_obj, buf, size);
}
int create_broadcast_add_device(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return merge_broadcast_add_device_xml(msg->priv_obj, buf, size);
}
int create_broadcast_del_device(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return merge_broadcast_del_device_xml(msg->priv_obj, buf, size);
}
int create_broadcast_modify_device(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return merge_broadcast_modify_device_xml(msg->priv_obj, buf, size);
}
int create_broadcast_device_status(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return merge_broadcast_device_status_xml(msg->priv_obj, buf, size);
}
int create_get_server_config_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_server_config_request_xml(msg->priv_obj, buf, size);
}
int create_get_server_config_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_server_config_response_xml(msg->priv_obj, buf, size);
}
int create_set_server_config_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_server_config_request_xml(msg->priv_obj, buf, size);
}
int create_set_server_config_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_server_config_result_xml(msg->priv_obj, buf, size);
}

int create_download_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_download_request_xml(msg->priv_obj, buf, size);
}
int create_download_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_download_response_xml(msg->priv_obj, buf, size);
}
int create_upload_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_upload_request_xml(msg->priv_obj, buf, size);
}
int create_upload_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_upload_response_xml(msg->priv_obj, buf, size);
}


# endif

//channel info
NmpXmlMsg *parse_get_channel_info_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return parse_get_info(parse_get_channel_info_request_xml, 
			GET_CHANNEL_INFO_REQUEST_ID, buf, size, err, flags);
}
NmpXmlMsg *parse_get_channel_info_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, GET_CHANNEL_INFO_RESPONSE_ID, 
			sizeof(ChannelInfoPacket), parse_get_channel_info_response_xml);
}
NmpXmlMsg *parse_set_channel_info_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_CHANNEL_INFO_REQUEST_ID, 
			sizeof(ChannelInfoPacket), parse_set_channel_info_request_xml);
}
NmpXmlMsg *parse_set_channel_info_result(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return parse_set_result(parse_set_channel_info_request_xml, 
			SET_CHANNEL_INFO_RESULT_ID, buf, size, err, flags);
}

//picture info
NmpXmlMsg *parse_get_picture_info_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return parse_get_info(parse_get_picture_info_request_xml, 
			GET_PICTURE_INFO_REQUEST_ID, buf, size, err, flags);
}
NmpXmlMsg *parse_get_picture_info_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, GET_PICTURE_INFO_RESPONSE_ID, 
			sizeof(PictureInfoPacket), parse_get_picture_info_response_xml);
}
NmpXmlMsg *parse_set_picture_info_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_PICTURE_INFO_REQUEST_ID, 
			sizeof(PictureInfoPacket), parse_set_picture_info_request_xml);
}
NmpXmlMsg *parse_set_picture_info_result(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return parse_set_result(parse_set_picture_info_result_xml, 
			SET_PICTURE_INFO_RESULT_ID, buf, size, err, flags);
}

//hl picture info
/*NmpXmlMsg *parse_get_hl_picture_info_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return parse_get_info(parse_get_hl_picture_info_request_xml, 
			GET_HL_PICTURE_INFO_REQUEST_ID, buf, size, err, flags);
}
NmpXmlMsg *parse_get_hl_picture_info_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, GET_HL_PICTURE_INFO_RESPONSE_ID, 
			sizeof(HLPictureInfoPacket), parse_get_hl_picture_info_response_xml);
}
NmpXmlMsg *parse_set_hl_picture_info_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_HL_PICTURE_INFO_REQUEST_ID, 
			sizeof(HLPictureInfoPacket), parse_set_hl_picture_info_request_xml);
}
NmpXmlMsg *parse_set_hl_picture_info_result(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return parse_set_result(parse_set_hl_picture_info_result_xml, 
			SET_HL_PICTURE_INFO_RESULT_ID, buf, size, err, flags);
}*/

//wifi config
NmpXmlMsg *parse_get_wifi_config_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return parse_get_info(parse_get_wifi_config_request_xml, 
			GET_WIFI_CONFIG_REQUEST_ID, buf, size, err, flags);
}
NmpXmlMsg *parse_get_wifi_config_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, GET_WIFI_CONFIG_RESPONSE_ID, 
			sizeof(WifiConfigPacket), parse_get_wifi_config_response_xml);
}
NmpXmlMsg *parse_set_wifi_config_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_WIFI_CONFIG_REQUEST_ID, 
			sizeof(WifiConfigPacket), parse_set_wifi_config_request_xml);
}
NmpXmlMsg *parse_set_wifi_config_result(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return parse_set_result(parse_set_wifi_config_result_xml, 
			SET_WIFI_CONFIG_RESULT_ID, buf, size, err, flags);
}

//wifi search
NmpXmlMsg *parse_wifi_search_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return parse_get_info(parse_wifi_search_request_xml, 
			WIFI_SEARCH_REQUEST_ID, buf, size, err, flags);
}
NmpXmlMsg *parse_wifi_search_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, WIFI_SEARCH_RESPONSE_ID, 
			sizeof(WifiSearchResPacket), parse_wifi_search_response_xml);
}

//network status
NmpXmlMsg *parse_get_network_status_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return parse_get_info(parse_get_network_status_request_xml, 
			GET_NETWORK_STATUS_REQUEST_ID, buf, size, err, flags);
}
NmpXmlMsg *parse_get_network_status_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, GET_NETWORK_STATUS_RESPONSE_ID, 
			sizeof(NetworkStatusPacket), parse_get_network_status_response_xml);
}

//control device
NmpXmlMsg *parse_control_device_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	/*return parse_get_info(parse_control_device_request_xml, 
			CONTROL_DEVICE_REQUEST_ID, buf, size, err, flags);*/
	return create_xml_msg(buf, size, CONTROL_DEVICE_REQUEST_ID, 
			sizeof(ControlDevicePacket), parse_control_device_request_xml);
}
NmpXmlMsg *parse_control_device_result(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	/*return create_xml_msg(buf, size, CONTROL_DEVICE_RESULT_ID, 
			sizeof(ControlDevicePacket), parse_control_device_result_xml);*/
	return parse_set_result(parse_control_device_result_xml, 
			CONTROL_DEVICE_RESULT_ID, buf, size, err, flags);
}

//ddns config
NmpXmlMsg *parse_get_ddns_config_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return parse_get_info(parse_get_ddns_config_request_xml, 
			GET_DDNS_CONFIG_REQUEST_ID, buf, size, err, flags);
}
NmpXmlMsg *parse_get_ddns_config_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, GET_DDNS_CONFIG_RESPONSE_ID, 
			sizeof(DdnsConfigPacket), parse_get_ddns_config_response_xml);
}
NmpXmlMsg *parse_set_ddns_config_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_DDNS_CONFIG_REQUEST_ID, 
			sizeof(DdnsConfigPacket), parse_set_ddns_config_request_xml);
}
NmpXmlMsg *parse_set_ddns_config_result(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return parse_set_result(parse_set_ddns_config_result_xml, 
			SET_DDNS_CONFIG_RESULT_ID, buf, size, err, flags);
}
NmpXmlMsg *parse_get_def_display_info_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	return parse_get_info(parse_get_def_display_info_request_xml, GET_DEF_DISPLAY_INFO_REQUEST_ID, 
							buf, size, err, flags);
}
NmpXmlMsg *parse_get_def_display_info_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, GET_DEF_DISPLAY_INFO_RESPONSE_ID, 
			sizeof(DisplayParameterPacket), parse_get_def_display_info_response_xml);
}

//default picture info
NmpXmlMsg *parse_get_def_picture_info_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return parse_get_info(parse_get_def_picture_info_request_xml, 
			GET_DEF_PICTURE_INFO_REQUEST_ID, buf, size, err, flags);
}
NmpXmlMsg *parse_get_def_picture_info_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, GET_DEF_PICTURE_INFO_RESPONSE_ID, 
			sizeof(PictureInfoPacket), parse_get_def_picture_info_response_xml);
}

//avd config
NmpXmlMsg *parse_get_avd_config_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return parse_get_info(parse_get_avd_config_request_xml, 
			GET_AVD_CONFIG_REQUEST_ID, buf, size, err, flags);
}
NmpXmlMsg *parse_get_avd_config_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, GET_AVD_CONFIG_RESPONSE_ID, 
			sizeof(AvdConfigPacket), parse_get_avd_config_response_xml);
}
NmpXmlMsg *parse_set_avd_config_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_AVD_CONFIG_REQUEST_ID, 
			sizeof(AvdConfigPacket), parse_set_avd_config_request_xml);
}
NmpXmlMsg *parse_set_avd_config_result(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return parse_set_result(parse_set_avd_config_result_xml, 
			SET_AVD_CONFIG_RESULT_ID, buf, size, err, flags);
}

//channel info
int create_get_channel_info_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_channel_info_request_xml(msg->priv_obj, buf, size);
}
int create_get_channel_info_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_channel_info_response_xml(msg->priv_obj, buf, size);
}
int create_set_channel_info_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_channel_info_request_xml(msg->priv_obj, buf, size);
}
int create_set_channel_info_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_channel_info_result_xml(msg->priv_obj, buf, size);
}

//picture info
int create_get_picture_info_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_picture_info_request_xml(msg->priv_obj, buf, size);
}
int create_get_picture_info_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_picture_info_response_xml(msg->priv_obj, buf, size);
}
int create_set_picture_info_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_picture_info_request_xml(msg->priv_obj, buf, size);
}
int create_set_picture_info_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_picture_info_result_xml(msg->priv_obj, buf, size);
}

//wifi config
int create_get_wifi_config_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_wifi_config_request_xml(msg->priv_obj, buf, size);
}
int create_get_wifi_config_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_wifi_config_response_xml(msg->priv_obj, buf, size);
}
int create_set_wifi_config_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_wifi_config_request_xml(msg->priv_obj, buf, size);
}
int create_set_wifi_config_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_wifi_config_result_xml(msg->priv_obj, buf, size);
}

//wifi search
int create_wifi_search_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_wifi_search_request_xml(msg->priv_obj, buf, size);
}
int create_wifi_search_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_wifi_search_response_xml(msg->priv_obj, buf, size);
}

//network status
int create_get_network_status_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_network_status_request_xml(msg->priv_obj, buf, size);
}
int create_get_network_status_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_network_status_response_xml(msg->priv_obj, buf, size);
}

//control device
int create_control_device_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_control_device_request_xml(msg->priv_obj, buf, size);
}
int create_control_device_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_control_device_result_xml(msg->priv_obj, buf, size);
}

//ddns config
int create_get_ddns_config_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_ddns_config_request_xml(msg->priv_obj, buf, size);
}
int create_get_ddns_config_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_ddns_config_response_xml(msg->priv_obj, buf, size);
}
int create_set_ddns_config_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_ddns_config_request_xml(msg->priv_obj, buf, size);
}
int create_set_ddns_config_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_ddns_config_result_xml(msg->priv_obj, buf, size);
}

//default display_para
int create_get_def_display_info_requst(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_def_display_info_request_xml(msg->priv_obj, buf, size);
}
int create_get_def_display_info_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_def_display_info_response_xml(msg->priv_obj, buf, size);
}

//default picture info
int create_get_def_picture_info_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_def_picture_info_request_xml(msg->priv_obj, buf, size);
}
int create_get_def_picture_info_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_def_picture_info_response_xml(msg->priv_obj, buf, size);
}

//avd config
int create_get_avd_config_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_avd_config_request_xml(msg->priv_obj, buf, size);
}
int create_get_avd_config_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_avd_config_response_xml(msg->priv_obj, buf, size);
}
int create_set_avd_config_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_avd_config_request_xml(msg->priv_obj, buf, size);
}
int create_set_avd_config_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_avd_config_result_xml(msg->priv_obj, buf, size);
}























static void destory_transparent_param(void *priv_obj, size_t size)
{
	TransparentPacket *trans = NULL;
	
	J_ASSERT(priv_obj && size);

	trans = (TransparentPacket*)priv_obj;

	if (trans->data && trans->length)
		j_xml_dealloc((void*)trans->data, trans->length);
	j_xml_dealloc((void*)trans, sizeof(TransparentPacket));
}
static NmpXmlMsg *create_transparent_param_xml_msg(char buf[], 
		size_t buf_size, int data_id, parse_func func)
{
	int ret = -1;
	int flag = -1;
	char *new_buf = NULL;
	char page_buf[J_SDK_MAX_PAGE_SIZE];
	
	NmpXmlMsg *msg = NULL;
	TransparentPacket *tran_packet = NULL;
	
	J_ASSERT(buf && buf_size);
	
	flag = get_xml_buf(&new_buf, page_buf, buf, buf_size);
	tran_packet = j_xml_alloc(sizeof(TransparentPacket));
	
	ret = (*func)(tran_packet, new_buf);
	
	if (1 == flag)
		j_xml_dealloc(new_buf, buf_size+1);
	
	if (0 == ret)
	{
		msg = jpf_xml_msg_new_2(data_id, (void*)tran_packet, 
				sizeof(TransparentPacket), destory_transparent_param);
	}
	
	return msg;
}

NmpXmlMsg *parse_get_transparent_param_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_transparent_param_xml_msg(buf, size, 
				GET_TRANSPARENTPARAM_REQUEST_ID, 
				parse_get_transparent_param_request_xml);
}
NmpXmlMsg *parse_get_transparent_param_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_transparent_param_xml_msg(buf, size, 
				GET_TRANSPARENTPARAM_RESPONSE_ID, 
				parse_get_transparent_param_response_xml);
}
NmpXmlMsg *parse_set_transparent_param_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_transparent_param_xml_msg(buf, size, 
				SET_TRANSPARENTPARAM_REQUEST_ID, 
				parse_set_transparent_param_request_xml);
}
NmpXmlMsg *parse_set_transparent_param_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_TRANSPARENTPARAM_RESPONSE_ID, 
			sizeof(int), parse_set_transparent_param_response_xml);
}
NmpXmlMsg *parse_transparent_notify_enevt(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_transparent_param_xml_msg(buf, size, 
				TRANSPARENTPARAM_NOTIFYEVENT_ID, 
				parse_transparent_notify_enevt_xml);
}
NmpXmlMsg *parse_transparent_control_device(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_transparent_param_xml_msg(buf, size, 
				TRANSPARENTPARAM_CONTROLDEVICE_ID, 
				parse_transparent_control_device_xml);
}
int create_get_transparent_param_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_transparent_param_request_xml(msg->priv_obj, buf, size);
}
int create_get_transparent_param_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_transparent_param_response_xml(msg->priv_obj, buf, size);
}
int create_set_transparent_param_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_transparent_param_request_xml(msg->priv_obj, buf, size);
}
int create_set_transparent_param_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_transparent_param_response_xml(msg->priv_obj, buf, size);
}
int create_transparent_notify_enevt(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_transparent_notify_enevt_xml(msg->priv_obj, buf, size);
}
int create_transparent_control_device(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_transparent_control_device_xml(msg->priv_obj, buf, size);
}

//##########################################################################
# ifdef _USE_DECODER_PROTO_

NmpXmlMsg *parse_query_division_mode_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, QUERY_DIVISION_MODE_REQUEST_ID, 
			sizeof(DivisionModePacket), parse_query_division_mode_request_xml);
}
NmpXmlMsg *parse_query_division_mode_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, QUERY_DIVISION_MODE_RESPONSE_ID, 
			sizeof(DivisionModePacket), parse_query_division_mode_response_xml);
}

int create_query_division_mode_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_query_division_mode_request_xml(msg->priv_obj, buf, size);
}
int create_query_division_mode_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_query_division_mode_response_xml(msg->priv_obj, buf, size);
}

NmpXmlMsg *parse_get_screen_state_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, GET_SCREEN_STATE_REQUEST_ID, 
			sizeof(ScreenStatePacket), parse_get_screen_state_request_xml);
}
NmpXmlMsg *parse_get_screen_state_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, GET_SCREEN_STATE_RESPONSE_ID, 
			sizeof(ScreenStatePacket), parse_get_screen_state_response_xml);
}

int create_get_screen_state_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_screen_state_request_xml(msg->priv_obj, buf, size);
}
int create_get_screen_state_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_screen_state_response_xml(msg->priv_obj, buf, size);
}

NmpXmlMsg *parse_set_division_mode_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_DIVISION_MODE_REQUEST_ID, 
			sizeof(ChangeDModePacket), parse_set_division_mode_request_xml);
}
NmpXmlMsg *parse_set_division_mode_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_DIVISION_MODE_RESULT_ID, 
			sizeof(int), parse_set_division_mode_response_xml);
}

int create_set_division_mode_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_division_mode_request_xml(msg->priv_obj, buf, size);
}
int create_set_division_mode_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_division_mode_response_xml(msg->priv_obj, buf, size);
}

NmpXmlMsg *parse_set_full_screen_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_FULL_SCREEN_REQUEST_ID, 
			sizeof(FullScreenPacket), parse_set_full_screen_request_xml);
}
NmpXmlMsg *parse_set_full_screen_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_FULL_SCREEN_RESULT_ID, 
			sizeof(int), parse_set_full_screen_response_xml);
}

int create_set_full_screen_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_full_screen_request_xml(msg->priv_obj, buf, size);
}
int create_set_full_screen_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_full_screen_response_xml(msg->priv_obj, buf, size);
}

NmpXmlMsg *parse_exit_full_screen_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, EXIT_FULL_SCREEN_REQUEST_ID, 
			sizeof(int), parse_exit_full_screen_request_xml);
}
NmpXmlMsg *parse_exit_full_screen_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, EXIT_FULL_SCREEN_RESULT_ID, 
			sizeof(int), parse_exit_full_screen_response_xml);
}

int create_exit_full_screen_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_exit_full_screen_request_xml(msg->priv_obj, buf, size);
}
int create_exit_full_screen_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_exit_full_screen_response_xml(msg->priv_obj, buf, size);
}

NmpXmlMsg *parse_tv_wall_play_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, TV_WALL_PLAY_REQUEST_ID, 
			sizeof(TVWallPlayPacket), parse_tv_wall_play_request_xml);
}
NmpXmlMsg *parse_tv_wall_play_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, TV_WALL_PLAY_RESULT_ID, 
			sizeof(TVWallPlayPacket), parse_tv_wall_play_response_xml);
}

int create_tv_wall_play_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_tv_wall_play_request_xml(msg->priv_obj, buf, size);
}
int create_tv_wall_play_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_tv_wall_play_response_xml(msg->priv_obj, buf, size);
}

NmpXmlMsg *parse_clear_division_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, CLEAR_DIVISION_REQUEST_ID, 
			sizeof(ClearDivisionPacket), parse_clear_division_request_xml);
}
NmpXmlMsg *parse_clear_division_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, CLEAR_DIVISION_RESULT_ID, 
			sizeof(int), parse_clear_division_response_xml);
}

int create_clear_division_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_clear_division_request_xml(msg->priv_obj, buf, size);
}
int create_clear_division_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_clear_division_response_xml(msg->priv_obj, buf, size);
}

# endif //_USE_DECODER_PROTO_

int create_get_operation_log_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_operation_log_request_xml(msg->priv_obj, buf, size);
}
int create_get_operation_log_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);

 	return merge_get_operation_log_response_xml(msg->priv_obj, buf, size);
}

NmpXmlMsg *parse_get_operation_log_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, GET_OPERATION_LOG_REQUEST_ID, 
			sizeof(OperationLogPacket), parse_get_operation_log_request_xml);
}
NmpXmlMsg *parse_get_operation_log_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);

	return create_xml_msg(buf, size, GET_OPERATION_LOG_RESPONSE_ID, 
			sizeof(OperationLogPacket), parse_get_operation_log_response_xml);
}

NmpXmlMsg *parse_set_alarm_upload_config_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_ALARM_UPLOAD_CONFIG_REQUEST_ID, 
			sizeof(JAlarmUploadCfg), parse_set_alarm_upload_config_request_xml);
}
NmpXmlMsg *parse_set_alarm_upload_config_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_ALARM_UPLOAD_CONFIG_RESULT_ID, 
			sizeof(int), parse_set_alarm_upload_config_response_xml);
}

int create_set_alarm_upload_config_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_alarm_upload_config_request_xml(msg->priv_obj, buf, size);
}
int create_set_alarm_upload_config_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_alarm_upload_config_response_xml(msg->priv_obj, buf, size);
}

NmpXmlMsg *parse_get_preset_point_set_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return parse_get_info(parse_get_preset_point_set_request_xml, 
			GET_PRESET_POINT_SET_REQUEST_ID, buf, size, err, flags);
}
NmpXmlMsg *parse_get_preset_point_set_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, GET_PRESET_POINT_SET_RESPONSE_ID, 
			sizeof(PPSetPacket), parse_get_preset_point_set_response_xml);
}
NmpXmlMsg *parse_set_preset_point_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_PRESET_POINT_REQUEST_ID, 
			sizeof(PPConfigPacket), parse_set_preset_point_request_xml);
}
NmpXmlMsg *parse_set_preset_point_result(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return parse_set_result(parse_set_preset_point_result_xml, 
			SET_PRESET_POINT_RESULT_ID, buf, size, err, flags);
}

NmpXmlMsg *parse_get_cruise_way_set_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return parse_get_info(parse_get_cruise_way_set_request_xml, 
			GET_CRUISE_WAY_SET_REQUEST_ID, buf, size, err, flags);
}
NmpXmlMsg *parse_get_cruise_way_set_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, GET_CRUISE_WAY_SET_RESPONSE_ID, 
			sizeof(CruiseWaySetPacket), parse_get_cruise_way_set_response_xml);
}
NmpXmlMsg *parse_set_cruise_way_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_CRUISE_WAY_REQUEST_ID, 
			sizeof(CruiseConfigPacket), parse_set_cruise_way_request_xml);
}
NmpXmlMsg *parse_set_cruise_way_result(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return parse_set_result(parse_set_cruise_way_result_xml, 
			SET_CRUISE_WAY_RESULT_ID, buf, size, err, flags);
}
NmpXmlMsg *parse_get_cruise_way_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return parse_get_info(parse_get_cruise_way_request_xml, 
			GET_CRUISE_WAY_REQUEST_ID, buf, size, err, flags);
}
NmpXmlMsg *parse_get_cruise_way_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, GET_CRUISE_WAY_RESPONSE_ID, 
			sizeof(CruiseWayPacket), parse_get_cruise_way_response_xml);
}
NmpXmlMsg *parse_add_cruise_way_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, ADD_CRUISE_WAY_REQUEST_ID, 
			sizeof(CruiseWayPacket), parse_add_cruise_way_request_xml);
}
NmpXmlMsg *parse_add_cruise_way_result(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return parse_set_result(parse_add_cruise_way_result_xml, 
			ADD_CRUISE_WAY_RESULT_ID, buf, size, err, flags);
}
NmpXmlMsg *parse_modify_cruise_way_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, MODIFY_CRUISE_WAY_REQUEST_ID, 
			sizeof(CruiseWayPacket), parse_modify_cruise_way_request_xml);
}
NmpXmlMsg *parse_modify_cruise_way_result(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return parse_set_result(parse_modify_cruise_way_result_xml, 
			MODIFY_CRUISE_WAY_RESULT_ID, buf, size, err, flags);
}

int create_get_preset_point_set_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_preset_point_set_request_xml(msg->priv_obj, buf, size);
}
int create_get_preset_point_set_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_preset_point_set_response_xml(msg->priv_obj, buf, size);
}

int create_set_preset_point_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_preset_point_request_xml(msg->priv_obj, buf, size);
}
int create_set_preset_point_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_preset_point_result_xml(msg->priv_obj, buf, size);
}

int create_get_cruise_way_set_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_cruise_way_set_request_xml(msg->priv_obj, buf, size);
}
int create_get_cruise_way_set_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_cruise_way_set_response_xml(msg->priv_obj, buf, size);
}

int create_set_cruise_way_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_cruise_way_request_xml(msg->priv_obj, buf, size);
}
int create_set_cruise_way_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_cruise_way_result_xml(msg->priv_obj, buf, size);
}

int create_get_cruise_way_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_cruise_way_request_xml(msg->priv_obj, buf, size);
}
int create_get_cruise_way_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_cruise_way_response_xml(msg->priv_obj, buf, size);
}

int create_add_cruise_way_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_add_cruise_way_request_xml(msg->priv_obj, buf, size);
}
int create_add_cruise_way_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_add_cruise_way_result_xml(msg->priv_obj, buf, size);
}

int create_modify_cruise_way_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_modify_cruise_way_request_xml(msg->priv_obj, buf, size);
}
int create_modify_cruise_way_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_modify_cruise_way_result_xml(msg->priv_obj, buf, size);
}

NmpXmlMsg *parse_3d_control_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, _3D_CONTROL_REQUEST_ID, 
			sizeof(_3DControlPacket), parse_3d_control_request_xml);
}
NmpXmlMsg *parse_3d_control_result(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return parse_set_result(parse_3d_control_result_xml, 
			_3D_CONTROL_RESULT_ID, buf, size, err, flags);
}
NmpXmlMsg *parse_3d_goback_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return parse_get_info(parse_3d_goback_request_xml, 
			_3D_GOBACK_REQUEST_ID, buf, size, err, flags);
}
NmpXmlMsg *parse_3d_goback_result(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return parse_set_result(parse_3d_goback_result_xml, 
			_3D_GOBACK_RESULT_ID, buf, size, err, flags);
}
int create_3d_control_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_3d_control_request_xml(msg->priv_obj, buf, size);
}
int create_3d_control_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_3d_control_result_xml(msg->priv_obj, buf, size);
}
int create_3d_goback_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_3d_goback_request_xml(msg->priv_obj, buf, size);
}
int create_3d_goback_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_3d_goback_result_xml(msg->priv_obj, buf, size);
}

NmpXmlMsg *parse_alarm_link_io_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, ALARM_LINK_IO_REQUEST_ID, 
			sizeof(LinkIOPacket), parse_alarm_link_io_request_xml);
}
NmpXmlMsg *parse_alarm_link_io_result(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return parse_set_result(parse_alarm_link_io_result_xml, 
			ALARM_LINK_IO_RESULT_ID, buf, size, err, flags);
}
int create_alarm_link_io_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_alarm_link_io_request_xml(msg->priv_obj, buf, size);
}
int create_alarm_link_io_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_alarm_link_io_result_xml(msg->priv_obj, buf, size);
}

NmpXmlMsg *parse_alarm_link_preset_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, ALARM_LINK_PRESET_REQUEST_ID, 
			sizeof(LinkPresetPacket), parse_alarm_link_preset_request_xml);
}
NmpXmlMsg *parse_alarm_link_preset_result(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return parse_set_result(parse_alarm_link_preset_result_xml, 
			ALARM_LINK_PRESET_RESULT_ID, buf, size, err, flags);
}
int create_alarm_link_preset_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_alarm_link_preset_request_xml(msg->priv_obj, buf, size);
}
int create_alarm_link_preset_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_alarm_link_preset_result_xml(msg->priv_obj, buf, size);
}

NmpXmlMsg *parse_get_resolution_info_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return parse_get_info(parse_get_resolution_info_request_xml, 
			GET_RESOLUTION_INFO_REQUEST_ID, buf, size, err, flags);
}
NmpXmlMsg *parse_get_resolution_info_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, GET_RESOLUTION_INFO_RESPONSE_ID, 
			sizeof(ResolutionInfoPacket), parse_get_resolution_info_response_xml);
}
NmpXmlMsg *parse_set_resolution_info_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_RESOLUTION_INFO_REQUEST_ID, 
			sizeof(ResolutionInfoPacket), parse_set_resolution_info_request_xml);
}
NmpXmlMsg *parse_set_resolution_info_result(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return parse_set_result(parse_set_resolution_info_result_xml, 
			SET_RESOLUTION_INFO_RESULT_ID, buf, size, err, flags);
}

int create_get_resolution_info_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_resolution_info_request_xml(msg->priv_obj, buf, size);
}
int create_get_resolution_info_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_resolution_info_response_xml(msg->priv_obj, buf, size);
}
int create_set_resolution_info_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_resolution_info_request_xml(msg->priv_obj, buf, size);
}
int create_set_resolution_info_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_resolution_info_result_xml(msg->priv_obj, buf, size);
}

NmpXmlMsg *parse_get_ircut_control_info_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return parse_get_info(parse_get_ircut_control_info_request_xml, 
			GET_IRCUT_CONTROL_REQUEST_ID, buf, size, err, flags);
}
NmpXmlMsg *parse_get_ircut_control_info_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, GET_IRCUT_CONTROL_RESPONSE_ID, 
			sizeof(IrcutControlPacket), parse_get_ircut_control_info_response_xml);
}
NmpXmlMsg *parse_set_ircut_control_info_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_IRCUT_CONTROL_REQUEST_ID, 
			sizeof(IrcutControlPacket), parse_set_ircut_control_info_request_xml);
}
NmpXmlMsg *parse_set_ircut_control_info_result(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return parse_set_result(parse_set_ircut_control_info_result_xml, 
			SET_IRCUT_CONTROL_RESULT_ID, buf, size, err, flags);
}

int create_get_ircut_control_info_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_ircut_control_info_request_xml(msg->priv_obj, buf, size);
}
int create_get_ircut_control_info_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_ircut_control_info_response_xml(msg->priv_obj, buf, size);
}
int create_set_ircut_control_info_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_ircut_control_info_request_xml(msg->priv_obj, buf, size);
}
int create_set_ircut_control_info_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_ircut_control_info_result_xml(msg->priv_obj, buf, size);
}


NmpXmlMsg *parse_get_extranet_port_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);

	return parse_get_info(parse_get_extranet_port_request_xml, 
			GET_EXTRANET_PORT_REQUEST_ID, buf, size, err, flags);
}
NmpXmlMsg *parse_get_extranet_port_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);

	return create_xml_msg(buf, size, GET_EXTRANET_PORT_RESPONSE_ID, 
			sizeof(ExtranetPortPacket), parse_get_extranet_port_response_xml);
}
int create_get_extranet_port_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);

	return merge_get_extranet_port_request_xml(msg->priv_obj, buf, size);
}
int create_get_extranet_port_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);

	return merge_get_extranet_port_response_xml(msg->priv_obj, buf, size);
}

NmpXmlMsg *parse_get_herd_analyse_info_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return parse_get_info(parse_get_herd_analyse_info_request_xml, 
			GET_HERD_ANALYSE_REQUEST_ID, buf, size, err, flags);
}
NmpXmlMsg *parse_get_herd_analyse_info_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, GET_HERD_ANALYSE_RESPONSE_ID, 
			sizeof(IrcutControlPacket), parse_get_herd_analyse_info_response_xml);
}
NmpXmlMsg *parse_set_herd_analyse_info_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, SET_HERD_ANALYSE_REQUEST_ID, 
			sizeof(IrcutControlPacket), parse_set_herd_analyse_info_request_xml);
}
NmpXmlMsg *parse_set_herd_analyse_info_result(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return parse_set_result(parse_set_herd_analyse_info_result_xml, 
			SET_HERD_ANALYSE_RESULT_ID, buf, size, err, flags);
}

int create_get_herd_analyse_info_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_herd_analyse_info_request_xml(msg->priv_obj, buf, size);
}
int create_get_herd_analyse_info_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_herd_analyse_info_response_xml(msg->priv_obj, buf, size);
}
int create_set_herd_analyse_info_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_herd_analyse_info_request_xml(msg->priv_obj, buf, size);
}
int create_set_herd_analyse_info_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_set_herd_analyse_info_result_xml(msg->priv_obj, buf, size);
}

NmpXmlMsg *parse_get_grass_percent_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return parse_get_info(parse_get_grass_percent_request_xml, 
			GET_GRASS_PERCENT_REQUEST_ID, buf, size, err, flags);
}
NmpXmlMsg *parse_get_grass_percent_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, GET_GRASS_PERCENT_RESPONSE_ID, 
			sizeof(GrassPercentPacket), parse_get_grass_percent_response_xml);
}
int create_get_grass_percent_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_grass_percent_request_xml(msg->priv_obj, buf, size);
}
int create_get_grass_percent_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_grass_percent_response_xml(msg->priv_obj, buf, size);
}

NmpXmlMsg *parse_get_p2p_id_request(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	return create_xml_msg(buf, size, GET_P2P_ID_REQUEST_ID, 
			sizeof(P2PIdPacket), parse_get_p2p_id_response_xml);
}
NmpXmlMsg *parse_get_p2p_id_response(char buf[], size_t size, 
			int *err, unsigned int flags)
{
	J_ASSERT(buf && size);
	
	return create_xml_msg(buf, size, GET_P2P_ID_RESPONSE_ID, 
			sizeof(P2PIdPacket), parse_get_p2p_id_response_xml);
}
int create_get_p2p_id_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_p2p_id_request_xml(msg->priv_obj, buf, size);
}
int create_get_p2p_id_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags)
{
	J_ASSERT(msg && size);
	
	return merge_get_p2p_id_response_xml(msg->priv_obj, buf, size);
}

