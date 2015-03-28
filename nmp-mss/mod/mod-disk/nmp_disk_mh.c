#include "nmp_mod_disk.h"
#include "nmp_mods.h"
#include "nmp_debug.h"
#include "nmp_errno.h"
#include "nmp_memory.h"
#include "nmp_message.h"
#include "nmp_sysctl.h"
#include "nmp_ipsan.h"
#include "nmp_share_struct.h"


#define STRING_LEN				512
#define CONNECT_INTERVAL		10

#define POPEN_OPERATE_BASE	"system_ipsan_config"
//#define POPEN_OPERATE_BASE	"/home/zyt/system_ipsan_config"
#define POPEN_CONNECT_IPSAN		POPEN_OPERATE_BASE" connect_ipsan"
#define POPEN_DISCONNECT_IPSAN	POPEN_OPERATE_BASE" disconnect_ipsan"
#define POPEN_GET_INITIATOR_NAME	POPEN_OPERATE_BASE" get_initiator_name"
#define POPEN_SET_INITIATOR_NAME	POPEN_OPERATE_BASE" set_initiator_name"
#define POPEN_IF_SUPPORT_ISCSI		POPEN_OPERATE_BASE" if_support_iscsi"
#define POPEN_GET_ISCSI_IP			POPEN_OPERATE_BASE" get_iscsi_ip"
#define POPEN_GET_ISCSI_INFO		POPEN_OPERATE_BASE" get_iscsi_info"

#define MAX_IPSAN_TARGET_LEN		64
#define MAX_IPSAN_IP_COUNT			4
typedef struct
{
	gint enabled;
	gchar target[MAX_IPSAN_TARGET_LEN];
	gint ip_count;
	gchar ip[MAX_IPSAN_IP_COUNT][MAX_IP_LEN];
} ipsan_msg;

static ipsan_msg g_ipsan_msg[MAX_IPSAN_NUM];


unsigned int ip_to_uint(const char *ip)
{
	unsigned int result = 0;
	int nshift = 24;
	int tmp = 0;
	const char *pstart = ip;
	const char *pend = ip;

	while (*pend != '\0')
	{
		while (*pend != '.' && *pend != '\0')
			pend++;
		tmp = 0;

		for (; pstart != pend; pstart++)
		{
			tmp = tmp * 10 + *pstart - '0';
		}

		result += tmp << nshift;
		nshift -= 8;

		if (*pend == '\0')
			break;
		pstart = pend + 1;
		pend++;
		
		if (nshift < 0)
			return -1;
	}

	return result;
}

void uint_to_ip(int num, char *ip)
{
	sprintf(ip, "%u.%u.%u.%u", 
		(unsigned char)(num >> 24),
		(unsigned char)((num % (1 << 24)) >> 16),
		(unsigned char)((num % (1 << 16)) >> 8),
		(unsigned char)(num % (1 << 8)));
}

static void nmp_remove_iscsi_nodes()
{
	system("rm /var/lib/iscsi/nodes/ -rf");
	system("rm /var/lib/iscsi/send_targets/ -rf");
}


NmpMsgFunRet 
nmp_mod_disk_query_idles_hd_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModDisk *self = (NmpModDisk*)app_obj;
	NmpQueryAllHd *req_info;
	NmpQueryAllHdRes *res_info;
	NmpDiskGrp *idles;
	gint cur, size, disk_count = 0;

	G_ASSERT(app_obj != NULL && msg != NULL);
	if (!self->storage)
	{
		size = sizeof(NmpQueryAllHdRes);
		res_info = nmp_mem_kalloc(size);
		if (G_LIKELY(res_info))
		{
			req_info = MSG_GET_DATA(msg);
			req_info->session[USER_NAME_LEN - 1] = 0;
			strcpy(res_info->session, req_info->session);

			SET_CODE(res_info, E_DISKNOPREPARE);
			nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
		}
		else
			nmp_sysmsg_set_private(msg, 0, 0, NULL);
		
		goto end;
	}

	idles = nmp_get_idle_disks(self->storage);
	if (idles)
	{
		disk_count = idles->disk_count;
		disk_count = disk_count <= 0 ? 0 : disk_count;
	}

	size = sizeof(NmpQueryAllHdRes) + (disk_count * sizeof(NmpHdInfo));
	res_info = nmp_mem_kalloc(size);
	if (G_LIKELY(res_info))
	{
		req_info = MSG_GET_DATA(msg);
		req_info->session[USER_NAME_LEN-1] = 0;
		strcpy(res_info->session, req_info->session);
		res_info->total_num = disk_count;

		for (cur = 0; cur < disk_count; ++cur)
		{
			res_info->hd_info[cur].hd_name[HD_NAME_LEN - 1] = 0;
			strncpy(res_info->hd_info[cur].hd_name, idles->disks[cur].label, HD_NAME_LEN - 1);
			res_info->hd_info[cur].hd_id = idles->disks[cur].disk_no;
			res_info->hd_info[cur].total_size = idles->disks[cur].total;
			res_info->hd_info[cur].usedSize = idles->disks[cur].used;
			res_info->hd_info[cur].hd_status = idles->disks[cur].status;
			res_info->hd_info[cur].fs_type = idles->disks[cur].fs_type;
		}

		SET_CODE(res_info, 0);
		nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
	}
	else
	{
		nmp_sysmsg_set_private(msg, 0, 0, NULL);
	}

	if (idles)
	{
		nmp_put_idle_disks(idles);
	}
	
