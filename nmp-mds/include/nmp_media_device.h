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
    NMP_DEV_STAT_NEW,           /* after created */
    NMP_DEV_STAT_REGISTERED,
    NMP_DEV_STAT_ILLEGAL
}NmpDeviceState;


typedef struct _NmpMediaDevice NmpMediaDevice;
typedef void (*NmpDeviceFin)(NmpMediaDevice *device);
struct _NmpMediaDevice
{
    gint                ref_count;      /* 引用计数 */
    gchar               id[MAX_DEVICE_ID_LEN];  /* 设备ID */
    gchar				localip[__MAX_IP_LEN];	/* 设备连接的MDS IP */
    gchar				devip[__MAX_IP_LEN];	/* 设备NAT IP */
    NmpDeviceState      state;          /* 设备状态 */
	NmpDeviceMediaType	media_type;		/* 设备流类型 */
    GstRtspWatch        *watch;         /* RTSP watch */

    GList               *live_medias;   /* 该设备的媒体 */

	gint                media_count;
    gint                ttd;            /* 当前生存期 */
    gint                time_to_die;    /* 设备生存期 */
    gint                seq_generator;  /* SEQ生成器 */
    GMutex              *device_lock;

    gpointer            private_data;
    NmpDeviceFin        finalize;
};


NmpMediaDevice *nmp_rtsp_device_new( void );
void nmp_rtsp_device_unref(NmpMediaDevice *device);
NmpMediaDevice *nmp_rtsp_device_ref(NmpMediaDevice *device);

gboolean nmp_rtsp_device_accept(NmpMediaDevice *device, 
    GEvent *e_listen, gint *errp);

gchar *nmp_rtsp_device_get_localip(NmpMediaDevice *device);

void nmp_rtsp_device_set_info(NmpMediaDevice *device, gchar *id,
	gchar *ip, gint ttd);

void nmp_rtsp_device_set_media_type(NmpMediaDevice *device,
	NmpDeviceMediaType mt);

void nmp_rtsp_device_update_ttd(NmpMediaDevice *device);

void nmp_rtsp_device_attach(NmpMediaDevice *device, void *ctx);

void nmp_rtsp_device_send_response(NmpMediaDevice *device,
    GstRTSPMessage *response);

gboolean nmp_rtsp_device_is_new(NmpMediaDevice *device);
void nmp_rtsp_device_set_registered(NmpMediaDevice *device);
void nmp_rtsp_device_set_illegal(NmpMediaDevice *device);

gboolean nmp_rtsp_device_check_id(NmpMediaDevice *device, 
    const gchar *id, gint ttd);

gint nmp_rtsp_device_request(NmpMediaDevice *device, 
    NmpRtspMedia *media);

gint nmp_rtsp_device_extend_request(NmpMediaDevice *device, 
	NmpRtspMedia *media, gint name, gpointer value);

NmpRtspMedia *nmp_rtsp_device_find_media(NmpMediaDevice *device, 
    NmpMediaUri *media_uri);

NmpRtspMedia *nmp_rtsp_device_get_media(NmpMediaDevice *device,
    gint seq);

void nmp_rtsp_device_remove_media(NmpMediaDevice *device,
	NmpRtspMedia *media);

G_END_DECLS

#endif /* __NMP_MEDIA_DEVICE_H__ */
