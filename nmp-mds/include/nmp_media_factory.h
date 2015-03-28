#include <rtspwatch.h>

#include "nmp_media.h"

#ifndef __NMP_MEDIA_FACTORY_H__
#define __NMP_MEDIA_FACTORY_H__

G_BEGIN_DECLS

#define NMP_TYPE_RTSP_MEDIA_FACTORY	(nmp_rtsp_media_factory_get_type())
#define NMP_IS_RTSP_MEDIA_FACTORY(o) \
	(G_TYPE_CHECK_INSTANCE_TYPE((o), NMP_TYPE_RTSP_MEDIA_FACTORY))
#define NMP_IS_RTSP_MEDIA_FACTORY_CLASS(c) \
	(G_TYPE_CHECK_CLASS_TYPE((c), NMP_TYPE_RTSP_MEDIA_FACTORY))
#define NMP_RTSP_MEDIA_FACTORY(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), NMP_TYPE_RTSP_MEDIA_FACTORY, NmpRtspMediaFactory))
#define NMP_RTSP_MEDIA_FACTORY_CLASS(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), NMP_TYPE_RTSP_MEDIA_FACTORY, NmpRtspMediaFactoryClass))
#define NMP_RTSP_MEDIA_FACTORY_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), NMP_TYPE_RTSP_MEDIA_FACTORY, NmpRtspMediaFactoryClass))

typedef struct _NmpRtspMediaFactory NmpRtspMediaFactory;
typedef struct _NmpRtspMediaFactoryClass NmpRtspMediaFactoryClass;

struct _NmpRtspMediaFactory
{
	GObject			parent;
};


struct _NmpRtspMediaFactoryClass
{
	GObjectClass	parent_class;

	NmpRtspMedia* (*create_media)(NmpRtspMediaFactory *factory, 
		const NmpMediaUri *media_info);
};

extern NmpRtspMediaFactory *jxj_device_factory;

GType nmp_rtsp_media_factory_get_type( void );
NmpRtspMediaFactory *nmp_rtsp_media_factory_new( void );
NmpRtspMedia *nmp_rtsp_media_factory_create_media(NmpRtspMediaFactory *factory,
	const NmpMediaUri *media_info);

void nmp_rtsp_media_factory_jxj_init( void );

G_END_DECLS

#endif /* __NMP_MEDIA_FACTORY_H__ */