end:
	MSG_SET_RESPONSE(msg);
	MSG_SET_DSTPOS(msg, BUSSLOT_POS_CMS);
	return MFR_DELIVER_BACK;
}


NmpMsgFunRet 
nmp_mod_disk_add_hdgroup_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModDisk *self = (NmpModDisk*)app_obj;
	NmpAddHdGroup *req_info;
	NmpAddHdGroupRes *res_info;
	gint ret;
	gint size;

	G_ASSERT(app_obj != NULL && msg != NULL);
	if (!self->storage)
	{
		size = sizeof(NmpAddHdGroupRes);
		res_info = nmp_mem_kalloc(size);
		if (G_LIKELY(res_info))
		{
			req_info = MSG_GET_DATA(msg);
			req_info->session[USER_NAME_LEN - 1] = 0;
			strcpy(res_info->session, req_info->session);

			SET_CODE(res_info, E_DISKNOPREPARE);
			nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
		}
		else
			nmp_sysmsg_set_private(msg, 0, 0, NULL);
		
		goto end;
	}

	req_info = MSG_GET_DATA(msg);

	res_info = 	nmp_mem_kalloc(sizeof(*res_info));
	if (G_LIKELY(res_info))
	{
		ret = nmp_create_grp(self->storage, req_info->hd_group_name);
		req_info->session[USER_NAME_LEN-1] = 0;
		strcpy(res_info->session, req_info->session);
		req_info->mss_id[MSS_ID_LEN-1] = 0;
		strcpy(res_info->mss_id, req_info->mss_id);
		res_info->hd_group_id = ret;
		SET_CODE(res_info, ret >= 0 ? 0 : ret);
		nmp_sysmsg_set_private(msg, res_info, sizeof(*res_info), nmp_mem_kfree);
	}
	else
	{
		nmp_sysmsg_set_private(msg, 0, 0, NULL);
	}

end:
	MSG_SET_RESPONSE(msg);
	MSG_SET_DSTPOS(msg, BUSSLOT_POS_CMS);
	return MFR_DELIVER_BACK;
}


NmpMsgFunRet 
nmp_mod_disk_query_hdgroups_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModDisk *self = (NmpModDisk*)app_obj;
	NmpQueryAllHdGroup *req_info;
	NmpQueryAllHdGroupRes *res_info;
	NmpDiskGrp *grps;

	gint cur, size, grp_count = 0;

	G_ASSERT(app_obj != NULL && msg != NULL);
	if (!self->storage)
	{
		size = sizeof(NmpQueryAllHdGroupRes);
		res_info = nmp_mem_kalloc(size);
		if (G_LIKELY(res_info))
		{
			req_info = MSG_GET_DATA(msg);
			req_info->session[USER_NAME_LEN - 1] = 0;
			strcpy(res_info->session, req_info->session);

			SET_CODE(res_info, E_DISKNOPREPARE);
			nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
		}
		else
			nmp_sysmsg_set_private(msg, 0, 0, NULL);

		goto end;
	}

	grps = nmp_get_disk_grps(self->storage, &grp_count);
	size = sizeof(*res_info) + (grp_count * sizeof(NmpHdGroupInfo));

	res_info = nmp_mem_kalloc(size);
	if (G_LIKELY(res_info))
	{
		req_info = MSG_GET_DATA(msg);
		req_info->session[USER_NAME_LEN-1] = 0;
		strcpy(res_info->session, req_info->session);
		res_info->total_num = grp_count;

		for (cur = 0; cur < grp_count; ++cur)
		{
			res_info->group_info[cur].hd_group_id = grps[cur].grp_no;
			res_info->group_info[cur].hd_group_name[GROUP_NAME_LEN - 1] = 0;
			strncpy(res_info->group_info[cur].hd_group_name, grps[cur].grp_label, GROUP_NAME_LEN - 1);
		}

		SET_CODE(res_info, 0);
		nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
	}
	else
	{
		nmp_sysmsg_set_private(msg, 0, 0, NULL);
	}

	if (grps)
	{
		nmp_put_disk_grps(grps, grp_count);
	}	

end:
	MSG_SET_RESPONSE(msg);
	MSG_SET_DSTPOS(msg, BUSSLOT_POS_CMS);
	return MFR_DELIVER_BACK;
}


