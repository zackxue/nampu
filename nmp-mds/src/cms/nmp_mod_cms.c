#include <nmp_net.h>
#include <nmp_timer.h>
#include "nmp_errno.h"
#include "nmp_debug.h"
#include "nmp_proto.h"
#include "nmp_sysctl.h"
#include "nmp_msg_translate.h"
#include "nmp_media_server.h"
#include "nmp_msg_table.h"

#define DEFAULT_CONNECTING_SECS		5
#define DEFAULT_WAITING_SECS		5
#define HB_PER_TTL					3
#define DEFAULT_TTL					15
#define MDS_PORT_OFFSET				2

#define UNREG_PACK_SENT				1
#define UNREG_PACK_UNSENT			-1

typedef enum
{
	DISCONNECTED,
	CONNECTING,
	CONNECTED,
	DISCONNECTING
}JpfConnState;

struct __JpfModCms
{
	NmpNet		*network;
	NmpNetIO	*bridge;		/* communication bridge */

	gchar		ip[MAX_IP_LEN];
	gint		port;
	gint		rtsp_port;
	gint		pu_port;

	GMutex		*lock;
	JpfConnState state;
	guint		state_timer;
	guint 		ttl;
	guint 		ttd;
	guint		hb;
	guint 		unregistered;
	guint		seq_generator;
	JpfMsgTable	*msg_table;
	guint		check_timer;
};


static __inline__ JpfConnState
nmp_mod_cms_get_state(JpfModCms *mod, gint *timer)
{
	JpfConnState state;

	g_mutex_lock(mod->lock);
	state = mod->state;
	*timer = ++mod->state_timer;
	g_mutex_unlock(mod->lock);

	return state;
}


gint
nmp_mds_cms_state(JpfModCms *mod)
{
	if (mod && mod->state == CONNECTED && !mod->unregistered)
		return 0;
	return 1;
}


static __inline__ void
__nmp_mod_cms_set_state(JpfModCms *mod, JpfConnState state)
{
	mod->state = state;
	mod->state_timer = 0;
	mod->ttd = mod->ttl;
}

static __inline__ void
nmp_mod_cms_set_state(JpfModCms *mod, JpfConnState state)
{
	g_mutex_lock(mod->lock);
	__nmp_mod_cms_set_state(mod, state);
	g_mutex_unlock(mod->lock);
}


static __inline__ void
nmp_mod_cms_set_bridge(JpfModCms *mod, NmpNetIO *bridge)
{
	NmpNetIO *old_bridge;

	g_mutex_lock(mod->lock);

	old_bridge = mod->bridge;
	mod->bridge = bridge;

	mod->ttl = DEFAULT_TTL;
	mod->ttd = mod->ttl;
	mod->unregistered = UNREG_PACK_UNSENT;

	g_mutex_unlock(mod->lock);

	if (old_bridge)
	{
		nmp_net_kill_io(mod->network, old_bridge);
		nmp_net_unref_io(old_bridge);
	}
}


static __inline__ void
nmp_cms_bridge_disconnect(JpfModCms *mod)
{
	nmp_mod_cms_set_bridge(mod, NULL);
}


static void
jpd_mod_cms_connect_ok(NmpNetIO *bridge, void *init_data)
{
	JpfModCms *mod = (JpfModCms*)init_data;

	nmp_print("Connect cms ok.");
	nmp_mod_cms_set_state(mod, CONNECTED);
}


