#include "nmp_mod_pu.h"
#include "nmp_debug.h"
#include "nmp_pu_struct.h"
#include "nmp_errno.h"
#include "nmp_ports.h"
#include "nmp_proto.h"
#include "nmp_message.h"
#include "nmp_msg_share.h"
#include "nmp_msg_struct.h"
#include "nmp_shared.h"
#include "nmp_memory.h"
#include "nmp_msg_pu.h"

G_DEFINE_TYPE(JpfModPu, nmp_mod_pu, NMP_TYPE_MODACCESS);
//static guint msg_seq_generator = 0;


void
nmp_mod_pu_register_msg_handler(JpfModPu *self);


static __inline__ void
nmp_mod_pu_struct_init(JpfPu *pu, JpfPuType type)
{
	pu->type = type;
	pu->state = STAT_PU_REGISTERING;
	pu->age = 0;

	nmp_mod_init_resource(&pu->res_ctl);
}


void
nmp_mod_pu_change_pu_online_status(NmpAppObj *app_obj,
    JpfPuOnlineStatusChange notify_info)
{
	nmp_cms_mod_deliver_msg_2(app_obj, BUSSLOT_POS_DBS,
		MESSAGE_CHANGED_PU_ONLINE_STATE, &notify_info, sizeof(notify_info));
}


static void
nmp_mod_pu_destroy(JpfGuestBase *obj, gpointer priv_data)
{
    G_ASSERT(obj != NULL);

    NmpAppObj *self = NMP_APPOBJ(priv_data);
    JpfPuOnlineStatusChange notify_info;
    JpfPu *pu = NULL;

    pu = (JpfPu *)obj;
    if (pu->state == STAT_PU_ONLINE)
    {
        memset(&notify_info, 0, sizeof(notify_info));
        strncpy(notify_info.puid, ID_OF_GUEST(obj), MAX_ID_LEN - 1);
        strncpy(notify_info.domain_id, jpf_get_local_domain_id(), DOMAIN_ID_LEN - 1);
        notify_info.new_status = 0;
        nmp_mod_pu_change_pu_online_status(self, notify_info);
    }
}


gint
nmp_mod_pu_register(JpfModPu *self, JpfNetIO *io, const gchar *id,
	JpfPuType t, JpfID *conflict)
{
	JpfGuestBase *pu_base;
	gint ret;
	G_ASSERT(self != NULL && io != NULL);

	pu_base = jpf_mods_guest_new(sizeof(JpfPu), id, nmp_mod_pu_destroy, self);
	if (G_UNLIKELY(!pu_base))
		return -E_NOMEM;

	nmp_mod_pu_struct_init((JpfPu*)pu_base, t);
	jpf_mods_guest_attach_io(pu_base, io);

	ret = jpf_mods_container_add_guest(self->container,
		pu_base, conflict);
	if (G_UNLIKELY(ret))
	{
		jpf_print(
			"<JpfModPu> pu-%s register failed, err:%d", id, ret
		);
	}

	jpf_mods_guest_unref(pu_base);
	return ret;
}


static void
nmp_mod_pu_set_recheck_tag_one(JpfGuestBase *obj, gpointer data)
{
	JpfPu *pu = NULL;

	pu = (JpfPu *)obj;
	pu->recheck = 1;
}


void
nmp_mod_pu_set_recheck_tag(JpfModPu *self)
{
	jpf_mods_container_do_for_each(self->container,
		nmp_mod_pu_set_recheck_tag_one, NULL);
}


gint
nmp_mod_pu_sync_req(JpfModPu *self, NmpMsgID msg_id,
       gpointer req, gint req_size,  gpointer res, gint res_size)
{
	gint err = 0;
	JpfMsgErrCode *res_info;
	NmpSysMsg *msg;
	G_ASSERT(self != NULL);

	msg = jpf_sysmsg_new_2(msg_id, req, req_size, ++msg_seq_generator);
	if (G_UNLIKELY(!msg))
		return -E_NOMEM;

       MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
	err = nmp_app_mod_sync_request((NmpAppMod*)self, &msg);
 	if (G_UNLIKELY(err))	/* send failed */
	{
		jpf_warning(
			"<JpfModPu> request cmd %d failed.", msg_id
		);
		jpf_sysmsg_destroy(msg);
		return err;
	}

	if (G_UNLIKELY(!msg))	/* sent, but no response */
	{
		jpf_warning(
			"<JpfModPu> request cmd %d timeout.", msg_id
		);
		return -E_TIMEOUT;
	}

	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (G_UNLIKELY(res))
		memcpy(res, res_info, res_size);

	err = RES_CODE(res_info);
	jpf_sysmsg_destroy(msg);

	return err;
}