NmpMsgFunRet 
nmp_mod_disk_query_hdgroup_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModDisk *self = (NmpModDisk*)app_obj;
	NmpQueryHdGroupInfo *req_info;
	NmpQueryHdGroupInfoRes *res_info;
	NmpDiskGrp *grp;
	gint err, cur, size, disk_count = 0;

	G_ASSERT(app_obj != NULL && msg != NULL);
	if (!self->storage)
	{
		size = sizeof(NmpQueryHdGroupInfoRes);
		res_info = nmp_mem_kalloc(size);
		if (G_LIKELY(res_info))
		{
			req_info = MSG_GET_DATA(msg);
			req_info->session[USER_NAME_LEN - 1] = 0;
			strcpy(res_info->session, req_info->session);

			SET_CODE(res_info, E_DISKNOPREPARE);
			nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
		}
		else
			nmp_sysmsg_set_private(msg, 0, 0, NULL);

		goto end;
	}

	req_info = MSG_GET_DATA(msg);
	grp = nmp_get_grp_info(self->storage, req_info->hd_group_id);
	if (grp)
	{
		err = 0;
		disk_count = grp->disk_count;
	}
	else
	{
		err = -ENOENT;
	}

	size = sizeof(*res_info) + (disk_count * sizeof(NmpHdInfo));

	res_info = nmp_mem_kalloc(size);
	if (G_LIKELY(res_info))
	{
		req_info->session[USER_NAME_LEN-1] = 0;
		strcpy(res_info->session, req_info->session);
		res_info->total_num = disk_count;

		for (cur = 0; cur < disk_count; ++cur)
		{
			res_info->hd_info[cur].hd_name[HD_NAME_LEN - 1] = 0;
			strncpy(res_info->hd_info[cur].hd_name, grp->disks[cur].label, HD_NAME_LEN - 1);
			res_info->hd_info[cur].hd_id = grp->disks[cur].disk_no;
			res_info->hd_info[cur].total_size = grp->disks[cur].total;
			res_info->hd_info[cur].usedSize = grp->disks[cur].used;
			res_info->hd_info[cur].hd_status = grp->disks[cur].status;
			res_info->hd_info[cur].fs_type = grp->disks[cur].fs_type;
		}

		SET_CODE(res_info, err);
		nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
	}
	else
	{
		nmp_sysmsg_set_private(msg, 0, 0, NULL);
	}

	if (grp)
	{
		nmp_put_grp_info(grp);
	}	

end:
	MSG_SET_RESPONSE(msg);
	MSG_SET_DSTPOS(msg, BUSSLOT_POS_CMS);
	return MFR_DELIVER_BACK;
}


NmpMsgFunRet 
nmp_mod_disk_add_group_disk_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModDisk *self = (NmpModDisk*)app_obj;
	NmpAddHdToGroup *req_info;
	NmpAddHdToGroupRes *res_info;
	gint ret;
	gint size;

	G_ASSERT(app_obj != NULL && msg != NULL);
	if (!self->storage)
	{
		size = sizeof(NmpAddHdToGroupRes);
		res_info = nmp_mem_kalloc(size);
		if (G_LIKELY(res_info))
		{
			req_info = MSG_GET_DATA(msg);
			req_info->session[USER_NAME_LEN - 1] = 0;
			strcpy(res_info->session, req_info->session);

			SET_CODE(res_info, E_DISKNOPREPARE);
			nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
		}
		else
			nmp_sysmsg_set_private(msg, 0, 0, NULL);

		goto end;
	}

	req_info = MSG_GET_DATA(msg);

	res_info = 	nmp_mem_kalloc(sizeof(*res_info));
	if (G_LIKELY(res_info))
	{
		ret = nmp_grp_add_disk(self->storage, req_info->hd_group_id, req_info->hd_id);
		req_info->session[USER_NAME_LEN-1] = 0;
		strcpy(res_info->session, req_info->session);
		SET_CODE(res_info, ret);
		nmp_sysmsg_set_private(msg, res_info, sizeof(*res_info), nmp_mem_kfree);
	}
	else
	{
		nmp_sysmsg_set_private(msg, 0, 0, NULL);
	}

end:
	MSG_SET_RESPONSE(msg);
	MSG_SET_DSTPOS(msg, BUSSLOT_POS_CMS);
	return MFR_DELIVER_BACK;
}


NmpMsgFunRet 
nmp_mod_disk_del_group_disk_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModDisk *self = (NmpModDisk*)app_obj;
	NmpDelHdFromGroup *req_info;
	NmpDelHdFromGroupRes *res_info;
	gint ret;
	gint size;

	G_ASSERT(app_obj != NULL && msg != NULL);
	if (!self->storage)
	{
		size = sizeof(NmpDelHdFromGroupRes);
		res_info = nmp_mem_kalloc(size);
		if (G_LIKELY(res_info))
		{
			req_info = MSG_GET_DATA(msg);
			req_info->session[USER_NAME_LEN - 1] = 0;
			strcpy(res_info->session, req_info->session);

			SET_CODE(res_info, E_DISKNOPREPARE);
			nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
		}
		else
			nmp_sysmsg_set_private(msg, 0, 0, NULL);

		goto end;
	}

	req_info = MSG_GET_DATA(msg);

	res_info = 	nmp_mem_kalloc(sizeof(*res_info));
	if (G_LIKELY(res_info))
	{
		ret = nmp_grp_del_disk(self->storage, req_info->hd_group_id, req_info->hd_id);
		req_info->session[USER_NAME_LEN-1] = 0;
		strcpy(res_info->session, req_info->session);
		SET_CODE(res_info, ret);
		nmp_sysmsg_set_private(msg, res_info, sizeof(*res_info), nmp_mem_kfree);
	}
	else
	{
		nmp_sysmsg_set_private(msg, 0, 0, NULL);
	}

end:
	MSG_SET_RESPONSE(msg);
	MSG_SET_DSTPOS(msg, BUSSLOT_POS_CMS);
	return MFR_DELIVER_BACK;
}


