
#include "nmp_packet.h"

#include "nmp_net_impl.h"
#include "nmp_proto_impl.h"
#include "nmp_resolve_host.h"
#include "nmp_plt_parm_impl.h"


#define PU_CMS_PROT_OFFSET          1

#define UNREG_PACK_SENT             1
#define UNREG_PACK_UNSENT          -1

static __inline__ int
process_parm_special_msg(platform_service_t *plt_srv, msg_t *msg);


static __inline__ nmp_net_t *
get_service_2lp_net(platform_service_t *plt_srv)
{
    plt_service_basic_t *plt_basic;
    plt_basic = (plt_service_basic_t*)((struct service*)plt_srv)->tm;
    return plt_basic->_2lp_net;
}
static __inline__ packet_opt_t *
get_service_pack_opt(platform_service_t *plt_srv)
{
    plt_service_basic_t *plt_basic;
    plt_basic = (plt_service_basic_t*)((struct service*)plt_srv)->tm;
    return plt_basic->pack_opt;
}
msg_engine_t *
get_service_msg_engine(platform_service_t *plt_srv)
{
    plt_service_basic_t *plt_basic;
    plt_basic = (plt_service_basic_t*)((struct service*)plt_srv)->tm;
    return plt_basic->plt_me;
}

static __inline__ nmpio_t *
set_parm_module_io(platform_service_t *plt_srv, nmpio_t *io)
{
    nmpio_t *old;
    plt_parm_t *pm = &plt_srv->parm;

    nmp_mutex_lock(pm->lock);
    old = pm->io;
    pm->io = io;
    pm->ttd = pm->ttl;
    
    if (!io)
        pm->unregistered = UNREG_PACK_UNSENT;
    nmp_mutex_unlock(pm->lock);

    if (old)
    {show_debug(">>>>>>>>>>>>net_kill_io<<<<<<<<<<<<<<<<<<<<<\n");
        net_kill_io(get_service_2lp_net(plt_srv), old);
    }

    return io;
}

static __inline__ CONN_STATE_E
get_parm_module_state(plt_parm_t *pm, int *times)
{
    CONN_STATE_E state;

    nmp_mutex_lock(pm->lock);
    state = pm->state;
    *times = ++pm->state_timer;
    nmp_mutex_unlock(pm->lock);

    return state;
}

static __inline__ void
set_parm_module_state(platform_service_t *plt_srv, CONN_STATE_E state)
{
    plt_parm_t *pm = &plt_srv->parm;

    nmp_mutex_lock(pm->lock);
    pm->state = state;
    pm->state_timer = 0;
    pm->ttd = pm->ttl;
    nmp_mutex_unlock(pm->lock);
}

static __inline__ void 
report_parm_module_state(platform_service_t *plt_srv, PROXY_STATE_E state, int err)
{
    proxy_state_t dev_state;
    dev_state.state = state;
    dev_state.error = err;
    proxy_set_device_state(plt_srv->owner, &dev_state);
}

static __inline__ void
module_make_register_msg(platform_service_t *plt_srv, proxy_plt_t *plt_info, msg_t **msg)
{
    msg_t *reg_msg;
    packet_opt_t *pack_opt;

    proxy_reg_t reg_info;
    proxy_sdk_t sdk_info;

    memset(&reg_info, 0, sizeof(proxy_reg_t));
    memset(&sdk_info, 0, sizeof(proxy_sdk_t));

    if (proxy_get_device_private(plt_srv->owner, DEV_PRI_PRX_SDK, &sdk_info))
    {
        BUG();
    }

    reg_info.pu_type = plt_info->pu_type;
    strncpy(reg_info.pu_id, plt_info->pu_id, 
        sizeof(reg_info.pu_id)-1);
    strncpy(reg_info.cms_host, plt_info->cms_host, 
        sizeof(reg_info.cms_host)-1);
    strncpy(reg_info.dev_host, sdk_info.dev_host, 
        sizeof(reg_info.dev_host)-1);

    pack_opt = get_service_pack_opt(plt_srv);
    reg_msg = (*pack_opt->create_special_packet)(&reg_info, 
                    PACK_PTL_REGISTER, ++plt_srv->parm.seq_generator);
    if (reg_msg)
    {
        attach_msg_io(reg_msg, plt_srv->parm.io);
        *msg = reg_msg;
    }

    return ;
}

