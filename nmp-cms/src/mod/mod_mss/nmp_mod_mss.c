#include "nmp_mod_mss.h"
#include "nmp_ports.h"
#include "nmp_errno.h"
#include "nmp_mss_struct.h"
#include "message/nmp_msg_mss.h"
#include "nmp_debug.h"
#include "nmp_message.h"
#include "nmp_memory.h"
#include "nmp_proto.h"
#include "nmp_internal_msg.h"


USING_MSG_ID_MAP(cms);

G_DEFINE_TYPE(JpfModMss, nmp_mod_mss, NMP_TYPE_MODACCESS);

void
nmp_mod_mss_register_msg_handler(JpfModMss *self);


#define NMP_MSS_CHECK_TIME			(60 * 1000)	//60s


static __inline__ void
nmp_mod_mss_struct_init(JpfMss *mss)
{
	mss->mss_state = STAT_MSS_REGISTERING;
}


void
nmp_mod_mss_change_mss_online_status(NmpAppObj *app_obj,
    JpfMsgMssOnlineChange notify_info)
{
    NmpSysMsg *msg_notify;

    msg_notify = jpf_sysmsg_new_2(MSG_MSS_ONLINE_CHANGE,
		&notify_info, sizeof(notify_info), ++msg_seq_generator);
    if (G_UNLIKELY(!msg_notify))
        return;

    MSG_SET_DSTPOS(msg_notify, BUSSLOT_POS_DBS);
    nmp_app_obj_deliver_out(app_obj, msg_notify);
}


static void
nmp_mod_mss_destroy(JpfGuestBase *obj, gpointer priv_data)
{
    G_ASSERT(obj != NULL);

    NmpAppObj *self = NMP_APPOBJ(priv_data);
    JpfMsgMssOnlineChange notify_info;
    JpfMss *mss = NULL;

    mss = (JpfMss *)obj;
    if (mss->mss_state == STAT_MSS_ONLINE)
    {
	    memset(&notify_info, 0, sizeof(notify_info));
	    strncpy(notify_info.mss_id, ID_OF_GUEST(obj), MSS_ID_LEN - 1);
	    notify_info.new_status = 0;
	    nmp_mod_mss_change_mss_online_status(self, notify_info);
    }
}



gint
nmp_mod_mss_new_mss(JpfModMss *self, JpfNetIO *io, const gchar *id,
	JpfID *conflict)
{
	JpfGuestBase *mss;
	gint ret;
	G_ASSERT(self != NULL && io != NULL);

	mss = jpf_mods_guest_new(sizeof(JpfMss), id, nmp_mod_mss_destroy, self);
	if (G_UNLIKELY(!mss))
		return -E_NOMEM;

	nmp_mod_mss_struct_init((JpfMss*)mss);
	jpf_mods_guest_attach_io(mss, io);
	ret = jpf_mods_container_add_guest(self->container,
		mss, conflict);
	if (G_UNLIKELY(ret))
	{
		jpf_print(
			"<JpfModMds> mss:%s register failed, err:%d", id, ret
		);
	}

	jpf_mods_guest_unref(mss);
	return ret;
}


