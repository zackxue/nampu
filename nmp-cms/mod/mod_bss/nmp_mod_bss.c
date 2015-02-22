#include <pthread.h>
#include "nmp_mod_bss.h"
#include "nmp_ports.h"
#include "nmp_errno.h"
#include "nmp_bss_struct.h"
#include "message/nmp_msg_bss.h"
#include "nmp_debug.h"
#include "nmp_message.h"
#include "nmp_memory.h"
#include "nmp_proto.h"
#include "search_device.h"



G_DEFINE_TYPE(JpfModBss, jpf_mod_bss, JPF_TYPE_MODACCESS);

//static guint msg_seq_generator = 0;

pthread_mutex_t g_search_pu_mutex;

void
jpf_mod_bss_register_msg_handler(JpfModBss *self);

gint
jpf_mod_bss_new_admin(JpfModBss *self, JpfNetIO *io, const gchar *id,
	JpfID *conflict)
{
	JpfGuestBase *admin;
	gint ret;
	G_ASSERT(self != NULL && io != NULL);

	admin = jpf_mods_guest_new(sizeof(JpfBss), id, NULL, NULL);
	if (G_UNLIKELY(!admin))
		return -E_NOMEM;

	jpf_mods_guest_attach_io(admin, io);

	ret = jpf_mods_container_add_guest(self->container,
		admin, conflict);
	if (G_UNLIKELY(ret))
	{
		jpf_print(
			"<JpfModBss> add bss user-%s failed, err:%d", id, ret
		);
	}

	jpf_mods_guest_unref(admin);
	return ret;
}