static __inline__ void
module_make_heartbeat_msg(platform_service_t *plt_srv,
    proxy_plt_t *plt_info, msg_t **msg)
{
    msg_t *hb_msg;
    packet_opt_t *pack_opt;

    proxy_reg_t reg_info;
    proxy_sdk_t sdk_info;

    memset(&reg_info, 0, sizeof(proxy_reg_t));
    memset(&sdk_info, 0, sizeof(proxy_sdk_t));

    if (proxy_get_device_private(plt_srv->owner, DEV_PRI_PRX_SDK, &sdk_info))
    {
        BUG();
    }

    reg_info.pu_type = plt_info->pu_type;
    strncpy(reg_info.pu_id, plt_info->pu_id, 
        sizeof(reg_info.pu_id)-1);
    strncpy(reg_info.cms_host, plt_info->cms_host, 
        sizeof(reg_info.cms_host)-1);
    strncpy(reg_info.dev_host, sdk_info.dev_host, 
        sizeof(reg_info.dev_host)-1);

    pack_opt = get_service_pack_opt(plt_srv);
    hb_msg = (*pack_opt->create_special_packet)(&reg_info, 
                PACK_PTL_HEARTBEAT, ++plt_srv->parm.seq_generator);
    if (hb_msg)
    {
        attach_msg_io(hb_msg, plt_srv->parm.io);
        *msg = hb_msg;
    }

    return ;
}

msg_t *
module_make_mds_info_msg(platform_service_t *plt_srv, 
    proxy_plt_t *plt_info, msg_t **msg)
{
    msg_t *mds_msg;
    packet_opt_t *pack_opt;
    pack_opt = get_service_pack_opt(plt_srv);

    mds_msg = (*pack_opt->create_special_packet)(plt_info, 
        PACK_PLT_GETMDSINFO, ++plt_srv->parm.seq_generator);
    if (mds_msg)
    {
        attach_msg_io(mds_msg, plt_srv->parm.io);
        *msg = mds_msg;
    }

    return *msg;
}

static __inline__ void
module_parm_channel_is_alive(plt_parm_t *pm)
{
    nmp_mutex_lock(pm->lock);
    if (!pm->unregistered)
        pm->ttd = pm->ttl;
    nmp_mutex_unlock(pm->lock);
}

static int
module_parm_recv_cms_msg(nmpio_t *net_io, void *net_msg, void *init_data)
{
    msg_t *msg;
    platform_service_t *plt_srv;
    plt_service_basic_t *plt_basic;

    NMP_ASSERT(net_io && net_msg && init_data);

    show_debug("recv from cms ......!\n");

    msg = (msg_t*)net_msg;
    plt_srv = (platform_service_t*)init_data;
    plt_basic = (plt_service_basic_t*)plt_srv->base.tm;

    attach_msg_io(msg, net_io);
    if (process_parm_special_msg(plt_srv, msg))
    {
        msg_owner_t owner;
        owner.object = plt_srv->owner;
        owner.ref    = (void*)proxy_device_ref;
        owner.unref  = (void*)proxy_device_unref;

        attach_msg_owner(msg, &owner);
        proxy_deliver_device_msg(plt_srv->owner, plt_basic->plt_me, msg);
    }

    return 0;
}

static __inline__ void
module_parm_disconnect(platform_service_t *plt_srv)
{
    set_parm_module_io(plt_srv, NULL);
    set_parm_module_state(plt_srv, DISCONNECTING);
}

static __inline__ void
module_parm_connect_timeout(platform_service_t *plt_srv)
{
    show_warn("Connect CMS timeout!\n");
    module_parm_disconnect(plt_srv);
    report_parm_module_state(plt_srv, STATE_DISCONNECT, -E_CONNECT_TIMEOUT);
    return ;
}

static __inline__ void
module_parm_connect_die(platform_service_t *plt_srv)
{
    show_warn("CMS Connection Die!!!\n");
    module_parm_disconnect(plt_srv);
    report_parm_module_state(plt_srv, STATE_DISCONNECT, -E_ONLINE_TIMEOUT);
}

