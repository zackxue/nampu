
#include "nmp_net_impl.h"
#include "nmp_resolve_host.h"
#include "nmp_config_backup.h"
#include "nmp_config_handler.h"
#include "nmp_config_srv_impl.h"


static int establish_io(nmpio_t *io, void *init_data)
{
    return config_new_client_connect((cfg_service_basic_t*)init_data, io);
}
static void finalize_io(nmpio_t *io, int err, void *init_data)
{
    config_finalize_client((config_service_t*)init_data, io, err);
}

static void config_foreach_service(void *orig, void *custom)
{
    struct service *srv = (struct service*)orig;
    struct service_template *basic = (struct service_template*)custom;
    (*basic->check_service)(basic, srv);
    return ;
}
static int on_config_service_timer(void *date)
{
    cfg_service_basic_t *cfg_basic = (cfg_service_basic_t*)date;
    config_service_list_t *cfg_srv_list = cfg_basic->srv_list;

    nmp_mutex_lock(cfg_srv_list->lock);
    nmp_list_foreach(cfg_srv_list->list, config_foreach_service, (void*)cfg_basic);
    config_reap_timeout_client(cfg_basic);
    nmp_mutex_unlock(cfg_srv_list->lock);

    return 0;
}

static int config_init_service(struct service_template *self)
{
    proxy_config_t cfg;
    cfg_service_basic_t *cfg_basic;

    NMP_ASSERT(NULL != self);

    cfg_basic = (cfg_service_basic_t*)self;

    cfg_basic->_2lp_net = create_2lp_net_object(establish_io, finalize_io);
    if (!cfg_basic->_2lp_net)
        goto NET_ERROR;

    cfg_basic->pack_opt = nmp_new(packet_opt_t, 1);
    if (cfg_basic->pack_opt)
    {
        memcpy((void*)cfg_basic->pack_opt, &pkt_opt, sizeof(pkt_opt));
        cfg_basic->pack_opt->proto.init_data = (void*)cfg_basic;
    }
    else
        goto PACKE_ERROR;

    get_proxy_config(&cfg);
    cfg_basic->listener = create_local_listener(cfg_basic->_2lp_net, 
                            &cfg_basic->pack_opt->proto, cfg.cmd_port);
    if(cfg_basic->listener)
    {
        cfg_basic->clt_me = create_msg_engine(TRUE, 0);
        config_register_all_msg_handlers(cfg_basic->clt_me);

        cfg_basic->usr_list = config_create_user_list();
        cfg_basic->fct_list = config_create_factory_list();
        cfg_basic->dev_list = config_create_device_list();

        cfg_basic->srv_list = nmp_new(config_service_list_t, 1);
        cfg_basic->srv_list->lock = nmp_mutex_new();
        cfg_basic->srv_list->list = NULL;

        cfg_basic->timer = nmp_set_timer(1000, on_config_service_timer, (void*)cfg_basic);
    }
    else
        goto LISTEN_ERROR;

    init_config_backup();
    return 0;

NET_ERROR:
    return -1;

PACKE_ERROR:
    destory_2lp_net_object(cfg_basic->_2lp_net);
    return -1;

LISTEN_ERROR:
    show_debug("create local listener failure!!!\n");
    destory_2lp_net_object(cfg_basic->_2lp_net);
    nmp_del(cfg_basic->pack_opt, packet_opt_t, 1);
    cfg_basic->pack_opt = NULL;
    return -1;
}
static int config_cleanup_service(struct service_template *self)
{
    cfg_service_basic_t *cfg_basic;
    NMP_ASSERT(NULL != self);

    cfg_basic = (cfg_service_basic_t*)self;

    if (cfg_basic->timer)
        nmp_del_timer(cfg_basic->timer);

    if (cfg_basic->srv_list->lock)
        nmp_mutex_free(cfg_basic->srv_list->lock);

    nmp_del(cfg_basic->srv_list, config_service_list_t, 1);
    cfg_basic->srv_list = NULL;

    destory_msg_engine(cfg_basic->clt_me);
    destory_local_listener(cfg_basic->_2lp_net, cfg_basic->listener);
    destory_2lp_net_object(cfg_basic->_2lp_net);
    nmp_del(cfg_basic->pack_opt, packet_opt_t, 1);
    cfg_basic->pack_opt = NULL;

    return 0;
}
static struct service *config_create_service(struct service_template *self, 
        void *init_data)
{
    config_service_t *cfg_srv;
    NMP_ASSERT(NULL != self);

    cfg_srv = (config_service_t*)nmp_new0(config_service_t, 1);
    cfg_srv->base.tm = self;

    cfg_srv->client.lock = nmp_mutex_new();
    cfg_srv->client.io = (nmpio_t*)init_data;

    cfg_srv->client.ttl = DEFAULT_TTL;
    cfg_srv->client.ttd = cfg_srv->client.ttl;
    cfg_srv->client.seq = 0;
    cfg_srv->client.keep_alive_freq = 0;
    cfg_srv->client.user_info.status = LOGOUT;

    return (struct service*)cfg_srv;
}
static void config_delete_service(struct service_template *self, 
        struct service *srv)
{
    nmpio_t *old = NULL;
    client_t *client;
    config_service_t *cfg_srv;
    cfg_service_basic_t *cfg_basic;

