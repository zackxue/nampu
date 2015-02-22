/*
 * nmp_mods.h
 *
 * This file declares data structures shared by all mods.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#ifndef __NMP_MOD_H__
#define __NMP_MOD_H__

#include <glib.h>
#include "nmp_afx.h"


//@{mods slots:}
#define BUSSLOT_POS_CU		BUSSLOT_POS_0
#define BUSSLOT_POS_PU		BUSSLOT_POS_1
#define BUSSLOT_POS_DBS	BUSSLOT_POS_2
#define BUSSLOT_POS_BSS	BUSSLOT_POS_3
#define BUSSLOT_POS_MDS	BUSSLOT_POS_4
#define BUSSLOT_POS_MSS	BUSSLOT_POS_5
#define BUSSLOT_POS_AAA	BUSSLOT_POS_6
#define BUSSLOT_POS_AMS	BUSSLOT_POS_7
#define BUSSLOT_POS_CASCADE	BUSSLOT_POS_8
#define BUSSLOT_POS_TW			BUSSLOT_POS_9
#define BUSSLOT_POS_WDD		BUSSLOT_POS_10
#define BUSSLOT_POS_LOG		BUSSLOT_POS_11
#define BUSSLOT_POS_IVS		BUSSLOT_POS_12
//:}

#define MAX_ID_LEN			32
#define ID_OF_GUEST(pbo)	(&(((JpfGuestBase*)pbo)->id.id_value[0]))
#define ID_STR(pid)			(&((JpfID*)pid)->id_value[0])
#define IO_OF_GUEST(guest)   (((JpfGuestBase*)guest)->io)

extern guint msg_seq_generator;

G_BEGIN_DECLS

typedef struct _JpfID JpfID;
struct _JpfID
{
	guint		id_hash;	/* Hash value */
	gchar		id_value[MAX_ID_LEN];
};


typedef struct _JpfNewIOList JpfNewIOList;
struct _JpfNewIOList
{
	GList			*list;		/* hold io objects */
	GMutex			*lock;		/* list lock */

	guint			timeout;	/* seconds */
	guint			timer_id;	/* for timeout clear */
	gpointer		owner;		/* point to mods */
};


typedef struct _JpfNewIO JpfNewIO;
struct _JpfNewIO
{
	JpfNewIOList	*list;		/* list we are in */
	JpfNetIO		*io;
	guint			elapse;
};


typedef enum
{
	MOD_CMD_DESTROY_ENT = 1,
	MOD_CMD_CHECK_ENT
}JpfModCmd;


typedef void (*JpfCmdBlockPrivDes)(gpointer priv);
typedef struct _JpfCmdBlock JpfCmdBlock;
struct _JpfCmdBlock
{
	JpfModCmd			cmd;		/* cmd id */
	gpointer			priv;		/* private data */
	JpfCmdBlockPrivDes	priv_des;	/* private data destructor */
};


typedef struct _JpfGuestContainer JpfGuestContainer;
struct _JpfGuestContainer
{
	JpfNewIOList	*unrecognized_list;	/* unrecognized io objects */

	GAsyncQueue		*cmd_queue;
	GHashTable		*guest_table;
	GMutex			*table_lock;
	GThread			*container_manager;

	gpointer		owner;				/* host mod */
};


typedef struct _JpfGuestBase JpfGuestBase;
typedef void (*JpfGuestFin)(JpfGuestBase *obj, gpointer priv_data);
struct _JpfGuestBase
{
	JpfID				id;				/* must be first */
	gint				ref_count;

	JpfNetIO			*io;
	JpfGuestContainer	*container;
	JpfGuestFin			finalize;
	gpointer			priv_data;
};

typedef void (*JpfGuestVisit)(JpfGuestBase *obj, gpointer data);


JpfGuestContainer *jpf_mods_container_new(gpointer owner,
	guint timeout);

gint jpf_mod_container_add_io(JpfGuestContainer *container,
	JpfNetIO *io);
gint jpf_mod_container_del_io(JpfGuestContainer *container,
	JpfNetIO *io);

gint jpf_mods_container_add_guest(JpfGuestContainer *container,
	JpfGuestBase *guest, JpfID *conflict);
gint jpf_mods_container_del_guest(JpfGuestContainer *container,
	JpfGuestBase *guest);
gint jpf_mods_container_del_guest_2(JpfGuestContainer *container,
	JpfNetIO *io, JpfID *out);

JpfGuestBase *jpf_mods_container_get_guest(JpfGuestContainer *container,
	JpfNetIO *io);
JpfGuestBase *jpf_mods_container_get_guest_2(JpfGuestContainer *container,
	const gchar *id_str);
void jpf_mods_container_put_guest(JpfGuestContainer *container,
	JpfGuestBase *guest);

gint jpf_mods_container_guest_counts(JpfGuestContainer *container);

JpfGuestBase *jpf_mods_guest_new(gsize size, const gchar *id,
	JpfGuestFin finalize, gpointer priv_data);

void jpf_mods_guest_attach_io(JpfGuestBase *guest, JpfNetIO *io);
void jpf_mods_guest_ref(JpfGuestBase *base_obj);
void jpf_mods_guest_unref(JpfGuestBase *base_obj);
gint jpf_cms_mod_deliver_msg(JpfAppObj *self, gint dst, gint msg_id,
	void *parm, gint size, void (*destroy)(void *, gsize size));
gint jpf_cms_mod_deliver_msg_2(JpfAppObj *self, gint dst, gint msg_id,
	void *parm, gint size);
gint jpf_cms_mod_cpy_msg(JpfAppObj *app_obj,JpfSysMsg *msg,  gint dst);

void jpf_mods_container_do_for_each(JpfGuestContainer *container,
	JpfGuestVisit func, gpointer user_data);

G_END_DECLS

#endif	//__NMP_MOD_H__
