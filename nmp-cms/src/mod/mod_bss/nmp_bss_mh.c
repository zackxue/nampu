#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "nmp_mod_bss.h"
#include "nmp_message.h"
#include "nmp_errno.h"
#include "nmp_memory.h"
#include "nmp_shared.h"
#include "nmp_sysctl.h"
#include "nmp_internal_msg.h"
#include "nmp_bss_struct.h"
#include "search_device.h"
#include "nmp_search_pu.h"

USING_MSG_ID_MAP(cms);

#define BSS_TIMEOUT    (45 * 1000)
#define POPEN_OPERATE_BASE	"system_network_config"
#define POPEN_OPERATE_BASE2	"system_generic_config"

#define POPEN_GET_NET_INTERFACE	POPEN_OPERATE_BASE" get_net_interface"
#define POPEN_GET_NETWORK_CONFIG	POPEN_OPERATE_BASE" get_network_config"
#define POPEN_SET_NETWORK_CONFIG	POPEN_OPERATE_BASE" set_network_config"
#define POPEN_SET_NETWORK_CONFIG_END	POPEN_OPERATE_BASE" set_network_config_end"
#define POPEN_GET_IPS_BESIDES		POPEN_OPERATE_BASE" get_ips_besides"

#define POPEN_GET_MODULES_FLAG	POPEN_OPERATE_BASE2" get_modules_flag"
#define POPEN_GET_MDS_STATE		POPEN_OPERATE_BASE2" get_mds_state"
#define POPEN_GET_MSS_STATE		POPEN_OPERATE_BASE2" get_mss_state"
#define POPEN_GET_IVS_STATE		POPEN_OPERATE_BASE2" get_ivs_state"
#define MAX_STATE_NUM		(0xffff)

#define QUERY_INFO(str, type)	(strstr(str, type))
#define GET_INFO_ADDR(str)		(strchr(str, ':') + 1)

/*
 *	operate res must be NmpMsgErrCode!
 */
#define NMP_DEAL_GET_BSS_USR_NAME(app_obj, msg, name, ret) do {	\
	ret = nmp_mod_bss_get_admin_name(app_obj, msg, name);	\
	if (ret) {	\
		return nmp_mod_deal_get_admin_name_failed(app_obj, msg, ret);	\
	}	\
} while (0)

#define NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, name) do {	\
	name[USER_NAME_LEN - 1] = '\0';	\
	strncpy(name, ID_OF_GUEST(bss_base), USER_NAME_LEN - 1);	\
} while (0)

#define NMP_CREATE_MSG_TO_LOG(msg_id, priv_p, size) do { \
	NmpSysMsg *_msg_to_log; \
	_msg_to_log = nmp_sysmsg_new_2(msg_id, priv_p, size, 0); \
	MSG_SET_DSTPOS(_msg_to_log, BUSSLOT_POS_LOG); \
	nmp_app_obj_deliver_out((NmpAppObj *)self, _msg_to_log); \
} while (0)


gint
nmp_mod_bss_get_admin_name(NmpAppObj *app_obj, NmpSysMsg *msg, gchar *name)
{
    NmpModBss *self;
    NmpGuestBase *bss_base;
    NmpNetIO *io;
    NmpMsgID msg_id;
    gint ret = 0;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);
    msg_id = MSG_GETID(msg);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
	 nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        name[USER_NAME_LEN - 1] = '\0';
        strncpy(name, ID_OF_GUEST(bss_base), USER_NAME_LEN - 1);
        nmp_mods_container_put_guest(self->container, bss_base);
    }

    return ret;
}


NmpMsgFunRet
nmp_mod_bss_forward(NmpModBss *self, NmpSysMsg *msg, const gchar *id_str)
{
    NmpErrRes	code;
    NmpNetIO *io;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    io = MSG_IO(msg);
    BUG_ON(!io);
    msg_id = MSG_GETID(msg);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        MSG_SET_RESPONSE(msg);
        memset(&code, 0, sizeof(code));
        SET_CODE(&code, ret);
	 if (id_str)
	 	strncpy(code.session, id_str, USER_NAME_LEN - 1);
        nmp_sysmsg_set_private_2(msg, &code, sizeof(code));

        return MFR_DELIVER_BACK;
    }

    MSG_SET_DSTPOS(msg, BUSSLOT_POS_MSS);
    nmp_mods_container_put_guest(self->container, bss_base);

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_bss_backward(NmpModBss *self, NmpSysMsg *msg, const gchar *id_str)
{
    NmpGuestBase *bss_base;
    NmpNetIO *io;
    gint msg_id;

    msg_id = MSG_GETID(msg);
    bss_base = nmp_mods_container_get_guest_2(self->container, id_str);
    if (G_UNLIKELY(!bss_base))
    {
        nmp_warning("<NmpModBss> deliver msg '%s' failed, session:%s no such name.",
            MESSAGE_ID_TO_STR(cms, msg_id), id_str);
        nmp_sysmsg_destroy(msg);

        return MFR_ACCEPTED;
    }

    io = IO_OF_GUEST(bss_base);
    BUG_ON(!io);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_attach_io(msg, io);
    nmp_mods_container_put_guest(self->container, bss_base);

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_bss_forward_2(NmpAppObj *app_obj, NmpSysMsg *msg, gchar *name)
{
    gint ret;
    NmpMsgErrCode res_info;

    ret = nmp_mod_bss_get_admin_name(app_obj, msg, name);
    if (!ret)
    {
        MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
        return MFR_DELIVER_AHEAD;
    }

    memset(&res_info, 0, sizeof(res_info));
    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_forward_to_log(NmpAppObj *app_obj, NmpSysMsg *msg, gchar *name)
{
    gint ret;
    NmpMsgErrCode res_info;

    ret = nmp_mod_bss_get_admin_name(app_obj, msg, name);
    if (!ret)
    {
        MSG_SET_DSTPOS(msg, BUSSLOT_POS_LOG);
        return MFR_DELIVER_AHEAD;
    }

    memset(&res_info, 0, sizeof(res_info));
    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_deal_get_admin_name_failed(NmpAppObj *app_obj, NmpSysMsg *msg, gint ret)
{
	NmpMsgErrCode res_info;

	memset(&res_info, 0, sizeof(res_info));
	MSG_SET_RESPONSE(msg);
	SET_CODE(&res_info, -ret);
	nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

	return MFR_DELIVER_BACK;
}


static __inline__ gint
nmp_mod_bss_admin_login(NmpModBss *self, NmpNetIO *io,  NmpMsgID msg_id,
    NmpBssLoginInfo *admin, NmpBssLoginRes *res)
{
    gint ret;
    NmpID conflict;

    G_ASSERT(self != NULL && io != NULL && admin != NULL && res != NULL);

    ret = nmp_mod_bss_new_admin(self, io,  admin->admin_name, &conflict);
    if (G_UNLIKELY(ret))
        return ret;

    ret = nmp_mod_bss_sync_req(self, msg_id, admin,
         sizeof(NmpBssLoginInfo), res, sizeof(NmpBssLoginRes));

    return ret;
}


NmpMsgFunRet
nmp_mod_bss_login_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpBssLoginInfo *req_info;
    NmpBssLoginRes  res_info;
    NmpMsgID msg_id;
    gint ret;
    NmpResourcesCap res_cap;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    req_info = (NmpBssLoginInfo*)MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    ret = nmp_mod_bss_admin_login(self, io, msg_id, req_info, &res_info);
    if (ret)
    {
        nmp_warning(
            "<NmpModBss> admin:%s login failed, err:%d",
            req_info->admin_name, ret
        );

        SET_CODE(&res_info, -ret);
        MSG_SET_RESPONSE(msg);
        nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

        nmp_app_obj_deliver_in((NmpAppObj*)self, msg);
        nmp_mod_acc_release_io((NmpModAccess*)self, io);
        nmp_mod_container_del_io(self->container, io);

        return MFR_ACCEPTED;
    }
    else
    {
        nmp_print(
            "<NmpModBss> admin:%s login ok",
            req_info->admin_name
        );
        memset(&res_cap, 0, sizeof(res_cap));
        nmp_mod_get_resource_cap(&res_cap);
        res_info.module_sets = res_cap.module_bits;
        nmp_net_set_io_ttd(io, BSS_TIMEOUT);
        SET_CODE(&res_info, -ret);
        MSG_SET_RESPONSE(msg);
        nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

        return MFR_DELIVER_BACK;
    }
}


NmpMsgFunRet
nmp_mod_bss_heart_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpGuestBase *bss_base;
    NmpBssHeart *req_info;
    NmpBssHeartResp res_info;
    NmpMsgID msg_id;
    gint ret = 0;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest_2(self->container, req_info->admin_name);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> admin name:%s No such guest.", req_info->admin_name);
    }
    else
    {
        //snprintf(res_info.server_time, TIME_INFO_LEN, "%d", time(NULL));
        nmp_get_utc_time(res_info.server_time);
        printf("res_info.server_time=%s\n",res_info.server_time);
        nmp_mods_container_put_guest(self->container, bss_base);
     }

    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_validata_admin_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpAdminInfo *req_info;
    NmpMsgErrCode res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        ret = nmp_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(NmpAdminInfo), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> validata admin:%s failed, err:%d",
                req_info->admin_name, ret
            );
        }
        else
        {
            nmp_print(
                "<NmpModBss> validata admin:%s ok",
                req_info->admin_name
            );
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_validata_user_group_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpValidateUserGroup *req_info;
    NmpMsgErrCode res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        ret = nmp_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(NmpValidateUserGroup), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> validata user group:%s exist, err:%d",
                req_info->group_name, ret
            );
        }
        else
        {
            nmp_print(
                "<NmpModBss> validata user group:%s inexist",
                req_info->group_name
            );
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_validata_user_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpValidateUser *req_info;
    NmpMsgErrCode res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        ret = nmp_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(NmpValidateUser), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> validata user:%s failed, err:%d",
                req_info->username, ret
            );
        }
        else
        {
            nmp_print(
                "<NmpModBss> validata user:%s ok",
                req_info->username
            );
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_validata_area_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpValidateArea *req_info;
    NmpMsgErrCode res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        ret = nmp_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(NmpValidateArea), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> validata area:%s failed, err:%d",
                req_info->area_name, ret
            );
        }
        else
        {
            nmp_print(
                "<NmpModBss> validata area:%s ok",
                req_info->area_name
            );
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_validata_pu_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpValidatePu *req_info;
    NmpMsgErrCode res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        ret = nmp_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(NmpValidatePu), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> validata pu:%s failed, err:%d",
                req_info->puid, ret
            );
        }
        else
        {
            nmp_print(
                "<NmpModBss> validata pu:%s ok",
                req_info->puid
            );
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_general_cmd_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpBssRes *res_info;
    NmpMsgID msg_id;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    msg_id = MSG_GETID(msg);
    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    if (RES_CODE(res_info))
    {
        nmp_warning("<NmpModBss> msg:%s failed, err:%d.",
            MESSAGE_ID_TO_STR(cms, msg_id), -RES_CODE(res_info));
    }
    else
    {
        nmp_print("<NmpModBss> msg:%s operator ok.",
            MESSAGE_ID_TO_STR(cms, msg_id));
    }

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_general_modify_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpBssRes *res_info;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    msg_id = MSG_GETID(msg);
    ret = RES_CODE(res_info);
    if (ret < 0)
        SET_CODE(res_info, -ret);

    ret = RES_CODE(res_info);
    if (ret && (ret != E_NODBENT))
    {
        nmp_warning("<NmpModBss> msg:%s failed, err:%d.",
            MESSAGE_ID_TO_STR(cms, msg_id), -RES_CODE(res_info));
    }
    else
    {
         SET_CODE(res_info, 0);
        nmp_print("<NmpModBss> msg:%s operator ok.",
            MESSAGE_ID_TO_STR(cms, msg_id));
    }

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_add_admin_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpAddAdmin *req_info;
    NmpMsgErrCode res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = (NmpAddAdmin*)MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        if ((regex_mached(req_info->admin_name, exclude_reg))
            || (regex_mached(req_info->password, exclude_reg)))
        {
            ret = -E_STRINGFORMAT;
            goto add_admin_string_format_wrong;
        }

        ret = nmp_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(NmpAddAdmin), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> add admin:%s failed, err:%d",
                req_info->admin_name, ret
            );
        }
        else
        {
            nmp_print(
                "<NmpModBss> add admin:%s ok",
                req_info->admin_name
            );
        }
    add_admin_string_format_wrong:
        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_add_user_group_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpAddUserGroup *req_info;
    NmpMsgErrCode res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        if ((regex_mached(req_info->group_name, exclude_reg)))
        {
            ret = -E_STRINGFORMAT;
            goto add_user_group_string_format_wrong;
        }

        ret = nmp_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(NmpAddUserGroup), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> add user group:%s failed, err:%d",
                req_info->group_name, ret
            );
        }
        else
        {
            nmp_print(
                "<NmpModBss> add user group:%s ok",
                req_info->group_name
            );
        }
       add_user_group_string_format_wrong:
        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}



NmpMsgFunRet
nmp_mod_bss_add_user_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpAddUser *req_info;
    NmpMsgErrCode res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    printf("=======================bss mh group_id:%d\n",req_info->group_id);
    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        if ((regex_mached(req_info->username, exclude_reg))
            || (regex_mached(req_info->password, exclude_reg))
            || (regex_mached(req_info->user_phone, exclude_reg))
            || (regex_mached(req_info->user_description, exclude_reg)))
        {
            ret = -E_STRINGFORMAT;
            goto add_user_string_format_wrong;
        }

        ret = nmp_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(NmpAddUser), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> add user:%s failed, err:%d",
                req_info->username, ret
            );
        }
        else
        {
            nmp_print(
                "<NmpModBss> add user:%s ok",
                req_info->username
            );
        }

        add_user_string_format_wrong:
        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_add_area_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpAddArea *req_info;
    NmpMsgErrCode res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        if ((regex_mached(req_info->area_name, exclude_reg)))
        {
            ret = -E_STRINGFORMAT;
            goto add_area_string_format_wrong;
        }
        ret = nmp_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(NmpAddArea), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> add area:%s failed, err:%d",
                req_info->area_name, ret
            );
        }
        else
        {
            nmp_print(
                "<NmpModBss> add arear:%s ok",
                req_info->area_name
            );
        }

    add_area_string_format_wrong:
        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}

/*
NmpMsgFunRet
nmp_mod_bss_add_pu_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpAddPu *req_info;
    NmpMsgErrCode res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        if ((!regex_mached(req_info->puid, puid_reg))
			 || (regex_mached(req_info->pu_info, exclude_reg))
			 || (!regex_mached(req_info->domain_id, domain_reg)) )
        {
            ret = -E_STRINGFORMAT;
            goto add_pu_string_format_wrong;
        }
        ret = nmp_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(NmpAddPu), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> add pu:%s failed, err:%d",
                req_info->puid, ret
            );
        }
        else
        {
            nmp_print(
                "<NmpModBss> add pu:%s ok",
                req_info->puid
            );
        }
   add_pu_string_format_wrong:
        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}
*/


NmpMsgFunRet
nmp_mod_bss_add_pu_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpAddPu *req_info;
    NmpMsgErrCode res_info;
    NmpResourcesCap res_cap;
    gchar mf[MF_ID_LEN] = {0};
    gint ret;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if ((!regex_mached(req_info->puid, puid_reg))
			 || (regex_mached(req_info->pu_info, exclude_reg))
			 || (!regex_mached(req_info->domain_id, domain_reg)) )
    {
        SET_CODE(&res_info, E_STRINGFORMAT);
        goto err_add_pu;
    }

    nmp_get_mf_from_guid(req_info->puid, mf);
    memset(&res_cap, 0, sizeof(res_cap));
    nmp_mod_get_resource_cap(&res_cap);
    if (res_cap.module_bits&MODULE_CMS_BIT)
    {
        ret = nmp_compare_manufacturer(res_cap.modules_data[SYS_MODULE_CMS], mf);
        if (ret)
        {
             SET_CODE(&res_info, -ret);
             goto err_add_pu;
        }
    }
    else
        goto err_add_pu;

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);

err_add_pu:
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
    nmp_warning(
        "<NmpModBss> add pu:%s failed, err:%d",
        req_info->puid, RES_CODE(&res_info)
    );
    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_add_pu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpAddPuRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    if (RES_CODE(res_info))
    {
	    nmp_warning("<NmpModBss> admin:%s add pu failed, err:%d",
	    	res_info->bss_usr, -RES_CODE(res_info));
    }
    else
	    nmp_warning("<NmpModBss> admin:%s add pu ok", res_info->bss_usr);

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}

/*
NmpMsgFunRet
nmp_mod_bss_add_gu_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpAddGu *req_info;
    NmpMsgErrCode res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
       if ((!regex_mached(req_info->guid, guid_reg))
			 || (regex_mached(req_info->gu_name, exclude_reg))
			 || (!regex_mached(req_info->domain_id, domain_reg)))
        {
            ret = -E_STRINGFORMAT;
            goto add_gu_string_format_wrong;
        }

        ret = nmp_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(NmpAddGu), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> add gu:%s failed, err:%d",
                req_info->guid, ret
            );
        }
        else
        {
            nmp_print(
                "<NmpModBss> add gu:%s ok",
                req_info->guid
            );
        }
      add_gu_string_format_wrong:
        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}
*/