static __inline__ void
nmp_cms_bridge_connect(JpfModCms *mod)
{
	struct sockaddr_in cms_addr;
	gint err;
	NmpNetIO *bridge;
	NmpConnection *conn;

	err = nmp_resolve_host(&cms_addr, mod->ip, mod->port);
	if (err)
	{
		nmp_warning(
			"Resolve cms ip failed while connecting cms."
		);
		nmp_mod_cms_set_state(mod, DISCONNECTING);
		return;
	}

	conn = nmp_connection_new(NULL, CF_TYPE_TCP|CF_FLGS_NONBLOCK, &err);
	if (!conn)
	{
		nmp_warning(
			"Create connection while connecting cms, err: '%d'.",
			err
		);
		nmp_mod_cms_set_state(mod, DISCONNECTING);
		return;
	}

	err = nmp_connection_connect(conn, (struct sockaddr*)&cms_addr);
	if (!err || err == -EINPROGRESS)
	{
		if (!err)
		{
			nmp_print(
				"Connect cms ok"
			);
			nmp_mod_cms_set_state(mod, CONNECTED);
		}
		else
		{
			nmp_print(
				"Connecting cms ..."
			);
			nmp_mod_cms_set_state(mod, CONNECTING);
		}

		bridge = nmp_net_create_io(mod->network, conn,
			jpd_mod_cms_connect_ok, &err);
		if (!bridge)	/* conn was destroyed */
		{
			nmp_warning(
				"Create net io failed while connecting cms, err:'%d'.", err
			);
			nmp_mod_cms_set_state(mod, DISCONNECTING);
			return;
		}

		nmp_mod_cms_set_bridge(mod, bridge);
		return;
	}

	nmp_mod_cms_set_state(mod, DISCONNECTING);
	nmp_connection_close(conn);	
}


static __inline__ void
nmp_mod_cms_connect_die(JpfModCms *mod)
{
	nmp_cms_bridge_disconnect(mod);
	nmp_mod_cms_set_state(mod, DISCONNECTING);
}


static __inline__ void
nmp_mod_cms_io_finalized(JpfModCms *mod, NmpNetIO *bridge)
{
	gint unrefed = 1;

	g_mutex_lock(mod->lock);
	if (mod->bridge == bridge)
	{
		__nmp_mod_cms_set_state(mod, DISCONNECTING);
		mod->bridge = NULL;
		unrefed = 0;
	}
	g_mutex_unlock(mod->lock);

	if (!unrefed)
		nmp_net_unref_io(bridge);
}


static __inline__ gint
nmp_mod_cms_send_msg(JpfModCms *mod, JpfMdsMsg *msg)
{
	NmpNetIO *bridge;
	gint ret;

	g_mutex_lock(mod->lock);
	bridge = mod->bridge;
	if (bridge)
	{
		nmp_net_ref_io(bridge);
	}
	g_mutex_unlock(mod->lock);

	if (!bridge)
		return -E_BADFD;

	ret = nmp_net_write_io(bridge, msg);	/* trigger io_close() */
	if (G_UNLIKELY(ret < 0))	/* nerver send with mod->lock held!! */
	{
		nmp_warning(
			"Send packet to cms failed, err:%d", -ret
		);
	}

	nmp_net_unref_io(bridge);
    nmp_free_msg(msg);
    return ret;
}


static __inline__ void
nmp_mod_cms_make_hb_msg(JpfModCms *mod, JpfMdsMsg **msg)
{
	JpfMediaServer *server;
	gchar *server_id;
	JpfMdsHeart hb_info;

	server = nmp_media_server_get();
	server_id = nmp_media_server_get_id(server);

	memset(&hb_info, 0, sizeof(hb_info));
	strncpy(hb_info.mds_id, server_id, MDS_ID_LEN - 1);
	strncpy(hb_info.cms_ip, (gchar*)nmp_get_sysctl_value(SC_CMS_HOST), MAX_IP_LEN - 1);

	*msg = nmp_alloc_msg(MESSAGE_MDU_HEART, &hb_info,
		sizeof(hb_info), ++mod->seq_generator);
}


static __inline__ void
nmp_mod_cms_make_reg_msg(JpfModCms *mod, JpfMdsMsg **msg)
{
	JpfMediaServer *server;
	gchar *server_id;
	JpfMdsRegister reg_info;

	server = nmp_media_server_get();
	server_id = nmp_media_server_get_id(server);

	memset(&reg_info, 0, sizeof(reg_info));
	strncpy(reg_info.mds_id, server_id, MDS_ID_LEN - 1);
	strncpy(reg_info.cms_ip, (gchar*)nmp_get_sysctl_value(SC_CMS_HOST), MAX_IP_LEN - 1);
printf("======>sizeof(JpfMdsRegister):%d, MDS_ID_LEN:%d, MAX_IP_LEN:%d\n", sizeof(JpfMdsRegister), MDS_ID_LEN, MAX_IP_LEN);
	*msg = nmp_alloc_msg(MESSAGE_MDU_REGISTER, &reg_info,
		sizeof(reg_info), ++mod->seq_generator);
}


