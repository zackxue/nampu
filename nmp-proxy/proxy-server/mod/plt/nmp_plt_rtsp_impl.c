
#include "nmp_plt_rtsp_impl.h"

extern msg_t *
module_make_mds_info_msg(platform_service_t *plt_srv, 
    proxy_plt_t *plt_info, msg_t **msg);

static __inline__ rtsp_server_t *
get_service_rtsp_server(platform_service_t *plt_srv)
{
    plt_service_basic_t *tm;
    tm = (plt_service_basic_t*)((struct service*)plt_srv)->tm;
    return tm->rtsp_srv;
}

static __inline__ CONN_STATE_E
get_rtsp_module_state(plt_rtsp_t *rm, int *times)
{
    CONN_STATE_E state;

    nmp_mutex_lock(rm->lock);
    state = rm->state;
    *times = ++rm->state_timer;
    nmp_mutex_unlock(rm->lock);

    return state;
}

static __inline__ void
set_rtsp_module_state(plt_rtsp_t *rm, CONN_STATE_E state)
{
    nmp_mutex_lock(rm->lock);
    rm->state = state;
    rm->state_timer = 0;
    nmp_mutex_unlock(rm->lock);
}

static __inline__ reverse_cntr_t *
get_rtsp_module_reverse_connector(plt_rtsp_t *rm)
{
    reverse_cntr_t *rev_cntr;

    nmp_mutex_lock(rm->lock);
    rev_cntr = rm->rev_cntr;
    nmp_mutex_unlock(rm->lock);

    return rev_cntr;
}

static __inline__ void
set_rtsp_module_reverse_connector(platform_service_t *plt_srv, reverse_cntr_t *rev_cntr)
{
    rtsp_server_t *rtsp_srv;
    reverse_cntr_t *old;
    plt_rtsp_t *rm = &plt_srv->rtsp;

    nmp_mutex_lock(rm->lock);
    old = rm->rev_cntr;
    rm->rev_cntr = rev_cntr;
    nmp_mutex_unlock(rm->lock);

    if (old)
    {
        rtsp_srv = get_service_rtsp_server(plt_srv);
        rtsp_server_release_reverse_connector(rtsp_srv, old);
    }
}

static __inline__ void
module_rtsp_release_connector(platform_service_t *plt_srv)
{
    set_rtsp_module_reverse_connector(plt_srv, NULL);
    set_rtsp_module_state(&plt_srv->rtsp, DISCONNECTED);
    return ;
}

static void 
module_rtsp_connect_cb(Reverse_connector *cntr, gint err, gpointer user_data)
{
    platform_service_t *plt_srv = (platform_service_t*)user_data;
    NMP_ASSERT(cntr && user_data);

    show_debug("Rtsp server connect cb, err:%d [OK:%d]\n", err, E_CONN_OK);
    if (err != E_CONN_OK)
    {
        if ((reverse_cntr_t*)cntr == get_rtsp_module_reverse_connector(&plt_srv->rtsp))
        {
            module_rtsp_release_connector(plt_srv);
        }
    }
}

static reverse_cntr_t *
module_rtsp_connect_server(platform_service_t *plt_srv, proxy_plt_t *plt_info)
{
    rtsp_server_t *rtsp_srv;
    reverse_cntr_t *r_cntr = NULL;

    rtsp_srv = get_service_rtsp_server(plt_srv);
    if (rtsp_srv)
    {
        r_cntr = rtsp_server_reverse_connect(rtsp_srv, 
                    plt_info->mds_host, plt_info->mds_port, 
                    plt_info->pu_id, plt_info->keep_alive_freq, 
                    plt_info->protocol, module_rtsp_connect_cb, plt_srv);

        show_debug("rtsp_server_reverse_connect %s, r_cntr: '%p' <-----------------------\n", 
            r_cntr ? "OK" : "failed", r_cntr);
    }
    else
        show_warn("rtsp_srv NULL!<-----------------\n");

    return r_cntr;
}

///////////////////////////////////////////////////////////////////////////////////////
void init_rtsp_module(platform_service_t *plt_srv, plt_rtsp_t *rm)
{
    NMP_ASSERT(plt_srv && rm);

    rm->lock = nmp_mutex_new();
    rm->state = DISCONNECTED;
    rm->state_timer = 0;
    rm->rev_cntr = NULL;

    return ;
}
void cleanup_rtsp_module(platform_service_t *plt_srv, plt_rtsp_t *rm)
{
    NMP_ASSERT(plt_srv && rm);

    module_rtsp_release_connector(plt_srv);
    nmp_mutex_free(rm->lock);
    return ;
}

void control_rtsp_module(platform_service_t *plt_srv, int cmd, void *user)
{
    switch (cmd)
    {
        default:
            break;
    }
}

void check_rtsp_module(platform_service_t *plt_srv, proxy_plt_t *plt_info)
{
    int dev_state, state_time, flag = -1;
    reverse_cntr_t *r_cntr = NULL;

    plt_rtsp_t *rm = &plt_srv->rtsp;

    dev_state = proxy_get_device_state(plt_srv->owner);
    switch (get_rtsp_module_state(rm, &state_time))
    {
        case DISCONNECTED:
            if (DEFAULT_CONNECTING_SECS < state_time)
            {
                set_rtsp_module_state(rm, DISCONNECTED);
                if (STATE_REGISTERED == dev_state)
                {
                    flag = 0;
                    r_cntr = get_rtsp_module_reverse_connector(rm);
                }
            }
            break;

        case CONNECTED:
            if (STATE_REGISTERED != dev_state)
                module_rtsp_release_connector(plt_srv);
            break;

        default:
            break;
    }

    if (!flag && !r_cntr)
    {
        if (0 != plt_info->mds_port && strlen(plt_info->mds_host))
        {
            r_cntr = module_rtsp_connect_server(plt_srv, plt_info);
            if (r_cntr)
            {
                set_rtsp_module_reverse_connector(plt_srv, r_cntr);
                set_rtsp_module_state(&plt_srv->rtsp, CONNECTED);
            }
        }
        else
        {
            msg_t *mds_msg = NULL;
            if (module_make_mds_info_msg(plt_srv, plt_info, &mds_msg))
            {
                proxy_send_device_msg(plt_srv->owner, mds_msg);
                free_msg(mds_msg);
            }
        }
    }

    return ;
}



