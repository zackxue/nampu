#ifndef __NMP_MEDIA_SERVER_H__
#define __NMP_MEDIA_SERVER_H__

#include <rtspwatch.h>
#include "nmp_device_mng.h"
#include "nmp_client_mng.h"
#include "nmp_authorise.h"
#include "nmp_mod_cms.h"

G_BEGIN_DECLS


typedef struct _JpfMediaServer JpfMediaServer;

struct _JpfMediaServer
{
	/* server id */
	gchar			*server_id;

	/* mod cms */
	JpfModCms		*module_cms;

	/* for clients management */
	JpfClientMng	*client_mng;

	/* for devices management */
	JpfDevMng		*device_mng;

	/* for authorising */
	JpfAuthorise	*authorise;
};


void nmp_media_server_init( void );
JpfMediaServer *nmp_media_server_get( void );
void nmp_media_server_set_id(JpfMediaServer *server,
	const gchar *id);
gchar *nmp_media_server_get_id(JpfMediaServer *server);

G_END_DECLS

#endif	/* __NMP_MEDIA_SERVER_H__ */
