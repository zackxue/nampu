#include "nmp_mod_cu.h"
#include "nmp_message.h"
#include "nmp_internal_msg.h"
#include "nmp_errno.h"
#include "nmp_shared.h"
#include "nmp_cu_msg.h"
#include "nmp_version.h"
#include "nmp_tw_interface.h"

USING_MSG_ID_MAP(cms);


#define CU_TIMEOUT   	(60*1000)
#define DECODE_NAME  	"JDCUPlayCtrl"

#define FTP_PORT   2121

extern void
nmp_mod_cu_update_usr_group_permissions(JpfModCu *self,
    gint group_id, gint permission, gint rank);


gint
nmp_mod_cu_check_permissions(JpfModCu *self, JpfNetIO *io,
    JpfUsrGrpPermissions perm, gint *p_rank)
{
    JpfGuestBase *cu_base;
    gint permissions, right_flag, rank ;

    cu_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!cu_base))
        return -E_NOSUCHGUEST;

    JpfCu *cu;
    cu = (JpfCu*)cu_base;

    permissions = jpf_get_usr_permissions(cu);
    right_flag = NMP_USR_GRP_PERMISSION(permissions, perm);
    rank = jpf_get_usr_rank(cu);
    jpf_mods_container_put_guest(self->container, cu_base);

    if (G_LIKELY(right_flag == perm))
    {
    	if (p_rank)
		*p_rank = rank;
        return 0;
    }

    jpf_warning("<JpfModCu> check cu permission err:%d, username:%s.",
    	-E_NOPERMISSION, jpf_get_usr_name(cu)
    );
    return -E_NOPERMISSION;
}

static __inline__ void
nmp_mod_cu_check_rank(JpfModCu *self, NmpSysMsg *msg, JpfUsrGrpPermissions perm, gint rank)
{
    JpfControlPtz *req_info;

    if (perm&USR_GRP_PTZ_CONTROL)
    {
        req_info = MSG_GET_DATA(msg);
        BUG_ON(!req_info);
	 req_info->rank = rank;
    }
}

