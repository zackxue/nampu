
#include "nmp_net_impl.h"
#include "nmp_proxy_device.h"

#include "nmp_config_handler.h"
#include "nmp_config_srv_impl.h"


static void config_send_msg_out(config_service_t *cfg_srv, msg_t *msg);


#if _DEBUG_
static void show_service_list_nolock(config_service_list_t *cfg_srv_list)
{
    nmp_list_t *list;
    int total_count = 0;
    struct config_service *cfg_srv;

    list = cfg_srv_list->list;
    while (list)
    {
        cfg_srv = (struct config_service*)nmp_list_data(list);

        nmp_mutex_lock(cfg_srv->client.lock);
        show_debug("\n");
        show_debug("      io: %p\n", cfg_srv->client.io);
        show_debug("     ttl: %d\n", cfg_srv->client.ttl);
        show_debug("     ttd: %d\n", cfg_srv->client.ttd);
        show_debug("     seq: %d\n", cfg_srv->client.seq);
        show_debug("    live: %d\n", cfg_srv->client.keep_alive_freq);
        show_debug("   state: %d\n", cfg_srv->client.user_info.status);
        show_debug("username: %s\n", cfg_srv->client.user_info.username);
        nmp_mutex_unlock(cfg_srv->client.lock);

        list = nmp_list_next(list);
        total_count++;
    }
    show_debug("total_count: %d<-------------------------------\n", total_count);
}

static void show_service_list(config_service_list_t *cfg_srv_list)
{
    nmp_mutex_lock(cfg_srv_list->lock);
    show_service_list_nolock(cfg_srv_list);
    nmp_mutex_unlock(cfg_srv_list->lock);
}
#endif

cfg_service_basic_t *
config_get_service_basic(config_service_t *cfg_srv)
{
    return (cfg_service_basic_t*)((struct service*)cfg_srv)->tm;
}

nmp_net_t *
config_get_2lp_net(config_service_t *cfg_srv)
{
    cfg_service_basic_t *cfg_basic;
    cfg_basic = (cfg_service_basic_t*)((struct service*)cfg_srv)->tm;
    return cfg_basic->_2lp_net;
}
static __inline__ packet_opt_t *
config_get_packet_opt(config_service_t *cfg_srv)
{
    cfg_service_basic_t *cfg_basic;
    cfg_basic = (cfg_service_basic_t*)((struct service*)cfg_srv)->tm;
    return cfg_basic->pack_opt;
}
msg_engine_t *
config_get_msg_engine(config_service_t *cfg_srv)
{
    cfg_service_basic_t *cfg_basic;
    cfg_basic = (cfg_service_basic_t*)((struct service*)cfg_srv)->tm;
    return cfg_basic->clt_me;
}

static void 
config_add_new_service(config_service_list_t *cfg_srv_list, 
        config_service_t *cfg_srv)
{
    NMP_ASSERT(cfg_srv_list && cfg_srv);

    nmp_mutex_lock(cfg_srv_list->lock);
    cfg_srv_list->list = nmp_list_add_tail(cfg_srv_list->list, cfg_srv);
    nmp_mutex_unlock(cfg_srv_list->lock);

    return ;
}

static __inline__ void 
config_remove_one_service_unlock(config_service_list_t *cfg_srv_list, 
        config_service_t *cfg_srv)
{
    cfg_srv_list->list = nmp_list_remove(cfg_srv_list->list, cfg_srv);
}
static void 
config_remove_one_service(config_service_list_t *cfg_srv_list, 
        config_service_t *cfg_srv)
{
    NMP_ASSERT(cfg_srv_list && cfg_srv);
    
    nmp_mutex_lock(cfg_srv_list->lock);
    config_remove_one_service_unlock(cfg_srv_list, cfg_srv);
    nmp_mutex_unlock(cfg_srv_list->lock);

    return ;
}
static __inline__ int 
config_client_is_register(client_t *client)
{
    int flag;
    nmp_mutex_lock(client->lock);
    if (LOGOUT == client->user_info.status)
        flag = 0;
    else
    {
        client->ttd = client->ttl;
        flag = 1;
    }
    nmp_mutex_unlock(client->lock);

    return flag;
}

