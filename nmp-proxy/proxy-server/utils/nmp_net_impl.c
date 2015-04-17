
#include "nmp_conn.h"

#include "nmp_proxy_log.h"
#include "nmp_proto_impl.h"
#include "nmp_net_impl.h"


#define DEFAULT_NET_LOOPS           1


nmp_net_t *create_2lp_net_object(net_io_new new_func, net_io_fin fin_func)
{
    nmp_net_t *_2lp_net;
    NMP_ASSERT(new_func && fin_func);

    _2lp_net = nmp_net_new(DEFAULT_NET_LOOPS);
    if (_2lp_net)
        nmp_net_set_funcs(_2lp_net, new_func, fin_func);

    return _2lp_net;
}

void destory_2lp_net_object(nmp_net_t *_2lp_net)
{
    NMP_ASSERT(_2lp_net);
    //nmp_net_release(_2lp_net);      //-->TODO
}

nmpio_t *create_local_listener(nmp_net_t *_2lp_net, nmp_2proto_t *proto, int listen_port)
{show_debug("Create listener: [port: %d]<<----------------\n", listen_port);
    int err;
    struct sockaddr_in sa;
    NMP_ASSERT(_2lp_net);

    sa.sin_family      = DEF_LOCAL_LISTENER_FAMILY;
    sa.sin_port        = htons(listen_port);
    sa.sin_addr.s_addr = htonl(DEF_LOCAL_LISTENER_INADDR);
    bzero(&sa.sin_zero, sizeof(sa.sin_zero));

    return nmp_net_create_listen_io_2(_2lp_net, (struct sockaddr*)&sa, proto, &err);
}

void destory_local_listener(nmp_net_t *_2lp_net, nmpio_t *listener)
{
    NMP_ASSERT(_2lp_net && listener);
    net_kill_io(_2lp_net, listener);
}

void net_kill_io(nmp_net_t *_2lp_net, nmpio_t *io)
{
    NMP_ASSERT(_2lp_net);

    if (io)
    {
        nmp_net_kill_io(_2lp_net, io);
        nmp_net_unref_io(io);
    }
}

void send_msg(msg_t *msg)
{
    int written;
    nmpio_t *net_io;

    NMP_ASSERT(msg);

    net_io = MSG_IO(msg);
    if (!net_io)
    {
        show_warn("Send msg '%p' failed, dst unknown.\n", msg);
        return ;
    }

    written = nmp_net_write_io(net_io, msg);
    if (written <= 0)
    {
        show_warn("Send msg '%p' failed, Err:'%d'\n", msg, written);
    }

    return ;
}