NmpMsgFunRet
nmp_mod_cu_forward(JpfModCu *self, NmpSysMsg *msg,
    JpfUsrGrpPermissions perm,JpfBusSlotPos dst_pos)
{
    JpfNetIO *io;
    JpfMsgErrCode	code;
    gint msg_id, rank, ret;

    io = MSG_IO(msg);
    BUG_ON(!io);

    ret = nmp_mod_cu_check_permissions(self, io, perm, &rank);
    if (G_LIKELY(!ret))
    {
    	nmp_mod_cu_check_rank(self, msg, perm, rank);
        MSG_SET_DSTPOS(msg, dst_pos);
        return MFR_DELIVER_AHEAD;
    }

    msg_id = MSG_GETID(msg);
    SET_CODE(&code, -ret);
    jpf_warning("<JpfModCu> deliver msg '%s' failed, user no such permission.",
            MESSAGE_ID_TO_STR(cms, msg_id));
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &code, sizeof(code));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_cu_backward(JpfModCu *self, NmpSysMsg *msg, const gchar *id_str)
{
    JpfGuestBase *cu_base;
    JpfNetIO *io;
    gint msg_id;

    msg_id = MSG_GETID(msg);
    cu_base = jpf_mods_container_get_guest_2(self->container, id_str);
    if (G_UNLIKELY(!cu_base))
    {
        jpf_warning("<JpfModCu> deliver msg '%s' failed, session:%s no such cu.",
            MESSAGE_ID_TO_STR(cms, msg_id), id_str);
        jpf_sysmsg_destroy(msg);

        return MFR_ACCEPTED;
    }

    io = IO_OF_GUEST(cu_base);
    BUG_ON(!io);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_attach_io(msg, io);
    jpf_mods_container_put_guest(self->container, cu_base);

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_cu_forward_2(JpfModCu *self, NmpSysMsg *msg,
    JpfUsrGrpPermissions perm, gpointer pri_data, gint size)
{
    JpfNetIO *io;
    gint msg_id, rank, ret;

    io = MSG_IO(msg);
    BUG_ON(!io);

    ret = nmp_mod_cu_check_permissions(self, io, perm, &rank);
    if (G_LIKELY(!ret))
    {
    	nmp_mod_cu_check_rank(self, msg, perm, rank);
        MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
        return MFR_DELIVER_AHEAD;
    }

    msg_id = MSG_GETID(msg);
    SET_CODE(pri_data, -ret);
    jpf_warning("<JpfModCu> deliver msg '%s' failed, user no such permission.",
            MESSAGE_ID_TO_STR(cms, msg_id));
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, pri_data, size);

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_cu_get_device_info(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetDeviceInfo *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
}


NmpMsgFunRet
nmp_mod_cu_get_device_channel_para(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetDeviceChannelInfo *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
}


NmpMsgFunRet
nmp_mod_cu_set_device_para_res(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfSetDeviceParaRes	 *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_online_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfNetIO *io;
    JpfGuestBase *cu_base;
    JpfCuLoginInfo *req_info;
    JpfCuLoginResp res_info;
    gint ret;
    gchar session[SESSION_ID_LEN];
    JpfResourcesCap res_cap;

    self = (JpfModCu*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    snprintf(res_info.cms_version, VERSION_LEN, "%d", __CMS_VERSION__);
    snprintf(res_info.cu_min_version, VERSION_LEN, "%d", __CU_MIN_VERSION__);
    if ((req_info->cu_version < __CU_MIN_VERSION__) || (req_info->cu_version > __CMS_VERSION__))
    {
        ret = -E_VERSION;
        goto err_login;
    }

    if ((regex_mached(req_info->username, exclude_reg))
        || (regex_mached(req_info->password, exclude_reg)))
    {
        ret = -E_STRINGFORMAT;
        goto err_login;
    }

    ret = nmp_mod_cu_user_session_new(self, io, req_info->username,
        req_info->password, session, SESSION_ID_LEN);
    if (ret)
    {
        jpf_warning("<JpfModCu> user:%s login failed,err=%d", req_info->username, ret);
        goto err_login;
    }

    jpf_warning("<JpfModCu> user:%s login ok", req_info->username);
    JpfMsgUserLoginInfoRes LoginInfoRes;
    memset(&LoginInfoRes, 0, sizeof(LoginInfoRes));
    ret = nmp_mod_cu_get_user_login_info(self, &LoginInfoRes);
    if (ret)
        goto err_login;

    if (!RES_CODE(&LoginInfoRes))
    {
        strcpy(res_info.domain_id,LoginInfoRes.domain_id);
        strcpy(res_info.domain_name,LoginInfoRes.domain_name);
        res_info.root_area_id = LoginInfoRes.root_area_id;
        strcpy(res_info.root_area_name,LoginInfoRes.root_area_name);
    }

    SET_CODE(&res_info, RES_CODE(&LoginInfoRes));
    strcpy(res_info.session, session);
    cu_base = jpf_mods_container_get_guest_2(self->container, session);
    if (G_UNLIKELY(!cu_base))
    {
        ret = -E_NOSUCHGUEST;
        SET_CODE(&res_info, -ret);
        jpf_warning("<JpfModCu> session:%s No such guest.", session);
    }
    else
    {
        JpfCu *cu;
        cu = (JpfCu*)cu_base;
        res_info.usr_permissions = jpf_get_usr_permissions(cu);
        jpf_mods_container_put_guest(self->container, cu_base);
    }
    memset(&res_cap, 0, sizeof(res_cap));
    nmp_mod_get_resource_cap(&res_cap);
    res_info.module_sets = res_cap.module_bits;
    jpf_net_io_set_ttd(io, CU_TIMEOUT);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
err_login:
    jpf_print("<JpfModCu> user:%s login failed, err:%d", req_info->username, ret);
    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    nmp_app_obj_deliver_in((NmpAppObj*)self, msg);
    nmp_mod_acc_release_io((JpfModAccess*)self, io);
    nmp_mod_container_del_io(self->container, io);

    return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_cu_user_force_usr_offline_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfMsgDelUser *res_info = NULL;
    JpfModCu *self;
    JpfForceUsrOffline notify_info;
    NmpSysMsg *msg_notify;
    gchar *username;
    gchar *name;
    gchar  offline_name[USER_NAME_LEN];

    self = NMP_MODCU(app_obj);
    res_info = MSG_GET_DATA(msg);
    name = res_info->name;

    memset(&notify_info, 0, sizeof(notify_info));
    if (name[0] == '\'')
        notify_info.reason = 0;   // user has been deleted
    else
        notify_info.reason = 1;  //user info has been modified

    msg_notify = jpf_sysmsg_new_2(MESSAGE_FORCE_USR_OFFLINE,
        &notify_info, sizeof(notify_info), ++msg_seq_generator);
    if (G_UNLIKELY(!msg_notify))
    {
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    while((username = strsep(&name, ",")) != NULL)
    {
        memset(offline_name, 0, USER_NAME_LEN);
        if (username[0] == '\'')
            strncpy(offline_name,&username[1],strlen(username) - 2);
        else
            strncpy(offline_name,username,strlen(username));

        offline_name[USER_NAME_LEN - 1] = 0;
        nmp_mod_cu_force_usr_offline(self, offline_name, msg_notify);
    }

    jpf_sysmsg_destroy(msg_notify);
    jpf_sysmsg_destroy(msg);

    return MFR_ACCEPTED;
}



NmpMsgFunRet
nmp_mod_cu_group_force_usr_offline_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfMsgDelUserGroup *res_info = NULL;
    JpfModCu *self;
    JpfForceUsrOffline notify_info;
    NmpSysMsg *msg_notify;
    gint group_id;
    gchar *del_group;
    gchar *group_ids;

    self = NMP_MODCU(app_obj);
    res_info = MSG_GET_DATA(msg);
    group_ids = res_info->group_ids;

    memset(&notify_info, 0, sizeof(notify_info));
    if (group_ids[0] == '\'')
    {
        notify_info.reason = 3; //group info of user has been modified
        group_ids = &res_info->group_ids[1];
        group_id = atoi(group_ids);
        nmp_mod_cu_update_usr_group_permissions(self, group_id,
        	res_info->group_permissions, res_info->group_rank);
    }
    else
        notify_info.reason = 2;//group of user has been deleted
    msg_notify = jpf_sysmsg_new_2(MESSAGE_FORCE_USR_OFFLINE,
        &notify_info, sizeof(notify_info), ++msg_seq_generator);
    if (G_UNLIKELY(!msg_notify))
    {
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    while((del_group = strsep(&group_ids, ",")) != NULL)
    {
        group_id = atoi(del_group);
        printf("&&&&&&&&&&&&&&&group : %d\n",group_id);
        nmp_mod_cu_force_usr_offline_by_group(self, group_id, msg_notify);
    }

    jpf_sysmsg_destroy(msg_notify);
    jpf_sysmsg_destroy(msg);

    return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_cu_change_pu_online_state_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfPuOwnToAllCu *res_info = NULL;
    JpfModCu *self;
    gint i;

    self = NMP_MODCU(app_obj);
    res_info = MSG_GET_DATA(msg);

    for(i = 0; i < res_info->total_num; i++)
        nmp_mod_cu_deliver_msg(self, res_info->cu_list[i].username, msg);

    jpf_sysmsg_destroy(msg);

    return MFR_ACCEPTED;
}


/*
NmpMsgFunRet
nmp_mod_cu_broadcast_generic_msg_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfAuthorizationExpired *auth_exp;
    gchar param1[GENERAL_MSG_PARM_LEN], param2[GENERAL_MSG_PARM_LEN];

    self = NMP_MODCU(app_obj);
    auth_exp = MSG_GET_DATA(msg);
    snprintf(param1, GENERAL_MSG_PARM_LEN - 1, "%d", auth_exp->type);
    strncpy(param2, auth_exp->expired_time, TIME_LEN - 1);
    nmp_mod_cu_broadcast_generic_msg(self, MSG_AUTHORIZATION_EXPIRED, param1, param2, NULL, NULL);
    jpf_sysmsg_destroy(msg);

    return MFR_ACCEPTED;
}*/

NmpMsgFunRet
nmp_mod_cu_broadcast_generic_msg_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;

    self = NMP_MODCU(app_obj);
    nmp_mod_cu_broadcast_to_all_user(self, msg);
    jpf_sysmsg_destroy(msg);

    return MFR_ACCEPTED;
}

NmpMsgFunRet
nmp_mod_cu_notify_modify_domain_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;

    self = NMP_MODCU(app_obj);

    nmp_mod_cu_deliver_msg_2(self, msg);
    jpf_sysmsg_destroy(msg);

    return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_cu_heart_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfNetIO *io;
    JpfGuestBase *cu_base;
    JpfCuHeart *req_info;
    JpfCuHeartResp res_info;
    NmpMsgID msg_id;
    gint ret = 0;

    self = (JpfModCu*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    cu_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!cu_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModCu> session:%s No such guest.", req_info->session);
    }
    else
    {
        jpf_get_utc_time(res_info.server_time);
        jpf_mods_container_put_guest(self->container, cu_base);
     }

    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_cu_submit_alarm_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfSubmitAlarmRes *res_info = NULL;
    JpfModCu *self;
    gint i;

    self = NMP_MODCU(app_obj);
    res_info = MSG_GET_DATA(msg);

    for(i = 0; i < res_info->total_num; i++)
        nmp_mod_cu_deliver_msg(self, res_info->cu_list[i].username, msg);

    jpf_sysmsg_destroy(msg);

    return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_cu_get_all_area_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfNetIO *io;
    JpfGuestBase *cu_base;
    JpfGetArea *req_info;
    JpfGetAreaRes res_info;
    NmpMsgID msg_id;
    gint ret;

    self = (JpfModCu*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    cu_base  = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!cu_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModCu> session:%s No such guest.", req_info->session);
        MSG_SET_RESPONSE(msg);
        memset(&res_info, 0, sizeof(res_info));
        SET_CODE(&res_info, -ret);
        jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

        return MFR_DELIVER_BACK;
    }

    MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
    jpf_mods_container_put_guest(self->container, cu_base);

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_cu_get_all_area_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetAreaRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_get_area_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfNetIO *io;
    JpfGuestBase *cu_base;
    JpfGetAreaInfo *req_info;
    JpfGetAreaInfoRes res_info;
    NmpMsgID msg_id;
    gint ret;
    JpfCu *cu;
    gint permissions, right_flag;

    self = (JpfModCu*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    cu_base  = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!cu_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModCu> session:%s No such guest.", req_info->session);
        MSG_SET_RESPONSE(msg);
        memset(&res_info, 0, sizeof(res_info));
        SET_CODE(&res_info, -ret);
        jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

        return MFR_DELIVER_BACK;
    }

    cu = (JpfCu*)cu_base;
    permissions = jpf_get_usr_permissions(cu);
    right_flag = NMP_USR_GRP_PERMISSION(permissions, USR_GRP_BROWSE_DEV_STATUS);
    if (G_LIKELY(right_flag != USR_GRP_BROWSE_DEV_STATUS))
    {
        ret = -E_NOPERMISSION;;
        jpf_warning("<JpfModCu> user:%s No such permissiont.", cu->user->user_name);
        MSG_SET_RESPONSE(msg);
        SET_CODE(&res_info, -ret);
        jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

        return MFR_DELIVER_BACK;
    }
    strncpy(req_info->username, cu->user->user_name, USER_NAME_LEN - 1);
    MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
    jpf_mods_container_put_guest(self->container, cu_base);

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_cu_get_area_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetAreaInfoRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_get_device_list_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfNetIO *io;
    JpfGuestBase *cu_base;
    JpfGetDevice *req_info;
    JpfMsgErrCode res_info;
    NmpMsgID msg_id;
    JpfCu *cu;
    gint ret;

    self = (JpfModCu*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    cu_base = jpf_mods_container_get_guest_2(self->container, req_info->session);
    if (G_UNLIKELY(!cu_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModCu> session:%s No such guest.", req_info->session);
        MSG_SET_RESPONSE(msg);
        SET_CODE(&res_info, -ret);
        jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

        return MFR_DELIVER_BACK;
    }

    cu = (JpfCu*)cu_base;
    strncpy(req_info->username, cu->user->user_name, USER_NAME_LEN - 1)	;
    MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
    jpf_sysmsg_set_private_2(msg, req_info, sizeof(JpfGetDevice));

    jpf_mods_container_put_guest(self->container, cu_base);

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_cu_get_device_list_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetDeviceRes *res_info;
    self = (JpfModCu*)app_obj;

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_get_area_device_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfNetIO *io;
    JpfGuestBase *cu_base;
    JpfGetAreaDevice *req_info;
    JpfGetAreaInfoRes res_info;
    NmpMsgID msg_id;
    JpfCu *cu;
    gint ret;
    gint permissions, right_flag;

    self = (JpfModCu*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    cu_base = jpf_mods_container_get_guest_2(self->container, req_info->session);
    if (G_UNLIKELY(!cu_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModCu> session:%s No such guest.", req_info->session);
        MSG_SET_RESPONSE(msg);
        SET_CODE(&res_info, -ret);
        jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

        return MFR_DELIVER_BACK;
    }

    cu = (JpfCu*)cu_base;
    permissions = jpf_get_usr_permissions(cu);
    right_flag = NMP_USR_GRP_PERMISSION(permissions, USR_GRP_BROWSE_DEV_STATUS);
    if (G_LIKELY(right_flag != USR_GRP_BROWSE_DEV_STATUS))
    {
        ret = -E_NOPERMISSION;;
        jpf_warning("<JpfModCu> user:%s No such permissiont.", cu->user->user_name);
        MSG_SET_RESPONSE(msg);
        SET_CODE(&res_info, -ret);
        jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

        return MFR_DELIVER_BACK;
    }
    strncpy(req_info->username, cu->user->user_name, USER_NAME_LEN - 1);
    MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
    jpf_mods_container_put_guest(self->container, cu_base);

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_cu_get_area_device_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetAreaDeviceRes *res_info;
    self = (JpfModCu*)app_obj;

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}

NmpMsgFunRet
nmp_mod_cu_get_media_url_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfNetIO *io;
    JpfGuestBase *cu_base;
    JpfGetMediaUrl *req_info;
    JpfGetMediaUrlRes res_info;
    NmpMsgID msg_id;
    gint permissions, right_flag, right_type = 0;
    gint ret;
    gchar *cu_ip;

    self = (JpfModCu*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    cu_base = jpf_mods_container_get_guest_2(self->container, req_info->session);
    if (G_UNLIKELY(!cu_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModCu> session:%s No such guest.", req_info->session);
        SET_CODE(&res_info, -ret);
    }
    else
    {
        JpfCu *cu;
        cu = (JpfCu*)cu_base;
        strncpy(req_info->username, cu->user->user_name, USER_NAME_LEN - 1);
        permissions = jpf_get_usr_permissions(cu);
        switch (req_info->media)
        {
        case 0:
            right_type =  USR_GRP_VIDEO_LIVE;
            break;
        case 1:
            right_type =  USR_GRP_VIDEO_LIVE;
            break;
        case 2:
	     right_type =  USR_GRP_VIDEO_HISTORY;
            break;
        case 3:
	     right_type =  USR_GRP_VIDEO_HISTORY;
            break;
        case 4:
            right_type =  USR_GRP_VIDEO_HISTORY;
            break;
	 default:
	 	break;
        }

        right_flag = NMP_USR_GRP_PERMISSION(permissions, right_type);
        jpf_mods_container_put_guest(self->container, cu_base);
        printf("=============user permissions =%d, right=%d\n",permissions, right_flag);
        if (G_UNLIKELY(right_flag > 0))
        {
            cu_ip = jpf_net_get_io_peer_name(io);
            if (G_LIKELY(cu_ip))
            {
                strncpy(req_info->cu_ip, cu_ip, MAX_IP_LEN - 1);
                g_free(cu_ip);
            }

             //直连与否都要到pu模块判断设备在线状态

            if (req_info->connect_mode == 0)
                MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS|BUSSLOT_POS_PU);
           else
                MSG_SET_DSTPOS(msg, BUSSLOT_POS_PU);

            return MFR_DELIVER_AHEAD;
        }
        else
        {
            SET_CODE(&res_info, E_NOPERMISSION);
        }
    }

    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_cu_get_media_url_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetMediaUrlRes *res_info;
    gchar manufactur[MF_ID_LEN] = {0};
    gchar path[FILE_PATH_LEN] = {0};

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (!RES_CODE(res_info))
    {
        strncpy(res_info->decode_path, jpf_get_sys_parm_str(SYS_PARM_DECODERPATH),
        FILE_PATH_LEN - 1);

        strncpy(manufactur, res_info->guid, MF_ID_LEN - 1);
	if (!strcmp(manufactur, MANUFACTUR_JXJ_2))
		strcpy(manufactur, MANUFACTUR_JXJ);

        res_info->decode_name[DECODE_NAME_LEN - 1] = 0;
        sprintf(res_info->decode_name, "%s%s", DECODE_NAME, manufactur);
        if ((strlen(res_info->decode_path) + strlen(res_info->decode_name) + 1) >= FILE_PATH_LEN)
        {
            jpf_warning("decoder path is too longth:%s.",res_info->decode_path);
        }
        else
        {
            sprintf(path,"%s/%s", res_info->decode_path, res_info->decode_name);
            if ((access(path, R_OK) == -1) || (strlen(res_info->ip) == 0))
                memset(res_info->decode_path, 0, FILE_PATH_LEN);
            else
                sprintf(res_info->decode_path, "ftp://%s:%d/%s",res_info->ip, FTP_PORT, path);
        }
    }
    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_get_platform_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_get_device_info(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_platform_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetPlatformInfoRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

   return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_set_platform_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfSetPlatformInfo *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
}


NmpMsgFunRet
nmp_mod_cu_set_platform_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_set_device_para_res(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_device_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_get_device_info(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_device_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetDeviceInfoRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_get_network_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetDeviceInfo *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
}


NmpMsgFunRet
nmp_mod_cu_get_network_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetNetworkInfoRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_set_network_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfSetNetworkInfo *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
}


NmpMsgFunRet
nmp_mod_cu_set_network_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_set_device_para_res(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_pppoe_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_get_device_info(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_pppoe_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetPppoeInfoRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

   return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_set_pppoe_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfSetPppoeInfo *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
}


NmpMsgFunRet
nmp_mod_cu_set_pppoe_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_set_device_para_res(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_encode_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetDeviceChannelInfo *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
  }


NmpMsgFunRet
nmp_mod_cu_get_encode_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetEncodeParaRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_set_encode_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfSetEncodePara *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
}


NmpMsgFunRet
nmp_mod_cu_set_encode_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_set_device_para_res(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_display_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_get_device_channel_para(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_display_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetDisplayParaRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);

}


NmpMsgFunRet
nmp_mod_cu_set_display_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfSetDisplayPara *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);

}


NmpMsgFunRet
nmp_mod_cu_set_display_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_set_device_para_res(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_OSD_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_get_device_channel_para(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_OSD_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetOSDParaRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_set_OSD_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfSetOSDPara *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
}


NmpMsgFunRet
nmp_mod_cu_set_OSD_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_set_device_para_res(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_record_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_get_device_channel_para(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_record_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetRecordParaRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_set_record_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfSetRecordPara *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
}


NmpMsgFunRet
nmp_mod_cu_set_record_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_set_device_para_res(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_hide_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_get_device_channel_para(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_hide_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetHideParaRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_set_hide_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfSetHidePara *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
}


NmpMsgFunRet
nmp_mod_cu_set_hide_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_set_device_para_res(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_serial_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetSerialPara  *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
}


NmpMsgFunRet
nmp_mod_cu_get_serial_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetSerialParaRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_set_serial_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfSetSerialPara *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
}


NmpMsgFunRet
nmp_mod_cu_set_serial_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_set_device_para_res(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_move_detection_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
   return nmp_mod_cu_get_device_channel_para(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_move_detection_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetMoveAlarmParaRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_set_move_detection_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfSetMoveAlarmPara *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
}


NmpMsgFunRet
nmp_mod_cu_set_move_detection_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_set_device_para_res(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_video_lost_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
   return nmp_mod_cu_get_device_channel_para(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_video_lost_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetVideoLostParaRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_set_video_lost_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfSetVideoLostPara *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
}


NmpMsgFunRet
nmp_mod_cu_set_video_lost_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_set_device_para_res(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_hide_alarm_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
   return nmp_mod_cu_get_device_channel_para(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_hide_alarm_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetHideAlarmParaRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_set_hide_alarm_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfSetHideAlarmPara *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
}


NmpMsgFunRet
nmp_mod_cu_set_hide_alarm_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_set_device_para_res(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_io_alarm_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
   return nmp_mod_cu_get_device_channel_para(app_obj, msg);

}


NmpMsgFunRet
nmp_mod_cu_get_io_alarm_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetIOAlarmParaRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_set_io_alarm_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfSetIOAlarmPara *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
}


NmpMsgFunRet
nmp_mod_cu_set_io_alarm_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_set_device_para_res(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_link_io_alarm_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfAmsActionIO *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
}


NmpMsgFunRet
nmp_mod_cu_link_io_alarm_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_set_device_para_res(app_obj, msg);
}

NmpMsgFunRet
nmp_mod_cu_get_joint_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetJointPara *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
}


NmpMsgFunRet
nmp_mod_cu_get_joint_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetJointParaRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_set_joint_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfSetJointPara *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
}


NmpMsgFunRet
nmp_mod_cu_set_joint_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_set_device_para_res(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_PTZ_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
   return nmp_mod_cu_get_device_info(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_PTZ_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetPtzParaRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_set_PTZ_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfSetPtzPara *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
}


NmpMsgFunRet
nmp_mod_cu_set_PTZ_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_set_device_para_res(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_control_PTZ_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfControlPtz *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PTZ_CONTROL, BUSSLOT_POS_PU);

}


NmpMsgFunRet
nmp_mod_cu_control_PTZ_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfSetDeviceParaRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_get_preset_point_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetDeviceChannelInfo *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
  }


NmpMsgFunRet
nmp_mod_cu_get_preset_point_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetPresetPointRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_set_preset_point_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfSetPresetPoint *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
}


NmpMsgFunRet
nmp_mod_cu_set_preset_point_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_set_device_para_res(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_cruise_way_set_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetDeviceChannelInfo *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
  }


NmpMsgFunRet
nmp_mod_cu_get_cruise_way_set_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetCruiseWaySetRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_get_cruise_way_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetDeviceChannelInfo *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
  }


NmpMsgFunRet
nmp_mod_cu_get_cruise_way_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetCruiseWayRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_add_cruise_way_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfAddCruiseWay *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
}


NmpMsgFunRet
nmp_mod_cu_add_cruise_way_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_set_device_para_res(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_modify_cruise_way_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfModifyCruiseWay *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
}


NmpMsgFunRet
nmp_mod_cu_modify_cruise_way_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_set_device_para_res(app_obj, msg);
}

