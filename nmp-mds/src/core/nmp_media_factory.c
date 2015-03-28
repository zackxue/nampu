#include "nmp_debug.h"
#include "nmp_media_factory.h"

G_DEFINE_TYPE (NmpRtspMediaFactory, nmp_rtsp_media_factory,
    G_TYPE_OBJECT);

static NmpRtspMedia* 
nmp_rtsp_default_create_media(NmpRtspMediaFactory *factory, 
	const NmpMediaUri *media_uri);

NmpRtspMediaFactory *jxj_device_factory = NULL;

static void
nmp_rtsp_media_factory_init(NmpRtspMediaFactory *factory)
{
}


static void
nmp_rtsp_media_factory_finalize(GObject *obj)
{
/*	NmpRtspMediaFactory *factory = NMP_RTSP_MEDIA_FACTORY(obj); */
	G_OBJECT_CLASS(nmp_rtsp_media_factory_parent_class)->finalize(obj);
}


static void
nmp_rtsp_media_factory_class_init(NmpRtspMediaFactoryClass *klass)
{
	GObjectClass *gobject_class;

	gobject_class = G_OBJECT_CLASS (klass);
	klass->create_media = nmp_rtsp_default_create_media;
}


NmpRtspMediaFactory *
nmp_rtsp_media_factory_new( void )
{
	NmpRtspMediaFactory *result;

	result = g_object_new(NMP_TYPE_RTSP_MEDIA_FACTORY, NULL);
	return result;	
}


void
nmp_rtsp_media_factory_jxj_init( void )
{
	 jxj_device_factory = nmp_rtsp_media_factory_new();
	 g_assert(jxj_device_factory);
}


static NmpRtspMedia* 
nmp_rtsp_default_create_media(NmpRtspMediaFactory *factory, 
	const NmpMediaUri *media_uri)
{
	NmpRtspMedia *media;

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


NmpRtspMedia *
nmp_rtsp_media_factory_create_media(NmpRtspMediaFactory *factory,
	const NmpMediaUri *media_uri)
{
	g_return_val_if_fail(NMP_IS_RTSP_MEDIA_FACTORY(factory), NULL);
	g_return_val_if_fail(media_uri != NULL, NULL);

	return NMP_RTSP_MEDIA_FACTORY_GET_CLASS(factory)->create_media(
		factory, media_uri);
}


//:~ End
