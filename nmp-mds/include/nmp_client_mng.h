#ifndef __NMP_CLIENT_MANAGEMENT_H__
#define __NMP_CLIENT_MANAGEMENT_H__

#include <glib.h>
#include "nmp_client.h"

typedef struct _NmpClientMng NmpClientMng;

struct _NmpClientMng
{
	gchar			*address;	/* service ip address */
	gchar			*service;	/* service port info */

	GList			*clients;	/* clients list */
	GMutex			*lock;		/* list lock */

	guint			 timer;		/* manager */
};


NmpClientMng *nmp_rtsp_client_mng_new(gint service);
void nmp_rtsp_client_mng_unref(NmpClientMng *client_mng);

guint nmp_rtsp_client_mng_attach(NmpClientMng *client_mng);

void nmp_rtsp_client_mng_remove_client(NmpClientMng *client_mng, 
	NmpRtspClient *client);

#endif	/* __NMP_CLIENT_MANAGEMENT_H__ */
