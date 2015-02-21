
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdio.h>

#include "rtsp_msg.h"
#include "rtsp_log.h"


#define MAX_COPY_LEN	(max_len - copy_len)
/*
 *	if_continue 与 copy_len 需定义并初始化方可使用，需用do {} while(0)形式
 */
#define my_snprintf(str, size, args...) do {		\
	if (if_continue) {	\
		uint32 want_copy_len = snprintf(&str[copy_len], size, ##args);	\
		if (want_copy_len >= size) {	\
			copy_len += size;	\
			if_continue = 0;	\
			log_0("memory is not enough\n");	\
		} else	\
			copy_len += want_copy_len;	\
	}	\
} while(0)

#define my_str_append_c(str, size, ch) do {	\
	if (if_continue) {	\
		if (size >= 1) {	\
			str[copy_len] = ch;	\
			copy_len += 1;	\
		} else {	\
			if_continue = 0;	\
			log_0("memory is not enough\n");	\
		}	\
	}	\
} while(0)


static void rtsp_key_values_new(rtsp_key_values **key_values)
{
	rtsp_key_values *new_key_values;

	func_begin("\n");
	return_if_fail(key_values != NULL);

	new_key_values = (rtsp_key_values *)my_alloc(sizeof(rtsp_key_values));
	memset(new_key_values, 0, sizeof(rtsp_key_values));

	new_key_values->size = DEF_MAX_KEY_VALUE;
	new_key_values->key_value_sum = 0;
	new_key_values->key_value = new_key_values->init;

	*key_values = new_key_values;
}


static RTSP_RESULT take_key_values_to_larger(rtsp_key_values *key_values, 
	uint32 larger_size)
{
	rtsp_key_value *new_key_value;
	int i;

	func_begin("\n");
	return_val_if_fail(key_values != NULL, RTSP_EINVAL);
	return_val_if_fail(larger_size > key_values->size, RTSP_ERROR);

	new_key_value = (rtsp_key_value *)my_alloc(sizeof(rtsp_key_value) * larger_size);
	memset(new_key_value, 0, sizeof(rtsp_key_value) * larger_size);

	memcpy(new_key_value, key_values->key_value, sizeof(rtsp_key_value) * 
		key_values->size);
	for (i = 0; i < key_values->key_value_sum; i++)
	{
		/* 注意 */
		if (key_values->key_value[i].value == key_values->key_value[i].init)
			new_key_value[i].value = new_key_value[i].init;
	}
	
	if (key_values->key_value != key_values->init)
		my_free(key_values->key_value, sizeof(rtsp_key_value) * key_values->size);

	key_values->key_value = new_key_value;
	key_values->size = larger_size;

	return RTSP_OK;
}


static void rtsp_fill_key_value(rtsp_key_value *key_value, 
	RTSP_HEADER_FIELD field, const char *value, uint32 value_len)
{
	func_begin("\n");

	if (value_len <= DEF_MAX_VALUE_LEN)
	{
		key_value->value = key_value->init;
	}
	else
	{
		key_value->value = (char *)my_alloc(value_len);
		memset(key_value->value, 0, value_len);
	}

	key_value->field = field;
	key_value->value_len = value_len;
	memcpy(key_value->value, value, value_len);
}


static void rtsp_add_key_value_end(rtsp_key_values *key_values, 
	RTSP_HEADER_FIELD field, const char *value, uint32 value_len)
{
	func_begin("\n");

	rtsp_key_value *cur_key_value = 
		&key_values->key_value[key_values->key_value_sum];

	rtsp_fill_key_value(cur_key_value, field, value, value_len);

	key_values->key_value_sum++;
}


static void rtsp_key_value_unset(rtsp_key_value *key_value)
{
	func_begin("\n");

	if (key_value == NULL)
	{
		log_2("key_value == NULL\n");
		return ;
	}
	
	if (key_value->value != key_value->init)
	{
		my_free(key_value->value, key_value->value_len);
	}
	memset(key_value, 0, sizeof(rtsp_key_value));
}