NmpMsgFunRet
nmp_mod_bss_add_gu_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpAddGu *req_info;
    NmpMsgErrCode res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if ((!regex_mached(req_info->guid, guid_reg))
			 || (regex_mached(req_info->gu_name, exclude_reg))
			 || (!regex_mached(req_info->domain_id, domain_reg)))
    {
        SET_CODE(&res_info, E_STRINGFORMAT);
        MSG_SET_RESPONSE(msg);
        nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
        nmp_warning(
                "<NmpModBss> add gu:%s-* failed, err:%d",
                req_info->puid, E_STRINGFORMAT
        );
        return MFR_DELIVER_BACK;
    }

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_add_gu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpBssRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    if (RES_CODE(res_info))
	    nmp_warning("<NmpModBss> add gu failed, err:%d", -RES_CODE(res_info));
    else
	    nmp_print("<NmpModBss> add gu ok");

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_add_mds_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpAddMds *req_info;
    NmpMsgErrCode res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        if ((!regex_mached(req_info->mds_id, mds_reg))
            || (regex_mached(req_info->mds_name, exclude_reg)))
        {
            ret = -E_STRINGFORMAT;
            goto add_mds_string_format_wrong;
        }

        ret = nmp_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(NmpAddMds), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> add mds:%s failed, err:%d",
                req_info->mds_name, ret
            );
        }
        else
        {
            nmp_print(
                "<NmpModBss> add mds:%s ok",
                req_info->mds_name
            );
        }
    add_mds_string_format_wrong:
        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_add_mds_ip_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpAddMdsIp *req_info;
    NmpMsgErrCode res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = nmp_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(NmpAddMdsIp), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> add mds ip:%s failed, err:%d",
                req_info->mds_id, ret
            );
        }
        else
        {
            nmp_print(
                "<NmpModBss> add mds ip:%s ok",
                req_info->mds_id
            );
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_add_mss_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpAddMss *req_info;
    NmpMsgErrCode res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        if ((!regex_mached(req_info->mss_id, mss_reg))
            || (regex_mached(req_info->mss_name, exclude_reg)))
        {
            ret = -E_STRINGFORMAT;
            goto add_mss_string_format_wrong;
        }

        ret = nmp_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(NmpAddMss), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> add mss:%s failed, err:%d",
                req_info->mss_name, ret
            );
        }
        else
        {
            nmp_print(
                "<NmpModBss> add mss:%s ok",
                req_info->mss_name
            );
        }
    add_mss_string_format_wrong:
        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_add_gu_to_user_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpAddGuToUser *req_info;
    NmpMsgErrCode res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret,size;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);
    size =  MSG_DATA_SIZE(msg);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = nmp_mod_bss_sync_req(self, msg_id, req_info, size, NULL, 0);
        if (ret)
            nmp_warning( "<NmpModBss> add gu to user: failed, err:%d", ret);
        else
            nmp_print( "<NmpModBss> add gu to user: ok" );

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_add_tw_to_user_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpAddTwToUser *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_add_tw_to_user_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpBssRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_add_tour_to_user_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpAddTourToUser *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_add_tour_to_user_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpBssRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_add_defence_area_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpAddDefenceArea *req_info;
    NmpMsgErrCode res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = nmp_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(NmpAddDefenceArea), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> add defence area:%d failed, err:%d",
                req_info->defence_area_id, ret
            );
        }
        else
        {
            nmp_print(
                "<NmpModBss> add defence area:%d ok",
                req_info->defence_area_id
            );
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_add_defence_map_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpAddDefenceMap *req_info;
    NmpMsgErrCode res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = nmp_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(NmpAddDefenceMap), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> add defence map failed,defence id:%d, map id:%d,err:%d",
                req_info->defence_area_id, req_info->map_id, ret
            );
        }
        else
        {
            nmp_print(
                "<NmpModBss> add defence map ok,defence id:%d, map id:%d",
                req_info->defence_area_id, req_info->map_id
            );
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_add_defence_gu_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpAddDefenceGu *req_info;
    NmpMsgErrCode res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = nmp_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(NmpAddDefenceGu), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> add defence gu:%s failed, err:%d",
                req_info->guid, ret
            );
        }
        else
        {
            nmp_print(
                "<NmpModBss> add defence gu:%s ok",
                req_info->guid
            );
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_set_map_href_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpSetMapHref *req_info;
    NmpMsgErrCode res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = nmp_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(NmpSetMapHref), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> add map Href:%d failed, err:%d",
                req_info->dst_map_id, ret
            );
        }
        else
        {
            nmp_print(
                "<NmpModBss> add map Href:%d ok",
                req_info->dst_map_id
            );
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_set_del_alarm_policy_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpDelAlarmPolicy *req_info;
    NmpMsgErrCode res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = nmp_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(NmpDelAlarmPolicy), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> set auto del alarm policy failed, err:%d",
                 ret
            );
        }
        else
        {
            nmp_print(
                "<NmpModBss> set auto del alarm policy ok"
            );
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_add_tw_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpAddTw *req_info;
    NmpMsgErrCode res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = nmp_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(NmpAddTw), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> add tw:%s failed, err:%d",
                req_info->tw_name, ret
            );
        }
        else
        {
            nmp_print(
                "<NmpModBss> add tw %s ok",
                req_info->tw_name
            );
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_add_screen_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpAddScreen *req_info;
    NmpMsgErrCode res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms
            , msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = nmp_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(NmpAddScreen), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> add tw %d screen failed, err:%d",
                req_info->tw_id, ret
            );
        }
        else
        {
            nmp_print(
                "<NmpModBss> add tw %d screen ok",
                req_info->tw_id
            );
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_add_tour_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpAddTour *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_add_tour_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpBssRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_add_tour_step_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpAddTourStep *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_add_tour_step_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpBssRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_add_group_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpAddGroup *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_add_group_step_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpAddGroupStep *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_add_ivs_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpAddIvs *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_add_ivs_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpBssRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_validate_gu_map_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpValidateGuMap *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_validate_gu_map_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpBssRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}

