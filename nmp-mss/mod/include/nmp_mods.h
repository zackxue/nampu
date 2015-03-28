/*
 * nmp_mods.h
 *
 * This file declares data structures shared by all mods.
 *
 * Copyright(c) by HiMickey, 2010~2014
 * Author:
*/

#ifndef __NMP_MODS_H__
#define __NMP_MODS_H__

#include <glib.h>
#include "nmp_afx.h"
#include "nmp_share_struct.h"


G_BEGIN_DECLS

#define MSS_RUNNING_MODE_INIT	0x10000000
#define MSS_RUNNING_MODE_REC	0x0
#define MSS_RUNNING_MODE_CONF	0x1


//@{mods slots:}
#define BUSSLOT_POS_CMS		BUSSLOT_POS_0
#define BUSSLOT_POS_POLICY	BUSSLOT_POS_1
#define BUSSLOT_POS_DISK	BUSSLOT_POS_2
#define BUSSLOT_POS_STREAM	BUSSLOT_POS_3
#define BUSSLOT_POS_BUFFERING BUSSLOT_POS_4
#define BUSSLOT_POS_VOD		BUSSLOT_POS_5

extern char g_mss_name[MSS_NAME_LEN];

gint nmp_mss_mod_deliver_msg(NmpAppMod *self, gint dst, gint msg_id,
	void *parm, gint size, void (*destroy)(void *, gsize size));

gint nmp_mss_mod_deliver_msg_2(NmpAppMod *self, gint dst, gint msg_id,
	void *parm, gint size);

gint nmp_mss_get_running_mode( void );
void nmp_mss_set_running_mode(gint mode);

void nmp_mss_reboot(const gchar *why);

G_END_DECLS

#endif	//__NMP_MODS_H__
