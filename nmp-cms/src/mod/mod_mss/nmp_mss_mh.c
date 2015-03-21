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
nmp_mod_mss_mss_register(NmpModMss *self, NmpNetIO *io,  NmpMsgID msg_id,
    NmpMssRegister *req, NmpMssRegisterRes *res)
{
    gint ret;
    NmpID conflict;

    G_ASSERT(self != NULL && io != NULL && req != NULL && res != NULL);

    ret = nmp_mod_mss_new_mss(self, io,  req->mss_id, &conflict);
    if (G_UNLIKELY(ret))
        return ret;

    ret = nmp_mod_mss_sync_req(self, msg_id, req,
         sizeof(NmpMssRegister), res, sizeof(NmpMssRegisterRes));

    return ret;
}


NmpMsgFunRet
nmp_mod_mss_register_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModMss *self;
    NmpNetIO *io;
    NmpMssRegister *req_info;
    NmpMssRegisterRes res_info;
    NmpMsgID msg_id;
    NmpGuestBase *mss_base;
    NmpMss *mss;
    NmpMsgMssOnlineChange notify_info;
    gint ret;
    gchar *mss_ip;
    NmpResourcesCap res_cap;

    self = (NmpModMss*)app_obj;
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
        nmp_print(
            "<NmpModMss> mss:%s register failed, err:%d",
            req_info->mss_id, -ret
        );

        SET_CODE(&res_info, -ret);
        MSG_SET_RESPONSE(msg);
        nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

        nmp_app_obj_deliver_in((NmpAppObj*)self, msg);
        nmp_mod_acc_release_io((NmpModAccess*)self, io);
        nmp_mod_container_del_io(self->container, io);

        return MFR_ACCEPTED;
    }

    mss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!mss_base))
    {
        ret = -E_NOSUCHGUEST;
	 nmp_warning("<NmpModMss> Can't find mss:'%s' in container, io timeout?", notify_info.mss_id);
    }
    else
   {
        mss = (NmpMss *)mss_base;
        mss->mss_state = STAT_MSS_ONLINE;
        nmp_mods_container_put_guest(self->container, mss_base);

        nmp_print(
             "<NmpModMss> mss:%s register ok",
             req_info->mss_id
         );

        strncpy(res_info.domain_id, nmp_get_local_domain_id(), DOMAIN_ID_LEN - 1);

        nmp_check_keepalive_time(&res_info.keep_alive_time);
        nmp_net_io_set_ttd(io, res_info.keep_alive_time*1000*TIMEOUT_N_PERIODS);

        mss_ip = nmp_net_get_io_peer_name(io);
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
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
    return MFR_DELIVER_BACK;

}


