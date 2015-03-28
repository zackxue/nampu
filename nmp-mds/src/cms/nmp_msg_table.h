#ifndef __NMP_MSG_TABLE_H__
#define __NMP_MSG_TABLE_H__
#include "nmp_msg.h"

#define MAX_ENTRIES		32

typedef void (*JpfMsgFunc)(JpfMdsMsg *msg, gpointer data);

typedef struct _JpfMsgTable JpfMsgTable;
struct _JpfMsgTable
{
	gint entries;

	struct _JpfMsgEntry {
		gint	msg_Id;
		JpfMsgFunc	fun;
	}msg_entries[MAX_ENTRIES];
};

JpfMsgTable *nmp_msg_table_new( void );

gint nmp_msg_table_register(JpfMsgTable *table, gint msg_Id,
	JpfMsgFunc fun);

void nmp_msg_table_call(JpfMsgTable *table, JpfMdsMsg *msg,
	gpointer data);

void nmp_msg_table_release(JpfMsgTable *table);

#endif /* __NMP_MSG_TABLE_H__*/