NmpMsgFunRet 
nmp_mod_disk_del_hdgroup_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModDisk *self = (NmpModDisk*)app_obj;
	NmpDelHdGroup *req_info;
	NmpDelHdGroupRes *res_info;
	gint ret;
	gint size;

	G_ASSERT(app_obj != NULL && msg != NULL);
	if (!self->storage)
	{
		size = sizeof(NmpDelHdGroupRes);
		res_info = nmp_mem_kalloc(size);
		if (G_LIKELY(res_info))
		{
			req_info = MSG_GET_DATA(msg);
			req_info->session[USER_NAME_LEN - 1] = 0;
			strcpy(res_info->session, req_info->session);

			SET_CODE(res_info, E_DISKNOPREPARE);
			nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
		}
		else
			nmp_sysmsg_set_private(msg, 0, 0, NULL);

		goto end;
	}

	req_info = MSG_GET_DATA(msg);

	res_info = 	nmp_mem_kalloc(sizeof(*res_info));
	if (G_LIKELY(res_info))
	{
		ret = nmp_delete_grp(self->storage, req_info->hd_group_id);
		req_info->session[USER_NAME_LEN-1] = 0;
		strcpy(res_info->session, req_info->session);
		SET_CODE(res_info, ret);
		nmp_sysmsg_set_private(msg, res_info, sizeof(*res_info), nmp_mem_kfree);
	}
	else
	{
		nmp_sysmsg_set_private(msg, 0, 0, NULL);
	}

end:
	MSG_SET_RESPONSE(msg);
	MSG_SET_DSTPOS(msg, BUSSLOT_POS_CMS);
	return MFR_DELIVER_BACK;
}


NmpMsgFunRet 
nmp_mod_disk_get_hd_format_progress_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModDisk *self = (NmpModDisk*)app_obj;
	NmpGetHdFormatProgress *req_info;
	NmpGetHdFormatProgressRes *res_info;
	NmpDisk *disk = NULL;
	gint progress;
	gint size;

	G_ASSERT(app_obj != NULL && msg != NULL);
	if (!self->storage)
	{
		size = sizeof(NmpGetHdFormatProgressRes);
		res_info = nmp_mem_kalloc(size);
		if (G_LIKELY(res_info))
		{
			req_info = MSG_GET_DATA(msg);
			req_info->session[USER_NAME_LEN - 1] = 0;
			strcpy(res_info->session, req_info->session);

			SET_CODE(res_info, E_DISKNOPREPARE);
			nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
		}
		else
			nmp_sysmsg_set_private(msg, 0, 0, NULL);

		goto end;
	}

	req_info = MSG_GET_DATA(msg);

	res_info = 	nmp_mem_kalloc(sizeof(*res_info));
	if (G_LIKELY(res_info))
	{
		disk = nmp_get_format_disk(self->storage, &progress);

		req_info->session[USER_NAME_LEN-1] = 0;
		strcpy(res_info->session, req_info->session);

		if (disk)
		{
			SET_CODE(res_info, 0);
			res_info->hd_group_id = disk->group;
			strncpy(res_info->hd_name, disk->label, HD_NAME_LEN - 1);
			res_info->hd_name[HD_NAME_LEN - 1] = 0;
			res_info->hd_id = disk->disk_no;
    		res_info->percent = progress;

			nmp_put_disk(disk);
		}
		else
		{
			SET_CODE(res_info, -EINVAL);
		}

		nmp_sysmsg_set_private(msg, res_info, sizeof(*res_info), nmp_mem_kfree);
	}
	else
	{
		nmp_sysmsg_set_private(msg, 0, 0, NULL);
	}

end:
	MSG_SET_RESPONSE(msg);
	MSG_SET_DSTPOS(msg, BUSSLOT_POS_CMS);
	return MFR_DELIVER_BACK;
}


gint nmp_if_ipsan_is_starting(gchar *buf)
{
	char *p = strstr(buf, "Starting iscsid");
	return p ? 1 : 0;
}

gint nmp_if_ipsan_connect_success(gchar *buf, gchar *ip)
{
	char *p = strstr(buf, ip);
	return p == buf ? 1 : 0;
}

gint nmp_standardize_ip(gchar *ip)
{
	unsigned int tmp = 0;
	
	tmp = ip_to_uint(ip);
	if (tmp == -1)
	{
		nmp_warning("<NmpModDisk> ip standardize error, ip:%s", ip);
		return -1;
	}
	uint_to_ip(tmp, ip);

	return 0;
}


