#ifndef __NMP_MSG_ID_TABLE_H__
#define __NMP_MSG_ID_TABLE_H__

#include <string.h>
#include "nmp_debug.h"

#define MESSAGE_RANGE_LOWER                         -1  /* {{Range lower */
#define MESSAGE_RANGE_UPPER                         512 /* Range upper}} */

typedef struct _JpfMsgMap JpfMsgMap;
struct _JpfMsgMap
{
    struct _JpfMsgMapEntry
    {
        const gchar *msg_str;
    }map_entries[MESSAGE_RANGE_UPPER];
};

#define MESSAGE_INVALID_ID(msg_id) \
    ((msg_id) <= MESSAGE_RANGE_LOWER || (msg_id) >= MESSAGE_RANGE_UPPER)

#define MESSAGE_ID_TO_STR(map, msg_id) \
	jpf_message_id_2_str_##map(msg_id)

#define MESSAGE_STR_TO_ID(map, msg_str)  \
    jpf_message_str_2_id_##map(msg_str)

#define USING_MSG_ID_MAP(map) \
	extern gint jpf_message_str_2_id_##map(const gchar *msg_str); \
	extern const gchar *jpf_message_id_2_str_##map(gint id);

#define BEGIN_MSG_ID_MAPPING(name) \
JpfMsgMap g_msg_map_##name = \
{\
	{

#define END_MSG_ID_MAPPING(name) \
		{ NULL } \
	}\
};\
\
\
gint jpf_message_str_2_id_##name(const gchar *msg_str) \
{\
    gint id = MESSAGE_RANGE_LOWER;\
    G_ASSERT(msg_str != NULL);\
\
    for (; ++id < MESSAGE_RANGE_UPPER; ) \
    {\
        const gchar *str = g_msg_map_##name.map_entries[id].msg_str; \
        if (str && !strcmp(str, msg_str)) \
            return id; \
    } \
\
    return id; \
}\
\
\
const gchar *jpf_message_id_2_str_##name(gint msg_id)\
{\
	return MESSAGE_INVALID_ID(msg_id) ? "InvalidMsg" \
		: g_msg_map_##name.map_entries[msg_id].msg_str; \
}


#endif	/* __NMP_MSG_ID_TABLE_H__ */