NmpMsgFunRet
nmp_mod_bss_config_group_step_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpConfigGroupStep *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_link_time_policy_config_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpLinkTimePolicyConfig *req_info;
    NmpMsgErrCode res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if ((!regex_mached(req_info->guid, guid_reg))
			 || (!regex_mached(req_info->domain, domain_reg)))
    {
        SET_CODE(&res_info, E_STRINGFORMAT);
        MSG_SET_RESPONSE(msg);
        nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
        nmp_warning(
                "<NmpModBss> link time policy config:%s-* failed, err:%d",
                req_info->guid, E_STRINGFORMAT
        );
        return MFR_DELIVER_BACK;
    }

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_link_record_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpLinkRecord *req_info;
    NmpMsgErrCode res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if ((!regex_mached(req_info->guid, guid_reg))
			 || (!regex_mached(req_info->domain, domain_reg))
			 || (!regex_mached(req_info->link_guid, guid_reg))
			 || (!regex_mached(req_info->link_domain, domain_reg)))
    {
        SET_CODE(&res_info, E_STRINGFORMAT);
        MSG_SET_RESPONSE(msg);
        nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
        nmp_warning(
                "<NmpModBss> link record:%s-* failed, err:%d",
                req_info->guid, E_STRINGFORMAT
        );
        return MFR_DELIVER_BACK;
    }

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_link_IO_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpLinkIO *req_info;
    NmpMsgErrCode res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if ((!regex_mached(req_info->guid, guid_reg))
			 || (!regex_mached(req_info->domain, domain_reg))
			 || (!regex_mached(req_info->link_guid, guid_reg))
			 || (!regex_mached(req_info->link_domain, domain_reg)))
    {
        SET_CODE(&res_info, E_STRINGFORMAT);
        MSG_SET_RESPONSE(msg);
        nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
        nmp_warning(
                "<NmpModBss> link IO:%s-* failed, err:%d",
                req_info->guid, E_STRINGFORMAT
        );
        return MFR_DELIVER_BACK;
    }

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_link_snapshot_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpLinkSnapshot *req_info;
    NmpMsgErrCode res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if ((!regex_mached(req_info->guid, guid_reg))
			 || (!regex_mached(req_info->domain, domain_reg))
			 || (!regex_mached(req_info->link_guid, guid_reg))
			 || (!regex_mached(req_info->link_domain, domain_reg)))
    {
        SET_CODE(&res_info, E_STRINGFORMAT);
        MSG_SET_RESPONSE(msg);
        nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
        nmp_warning(
                "<NmpModBss> link snapshot:%s-* failed, err:%d",
                req_info->guid, E_STRINGFORMAT
        );
        return MFR_DELIVER_BACK;
    }

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_link_preset_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpLinkPreset *req_info;
    NmpMsgErrCode res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if ((!regex_mached(req_info->guid, guid_reg))
			 || (!regex_mached(req_info->domain, domain_reg))
			 || (!regex_mached(req_info->link_guid, guid_reg))
			 || (!regex_mached(req_info->link_domain, domain_reg)))
    {
        SET_CODE(&res_info, E_STRINGFORMAT);
        MSG_SET_RESPONSE(msg);
        nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
        nmp_warning(
                "<NmpModBss> link preset:%s-* failed, err:%d",
                req_info->guid, E_STRINGFORMAT
        );
        return MFR_DELIVER_BACK;
    }

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_link_step_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpLinkStepConfig *req_info;
    NmpMsgErrCode res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if ((!regex_mached(req_info->guid, guid_reg))
			 || (!regex_mached(req_info->domain, domain_reg))
			 || (!regex_mached(req_info->link_guid, guid_reg))
			 || (!regex_mached(req_info->link_domain, domain_reg)))
    {
        SET_CODE(&res_info, E_STRINGFORMAT);
        MSG_SET_RESPONSE(msg);
        nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
        nmp_warning(
                "<NmpModBss> link step:%s-* failed, err:%d",
                req_info->guid, E_STRINGFORMAT
        );
        return MFR_DELIVER_BACK;
    }

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_link_tour_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpLinkTourConfig *req_info;
    NmpMsgErrCode res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if ((!regex_mached(req_info->guid, guid_reg))
			 || (!regex_mached(req_info->domain, domain_reg)))
    {
        SET_CODE(&res_info, E_STRINGFORMAT);
        MSG_SET_RESPONSE(msg);
        nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
        nmp_warning(
                "<NmpModBss> link tour:%s-* failed, err:%d",
                req_info->guid, E_STRINGFORMAT
        );
        return MFR_DELIVER_BACK;
    }

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_link_group_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpLinkGroupConfig *req_info;
    NmpMsgErrCode res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if ((!regex_mached(req_info->guid, guid_reg))
			 || (!regex_mached(req_info->domain, domain_reg)))
    {
        SET_CODE(&res_info, E_STRINGFORMAT);
        MSG_SET_RESPONSE(msg);
        nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
        nmp_warning(
                "<NmpModBss> link group:%s-* failed, err:%d",
                req_info->guid, E_STRINGFORMAT
        );
        return MFR_DELIVER_BACK;
    }

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_link_map_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpLinkMap *req_info;
    NmpMsgErrCode res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if ((!regex_mached(req_info->guid, guid_reg))
			 || (!regex_mached(req_info->domain, domain_reg))
			 || (!regex_mached(req_info->link_guid, guid_reg))
			 || (!regex_mached(req_info->link_domain, domain_reg)))
    {
        SET_CODE(&res_info, E_STRINGFORMAT);
        MSG_SET_RESPONSE(msg);
        nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
        nmp_warning(
                "<NmpModBss> link map:%s-* failed, err:%d",
                req_info->guid, E_STRINGFORMAT
        );
        return MFR_DELIVER_BACK;
    }

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_modify_admin_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpAddAdmin *req_info;
    NmpMsgErrCode  res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s failed,No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        if ((regex_mached(req_info->password, exclude_reg)))
        {
            ret = -E_STRINGFORMAT;
            goto modify_admin_string_format_wrong;
        }

        ret = nmp_mod_bss_sync_req(self, msg_id, req_info, sizeof(NmpAddAdmin), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> modify admin:%s failed, err:%d",
                req_info->admin_name, ret
            );
        }
        else
        {
            nmp_print(
                "<NmpModBss> modify admin:%s ok",
                req_info->admin_name
            );
        }

     modify_admin_string_format_wrong:
        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_modify_user_group_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpUserGroupInfo *req_info;
    NmpMsgErrCode  res_info;
    NmpGuestBase *bss_base;
	NmpSysMsg *msg_del_user;
	NmpMsgDelUserGroup del_group;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

	memset(&del_group, 0, sizeof(del_group));
    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        if ((regex_mached(req_info->group_name, exclude_reg)))
        {
            ret = -E_STRINGFORMAT;
            goto modify_user_group_string_format_wrong;
        }

        ret = nmp_mod_bss_sync_req(self, msg_id, req_info, sizeof(NmpUserGroupInfo), NULL, 0);
        if (ret && (ret != -E_NODBENT))
        {
            nmp_warning(
                "<NmpModBss> modify user group:%s failed, err:%d",
                req_info->group_name, ret
            );
        }
        else
        {

            nmp_print(
                      "<NmpModBss> modify user group:%s ok",
                      req_info->group_name
                  );

	   		sprintf(del_group.group_ids,"'%d",req_info->group_id);
	    	del_group.group_permissions = req_info->group_permissions;
	    	del_group.group_rank = req_info->group_rank;
			msg_del_user = nmp_sysmsg_new_2(MSG_DEL_USER_GROUP, &del_group, sizeof(del_group), ++msg_seq_generator);
			if (G_UNLIKELY(!msg_del_user))
			{
				nmp_mods_container_put_guest(self->container, bss_base);
				return MFR_DELIVER_BACK;
			}

			MSG_SET_DSTPOS(msg_del_user, BUSSLOT_POS_CU);
			nmp_mod_bss_deliver_out_msg(app_obj, msg_del_user);

	    	ret = 0;
        }
        modify_user_group_string_format_wrong:
        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_modify_user_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpUserInfo *req_info;
    NmpMsgErrCode  res_info;
    NmpGuestBase *bss_base;
    NmpSysMsg *msg_del_user;
    NmpMsgDelUser user;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&user, 0, sizeof(user));
    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
	 nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        if ((regex_mached(req_info->password, exclude_reg))
            || (regex_mached(req_info->user_phone, exclude_reg))
            || (regex_mached(req_info->user_description, exclude_reg)))
        {
            ret = -E_STRINGFORMAT;
            goto modify_user_string_format_wrong;
        }

        ret = nmp_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(NmpUserInfo), NULL, 0);
        if (ret && (ret != -E_NODBENT))
        {
            nmp_warning(
                "<NmpModBss> modify user:%s failed, err:%d",
                req_info->username, ret
            );
        }
        else
        {

            nmp_print(
                "<NmpModBss> modify user:%s ok",
                req_info->username
            );

            if (ret != -E_NODBENT)
            {
                strncpy(user.name, req_info->username, USER_NAME_LEN -1);
                msg_del_user = nmp_sysmsg_new_2(MSG_DEL_USER, &user, sizeof(user), ++msg_seq_generator);
                if (G_UNLIKELY(!msg_del_user))
                {
                    nmp_mods_container_put_guest(self->container, bss_base);
                    return MFR_DELIVER_BACK;
                }

                MSG_SET_DSTPOS(msg_del_user, BUSSLOT_POS_CU);
                nmp_mod_bss_deliver_out_msg(app_obj, msg_del_user);
            }

            ret = 0;
        }
      modify_user_string_format_wrong:
        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_modify_domain_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpDomainInfo *req_info;
    NmpMsgErrCode  res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    NmpSysMsg *msg_modify_domain;
    NmpModifyDomain domain;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        if (regex_mached(req_info->domain_name, exclude_reg))
        {
            ret = -E_STRINGFORMAT;
            goto modify_domain_string_format_wrong;
        }

        ret = nmp_mod_bss_sync_req(self,  msg_id, req_info, sizeof(NmpDomainInfo), NULL, 0);
        if (ret && (ret != -E_NODBENT))
        {
            nmp_warning(
                "<NmpModBss> modify domain:%s failed, err:%d",
                req_info->domain_id, ret
            );
        }
        else
        {
            nmp_print(
                "<NmpModBss> modify domain:%s ok",
                req_info->domain_id
            );

            if (ret != -E_NODBENT)
            {
                strncpy(domain.dm_name, req_info->domain_name, DOMAIN_NAME_LEN -1);
                msg_modify_domain = nmp_sysmsg_new_2(MESSAGE_NOTIFY_MODIFY_DOMAIN, &domain, sizeof(domain), ++msg_seq_generator);
                if (G_UNLIKELY(!msg_modify_domain))
                {
                    nmp_mods_container_put_guest(self->container, bss_base);
                    return MFR_DELIVER_BACK;
                }

                MSG_SET_DSTPOS(msg_modify_domain, BUSSLOT_POS_CU);
                nmp_mod_bss_deliver_out_msg(app_obj, msg_modify_domain);
            }

	     ret = 0;
        }

        modify_domain_string_format_wrong:
        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_add_modify_area_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpAreaInfo *req_info;
    NmpMsgErrCode  res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        if ((regex_mached(req_info->area_name, exclude_reg)))
        {
            ret = -E_STRINGFORMAT;
            goto modify_area_string_format_wrong;
        }

        ret = nmp_mod_bss_sync_req(self,  msg_id, req_info, sizeof(NmpAreaInfo), NULL, 0);
        if (ret && (ret != -E_NODBENT))
        {
            nmp_warning(
                "<NmpModBss> modify area:%s failed, err:%d",
                req_info->area_name, ret
            );
        }
        else
        {
            ret = 0;
            nmp_print(
                "<NmpModBss> add or modify area:%s ok",
                req_info->area_name
            );
        }
   modify_area_string_format_wrong:
        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_add_modify_manufacturer_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpAddModifyManufacturer *req_info;
    NmpMsgErrCode  res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        if ((!regex_mached(req_info->mf_id, manufactur_reg))
		|| (regex_mached(req_info->mf_name, mf_exclude_reg)))
        {
            ret = -E_STRINGFORMAT;
            goto modify_manufacturer_string_format_wrong;
        }

        ret = nmp_mod_bss_sync_req(self,  msg_id, req_info,
              sizeof(NmpAddModifyManufacturer), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> modify manufacturer:%s failed, err:%d",
                req_info->mf_id, ret
            );
        }
        else
        {
            ret = 0;
            nmp_print(
                "<NmpModBss> modify manufacturer:%s ok",
                req_info->mf_id
            );
        }
        modify_manufacturer_string_format_wrong:
        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_modify_pu_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpPuInfo *req_info;
    NmpMsgErrCode  res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        if ((regex_mached(req_info->pu_info, exclude_reg)))
        {
            ret = -E_STRINGFORMAT;
            goto modify_pu_string_format_wrong;
        }

        ret = nmp_mod_bss_sync_req(self,  msg_id, req_info, sizeof(NmpPuInfo), NULL, 0);
        if (ret && (ret != -E_NODBENT))
        {
            nmp_warning(
                "<NmpModBss> modify pu:%s failed, err:%d",
                req_info->puid, ret
            );
        }
        else
        {
            ret = 0;
            nmp_print(
                "<NmpModBss> modify pu:%s ok",
                req_info->puid
            );
        }
      modify_pu_string_format_wrong:
        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_modify_gu_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpGuInfo *req_info;
    NmpMsgErrCode  res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        if (regex_mached(req_info->gu_name, exclude_reg))
        {
            ret = -E_STRINGFORMAT;
            goto modify_gu_string_format_wrong;
        }

        ret = nmp_mod_bss_sync_req(self,  msg_id, req_info, sizeof(NmpGuInfo), NULL, 0);
        if (ret && (ret != -E_NODBENT))
        {
            nmp_warning(
                "<NmpModBss> modify gu:%s failed, err:%d",
                req_info->guid, ret
            );
        }
        else
        {
            nmp_print(
                "<NmpModBss> modify gu:%s ok",
                req_info->guid
            );
            ret = 0;
        }
        modify_gu_string_format_wrong:
        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_modify_mds_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpMdsInfo *req_info;
    NmpMsgErrCode  res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        if (regex_mached(req_info->mds_name, exclude_reg))
        {
            ret = -E_STRINGFORMAT;
            goto modify_mds_string_format_wrong;
        }

        ret = nmp_mod_bss_sync_req(self,  msg_id, req_info, sizeof(NmpMdsInfo), NULL, 0);
        if (ret && (ret != -E_NODBENT))
        {
            nmp_warning(
                "<NmpModBss> modify mds:%s failed, err:%d",
                req_info->mds_name, ret
            );
        }
        else
        {
            ret = 0;
            nmp_print(
                "<NmpModBss> modify mds:%s ok",
                req_info->mds_name
            );
        }

   modify_mds_string_format_wrong:
        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_modify_mss_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpMssInfo *req_info;
    NmpMsgErrCode  res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        if (regex_mached(req_info->mss_name, exclude_reg))
        {
            ret = -E_STRINGFORMAT;
            goto modify_mss_string_format_wrong;
        }

        ret = nmp_mod_bss_sync_req(self,  msg_id, req_info, sizeof(NmpMssInfo), NULL, 0);
        if (ret && (ret != -E_NODBENT))
        {
            nmp_warning(
                "<NmpModBss> modify mss:%s failed, err:%d",
                req_info->mss_name, ret
            );
        }
        else
        {
            ret = 0;
            nmp_print(
                "<NmpModBss> modify mss:%s ok",
                req_info->mss_name
            );
        }

   modify_mss_string_format_wrong:
        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_modify_defence_area_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpDefenceAreaInfo *req_info;
    NmpMsgErrCode  res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = nmp_mod_bss_sync_req(self,  msg_id, req_info, sizeof(NmpDefenceAreaInfo), NULL, 0);
        if (ret && (ret != -E_NODBENT))
        {
            nmp_warning(
                "<NmpModBss> modify defence area id:%d failed, err:%d",
                req_info->defence_area_id, ret
            );
        }
        else
        {
            ret = 0;
            nmp_print(
                "<NmpModBss> modify defence area id:%d ok",
                req_info->defence_area_id
            );
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_modify_defence_gu_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpModifyDefenceGu *req_info;
    NmpMsgErrCode  res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = nmp_mod_bss_sync_req(self,  msg_id, req_info, sizeof(NmpModifyDefenceGu), NULL, 0);
        if (ret && (ret != -E_NODBENT))
        {
            nmp_warning(
                "<NmpModBss> modify defence gu:%d failed, err:%d",
                req_info->map_id, ret
            );
        }
        else
        {
            ret = 0;
            nmp_print(
                "<NmpModBss> modify defence gu:%d ok",
                req_info->map_id
            );
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_modify_map_href_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpModifyMapHref *req_info;
    NmpMsgErrCode  res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = nmp_mod_bss_sync_req(self,  msg_id, req_info, sizeof(NmpModifyMapHref), NULL, 0);
        if (ret && (ret != -E_NODBENT))
        {
            nmp_warning(
                "<NmpModBss> modify map href:%d failed, err:%d",
                req_info->src_map_id, ret
            );
        }
        else
        {
            ret = 0;
            nmp_print(
                "<NmpModBss> modify map href:%d ok",
                req_info->src_map_id
            );
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_modify_tw_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpModifyTw *req_info;
    NmpMsgErrCode  res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = nmp_mod_bss_sync_req(self,  msg_id, req_info, sizeof(NmpModifyTw), NULL, 0);
        if (ret && (ret != -E_NODBENT))
        {
            nmp_warning(
                "<NmpModBss> modify tw:%s failed, err:%d",
                req_info->tw_name, ret
            );
        }
        else
        {
            ret = 0;
            nmp_print(
                "<NmpModBss> modify tw:%s ok",
                req_info->tw_name
            );
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_modify_screen_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpModifyScreen *req_info;
    NmpMsgErrCode  res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = nmp_mod_bss_sync_req(self,  msg_id, req_info, sizeof(NmpModifyScreen), NULL, 0);
        if (ret && (ret != -E_NODBENT))
        {
            nmp_warning(
                "<NmpModBss> modify screen:%d failed, err:%d",
                req_info->screen_id, ret
            );
        }
        else
        {
            ret = 0;
            nmp_print(
                "<NmpModBss> modify screen:%d ok",
                req_info->screen_id
            );
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_modify_tour_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModifyTour *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_modify_tour_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpBssRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_modify_group_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModifyGroup *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_modify_group_step_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModifyGroupStep *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_modify_group_step_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModifyGroupStepInfo *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_modify_link_time_policy_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModifyLinkTimePolicy *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_modify_link_record_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModifyLinkRecord *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_modify_link_IO_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModifyLinkIO *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_modify_link_snapshot_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModifyLinkRecord *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_modify_link_preset_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModifyLinkPreset *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_modify_link_step_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModifyLinkStep *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_modify_link_tour_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModifyLinkTour *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_modify_link_group_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModifyLinkGroup *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_modify_link_map_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModifyLinkMap *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_modify_ivs_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpIvsInfo *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_modify_ivs_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpBssRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_del_admin_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpDelAdmin *req_info;
    NmpMsgErrCode  res_info;
    NmpGuestBase *bss_base;
    NmpSysMsg *msg_notify;
    NmpForceUsrOffline notify_info;
    NmpMsgID msg_id;
    gint ret;
	gchar *username;
	gchar *name;
	gchar  offline_name[USER_NAME_LEN];

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = (NmpDelAdmin*)MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = nmp_mod_bss_sync_req(self, msg_id, req_info, sizeof(NmpDelAdmin), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> del admin:%s failed, err:%d",
                req_info->admin_name, ret
            );
        }
        else
        {
            nmp_print(
                "<NmpModBss> del admin:%s ok",
                req_info->admin_name
            );

            memset(&notify_info, 0, sizeof(notify_info));
            //strncpy(notify_info.reason, "user has been delete", OFFLINE_REASON_LEN - 1);
            notify_info.reason = 0;
            msg_notify = nmp_sysmsg_new_2(MESSAGE_FORCE_USR_OFFLINE,
                 &notify_info, sizeof(notify_info), ++msg_seq_generator);
            if (G_UNLIKELY(!msg_notify))
            {
                nmp_sysmsg_destroy(msg);
                nmp_warning("create sysmsg:%d err", MESSAGE_FORCE_USR_OFFLINE);
            }

            name = req_info->admin_name;
            printf("##################name : %s\n",name);
            while((username = strsep(&name, ",")) != NULL)
            {
                memset(offline_name, 0, USER_NAME_LEN);
                strncpy(offline_name,&username[1],strlen(username) - 2);
                offline_name[USER_NAME_LEN - 1] = 0;
                printf("&&&&&&&&&&&&&&&username : %s,offline_name:%s\n",username,offline_name);
                nmp_mod_bss_force_usr_offline(self, offline_name, msg_notify);
            }

	        nmp_sysmsg_destroy(msg_notify);
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_del_user_group_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpDelUserGroup *req_info;
    NmpMsgErrCode  res_info;
    NmpGuestBase *bss_base;
	NmpSysMsg *msg_del_user;
	NmpMsgDelUserGroup del_group;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&del_group, 0, sizeof(del_group));
    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = nmp_mod_bss_sync_req(self, msg_id,  req_info, sizeof(NmpDelUserGroup), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> del user group:%s failed, err:%d",
                req_info->group_id, ret
            );
        }
        else
        {
            nmp_print(
                "<NmpModBss> del user group:%s ok",
                req_info->group_id
            );

			strncpy(del_group.group_ids, req_info->group_id, MULTI_NAME_LEN -1);
			msg_del_user = nmp_sysmsg_new_2(MSG_DEL_USER_GROUP, &del_group, sizeof(del_group), ++msg_seq_generator);
			if (G_UNLIKELY(!msg_del_user))
			{
				nmp_mods_container_put_guest(self->container, bss_base);
				return MFR_DELIVER_BACK;
			}

			MSG_SET_DSTPOS(msg_del_user, BUSSLOT_POS_CU);
			nmp_mod_bss_deliver_out_msg(app_obj, msg_del_user);
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_del_user_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpDelUser *req_info;
    NmpMsgErrCode  res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    NmpSysMsg *msg_del_user;
    NmpMsgDelUser user;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&user, 0, sizeof(user));
    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = nmp_mod_bss_sync_req(self, msg_id,  req_info, sizeof(NmpDelUser), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> del user:%s failed, err:%d",
                req_info->username, ret
            );
        }
        else
        {
            nmp_print(
                "<NmpModBss> del user:%s ok",
                req_info->username
            );

	    strncpy(user.name, req_info->username, USER_NAME_LEN -1);
           msg_del_user = nmp_sysmsg_new_2(MSG_DEL_USER, &user, sizeof(user), ++msg_seq_generator);
           if (G_UNLIKELY(!msg_del_user))
           {
              nmp_mods_container_put_guest(self->container, bss_base);
              return MFR_DELIVER_BACK;
           }

           MSG_SET_DSTPOS(msg_del_user, BUSSLOT_POS_CU);
           nmp_mod_bss_deliver_out_msg(app_obj, msg_del_user);
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_del_area_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpDelArea *req_info;
    NmpMsgErrCode  res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = nmp_mod_bss_sync_req(self, msg_id, req_info, sizeof(NmpDelArea), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> del area:%d failed, err:%d",
                req_info->area_id, ret
            );
        }
        else
        {
            nmp_print( "<NmpModBss> del area:%d ok", req_info->area_id);
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_del_pu_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpDelPu *req_info;
    NmpMsgErrCode  res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret, size;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        size = sizeof(NmpDelPu) + req_info->count*sizeof(NmpPuPoint);
        ret = nmp_mod_bss_sync_req(self, msg_id,  req_info,size, NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> del pu failed, err:%d",
                 ret
            );
        }
        else
        {
            nmp_print( "<NmpModBss> del pu ok");
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_del_gu_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpDelGu *req_info;
    NmpMsgErrCode  res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = nmp_mod_bss_sync_req(self, msg_id,  req_info, sizeof(NmpDelGu), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> del gu:%s failed, err:%d",
                req_info->guid, ret
            );
        }
        else
        {
            nmp_print( "<NmpModBss> del gu:%s ok", req_info->guid);
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_del_manufacturer_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpDelManufacturer *req_info;
    NmpMsgErrCode  res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = nmp_mod_bss_sync_req(
	     self, msg_id,  req_info,
            sizeof(NmpDelManufacturer), NULL, 0
        );
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> del manufacturer:%s failed, err:%d",
                req_info->mf_id, ret
            );
        }
        else
        {
            nmp_print( "<NmpModBss> del manufacturer:%s ok", req_info->mf_id);
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_del_mds_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpDelMds *req_info;
    NmpMsgErrCode  res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = nmp_mod_bss_sync_req(self, msg_id,  req_info, sizeof(NmpDelMds), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> del mds:%s failed, err:%d",
                req_info->mds_id, ret
            );
        }
        else
        {
            nmp_print( "<NmpModBss> del mds:%s ok", req_info->mds_id);
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_del_mds_ip_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpDelMdsIp *req_info;
    NmpMsgErrCode  res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = nmp_mod_bss_sync_req(self, msg_id, req_info, sizeof(NmpDelMdsIp), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> del mds ip:%s failed, err:%d",
                req_info->mds_id, ret
            );
        }
        else
        {
            nmp_print( "<NmpModBss> del mds ip:%s ok", req_info->mds_id);
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_del_mss_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpDelMss *req_info;
    NmpMsgErrCode  res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = nmp_mod_bss_sync_req(self, msg_id,  req_info, sizeof(NmpDelMss), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> del mss:%s failed, err:%d",
                req_info->mss_id, ret
            );
        }
        else
        {
            nmp_print( "<NmpModBss> del mss:%s ok", req_info->mss_id);
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_del_defence_area_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpDelDefenceArea *req_info;
    NmpMsgErrCode  res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = nmp_mod_bss_sync_req(self, msg_id,  req_info, sizeof(NmpDelDefenceArea), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> del defence area:%d failed, err:%d",
                req_info->defence_area_id, ret
            );
        }
        else
        {
            nmp_print( "<NmpModBss> del defence area:%d ok", req_info->defence_area_id);
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_del_defence_map_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpDelDefenceMap *req_info;
    NmpMsgErrCode  res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = nmp_mod_bss_sync_req(self, msg_id,  req_info, sizeof(NmpDelDefenceMap), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> del defence map:%d failed, err:%d",
                req_info->defence_area_id, ret
            );
        }
        else
        {
            nmp_print( "<NmpModBss> del defence map:%d ok", req_info->defence_area_id);
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_del_defence_gu_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpDelDefenceGu *req_info;
    NmpMsgErrCode  res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = nmp_mod_bss_sync_req(self, msg_id,  req_info, sizeof(NmpDelDefenceGu), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> del defence gu:%s failed, err:%d",
                req_info->guid, ret
            );
        }
        else
        {
            nmp_print( "<NmpModBss> del defence gu:%s ok", req_info->guid);
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_del_map_href_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpDelMapHref *req_info;
    NmpMsgErrCode  res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = nmp_mod_bss_sync_req(self, msg_id,  req_info, sizeof(NmpDelMapHref), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> del map Href:%d failed, err:%d",
                req_info->dst_map_id, ret
            );
        }
        else
        {
            nmp_print( "<NmpModBss> del map Href:%d ok", req_info->dst_map_id);
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_del_tw_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpDelTw *req_info;
    NmpMsgErrCode  res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = nmp_mod_bss_sync_req(self, msg_id,  req_info, sizeof(NmpDelTw), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> del tw:%d failed, err:%d",
                req_info->tw_id, ret
            );
        }
        else
        {
            nmp_print( "<NmpModBss> del tw:%d ok", req_info->tw_id);
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_del_screen_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpDelScreen *req_info;
    NmpMsgErrCode  res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = nmp_mod_bss_sync_req(self, msg_id,  req_info, sizeof(NmpDelScreen), NULL, 0);
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> del screen failed, err:%d",
                 ret
            );
        }
        else
        {
            nmp_print( "<NmpModBss> del screen ok");
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_del_tour_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpDelTour *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_del_tour_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpBssRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_del_group_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpDelGroup *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_del_group_step_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpDelGroupStep *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_del_group_step_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpDelGroupStepInfo *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_del_link_time_policy_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpDelLinkTimePolicy *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_del_link_record_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpDelLinkRecord *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_del_link_IO_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpDelLinkIO *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_del_link_snapshot_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpDelLinkSnapshot *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_del_link_preset_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpDelLinkPreset *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_del_link_step_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpDelLinkStep *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_del_link_tour_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpDelLinkTour *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_del_link_group_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpDelLinkGroup *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_del_link_map_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpDelLinkMap *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_del_ivs_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpDelIvs *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_del_ivs_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpBssRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


/*
NmpMsgFunRet
nmp_mod_bss_query_admin_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpQueryAdmin *req_info;
    NmpQueryAdminRes  *res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = nmp_mem_kalloc(sizeof(NmpMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            nmp_warning("<NmpModBss> alloc memory error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
    	req_info = MSG_GET_DATA(msg);
    	BUG_ON(!req_info);

        res_info = nmp_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(NmpQueryAdmin), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
            //nmp_print("<NmpModBss> query admin list ok");
        }
        else
        {
            ret = -ENOMEM;
            nmp_error("query admin failed, err:%d", ret);
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    MSG_SET_RESPONSE(msg);
    if(res_info)
    {
        SET_CODE(res_info, -ret);
    	 nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
    }
    else
    {
        nmp_sysmsg_destroy(msg);
	 return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}
*/

NmpMsgFunRet
nmp_mod_bss_query_admin_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpQueryAdmin *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_admin_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryAdminRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


/*
NmpMsgFunRet
nmp_mod_bss_query_user_group_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpQueryUserGroup *req_info;
    NmpQueryUserGroupRes *res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = nmp_mem_kalloc(sizeof(NmpMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            nmp_error("<NmpModBss> alloc memory error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
        req_info = MSG_GET_DATA(msg);
        BUG_ON(!req_info);

        res_info = nmp_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(NmpQueryUserGroup), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
            //nmp_print("<NmpModBss> query user group list ok");
        }
        else
        {
            ret = -ENOMEM;
            nmp_error("<NmpModBss> query user group failed, err:%d", ret);
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    MSG_SET_RESPONSE(msg);
    if(res_info)
    {
        SET_CODE(res_info, -ret);
    	 nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
    }
    else
    {
        nmp_sysmsg_destroy(msg);
	 return MFR_ACCEPTED;
    }
    return MFR_DELIVER_BACK;
}
*/

NmpMsgFunRet
nmp_mod_bss_query_user_group_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpQueryUserGroup *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_user_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryUserGroupRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}

