#include <pthread.h>
#include "nmp_mod_bss.h"
#include "nmp_ports.h"
#include "nmp_share_errno.h"
#include "nmp_bss_struct.h"
#include "message/nmp_msg_bss.h"
#include "nmp_share_debug.h"
#include "nmp_message.h"
#include "nmp_memory.h"
#include "nmp_proto.h"
#include "search_device.h"



G_DEFINE_TYPE(NmpModBss, nmp_mod_bss, NMP_TYPE_MODACCESS);

//static guint msg_seq_generator = 0;

pthread_mutex_t g_search_pu_mutex;

void
nmp_mod_bss_register_msg_handler(NmpModBss *self);

gint
nmp_mod_bss_new_admin(NmpModBss *self, NmpNetIO *io, const gchar *id,
	NmpID *conflict)
{
	NmpGuestBase *admin;
	gint ret;
	G_ASSERT(self != NULL && io != NULL);

	admin = nmp_mods_guest_new(sizeof(NmpBss), id, NULL, NULL);
	if (G_UNLIKELY(!admin))
		return -E_NOMEM;

	nmp_mods_guest_attach_io(admin, io);

	ret = nmp_mods_container_add_guest(self->container,
		admin, conflict);
	if (G_UNLIKELY(ret))
	{
		nmp_print(
			"<NmpModBss> add bss user-%s failed, err:%d", id, ret
		);
	}

	nmp_mods_guest_unref(admin);
	return ret;
}