static void key_value_for_each(rtsp_key_values *key_values)
{
	int i;

	//func_begin("\n");
	if (key_values == NULL)
	{
		//log_2("key_values == NULL\n");
		return ;
	}

	for (i = 0; i < key_values->key_value_sum; i++)
	{
		rtsp_key_value *cur_key_value = &key_values->key_value[i];
		print("   key: '%s', value: '%s'\n", 
			rtsp_header_as_text(cur_key_value->field), cur_key_value->value);
	}
}


static void util_dump_mem(const uint8 *mem, uint32 size)
{
	char string[50], chars[18];
	int string_len = 0;
	int i = 0, j = 0;

	func_begin("\n");
	memset(string, 0, sizeof(string));
	memset(chars, 0, sizeof(chars));

	i = j = 0;
	while (i < size)
	{
		if (isprint(mem[i]))
			chars[j] = mem[i];
		else
			chars[j] = '.';

		snprintf(&string[string_len], 4, "%02x ", mem[i]);

		i++;
		j++;
		string_len += 3;

		if (j == 16 || i == size)
		{
			print("%08x (%p): %-48.48s %-16.16s\n", i - j, mem + i - j, 
				string, chars);
			memset(string, 0, sizeof(string));
			memset(chars, 0, sizeof(chars));
			j = 0;
			string_len = 0;
		}
	}
}


/* memory management */
RTSP_RESULT rtsp_message_new(rtsp_message **msg)
{
	rtsp_message *new_msg;

	func_begin("\n");
	return_val_if_fail(msg != NULL, RTSP_EINVAL);

	new_msg = (rtsp_message *)my_alloc(sizeof(rtsp_message));
	memset(new_msg, 0, sizeof(rtsp_message));

	*msg = new_msg;

	return rtsp_message_init(new_msg);
}


RTSP_RESULT rtsp_message_init(rtsp_message *msg)
{
	func_begin("\n");
	return_val_if_fail(msg != NULL, RTSP_EINVAL);

	rtsp_message_unset(msg);

	msg->type = RTSP_MESSAGE_INVALID;

	return RTSP_OK;
}


RTSP_RESULT rtsp_message_unset(rtsp_message *msg)
{
	func_begin("\n");
	return_val_if_fail(msg != NULL, RTSP_EINVAL);

	switch (msg->type)
	{
	case RTSP_MESSAGE_INVALID:
		break;
		
  	case RTSP_MESSAGE_REQUEST:
		my_free(msg->type_data.request.uri, msg->type_data.request.uri_len);
		break;
		
	case RTSP_MESSAGE_RESPONSE:
		my_free(msg->type_data.response.reason, msg->type_data.response.reason_len);
		break;
		
	case RTSP_MESSAGE_DATA:
		break;
	default:
		return_val_if_reached(RTSP_EINVAL);
	}

	if (msg->hdr_fields != NULL)
	{
		int i;

		for (i = 0; i < msg->hdr_fields->key_value_sum; i++)
		{
			rtsp_key_value *key_val = &msg->hdr_fields->key_value[i];
			rtsp_key_value_unset(key_val);
		}
		if (msg->hdr_fields->key_value != msg->hdr_fields->init)
			my_free(msg->hdr_fields->key_value, sizeof(rtsp_key_value) * 
			msg->hdr_fields->size);
		my_free(msg->hdr_fields, sizeof(rtsp_key_values));
	}

	my_free(msg->body, msg->body_size);

	memset(msg, 0, sizeof(rtsp_message));

	return RTSP_OK;
}


RTSP_RESULT rtsp_message_free(rtsp_message *msg)
{
	RTSP_RESULT res;

	func_begin("\n");	
	return_val_if_fail(msg != NULL, RTSP_EINVAL);

	res = rtsp_message_unset(msg);
	if (res == RTSP_OK)
		my_free(msg, sizeof(rtsp_message));

	return res;
}


RTSP_MSG_TYPE rtsp_message_get_type(rtsp_message *msg)
{
	func_begin("\n");
	return_val_if_fail(msg != NULL, RTSP_MESSAGE_INVALID);

	return msg->type;
}


