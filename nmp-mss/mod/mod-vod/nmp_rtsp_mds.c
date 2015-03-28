#include "nmp_rtsp_mds.h"
#include "rtsp-server.h"
#include "nmp_sysctl.h"
#include "nmp_debug.h"


typedef struct __NmpMdsPoolPriv NmpMdsPoolPriv;
struct __NmpMdsPoolPriv
{
	RTSP_server *server;
};


typedef struct __NmpMdsPri NmpMdsPri;
struct __NmpMdsPri
{
	Reverse_connector *r_cntr;
};


static gint
nmp_rtsp_mds_pool_init(NmpMdsPool *p)
{
	static NmpMdsPoolPriv s_priv;

	s_priv.server = rtsp_server_new();
	rtsp_server_set_port(s_priv.server, 6550);

	if (rtsp_server_bind_port(s_priv.server))
	{
		nmp_warning(
			"rtsp_server_bind_port() failed."
		);
		FATAL_ERROR_EXIT;
	}

	p->private = &s_priv;
	nmp_print("rtsp_server_bind_port() ok.");

	return 0;
}


static gint
nmp_rtsp_mds_init(NmpMds *mds)
{
	NmpMdsPri *pri;

	pri = g_new0(NmpMdsPri, 1);
	mds->private = pri;
	return 0;
}


static void
nmp_rtsp_mds_connexp_cb(Reverse_connector *cntr, gint err, gpointer user_data)
{
	NmpMds *mds = (NmpMds*)user_data;
	NmpMdsPool	*pool;
	NmpMdsPoolPriv *p_priv;

	pool = mds->pool;
	BUG_ON(!pool);

	p_priv = (NmpMdsPoolPriv*)pool->private;
	BUG_ON(!p_priv);

	if (err != E_CONN_OK)
	{
		if (!g_atomic_int_compare_and_exchange(&mds->connected, 1, 0))
			return;

		rtsp_server_release_reverse_connector(p_priv->server, cntr);
		nmp_warning(
			"vod: connect mds '%s' '%s:%d' failed, err:'%d'.",
			mds->id,
			mds->ip,
			mds->port,
			err
		);

		nmp_mds_unref(mds);
		return;
	}

	nmp_print(
		"vod: connect mds '%s' '%s:%d' ok.",
		mds->id,
		mds->ip,
		mds->port
	);
	return;
}


static gint
nmp_rtsp_mds_connect(NmpMds *mds)
{
	Reverse_connector *r_cntr;
	NmpMdsPool	*pool;
	NmpMdsPoolPriv *p_priv;

	pool = mds->pool;
	BUG_ON(!pool);

	p_priv = (NmpMdsPoolPriv*)pool->private;
	BUG_ON(!p_priv);

	nmp_print(
		"vod: connecting mds '%s' '%s:%d'.",
		mds->id,
		mds->ip,
		mds->port
	);

	nmp_mds_ref(mds);

	r_cntr = rtsp_server_reverse_connect(
		p_priv->server,
		mds->ip,
		mds->port,
		(gchar*)nmp_get_sysctl_value(SC_MSS_ID),
		30,
		0,	/* TCP */
		nmp_rtsp_mds_connexp_cb,
		mds
	);

	if (!r_cntr)
	{
		nmp_mds_unref(mds);
		return 1;
	}

	((NmpMdsPri*)mds->private)->r_cntr = r_cntr;
	return 0;
}


static gint
nmp_rtsp_mds_disconnect(NmpMds *mds)
{
	Reverse_connector *r_cntr;
	NmpMdsPool	*pool;
	NmpMdsPoolPriv *p_priv;

	pool = mds->pool;
	BUG_ON(!pool);

	p_priv = (NmpMdsPoolPriv*)pool->private;
	BUG_ON(!p_priv);

	if (!g_atomic_int_compare_and_exchange(&mds->connected, 1, 0))
		return 0;

	r_cntr = ((NmpMdsPri*)mds->private)->r_cntr;
	BUG_ON(!r_cntr);

	rtsp_server_release_reverse_connector(p_priv->server, r_cntr);
	nmp_mds_unref(mds);		/* fixme! race against cb() */

	nmp_print(
		"Vod: disconnect mds '%s' '%s:%d'.",
		mds->id,
		mds->ip,
		mds->port
	);

	return 0;
}


static gint
nmp_rtsp_mds_fin(NmpMds *mds)
{
	g_free(mds->private);
	return 0;
}


NmpMdsOps nmp_rtsp_mds_ops =
{
	.init_pool	= nmp_rtsp_mds_pool_init,
	.init_inst	= nmp_rtsp_mds_init,
	.connect	= nmp_rtsp_mds_connect,
	.disconnect	= nmp_rtsp_mds_disconnect,
	.fin		= nmp_rtsp_mds_fin
};

//:~ End