NmpMsgFunRet
nmp_mod_cu_set_cruise_way_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfSetPresetPoint *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
}


NmpMsgFunRet
nmp_mod_cu_set_cruise_way_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_set_device_para_res(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_3D_control_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    Jpf3DControl *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
}


NmpMsgFunRet
nmp_mod_cu_3D_control_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_set_device_para_res(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_3D_goback_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_get_device_channel_para(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_3D_goback_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_set_device_para_res(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_device_time_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_get_device_info(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_device_time_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetDeviceTimeRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

   return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_set_device_time_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfSetDeviceTime *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
}


NmpMsgFunRet
nmp_mod_cu_set_device_time_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_set_device_para_res(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_ntp_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_get_device_info(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_ntp_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetNTPInfoRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

   return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_set_ntp_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfSetNTPInfo *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
}


NmpMsgFunRet
nmp_mod_cu_set_ntp_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_set_device_para_res(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_ftp_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_get_device_info(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_ftp_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetFtpParaRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

   return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_set_ftp_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfSetFtpPara *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
}


NmpMsgFunRet
nmp_mod_cu_set_ftp_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_set_device_para_res(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_smtp_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_get_device_info(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_smtp_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetSmtpParaRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

   return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_set_smtp_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfSetSmtpPara *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
}


NmpMsgFunRet
nmp_mod_cu_set_smtp_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_set_device_para_res(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_upnp_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_get_device_info(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_upnp_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetUpnpParaRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

   return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_set_upnp_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfSetUpnpPara *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
}


NmpMsgFunRet
nmp_mod_cu_set_upnp_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_set_device_para_res(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_transparent_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetTransparentPara *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
}


NmpMsgFunRet
nmp_mod_cu_get_transparent_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetTransparentParaRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

   return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_set_transparent_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfSetTransparentPara *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
}


NmpMsgFunRet
nmp_mod_cu_set_transparent_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_set_device_para_res(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_ddns_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_get_device_info(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_ddns_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetDdnsParaRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

   return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_set_ddns_para_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfSetDdnsPara *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
}


NmpMsgFunRet
nmp_mod_cu_set_ddns_para_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_set_device_para_res(app_obj, msg);
}

NmpMsgFunRet
nmp_mod_cu_get_disk_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_get_device_info(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_disk_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetDiskInfoRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

   return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_get_resolution_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_get_device_channel_para(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_resolution_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetResolutionInfoRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);

}


NmpMsgFunRet
nmp_mod_cu_set_resolution_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfSetResolutionInfo *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);

}


NmpMsgFunRet
nmp_mod_cu_set_resolution_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_set_device_para_res(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_ircut_control_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_get_device_channel_para(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_get_ircut_control_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetIrcutControlInfoRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);

}


NmpMsgFunRet
nmp_mod_cu_set_ircut_control_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfSetIrcutControlInfo *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);

}


NmpMsgFunRet
nmp_mod_cu_set_ircut_control_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_set_device_para_res(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_format_disk_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfFormatDisk *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
}


NmpMsgFunRet
nmp_mod_cu_format_disk_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_set_device_para_res(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_submit_format_pro_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfSubmitFormatPosRes *res_info = NULL;
    JpfModCu *self;
    gint i;

    self = NMP_MODCU(app_obj);
    res_info = MSG_GET_DATA(msg);

    for(i = 0; i < res_info->total_num; i++)
        nmp_mod_cu_deliver_msg(self, res_info->cu_list[i].username, msg);

    jpf_sysmsg_destroy(msg);

    return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_cu_get_alarm_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetAlarm *req_info;
    JpfNetIO *io;
    JpfMsgErrCode	code;
    gint msg_id, ret = 0;
    JpfGuestBase *cu_base;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);
    io = MSG_IO(msg);
    BUG_ON(!io);

    ret = nmp_mod_cu_check_permissions(self, io, USR_GRP_DEAL_ALARM, NULL);
    if (G_LIKELY(!ret))
    {
        cu_base = jpf_mods_container_get_guest(self->container, io);
        if (G_UNLIKELY(!cu_base))
            return -E_NOSUCHGUEST;

        JpfCu *cu;
        cu = (JpfCu*)cu_base;

        jpf_mods_container_put_guest(self->container, cu_base);
	 strncpy(req_info->username, jpf_get_usr_name(cu), USER_NAME_LEN - 1);
        MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
        return MFR_DELIVER_AHEAD;
    }

    msg_id = MSG_GETID(msg);
    SET_CODE(&code, -ret);
    jpf_warning("<JpfModCu> deliver msg '%s' failed, user no such permission.",
    MESSAGE_ID_TO_STR(cms, msg_id));
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &code, sizeof(code));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_cu_get_alarm_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetAlarmRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_get_alarm_state_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetAlarmState *req_info;
    JpfNetIO *io;
    JpfMsgErrCode	code;
    gint msg_id, ret;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);
    io = MSG_IO(msg);
    BUG_ON(!io);

    ret = nmp_mod_cu_check_permissions(self, io, USR_GRP_DEAL_ALARM, NULL);
    if (G_LIKELY(!ret))
    {
        MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
        return MFR_DELIVER_AHEAD;
    }

    msg_id = MSG_GETID(msg);
    SET_CODE(&code, -ret);
    jpf_warning("<JpfModCu> deliver msg '%s' failed, user no such permission.",
        MESSAGE_ID_TO_STR(cms, msg_id));
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &code, sizeof(code));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_cu_get_alarm_state_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetAlarmStateRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_deal_alarm_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfDealAlarm *req_info;
    JpfNetIO *io;
    JpfMsgErrCode	code;
    gint msg_id, ret;
    JpfGuestBase *cu_base;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);
    io = MSG_IO(msg);
    BUG_ON(!io);
    printf("----------------enter nmp_mod_cu_get_store_log_f\n");
    ret = nmp_mod_cu_check_permissions(self, io, USR_GRP_DEAL_ALARM, NULL);
    if (G_LIKELY(!ret))
    {
        cu_base = jpf_mods_container_get_guest(self->container, io);
        if (G_UNLIKELY(!cu_base))
            return -E_NOSUCHGUEST;

        JpfCu *cu;
        cu = (JpfCu*)cu_base;

        jpf_mods_container_put_guest(self->container, cu_base);
	 strncpy(req_info->operator, jpf_get_usr_name(cu), USER_NAME_LEN - 1);
        MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
        return MFR_DELIVER_AHEAD;
    }

    msg_id = MSG_GETID(msg);
    SET_CODE(&code, -ret);
    jpf_warning("<JpfModCu> deliver msg '%s' failed, user no such permission.",
        MESSAGE_ID_TO_STR(cms, msg_id));
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &code, sizeof(code));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_cu_deal_alarm_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfDealAlarmRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}

