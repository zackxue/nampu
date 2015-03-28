#include <nmp_net.h>
#include <nmp_timer.h>
#include "nmp_errno.h"
#include "nmp_debug.h"
#include "nmp_proto.h"
#include "nmp_cms_conn.h"
#include "nmp_mod_cms.h"
#include "nmp_sysctl.h"
#include "nmp_message.h"


#define DEFAULT_CONNECTING_SECS		5
#define DEFAULT_WAITING_SECS		5
#define HB_PER_TTL					3
#define DEFAULT_TTL					15
#define MSS_PORT_OFFSET				3

#define UNREG_PACK_SENT				1
#define UNREG_PACK_UNSENT			-1


static __inline__ NmpConnState
nmp_cms_conn_get_state(NmpCmsConn *conn, gint *timer)
{
	NmpConnState state;

	g_mutex_lock(conn->lock);
	state = conn->state;
	*timer = ++conn->state_timer;
	g_mutex_unlock(conn->lock);

	return state;
}


gint nmp_cms_conn_state(NmpCmsConn *conn)
{
	if (conn && conn->state == CONNECTED && !conn->unregistered)
		return 0;

	return 1;
}


static __inline__ void
__nmp_cms_conn_set_state(NmpCmsConn *conn, NmpConnState state)
{
	conn->state = state;
	conn->state_timer = 0;
	conn->ttd = conn->ttl;
}


static __inline__ void
nmp_cms_conn_set_state(NmpCmsConn *conn, NmpConnState state)
{
	g_mutex_lock(conn->lock);
	__nmp_cms_conn_set_state(conn, state);
	g_mutex_unlock(conn->lock);
}


static __inline__ void
nmp_cms_conn_set_bridge(NmpModCms *mod, NmpCmsConn *conn, NmpNetIO *bridge)
{
	NmpNetIO *old_bridge;

	g_mutex_lock(conn->lock);

	old_bridge = conn->bridge;
	conn->bridge = bridge;

	conn->ttl = DEFAULT_TTL;
	conn->ttd = conn->ttl;
	conn->unregistered = UNREG_PACK_UNSENT;

	g_mutex_unlock(conn->lock);

	if (old_bridge)
	{
		nmp_net_kill_io(((NmpModAccess*)mod)->net, old_bridge);
		nmp_net_unref_io(old_bridge);
	}
}


static __inline__ void
nmp_cms_bridge_disconnect(NmpModCms *mod, NmpCmsConn *conn)
{
	nmp_cms_conn_set_bridge(mod, conn, NULL);
}


static void
jpd_mod_cms_connect_ok(NmpNetIO *bridge, void *init_data)
{
	NmpModCms *mod = (NmpModCms*)init_data;

	nmp_print("Connect cms ok.");
	nmp_cms_conn_set_state(mod->cms_conn, CONNECTED);
}


static __inline__ void
nmp_cms_bridge_connect(NmpModCms *mod, NmpCmsConn *conn)
{
	struct sockaddr_in cms_addr;
	gint err;
	NmpNetIO *bridge;
	NmpConnection *c;

	err = nmp_resolve_host(&cms_addr, conn->ip, conn->port);
	if (err)
	{
		nmp_warning(
			"Resolve cms ip failed while connecting cms."
		);
		nmp_cms_conn_set_state(conn, DISCONNECTING);
		return;
	}

	c = nmp_connection_new(NULL, CF_TYPE_TCP|CF_FLGS_NONBLOCK, &err);
	if (!c)
	{
		nmp_warning(
			"Create connection while connecting cms, err: '%d'.",
			err
		);
		nmp_cms_conn_set_state(conn, DISCONNECTING);
		return;
	}

	err = nmp_connection_connect(c, (struct sockaddr*)&cms_addr);
	if (!err || err == -EINPROGRESS)
	{
		if (!err)
		{
			nmp_print(
				"Connect cms ok"
			);
			nmp_cms_conn_set_state(conn, CONNECTED);
		}
		else
		{
			nmp_print(
				"Connecting cms ..."
			);
			nmp_cms_conn_set_state(conn, CONNECTING);
		}

		bridge = nmp_net_create_io(((NmpModAccess*)mod)->net, c,
			jpd_mod_cms_connect_ok, &err);
		if (!bridge)	/* c was destroyed */
		{
			nmp_warning(
				"Create net io failed while connecting cms, err:'%d'.", err
			);
			nmp_cms_conn_set_state(conn, DISCONNECTING);
			return;
		}

		nmp_cms_conn_set_bridge(mod, conn, bridge);
		return;
	}

	nmp_cms_conn_set_state(conn, DISCONNECTING);
	nmp_connection_close(c);	
}