static void
module_parm_connect_ok(nmpio_t *io, void *init_data)
{
    platform_service_t *plt_srv;
    NMP_ASSERT(io != NULL && init_data != NULL);

    show_debug("Connect CMS OK!!!\n");

    plt_srv = (platform_service_t*)init_data;
    set_parm_module_state(plt_srv, CONNECTED);
    report_parm_module_state(plt_srv, STATE_CONNECTING, 0);
    nmp_net_set_io_reader(io, module_parm_recv_cms_msg, plt_srv);

    return ;
}

static __inline__ void
module_parm_connect(platform_service_t *plt_srv, proxy_plt_t *plt_info)
{
    char cms_ip[MAX_IP_LEN];
    int err, cms_port;

    nmpio_t *io;
    nmp_conn_t *conn;
    struct sockaddr_in socket;

    cms_port = plt_info->cms_port + PU_CMS_PROT_OFFSET;

    show_debug("Start connecting CMS, host: '%s', port: '%d'.\n", 
        plt_info->cms_host, cms_port);

    memset(cms_ip, 0, sizeof(cms_ip));

    if (!proxy_resolve_host(plt_info->cms_host, cms_ip, sizeof(cms_ip)))
    {
        show_warn("CMS host '%s' unsolved.\n", plt_info->cms_host);
        set_parm_module_state(plt_srv, DISCONNECTING);
        return ;
    }

    conn = nmp_connection_new(NULL, CF_TYPE_TCP|CF_FLGS_NONBLOCK, &err);
    if (!conn)
    {
        show_warn("Create TCP connection failed, err: '%d'.\n", err);
        set_parm_module_state(plt_srv, DISCONNECTING);
        return ;
    }

    nmp_resolve_host(&socket, cms_ip, cms_port);
    err = nmp_connection_connect(conn, (struct sockaddr*)&socket);
    if (!err || err == -EINPROGRESS)
    {
        if (!err)
        {
            io = nmp_net_create_io(get_service_2lp_net(plt_srv), conn, 
                    &plt_srv->proto, NULL, &err);
            if (io)
            {
                module_parm_connect_ok(io, (void*)plt_srv);
            }
        }
        else
        {
            show_debug("Connecting CMS ...\n");
            io = nmp_net_create_io(get_service_2lp_net(plt_srv), conn, 
                    &plt_srv->proto, module_parm_connect_ok, &err);
            if (io)
                set_parm_module_state(plt_srv, CONNECTING);
        }

        if (!io)    // conn was destroyed 
        {
            show_warn("Create net io object failed, err:'%d'\n", err);
            set_parm_module_state(plt_srv, DISCONNECTING);
            nmp_connection_close(conn);
            return ;
        }

        set_parm_module_io(plt_srv, io);
        return ;
    }
    else
    {
        set_parm_module_state(plt_srv, DISCONNECTING);
        nmp_connection_close(conn);
    }

    return ;
}

static __inline__ void
module_parm_register_timeout(platform_service_t *plt_srv)
{
    module_parm_disconnect(plt_srv);
    report_parm_module_state(plt_srv, STATE_UNREGISTER, -E_REGISTER_TIMEOUT);
}

static __inline__ void
module_parm_register_request(platform_service_t *plt_srv, proxy_plt_t *plt_info)
{
    int flag = 0;
    msg_t *msg = NULL;
    plt_parm_t *pm = &plt_srv->parm;

    nmp_mutex_lock(pm->lock);
    if (!pm->io || 0 >= --pm->ttd)
        flag = -1;
    else if (pm->unregistered == UNREG_PACK_UNSENT)
    {
        module_make_register_msg(plt_srv, plt_info, &msg);
        if (msg)
            pm->unregistered = UNREG_PACK_SENT;
        flag = 1;
    }
    nmp_mutex_unlock(pm->lock);

    if (0 > flag)
    {
        module_parm_register_timeout(plt_srv);
        return ;
    }

    if (0 < flag)
    {
        if (msg)
        {
            proxy_send_device_msg(plt_srv->owner, msg);
            free_msg(msg);
        }
    }

    return ;
}