NmpMsgFunRet
nmp_mod_cu_get_store_log_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetStoreLog *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    JpfNetIO *io;
    JpfMsgErrCode	code;
    gint msg_id, ret;

    io = MSG_IO(msg);
    BUG_ON(!io);
    printf("----------------enter nmp_mod_cu_get_store_log_f\n");
    ret = nmp_mod_cu_check_permissions(self, io, USR_GRP_VIDEO_HISTORY, NULL);
    if (G_LIKELY(!ret))
    {
        if (req_info->store_path == 0)
            MSG_SET_DSTPOS(msg, BUSSLOT_POS_PU);
        else
            MSG_SET_DSTPOS(msg, BUSSLOT_POS_MSS);

        return MFR_DELIVER_AHEAD;
    }

    msg_id = MSG_GETID(msg);
    SET_CODE(&code, -ret);
    jpf_warning("<JpfModCu> deliver msg '%s' failed, user no such permission.",
    MESSAGE_ID_TO_STR(cms, msg_id));
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &code, sizeof(code));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_cu_get_store_log_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetStoreLogRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_get_mss_store_log_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetMssStoreLog *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    JpfNetIO *io;
    JpfMsgErrCode	code;
    gint msg_id, ret;

    io = MSG_IO(msg);
    BUG_ON(!io);
    printf("----------------enter nmp_mod_cu_get_store_log_f\n");
    ret = nmp_mod_cu_check_permissions(self, io, USR_GRP_VIDEO_HISTORY, NULL);
    if (G_LIKELY(!ret))
    {
        MSG_SET_DSTPOS(msg, BUSSLOT_POS_MSS);

        return MFR_DELIVER_AHEAD;
    }

    msg_id = MSG_GETID(msg);
    SET_CODE(&code, -ret);
    jpf_warning("<JpfModCu> deliver msg '%s' failed, user no such permission.",
    MESSAGE_ID_TO_STR(cms, msg_id));
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &code, sizeof(code));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_cu_get_mss_store_log_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetStoreLogRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_firmware_upgrade_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfPuUpgrade *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);
}


NmpMsgFunRet
nmp_mod_cu_firmware_upgrade_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    return nmp_mod_cu_set_device_para_res(app_obj, msg);
}


NmpMsgFunRet
nmp_mod_cu_control_device_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfControlDevice *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET, BUSSLOT_POS_PU);

}


NmpMsgFunRet
nmp_mod_cu_control_device_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfSetDeviceInfoRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_get_gu_mss_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetGuMss *req_info;

    self = (JpfModCu*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    JpfNetIO *io;
    JpfMsgErrCode	code;
    gint msg_id, ret;

    io = MSG_IO(msg);
    BUG_ON(!io);
    printf("----------------enter nmp_mod_cu_get_gu_mss_f\n");
    ret = nmp_mod_cu_check_permissions(self, io, USR_GRP_VIDEO_HISTORY, NULL);
    if (G_LIKELY(!ret))
    {
        MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);

        return MFR_DELIVER_AHEAD;
    }

    msg_id = MSG_GETID(msg);
    SET_CODE(&code, -ret);
    jpf_warning("<JpfModCu> deliver msg '%s' failed, user no such permission.",
        MESSAGE_ID_TO_STR(cms, msg_id));
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &code, sizeof(code));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_cu_get_gu_mss_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetGuMssRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_get_defence_area_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetDefenceArea *req_info;
    JpfGetDefenceAreaRes res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);
    memset(&res_info, 0, sizeof(res_info));
    self = (JpfModCu*)app_obj;
    return nmp_mod_cu_forward_2(self, msg, USR_GRP_ELECTRONIC_MAP, &res_info, sizeof(res_info));
}


