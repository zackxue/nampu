#include "nmp_mod_mds.h"
#include "nmp_ports.h"
#include "nmp_errno.h"
#include "nmp_mds_struct.h"
#include "message/nmp_msg_mds.h"
#include "nmp_debug.h"
#include "nmp_message.h"
#include "nmp_memory.h"
#include "nmp_proto.h"
#include "nmp_internal_msg.h"


G_DEFINE_TYPE(JpfModMds, nmp_mod_mds, NMP_TYPE_MODACCESS);

//static guint msg_seq_generator = 0;

void
nmp_mod_mds_register_msg_handler(JpfModMds *self);


void
nmp_mod_mds_change_mds_online_status(NmpAppObj *app_obj,
    JpfMsgMdsOnlineChange notify_info)
{
    NmpSysMsg *msg_notify;

    msg_notify = jpf_sysmsg_new_2(MSG_MDS_ONLINE_CHANGE,
		&notify_info, sizeof(notify_info), ++msg_seq_generator);
    if (G_UNLIKELY(!msg_notify))
        return;

    MSG_SET_DSTPOS(msg_notify, BUSSLOT_POS_DBS);
    nmp_app_obj_deliver_out(app_obj, msg_notify);
}


static __inline__ void
nmp_mod_mds_struct_init(JpfMds *mds)
{
	mds->mds_state = STAT_MDS_REGISTERING;
}

static void
nmp_mod_mds_destroy(JpfGuestBase *obj, gpointer priv_data)
{
    G_ASSERT(obj != NULL);

    NmpAppObj *self = NMP_APPOBJ(priv_data);
    JpfMsgMdsOnlineChange notify_info;
    JpfMds *mds = NULL;

    mds = (JpfMds *)obj;
    if (mds->mds_state == STAT_MDS_ONLINE)
    {
	    memset(&notify_info, 0, sizeof(notify_info));
	    strncpy(notify_info.mds_id, ID_OF_GUEST(obj), MDS_ID_LEN - 1);
	    notify_info.new_status = 0;
	    nmp_mod_mds_change_mds_online_status(self, notify_info);
    }
}


gint
nmp_mod_mds_new_mds(JpfModMds *self, JpfNetIO *io, const gchar *id,
	JpfID *conflict)
{
	JpfGuestBase *mds;
	gint ret;
	G_ASSERT(self != NULL && io != NULL);

	mds = jpf_mods_guest_new(sizeof(JpfMds), id, nmp_mod_mds_destroy, self);
	if (G_UNLIKELY(!mds))
		return -E_NOMEM;

	nmp_mod_mds_struct_init((JpfMds*)mds);
	jpf_mods_guest_attach_io(mds, io);
	ret = jpf_mods_container_add_guest(self->container,
		mds, conflict);
	if (G_UNLIKELY(ret))
	{
		jpf_print(
			"<JpfModMds> mds:%s register failed, err:%d", id, ret
		);
	}

	jpf_mods_guest_unref(mds);
	return ret;
}


gint
nmp_mod_mds_sync_req(JpfModMds *self, NmpMsgID msg_id,
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

	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (G_UNLIKELY(res))
		memcpy(res, MSG_GET_DATA(msg), res_size);

	err = RES_CODE(res_info);
	jpf_sysmsg_destroy(msg);

	return err;
}


gpointer
nmp_mod_mds_sync_req_2(JpfModMds *self, NmpMsgID msg_id,
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
        	"<JpfModMds> request cmd %d failed!", msg_id
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


static void
nmp_mod_mds_io_close(JpfModAccess *s_self, JpfNetIO *io, gint err)
{
	gint ret;
	JpfModMds *self;
	G_ASSERT(s_self != NULL);

	self = (JpfModMds*)s_self;

	ret = nmp_mod_container_del_io(self->container, io);
	if (G_UNLIKELY(!ret))
	{
		jpf_print(
			"<JpfModMds> unrecognized connection closed, err:%d.",
			err
		);
	}
}


static gint
nmp_mod_mds_io_init(JpfModAccess *s_self, JpfNetIO *io)
{
	gint err;
	JpfModMds *self;
	G_ASSERT(s_self != NULL);

	self = (JpfModMds*)s_self;

	err = nmp_mod_container_add_io(self->container, io);
	if (err)
	{
		jpf_error(
			"<JpfModMds> insert io to temp list err: %d!",
			err
		);
		return err;
	}

	jpf_net_unref_io(io);
	return 0;
}


gint
nmp_mod_mds_setup(NmpAppMod *am_self)
{
	gint err;
	JpfModAccess *ma_self;
	JpfModMds *self;
	struct sockaddr_in sin;
	G_ASSERT(am_self != NULL);

	self = (JpfModMds*)am_self;
	ma_self = (JpfModAccess*)am_self;

	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(JPFCMS_MDU_PORT);
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

	jpf_net_set_heavy_io_load(self->listen_io);
	nmp_app_mod_set_name(am_self, "MOD-MDU");
	nmp_mod_mds_register_msg_handler(self);

	return 0;
}


static void
nmp_mod_mds_init(JpfModMds *self)
{
	self->container = jpf_mods_container_new(
		self,
       jpf_get_sys_parm_int(SYS_PARM_WAIT_AFTER_CONNECTED)
	);

	if (G_UNLIKELY(!self->container))
	{
		jpf_error("<JpfModMds> alloc guest container failed!");
		FATAL_ERROR_EXIT;
	}

	//INIT_LIST_HEAD(&self->list_user);
	//g_static_mutex_init(&self->list_ulock);
    self->listen_io = NULL;
}


static void
nmp_mod_mds_class_init(JpfModMdsClass *k_class)
{
	JpfModAccessClass *ma_class = (JpfModAccessClass*)k_class;
	NmpAppModClass *am_class = (NmpAppModClass*)k_class;

	ma_class->io_close	= nmp_mod_mds_io_close;
	ma_class->io_init	= nmp_mod_mds_io_init;
	am_class->setup_mod	= nmp_mod_mds_setup;
}


//:~ End




