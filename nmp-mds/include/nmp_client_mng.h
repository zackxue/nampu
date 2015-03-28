#ifndef __NMP_CLIENT_MANAGEMENT_H__
#define __NMP_CLIENT_MANAGEMENT_H__

#include <glib.h>
#include "nmp_client.h"

typedef struct _JpfClientMng JpfClientMng;

struct _JpfClientMng
{
	gchar			*address;	/* service ip address */
	gchar			*service;	/* service port info */

	GList			*clients;	/* clients list */
	GMutex			*lock;		/* list lock */

	guint			 timer;		/* manager */
};


JpfClientMng *nmp_rtsp_client_mng_new(gint service);
void nmp_rtsp_client_mng_unref(JpfClientMng *client_mng);

guint nmp_rtsp_client_mng_attach(JpfClientMng *client_mng);

void nmp_rtsp_client_mng_remove_client(JpfClientMng *client_mng, 
	JpfRtspClient *client);

#endif	/* __NMP_CLIENT_MANAGEMENT_H__ */
