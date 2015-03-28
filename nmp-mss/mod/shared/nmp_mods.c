#include <stdlib.h>
#include "nmp_mods.h"
#include "nmp_errno.h"
#include "nmp_debug.h"


static gint mss_running_mode = MSS_RUNNING_MODE_INIT;
static guint internal_msg_seq_generator = 0;
char g_mss_name[MSS_NAME_LEN];

gint
nmp_mss_mod_deliver_msg(NmpAppMod *self, gint dst, gint msg_id,
	void *parm, gint size, void (*destroy)(void *, gsize size))
{
	NmpSysMsg *msg;

	msg = nmp_sysmsg_new(msg_id, parm, size, ++internal_msg_seq_generator,
		(NmpMsgPrivDes)destroy);
	if (G_UNLIKELY(!msg))
		return -ENOMEM;

	MSG_SET_DSTPOS(msg, dst);
	nmp_app_obj_deliver_out((NmpAppObj*)self, msg);
	return 0;
}


gint
nmp_mss_mod_deliver_msg_2(NmpAppMod *self, gint dst, gint msg_id,
	void *parm, gint size)
{
	NmpSysMsg *msg;

	msg = nmp_sysmsg_new_2(msg_id, parm, size, ++internal_msg_seq_generator);
	if (G_UNLIKELY(!msg))
		return -ENOMEM;

	MSG_SET_DSTPOS(msg, dst);
	nmp_app_obj_deliver_out((NmpAppObj*)self, msg);
	return 0;
}


gint
nmp_mss_get_running_mode( void )
{
	return mss_running_mode;
}


void
nmp_mss_set_running_mode(gint mode)
{
	mss_running_mode = mode;
}


void
nmp_mss_reboot(const gchar *why)
{
	nmp_warning(
		"Reboot Mss Due To '%s'!", why
	);

	system("sync");
	FATAL_ERROR_EXIT;
}

//:~ End