/*
NmpMsgFunRet
nmp_mod_bss_query_user_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpQueryUser *req_info;
    NmpQueryUserRes *res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = nmp_mem_kalloc(sizeof(NmpMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            nmp_warning("<NmpModBss> alloc memory error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
        req_info = MSG_GET_DATA(msg);
        BUG_ON(!req_info);

        res_info = nmp_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(NmpQueryUser), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
            //nmp_print("<NmpModBss> query user list ok");
        }
        else
        {
            ret = -ENOMEM;
            nmp_error("<NmpModBss> query user failed, err:%d", ret);
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    if(res_info)
    {
        MSG_SET_RESPONSE(msg);
        SET_CODE(res_info, -ret);
    	 nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
    }
    else
    {
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}
*/

NmpMsgFunRet
nmp_mod_bss_query_user_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpQueryUser *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_user_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryUserRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}

/*
NmpMsgFunRet
nmp_mod_bss_query_domain_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpQueryDomain *req_info;
    NmpQueryDomainRes *res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = nmp_mem_kalloc(sizeof(NmpMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            nmp_warning("<NmpModBss> alloc memory error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

    }
    else
    {
        req_info = MSG_GET_DATA(msg);
        BUG_ON(!req_info);

        res_info = nmp_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(NmpQueryDomain), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
            //nmp_print("<NmpModBss> query domain ok");
        }
        else
        {
            ret = -ENOMEM;
            nmp_error("<NmpModBss> query domain failed, err:%d", ret);
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    if(res_info)
    {
        MSG_SET_RESPONSE(msg);
        SET_CODE(res_info, -ret);
    	 nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
    }
    else
    {
          nmp_sysmsg_destroy(msg);
	   return MFR_ACCEPTED;
    }
    return MFR_DELIVER_BACK;
}
*/

NmpMsgFunRet
nmp_mod_bss_query_domain_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpQueryDomain *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_domain_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryDomainRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}

/*
NmpMsgFunRet
nmp_mod_bss_query_area_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpQueryArea *req_info;
    NmpQueryAreaRes *res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = nmp_mem_kalloc(sizeof(NmpMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            nmp_error("<NmpModBss> alloc memory error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
        req_info = MSG_GET_DATA(msg);
    	 BUG_ON(!req_info);
        res_info = nmp_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(NmpQueryArea), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
            //nmp_print("<NmpModBss> query area list ok");
        }
        else
        {
            ret = -ENOMEM;
            nmp_error("<NmpModBss> query area failed, err:%d", ret);
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    if(res_info)
    {
        MSG_SET_RESPONSE(msg);
        SET_CODE(res_info, -ret);
    	 nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
    }
    else
    {
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}
*/

NmpMsgFunRet
nmp_mod_bss_query_area_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpQueryArea *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_area_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryAreaRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}

/*
NmpMsgFunRet
nmp_mod_bss_query_pu_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpQueryPu *req_info;
    NmpQueryPuRes *res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = nmp_mem_kalloc(sizeof(NmpMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            nmp_error("<NmpModBss> alloc memory error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
    	req_info = MSG_GET_DATA(msg);
    	BUG_ON(!req_info);

        res_info = nmp_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(NmpQueryPu), &size);
        if (res_info)
       {
            ret = RES_CODE(res_info);
            //nmp_print("<NmpModBss> query pu list ok");
        }
        else
        {
            ret = -ENOMEM;
            nmp_error("<NmpModBss> query pu failed, err:%d", ret);
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    if(res_info)
    {
        MSG_SET_RESPONSE(msg);
        SET_CODE(res_info, -ret);
    	 nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
    }
    else
    {
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}
*/


NmpMsgFunRet
nmp_mod_bss_query_pu_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpQueryPu *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_pu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryPuRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}

/*
NmpMsgFunRet
nmp_mod_bss_query_gu_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpQueryGu *req_info;
    NmpQueryGuRes *res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = nmp_mem_kalloc(sizeof(NmpMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            nmp_error("<NmpModBss> alloc memory error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
        req_info = MSG_GET_DATA(msg);
        BUG_ON(!req_info);

        res_info = nmp_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(NmpQueryGu), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
            //nmp_print("<NmpModBss> query pu list ok");
        }
        else
        {
            ret = -ENOMEM;
            nmp_error("<NmpModBss> query pu failed, err:%d", ret);
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    if(res_info)
    {
        MSG_SET_RESPONSE(msg);
        SET_CODE(res_info, -ret);
    	 nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
    }
    else
    {
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}
*/

NmpMsgFunRet
nmp_mod_bss_query_gu_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpQueryGu *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_gu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryGuRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}

/*
NmpMsgFunRet
nmp_mod_bss_query_manufacturer_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpQueryManufacturer *req_info;
    NmpQueryManufacturerRes *res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = nmp_mem_kalloc(sizeof(NmpMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            nmp_warning("<NmpModBss> alloc memory error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
        req_info = MSG_GET_DATA(msg);
    	 BUG_ON(!req_info);
        res_info = nmp_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(NmpQueryManufacturer), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
            //nmp_print("<NmpModBss> query manufacturer list ok");
        }
        else
        {
            ret = -ENOMEM;
            nmp_error("<NmpModBss> query manufacturer failed, err:%d", ret);
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    if(res_info)
    {
        MSG_SET_RESPONSE(msg);
        SET_CODE(res_info, -ret);
    	 nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
    }
    else
    {
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}
*/

NmpMsgFunRet
nmp_mod_bss_query_manufacturer_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpQueryManufacturer *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_manufacturer_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryManufacturerRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}

/*
NmpMsgFunRet
nmp_mod_bss_query_mds_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpQueryMds *req_info;
    NmpQueryMdsRes *res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = nmp_mem_kalloc(sizeof(NmpMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            nmp_error("<NmpModBss> alloc memory error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
        req_info = MSG_GET_DATA(msg);
    	 BUG_ON(!req_info);
        res_info = nmp_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(NmpQueryMds), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
            //nmp_print("<NmpModBss> query mds list ok");
        }
        else
        {
            ret = -ENOMEM;
            nmp_error("<NmpModBss> query mds failed, err:%d", ret);
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    if(res_info)
    {
        MSG_SET_RESPONSE(msg);
        SET_CODE(res_info, -ret);
    	 nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
    }
    else
    {
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}
*/

NmpMsgFunRet
nmp_mod_bss_query_mds_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpQueryUserGroup *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_mds_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryMdsRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}

/*
NmpMsgFunRet
nmp_mod_bss_query_mds_ip_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpQueryMdsIp *req_info;
    NmpQueryMdsIpRes *res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = nmp_mem_kalloc(sizeof(NmpMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            nmp_error("<NmpModBss> alloc memory error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
        req_info = MSG_GET_DATA(msg);
    	 BUG_ON(!req_info);
        res_info = nmp_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(NmpQueryMdsIp), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
            //nmp_print("<NmpModBss> query mds list ok");
        }
        else
        {
            ret = -ENOMEM;
            nmp_error("<NmpModBss> query mds failed, err:%d", ret);
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    if(res_info)
    {
        MSG_SET_RESPONSE(msg);
        SET_CODE(res_info, -ret);
    	 nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
    }
    else
    {
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}
*/

NmpMsgFunRet
nmp_mod_bss_query_mds_ip_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpQueryMdsIp *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_mds_ip_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryMdsIpRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}

/*
NmpMsgFunRet
nmp_mod_bss_query_mss_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpQueryMss *req_info;
    NmpQueryMssRes *res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = nmp_mem_kalloc(sizeof(NmpMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            nmp_error("<NmpModBss> alloc memory error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
        req_info = MSG_GET_DATA(msg);
    	 BUG_ON(!req_info);
        res_info = nmp_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(NmpQueryMss), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
            //nmp_print("<NmpModBss> query mds list ok");
        }
        else
        {
            ret = -ENOMEM;
            nmp_error("<NmpModBss> query mds failed, err:%d", ret);
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    if(res_info)
    {
        MSG_SET_RESPONSE(msg);
        SET_CODE(res_info, -ret);
    	 nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
    }
    else
    {
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}
*/

NmpMsgFunRet
nmp_mod_bss_query_mss_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpQueryMss *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_mss_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryMssRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}

/*
NmpMsgFunRet
nmp_mod_bss_query_defence_area_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpQueryDefenceArea *req_info;
    NmpQueryDefenceAreaRes *res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = nmp_mem_kalloc(sizeof(NmpMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            nmp_error("<NmpModBss> alloc memory error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
        req_info = MSG_GET_DATA(msg);
    	 BUG_ON(!req_info);
        res_info = nmp_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(NmpQueryDefenceArea), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
           // nmp_print("<NmpModBss> query defence area list ok");
        }
        else
        {
            ret = -ENOMEM;
            nmp_error("<NmpModBss> query defence area failed, err:%d", ret);
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    if(res_info)
    {
        MSG_SET_RESPONSE(msg);
        SET_CODE(res_info, -ret);
    	 nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
    }
    else
    {
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}
*/

NmpMsgFunRet
nmp_mod_bss_query_defence_area_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpQueryDefenceArea *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_defence_area_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryDefenceAreaRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}

/*
NmpMsgFunRet
nmp_mod_bss_query_defence_map_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpQueryDefenceMap*req_info;
    NmpQueryDefenceMapRes *res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = nmp_mem_kalloc(sizeof(NmpMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            nmp_error("<NmpModBss> alloc memory error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
        req_info = MSG_GET_DATA(msg);
    	 BUG_ON(!req_info);
        res_info = nmp_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(NmpQueryDefenceMap), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
            //nmp_print("<NmpModBss> query defence map list ok");
        }
        else
        {
            ret = -ENOMEM;
            nmp_error("<NmpModBss> query defence map failed, err:%d", ret);
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    if(res_info)
    {
        MSG_SET_RESPONSE(msg);
        SET_CODE(res_info, -ret);
    	 nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
    }
    else
    {
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}
*/

NmpMsgFunRet
nmp_mod_bss_query_defence_map_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpQueryDefenceMap *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_defence_map_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryDefenceMapRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}

/*
NmpMsgFunRet
nmp_mod_bss_query_defence_gu_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpQueryDefenceGu *req_info;
    NmpQueryDefenceGuRes *res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = nmp_mem_kalloc(sizeof(NmpMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            nmp_error("<NmpModBss> alloc memory error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
        req_info = MSG_GET_DATA(msg);
    	 BUG_ON(!req_info);
        res_info = nmp_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(NmpQueryDefenceGu), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
            //nmp_print("<NmpModBss> query defence gu list ok");
        }
        else
        {
            ret = -ENOMEM;
            nmp_error("<NmpModBss> query mds failed, err:%d", ret);
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    if(res_info)
    {
        MSG_SET_RESPONSE(msg);
        SET_CODE(res_info, -ret);
    	 nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
    }
    else
    {
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}
*/

NmpMsgFunRet
nmp_mod_bss_query_defence_gu_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpQueryDefenceGu *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_defence_gu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryDefenceGuRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}

/*
NmpMsgFunRet
nmp_mod_bss_query_map_href_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpQueryMapHref *req_info;
    NmpQueryMapHrefRes *res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = nmp_mem_kalloc(sizeof(NmpMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            nmp_error("<NmpModBss> alloc memory error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
        req_info = MSG_GET_DATA(msg);
    	 BUG_ON(!req_info);
        res_info = nmp_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(NmpQueryMapHref), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
            //nmp_print("<NmpModBss> query defence gu list ok");
        }
        else
        {
            ret = -ENOMEM;
            nmp_error("<NmpModBss> query mds failed, err:%d", ret);
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    if(res_info)
    {
        MSG_SET_RESPONSE(msg);
        SET_CODE(res_info, -ret);
    	 nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
    }
    else
    {
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}
*/

NmpMsgFunRet
nmp_mod_bss_query_map_href_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpQueryMapHref *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_map_href_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryMapHrefRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}
/*
NmpMsgFunRet
nmp_mod_bss_query_tw_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpQueryTw *req_info;
    NmpQueryTwRes *res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = nmp_mem_kalloc(sizeof(NmpMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            nmp_error("<NmpModBss> alloc memory error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
        req_info = MSG_GET_DATA(msg);
    	 BUG_ON(!req_info);
        res_info = nmp_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(NmpQueryTw), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
            //nmp_print("<NmpModBss> query tw list ok");
        }
        else
        {
            ret = -ENOMEM;
            nmp_error("<NmpModBss> query tw failed, err:%d", ret);
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    if(res_info)
    {
        MSG_SET_RESPONSE(msg);
        SET_CODE(res_info, -ret);
    	 nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
    }
    else
    {
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}
*/

NmpMsgFunRet
nmp_mod_bss_query_tw_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpQueryTw *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_tw_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryTwRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}

/*
NmpMsgFunRet
nmp_mod_bss_query_screen_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpQueryScreen *req_info;
    NmpQueryScreenRes *res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = nmp_mem_kalloc(sizeof(NmpMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            nmp_error("<NmpModBss> alloc memory error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
        req_info = MSG_GET_DATA(msg);
    	 BUG_ON(!req_info);
        res_info = nmp_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(NmpQueryScreen), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
            //nmp_print("<NmpModBss> query screen list ok");
        }
        else
        {
            ret = -ENOMEM;
            nmp_error("<NmpModBss> query screen failed, err:%d", ret);
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    if(res_info)
    {
        MSG_SET_RESPONSE(msg);
        SET_CODE(res_info, -ret);
    	 nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
    }
    else
    {
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}
*/

NmpMsgFunRet
nmp_mod_bss_query_screen_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpQueryScreen *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_screen_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryScreenRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}

/*
NmpMsgFunRet
nmp_mod_bss_query_screen_division_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpQueryScrDiv *req_info;
    NmpQueryScrDivRes *res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = nmp_mem_kalloc(sizeof(NmpMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            nmp_error("<NmpModBss> alloc memory error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
        req_info = MSG_GET_DATA(msg);
    	 BUG_ON(!req_info);
        res_info = nmp_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(NmpQueryScrDiv), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
            //nmp_print("<NmpModBss> query screen division list ok");
        }
        else
        {
            ret = -ENOMEM;
            nmp_error("<NmpModBss> query screen division failed, err:%d", ret);
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    if(res_info)
    {
        MSG_SET_RESPONSE(msg);
        SET_CODE(res_info, -ret);
    	 nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
    }
    else
    {
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}
*/

NmpMsgFunRet
nmp_mod_bss_query_screen_division_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpQueryScrDiv *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_screen_division_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryScrDivRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_tour_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpQueryTour *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_tour_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryTourRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_tour_step_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpQueryTourStep *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_tour_step_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryTourStepRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_group_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpQueryGroup *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryGroupRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_group_step_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpQueryGroupStep *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_group_step_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryGroupStepRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_group_step_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpQueryGroupStepInfo *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_group_step_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryGroupStepInfoRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_group_step_div_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpQueryGroupStepDiv *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_group_step_div_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryGroupStepDivRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}
/*
NmpMsgFunRet
nmp_mod_bss_query_record_policy_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpQueryRecordPolicy *req_info;
    NmpQueryRecordPolicyRes *res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = nmp_mem_kalloc(sizeof(NmpMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            nmp_error("<NmpModBss> alloc memory error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
        req_info = MSG_GET_DATA(msg);
    	 BUG_ON(!req_info);
        res_info = nmp_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(NmpQueryRecordPolicy), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
            res_info->type = req_info->type;
            //nmp_print("<NmpModBss> query mds list ok");
        }
        else
        {
            ret = -ENOMEM;
            nmp_error("<NmpModBss> query mds failed, err:%d", ret);
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    if(res_info)
    {
        MSG_SET_RESPONSE(msg);
        SET_CODE(res_info, -ret);
    	 nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
    }
    else
    {
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}
*/