/* request */
RTSP_RESULT rtsp_message_new_request(rtsp_message **msg,
                                                     RTSP_METHOD method,
                                                     const char *uri)
{
	rtsp_message *new_msg;

	func_begin("\n");
	return_val_if_fail(msg != NULL, RTSP_EINVAL);
	return_val_if_fail(uri != NULL, RTSP_EINVAL);

	new_msg = (rtsp_message *)my_alloc(sizeof(rtsp_message));
	memset(new_msg, 0, sizeof(rtsp_message));

	*msg = new_msg;

	return rtsp_message_init_request(new_msg, method, uri);
}


RTSP_RESULT rtsp_message_init_request(rtsp_message *msg,
                                                     RTSP_METHOD method,
                                                     const char *uri)
{
	func_begin("\n");
	return_val_if_fail(msg != NULL, RTSP_EINVAL);
	return_val_if_fail(uri != NULL, RTSP_EINVAL);

	rtsp_message_unset(msg);

	msg->type = RTSP_MESSAGE_REQUEST;
	msg->type_data.request.method = method;
	msg->type_data.request.uri = my_strdup(uri);
	msg->type_data.request.uri_len = STRLEN1(uri);
	msg->type_data.request.version = RTSP_VERSION_1_0;

	return RTSP_OK;
}


RTSP_RESULT rtsp_message_parse_request(rtsp_message *msg,
                                                     RTSP_METHOD *method,
                                                     const char **uri,
                                                     uint32 *uri_len,
						               RTSP_VERSION *version)
{
	func_begin("\n");
	return_val_if_fail(msg != NULL, RTSP_EINVAL);
	return_val_if_fail(msg->type == RTSP_MESSAGE_REQUEST, RTSP_EINVAL);

	if (method)
		*method = msg->type_data.request.method;
	if (uri)
		*uri = msg->type_data.request.uri;
	if (uri_len)
		*uri_len = msg->type_data.request.uri_len;
	if (version)
		*version = msg->type_data.request.version;

	return RTSP_OK;
}


/* response */
RTSP_RESULT rtsp_message_new_response(rtsp_message **msg,
                                                     RTSP_STATUS_CODE code,
                                                     const char *reason,
                                                     const rtsp_message *request)
{
	rtsp_message *new_msg;

	func_begin("\n");
	return_val_if_fail(msg != NULL, RTSP_EINVAL);

	new_msg = (rtsp_message *)my_alloc(sizeof(rtsp_message));
	memset(new_msg, 0, sizeof(rtsp_message));

	*msg = new_msg;

	return rtsp_message_init_response(new_msg, code, reason, request);
}


RTSP_RESULT rtsp_message_init_response(rtsp_message *msg,
                                                     RTSP_STATUS_CODE code,
                                                     const char *reason,
                                                     const rtsp_message *request)
{
	func_begin("\n");
	return_val_if_fail(msg != NULL, RTSP_EINVAL);

	rtsp_message_unset(msg);

	msg->type = RTSP_MESSAGE_RESPONSE;
	msg->type_data.response.code = code;
	msg->type_data.response.version = RTSP_VERSION_1_0;
	if (reason == NULL)
		msg->type_data.response.reason = my_strdup(rtsp_status_as_text(code));
	else
		msg->type_data.response.reason = my_strdup(reason);
	msg->type_data.response.reason_len = STRLEN1(msg->type_data.response.reason);

	if (request)
	{
		char *header;
	
		/* copy CSEQ */
		if (rtsp_message_get_header(request, RTSP_HDR_CSEQ, &header, 0)
			== RTSP_OK)
		{
			rtsp_message_take_header(msg, RTSP_HDR_CSEQ, header, 
				STRLEN1(header));
		}

		/* copy session id */
		if (rtsp_message_get_header(request, RTSP_HDR_SESSION, &header, 0)
			== RTSP_OK)
		{
			char *pos;
			
			header = my_strdup(header);
			uint32 header_len = STRLEN1(header);
			
			if ((pos = strchr(header, ';')))
				*pos = '\0';
			my_strchomp(header);
			rtsp_message_take_header(msg, RTSP_HDR_SESSION, header, 
				STRLEN1(header));

			/* 注意:此处不能直接用STRLEN1 */
			my_free(header, header_len);
		}
		/* FIXME copy more headers? */
	}

	return RTSP_OK;
}