NmpMsgFunRet
nmp_mod_cu_get_defence_area_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetDefenceAreaRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_get_defence_map_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetDefenceMap *req_info;
    JpfGetDefenceMapRes res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);
    memset(&res_info, 0, sizeof(res_info));
    self = (JpfModCu*)app_obj;
    return nmp_mod_cu_forward_2(self, msg, USR_GRP_ELECTRONIC_MAP, &res_info, sizeof(res_info));
}


NmpMsgFunRet
nmp_mod_cu_get_defence_map_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetDefenceMapRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_get_defence_gu_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfNetIO *io;
    JpfGuestBase *cu_base;
    JpfGetDefenceGu *req_info;
    JpfGetDefenceGuRes res_info;
    NmpMsgID msg_id;
    gint ret;

    self = (JpfModCu*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    cu_base  = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!cu_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModCu> session:%s No such guest.", req_info->session);
        MSG_SET_RESPONSE(msg);
        memset(&res_info, 0, sizeof(res_info));
        SET_CODE(&res_info, -ret);
        jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

        return MFR_DELIVER_BACK;
    }

    JpfCu *cu;
    cu = (JpfCu*)cu_base;
    strncpy(req_info->username, jpf_get_usr_name(cu), USER_NAME_LEN - 1);

    MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
    jpf_mods_container_put_guest(self->container, cu_base);

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_cu_get_defence_gu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetDefenceGuRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_get_map_href_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetMapHref *req_info;
    JpfGetMapHrefRes res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);
    memset(&res_info, 0, sizeof(res_info));
    self = (JpfModCu*)app_obj;
    return nmp_mod_cu_forward_2(self, msg, USR_GRP_ELECTRONIC_MAP, &res_info, sizeof(res_info));
}


NmpMsgFunRet
nmp_mod_cu_get_map_href_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetMapHrefRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_get_gu_map_location_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetGuMapLocation *req_info;
    JpfGetGuMapLocationRes res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);
    memset(&res_info, 0, sizeof(res_info));
    self = (JpfModCu*)app_obj;
    return nmp_mod_cu_forward_2(self, msg, USR_GRP_ELECTRONIC_MAP, &res_info, sizeof(res_info));
}


NmpMsgFunRet
nmp_mod_cu_get_gu_map_location_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetGuMapLocationRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_get_tw_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetTw *req_info;
    JpfGetTwRes res_info;
    JpfGuestBase *cu_base;
    gint ret;
    JpfCu *cu;
    JpfNetIO *io;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);
    memset(&res_info, 0, sizeof(res_info));
    self = (JpfModCu*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    cu_base  = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!cu_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModCu> session:%s No such guest.", req_info->session);
        MSG_SET_RESPONSE(msg);
        memset(&res_info, 0, sizeof(res_info));
        SET_CODE(&res_info, -ret);
        jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

        return MFR_DELIVER_BACK;
    }

    cu = (JpfCu*)cu_base;
    strncpy(req_info->username, cu->user->user_name, USER_NAME_LEN - 1);
    jpf_mods_container_put_guest(self->container, cu_base);
    return nmp_mod_cu_forward_2(self, msg, USR_GRP_WALL_CONTROL, &res_info, sizeof(res_info));
}


NmpMsgFunRet
nmp_mod_cu_get_tw_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetTwRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_get_screen_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	JpfModCu *self;
	JpfGetScreen *req_info;
	JpfGetScreenRes res_info;
	JpfGuestBase *cu_base;
	JpfCu *cu;
	JpfNetIO *io;
	gint ret;
	memset(&res_info, 0, sizeof(res_info));

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);
	self = (JpfModCu*)app_obj;
	io = MSG_IO(msg);
	BUG_ON(!io);

	cu_base = jpf_mods_container_get_guest(self->container, io);
	if (G_UNLIKELY(!cu_base))
	{
		ret = -E_NOSUCHGUEST;
		jpf_warning("<JpfModCu> session:%s No such guest.", req_info->session);
		MSG_SET_RESPONSE(msg);
		memset(&res_info, 0, sizeof(res_info));
		SET_CODE(&res_info, -ret);
		jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

		return MFR_DELIVER_BACK;
	}

	cu = (JpfCu*)cu_base;
	NMP_COPY_VAL(req_info->user, cu->user->user_name, USER_NAME_LEN);
	jpf_mods_container_put_guest(self->container, cu_base);

	return nmp_mod_cu_forward_2(self, msg, USR_GRP_WALL_CONTROL, &res_info, sizeof(res_info));
}


NmpMsgFunRet
nmp_mod_cu_get_screen_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetScreenRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_get_scr_div_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetDivMode *req_info;
    JpfGetDivModeRes res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);
    memset(&res_info, 0, sizeof(res_info));
    self = (JpfModCu*)app_obj;
    return nmp_mod_cu_forward_2(self, msg, USR_GRP_WALL_CONTROL, &res_info, sizeof(res_info));
}


NmpMsgFunRet
nmp_mod_cu_get_scr_div_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetDivModeRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_get_scr_state_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetScrState *req_info;
    JpfGetScrStateRes res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);
    memset(&res_info, 0, sizeof(res_info));
    self = (JpfModCu*)app_obj;
    return nmp_mod_cu_forward_2(self, msg, USR_GRP_WALL_CONTROL, &res_info, sizeof(res_info));
}


NmpMsgFunRet
nmp_mod_cu_get_scr_state_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetScrStateRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    JpfGuestBase *cu_base;
    JpfNetIO *io;
    gint msg_id;

    msg_id = MSG_GETID(msg);
    cu_base = jpf_mods_container_get_guest_2(self->container, res_info->session);
    if (G_UNLIKELY(!cu_base))
    {
        jpf_warning("<JpfModCu> deliver msg '%s' failed, session:%s no such cu.",
            MESSAGE_ID_TO_STR(cms, msg_id), res_info->session);
        jpf_sysmsg_destroy(msg);

        return MFR_ACCEPTED;
    }

    io = IO_OF_GUEST(cu_base);
    BUG_ON(!io);

    jpf_sysmsg_attach_io(msg, io);
    jpf_mods_container_put_guest(self->container, cu_base);

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_cu_change_div_mode_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfChangeDivMode *req_info;
    JpfCuExecuteRes res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);
    memset(&res_info, 0, sizeof(res_info));
    self = (JpfModCu*)app_obj;
    return nmp_mod_cu_forward(self, msg, USR_GRP_WALL_CONTROL, BUSSLOT_POS_TW);
}


NmpMsgFunRet
nmp_mod_cu_change_div_mode_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfCuExecuteRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_run_step_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    tw_run_step_request *req_info;
    JpfCuExecuteRes res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);
    memset(&res_info, 0, sizeof(res_info));
    self = (JpfModCu*)app_obj;
  //  return nmp_mod_cu_forward_2(self, msg, USR_GRP_WALL_CONTROL, &res_info, sizeof(res_info));

    JpfNetIO *io;
    gint msg_id, rank, ret;

    io = MSG_IO(msg);
    BUG_ON(!io);

    ret = nmp_mod_cu_check_permissions(self, io, USR_GRP_WALL_CONTROL, &rank);
    if (G_LIKELY(!ret))
    {
    	nmp_mod_cu_check_rank(self, msg, USR_GRP_WALL_CONTROL, rank);
        MSG_SET_DSTPOS(msg, BUSSLOT_POS_TW);
        return MFR_DELIVER_AHEAD;
    }

    msg_id = MSG_GETID(msg);
    SET_CODE(&res_info, -ret);
    jpf_warning("<JpfModCu> deliver msg '%s' failed, user no such permission.",
            MESSAGE_ID_TO_STR(cms, msg_id));
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;

}


NmpMsgFunRet
nmp_mod_cu_run_step_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfCuExecuteRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_play_notify_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    tw_screen_to_cu *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    nmp_mod_cu_broadcast_to_all_user(self, msg);
    jpf_sysmsg_destroy(msg);

    return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_cu_full_screen_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    tw_operate *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    self = (JpfModCu*)app_obj;
    return nmp_mod_cu_forward(self, msg, USR_GRP_WALL_CONTROL, BUSSLOT_POS_TW);
}


NmpMsgFunRet
nmp_mod_cu_full_screen_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfCuExecuteRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_exit_full_screen_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    tw_operate *req_info;
    JpfCuExecuteRes res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);
    memset(&res_info, 0, sizeof(res_info));
    self = (JpfModCu*)app_obj;
    return nmp_mod_cu_forward(self, msg, USR_GRP_WALL_CONTROL, BUSSLOT_POS_TW);
}


NmpMsgFunRet
nmp_mod_cu_exit_full_screen_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfCuExecuteRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_clear_division_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    tw_operate *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    self = (JpfModCu*)app_obj;
    return nmp_mod_cu_forward(self, msg, USR_GRP_WALL_CONTROL, BUSSLOT_POS_TW);
}


NmpMsgFunRet
nmp_mod_cu_clear_division_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfCuExecuteRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_tw_operate_notify_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    self = (JpfModCu*)app_obj;

    nmp_mod_cu_broadcast_to_all_user(self, msg);
    jpf_sysmsg_destroy(msg);

    return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_cu_get_tour_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetTour *req_info;
    JpfGetTourRes res_info;
    JpfGuestBase *cu_base;
    gint ret;
    JpfCu *cu;
    JpfNetIO *io;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);
    memset(&res_info, 0, sizeof(res_info));
    self = (JpfModCu*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    cu_base  = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!cu_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModCu> session:%s No such guest.", req_info->session);
        MSG_SET_RESPONSE(msg);
        memset(&res_info, 0, sizeof(res_info));
        SET_CODE(&res_info, -ret);
        jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

        return MFR_DELIVER_BACK;
    }

    cu = (JpfCu*)cu_base;
    strncpy(req_info->username, cu->user->user_name, USER_NAME_LEN - 1);
    jpf_mods_container_put_guest(self->container, cu_base);

    return nmp_mod_cu_forward_2(self, msg, USR_GRP_WALL_CONTROL,
        &res_info, sizeof(res_info));
}


