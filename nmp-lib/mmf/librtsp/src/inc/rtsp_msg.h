/*
 *	author:	zyt
 *	time:	begin in 2012/7/24
 */
#ifndef __RTSP_MESSAGE_H__
#define __RTSP_MESSAGE_H__

#include "rtsp_defs.h"
#include "rtsp_mem.h"


#define DEF_MAX_KEY_VALUE	(8)
#define TOP_MAX_KEY_VALUE	(DEF_MAX_KEY_VALUE * 2)

#define DEF_MAX_VALUE_LEN	(64)

#define MIN_TO_STRING_LEN	(32)
#define MIN(x, y) ((x) < (y) ? (x) : (y))


typedef enum
{
  RTSP_MESSAGE_INVALID,
  RTSP_MESSAGE_REQUEST,
  RTSP_MESSAGE_RESPONSE,
  RTSP_MESSAGE_DATA
} RTSP_MSG_TYPE;


typedef struct
{
	RTSP_HEADER_FIELD field;
	uint32 value_len;	//长度包括结束符'\0'
	char *value;
	char init[DEF_MAX_VALUE_LEN];
} rtsp_key_value;


typedef struct
{
	uint32 size;
	uint32 key_value_sum;
	rtsp_key_value *key_value;
	rtsp_key_value init[DEF_MAX_KEY_VALUE];
} rtsp_key_values;


typedef struct
{
	RTSP_MSG_TYPE type;

	union
	{
		struct
		{
			RTSP_METHOD	method;
			char 	*uri;
			uint32	uri_len;
			RTSP_VERSION	version;
		} request;
		struct {
			RTSP_STATUS_CODE	code;
			char		*reason;
			uint32	reason_len;
			RTSP_VERSION	version;
		} response;
		struct {
			uint8	channel;
		} data;
	} type_data;

	/*< private >*/
	rtsp_key_values	*hdr_fields;

	uint8	*body;
	uint32	body_size;	//body内容与内存长度一样长,包括'\0'
} rtsp_message;


/* memory management */
RTSP_RESULT rtsp_message_new(rtsp_message **msg);
RTSP_RESULT rtsp_message_init(rtsp_message *msg);
RTSP_RESULT rtsp_message_unset(rtsp_message *msg);
RTSP_RESULT rtsp_message_free(rtsp_message *msg);

RTSP_MSG_TYPE rtsp_message_get_type(rtsp_message *msg);

/* request */
RTSP_RESULT rtsp_message_new_request(rtsp_message **msg,
                                                     RTSP_METHOD method,
                                                     const char *uri);
RTSP_RESULT rtsp_message_init_request(rtsp_message *msg,
                                                     RTSP_METHOD method,
                                                     const char *uri);
RTSP_RESULT rtsp_message_parse_request(rtsp_message *msg,
                                                     RTSP_METHOD *method,
                                                     const char **uri,
                                                     uint32 *uri_len,
						               RTSP_VERSION *version);

/* response */
RTSP_RESULT rtsp_message_new_response(rtsp_message **msg,
                                                     RTSP_STATUS_CODE code,
                                                     const char *reason,
                                                     const rtsp_message *request);
RTSP_RESULT rtsp_message_init_response(rtsp_message *msg,
                                                     RTSP_STATUS_CODE code,
                                                     const char *reason,
                                                     const rtsp_message *request);
RTSP_RESULT rtsp_message_parse_response(rtsp_message *msg,
                                                     RTSP_STATUS_CODE *code,
                                                     const char **reason,
                                                     uint32 *reason_len,
                                                     RTSP_VERSION *version);

/* data */
RTSP_RESULT rtsp_message_new_data(rtsp_message **msg,
                                                     uint8 channel);
RTSP_RESULT rtsp_message_init_data(rtsp_message *msg,
                                                     uint8 channel);
RTSP_RESULT rtsp_message_parse_data(rtsp_message *msg,
                                                     uint8 *channel);

/* headers */
RTSP_RESULT rtsp_message_add_header(rtsp_message *msg,
                                                     RTSP_HEADER_FIELD field,
                                                     const char *value);
RTSP_RESULT rtsp_message_take_header(rtsp_message *msg,
                                                     RTSP_HEADER_FIELD field,
                                                     const char *value,
                                                     uint32 value_len);
RTSP_RESULT rtsp_message_remove_header(rtsp_message *msg,
                                                     RTSP_HEADER_FIELD field,
                                                     int indx);
RTSP_RESULT rtsp_message_get_header(const rtsp_message *msg,
                                                     RTSP_HEADER_FIELD field,
                                                     char **value,
                                                     int indx);
RTSP_RESULT rtsp_message_append_headers(const rtsp_message *msg,
                                                     char *str,
                                                     uint32 str_size);

/* handling the body */
RTSP_RESULT rtsp_message_set_body(rtsp_message *msg,
                                                     const uint8 *data,
                                                     uint32 size);
RTSP_RESULT rtsp_message_take_body(rtsp_message *msg,
                                                     uint8 *data,
                                                     uint32 size);
RTSP_RESULT rtsp_message_get_body(const rtsp_message *msg,
                                                     uint8 **data,
                                                     uint32 *size);
RTSP_RESULT rtsp_message_steal_body(rtsp_message *msg,
                                                     uint8 **data,
                                                     uint32 *size);
/* mem dump */
int rtsp_message_to_string(rtsp_message *msg, char *str, uint32 str_size);

/* debug */
RTSP_RESULT rtsp_message_dump(rtsp_message *msg);

/* message dup */
rtsp_message *rtsp_message_dup_request(rtsp_message *request);


/* test */
//void rtsp_key_values_new(rtsp_key_values **key_values);

char *rtsp_mem_alloc(size_t size);
void rtsp_mem_free(void *ptr, size_t size);

#endif
 
