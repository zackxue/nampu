#include "nmp_mod_ams.h"
#include "nmp_ports.h"
#include "nmp_errno.h"
#include "nmp_ams_struct.h"
#include "message/nmp_msg_ams.h"
#include "nmp_debug.h"
#include "nmp_message.h"
#include "nmp_memory.h"
#include "nmp_proto.h"
#include "nmp_internal_msg.h"
#include "nmp_ams_policy.h"


static JpfModAms *g_nmp_mod_ams;


USING_MSG_ID_MAP(cms);

G_DEFINE_TYPE(JpfModAms, nmp_mod_ams, NMP_TYPE_MODACCESS);

void
nmp_mod_ams_register_msg_handler(JpfModAms *self);

gint
nmp_mod_ams_action_handler(gint dst, gint msg_type, void *parm, guint size);

JpfModAms *jpf_get_mod_ams()
{
	return g_nmp_mod_ams;
}


static __inline__ void
nmp_mod_ams_struct_init(JpfAms *ams)
{
	ams->ams_state = STAT_AMS_REGISTERING;
}


void
nmp_mod_ams_change_ams_online_status(NmpAppObj *app_obj,
    JpfMsgAmsOnlineChange notify_info)
{
    NmpSysMsg *msg_notify;

    msg_notify = jpf_sysmsg_new_2(MSG_AMS_ONLINE_CHANGE,
		&notify_info, sizeof(notify_info), ++msg_seq_generator);
    if (G_UNLIKELY(!msg_notify))
        return;

    MSG_SET_DSTPOS(msg_notify, BUSSLOT_POS_DBS);
    nmp_app_obj_deliver_out(app_obj, msg_notify);
}


static void
nmp_mod_ams_destroy(JpfGuestBase *obj, gpointer priv_data)
{
    G_ASSERT(obj != NULL);

    NmpAppObj *self = NMP_APPOBJ(priv_data);
    JpfMsgAmsOnlineChange notify_info;
    JpfAms *ams = NULL;

    ams = (JpfAms *)obj;
    if (ams->ams_state == STAT_AMS_ONLINE)
    {
	    memset(&notify_info, 0, sizeof(notify_info));
	    strncpy(notify_info.ams_id, ID_OF_GUEST(obj), AMS_ID_LEN - 1);
	    notify_info.new_status = 0;
	    nmp_mod_ams_change_ams_online_status(self, notify_info);
    }
}


gint
nmp_mod_ams_new_ams(JpfModAms *self, JpfNetIO *io, const gchar *id,
	JpfID *conflict)
{
	JpfGuestBase *ams;
	gint ret;
	G_ASSERT(self != NULL && io != NULL);

	ams = jpf_mods_guest_new(sizeof(JpfAms), id, nmp_mod_ams_destroy, self);
	if (G_UNLIKELY(!ams))
		return -E_NOMEM;

	nmp_mod_ams_struct_init((JpfAms*)ams);
	jpf_mods_guest_attach_io(ams, io);
	ret = jpf_mods_container_add_guest(self->container,
		ams, conflict);
	if (G_UNLIKELY(ret))
	{
		jpf_print(
			"<JpfModMds> ams:%s register failed, err:%d", id, ret
		);
	}

	jpf_mods_guest_unref(ams);
	return ret;
}


gint
nmp_mod_ams_sync_req(JpfModAms *self, NmpMsgID msg_id,
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
			"<JpfModAms> request cmd %d failed!", msg_id
		);
		jpf_sysmsg_destroy(msg);
		return err;
	}

	if (G_UNLIKELY(!msg))	/* sent, but no response */
	{
		jpf_warning(
			"<JpfModAms> request cmd %d timeout!", msg_id
		);
		return -E_TIMEOUT;
	}

	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (G_UNLIKELY(res))
		memcpy(res, MSG_GET_DATA(msg), res_size);

	err = RES_CODE(res_info);
	jpf_sysmsg_destroy(msg);

	return err;
}