static __inline__ void
process_client_register_request(config_service_t *cfg_srv, msg_t *msg)
{
    int ret;
    msg_t *resp_msg;
    JUserInfo *prx_one_user;

    client_t *client;
    client_t new_client;

    client = &cfg_srv->client;
    prx_one_user = (JUserInfo*)MSG_DATA(msg);

    memcpy(&new_client, client, sizeof(client_t));

    new_client.user_info.status = LOGOUT;
    memcpy(new_client.user_info.username, prx_one_user->username, 
        sizeof(new_client.user_info.username));
    memcpy(new_client.user_info.password, prx_one_user->password, 
        sizeof(new_client.user_info.password));

    ret = config_user_login_request((struct service*)cfg_srv, 
            &new_client.user_info);
    if (LOGIN_SUCCESS == ret)
    {
        nmp_mutex_lock(client->lock);

        new_client.keep_alive_freq = DEFAULT_HEARTBEAT_SECS;
        new_client.ttl = new_client.keep_alive_freq * DEFAULT_HEARTBEAT_TIMES;
        new_client.ttd = new_client.ttl;

        new_client.user_info.status = LOGIN_SUCCESS;
        memcpy(client, &new_client, sizeof(client_t));

        nmp_mutex_unlock(client->lock);
    }
    else if (LOGIN_FAILURE == ret)
    {/*注意: 在生成登录结果xml 时，原是根据status 值判断登录的
         结果: -1登录失败，0: 登录成功，1: 已登录;
         后来添加了设备的心跳时长，为了快速重用xml层源码，根据
         传入的keep_alive_freq值进行分析并生成登录结果xml : -1: 登录失败，
         0 : 已经登录，>0 : 登录成功
     */
        new_client.keep_alive_freq = 0;
    }
    else
        new_client.keep_alive_freq = -1;

    packet_opt_t *pack_opt = config_get_packet_opt(cfg_srv);
    resp_msg = (*pack_opt->create_special_packet)(&new_client.keep_alive_freq, 
                    PACK_CFG_REGISTER, msg->seq);
    if (resp_msg)
    {
        config_send_msg_out(cfg_srv, resp_msg);
        free_msg(resp_msg);
    }

    return ;
}

static __inline__ void
process_client_heartbeat_request(config_service_t *cfg_srv, msg_t *msg)
{
    msg_t *resp_msg;
    packet_opt_t *pack_opt = config_get_packet_opt(cfg_srv);
    resp_msg = (*pack_opt->create_special_packet)(&cfg_srv->client, 
                    PACK_CFG_HEARTBEAT, msg->seq);
    if (resp_msg)
    {
        config_send_msg_out(cfg_srv, resp_msg);
        free_msg(resp_msg);
    }

    return ;
}

static __inline__ int
config_process_special_msg(config_service_t *cfg_srv, msg_t *msg)
{
    packet_opt_t *pack_opt = config_get_packet_opt(cfg_srv);
    if (!(*pack_opt->check_special_packet)(msg, PACK_CFG_REGISTER))
    {
        process_client_register_request(cfg_srv, msg);
        free_msg(msg);
        return RET_ACCEPTED;
    }

    if (!config_client_is_register(&cfg_srv->client))
        return RET_DROP;

    if (!(*pack_opt->check_special_packet)(msg, PACK_CFG_HEARTBEAT))
    {
        process_client_heartbeat_request(cfg_srv, msg);
        free_msg(msg);
        return RET_ACCEPTED;
    }

    return RET_BACK;
}

static __inline__ void 
config_deliver_msg(config_service_t *cfg_srv, msg_t *msg)
{
    msg_engine_t *me;

    NMP_ASSERT(cfg_srv && msg);

    me = config_get_msg_engine(cfg_srv);
    if (me)
    {
        deliver_msg(me, msg);
    }
}