static __inline__ gint
__nmp_mod_cms_check_online_state(JpfModCms *mod, JpfMdsMsg **msg)
{
	if (--mod->ttd <= 0)
		return -1;	/* dying */

	if (!mod->unregistered)
	{
		if (!(mod->state_timer % mod->hb))
		{
			nmp_mod_cms_make_hb_msg(mod, msg);
			return 1;
		}
	}
	else
	{
		if (mod->unregistered == UNREG_PACK_UNSENT)
		{
			nmp_mod_cms_make_reg_msg(mod, msg);
			mod->unregistered = UNREG_PACK_SENT;
			return 1;
		}
	}

	return 0;
}


static __inline__ void
nmp_mod_cms_check_online_state(JpfModCms *mod)
{
	JpfMdsMsg *msg;
	gint ret;

	g_mutex_lock(mod->lock);
	ret = __nmp_mod_cms_check_online_state(mod, &msg);
	g_mutex_unlock(mod->lock);

	if (ret < 0)
	{
		nmp_mod_cms_connect_die(mod);
		return;
	}

	if (ret > 0 && msg)
	{
		nmp_mod_cms_send_msg(mod, msg);
		return;
	}
}


static __inline__ void
nmp_mod_cms_check_state(JpfModCms *mod)
{
	gint state_timer;

	switch (nmp_mod_cms_get_state(mod, &state_timer))
	{
	case DISCONNECTED:
		nmp_cms_bridge_connect(mod);
		break;

	case CONNECTING:
		if (state_timer > DEFAULT_CONNECTING_SECS)
			nmp_mod_cms_connect_die(mod);
		break;

	case CONNECTED:
		nmp_mod_cms_check_online_state(mod);
		break;

	case DISCONNECTING:
		if (state_timer > DEFAULT_WAITING_SECS)
			nmp_mod_cms_set_state(mod, DISCONNECTED);
		break;

	default:
		break;
	}
}


static __inline__ void
nmp_cms_bridge_init(JpfModCms *mod)
{
	nmp_cms_bridge_connect(mod);
}


static gboolean
nmp_mod_cms_on_check_timer(gpointer user_data)
{
	JpfModCms *mod = (JpfModCms*)user_data;
	G_ASSERT(mod != NULL);
printf("ooooooooooooooooooon cms check timer!!\n");
	nmp_mod_cms_check_state(mod);

	return TRUE;
}


static __inline__ void
nmp_mod_cms_is_alive(JpfModCms *mod)
{
	g_mutex_lock(mod->lock);
	if (!mod->unregistered)
		mod->ttd = mod->ttl;
	g_mutex_unlock(mod->lock);
}


static __inline__ void
nmp_mod_cms_set_registered(JpfModCms *mod, JpfmdsRegisterRes *res)
{
	NmpNetIO *bridge;

	g_mutex_lock(mod->lock);
	mod->unregistered = 0;
	mod->hb = res->keep_alive_time > 0 ?
		res->keep_alive_time : DEFAULT_TTL/HB_PER_TTL;
	mod->ttl = HB_PER_TTL*mod->hb;
	mod->ttd = mod->ttl;
	mod->rtsp_port = res->rtsp_port;
	mod->pu_port = res->pu_port;

	bridge = mod->bridge;
	if (bridge)
		nmp_net_ref_io(bridge);

	g_mutex_unlock(mod->lock);

	if (bridge)
	{
		nmp_net_set_io_ttd(bridge, 1000*mod->ttl);
		nmp_net_unref_io(bridge);
	}
}


