/*
 *	author:	zyt
 *	time:	begin in 2012/7/31
 */
#ifndef __SDP_H_20120731__
#define __SDP_H_20120731__

#include "rtsp_mem.h"


#define DEF_SDP_STRING_LEN		(64)

#define DEF_SDP_ARRAY_SIZE		(2)	//只能为2的n次方
#define TOP_SDP_ARRAY_SIZE		(64)	//只能为2的n次方

#define MIN_AS_TEXT_LEN			(32)
#define MAX_MEDIA_TEXT_LEN		(1024 * 4)	//zyt暂定
#define MAX_MESSAGE_TEXT_LEN	(1024 * 8)	//zyt暂定

#define MAX_URI_LEN				(1024)

typedef enum
{
	SDP_OK		= 0,
	SDP_EINVAL	= -1
} SDP_RESULT;


typedef struct
{
	uint32	str_len;	//长度包括结束符'\0'
	char		*str;
	char		init[DEF_SDP_STRING_LEN];
} sdp_string;

typedef struct
{
	sdp_string	username;
	sdp_string	sess_id;
	sdp_string	sess_version;
	sdp_string	nettype;
	sdp_string	addrtype;
	sdp_string	addr;
} sdp_origin;

typedef struct
{
	uint32		size;
	uint32		array_len;	//元素的个数
	sdp_string	*array;
	sdp_string	init[DEF_SDP_ARRAY_SIZE];
} sdp_string_array;

typedef struct
{
	sdp_string	nettype;
	sdp_string	addrtype;
	sdp_string	address;
	uint32		ttl;
	uint32		addr_number;
} sdp_connection;

typedef struct
{
	sdp_string	bwtype;
	uint32		bandwidth;
} sdp_bandwidth;

typedef struct
{
	uint32			size;
	uint32			array_len;
	sdp_bandwidth	*array;
	sdp_bandwidth	init[DEF_SDP_ARRAY_SIZE];
} sdp_bandwidth_array;

typedef struct
{
	sdp_string		start;
	sdp_string		stop;
	sdp_string_array	repeat;
} sdp_time;

typedef struct
{
	uint32		size;
	uint32		array_len;
	sdp_time		*array;
	sdp_time		init[DEF_SDP_ARRAY_SIZE];
} sdp_time_array;

typedef struct
{
	sdp_string	time;
	sdp_string	typed_time;
} sdp_zone;

typedef struct
{
	uint32		size;
	uint32		array_len;
	sdp_zone		*array;
	sdp_zone		init[DEF_SDP_ARRAY_SIZE];
} sdp_zone_array;

typedef struct
{
	sdp_string	type;
	sdp_string	data;
} sdp_key;

typedef struct
{
	sdp_string	key;
	sdp_string	value;
} sdp_attribute;

typedef struct
{
	uint32			size;
	uint32			array_len;
	sdp_attribute		*array;
	sdp_attribute		init[DEF_SDP_ARRAY_SIZE];
} sdp_attribute_array;

typedef struct
{
	uint32			size;
	uint32			array_len;
	sdp_connection	*array;
	sdp_connection	init[DEF_SDP_ARRAY_SIZE];
} sdp_connection_array;


typedef struct
{
	sdp_string				media;
	uint32					port;
	uint32					num_ports;
	sdp_string				proto;
	sdp_string_array			fmts;
	sdp_string				information;
	sdp_connection_array		connections;
	sdp_bandwidth_array		bandwidths;
	sdp_key					key;
	sdp_attribute_array		attributes;
} sdp_media;


typedef struct
{
	uint32		size;
	uint32		array_len;
	sdp_media	*array;
	sdp_media	init[DEF_SDP_ARRAY_SIZE];
} sdp_media_array;


typedef struct
{
	sdp_string			version;
	sdp_origin			origin;
	sdp_string			session_name;
	sdp_string			information;
	sdp_string			uri;
	sdp_string_array		emails;
	sdp_string_array		phones;
	sdp_connection		connection;
	sdp_bandwidth_array	bandwidths;
	sdp_time_array		times;
	sdp_zone_array		zones;
	sdp_key				key;
	sdp_attribute_array	attributes;
	sdp_media_array		medias;
} sdp_message;