NmpMsgFunRet
nmp_mod_cu_get_tour_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetTourRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_get_tour_step_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetTourStep *req_info;
    JpfGetTourStepRes res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);
    memset(&res_info, 0, sizeof(res_info));
    self = (JpfModCu*)app_obj;
    return nmp_mod_cu_forward_2(self, msg, USR_GRP_WALL_CONTROL,
        &res_info, sizeof(res_info));
}


NmpMsgFunRet
nmp_mod_cu_get_tour_step_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetTourStepRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_run_tour_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    tw_run_tour_request *req_info;
    JpfCuExecuteRes res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);
    memset(&res_info, 0, sizeof(res_info));
    self = (JpfModCu*)app_obj;
   return nmp_mod_cu_forward(self, msg, USR_GRP_WALL_CONTROL,BUSSLOT_POS_TW);
}


NmpMsgFunRet
nmp_mod_cu_run_tour_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfCuExecuteRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_stop_tour_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    tw_stop_tour_request *req_info;
    JpfCuExecuteRes res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);
    memset(&res_info, 0, sizeof(res_info));
    self = (JpfModCu*)app_obj;
   return nmp_mod_cu_forward(self, msg, USR_GRP_WALL_CONTROL,BUSSLOT_POS_TW);
}


NmpMsgFunRet
nmp_mod_cu_stop_tour_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfCuExecuteRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_get_group_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	JpfModCu *self;
	JpfGetGroup *req_info;
	JpfGetGroupRes res_info;
	JpfGuestBase *cu_base;
	JpfCu *cu;
	JpfNetIO *io;
	gint ret;
	memset(&res_info, 0, sizeof(res_info));

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);
	self = (JpfModCu*)app_obj;
	io = MSG_IO(msg);
	BUG_ON(!io);

	cu_base = jpf_mods_container_get_guest(self->container, io);
	if (G_UNLIKELY(!cu_base))
	{
		ret = -E_NOSUCHGUEST;
		jpf_warning("<JpfModCu> session:%s No such guest.", req_info->session);
		MSG_SET_RESPONSE(msg);
		memset(&res_info, 0, sizeof(res_info));
		SET_CODE(&res_info, -ret);
		jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

		return MFR_DELIVER_BACK;
	}

	cu = (JpfCu*)cu_base;
	NMP_COPY_VAL(req_info->user, cu->user->user_name, USER_NAME_LEN);
	jpf_mods_container_put_guest(self->container, cu_base);

	return nmp_mod_cu_forward_2(self, msg, USR_GRP_WALL_CONTROL,
		&res_info, sizeof(res_info));
}


NmpMsgFunRet
nmp_mod_cu_get_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetGroupRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_run_group_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    tw_run_group_request *req_info;
    JpfCuExecuteRes res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);
    memset(&res_info, 0, sizeof(res_info));
    self = (JpfModCu*)app_obj;
   return nmp_mod_cu_forward(self, msg, USR_GRP_WALL_CONTROL,BUSSLOT_POS_TW);
}


NmpMsgFunRet
nmp_mod_cu_run_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfCuExecuteRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_stop_group_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    tw_stop_group_request *req_info;
    JpfCuExecuteRes res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);
    memset(&res_info, 0, sizeof(res_info));
    self = (JpfModCu*)app_obj;
   return nmp_mod_cu_forward(self, msg, USR_GRP_WALL_CONTROL,BUSSLOT_POS_TW);
}


NmpMsgFunRet
nmp_mod_cu_stop_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfCuExecuteRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_get_license_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetLicenseInfo *req_info;
    JpfGetLicenseInfoRes res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);
    memset(&res_info, 0, sizeof(res_info));
    self = (JpfModCu*)app_obj;
   return nmp_mod_cu_forward(self, msg, USR_GRP_PARAMETER_SET,BUSSLOT_POS_WDD);
}


NmpMsgFunRet
nmp_mod_cu_get_license_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfGetLicenseInfoRes *res_info;

    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_get_tw_license_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfNetIO *io;
    JpfGuestBase *cu_base;
    JpfGetTwLicenseInfo *req_info;
    JpfGetTwLicenseInfoRes res_info;
    NmpMsgID msg_id;
    gint ret = 0;
    JpfResourcesCap res_cap;

    self = (JpfModCu*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    cu_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!cu_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModCu> session:%s No such guest.", req_info->session);
    }
    else
    {
        memset(&res_cap, 0, sizeof(res_cap));
        nmp_mod_get_resource_cap(&res_cap);
        res_info.tw_auth_type = res_cap.modules_data[SYS_MODULE_TW];
        jpf_mods_container_put_guest(self->container, cu_base);
     }

    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_cu_alarm_link_map_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfAmsActionMap *res_info = NULL;
    JpfModCu *self;
    gint i;

    self = NMP_MODCU(app_obj);
    res_info = MSG_GET_DATA(msg);

    for(i = 0; i < res_info->cu_count; i++)
        nmp_mod_cu_deliver_msg(self, res_info->cu_list[i].username, msg);

    jpf_sysmsg_destroy(msg);

    return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_cu_modify_user_pwd_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfCuModifyUserPwd *req_info;
    JpfCuModifyUserPwdRes res_info;
    JpfGuestBase *cu_base;
    gint ret;
    JpfCu *cu;
    JpfNetIO *io;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);
    memset(&res_info, 0, sizeof(res_info));
    self = (JpfModCu*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    cu_base  = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!cu_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModCu> session:%s No such guest.", req_info->session);
        MSG_SET_RESPONSE(msg);
        SET_CODE(&res_info, -ret);
        jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

        return MFR_DELIVER_BACK;
    }

    cu = (JpfCu*)cu_base;
    strncpy(req_info->username, cu->user->user_name, USER_NAME_LEN - 1);
    jpf_mods_container_put_guest(self->container, cu_base);
    return nmp_mod_cu_forward_2(self, msg, USR_GRP_PARAMETER_SET, &res_info, sizeof(res_info));
}


NmpMsgFunRet
nmp_mod_cu_modify_user_pwd_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    JpfModCu *self;
    JpfCuModifyUserPwdRes *res_info;
	JpfCu *cu;
    self = (JpfModCu*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);


    JpfGuestBase *cu_base;
    JpfNetIO *io;
    gint msg_id;

    msg_id = MSG_GETID(msg);
    cu_base = jpf_mods_container_get_guest_2(self->container, res_info->session);
    if (G_UNLIKELY(!cu_base))
    {
        jpf_warning("<JpfModCu> deliver msg '%s' failed, session:%s no such cu.",
            MESSAGE_ID_TO_STR(cms, msg_id), res_info->session);
        jpf_sysmsg_destroy(msg);

        return MFR_ACCEPTED;
    }

    io = IO_OF_GUEST(cu_base);
    BUG_ON(!io);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_attach_io(msg, io);
    if (!RES_CODE(res_info))
    {
		cu = (JpfCu*)cu_base;
		strncpy(cu->user->user_passwd, res_info->new_password, MAX_PASSWD_LEN - 1);
    }
    jpf_mods_container_put_guest(self->container, cu_base);

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_cu_query_guid_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	JpfModCu *self;
	JpfCuQueryGuid *req_info;
	JpfCuQueryGuidRes res_info;
	memset(&res_info, 0, sizeof(res_info));

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);
	self = (JpfModCu*)app_obj;

	return nmp_mod_cu_forward_2(self, msg, USR_GRP_NONE_PERMISSION, &res_info, sizeof(res_info));
}


NmpMsgFunRet
nmp_mod_cu_query_guid_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	JpfModCu *self;
	JpfCuQueryGuidRes *res_info;

	self = (JpfModCu*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_query_screen_id_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	JpfModCu *self;
	JpfCuQueryScreenID *req_info;
	JpfCuQueryScreenIDRes res_info;
	memset(&res_info, 0, sizeof(res_info));

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);
	self = (JpfModCu*)app_obj;

	return nmp_mod_cu_forward_2(self, msg, USR_GRP_NONE_PERMISSION, &res_info, sizeof(res_info));
}


NmpMsgFunRet
nmp_mod_cu_query_screen_id_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	JpfModCu *self;
	JpfCuQueryScreenIDRes *res_info;

	self = (JpfModCu*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_query_user_guids_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	JpfModCu *self;
	JpfCuQueryUserGuids *req_info;
	JpfCuQueryUserGuidsRes res_info;
	memset(&res_info, 0, sizeof(res_info));

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);
	self = (JpfModCu*)app_obj;

	return nmp_mod_cu_forward_2(self, msg, USR_GRP_NONE_PERMISSION, &res_info, sizeof(res_info));
}


NmpMsgFunRet
nmp_mod_cu_query_user_guids_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	JpfModCu *self;
	JpfCuQueryUserGuidsRes *res_info;

	self = (JpfModCu*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_set_user_guids_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	JpfModCu *self;
	JpfCuSetUserGuids *req_info;
	JpfResult res_info;
	memset(&res_info, 0, sizeof(res_info));

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);
	self = (JpfModCu*)app_obj;

	return nmp_mod_cu_forward_2(self, msg, USR_GRP_NONE_PERMISSION, &res_info, sizeof(res_info));
}


