#include "nmp_mod_ivs.h"
#include "nmp_message.h"
#include "nmp_errno.h"
#include "nmp_memory.h"
#include "nmp_msg_ivs.h"
#include "nmp_internal_msg.h"
#include "nmp_shared.h"
#include "nmp_ivs_struct.h"


USING_MSG_ID_MAP(cms);

//static guint msg_seq_generator = 0;

static __inline__ gint
jpf_mod_ivs_ivs_register(JpfModIvs *self, JpfNetIO *io,  JpfMsgID msg_id,
    JpfIvsRegister *req, JpfIvsRegisterRes *res)
{
    gint ret;
    JpfID conflict;

    G_ASSERT(self != NULL && io != NULL && req != NULL && res != NULL);

    ret = jpf_mod_ivs_new_ivs(self, io,  req->ivs_id, &conflict);
    if (G_UNLIKELY(ret))
        return ret;

    ret = jpf_mod_ivs_sync_req(self, msg_id, req,
         sizeof(JpfIvsRegister), res, sizeof(JpfIvsRegisterRes));

    return ret;
}


JpfMsgFunRet
jpf_mod_ivs_register_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModIvs *self;
    JpfNetIO *io;
    JpfIvsRegister *req_info;
    JpfIvsRegisterRes res_info;
    JpfMsgID msg_id;
    JpfGuestBase *ivs_base;
    JpfIvs *ivs;
    JpfMsgIvsOnlineChange notify_info;
    gint ret;
    gchar *ivs_ip;
    JpfResourcesCap res_cap;

    self = (JpfModIvs*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_cap, 0, sizeof(res_cap));
    /*jpf_mod_get_resource_cap(&res_cap);
    if (!(res_cap.module_bits&MODULE_IVS_BIT))
    {
        ret = E_EXPIRED;
        goto ivs_register_err;
    }*/
    memset(&notify_info, 0, sizeof(notify_info));
    notify_info.ivs_id[IVS_ID_LEN - 1] = 0;
    strncpy(notify_info.ivs_id, req_info->ivs_id, IVS_ID_LEN - 1);
    memset(&res_info, 0, sizeof(res_info));
    ret = jpf_mod_ivs_ivs_register(self, io, msg_id, req_info, &res_info);
 //ivs_register_err:
    if (ret)
    {
        jpf_print(
            "<JpfModIvs> ivs:%s register failed, err:%d",
            req_info->ivs_id, -ret
        );

        SET_CODE(&res_info, -ret);
        MSG_SET_RESPONSE(msg);
        jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

        jpf_app_obj_deliver_in((JpfAppObj*)self, msg);
        jpf_mod_acc_release_io((JpfModAccess*)self, io);
        jpf_mod_container_del_io(self->container, io);

        return MFR_ACCEPTED;
    }

    ivs_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!ivs_base))
    {
        ret = -E_NOSUCHGUEST;
	 jpf_warning("<JpfModIvs> Can't find ivs:'%s' in container, io timeout?", notify_info.ivs_id);
    }
    else
   {
        ivs = (JpfIvs *)ivs_base;
        ivs->ivs_state = STAT_IVS_ONLINE;
        jpf_mods_container_put_guest(self->container, ivs_base);

        jpf_print(
             "<JpfModIvs> ivs:%s register ok",
             req_info->ivs_id
         );

        strncpy(res_info.domain_id, jpf_get_local_domain_id(), DOMAIN_ID_LEN - 1);

        jpf_check_keepalive_time(&res_info.keep_alive_time);
        jpf_net_io_set_ttd(io, res_info.keep_alive_time*1000*TIMEOUT_N_PERIODS);

        ivs_ip = jpf_net_get_io_peer_name(io);
        if (G_LIKELY(ivs_ip))
        {
             strncpy(notify_info.ivs_ip, ivs_ip, MAX_IP_LEN - 1);
             g_free(ivs_ip);
        }
        notify_info.new_status = 1;
        jpf_mod_ivs_change_ivs_online_status(app_obj, notify_info);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
    return MFR_DELIVER_BACK;

}