static gint nmp_connect_one_ipsan(gchar *ip, gint port)
{
	FILE *fp;
	gchar query_buf[STRING_LEN] = {0};
	gchar buffer[STRING_LEN] = {0};
	gint ret = -1;
	
	snprintf(query_buf, STRING_LEN - 1, "%s %s %d", POPEN_CONNECT_IPSAN, 
		ip, port);
	fp = popen(query_buf, "r");
	if (!fp)
	{
		nmp_warning("<NmpModDisk> connect ipsan, popen failed");
		ret = -errno;
		return ret;
	}

	while (fgets(buffer, sizeof(buffer), fp) != NULL)
	{
		if (buffer[strlen(buffer) - 1] == '\n' || buffer[strlen(buffer) - 1] == '\r')
			buffer[strlen(buffer) - 1] = '\0';

		if (nmp_if_ipsan_is_starting(buffer))
		{
			nmp_print("<NmpModDisk> ipsan is starting...");
			continue;
		}
		else if (nmp_if_ipsan_connect_success(buffer, ip))
		{
			ret = 0;
			nmp_print("<NmpModDisk> connect ipsan success, ip,%s:%d", ip, port);
		}
		else
		{
			ret = -1;
			nmp_warning("<NmpModDisk> connect ipsan failed, buffer:%s", 
				buffer);
		}
		break;
	}
	pclose(fp);

	return ret;
}


static gint nmp_disconnect_one_ipsan(gchar *ip, gint port)
{
	FILE *fp;
	gchar query_buf[STRING_LEN] = {0};
	gint ret = 0;
	
	snprintf(query_buf, STRING_LEN - 1, "%s %s %d", POPEN_DISCONNECT_IPSAN, 
		ip, port);
	fp = popen(query_buf, "r");
	if (!fp)
	{
		nmp_warning("<NmpModDisk> disconnect ipsan, popen failed");
		ret = -errno;
		return ret;
	}
	pclose(fp);

	return ret;
}


NmpMsgFunRet 
nmp_mod_add_one_ipsan_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpMssAddOneIpsan *req_info;
	NmpMssAddOneIpsanRes res_info;
	gint ret = 0;
	memset(&res_info, 0, sizeof(res_info));

	G_ASSERT(app_obj != NULL && msg != NULL);

	req_info = MSG_GET_DATA(msg);
	if (nmp_standardize_ip(req_info->info.ip) < 0)
	{
		return -2;
	}

	if ((ret = nmp_connect_one_ipsan(req_info->info.ip, req_info->info.port)) 
		!= 0)
	{
		nmp_warning("<NmpModDisk> nmp_connect_one_ipsan failed, ret = %d", ret);
		goto end;
	}

	ret = nmp_add_one_ipsan(&req_info->info);
	if (ret < 0)
		nmp_warning("<NmpModDisk> nmp_add_one_ipsan failed, ret = %d", ret);

end:
	SET_CODE(&res_info, -ret);
	strncpy(res_info.session, req_info->session, USER_NAME_LEN - 1);
	nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
	MSG_SET_DSTPOS(msg, BUSSLOT_POS_CMS);
	
	return MFR_DELIVER_BACK;
}


NmpMsgFunRet 
nmp_mod_get_ipsans_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpMssGetIpsans *req_info;
	NmpMssGetIpsansRes res_info;
	gint ret = 0;
	memset(&res_info, 0, sizeof(res_info));

	G_ASSERT(app_obj != NULL && msg != NULL);

	req_info = MSG_GET_DATA(msg);

	ret = nmp_get_ipsan_info(&res_info.info);
	if (ret < 0)
		nmp_warning("<NmpModDisk> nmp_get_ipsan_info failed, ret = %d", ret);

	SET_CODE(&res_info, -ret);
	strncpy(res_info.session, req_info->session, USER_NAME_LEN - 1);
	nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
	MSG_SET_DSTPOS(msg, BUSSLOT_POS_CMS);
	
	return MFR_DELIVER_BACK;
}


NmpMsgFunRet 
nmp_mod_set_ipsans_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpMssSetIpsans *req_info;
	NmpMssSetIpsansRes res_info;
	gint ret = 0;
	memset(&res_info, 0, sizeof(res_info));

	G_ASSERT(app_obj != NULL && msg != NULL);

	req_info = MSG_GET_DATA(msg);

	ret = nmp_set_ipsan_info(&req_info->info);
	if (ret < 0)
		nmp_warning("<NmpModDisk> nmp_set_ipsan_info failed, ret = %d", ret);

	SET_CODE(&res_info, -ret);
	strncpy(res_info.session, req_info->session, USER_NAME_LEN - 1);
	nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
	MSG_SET_DSTPOS(msg, BUSSLOT_POS_CMS);
	
	return MFR_DELIVER_BACK;
}


static gint nmp_check_support_iscsi()
{
	FILE *fp;
	gchar query_buf[STRING_LEN] = {0};
	gchar buffer[STRING_LEN] = {0};
	gint ret = -1;
	
	snprintf(query_buf, STRING_LEN - 1, "%s", POPEN_IF_SUPPORT_ISCSI);
	fp = popen(query_buf, "r");
	if (!fp)
	{
		nmp_warning("<NmpModDisk> popen failed");
		ret = -errno;
		return ret;
	}

	while (fgets(buffer, sizeof(buffer), fp) != NULL)
	{
		if (buffer[strlen(buffer) - 1] == '\n' || buffer[strlen(buffer) - 1] == '\r')
			buffer[strlen(buffer) - 1] = '\0';

		ret = atoi(buffer);
		break;
	}
	pclose(fp);
	
	return ret;
}


