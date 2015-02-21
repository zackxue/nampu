
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

#include "rtsp_sdp.h"
#include "rtsp_log.h"


int my_sdp_debug = 0;

#define MAX_LINE_LEN	(1024 * 16)

#define STR_NULL(str) ((str) ? (str) : "(NULL)")


#define DEFINE_STRING_SETTER(field)		\
SDP_RESULT sdp_message_set_##field(sdp_message *msg, const char *val) {	\
	sdp_read_string(&msg->field, val);	\
	return SDP_OK;	\
}

#define DEFINE_STRING_GETTER(field)		\
const char *sdp_message_get_##field(const sdp_message *msg) {	\
	return msg->field.str;	\
}

#define DEFINE_ARRAY_LEN(field)		\
uint32 sdp_message_##field##_len(const sdp_message *msg) {	\
	return msg->field.array_len;	\
}

#define DEFINE_PTR_ARRAY_GETTER(method, field, type)		\
type sdp_message_get_##method(const sdp_message *msg, uint32 idx) {		\
	if (idx >= msg->field.array_len) {	\
		log_0("idx(%u) >= msg->%s.array_len(%u)\n", 	\
			idx, #field, msg->field.array_len);	\
		return NULL;		\
	}		\
	return msg->field.array[idx].str;		\
}

#define DEFINE_ARRAY_GETTER(method, field, type)		\
type *sdp_message_get_##method(const sdp_message *msg, uint32 idx) {		\
	if (idx >= msg->field.array_len) {	\
		log_0("idx(%u) >= msg->%s.array_len(%u)\n", 	\
			idx, #field, msg->field.array_len);	\
		return NULL;		\
	}		\
	return &msg->field.array[idx];		\
}


/*
 *	加多声明，方便查看
 */
SDP_RESULT sdp_string_array_init(sdp_string_array *sdp_array);
SDP_RESULT sdp_bandwidth_array_init(sdp_bandwidth_array *sdp_array);
SDP_RESULT sdp_time_array_init(sdp_time_array *sdp_array);
SDP_RESULT sdp_zone_array_init(sdp_zone_array *sdp_array);
SDP_RESULT sdp_attribute_array_init(sdp_attribute_array *sdp_array);
SDP_RESULT sdp_connection_array_init(sdp_connection_array *sdp_array);
SDP_RESULT sdp_media_array_init(sdp_media_array *sdp_array);