NmpMsgFunRet
nmp_mod_cu_set_user_guids_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	JpfModCu *self;
	JpfResult *res_info;

	self = (JpfModCu*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_set_screen_num_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	JpfModCu *self;
	JpfCuSetScreenNum *req_info;
	JpfResult res_info;
	memset(&res_info, 0, sizeof(res_info));

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);
	self = (JpfModCu*)app_obj;

	return nmp_mod_cu_forward_2(self, msg, USR_GRP_NONE_PERMISSION, &res_info, sizeof(res_info));
}


NmpMsgFunRet
nmp_mod_cu_set_screen_num_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	JpfModCu *self;
	JpfResult *res_info;

	self = (JpfModCu*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_query_tour_id_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	JpfModCu *self;
	JpfCuQueryTourID *req_info;
	JpfCuQueryTourIDRes res_info;
	memset(&res_info, 0, sizeof(res_info));

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);
	self = (JpfModCu*)app_obj;

	return nmp_mod_cu_forward_2(self, msg, USR_GRP_NONE_PERMISSION, &res_info, sizeof(res_info));
}


NmpMsgFunRet
nmp_mod_cu_query_tour_id_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	JpfModCu *self;
	JpfCuQueryTourIDRes *res_info;

	self = (JpfModCu*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_set_tour_num_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	JpfModCu *self;
	JpfCuSetTourNum *req_info;
	JpfResult res_info;
	memset(&res_info, 0, sizeof(res_info));

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);
	self = (JpfModCu*)app_obj;

	return nmp_mod_cu_forward_2(self, msg, USR_GRP_NONE_PERMISSION, &res_info, sizeof(res_info));
}


NmpMsgFunRet
nmp_mod_cu_set_tour_num_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	JpfModCu *self;
	JpfResult *res_info;

	self = (JpfModCu*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_query_group_id_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	JpfModCu *self;
	JpfCuQueryGroupID *req_info;
	JpfCuQueryGroupIDRes res_info;
	memset(&res_info, 0, sizeof(res_info));

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);
	self = (JpfModCu*)app_obj;

	return nmp_mod_cu_forward_2(self, msg, USR_GRP_NONE_PERMISSION, &res_info, sizeof(res_info));
}


NmpMsgFunRet
nmp_mod_cu_query_group_id_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	JpfModCu *self;
	JpfCuQueryGroupIDRes *res_info;

	self = (JpfModCu*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	return nmp_mod_cu_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_cu_set_group_num_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	JpfModCu *self;
	JpfCuSetGroupNum *req_info;
	JpfResult res_info;
	memset(&res_info, 0, sizeof(res_info));

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);
	self = (JpfModCu*)app_obj;

	return nmp_mod_cu_forward_2(self, msg, USR_GRP_NONE_PERMISSION, &res_info, sizeof(res_info));
}


NmpMsgFunRet
nmp_mod_cu_set_group_num_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	JpfModCu *self;
	JpfResult *res_info;

	self = (JpfModCu*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	return nmp_mod_cu_backward(self, msg, res_info->session);
}