/* Session descriptions */
SDP_RESULT sdp_message_new(sdp_message **msg);
SDP_RESULT sdp_message_init(sdp_message *msg);
SDP_RESULT sdp_message_uninit(sdp_message *msg);
SDP_RESULT sdp_message_free(sdp_message *msg);

SDP_RESULT sdp_message_parse_buffer(const uint8 *data, uint32 size, 
	sdp_message *msg);
int sdp_message_as_text(const sdp_message *msg, char *str, uint32 str_size);

/* convert from/to uri */
SDP_RESULT sdp_message_parse_uri(const char *uri, sdp_message *msg);
int sdp_message_as_uri(const char *scheme, const sdp_message *msg, 
	char *str, uint32 str_size);

/* utils */
mbool sdp_address_is_multicast(const char *nettype, const char *addrtype, 
	const char *addr);

/* v=.. */
const char * sdp_message_get_version(const sdp_message *msg);
SDP_RESULT sdp_message_set_version(sdp_message *msg, const char *version);

/* o=<username> <sess-id> <sess-version> <nettype> <addrtype> <unicast-address> */
const sdp_origin *sdp_message_get_origin(const sdp_message *msg);
SDP_RESULT sdp_message_set_origin(sdp_message *msg, const char *username, 
	const char *sess_id, const char *sess_version, const char *nettype, 
	const char *addrtype, const char *addr);

/* s=<session name> */
const char *sdp_message_get_session_name(const sdp_message *msg);
SDP_RESULT sdp_message_set_session_name(sdp_message *msg, const char *session_name);

/* i=<session description> */
const char *sdp_message_get_information(const sdp_message *msg);
SDP_RESULT sdp_message_set_information(sdp_message *msg, const char *information);

/* u=<uri> */
const char *sdp_message_get_uri(const sdp_message *msg);
SDP_RESULT sdp_message_set_uri(sdp_message *msg, const char *uri);

/* e=<email-address> */
uint32 sdp_message_emails_len(const sdp_message *msg);
const char *sdp_message_get_email(const sdp_message *msg, uint32 idx);
SDP_RESULT sdp_message_add_email(sdp_message *msg, const char *email);

/* p=<phone-number> */
uint32 sdp_message_phones_len(const sdp_message *msg);
const char *sdp_message_get_phone(const sdp_message *msg, uint32 idx);
SDP_RESULT sdp_message_add_phone(sdp_message *msg, const char *phone);

/* c=<nettype> <addrtype> <connection-address>[/<ttl>][/<number of addresses>] */
const sdp_connection *sdp_message_get_connection(const sdp_message *msg);
SDP_RESULT sdp_message_set_connection(sdp_message *msg, const char *nettype, 
	const char *addrtype, const char *address, uint32 ttl, uint32 addr_number);

/* b=<bwtype>:<bandwidth> */
uint32 sdp_message_bandwidths_len(const sdp_message *msg);
const sdp_bandwidth *sdp_message_get_bandwidth(const sdp_message *msg, uint32 idx);
SDP_RESULT sdp_message_add_bandwidth(sdp_message *msg, const char *bwtype, 
	uint32 bandwidth);

/* t=<start-time> <stop-time> and
 * r=<repeat interval> <active duration> <offsets from start-time> */
uint32 sdp_message_times_len(const sdp_message *msg);
const sdp_time *sdp_message_get_time(const sdp_message *msg, uint32 idx);
SDP_RESULT sdp_message_add_time(sdp_message *msg, const char *start, 
	const char *stop, const char **repeat);

/* z=<adjustment time> <offset> <adjustment time> <offset> .... */
uint32 sdp_message_zones_len(const sdp_message *msg);
const sdp_zone *sdp_message_get_zone(const sdp_message *msg, uint32 idx);
SDP_RESULT sdp_message_add_zone(sdp_message *msg, const char *adj_time, 
	const char *typed_time);

