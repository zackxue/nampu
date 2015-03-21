#include <sys/unistd.h>
#include "nmp_mod_pu.h"
#include "nmp_pu_struct.h"
#include "nmp_message.h"
#include "nmp_errno.h"
#include "nmp_memory.h"
#include "nmp_shared.h"
#include "nmp_internal_msg.h"
#include "nmp_tw_interface.h"
#include "nmp_version.h"
//#include "nmp_res_ctl.h"

USING_MSG_ID_MAP(cms);

//static guint msg_seq_generator = 0;

gint
jpf_pu_register_info(JpfModPu *self, JpfNetIO *io, NmpSysMsg *msg, JpfPuRegRes *res)
{
    gint ret;
    JpfPuRegInfo *req_info;
    NmpMsgID msg_id;
    JpfID conflict;

    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    ret = nmp_mod_pu_register(self, io, req_info->puid, req_info->pu_type, &conflict);
    if (G_UNLIKELY(ret))
        return ret;

    ret = nmp_mod_pu_sync_req(self, msg_id, req_info, sizeof(JpfPuRegInfo),
	     res, sizeof(JpfPuRegRes));

    return ret;
}


NmpMsgFunRet
nmp_mod_pu_register_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfNetIO *io;
    JpfPuRegInfo *req_info;
    JpfPuRegRes res_info;
    NmpMsgID msg_id;
    JpfGuestBase *pu_base;
    JpfPu *pu;
    JpfPuOnlineStatusChange notify_info;
    gint ret = 0;
    gchar *pu_ip;
    JpfResourcesCap res_cap;
    gchar mf[MF_ID_LEN] = {0};

    self = (JpfModPu*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    jpf_get_mf_from_guid(req_info->puid, mf);
    memset(&res_cap, 0, sizeof(res_cap));
    nmp_mod_get_resource_cap(&res_cap);
    if (res_cap.module_bits&MODULE_CMS_BIT)
    {
        ret = jpf_compare_manufacturer(res_cap.modules_data[SYS_MODULE_CMS], mf);
        if (ret)
             goto err_login;
    }
    memset(&res_info, 0 ,sizeof(res_info));
    memset(&notify_info, 0, sizeof(notify_info));
    notify_info.puid[MAX_ID_LEN - 1] = 0;
    strncpy(notify_info.domain_id, jpf_get_local_domain_id(), DOMAIN_ID_LEN - 1);
    strncpy(notify_info.puid, req_info->puid, MAX_ID_LEN - 1);
    strcpy(notify_info.cms_ip, req_info->cms_ip);
    if (strlen(req_info->pu_ip))
        strcpy(notify_info.pu_ip, req_info->pu_ip);

    if ((req_info->pu_type == TYPE_SDEC)&&(req_info->pu_version < __PU_MIN_VERSION__))
    {
        ret = -E_VERSION;
        goto err_login;
    }

    jpf_covert_pu_type(&req_info->pu_type, &req_info->pu_type);
    ret = jpf_pu_register_info(self, io, msg, &res_info);
err_login:
    if (ret)
    {
         SET_CODE(&res_info, -ret);
         MSG_SET_RESPONSE(msg);
         jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
         jpf_print(
             "PU '%s' registers  failed, err:%d, IO '%p'.",
             notify_info.puid, ret, io
         );

      	 nmp_app_obj_deliver_in((NmpAppObj*)self, msg);
      	 nmp_mod_acc_release_io((JpfModAccess*)self, io);
      	 nmp_mod_container_del_io(self->container, io);

        return MFR_ACCEPTED;
    }

    pu_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!pu_base))
    {
        ret = -E_NOSUCHGUEST;
	 jpf_warning("<JpfModPu> Can't find pu:'%s' in container, io timeout?", notify_info.puid);
    }
    else
   {
        pu = (JpfPu *)pu_base;
        pu->state = STAT_PU_ONLINE;
        jpf_mods_container_put_guest(self->container, pu_base);

        jpf_print(
            "PU '%s' registered ok.",
            notify_info.puid
        );

        jpf_check_keepalive_time(&res_info.keep_alive_time);
        jpf_net_io_set_ttd(io, res_info.keep_alive_time*1000*TIMEOUT_N_PERIODS);

        if (!strlen(notify_info.pu_ip))
        {
            pu_ip = jpf_net_get_io_peer_name(io);
            if (G_LIKELY(pu_ip))
            {
                strncpy(notify_info.pu_ip, pu_ip, MAX_IP_LEN - 1);
                 g_free(pu_ip);
            }
        }

        notify_info.new_status = 1;
        nmp_mod_pu_change_pu_online_status(app_obj, notify_info);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_pu_heart_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfNetIO *io;
    JpfGuestBase *pu_base;
    JpfPuHeart *req_info;
    JpfPuHeartResp res_info;
    NmpMsgID msg_id;
    gint ret = 0;
    gchar *pu_ip;
    JpfPu *pu;
    JpfResourcesCap res_cap;
    gchar mf[MF_ID_LEN] = {0};

    self = (JpfModPu*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    jpf_get_mf_from_guid(req_info->puid, mf);
    memset(&res_cap, 0, sizeof(res_cap));
    nmp_mod_get_resource_cap(&res_cap);
    if (res_cap.module_bits&MODULE_CMS_BIT)
    {
        ret = jpf_compare_manufacturer(res_cap.modules_data[SYS_MODULE_CMS], mf);
        if (ret)
        {
            SET_CODE(&res_info, -ret);
            MSG_SET_RESPONSE(msg);
            nmp_app_obj_deliver_in((NmpAppObj*)self, msg);
            nmp_mod_acc_release_io((JpfModAccess*)self, io);
            nmp_mod_container_del_io(self->container, io);

            return MFR_ACCEPTED;
         }
    }
    pu_base = jpf_mods_container_get_guest(self->container, io);
    if (G_LIKELY(pu_base))
    {
        pu = (JpfPu *)pu_base;

        if (!pu->recheck)
        {
            jpf_get_utc_time(res_info.server_time);
            jpf_mods_container_put_guest(self->container, pu_base);
            ret = 0;
            goto end;
        }

	jpf_warning("<haha> pu recheck...");
        pu->recheck = 0;
        if (!strlen(req_info->pu_ip))
        {
            pu_ip = jpf_net_get_io_peer_name(io);
            if (G_LIKELY(pu_ip))
            {
                strncpy(req_info->pu_ip, pu_ip, MAX_IP_LEN - 1);
        	     g_free(pu_ip);
            }
        }
        MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
        jpf_mods_container_put_guest(self->container, pu_base);
        return MFR_DELIVER_AHEAD;
    }

    ret = -E_NOSUCHGUEST;
    jpf_warning("<JpfModPu> puid:%s No such puid.", req_info->puid);
end:
    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_pu_heart_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfPuHeartResp *res_info;
    JpfGuestBase *pu_base;
    JpfNetIO *io;
    gint msg_id;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    msg_id = MSG_GETID(msg);
    pu_base = jpf_mods_container_get_guest_2(self->container, res_info->puid);
    if (G_UNLIKELY(!pu_base))
    {
        jpf_warning("<JpfModPu> deliver msg '%s' failed, puid:%s no such pu.",
            MESSAGE_ID_TO_STR(cms, msg_id), res_info->puid);
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    jpf_get_utc_time(res_info->server_time);
    io = IO_OF_GUEST(pu_base);
    BUG_ON(!io);
    jpf_sysmsg_attach_io(msg, io);
    jpf_mods_container_put_guest(self->container, pu_base);

    if (RES_CODE(res_info))
    {
        nmp_app_obj_deliver_in((NmpAppObj*)self, msg);
      	 nmp_mod_acc_release_io((JpfModAccess*)self, io);
      	 nmp_mod_container_del_io(self->container, io);
	 return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_backward(JpfModPu *self, NmpSysMsg *msg, const gchar *id_str,
    const gchar *session_id)
{
    JpfGuestBase *pu_base;
    JpfNetIO *io;
    gint msg_id;
    JpfGetDeviceInfoErr      code;

    msg_id = MSG_GETID(msg);
    memset(&code, 0, sizeof(code));
    pu_base = jpf_mods_container_get_guest_2(self->container, id_str);
    if (G_UNLIKELY(!pu_base))
    {
        jpf_warning("<JpfModPu> deliver msg '%s' failed, puid:%s no such pu.",
            MESSAGE_ID_TO_STR(cms, msg_id), id_str);

        SET_CODE(&code, E_NOSUCHGUEST);


		if (session_id)
		{
		 	if (strcmp(session_id, jpf_get_local_domain_id()) == 0)
		    {
				jpf_sysmsg_destroy(msg);
		        return MFR_ACCEPTED;
		    }
		    memcpy(code.session, session_id, SESSION_ID_LEN - 1);
		}

        MSG_SET_DSTPOS(msg, BUSSLOT_POS_CU);
        jpf_sysmsg_set_private_2(msg, &code, sizeof(code));
        MSG_SET_RESPONSE(msg);
        return MFR_DELIVER_BACK;
    }

    io = IO_OF_GUEST(pu_base);
    BUG_ON(!io);
    jpf_sysmsg_attach_io(msg, io);
    jpf_mods_container_put_guest(self->container, pu_base);

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_backward_2(JpfModPu *self, NmpSysMsg *msg,
    const gchar *id_str, JpfBusSlotPos des_pos)
{
    JpfGuestBase *pu_base;
    JpfNetIO *io;
    gint msg_id;
    JpfGetDeviceInfoErr      code;

    msg_id = MSG_GETID(msg);
    memset(&code, 0, sizeof(code));
    pu_base = jpf_mods_container_get_guest_2(self->container, id_str);
    if (G_UNLIKELY(!pu_base))
    {
        jpf_warning("<JpfModPu> deliver msg '%s' failed, puid:%s no such pu.",
            MESSAGE_ID_TO_STR(cms, msg_id), id_str);

        SET_CODE(&code, E_NOSUCHGUEST);

        MSG_SET_DSTPOS(msg, des_pos);
        jpf_sysmsg_set_private_2(msg, &code, sizeof(code));
        MSG_SET_RESPONSE(msg);
        return MFR_DELIVER_BACK;
    }

    io = IO_OF_GUEST(pu_base);
    BUG_ON(!io);
    jpf_sysmsg_attach_io(msg, io);
    jpf_mods_container_put_guest(self->container, pu_base);

    return MFR_DELIVER_AHEAD;
}


gint
nmp_mod_pu_forward(JpfModPu *self, NmpSysMsg *msg, JpfBusSlotPos des_pos)
{
    JpfGuestBase *pu_base;
    JpfNetIO *io;

    io = MSG_IO(msg);
    BUG_ON(!io);

    pu_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!pu_base))
        return -E_NOSUCHGUEST;

    MSG_SET_DSTPOS(msg, des_pos);
    jpf_mods_container_put_guest(self->container, pu_base);

    return 0;
}


NmpMsgFunRet
nmp_mod_pu_get_mds_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetMdsInfo *req_info;
    JpfGetMdsInfoRes res_info;
    JpfGuestBase *pu_base;
    JpfNetIO *io;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    io = MSG_IO(msg);
    BUG_ON(!io);

    pu_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!pu_base))
   {
        memset(&res_info, 0, sizeof(res_info));
        SET_CODE(&res_info, E_NOSUCHGUEST);
        jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
        MSG_SET_RESPONSE(msg);
        jpf_warning("<JpfModPu> puid:%s No such guest.", req_info->puid);

        return MFR_DELIVER_BACK;
   }

    MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
    jpf_mods_container_put_guest(self->container, pu_base);

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_get_mds_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetMdsInfoRes *res_info;
    JpfGuestBase *pu_base;
    JpfNetIO *io;
    gint msg_id;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    msg_id = MSG_GETID(msg);
    pu_base = jpf_mods_container_get_guest_2(self->container, res_info->puid);
    if (G_UNLIKELY(!pu_base))
    {
        jpf_warning("<JpfModPu> deliver msg '%s' failed, puid:%s no such pu.",
            MESSAGE_ID_TO_STR(cms, msg_id), res_info->puid);
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    io = IO_OF_GUEST(pu_base);
    BUG_ON(!io);
    jpf_sysmsg_attach_io(msg, io);
    jpf_mods_container_put_guest(self->container, pu_base);

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_get_device_info(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetDeviceInfo *req_info;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_pu_backward(self, msg, req_info->puid, req_info->session);
}


gint
nmp_mod_pu_get_puid_from_guid(gchar *guid, gchar *puid, gint *channel)
{
    G_ASSERT(guid != NULL && puid != NULL);

    gchar gutype[PU_ID_LEN] = {0};

    return sscanf(guid, "%16s-%4s-%d",puid, gutype, channel) != 3;
}


NmpMsgFunRet
nmp_mod_pu_get_device_channel_para(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetDeviceChannelInfo *req_info;
    gchar puid[MAX_ID_LEN] = {0};
    gint channel = 0;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    nmp_mod_pu_get_puid_from_guid(req_info->guid, puid, &channel);

    return nmp_mod_pu_backward(self, msg, puid, req_info->session);
}


NmpMsgFunRet
nmp_mod_pu_set_device_info_resp(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfSetDeviceInfoRes *res_info;
    JpfModPu *self;
    gint ret, msg_id;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    msg_id = MSG_GETID(msg);
    ret = nmp_mod_pu_forward(self, msg, BUSSLOT_POS_CU);
    if (G_UNLIKELY(!ret))
	    return MFR_DELIVER_AHEAD;

    jpf_warning("<JpfModPu> deliver msg '%s' failed, puid:%s no such pu.",
        	    MESSAGE_ID_TO_STR(cms, msg_id), res_info->puid);
    jpf_sysmsg_destroy(msg);
    return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_pu_set_device_para_resp(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfSetDeviceParaRes *res_info;
    JpfModPu *self;
    gint ret, msg_id;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    msg_id = MSG_GETID(msg);
    ret = nmp_mod_pu_forward(self, msg, BUSSLOT_POS_CU);
    if (G_UNLIKELY(!ret))
	    return MFR_DELIVER_AHEAD;

    jpf_warning("<JpfModPu> deliver msg '%s' failed, puid:%16s no such pu.",
        	    MESSAGE_ID_TO_STR(cms, msg_id), res_info->guid);
    jpf_sysmsg_destroy(msg);
    return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_pu_change_dispatch_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfChangeDispatch *req_info;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_pu_backward(self, msg, req_info->puid, NULL);
}

//get msg from mod
NmpMsgFunRet
nmp_mod_pu_get_platform_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetPlatformInfoRes *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_pu_forward(self, msg, BUSSLOT_POS_CU);
    if (ret)
    {
        SET_CODE(&res_info->code, -ret);
        jpf_warning("<JpfModPu> puid:%s No such guest.", res_info->puid);
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_get_platform_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetDeviceInfo *req_info;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_pu_backward(self, msg, req_info->puid, req_info->session);
}


NmpMsgFunRet
nmp_mod_pu_set_platform_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_set_device_info_resp(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_platform_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfSetPlatformInfo *req_info;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    //nmp_mod_pu_get_puid_from_guid(req_info->puid, puid, &channel);

    return nmp_mod_pu_backward(self, msg, req_info->puid, req_info->session);
}


NmpMsgFunRet
nmp_mod_pu_get_media_url_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetMediaUrlRes *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_pu_forward(self, msg, BUSSLOT_POS_CU);
    if (ret)
    {
        SET_CODE(&res_info->code, -ret);
        jpf_warning("<JpfModPu> puid:%16s No such guest.", res_info->guid);
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_get_media_url_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetMediaUrl *req_info;
    gchar puid[MAX_ID_LEN] = {0};
    JpfGuestBase *pu_base;
    JpfNetIO *io;
    gint msg_id;
    JpfGetDeviceInfoErr      code;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    jpf_get_puid_from_guid(req_info->guid, puid);
    msg_id = MSG_GETID(msg);
    memset(&code, 0, sizeof(code));
    pu_base = jpf_mods_container_get_guest_2(self->container, puid);
    if (G_UNLIKELY(!pu_base))
    {
        jpf_warning("<JpfModPu> deliver msg '%s' failed, puid:%s no such pu.",
            MESSAGE_ID_TO_STR(cms, msg_id), puid);

        JpfPuOnlineStatusChange notify_info;
        memset(&notify_info, 0, sizeof(notify_info));
        notify_info.new_status = 0;
        strcpy(notify_info.puid, puid);
        strncpy(notify_info.domain_id, jpf_get_local_domain_id(), DOMAIN_ID_LEN - 1);
        nmp_mod_pu_change_pu_online_status(app_obj, notify_info);
        if (req_info->connect_mode == 0)
        {
            goto end_get_media_url;
        }

        SET_CODE(&code, E_NOSUCHGUEST);
	 if (req_info->session)
            strncpy(code.session, req_info->session, SESSION_ID_LEN - 1);

        MSG_SET_DSTPOS(msg, BUSSLOT_POS_CU);
        jpf_sysmsg_set_private_2(msg, &code, sizeof(code));
        MSG_SET_RESPONSE(msg);
        return MFR_DELIVER_BACK;
    }

    if (req_info->connect_mode == 0)
    {
        jpf_mods_container_put_guest(self->container, pu_base);
        goto end_get_media_url;
    }

    io = IO_OF_GUEST(pu_base);
    BUG_ON(!io);
    jpf_sysmsg_attach_io(msg, io);
    jpf_mods_container_put_guest(self->container, pu_base);

    return MFR_DELIVER_AHEAD;

end_get_media_url:
    jpf_sysmsg_destroy(msg);
    return MFR_ACCEPTED;
}

gint nmp_mod_pu_get_device_manufact(JpfModPu *self, gchar *mf_id, JpfMsgGetManufactRes *res)
{
    JpfMsgGetManufact req_info;
    JpfMsgGetManufactRes *res_info;
    NmpSysMsg *msg;
    gint ret;

    memset(&req_info, 0, sizeof(req_info));
    strncpy(req_info.mf_id, mf_id, MF_ID_LEN);
    msg = jpf_sysmsg_new_2(MSG_GET_DEVICE_MANUFACT, &req_info,
    	sizeof(req_info), ++msg_seq_generator);
    if (G_UNLIKELY(!msg))
    	return -E_NOMEM;

    MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
    ret = nmp_app_mod_sync_request((NmpAppMod*)self, &msg);
    if (G_UNLIKELY(ret))	/* send failed */
    {
        jpf_warning(
    		"<JpfModPu> request manufacturer failed!"
    	 );
    	 jpf_sysmsg_destroy(msg);

    	 return ret;
    }

    if (G_UNLIKELY(!msg))	/* sent, but no response */
    {
        jpf_warning(
        	"<JpfModPu> request manufacturer timeout!"
        );

        return -E_TIMEOUT;
    }

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);
    ret = RES_CODE(res_info);
    if (ret)
    {
    	jpf_warning(
    		"<JpfModPu> request manufacturer err:%d!", ret
    	);
    	jpf_sysmsg_destroy(msg);
    	return ret;
    }

    memcpy(res, res_info,sizeof(JpfMsgGetManufactRes));
    jpf_sysmsg_destroy(msg);

    return 0;
}

NmpMsgFunRet
nmp_mod_pu_get_device_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetDeviceInfoRes *res_info;
    JpfMsgGetManufactRes mf_info;
    gchar mf_id[MF_ID_LEN] = {0};
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    strncpy(mf_id, res_info->puid, MF_ID_LEN - 1);
    memset(&mf_info, 0, sizeof(mf_info));
    ret = nmp_mod_pu_get_device_manufact(self, mf_id, &mf_info);
    if (ret||RES_CODE(&mf_info))
    {
         strcpy(res_info->manu_info, "unkown");
    }

    strncpy(res_info->manu_info, mf_info.mf_name, DESCRIPTION_INFO_LEN - 1);
    ret = nmp_mod_pu_forward(self, msg, BUSSLOT_POS_CU);
    if (ret)
    {
        SET_CODE(&res_info->code, -ret);
        jpf_warning("<JpfModPu> puid:%s No such guest.", res_info->puid);
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_get_device_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_get_device_info(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_get_network_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetNetworkInfoRes *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_pu_forward(self, msg, BUSSLOT_POS_CU);
    if (ret)
    {
        SET_CODE(&res_info->code, -ret);
        jpf_warning("<JpfModPu> puid:%s No such guest.", res_info->puid);
	 jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_get_network_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_get_device_info(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_network_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_set_device_info_resp(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_network_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfSetNetworkInfo *req_info;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    //nmp_mod_pu_get_puid_from_guid(req_info->puid, puid, &channel);

    return nmp_mod_pu_backward(self, msg, req_info->puid, req_info->session);
}

NmpMsgFunRet
nmp_mod_pu_get_pppoe_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetPppoeInfoRes *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_pu_forward(self, msg, BUSSLOT_POS_CU);
    if (ret)
    {
        SET_CODE(&res_info->code, -ret);
        jpf_warning("<JpfModPu> puid:%s No such guest.", res_info->puid);
	 jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_get_pppoe_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_get_device_info(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_pppoe_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_set_device_info_resp(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_pppoe_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfSetPppoeInfo *req_info;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    //nmp_mod_pu_get_puid_from_guid(req_info->puid, puid, &channel);

    return nmp_mod_pu_backward(self, msg, req_info->puid, req_info->session);
}



NmpMsgFunRet
nmp_mod_pu_get_encode_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetEncodeParaRes *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_pu_forward(self, msg, BUSSLOT_POS_CU);
    if (ret)
    {
        SET_CODE(&res_info->code, -ret);
        jpf_warning("<JpfModPu> puid:%16s No such guest.", res_info->guid);
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_get_encode_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_get_device_channel_para(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_encode_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_set_device_para_resp(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_encode_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfSetEncodePara *req_info;
    gchar puid[MAX_ID_LEN] = {0};
    gint channel = 0;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    nmp_mod_pu_get_puid_from_guid(req_info->guid, puid, &channel);

    return nmp_mod_pu_backward(self, msg, puid, req_info->session);
}


NmpMsgFunRet
nmp_mod_pu_get_display_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetDisplayParaRes *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_pu_forward(self, msg, BUSSLOT_POS_CU);
    if (ret)
    {
        SET_CODE(&res_info->code, -ret);
        jpf_warning("<JpfModPu> puid:%16s No such guest.", res_info->guid);
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_get_display_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_get_device_channel_para(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_display_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_set_device_para_resp(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_display_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfSetDisplayPara *req_info;
    gchar puid[MAX_ID_LEN] = {0};
	gint channel = 0;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    nmp_mod_pu_get_puid_from_guid(req_info->guid, puid, &channel);

    return nmp_mod_pu_backward(self, msg, puid, req_info->session);
}


NmpMsgFunRet
nmp_mod_pu_get_OSD_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetOSDParaRes *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_pu_forward(self, msg, BUSSLOT_POS_CU);
    if (ret)
    {
        SET_CODE(&res_info->code, -ret);
        jpf_warning("<JpfModPu> puid:%16s No such guest.", res_info->guid);
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_get_OSD_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_get_device_channel_para(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_OSD_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_set_device_para_resp(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_OSD_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfSetOSDPara *req_info;
    gchar puid[MAX_ID_LEN] = {0};
    gint channel = 0;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    nmp_mod_pu_get_puid_from_guid(req_info->guid, puid, &channel);

    return nmp_mod_pu_backward(self, msg, puid, req_info->session);
}



NmpMsgFunRet
nmp_mod_pu_get_record_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetRecordParaRes *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_pu_forward(self, msg, BUSSLOT_POS_CU);
    if (ret)
    {
        SET_CODE(&res_info->code, -ret);
        jpf_warning("<JpfModPu> puid:%16s No such guest.", res_info->guid);
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_get_record_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_get_device_channel_para(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_record_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_set_device_para_resp(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_record_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    gchar puid[MAX_ID_LEN] = {0};
    gint channel = 0;
    JpfSetRecordPara *req_info;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    nmp_mod_pu_get_puid_from_guid(req_info->guid, puid, &channel);
    return nmp_mod_pu_backward(self, msg, puid, req_info->session);
}


NmpMsgFunRet
nmp_mod_pu_get_hide_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetHideParaRes *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_pu_forward(self, msg, BUSSLOT_POS_CU);
    if (ret)
    {
        SET_CODE(&res_info->code, ret);
        jpf_warning("<JpfModPu> puid:%16s No such guest.", res_info->guid);
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_get_hide_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_get_device_channel_para(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_hide_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_set_device_para_resp(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_hide_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfSetHidePara *req_info;
    gchar puid[MAX_ID_LEN] = {0};
    gint channel = 0;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    nmp_mod_pu_get_puid_from_guid(req_info->guid, puid, &channel);

    return nmp_mod_pu_backward(self, msg, puid, req_info->session);
}


NmpMsgFunRet
nmp_mod_pu_get_serial_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetSerialParaRes *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_pu_forward(self, msg, BUSSLOT_POS_CU);
    if (ret)
    {
        SET_CODE(&res_info->code, ret);
        jpf_warning("<JpfModPu> puid:%s No such guest.", res_info->puid);
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_get_serial_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetSerialPara *req_info;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_pu_backward(self, msg, req_info->puid, req_info->session);
}


NmpMsgFunRet
nmp_mod_pu_set_serial_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_set_device_info_resp(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_serial_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfSetSerialPara *req_info;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_pu_backward(self, msg, req_info->puid, req_info->session);
}


NmpMsgFunRet
nmp_mod_pu_get_move_detection_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetMoveAlarmParaRes *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_pu_forward(self, msg, BUSSLOT_POS_CU);
    if (ret)
    {
        SET_CODE(&res_info->code, ret);
        jpf_warning("<JpfModPu> puid:%16s No such guest.", res_info->guid);
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_get_move_detection_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_get_device_channel_para(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_move_detection_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_set_device_para_resp(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_move_detection_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfSetMoveAlarmPara *req_info;
    gchar puid[MAX_ID_LEN] = {0};
    gint channel = 0;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    nmp_mod_pu_get_puid_from_guid(req_info->guid, puid, &channel);

    return nmp_mod_pu_backward(self, msg, puid, req_info->session);
}


NmpMsgFunRet
nmp_mod_pu_get_video_lost_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetVideoLostParaRes *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_pu_forward(self, msg, BUSSLOT_POS_CU);
    if (ret)
    {
        SET_CODE(&res_info->code, ret);
        jpf_warning("<JpfModPu> puid:%16s No such guest.", res_info->guid);
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_get_video_lost_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_get_device_channel_para(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_video_lost_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_set_device_para_resp(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_video_lost_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfSetVideoLostPara *req_info;
    gchar puid[MAX_ID_LEN] = {0};
    gint channel = 0;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    nmp_mod_pu_get_puid_from_guid(req_info->guid, puid, &channel);

    return nmp_mod_pu_backward(self, msg, puid, req_info->session);
}


NmpMsgFunRet
nmp_mod_pu_get_hide_alarm_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetHideAlarmParaRes *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_pu_forward(self, msg, BUSSLOT_POS_CU);
    if (ret)
    {
        SET_CODE(&res_info->code, ret);
        jpf_warning("<JpfModPu> puid:%16s No such guest.", res_info->guid);
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_get_hide_alarm_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_get_device_channel_para(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_hide_alarm_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_set_device_para_resp(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_hide_alarm_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfSetHideAlarmPara *req_info;
    gchar puid[MAX_ID_LEN] = {0};
    gint channel = 0;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    nmp_mod_pu_get_puid_from_guid(req_info->guid, puid, &channel);

    return nmp_mod_pu_backward(self, msg, puid, req_info->session);
}


NmpMsgFunRet
nmp_mod_pu_get_io_alarm_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetIOAlarmParaRes *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_pu_forward(self, msg, BUSSLOT_POS_CU);
    if (ret)
    {
        SET_CODE(&res_info->code, ret);
        jpf_warning("<JpfModPu> puid:%16s No such guest.", res_info->guid);
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_get_io_alarm_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_get_device_channel_para(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_io_alarm_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_set_device_para_resp(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_io_alarm_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfSetIOAlarmPara *req_info;
    gchar puid[MAX_ID_LEN] = {0};
    gint channel = 0;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    nmp_mod_pu_get_puid_from_guid(req_info->guid, puid, &channel);

    return nmp_mod_pu_backward(self, msg, puid, req_info->session);
}



NmpMsgFunRet
nmp_mod_pu_get_joint_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetJointParaRes *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_pu_forward(self, msg, BUSSLOT_POS_CU);
    if (ret)
    {
        SET_CODE(&res_info->code, ret);
        jpf_warning("<JpfModPu> puid:%16s No such guest.", res_info->guid);
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_get_joint_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetJointPara *req_info;
    gchar puid[MAX_ID_LEN] = {0};
	 gint channel = 0;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    nmp_mod_pu_get_puid_from_guid(req_info->guid, puid, &channel);

    return nmp_mod_pu_backward(self, msg, puid, req_info->session);
}


NmpMsgFunRet
nmp_mod_pu_set_joint_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_set_device_para_resp(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_joint_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfSetJointPara *req_info;
    gchar puid[MAX_ID_LEN] = {0};
    gint channel = 0;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    nmp_mod_pu_get_puid_from_guid(req_info->guid, puid, &channel);

    return nmp_mod_pu_backward(self, msg, puid, req_info->session);
}


NmpMsgFunRet
nmp_mod_pu_get_ptz_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetPtzParaRes *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_pu_forward(self, msg, BUSSLOT_POS_CU);
    if (ret)
    {
        SET_CODE(&res_info->code, ret);
        jpf_warning("<JpfModPu> puid:%16s No such guest.", res_info->guid);
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_get_ptz_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_get_device_channel_para(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_ptz_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_set_device_para_resp(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_ptz_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfSetPtzPara *req_info;
    gchar puid[MAX_ID_LEN] = {0};
    gint channel = 0;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    nmp_mod_pu_get_puid_from_guid(req_info->guid, puid, &channel);

    return nmp_mod_pu_backward(self, msg, puid, req_info->session);
}


NmpMsgFunRet
nmp_mod_pu_control_ptz_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_set_device_para_resp(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_control_ptz_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfControlPtz *req_info;
    gchar puid[MAX_ID_LEN] = {0};
    gint channel = 0, ret;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    nmp_mod_pu_get_puid_from_guid(req_info->guid, puid, &channel);
   // return nmp_mod_pu_backward(self, msg, puid, req_info->session);

    JpfGuestBase *pu_base;
    JpfNetIO *io;
    gint msg_id;
    JpfGetDeviceInfoErr      code;

    msg_id = MSG_GETID(msg);
    memset(&code, 0, sizeof(code));
    pu_base = jpf_mods_container_get_guest_2(self->container, puid);
    if (G_UNLIKELY(!pu_base))
    {
        jpf_warning("<JpfModPu> deliver msg '%s' failed, puid:%s no such pu.",
            MESSAGE_ID_TO_STR(cms, msg_id), puid);
        ret = -E_NOSUCHGUEST;
        goto err_control_ptz;
    }

    JpfPu *pu;
    pu = (JpfPu*)pu_base;
    ret = nmp_mod_ctl_resource(&pu->res_ctl, RESOURCE_PTZ, req_info->rank);
    if (ret)
    {
        jpf_mods_container_put_guest(self->container, pu_base);
        goto err_control_ptz;
    }

    io = IO_OF_GUEST(pu_base);
    BUG_ON(!io);
    jpf_sysmsg_attach_io(msg, io);
    jpf_mods_container_put_guest(self->container, pu_base);

    return MFR_DELIVER_AHEAD;
err_control_ptz:
    SET_CODE(&code, ret);
    memcpy(code.session, req_info->session, SESSION_ID_LEN - 1);
    MSG_SET_DSTPOS(msg, BUSSLOT_POS_CU);
    jpf_sysmsg_set_private_2(msg, &code, sizeof(code));
    MSG_SET_RESPONSE(msg);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_pu_get_preset_point_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetPresetPointRes *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_pu_forward(self, msg, BUSSLOT_POS_CU);
    if (ret)
    {
        SET_CODE(&res_info->code, -ret);
        jpf_warning("<JpfModPu> puid:%16s No such guest.", res_info->guid);
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_get_preset_point_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_get_device_channel_para(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_preset_point_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_set_device_para_resp(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_preset_point_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfSetPresetPoint *req_info;
    gchar puid[MAX_ID_LEN] = {0};
    gint channel = 0;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    nmp_mod_pu_get_puid_from_guid(req_info->guid, puid, &channel);

    return nmp_mod_pu_backward(self, msg, puid, req_info->session);
}


NmpMsgFunRet
nmp_mod_pu_get_cruise_way_set_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetCruiseWaySetRes *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_pu_forward(self, msg, BUSSLOT_POS_CU);
    if (ret)
    {
        SET_CODE(&res_info->code, -ret);
        jpf_warning("<JpfModPu> puid:%16s No such guest.", res_info->guid);
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_get_cruise_way_set_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_get_device_channel_para(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_get_cruise_way_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetCruiseWayRes *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_pu_forward(self, msg, BUSSLOT_POS_CU);
    if (ret)
    {
        SET_CODE(&res_info->code, -ret);
        jpf_warning("<JpfModPu> puid:%16s No such guest.", res_info->guid);
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_get_cruise_way_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_get_device_channel_para(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_add_cruise_way_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_set_device_para_resp(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_add_cruise_way_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfAddCruiseWay *req_info;
    gchar puid[MAX_ID_LEN] = {0};
    gint channel = 0;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    nmp_mod_pu_get_puid_from_guid(req_info->guid, puid, &channel);

    return nmp_mod_pu_backward(self, msg, puid, req_info->session);
}


NmpMsgFunRet
nmp_mod_pu_modify_cruise_way_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_set_device_para_resp(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_modify_cruise_way_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfModifyCruiseWay *req_info;
    gchar puid[MAX_ID_LEN] = {0};
    gint channel = 0;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    nmp_mod_pu_get_puid_from_guid(req_info->guid, puid, &channel);

    return nmp_mod_pu_backward(self, msg, puid, req_info->session);
}


NmpMsgFunRet
nmp_mod_pu_set_cruise_way_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_set_device_para_resp(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_cruise_way_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfSetCruiseWay *req_info;
    gchar puid[MAX_ID_LEN] = {0};
    gint channel = 0;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    nmp_mod_pu_get_puid_from_guid(req_info->guid, puid, &channel);

    return nmp_mod_pu_backward(self, msg, puid, req_info->session);
}


NmpMsgFunRet
nmp_mod_pu_3D_control_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_set_device_para_resp(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_3D_control_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    Jpf3DControl *req_info;
    gchar puid[MAX_ID_LEN] = {0};
    gint channel = 0;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    nmp_mod_pu_get_puid_from_guid(req_info->guid, puid, &channel);

    return nmp_mod_pu_backward(self, msg, puid, req_info->session);
}


NmpMsgFunRet
nmp_mod_pu_3D_goback_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_set_device_para_resp(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_3D_goback_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_get_device_channel_para(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_get_device_time_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetDeviceTimeRes *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_pu_forward(self, msg, BUSSLOT_POS_CU);
    if (ret)
    {
        SET_CODE(&res_info->code, -ret);
        jpf_warning("<JpfModPu> puid:%s No such guest.", res_info->puid);
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_get_device_time_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_get_device_info(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_device_time_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_set_device_info_resp(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_device_time_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfSetDeviceTime *req_info;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    //nmp_mod_pu_get_puid_from_guid(req_info->puid, puid, &channel);
		printf("-------nmp_mod_pu_set_device_time_b, puid=%s,session=%s\n",
		req_info->puid, req_info->session);
    return nmp_mod_pu_backward(self, msg, req_info->puid, req_info->session);
}


NmpMsgFunRet
nmp_mod_pu_get_ntp_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetNTPInfoRes *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_pu_forward(self, msg, BUSSLOT_POS_CU);
    if (ret)
    {
        SET_CODE(&res_info->code, -ret);
        jpf_warning("<JpfModPu> puid:%s No such guest.", res_info->puid);
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_get_ntp_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_get_device_info(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_ntp_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_set_device_info_resp(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_ntp_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfSetNTPInfo *req_info;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    //nmp_mod_pu_get_puid_from_guid(req_info->puid, puid, &channel);

    return nmp_mod_pu_backward(self, msg, req_info->puid, req_info->session);
}


NmpMsgFunRet
nmp_mod_pu_get_ftp_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetFtpParaRes *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_pu_forward(self, msg, BUSSLOT_POS_CU);
    if (ret)
    {
        SET_CODE(&res_info->code, -ret);
        jpf_warning("<JpfModPu> puid:%s No such guest.", res_info->puid);
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_get_ftp_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_get_device_info(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_ftp_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_set_device_info_resp(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_ftp_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfSetFtpPara *req_info;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_pu_backward(self, msg, req_info->puid, req_info->session);
}


NmpMsgFunRet
nmp_mod_pu_get_smtp_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetSmtpParaRes *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_pu_forward(self, msg,BUSSLOT_POS_CU);
    if (ret)
    {
        SET_CODE(&res_info->code, -ret);
        jpf_warning("<JpfModPu> puid:%s No such guest.", res_info->puid);
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_get_smtp_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_get_device_info(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_smtp_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_set_device_info_resp(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_smtp_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfSetSmtpPara *req_info;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_pu_backward(self, msg, req_info->puid, req_info->session);
}


NmpMsgFunRet
nmp_mod_pu_get_upnp_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetUpnpParaRes *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_pu_forward(self, msg,BUSSLOT_POS_CU);
    if (ret)
    {
        SET_CODE(&res_info->code, -ret);
        jpf_warning("<JpfModPu> puid:%s No such guest.", res_info->puid);
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_get_upnp_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_get_device_info(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_upnp_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_set_device_info_resp(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_upnp_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfSetUpnpPara *req_info;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_pu_backward(self, msg, req_info->puid, req_info->session);
}


NmpMsgFunRet
nmp_mod_pu_get_transparent_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetTransparentParaRes *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_pu_forward(self, msg, BUSSLOT_POS_CU);
    if (ret)
    {
        SET_CODE(&res_info->code, -ret);
        jpf_warning("<JpfModPu> puid:%s No such guest.", res_info->puid);
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_get_transparent_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_get_device_info(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_transparent_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_set_device_info_resp(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_transparent_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfSetTransparentPara *req_info;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_pu_backward(self, msg, req_info->puid, req_info->session);
}


NmpMsgFunRet
nmp_mod_pu_get_ddns_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetDdnsParaRes *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_pu_forward(self, msg, BUSSLOT_POS_CU);
    if (ret)
    {
        SET_CODE(&res_info->code, -ret);
        jpf_warning("<JpfModPu> puid:%s No such guest.", res_info->puid);
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_get_ddns_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_get_device_info(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_ddns_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_set_device_info_resp(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_ddns_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfSetDdnsPara *req_info;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_pu_backward(self, msg, req_info->puid, req_info->session);
}




NmpMsgFunRet
nmp_mod_pu_get_disk_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetDiskInfoRes *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_pu_forward(self, msg, BUSSLOT_POS_CU);
    if (ret)
    {
        SET_CODE(&res_info->code, -ret);
        jpf_warning("<JpfModPu> puid:%s No such guest.", res_info->puid);
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_get_disk_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_get_device_info(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_get_resolution_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetResolutionInfoRes *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_pu_forward(self, msg,BUSSLOT_POS_CU);
    if (ret)
    {
        SET_CODE(&res_info->code, -ret);
        jpf_warning("<JpfModPu> puid:%16s No such guest.", res_info->guid);
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_get_resolution_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_get_device_channel_para(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_resolution_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_set_device_para_resp(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_resolution_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfSetResolutionInfo *req_info;
    gchar puid[MAX_ID_LEN] = {0};
    gint channel = 0;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    nmp_mod_pu_get_puid_from_guid(req_info->guid, puid, &channel);

    return nmp_mod_pu_backward(self, msg, puid, req_info->session);
}


NmpMsgFunRet
nmp_mod_pu_get_ircut_control_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetIrcutControlInfoRes *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_pu_forward(self, msg, BUSSLOT_POS_CU);
    if (ret)
    {
        SET_CODE(&res_info->code, -ret);
        jpf_warning("<JpfModPu> puid:%16s No such guest.", res_info->guid);
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_get_ircut_control_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_get_device_channel_para(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_ircut_control_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_set_device_para_resp(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_set_ircut_control_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfSetIrcutControlInfo *req_info;
    gchar puid[MAX_ID_LEN] = {0};
    gint channel = 0;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    nmp_mod_pu_get_puid_from_guid(req_info->guid, puid, &channel);

    return nmp_mod_pu_backward(self, msg, puid, req_info->session);
}



NmpMsgFunRet
nmp_mod_pu_format_disk_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_set_device_info_resp(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_format_disk_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfFormatDisk *req_info;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_pu_backward(self, msg, req_info->puid, req_info->session);
}


NmpMsgFunRet
nmp_mod_pu_submit_format_pos_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfNetIO *io;
    JpfGuestBase *pu_base;
    JpfSubmitFormatPos *req_info;
    NmpMsgID msg_id;

    self = (JpfModPu*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    pu_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!pu_base))
    {
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
     }

    MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
    jpf_mods_container_put_guest(self->container, pu_base);

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_submit_alarm_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfNetIO *io;
    JpfGuestBase *pu_base;
    JpfSubmitAlarm *req_info;
    NmpMsgID msg_id;
    JpfResourcesCap res_cap;

    self = (JpfModPu*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    pu_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!pu_base))
    {
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
     }

    memset(&res_cap, 0, sizeof(res_cap));
    nmp_mod_get_resource_cap(&res_cap);
    if (!(res_cap.module_bits&MODULE_ALM_BIT))
    {
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    strncpy(req_info->domain_id, jpf_get_local_domain_id(), DOMAIN_ID_LEN - 1);
    MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
    jpf_mods_container_put_guest(self->container, pu_base);

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_get_store_log_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetStoreLogRes *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_pu_forward(self, msg, BUSSLOT_POS_CU);
    if (ret)
    {
        SET_CODE(&res_info->code, -ret);
        jpf_warning("<JpfModPu> puid:%16s No such guest.", res_info->guid);
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_get_store_log_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetStoreLog *req_info;
    gchar puid[MAX_ID_LEN] = {0};
    gint channel = 0;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    nmp_mod_pu_get_puid_from_guid(req_info->guid, puid, &channel);

    return nmp_mod_pu_backward(self, msg, puid, req_info->session);
}


NmpMsgFunRet
nmp_mod_pu_firmware_upgrade_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_set_device_info_resp(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_firmware_upgrade_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfPuUpgrade *req_info;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_pu_backward(self, msg, req_info->puid, req_info->session);
}


NmpMsgFunRet
nmp_mod_pu_control_device_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_pu_set_device_info_resp(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_pu_control_device_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfControlDevice *req_info;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_pu_backward(self, msg, req_info->puid, req_info->session);
}


NmpMsgFunRet
nmp_mod_pu_query_div_mode_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfNetIO *io;
    JpfGuestBase *pu_base;
    JpfPuGetDivMode *req_info;
    JpfGetDivModeRes res_info;
    NmpMsgID msg_id;
    gint ret = 0;

    self = (JpfModPu*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    pu_base = jpf_mods_container_get_guest(self->container, io);
    if (G_LIKELY(pu_base))
    {
        MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
        jpf_mods_container_put_guest(self->container, pu_base);
	 return MFR_DELIVER_AHEAD;
    }

    ret = -E_NOSUCHGUEST;
    jpf_warning("<JpfModPu> puid:%s No such puid.", req_info->puid);

    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info.code, -ret);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_pu_query_div_mode_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetDivModeRes *res_info;
    JpfGuestBase *pu_base;
    JpfNetIO *io;
    gint msg_id;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    msg_id = MSG_GETID(msg);
    pu_base = jpf_mods_container_get_guest_2(self->container, res_info->session);
    if (G_UNLIKELY(!pu_base))
    {
        jpf_warning("<JpfModPu> deliver msg '%s' failed, puid:%s no such pu.",
            MESSAGE_ID_TO_STR(cms, msg_id), res_info->session);
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    io = IO_OF_GUEST(pu_base);
    BUG_ON(!io);
    jpf_sysmsg_attach_io(msg, io);
    jpf_mods_container_put_guest(self->container, pu_base);

    if (RES_CODE(res_info))
    {
        nmp_app_obj_deliver_in((NmpAppObj*)self, msg);
      	 nmp_mod_acc_release_io((JpfModAccess*)self, io);
      	 nmp_mod_container_del_io(self->container, io);
	 return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_get_screen_state_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetScrStateRes *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    MSG_SET_RESPONSE(msg);
    ret = nmp_mod_pu_forward(self, msg, BUSSLOT_POS_CU);
    if (ret)
    {
        SET_CODE(&res_info->code, -ret);
        jpf_warning("<JpfModPu> puid No such guest.");
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_get_screen_state_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfGetScrState *req_info;
    gchar puid[MAX_ID_LEN] = {0};

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    jpf_get_puid_from_guid(req_info->guid.guid, puid);

    return nmp_mod_pu_backward(self, msg, puid, req_info->session);
}


NmpMsgFunRet
nmp_mod_pu_change_div_mode_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfCuExecuteRes *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    MSG_SET_RESPONSE(msg);
    ret = nmp_mod_pu_forward(self, msg, BUSSLOT_POS_TW);
    if (ret)
    {
        SET_CODE(&res_info->code, -ret);
        jpf_warning("<JpfModPu> puid: No such guest.");
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_change_div_mode_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    tw_operate_to_decoder *req_info;
    gchar puid[MAX_ID_LEN] = {0};

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    jpf_get_puid_from_guid(req_info->dis_guid, puid);

   return nmp_mod_pu_backward_2(self, msg, puid, BUSSLOT_POS_TW);
}


NmpMsgFunRet
nmp_mod_pu_full_screen_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfCuExecuteRes *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    MSG_SET_RESPONSE(msg);
    ret = nmp_mod_pu_forward(self, msg, BUSSLOT_POS_TW);
    if (ret)
    {
        SET_CODE(&res_info->code, -ret);
        jpf_warning("<JpfModPu> puid: No such guest.");
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_full_screen_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    tw_operate_to_decoder *req_info;
    gchar puid[MAX_ID_LEN] = {0};

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    jpf_get_puid_from_guid(req_info->dis_guid, puid);

    return nmp_mod_pu_backward_2(self, msg, puid, BUSSLOT_POS_TW);
}


NmpMsgFunRet
nmp_mod_pu_clear_division_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfCuExecuteRes *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    MSG_SET_RESPONSE(msg);
    ret = nmp_mod_pu_forward(self, msg, BUSSLOT_POS_TW);
    if (ret)
    {
        SET_CODE(&res_info->code, -ret);
        jpf_warning("<JpfModPu> puid: No such guest.");
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_clear_division_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    tw_operate_to_decoder *req_info;
    gchar puid[MAX_ID_LEN] = {0};

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    jpf_get_puid_from_guid(req_info->dis_guid, puid);

    return nmp_mod_pu_backward_2(self, msg, puid, BUSSLOT_POS_TW);
}


NmpMsgFunRet
nmp_mod_pu_exit_full_screen_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfCuExecuteRes *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    MSG_SET_RESPONSE(msg);
    ret = nmp_mod_pu_forward(self, msg, BUSSLOT_POS_TW);
    if (ret)
    {
        SET_CODE(&res_info->code, -ret);
        jpf_warning("<JpfModPu> puid: No such guest.");
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_exit_full_screen_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    tw_operate_to_decoder *req_info;
    gchar puid[MAX_ID_LEN] = {0};

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    jpf_get_puid_from_guid(req_info->dis_guid, puid);

    return nmp_mod_pu_backward_2(self, msg, puid, BUSSLOT_POS_TW);
}


NmpMsgFunRet
nmp_mod_pu_tw_play_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    tw_decoder_rsp *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = nmp_mod_pu_forward(self, msg, BUSSLOT_POS_TW);
    if (ret)
    {
        res_info->result = -ret;
        jpf_warning("<JpfModPu> puid: No such guest.");
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_tw_play_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    tw_screen_to_decoder *req_info;
    tw_decoder_rsp res_info;
    gchar puid[MAX_ID_LEN] = {0};
    JpfGuestBase *pu_base;
    JpfNetIO *io;
    gint msg_id, div;
    JpfPu *pu;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    jpf_get_puid_from_guid(req_info->dis_guid, puid);
    msg_id = MSG_GETID(msg);
    memset(&res_info, 0, sizeof(res_info));
    pu_base = jpf_mods_container_get_guest_2(self->container, puid);
    if (G_UNLIKELY(!pu_base))
    {
        jpf_warning("<JpfModPu> deliver msg '%s' failed, puid:%s no such pu.",
            MESSAGE_ID_TO_STR(cms, msg_id), puid);

        res_info.result = -E_NOSUCHGUEST;
        goto tw_play_faild;
    }

    pu = (JpfPu *)pu_base;
    if (pu->state != STAT_PU_ONLINE)
    {
        res_info.result = -E_NOSUCHGUEST;
        jpf_mods_container_put_guest(self->container, pu_base);
        goto tw_play_faild;
    }

    io = IO_OF_GUEST(pu_base);
    BUG_ON(!io);
    jpf_sysmsg_attach_io(msg, io);
    jpf_mods_container_put_guest(self->container, pu_base);

    return MFR_DELIVER_AHEAD;

tw_play_faild:
    for (div = 0; div < req_info->div_sum; div++)
    {
        res_info.divisions[div].division_num = req_info->divisions[div].division_num;
        strcpy(res_info.divisions[div].ec_name, req_info->divisions[div].ec_name);
        res_info.divisions[div].result = -1;
    }

    MSG_SET_DSTPOS(msg, BUSSLOT_POS_TW);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
    MSG_SET_RESPONSE(msg);
    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_pu_alarm_link_io_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfAmsActionIORes *res_info;
    gint ret;

    self = (JpfModPu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (strcmp(res_info->session, jpf_get_local_domain_id()) == 0)
    {
		jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    ret = nmp_mod_pu_forward(self, msg,BUSSLOT_POS_CU);
    if (ret)
    {
        SET_CODE(&res_info->code, -ret);
        jpf_warning("<JpfModPu> puid:%16s No such guest.", res_info->guid);
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_pu_alarm_link_io_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfAmsActionIO *req_info;
    gchar puid[MAX_ID_LEN] = {0};
    gint channel = 0;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    nmp_mod_pu_get_puid_from_guid(req_info->action_guid.guid, puid, &channel);

    return nmp_mod_pu_backward(self, msg, puid, req_info->session);
}


NmpMsgFunRet
nmp_mod_pu_alarm_link_preset_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModPu *self;
    JpfAmsActionPreset *req_info;
    gchar puid[MAX_ID_LEN] = {0};
    gint channel = 0;

    self = (JpfModPu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    nmp_mod_pu_get_puid_from_guid(req_info->action_guid.guid, puid, &channel);

    return nmp_mod_pu_backward(self, msg, puid, req_info->session);
}


NmpMsgFunRet
nmp_mod_pu_set_recheck_tag_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	JpfModPu *self;

	self = (JpfModPu*)app_obj;

	nmp_mod_pu_set_recheck_tag(self);

	jpf_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}



void
nmp_mod_pu_register_msg_handler(JpfModPu *self)
{
    NmpAppMod *super_self = (NmpAppMod*)self;

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_PU_REGISTER,
        nmp_mod_pu_register_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
    	super_self,
    	MESSAGE_PU_HEART,
    	nmp_mod_pu_heart_f,
    	nmp_mod_pu_heart_b,
    	0
    );

    nmp_app_mod_register_msg(
    	super_self,
    	MESSAGE_SUBMIT_ALARM,
    	nmp_mod_pu_submit_alarm_f,
    	NULL,
    	0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_CHANGE_DISPATCH,
        NULL,
        nmp_mod_pu_change_dispatch_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_PLATFORM_INFO,
        nmp_mod_pu_get_platform_info_f,
        nmp_mod_pu_get_platform_info_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_PLATFORM_INFO,
        nmp_mod_pu_set_platform_info_f,
        nmp_mod_pu_set_platform_info_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_DEVICE_INFO,
        nmp_mod_pu_get_device_info_f,
        nmp_mod_pu_get_device_info_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_NETWORK_INFO,
        nmp_mod_pu_get_network_info_f,
        nmp_mod_pu_get_network_info_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_NETWORK_INFO,
        nmp_mod_pu_set_network_info_f,
        nmp_mod_pu_set_network_info_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_PPPOE_INFO,
        nmp_mod_pu_get_pppoe_para_f,
        nmp_mod_pu_get_pppoe_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_PPPOE_INFO,
        nmp_mod_pu_set_pppoe_para_f,
        nmp_mod_pu_set_pppoe_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_MEDIA_URL,
        nmp_mod_pu_get_media_url_f,
        nmp_mod_pu_get_media_url_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_ENCODE_PARA,
        nmp_mod_pu_get_encode_para_f,
        nmp_mod_pu_get_encode_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_ENCODE_PARA,
        nmp_mod_pu_set_encode_para_f,
        nmp_mod_pu_set_encode_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_DISPLAY_PARA,
        nmp_mod_pu_get_display_para_f,
        nmp_mod_pu_get_display_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_DEF_DISPLAY_PARA,
        nmp_mod_pu_get_display_para_f,
        nmp_mod_pu_get_display_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_DISPLAY_PARA,
        nmp_mod_pu_set_display_para_f,
        nmp_mod_pu_set_display_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_OSD_PARA,
        nmp_mod_pu_get_OSD_para_f,
        nmp_mod_pu_get_OSD_para_b,
        0
    );

	nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_OSD_PARA,
        nmp_mod_pu_set_OSD_para_f,
        nmp_mod_pu_set_OSD_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_MOVE_DETECT,
        nmp_mod_pu_get_move_detection_f,
        nmp_mod_pu_get_move_detection_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_MOVE_DETECT,
        nmp_mod_pu_set_move_detection_f,
        nmp_mod_pu_set_move_detection_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_VIDEO_LOST,
        nmp_mod_pu_get_video_lost_f,
        nmp_mod_pu_get_video_lost_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_VIDEO_LOST,
        nmp_mod_pu_set_video_lost_f,
        nmp_mod_pu_set_video_lost_b,
        0
    );

   nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_HIDE_PARA,
        nmp_mod_pu_get_hide_para_f,
        nmp_mod_pu_get_hide_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_HIDE_PARA,
        nmp_mod_pu_set_hide_para_f,
        nmp_mod_pu_set_hide_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_HIDE_ALARM,
        nmp_mod_pu_get_hide_alarm_f,
        nmp_mod_pu_get_hide_alarm_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_HIDE_ALARM,
        nmp_mod_pu_set_hide_alarm_f,
        nmp_mod_pu_set_hide_alarm_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_IO_ALARM,
        nmp_mod_pu_get_io_alarm_f,
        nmp_mod_pu_get_io_alarm_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_IO_ALARM,
        nmp_mod_pu_set_io_alarm_f,
        nmp_mod_pu_set_io_alarm_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_RECORD_PARA,
        nmp_mod_pu_get_record_para_f,
        nmp_mod_pu_get_record_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_RECORD_PARA,
        nmp_mod_pu_set_record_para_f,
        nmp_mod_pu_set_record_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_JOINT_PARA,
        nmp_mod_pu_get_joint_para_f,
        nmp_mod_pu_get_joint_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_JOINT_PARA,
        nmp_mod_pu_set_joint_para_f,
        nmp_mod_pu_set_joint_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_PTZ_PARA,
        nmp_mod_pu_get_ptz_para_f,
        nmp_mod_pu_get_ptz_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_PTZ_PARA,
        nmp_mod_pu_set_ptz_para_f,
        nmp_mod_pu_set_ptz_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_CONTROL_PTZ,
        nmp_mod_pu_control_ptz_f,
        nmp_mod_pu_control_ptz_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_PRESET_POINT,
        nmp_mod_pu_get_preset_point_f,
        nmp_mod_pu_get_preset_point_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_PRESET_POINT,
        nmp_mod_pu_set_preset_point_f,
        nmp_mod_pu_set_preset_point_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_CRUISE_WAY_SET,
        nmp_mod_pu_get_cruise_way_set_f,
        nmp_mod_pu_get_cruise_way_set_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_CRUISE_WAY,
        nmp_mod_pu_get_cruise_way_f,
        nmp_mod_pu_get_cruise_way_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_CRUISE_WAY,
        nmp_mod_pu_add_cruise_way_f,
        nmp_mod_pu_add_cruise_way_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_CRUISE_WAY,
        nmp_mod_pu_modify_cruise_way_f,
        nmp_mod_pu_modify_cruise_way_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_CRUISE_WAY,
        nmp_mod_pu_set_cruise_way_f,
        nmp_mod_pu_set_cruise_way_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_3D_CONTROL,
        nmp_mod_pu_3D_control_f,
        nmp_mod_pu_3D_control_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_3D_GOBACK,
        nmp_mod_pu_3D_goback_f,
        nmp_mod_pu_3D_goback_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_SERIAL_PARA,
        nmp_mod_pu_get_serial_para_f,
        nmp_mod_pu_get_serial_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_SERIAL_PARA,
        nmp_mod_pu_set_serial_para_f,
        nmp_mod_pu_set_serial_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_DEVICE_TIME,
        nmp_mod_pu_get_device_time_f,
        nmp_mod_pu_get_device_time_b,
        0
    );


    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_DEVICE_TIME,
        nmp_mod_pu_set_device_time_f,
        nmp_mod_pu_set_device_time_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_NTP_INFO,
        nmp_mod_pu_get_ntp_info_f,
        nmp_mod_pu_get_ntp_info_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_NTP_INFO,
        nmp_mod_pu_set_ntp_info_f,
        nmp_mod_pu_set_ntp_info_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_FTP_PARA,
        nmp_mod_pu_get_ftp_para_f,
        nmp_mod_pu_get_ftp_para_b,
        0
    );


    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_FTP_PARA,
        nmp_mod_pu_set_ftp_para_f,
        nmp_mod_pu_set_ftp_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_SMTP_PARA,
        nmp_mod_pu_get_smtp_para_f,
        nmp_mod_pu_get_smtp_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_SMTP_PARA,
        nmp_mod_pu_set_smtp_para_f,
        nmp_mod_pu_set_smtp_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_UPNP_PARA,
        nmp_mod_pu_get_upnp_para_f,
        nmp_mod_pu_get_upnp_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_UPNP_PARA,
        nmp_mod_pu_set_upnp_para_f,
        nmp_mod_pu_set_upnp_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_TRANSPARENT_PARA,
        nmp_mod_pu_get_transparent_para_f,
        nmp_mod_pu_get_transparent_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_TRANSPARENT_PARA,
        nmp_mod_pu_set_transparent_para_f,
        nmp_mod_pu_set_transparent_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_DDNS_PARA,
        nmp_mod_pu_get_ddns_para_f,
        nmp_mod_pu_get_ddns_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_DDNS_PARA,
        nmp_mod_pu_set_ddns_para_f,
        nmp_mod_pu_set_ddns_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_DISK_INFO,
        nmp_mod_pu_get_disk_info_f,
        nmp_mod_pu_get_disk_info_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_RESOLUTION_INFO,
        nmp_mod_pu_get_resolution_info_f,
        nmp_mod_pu_get_resolution_info_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_RESOLUTION_INFO,
        nmp_mod_pu_set_resolution_info_f,
        nmp_mod_pu_set_resolution_info_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_IRCUTCONTROL_INFO,
        nmp_mod_pu_get_ircut_control_info_f,
        nmp_mod_pu_get_ircut_control_info_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_IRCUTCONTROL_INFO,
        nmp_mod_pu_set_ircut_control_info_f,
        nmp_mod_pu_set_ircut_control_info_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_FORMAT_DISK,
        nmp_mod_pu_format_disk_f,
        nmp_mod_pu_format_disk_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SUBMIT_FORMAT_POS,
        nmp_mod_pu_submit_format_pos_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_STORE_LOG,
        nmp_mod_pu_get_store_log_f,
        nmp_mod_pu_get_store_log_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_FIRMWARE_UPGRADE,
        nmp_mod_pu_firmware_upgrade_f,
        nmp_mod_pu_firmware_upgrade_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_CONTROL_DEVICE,
        nmp_mod_pu_control_device_f,
        nmp_mod_pu_control_device_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_MDS_INFO,
        nmp_mod_pu_get_mds_info_f,
        nmp_mod_pu_get_mds_info_b,
        0
    );

   nmp_app_mod_register_msg(
        super_self,
        MESSAGE_PU_GET_DIV_MODE,
        nmp_mod_pu_query_div_mode_f,
        nmp_mod_pu_query_div_mode_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_SCR_STATE,
        nmp_mod_pu_get_screen_state_f,
        nmp_mod_pu_get_screen_state_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_CHANGE_DIV_MODE,
        nmp_mod_pu_change_div_mode_f,
        nmp_mod_pu_change_div_mode_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_FULL_SCREEN,
        nmp_mod_pu_full_screen_f,
        nmp_mod_pu_full_screen_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_EXIT_FULL_SCREEN,
        nmp_mod_pu_exit_full_screen_f,
        nmp_mod_pu_exit_full_screen_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_TW_CLEAR_DIVISION,
        nmp_mod_pu_clear_division_f,
        nmp_mod_pu_clear_division_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_TW_PLAY,
        nmp_mod_pu_tw_play_f,
        nmp_mod_pu_tw_play_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ALARM_LINK_IO,
        nmp_mod_pu_alarm_link_io_f,
        nmp_mod_pu_alarm_link_io_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ALARM_LINK_PRESET,
        NULL,
        nmp_mod_pu_alarm_link_preset_b,
        0
    );

	nmp_app_mod_register_msg(
		super_self,
		MSG_PU_RECHECK,
		NULL,
		nmp_mod_pu_set_recheck_tag_b,
		0
	);

}