static __inline__ void
module_parm_register_reponse(platform_service_t *plt_srv, msg_t *msg)
{
    int ret;
    proxy_plt_t plt_info;
    plt_parm_t *pm = &plt_srv->parm;
    packet_opt_t *pack_opt = get_service_pack_opt(plt_srv);

    if (proxy_get_device_private(plt_srv->owner, DEV_PRI_PLT_SRV, &plt_info))
    {
        BUG();
    }

    nmp_mutex_lock(pm->lock);
    ret = (*pack_opt->process_special_packet)(msg, &plt_info, PACK_PTL_REGISTER);
    if (!ret)
    {
        if (plt_info.keep_alive_freq < DEFAULT_MIN_HEART_SECS)
            plt_info.keep_alive_freq = DEFAULT_MIN_HEART_SECS;
        if (plt_info.keep_alive_freq > DEFAULT_MAX_HEART_SECS)
            plt_info.keep_alive_freq = DEFAULT_MAX_HEART_SECS;

        pm->unregistered = 0;
        pm->ttl = plt_info.keep_alive_freq * DEFAULT_HEARTBEAT_TIMES;
        pm->ttd = pm->ttl;
    }
    nmp_mutex_unlock(pm->lock);

    show_debug("Register CMS %s, Err:'%d'\n", !ret ? "OK" : "failed", ret);
    if (!ret)
    {
        set_parm_module_state(plt_srv, REGISTERED);
        report_parm_module_state(plt_srv, STATE_REGISTERED, ret);
        proxy_set_device_private(plt_srv->owner, DEV_PRI_PLT_SRV, sizeof(plt_info), &plt_info);
    }
    else
    {
        module_parm_disconnect(plt_srv);
        report_parm_module_state(plt_srv, STATE_UNREGISTER, ret);
    }
}

