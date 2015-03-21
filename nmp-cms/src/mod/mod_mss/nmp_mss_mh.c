#include "nmp_mod_mss.h"
#include "nmp_message.h"
#include "nmp_errno.h"
#include "nmp_memory.h"
#include "nmp_msg_mss.h"
#include "nmp_internal_msg.h"
#include "nmp_shared.h"
#include "nmp_mss_struct.h"


USING_MSG_ID_MAP(cms);

//static guint msg_seq_generator = 0;

static __inline__ gint
nmp_mod_mss_mss_register(JpfModMss *self, JpfNetIO *io,  NmpMsgID msg_id,
    JpfMssRegister *req, JpfMssRegisterRes *res)
{
    gint ret;
    JpfID conflict;

    G_ASSERT(self != NULL && io != NULL && req != NULL && res != NULL);

    ret = nmp_mod_mss_new_mss(self, io,  req->mss_id, &conflict);
    if (G_UNLIKELY(ret))
        return ret;

    ret = nmp_mod_mss_sync_req(self, msg_id, req,
         sizeof(JpfMssRegister), res, sizeof(JpfMssRegisterRes));

    return ret;
}


NmpMsgFunRet
nmp_mod_mss_register_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModMss *self;
    JpfNetIO *io;
    JpfMssRegister *req_info;
    JpfMssRegisterRes res_info;
    NmpMsgID msg_id;
    JpfGuestBase *mss_base;
    JpfMss *mss;
    JpfMsgMssOnlineChange notify_info;
    gint ret;
    gchar *mss_ip;
    JpfResourcesCap res_cap;

    self = (JpfModMss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_cap, 0, sizeof(res_cap));
    nmp_mod_get_resource_cap(&res_cap);
    if (!(res_cap.module_bits&MODULE_MSS_BIT))
    {
        ret = E_EXPIRED;
        goto mss_register_err;
    }
    memset(&notify_info, 0, sizeof(notify_info));
    notify_info.mss_id[MSS_ID_LEN - 1] = 0;
    strncpy(notify_info.mss_id, req_info->mss_id, MSS_ID_LEN - 1);
    memset(&res_info, 0, sizeof(res_info));
    ret = nmp_mod_mss_mss_register(self, io, msg_id, req_info, &res_info);
 mss_register_err:
    if (ret)
    {
        jpf_print(
            "<JpfModMss> mss:%s register failed, err:%d",
            req_info->mss_id, -ret
        );

        SET_CODE(&res_info, -ret);
        MSG_SET_RESPONSE(msg);
        jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

        nmp_app_obj_deliver_in((NmpAppObj*)self, msg);
        nmp_mod_acc_release_io((JpfModAccess*)self, io);
        nmp_mod_container_del_io(self->container, io);

        return MFR_ACCEPTED;
    }

    mss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!mss_base))
    {
        ret = -E_NOSUCHGUEST;
	 jpf_warning("<JpfModMss> Can't find mss:'%s' in container, io timeout?", notify_info.mss_id);
    }
    else
   {
        mss = (JpfMss *)mss_base;
        mss->mss_state = STAT_MSS_ONLINE;
        jpf_mods_container_put_guest(self->container, mss_base);

        jpf_print(
             "<JpfModMss> mss:%s register ok",
             req_info->mss_id
         );

        strncpy(res_info.domain_id, jpf_get_local_domain_id(), DOMAIN_ID_LEN - 1);

        jpf_check_keepalive_time(&res_info.keep_alive_time);
        jpf_net_io_set_ttd(io, res_info.keep_alive_time*1000*TIMEOUT_N_PERIODS);

        mss_ip = jpf_net_get_io_peer_name(io);
        if (G_LIKELY(mss_ip))
        {
             strncpy(notify_info.mss_ip, mss_ip, MAX_IP_LEN - 1);
             g_free(mss_ip);
        }
        notify_info.new_status = 1;
        nmp_mod_mss_change_mss_online_status(app_obj, notify_info);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
    return MFR_DELIVER_BACK;

}


