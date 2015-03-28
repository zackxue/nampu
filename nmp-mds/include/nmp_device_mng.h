#include <rtspwatch.h>
#include "nmp_media_device.h"

#ifndef __NMP_DEVICE_POOL_H__
#define __NMP_DEVICE_POOL_H__

G_BEGIN_DECLS

typedef struct _JpfDevicePool JpfDevicePool;
struct _JpfDevicePool {	/* 设备池/哈希表 */
	guint			device_count;	/* 已连接上的设备数 */

	GMutex			*table_lock;
	GHashTable		*table_devices;

	guint			 timer;			/* 定时器 */
};


typedef struct _JpfAcceptDevMng JpfAcceptDevMng;
struct _JpfAcceptDevMng
{
	GList 			*devices;
	GMutex			*lock;

	guint			 timer;			/* 定时器 */
};


typedef struct _JpfDevMng JpfDevMng;
struct _JpfDevMng {
	gint			ref_count;	/* 引用计数 */
	JpfDevicePool	*dev_pool;	/* 设备池 */
	gchar			*address;	/* 服务IP */
	gchar			*service;	/* 服务端口 */

	JpfAcceptDevMng *dev_unrecognized;	/* ACCEPT设备*/
};


JpfDevMng *nmp_rtsp_device_mng_new(gint service);
void nmp_rtsp_device_mng_unref(JpfDevMng *dev_mng);
guint nmp_rtsp_device_mng_attach(JpfDevMng *dev_mng);

gboolean nmp_rtsp_device_mng_accepted(JpfDevMng *dev_mng, 
	JpfMediaDevice *device);

gint nmp_rtsp_device_mng_add_dev(JpfDevMng *dev_mng,
	JpfMediaDevice *device, gchar old_ip[]);
void nmp_rtsp_device_mng_remove(JpfMediaDevice *device);

JpfMediaDevice *nmp_rtsp_device_mng_find_and_get_dev(JpfDevMng *dev_mng, 
	const gchar *id);

G_END_DECLS

#endif /* __NMP_DEVICE_POOL_H__ */
