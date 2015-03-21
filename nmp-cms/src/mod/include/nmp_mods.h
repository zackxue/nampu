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
#define ID_OF_GUEST(pbo)	(&(((NmpGuestBase*)pbo)->id.id_value[0]))
#define ID_STR(pid)			(&((NmpID*)pid)->id_value[0])
#define IO_OF_GUEST(guest)   (((NmpGuestBase*)guest)->io)

extern guint msg_seq_generator;

G_BEGIN_DECLS

typedef struct _NmpID NmpID;
struct _NmpID
{
	guint		id_hash;	/* Hash value */
	gchar		id_value[MAX_ID_LEN];
};


typedef struct _NmpNewIOList NmpNewIOList;
struct _NmpNewIOList
{
	GList			*list;		/* hold io objects */
	GMutex			*lock;		/* list lock */

	guint			timeout;	/* seconds */
	guint			timer_id;	/* for timeout clear */
	gpointer		owner;		/* point to mods */
};


typedef struct _NmpNewIO NmpNewIO;
struct _NmpNewIO
{
	NmpNewIOList	*list;		/* list we are in */
	NmpNetIO		*io;
	guint			elapse;
};


typedef enum
{
	MOD_CMD_DESTROY_ENT = 1,
	MOD_CMD_CHECK_ENT
}NmpModCmd;


typedef void (*NmpCmdBlockPrivDes)(gpointer priv);
typedef struct _NmpCmdBlock NmpCmdBlock;
struct _NmpCmdBlock
{
	NmpModCmd			cmd;		/* cmd id */
	gpointer			priv;		/* private data */
	NmpCmdBlockPrivDes	priv_des;	/* private data destructor */
};


typedef struct _NmpGuestContainer NmpGuestContainer;
struct _NmpGuestContainer
{
	NmpNewIOList	*unrecognized_list;	/* unrecognized io objects */

	GAsyncQueue		*cmd_queue;
	GHashTable		*guest_table;
	GMutex			*table_lock;
	GThread			*container_manager;

	gpointer		owner;				/* host mod */
};


typedef struct _NmpGuestBase NmpGuestBase;
typedef void (*NmpGuestFin)(NmpGuestBase *obj, gpointer priv_data);
struct _NmpGuestBase
{
	NmpID				id;				/* must be first */
	gint				ref_count;

	NmpNetIO			*io;
	NmpGuestContainer	*container;
	NmpGuestFin			finalize;
	gpointer			priv_data;
};

typedef void (*NmpGuestVisit)(NmpGuestBase *obj, gpointer data);


NmpGuestContainer *nmp_mods_container_new(gpointer owner,
	guint timeout);

gint nmp_mod_container_add_io(NmpGuestContainer *container,
	NmpNetIO *io);
gint nmp_mod_container_del_io(NmpGuestContainer *container,
	NmpNetIO *io);

gint nmp_mods_container_add_guest(NmpGuestContainer *container,
	NmpGuestBase *guest, NmpID *conflict);
gint nmp_mods_container_del_guest(NmpGuestContainer *container,
	NmpGuestBase *guest);
gint nmp_mods_container_del_guest_2(NmpGuestContainer *container,
	NmpNetIO *io, NmpID *out);

NmpGuestBase *nmp_mods_container_get_guest(NmpGuestContainer *container,
	NmpNetIO *io);
NmpGuestBase *nmp_mods_container_get_guest_2(NmpGuestContainer *container,
	const gchar *id_str);
void nmp_mods_container_put_guest(NmpGuestContainer *container,
	NmpGuestBase *guest);

gint nmp_mods_container_guest_counts(NmpGuestContainer *container);

NmpGuestBase *nmp_mods_guest_new(gsize size, const gchar *id,
	NmpGuestFin finalize, gpointer priv_data);

void nmp_mods_guest_attach_io(NmpGuestBase *guest, NmpNetIO *io);
void nmp_mods_guest_ref(NmpGuestBase *base_obj);
void nmp_mods_guest_unref(NmpGuestBase *base_obj);
gint nmp_cms_mod_deliver_msg(NmpAppObj *self, gint dst, gint msg_id,
	void *parm, gint size, void (*destroy)(void *, gsize size));
gint nmp_cms_mod_deliver_msg_2(NmpAppObj *self, gint dst, gint msg_id,
	void *parm, gint size);
gint nmp_cms_mod_cpy_msg(NmpAppObj *app_obj,NmpSysMsg *msg,  gint dst);

void nmp_mods_container_do_for_each(NmpGuestContainer *container,
	NmpGuestVisit func, gpointer user_data);

G_END_DECLS

#endif	//__NMP_MOD_H__