NmpMsgFunRet
nmp_mod_mss_heart_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModMss *self;
    JpfNetIO *io;
    JpfGuestBase *mss_base;
    JpfMssHeart *req_info;
    JpfMssHeartRes res_info;
    NmpMsgID msg_id;
    gint ret = 0;
    JpfResourcesCap res_cap;

    self = (JpfModMss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_cap, 0, sizeof(res_cap));
    nmp_mod_get_resource_cap(&res_cap);
    if (!(res_cap.module_bits&MODULE_MSS_BIT))
    {
        nmp_app_obj_deliver_in((NmpAppObj*)self, msg);
        nmp_mod_acc_release_io((JpfModAccess*)self, io);
        nmp_mod_container_del_io(self->container, io);
        return MFR_ACCEPTED;
    }
    memset(&res_info, 0, sizeof(res_info));
    mss_base = jpf_mods_container_get_guest_2(self->container, req_info->mss_id);
    if (G_UNLIKELY(!mss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModMss> mssId:%s No such guest.", req_info->mss_id);
    }
    else
    {
        jpf_get_utc_time(res_info.server_time);
        jpf_mods_container_put_guest(self->container, mss_base);
    }

    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_mss_get_guid_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    JpfModMss *self;
    JpfNetIO *io;
    JpfGuestBase *mss_base;
    JpfMssGetGuid *req_info;
    JpfMssGetGuidRes res_info;
    NmpMsgID msg_id;
    gint ret = 0;

    self = (JpfModMss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    mss_base = jpf_mods_container_get_guest_2(self->container, req_info->mss_id);
    if (G_UNLIKELY(!mss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModMss> mssId:%s No such guest.", req_info->mss_id);
    }
    else
    {
        MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
        jpf_mods_container_put_guest(self->container, mss_base);
	  return MFR_DELIVER_AHEAD;
    }

    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_mss_backward(JpfModMss *self, NmpSysMsg *msg, const gchar *id_str)
{
    JpfGuestBase *mss_base;
    JpfNetIO *io;
    gint msg_id;

    msg_id = MSG_GETID(msg);
    mss_base = jpf_mods_container_get_guest_2(self->container, id_str);
    if (G_UNLIKELY(!mss_base))
    {
        jpf_warning("<JpfModMss> deliver msg '%s' failed, MssId:%s no such mss.",
            MESSAGE_ID_TO_STR(cms, msg_id), id_str);
        jpf_sysmsg_destroy(msg);

        return MFR_ACCEPTED;
    }

    io = IO_OF_GUEST(mss_base);
    BUG_ON(!io);

    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_attach_io(msg, io);
    jpf_mods_container_put_guest(self->container, mss_base);

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_backward_2(JpfModMss *self, NmpSysMsg *msg, const gchar *id_str, const gchar *session_id)
{
    JpfGuestBase *mss_base;
    JpfNetIO *io;
    gint msg_id;
    JpfErrRes      code;

    memset(&code, 0, sizeof(code));
    msg_id = MSG_GETID(msg);
    mss_base = jpf_mods_container_get_guest_2(self->container, id_str);
    if (G_UNLIKELY(!mss_base))
    {
        jpf_warning("<JpfModMss> deliver msg '%s' failed, MssId:%s no such mss.",
            MESSAGE_ID_TO_STR(cms, msg_id), id_str);
        //jpf_sysmsg_destroy(msg);


        SET_CODE(&code.code, E_NOSUCHGUEST);
	if (session_id)
            memcpy(code.session, session_id, USER_NAME_LEN - 1);

        MSG_SET_DSTPOS(msg, BUSSLOT_POS_BSS);
        jpf_sysmsg_set_private_2(msg, &code, sizeof(code));
        MSG_SET_RESPONSE(msg);
        return MFR_DELIVER_BACK;
    }

    io = IO_OF_GUEST(mss_base);
    BUG_ON(!io);

    jpf_sysmsg_attach_io(msg, io);
    jpf_mods_container_put_guest(self->container, mss_base);

    return MFR_DELIVER_AHEAD;
}



NmpMsgFunRet
nmp_mod_mss_get_guid_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModMss *self;
    JpfMssGetGuidRes *res_info;

    self = (JpfModMss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_mss_backward(self, msg, res_info->mss_id);
}


NmpMsgFunRet
nmp_mod_mss_get_record_policy_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    JpfModMss *self;
    JpfNetIO *io;
    JpfGuestBase *mss_base;
    JpfMssGetRecordPolicy *req_info;
    JpfMssGetRecordPolicyRes res_info;
    NmpMsgID msg_id;
    gint ret = 0;

    self = (JpfModMss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    mss_base = jpf_mods_container_get_guest_2(self->container, req_info->mss_id);
    if (G_UNLIKELY(!mss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModMss> mssId:%s No such guest.", req_info->mss_id);
    }
    else
    {
        MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
        jpf_mods_container_put_guest(self->container, mss_base);
	  return MFR_DELIVER_AHEAD;
    }

    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_mss_get_record_policy_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModMss *self;
    JpfMssGetRecordPolicyRes *res_info;

    self = (JpfModMss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_mss_backward(self, msg, res_info->mss_id);
}


NmpMsgFunRet
nmp_mod_mss_notify_policy_change_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfNotifyPolicyChange *res_info = NULL;
    JpfModMss *self;
    JpfGuestBase *mss_base;
    JpfNetIO *io;
    gint msg_id;

    self = NMP_MODMSS(app_obj);
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    msg_id = MSG_GETID(msg);
    mss_base = jpf_mods_container_get_guest_2(self->container, res_info->mss_id);
    if (G_UNLIKELY(!mss_base))
    {
        jpf_warning("<JpfModMss> deliver msg '%s' failed, MssId:%s no such mss.",
            MESSAGE_ID_TO_STR(cms, msg_id), res_info->mss_id);
        jpf_sysmsg_destroy(msg);

        return MFR_ACCEPTED;
    }

    io = IO_OF_GUEST(mss_base);
    BUG_ON(!io);

    jpf_sysmsg_attach_io(msg, io);
    jpf_mods_container_put_guest(self->container, mss_base);

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_gu_list_change_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModMss *self;
    JpfGuestBase *mss_base;
    gint msg_id;
    JpfMssId *req_info = NULL;

    self = NMP_MODMSS(app_obj);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    msg_id = MSG_GETID(msg);
    mss_base = jpf_mods_container_get_guest_2(self->container, req_info->mss_id);
    if (G_UNLIKELY(!mss_base))
    {
        jpf_warning("<JpfModMss> deliver msg '%s' failed, MssId:%s no such mss.",
            MESSAGE_ID_TO_STR(cms, msg_id), req_info->mss_id);
        jpf_sysmsg_destroy(msg);

        return MFR_ACCEPTED;
    }
    jpf_sysmsg_attach_io(msg, IO_OF_GUEST(mss_base));
    jpf_mods_container_put_guest(self->container, mss_base);

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_get_route_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    JpfModMss *self;
    JpfNetIO *io;
    JpfGuestBase *mss_base;
    JpfMssGetRoute *req_info;
    JpfMssGetRouteRes res_info;
    NmpMsgID msg_id;
    gint ret = 0;

    self = (JpfModMss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    mss_base = jpf_mods_container_get_guest_2(self->container, req_info->mss_id);
    if (G_UNLIKELY(!mss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModMss> mssId:%s No such guest.", req_info->mss_id);
    }
    else
    {
        MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
        jpf_mods_container_put_guest(self->container, mss_base);
	  return MFR_DELIVER_AHEAD;
    }

    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_mss_get_route_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModMss *self;
    JpfMssGetRouteRes *res_info;

    self = (JpfModMss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_mss_backward(self, msg, res_info->mss_id);
}


NmpMsgFunRet
nmp_mod_mss_get_mds_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    JpfModMss *self;
    JpfNetIO *io;
    JpfGuestBase *mss_base;
    JpfMssGetMds *req_info;
    JpfMssGetMdsRes res_info;
    NmpMsgID msg_id;
    gint ret = 0;

    self = (JpfModMss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    mss_base = jpf_mods_container_get_guest_2(self->container, req_info->mss_id);
    if (G_UNLIKELY(!mss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModMss> mssId:%s No such guest.", req_info->mss_id);
    }
    else
    {
        MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
        jpf_mods_container_put_guest(self->container, mss_base);
	  return MFR_DELIVER_AHEAD;
    }

    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_mss_get_mds_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModMss *self;
    JpfMssGetMdsRes *res_info;

    self = (JpfModMss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_mss_backward(self, msg, res_info->mss_id);
}


NmpMsgFunRet
nmp_mod_mss_get_mds_ip_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    JpfModMss *self;
    JpfNetIO *io;
    JpfGuestBase *mss_base;
    JpfMssGetMdsIp *req_info;
    JpfMssGetMdsIpRes res_info;
    NmpMsgID msg_id;
    gint ret = 0;

    self = (JpfModMss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    mss_base = jpf_mods_container_get_guest_2(self->container, req_info->mss_id);
    if (G_UNLIKELY(!mss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModMss> mssId:%s No such guest.", req_info->mss_id);
    }
    else
    {
        MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
        jpf_mods_container_put_guest(self->container, mss_base);
	  return MFR_DELIVER_AHEAD;
    }

    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_mss_get_mds_ip_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModMss *self;
    JpfMssGetMdsIpRes *res_info;

    self = (JpfModMss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_mss_backward(self, msg, res_info->mss_id);
}

gint
nmp_mod_mss_forward(JpfModMss *self, NmpSysMsg *msg)
{
    JpfGuestBase *mss_base;
    JpfNetIO *io;

    io = MSG_IO(msg);
    BUG_ON(!io);

    mss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!mss_base))
        return -E_NOSUCHGUEST;

    MSG_SET_DSTPOS(msg, BUSSLOT_POS_BSS);
    jpf_mods_container_put_guest(self->container, mss_base);

    return 0;
}


NmpMsgFunRet
nmp_mod_mss_add_hd_group_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    JpfModMss *self;
    JpfAddHdGroupRes *res_info;
    gint ret = 0;

    self = (JpfModMss*)app_obj;

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_mss_forward(self, msg);
    if (ret)
    {
        SET_CODE(res_info, -ret);
        jpf_warning("<JpfModMss> No such guest.");
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_add_hd_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModMss *self;
    JpfAddHdGroup *req_info;

    self = (JpfModMss*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_mss_backward_2(self, msg, req_info->mss_id, req_info->session);
}


NmpMsgFunRet
nmp_mod_mss_add_hd_to_group_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    JpfModMss *self;
    JpfAddHdToGroupRes *res_info;
    gint ret = 0;

    self = (JpfModMss*)app_obj;

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_mss_forward(self, msg);
    if (ret)
    {
        SET_CODE(res_info, -ret);
        jpf_warning("<JpfModMss> No such guest.");
	 jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_add_hd_to_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModMss *self;
    JpfAddHdToGroup *req_info;

    self = (JpfModMss*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_mss_backward_2(self, msg, req_info->mss_id, req_info->session);
}


NmpMsgFunRet
nmp_mod_mss_del_hd_from_group_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    JpfModMss *self;
    JpfDelHdFromGroupRes *res_info;
    gint ret = 0;

    self = (JpfModMss*)app_obj;

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_mss_forward(self, msg);
    if (ret)
    {
        SET_CODE(res_info, -ret);
        jpf_warning("<JpfModMss> No such guest.");
	 jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_del_hd_from_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModMss *self;
    JpfDelHdFromGroup *req_info;

    self = (JpfModMss*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_mss_backward_2(self, msg, req_info->mss_id, req_info->session);
}


NmpMsgFunRet
nmp_mod_mss_reboot_mss_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    JpfModMss *self;
    JpfRebootMssRes *res_info;
    gint ret = 0;

    self = (JpfModMss*)app_obj;

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_mss_forward(self, msg);
    if (ret)
    {
        SET_CODE(res_info, -ret);
        jpf_warning("<JpfModMss> No such guest.");
	 jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_reboot_mss_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModMss *self;
    JpfRebootMss *req_info;

    self = (JpfModMss*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_mss_backward_2(self, msg, req_info->mss_id, req_info->session);
}


NmpMsgFunRet
nmp_mod_mss_query_all_hd_group_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    JpfModMss *self;
    JpfQueryAllHdGroupRes *res_info;
    gint ret = 0;

    self = (JpfModMss*)app_obj;

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_mss_forward(self, msg);
    if (ret)
    {
        SET_CODE(res_info, -ret);
        jpf_warning("<JpfModMss> No such guest.");
	 jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_query_all_hd_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModMss *self;
    JpfQueryAllHdGroup *req_info;

    self = (JpfModMss*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_mss_backward_2(self, msg, req_info->mss_id, req_info->session);
}


NmpMsgFunRet
nmp_mod_mss_query_hd_group_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    JpfModMss *self;
    JpfQueryHdGroupInfoRes *res_info;
    gint ret = 0;

    self = (JpfModMss*)app_obj;

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_mss_forward(self, msg);
    if (ret)
    {
        SET_CODE(res_info, -ret);
        jpf_warning("<JpfModMss> No such guest.");
	 jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_query_hd_group_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModMss *self;
    JpfQueryHdGroupInfo *req_info;

    self = (JpfModMss*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_mss_backward_2(self, msg, req_info->mss_id, req_info->session);
}


NmpMsgFunRet
nmp_mod_mss_query_all_hd_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    JpfModMss *self;
    JpfQueryAllHdRes *res_info;
    gint ret = 0;

    self = (JpfModMss*)app_obj;

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_mss_forward(self, msg);
    if (ret)
    {
        SET_CODE(res_info, -ret);
        jpf_warning("<JpfModMss> No such guest.");
	 jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_query_all_hd_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModMss *self;
    JpfQueryAllHd *req_info;

    self = (JpfModMss*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_mss_backward_2(self, msg, req_info->mss_id, req_info->session);
}


NmpMsgFunRet
nmp_mod_mss_del_hd_group_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    JpfModMss *self;
    JpfDelHdGroupRes *res_info;
    gint ret = 0;

    self = (JpfModMss*)app_obj;

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_mss_forward(self, msg);
    if (ret)
    {
        SET_CODE(res_info, -ret);
        jpf_warning("<JpfModMss> No such guest.");
	 jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_del_hd_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModMss *self;
    JpfDelHdGroup *req_info;

    self = (JpfModMss*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_mss_backward_2(self, msg, req_info->mss_id, req_info->session);
}


NmpMsgFunRet
nmp_mod_mss_get_hd_format_progress_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    JpfModMss *self;
    JpfGetHdFormatProgressRes *res_info;
    gint ret = 0;

    self = (JpfModMss*)app_obj;

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_mss_forward(self, msg);
    if (ret)
    {
        SET_CODE(res_info, -ret);
        jpf_warning("<JpfModMss> No such guest.");
	 jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_get_hd_format_progress_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModMss *self;
    JpfGetHdFormatProgress *req_info;

    self = (JpfModMss*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_mss_backward_2(self, msg, req_info->mss_id, req_info->session);
}



NmpMsgFunRet
nmp_mod_mss_get_store_log_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModMss *self;
    JpfGetStoreLogRes *res_info;
    JpfGuestBase *mss_base;
    JpfNetIO *io;

    self = (JpfModMss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    io = MSG_IO(msg);
    BUG_ON(!io);

    mss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!mss_base))
    {
        SET_CODE(&res_info->code, E_NOSUCHGUEST);
        jpf_warning("<JpfModMss> mssId: No such guest.");
	 jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    MSG_SET_DSTPOS(msg, BUSSLOT_POS_CU);
    jpf_mods_container_put_guest(self->container, mss_base);

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_get_store_log_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModMss *self;
    JpfGetMssStoreLog *req_info;
    JpfGuestBase *mss_base;
    JpfNetIO *io;
    gint msg_id;
    JpfErrRes      code;

    self = (JpfModMss*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&code, 0, sizeof(code));
    msg_id = MSG_GETID(msg);
    mss_base = jpf_mods_container_get_guest_2(self->container, req_info->mss_id);
    if (G_UNLIKELY(!mss_base))
    {
        jpf_warning("<JpfModMss> deliver msg '%s' failed, MssId:%s no such mss.",
            MESSAGE_ID_TO_STR(cms, msg_id), req_info->mss_id);

        SET_CODE(&code.code, E_NOSUCHGUEST);
        memcpy(code.session, req_info->session, SESSION_ID_LEN - 1);
        MSG_SET_DSTPOS(msg, BUSSLOT_POS_CU);
        jpf_sysmsg_set_private_2(msg, &code, sizeof(code));
        MSG_SET_RESPONSE(msg);
        return MFR_DELIVER_BACK;
    }

    io = IO_OF_GUEST(mss_base);
    BUG_ON(!io);

    jpf_sysmsg_attach_io(msg, io);
    jpf_mods_container_put_guest(self->container, mss_base);

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_query_gu_record_status_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    JpfModMss *self;
    JpfQueryGuRecordStatusRes *res_info;
    gint ret = 0;

    self = (JpfModMss*)app_obj;

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_mss_forward(self, msg);
    if (ret)
    {
        SET_CODE(res_info, -ret);
        jpf_warning("<JpfModMss> No such guest.");
	 jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_query_gu_record_status_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModMss *self;
    JpfQueryGuRecordStatus *req_info;

    self = (JpfModMss*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_mss_backward_2(self, msg, req_info->mss_id, req_info->session);
}


NmpMsgFunRet
nmp_mod_mss_notify_mss_change_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfChangeMss *res_info = NULL;
    JpfModMss *self;
    JpfGuestBase *mss_base;
    JpfNetIO *io;
    gint msg_id;

    self = NMP_MODMSS(app_obj);
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    msg_id = MSG_GETID(msg);
    mss_base = jpf_mods_container_get_guest_2(self->container, res_info->mss_id);
    if (G_UNLIKELY(!mss_base))
    {
        jpf_warning("<JpfModMss> deliver msg '%s' failed, MssId:%s no such mss.",
            MESSAGE_ID_TO_STR(cms, msg_id), res_info->mss_id);
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    io = IO_OF_GUEST(mss_base);
    BUG_ON(!io);

    jpf_sysmsg_attach_io(msg, io);
    jpf_mods_container_put_guest(self->container, mss_base);

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_alarm_link_record_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModMss *self;
    JpfGuestBase *mss_base;
    gint msg_id;
    JpfAmsActionRecord *req_info = NULL;

    self = NMP_MODMSS(app_obj);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    msg_id = MSG_GETID(msg);
    mss_base = jpf_mods_container_get_guest_2(self->container,
		req_info->mss_id.mss_id);
    if (G_UNLIKELY(!mss_base))
    {
        jpf_warning("<JpfModMss> deliver msg '%s' failed, MssId:%s no such mss.",
            MESSAGE_ID_TO_STR(cms, msg_id), req_info->mss_id.mss_id);
        jpf_sysmsg_destroy(msg);

        return MFR_ACCEPTED;
    }
    jpf_sysmsg_attach_io(msg, IO_OF_GUEST(mss_base));
    jpf_mods_container_put_guest(self->container, mss_base);

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_alarm_link_snapshot_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModMss *self;
    JpfGuestBase *mss_base;
    gint msg_id;
    JpfAmsActionSnapshot *req_info = NULL;

    self = NMP_MODMSS(app_obj);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    msg_id = MSG_GETID(msg);
    mss_base = jpf_mods_container_get_guest_2(self->container,
		req_info->mss_id.mss_id);
    if (G_UNLIKELY(!mss_base))
    {
        jpf_warning("<JpfModMss> deliver msg '%s' failed, MssId:%s no such mss.",
            MESSAGE_ID_TO_STR(cms, msg_id), req_info->mss_id.mss_id);
        jpf_sysmsg_destroy(msg);

        return MFR_ACCEPTED;
    }
    jpf_sysmsg_attach_io(msg, IO_OF_GUEST(mss_base));
    jpf_mods_container_put_guest(self->container, mss_base);

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_get_initiator_name_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    JpfModMss *self;
    JpfGetInitNameRes *res_info;
    gint ret = 0;

    self = (JpfModMss*)app_obj;

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_mss_forward(self, msg);
    if (ret)
    {
        SET_CODE(res_info, -ret);
        jpf_warning("<JpfModMss> No such guest.");
	 jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_get_initiator_name_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModMss *self;
    JpfGetInitName *req_info;

    self = (JpfModMss*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_mss_backward_2(self, msg, req_info->mss_id, req_info->session);
}


NmpMsgFunRet
nmp_mod_mss_set_initiator_name_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    JpfModMss *self;
    JpfSetInitNameRes *res_info;
    gint ret = 0;

    self = (JpfModMss*)app_obj;

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_mss_forward(self, msg);
    if (ret)
    {
        SET_CODE(res_info, -ret);
        jpf_warning("<JpfModMss> No such guest.");
	 jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_set_initiator_name_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModMss *self;
    JpfSetInitName *req_info;

    self = (JpfModMss*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_mss_backward_2(self, msg, req_info->mss_id, req_info->session);
}


NmpMsgFunRet
nmp_mod_mss_get_ipsan_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    JpfModMss *self;
    JpfGetIpsanInfoRes *res_info;
    gint ret = 0;

    self = (JpfModMss*)app_obj;

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_mss_forward(self, msg);
    if (ret)
    {
        SET_CODE(res_info, -ret);
        jpf_warning("<JpfModMss> No such guest.");
	 jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_get_ipsan_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModMss *self;
    JpfGetIpsanInfo *req_info;

    self = (JpfModMss*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_mss_backward_2(self, msg, req_info->mss_id, req_info->session);
}


NmpMsgFunRet
nmp_mod_mss_add_one_ipsan_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    JpfModMss *self;
    JpfAddOneIpsanRes *res_info;
    gint ret = 0;

    self = (JpfModMss*)app_obj;

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_mss_forward(self, msg);
    if (ret)
    {
        SET_CODE(res_info, -ret);
        jpf_warning("<JpfModMss> No such guest.");
	 jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_add_one_ipsan_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModMss *self;
    JpfAddOneIpsan *req_info;

    self = (JpfModMss*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_mss_backward_2(self, msg, req_info->mss_id, req_info->session);
}


NmpMsgFunRet
nmp_mod_mss_delete_one_ipsan_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    JpfModMss *self;
    JpfDeleteOneIpsanRes *res_info;
    gint ret = 0;

    self = (JpfModMss*)app_obj;

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_mss_forward(self, msg);
    if (ret)
    {
        SET_CODE(res_info, -ret);
        jpf_warning("<JpfModMss> No such guest.");
	 jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_delete_one_ipsan_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModMss *self;
    JpfDeleteOneIpsan *req_info;

    self = (JpfModMss*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_mss_backward_2(self, msg, req_info->mss_id, req_info->session);
}


NmpMsgFunRet
nmp_mod_mss_get_one_ipsan_detail_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    JpfModMss *self;
    JpfGetOneIpsanDetailRes *res_info;
    gint ret = 0;

    self = (JpfModMss*)app_obj;

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_mss_forward(self, msg);
    if (ret)
    {
        SET_CODE(res_info, -ret);
        jpf_warning("<JpfModMss> No such guest.");
	 jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_get_one_ipsan_detail_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModMss *self;
    JpfGetOneIpsanDetail *req_info;

    self = (JpfModMss*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_mss_backward_2(self, msg, req_info->mss_id, req_info->session);
}


NmpMsgFunRet
nmp_mod_mss_notify_message_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    JpfModMss *self;
    JpfNetIO *io;
    JpfGuestBase *mss_base;
    JpfNotifyMessage *req_info;
    NmpMsgID msg_id;
    gint ret = 0;

    self = (JpfModMss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    mss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!mss_base))
    {
        ret = -E_NOSUCHGUEST;
        //jpf_warning("<JpfModMss> mssId:%s No such guest.", req_info->mss_id);
    }
    else
    {
        MSG_SET_DSTPOS(msg, BUSSLOT_POS_CU);
        jpf_mods_container_put_guest(self->container, mss_base);
	  return MFR_DELIVER_AHEAD;
    }

    jpf_sysmsg_destroy(msg);
    return MFR_ACCEPTED;
}


void
nmp_mod_mss_register_msg_handler(JpfModMss *self)
{
    NmpAppMod *super_self = (NmpAppMod*)self;

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MSS_REGISTER,
        nmp_mod_mss_register_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MSS_HEART,
        nmp_mod_mss_heart_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MSS_GET_GUID,
        nmp_mod_mss_get_guid_f,
        nmp_mod_mss_get_guid_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MSS_GET_RECORD_POLICY,
        nmp_mod_mss_get_record_policy_f,
        nmp_mod_mss_get_record_policy_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MSS_RECORD_POLICY_CHANGE,
        NULL,
        nmp_mod_mss_notify_policy_change_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MSS_GET_ROUTE,
        nmp_mod_mss_get_route_f,
        nmp_mod_mss_get_route_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MSS_GET_MDS,
        nmp_mod_mss_get_mds_f,
        nmp_mod_mss_get_mds_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MSS_GET_MDS_IP,
        nmp_mod_mss_get_mds_ip_f,
        nmp_mod_mss_get_mds_ip_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_HD_GROUP,
        nmp_mod_mss_add_hd_group_f,
        nmp_mod_mss_add_hd_group_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_HD,
        nmp_mod_mss_add_hd_to_group_f,
        nmp_mod_mss_add_hd_to_group_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_HD,
        nmp_mod_mss_del_hd_from_group_f,
        nmp_mod_mss_del_hd_from_group_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_REBOOT_MSS,
        nmp_mod_mss_reboot_mss_f,
        nmp_mod_mss_reboot_mss_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_ALL_HD_GROUP,
        nmp_mod_mss_query_all_hd_group_f,
        nmp_mod_mss_query_all_hd_group_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_HD_GROUP_INFO,
        nmp_mod_mss_query_hd_group_info_f,
        nmp_mod_mss_query_hd_group_info_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_ALL_HD,
        nmp_mod_mss_query_all_hd_f,
        nmp_mod_mss_query_all_hd_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_HD_GROUP,
        nmp_mod_mss_del_hd_group_f,
        nmp_mod_mss_del_hd_group_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_HD_FORMAT_PROGRESS,
        nmp_mod_mss_get_hd_format_progress_f,
        nmp_mod_mss_get_hd_format_progress_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MSS_GU_LIST_CHANGE,
        NULL,
        nmp_mod_mss_gu_list_change_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_MSS_STORE_LOG,
        nmp_mod_mss_get_store_log_f,
        nmp_mod_mss_get_store_log_b,
        0
    );

   nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_GU_RECORD_STATUS,
        nmp_mod_mss_query_gu_record_status_f,
        nmp_mod_mss_query_gu_record_status_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_CHANGE_MSS,
        NULL,
        nmp_mod_mss_notify_mss_change_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ALARM_LINK_RECORD,
        NULL,
        nmp_mod_mss_alarm_link_record_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ALARM_LINK_SNAPSHOT,
        NULL,
        nmp_mod_mss_alarm_link_snapshot_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_INIT_NAME,
        nmp_mod_mss_get_initiator_name_f,
        nmp_mod_mss_get_initiator_name_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_INIT_NAME,
        nmp_mod_mss_set_initiator_name_f,
        nmp_mod_mss_set_initiator_name_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_IPSAN_INFO,
        nmp_mod_mss_get_ipsan_info_f,
        nmp_mod_mss_get_ipsan_info_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_ONE_IPSAN,
        nmp_mod_mss_add_one_ipsan_f,
        nmp_mod_mss_add_one_ipsan_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DELETE_ONE_IPSAN,
        nmp_mod_mss_delete_one_ipsan_f,
        nmp_mod_mss_delete_one_ipsan_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_ONE_IPSAN,
        nmp_mod_mss_get_one_ipsan_detail_f,
        nmp_mod_mss_get_one_ipsan_detail_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_BROADCAST_GENERAL_MSG,
        nmp_mod_mss_notify_message_f,
        NULL,
        0
    );
}

