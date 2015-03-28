#include "nmp_media_server.h"
#include "nmp_debug.h"
#include "nmp_sysctl.h"

static JpfMediaServer glb_server;

JpfMediaServer *
nmp_media_server_get( void )
{
	return &glb_server;
}

static __inline__ void
__nmp_media_server_init(JpfMediaServer *server)
{
	server->module_cms = nmp_mod_cms_new();

	server->client_mng = nmp_rtsp_client_mng_new(
		nmp_mod_cms_rtsp_port(server->module_cms)
	);
	if (G_UNLIKELY(!server->client_mng))
	{
		nmp_warning(
			"alloc client-mng object failed!"
		);
		FATAL_ERROR_EXIT;
	}

	server->device_mng = nmp_rtsp_device_mng_new(
		nmp_mod_cms_pu_port(server->module_cms)
	);
	if (G_UNLIKELY(!server->device_mng))
	{
		nmp_warning(
			"alloc device-mng object failed!"
		);
		FATAL_ERROR_EXIT;
	}

	server->authorise = NULL;
}


static __inline__ void
nmp_media_server_attach(JpfMediaServer *server)
{
	G_ASSERT(server != NULL);

	if (!nmp_rtsp_client_mng_attach(server->client_mng))
	{
		nmp_warning("attach client mng failed!");
		FATAL_ERROR_EXIT;
	}

	if (!nmp_rtsp_device_mng_attach(server->device_mng))
	{
		nmp_warning("attach device mng failed!");
		FATAL_ERROR_EXIT;
	}
}


void
nmp_media_server_init( void )
{
	JpfMediaServer *server;

	server = nmp_media_server_get();
	__nmp_media_server_init(server);
	nmp_media_server_attach(server);
}


gchar *
nmp_media_server_get_id(JpfMediaServer *server)
{
	G_ASSERT(server != NULL);

	return (gchar*)nmp_get_sysctl_value(SC_MDS_ID);
}


gint
nmp_media_server_state( void )
{
	JpfMediaServer *svr = nmp_media_server_get();

	return nmp_mds_cms_state(svr->module_cms);
}

//:~ End