#define DEFINE_SDP_ARRAY_INIT(method)		\
SDP_RESULT sdp_##method##_array_init(sdp_##method##_array *sdp_array) {	\
	uint32 i;										\
	if (sdp_array->array != NULL) {					\
		for (i = 0; i < sdp_array->array_len; i++)		\
			sdp_##method##_init(&sdp_array->array[i]);		\
		if (sdp_array->array != sdp_array->init)		\
			my_free(sdp_array->array, sizeof(sdp_##method) * sdp_array->size);	\
	}											\
	sdp_array->size = DEF_SDP_ARRAY_SIZE;			\
	sdp_array->array_len = 0;						\
	sdp_array->array = sdp_array->init;	\
	return SDP_OK;								\
}

/*
 *	加多声明，方便查看
 */
SDP_RESULT sdp_string_array_larger(sdp_string_array *sdp_array, 
	uint32 larger_size);
SDP_RESULT sdp_bandwidth_array_larger(sdp_bandwidth_array *sdp_array, 
	uint32 larger_size);
SDP_RESULT sdp_time_array_larger(sdp_time_array *sdp_array, 
	uint32 larger_size);
SDP_RESULT sdp_zone_array_larger(sdp_zone_array *sdp_array, 
	uint32 larger_size);
SDP_RESULT sdp_attribute_array_larger(sdp_attribute_array *sdp_array, 
	uint32 larger_size);
SDP_RESULT sdp_connection_array_larger(sdp_connection_array *sdp_array, 
	uint32 larger_size);
SDP_RESULT sdp_media_array_larger(sdp_media_array *sdp_array, 
	uint32 larger_size);

#define DEFINE_SDP_ARRAY_LARGER(method)		\
SDP_RESULT sdp_##method##_array_larger(sdp_##method##_array *sdp_array, 	\
	uint32 larger_size) {						\
	sdp_##method *new_array;				\
	uint32 struct_size = sizeof(sdp_##method);	\
	uint32 i;									\
		\
	return_val_if_fail(sdp_array != NULL, SDP_EINVAL);	\
	if (sdp_array->size == 0) {			\
		sdp_array->size = DEF_SDP_ARRAY_SIZE;			\
		sdp_array->array = sdp_array->init;				\
		return SDP_OK;				\
	}	\
	if (larger_size <= sdp_array->size) {					\
		log_0("larger_size(%u) <= sdp_array->size(%u)\n", 	\
			larger_size, sdp_array->size);				\
		return SDP_EINVAL;							\
	}	\
		\
	new_array = (sdp_##method *)my_alloc(struct_size * larger_size);	\
	memset(new_array, 0, struct_size * larger_size);					\
		\
	memcpy(new_array, sdp_array->array, struct_size * sdp_array->size);	\
	for (i = 0; i < sdp_array->array_len; i++) {				\
		sdp_##method##_reinit(&new_array[i], &sdp_array->array[i]); 	\
	}	\
		\
	if (sdp_array->array != sdp_array->init)						\
		my_free(sdp_array->array, struct_size * sdp_array->size);	\
		\
	sdp_array->array = new_array;		\
	sdp_array->size = larger_size;		\
	return SDP_OK;					\
}


#define MAX_COPY_LEN	(max_len - copy_len)
/*
 *	if_continue 与 copy_len 需定义并初始化方可使用
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


static void sdp_string_init(sdp_string *string);



char *strrchr(const char *s, int c)
{
	const char ch = c;
	const char *sc;

	for (sc = NULL; ; ++s)
	{
		if (*s == ch)
			sc = s;
		if (*s == '\0')
			return ((char *) sc);
	}

	return NULL;
}

static int hex_to_int (char c)
{
  return c >= '0' && c <= '9' ? c - '0'
      : c >= 'A' && c <= 'F' ? c - 'A' + 10
      : c >= 'a' && c <= 'f' ? c - 'a' + 10 : 0;
}

static void sdp_string_reinit(sdp_string *cur, sdp_string *old)
{
	if (old->str == old->init)
		cur->str = cur->init;
}

static void sdp_string_array_reinit(sdp_string_array *cur, sdp_string_array *old)
{
	uint32 i;

	if (old->array == old->init)
		cur->array = cur->init;

	for (i = 0; i < old->array_len; i++)
		sdp_string_reinit(&cur->array[i], &old->array[i]);
}

static void sdp_bandwidth_reinit(sdp_bandwidth *cur, sdp_bandwidth *old)
{
	sdp_string_reinit(&cur->bwtype, &old->bwtype);
}

static void sdp_bandwidth_array_reinit(sdp_bandwidth_array *cur, 
	sdp_bandwidth_array *old)
{
	uint32 i;

	if (old->array == old->init)
		cur->array = cur->init;

	for (i = 0; i < old->array_len; i++)
		sdp_bandwidth_reinit(&cur->array[i], &old->array[i]);
}

static void sdp_time_reinit(sdp_time *cur, sdp_time *old)
{
	sdp_string_reinit(&cur->start, &old->start);
	sdp_string_reinit(&cur->stop, &old->stop);
	sdp_string_array_reinit(&cur->repeat, &old->repeat);
}

static void sdp_zone_reinit(sdp_zone *cur, sdp_zone *old)
{
	sdp_string_reinit(&cur->time, &old->time);
	sdp_string_reinit(&cur->typed_time, &old->typed_time);
}

static void sdp_attribute_reinit(sdp_attribute *cur, sdp_attribute *old)
{
	sdp_string_reinit(&cur->key, &old->key);
	sdp_string_reinit(&cur->value, &old->value);
}

static void sdp_attribute_array_reinit(sdp_attribute_array *cur, 
	sdp_attribute_array *old)
{
	uint32 i;

	if (old->array == old->init)
		cur->array = cur->init;

	for (i = 0; i < old->array_len; i++)
		sdp_attribute_reinit(&cur->array[i], &old->array[i]);
}

static void sdp_connection_reinit(sdp_connection *cur, sdp_connection *old)
{
	sdp_string_reinit(&cur->nettype, &old->nettype);
	sdp_string_reinit(&cur->addrtype, &old->addrtype);
	sdp_string_reinit(&cur->address, &old->address);
}

static void sdp_connection_array_reinit(sdp_connection_array *cur, 
	sdp_connection_array *old)
{
	uint32 i;

	if (old->array == old->init)
		cur->array = cur->init;

	for (i = 0; i < old->array_len; i++)
		sdp_connection_reinit(&cur->array[i], &old->array[i]);
}

static void sdp_key_reinit(sdp_key *cur, sdp_key *old)
{
	sdp_string_reinit(&cur->type, &old->type);
	sdp_string_reinit(&cur->data, &old->data);
}

static void sdp_media_reinit(sdp_media *cur, sdp_media *old)
{
	sdp_string_reinit(&cur->media, &old->media);
	sdp_string_reinit(&cur->proto, &old->proto);
	sdp_string_array_reinit(&cur->fmts, &old->fmts);
	sdp_string_reinit(&cur->information, &old->information);
	sdp_connection_array_reinit(&cur->connections, &old->connections);
	sdp_bandwidth_array_reinit(&cur->bandwidths, &old->bandwidths);
	sdp_key_reinit(&cur->key, &old->key);
	sdp_attribute_array_reinit(&cur->attributes, &old->attributes);
}


static void sdp_read_string(sdp_string *string, const char *val)
{
	sdp_string_init(string);
	
	if (val)
	{
		string->str_len = STRLEN1(val);
		
		if (string->str_len > DEF_SDP_STRING_LEN)
		{
			string->str = my_strdup(val);
		}
		else
		{
			string->str = string->init;
			memcpy(string->str, val, string->str_len);
		}
	}
	else
	{
		log_0("val = NULL\n");
	}
}


static void sdp_string_init(sdp_string *string)
{
	if (string->str != NULL && string->str != string->init)
	{
		my_free(string->str, string->str_len);
	}
	
	memset(string, 0, sizeof(sdp_string));
}


static void sdp_origin_init(sdp_origin *origin)
{
	sdp_string_init(&origin->username);
	sdp_string_init(&origin->sess_id);
	sdp_string_init(&origin->sess_version);
	sdp_string_init(&origin->nettype);
	sdp_string_init(&origin->addrtype);
	sdp_string_init(&origin->addr);
}

DEFINE_SDP_ARRAY_INIT(string);


static void sdp_connection_init(sdp_connection *connection)
{
	sdp_string_init(&connection->nettype);
	sdp_string_init(&connection->addrtype);
	sdp_string_init(&connection->address);
	connection->ttl = 0;
	connection->addr_number = 0;
}


static void sdp_bandwidth_init(sdp_bandwidth *bandwidth)
{
	sdp_string_init(&bandwidth->bwtype);
	bandwidth->bandwidth = 0;
}

DEFINE_SDP_ARRAY_INIT(bandwidth);


static void sdp_time_init(sdp_time *time)
{
	sdp_string_init(&time->start);
	sdp_string_init(&time->stop);
	sdp_string_array_init(&time->repeat);
}

DEFINE_SDP_ARRAY_INIT(time);


static void sdp_zone_init(sdp_zone *zone)
{
	sdp_string_init(&zone->time);
	sdp_string_init(&zone->typed_time);
}

DEFINE_SDP_ARRAY_INIT(zone);


static void sdp_key_init(sdp_key *key)
{
	sdp_string_init(&key->type);
	sdp_string_init(&key->data);
}


static void sdp_attribute_init(sdp_attribute *attribute)
{
	sdp_string_init(&attribute->key);
	sdp_string_init(&attribute->value);
}

DEFINE_SDP_ARRAY_INIT(attribute);


DEFINE_SDP_ARRAY_INIT(connection);


DEFINE_SDP_ARRAY_INIT(media);



/* Session descriptions */
SDP_RESULT sdp_message_new(sdp_message **msg)
{
	sdp_message *new_msg;

	func_begin("\n");
	return_val_if_fail(msg != NULL, SDP_EINVAL);

	new_msg = (sdp_message *)my_alloc(sizeof(sdp_message));
	memset(new_msg, 0, sizeof(sdp_message));

	*msg = new_msg;

	return sdp_message_init(new_msg);
}


SDP_RESULT sdp_message_init(sdp_message *msg)
{
	return_val_if_fail(msg != NULL, SDP_EINVAL);
	
	sdp_string_init(&msg->version);
	sdp_origin_init(&msg->origin);
	sdp_string_init(&msg->session_name);
	sdp_string_init(&msg->information);
	sdp_string_init(&msg->uri);
	sdp_string_array_init(&msg->emails);
	sdp_string_array_init(&msg->phones);
	sdp_connection_init(&msg->connection);
	sdp_bandwidth_array_init(&msg->bandwidths);
	sdp_time_array_init(&msg->times);
	sdp_zone_array_init(&msg->zones);
	sdp_key_init(&msg->key);
	sdp_attribute_array_init(&msg->attributes);
	sdp_media_array_init(&msg->medias);

	return SDP_OK;
}


SDP_RESULT sdp_message_uninit(sdp_message *msg)
{
	return_val_if_fail(msg != NULL, SDP_EINVAL);

	return sdp_message_init(msg);
}


SDP_RESULT sdp_message_free(sdp_message *msg)
{
	return_val_if_fail(msg != NULL, SDP_EINVAL);

	sdp_message_uninit(msg);
	my_free(msg, sizeof(sdp_message));

	return SDP_OK;
}


mbool sdp_address_is_multicast(const char *nettype, const char *addrtype, 
	const char *addr)
{
	struct addrinfo hints;
	struct addrinfo *ai;
	struct addrinfo *res;
	mbool ret = 0;

	return_val_if_fail(addr, 0);
	/* we only support IN */
	if (nettype && strcmp (nettype, "IN") != 0)
		return 0;

	memset (&hints, 0, sizeof (hints));
	hints.ai_socktype = SOCK_DGRAM;

	/* set the address type as a hint */
	if (addrtype) {
		if (!strcmp (addrtype, "IP4"))
			hints.ai_family = AF_INET;
		else if (!strcmp (addrtype, "IP6"))
			hints.ai_family = AF_INET6;
	}

	if (getaddrinfo (addr, NULL, &hints, &res) < 0)
		return 0;

	for (ai = res; !ret && ai; ai = ai->ai_next) {
		if (ai->ai_family == AF_INET)
			ret = IN_MULTICAST (ntohl (((struct sockaddr_in *) ai->ai_addr)->
			sin_addr.s_addr)) ? 1 : 0;
		else
			ret = IN6_IS_ADDR_MULTICAST (&((struct sockaddr_in6 *) ai->
			ai_addr)->sin6_addr) ? 1 : 0;
	}
	
	freeaddrinfo (res);

	return ret;
}


int sdp_message_as_text(const sdp_message *msg, char *str, uint32 str_size)
{
	/* 定义、初始化if_continue, copy_len 和 max_len */
	char if_continue = 1;
	int copy_len = 0;
	int max_len = str_size;
	
	uint32 i;

	func_begin("\n");
	return_val_if_fail(msg != NULL, -1);
	if (str_size < MIN_AS_TEXT_LEN)
	{
		log_0("str_size(%u) < MIN_AS_TEXT_LEN(%u)\n", str_size, MIN_AS_TEXT_LEN);
		return -1;
	}

	if (msg->version.str)
		my_snprintf(str, MAX_COPY_LEN, "v=%s\r\n", msg->version.str);

	if (msg->origin.sess_id.str && msg->origin.sess_version.str && 
		msg->origin.nettype.str && msg->origin.addrtype.str && 
		msg->origin.addr.str)
	my_snprintf(str, MAX_COPY_LEN, "o=%s %s %s %s %s %s\r\n", 
		msg->origin.username.str ? msg->origin.username.str : "-", 
		msg->origin.sess_id.str, msg->origin.sess_version.str, 
		msg->origin.nettype.str, msg->origin.addrtype.str, msg->origin.addr.str);

	if (msg->session_name.str)
		my_snprintf(str, MAX_COPY_LEN, "s=%s\r\n", msg->session_name.str);

	if (msg->information.str)
		my_snprintf(str, MAX_COPY_LEN, "i=%s\r\n", msg->information.str);

	if (msg->uri.str)
		my_snprintf(str, MAX_COPY_LEN, "u=%s\r\n", msg->uri.str);

	for (i = 0; i < msg->emails.array_len; i++)
		my_snprintf(str, MAX_COPY_LEN, "e=%s\r\n", msg->emails.array[i].str);

	for (i = 0; i < msg->phones.array_len; i++)
		my_snprintf(str, MAX_COPY_LEN, "p=%s\r\n", msg->phones.array[i].str);

	if (msg->emails.array_len == 0 && msg->phones.array_len == 0)
		my_snprintf(str, MAX_COPY_LEN, "e=NONE\r\n");

	if (msg->connection.nettype.str && msg->connection.addrtype.str && 
		msg->connection.address.str)
	{
		my_snprintf(str, MAX_COPY_LEN, "c=%s %s %s", 
			msg->connection.nettype.str, msg->connection.addrtype.str, 
			msg->connection.address.str);
		if (sdp_address_is_multicast(msg->connection.nettype.str, 
			msg->connection.addrtype.str, msg->connection.address.str)) {
			/* only add ttl for IP4 */
			if (strcmp(msg->connection.addrtype.str, "IP4") == 0)
				my_snprintf(str, MAX_COPY_LEN, "/%u", msg->connection.ttl);
			if (msg->connection.addr_number > 1)
				my_snprintf(str, MAX_COPY_LEN, "/%u", 
				msg->connection.addr_number);
		}
		my_snprintf(str, MAX_COPY_LEN, "\r\n");
	}

	for (i = 0; i < msg->bandwidths.array_len; i++)
	{
		const sdp_bandwidth *bandwidth = &msg->bandwidths.array[i];
		my_snprintf(str, MAX_COPY_LEN, "b=%s:%u\r\n", 
			bandwidth->bwtype.str, bandwidth->bandwidth);
	}

	for (i = 0; i < msg->times.array_len; i++)
	{
		const sdp_time *time = &msg->times.array[i];
		my_snprintf(str, MAX_COPY_LEN, "t=%s %s\r\n", time->start.str, 
			time->stop.str);

		if (time->repeat.array != NULL) {
			uint32 j;

			my_snprintf(str, MAX_COPY_LEN, "r=%s", time->repeat.array[0].str);
			for (j = 1; j < time->repeat.array_len; j++)
				my_snprintf(str, MAX_COPY_LEN, " %s", time->repeat.array[j].str);
			my_snprintf(str, MAX_COPY_LEN, "\r\n");
		}
	}

	if (msg->zones.array_len > 0)
	{
		const sdp_zone *zone = &msg->zones.array[0];
		my_snprintf(str, MAX_COPY_LEN, "z=%s %s", zone->time.str, 
			zone->typed_time.str);
		for (i = 1; i < msg->zones.array_len; i++) {
			zone = &msg->zones.array[i];
			my_snprintf(str, MAX_COPY_LEN, " %s %s", zone->time.str, 
				zone->typed_time.str);
		}
		my_snprintf(str, MAX_COPY_LEN, "\r\n");
	}

	if (msg->key.type.str)
	{
		my_snprintf(str, MAX_COPY_LEN, "k=%s", msg->key.type.str);
		if (msg->key.data.str)
			my_snprintf(str, MAX_COPY_LEN, ":%s", msg->key.data.str);
		my_snprintf(str, MAX_COPY_LEN, "\r\n");
	}

	for (i = 0; i < msg->attributes.array_len; i++)
	{
		const sdp_attribute *attr = &msg->attributes.array[i];

		if (attr->key.str) {
			my_snprintf(str, MAX_COPY_LEN, "a=%s", attr->key.str);
			if (attr->value.str)
				my_snprintf(str, MAX_COPY_LEN, ":%s", attr->value.str);
			my_snprintf(str, MAX_COPY_LEN, "\r\n");
		}
	}

	for (i = 0; i < msg->medias.array_len; i++)
	{
		sdp_media *media = &msg->medias.array[i];
		char sdp_media_str[MAX_MEDIA_TEXT_LEN] = {0};
		
		sdp_media_as_text(media, sdp_media_str, sizeof(sdp_media_str));	//zyt边界问题待处理
		my_snprintf(str, MAX_COPY_LEN, "%s", sdp_media_str);
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



/* convert from/to uri */
/* zyt,找不到包含#的uri，暂未测试 */
SDP_RESULT sdp_message_parse_uri(const char *uri, sdp_message *msg)
{
	/* 定义、初始化if_continue, copy_len 和 max_len */
	char if_continue = 1;
	int copy_len = 0;
	char line[MAX_URI_LEN * 2];
	int max_len = MAX_URI_LEN * 2;
	
	const char *colon, *slash, *hash, *p;
	SDP_RESULT res;

	return_val_if_fail(uri != NULL, SDP_EINVAL);
	return_val_if_fail(msg != NULL, SDP_EINVAL);
	memset(line, 0, sizeof(line));

	colon = strstr (uri, "://");
	 if (!colon)
		goto no_colon;

	/* FIXME connection info goes here */

	slash = strstr (colon + 3, "/");
	if (!slash)
		goto no_slash;

	/* FIXME session name goes here */

	hash = strstr (slash + 1, "#");
	if (!hash)
		goto no_hash;

	/* unescape */
	for (p = hash + 1; *p; p++) {
		if (*p == '&') {
			my_snprintf(line, MAX_COPY_LEN, "\r\n");
		}
		else if (*p == '+') {
			my_str_append_c(line, MAX_COPY_LEN, ' ');
		}
		else if (*p == '%') {
			char a, b;

			if ((a = p[1])) {
				if ((b = p[2])) {
					my_str_append_c(line, MAX_COPY_LEN, 
						(hex_to_int (a) << 4) | hex_to_int (b));
					p += 2;
				}
			} else {
			p++;
			}
		} else
		my_str_append_c(line, MAX_COPY_LEN, *p);
	}

	if (copy_len >= sizeof(line))
		line[sizeof(line) - 1] = '\0';

	res = sdp_message_parse_buffer((const uint8 *)line, strlen(line), msg);

	
	/* ERRORS */
no_colon:
	{
		return SDP_EINVAL;
	}
no_slash:
	{
		return SDP_EINVAL;
	}
no_hash:
	{
		return SDP_EINVAL;
	}
}



static const uint8 acceptable[96] = {
  /* X0   X1    X2    X3    X4    X5    X6    X7    X8    X9    XA    XB    XC    XD    XE    XF */
  0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x00,       /* 2X  !"#$%&'()*+,-./   */
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,       /* 3X 0123456789:;<=>?   */
  0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,       /* 4X @ABCDEFGHIJKLMNO   */
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01,       /* 5X PQRSTUVWXYZ[\]^_   */
  0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,       /* 6X `abcdefghijklmno   */
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00        /* 7X pqrstuvwxyz{|}~DEL */
};

static const char hex[16] = "0123456789ABCDEF";

#define ACCEPTABLE_CHAR(a) (((uint8)(a))>=32 && ((uint8)(a))<128 && acceptable[(((uint8)a))-32])


int sdp_message_as_uri(const char *scheme, const sdp_message *msg, 
	char *str, uint32 str_size)
{
	/* 定义、初始化if_continue, copy_len 和 max_len */
	char if_continue = 1;
	int copy_len = 0;
	int max_len = str_size;

	char *serialized, *p;
	mbool first;
	char msg_to_text[MAX_MESSAGE_TEXT_LEN];

	return_val_if_fail(scheme != NULL, -1);
	return_val_if_fail(msg != NULL, -1);

	sdp_message_as_text(msg, msg_to_text, sizeof(msg_to_text));
	p = serialized = msg_to_text;

	my_snprintf(str, MAX_COPY_LEN, "%s:///#", scheme);

	first = 1;
	for (p = serialized; *p; p++)
	{
		if (first) {
			my_snprintf(str, MAX_COPY_LEN, "%c=", *p);
			if (*(p + 1))
				p++;
			first = 0;
			continue;
		}
		if (*p == '\r')
			continue;
		else if (*p == '\n') {
			if (*(p + 1))
				my_str_append_c(str, MAX_COPY_LEN, '&');
			first = 1;
		} else if (*p == ' ') {
			my_str_append_c(str, MAX_COPY_LEN, '+');
		}
		else if (ACCEPTABLE_CHAR (*p)) {
			my_str_append_c(str, MAX_COPY_LEN, *p);
		}
		else {
			/* escape */
			my_snprintf(str, MAX_COPY_LEN, "%%%c%c", hex[*p >> 4], hex[*p & 0xf]);
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


static void
read_string (char * dest, uint32 size, char ** src)
{
  uint32 idx;

  idx = 0;
  /* skip spaces */
  while (isspace (**src))
    (*src)++;

  while (!isspace (**src) && **src != '\0') {
    if (idx < size - 1)
      dest[idx++] = **src;
    (*src)++;
  }
  if (size > 0)
    dest[idx] = '\0';
}

static void
read_string_del (char * dest, uint32 size, char del, char ** src)
{
  uint32 idx;

  idx = 0;
  /* skip spaces */
  while (isspace (**src))
    (*src)++;

  while (**src != del && **src != '\0') {
    if (idx < size - 1)
      dest[idx++] = **src;
    (*src)++;
  }
  if (size > 0)
    dest[idx] = '\0';
}


DEFINE_SDP_ARRAY_LARGER(string);

DEFINE_SDP_ARRAY_LARGER(bandwidth);

DEFINE_SDP_ARRAY_LARGER(time);

DEFINE_SDP_ARRAY_LARGER(zone);

DEFINE_SDP_ARRAY_LARGER(attribute);

DEFINE_SDP_ARRAY_LARGER(connection);

DEFINE_SDP_ARRAY_LARGER(media);


SDP_RESULT sdp_add_string(sdp_string_array *string_array, 
	const char *str)
{
	if (string_array->array_len >= TOP_SDP_ARRAY_SIZE)
	{
		log_0("string_array->array_len(%u) >= TOP_SDP_ARRAY_SIZE(%u)\n", 
			string_array->array_len, TOP_SDP_ARRAY_SIZE);
		return SDP_EINVAL;
	}

	if (string_array->array_len == string_array->size)
	{
		if (sdp_string_array_larger(string_array, string_array->size << 1) != 
			SDP_OK)
			return SDP_EINVAL;
	}

	sdp_string *add_string = &string_array->array[string_array->array_len];
	
	sdp_read_string(add_string, str);
	string_array->array_len++;
	return SDP_OK;
}


/* v=.. */
//const char * sdp_message_get_version(const sdp_message *msg);
DEFINE_STRING_GETTER(version);

//SDP_RESULT sdp_message_set_version(sdp_message *msg, const char *version);
DEFINE_STRING_SETTER(version);


/* o=<username> <sess-id> <sess-version> <nettype> <addrtype> <unicast-address> */
const sdp_origin *sdp_message_get_origin(const sdp_message *msg)
{
	return &msg->origin;
}

SDP_RESULT sdp_message_set_origin(sdp_message *msg, const char *username, 
	const char *sess_id, const char *sess_version, const char *nettype, 
	const char *addrtype, const char *addr)
{
	sdp_read_string(&msg->origin.username, username);
	sdp_read_string(&msg->origin.sess_id, sess_id);
	sdp_read_string(&msg->origin.sess_version, sess_version);
	sdp_read_string(&msg->origin.nettype, nettype);
	sdp_read_string(&msg->origin.addrtype, addrtype);
	sdp_read_string(&msg->origin.addr, addr);
	return SDP_OK;
}


/* s=<session name> */
//const char *sdp_message_get_session_name(const sdp_message *msg);
DEFINE_STRING_GETTER(session_name);

//SDP_RESULT sdp_message_set_session_name(sdp_message *msg, const char *session_name);
DEFINE_STRING_SETTER(session_name);


/* i=<session description> */
//const char *sdp_message_get_information(const sdp_message *msg);
DEFINE_STRING_GETTER(information);

//SDP_RESULT sdp_message_set_information(sdp_message *msg, const char *information);
DEFINE_STRING_SETTER(information);


/* u=<uri> */
//const char *sdp_message_get_uri(const sdp_message *msg);
DEFINE_STRING_GETTER(uri);

//SDP_RESULT sdp_message_set_uri(sdp_message *msg, const char *uri);
DEFINE_STRING_SETTER(uri);


/* e=<email-address> */
//uint32 sdp_message_emails_len(const sdp_message *msg);
DEFINE_ARRAY_LEN(emails);

//const char *sdp_message_get_email(const sdp_message *msg, uint32 idx);
DEFINE_PTR_ARRAY_GETTER(email, emails, const char *);

SDP_RESULT sdp_message_add_email(sdp_message *msg, const char *email)
{
	return sdp_add_string(&msg->emails, email);
}


/* p=<phone-number> */
//uint32 sdp_message_phones_len(const sdp_message *msg);
DEFINE_ARRAY_LEN(phones);

//const char *sdp_message_get_phone(const sdp_message *msg, uint32 idx);
DEFINE_PTR_ARRAY_GETTER(phone, phones, const char *);

SDP_RESULT sdp_message_add_phone(sdp_message *msg, const char *phone)
{
	return sdp_add_string(&msg->phones, phone);
}


/* c=<nettype> <addrtype> <connection-address>[/<ttl>][/<number of addresses>] */
const sdp_connection *sdp_message_get_connection(const sdp_message *msg)
{
	return &msg->connection;
}

SDP_RESULT sdp_message_set_connection(sdp_message *msg, const char *nettype, 
	const char *addrtype, const char *address, uint32 ttl, uint32 addr_number)
{
	sdp_read_string(&msg->connection.nettype, nettype);
	sdp_read_string(&msg->connection.addrtype, addrtype);
	sdp_read_string(&msg->connection.address, address);
	msg->connection.ttl = ttl;
	msg->connection.addr_number = addr_number;

	return SDP_OK;
}


/* b=<bwtype>:<bandwidth> */
//uint32 sdp_message_bandwidths_len(const sdp_message *msg);
DEFINE_ARRAY_LEN(bandwidths);

//const sdp_bandwidth *sdp_message_get_bandwidth(const sdp_message *msg, uint32 idx);
DEFINE_ARRAY_GETTER(bandwidth, bandwidths, const sdp_bandwidth);


SDP_RESULT sdp_add_bandwidth(sdp_bandwidth_array *bandwidth_array, 
	const char *bwtype, uint32 bandwidth)
{
	if (bandwidth_array->array_len >= TOP_SDP_ARRAY_SIZE)
	{
		log_0("bandwidth_array->array_len(%u) >= TOP_SDP_ARRAY_SIZE(%u)\n", 
			bandwidth_array->array_len, TOP_SDP_ARRAY_SIZE);
		return SDP_EINVAL;
	}

	if (bandwidth_array->array_len == bandwidth_array->size)
	{
		if (sdp_bandwidth_array_larger(bandwidth_array, bandwidth_array->size << 1) != 
			SDP_OK)
			return SDP_EINVAL;
	}

	sdp_bandwidth *add_bandwidth = &bandwidth_array->array[bandwidth_array->array_len];
	sdp_read_string(&add_bandwidth->bwtype, bwtype);
	add_bandwidth->bandwidth = bandwidth;

	bandwidth_array->array_len++;

	return SDP_OK;
}

SDP_RESULT sdp_message_add_bandwidth(sdp_message *msg, const char *bwtype, 
	uint32 bandwidth)
{
	sdp_bandwidth_array *bandwidth_array = &msg->bandwidths;

	return sdp_add_bandwidth(bandwidth_array, bwtype, bandwidth);
}


/* t=<start-time> <stop-time> and
 * r=<repeat interval> <active duration> <offsets from start-time> */
//uint32 sdp_message_times_len(const sdp_message *msg);
DEFINE_ARRAY_LEN(times);

//const sdp_time *sdp_message_get_time(const sdp_message *msg, uint32 idx);
DEFINE_ARRAY_GETTER(time, times, const sdp_time);

SDP_RESULT sdp_message_add_time(sdp_message *msg, const char *start, 
	const char *stop, const char **repeat)
{
	sdp_time_array *time_array = &msg->times;
	if (time_array->array_len >= TOP_SDP_ARRAY_SIZE)
	{
		log_0("time_array->array_len(%u) >= TOP_SDP_ARRAY_SIZE(%u)\n", 
			time_array->array_len, TOP_SDP_ARRAY_SIZE);
		return SDP_EINVAL;
	}

	if (time_array->array_len == time_array->size)
	{
		if (sdp_time_array_larger(time_array, time_array->size << 1) != 
			SDP_OK)
			return SDP_EINVAL;
	}

	sdp_time *add_time = &time_array->array[time_array->array_len];
	sdp_read_string(&add_time->start, start);
	sdp_read_string(&add_time->stop, stop);

	if (repeat) {
		for (; *repeat; repeat++) {
			sdp_add_string(&add_time->repeat, *repeat);
		}
	}
	else {
		/* zyt do nothing */
	}

	time_array->array_len++;
	return SDP_OK;
}


/* z=<adjustment time> <offset> <adjustment time> <offset> .... */
//uint32 sdp_message_zones_len(const sdp_message *msg);
DEFINE_ARRAY_LEN(zones);

//const sdp_zone *sdp_message_get_zone(const sdp_message *msg, uint32 idx);
DEFINE_ARRAY_GETTER(zone, zones, const sdp_zone);

SDP_RESULT sdp_message_add_zone(sdp_message *msg, const char *adj_time, 
	const char *typed_time)
{
	sdp_zone_array *zone_array = &msg->zones;
	if (zone_array->array_len >= TOP_SDP_ARRAY_SIZE)
	{
		log_0("zone_array->array_len(%u) >= TOP_SDP_ARRAY_SIZE(%u)\n", 
			zone_array->array_len, TOP_SDP_ARRAY_SIZE);
		return SDP_EINVAL;
	}

	if (zone_array->array_len == zone_array->size)
	{
		if (sdp_zone_array_larger(zone_array, zone_array->size << 1) != 
			SDP_OK)
			return SDP_EINVAL;
	}

	sdp_zone *add_zone = &zone_array->array[zone_array->array_len];
	sdp_read_string(&add_zone->time, adj_time);
	sdp_read_string(&add_zone->typed_time, typed_time);

	zone_array->array_len++;

	return SDP_OK;
}


/* k=<method>[:<encryption key>] */
const sdp_key *sdp_message_get_key(const sdp_message *msg)
{
	return &msg->key;
}

SDP_RESULT sdp_message_set_key(sdp_message *msg, const char *type, 
	const char *data)
{
	sdp_read_string(&msg->key.type, type);
	sdp_read_string(&msg->key.data, data);

	return SDP_OK;
}


/* a=... */
//uint32 sdp_message_attributes_len(const sdp_message *msg);
DEFINE_ARRAY_LEN(attributes);

//const sdp_attribute *sdp_message_get_attribute(const sdp_message *msg, uint32 idx);
DEFINE_ARRAY_GETTER(attribute, attributes, const sdp_attribute);

const char *sdp_get_attribute_val_n(const sdp_attribute_array *attribute_array, 
	const char *key, uint32 nth)
{
	uint32 i;

	for (i = 0; i < attribute_array->array_len; i++)
	{
		sdp_attribute *attr;

		attr = &attribute_array->array[i];
		if (!strcmp(attr->key.str, key)) {
			if (nth == 0)
				return attr->value.str;
			else
				nth--;
		}
	}
	return NULL;
}

const char *sdp_message_get_attribute_val_n(const sdp_message *msg, 
	const char *key, uint32 nth)
{
	const sdp_attribute_array *attribute_array = &msg->attributes;

	return sdp_get_attribute_val_n(attribute_array, key, nth);
}

const char *sdp_message_get_attribute_val(const sdp_message *msg, const char *key)
{
	return sdp_message_get_attribute_val_n(msg, key, 0);
}

SDP_RESULT sdp_add_attribute(sdp_attribute_array *attribute_array, 
	const char *key, const char *value)
{
	if (attribute_array->array_len >= TOP_SDP_ARRAY_SIZE)
	{
		log_0("attribute_array->array_len(%u) >= TOP_SDP_ARRAY_SIZE(%u)\n", 
			attribute_array->array_len, TOP_SDP_ARRAY_SIZE);
		return SDP_EINVAL;
	}

	if (attribute_array->array_len == attribute_array->size)
	{
		if (sdp_attribute_array_larger(attribute_array, attribute_array->size << 1) != 
			SDP_OK)
			return SDP_EINVAL;
	}

	sdp_attribute *add_attribute = &attribute_array->array[attribute_array->array_len];
	sdp_read_string(&add_attribute->key, key);
	sdp_read_string(&add_attribute->value, value);

	attribute_array->array_len++;
	
	return SDP_OK;
}

SDP_RESULT sdp_message_add_attribute(sdp_message *msg, const char *key, 
	const char *value)
{
	sdp_attribute_array *attribute_array = &msg->attributes;

	return sdp_add_attribute(attribute_array, key, value);
}


/* m=.. sections */
//uint32 sdp_message_medias_len(const sdp_message *msg);
DEFINE_ARRAY_LEN(medias);

//const sdp_media *sdp_message_get_media(const sdp_message *msg, uint32 idx);
DEFINE_ARRAY_GETTER(media, medias, const sdp_media);

/*
 *	暂时未使用(使用sdp_message_add_media_check实现最终目的)
 */
SDP_RESULT sdp_message_add_media(sdp_message *msg, sdp_media *media)
{
	sdp_media_array *media_array = &msg->medias;
	if (media_array->array_len > TOP_SDP_ARRAY_SIZE)
	{
		log_0("media_array->array_len(%u) > TOP_SDP_ARRAY_SIZE(%u)\n", 
			media_array->array_len, TOP_SDP_ARRAY_SIZE);
		return SDP_EINVAL;
	}

	if (media_array->array_len == media_array->size)
	{
		sdp_media_array_larger(media_array, media_array->size << 1);
	}

	sdp_media *add_media = &media_array->array[media_array->array_len];
	++media_array->array_len;
	memcpy(add_media, media, sizeof(sdp_media));
	sdp_media_reinit(add_media, media);
	/* 注意上下两函数的调用 */
	memset(media, 0, sizeof(sdp_media));
	
	return SDP_OK;
}

static void print_media(sdp_media *media)
{
	print("   media:       '%s'\n", STR_NULL(media->media.str));
	print("   port:        '%u'\n", media->port);
	print("   num_ports:   '%u'\n", media->num_ports);
	print("   proto:       '%s'\n", STR_NULL(media->proto.str));
	if (media->fmts.array_len> 0) {
		uint32 i;

		print("   formats:\n");
		for (i = 0; i < media->fmts.array_len; i++) {
			print("    format  '%s'\n", media->fmts.array[i].str);
		}
	}
	print("   information: '%s'\n", STR_NULL(media->information.str));
	if (media->connections.array_len> 0) {
		uint32 i;

		print("   connections:\n");
		for (i = 0; i < media->connections.array_len; i++) {
			sdp_connection *conn = &media->connections.array[i];
			
			print("    nettype:      '%s'\n", STR_NULL(conn->nettype.str));
			print("    addrtype:     '%s'\n", STR_NULL(conn->addrtype.str));
			print("    address:      '%s'\n", STR_NULL(conn->address.str));
			print("    ttl:          '%u'\n", conn->ttl);
			print("    addr_number:  '%u'\n", conn->addr_number);
		}
	}
	if (media->bandwidths.array_len> 0) {
		uint32 i;

		print("   bandwidths:\n");
		for (i = 0; i < media->bandwidths.array_len; i++) {
			sdp_bandwidth *bw = &media->bandwidths.array[i];

			print("    type:         '%s'\n", STR_NULL(bw->bwtype.str));
			print("    bandwidth:    '%u'\n", bw->bandwidth);
		}
	}
	print("   key:\n");
	print("    type:       '%s'\n", STR_NULL(media->key.type.str));
	print("    data:       '%s'\n", STR_NULL(media->key.data.str));
	if (media->attributes.array_len > 0) {
		uint32 i;

		print("   attributes:\n");
		for (i = 0; i < media->attributes.array_len; i++) {
			sdp_attribute *attr = &media->attributes.array[i];

			print("    attribute '%s' : '%s'\n", attr->key.str, attr->value.str);
		}
	}
}

SDP_RESULT sdp_message_dump(const sdp_message *msg)
{
	func_begin("\n");
	return_val_if_fail(msg != NULL, SDP_EINVAL);

	print("sdp packet %p:\n", msg);
	print(" version:       '%s'\n", STR_NULL(msg->version.str));
	print(" origin:\n");
	print("  username:     '%s'\n", STR_NULL(msg->origin.username.str));
	print("  sess_id:      '%s'\n", STR_NULL(msg->origin.sess_id.str));
	print("  sess_version: '%s'\n", STR_NULL(msg->origin.sess_version.str));
	print("  nettype:      '%s'\n", STR_NULL(msg->origin.nettype.str));
	print("  addrtype:     '%s'\n", STR_NULL(msg->origin.addrtype.str));
	print("  addr:         '%s'\n", STR_NULL(msg->origin.addr.str));
	print(" session_name:  '%s'\n", STR_NULL(msg->session_name.str));
	print(" information:   '%s'\n", STR_NULL(msg->information.str));
	print(" uri:           '%s'\n", STR_NULL(msg->uri.str));
	if (msg->emails.array_len > 0) {
		uint32 i;
		
		print(" emails:\n");
		for (i = 0; i < msg->emails.array_len; i++) {
			print("  email '%s'\n", msg->emails.array[i].str);
		}
	}
	if (msg->phones.array_len > 0) {
		uint32 i;

		print(" phones:\n");
		for (i = 0; i < msg->phones.array_len; i++) {
			print("  phone '%s'\n", msg->phones.array[i].str);
		}
	}
	print(" connection:\n");
	print("  nettype:      '%s'\n", STR_NULL(msg->connection.nettype.str));
	print("  addrtype:     '%s'\n", STR_NULL(msg->connection.addrtype.str));
	print("  address:      '%s'\n", STR_NULL(msg->connection.address.str));
	print("  ttl:          '%u'\n", msg->connection.ttl);
	print("  addr_number:  '%u'\n", msg->connection.addr_number);
	if (msg->bandwidths.array_len > 0) {
		uint32 i;

		print(" bandwidths:\n");
		for (i = 0; i < msg->bandwidths.array_len; i++) {
			sdp_bandwidth *bw = &msg->bandwidths.array[i];

			print("  type:         '%s'\n", STR_NULL(bw->bwtype.str));
			print("  bandwidth:    '%u'\n", bw->bandwidth);
		}
	}
	print(" key:\n");
	print("  type:         '%s'\n", STR_NULL(msg->key.type.str));
	print("  data:         '%s'\n", STR_NULL(msg->key.data.str));
	if (msg->attributes.array_len > 0) {
		uint32 i;

		print(" attributes:\n");
		for (i = 0; i < msg->attributes.array_len; i++) {
			sdp_attribute *attr = &msg->attributes.array[i];

			print("  attribute '%s' : '%s'\n", attr->key.str, attr->value.str);
		}
	}
	if (msg->medias.array_len > 0) {
		uint32 i;
		
		print(" medias:\n");
		for (i = 0; i < msg->medias.array_len; i++) {
			print("  media %u:\n", i);
			print_media(&msg->medias.array[i]);
		}
	}
	return SDP_OK;
}


/* Media descriptions */
SDP_RESULT sdp_media_new(sdp_media **media)
{
	sdp_media *new_media;

	return_val_if_fail(media, SDP_EINVAL);

	new_media = (sdp_media *)my_alloc(sizeof(sdp_media));
	memset(new_media, 0, sizeof(sdp_media));

	*media = new_media;
	
	return sdp_media_init(new_media);
}

SDP_RESULT sdp_media_init(sdp_media *media)
{
	return_val_if_fail(media != NULL, SDP_EINVAL);

	sdp_string_init(&media->media);
	media->port = 0;
	media->num_ports = 0;
	sdp_string_init(&media->proto);
	sdp_string_array_init(&media->fmts);
	sdp_string_init(&media->information);
	sdp_connection_array_init(&media->connections);
	sdp_bandwidth_array_init(&media->bandwidths);
	sdp_key_init(&media->key);
	sdp_attribute_array_init(&media->attributes);

	return SDP_OK;
}

SDP_RESULT sdp_media_uninit(sdp_media *media)
{
	return_val_if_fail(media != NULL, SDP_EINVAL);

	return sdp_media_init(media);
}

/*
 *	zyt 调用时注意
 */
SDP_RESULT sdp_media_free(sdp_media *media)
{
	return_val_if_fail(media != NULL, SDP_EINVAL);

	sdp_media_uninit(media);
	my_free(media, sizeof(sdp_media));

	return SDP_OK;
}

int sdp_media_as_text(const sdp_media *media, char *str, uint32 str_size)
{
	/* 定义、初始化if_continue, copy_len 和 max_len */
	char if_continue = 1;
	int copy_len = 0;
	int max_len = str_size;

	uint32 i;

	return_val_if_fail(media != NULL, -1);
	if (str_size < MIN_AS_TEXT_LEN)
	{
		log_0("str_size(%u) < MIN_AS_TEXT_LEN(%u)\n", str_size, MIN_AS_TEXT_LEN);
		return -1;
	}

	if (media->media.str)
		my_snprintf(str, MAX_COPY_LEN, "m=%s", media->media.str);

	my_snprintf(str, MAX_COPY_LEN, " %u", media->port);

	if ((int)(media->num_ports) > 1)	//zyt,if (media->num_ports > 1)作更改
		my_snprintf(str, MAX_COPY_LEN, "/%u", media->num_ports);

	my_snprintf(str, MAX_COPY_LEN, " %s", media->proto.str);

	for (i = 0; i < media->fmts.array_len; i++)
		my_snprintf(str, MAX_COPY_LEN, " %s", media->fmts.array[i].str);
	my_snprintf(str, MAX_COPY_LEN, "\r\n");

	if (media->information.str)
		my_snprintf(str, MAX_COPY_LEN, "i=%s", media->information.str);

	for (i = 0; i < media->connections.array_len; i++)
	{
		sdp_connection *conn = &media->connections.array[i];

		if (conn->nettype.str && conn->addrtype.str && conn->address.str) {
			my_snprintf(str, MAX_COPY_LEN, "c=%s %s %s", conn->nettype.str, 
				conn->addrtype.str, conn->address.str);
			if (sdp_address_is_multicast(conn->nettype.str, conn->addrtype.str, 
				conn->address.str)) {
				/* only add TTL for IP4 multicast */
				if (strcmp (conn->addrtype.str, "IP4") == 0)
					my_snprintf(str, MAX_COPY_LEN, "/%u", conn->ttl);
				if (conn->addr_number > 1)
					my_snprintf(str, MAX_COPY_LEN, "/%u", conn->addr_number);
			}
			my_snprintf(str, MAX_COPY_LEN, "\r\n");
		}
	}

	for (i = 0; i < media->bandwidths.array_len; i++)
	{
		const sdp_bandwidth *bandwidth = &media->bandwidths.array[i];
		my_snprintf(str, MAX_COPY_LEN, "b=%s:%u\r\n", 
			bandwidth->bwtype.str, bandwidth->bandwidth);
	}

	if (media->key.type.str)
	{
		my_snprintf(str, MAX_COPY_LEN, "k=%s", media->key.type.str);
		if (media->key.data.str)
			my_snprintf(str, MAX_COPY_LEN, ":%s", media->key.data.str);
		my_snprintf(str, MAX_COPY_LEN, "\r\n");
	}

	for (i = 0; i < media->attributes.array_len; i++)
	{
		const sdp_attribute *attr = &media->attributes.array[i];

		if (attr->key.str) {
			my_snprintf(str, MAX_COPY_LEN, "a=%s", attr->key.str);
			if (attr->value.str)
				my_snprintf(str, MAX_COPY_LEN, ":%s", attr->value.str);
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


/* m=<media> <port>/<number of ports> <proto> <fmt> ... */
const char *sdp_media_get_media(const sdp_media *media)
{
	return media->media.str;
}

SDP_RESULT sdp_media_set_media(sdp_media *media, const char *med)
{
	sdp_read_string(&media->media, med);

	return SDP_OK;
}

uint32 sdp_media_get_port(const sdp_media *media)
{
	return media->port;
}

uint32 sdp_media_get_num_ports(const sdp_media *media)
{
	return media->num_ports;
}

SDP_RESULT sdp_media_set_port_info(sdp_media *media, uint32 port, 
	uint32 num_ports)
{
	media->port = port;
	media->num_ports = num_ports;

	return SDP_OK;
}

const char *sdp_media_get_proto(const sdp_media *media)
{
	return media->proto.str;
}

SDP_RESULT sdp_media_set_proto(sdp_media *media, const char *proto)
{
	sdp_read_string(&media->proto, proto);

	return SDP_OK;
}

uint32 sdp_media_formats_len(const sdp_media *media)
{
	return media->fmts.array_len;
}

const char *sdp_media_get_format(const sdp_media *media, uint32 idx)
{
	if (idx >= media->fmts.array_len)
	{
		log_0("idx(%u) >= media->fmts.array_len(%u)\n", 
			idx, media->fmts.array_len);
		return NULL;
	}
	return media->fmts.array[idx].str;
}

SDP_RESULT sdp_media_add_format(sdp_media *media, const char *format)
{
	return sdp_add_string(&media->fmts, format);
}


/* i=<session description> */
const char *sdp_media_get_information(const sdp_media *media)
{
	return media->information.str;
}

SDP_RESULT sdp_media_set_information(sdp_media *media, const char *information)
{
	sdp_read_string(&media->information, information);

	return SDP_OK;
}


/* c=<nettype> <addrtype> <connection-address>[/<ttl>][/<number of addresses>] */
uint32 sdp_media_connections_len(const sdp_media *media)
{
	return media->connections.array_len;
}

const sdp_connection *sdp_media_get_connection(const sdp_media *media, uint32 idx)
{
	if (idx >= media->connections.array_len) {
		log_0("idx(%u) >= media->connections.array_len(%u)\n", 
			idx, media->connections.array_len);
		return NULL;
	}
	return &media->connections.array[idx];
}

SDP_RESULT sdp_media_add_connection(sdp_media *media, const char *nettype, 
	const char *addrtype, const char *address, uint32 ttl, uint32 addr_number)
{
	sdp_connection_array *connection_array = &media->connections;
	if (connection_array->array_len >= TOP_SDP_ARRAY_SIZE)
	{
		log_0("connection_array->array_len(%u) >= TOP_SDP_ARRAY_SIZE(%u)\n", 
			connection_array->array_len, TOP_SDP_ARRAY_SIZE);
		return SDP_EINVAL;
	}

	if (connection_array->array_len == connection_array->size)
	{
		if (sdp_connection_array_larger(connection_array, connection_array->size << 1) != 
			SDP_OK)
			return SDP_EINVAL;
	}

	sdp_connection *add_connection = 
		&connection_array->array[connection_array->array_len];
	sdp_read_string(&add_connection->nettype, nettype);
	sdp_read_string(&add_connection->addrtype, addrtype);
	sdp_read_string(&add_connection->address, address);
	add_connection->ttl = ttl;
	add_connection->addr_number = addr_number;

	connection_array->array_len++;
	return SDP_OK;
}


/* b=<bwtype>:<bandwidth> */
uint32 sdp_media_bandwidths_len(const sdp_media *media)
{
	return media->bandwidths.array_len;
}

const sdp_bandwidth *sdp_media_get_bandwidth(const sdp_media *media, uint32 idx)
{
	if (idx >= media->bandwidths.array_len)
	{
		log_0("idx(%u) >= media->bandwidths.array_len(%u)\n", 
			idx, media->bandwidths.array_len);
		return NULL;
	}
	return &media->bandwidths.array[idx];
}

SDP_RESULT sdp_media_add_bandwidth(sdp_media *media, const char *bwtype, 
	uint32 bandwidth)
{
	sdp_bandwidth_array *bandwidth_array = &media->bandwidths;

	return sdp_add_bandwidth(bandwidth_array, bwtype, bandwidth);
}


/* k=<method>:<encryption key> */
const sdp_key *sdp_media_get_key(const sdp_media *media)
{
	return &media->key;
}

SDP_RESULT sdp_media_set_key(sdp_media *media, const char *type, const char *data)
{
	sdp_read_string(&media->key.type, type);
	sdp_read_string(&media->key.data, data);

	return SDP_OK;
}


/* a=... */
uint32 sdp_media_attributes_len(const sdp_media *media)
{
	return media->attributes.array_len;
}

const sdp_attribute *sdp_media_get_attribute(const sdp_media *media, uint32 idx)
{
	if (idx >= media->attributes.array_len) {
		log_0("idx(%u) >= media->attributes.array_len(%u)\n", 
			idx, media->attributes.array_len);
		return NULL;
	}
	return &media->attributes.array[idx];
}

const char *sdp_media_get_attribute_val_n(const sdp_media *media, const char *key, 
	uint32 nth)
{
	const sdp_attribute_array *attribute_array = &media->attributes;

	return sdp_get_attribute_val_n(attribute_array, key, nth);
}

const char *sdp_media_get_attribute_val(const sdp_media *media, const char *key)
{
	return sdp_media_get_attribute_val_n(media, key, 0);
}

SDP_RESULT sdp_media_add_attribute(sdp_media *media, const char *key, 
	const char *value)
{
	sdp_attribute_array *attribute_array = &media->attributes;

	return sdp_add_attribute(attribute_array, key, value);
}


/*
 *	检查是否有足够的空间->扩展
 */
sdp_media *sdp_message_add_media_check(sdp_message *msg)
{
	sdp_media_array *media_array = &msg->medias;

	if (media_array->array_len >= TOP_SDP_ARRAY_SIZE)
	{
		log_0("media_array->array_len(%u) >= TOP_SDP_ARRAY_SIZE(%u)\n", 
			media_array->array_len, TOP_SDP_ARRAY_SIZE);
		return NULL;
	}

	if (media_array->array_len == media_array->size)
	{
		if (sdp_media_array_larger(media_array, media_array->size << 1) != 
			SDP_OK)
			return NULL;
	}
	
	return (&media_array->array[media_array->array_len]);
}


enum
{
	SDP_SESSION,
	SDP_MEDIA,
};

typedef struct
{
	uint32 state;
	sdp_message *msg;
	sdp_media *media;
}sdp_context;

static mbool sdp_parse_line(sdp_context *c, char type, char *buffer)
{
	char str[8192];
	char *p = buffer;

	#define READ_STRING(field) read_string(str, sizeof(str), &p); \
		sdp_read_string(field, str)
	#define READ_UINT(field) read_string(str, sizeof (str), &p); \
		field = strtoul(str, NULL, 10)

	func_begin(" type = %c\n", type);
	switch (type)
	{
	case 'v':
		if (buffer[0] != '0')
			log_0("wrong SDP version\n");
		sdp_message_set_version(c->msg, buffer);
		break;
	case 'o':
		READ_STRING(&c->msg->origin.username);
		READ_STRING(&c->msg->origin.sess_id);
		READ_STRING(&c->msg->origin.sess_version);
		READ_STRING(&c->msg->origin.nettype);
		READ_STRING(&c->msg->origin.addrtype);
		READ_STRING(&c->msg->origin.addr);
		break;
	case 's':
		sdp_read_string(&c->msg->session_name, buffer);
		break;
	case 'i':
		if (c->state == SDP_SESSION) {
			sdp_read_string(&c->msg->information, buffer);
		} else {
			sdp_read_string(&c->media->information, buffer);
		}
		break;
	case 'u':
		sdp_read_string(&c->msg->uri, buffer);
		break;
	case 'e':
		sdp_message_add_email(c->msg, buffer);
		break;
	case 'p':
		sdp_message_add_phone(c->msg, buffer);
		break;
	case 'c':
		{
			char *str2;
			sdp_connection conn;

			memset(&conn, 0, sizeof(conn));
			str2 = p;
			while ((str2 = strchr (str2, '/')))
				*str2++ = ' ';
			READ_STRING(&conn.nettype);
			READ_STRING(&conn.addrtype);
			READ_STRING(&conn.address);
			/* only read TTL for IP4 */
			if (strcmp(conn.addrtype.str, "IP4") == 0)
				READ_UINT(conn.ttl);
			READ_UINT(conn.addr_number);

			if (c->state == SDP_SESSION) {
				sdp_message_set_connection(c->msg, conn.nettype.str, 
				conn.addrtype.str, conn.address.str, conn.ttl, conn.addr_number);
			} else {
				sdp_media_add_connection(c->media, conn.nettype.str, 
					conn.addrtype.str, conn.address.str, conn.ttl, conn.addr_number);
			}
			sdp_connection_init(&conn);
			break;
		}
	case 'b':
		{
			char str2[MAX_LINE_LEN];
			
			read_string_del(str, sizeof(str), ':', &p);
			if (*p != '\0')
				p++;
			read_string(str2, sizeof(str2), &p);

			if (c->state == SDP_SESSION)
				sdp_message_add_bandwidth(c->msg, str, atoi(str2));
			else
				sdp_media_add_bandwidth(c->media, str, atoi(str2));
			break;
		}
	case 't':
		break;
	case 'k':
		break;
	case 'a':
		{
			read_string_del(str, sizeof(str), ':', &p);
			if (*p != '\0')
				p++;

			if (c->state == SDP_SESSION)
				sdp_message_add_attribute(c->msg, str, p);
			else
				sdp_media_add_attribute(c->media, str, p);
			break;
		}
	case 'm':
		{
			char *slash;
			sdp_media *nmedia = sdp_message_add_media_check(c->msg);
			if (nmedia == NULL)
			{
				log_0("nmedia == NULL\n");
				return 0;
			}
			c->msg->medias.array_len++;
			c->state = SDP_MEDIA;
			c->media = nmedia;

			/* m=<media> <port>/<number of ports> <proto> <fmt> ... */
			READ_STRING(&nmedia->media);
			read_string(str, sizeof(str), &p);
			slash = (char *)strrchr(str, '/');
			if (slash) {
				*slash = '\0';
				nmedia->port = atoi (str);
				nmedia->num_ports = atoi (slash + 1);
			} else {
				nmedia->port = atoi (str);
				nmedia->num_ports = -1;
			}
			
			READ_STRING(&nmedia->proto);
			do {
				read_string(str, sizeof(str), &p);
				sdp_media_add_format(nmedia, str);
			} while(*p != '\0');
			
			break;
		}
	default:
		log_0("type = %c\n", type);
		break;
	}
	return 1;
}


SDP_RESULT sdp_message_parse_buffer(const uint8 *data, uint32 size, 
	sdp_message *msg)
{
	char *p;
	sdp_context c;
	char type;
	char buffer[MAX_LINE_LEN];
	uint32 idx = 0;

	func_begin("\n");
	return_val_if_fail(msg != NULL, SDP_EINVAL);
	return_val_if_fail(data != NULL, SDP_EINVAL);
	return_val_if_fail(size != 0, SDP_EINVAL);

	c.state = SDP_SESSION;
	c.msg = msg;
	c.media = NULL;

	p = (char *)data;
	while (1)
	{
		while (isspace(*p))
			p++;

		type = *p++;
		if (type == '\0')
			break;

		if (*p != '=')
			goto line_done;
		p++;

		idx = 0;
		while (*p != '\n' && *p != '\r' && *p != '\0')
		{
			if (idx < sizeof (buffer) - 1)
				buffer[idx++] = *p;
			p++;
		}
		buffer[idx] = '\0';
		sdp_parse_line(&c, type, buffer);

	line_done:
		while (*p != '\n' && *p != '\0')
			p++;
		if (*p == '\n')
			p++;
	}

	return SDP_OK;
}

