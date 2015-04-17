/*
 *          file: nmp_net_impl.h
 *          description:ÍøÂç²ã
 *
 *          hegui,heguijiss@gmail.com
 *          May 16th, 2013
 */

#ifndef __NET_IMPL_H__
#define __NET_IMPL_H__

#include "nmp_net.h"

#include "nmp_msg_impl.h"

#define DEF_LOCAL_LISTENER_FAMILY       AF_INET
#define DEF_LOCAL_LISTENER_INADDR       INADDR_ANY


typedef nmp_io_init_func net_io_new;
typedef nmp_io_fin_func net_io_fin;


#ifdef __cplusplus
extern "C" {
#endif

nmp_net_t *create_2lp_net_object(net_io_new new, net_io_fin fin);
void destory_2lp_net_object(nmp_net_t *_2lp_net);

nmpio_t *create_local_listener(nmp_net_t *_2lp_net, nmp_2proto_t *proto, int listen_port);
void destory_local_listener(nmp_net_t *_2lp_net, nmpio_t *listener);

void net_kill_io(nmp_net_t *_2lp_net, nmpio_t *io);

void send_msg(msg_t *msg);



#ifdef __cplusplus
    }
#endif


#endif  //__NET_IMPL_H__