gint
nmp_mod_mss_sync_req(JpfModMss *self, NmpMsgID msg_id,
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
			"<JpfModMss> request cmd %d failed!", msg_id
		);
		jpf_sysmsg_destroy(msg);
		return err;
	}

	if (G_UNLIKELY(!msg))	/* sent, but no response */
	{
		jpf_warning(
			"<JpfModMss> request cmd %d timeout!", msg_id
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
nmp_mod_mss_sync_req_2(JpfModMss *self, NmpMsgID msg_id,
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
        	"<JpfModMss> request cmd %d failed!", msg_id
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
nmp_mod_mss_deliver_msg(JpfModMss *self, const char *mss_id, NmpSysMsg *msg)
{
    JpfGuestBase *mss_base;
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

    mss_base = jpf_mods_container_get_guest_2(self->container, mss_id);
    if (G_UNLIKELY(!mss_base))
    {
        jpf_warning("<JpfModMss> deliver msg '%s' failed, MssId:%s no such mss.",
            MESSAGE_ID_TO_STR(cms, msg_id), mss_id);
        jpf_sysmsg_destroy(msg);

        //return MFR_ACCEPTED;
    }
    jpf_sysmsg_attach_io(msg_copy, IO_OF_GUEST(mss_base));
    nmp_app_obj_deliver_in((NmpAppObj*)self, msg_copy);
    nmp_mod_acc_release_io((JpfModAccess*)self,  IO_OF_GUEST(mss_base));
    nmp_mod_container_del_io(self->container,  IO_OF_GUEST(mss_base));
    jpf_mods_container_put_guest(self->container, mss_base);
}

static void
nmp_mod_mss_io_close(JpfModAccess *s_self, JpfNetIO *io, gint err)
{
	gint ret;
	JpfModMss *self;
	G_ASSERT(s_self != NULL);

	self = (JpfModMss*)s_self;

	ret = nmp_mod_container_del_io(self->container, io);
	if (G_UNLIKELY(!ret))
	{
		jpf_print(
			"<JpfModMss> unrecognized connection closed, err:%d.",
			err
		);
	}
}


static gint
nmp_mod_mss_io_init(JpfModAccess *s_self, JpfNetIO *io)
{
	gint err;
	JpfModMss *self;
	G_ASSERT(s_self != NULL);

	self = (JpfModMss*)s_self;

	err = nmp_mod_container_add_io(self->container, io);
	if (err)
	{
		jpf_error(
			"<JpfModMss> insert io to temp list err: %d!",
			err
		);
		return err;
	}

	jpf_net_unref_io(io);
	return 0;
}


gint
nmp_mod_mss_setup(NmpAppMod *am_self)
{
	gint err;
	JpfModAccess *ma_self;
	JpfModMss *self;
	struct sockaddr_in sin;
	G_ASSERT(am_self != NULL);

	self = (JpfModMss*)am_self;
	ma_self = (JpfModAccess*)am_self;

	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(JPFCMS_MSS_PORT);
	sin.sin_addr.s_addr = INADDR_ANY;

	nmp_mod_acc_init_net(ma_self, &jxj_packet_proto, &jxj_xml_proto);

	self->listen_io = nmp_mod_acc_create_listen_io(
		ma_self, (struct sockaddr*)&sin, &err
	);
	if (!self->listen_io)
	{
		jpf_error("<JpfModMss> create listen io failed!");
		return err;
	}

	jpf_net_set_heavy_io_load(self->listen_io);
	nmp_app_mod_set_name(am_self, "MOD-MSS");
	nmp_mod_mss_register_msg_handler(self);

	return 0;
}


static void
nmp_mod_mss_check_mss_state(NmpAppObj *app_obj)
{
	NmpSysMsg *msg;

	msg = jpf_sysmsg_new_2(MSG_CHECK_MSS_STATE,
		NULL, 0, ++msg_seq_generator);
	if (G_UNLIKELY(!msg))
		return;

	MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
	nmp_app_obj_deliver_out(app_obj, msg);
}


static gboolean
nmp_mod_mss_timer(gpointer user_data)
{
	JpfModMss * self = (JpfModMss *)user_data;

	nmp_mod_mss_check_mss_state((NmpAppObj *)self);

	return TRUE;
}


static void
nmp_mod_mss_init(JpfModMss *self)
{
	self->container = jpf_mods_container_new(
		self,
       jpf_get_sys_parm_int(SYS_PARM_WAIT_AFTER_CONNECTED)
	);

	if (G_UNLIKELY(!self->container))
	{
		jpf_error("<JpfModMss> alloc guest container failed!");
		FATAL_ERROR_EXIT;
	}

	self->listen_io = NULL;
	jpf_set_timer(NMP_MSS_CHECK_TIME, nmp_mod_mss_timer, self);
}


static void
nmp_mod_mss_class_init(JpfModMssClass *k_class)
{
	JpfModAccessClass *ma_class = (JpfModAccessClass*)k_class;
	NmpAppModClass *am_class = (NmpAppModClass*)k_class;

	ma_class->io_close	= nmp_mod_mss_io_close;
	ma_class->io_init	= nmp_mod_mss_io_init;
	am_class->setup_mod	= nmp_mod_mss_setup;
}


//:~ End