void
nmp_mod_cu_register_msg_handler(JpfModCu *self)
{
    NmpAppMod *super_self = (NmpAppMod*)self;

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_CU_ONLINE,
        nmp_mod_cu_online_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_CHANGED_PU_ONLINE_STATE,
        NULL,
        nmp_mod_cu_change_pu_online_state_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_BROADCAST_GENERAL_MSG,
        NULL,
        nmp_mod_cu_broadcast_generic_msg_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEC_ONLINE_STATE_NOTIFY,
        NULL,
        nmp_mod_cu_broadcast_generic_msg_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_NOTIFY_MODIFY_DOMAIN,
        NULL,
        nmp_mod_cu_notify_modify_domain_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MSG_DEL_USER,
        NULL,
        nmp_mod_cu_user_force_usr_offline_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MSG_DEL_USER_GROUP,
        NULL,
        nmp_mod_cu_group_force_usr_offline_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_CU_HEART,
        nmp_mod_cu_heart_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_ALL_AREA,
        nmp_mod_cu_get_all_area_f,
        nmp_mod_cu_get_all_area_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_AREA_INFO,
        nmp_mod_cu_get_area_info_f,
        nmp_mod_cu_get_area_info_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_DEVICE_LIST,
        nmp_mod_cu_get_device_list_f,
        nmp_mod_cu_get_device_list_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_AREA_DEVICE_INFO,
        nmp_mod_cu_get_area_device_f,
        nmp_mod_cu_get_area_device_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_MEDIA_URL,
        nmp_mod_cu_get_media_url_f,
        nmp_mod_cu_get_media_url_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
      	 MESSAGE_GET_PLATFORM_INFO,
        nmp_mod_cu_get_platform_info_f,
      	 nmp_mod_cu_get_platform_info_b,
      	 0
    );

   nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_PLATFORM_INFO,
        nmp_mod_cu_set_platform_info_f,
        nmp_mod_cu_set_platform_info_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_DEVICE_INFO,
        nmp_mod_cu_get_device_info_f,
        nmp_mod_cu_get_device_info_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_NETWORK_INFO,
        nmp_mod_cu_get_network_info_f,
        nmp_mod_cu_get_network_info_b,
        0
    );

   nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_NETWORK_INFO,
        nmp_mod_cu_set_network_info_f,
        nmp_mod_cu_set_network_info_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_PPPOE_INFO,
        nmp_mod_cu_get_pppoe_info_f,
        nmp_mod_cu_get_pppoe_info_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_PPPOE_INFO,
        nmp_mod_cu_set_pppoe_info_f,
        nmp_mod_cu_set_pppoe_info_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_ENCODE_PARA,
        nmp_mod_cu_get_encode_para_f,
        nmp_mod_cu_get_encode_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_ENCODE_PARA,
        nmp_mod_cu_set_encode_para_f,
        nmp_mod_cu_set_encode_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_DISPLAY_PARA,
        nmp_mod_cu_get_display_para_f,
        nmp_mod_cu_get_display_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_DEF_DISPLAY_PARA,
        nmp_mod_cu_get_display_para_f,
        nmp_mod_cu_get_display_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_DISPLAY_PARA,
        nmp_mod_cu_set_display_para_f,
        nmp_mod_cu_set_display_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_OSD_PARA,
        nmp_mod_cu_get_OSD_para_f,
        nmp_mod_cu_get_OSD_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_OSD_PARA,
        nmp_mod_cu_set_OSD_para_f,
        nmp_mod_cu_set_OSD_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_MOVE_DETECT,
        nmp_mod_cu_get_move_detection_f,
        nmp_mod_cu_get_move_detection_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_MOVE_DETECT,
        nmp_mod_cu_set_move_detection_f,
        nmp_mod_cu_set_move_detection_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_VIDEO_LOST,
        nmp_mod_cu_get_video_lost_f,
        nmp_mod_cu_get_video_lost_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_VIDEO_LOST,
        nmp_mod_cu_set_video_lost_f,
        nmp_mod_cu_set_video_lost_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_HIDE_PARA,
        nmp_mod_cu_get_hide_para_f,
        nmp_mod_cu_get_hide_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_HIDE_PARA,
        nmp_mod_cu_set_hide_para_f,
        nmp_mod_cu_set_hide_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_HIDE_ALARM,
        nmp_mod_cu_get_hide_alarm_f,
        nmp_mod_cu_get_hide_alarm_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_HIDE_ALARM,
        nmp_mod_cu_set_hide_alarm_f,
        nmp_mod_cu_set_hide_alarm_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_IO_ALARM,
        nmp_mod_cu_get_io_alarm_f,
        nmp_mod_cu_get_io_alarm_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_IO_ALARM,
        nmp_mod_cu_set_io_alarm_f,
        nmp_mod_cu_set_io_alarm_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ALARM_LINK_IO,
        nmp_mod_cu_link_io_alarm_f,
        nmp_mod_cu_link_io_alarm_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_RECORD_PARA,
        nmp_mod_cu_get_record_para_f,
        nmp_mod_cu_get_record_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_RECORD_PARA,
        nmp_mod_cu_set_record_para_f,
        nmp_mod_cu_set_record_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_JOINT_PARA,
        nmp_mod_cu_get_joint_para_f,
        nmp_mod_cu_get_joint_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_JOINT_PARA,
        nmp_mod_cu_set_joint_para_f,
        nmp_mod_cu_set_joint_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_PTZ_PARA,
        nmp_mod_cu_get_PTZ_para_f,
        nmp_mod_cu_get_PTZ_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_PTZ_PARA,
        nmp_mod_cu_set_PTZ_para_f,
        nmp_mod_cu_set_PTZ_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_CONTROL_PTZ,
        nmp_mod_cu_control_PTZ_f,
        nmp_mod_cu_control_PTZ_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_PRESET_POINT,
        nmp_mod_cu_get_preset_point_f,
        nmp_mod_cu_get_preset_point_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_PRESET_POINT,
        nmp_mod_cu_set_preset_point_f,
        nmp_mod_cu_set_preset_point_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_CRUISE_WAY_SET,
        nmp_mod_cu_get_cruise_way_set_f,
        nmp_mod_cu_get_cruise_way_set_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_CRUISE_WAY,
        nmp_mod_cu_get_cruise_way_f,
        nmp_mod_cu_get_cruise_way_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_CRUISE_WAY,
        nmp_mod_cu_add_cruise_way_f,
        nmp_mod_cu_add_cruise_way_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_CRUISE_WAY,
        nmp_mod_cu_modify_cruise_way_f,
        nmp_mod_cu_modify_cruise_way_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_CRUISE_WAY,
        nmp_mod_cu_set_cruise_way_f,
        nmp_mod_cu_set_cruise_way_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_3D_CONTROL,
        nmp_mod_cu_3D_control_f,
        nmp_mod_cu_3D_control_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_3D_GOBACK,
        nmp_mod_cu_3D_goback_f,
        nmp_mod_cu_3D_goback_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_SERIAL_PARA,
        nmp_mod_cu_get_serial_para_f,
        nmp_mod_cu_get_serial_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_SERIAL_PARA,
        nmp_mod_cu_set_serial_para_f,
        nmp_mod_cu_set_serial_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_DEVICE_TIME,
        nmp_mod_cu_get_device_time_f,
        nmp_mod_cu_get_device_time_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_DEVICE_TIME,
        nmp_mod_cu_set_device_time_f,
        nmp_mod_cu_set_device_time_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_NTP_INFO,
        nmp_mod_cu_get_ntp_info_f,
        nmp_mod_cu_get_ntp_info_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_NTP_INFO,
        nmp_mod_cu_set_ntp_info_f,
        nmp_mod_cu_set_ntp_info_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_FTP_PARA,
        nmp_mod_cu_get_ftp_para_f,
        nmp_mod_cu_get_ftp_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_FTP_PARA,
        nmp_mod_cu_set_ftp_para_f,
        nmp_mod_cu_set_ftp_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_SMTP_PARA,
        nmp_mod_cu_get_smtp_para_f,
        nmp_mod_cu_get_smtp_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_SMTP_PARA,
        nmp_mod_cu_set_smtp_para_f,
        nmp_mod_cu_set_smtp_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_UPNP_PARA,
        nmp_mod_cu_get_upnp_para_f,
        nmp_mod_cu_get_upnp_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_UPNP_PARA,
        nmp_mod_cu_set_upnp_para_f,
        nmp_mod_cu_set_upnp_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_TRANSPARENT_PARA,
        nmp_mod_cu_get_transparent_para_f,
        nmp_mod_cu_get_transparent_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_TRANSPARENT_PARA,
        nmp_mod_cu_set_transparent_para_f,
        nmp_mod_cu_set_transparent_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_DDNS_PARA,
        nmp_mod_cu_get_ddns_para_f,
        nmp_mod_cu_get_ddns_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_DDNS_PARA,
        nmp_mod_cu_set_ddns_para_f,
        nmp_mod_cu_set_ddns_para_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_DISK_INFO,
        nmp_mod_cu_get_disk_info_f,
        nmp_mod_cu_get_disk_info_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_RESOLUTION_INFO,
        nmp_mod_cu_get_resolution_info_f,
        nmp_mod_cu_get_resolution_info_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_RESOLUTION_INFO,
        nmp_mod_cu_set_resolution_info_f,
        nmp_mod_cu_set_resolution_info_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_IRCUTCONTROL_INFO,
        nmp_mod_cu_get_ircut_control_info_f,
        nmp_mod_cu_get_ircut_control_info_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_IRCUTCONTROL_INFO,
        nmp_mod_cu_set_ircut_control_info_f,
        nmp_mod_cu_set_ircut_control_info_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_FORMAT_DISK,
        nmp_mod_cu_format_disk_f,
        nmp_mod_cu_format_disk_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SUBMIT_FORMAT_POS,
        NULL,
        nmp_mod_cu_submit_format_pro_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SUBMIT_ALARM,
        NULL,
        nmp_mod_cu_submit_alarm_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_ALARM,
        nmp_mod_cu_get_alarm_f,
        nmp_mod_cu_get_alarm_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_ALARM_STATE,
        nmp_mod_cu_get_alarm_state_f,
        nmp_mod_cu_get_alarm_state_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEAL_ALARM,
        nmp_mod_cu_deal_alarm_f,
        nmp_mod_cu_deal_alarm_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_STORE_LOG,
        nmp_mod_cu_get_store_log_f,
        nmp_mod_cu_get_store_log_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_MSS_STORE_LOG,
        nmp_mod_cu_get_mss_store_log_f,
        nmp_mod_cu_get_mss_store_log_b,
        0
    );


    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_FIRMWARE_UPGRADE,
        nmp_mod_cu_firmware_upgrade_f,
        nmp_mod_cu_firmware_upgrade_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_CONTROL_DEVICE,
        nmp_mod_cu_control_device_f,
        nmp_mod_cu_control_device_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_GU_MSS,
        nmp_mod_cu_get_gu_mss_f,
        nmp_mod_cu_get_gu_mss_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_DEFENCE_AREA,
        nmp_mod_cu_get_defence_area_f,
        nmp_mod_cu_get_defence_area_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_DEFENCE_MAP,
        nmp_mod_cu_get_defence_map_f,
        nmp_mod_cu_get_defence_map_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_DEFENCE_GU,
        nmp_mod_cu_get_defence_gu_f,
        nmp_mod_cu_get_defence_gu_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_MAP_HREF,
        nmp_mod_cu_get_map_href_f,
        nmp_mod_cu_get_map_href_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_GU_MAP_LOCATION,
        nmp_mod_cu_get_gu_map_location_f,
        nmp_mod_cu_get_gu_map_location_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_TW,
        nmp_mod_cu_get_tw_f,
        nmp_mod_cu_get_tw_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_SCREEN,
        nmp_mod_cu_get_screen_f,
        nmp_mod_cu_get_screen_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_SCR_DIV,
        nmp_mod_cu_get_scr_div_f,
        nmp_mod_cu_get_scr_div_b,
        0
    );
    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_SCR_STATE,
        nmp_mod_cu_get_scr_state_f,
        nmp_mod_cu_get_scr_state_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_CHANGE_DIV_MODE,
        nmp_mod_cu_change_div_mode_f,
        nmp_mod_cu_change_div_mode_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_TW_RUN_STEP,
        nmp_mod_cu_run_step_f,
        nmp_mod_cu_run_step_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_TW_PLAY_NOTIFY,
        NULL,
        nmp_mod_cu_play_notify_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_FULL_SCREEN,
        nmp_mod_cu_full_screen_f,
        nmp_mod_cu_full_screen_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_EXIT_FULL_SCREEN,
        nmp_mod_cu_exit_full_screen_f,
        nmp_mod_cu_exit_full_screen_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_TW_CLEAR_DIVISION,
        nmp_mod_cu_clear_division_f,
        nmp_mod_cu_clear_division_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_TW_OPERATE_NOTIFY,
        NULL,
        nmp_mod_cu_tw_operate_notify_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_TOUR,
        nmp_mod_cu_get_tour_f,
        nmp_mod_cu_get_tour_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_TOUR_STEP,
        nmp_mod_cu_get_tour_step_f,
        nmp_mod_cu_get_tour_step_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_TW_RUN_TOUR,
        nmp_mod_cu_run_tour_f,
        nmp_mod_cu_run_tour_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_TW_STOP_TOUR,
        nmp_mod_cu_stop_tour_f,
        nmp_mod_cu_stop_tour_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_GROUP,
        nmp_mod_cu_get_group_f,
        nmp_mod_cu_get_group_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_TW_RUN_GROUP,
        nmp_mod_cu_run_group_f,
        nmp_mod_cu_run_group_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_TW_STOP_GROUP,
        nmp_mod_cu_stop_group_f,
        nmp_mod_cu_stop_group_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_LICENSE_INFO,
        nmp_mod_cu_get_license_info_f,
        nmp_mod_cu_get_license_info_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_TW_AUTH_INFO,
        nmp_mod_cu_get_tw_license_info_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ALARM_LINK_MAP,
        NULL,
        nmp_mod_cu_alarm_link_map_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_CU_MODIFY_USER_PWD,
        nmp_mod_cu_modify_user_pwd_f,
        nmp_mod_cu_modify_user_pwd_b,
        0
    );

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_QUERY_GUID,
		nmp_mod_cu_query_guid_f,
		nmp_mod_cu_query_guid_b,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_QUERY_SCREEN_ID,
		nmp_mod_cu_query_screen_id_f,
		nmp_mod_cu_query_screen_id_b,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_QUERY_USER_GUIDS,
		nmp_mod_cu_query_user_guids_f,
		nmp_mod_cu_query_user_guids_b,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_SET_USER_GUIDS,
		nmp_mod_cu_set_user_guids_f,
		nmp_mod_cu_set_user_guids_b,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_SET_SCREEN_NUM,
		nmp_mod_cu_set_screen_num_f,
		nmp_mod_cu_set_screen_num_b,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_QUERY_TOUR_ID,
		nmp_mod_cu_query_tour_id_f,
		nmp_mod_cu_query_tour_id_b,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_SET_TOUR_NUM,
		nmp_mod_cu_set_tour_num_f,
		nmp_mod_cu_set_tour_num_b,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_QUERY_GROUP_ID,
		nmp_mod_cu_query_group_id_f,
		nmp_mod_cu_query_group_id_b,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_SET_GROUP_NUM,
		nmp_mod_cu_set_group_num_f,
		nmp_mod_cu_set_group_num_b,
		0
	);
}


//:~ End