NmpMsgFunRet 
nmp_mod_get_initiator_name_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpMssGetInitiatorName *req_info;
	NmpMssGetInitiatorNameRes res_info;
	gint ret = 0;
	FILE *fp;
	gchar query_buf[STRING_LEN] = {0};
	gchar buffer[STRING_LEN] = {0};
	memset(&res_info, 0, sizeof(res_info));

	G_ASSERT(app_obj != NULL && msg != NULL);

	req_info = MSG_GET_DATA(msg);
	ret = nmp_check_support_iscsi();
	if (ret != 0)
	{
		nmp_warning("<NmpModDisk> not support iscsi protocal, ret = %d!", ret);
		ret = -E_NOISCSI;
		goto end;
	}

	snprintf(query_buf, STRING_LEN - 1, "%s", POPEN_GET_INITIATOR_NAME);
	fp = popen(query_buf, "r");
	if (!fp)
	{
		nmp_warning("<NmpModDisk> get initiator name, popen failed");
		ret = -errno;
		goto end;
	}

	while (fgets(buffer, sizeof(buffer), fp) != NULL)
	{
		if (buffer[strlen(buffer) - 1] == '\n' || buffer[strlen(buffer) - 1] == '\r')
			buffer[strlen(buffer) - 1] = '\0';
		
		strncpy(res_info.name, buffer, MAX_INITIATOR_NAME_LEN - 1);
		break;
	}
	pclose(fp);

end:
	SET_CODE(&res_info, -ret);
	strncpy(res_info.session, req_info->session, USER_NAME_LEN - 1);
	nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
	MSG_SET_DSTPOS(msg, BUSSLOT_POS_CMS);

	return MFR_DELIVER_BACK;
}

/*
static void nmp_will_reboot()
{
	system("sleep 1 && reboot &");
};
*/

NmpMsgFunRet 
nmp_mod_set_initiator_name_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpMssSetInitiatorName *req_info;
	NmpMssSetInitiatorNameRes res_info;
	gint ret = 0;
	FILE *fp;
	gchar query_buf[STRING_LEN] = {0};
	memset(&res_info, 0, sizeof(res_info));

	G_ASSERT(app_obj != NULL && msg != NULL);

	req_info = MSG_GET_DATA(msg);

	snprintf(query_buf, STRING_LEN - 1, "%s %s", POPEN_SET_INITIATOR_NAME, 
		req_info->name);
	fp = popen(query_buf, "r");
	if (!fp)
	{
		nmp_warning("<NmpModDisk> set initiator name, popen failed");
		ret = -errno;
		goto end;
	}
	pclose(fp);

end:
	SET_CODE(&res_info, -ret);
	strncpy(res_info.session, req_info->session, USER_NAME_LEN - 1);
	nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
	MSG_SET_DSTPOS(msg, BUSSLOT_POS_CMS);
	//if (ret == 0)
	//	nmp_will_reboot();

	return MFR_DELIVER_BACK;
}


NmpMsgFunRet 
nmp_mod_get_one_ipsan_detail_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpMssGetOneIpsanDetail *req_info;
	NmpMssGetOneIpsanDetailRes res_info;
	gint ret = -1;
	FILE *fp;
	gchar query_buf[STRING_LEN] = {0};
	gchar buffer[STRING_LEN] = {0};
	memset(&res_info, 0, sizeof(res_info));

	G_ASSERT(app_obj != NULL && msg != NULL);

	req_info = MSG_GET_DATA(msg);

	snprintf(query_buf, STRING_LEN - 1, "%s %s %d", POPEN_CONNECT_IPSAN, 
		req_info->info.ip, req_info->info.port);
	fp = popen(query_buf, "r");
	if (!fp)
	{
		nmp_warning("<NmpModDisk> connect ipsan, popen failed");
		ret = -errno;
		goto end;
	}

	while (fgets(buffer, sizeof(buffer), fp) != NULL)
	{
		if (buffer[strlen(buffer) - 1] == '\n' || buffer[strlen(buffer) - 1] == '\r')
			buffer[strlen(buffer) - 1] = '\0';

		if (nmp_if_ipsan_is_starting(buffer))
		{
			nmp_print("<NmpModDisk> ipsan is starting...");
			continue;
		}
		else if (nmp_if_ipsan_connect_success(buffer, req_info->info.ip))
		{
			ret = 0;
			res_info.connect_state = 0;
			const char *target_p = strchr(buffer, ' ');
			if (target_p)
				strncpy(res_info.target_name, target_p + 1, MAX_TARGET_NAME_LEN - 1);
			nmp_print("<NmpModDisk> connect ipsan success, ip,%s:%d", 
				req_info->info.ip, req_info->info.port);
		}
		else
		{
			ret = -1;
			res_info.connect_state = -1;
			nmp_warning("<NmpModDisk> connect ipsan failed, buffer:%s", 
				buffer);
		}
		break;
	}
	pclose(fp);
	
end:
	SET_CODE(&res_info, -ret);
	strncpy(res_info.session, req_info->session, USER_NAME_LEN - 1);
	nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
	MSG_SET_DSTPOS(msg, BUSSLOT_POS_CMS);

	return MFR_DELIVER_BACK;
}


