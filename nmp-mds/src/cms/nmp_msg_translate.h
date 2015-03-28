#ifndef __NMP_PROCXML_H__
#define __NMP_PROCXML_H__

#include "nmp_msg.h"
#include "nmp_xml.h"

#define MESSAGE_MDU_REGISTER				0
#define MESSAGE_MDU_HEART					1
#define MESSAGE_RANGE_UPPER					16

typedef struct _JpfMsgMap JpfMsgMap;
struct _JpfMsgMap
{
	struct _JpfMsgMapEntry
	{
		const gchar *msg_str;
	}map_entries[MESSAGE_RANGE_UPPER];
};

extern JpfMsgMap g_msg_map;

static __inline__ gint
nmp_message_str_2_id(const gchar *msg_str)
{
	gint id = -1;
	G_ASSERT(msg_str != NULL);

	for (; ++id < MESSAGE_RANGE_UPPER; )
	{
		const gchar *str = g_msg_map.map_entries[id].msg_str;
		if (str && !strcmp(str, msg_str))
			return id;
	}

	return id;
}

#define MESSAGE_INVALID_ID(msg_id) \
	((msg_id) < 0 || (msg_id) >= MESSAGE_RANGE_UPPER)

#define MESSAGE_ID_TO_STR(msg_id) \
	(MESSAGE_INVALID_ID(msg_id) ? "InvalidMsg" : g_msg_map.map_entries[msg_id].msg_str)

#define MESSAGE_STR_TO_ID(msg_str)	\
	nmp_message_str_2_id(msg_str) 

JpfMdsMsg *
nmp_get_sysmsg_from_xml(char *xml_buff, gint xml_len, guint seq);

gint 
nmp_proto_create_xml_str(char *xml_buff, int *buff_size, JpfMdsMsg *sys_msg)
;
#endif