RTSP_RESULT rtsp_message_parse_response(rtsp_message *msg,
                                                     RTSP_STATUS_CODE *code,
                                                     const char **reason,
                                                     uint32 *reason_len,
                                                     RTSP_VERSION *version)
{
	func_begin("\n");
	return_val_if_fail(msg != NULL, RTSP_EINVAL);
	return_val_if_fail(msg->type == RTSP_MESSAGE_RESPONSE, RTSP_EINVAL);

	if (code)
		*code = msg->type_data.response.code;
	if (reason)
		*reason = msg->type_data.response.reason;
	if (reason_len)
		*reason_len = msg->type_data.response.reason_len;
	if (version)
		*version = msg->type_data.response.version;

	return RTSP_OK;
}


/* data */
RTSP_RESULT rtsp_message_new_data(rtsp_message **msg,
                                                     uint8 channel)
{
	rtsp_message *new_msg;

	func_begin("\n");
	return_val_if_fail(msg != NULL, RTSP_EINVAL);

	new_msg = (rtsp_message *)my_alloc(sizeof(rtsp_message));
	memset(new_msg, 0, sizeof(rtsp_message));

	*msg = new_msg;

	return rtsp_message_init_data(new_msg, channel);
}


RTSP_RESULT rtsp_message_init_data(rtsp_message *msg,
                                                     uint8 channel)
{
	func_begin("\n");
	return_val_if_fail(msg != NULL, RTSP_EINVAL);

	rtsp_message_unset(msg);

	msg->type = RTSP_MESSAGE_DATA;
	msg->type_data.data.channel = channel;

	return RTSP_OK;
}


RTSP_RESULT rtsp_message_parse_data(rtsp_message *msg,
                                                     uint8 *channel)
{
	func_begin("\n");
	return_val_if_fail(msg != NULL, RTSP_EINVAL);
	return_val_if_fail(msg->type == RTSP_MESSAGE_DATA, RTSP_EINVAL);

	if (channel)
		*channel = msg->type_data.data.channel;

	return RTSP_OK;
}


/* headers */
/* this function not used */
RTSP_RESULT rtsp_message_add_header(rtsp_message *msg,
                                                     RTSP_HEADER_FIELD field,
                                                     const char *value)
{
	func_begin("\n");
	return rtsp_message_take_header(msg, field, value, STRLEN1(value));
}


RTSP_RESULT rtsp_message_take_header(rtsp_message *msg,
                                                     RTSP_HEADER_FIELD field,
                                                     const char *value,
                                                     uint32 value_len)
{
	rtsp_key_values *key_values;

	func_begin("\n");
	return_val_if_fail(msg != NULL, RTSP_EINVAL);
	return_val_if_fail(value != NULL, RTSP_EINVAL);

	if (msg->hdr_fields == NULL)
		rtsp_key_values_new(&msg->hdr_fields);

	key_values = msg->hdr_fields;
	if (key_values->size == DEF_MAX_KEY_VALUE)
	{
		if (key_values->key_value_sum == key_values->size)
		{
			if (take_key_values_to_larger(key_values, TOP_MAX_KEY_VALUE) != 
				RTSP_OK)
				return RTSP_ERROR;
		}
		
		rtsp_add_key_value_end(key_values, field, value, value_len);
		return RTSP_OK;
	}
	else if (key_values->size == TOP_MAX_KEY_VALUE)
	{
		if (key_values->key_value_sum == key_values->size)
		{
			//return_val_if_reached(RTSP_ERROR);	/* zyt? 返回值处理 */
			log_0("key_values->key_value_sum = TOP_MAX_KEY_VALUE\n");
			return_val_if_reached(RTSP_ERROR);
		}
		
		rtsp_add_key_value_end(key_values, field, value, value_len);
		return RTSP_OK;
	}

	return_val_if_reached(RTSP_ERROR);
}