NmpMsgFunRet
nmp_mod_bss_query_record_policy_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpQueryRecordPolicy *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_record_policy_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryRecordPolicyRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_record_policy_config_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpRecordPolicyConfig *req_info;
    NmpMsgErrCode  res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret, i;
    gint size = 0;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);
    size = MSG_DATA_SIZE(msg);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = nmp_mod_bss_sync_req(self,  msg_id, req_info, size, NULL, 0);
        if (ret && (ret != -E_NODBENT))
        {
            nmp_warning(
                "<NmpModBss> modify policy failed, err:%d",
                 ret
            );
        }
        else
        {
            nmp_print(
                "<NmpModBss> modify policy ok"
            );

            if (ret != -E_NODBENT)
            {
                NmpNotifyPolicyChange policy_change;
                memset(&policy_change, 0, sizeof(policy_change));
                switch (req_info->type ){
                case 0:
                    for (i = 0; i < req_info->gu_count; i++)
                    {
                        strncpy(policy_change.guid, req_info->record_policy[i].guid, MAX_ID_LEN -1);
                        strncpy(policy_change.domain_id, req_info->record_policy[i].domain_id, DOMAIN_ID_LEN -1);
                        strncpy(policy_change.mss_id, req_info->record_policy[i].mss_id, MSS_ID_LEN -1);
                        nmp_mod_bss_notify_policy_change(app_obj, &policy_change, sizeof(policy_change));
                    }
                    break;
                case 1:
                    strncpy(policy_change.mss_id, req_info->mss_id, MSS_ID_LEN -1);
                    policy_change.all_changed = 1;
                    nmp_mod_bss_notify_policy_change(app_obj, &policy_change, sizeof(policy_change));
                    break;
                }
            }

	     ret = 0;
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


/*
NmpMsgFunRet
nmp_mod_bss_query_user_own_gu_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpQueryUserOwnGu *req_info;
    NmpQueryUserOwnGuRes *res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = nmp_mem_kalloc(sizeof(NmpMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            nmp_error("<NmpModBss> alloc memory error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
        req_info = MSG_GET_DATA(msg);
    	 BUG_ON(!req_info);

        res_info = nmp_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(NmpQueryUserOwnGu), &size);
        if (res_info)
        {
             ret = RES_CODE(res_info);
             //nmp_print("<NmpModBss> query mds list ok");
        }
        else
        {
            ret = -ENOMEM;
            nmp_error("query mds failed, err:%d", ret);
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    if(res_info)
    {
        MSG_SET_RESPONSE(msg);
        SET_CODE(res_info, -ret);
        nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
    }
    else
    {
        nmp_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}
*/

NmpMsgFunRet
nmp_mod_bss_query_user_own_gu_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpQueryUserOwnGu *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_user_own_gu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryUserOwnGuRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_user_own_tw_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpQueryUserOwnTw *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_user_own_tw_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryUserOwnTwRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_user_own_tour_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpQueryUserOwnTour *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_user_own_tour_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryUserOwnTourRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_del_alarm_policy_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpQueryDelAlarmPolicyRes res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        ret = nmp_mod_bss_sync_req(self, msg_id, NULL,
            0, &res_info, sizeof(res_info));
        if (ret)
        {
            nmp_warning("<NmpModBss> query del_alarm_policy failed, err:%d", ret);
        }
        else
        {
	     //nmp_print("<NmpModBss> query del_alarm_policy ok");
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}

NmpMsgFunRet
nmp_mod_bss_query_system_time_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpQueryServerTimeRes res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        ret = snprintf( res_info.system_time, TIME_LEN - 1, "%ld", time(NULL));
        nmp_mods_container_put_guest(self->container, bss_base);
    }

    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, 0);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_set_system_time_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpSetServerTime *req_info;
    NmpSetServerTimeRes res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;
    memset(&res_info, 0, sizeof(res_info));

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, res_info.bss_usr);
        req_info = MSG_GET_DATA(msg);
    	 BUG_ON(!req_info);
        ret = nmp_set_system_time(req_info->system_time, req_info->time_zone);
        nmp_mods_container_put_guest(self->container, bss_base);
    }

    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);

    /*** sent msg to mod_log ***/
    strncpy(res_info.time_zone, req_info->time_zone, TIME_ZONE_LEN - 1);	//must before nmp_sysmsg_set_private_2
    strncpy(res_info.system_time, req_info->system_time, TIME_LEN - 1);
    NMP_CREATE_MSG_TO_LOG(MSG_LOG_SET_SYSTEM_TIME, &res_info, sizeof(res_info));

    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_query_link_time_policy_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpQueryLinkTimePolicy *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_link_time_policy_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModBss *self;
	NmpQueryLinkTimePolicyRes *res_info;

	self = (NmpModBss*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (RES_CODE(res_info) < 0)
	    SET_CODE(res_info, -RES_CODE(res_info));

	return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_link_record_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpQueryLinkRecord *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_link_record_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModBss *self;
	NmpQueryLinkRecordRes *res_info;

	self = (NmpModBss*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (RES_CODE(res_info) < 0)
		SET_CODE(res_info, -RES_CODE(res_info));

	return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_link_IO_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpQueryLinkIO *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_link_IO_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModBss *self;
	NmpQueryLinkIORes *res_info;

	self = (NmpModBss*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (RES_CODE(res_info) < 0)
		SET_CODE(res_info, -RES_CODE(res_info));

	return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_link_snapshot_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpQueryLinkSnapshot *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_link_snapshot_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModBss *self;
	NmpQueryLinkSnapshotRes *res_info;

	self = (NmpModBss*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (RES_CODE(res_info) < 0)
		SET_CODE(res_info, -RES_CODE(res_info));

	return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_link_preset_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpQueryLinkPreset *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_link_preset_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModBss *self;
	NmpQueryLinkPresetRes *res_info;

	self = (NmpModBss*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (RES_CODE(res_info) < 0)
		SET_CODE(res_info, -RES_CODE(res_info));

	return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_link_step_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpQueryLinkStep *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_link_step_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModBss *self;
	NmpQueryLinkStepRes *res_info;

	self = (NmpModBss*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (RES_CODE(res_info) < 0)
		SET_CODE(res_info, -RES_CODE(res_info));

	return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_link_tour_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpQueryLinkTour *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_link_tour_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModBss *self;
	NmpQueryLinkTourRes *res_info;

	self = (NmpModBss*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (RES_CODE(res_info) < 0)
		SET_CODE(res_info, -RES_CODE(res_info));

	return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_link_group_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpQueryLinkGroup *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_link_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModBss *self;
	NmpQueryLinkGroupRes *res_info;

	self = (NmpModBss*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (RES_CODE(res_info) < 0)
		SET_CODE(res_info, -RES_CODE(res_info));

	return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_link_map_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpQueryLinkMap *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_link_map_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModBss *self;
	NmpQueryLinkMapRes *res_info;

	self = (NmpModBss*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (RES_CODE(res_info) < 0)
		SET_CODE(res_info, -RES_CODE(res_info));

	return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_ivs_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpQueryIvs *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_ivs_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryIvsRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_database_backup_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpModBss *self;
    NmpNetIO *io;
    NmpDbBackup *req_info;
    NmpMsgErrCode res_info;
    NmpMsgID msg_id;
    struct stat file_state;
    gint ret = 0;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    if (stat(nmp_get_sys_parm_str(SYS_PARM_DBBACKUPPATH),&file_state) ||
    		!S_ISDIR(file_state.st_mode))
    {
        ret = -ENOENT;
        nmp_warning("<NmpModBss> backup directory:%s does not exist.",
        			nmp_get_sys_parm_str(SYS_PARM_DBBACKUPPATH));
        MSG_SET_RESPONSE(msg);
        SET_CODE(&res_info, -ret);
        nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
        return MFR_DELIVER_BACK;
    }

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_database_backup_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

	NmpModBss *self;
	NmpBssRes *res_info;

	self = (NmpModBss*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (RES_CODE(res_info) < 0)
		SET_CODE(res_info, -RES_CODE(res_info));

	return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_database_import_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    NmpModBss *self;
    NmpNetIO *io;
    NmpDbImport *req_info;
    NmpDbImportRes res_info;
    NmpMsgID msg_id;
    struct stat file_state;
    gint ret = 0;
    char filename[STRING_LEN] = {0};

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));

    sprintf(filename, "%s/%s", nmp_get_sys_parm_str(SYS_PARM_DBBACKUPPATH),
			req_info->filename);
     if (stat(filename, &file_state) ||
 				!S_ISREG(file_state.st_mode))
     {
         ret = -ENOENT;
         nmp_warning("<NmpModBss> backup directory:%s does not exist.",
						nmp_get_sys_parm_str(SYS_PARM_DBBACKUPPATH));

        MSG_SET_RESPONSE(msg);
        SET_CODE(&res_info, -ret);
        nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

        return MFR_DELIVER_BACK;
     }

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_database_import_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModBss *self;
	NmpDbImportRes *res_info;

	self = (NmpModBss*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (RES_CODE(res_info) < 0)
		SET_CODE(res_info, -RES_CODE(res_info));

	if (RES_CODE(res_info) == 0)
      {
        sleep(2);
        exit(-1);
      }

	return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


static void comma_to_space(char *str, int len)
{
	int i;
	for (i = 0; i < len; i++)
	{
		if (str[i] == ',')
			str[i] = ' ';
	}
}

static int check_interface_valid(char *eth_str)
{
	if (strlen(eth_str) == strlen("eth0") &&
		strstr(eth_str, "eth") &&
		eth_str[3] >= '0' && eth_str[3] <= '9')
		return 0;

	return -1;
}


NmpMsgFunRet
nmp_mod_bss_get_net_interface_config_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModBss *self;
	NmpNetIO *io;
	NmpGuestBase *bss_base;
	NmpGetNetInterfaceConfigRes res_info;
	NmpMsgID msg_id;
	gint ret = 0;
	FILE *fp;
	char query_buf[STRING_LEN] = {0};
	char buffer[STRING_LEN] = {0};
	char res[STRING_LEN] = {0};

	self = (NmpModBss*)app_obj;
	io = MSG_IO(msg);
	BUG_ON(!io);

	msg_id = MSG_GETID(msg);
	memset(&res_info, 0, sizeof(res_info));
	bss_base = nmp_mods_container_get_guest(self->container, io);
	if (G_UNLIKELY(!bss_base))
	{
		ret = -E_NOSUCHGUEST;
		nmp_warning("<NmpModBss> user has not login.");
		goto end;
	}
	nmp_mods_container_put_guest(self->container, bss_base);

	snprintf(query_buf, STRING_LEN - 1, POPEN_GET_NET_INTERFACE);
	fp = popen(query_buf, "r");
	if (!fp)
	{
		nmp_warning("<NmpModBss> get net interface, popen failed");
		ret = -errno;
		goto end;
	}

	int first = 1;
	int str_len = 0;
	while (fgets(buffer, sizeof(buffer), fp) != NULL)
	{
		if (buffer[strlen(buffer) - 1] == '\n' || buffer[strlen(buffer) - 1] == '\r')
			buffer[strlen(buffer) - 1] = '\0';
		if (first)
		{
			str_len = snprintf(res, STRING_LEN, "%s", buffer);
			first = 0;
		}
		else
			str_len += snprintf(&res[str_len], STRING_LEN - str_len, ",%s", buffer);

		memset(buffer, 0, sizeof(buffer));
	}
	pclose(fp);
	res_info.network_interface[NET_INTERFACE_LEN - 1] = '\0';
	strncpy(res_info.network_interface, res, NET_INTERFACE_LEN - 1);

end:
	MSG_SET_RESPONSE(msg);
	SET_CODE(&res_info, -ret);
	nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

	return MFR_DELIVER_BACK;
}


static void get_net_info(NmpGetNetworkConfigRes *res_info, int ip_i, char *buffer)
{
	IpInfo *cur_ip_info = &res_info->ip_list[ip_i];

	if (QUERY_INFO(buffer, "ip"))
	{
		cur_ip_info->ip[MAX_IP_LEN - 1] = '\0';
		strncpy(cur_ip_info->ip, GET_INFO_ADDR(buffer), MAX_IP_LEN - 1);
	}
	else if (QUERY_INFO(buffer, "mask"))
	{
		cur_ip_info->netmask[MAX_IP_LEN - 1] = '\0';
		strncpy(cur_ip_info->netmask, GET_INFO_ADDR(buffer), MAX_IP_LEN - 1);
	}
	else if (QUERY_INFO(buffer, "gateway"))
	{
		cur_ip_info->gateway[MAX_IP_LEN - 1] = '\0';
		strncpy(cur_ip_info->gateway, GET_INFO_ADDR(buffer), MAX_IP_LEN - 1);
	}
	else if (QUERY_INFO(buffer, "dns"))
	{
		res_info->dns[MAX_IP_LEN - 1] = '\0';
		if (res_info->dns[0] == '\0')
			strncpy(res_info->dns, GET_INFO_ADDR(buffer), MAX_IP_LEN - 1);
		else
		{
			res_info->dns[strlen(res_info->dns)] = ',';
			strncpy(&res_info->dns[strlen(res_info->dns)], GET_INFO_ADDR(buffer),
				MAX_IP_LEN - 1 - strlen(res_info->dns));
			res_info->dns[MAX_IP_LEN - 1] = '\0';
		}
	}
}

NmpMsgFunRet
nmp_mod_bss_get_network_config_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModBss *self;
	NmpNetIO *io;
	NmpGuestBase *bss_base;
	NmpGetNetworkConfig *req_info;
	NmpGetNetworkConfigRes *res_info = NULL;
	NmpMsgID msg_id;
	gint ret = 0;
	gint size = 0;
	FILE *fp;
	char query_buf[STRING_LEN] = {0};
	char buffer[STRING_LEN] = {0};

	self = (NmpModBss*)app_obj;
	io = MSG_IO(msg);
	BUG_ON(!io);

	req_info = (NmpGetNetworkConfig *)MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	msg_id = MSG_GETID(msg);
	bss_base = nmp_mods_container_get_guest(self->container, io);
	if (G_UNLIKELY(!bss_base))
	{
		ret = -E_NOSUCHGUEST;
		nmp_warning("<NmpModBss> user has not login.");
		goto end;
	}
	nmp_mods_container_put_guest(self->container, bss_base);

	nmp_warning("<NmpModBss> get network configure");
	if (check_interface_valid(req_info->network_interface) != 0)
	{
		ret = -E_STRINGFORMAT;
		nmp_warning("<NmpModBss> interface msg error, network_interface:%s",
			req_info->network_interface);
		goto end;
	}

	snprintf(query_buf, STRING_LEN - 1, "%s %s", POPEN_GET_NETWORK_CONFIG,
		req_info->network_interface);
	fp = popen(query_buf, "r");
	if (!fp)
	{
		ret = -errno;
		nmp_warning("<NmpModBss> get network configure, popen failed\n");
		goto end;
	}

	int ip_i = -1;
	int first_line = 1;
	while (fgets(buffer, sizeof(buffer), fp) != NULL)
	{
		if (buffer[strlen(buffer) - 1] == '\n' || buffer[strlen(buffer) - 1] == '\r')
			buffer[strlen(buffer) - 1] = '\0';

		if (first_line)
		{
			int count = atoi(buffer);
			if (count < 0 || count > 255)
			{
				ret = -E_STRINGFORMAT;
				nmp_warning("<NmpModBss> error: count = %d", count);
				pclose(fp);
				goto end;
			}

			size = sizeof(NmpGetNetworkConfigRes) + sizeof(IpInfo) * count;
			res_info = (NmpGetNetworkConfigRes *)malloc(size);
			if (!res_info)
			{
				ret = -ENOMEM;
				nmp_warning("<NmpModBss> no memory!");
				size = 0;
				pclose(fp);
				goto end;
			}
			memset(res_info, 0, size);
			res_info->count = count;
			first_line = 0;
			continue;
		}

		if (QUERY_INFO(buffer, "IPINFO"))	//一组ip信息的开始
		{
			ip_i++;
			if (ip_i >= res_info->count)
				break;
			continue;
		}
		get_net_info(res_info, ip_i, buffer);
		memset(buffer, 0, sizeof(buffer));
	}
	pclose(fp);

end:
	MSG_SET_RESPONSE(msg);
	if (!res_info)
	{
		res_info = (NmpGetNetworkConfigRes *)malloc(sizeof(NmpGetNetworkConfigRes));
		if (!res_info)
		{
			nmp_sysmsg_destroy(msg);
			return MFR_ACCEPTED;
		}
	}
	SET_CODE(res_info, -ret);
	nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);

	return MFR_DELIVER_BACK;
}


static void get_network(char *network, char *ip, char *mask)
{
	unsigned int ip_uint = inet_addr(ip);
	unsigned int mask_uint = inet_addr(mask);
	unsigned int network_uint = ip_uint & mask_uint;
	char *tmp = NULL;

	struct in_addr inaddr;
	inaddr.s_addr = network_uint;
	tmp = inet_ntoa(inaddr);

	strncpy(network, tmp, MAX_IP_LEN);
}

static void get_broadcast(char *broadcast, char *ip, char *mask)
{
	unsigned int ip_uint = inet_addr(ip);
	unsigned int mask_uint = inet_addr(mask);
	unsigned int broadcast_uint = ip_uint | (~mask_uint);
	char *tmp = NULL;

	struct in_addr inaddr;
	inaddr.s_addr = broadcast_uint;
	tmp = inet_ntoa(inaddr);

	strncpy(broadcast, tmp, MAX_IP_LEN);
}


static int if_ip_conflict(char *ip1, char *mask1, char *ip2, char *mask2)
{
	//nmp_print("ip1:%s, mask1:%s, ip2:%s, mask2:%s", ip1, mask1, ip2, mask2);
	unsigned int ip1_uint = inet_addr(ip1), ip2_uint = inet_addr(ip2);
	unsigned int mask1_uint = inet_addr(mask1), mask2_uint = inet_addr(mask2);
	if (mask1_uint != mask2_uint)
		return 0;
	if ((ip1_uint & mask1_uint) == (ip2_uint & mask2_uint))
		return 1;
	return 0;
}


static int 
__check_netmask_valid(char *mask)
{
	unsigned int mask_host = ntohl(inet_addr(mask));
	unsigned int temp = ~mask_host + 1;

	if (mask_host != 0 && (temp & (temp - 1)) == 0)
		return 0;
	return 1;
}


static int check_netmask_valid(NmpSetNetworkConfig *req_info)
{
	int ip_i;

	for (ip_i = 0; ip_i < req_info->count; ip_i++)
	{
		IpInfo *cur_ip_info = &req_info->ip_list[ip_i];
		if (__check_netmask_valid(cur_ip_info->netmask) != 0)
		{
			nmp_warning("<NmpModBss> netmask error, netmask:%s", 
				cur_ip_info->netmask);
			return -E_NETMASKERR;
		}
	}

	return 0;
}


static int check_ip_conflict(NmpSetNetworkConfig *req_info)
{
	char *eth_str = req_info->network_interface;
	char query_buf[STRING_LEN] = {0};
	char buffer[STRING_LEN] = {0};
	char tmp_ip[MAX_IP_LEN] = {0}, tmp_mask[MAX_IP_LEN] = {0};
	int start_check = 0;
	FILE *fp;

	snprintf(query_buf, STRING_LEN - 1, "%s %s", POPEN_GET_IPS_BESIDES,
		eth_str);
	fp = popen(query_buf, "r");
	if (!fp)
	{
		nmp_warning("<NmpModBss> get_ips_besides, popen failed\n");
		return (-errno);
	}

	while (fgets(buffer, sizeof(buffer), fp) != NULL)
	{
		if (buffer[strlen(buffer) - 1] == '\n' || buffer[strlen(buffer) - 1] == '\r')
			buffer[strlen(buffer) - 1] = '\0';

		if (QUERY_INFO(buffer, "ip"))
		{
			tmp_ip[MAX_IP_LEN - 1] = '\0';
			strncpy(tmp_ip, GET_INFO_ADDR(buffer), MAX_IP_LEN - 1);
			start_check = 0;
		}
		else if (QUERY_INFO(buffer, "mask"))
		{
			tmp_mask[MAX_IP_LEN - 1] = '\0';
			strncpy(tmp_mask, GET_INFO_ADDR(buffer), MAX_IP_LEN - 1);
			start_check = 1;
		}
		else
		{
			nmp_warning("<NmpModBss> error, popen read buffer:%s", buffer);
			pclose(fp);
			return -E_GUESTIOCFLT;
		}

		if (!start_check)
			continue;

		int ip_i;
		for (ip_i = 0; ip_i < req_info->count; ip_i++)
		{
			IpInfo *cur_ip_info = &req_info->ip_list[ip_i];

			if (if_ip_conflict(tmp_ip, tmp_mask, cur_ip_info->ip,
				cur_ip_info->netmask))
			{
				nmp_warning("<NmpModBss> ip conflict, old ip:%s, mask:%s, " \
					"new ip:%s, mask:%s", tmp_ip, tmp_mask, cur_ip_info->ip,
					cur_ip_info->netmask);
				pclose(fp);
				return -E_GUESTIOCFLT;
			}
		}
	}
	pclose(fp);

	return 0;
}


NmpMsgFunRet
nmp_mod_bss_set_network_config_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModBss *self;
	NmpNetIO *io;
	NmpGuestBase *bss_base;
	NmpSetNetworkConfig *req_info;
	NmpSetResult	res_info;
	NmpMsgID msg_id;
	gint ret = 0;
	FILE *fp;
	char query_buf[STRING_LEN] = {0};
	gchar network[MAX_IP_LEN] = {0};
	gchar broadcast[MAX_IP_LEN] = {0};
	gchar dns[MAX_IP_LEN] = {0};

	self = (NmpModBss*)app_obj;
	io = MSG_IO(msg);
	BUG_ON(!io);

	msg_id = MSG_GETID(msg);
	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);
	strncpy(dns, req_info->dns, MAX_IP_LEN - 1);
	comma_to_space(dns, MAX_IP_LEN - 1);

	memset(&res_info, 0, sizeof(res_info));
	bss_base = nmp_mods_container_get_guest(self->container, io);
	if (G_UNLIKELY(!bss_base))
	{
		ret = -E_NOSUCHGUEST;
		nmp_warning("<NmpModBss> user has not login.");
		goto end;
	}
	NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, res_info.bss_usr);
	nmp_mods_container_put_guest(self->container, bss_base);

	nmp_warning("<NmpModBss> set network configure");
	if (check_interface_valid(req_info->network_interface) != 0)
	{
		ret = -E_STRINGFORMAT;
		nmp_warning("<NmpModBss> interface msg error, network_interface:%s",
			req_info->network_interface);
		goto end;
	}

	if ((ret = check_netmask_valid(req_info)) != 0)
	{
		goto end;
	}

	if ((ret = check_ip_conflict(req_info)) != 0)
	{
		nmp_warning("<NmpModBss> set %s, ip conflict", req_info->network_interface);
		goto end;
	}

	int ip_i = 0;
	for (ip_i = 0; ip_i < req_info->count; ip_i++)
	{
		IpInfo *cur_ip_info = &req_info->ip_list[ip_i];
		get_network(network, cur_ip_info->ip, cur_ip_info->netmask);
		get_broadcast(broadcast, cur_ip_info->ip, cur_ip_info->netmask);
		snprintf(query_buf, STRING_LEN - 1, "%s %s %d %s %s %s %s %s %s",
			POPEN_SET_NETWORK_CONFIG, req_info->network_interface,
			ip_i, cur_ip_info->ip, cur_ip_info->netmask, network, broadcast,
			cur_ip_info->gateway, dns);

		fp = popen(query_buf, "r");
		if (!fp)
		{
			nmp_warning("<NmpModBss> set network configure, popen failed\n");
			ret = -errno;
			goto end;
		}
		pclose(fp);
		memset(query_buf, 0, sizeof(query_buf));
	}
	snprintf(query_buf, STRING_LEN - 1, "%s %s %d &",
		POPEN_SET_NETWORK_CONFIG_END, req_info->network_interface, req_info->count);
	system(query_buf);
	sleep(3);

end:
	MSG_SET_RESPONSE(msg);
	SET_CODE(&res_info, -ret);

	NMP_CREATE_MSG_TO_LOG(MSG_LOG_SET_NETWORK_CONFIG, &res_info, sizeof(res_info));

	nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
	return MFR_DELIVER_BACK;
}

NmpMsgFunRet
nmp_mod_bss_add_hd_group_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpAddHdGroup *req_info;
    NmpAddHdGroupRes res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        MSG_SET_RESPONSE(msg);
        memset(&res_info, 0, sizeof(res_info));
        SET_CODE(&res_info, -ret);
        nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

        return MFR_DELIVER_BACK;
    }

    MSG_SET_DSTPOS(msg, BUSSLOT_POS_MSS);
    nmp_mods_container_put_guest(self->container, bss_base);

    return MFR_DELIVER_AHEAD;
}


NmpMsgFunRet
nmp_mod_bss_add_hd_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpAddHdGroupRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_bss_add_hd_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpAddHdToGroup *req_info;

    self = (NmpModBss*)app_obj;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward(self, msg, req_info->session);
}


