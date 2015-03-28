#ifndef __NMP_MSG_TABLE_H__
#define __NMP_MSG_TABLE_H__
#include "nmp_msg.h"

#define MAX_ENTRIES		32

typedef void (*NmpMsgFunc)(NmpMdsMsg *msg, gpointer data);

typedef struct _NmpMsgTable NmpMsgTable;
struct _NmpMsgTable
{
	gint entries;

	struct _NmpMsgEntry {
		gint	msg_Id;
		NmpMsgFunc	fun;
	}msg_entries[MAX_ENTRIES];
};

NmpMsgTable *nmp_msg_table_new( void );

gint nmp_msg_table_register(NmpMsgTable *table, gint msg_Id,
	NmpMsgFunc fun);

void nmp_msg_table_call(NmpMsgTable *table, NmpMdsMsg *msg,
	gpointer data);

void nmp_msg_table_release(NmpMsgTable *table);

#endif /* __NMP_MSG_TABLE_H__*/