RTSP_RESULT rtsp_message_remove_header(rtsp_message *msg,
                                                     RTSP_HEADER_FIELD field,
                                                     int indx)
{
	RTSP_RESULT res = RTSP_ENOTIMPL;
	rtsp_key_values *key_values;
	int i = 0;
	int cnt = 0;

	func_begin("\n");
	return_val_if_fail(msg != NULL, RTSP_EINVAL);
	
	key_values = msg->hdr_fields;
	if (key_values == NULL)
		return RTSP_OK;

	for (i = 0; i < key_values->key_value_sum; i++)
	{
		rtsp_key_value *cur_key_value = &key_values->key_value[i];

		if (cur_key_value->field == field && (indx == -1 || cnt++ == indx))
		{
			rtsp_key_value_unset(cur_key_value);
			
			int last_num = key_values->key_value_sum - 1;
			if (i < last_num)
			{
				/* 把最后一个key_value拷贝到移除的key_value的位置 */
				rtsp_key_value *last_key_value = &key_values->key_value[last_num];
				rtsp_fill_key_value(cur_key_value, last_key_value->field, 
				last_key_value->value, last_key_value->value_len);

				/* 注意 -> 释放内存 */
				rtsp_key_value_unset(last_key_value);
			}

			key_values->key_value_sum--;
			res = RTSP_OK;
			break;
		}
	}

	return res;
}


RTSP_RESULT rtsp_message_get_header(const rtsp_message *msg,
                                                     RTSP_HEADER_FIELD field,
                                                     char **value,
                                                     int indx)
{
	int i;
	int cnt = 0;

	func_begin("\n");
	return_val_if_fail(msg != NULL, RTSP_EINVAL);

	if (msg->hdr_fields == NULL)
		return RTSP_ENOTIMPL;

	for (i = 0; i < msg->hdr_fields->key_value_sum; i++)
	{
		rtsp_key_value *key_value = &msg->hdr_fields->key_value[i];
		
		if (key_value->field == field && cnt++ == indx)
		{
			if (value)
				*value = key_value->value;
			return RTSP_OK;
		}
	}

	return RTSP_ENOTIMPL;
}


int rtsp_message_append_headers(const rtsp_message *msg,
                                                     char *str,
                                                     uint32 str_size)
{
	/* 定义、初始化if_continue, copy_len 和 max_len */
	char if_continue = 1;
	int copy_len = 0;
	int max_len = str_size;
	
	int i;

	func_begin("\n");
	return_val_if_fail(msg != NULL, RTSP_EINVAL);
	return_val_if_fail(str != NULL, RTSP_EINVAL);

	for (i = 0; i < msg->hdr_fields->key_value_sum; i++)
	{
		rtsp_key_value *key_value = &msg->hdr_fields->key_value[i];

		my_snprintf(str, MAX_COPY_LEN, "%s: %s\r\n", 
			rtsp_header_as_text(key_value->field), 
			key_value->value);
	}

	if (copy_len < str_size)
	{
		str[copy_len] = '\0';
		return copy_len;
	}
	else
	{
		str[str_size - 1] = '\0';
		log_0("memory is not enough\n");
		return str_size;
	}

	return copy_len;
}


/* handling the body */
RTSP_RESULT rtsp_message_set_body(rtsp_message *msg,
                                                     const uint8 *data,
                                                     uint32 size)
{
	func_begin("\n");
	return_val_if_fail(msg != NULL, RTSP_EINVAL);

	return rtsp_message_take_body(msg, my_memdup(data, size), size);
}


RTSP_RESULT rtsp_message_take_body(rtsp_message *msg,
                                                     uint8 *data,
                                                     uint32 size)
{
	func_begin("\n");
	return_val_if_fail(msg != NULL, RTSP_EINVAL);
	return_val_if_fail(data != NULL || size == 0, RTSP_EINVAL);

	if (msg->body)
		my_free(msg->body, msg->body_size);

	msg->body = data;
	msg->body_size = size;

	return RTSP_OK;
}


RTSP_RESULT rtsp_message_get_body(const rtsp_message *msg,
                                                     uint8 **data,
                                                     uint32 *size)
{
	func_begin("\n");
	return_val_if_fail(msg != NULL, RTSP_EINVAL);
	return_val_if_fail(data != NULL, RTSP_EINVAL);
	return_val_if_fail(size != NULL, RTSP_EINVAL);

	*data = msg->body;
	*size = msg->body_size;
	
	return RTSP_OK;
}