NmpMsgFunRet
nmp_mod_bss_add_hd_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpAddHdToGroupRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->session);
}

NmpMsgFunRet
nmp_mod_bss_del_hd_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpDelHdFromGroup *req_info;

    self = (NmpModBss*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward(self, msg, req_info->session);
}


NmpMsgFunRet
nmp_mod_bss_del_hd_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpDelHdFromGroupRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_bss_reboot_mss_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpRebootMss *req_info;

    self = (NmpModBss*)app_obj;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward(self, msg, req_info->session);
}


NmpMsgFunRet
nmp_mod_bss_reboot_mss_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpRebootMssRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_bss_query_all_hd_group_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpQueryAllHdGroup *req_info;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward(self, msg, req_info->session);
}


NmpMsgFunRet
nmp_mod_bss_query_all_hd_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryAllHdGroupRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));
    return nmp_mod_bss_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_bss_query_hd_group_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpQueryHdGroupInfo *req_info;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward(self, msg, req_info->session);
}


NmpMsgFunRet
nmp_mod_bss_query_hd_group_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryHdGroupInfoRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_bss_query_all_hd_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpQueryAllHd *req_info;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward(self, msg, req_info->session);
}


NmpMsgFunRet
nmp_mod_bss_query_all_hd_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryAllHdRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_bss_del_hd_group_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpDelHdGroup *req_info;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward(self, msg, req_info->session);
}


NmpMsgFunRet
nmp_mod_bss_del_hd_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpDelHdGroupRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_bss_get_hd_format_progress_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpGetHdFormatProgress *req_info;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward(self, msg, req_info->session);
}


NmpMsgFunRet
nmp_mod_bss_get_hd_format_progress_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpGetHdFormatProgressRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_bss_query_gu_record_status_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpQueryGuRecordStatus *req_info;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward(self, msg, req_info->session);
}


NmpMsgFunRet
nmp_mod_bss_query_gu_record_status_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryGuRecordStatusRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_bss_platform_upgrade_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpGuestBase *bss_base;
    NmpPlatformUpgrade *req_info;
    NmpPlatformUpgradeResp res_info;
    NmpMsgID msg_id;
    gchar script_path[MAX_FILE_PATH + 2] = {0};
    gint ret = 0;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, res_info.bss_usr);
        nmp_warning("system begin to upgrade");
	 ret = nmp_get_platform_upgrade_script(script_path);
	 if (ret)
	 {
	     nmp_warning("get platform upgrade script error:%d", ret);
            system("upgrade-platform-system &");
	 }
	 else
	 {
	     strcat(script_path, " &");
	     system(script_path);
	 }

        nmp_mods_container_put_guest(self->container, bss_base);
     }

    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);

    NMP_CREATE_MSG_TO_LOG(MSG_LOG_PLATFORM_UPGRADE, &res_info, sizeof(res_info));

    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_query_alarm_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpQueryAlarm *req_info;
    NmpQueryAlarmRes  *res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = nmp_mem_kalloc(sizeof(NmpMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            nmp_warning("<NmpModBss> alloc memory error");
            nmp_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
    	req_info = MSG_GET_DATA(msg);
    	BUG_ON(!req_info);
       //NmpBss *bss;
     //  bss = (NmpBss*)bss_base;
       strncpy(req_info->bss_usr, ID_OF_GUEST(bss_base), USER_NAME_LEN - 1);

       /* res_info = nmp_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(NmpQueryAlarm), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
            printf("query alarm list code %d\n",ret);
            nmp_print("<NmpModBss> query alarm list ok");
        }
        else
        {
            ret = -ENOMEM;
            nmp_error("query alarm failed, err:%d", ret);
        }*/

        nmp_mods_container_put_guest(self->container, bss_base);
	 MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
        return MFR_DELIVER_AHEAD;
    }

    MSG_SET_RESPONSE(msg);
    if(res_info)
    {
        SET_CODE(res_info, -ret);
    	 nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
    }
    else
    {
        nmp_sysmsg_destroy(msg);
	 return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_query_alarm_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryAlarmRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_del_alarm_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpDelAlarm *req_info;
    NmpMsgCode  res_info;
    NmpGuestBase *bss_base;
    NmpMsgID msg_id;
    gint ret;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        NMP_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = nmp_mod_bss_sync_req(self, msg_id, req_info, sizeof(NmpDelAlarm), &res_info, sizeof(res_info));
        if (ret)
        {
            nmp_warning(
                "<NmpModBss> del alarm failed, err:%d",
                 ret
            );
        }
        else
        {
            nmp_print( "<NmpModBss> del alarm ok");
        }

        nmp_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_query_server_resource_info_f(NmpAppObj *app_obj,
	NmpSysMsg *msg)
{
	NmpQueryServerResourceInfo *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_server_resource_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModBss *self;
	NmpQueryServerResourceInfoRes *res_info;

	self = (NmpModBss*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (RES_CODE(res_info) < 0)
		SET_CODE(res_info, -RES_CODE(res_info));

	return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_search_device_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpGuestBase *bss_base;
    NmpSearchPuRes res_info;
    search_array_t  *pu_list;
    NmpMsgID msg_id;
    gint ret = 0;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    memset(&res_info, 0, sizeof(NmpSearchPuRes));
    msg_id = MSG_GETID(msg);
    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.",
        	MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        pu_list = search_device();
        nmp_search_pu_lock();
        nmp_set_search_pu_list(pu_list);
        nmp_search_pu_unlock();
        res_info.pu_count = pu_list->count;
        // res_info.pu_count = 10;
        nmp_warning("---------pu_list->count=%d",pu_list->count);
        destory_search_result(pu_list);
        ret = 0;
        nmp_mods_container_put_guest(self->container, bss_base);
    }

    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpGetSearchedPuRes *
nmp_get_searched_pus_list(NmpGetSearchedPu *req, search_pu_list *pu_list)
{
	gint size, start_num, res_count;
	NmpGetSearchedPuRes *get_search_pu = NULL;

	if ((req->start_num + req->req_num) > pu_list->count)
	{
		if (req->req_num > pu_list->count)
		{
			size = sizeof(NmpGetSearchedPuRes) + pu_list->count*sizeof(search_result_t);
			res_count = pu_list->count;
			start_num = 0;
		}
		else
		{
  			res_count = pu_list->count - req->start_num;

			size = sizeof(NmpGetSearchedPuRes) + res_count*sizeof(search_result_t);
			start_num = req->start_num;
		}

		get_search_pu = nmp_mem_kalloc(size);
		if (!get_search_pu)
			return NULL;

		memset(get_search_pu, 0, size);
		get_search_pu->pu_count = pu_list->count;
		get_search_pu->res_count = res_count;
		memcpy(get_search_pu->search_pu, &pu_list->result[start_num],
			res_count*sizeof(search_result_t));
		return get_search_pu;
	}

	size = sizeof(NmpGetSearchedPuRes) + req->req_num*sizeof(search_result_t);
	get_search_pu = nmp_mem_kalloc(size);
	if (get_search_pu)
	{
		memset(get_search_pu, 0, size);
		start_num = req->start_num;
		res_count = req->req_num;
		get_search_pu->pu_count = pu_list->count;
		get_search_pu->res_count = res_count;
		memcpy(get_search_pu->search_pu, &pu_list->result[start_num],
			res_count*sizeof(search_result_t));
	}
	return get_search_pu;
}


NmpMsgFunRet
nmp_mod_bss_get_searched_device_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpGuestBase *bss_base;
    NmpGetSearchedPu *req_info;
    NmpGetSearchedPuRes *res_info = NULL;
    search_pu_list  *pu_list = NULL;
    NmpMsgID msg_id;
    gint ret = 0, size;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    msg_id = MSG_GETID(msg);
    size = sizeof(NmpGetSearchedPuRes);
    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.",
        	MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        nmp_search_pu_lock();
        pu_list = nmp_get_search_pu_list(pu_list);
        res_info = nmp_get_searched_pus_list(req_info, pu_list);
        nmp_search_pu_unlock();
        if (res_info)
        {
            size = sizeof(NmpGetSearchedPuRes) + res_info->res_count*sizeof(search_result_t);
            int i;
            for (i = 0; i < res_info->res_count; i++)
            {
                nmp_covert_pu_type(&res_info->search_pu[i].nmp_srch.dev_info.pu_type,
                    &res_info->search_pu[i].nmp_srch.dev_info.pu_type);
             }
         }
        ret = 0;
        nmp_mods_container_put_guest(self->container, bss_base);
    }

    if (!res_info)
    {
        res_info = nmp_mem_kalloc(size);
        memset(res_info, 0, size);
    }
    MSG_SET_RESPONSE(msg);
    SET_CODE(res_info, -ret);
    nmp_sysmsg_set_private_2(msg, res_info, size);
    nmp_mem_kfree(res_info, size);
    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_query_cms_all_ips_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpGuestBase *bss_base;
    NmpQueryCmsAllIpRes *res_info = NULL;
    NmpHostIps ips;
    NmpMsgID msg_id;
    gint ret = 0, size;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s No such guest.",
        	MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        memset(&ips, 0, sizeof(ips));
        nmp_get_host_ips(&ips);
        size = sizeof(NmpQueryCmsAllIpRes) + ips.count*sizeof(NmpCmsIp);
        res_info = nmp_mem_kalloc(size);
        memset(res_info, 0, size);
        res_info->count = ips.count;
        memcpy(res_info->ips, ips.ips, ips.count*sizeof(NmpCmsIp));
        nmp_mods_container_put_guest(self->container, bss_base);
     }

    MSG_SET_RESPONSE(msg);
    if (!res_info)
    {
        size = sizeof(NmpQueryCmsAllIpRes);
        res_info = nmp_mem_kalloc(size);
        memset(res_info, 0, size);
    }

    SET_CODE(res_info, -ret);
    nmp_sysmsg_set_private_2(msg, res_info, size);
    nmp_mem_kfree(res_info, size);
    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_query_tw_auth_info_f(NmpAppObj *app_obj,
	NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpGuestBase *bss_base;
    NmpQueryTwAuthInfo *req_info;
    NmpQueryTwAuthInfoRes res_info;
    NmpMsgID msg_id;
    gint ret = 0;
    NmpResourcesCap res_cap;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s error,admin no login.",
            MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        memset(&res_cap, 0, sizeof(res_cap));
        nmp_mod_get_resource_cap(&res_cap);
        res_info.tw_auth_type = res_cap.modules_data[SYS_MODULE_TW];
        nmp_mods_container_put_guest(self->container, bss_base);
     }

    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_query_alarm_link_auth_info_f(NmpAppObj *app_obj,
	NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpGuestBase *bss_base;
    NmpQueryAlarmLinkAuthInfo *req_info;
    NmpQueryAlarmLinkAuthInfoRes res_info;
    NmpMsgID msg_id;
    gint ret = 0;
    NmpResourcesCap res_cap;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = nmp_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        nmp_warning("<NmpModBss> msg:%s error,admin no login.",
            MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        memset(&res_cap, 0, sizeof(res_cap));
        nmp_mod_get_resource_cap(&res_cap);
        res_info.alarm_link_auth_type = res_cap.modules_data[SYS_MODULE_ALM];
        nmp_mods_container_put_guest(self->container, bss_base);
     }

    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_auto_add_pu_f(NmpAppObj *app_obj,
	NmpSysMsg *msg)
{
    NmpAutoAddPu *req_info;
    NmpMsgErrCode res_info;
    NmpResourcesCap res_cap;
    gchar mf[MF_ID_LEN] = {0};
    gint ret;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if ((!regex_mached(req_info->puid, puid_reg))
                    ||(regex_mached(req_info->dev_name, exclude_reg))
			 || (!regex_mached(req_info->domain_id, domain_reg)))
    {
        SET_CODE(&res_info, E_STRINGFORMAT);
        goto err_auto_add_pu;
    }

    nmp_get_mf_from_guid(req_info->puid, mf);
    memset(&res_cap, 0, sizeof(res_cap));
    nmp_mod_get_resource_cap(&res_cap);
    if (res_cap.module_bits&MODULE_CMS_BIT)
    {
        ret = nmp_compare_manufacturer(res_cap.modules_data[SYS_MODULE_CMS], mf);
        if (ret)
        {
             SET_CODE(&res_info, -ret);
             goto err_auto_add_pu;
        }
    }
    else
        goto err_auto_add_pu;

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
err_auto_add_pu:
    MSG_SET_RESPONSE(msg);
    nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
    nmp_warning(
            "<NmpModBss> auto add pu:%s failed, err:%d",
            req_info->puid, E_STRINGFORMAT
        );
    return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_auto_add_pu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModBss *self;
	NmpBssRes *res_info;

	self = (NmpModBss*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (RES_CODE(res_info) < 0)
		SET_CODE(res_info, -RES_CODE(res_info));

	return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_get_next_puno_f(NmpAppObj *app_obj,
	NmpSysMsg *msg)
{
	NmpGetNextPuNo *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_get_next_puno_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModBss *self;
	NmpGetNextPuNoRes *res_info;

	self = (NmpModBss*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (RES_CODE(res_info) < 0)
		SET_CODE(res_info, -RES_CODE(res_info));

	return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_get_initiator_name_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpGetInitName *req_info;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward(self, msg, req_info->session);
}


NmpMsgFunRet
nmp_mod_bss_get_initiator_name_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpGetInitNameRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_bss_set_initiator_name_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpSetInitName *req_info;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward(self, msg, req_info->session);
}


NmpMsgFunRet
nmp_mod_bss_set_initiator_name_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpSetInitNameRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_bss_get_ipsan_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpGetIpsanInfo *req_info;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward(self, msg, req_info->session);
}


NmpMsgFunRet
nmp_mod_bss_get_ipsan_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpGetIpsanInfoRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_bss_add_one_ipsan_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpAddOneIpsan *req_info;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward(self, msg, req_info->session);
}


NmpMsgFunRet
nmp_mod_bss_add_one_ipsan_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpAddOneIpsanRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_bss_delete_one_ipsan_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpDeleteOneIpsan *req_info;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward(self, msg, req_info->session);
}


NmpMsgFunRet
nmp_mod_bss_delete_one_ipsan_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpDeleteOneIpsanRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_bss_get_one_ipsan_detail_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpNetIO *io;
    NmpGetOneIpsanDetail *req_info;

    self = (NmpModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward(self, msg, req_info->session);
}


NmpMsgFunRet
nmp_mod_bss_get_one_ipsan_detail_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpGetOneIpsanDetailRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->session);
}


