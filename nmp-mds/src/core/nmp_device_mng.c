#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

#include "nmp_device_mng.h"
#include "nmp_errno.h"
#include "nmp_debug.h"
#include "nmpevsched.h"
#include "nmp_utils.h"

#define DEFAULT_LISTEN_ADDRESS		"0.0.0.0"
#define MAX_LISTEN_BACKLOG			5


static void
nmp_rtsp_device_1s_elapsed(gpointer key, gpointer value,
	gpointer user_data)
{
	NmpMediaDevice *device;
	G_ASSERT(value != NULL);

	device = (NmpMediaDevice*)value;
	--device->ttd;
}


static gboolean
nmp_rtsp_device_timeout(gpointer key, gpointer value,
	gpointer user_data)
{
	NmpMediaDevice *device;
	G_ASSERT(value != NULL);

	device = (NmpMediaDevice*)value;
	if (device->ttd >= 0)
		return FALSE;

	return TRUE;
}


static __inline__ void
__nmp_rtsp_device_pool_on_timer(NmpDevicePool *dev_pool)
{
	NmpMediaDevice *device;

	g_hash_table_foreach(
		dev_pool->table_devices,
		nmp_rtsp_device_1s_elapsed,
		NULL
	);

	for (;;)
	{
		device = g_hash_table_find(
			dev_pool->table_devices,
			nmp_rtsp_device_timeout,
			NULL
		);

		if (!device)
			break;

		nmp_rtsp_device_ref(device);

		if (!g_hash_table_remove(
			dev_pool->table_devices, device->id))
		{
			nmp_error(
				"Dev-Timer remove failed."
			);
			FATAL_ERROR_EXIT;
		}

		g_mutex_unlock(dev_pool->table_lock);

		nmp_print(
			"Dev-Timer remove device '%p', ID:'%s'.",
			device, device->id
		);

		nmp_rtsp_device_set_illegal(device);
		nmp_rtsp_device_unref(device);

		g_mutex_lock(dev_pool->table_lock);
	}
}


static __inline__ void
nmp_rtsp_device_pool_on_timer(NmpDevicePool *dev_pool)
{
	g_mutex_lock(dev_pool->table_lock);
	__nmp_rtsp_device_pool_on_timer(dev_pool);
	g_mutex_unlock(dev_pool->table_lock);	
}


static gboolean
nmp_rtsp_device_pool_timer(gpointer user_data)
{
	NmpDevicePool *dev_pool;
	G_ASSERT(user_data != NULL);

	dev_pool = (NmpDevicePool*)user_data;
	nmp_rtsp_device_pool_on_timer(dev_pool);

	return TRUE;
}


static __inline__ NmpDevicePool *
nmp_rtsp_device_pool_new( void )
{
	NmpDevicePool *dev_pool;

	dev_pool = g_new0(NmpDevicePool, 1);
	dev_pool->device_count = 0;
	dev_pool->table_lock = g_mutex_new();
	dev_pool->table_devices = g_hash_table_new_full(
		g_str_hash, 
		g_str_equal, 
		NULL, 
		(GDestroyNotify)nmp_rtsp_device_unref
	);

	dev_pool->timer = nmp_set_timer(
		1000,	/* ¼äÐª¶¨Ê±Æ÷£¬1Ãë */
		nmp_rtsp_device_pool_timer,
		dev_pool
	);

	return dev_pool;
}


static __inline__ gint
__nmp_rtsp_device_pool_add_dev(NmpDevicePool *dev_pool, 
	NmpMediaDevice *device, gchar old_ip[])
{
	NmpMediaDevice *old_dev;

	old_dev = g_hash_table_lookup(
		dev_pool->table_devices, device->id
	);

	if (G_UNLIKELY(old_dev))
	{
		if (old_ip)
		{
			strncpy(old_ip, old_dev->devip, __MAX_IP_LEN - 1);
			old_ip[__MAX_IP_LEN - 1] = 0;
		}
		return -E_EXISTDEV;
	}

	if (old_ip)
	{
		strcpy(old_ip, "");
	}

	nmp_rtsp_device_ref(device);
	g_hash_table_insert(
		dev_pool->table_devices, device->id, device
	);

	return 0;
}