static __inline__ void
module_parm_check_online(platform_service_t *plt_srv, proxy_plt_t *plt_info)
{
    int flag = 0;
    msg_t *msg = NULL;
    plt_parm_t *pm = &plt_srv->parm;

    nmp_mutex_lock(pm->lock);
    if (!pm->io || 0 >= --pm->ttd)
        flag = -1;
    else if (!pm->unregistered)
    {
        if (!(pm->state_timer % plt_info->keep_alive_freq))
        {
            flag = 1;
            module_make_heartbeat_msg(plt_srv, plt_info, &msg);
        }
    }
    nmp_mutex_unlock(pm->lock);

    if (0 > flag)
    {
        show_warn("Online timeout!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        module_parm_connect_die(plt_srv);
        return ;
    }

    if (0 < flag)
    {
        if (msg)
        {
            proxy_send_device_msg(plt_srv->owner, msg);
            free_msg(msg);
        }
    }

    return ;
}

static __inline__ int
process_parm_special_msg(platform_service_t *plt_srv, msg_t *msg)
{
    proxy_plt_t plt_info;
    packet_opt_t *pack_opt = get_service_pack_opt(plt_srv);

    if (!(*pack_opt->check_special_packet)(msg, PACK_PTL_REGISTER))
    {
        module_parm_register_reponse(plt_srv, msg);
        free_msg(msg);
        return 0;
    }

    module_parm_channel_is_alive(&plt_srv->parm);

    if (!(*pack_opt->check_special_packet)(msg, PACK_PTL_HEARTBEAT))
    {
        free_msg(msg);
        return 0;
    }

    if (!(*pack_opt->check_special_packet)(msg, PACK_PLT_MDSCHANGED))
    {
        if (!proxy_get_device_private(plt_srv->owner, DEV_PRI_PLT_SRV, &plt_info))
        {
            if (0 != plt_info.mds_port && strlen(plt_info.mds_host))
            {
                plt_info.mds_port = 0;
                memset(plt_info.mds_host, 0, sizeof(plt_info.mds_host));
                proxy_set_device_private(plt_srv->owner, 
                    DEV_PRI_PLT_SRV, sizeof(plt_info), &plt_info);
            }
        }

        free_msg(msg);
        return 0;
    }

    if (!(*pack_opt->check_special_packet)(msg, PACK_PLT_GETMDSINFO))
    {
        if (proxy_get_device_private(plt_srv->owner, DEV_PRI_PLT_SRV, &plt_info))
        {
            BUG();
        }

        if (!(*pack_opt->process_special_packet)(msg, &plt_info, PACK_PLT_GETMDSINFO))
        {
            if (0 != plt_info.mds_port && strlen(plt_info.mds_host))
            {
                show_debug(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>[%s:%d]\n", 
                plt_info.mds_host, plt_info.mds_port);
                proxy_set_device_private(plt_srv->owner, 
                    DEV_PRI_PLT_SRV, sizeof(plt_info), &plt_info);
            }
        }

        free_msg(msg);
        return 0;
    }

    return 1;
}

void init_parm_module(platform_service_t *plt_srv, plt_parm_t *pm)
{
    NMP_ASSERT(plt_srv && pm);

    pm->lock = nmp_mutex_new();
    pm->io = NULL;
    pm->state = DISCONNECTED;
    pm->state_timer = 0;
    pm->ttl = DEFAULT_TTL;
    pm->ttd = pm->ttl;
    pm->unregistered = UNREG_PACK_UNSENT;

    return ;
}

void cleanup_parm_module(platform_service_t *plt_srv, plt_parm_t *pm)
{
    NMP_ASSERT(plt_srv && pm);

    set_parm_module_io(plt_srv, NULL);

    nmp_mutex_lock(pm->lock);
    pm->state = DISCONNECTED;
    pm->state_timer = 0;
    pm->ttl = 0;
    pm->ttd = pm->ttl;
    pm->unregistered = UNREG_PACK_UNSENT;
    nmp_mutex_unlock(pm->lock);

    nmp_mutex_free(pm->lock);
    
    return ;
}

void control_parm_module(platform_service_t *plt_srv, int cmd, void *user)
{
    void *submit;
    nmpio_t *io = NULL;
    msg_t *msg = NULL;
    proxy_plt_t plt_info;
    plt_parm_t *pm = &plt_srv->parm;

    switch (cmd)
    {
        case CTRL_CMD_SUBMIT:
            msg = (msg_t*)user;
            submit = MSG_DATA(msg);
            if (!proxy_get_device_private(plt_srv->owner, DEV_PRI_PLT_SRV, &plt_info))
            {
                strncpy(submit, plt_info.pu_id, J_SDK_MAX_ID_LEN-1);
            }

            nmp_mutex_lock(pm->lock);
            if (pm->io)
                io = nmp_net_ref_io(pm->io);
            else
                io = NULL;
            nmp_mutex_unlock(pm->lock);

            if (io)
            {
                attach_msg_io(msg, io);
                nmp_net_unref_io(io);

                send_msg(msg);
            }
            break;
        default:
            break;
    }
}

void check_parm_module(platform_service_t *plt_srv, proxy_plt_t *plt_info)
{
    int dev_state;
    int state_timer;
    plt_parm_t *pm = &plt_srv->parm;

    dev_state = proxy_get_device_state(plt_srv->owner);
    switch (get_parm_module_state(pm, &state_timer))
    {
        case DISCONNECTED:
            if (STATE_LOGIN == dev_state)
                module_parm_connect(plt_srv, plt_info);
            break;

        case CONNECTING:
            if (state_timer > DEFAULT_CONNECTING_SECS)
                module_parm_connect_timeout(plt_srv);
            break;

        case CONNECTED:
            if (STATE_CONNECTING == dev_state)
                module_parm_register_request(plt_srv, plt_info);
            else
                module_parm_disconnect(plt_srv);
            break;

        case REGISTERED:
            if (STATE_REGISTERED == dev_state)
                module_parm_check_online(plt_srv, plt_info);
            else
                module_parm_disconnect(plt_srv);
            break;

        case DISCONNECTING:
            if (state_timer > DEFAULT_DISCONNECTING_SECS)
                set_parm_module_state(plt_srv, DISCONNECTED);
            break;

        default:
            break;
    }

    return ;
}

void finalize_parm_io(platform_service_t *plt_srv, nmpio_t *io, int err)
{show_debug("Enter %s()\n", __FUNCTION__);
    plt_parm_t *pm = &plt_srv->parm;

    nmp_mutex_lock(pm->lock);
    if (io != pm->io)
    {
        io = NULL;
    }
    else
    {
        pm->io = NULL;
        pm->ttl = DEFAULT_TTL;
        pm->ttd = pm->ttl;
        pm->unregistered = UNREG_PACK_UNSENT;
    }
    nmp_mutex_unlock(pm->lock);

    if (io)
        nmp_net_unref_io(io);

    set_parm_module_state(plt_srv, DISCONNECTING);

    if (!err)
        err = -104;  //104:closed by peer
    report_parm_module_state(plt_srv, STATE_DISCONNECT, err);

    show_debug("Left %s()\n", __FUNCTION__);
    return ;
}












