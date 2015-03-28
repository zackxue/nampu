#ifndef __NMP_MEDIA_SERVER_H__
#define __NMP_MEDIA_SERVER_H__

#include <rtspwatch.h>
#include "nmp_device_mng.h"
#include "nmp_client_mng.h"
#include "nmp_authorise.h"
#include "nmp_mod_cms.h"

G_BEGIN_DECLS


typedef struct _NmpMediaServer NmpMediaServer;

struct _NmpMediaServer
{
	/* server id */
	gchar			*server_id;

	/* mod cms */
	NmpModCms		*module_cms;

	/* for clients management */
	NmpClientMng	*client_mng;

	/* for devices management */
	NmpDevMng		*device_mng;

	/* for authorising */
	NmpAuthorise	*authorise;
};


void nmp_media_server_init( void );
NmpMediaServer *nmp_media_server_get( void );
void nmp_media_server_set_id(NmpMediaServer *server,
	const gchar *id);
gchar *nmp_media_server_get_id(NmpMediaServer *server);

G_END_DECLS

#endif	/* __NMP_MEDIA_SERVER_H__ */