static __inline__ gint
nmp_rtsp_device_pool_add_dev(NmpDevicePool *dev_pool,
	NmpMediaDevice *device, gchar old_ip[])
{
	gint err;
	g_assert(dev_pool != NULL && device != NULL);

	g_mutex_lock(dev_pool->table_lock);
	err = __nmp_rtsp_device_pool_add_dev(dev_pool, device, old_ip);
	g_mutex_unlock(dev_pool->table_lock);

	return err;
}


static __inline__ void
nmp_rtsp_device_pool_remove_dev(NmpDevicePool *dev_pool, 
	NmpMediaDevice *device)
{
	g_mutex_lock(dev_pool->table_lock);

	g_hash_table_remove(
		dev_pool->table_devices, device->id
	);

	g_mutex_unlock(dev_pool->table_lock);
}


static __inline__ NmpMediaDevice *
__nmp_rtsp_device_pool_find_and_get_dev(NmpDevicePool *dev_pool,
	const gchar *id)
{
	NmpMediaDevice *dev;

	dev = g_hash_table_lookup(dev_pool->table_devices, id);
	if (dev)
	{
		nmp_rtsp_device_ref(dev);
	}

	return dev;
}


static __inline__ NmpMediaDevice *
nmp_rtsp_device_pool_find_and_get_dev(NmpDevicePool *dev_pool, 
	const gchar *id)
{
	NmpMediaDevice *dev;
	g_assert(dev_pool != NULL);
	
	g_mutex_lock(dev_pool->table_lock);
	dev = __nmp_rtsp_device_pool_find_and_get_dev(dev_pool, id);
	g_mutex_unlock(dev_pool->table_lock);

	if (!dev)
	{
		nmp_print(
			"Can't find device, PUID '%s'", id
		);
	}

	return dev;
}


static gint
nmp_accept_dev_timeout(gconstpointer a, gconstpointer null)
{
	NmpMediaDevice *dev;
	G_ASSERT(a != NULL);

	dev = (NmpMediaDevice*)a;
	return dev->ttd >= 0 ? 1 : 0;
}


static void
nmp_accept_dev_1s_elapsed(gpointer data, gpointer null)
{
	NmpMediaDevice *dev;
	G_ASSERT(data != NULL);

	dev = (NmpMediaDevice*)data;
	--dev->ttd;
}


static __inline__ void
__nmp_accept_dev_mng_timer(NmpAcceptDevMng *dev_mng)
{
	NmpMediaDevice *dev;
	GList *list;

	g_list_foreach(
		dev_mng->devices,
		nmp_accept_dev_1s_elapsed,
		NULL
	);

	for (;;)
	{
		list = g_list_find_custom(
			dev_mng->devices,
			NULL, 
			nmp_accept_dev_timeout
		);

		if (!list)
			break;

		dev = (NmpMediaDevice*)list->data;

		dev_mng->devices = g_list_delete_link(
			dev_mng->devices,
			list
		);

		g_mutex_unlock(dev_mng->lock);

		nmp_print(
			"Device '%p' removed by Accept-Timer.", dev
		);

		nmp_rtsp_device_set_illegal(dev);
		nmp_rtsp_device_unref(dev);

		g_mutex_lock(dev_mng->lock);
	}
}


static gboolean
nmp_accept_dev_mng_timer(gpointer user_data)
{
	NmpAcceptDevMng *dev_mng;
	dev_mng = (NmpAcceptDevMng*)user_data;

	g_mutex_lock(dev_mng->lock);
	__nmp_accept_dev_mng_timer(dev_mng);
	g_mutex_unlock(dev_mng->lock);

	return TRUE;
}


static __inline__ NmpAcceptDevMng *
nmp_accept_dev_mng_new( void )
{
	NmpAcceptDevMng *dev_mng;

	dev_mng = g_new0(NmpAcceptDevMng, 1);
	dev_mng->lock = g_mutex_new();

	dev_mng->timer = nmp_set_timer(
		1000,
		nmp_accept_dev_mng_timer,
		dev_mng
	);

	return dev_mng;
}


static __inline__ void
__nmp_accept_dev_mng_add(NmpAcceptDevMng *dev_mng,
	NmpMediaDevice *device)
{
	BUG_ON(g_list_find(dev_mng->devices, device));

	dev_mng->devices = g_list_append(
		dev_mng->devices, device
	);

	nmp_rtsp_device_ref(device);
}


