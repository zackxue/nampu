#include <rtspwatch.h>

#include "nmp_media.h"

#ifndef __NMP_MEDIA_FACTORY_H__
#define __NMP_MEDIA_FACTORY_H__

G_BEGIN_DECLS

#define JPF_TYPE_RTSP_MEDIA_FACTORY	(nmp_rtsp_media_factory_get_type())
#define JPF_IS_RTSP_MEDIA_FACTORY(o) \
	(G_TYPE_CHECK_INSTANCE_TYPE((o), JPF_TYPE_RTSP_MEDIA_FACTORY))
#define JPF_IS_RTSP_MEDIA_FACTORY_CLASS(c) \
	(G_TYPE_CHECK_CLASS_TYPE((c), JPF_TYPE_RTSP_MEDIA_FACTORY))
#define JPF_RTSP_MEDIA_FACTORY(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), JPF_TYPE_RTSP_MEDIA_FACTORY, JpfRtspMediaFactory))
#define JPF_RTSP_MEDIA_FACTORY_CLASS(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), JPF_TYPE_RTSP_MEDIA_FACTORY, JpfRtspMediaFactoryClass))
#define JPF_RTSP_MEDIA_FACTORY_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), JPF_TYPE_RTSP_MEDIA_FACTORY, JpfRtspMediaFactoryClass))

typedef struct _JpfRtspMediaFactory JpfRtspMediaFactory;
typedef struct _JpfRtspMediaFactoryClass JpfRtspMediaFactoryClass;

struct _JpfRtspMediaFactory
{
	GObject			parent;
};


struct _JpfRtspMediaFactoryClass
{
	GObjectClass	parent_class;

	JpfRtspMedia* (*create_media)(JpfRtspMediaFactory *factory, 
		const JpfMediaUri *media_info);
};

extern JpfRtspMediaFactory *jxj_device_factory;

GType nmp_rtsp_media_factory_get_type( void );
JpfRtspMediaFactory *nmp_rtsp_media_factory_new( void );
JpfRtspMedia *nmp_rtsp_media_factory_create_media(JpfRtspMediaFactory *factory,
	const JpfMediaUri *media_info);

void nmp_rtsp_media_factory_jxj_init( void );

G_END_DECLS

#endif /* __NMP_MEDIA_FACTORY_H__ */
