#include "nmp_mod_mss.h"
#include "nmp_ports.h"
#include "nmp_share_errno.h"
#include "nmp_mss_struct.h"
#include "message/nmp_msg_mss.h"
#include "nmp_share_debug.h"
#include "nmp_message.h"
#include "nmp_memory.h"
#include "nmp_proto.h"
#include "nmp_internal_msg.h"


USING_MSG_ID_MAP(cms);

G_DEFINE_TYPE(NmpModMss, nmp_mod_mss, NMP_TYPE_MODACCESS);

void
nmp_mod_mss_register_msg_handler(NmpModMss *self);


#define NMP_MSS_CHECK_TIME			(60 * 1000)	//60s


static __inline__ void
nmp_mod_mss_struct_init(NmpMss *mss)
{
	mss->mss_state = STAT_MSS_REGISTERING;
}


void
nmp_mod_mss_change_mss_online_status(NmpAppObj *app_obj,
    NmpMsgMssOnlineChange notify_info)
{
    NmpSysMsg *msg_notify;

    msg_notify = nmp_sysmsg_new_2(MSG_MSS_ONLINE_CHANGE,
		&notify_info, sizeof(notify_info), ++msg_seq_generator);
    if (G_UNLIKELY(!msg_notify))
        return;

    MSG_SET_DSTPOS(msg_notify, BUSSLOT_POS_DBS);
    nmp_app_obj_deliver_out(app_obj, msg_notify);
}


static void
nmp_mod_mss_destroy(NmpGuestBase *obj, gpointer priv_data)
{
    G_ASSERT(obj != NULL);

    NmpAppObj *self = NMP_APPOBJ(priv_data);
    NmpMsgMssOnlineChange notify_info;
    NmpMss *mss = NULL;

    mss = (NmpMss *)obj;
    if (mss->mss_state == STAT_MSS_ONLINE)
    {
	    memset(&notify_info, 0, sizeof(notify_info));
	    strncpy(notify_info.mss_id, ID_OF_GUEST(obj), MSS_ID_LEN - 1);
	    notify_info.new_status = 0;
	    nmp_mod_mss_change_mss_online_status(self, notify_info);
    }
}



gint
nmp_mod_mss_new_mss(NmpModMss *self, NmpNetIO *io, const gchar *id,
	NmpID *conflict)
{
	NmpGuestBase *mss;
	gint ret;
	G_ASSERT(self != NULL && io != NULL);

	mss = nmp_mods_guest_new(sizeof(NmpMss), id, nmp_mod_mss_destroy, self);
	if (G_UNLIKELY(!mss))
		return -E_NOMEM;

	nmp_mod_mss_struct_init((NmpMss*)mss);
	nmp_mods_guest_attach_io(mss, io);
	ret = nmp_mods_container_add_guest(self->container,
		mss, conflict);
	if (G_UNLIKELY(ret))
	{
		nmp_print(
			"<NmpModMds> mss:%s register failed, err:%d", id, ret
		);
	}

	nmp_mods_guest_unref(mss);
	return ret;
}


gint
nmp_mod_mss_sync_req(NmpModMss *self, NmpMsgID msg_id,
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
			"<NmpModMss> request cmd %d failed!", msg_id
		);
		nmp_sysmsg_destroy(msg);
		return err;
	}

	if (G_UNLIKELY(!msg))	/* sent, but no response */
	{
		nmp_warning(
			"<NmpModMss> request cmd %d timeout!", msg_id
		);
		return -E_TIMEOUT;
	}

	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (G_UNLIKELY(res))
		memcpy(res, MSG_GET_DATA(msg), res_size);

	err = RES_CODE(res_info);
	nmp_sysmsg_destroy(msg);

	return err;
}