NmpMsgFunRet
nmp_mod_mss_heart_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModMss *self;
    NmpNetIO *io;
    NmpGuestBase *mss_base;
    NmpMssHeart *req_info;
    NmpMssHeartRes res_info;
    NmpMsgID msg_id;
    gint ret = 0;
    NmpResourcesCap res_cap;

    self = (NmpModMss*)app_obj;
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
        nmp_mod_acc_release_io((NmpModAccess*)self, io);
        nmp_mod_container_del_io(self->container, io);
        return MFR_ACCEPTED;
    }
    memset(&res_info, 0, sizeof(res_info));
    mss_base = nmp_mods_container_get_guest_2(self->container, req_info->mss_id);
    if (G_UNLIKELY(!mss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModMss> mssId:%s No such guest.", req_info->mss_id);
    }
    else
    {
        nmp_get_utc_time(res_info.server_time);
        nmp_mods_container_put_guest(self->container, mss_base);
    }

    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_mss_get_guid_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    NmpModMss *self;
    NmpNetIO *io;
    NmpGuestBase *mss_base;
    NmpMssGetGuid *req_info;
    NmpMssGetGuidRes res_info;
    NmpMsgID msg_id;
    gint ret = 0;

    self = (NmpModMss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    mss_base = nmp_mods_container_get_guest_2(self->container, req_info->mss_id);
    if (G_UNLIKELY(!mss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModMss> mssId:%s No such guest.", req_info->mss_id);
    }
    else
    {
        MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
        nmp_mods_container_put_guest(self->container, mss_base);
	  return MFR_DELIVER_AHEAD;
    }

    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_mss_backward(NmpModMss *self, NmpSysMsg *msg, const gchar *id_str)
{
    NmpGuestBase *mss_base;
    NmpNetIO *io;
    gint msg_id;

    msg_id = MSG_GETID(msg);
    mss_base = nmp_mods_container_get_guest_2(self->container, id_str);
    if (G_UNLIKELY(!mss_base))
    {
        nmp_warning("<NmpModMss> deliver msg '%s' failed, MssId:%s no such mss.",
            MESSAGE_ID_TO_STR(cms, msg_id), id_str);
        nmp_sysmsg_destroy(msg);

        return MFR_ACCEPTED;
    }

    io = IO_OF_GUEST(mss_base);
    BUG_ON(!io);

    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_attach_io(msg, io);
    nmp_mods_container_put_guest(self->container, mss_base);

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_backward_2(NmpModMss *self, NmpSysMsg *msg, const gchar *id_str, const gchar *session_id)
{
    NmpGuestBase *mss_base;
    NmpNetIO *io;
    gint msg_id;
    NmpErrRes      code;

    memset(&code, 0, sizeof(code));
    msg_id = MSG_GETID(msg);
    mss_base = nmp_mods_container_get_guest_2(self->container, id_str);
    if (G_UNLIKELY(!mss_base))
    {
        nmp_warning("<NmpModMss> deliver msg '%s' failed, MssId:%s no such mss.",
            MESSAGE_ID_TO_STR(cms, msg_id), id_str);
        //nmp_sysmsg_destroy(msg);


        SET_CODE(&code.code, E_NOSUCHGUEST);
	if (session_id)
            memcpy(code.session, session_id, USER_NAME_LEN - 1);

        MSG_SET_DSTPOS(msg, BUSSLOT_POS_BSS);
        nmp_sysmsg_set_private_2(msg, &code, sizeof(code));
        MSG_SET_RESPONSE(msg);
        return MFR_DELIVER_BACK;
    }

    io = IO_OF_GUEST(mss_base);
    BUG_ON(!io);

    nmp_sysmsg_attach_io(msg, io);
    nmp_mods_container_put_guest(self->container, mss_base);

    return MFR_DELIVER_AHEAD;
}



NmpMsgFunRet
nmp_mod_mss_get_guid_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModMss *self;
    NmpMssGetGuidRes *res_info;

    self = (NmpModMss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_mss_backward(self, msg, res_info->mss_id);
}


NmpMsgFunRet
nmp_mod_mss_get_record_policy_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    NmpModMss *self;
    NmpNetIO *io;
    NmpGuestBase *mss_base;
    NmpMssGetRecordPolicy *req_info;
    NmpMssGetRecordPolicyRes res_info;
    NmpMsgID msg_id;
    gint ret = 0;

    self = (NmpModMss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    mss_base = nmp_mods_container_get_guest_2(self->container, req_info->mss_id);
    if (G_UNLIKELY(!mss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModMss> mssId:%s No such guest.", req_info->mss_id);
    }
    else
    {
        MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
        nmp_mods_container_put_guest(self->container, mss_base);
	  return MFR_DELIVER_AHEAD;
    }

    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_mss_get_record_policy_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModMss *self;
    NmpMssGetRecordPolicyRes *res_info;

    self = (NmpModMss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_mss_backward(self, msg, res_info->mss_id);
}


NmpMsgFunRet
nmp_mod_mss_notify_policy_change_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpNotifyPolicyChange *res_info = NULL;
    NmpModMss *self;
    NmpGuestBase *mss_base;
    NmpNetIO *io;
    gint msg_id;

    self = NMP_MODMSS(app_obj);
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    msg_id = MSG_GETID(msg);
    mss_base = nmp_mods_container_get_guest_2(self->container, res_info->mss_id);
    if (G_UNLIKELY(!mss_base))
    {
        nmp_warning("<NmpModMss> deliver msg '%s' failed, MssId:%s no such mss.",
            MESSAGE_ID_TO_STR(cms, msg_id), res_info->mss_id);
        nmp_sysmsg_destroy(msg);

        return MFR_ACCEPTED;
    }

    io = IO_OF_GUEST(mss_base);
    BUG_ON(!io);

    nmp_sysmsg_attach_io(msg, io);
    nmp_mods_container_put_guest(self->container, mss_base);

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_gu_list_change_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModMss *self;
    NmpGuestBase *mss_base;
    gint msg_id;
    NmpMssId *req_info = NULL;

    self = NMP_MODMSS(app_obj);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    msg_id = MSG_GETID(msg);
    mss_base = nmp_mods_container_get_guest_2(self->container, req_info->mss_id);
    if (G_UNLIKELY(!mss_base))
    {
        nmp_warning("<NmpModMss> deliver msg '%s' failed, MssId:%s no such mss.",
            MESSAGE_ID_TO_STR(cms, msg_id), req_info->mss_id);
        nmp_sysmsg_destroy(msg);

        return MFR_ACCEPTED;
    }
    nmp_sysmsg_attach_io(msg, IO_OF_GUEST(mss_base));
    nmp_mods_container_put_guest(self->container, mss_base);

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_get_route_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    NmpModMss *self;
    NmpNetIO *io;
    NmpGuestBase *mss_base;
    NmpMssGetRoute *req_info;
    NmpMssGetRouteRes res_info;
    NmpMsgID msg_id;
    gint ret = 0;

    self = (NmpModMss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    mss_base = nmp_mods_container_get_guest_2(self->container, req_info->mss_id);
    if (G_UNLIKELY(!mss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModMss> mssId:%s No such guest.", req_info->mss_id);
    }
    else
    {
        MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
        nmp_mods_container_put_guest(self->container, mss_base);
	  return MFR_DELIVER_AHEAD;
    }

    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_mss_get_route_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModMss *self;
    NmpMssGetRouteRes *res_info;

    self = (NmpModMss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_mss_backward(self, msg, res_info->mss_id);
}


NmpMsgFunRet
nmp_mod_mss_get_mds_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    NmpModMss *self;
    NmpNetIO *io;
    NmpGuestBase *mss_base;
    NmpMssGetMds *req_info;
    NmpMssGetMdsRes res_info;
    NmpMsgID msg_id;
    gint ret = 0;

    self = (NmpModMss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    mss_base = nmp_mods_container_get_guest_2(self->container, req_info->mss_id);
    if (G_UNLIKELY(!mss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModMss> mssId:%s No such guest.", req_info->mss_id);
    }
    else
    {
        MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
        nmp_mods_container_put_guest(self->container, mss_base);
	  return MFR_DELIVER_AHEAD;
    }

    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_mss_get_mds_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModMss *self;
    NmpMssGetMdsRes *res_info;

    self = (NmpModMss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_mss_backward(self, msg, res_info->mss_id);
}


NmpMsgFunRet
nmp_mod_mss_get_mds_ip_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    NmpModMss *self;
    NmpNetIO *io;
    NmpGuestBase *mss_base;
    NmpMssGetMdsIp *req_info;
    NmpMssGetMdsIpRes res_info;
    NmpMsgID msg_id;
    gint ret = 0;

    self = (NmpModMss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    mss_base = nmp_mods_container_get_guest_2(self->container, req_info->mss_id);
    if (G_UNLIKELY(!mss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModMss> mssId:%s No such guest.", req_info->mss_id);
    }
    else
    {
        MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
        nmp_mods_container_put_guest(self->container, mss_base);
	  return MFR_DELIVER_AHEAD;
    }

    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_mss_get_mds_ip_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModMss *self;
    NmpMssGetMdsIpRes *res_info;

    self = (NmpModMss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_mss_backward(self, msg, res_info->mss_id);
}

gint
nmp_mod_mss_forward(NmpModMss *self, NmpSysMsg *msg)
{
    NmpGuestBase *mss_base;
    NmpNetIO *io;

    io = MSG_IO(msg);
    BUG_ON(!io);

    mss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!mss_base))
        return -E_NOSUCHGUEST;

    MSG_SET_DSTPOS(msg, BUSSLOT_POS_BSS);
    nmp_mods_container_put_guest(self->container, mss_base);

    return 0;
}


NmpMsgFunRet
nmp_mod_mss_add_hd_group_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    NmpModMss *self;
    NmpAddHdGroupRes *res_info;
    gint ret = 0;

    self = (NmpModMss*)app_obj;

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_mss_forward(self, msg);
    if (ret)
    {
        SET_CODE(res_info, -ret);
        nmp_warning("<NmpModMss> No such guest.");
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_add_hd_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModMss *self;
    NmpAddHdGroup *req_info;

    self = (NmpModMss*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_mss_backward_2(self, msg, req_info->mss_id, req_info->session);
}


NmpMsgFunRet
nmp_mod_mss_add_hd_to_group_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    NmpModMss *self;
    NmpAddHdToGroupRes *res_info;
    gint ret = 0;

    self = (NmpModMss*)app_obj;

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_mss_forward(self, msg);
    if (ret)
    {
        SET_CODE(res_info, -ret);
        nmp_warning("<NmpModMss> No such guest.");
	 nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_add_hd_to_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModMss *self;
    NmpAddHdToGroup *req_info;

    self = (NmpModMss*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_mss_backward_2(self, msg, req_info->mss_id, req_info->session);
}


NmpMsgFunRet
nmp_mod_mss_del_hd_from_group_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    NmpModMss *self;
    NmpDelHdFromGroupRes *res_info;
    gint ret = 0;

    self = (NmpModMss*)app_obj;

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_mss_forward(self, msg);
    if (ret)
    {
        SET_CODE(res_info, -ret);
        nmp_warning("<NmpModMss> No such guest.");
	 nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_del_hd_from_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModMss *self;
    NmpDelHdFromGroup *req_info;

    self = (NmpModMss*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_mss_backward_2(self, msg, req_info->mss_id, req_info->session);
}


NmpMsgFunRet
nmp_mod_mss_reboot_mss_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    NmpModMss *self;
    NmpRebootMssRes *res_info;
    gint ret = 0;

    self = (NmpModMss*)app_obj;

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_mss_forward(self, msg);
    if (ret)
    {
        SET_CODE(res_info, -ret);
        nmp_warning("<NmpModMss> No such guest.");
	 nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_reboot_mss_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModMss *self;
    NmpRebootMss *req_info;

    self = (NmpModMss*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_mss_backward_2(self, msg, req_info->mss_id, req_info->session);
}


NmpMsgFunRet
nmp_mod_mss_query_all_hd_group_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    NmpModMss *self;
    NmpQueryAllHdGroupRes *res_info;
    gint ret = 0;

    self = (NmpModMss*)app_obj;

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_mss_forward(self, msg);
    if (ret)
    {
        SET_CODE(res_info, -ret);
        nmp_warning("<NmpModMss> No such guest.");
	 nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_query_all_hd_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModMss *self;
    NmpQueryAllHdGroup *req_info;

    self = (NmpModMss*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_mss_backward_2(self, msg, req_info->mss_id, req_info->session);
}


NmpMsgFunRet
nmp_mod_mss_query_hd_group_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    NmpModMss *self;
    NmpQueryHdGroupInfoRes *res_info;
    gint ret = 0;

    self = (NmpModMss*)app_obj;

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_mss_forward(self, msg);
    if (ret)
    {
        SET_CODE(res_info, -ret);
        nmp_warning("<NmpModMss> No such guest.");
	 nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_query_hd_group_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModMss *self;
    NmpQueryHdGroupInfo *req_info;

    self = (NmpModMss*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_mss_backward_2(self, msg, req_info->mss_id, req_info->session);
}


NmpMsgFunRet
nmp_mod_mss_query_all_hd_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    NmpModMss *self;
    NmpQueryAllHdRes *res_info;
    gint ret = 0;

    self = (NmpModMss*)app_obj;

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_mss_forward(self, msg);
    if (ret)
    {
        SET_CODE(res_info, -ret);
        nmp_warning("<NmpModMss> No such guest.");
	 nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_query_all_hd_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModMss *self;
    NmpQueryAllHd *req_info;

    self = (NmpModMss*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_mss_backward_2(self, msg, req_info->mss_id, req_info->session);
}


NmpMsgFunRet
nmp_mod_mss_del_hd_group_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    NmpModMss *self;
    NmpDelHdGroupRes *res_info;
    gint ret = 0;

    self = (NmpModMss*)app_obj;

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_mss_forward(self, msg);
    if (ret)
    {
        SET_CODE(res_info, -ret);
        nmp_warning("<NmpModMss> No such guest.");
	 nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_del_hd_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModMss *self;
    NmpDelHdGroup *req_info;

    self = (NmpModMss*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_mss_backward_2(self, msg, req_info->mss_id, req_info->session);
}


NmpMsgFunRet
nmp_mod_mss_get_hd_format_progress_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    NmpModMss *self;
    NmpGetHdFormatProgressRes *res_info;
    gint ret = 0;

    self = (NmpModMss*)app_obj;

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_mss_forward(self, msg);
    if (ret)
    {
        SET_CODE(res_info, -ret);
        nmp_warning("<NmpModMss> No such guest.");
	 nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_get_hd_format_progress_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModMss *self;
    NmpGetHdFormatProgress *req_info;

    self = (NmpModMss*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_mss_backward_2(self, msg, req_info->mss_id, req_info->session);
}



NmpMsgFunRet
nmp_mod_mss_get_store_log_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModMss *self;
    NmpGetStoreLogRes *res_info;
    NmpGuestBase *mss_base;
    NmpNetIO *io;

    self = (NmpModMss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    io = MSG_IO(msg);
    BUG_ON(!io);

    mss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!mss_base))
    {
        SET_CODE(&res_info->code, E_NOSUCHGUEST);
        nmp_warning("<NmpModMss> mssId: No such guest.");
	 nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    MSG_SET_DSTPOS(msg, BUSSLOT_POS_CU);
    nmp_mods_container_put_guest(self->container, mss_base);

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_get_store_log_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModMss *self;
    NmpGetMssStoreLog *req_info;
    NmpGuestBase *mss_base;
    NmpNetIO *io;
    gint msg_id;
    NmpErrRes      code;

    self = (NmpModMss*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&code, 0, sizeof(code));
    msg_id = MSG_GETID(msg);
    mss_base = nmp_mods_container_get_guest_2(self->container, req_info->mss_id);
    if (G_UNLIKELY(!mss_base))
    {
        nmp_warning("<NmpModMss> deliver msg '%s' failed, MssId:%s no such mss.",
            MESSAGE_ID_TO_STR(cms, msg_id), req_info->mss_id);

        SET_CODE(&code.code, E_NOSUCHGUEST);
        memcpy(code.session, req_info->session, SESSION_ID_LEN - 1);
        MSG_SET_DSTPOS(msg, BUSSLOT_POS_CU);
        nmp_sysmsg_set_private_2(msg, &code, sizeof(code));
        MSG_SET_RESPONSE(msg);
        return MFR_DELIVER_BACK;
    }

    io = IO_OF_GUEST(mss_base);
    BUG_ON(!io);

    nmp_sysmsg_attach_io(msg, io);
    nmp_mods_container_put_guest(self->container, mss_base);

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_query_gu_record_status_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    NmpModMss *self;
    NmpQueryGuRecordStatusRes *res_info;
    gint ret = 0;

    self = (NmpModMss*)app_obj;

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_mss_forward(self, msg);
    if (ret)
    {
        SET_CODE(res_info, -ret);
        nmp_warning("<NmpModMss> No such guest.");
	 nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_query_gu_record_status_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModMss *self;
    NmpQueryGuRecordStatus *req_info;

    self = (NmpModMss*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_mss_backward_2(self, msg, req_info->mss_id, req_info->session);
}


NmpMsgFunRet
nmp_mod_mss_notify_mss_change_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpChangeMss *res_info = NULL;
    NmpModMss *self;
    NmpGuestBase *mss_base;
    NmpNetIO *io;
    gint msg_id;

    self = NMP_MODMSS(app_obj);
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    msg_id = MSG_GETID(msg);
    mss_base = nmp_mods_container_get_guest_2(self->container, res_info->mss_id);
    if (G_UNLIKELY(!mss_base))
    {
        nmp_warning("<NmpModMss> deliver msg '%s' failed, MssId:%s no such mss.",
            MESSAGE_ID_TO_STR(cms, msg_id), res_info->mss_id);
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    io = IO_OF_GUEST(mss_base);
    BUG_ON(!io);

    nmp_sysmsg_attach_io(msg, io);
    nmp_mods_container_put_guest(self->container, mss_base);

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_alarm_link_record_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModMss *self;
    NmpGuestBase *mss_base;
    gint msg_id;
    NmpAmsActionRecord *req_info = NULL;

    self = NMP_MODMSS(app_obj);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    msg_id = MSG_GETID(msg);
    mss_base = nmp_mods_container_get_guest_2(self->container,
		req_info->mss_id.mss_id);
    if (G_UNLIKELY(!mss_base))
    {
        nmp_warning("<NmpModMss> deliver msg '%s' failed, MssId:%s no such mss.",
            MESSAGE_ID_TO_STR(cms, msg_id), req_info->mss_id.mss_id);
        nmp_sysmsg_destroy(msg);

        return MFR_ACCEPTED;
    }
    nmp_sysmsg_attach_io(msg, IO_OF_GUEST(mss_base));
    nmp_mods_container_put_guest(self->container, mss_base);

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_alarm_link_snapshot_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModMss *self;
    NmpGuestBase *mss_base;
    gint msg_id;
    NmpAmsActionSnapshot *req_info = NULL;

    self = NMP_MODMSS(app_obj);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    msg_id = MSG_GETID(msg);
    mss_base = nmp_mods_container_get_guest_2(self->container,
		req_info->mss_id.mss_id);
    if (G_UNLIKELY(!mss_base))
    {
        nmp_warning("<NmpModMss> deliver msg '%s' failed, MssId:%s no such mss.",
            MESSAGE_ID_TO_STR(cms, msg_id), req_info->mss_id.mss_id);
        nmp_sysmsg_destroy(msg);

        return MFR_ACCEPTED;
    }
    nmp_sysmsg_attach_io(msg, IO_OF_GUEST(mss_base));
    nmp_mods_container_put_guest(self->container, mss_base);

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_get_initiator_name_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    NmpModMss *self;
    NmpGetInitNameRes *res_info;
    gint ret = 0;

    self = (NmpModMss*)app_obj;

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_mss_forward(self, msg);
    if (ret)
    {
        SET_CODE(res_info, -ret);
        nmp_warning("<NmpModMss> No such guest.");
	 nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_get_initiator_name_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModMss *self;
    NmpGetInitName *req_info;

    self = (NmpModMss*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_mss_backward_2(self, msg, req_info->mss_id, req_info->session);
}


NmpMsgFunRet
nmp_mod_mss_set_initiator_name_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    NmpModMss *self;
    NmpSetInitNameRes *res_info;
    gint ret = 0;

    self = (NmpModMss*)app_obj;

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_mss_forward(self, msg);
    if (ret)
    {
        SET_CODE(res_info, -ret);
        nmp_warning("<NmpModMss> No such guest.");
	 nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_set_initiator_name_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModMss *self;
    NmpSetInitName *req_info;

    self = (NmpModMss*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_mss_backward_2(self, msg, req_info->mss_id, req_info->session);
}


NmpMsgFunRet
nmp_mod_mss_get_ipsan_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    NmpModMss *self;
    NmpGetIpsanInfoRes *res_info;
    gint ret = 0;

    self = (NmpModMss*)app_obj;

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_mss_forward(self, msg);
    if (ret)
    {
        SET_CODE(res_info, -ret);
        nmp_warning("<NmpModMss> No such guest.");
	 nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_get_ipsan_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModMss *self;
    NmpGetIpsanInfo *req_info;

    self = (NmpModMss*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_mss_backward_2(self, msg, req_info->mss_id, req_info->session);
}


NmpMsgFunRet
nmp_mod_mss_add_one_ipsan_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    NmpModMss *self;
    NmpAddOneIpsanRes *res_info;
    gint ret = 0;

    self = (NmpModMss*)app_obj;

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_mss_forward(self, msg);
    if (ret)
    {
        SET_CODE(res_info, -ret);
        nmp_warning("<NmpModMss> No such guest.");
	 nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_add_one_ipsan_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModMss *self;
    NmpAddOneIpsan *req_info;

    self = (NmpModMss*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_mss_backward_2(self, msg, req_info->mss_id, req_info->session);
}


NmpMsgFunRet
nmp_mod_mss_delete_one_ipsan_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    NmpModMss *self;
    NmpDeleteOneIpsanRes *res_info;
    gint ret = 0;

    self = (NmpModMss*)app_obj;

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_mss_forward(self, msg);
    if (ret)
    {
        SET_CODE(res_info, -ret);
        nmp_warning("<NmpModMss> No such guest.");
	 nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_delete_one_ipsan_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModMss *self;
    NmpDeleteOneIpsan *req_info;

    self = (NmpModMss*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_mss_backward_2(self, msg, req_info->mss_id, req_info->session);
}


NmpMsgFunRet
nmp_mod_mss_get_one_ipsan_detail_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    NmpModMss *self;
    NmpGetOneIpsanDetailRes *res_info;
    gint ret = 0;

    self = (NmpModMss*)app_obj;

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_mss_forward(self, msg);
    if (ret)
    {
        SET_CODE(res_info, -ret);
        nmp_warning("<NmpModMss> No such guest.");
	 nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mss_get_one_ipsan_detail_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModMss *self;
    NmpGetOneIpsanDetail *req_info;

    self = (NmpModMss*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_mss_backward_2(self, msg, req_info->mss_id, req_info->session);
}


NmpMsgFunRet
nmp_mod_mss_notify_message_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);
    NmpModMss *self;
    NmpNetIO *io;
    NmpGuestBase *mss_base;
    NmpNotifyMessage *req_info;
    NmpMsgID msg_id;
    gint ret = 0;

    self = (NmpModMss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    mss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!mss_base))
    {
        ret = -E_NOSUCHGUEST;
        //nmp_warning("<NmpModMss> mssId:%s No such guest.", req_info->mss_id);
    }
    else
    {
        MSG_SET_DSTPOS(msg, BUSSLOT_POS_CU);
        nmp_mods_container_put_guest(self->container, mss_base);
	  return MFR_DELIVER_AHEAD;
    }

    nmp_sysmsg_destroy(msg);
    return MFR_ACCEPTED;
}


void
nmp_mod_mss_register_msg_handler(NmpModMss *self)
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