    NMP_ASSERT(self && srv);

    cfg_basic = (cfg_service_basic_t*)self;
    cfg_srv = (config_service_t*)srv;
    client = &cfg_srv->client;

    nmp_mutex_lock(client->lock);
    if (client->io)
    {
        old = client->io;
        client->io = NULL;
        client->user_info.status = LOGOUT;
    }
    nmp_mutex_unlock(client->lock);

    nmp_mutex_free(client->lock);

    if (old)
        nmp_net_unref_io(old);

    nmp_del(cfg_srv, config_service_t, 1);

    return ;
}
static int config_control_service(struct service_template *self, 
        struct service *srv, int cmd, void *user)
{
    NMP_ASSERT(self && srv);

    switch (cmd)
    {
        default:
            break;
    }

    return 0;
}
static int 
config_check_service(struct service_template *self, struct service *srv)
{
    cfg_service_basic_t *cfg_basic;
    config_service_t *cfg_srv;
    client_t *client;

    NMP_ASSERT(self && srv);

    cfg_basic = (cfg_service_basic_t*)self;
    cfg_srv = (config_service_t*)srv;
    client = &cfg_srv->client;

    nmp_mutex_lock(client->lock);
    if (0 >= --client->ttd)
    {
        client->user_info.status = LOGOUT;
    }
    nmp_mutex_unlock(client->lock);

    return 0;
}

cfg_service_basic_t cfg_srv_basic = 
{
    {
        DEF_CONFIG_SERVICE_NAME,
        config_init_service,
        config_cleanup_service,
        config_create_service,
        config_delete_service,
        config_control_service,
        config_check_service,
    },
    NULL
};

static void config_notify_each_client(void *orig, void *custom)
{
    int flag = -1;
    nmpio_t *io = NULL;

    config_service_t *cfg_srv = (config_service_t*)orig;
    client_t *client = &cfg_srv->client;
    msg_t *msg = (msg_t*)custom;

    nmp_mutex_lock(client->lock);
    if (LOGIN_SUCCESS == client->user_info.status)
    {
        if (BROADCAST_DEVICE_STATUS_ID == MSG_ID(msg))
        {
            prx_device_state *state = MSG_DATA(msg);
            if (state->dev_id >= client->range.start && 
                state->dev_id <= client->range.end)
            {
                flag = 0;
            }
        }
        else
            flag = 0;

        if (!flag && client->io)
        {
            io = nmp_net_ref_io(client->io);
            msg->seq = client->seq++;
        }
    }
    nmp_mutex_unlock(client->lock);

    if (io)
    {
        msg->io = io;
        send_msg(msg);

        nmp_net_unref_io(io);
        msg->io = NULL;
    }
}
void config_broadcast_event(cfg_service_basic_t *cfg_basic, msg_t *bc_msg)
{
    config_service_list_t *cfg_srv_list = cfg_basic->srv_list;

    nmp_mutex_lock(cfg_srv_list->lock);
    nmp_list_foreach(cfg_srv_list->list, config_notify_each_client, (void*)bc_msg);
    nmp_mutex_unlock(cfg_srv_list->lock);

    return ;
}