gpointer
nmp_mod_ams_sync_req_2(JpfModAms *self, NmpMsgID msg_id,
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
        	"<JpfModAms> request cmd %d failed!", msg_id
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
			"<JpfModMds> request cmd %d timeout!", msg_id
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


void
nmp_mod_ams_deliver_msg(JpfModAms *self, const char *ams_id, NmpSysMsg *msg)
{
    JpfGuestBase *ams_base;
    gint msg_id;
    NmpSysMsg *msg_copy = NULL;

    msg_id = MSG_GETID(msg);
    msg_copy = jpf_sysmsg_copy_one(msg);
    if (!msg_copy)
    {
        jpf_error(
        	"<JpfModCu> copy sys-msg failed while delivering msg to cu."
        );
        FATAL_ERROR_EXIT;
    }

    ams_base = jpf_mods_container_get_guest_2(self->container, ams_id);
    if (G_UNLIKELY(!ams_base))
    {
        jpf_warning("<JpfModAms> deliver msg '%s' failed, AmsId:%s no such ams.",
            MESSAGE_ID_TO_STR(cms, msg_id), ams_id);
        jpf_sysmsg_destroy(msg);

        //return MFR_ACCEPTED;
    }
    jpf_sysmsg_attach_io(msg_copy, IO_OF_GUEST(ams_base));
    nmp_app_obj_deliver_in((NmpAppObj*)self, msg_copy);
    nmp_mod_acc_release_io((JpfModAccess*)self,  IO_OF_GUEST(ams_base));
    nmp_mod_container_del_io(self->container,  IO_OF_GUEST(ams_base));
    jpf_mods_container_put_guest(self->container, ams_base);
}

static void
nmp_mod_ams_io_close(JpfModAccess *s_self, JpfNetIO *io, gint err)
{
	gint ret;
	JpfModAms *self;
	G_ASSERT(s_self != NULL);

	self = (JpfModAms*)s_self;

	ret = nmp_mod_container_del_io(self->container, io);
	if (G_UNLIKELY(!ret))
	{
		jpf_print(
			"<JpfModAms> unrecognized connection closed, err:%d.",
			err
		);
	}
}


static gint
nmp_mod_ams_io_init(JpfModAccess *s_self, JpfNetIO *io)
{
	gint err;
	JpfModAms *self;
	G_ASSERT(s_self != NULL);

	self = (JpfModAms*)s_self;

	err = nmp_mod_container_add_io(self->container, io);
	if (err)
	{
		jpf_error(
			"<JpfModAms> insert io to temp list err: %d!",
			err
		);
		return err;
	}

	jpf_net_unref_io(io);
	return 0;
}


gint
nmp_mod_ams_setup(NmpAppMod *am_self)
{
	gint err;
	JpfModAccess *ma_self;
	JpfModAms *self;
	struct sockaddr_in sin;
	G_ASSERT(am_self != NULL);

	self = (JpfModAms*)am_self;
	ma_self = (JpfModAccess*)am_self;

	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(JPFCMS_AMS_PORT);
	sin.sin_addr.s_addr = INADDR_ANY;

	nmp_mod_acc_init_net(ma_self, &jxj_packet_proto, &jxj_xml_proto);

	self->listen_io = nmp_mod_acc_create_listen_io(
		ma_self, (struct sockaddr*)&sin, &err
	);
	if (!self->listen_io)
	{
		jpf_error("<JpfModAms> create listen io failed!");
		return err;
	}

	jpf_net_set_heavy_io_load(self->listen_io);
	nmp_app_mod_set_name(am_self, "MOD-AMS");
	nmp_mod_ams_register_msg_handler(self);

	return 0;
}


static void
nmp_mod_ams_init(JpfModAms *self)
{
	g_nmp_mod_ams = self;
	jpf_ams_set_action_handler(nmp_mod_ams_action_handler);
	jpf_ams_policy_init();

	self->container = jpf_mods_container_new(
		self,
       jpf_get_sys_parm_int(SYS_PARM_WAIT_AFTER_CONNECTED)
	);

	if (G_UNLIKELY(!self->container))
	{
		jpf_error("<JpfModAms> alloc guest container failed!");
		FATAL_ERROR_EXIT;
	}

	//INIT_LIST_HEAD(&self->list_user);
	//g_static_mutex_init(&self->list_ulock);
	self->listen_io = NULL;
}


static void
nmp_mod_ams_class_init(JpfModAmsClass *k_class)
{
	JpfModAccessClass *ma_class = (JpfModAccessClass*)k_class;
	NmpAppModClass *am_class = (NmpAppModClass*)k_class;

	ma_class->io_close	= nmp_mod_ams_io_close;
	ma_class->io_init	= nmp_mod_ams_io_init;
	am_class->setup_mod	= nmp_mod_ams_setup;
}


//:~ End
