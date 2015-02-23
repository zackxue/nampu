/*
 * nmp_net.h
 *
 * This file declares net interfaces.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#ifndef __NMP_NET_H__
#define __NMP_NET_H__

#include "nmp_netio.h"


typedef struct _J2Proto
{
	JPacketProto 	*ll_proto;
	JPayloadProto	*hl_proto;
	void            *init_data;		/* user data */
}J2Proto;


typedef void (*JIOFin)(JNetIO *io, int err, void *init_data);
typedef int (*JIOInit)(JNetIO *io, void *init_data);
typedef void (*JIOFinUser)(void *user);

#ifdef __cplusplus
extern "C" {
#endif

JNet *j_net_new(int loops);
void j_net_release(JNet *net);

void j_net_set_funcs(JNet *net, JIOInit init, JIOFin fin);
void j_net_set_io_reader(JNetIO *net_io, JIOReader reader, void *init_data);
int j_net_write_io(JNetIO *net_io, void *msg);

JNetIO *j_net_create_io(JNet *net, JConnection *conn,
	J2Proto *proto, JIOEst on_est, int *err);

JNetIO *j_net_create_listen_io(JNet *net, JConnection *conn,
	J2Proto *proto, int *err);

JNetIO *j_net_create_listen_io_2(JNet *net, struct sockaddr *sa,
	J2Proto *proto, int *err);

JNetIO *j_net_ref_io(JNetIO *net_io);
void j_net_unref_io(JNetIO *net_io);

void j_net_set_io_ttd(JNetIO *net_io, int milli_sec);
int j_net_kill_io(JNet *net, JNetIO *net_io);

void j_net_set_io_u(JNetIO *net_io, void *u, JIOFinUser *fu);
void *j_net_get_io_u(JNetIO *net_io);

char *j_net_get_io_host_name(JNetIO *net_io, char *ip);
char *j_net_get_io_peer_name(JNetIO *net_io, char *ip);

//Timer
void *j_set_timer(int timeout, int (*)(void*), void *data);
void j_del_timer(void *handle);

#ifdef __cplusplus
}
#endif

#endif	//__NMP_NET_H__