/* k=<method>[:<encryption key>] */
const sdp_key *sdp_message_get_key(const sdp_message *msg);
SDP_RESULT sdp_message_set_key(sdp_message *msg, const char *type, 
	const char *data);

/* a=... */
uint32 sdp_message_attributes_len(const sdp_message *msg);
const sdp_attribute *sdp_message_get_attribute(const sdp_message *msg, uint32 idx);
const char *sdp_message_get_attribute_val(const sdp_message *msg, const char *key);
const char *sdp_message_get_attribute_val_n(const sdp_message *msg, 
	const char *key, uint32 nth);
SDP_RESULT sdp_message_add_attribute(sdp_message *msg, const char *key, 
	const char *value);

/* m=.. sections */
uint32 sdp_message_medias_len(const sdp_message *msg);
const sdp_media *sdp_message_get_media(const sdp_message *msg, uint32 idx);
SDP_RESULT sdp_message_add_media(sdp_message *msg, sdp_media *media);

SDP_RESULT sdp_message_dump(const sdp_message *msg);

/* Media descriptions */
SDP_RESULT sdp_media_new(sdp_media **media);
SDP_RESULT sdp_media_init(sdp_media *media);
SDP_RESULT sdp_media_uninit(sdp_media *media);
SDP_RESULT sdp_media_free(sdp_media *media);

int sdp_media_as_text(const sdp_media * media, char * str, uint32 str_size);

/* m=<media> <port>/<number of ports> <proto> <fmt> ... */
const char *sdp_media_get_media(const sdp_media *media);
SDP_RESULT sdp_media_set_media(sdp_media *media, const char *med);

uint32 sdp_media_get_port(const sdp_media *media);
uint32 sdp_media_get_num_ports(const sdp_media *media);
SDP_RESULT sdp_media_set_port_info(sdp_media *media, uint32 port, 
	uint32 num_ports);

const char *sdp_media_get_proto(const sdp_media *media);
SDP_RESULT sdp_media_set_proto(sdp_media *media, const char *proto);

uint32 sdp_media_formats_len(const sdp_media *media);
const char *sdp_media_get_format(const sdp_media *media, uint32 idx);
SDP_RESULT sdp_media_add_format(sdp_media *media, const char *format);

/* i=<session description> */
const char *sdp_media_get_information(const sdp_media *media);
SDP_RESULT sdp_media_set_information(sdp_media *media, const char *information);

/* c=<nettype> <addrtype> <connection-address>[/<ttl>][/<number of addresses>] */
uint32 sdp_media_connections_len(const sdp_media *media);
const sdp_connection *sdp_media_get_connection(const sdp_media *media, uint32 idx);
SDP_RESULT sdp_media_add_connection(sdp_media *media, const char *nettype, 
	const char *addrtype, const char *address, uint32 ttl, uint32 addr_number);

/* b=<bwtype>:<bandwidth> */
uint32 sdp_media_bandwidths_len(const sdp_media *media);
const sdp_bandwidth *sdp_media_get_bandwidth(const sdp_media *media, uint32 idx);
SDP_RESULT sdp_media_add_bandwidth(sdp_media *media, const char *bwtype, 
	uint32 bandwidth);

/* k=<method>:<encryption key> */
const sdp_key *sdp_media_get_key(const sdp_media *media);
SDP_RESULT sdp_media_set_key(sdp_media *media, const char *type, const char *data);

/* a=... */
uint32 sdp_media_attributes_len(const sdp_media *media);
const sdp_attribute *sdp_media_get_attribute(const sdp_media *media, uint32 idx);
const char *sdp_media_get_attribute_val(const sdp_media *media, const char *key);
const char *sdp_media_get_attribute_val_n(const sdp_media *media, const char *key, 
	uint32 nth);
SDP_RESULT sdp_media_add_attribute(sdp_media *media, const char *key, const char *value);



#endif