NmpMsgFunRet
nmp_mod_bss_query_log_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpQueryLog *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return nmp_mod_bss_forward_to_log(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_log_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModBss *self;
	NmpQueryLogRes *res_info;

	self = (NmpModBss*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (RES_CODE(res_info) < 0)
		SET_CODE(res_info, -RES_CODE(res_info));

	return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


#if ONLINE_RATE_FLAG
NmpMsgFunRet
nmp_mod_bss_query_area_dev_rate_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpQueryAreaDevRate *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_area_dev_rate_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryAreaDevRateRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}
#endif


static int nmp_mod_get_state_num(char *operate)
{
	char query_buf[STRING_LEN] = {0};
	char buffer[STRING_LEN] = {0};
	FILE *fp;
	int res = -1;

	snprintf(query_buf, STRING_LEN - 1, operate);
	fp = popen(query_buf, "r");
	if (!fp)
	{
		nmp_warning("<NmpModBss> get_state_num, popen failed\n");
		return (-errno);
	}

	while (fgets(buffer, sizeof(buffer), fp) != NULL)
	{
		if (buffer[strlen(buffer) - 1] == '\n' || buffer[strlen(buffer) - 1] == '\r')
			buffer[strlen(buffer) - 1] = '\0';

		res = atoi(buffer);
		if (res < 0 || res > MAX_STATE_NUM)
		{
			nmp_warning("<NmpModBss> get_state_num error, state_num=%d\n", res);
			res = -E_STRINGFORMAT;
		}
		break;
	}
	pclose(fp);

	return res;
}


NmpMsgFunRet
nmp_mod_bss_get_server_flag_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModBss *self;
	NmpGetServerFlagRes res_info;
	gint ret = 0;
	memset(&res_info, 0, sizeof(res_info));

	self = (NmpModBss*)app_obj;

	res_info.server_flag = nmp_mod_get_state_num(POPEN_GET_MODULES_FLAG);
	if (res_info.server_flag < 0)
	{
		nmp_warning("<NmpModBss> get server_flag failed, server_flag = %d!",
			res_info.server_flag);
		ret = res_info.server_flag;
		res_info.server_flag = 0;
	}

	MSG_SET_RESPONSE(msg);
	SET_CODE(&res_info, -ret);
	nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

	return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_get_mds_config_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModBss *self;
	NmpNetIO *io;
	NmpGuestBase *bss_base;
	NmpGetMdsConfigRes res_info;
	gint ret = 0;
	memset(&res_info, 0, sizeof(res_info));

	self = (NmpModBss*)app_obj;
	io = MSG_IO(msg);
	BUG_ON(!io);

	bss_base = nmp_mods_container_get_guest(self->container, io);
	if (G_UNLIKELY(!bss_base))
	{
		ret = -E_NOSUCHGUEST;
		nmp_warning("<NmpModBss> user has not login.");
	}
	else
	{
		NmpMdsCtl gc;
		nmp_get_mds_parm(&gc);
		strncpy(res_info.mds_id, gc.mds_id, MDS_ID_LEN - 1);
		res_info.start_port = gc.start_port;
		res_info.end_port = gc.end_port;
		nmp_mods_container_put_guest(self->container, bss_base);
	}

	MSG_SET_RESPONSE(msg);
	SET_CODE(&res_info, -ret);
	nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

	return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_set_mds_config_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModBss *self;
	NmpNetIO *io;
	NmpGuestBase *bss_base;
	NmpSetMdsConfig *req_info;
	NmpSetResult res_info;
	NmpMsgID msg_id;
	gint ret = 0;
	memset(&res_info, 0, sizeof(res_info));

	self = (NmpModBss*)app_obj;
	io = MSG_IO(msg);
	BUG_ON(!io);

	msg_id = MSG_GETID(msg);
	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	bss_base = nmp_mods_container_get_guest(self->container, io);
	if (G_UNLIKELY(!bss_base))
	{
		ret = -E_NOSUCHGUEST;
		nmp_warning("<NmpModBss> user has not login.");
	}
	else
	{
		NmpMdsCtl gc, gc_old;
		memset(&gc, 0, sizeof(gc));
		memset(&gc_old, 0, sizeof(gc_old));
		nmp_get_mds_parm(&gc_old);
		strncpy(gc.mds_id, req_info->mds_id, MDS_ID_LEN - 1);
		gc.start_port = req_info->start_port;
		gc.end_port = req_info->end_port;

		nmp_set_mds_parm(gc);
		if (strcmp(gc.mds_id, gc_old.mds_id))
			system("killall -9 mds");
		nmp_mods_container_put_guest(self->container, bss_base);
	}

	MSG_SET_RESPONSE(msg);
	SET_CODE(&res_info, -ret);
	nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

	return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_get_mds_state_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModBss *self;
	NmpNetIO *io;
	NmpGuestBase *bss_base;
	NmpGetMdsStateRes res_info;
	NmpMsgID msg_id;
	gint ret = 0;

	self = (NmpModBss*)app_obj;
	io = MSG_IO(msg);
	BUG_ON(!io);

	msg_id = MSG_GETID(msg);
	memset(&res_info, 0, sizeof(res_info));
	bss_base = nmp_mods_container_get_guest(self->container, io);
	if (G_UNLIKELY(!bss_base))
	{
		ret = -E_NOSUCHGUEST;
		nmp_warning("<NmpModBss> user has not login.");
	}
	else
	{
		nmp_mods_container_put_guest(self->container, bss_base);
	}

	MSG_SET_RESPONSE(msg);
	res_info.state = nmp_mod_get_state_num(POPEN_GET_MDS_STATE);
	if (res_info.state < 0)
	{
		ret = res_info.state;
		res_info.state = 0;
	}
	SET_CODE(&res_info, -ret);
	nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

	return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_get_mss_config_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModBss *self;
	NmpNetIO *io;
	NmpGuestBase *bss_base;
	NmpGetMssConfigRes res_info;
	gint ret = 0;
	memset(&res_info, 0, sizeof(res_info));

	self = (NmpModBss*)app_obj;
	io = MSG_IO(msg);
	BUG_ON(!io);

	bss_base = nmp_mods_container_get_guest(self->container, io);
	if (G_UNLIKELY(!bss_base))
	{
		ret = -E_NOSUCHGUEST;
		nmp_warning("<NmpModBss> user has not login.");
	}
	else
	{
		NmpMssCtl gc;
		nmp_get_mss_parm(&gc);
		strncpy(res_info.mss_id, gc.mss_id, MSS_ID_LEN - 1);
		nmp_mods_container_put_guest(self->container, bss_base);
	}

	MSG_SET_RESPONSE(msg);
	SET_CODE(&res_info, -ret);
	nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

	return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_set_mss_config_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModBss *self;
	NmpNetIO *io;
	NmpGuestBase *bss_base;
	NmpSetMssConfig *req_info;
	NmpSetResult res_info;
	NmpMsgID msg_id;
	gint ret = 0;
	memset(&res_info, 0, sizeof(res_info));

	self = (NmpModBss*)app_obj;
	io = MSG_IO(msg);
	BUG_ON(!io);

	msg_id = MSG_GETID(msg);
	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	bss_base = nmp_mods_container_get_guest(self->container, io);
	if (G_UNLIKELY(!bss_base))
	{
		ret = -E_NOSUCHGUEST;
		nmp_warning("<NmpModBss> user has not login.");
	}
	else
	{
		NmpMssCtl gc, gc_old;
		memset(&gc, 0, sizeof(gc));
		memset(&gc_old, 0, sizeof(gc_old));
		nmp_get_mss_parm(&gc_old);
		strncpy(gc.mss_id, req_info->mss_id, MSS_ID_LEN - 1);

		nmp_set_mss_parm(gc);
		if (strcmp(gc.mss_id, gc_old.mss_id))
			system("killall -9 mss");
		nmp_mods_container_put_guest(self->container, bss_base);
	}

	MSG_SET_RESPONSE(msg);
	SET_CODE(&res_info, -ret);
	nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

	return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_get_mss_state_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModBss *self;
	NmpNetIO *io;
	NmpGuestBase *bss_base;
	NmpGetMssStateRes res_info;
	NmpMsgID msg_id;
	gint ret = 0;

	self = (NmpModBss*)app_obj;
	io = MSG_IO(msg);
	BUG_ON(!io);

	msg_id = MSG_GETID(msg);
	memset(&res_info, 0, sizeof(res_info));
	bss_base = nmp_mods_container_get_guest(self->container, io);
	if (G_UNLIKELY(!bss_base))
	{
		ret = -E_NOSUCHGUEST;
		nmp_warning("<NmpModBss> user has not login.");
	}
	else
	{
		nmp_mods_container_put_guest(self->container, bss_base);
	}

	MSG_SET_RESPONSE(msg);
	res_info.state = nmp_mod_get_state_num(POPEN_GET_MSS_STATE);
	if (res_info.state < 0)
	{
		ret = res_info.state;
		res_info.state = 0;
	}
	SET_CODE(&res_info, -ret);
	nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

	return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_get_ivs_config_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModBss *self;
	NmpNetIO *io;
	NmpGuestBase *bss_base;
	NmpGetIvsConfigRes res_info;
	gint ret = 0;
	memset(&res_info, 0, sizeof(res_info));

	self = (NmpModBss*)app_obj;
	io = MSG_IO(msg);
	BUG_ON(!io);

	bss_base = nmp_mods_container_get_guest(self->container, io);
	if (G_UNLIKELY(!bss_base))
	{
		ret = -E_NOSUCHGUEST;
		nmp_warning("<NmpModBss> user has not login.");
	}
	else
	{
		NmpIvsCtl gc;
		nmp_get_ivs_parm(&gc);
		strncpy(res_info.ivs_id, gc.ivs_id, IVS_ID_LEN - 1);
		nmp_mods_container_put_guest(self->container, bss_base);
	}

	MSG_SET_RESPONSE(msg);
	SET_CODE(&res_info, -ret);
	nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

	return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_set_ivs_config_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModBss *self;
	NmpNetIO *io;
	NmpGuestBase *bss_base;
	NmpSetIvsConfig *req_info;
	NmpSetResult res_info;
	NmpMsgID msg_id;
	gint ret = 0;
	memset(&res_info, 0, sizeof(res_info));

	self = (NmpModBss*)app_obj;
	io = MSG_IO(msg);
	BUG_ON(!io);

	msg_id = MSG_GETID(msg);
	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	bss_base = nmp_mods_container_get_guest(self->container, io);
	if (G_UNLIKELY(!bss_base))
	{
		ret = -E_NOSUCHGUEST;
		nmp_warning("<NmpModBss> user has not login.");
	}
	else
	{
		NmpIvsCtl gc, gc_old;
		memset(&gc, 0, sizeof(gc));
		memset(&gc_old, 0, sizeof(gc_old));
		nmp_get_ivs_parm(&gc_old);
		strncpy(gc.ivs_id, req_info->ivs_id, IVS_ID_LEN - 1);

		nmp_set_ivs_parm(gc);
		if (strcmp(gc.ivs_id, gc_old.ivs_id))
			system("killall -9 ivs");
		nmp_mods_container_put_guest(self->container, bss_base);
	}

	MSG_SET_RESPONSE(msg);
	SET_CODE(&res_info, -ret);
	nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

	return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_get_ivs_state_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModBss *self;
	NmpNetIO *io;
	NmpGuestBase *bss_base;
	NmpGetIvsStateRes res_info;
	NmpMsgID msg_id;
	gint ret = 0;

	self = (NmpModBss*)app_obj;
	io = MSG_IO(msg);
	BUG_ON(!io);

	msg_id = MSG_GETID(msg);
	memset(&res_info, 0, sizeof(res_info));
	bss_base = nmp_mods_container_get_guest(self->container, io);
	if (G_UNLIKELY(!bss_base))
	{
		ret = -E_NOSUCHGUEST;
		nmp_warning("<NmpModBss> user has not login.");
	}
	else
	{
		nmp_mods_container_put_guest(self->container, bss_base);
	}

	MSG_SET_RESPONSE(msg);
	res_info.state = nmp_mod_get_state_num(POPEN_GET_IVS_STATE);
	if (res_info.state < 0)
	{
		ret = res_info.state;
		res_info.state = 0;
	}
	SET_CODE(&res_info, -ret);
	nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

	return MFR_DELIVER_BACK;
}


static void nmp_will_shutdown()
{
	system("sleep 1 && shutdown -h now &");
};

static NmpMsgFunRet
nmp_mod_bss_cms_shutdown_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpSetResult res_info;
	memset(&res_info, 0, sizeof(res_info));

	SET_CODE(&res_info, 0);

	nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
	MSG_SET_RESPONSE(msg);

	nmp_will_shutdown();

	return MFR_DELIVER_BACK;
}


static void nmp_will_reboot()
{
	system("sleep 1 && reboot &");
};

static NmpMsgFunRet
nmp_mod_bss_cms_reboot_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpSetResult res_info;
	memset(&res_info, 0, sizeof(res_info));

	SET_CODE(&res_info, 0);

	nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
	MSG_SET_RESPONSE(msg);

	nmp_will_reboot();

	return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_bss_add_ams_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpAddAms *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_add_ams_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpBssRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_modify_ams_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModifyAms *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_modify_ams_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpBssRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_del_ams_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpDelAms *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_del_ams_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpBssRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_ams_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpQueryAms *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_ams_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryAmsRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_ams_pu_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpQueryAmsPu *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_query_ams_pu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpQueryAmsPuRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_modify_ams_pu_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModifyAmsPu *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return nmp_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


NmpMsgFunRet
nmp_mod_bss_modify_ams_pu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
    NmpModBss *self;
    NmpBssRes *res_info;

    self = (NmpModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return nmp_mod_bss_backward(self, msg, res_info->bss_usr);
}



