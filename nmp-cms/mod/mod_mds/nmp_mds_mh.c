#include "nmp_mod_mds.h"
#include "nmp_message.h"
#include "nmp_errno.h"
#include "nmp_memory.h"
#include "nmp_msg_mds.h"
#include "nmp_internal_msg.h"
#include "nmp_shared.h"
#include "nmp_mds_struct.h"

//static guint msg_seq_generator = 0;

static __inline__ gint
jpf_mod_mds_mds_register(JpfModMds *self, JpfNetIO *io,  JpfMsgID msg_id,
    JpfMdsRegister *req, JpfMdsRegisterRes *res)
{
    gint ret;
    JpfID conflict;

    G_ASSERT(self != NULL && io != NULL && req != NULL && res != NULL);

    ret = jpf_mod_mds_new_mds(self, io,  req->mds_id, &conflict);
    if (G_UNLIKELY(ret))
        return ret;

    ret = jpf_mod_mds_sync_req(self, msg_id, req,
         sizeof(JpfMdsRegister), res, sizeof(JpfMdsRegisterRes));

    return ret;
}


JpfMsgFunRet
jpf_mod_mds_register_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModMds *self;
    JpfNetIO *io;
    JpfMdsRegister *req_info;
    JpfMdsRegisterRes res_info;
    JpfMsgID msg_id;
    JpfGuestBase *mds_base;
    JpfMds *mds;
    JpfMsgMdsOnlineChange notify_info;
    gint ret;
    JpfResourcesCap res_cap;

    self = (JpfModMds*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_cap, 0, sizeof(res_cap));
    jpf_mod_get_resource_cap(&res_cap);
    if (!(res_cap.module_bits&MODULE_MDS_BIT))
    {
        ret = E_EXPIRED;
        goto mds_register_err;
    }
    memset(&notify_info, 0, sizeof(notify_info));
    notify_info.mds_id[MDS_ID_LEN - 1] = 0;
    strncpy(notify_info.mds_id, req_info->mds_id, MDS_ID_LEN - 1);
    memset(&res_info, 0, sizeof(res_info));
    jpf_get_ip_from_socket(io, req_info->mds_ip);
    ret = jpf_mod_mds_mds_register(self, io, msg_id, req_info, &res_info);
mds_register_err:
    if (ret)
    {
        jpf_print(
            "<JpfModMds> mds:%s register failed, err:%d",
            req_info->mds_id, -ret
        );

        SET_CODE(&res_info, -ret);
        MSG_SET_RESPONSE(msg);
        jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

        jpf_app_obj_deliver_in((JpfAppObj*)self, msg);
        jpf_mod_acc_release_io((JpfModAccess*)self, io);
        jpf_mod_container_del_io(self->container, io);

        return MFR_ACCEPTED;
    }

    mds_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!mds_base))
    {
        ret = -E_NOSUCHGUEST;
	 jpf_warning("<JpfModMds> Can't find mds:'%s' in container, io timeout?", notify_info.mds_id);
    }
    else
   {
        mds = (JpfMds *)mds_base;
        mds->mds_state = STAT_MDS_ONLINE;
        jpf_mods_container_put_guest(self->container, mds_base);

         jpf_print(
             "<JpfModMds> mds:%s register ok",
             req_info->mds_id
         );

        strncpy(res_info.domain_id, jpf_get_local_domain_id(), DOMAIN_ID_LEN - 1);

        jpf_check_keepalive_time(&res_info.keep_alive_time);
        jpf_net_io_set_ttd(io, res_info.keep_alive_time*1000*TIMEOUT_N_PERIODS);
        notify_info.new_status = 1;
        jpf_mod_mds_change_mds_online_status(app_obj, notify_info);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
    return MFR_DELIVER_BACK;
}

/*
JpfMsgFunRet
jpf_mod_mds_heart_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModMds *self;
    JpfNetIO *io;
    JpfGuestBase *mds_base;
    JpfMdsHeart *req_info;
    JpfMdsHeartRes res_info;
    JpfMsgID msg_id;
    gint ret = 0;

    self = (JpfModMds*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    mds_base = jpf_mods_container_get_guest_2(self->container, req_info->mds_id);
    if (G_UNLIKELY(!mds_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModMds> mdsId:%s No such guest.", req_info->mds_id);
    }
    else
    {
        jpf_get_utc_time(res_info.server_time);
        printf("res_info.server_time=%s\n",res_info.server_time);
        jpf_mods_container_put_guest(self->container, mds_base);
    }

    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}
*/

