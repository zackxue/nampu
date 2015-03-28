/*
 *	@author:	zyt
 *	@time:	2013/3/16
 */
#ifndef __NMP_SHARE_STRUCT_H__
#define __NMP_SHARE_STRUCT_H__


#define MAX_IP_LEN          		(32)
#define MAX_IPSAN_NUM			(8)
#define MAX_CONF_LEN			(32)
#define MSS_NAME_LEN 			(64)


typedef struct _NmpIpsanIp NmpIpsanIp;
struct _NmpIpsanIp
{
	gchar	ip[MAX_IP_LEN];
	gint		port;
};

typedef struct _NmpIpsanInfo NmpIpsanInfo;
struct _NmpIpsanInfo
{
	gint		total;
	NmpIpsanIp	ips[MAX_IPSAN_NUM];
};


#endif	/* __NMP_SHARE_STRUCT_H__ */