void
nmp_mod_bss_register_msg_handler(NmpModBss *self)
{
    NmpAppMod *super_self = (NmpAppMod*)self;

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_BSS_LOGIN,
        nmp_mod_bss_login_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_BSS_HEART,
        nmp_mod_bss_heart_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_ADMIN,
        nmp_mod_bss_add_admin_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_ADMIN,
        nmp_mod_bss_modify_admin_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_ADMIN,
        nmp_mod_bss_del_admin_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_ADMIN,
        nmp_mod_bss_query_admin_f,
        nmp_mod_bss_query_admin_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_VALIDATA_ADMIN,
        nmp_mod_bss_validata_admin_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_USER_GROUP,
        nmp_mod_bss_add_user_group_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_VALIDATA_USER_GROUP,
        nmp_mod_bss_validata_user_group_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_USER_GROUP,
        nmp_mod_bss_query_user_group_f,
        nmp_mod_bss_query_user_group_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_USER_GROUP,
        nmp_mod_bss_modify_user_group_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_USER_GROUP,
        nmp_mod_bss_del_user_group_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_USER,
        nmp_mod_bss_add_user_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_VALIDATA_USER,
        nmp_mod_bss_validata_user_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_USER,
        nmp_mod_bss_query_user_f,
        nmp_mod_bss_query_user_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_USER,
        nmp_mod_bss_modify_user_f,
        NULL,
        0
    );

   nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_USER,
        nmp_mod_bss_del_user_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_DOMAIN,
        nmp_mod_bss_query_domain_f,
        nmp_mod_bss_query_domain_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_DOMAIN,
        nmp_mod_bss_modify_domain_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_DOMAIN,
        NULL,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_DOMAIN,
        NULL,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_AREA,
        nmp_mod_bss_query_area_f,
        nmp_mod_bss_query_area_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_MODIFY_AREA,
        nmp_mod_bss_add_modify_area_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_AREA,
        nmp_mod_bss_del_area_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_PU,
        nmp_mod_bss_add_pu_f,
        nmp_mod_bss_add_pu_b,
        0
    );

    /*nmp_app_mod_register_msg(
        super_self,
        MESSAGE_VALIDATA_PU,
        nmp_mod_bss_validata_pu_f,
        NULL,
        0
    );*/

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_PU,
        nmp_mod_bss_query_pu_f,
        nmp_mod_bss_query_pu_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_PU,
        nmp_mod_bss_modify_pu_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_PU,
        nmp_mod_bss_del_pu_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_GU,
        nmp_mod_bss_add_gu_f,
        nmp_mod_bss_add_gu_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_GU,
        nmp_mod_bss_query_gu_f,
        nmp_mod_bss_query_gu_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_GU,
        nmp_mod_bss_modify_gu_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_GU,
        nmp_mod_bss_del_gu_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_MDS,
        nmp_mod_bss_add_mds_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_MDS,
        nmp_mod_bss_query_mds_f,
        nmp_mod_bss_query_mds_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_MDS,
        nmp_mod_bss_modify_mds_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_MDS,
        nmp_mod_bss_del_mds_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_MDS_IP,
        nmp_mod_bss_add_mds_ip_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_MDS_IP,
        nmp_mod_bss_query_mds_ip_f,
        nmp_mod_bss_query_mds_ip_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_MDS_IP,
        nmp_mod_bss_del_mds_ip_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_MSS,
        nmp_mod_bss_add_mss_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_MSS,
        nmp_mod_bss_query_mss_f,
        nmp_mod_bss_query_mss_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_MSS,
        nmp_mod_bss_modify_mss_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_MSS,
        nmp_mod_bss_del_mss_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_RECORD_POLICY,
        nmp_mod_bss_query_record_policy_f,
        nmp_mod_bss_query_record_policy_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_RECORD_POLICY_CONFIG,
        nmp_mod_bss_record_policy_config_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_MANUFACTURER,
        nmp_mod_bss_query_manufacturer_f,
        nmp_mod_bss_query_manufacturer_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_MODIFY_MANUFACTURER,
        nmp_mod_bss_add_modify_manufacturer_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_MANUFACTURER,
        nmp_mod_bss_del_manufacturer_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_GU_TO_USER,
        nmp_mod_bss_add_gu_to_user_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_USER_OWN_GU,
        nmp_mod_bss_query_user_own_gu_f,
        nmp_mod_bss_query_user_own_gu_b,
        0
    );

	nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_TW_TO_USER,
        nmp_mod_bss_add_tw_to_user_f,
        nmp_mod_bss_add_tw_to_user_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_USER_OWN_TW,
        nmp_mod_bss_query_user_own_tw_f,
        nmp_mod_bss_query_user_own_tw_b,
        0
    );

	nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_TOUR_TO_USER,
        nmp_mod_bss_add_tour_to_user_f,
        nmp_mod_bss_add_tour_to_user_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_USER_OWN_TOUR,
        nmp_mod_bss_query_user_own_tour_f,
        nmp_mod_bss_query_user_own_tour_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_TIME,
        nmp_mod_bss_query_system_time_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_TIME,
        nmp_mod_bss_set_system_time_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DATABASE_BACKUP,
        nmp_mod_bss_database_backup_f,
        nmp_mod_bss_database_backup_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DATABASE_IMPORT,
        nmp_mod_bss_database_import_f,
        nmp_mod_bss_database_import_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_NETINTERFACE_CONFIG,
        nmp_mod_bss_get_net_interface_config_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_NETWORK_CONFIG,
        nmp_mod_bss_get_network_config_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_NETWORK_CONFIG,
        nmp_mod_bss_set_network_config_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_HD_GROUP,
        nmp_mod_bss_add_hd_group_f,
        nmp_mod_bss_add_hd_group_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_HD,
        nmp_mod_bss_add_hd_f,
        nmp_mod_bss_add_hd_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_HD,
        nmp_mod_bss_del_hd_f,
        nmp_mod_bss_del_hd_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_REBOOT_MSS,
        nmp_mod_bss_reboot_mss_f,
        nmp_mod_bss_reboot_mss_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_ALL_HD_GROUP,
        nmp_mod_bss_query_all_hd_group_f,
        nmp_mod_bss_query_all_hd_group_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_HD_GROUP_INFO,
        nmp_mod_bss_query_hd_group_info_f,
        nmp_mod_bss_query_hd_group_info_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_ALL_HD,
        nmp_mod_bss_query_all_hd_f,
        nmp_mod_bss_query_all_hd_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_HD_GROUP,
        nmp_mod_bss_del_hd_group_f,
        nmp_mod_bss_del_hd_group_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_HD_FORMAT_PROGRESS,
        nmp_mod_bss_get_hd_format_progress_f,
        nmp_mod_bss_get_hd_format_progress_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_GU_RECORD_STATUS,
        nmp_mod_bss_query_gu_record_status_f,
        nmp_mod_bss_query_gu_record_status_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_DEFENCE_AREA,
        nmp_mod_bss_add_defence_area_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_DEFENCE_AREA,
        nmp_mod_bss_modify_defence_area_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_DEFENCE_AREA,
        nmp_mod_bss_del_defence_area_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_DEFENCE_AREA,
        nmp_mod_bss_query_defence_area_f,
        nmp_mod_bss_query_defence_area_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_DEFENCE_MAP,
        nmp_mod_bss_add_defence_map_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_DEFENCE_MAP,
        nmp_mod_bss_del_defence_map_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_DEFENCE_MAP,
        nmp_mod_bss_query_defence_map_f,
        nmp_mod_bss_query_defence_map_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_DEFENCE_GU,
        nmp_mod_bss_add_defence_gu_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_DEFENCE_GU,
        nmp_mod_bss_modify_defence_gu_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_DEFENCE_GU,
        nmp_mod_bss_del_defence_gu_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_DEFENCE_GU,
        nmp_mod_bss_query_defence_gu_f,
        nmp_mod_bss_query_defence_gu_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_MAP_HREF,
        nmp_mod_bss_set_map_href_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_MAP_HREF,
        nmp_mod_bss_modify_map_href_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_MAP_HREF,
        nmp_mod_bss_del_map_href_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_MAP_HREF,
        nmp_mod_bss_query_map_href_f,
        nmp_mod_bss_query_map_href_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_PLATFORM_UPGRADE,
        nmp_mod_bss_platform_upgrade_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_ALARM,
        nmp_mod_bss_query_alarm_f,
        nmp_mod_bss_query_alarm_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_ALARM,
        nmp_mod_bss_del_alarm_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_DEL_ALARM_POLICY,
        nmp_mod_bss_query_del_alarm_policy_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_DEL_ALARM_POLICY,
        nmp_mod_bss_set_del_alarm_policy_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_TW,
        nmp_mod_bss_add_tw_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_TW,
        nmp_mod_bss_modify_tw_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_TW,
        nmp_mod_bss_del_tw_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_TW,
        nmp_mod_bss_query_tw_f,
        nmp_mod_bss_query_tw_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_SCREEN,
        nmp_mod_bss_add_screen_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_SCREEN,
        nmp_mod_bss_modify_screen_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_SCREEN,
        nmp_mod_bss_del_screen_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_SCREEN,
        nmp_mod_bss_query_screen_f,
        nmp_mod_bss_query_screen_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_SCR_DIV,
        nmp_mod_bss_query_screen_division_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_TOUR,
        nmp_mod_bss_add_tour_f,
        nmp_mod_bss_add_tour_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_TOUR,
        nmp_mod_bss_modify_tour_f,
        nmp_mod_bss_modify_tour_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_TOUR,
        nmp_mod_bss_del_tour_f,
        nmp_mod_bss_del_tour_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_TOUR,
        nmp_mod_bss_query_tour_f,
        nmp_mod_bss_query_tour_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_TOUR_STEP,
        nmp_mod_bss_add_tour_step_f,
        nmp_mod_bss_add_tour_step_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_TOUR_STEP,
        nmp_mod_bss_query_tour_step_f,
        nmp_mod_bss_query_tour_step_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_GROUP,
        nmp_mod_bss_add_group_f,
        nmp_mod_bss_general_cmd_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_GROUP,
        nmp_mod_bss_modify_group_f,
        nmp_mod_bss_general_modify_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_GROUP,
        nmp_mod_bss_del_group_f,
        nmp_mod_bss_general_cmd_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_GROUP,
        nmp_mod_bss_query_group_f,
        nmp_mod_bss_query_group_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_GROUP_STEP,
        nmp_mod_bss_add_group_step_f,
        nmp_mod_bss_general_cmd_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_GROUP_STEP,
        nmp_mod_bss_modify_group_step_f,
        nmp_mod_bss_general_modify_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_GROUP_STEP,
        nmp_mod_bss_del_group_step_f,
        nmp_mod_bss_general_cmd_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_GROUP_STEP,
        nmp_mod_bss_query_group_step_f,
        nmp_mod_bss_query_group_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_CONFIG_GROUP_STEP,
        nmp_mod_bss_config_group_step_f,
        nmp_mod_bss_general_cmd_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_GROUP_STEP_INFO,
        nmp_mod_bss_modify_group_step_info_f,
        nmp_mod_bss_general_modify_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_GROUP_STEP_INFO,
        nmp_mod_bss_del_group_step_info_f,
        nmp_mod_bss_general_cmd_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_GROUP_STEP_INFO,
        nmp_mod_bss_query_group_step_info_f,
        nmp_mod_bss_query_group_step_info_b,
        0
    );

  	nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_GROUP_STEP_DIV,
        nmp_mod_bss_query_group_step_div_f,
        nmp_mod_bss_query_group_step_div_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_LINK_TIME_POLICY_CONFIG,
        nmp_mod_bss_link_time_policy_config_f,
        nmp_mod_bss_general_cmd_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_LINK_TIME_POLICY,
        nmp_mod_bss_modify_link_time_policy_f,
        nmp_mod_bss_general_modify_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_LINK_TIME_POLICY,
        nmp_mod_bss_query_link_time_policy_f,
        nmp_mod_bss_query_link_time_policy_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_LINK_TIME_POLICY,
        nmp_mod_bss_del_link_time_policy_f,
        nmp_mod_bss_general_cmd_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_LINK_RECORD_CONFIG,
        nmp_mod_bss_link_record_f,
        nmp_mod_bss_general_cmd_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_LINK_RECORD,
        nmp_mod_bss_modify_link_record_f,
        nmp_mod_bss_general_modify_b,
        0
    );


    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_LINK_RECORD,
        nmp_mod_bss_query_link_record_f,
        nmp_mod_bss_query_link_record_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_LINK_RECORD,
        nmp_mod_bss_del_link_record_f,
        nmp_mod_bss_general_cmd_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_LINK_IO_CONFIG,
        nmp_mod_bss_link_IO_f,
        nmp_mod_bss_general_cmd_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_LINK_IO,
        nmp_mod_bss_modify_link_IO_f,
        nmp_mod_bss_general_modify_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_LINK_IO,
        nmp_mod_bss_query_link_IO_f,
        nmp_mod_bss_query_link_IO_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_LINK_IO,
        nmp_mod_bss_del_link_IO_f,
        nmp_mod_bss_general_cmd_b,
        0
    );

nmp_app_mod_register_msg(
        super_self,
        MESSAGE_LINK_SNAPSHOT_CONFIG,
        nmp_mod_bss_link_snapshot_f,
        nmp_mod_bss_general_cmd_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_LINK_SNAPSHOT,
        nmp_mod_bss_modify_link_snapshot_f,
        nmp_mod_bss_general_modify_b,
        0
    );


    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_LINK_SNAPSHOT,
        nmp_mod_bss_query_link_snapshot_f,
        nmp_mod_bss_query_link_snapshot_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_LINK_SNAPSHOT,
        nmp_mod_bss_del_link_snapshot_f,
        nmp_mod_bss_general_cmd_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_LINK_PRESET_CONFIG,
        nmp_mod_bss_link_preset_f,
        nmp_mod_bss_general_cmd_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_LINK_PRESET,
        nmp_mod_bss_modify_link_preset_f,
        nmp_mod_bss_general_modify_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_LINK_PRESET,
        nmp_mod_bss_query_link_preset_f,
        nmp_mod_bss_query_link_preset_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_LINK_PRESET,
        nmp_mod_bss_del_link_preset_f,
        nmp_mod_bss_general_cmd_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_LINK_STEP_CONFIG,
        nmp_mod_bss_link_step_f,
        nmp_mod_bss_general_cmd_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_LINK_STEP,
        nmp_mod_bss_modify_link_step_f,
        nmp_mod_bss_general_modify_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_LINK_STEP,
        nmp_mod_bss_query_link_step_f,
        nmp_mod_bss_query_link_step_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_LINK_STEP,
        nmp_mod_bss_del_link_step_f,
        nmp_mod_bss_general_cmd_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_LINK_TOUR_CONFIG,
        nmp_mod_bss_link_tour_f,
        nmp_mod_bss_general_cmd_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_LINK_TOUR,
        nmp_mod_bss_modify_link_tour_f,
        nmp_mod_bss_general_modify_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_LINK_TOUR,
        nmp_mod_bss_query_link_tour_f,
        nmp_mod_bss_query_link_tour_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_LINK_TOUR,
        nmp_mod_bss_del_link_tour_f,
        nmp_mod_bss_general_cmd_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_LINK_GROUP_CONFIG,
        nmp_mod_bss_link_group_f,
        nmp_mod_bss_general_cmd_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_LINK_GROUP,
        nmp_mod_bss_modify_link_group_f,
        nmp_mod_bss_general_modify_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_LINK_GROUP,
        nmp_mod_bss_query_link_group_f,
        nmp_mod_bss_query_link_group_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_LINK_GROUP,
        nmp_mod_bss_del_link_group_f,
        nmp_mod_bss_general_cmd_b,
        0
    );

   nmp_app_mod_register_msg(
        super_self,
        MESSAGE_LINK_MAP_CONFIG,
        nmp_mod_bss_link_map_f,
        nmp_mod_bss_general_cmd_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_LINK_MAP,
        nmp_mod_bss_modify_link_map_f,
        nmp_mod_bss_general_modify_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_LINK_MAP,
        nmp_mod_bss_query_link_map_f,
        nmp_mod_bss_query_link_map_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_LINK_MAP,
        nmp_mod_bss_del_link_map_f,
        nmp_mod_bss_general_cmd_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_SERVER_RESOURCE,
        nmp_mod_bss_query_server_resource_info_f,
        nmp_mod_bss_query_server_resource_info_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_TW_AUTH_INFO,
        nmp_mod_bss_query_tw_auth_info_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_ALARM_LINK_AUTH_INFO,
        nmp_mod_bss_query_alarm_link_auth_info_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_CMS_ALL_IP,
        nmp_mod_bss_query_cms_all_ips_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SEARCH_PU,
        nmp_mod_bss_search_device_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_SEARCH_PUS,
        nmp_mod_bss_get_searched_device_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_AUTO_ADD_PU,
        nmp_mod_bss_auto_add_pu_f,
        nmp_mod_bss_auto_add_pu_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_NEXT_PUNO,
        nmp_mod_bss_get_next_puno_f,
        nmp_mod_bss_get_next_puno_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_INIT_NAME,
        nmp_mod_bss_get_initiator_name_f,
        nmp_mod_bss_get_initiator_name_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_INIT_NAME,
        nmp_mod_bss_set_initiator_name_f,
        nmp_mod_bss_set_initiator_name_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_IPSAN_INFO,
        nmp_mod_bss_get_ipsan_info_f,
        nmp_mod_bss_get_ipsan_info_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_ONE_IPSAN,
        nmp_mod_bss_add_one_ipsan_f,
        nmp_mod_bss_add_one_ipsan_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DELETE_ONE_IPSAN,
        nmp_mod_bss_delete_one_ipsan_f,
        nmp_mod_bss_delete_one_ipsan_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_ONE_IPSAN,
        nmp_mod_bss_get_one_ipsan_detail_f,
        nmp_mod_bss_get_one_ipsan_detail_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_LOG,
        nmp_mod_bss_query_log_f,
        nmp_mod_bss_query_log_b,
        0
    );

#if ONLINE_RATE_FLAG
    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_AREA_DEV_ONLINE_RATE,
        nmp_mod_bss_query_area_dev_rate_f,
        nmp_mod_bss_query_area_dev_rate_b,
        0
    );
#endif

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_VALIDATA_GU_MAP,
        nmp_mod_bss_validate_gu_map_f,
        nmp_mod_bss_validate_gu_map_b,
        0
    );

	nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_IVS,
        nmp_mod_bss_add_ivs_f,
        nmp_mod_bss_add_ivs_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_IVS,
        nmp_mod_bss_modify_ivs_f,
        nmp_mod_bss_modify_ivs_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_IVS,
        nmp_mod_bss_del_ivs_f,
        nmp_mod_bss_del_ivs_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_IVS,
        nmp_mod_bss_query_ivs_f,
        nmp_mod_bss_query_ivs_b,
        0
    );

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_GET_SERVER_FLAG,
		nmp_mod_bss_get_server_flag_f,
		NULL,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_GET_MDS_CONFIG,
		nmp_mod_bss_get_mds_config_f,
		NULL,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_SET_MDS_CONFIG,
		nmp_mod_bss_set_mds_config_f,
		NULL,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_GET_MDS_STATE,
		nmp_mod_bss_get_mds_state_f,
		NULL,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_GET_MSS_CONFIG,
		nmp_mod_bss_get_mss_config_f,
		NULL,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_SET_MSS_CONFIG,
		nmp_mod_bss_set_mss_config_f,
		NULL,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_GET_MSS_STATE,
		nmp_mod_bss_get_mss_state_f,
		NULL,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_GET_IVS_CONFIG,
		nmp_mod_bss_get_ivs_config_f,
		NULL,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_SET_IVS_CONFIG,
		nmp_mod_bss_set_ivs_config_f,
		NULL,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_GET_IVS_STATE,
		nmp_mod_bss_get_ivs_state_f,
		NULL,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_CMS_SHUTDOWN,
		nmp_mod_bss_cms_shutdown_f,
		NULL,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_CMS_REBOOT,
		nmp_mod_bss_cms_reboot_f,
		NULL,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_ADD_AMS,
		nmp_mod_bss_add_ams_f,
		nmp_mod_bss_add_ams_b,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_MODIFY_AMS,
		nmp_mod_bss_modify_ams_f,
		nmp_mod_bss_modify_ams_b,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_DEL_AMS,
		nmp_mod_bss_del_ams_f,
		nmp_mod_bss_del_ams_b,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_QUERY_AMS,
		nmp_mod_bss_query_ams_f,
		nmp_mod_bss_query_ams_b,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_QUERY_AMS_PU,
		nmp_mod_bss_query_ams_pu_f,
		nmp_mod_bss_query_ams_pu_b,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_MODIFY_AMS_PU,
		nmp_mod_bss_modify_ams_pu_f,
		nmp_mod_bss_modify_ams_pu_b,
		0
	);
}