gint
nmp_mod_bss_sync_req(NmpModBss *self, NmpMsgID msg_id,
       gpointer req, gint req_size,  gpointer res, gint res_size)
{
	gint err = 0;
	NmpMsgErrCode *res_info;
	NmpSysMsg *msg;
	G_ASSERT(self != NULL);

	msg = nmp_sysmsg_new_2(msg_id, req, req_size, ++msg_seq_generator);
	if (G_UNLIKELY(!msg))
		return -E_NOMEM;

    MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
	err = nmp_app_mod_sync_request((NmpAppMod*)self, &msg);
 	if (G_UNLIKELY(err))	/* send failed */
	{
		nmp_warning(
			"<NmpModBss> request cmd %d failed!", msg_id
		);
		nmp_sysmsg_destroy(msg);
		return err;
	}

	if (G_UNLIKELY(!msg))	/* sent, but no response */
	{
		nmp_warning(
			"<NmpModBss> request cmd %d timeout!", msg_id
		);
		return -E_TIMEOUT;
	}

	res_info = (NmpMsgErrCode *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (G_UNLIKELY(res))
		memcpy(res, MSG_GET_DATA(msg), res_size);

	err = RES_CODE(res_info);
	nmp_sysmsg_destroy(msg);

	return err;
}


gpointer
nmp_mod_bss_sync_req_2(NmpModBss *self, NmpMsgID msg_id,
       gpointer req, gint req_size, gint *res_size)
{
	gint err = 0;
	NmpMsgErrCode *res_info;
	gpointer res;
	NmpSysMsg *msg;
	G_ASSERT(self != NULL);

	msg = nmp_sysmsg_new_2(msg_id, req, req_size, ++msg_seq_generator);
	if (G_UNLIKELY(!msg))
		return NULL;

    MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
	err = nmp_app_mod_sync_request((NmpAppMod*)self, &msg);
 	if (G_UNLIKELY(err))	/* send failed */
	{
		nmp_warning(
			"<NmpModBss> request cmd %d failed!", msg_id
		);

		nmp_sysmsg_destroy(msg);
		res_info = nmp_mem_kalloc(sizeof(NmpMsgErrCode));
		if (res_info)
		{
			SET_CODE(res_info, err);
			*res_size = sizeof(NmpMsgErrCode);
		}

		return res_info;
	}

	if (G_UNLIKELY(!msg))	/* sent, but no response */
	{
		nmp_warning(
			"<NmpModBss> request cmd %d timeout!", msg_id
		);
		res_info = nmp_mem_kalloc(sizeof(NmpMsgErrCode));
		err = -E_TIMEOUT;
		if (res_info)
		{
			SET_CODE(res_info, err);
			*res_size = sizeof(NmpMsgErrCode);
		}
		return res_info;
	}

	res = MSG_GET_DATA(msg);
	if (!res)
	{
		nmp_sysmsg_destroy(msg);
		return NULL;
	}

	res_info = nmp_mem_kalloc(MSG_DATA_SIZE(msg));
	if (G_UNLIKELY(!res_info))
       {
           nmp_sysmsg_destroy(msg);
           return NULL;
       }

	*res_size =  MSG_DATA_SIZE(msg);
	memcpy(res_info, res, *res_size);
	nmp_sysmsg_destroy(msg);

	return res_info;
}


static void
nmp_mod_bss_io_close(NmpModAccess *s_self, NmpNetIO *io, gint err)
{
	gint ret;
	NmpModBss *self;
	G_ASSERT(s_self != NULL);

	self = (NmpModBss*)s_self;

	ret = nmp_mod_container_del_io(self->container, io);
	if (G_UNLIKELY(!ret))
	{
		nmp_print(
			"<NmpModBss> unrecognized connection closed, err:%d.",
			err
		);
	}
}


static gint
nmp_mod_bss_io_init(NmpModAccess *s_self, NmpNetIO *io)
{
	gint err;
	NmpModBss *self;
	G_ASSERT(s_self != NULL);

	self = (NmpModBss*)s_self;

	err = nmp_mod_container_add_io(self->container, io);
	if (err)
	{
		nmp_error(
			"<NmpModBss> insert io to temp list err: %d!",
			err
		);
		return err;
	}

	nmp_net_unref_io(io);
	return 0;
}


gint
nmp_mod_bss_setup(NmpAppMod *am_self)
{
	gint err;
	NmpModAccess *ma_self;
	NmpModBss *self;
	struct sockaddr_in sin;
	G_ASSERT(am_self != NULL);

	self = (NmpModBss*)am_self;
	ma_self = (NmpModAccess*)am_self;

	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(JPFCMS_BSS_PORT);
	sin.sin_addr.s_addr = INADDR_ANY;

	nmp_mod_acc_init_net(ma_self, &nmp_packet_proto, &nmp_xml_proto);

	self->listen_io = nmp_mod_acc_create_listen_io(
		ma_self, (struct sockaddr*)&sin, &err
	);
	if (!self->listen_io)
	{
		nmp_error("<NmpModPu> create listen io failed!");
		return err;
	}

	nmp_net_set_heavy_io_load(self->listen_io);
	nmp_app_mod_set_name(am_self, "MOD-BSS");
	nmp_mod_bss_register_msg_handler(self);

	return 0;
}


static void
nmp_mod_bss_init(NmpModBss *self)
{
	self->container = nmp_mods_container_new(
		self,
		nmp_get_sys_parm_int(SYS_PARM_WAIT_AFTER_CONNECTED)
	);
	if (G_UNLIKELY(!self->container))
	{
		nmp_error("<NmpModBss> alloc guest container failed!");
		FATAL_ERROR_EXIT;
	}

	self->listen_io = NULL;
	search_device_init();
	pthread_mutex_init(&g_search_pu_mutex, NULL);
}


static void
nmp_mod_bss_class_init(NmpModBssClass *k_class)
{
	NmpModAccessClass *ma_class = (NmpModAccessClass*)k_class;
	NmpAppModClass *am_class = (NmpAppModClass*)k_class;

	ma_class->io_close	= nmp_mod_bss_io_close;
	ma_class->io_init	= nmp_mod_bss_io_init;
	am_class->setup_mod	= nmp_mod_bss_setup;
}


void
nmp_mod_bss_deliver_out_msg(NmpAppObj *self, NmpSysMsg *msg)
{
    nmp_app_obj_deliver_out(self, msg);
}


static __inline__ void
__nmp_mod_bss_force_usr_offline(NmpModBss *self, const char *admin_name, NmpSysMsg *msg)
{
    NmpNetIO *io;
    NmpGuestBase *bss_base;
    NmpSysMsg *msg_copy;

    bss_base = nmp_mods_container_get_guest_2(self->container, admin_name);
    if (G_UNLIKELY(!bss_base))
    {
        nmp_warning("<NmpModBss> admin name:%s No such guest.", admin_name);
    }
    else
    {
    	msg_copy = nmp_sysmsg_copy_one(msg);
		if (!msg_copy)
		{
			nmp_error(
				"<NmpModCu> copy sys-msg failed while delivering msg to cu."
			);
			FATAL_ERROR_EXIT;
		}
       io = IO_OF_GUEST(bss_base);
       BUG_ON(!io);
       nmp_sysmsg_attach_io(msg_copy, io);
		nmp_app_obj_deliver_in((NmpAppObj*)self, msg_copy);
		nmp_mod_acc_release_io((NmpModAccess*)self,  io);
       nmp_mod_container_del_io(self->container,  io);
       nmp_mods_container_put_guest(self->container, bss_base);
     }
}


void
nmp_mod_bss_force_usr_offline(NmpModBss *self, const char *admin_name, NmpSysMsg *msg)
{
	G_ASSERT(self != NULL && msg != NULL);

	__nmp_mod_bss_force_usr_offline(self, admin_name, msg);
}


void
nmp_mod_bss_notify_policy_change(NmpAppObj *self,
    gpointer policy_change, gint size)
{
    NmpSysMsg *policy_change_msg;
    /*
    msg_copy = nmp_sysmsg_copy_one(msg);
    if (!msg_copy)
    {
    nmp_error(
    	"<NmpModCu> copy sys-msg failed while delivering msg to cu."
    );
    		FATAL_ERROR_EXIT;
    }
    */
    policy_change_msg = nmp_sysmsg_new_2(MESSAGE_MSS_RECORD_POLICY_CHANGE,
        policy_change, size, ++msg_seq_generator);

    if (G_UNLIKELY(!policy_change_msg))
    {
        return;
    }

    MSG_SET_DSTPOS(policy_change_msg, BUSSLOT_POS_MSS);
    nmp_app_obj_deliver_out(self, policy_change_msg);
}

void nmp_search_pu_lock()
{
    pthread_mutex_lock(&g_search_pu_mutex);
}


void nmp_search_pu_unlock()
{
    pthread_mutex_unlock(&g_search_pu_mutex);
}
//:~ End