static __inline__ void
nmp_accept_dev_mng_add(NmpAcceptDevMng *dev_mng,
	NmpMediaDevice *device)
{
	g_mutex_lock(dev_mng->lock);
	__nmp_accept_dev_mng_add(dev_mng, device);
	g_mutex_unlock(dev_mng->lock);
}


static __inline__ gboolean
__nmp_accept_dev_mng_remove(NmpAcceptDevMng *dev_mng,
	NmpMediaDevice *device)
{
	GList *list;

	list = g_list_find(dev_mng->devices, device);
	if (list)
	{
		dev_mng->devices = g_list_delete_link(
			dev_mng->devices,
			list
		);
		nmp_rtsp_device_unref(device);
	}

	return list != NULL;
}


static __inline__ gboolean
nmp_accept_dev_mng_remove(NmpAcceptDevMng *dev_mng,
	NmpMediaDevice *device)
{
	gboolean in_list;

	g_mutex_lock(dev_mng->lock);
	in_list = __nmp_accept_dev_mng_remove(
		dev_mng, device
	);
	g_mutex_unlock(dev_mng->lock);

	return in_list;
}


static __inline__ gboolean
nmp_accept_dev_mng_accepted(NmpAcceptDevMng *dev_mng,
	NmpMediaDevice *device)
{
	return nmp_accept_dev_mng_remove(
		dev_mng, device
	);
}


NmpDevMng *
nmp_rtsp_device_mng_new(gint service)
{
	NmpDevMng *dev_mng;

	dev_mng = g_new0(NmpDevMng, 1);
	dev_mng->ref_count = 1;
	dev_mng->dev_pool = nmp_rtsp_device_pool_new();
	dev_mng->address = g_strdup(DEFAULT_LISTEN_ADDRESS);
	dev_mng->service = g_strdup_printf("%d", service);
	dev_mng->dev_unrecognized = nmp_accept_dev_mng_new();
  
	return dev_mng;
}


void
nmp_rtsp_device_mng_unref(NmpDevMng *dev_mng)
{
	nmp_error(
		"Global object 'dev_mng' is unrefed by mistake!"
	);
	FATAL_ERROR_EXIT;
}


void
nmp_rtsp_device_mng_ref(NmpDevMng *dev_mng)
{//@{Nothing to do}
}


gint
nmp_rtsp_device_mng_add_dev(NmpDevMng *dev_mng, 
	NmpMediaDevice *device, gchar old_ip[])
{
	g_assert(dev_mng != NULL && device != NULL);

	return nmp_rtsp_device_pool_add_dev(
		dev_mng->dev_pool, device, old_ip
	);
}


static __inline__ void
nmp_rtsp_device_mng_remove_dev(NmpDevMng *dev_mng, 
	NmpMediaDevice *device)
{
	g_assert(dev_mng != NULL && device != NULL);

	nmp_accept_dev_mng_remove(
		dev_mng->dev_unrecognized, device
	);

	nmp_rtsp_device_pool_remove_dev(
		dev_mng->dev_pool, device
	);
}


static void
nmp_rtsp_device_mng_accept_link(NmpDevMng *dev_mng, 
	NmpMediaDevice *device)
{
	nmp_accept_dev_mng_add(
		dev_mng->dev_unrecognized,
		device
	);
}


gboolean
nmp_rtsp_device_mng_accepted(NmpDevMng *dev_mng,
	NmpMediaDevice *device)
{
	G_ASSERT(dev_mng != NULL && device != NULL);

	return nmp_accept_dev_mng_accepted(
		dev_mng->dev_unrecognized,
		device
	);
}


gboolean
nmp_rtsp_device_mng_io_func(GEvent *e_listen, gint revents, void *user_data)
{
	NmpDevMng *dev_mng = (NmpDevMng*)user_data;
	NmpMediaDevice *device;
	gint err;

	if (revents & EV_READ)
	{
		for(;;)
		{
			device = nmp_rtsp_device_new();
			if (G_UNLIKELY(!device))
			{
				nmp_error(
					"Create device object failed, OOM?"
				);
				FATAL_ERROR_EXIT;
	
				return FALSE;
			}
	
			device->private_data = dev_mng;
			if (G_UNLIKELY(!nmp_rtsp_device_accept(device, e_listen, &err)))
			{
				if (err == -EAGAIN)
				{
					nmp_rtsp_device_unref(device);
					break;
				}
	
				nmp_error(
					"Listen fd accept failed, err:'%d'.", err
				);
				FATAL_ERROR_EXIT;
				return FALSE;
			}
	
			nmp_rtsp_device_mng_accept_link(dev_mng, device);
			nmp_rtsp_device_attach(device, NULL /*dev_mng->loop_context*/);
			nmp_rtsp_device_unref(device);
		}

		return TRUE;
	}

	return TRUE;
}


