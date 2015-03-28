#include <stdlib.h>
#include <string.h>
#include <nmp_rw_file.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "nmp_sysctl.h"
#include "nmp_debug.h"
#include "nmp_errno.h"
#include "nmp_ipsan.h"


#define SC_CONFIG_IPSAN_NAME		"mss_ipsan.conf"
#define SC_MSS_CONFIG_PERMISSION	(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)


static __inline__ rw_file *
nmp_ipsan_config_file(const gchar *env_name)
{
	gchar *path, file_name[MAX_FILE_PATH];
	rw_file *rw_f;
	gint err;

	path = getenv(env_name);
	if (!path)
		path = SC_DEFAULT_CONFIG_PATH;

	if (strlen(path) + strlen("/") + strlen(SC_CONFIG_IPSAN_NAME)
		>= MAX_FILE_PATH)
	{
		nmp_warning(
			"Cms open 'mss_ipsan.conf' failed, path too long."
		);
		return NULL;		/* use default */
	}

	strcpy(file_name, path);
	strcat(file_name, "/");
	strcat(file_name, SC_CONFIG_IPSAN_NAME);

	rw_f = open_rw_file(file_name, SC_MSS_CONFIG_PERMISSION, &err);
	if (!rw_f)
	{
		nmp_warning(
			"Cms open 'mss_ipsan.conf' failed, err: %d.", err
		);
		return NULL;		/* use default */
	}

	return rw_f;
}


static __inline__ gint __nmp_get_ipsan_info(rw_file *conf, NmpIpsanInfo *info)
{
	const gchar *val;
	gint total = 0;
	gint i;

	for (i = 0; i < MAX_IPSAN_NUM; i++)
	{
		val = get_value_of(conf, "section.mss", i, "ip");
		if (!val)
			break;
		
		if (strlen(val) > MAX_CONF_LEN)
			return -E_STRINGLEN;

		sscanf(val, "%[0-9,.]:%d", info->ips[i].ip, &info->ips[i].port);
		total++;
	}
	info->total = total;

	return 0;
}

gint nmp_get_ipsan_info(NmpIpsanInfo *info)
{
	rw_file *rw_f;
	gint ret;

	rw_f = nmp_ipsan_config_file(SC_CONFIG_ENV_NAME);
	if (!rw_f)
		return -1;

	if ((ret = __nmp_get_ipsan_info(rw_f, info)) < 0)
		nmp_warning("Mss get ipsan info failed, ret = %d", ret);
	
	close_rw_file(rw_f);

	return ret;
}


static __inline__ gint __nmp_set_ipsan_info(rw_file *conf, NmpIpsanInfo *info)
{
	gint total = info->total;
	gint ret, i;
	gchar tmp[MAX_CONF_LEN];

	if (total == 0)
		return 0;
	if (total < 0 || total > MAX_IPSAN_NUM)
		return -E_UNKNOWN;

	for (i = 0; i < total; i++)
	{
		snprintf(tmp, MAX_CONF_LEN, "%s:%d", info->ips[i].ip, info->ips[i].port);
		if ((ret = set_multi_value_of(conf, "section.mss", "ip", tmp)) != 0)
			return ret;
	}

	return 0;
}

gint nmp_set_ipsan_info(NmpIpsanInfo *info)
{
	rw_file *rw_f;
	gint ret;

	rw_f = nmp_ipsan_config_file(SC_CONFIG_ENV_NAME);
	if (!rw_f)
		return -1;

	del_multi_value_of(rw_f, "section.mss", "ip");

	if ((ret = __nmp_set_ipsan_info(rw_f, info)) < 0)
		nmp_warning("Mss set ipsan info failed, ret = %d", ret);

	flush_rw_file(rw_f);
	close_rw_file(rw_f);

	return ret;
}


static __inline__ gint __nmp_add_one_ipsan(rw_file *conf, NmpIpsanIp *info)
{
	gchar tmp_ch[MAX_CONF_LEN];
	NmpIpsanInfo tmp;
	gint ret, i;

	memset(&tmp, 0, sizeof(NmpIpsanInfo));

	if ((ret = __nmp_get_ipsan_info(conf, &tmp)) < 0)
		return ret;

	if (tmp.total >= MAX_IPSAN_NUM)
		return -E_IPSANMAXNUM;
	for (i = 0; i < tmp.total; i++)
	{
		if (strcmp(info->ip, tmp.ips[i].ip) == 0)
			return -E_NAMEEXIST;
	}

	snprintf(tmp_ch, MAX_CONF_LEN, "%s:%d", info->ip, info->port);
	if ((ret = set_multi_value_of(conf, "section.mss", "ip", tmp_ch)) != 0)
		return ret;

	return 0;
}

gint nmp_add_one_ipsan(NmpIpsanIp *info)
{
	rw_file *rw_f;
	gint ret;

	rw_f = nmp_ipsan_config_file(SC_CONFIG_ENV_NAME);
	if (!rw_f)
		return -1;

	if ((ret = __nmp_add_one_ipsan(rw_f, info)) < 0)
		nmp_warning("Mss add one ipsan failed, ret = %d", ret);

	flush_rw_file(rw_f);
	close_rw_file(rw_f);
	
	return ret;
}


static __inline__ gint __nmp_del_one_ipsan(rw_file *conf, NmpIpsanIp *info)
{
	NmpIpsanInfo tmp;
	gint ret, i, j;

	memset(&tmp, 0, sizeof(NmpIpsanInfo));

	if ((ret = __nmp_get_ipsan_info(conf, &tmp)) < 0)
		return ret;
	
	for (i = 0; i < tmp.total; i++)
	{
		if (info->port == tmp.ips[i].port && strcmp(info->ip, tmp.ips[i].ip) == 0)
			break;
	}
	if (i >= tmp.total)
		return 0;

	for (j = i + 1; j < tmp.total; j++)
	{
		tmp.ips[j - 1] = tmp.ips[j];
	}
	tmp.total--;

	del_multi_value_of(conf, "section.mss", "ip");

	if ((ret = __nmp_set_ipsan_info(conf, &tmp)) < 0)
		return ret;

	return 0;
}

void nmp_del_one_ipsan(NmpIpsanIp *info)
{
	rw_file *rw_f;
	gint ret;

	rw_f = nmp_ipsan_config_file(SC_CONFIG_ENV_NAME);
	if (!rw_f)
		return ;

	if ((ret = __nmp_del_one_ipsan(rw_f, info)) < 0)
		nmp_warning("Mss del one ipsan failed, ret = %d", ret);

	flush_rw_file(rw_f);
	close_rw_file(rw_f);
}

