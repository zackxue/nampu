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
nmp_mod_mds_mds_register(NmpModMds *self, NmpNetIO *io,  NmpMsgID msg_id,
    NmpMdsRegister *req, NmpMdsRegisterRes *res)
{
    gint ret;
    NmpID conflict;

    G_ASSERT(self != NULL && io != NULL && req != NULL && res != NULL);

    ret = nmp_mod_mds_new_mds(self, io,  req->mds_id, &conflict);
    if (G_UNLIKELY(ret))
        return ret;

    ret = nmp_mod_mds_sync_req(self, msg_id, req,
         sizeof(NmpMdsRegister), res, sizeof(NmpMdsRegisterRes));

    return ret;
}


NmpMsgFunRet
nmp_mod_mds_register_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModMds *self;
    NmpNetIO *io;
    NmpMdsRegister *req_info;
    NmpMdsRegisterRes res_info;
    NmpMsgID msg_id;
    NmpGuestBase *mds_base;
    NmpMds *mds;
    NmpMsgMdsOnlineChange notify_info;
    gint ret;
    NmpResourcesCap res_cap;

    self = (NmpModMds*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_cap, 0, sizeof(res_cap));
    nmp_mod_get_resource_cap(&res_cap);
    if (!(res_cap.module_bits&MODULE_MDS_BIT))
    {
        ret = E_EXPIRED;
        goto mds_register_err;
    }
    memset(&notify_info, 0, sizeof(notify_info));
    notify_info.mds_id[MDS_ID_LEN - 1] = 0;
    strncpy(notify_info.mds_id, req_info->mds_id, MDS_ID_LEN - 1);
    memset(&res_info, 0, sizeof(res_info));
    nmp_get_ip_from_socket(io, req_info->mds_ip);
    ret = nmp_mod_mds_mds_register(self, io, msg_id, req_info, &res_info);
mds_register_err:
    if (ret)
    {
        nmp_print(
            "<NmpModMds> mds:%s register failed, err:%d",
            req_info->mds_id, -ret
        );

        SET_CODE(&res_info, -ret);
        MSG_SET_RESPONSE(msg);
        nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

        nmp_app_obj_deliver_in((NmpAppObj*)self, msg);
        nmp_mod_acc_release_io((NmpModAccess*)self, io);
        nmp_mod_container_del_io(self->container, io);

        return MFR_ACCEPTED;
    }

    mds_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!mds_base))
    {
        ret = -E_NOSUCHGUEST;
	 nmp_warning("<NmpModMds> Can't find mds:'%s' in container, io timeout?", notify_info.mds_id);
    }
    else
   {
        mds = (NmpMds *)mds_base;
        mds->mds_state = STAT_MDS_ONLINE;
        nmp_mods_container_put_guest(self->container, mds_base);

         nmp_print(
             "<NmpModMds> mds:%s register ok",
             req_info->mds_id
         );

        strncpy(res_info.domain_id, nmp_get_local_domain_id(), DOMAIN_ID_LEN - 1);

        nmp_check_keepalive_time(&res_info.keep_alive_time);
        nmp_net_io_set_ttd(io, res_info.keep_alive_time*1000*TIMEOUT_N_PERIODS);
        notify_info.new_status = 1;
        nmp_mod_mds_change_mds_online_status(app_obj, notify_info);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
    return MFR_DELIVER_BACK;
}

/*
NmpMsgFunRet
nmp_mod_mds_heart_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModMds *self;
    NmpNetIO *io;
    NmpGuestBase *mds_base;
    NmpMdsHeart *req_info;
    NmpMdsHeartRes res_info;
    NmpMsgID msg_id;
    gint ret = 0;

    self = (NmpModMds*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    mds_base = nmp_mods_container_get_guest_2(self->container, req_info->mds_id);
    if (G_UNLIKELY(!mds_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModMds> mdsId:%s No such guest.", req_info->mds_id);
    }
    else
    {
        nmp_get_utc_time(res_info.server_time);
        printf("res_info.server_time=%s\n",res_info.server_time);
        nmp_mods_container_put_guest(self->container, mds_base);
    }

    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}
*/