static __inline__ void
nmp_cms_conn_connect_die(NmpModCms *mod, NmpCmsConn *conn)
{
	nmp_cms_bridge_disconnect(mod, conn);
	nmp_cms_conn_set_state(conn, DISCONNECTING);
}


void
nmp_cms_conn_io_fin(NmpCmsConn *conn, NmpNetIO *bridge)
{
	gint unrefed = 1;

	g_mutex_lock(conn->lock);
	if (conn->bridge == bridge)
	{
		__nmp_cms_conn_set_state(conn, DISCONNECTING);
		conn->bridge = NULL;
		unrefed = 0;
	}
	g_mutex_unlock(conn->lock);

	if (!unrefed)
		nmp_net_unref_io(bridge);
}


static __inline__ gint
nmp_cms_conn_send_msg(NmpModCms *mod, NmpCmsConn *conn, NmpSysMsg *msg)
{
	NmpNetIO *bridge;

	g_mutex_lock(conn->lock);
	bridge = conn->bridge;
	if (bridge)
	{
		nmp_net_ref_io(bridge);
	}
	g_mutex_unlock(conn->lock);

	if (!bridge)
		return -E_BADFD;

	nmp_sysmsg_attach_io(msg, bridge);
	nmp_app_obj_deliver_in((NmpAppObj*)mod, msg);

	nmp_net_unref_io(bridge);

    return 0;
}


static __inline__ void
nmp_cms_conn_make_hb_msg(NmpCmsConn *conn, NmpSysMsg **msg)
{
	NmpMssHeart hb;

	memset(&hb, 0, sizeof(hb));
	strncpy(hb.mss_id, (gchar*)nmp_get_sysctl_value(SC_MSS_ID), 
		MSS_ID_LEN - 1);

	*msg = nmp_sysmsg_new_2(MESSAGE_KEEPALIVE_CMS, &hb,
		sizeof(hb), ++conn->seq_generator);
}


static __inline__ void
nmp_cms_conn_make_reg_msg(NmpCmsConn *conn, NmpSysMsg **msg)
{
	NmpMssRegister reg;

	memset(&reg, 0, sizeof(reg));
	strncpy(reg.mss_id, (gchar*)nmp_get_sysctl_value(SC_MSS_ID), 
		MSS_ID_LEN - 1);

	*msg = nmp_sysmsg_new_2(MESSAGE_REGISTER_CMS, &reg,
		sizeof(reg), ++conn->seq_generator);
}


static __inline__ gint
__nmp_cms_conn_check_online_state(NmpCmsConn *conn, NmpSysMsg **msg)
{
	if (--conn->ttd <= 0)
		return -1;	/* dying */

	if (!conn->unregistered)
	{
		if (!(conn->state_timer % conn->hb))
		{
			nmp_cms_conn_make_hb_msg(conn, msg);
			return 1;
		}
	}
	else
	{
		if (conn->unregistered == UNREG_PACK_UNSENT)
		{
			nmp_cms_conn_make_reg_msg(conn, msg);
			conn->unregistered = UNREG_PACK_SENT;
			return 1;
		}
	}

	return 0;
}


static __inline__ void
nmp_cms_conn_check_online_state(NmpModCms *mod, NmpCmsConn *conn)
{
	NmpSysMsg *msg;
	gint ret;

	g_mutex_lock(conn->lock);
	ret = __nmp_cms_conn_check_online_state(conn, &msg);
	g_mutex_unlock(conn->lock);

	if (ret < 0)
	{
		/**
		 * set conn ttd do this else well  
		 * nmp_cms_conn_connect_die(mod, conn);*/
		return;
	}

	if (ret > 0 && msg)
	{
		nmp_cms_conn_send_msg(mod, conn, msg);
		return;
	}
}


