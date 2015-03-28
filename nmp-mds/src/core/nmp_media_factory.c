#include "nmp_debug.h"
#include "nmp_media_factory.h"

G_DEFINE_TYPE (JpfRtspMediaFactory, nmp_rtsp_media_factory,
    G_TYPE_OBJECT);

static JpfRtspMedia* 
nmp_rtsp_default_create_media(JpfRtspMediaFactory *factory, 
	const JpfMediaUri *media_uri);

JpfRtspMediaFactory *jxj_device_factory = NULL;

static void
nmp_rtsp_media_factory_init(JpfRtspMediaFactory *factory)
{
}


static void
nmp_rtsp_media_factory_finalize(GObject *obj)
{
/*	JpfRtspMediaFactory *factory = JPF_RTSP_MEDIA_FACTORY(obj); */
	G_OBJECT_CLASS(nmp_rtsp_media_factory_parent_class)->finalize(obj);
}


static void
nmp_rtsp_media_factory_class_init(JpfRtspMediaFactoryClass *klass)
{
	GObjectClass *gobject_class;

	gobject_class = G_OBJECT_CLASS (klass);
	klass->create_media = nmp_rtsp_default_create_media;
}


JpfRtspMediaFactory *
nmp_rtsp_media_factory_new( void )
{
	JpfRtspMediaFactory *result;

	result = g_object_new(JPF_TYPE_RTSP_MEDIA_FACTORY, NULL);
	return result;	
}


void
nmp_rtsp_media_factory_jxj_init( void )
{
	 jxj_device_factory = nmp_rtsp_media_factory_new();
	 g_assert(jxj_device_factory);
}


static JpfRtspMedia* 
nmp_rtsp_default_create_media(JpfRtspMediaFactory *factory, 
	const JpfMediaUri *media_uri)
{
	JpfRtspMedia *media;

	media = nmp_rtsp_media_new();
	media->media_uri = nmp_rtsp_media_uri_dup(media_uri);

	nmp_print(
		"Create media object '%p' URI:'%s/media-%d/channel=%02d&level=%d'.",
		media,
		media->media_uri->device,
		media->media_uri->type,
		media->media_uri->channel,
		media->media_uri->rate_level
	);

	return media;
}


JpfRtspMedia *
nmp_rtsp_media_factory_create_media(JpfRtspMediaFactory *factory,
	const JpfMediaUri *media_uri)
{
	g_return_val_if_fail(JPF_IS_RTSP_MEDIA_FACTORY(factory), NULL);
	g_return_val_if_fail(media_uri != NULL, NULL);

	return JPF_RTSP_MEDIA_FACTORY_GET_CLASS(factory)->create_media(
		factory, media_uri);
}


//:~ End