RTSP_RESULT rtsp_message_steal_body(rtsp_message *msg,
                                                     uint8 **data,
                                                     uint32 *size)
{
	func_begin("\n");
	return_val_if_fail(msg != NULL, RTSP_EINVAL);
	return_val_if_fail(data != NULL, RTSP_EINVAL);
	return_val_if_fail(size != NULL, RTSP_EINVAL);

	*data = msg->body;
	*size = msg->body_size;

	msg->body = NULL;
	msg->body_size = 0;

	return RTSP_OK;
}


static void gen_date_string (char *date_string, uint32 len)
{
  func_begin("\n");
  
  static const char wkdays[7][4] =
      { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
  static const char months[12][4] =
      { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct",
    "Nov", "Dec"
  };
  struct tm tm;
  time_t t;

  time(&t);
  tm = *gmtime (&t);

  snprintf(date_string, len, "%s, %02d %s %04d %02d:%02d:%02d GMT",
      wkdays[tm.tm_wday], tm.tm_mday, months[tm.tm_mon], tm.tm_year + 1900,
      tm.tm_hour, tm.tm_min, tm.tm_sec);
}


/* mem dump */
/*
 *	传入空间长度尽量过长保证拷贝完整
 *	如果返回值长度	< 0						传入参数有问题，未完成copy
 *					> 0 && < 传入长度str_size	则完成copy
 *					> 0 && = 传入长度str_size	copy内容不完整
 */
int rtsp_message_to_string(rtsp_message *msg, char *str, uint32 str_size)
{
	/* 定义、初始化if_continue, copy_len 和 max_len */
	char if_continue = 1;
	int copy_len = 0;
	int max_len = str_size;
	
	int i;

	func_begin("\n");
	return_val_if_fail(msg != NULL, RTSP_EINVAL);
	return_val_if_fail(str != NULL, RTSP_EINVAL);
	if (str_size < MIN_TO_STRING_LEN)
	{
		log_0("str_size(%u) < MIN_TO_STRING_LEN(%u)\n", 
			str_size, MIN_TO_STRING_LEN);
		return -1;
	}

	switch (msg->type)
	{
	case RTSP_MESSAGE_REQUEST:
		/* create request string */
		my_snprintf(str, MAX_COPY_LEN, "%s %s RTSP/1.0\r\n", 
			rtsp_method_as_text(msg->type_data.request.method), 
			msg->type_data.request.uri);
		break;
		
	case RTSP_MESSAGE_RESPONSE:
		/* create response string */
		my_snprintf(str, MAX_COPY_LEN, "RTSP/1.0 %d %s\r\n", 
			msg->type_data.response.code, 
			msg->type_data.response.reason);
		break;
		
	case RTSP_MESSAGE_DATA:
	{
		uint8 data_header[4];

		/* prepare data header */
		data_header[0] = '$';
		data_header[1] = msg->type_data.data.channel;
		data_header[2] = (msg->body_size >> 8) & 0xff;
		data_header[3] = msg->body_size & 0xff;

		if (MAX_COPY_LEN <= msg->body_size + 4)
		{
			log_0("memory is not enough\n");
			return -1;
		}

		/* create string with header and data */
		for (i = 0; i < 4; i++)
			my_str_append_c(str, MAX_COPY_LEN, data_header[i]);

		memcpy(str + copy_len, msg->body, msg->body_size);
		copy_len += msg->body_size;
		break;
	}
	default:
		return_val_if_reached(-1);
		break;
	}
	
	/* append headers and body */
	if (msg->type != RTSP_MESSAGE_DATA)
	{
		char date_string[128];

		gen_date_string(date_string, sizeof(date_string));

		/* add date header */
		rtsp_message_remove_header(msg, RTSP_HDR_DATE, -1);
		rtsp_message_take_header(msg, RTSP_HDR_DATE, date_string, 
			STRLEN1(date_string));

		rtsp_message_remove_header(msg, RTSP_HDR_CONTENT_LENGTH, -1);
		if (msg->body != NULL && msg->body_size > 0)
		{
			sprintf(date_string, "%d", msg->body_size);
			rtsp_message_add_header(msg, RTSP_HDR_CONTENT_LENGTH, date_string);
		}

		/* append headers */	/* 注意条件判断 */
		if (copy_len < str_size)
			copy_len += rtsp_message_append_headers(msg, &str[copy_len], 
				str_size - copy_len);

		/* append Content-Length and body if needed */
		if (msg->body != NULL && msg->body_size > 0)
		{
			/* header ends here */
			my_snprintf(str, MAX_COPY_LEN, "\r\n");
			my_snprintf(str, MAX_COPY_LEN, "%s", msg->body);
		}
		else
		{
			my_snprintf(str, MAX_COPY_LEN, "\r\n");
		}
	}
	
	if (copy_len < str_size)
	{
		str[copy_len] = '\0';
		return copy_len;
	}
	else
	{
		str[str_size - 1] = '\0';
		log_0("memory is not enough\n");
		return str_size;
	}
}


/* debug */
RTSP_RESULT rtsp_message_dump(rtsp_message *msg)
{
	uint8 *data;
	uint32 len;

	func_begin("\n");
	return_val_if_fail(msg != NULL, RTSP_EINVAL);

	switch (msg->type)
	{
	case RTSP_MESSAGE_REQUEST:
		print("RTSP request message %p\n", msg);
		print(" request line:\n");
		print("   method: '%s'\n", 
			rtsp_method_as_text(msg->type_data.request.method));
		print("   uri:    '%s'\n", msg->type_data.request.uri);
		print("   version: '%s'\n", 
			rtsp_version_as_text(msg->type_data.request.version));
		print(" headers:\n");
		key_value_for_each(msg->hdr_fields);
		print(" body:\n");
		rtsp_message_get_body(msg, &data, &len);
		util_dump_mem(data, len);
		break;

	case RTSP_MESSAGE_RESPONSE:
		print("RTSP response message %p\n", msg);
		print(" status line:\n");
		print("   code:   '%d'\n", msg->type_data.response.code);
		print("   reason: '%s'\n", msg->type_data.response.reason);
		print("   version: '%s'\n", 
			rtsp_version_as_text(msg->type_data.response.version));
		print(" headers:\n");
		key_value_for_each(msg->hdr_fields);
		rtsp_message_get_body(msg, &data, &len);
		print(" body: length %d\n", len);
		util_dump_mem(data, len);
		break;

	case RTSP_MESSAGE_DATA:
		print("RTSP data message %p\n", msg);
		print(" channel: '%d'\n", msg->type_data.data.channel);
		print(" size:    '%d'\n", msg->body_size);
		rtsp_message_get_body(msg, &data, &len);
		util_dump_mem(data, len);
		break;
		
	default:
		print("unsupported message type %d\n", msg->type);
		return RTSP_EINVAL;
	}
	
	return RTSP_OK;
}


/* message dup */
rtsp_message *rtsp_message_dup_request(rtsp_message *request)
{
	rtsp_message *msg;
	RTSP_METHOD method;
	const char *uri;
	uint32 uri_len;
	char *value;
	RTSP_VERSION version;
	RTSP_HEADER_FIELD field;
	uint8 *body_data;
	uint32 body_size;

	func_begin("\n");
	return_val_if_fail(request != NULL, NULL);
	
	if (rtsp_message_get_type(request) != RTSP_MESSAGE_REQUEST)
		return NULL;

	if (rtsp_message_parse_request(request, &method, &uri, &uri_len, &version) != 
		RTSP_OK)
		return NULL;

	if (rtsp_message_new_request(&msg, method, uri) != RTSP_OK)
		return NULL;

	for (field = RTSP_HDR_INVALID + 1; field < RTSP_HDR_LAST; field++)
	{
		if (rtsp_message_get_header(request, field, &value, 0) == RTSP_OK)
			rtsp_message_take_header(msg, field, value, STRLEN1(value));
	}

	if (rtsp_message_get_body(request, &body_data, &body_size) == RTSP_OK)
		rtsp_message_set_body(msg, body_data, body_size);

	return msg;
}


char *rtsp_mem_alloc(size_t size)
{
	return my_alloc(size);
}


void rtsp_mem_free(void *ptr, size_t size)
{
	my_free(ptr, size);
}
