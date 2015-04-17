#include "nmp_cu_struct.h"
#include "nmp_internal_msg.h"
#include "nmp_sysmsg.h"
#include "nmp_mod_cu.h"
#include "nmp_share_errno.h"

//static guint msg_seq_generator = 0;

gint
nmp_mod_cu_get_user_info(NmpModCu *self, NmpUsr *user)
{
    NmpMsgUserInfo req_info;
    NmpMsgUserInfoRes *res_info;
    NmpSysMsg *msg;
    gint ret;

    memset(&req_info, 0, sizeof(req_info));
    strncpy(req_info.name, user->user_name, USER_NAME_LEN - 1);

    msg = nmp_sysmsg_new_2(MSG_GET_USER_INFO, &req_info,
    	sizeof(req_info), ++msg_seq_generator);
    if (G_UNLIKELY(!msg))
    	return -E_NOMEM;

    MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
    ret = nmp_app_mod_sync_request((NmpAppMod*)self, &msg);
    if (G_UNLIKELY(ret))	/* send failed */
    {
        nmp_warning(
        	"<NmpModCu> request user info failed!"
        );
        nmp_sysmsg_destroy(msg);
        return ret;
    }

    if (G_UNLIKELY(!msg))	/* sent, but no response */
    {
    	nmp_warning(
    		"<NmpModCu> request user info timeout!"
    	);
    	return -E_TIMEOUT;
    }

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = RES_CODE(res_info);
    if (ret)
    {
    	nmp_warning(
    		"<NmpModCu> request user info err:%d!", ret
    	);
    	nmp_sysmsg_destroy(msg);
    	return ret;
    }

    nmp_mod_cu_add_user_info(user, res_info->passwd,
    	res_info->group_id);
    nmp_sysmsg_destroy(msg);
    return 0;
}


gint
nmp_mod_cu_get_group_info(NmpModCu *self, NmpUsrGroup *grp)
{
    NmpMsgUserGroupInfo req_info;
    NmpMsgUserGroupInfoRes *res_info;
    NmpSysMsg *msg;
    gint ret;

    memset(&req_info, 0, sizeof(req_info));
    req_info.group_id = grp->id;

    msg = nmp_sysmsg_new_2(MSG_GET_USER_GROUP_INFO, &req_info,
    	sizeof(req_info), ++msg_seq_generator);
    if (G_UNLIKELY(!msg))
    	return -E_NOMEM;

    MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
    ret = nmp_app_mod_sync_request((NmpAppMod*)self, &msg);
    if (G_UNLIKELY(ret))	/* send failed */
    {
        nmp_warning(
    		"<NmpModCu> request group info failed!"
    	 );
    	 nmp_sysmsg_destroy(msg);

    	 return ret;
    }

    if (G_UNLIKELY(!msg))	/* sent, but no response */
    {
        nmp_warning(
        	"<NmpModCu> request group info timeout!"
        );

        return -E_TIMEOUT;
    }

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = RES_CODE(res_info);
    if (ret)
    {
        nmp_warning(
        	"<NmpModCu> request group info err:%d!", ret
        );
        nmp_sysmsg_destroy(msg);

        return ret;
    }

    nmp_mod_cu_add_group_info(grp, res_info->rank,
    	res_info->permission);

    nmp_sysmsg_destroy(msg);

    return 0;
}

gint
nmp_mod_cu_get_user_login_info(NmpModCu *self, NmpMsgUserLoginInfoRes *res)
{
	NmpMsgUserLoginInfoRes *res_info;
	NmpSysMsg *msg;
	gint ret;

	msg = nmp_sysmsg_new_2(MSG_GET_USER_LOGIN_INFO, NULL,
		0, ++msg_seq_generator);
	if (G_UNLIKELY(!msg))
		return -E_NOMEM;

	MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
	ret = nmp_app_mod_sync_request((NmpAppMod*)self, &msg);
	if (G_UNLIKELY(ret))	/* send failed */
	{
		nmp_warning(
			"<NmpModCu> request domain and root area info failed!"
		);
		nmp_sysmsg_destroy(msg);
		return ret;
	}

	if (G_UNLIKELY(!msg))	/* sent, but no response */
	{
		nmp_warning(
			"<NmpModCu> request domain and root area info timeout!"
		);
		return -E_TIMEOUT;
	}

	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = RES_CODE(res_info);
	if (ret)
	{
		nmp_warning(
			"<NmpModCu> request domain and root area info err:%d!", ret
		);
		nmp_sysmsg_destroy(msg);
		return ret;
	}

    memcpy(res, res_info,sizeof(NmpMsgUserLoginInfoRes));
    nmp_sysmsg_destroy(msg);

	return 0;
}


gint
nmp_mod_cu_get_mdu_ip(
    NmpModCu *self, NmpMsgGetMdsIp *req_info,
    NmpMsgGetMdsIpRes *getMduIpRes
)
{
    NmpMsgGetMdsIpRes *res_info;
    NmpSysMsg *msg;
    gint ret;

    msg = nmp_sysmsg_new_2(MSG_GET_MDS_IP, req_info,
    	sizeof(NmpMsgGetMdsIp), ++msg_seq_generator);
    if (G_UNLIKELY(!msg))
    	return -E_NOMEM;

    MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
    ret = nmp_app_mod_sync_request((NmpAppMod*)self, &msg);
    if (G_UNLIKELY(ret))	/* send failed */
    {
    	nmp_warning(
    		"<NmpModCu> request mdu ip failed!"
    	);
    	nmp_sysmsg_destroy(msg);
    	return ret;
    }

    if (G_UNLIKELY(!msg))	/* sent, but no response */
    {
    	nmp_warning(
    		"<NmpModCu> request mdu ip timeout!"
    	);
    	return -E_TIMEOUT;
    }

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = RES_CODE(res_info);
    if (ret)
    {
    	nmp_warning(
    		"<NmpModCu> request mdu ip err:%d!", ret
    	);
    	nmp_sysmsg_destroy(msg);
    	return ret;
    }

    memcpy(getMduIpRes, res_info,sizeof(NmpMsgGetMdsIpRes));
    nmp_sysmsg_destroy(msg);

    return 0;
}

//:~ End