JpfMsgFunRet
jpf_mod_ivs_heart_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModIvs *self;
    JpfNetIO *io;
    JpfGuestBase *ivs_base;
    JpfIvsHeart *req_info;
    JpfIvsHeartRes res_info;
    JpfMsgID msg_id;
    gint ret = 0;
    JpfResourcesCap res_cap;

    self = (JpfModIvs*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_cap, 0, sizeof(res_cap));
    /*jpf_mod_get_resource_cap(&res_cap);
    if (!(res_cap.module_bits&MODULE_IVS_BIT))
    {
        jpf_app_obj_deliver_in((JpfAppObj*)self, msg);
        jpf_mod_acc_release_io((JpfModAccess*)self, io);
        jpf_mod_container_del_io(self->container, io);
        return MFR_ACCEPTED;
    }*/
    memset(&res_info, 0, sizeof(res_info));
    ivs_base = jpf_mods_container_get_guest_2(self->container, req_info->ivs_id);
    if (G_UNLIKELY(!ivs_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModIvs> ivsId:%s No such guest.", req_info->ivs_id);
    }
    else
    {
        jpf_get_utc_time(res_info.server_time);
        jpf_mods_container_put_guest(self->container, ivs_base);
    }

    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_ivs_backward(JpfModIvs *self, JpfSysMsg *msg, const gchar *id_str)
{
    JpfGuestBase *ivs_base;
    JpfNetIO *io;
    gint msg_id;

    msg_id = MSG_GETID(msg);
    ivs_base = jpf_mods_container_get_guest_2(self->container, id_str);
    if (G_UNLIKELY(!ivs_base))
    {
        jpf_warning("<JpfModIvs> deliver msg '%s' failed, IvsId:%s no such ivs.",
            MESSAGE_ID_TO_STR(cms, msg_id), id_str);
        jpf_sysmsg_destroy(msg);

        return MFR_ACCEPTED;
    }

    io = IO_OF_GUEST(ivs_base);
    BUG_ON(!io);

    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_attach_io(msg, io);
    jpf_mods_container_put_guest(self->container, ivs_base);

    return MFR_DELIVER_AHEAD;
}


JpfMsgFunRet
jpf_mod_ivs_backward_2(JpfModIvs *self, JpfSysMsg *msg, const gchar *id_str, const gchar *session_id)
{
    JpfGuestBase *ivs_base;
    JpfNetIO *io;
    gint msg_id;
    JpfErrRes      code;

    memset(&code, 0, sizeof(code));
    msg_id = MSG_GETID(msg);
    ivs_base = jpf_mods_container_get_guest_2(self->container, id_str);
    if (G_UNLIKELY(!ivs_base))
    {
        jpf_warning("<JpfModIvs> deliver msg '%s' failed, IvsId:%s no such ivs.",
            MESSAGE_ID_TO_STR(cms, msg_id), id_str);
        //jpf_sysmsg_destroy(msg);


        SET_CODE(&code.code, E_NOSUCHGUEST);
	if (session_id)
            memcpy(code.session, session_id, USER_NAME_LEN - 1);

        MSG_SET_DSTPOS(msg, BUSSLOT_POS_CU);
        jpf_sysmsg_set_private_2(msg, &code, sizeof(code));
        MSG_SET_RESPONSE(msg);
        return MFR_DELIVER_BACK;
    }

    io = IO_OF_GUEST(ivs_base);
    BUG_ON(!io);

    jpf_sysmsg_attach_io(msg, io);
    jpf_mods_container_put_guest(self->container, ivs_base);

    return MFR_DELIVER_AHEAD;
}


gint
jpf_mod_ivs_forward(JpfModIvs *self, JpfSysMsg *msg)
{
    JpfGuestBase *mss_base;
    JpfNetIO *io;

    io = MSG_IO(msg);
    BUG_ON(!io);

    mss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!mss_base))
        return -E_NOSUCHGUEST;

    MSG_SET_DSTPOS(msg, BUSSLOT_POS_CU);
    jpf_mods_container_put_guest(self->container, mss_base);

    return 0;
}


void
jpf_mod_ivs_register_msg_handler(JpfModIvs *self)
{
    JpfAppMod *super_self = (JpfAppMod*)self;

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_IVS_REGISTER,
        jpf_mod_ivs_register_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_IVS_HEART,
        jpf_mod_ivs_heart_f,
        NULL,
        0
    );

}