static void
nmp_mod_cms_register_handler(JpfMdsMsg *msg, gpointer parm)
{
	JpfModCms *mod_cms;
	JpfmdsRegisterRes *res;
	gint res_code;
	JpfModCms *mod = (JpfModCms*)parm;
	G_ASSERT(parm != NULL);

	mod_cms = (JpfModCms*)parm;
	res = (JpfmdsRegisterRes*)MSG_DATA(msg);
	BUG_ON(!res);

	res_code = RES_CODE(res);
	if (!res_code)
	{
		nmp_print(
			"Mds register cms ok, keepalive '%d'.",
			res->keep_alive_time
		);
		nmp_mod_cms_set_registered(mod, res);
	}
	else
	{
		nmp_warning(
			"Mds register cms failed, err: '%d'",
			res_code
		);
		nmp_mod_cms_set_state(mod, DISCONNECTING);
	}
}


static void
nmp_mod_cms_heart_handler(JpfMdsMsg *msg, gpointer data)
{
}


static __inline__ void
nmp_mod_cms_register_msg_handler(JpfModCms *mod)
{
	G_ASSERT(mod->msg_table != NULL);

	nmp_msg_table_register(
		mod->msg_table,
		MESSAGE_MDU_REGISTER,
		nmp_mod_cms_register_handler
	);

	nmp_msg_table_register(
		mod->msg_table,
		MESSAGE_MDU_HEART,
		nmp_mod_cms_heart_handler
	);
}


static gint
nmp_mod_cms_recv(gpointer priv_data, NmpNetIO *net_io, gpointer msg)
{
	JpfModCms *mod = (JpfModCms*)priv_data;
	G_ASSERT(mod != NULL);

	nmp_mod_cms_is_alive(mod);
	nmp_msg_table_call(mod->msg_table, (JpfMdsMsg*)msg, mod);
	nmp_free_msg(msg);
	return 0;
}


static void
nmp_mod_cms_connect_fin(gpointer priv_data, NmpNetIO *io, gint err)
{
	JpfModCms *mod = (JpfModCms*)priv_data;

	nmp_mod_cms_io_finalized(mod, io);
}


static __inline__ void
nmp_init_cms_module(JpfModCms *mod)
{
	mod->network = nmp_net_new(&jxj_packet_proto, &jxj_xml_proto, mod);
	BUG_ON(!mod->network);
	nmp_net_set_reader(mod->network, nmp_mod_cms_recv);
	nmp_net_set_funcs(mod->network, NULL, nmp_mod_cms_connect_fin);
	mod->bridge = NULL;
	strncpy(mod->ip, (gchar*)nmp_get_sysctl_value(SC_CMS_HOST), MAX_IP_LEN - 1);
	mod->port = (gint)nmp_get_sysctl_value(SC_BASE_PORT) + MDS_PORT_OFFSET;
	mod->lock = g_mutex_new();
	mod->state = DISCONNECTED;
	mod->state_timer = 0;
	mod->ttl = DEFAULT_TTL;
	mod->ttd = mod->ttl;
	mod->hb = DEFAULT_TTL/HB_PER_TTL;
	mod->unregistered = UNREG_PACK_UNSENT;
	mod->seq_generator = 0;
	mod->msg_table = nmp_msg_table_new();
	nmp_mod_cms_register_msg_handler(mod);
	mod->check_timer = nmp_set_timer(1000, nmp_mod_cms_on_check_timer, mod);
}


static __inline__ void
nmp_mod_cms_wait(JpfModCms *mod)
{//TODO:
	while (1)
	{
		if (mod->unregistered)
			sleep(1);
		else
			break;
	}
}


gint
nmp_mod_cms_rtsp_port(JpfModCms *mod)
{
	gint port;
	G_ASSERT(mod != NULL);

	g_mutex_lock(mod->lock);
	port = mod->rtsp_port;
	g_mutex_unlock(mod->lock);

	nmp_print(
		"Mds rtsp port '%d'", port
	);
	return port;
}


gint
nmp_mod_cms_pu_port(JpfModCms *mod)
{
	gint port;
	G_ASSERT(mod != NULL);

	g_mutex_lock(mod->lock);
	port = mod->pu_port;
	g_mutex_unlock(mod->lock);

	nmp_print(
		"Mds device port '%d'", port
	);
	return port;
}


JpfModCms *
nmp_mod_cms_new( void )
{
	JpfModCms *mod;

	mod = g_new0(JpfModCms, 1);
	nmp_init_cms_module(mod);
	nmp_mod_cms_wait(mod);

	return mod;
}


//:~ End
