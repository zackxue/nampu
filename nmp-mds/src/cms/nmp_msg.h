#ifndef __NMP_MSG_H__
#define __NMP_MSG_H__

#include "nmp_net.h"

#define MSG_FLG_REPONSE		0x1

typedef void (*NmpMsgFin)(gpointer data, gsize size);
typedef struct _NmpMdsMsg NmpMdsMsg;
struct _NmpMdsMsg
{
	gint			msg_Id;
	guint			seq;
	guint			flags;
	NmpNetIO 		*io;	/* msg from */
	gpointer 		*private;
	gint			priv_size;
	NmpMsgFin		fin;
};

#define MSG_ID(msg) 	((msg)->msg_Id)
#define MSG_SEQ(msg) 	((msg)->seq)
#define MSG_IO(msg) 	((msg)->io)
#define MSG_DATA(msg) 	((msg)->private)
#define MSG_DATA_SIZE(msg) ((msg)->priv_size)
#define MSG_RESPONSE(msg) ((msg)->flags & MSG_FLG_REPONSE)

NmpMdsMsg *nmp_alloc_msg(gint msg_id, gpointer data, gsize size, guint seq);
NmpMdsMsg *nmp_alloc_msg_2(gint msg_id, gpointer data, gsize size,
	NmpMsgFin fin, guint seq);

void nmp_attach_msg_io(NmpMdsMsg *msg, NmpNetIO *io);
void nmp_set_msg_data(NmpMdsMsg *msg, gpointer data, gsize size);
void nmp_set_msg_data_2(NmpMdsMsg *msg, gpointer data, gsize size,
	NmpMsgFin fin);
void nmp_free_msg(NmpMdsMsg *msg);

#endif	/* __NMP_MSG_H__ */