static int config_recv_one_msg(nmpio_t *net_io, void *net_msg, void *init_data)
{
    msg_t *msg;
    msg_owner_t owner;
    config_service_t *cfg_srv;

    NMP_ASSERT(net_io && net_msg && init_data);

    msg = (msg_t*)net_msg;
    cfg_srv = (config_service_t*)init_data;

    owner.object = init_data;
    owner.ref = NULL;
    owner.unref = NULL;

    attach_msg_io(msg, net_io);
    attach_msg_owner(msg, &owner);
    switch (config_process_special_msg(cfg_srv, msg))
    {
        case RET_ACCEPTED:
            break;
        case RET_BACK:
            config_deliver_msg(cfg_srv, msg);
            break;
        case RET_DROP:
            free_msg(msg);
            break;
    }

    return 0;
}

static void config_send_msg_out(config_service_t *cfg_srv, msg_t *msg)
{
    NMP_ASSERT(cfg_srv && msg);

    attach_msg_io(msg, cfg_srv->client.io);
    send_msg(msg);
}

int config_new_client_connect(cfg_service_basic_t *cfg_basic, nmpio_t *io)
{
    config_service_t *cfg_srv;
    NMP_ASSERT(cfg_basic && io);

    cfg_srv = (config_service_t*)create_new_service((void*)io, 
                CFG_SRV, DEF_CONFIG_SERVICE_NAME);
    if (cfg_srv)
    {
        nmp_net_set_io_reader(io, config_recv_one_msg, (void*)cfg_srv);
        config_add_new_service(cfg_basic->srv_list, cfg_srv);
        show_info("New client connect successful[io:%p].\n", io);
#if _DEBUG_
        show_service_list(cfg_basic->srv_list);
#endif
        return 0;
    }
    else
        return -1;
}

void config_finalize_client(config_service_t *cfg_srv, nmpio_t *io, int err)
{
    int flag = -1;
    config_user_info_t *user_info;
    cfg_service_basic_t *cfg_basic;

    NMP_ASSERT(cfg_srv && io);
    show_info("Finalize client connect [%p], err: %d.\n", io, err);
    user_info = &cfg_srv->client.user_info;

    nmp_mutex_lock(cfg_srv->client.lock);
    if (io == cfg_srv->client.io)
    {
        flag = 0;
        cfg_srv->client.io = NULL;
        user_info->status = LOGOUT;
    }
    nmp_mutex_unlock(cfg_srv->client.lock);

    if (!flag)
    {
        config_modify_user_status(config_get_user_list(
            (struct service*)cfg_srv), user_info);

        cfg_basic = config_get_service_basic(cfg_srv);
        config_remove_one_service(cfg_basic->srv_list, cfg_srv);
        destory_service((struct service*)cfg_srv);
    }

#if _DEBUG_
    show_service_list(cfg_basic->srv_list);
#endif
    nmp_net_unref_io(io);
    return ;
}

static int check_client_timeout(void *orig, void *custom)
{
    int timeout;
    nmpio_t *io = NULL;
    config_service_t *cfg_srv = (config_service_t*)orig;

    nmp_mutex_lock(cfg_srv->client.lock);
    if (0 >= cfg_srv->client.ttd)
    {
        timeout = 0;
        io = cfg_srv->client.io;
        show_info("Reap timeout client[io:%p]!!!!!\n", io);
    }
    else
        timeout = -1;
    nmp_mutex_unlock(cfg_srv->client.lock);

    if (io)
        nmp_net_kill_io(config_get_2lp_net(cfg_srv), io);

    return timeout;
}

void 
config_reap_timeout_client(cfg_service_basic_t *cfg_basic)
{
    nmp_list_t *link;
    config_service_t *cfg_srv;
    config_service_list_t *cfg_srv_list = cfg_basic->srv_list;

    while (1)
    {
        cfg_srv = NULL;
        link = nmp_list_find_custom(cfg_srv_list->list, (void*)NULL, 
                    check_client_timeout);
        if (link)
        {
            cfg_srv = (config_service_t*)nmp_list_data(link);
            cfg_srv_list->list = nmp_list_delete_link(cfg_srv_list->list, link);
        }

        if (!cfg_srv)
            break;

        config_modify_user_status(config_get_user_list(
            (struct service*)cfg_srv), &cfg_srv->client.user_info);

        config_remove_one_service_unlock(cfg_srv_list, cfg_srv);
        destory_service((struct service*)cfg_srv);
#if _DEBUG_
        show_service_list_nolock(cfg_basic->srv_list);
#endif
    }

    return ;
}