gpointer
nmp_mod_mss_sync_req_2(NmpModMss *self, NmpMsgID msg_id,
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
        	"<NmpModMss> request cmd %d failed!", msg_id
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
			"<NmpModMds> request cmd %d timeout!", msg_id
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


void
nmp_mod_mss_deliver_msg(NmpModMss *self, const char *mss_id, NmpSysMsg *msg)
{
    NmpGuestBase *mss_base;
    gint msg_id;
    NmpSysMsg *msg_copy = NULL;

    msg_id = MSG_GETID(msg);
    msg_copy = nmp_sysmsg_copy_one(msg);
    if (!msg_copy)
    {
        nmp_error(
        	"<NmpModCu> copy sys-msg failed while delivering msg to cu."
        );
        FATAL_ERROR_EXIT;
    }

    mss_base = nmp_mods_container_get_guest_2(self->container, mss_id);
    if (G_UNLIKELY(!mss_base))
    {
        nmp_warning("<NmpModMss> deliver msg '%s' failed, MssId:%s no such mss.",
            MESSAGE_ID_TO_STR(cms, msg_id), mss_id);
        nmp_sysmsg_destroy(msg);

        //return MFR_ACCEPTED;
    }
    nmp_sysmsg_attach_io(msg_copy, IO_OF_GUEST(mss_base));
    nmp_app_obj_deliver_in((NmpAppObj*)self, msg_copy);
    nmp_mod_acc_release_io((NmpModAccess*)self,  IO_OF_GUEST(mss_base));
    nmp_mod_container_del_io(self->container,  IO_OF_GUEST(mss_base));
    nmp_mods_container_put_guest(self->container, mss_base);
}

static void
nmp_mod_mss_io_close(NmpModAccess *s_self, NmpNetIO *io, gint err)
{
	gint ret;
	NmpModMss *self;
	G_ASSERT(s_self != NULL);

	self = (NmpModMss*)s_self;

	ret = nmp_mod_container_del_io(self->container, io);
	if (G_UNLIKELY(!ret))
	{
		nmp_print(
			"<NmpModMss> unrecognized connection closed, err:%d.",
			err
		);
	}
}


static gint
nmp_mod_mss_io_init(NmpModAccess *s_self, NmpNetIO *io)
{
	gint err;
	NmpModMss *self;
	G_ASSERT(s_self != NULL);

	self = (NmpModMss*)s_self;

	err = nmp_mod_container_add_io(self->container, io);
	if (err)
	{
		nmp_error(
			"<NmpModMss> insert io to temp list err: %d!",
			err
		);
		return err;
	}

	nmp_net_unref_io(io);
	return 0;
}


gint
nmp_mod_mss_setup(NmpAppMod *am_self)
{
	gint err;
	NmpModAccess *ma_self;
	NmpModMss *self;
	struct sockaddr_in sin;
	G_ASSERT(am_self != NULL);

	self = (NmpModMss*)am_self;
	ma_self = (NmpModAccess*)am_self;

	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(JPFCMS_MSS_PORT);
	sin.sin_addr.s_addr = INADDR_ANY;

	nmp_mod_acc_init_net(ma_self, &nmp_packet_proto, &nmp_xml_proto);

	self->listen_io = nmp_mod_acc_create_listen_io(
		ma_self, (struct sockaddr*)&sin, &err
	);
	if (!self->listen_io)
	{
		nmp_error("<NmpModMss> create listen io failed!");
		return err;
	}

	nmp_net_set_heavy_io_load(self->listen_io);
	nmp_app_mod_set_name(am_self, "MOD-MSS");
	nmp_mod_mss_register_msg_handler(self);

	return 0;
}


static void
nmp_mod_mss_check_mss_state(NmpAppObj *app_obj)
{
	NmpSysMsg *msg;

	msg = nmp_sysmsg_new_2(MSG_CHECK_MSS_STATE,
		NULL, 0, ++msg_seq_generator);
	if (G_UNLIKELY(!msg))
		return;

	MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
	nmp_app_obj_deliver_out(app_obj, msg);
}


static gboolean
nmp_mod_mss_timer(gpointer user_data)
{
	NmpModMss * self = (NmpModMss *)user_data;

	nmp_mod_mss_check_mss_state((NmpAppObj *)self);

	return TRUE;
}


static void
nmp_mod_mss_init(NmpModMss *self)
{
	self->container = nmp_mods_container_new(
		self,
       nmp_get_sys_parm_int(SYS_PARM_WAIT_AFTER_CONNECTED)
	);

	if (G_UNLIKELY(!self->container))
	{
		nmp_error("<NmpModMss> alloc guest container failed!");
		FATAL_ERROR_EXIT;
	}

	self->listen_io = NULL;
	nmp_set_timer(NMP_MSS_CHECK_TIME, nmp_mod_mss_timer, self);
}


static void
nmp_mod_mss_class_init(NmpModMssClass *k_class)
{
	NmpModAccessClass *ma_class = (NmpModAccessClass*)k_class;
	NmpAppModClass *am_class = (NmpAppModClass*)k_class;

	ma_class->io_close	= nmp_mod_mss_io_close;
	ma_class->io_init	= nmp_mod_mss_io_init;
	am_class->setup_mod	= nmp_mod_mss_setup;
}


//:~ End