JpfMsgFunRet
jpf_mod_mds_heart_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModMds *self;
    JpfNetIO *io;
    JpfGuestBase *mds_base;
    JpfMdsHeart *req_info;
    JpfMdsHeartRes res_info;
    JpfMsgID msg_id;
    gint ret = 0;
    JpfResourcesCap res_cap;

    self = (JpfModMds*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_cap, 0, sizeof(res_cap));
    jpf_mod_get_resource_cap(&res_cap);
    if (!(res_cap.module_bits&MODULE_MDS_BIT))
    {
        jpf_app_obj_deliver_in((JpfAppObj*)self, msg);
        jpf_mod_acc_release_io((JpfModAccess*)self, io);
        jpf_mod_container_del_io(self->container, io);
        return MFR_ACCEPTED;
    }

    memset(&res_info, 0, sizeof(res_info));
    mds_base = jpf_mods_container_get_guest(self->container, io);
    if (G_LIKELY(mds_base))
    {
        jpf_get_ip_from_socket(io, req_info->mds_ip);
	  MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
        jpf_mods_container_put_guest(self->container, mds_base);
	 return MFR_DELIVER_AHEAD;
    }

    ret = -E_NOSUCHGUEST;
    jpf_warning("<JpfModMds> mdsId:%s No such mds.", req_info->mds_id);
    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_mds_heart_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModMds *self;
    JpfMdsHeartRes *res_info;
    JpfGuestBase *mds_base;
    JpfNetIO *io;
    gint msg_id;

    self = (JpfModMds*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    msg_id = MSG_GETID(msg);
    mds_base = jpf_mods_container_get_guest_2(self->container, res_info->mds_id);
    if (G_UNLIKELY(!mds_base))
    {
        jpf_warning("<JpfModMds> mds heart error, mdsId:%s no such mds.",
             res_info->mds_id);
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    jpf_get_utc_time(res_info->server_time);
    io = IO_OF_GUEST(mds_base);
    BUG_ON(!io);
    jpf_sysmsg_attach_io(msg, io);
    jpf_mods_container_put_guest(self->container, mds_base);

    if (RES_CODE(res_info))
    {
        jpf_app_obj_deliver_in((JpfAppObj*)self, msg);
      	 jpf_mod_acc_release_io((JpfModAccess*)self, io);
      	 jpf_mod_container_del_io(self->container, io);
	 return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


JpfMsgFunRet
jpf_mod_mds_get_media_url_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfModMds *self;
    JpfMsgGetUrl *req;
    JpfGetMediaUrlRes res;
    JpfGuestBase *mds_base;
    gchar puid[MAX_ID_LEN] = {0};
    gchar tmp[MAX_ID_LEN];
    gint channel, level;
    gint ret = 0;

    req = MSG_GET_DATA(msg);
	memset(&res, 0, sizeof(res));
	strncpy(res.session, req->session, SESSION_ID_LEN - 1);
    self = (JpfModMds*)app_obj;
    mds_base = jpf_mods_container_get_guest_2(self->container, req->mds_id);
    if (G_UNLIKELY(!mds_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModMds> mdsId:%s No such guest.", req->mds_id);
    }
    else
    {
        jpf_mods_container_put_guest(self->container, mds_base);
        sscanf(req->guid, "%16s-%2s-%d-%2d", puid, tmp, &level, &channel);
	 strcpy(res.guid, req->guid);
	 strcpy(res.ip, req->cms_ip);

	 switch (req->media)
	 {
        case MEDIA_VIDEO_BROWSE:
            snprintf(
                res.url, MAX_URL_LEN,
                "rtsp://%s:%d/dev=@%s/media=%d/channel=%d&level=%d",
                req->mds_ip, req->rtsp_port, puid, req->media, channel, level
            );
            break;

        default:
            if (strlen(req->mss_id))
            {
                snprintf(
                    res.url, MAX_URL_LEN,
                    "rtsp://%s:%d/dev=@%s/media=%d",
                    req->mds_ip, req->rtsp_port, req->mss_id, req->media
                 );
             }
             else
             {
                snprintf(
                    res.url, MAX_URL_LEN,
                    "rtsp://%s:%d/dev=@%s/media=%d/channel=%d&level=%d",
                    req->mds_ip, req->rtsp_port, puid, req->media, channel, level
                );
             }
            break;
	 }
    }

    SET_CODE(&res, -ret);
    jpf_sysmsg_set_private_2(msg, &res, sizeof(res));
    printf("jpf_dbs_modify_sysmsg_2 ret = %d,--url= %s\n",ret, res.url);
    MSG_SET_DSTPOS(msg, BUSSLOT_POS_CU);

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_mds_mss_get_route_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfModMds *self;
    JpfMssGetRouteRes *req;
    gint ret = 0;

    req = MSG_GET_DATA(msg);

    self = (JpfModMds*)app_obj;
    jpf_sysmsg_set_private_2(msg, req, sizeof(JpfMssGetRouteRes));
    printf("jpf_dbs_modify_sysmsg_2 ret = %d,--url= %s\n",ret, req->url);
    MSG_SET_DSTPOS(msg, BUSSLOT_POS_MSS);

    return MFR_DELIVER_BACK;
}


void
jpf_mod_mds_register_msg_handler(JpfModMds *self)
{
    JpfAppMod *super_self = (JpfAppMod*)self;

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MDS_REGISTER,
        jpf_mod_mds_register_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MDS_HEART,
        jpf_mod_mds_heart_f,
        jpf_mod_mds_heart_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_GET_MEDIA_URL,
        NULL,
        jpf_mod_mds_get_media_url_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MSS_GET_ROUTE,
        NULL,
        jpf_mod_mds_mss_get_route_b,
        0
    );


}