void
nmp_cms_conn_check_state(NmpModCms *mod, NmpCmsConn *conn)
{
	gint state_timer;

	switch (nmp_cms_conn_get_state(conn, &state_timer))
	{
	case DISCONNECTED:
		nmp_cms_bridge_connect(mod, conn);
		break;

	case CONNECTING:
		if (state_timer > DEFAULT_CONNECTING_SECS)
			nmp_cms_conn_connect_die(mod, conn);
		break;

	case CONNECTED:
		nmp_cms_conn_check_online_state(mod, conn);
		break;

	case DISCONNECTING:
		if (state_timer > DEFAULT_WAITING_SECS)
			nmp_cms_conn_set_state(conn, DISCONNECTED);
		break;

	default:
		break;	
	}
}


void
nmp_cms_conn_is_alive(NmpCmsConn *conn)
{
	g_mutex_lock(conn->lock);
	if (!conn->unregistered)
		conn->ttd = conn->ttl;
	g_mutex_unlock(conn->lock);
}


void
nmp_cms_conn_set_registered(NmpCmsConn *conn, NmpmssRegisterRes *res)
{
	NmpNetIO *bridge;

	nmp_print(
		"Mss '%s' register cms ok, keepalive '%d'.",
		(gchar*)nmp_get_sysctl_value(SC_MSS_ID),
		res->keep_alive_time
	);

	g_mutex_lock(conn->lock);
	conn->unregistered = 0;
	conn->hb = res->keep_alive_time > 0 ?
		res->keep_alive_time : DEFAULT_TTL/HB_PER_TTL;
	conn->ttl = HB_PER_TTL*conn->hb;
	conn->ttd = conn->ttl;
	conn->rtsp_port = 0;//res->rtsp_port;

	bridge = conn->bridge;
	if (bridge)
		nmp_net_ref_io(bridge);

	g_mutex_unlock(conn->lock);

	if (bridge)
	{
		nmp_net_set_io_ttd(bridge, 1000*conn->ttl);
		nmp_net_unref_io(bridge);
	}
}


NmpNetIO *
nmp_cms_conn_get_io(NmpCmsConn *conn)
{
	NmpNetIO *bridge = NULL;

	if (!conn)
		return NULL;

	g_mutex_lock(conn->lock);
	if (conn->bridge)
	{
		bridge = conn->bridge;
		nmp_net_ref_io(bridge);
	}
	g_mutex_unlock(conn->lock);

	return bridge;
}


void
nmp_cms_conn_set_register_failed(NmpCmsConn *conn, gint err)
{
	nmp_warning(
		"Mss '%s' register cms failed, err: '%d'",
		(gchar*)nmp_get_sysctl_value(SC_MSS_ID),
		err
	);
	nmp_cms_conn_set_state(conn, DISCONNECTING);
}


static __inline__ void
nmp_init_cms_module(NmpCmsConn *conn)
{
	conn->bridge = NULL;
	strncpy(conn->ip, (gchar*)nmp_get_sysctl_value(SC_CMS_HOST), MAX_IP_LEN - 1);
	conn->port = (gint)nmp_get_sysctl_value(SC_BASE_PORT) + MSS_PORT_OFFSET;
	conn->lock = g_mutex_new();
	conn->state = DISCONNECTED;
	conn->state_timer = 0;
	conn->ttl = DEFAULT_TTL;
	conn->ttd = conn->ttl;
	conn->hb = DEFAULT_TTL/HB_PER_TTL;
	conn->unregistered = UNREG_PACK_UNSENT;
	conn->seq_generator = 0;
}


gint
nmp_cms_conn_post_message(NmpModCms *mod, NmpCmsConn *conn, NmpSysMsg *msg)
{/* best effort */
	gint err = 0;
	G_ASSERT(conn != NULL);

	if (conn->unregistered)
		err = -E_BADFD;
	else
		err = nmp_cms_conn_send_msg(mod, conn, msg);

	return err;
}


gint
nmp_cms_conn_rtsp_port(NmpCmsConn *conn)
{
	gint port;
	G_ASSERT(conn != NULL);

	g_mutex_lock(conn->lock);
	port = conn->rtsp_port;
	g_mutex_unlock(conn->lock);

	nmp_print(
		"Mss rtsp port '%d'", port
	);
	return port;
}


NmpCmsConn *
nmp_create_cms_conn( void )
{
	NmpCmsConn *conn;

	conn = g_new0(NmpCmsConn, 1);
	nmp_init_cms_module(conn);
	return conn;
}


//:~ End
