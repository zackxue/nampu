#include <rtspwatch.h>
#include "nmp_media_device.h"

#ifndef __NMP_DEVICE_POOL_H__
#define __NMP_DEVICE_POOL_H__

G_BEGIN_DECLS

typedef struct _NmpDevicePool NmpDevicePool;
struct _NmpDevicePool {	/* 设备池/哈希表 */
	guint			device_count;	/* 已连接上的设备数 */

	GMutex			*table_lock;
	GHashTable		*table_devices;

	guint			 timer;			/* 定时器 */
};


typedef struct _NmpAcceptDevMng NmpAcceptDevMng;
struct _NmpAcceptDevMng
{
	GList 			*devices;
	GMutex			*lock;

	guint			 timer;			/* 定时器 */
};


typedef struct _NmpDevMng NmpDevMng;
struct _NmpDevMng {
	gint			ref_count;	/* 引用计数 */
	NmpDevicePool	*dev_pool;	/* 设备池 */
	gchar			*address;	/* 服务IP */
	gchar			*service;	/* 服务端口 */

	NmpAcceptDevMng *dev_unrecognized;	/* ACCEPT设备*/
};


NmpDevMng *nmp_rtsp_device_mng_new(gint service);
void nmp_rtsp_device_mng_unref(NmpDevMng *dev_mng);
guint nmp_rtsp_device_mng_attach(NmpDevMng *dev_mng);

gboolean nmp_rtsp_device_mng_accepted(NmpDevMng *dev_mng, 
	NmpMediaDevice *device);

gint nmp_rtsp_device_mng_add_dev(NmpDevMng *dev_mng,
	NmpMediaDevice *device, gchar old_ip[]);
void nmp_rtsp_device_mng_remove(NmpMediaDevice *device);

NmpMediaDevice *nmp_rtsp_device_mng_find_and_get_dev(NmpDevMng *dev_mng, 
	const gchar *id);

G_END_DECLS

#endif /* __NMP_DEVICE_POOL_H__ */
