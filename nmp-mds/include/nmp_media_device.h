#include <rtspwatch.h>

#include "nmp_media.h"
#include "nmp_timer.h"


#ifndef __NMP_MEDIA_DEVICE_H__
#define __NMP_MEDIA_DEVICE_H__

#define MAX_DEVICE_ID_LEN           32
#define MAX_DEVICE_TTD_SEC          450
#define MAX_DEVICE_ACC_SEC			3

#define REQUEST_DEVICE_IFRAME		0

#define __MAX_IP_LEN				64

G_BEGIN_DECLS

typedef enum
{
    JPF_DEV_STAT_NEW,           /* after created */
    JPF_DEV_STAT_REGISTERED,
    JPF_DEV_STAT_ILLEGAL
}JpfDeviceState;


typedef struct _JpfMediaDevice JpfMediaDevice;
typedef void (*JpfDeviceFin)(JpfMediaDevice *device);
struct _JpfMediaDevice
{
    gint                ref_count;      /* 引用计数 */
    gchar               id[MAX_DEVICE_ID_LEN];  /* 设备ID */
    gchar				localip[__MAX_IP_LEN];	/* 设备连接的MDS IP */
    gchar				devip[__MAX_IP_LEN];	/* 设备NAT IP */
    JpfDeviceState      state;          /* 设备状态 */
	JpfDeviceMediaType	media_type;		/* 设备流类型 */
    GstRtspWatch        *watch;         /* RTSP watch */

    GList               *live_medias;   /* 该设备的媒体 */

	gint                media_count;
    gint                ttd;            /* 当前生存期 */
    gint                time_to_die;    /* 设备生存期 */
    gint                seq_generator;  /* SEQ生成器 */
    GMutex              *device_lock;

    gpointer            private_data;
    JpfDeviceFin        finalize;
};


JpfMediaDevice *nmp_rtsp_device_new( void );
void nmp_rtsp_device_unref(JpfMediaDevice *device);
JpfMediaDevice *nmp_rtsp_device_ref(JpfMediaDevice *device);

gboolean nmp_rtsp_device_accept(JpfMediaDevice *device, 
    GEvent *e_listen, gint *errp);

gchar *nmp_rtsp_device_get_localip(JpfMediaDevice *device);

void nmp_rtsp_device_set_info(JpfMediaDevice *device, gchar *id,
	gchar *ip, gint ttd);

void nmp_rtsp_device_set_media_type(JpfMediaDevice *device,
	JpfDeviceMediaType mt);

void nmp_rtsp_device_update_ttd(JpfMediaDevice *device);

void nmp_rtsp_device_attach(JpfMediaDevice *device, void *ctx);

void nmp_rtsp_device_send_response(JpfMediaDevice *device,
    GstRTSPMessage *response);

gboolean nmp_rtsp_device_is_new(JpfMediaDevice *device);
void nmp_rtsp_device_set_registered(JpfMediaDevice *device);
void nmp_rtsp_device_set_illegal(JpfMediaDevice *device);

gboolean nmp_rtsp_device_check_id(JpfMediaDevice *device, 
    const gchar *id, gint ttd);

gint nmp_rtsp_device_request(JpfMediaDevice *device, 
    JpfRtspMedia *media);

gint nmp_rtsp_device_extend_request(JpfMediaDevice *device, 
	JpfRtspMedia *media, gint name, gpointer value);

JpfRtspMedia *nmp_rtsp_device_find_media(JpfMediaDevice *device, 
    JpfMediaUri *media_uri);

JpfRtspMedia *nmp_rtsp_device_get_media(JpfMediaDevice *device,
    gint seq);

void nmp_rtsp_device_remove_media(JpfMediaDevice *device,
	JpfRtspMedia *media);

G_END_DECLS

#endif /* __NMP_MEDIA_DEVICE_H__ */