NmpMsgFunRet 
nmp_mod_del_one_ipsan_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpMssDelOneIpsan *req_info;
	NmpMssDelOneIpsanRes res_info;
	gint ret = 0;
	memset(&res_info, 0, sizeof(res_info));

	G_ASSERT(app_obj != NULL && msg != NULL);

	req_info = MSG_GET_DATA(msg);

	nmp_del_one_ipsan(&req_info->info);
	nmp_disconnect_one_ipsan(req_info->info.ip, req_info->info.port);
	nmp_remove_iscsi_nodes();

	SET_CODE(&res_info, -ret);
	strncpy(res_info.session, req_info->session, USER_NAME_LEN - 1);
	nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
	MSG_SET_DSTPOS(msg, BUSSLOT_POS_CMS);
	
	return MFR_DELIVER_BACK;
}


static void nmp_deal_connect_result(NmpModDisk *self, NmpIpsanIp *ipsan_ip, 
	MSS_NOTIFY_TYPE type)
{
	NmpSysMsg *msg;
	NmpNotifyMessage info;
	memset(&info, 0, sizeof(NmpNotifyMessage));

	info.msg_id = type;
	if (ipsan_ip)
		snprintf(info.param1, GENERAL_MSG_PARM_LEN, "%s:%d", ipsan_ip->ip, 
			ipsan_ip->port);

	memcpy(info.param2, g_mss_name, MSS_NAME_LEN);
	
	msg = nmp_sysmsg_new_2(MESSAGE_NOTIFY_MESSAGE, &info, 
		sizeof(NmpNotifyMessage), 0);
	if (G_UNLIKELY(!msg))
	{
		nmp_warning("nmp_sysmsg_new_2 failed!");
		return ;
	}

	MSG_SET_DSTPOS(msg, BUSSLOT_POS_CMS);
	nmp_app_obj_deliver_out((NmpAppObj *)self, msg);
}


static void
nmp_connect_ipsans(NmpModDisk *self)
{
	NmpIpsanInfo info;
	gint ret = 0, i;

re_check:
	memset(&info, 0, sizeof(NmpIpsanInfo));
	ret = nmp_get_ipsan_info(&info);
	if (ret < 0)
	{
		nmp_warning("<NmpModDisk> nmp_get_ipsan_info failed, ret = %d", ret);
		return ;
	}
	if (info.total <= 0 || info.total >= MAX_IPSAN_NUM)
	{
		nmp_warning("<NmpModDisk>get ipsan info result maybe error, total = %d", 
			info.total);
		return ;
	}

	for (i = 0; i < info.total; i++)
	{
		while (1)
		{
			NmpIpsanIp *ipsan_ip = &info.ips[i];
			ret = nmp_connect_one_ipsan(ipsan_ip->ip, ipsan_ip->port);
			if (ret == 0)
			{
				nmp_print("<NmpModDisk> nmp_connect_one_ipsan success, ip,%s:%d", 
					ipsan_ip->ip, ipsan_ip->port);
				break;
			}
			
			nmp_warning("<NmpModDisk> nmp_connect_one_ipsan failed, ret = %d", ret);

			nmp_deal_connect_result(self, ipsan_ip, MSS_IPSAN_DISCONNECTED);
			sleep(CONNECT_INTERVAL);
			goto re_check;
		}
	}

	nmp_deal_connect_result(self, NULL, MSS_IPSAN_CONNECTED);
}


static gint nmp_stor_ipsan_msg(gchar *buffer)
{
	gchar target[MAX_IPSAN_TARGET_LEN] = {0};
	gchar ip[MAX_IP_LEN] = {0};
	gchar *target_p;
	gchar *ip_end;
	gint i;

	target_p = strchr(buffer, ' ');
	if (target_p)
		target_p++;
	else
		return -1;
	strncpy(target, target_p, MAX_IPSAN_TARGET_LEN - 1);

	strncpy(ip, buffer, MAX_IP_LEN - 1);
	ip_end = strchr(ip, ':');
	if (ip_end)
		*ip_end = '\0';
	else
		return -2;

	for (i = 0; i < MAX_IPSAN_NUM; i++)
	{
		ipsan_msg *p = &g_ipsan_msg[i];
		if (p->enabled)
		{
			if (strcmp(p->target, target) == 0)
			{
				if (p->ip_count < MAX_IPSAN_IP_COUNT)
				{
					strncpy(p->ip[p->ip_count], ip, MAX_IP_LEN - 1);
					p->ip_count++;
				}
				break;
			}
		}
		else
		{
			strncpy(p->target, target, MAX_IPSAN_TARGET_LEN - 1);
			strncpy(p->ip[p->ip_count], ip, MAX_IP_LEN - 1);
			p->ip_count++;
			p->enabled = 1;
			break;
		}
	}
	return 0;
}


static void nmp_mod_iscsi_ip_check2(gchar *ip)
{
	gint i, j;

	for (i = 0; i < MAX_IPSAN_NUM; i++)
	{
		ipsan_msg *p = &g_ipsan_msg[i];
		if (!p->enabled)
			continue;
		
		for (j = 0; j < p->ip_count; j++)
		{
			if (strcmp(p->ip[j], ip) == 0)
			{
				p->enabled = 0;
				break;
			}
		}
	}
}