static GEvent *
nmp_rtsp_dev_mng_create_listen_ev(NmpDevMng *dev_mng)
{
	GEvent *e_listen;
	gint ret, tag_onoff, sockfd = -1;
	struct addrinfo hints;
	struct addrinfo *result, *rp;

#ifdef USE_SOLINGER
  	struct linger linger;
#endif

	g_return_val_if_fail(dev_mng != NULL, NULL);

	memset (&hints, 0, sizeof (struct addrinfo));
	hints.ai_family = AF_UNSPEC; 
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE | AI_CANONNAME;
	hints.ai_protocol = 0;
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	ret = getaddrinfo(
		dev_mng->address, dev_mng->service, &hints, &result
	);
	if (ret != 0)
    	goto close_error;

	for (rp = result; rp; rp = rp->ai_next)
	{
		sockfd = socket(
			rp->ai_family, rp->ai_socktype, rp->ai_protocol
		);
		if (sockfd < 0)
			continue;

		tag_onoff = 1;
		setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
			(void *)&tag_onoff, sizeof(tag_onoff));

		if (!bind(sockfd, rp->ai_addr, rp->ai_addrlen))
			break;

		close(sockfd);
		sockfd = -1;
	}

	freeaddrinfo(result);

	if (sockfd == -1)
		goto close_error;

#ifdef USE_SOLINGER
	linger.l_onoff = 1;
	linger.l_linger = 5;
	if (setsockopt(sockfd, SOL_SOCKET, SO_LINGER, (void *)&linger, 
		sizeof (linger)) < 0)
    goto close_error;
#endif

	set_fd_flags(sockfd, O_NONBLOCK);

	if (listen(sockfd, MAX_LISTEN_BACKLOG) < 0)
		goto close_error;

	e_listen = g_event_new(sizeof(GEvent), sockfd, EV_READ);

	return e_listen;

close_error:
	if (sockfd >= 0)
		close (sockfd);
    return NULL;
}


static void
nmp_rtsp_device_listen_watch_fin(GEvent *ev)
{
	NmpDevMng *dev_mng;

	dev_mng = (NmpDevMng*)g_event_u(ev);
	nmp_rtsp_device_mng_unref(dev_mng);
}


static GEvent *
nmp_rtsp_device_mng_create_watch(NmpDevMng *dev_mng)
{
	GEvent *e_listen;
	g_assert(dev_mng != NULL);

	e_listen = nmp_rtsp_dev_mng_create_listen_ev(dev_mng);
	if (e_listen == NULL)
		goto no_channel;

	nmp_rtsp_device_mng_ref(dev_mng);

	g_event_set_callback(e_listen, nmp_rtsp_device_mng_io_func,
		dev_mng, nmp_rtsp_device_listen_watch_fin);

	return e_listen;

no_channel:
	nmp_warning("Failed to create device listen IO event-obj!");
	return NULL;
}


void
nmp_rtsp_device_mng_remove(NmpMediaDevice *device)
{
	g_assert(device != NULL);
	NmpDevMng *mng = device->private_data;

	nmp_rtsp_device_mng_remove_dev(mng, device);
}


guint
nmp_rtsp_device_mng_attach(NmpDevMng *dev_mng)
{
	GEvent *e_listen;
	g_assert(dev_mng != NULL);

	e_listen = nmp_rtsp_device_mng_create_watch(dev_mng);
	if (G_UNLIKELY(!e_listen))
		return 0;

	g_scheduler_add(e_listen, 1);
	g_event_unref(e_listen);

	return 1;
}


NmpMediaDevice *
nmp_rtsp_device_mng_find_and_get_dev(NmpDevMng *dev_mng,
	const gchar *id)
{
	g_assert(dev_mng != NULL && id != NULL);

	return nmp_rtsp_device_pool_find_and_get_dev(
		dev_mng->dev_pool, id
	);
}


//:~ End