NmpMsgFunRet
nmp_mod_mds_heart_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModMds *self;
    NmpNetIO *io;
    NmpGuestBase *mds_base;
    NmpMdsHeart *req_info;
    NmpMdsHeartRes res_info;
    NmpMsgID msg_id;
    gint ret = 0;
    NmpResourcesCap res_cap;

    self = (NmpModMds*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_cap, 0, sizeof(res_cap));
    nmp_mod_get_resource_cap(&res_cap);
    if (!(res_cap.module_bits&MODULE_MDS_BIT))
    {
        nmp_app_obj_deliver_in((NmpAppObj*)self, msg);
        nmp_mod_acc_release_io((NmpModAccess*)self, io);
        nmp_mod_container_del_io(self->container, io);
        return MFR_ACCEPTED;
    }

    memset(&res_info, 0, sizeof(res_info));
    mds_base = nmp_mods_container_get_guest(self->container, io);
    if (G_LIKELY(mds_base))
    {
        nmp_get_ip_from_socket(io, req_info->mds_ip);
	  MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
        nmp_mods_container_put_guest(self->container, mds_base);
	 return MFR_DELIVER_AHEAD;
    }

    ret = -E_NOSUCHGUEST;
    nmp_warning("<NmpModMds> mdsId:%s No such mds.", req_info->mds_id);
    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_mds_heart_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModMds *self;
    NmpMdsHeartRes *res_info;
    NmpGuestBase *mds_base;
    NmpNetIO *io;
    gint msg_id;

    self = (NmpModMds*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    msg_id = MSG_GETID(msg);
    mds_base = nmp_mods_container_get_guest_2(self->container, res_info->mds_id);
    if (G_UNLIKELY(!mds_base))
    {
        nmp_warning("<NmpModMds> mds heart error, mdsId:%s no such mds.",
             res_info->mds_id);
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    nmp_get_utc_time(res_info->server_time);
    io = IO_OF_GUEST(mds_base);
    BUG_ON(!io);
    nmp_sysmsg_attach_io(msg, io);
    nmp_mods_container_put_guest(self->container, mds_base);

    if (RES_CODE(res_info))
    {
        nmp_app_obj_deliver_in((NmpAppObj*)self, msg);
      	 nmp_mod_acc_release_io((NmpModAccess*)self, io);
      	 nmp_mod_container_del_io(self->container, io);
	 return MFR_ACCEPTED;
    }

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_mds_get_media_url_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpModMds *self;
    NmpMsgGetUrl *req;
    NmpGetMediaUrlRes res;
    NmpGuestBase *mds_base;
    gchar puid[MAX_ID_LEN] = {0};
    gchar tmp[MAX_ID_LEN];
    gint channel, level;
    gint ret = 0;

    req = MSG_GET_DATA(msg);
	memset(&res, 0, sizeof(res));
	strncpy(res.session, req->session, SESSION_ID_LEN - 1);
    self = (NmpModMds*)app_obj;
    mds_base = nmp_mods_container_get_guest_2(self->container, req->mds_id);
    if (G_UNLIKELY(!mds_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModMds> mdsId:%s No such guest.", req->mds_id);
    }
    else
    {
        nmp_mods_container_put_guest(self->container, mds_base);
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
    nmp_sysmsg_set_private_2(msg, &res, sizeof(res));
    printf("nmp_dbs_modify_sysmsg_2 ret = %d,--url= %s\n",ret, res.url);
    MSG_SET_DSTPOS(msg, BUSSLOT_POS_CU);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_mds_mss_get_route_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpModMds *self;
    NmpMssGetRouteRes *req;
    gint ret = 0;

    req = MSG_GET_DATA(msg);

    self = (NmpModMds*)app_obj;
    nmp_sysmsg_set_private_2(msg, req, sizeof(NmpMssGetRouteRes));
    printf("nmp_dbs_modify_sysmsg_2 ret = %d,--url= %s\n",ret, req->url);
    MSG_SET_DSTPOS(msg, BUSSLOT_POS_MSS);

    return MFR_DELIVER_BACK;
}


void
nmp_mod_mds_register_msg_handler(NmpModMds *self)
{
    NmpAppMod *super_self = (NmpAppMod*)self;

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MDS_REGISTER,
        nmp_mod_mds_register_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MDS_HEART,
        nmp_mod_mds_heart_f,
        nmp_mod_mds_heart_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_MEDIA_URL,
        NULL,
        nmp_mod_mds_get_media_url_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MSS_GET_ROUTE,
        NULL,
        nmp_mod_mds_mss_get_route_b,
        0
    );


}