static gint nmp_mod_reboot_for_ipsan_node()
{
	gint i;
	gint to_reboot = 0;
	gchar query_buf[STRING_LEN];

	for (i = 0; i < MAX_IPSAN_NUM; i++)
	{
		ipsan_msg *p = &g_ipsan_msg[i];
		if (!p->enabled)
			continue;

		nmp_warning("<NmpDiskMh> target:%s, ip:%s not in mss_ipsan.conf!", 
			p->target, p->ip[0]);
		
		snprintf(query_buf, STRING_LEN - 1, "iscsiadm -m node -o delete -T %s", 
			p->target);
		system(query_buf);

		to_reboot = 1;
	}

	return to_reboot;
}


void nmp_mod_iscsi_ip_check()
{
	NmpIpsanInfo info;
	gint ret, i;
	FILE *fp;
	gchar query_buf[STRING_LEN] = {0};
	gchar buffer[STRING_LEN] = {0};
	memset(&info, 0, sizeof(NmpIpsanInfo));
	
	ret = nmp_get_ipsan_info(&info);
	if (ret < 0)
	{
		nmp_warning("<NmpModDisk> nmp_get_ipsan_info failed, ret = %d", ret);
		return ;
	}

	snprintf(query_buf, STRING_LEN - 1, "%s", POPEN_GET_ISCSI_INFO);
	fp = popen(query_buf, "r");
	if (!fp)
	{
		nmp_warning("<NmpModDisk> get iscsi ip, popen failed");
		return ;
	}

	while (fgets(buffer, sizeof(buffer), fp) != NULL)
	{
		if (buffer[strlen(buffer) - 1] == '\n' || buffer[strlen(buffer) - 1] == '\r')
			buffer[strlen(buffer) - 1] = '\0';

		ret = nmp_stor_ipsan_msg(buffer);
		if (ret != 0)
		{
			nmp_warning("<NmpModDisk> nmp_stor_ipsan_msg failed, ret = %d, " \
				"buffer:%s", ret, buffer);
		}
	}
	pclose(fp);
	if (ret)
		return ;

	for (i = 0; i < info.total; i++)
	{
		nmp_mod_iscsi_ip_check2(info.ips[i].ip);
	}

	if (nmp_mod_reboot_for_ipsan_node())
	{
		nmp_remove_iscsi_nodes();

		nmp_warning("<NmpModDisk> reboot!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		system("reboot");
	}
}


NmpMsgFunRet 
nmp_mod_disk_registed_notify_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpRegistedNotifyMessage *req_info;
	NmpRegistedNotifyMessageRes res_info;
	NmpModDisk *self = (NmpModDisk*)app_obj;
	memset(&res_info, 0, sizeof(res_info));

	G_ASSERT(app_obj != NULL && msg != NULL);

	static gint atomic = 0;
	if (!g_atomic_int_compare_and_exchange(&atomic, 0, 1))
	{
		req_info = MSG_GET_DATA(msg);
		res_info.if_query_ids = req_info->if_query_ids;
		nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
		MSG_SET_DSTPOS(msg, BUSSLOT_POS_CMS);

		return MFR_DELIVER_BACK;
	}

	req_info = MSG_GET_DATA(msg);
	nmp_mod_iscsi_ip_check();
	nmp_connect_ipsans(self);

	nmp_print("before nmp_create_storage()");
	self->storage = nmp_create_storage("jfs-storage");
	if (!self->storage)
	{
		nmp_warning(
			"Create storage failed!"
		);
		FATAL_ERROR_EXIT;
	}

	nmp_print("after nmp_create_storage()");
	res_info.if_query_ids = req_info->if_query_ids;
	nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
	MSG_SET_DSTPOS(msg, BUSSLOT_POS_CMS | BUSSLOT_POS_POLICY);

	return MFR_DELIVER_BACK;
}


void
nmp_mod_disk_register_msg_handler(NmpModDisk *self)
{
    NmpAppMod *super_self = (NmpAppMod*)self;

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_IDLES_HD,
        NULL,
        nmp_mod_disk_query_idles_hd_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_HD_GROUP,
        NULL, 
        nmp_mod_disk_add_hdgroup_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_HD_GROUPS,
        NULL, 
        nmp_mod_disk_query_hdgroups_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_HD_GROUP_INFO,
        NULL, 
        nmp_mod_disk_query_hdgroup_info_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_GROUP_DISK,
        NULL, 
        nmp_mod_disk_add_group_disk_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_GROUP_DISK,
        NULL, 
        nmp_mod_disk_del_group_disk_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_HD_GROUP,
        NULL, 
        nmp_mod_disk_del_hdgroup_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_HDFMT_PROGRESS,
        NULL, 
        nmp_mod_disk_get_hd_format_progress_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_ONE_IPSAN,
        NULL, 
        nmp_mod_add_one_ipsan_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_IPSANS,
        NULL, 
        nmp_mod_get_ipsans_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_IPSANS,
        NULL, 
        nmp_mod_set_ipsans_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_INITIATOR_NAME,
        NULL, 
        nmp_mod_get_initiator_name_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_INITIATOR_NAME,
        NULL, 
        nmp_mod_set_initiator_name_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_ONE_IPSAN_DETAIL,
        NULL, 
        nmp_mod_get_one_ipsan_detail_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_ONE_IPSAN,
        NULL, 
        nmp_mod_del_one_ipsan_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_REGISTED_NOTIFY,
        NULL, 
        nmp_mod_disk_registed_notify_b,
        0
    );
}


//:~ End