gpointer
nmp_mod_pu_sync_req_2(JpfModPu *self, NmpMsgID msg_id,
       gpointer req, gint req_size, gint *res_size)
{
	gint err = 0;
	JpfMsgErrCode *res_info;
	gpointer res;
	NmpSysMsg *msg;
	G_ASSERT(self != NULL);

	msg = jpf_sysmsg_new_2(msg_id, req, req_size, ++msg_seq_generator);
	if (G_UNLIKELY(!msg))
		return NULL;

       MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
	err = nmp_app_mod_sync_request((NmpAppMod*)self, &msg);
 	if (G_UNLIKELY(err))	/* send failed */
	{
		jpf_warning(
			"<JpfModPu> request cmd %d failed!", msg_id
		);

		jpf_sysmsg_destroy(msg);
		res_info = (JpfMsgErrCode *)jpf_mem_kalloc(sizeof(JpfMsgErrCode));
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
			"<JpfModPu> request cmd %d timeout!", msg_id
		);
		res_info = (JpfMsgErrCode *)jpf_mem_kalloc(sizeof(JpfMsgErrCode));
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

	res_info = (JpfMsgErrCode *)jpf_mem_kalloc(MSG_DATA_SIZE(msg));
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
nmp_mod_pu_io_close(JpfModAccess *s_self, JpfNetIO *io, gint err)
{
	gint ret;
	JpfModPu *self;
	G_ASSERT(s_self != NULL);

	self = (JpfModPu*)s_self;
	ret = nmp_mod_container_del_io(self->container, io);
	if (G_UNLIKELY(!ret))
	{
		jpf_print(
			"<JpfModPu> unrecognized connection closed, err:%d.",
			err
		);
	}
}


static gint
nmp_mod_pu_io_init(JpfModAccess *s_self, JpfNetIO *io)
{
	gint err;
	JpfModPu *self;
	G_ASSERT(s_self != NULL);

	self = (JpfModPu*)s_self;
	err = nmp_mod_container_add_io(self->container, io);
	if (err)
	{
		jpf_error(
			"<JpfModPu> insert io to temp list err: %d!",
			err
		);
		return err;
	}

	jpf_net_unref_io(io);
	return 0;
}


gint
nmp_mod_pu_setup(NmpAppMod *am_self)
{
	gint err;
	JpfModAccess *ma_self;
	JpfModPu *self;
	struct sockaddr_in sin;
	G_ASSERT(am_self != NULL);

	self = (JpfModPu*)am_self;
	ma_self = (JpfModAccess*)am_self;

	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(JPFCMS_PU_PORT);
	sin.sin_addr.s_addr = INADDR_ANY;

	nmp_mod_acc_init_net(ma_self, &jxj_packet_proto, &jxj_xml_proto);

	self->listen_io = nmp_mod_acc_create_listen_io(
		ma_self, (struct sockaddr*)&sin, &err
	);
	if (!self->listen_io)
	{
		jpf_error("<JpfModPu> create listen io failed!");
		return err;
	}

	nmp_app_mod_set_name(am_self, "MOD-PU");
	nmp_mod_pu_register_msg_handler(self);
	return 0;
}


static void
nmp_mod_pu_init(JpfModPu *self)
{
	self->container = jpf_mods_container_new(
		self,
		jpf_get_sys_parm_int(SYS_PARM_WAIT_AFTER_CONNECTED)
	);
	if (G_UNLIKELY(!self->container))
	{
		jpf_error("<JpfModPu> alloc guest container failed!");
		FATAL_ERROR_EXIT;
	}

    self->listen_io = NULL;
}


void
nmp_mod_pu_update_online_status(NmpAppObj *self, NmpSysMsg *msg)
{
    nmp_app_obj_deliver_out(self, msg);
}


static void
nmp_mod_pu_class_init(JpfModPuClass *k_class)
{
	JpfModAccessClass *ma_class = (JpfModAccessClass*)k_class;
	NmpAppModClass *am_class = (NmpAppModClass*)k_class;

	ma_class->io_close	= nmp_mod_pu_io_close;
	ma_class->io_init	= nmp_mod_pu_io_init;
	am_class->setup_mod	= nmp_mod_pu_setup;
}


//:~ End