gint
jpf_mod_bss_sync_req(JpfModBss *self, JpfMsgID msg_id,
       gpointer req, gint req_size,  gpointer res, gint res_size)
{
	gint err = 0;
	JpfMsgErrCode *res_info;
	JpfSysMsg *msg;
	G_ASSERT(self != NULL);

	msg = jpf_sysmsg_new_2(msg_id, req, req_size, ++msg_seq_generator);
	if (G_UNLIKELY(!msg))
		return -E_NOMEM;

    MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
	err = jpf_app_mod_sync_request((JpfAppMod*)self, &msg);
 	if (G_UNLIKELY(err))	/* send failed */
	{
		jpf_warning(
			"<JpfModBss> request cmd %d failed!", msg_id
		);
		jpf_sysmsg_destroy(msg);
		return err;
	}

	if (G_UNLIKELY(!msg))	/* sent, but no response */
	{
		jpf_warning(
			"<JpfModBss> request cmd %d timeout!", msg_id
		);
		return -E_TIMEOUT;
	}

	res_info = (JpfMsgErrCode *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (G_UNLIKELY(res))
		memcpy(res, MSG_GET_DATA(msg), res_size);

	err = RES_CODE(res_info);
	jpf_sysmsg_destroy(msg);

	return err;
}


gpointer
jpf_mod_bss_sync_req_2(JpfModBss *self, JpfMsgID msg_id,
       gpointer req, gint req_size, gint *res_size)
{
	gint err = 0;
	JpfMsgErrCode *res_info;
	gpointer res;
	JpfSysMsg *msg;
	G_ASSERT(self != NULL);

	msg = jpf_sysmsg_new_2(msg_id, req, req_size, ++msg_seq_generator);
	if (G_UNLIKELY(!msg))
		return NULL;

    MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
	err = jpf_app_mod_sync_request((JpfAppMod*)self, &msg);
 	if (G_UNLIKELY(err))	/* send failed */
	{
		jpf_warning(
			"<JpfModBss> request cmd %d failed!", msg_id
		);

		jpf_sysmsg_destroy(msg);
		res_info = jpf_mem_kalloc(sizeof(JpfMsgErrCode));
		if (res_info)
		{
			SET_CODE(res_info, err);
			*res_size = sizeof(JpfMsgErrCode);
		}

		return res_info;
	}

	if (G_UNLIKELY(!msg))	/* sent, but no response */
	{
		jpf_warning(
			"<JpfModBss> request cmd %d timeout!", msg_id
		);
		res_info = jpf_mem_kalloc(sizeof(JpfMsgErrCode));
		err = -E_TIMEOUT;
		if (res_info)
		{
			SET_CODE(res_info, err);
			*res_size = sizeof(JpfMsgErrCode);
		}
		return res_info;
	}

	res = MSG_GET_DATA(msg);
	if (!res)
	{
		jpf_sysmsg_destroy(msg);
		return NULL;
	}

	res_info = jpf_mem_kalloc(MSG_DATA_SIZE(msg));
	if (G_UNLIKELY(!res_info))
       {
           jpf_sysmsg_destroy(msg);
           return NULL;
       }

	*res_size =  MSG_DATA_SIZE(msg);
	memcpy(res_info, res, *res_size);
	jpf_sysmsg_destroy(msg);

	return res_info;
}


static void
jpf_mod_bss_io_close(JpfModAccess *s_self, JpfNetIO *io, gint err)
{
	gint ret;
	JpfModBss *self;
	G_ASSERT(s_self != NULL);

	self = (JpfModBss*)s_self;

	ret = jpf_mod_container_del_io(self->container, io);
	if (G_UNLIKELY(!ret))
	{
		jpf_print(
			"<JpfModBss> unrecognized connection closed, err:%d.",
			err
		);
	}
}


static gint
jpf_mod_bss_io_init(JpfModAccess *s_self, JpfNetIO *io)
{
	gint err;
	JpfModBss *self;
	G_ASSERT(s_self != NULL);

	self = (JpfModBss*)s_self;

	err = jpf_mod_container_add_io(self->container, io);
	if (err)
	{
		jpf_error(
			"<JpfModBss> insert io to temp list err: %d!",
			err
		);
		return err;
	}

	jpf_net_unref_io(io);
	return 0;
}


gint
jpf_mod_bss_setup(JpfAppMod *am_self)
{
	gint err;
	JpfModAccess *ma_self;
	JpfModBss *self;
	struct sockaddr_in sin;
	G_ASSERT(am_self != NULL);

	self = (JpfModBss*)am_self;
	ma_self = (JpfModAccess*)am_self;

	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(JPFCMS_BSS_PORT);
	sin.sin_addr.s_addr = INADDR_ANY;

	jpf_mod_acc_init_net(ma_self, &jxj_packet_proto, &jxj_xml_proto);

	self->listen_io = jpf_mod_acc_create_listen_io(
		ma_self, (struct sockaddr*)&sin, &err
	);
	if (!self->listen_io)
	{
		jpf_error("<JpfModPu> create listen io failed!");
		return err;
	}

	jpf_net_set_heavy_io_load(self->listen_io);
	jpf_app_mod_set_name(am_self, "MOD-BSS");
	jpf_mod_bss_register_msg_handler(self);

	return 0;
}


static void
jpf_mod_bss_init(JpfModBss *self)
{
	self->container = jpf_mods_container_new(
		self,
		jpf_get_sys_parm_int(SYS_PARM_WAIT_AFTER_CONNECTED)
	);
	if (G_UNLIKELY(!self->container))
	{
		jpf_error("<JpfModBss> alloc guest container failed!");
		FATAL_ERROR_EXIT;
	}

	self->listen_io = NULL;
	search_device_init();
	pthread_mutex_init(&g_search_pu_mutex, NULL);
}


static void
jpf_mod_bss_class_init(JpfModBssClass *k_class)
{
	JpfModAccessClass *ma_class = (JpfModAccessClass*)k_class;
	JpfAppModClass *am_class = (JpfAppModClass*)k_class;

	ma_class->io_close	= jpf_mod_bss_io_close;
	ma_class->io_init	= jpf_mod_bss_io_init;
	am_class->setup_mod	= jpf_mod_bss_setup;
}


void
jpf_mod_bss_deliver_out_msg(JpfAppObj *self, JpfSysMsg *msg)
{
    jpf_app_obj_deliver_out(self, msg);
}


static __inline__ void
__jpf_mod_bss_force_usr_offline(JpfModBss *self, const char *admin_name, JpfSysMsg *msg)
{
    JpfNetIO *io;
    JpfGuestBase *bss_base;
    JpfSysMsg *msg_copy;

    bss_base = jpf_mods_container_get_guest_2(self->container, admin_name);
    if (G_UNLIKELY(!bss_base))
    {
        jpf_warning("<JpfModBss> admin name:%s No such guest.", admin_name);
    }
    else
    {
    	msg_copy = jpf_sysmsg_copy_one(msg);
		if (!msg_copy)
		{
			jpf_error(
				"<JpfModCu> copy sys-msg failed while delivering msg to cu."
			);
			FATAL_ERROR_EXIT;
		}
       io = IO_OF_GUEST(bss_base);
       BUG_ON(!io);
       jpf_sysmsg_attach_io(msg_copy, io);
		jpf_app_obj_deliver_in((JpfAppObj*)self, msg_copy);
		jpf_mod_acc_release_io((JpfModAccess*)self,  io);
       jpf_mod_container_del_io(self->container,  io);
       jpf_mods_container_put_guest(self->container, bss_base);
     }
}


void
jpf_mod_bss_force_usr_offline(JpfModBss *self, const char *admin_name, JpfSysMsg *msg)
{
	G_ASSERT(self != NULL && msg != NULL);

	__jpf_mod_bss_force_usr_offline(self, admin_name, msg);
}


void
jpf_mod_bss_notify_policy_change(JpfAppObj *self,
    gpointer policy_change, gint size)
{
    JpfSysMsg *policy_change_msg;
    /*
    msg_copy = jpf_sysmsg_copy_one(msg);
    if (!msg_copy)
    {
    jpf_error(
    	"<JpfModCu> copy sys-msg failed while delivering msg to cu."
    );
    		FATAL_ERROR_EXIT;
    }
    */
    policy_change_msg = jpf_sysmsg_new_2(MESSAGE_MSS_RECORD_POLICY_CHANGE,
        policy_change, size, ++msg_seq_generator);

    if (G_UNLIKELY(!policy_change_msg))
    {
        return;
    }

    MSG_SET_DSTPOS(policy_change_msg, BUSSLOT_POS_MSS);
    jpf_app_obj_deliver_out(self, policy_change_msg);
}

void jpf_search_pu_lock()
{
    pthread_mutex_lock(&g_search_pu_mutex);
}


void jpf_search_pu_unlock()
{
    pthread_mutex_unlock(&g_search_pu_mutex);
}
//:~ End
