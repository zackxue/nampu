/*
 * j_watch.h
 *
 * This file declares watch structure and interfaces.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#ifndef __J_WATCH_H__
#define __J_WATCH_H__

#include "nmplib.h"
#include "nmp_conn.h"
#include "nmp_netbuf.h"


#define j_error(...) fprintf(stderr, __VA_ARGS__);
#define j_print j_error
#define j_debug j_error
#define j_warning j_error


typedef struct _JWatch JWatch;
typedef struct _JWatchFuncs JWatchFuncs;

struct _JWatchFuncs
{
    JWatch *(*create)(JWatch *w, JConnection *conn);

    int (*recv)(JWatch *w, char *buf, size_t size);
    void (*error)(JWatch *w, int rw, int err);
    void (*close)(JWatch *w, int async);

    int (*format)(JWatch *w, void *msg, char buf[], size_t size);
};


struct _JWatch
{
    JEvent             source;

    JMutex             *lock;

    JNetBuf            *buffer;
    JConnection        *conn;
    JWatchFuncs        *funcs;

    JTimeVal           next_timeout;       /* timeout point */

    int                w_pending;
    int                killed;

    void               *priv_data;
};

#ifdef __cplusplus
extern "C" {
#endif

JWatch *j_watch_create(JConnection *conn,
    JWatchFuncs *funcs, int size);

JWatch *j_listen_watch_create(JConnection *conn,
    JWatchFuncs *funcs, int size);

void j_watch_attach(JWatch *watch, JEventLoop *loop);

int j_watch_recv_message(JWatch *watch, void *msg);
int j_watch_write_message(JWatch *watch, void *msg);

void j_watch_kill(JWatch *watch);

void j_watch_ref(JWatch *watch);
void j_watch_unref(JWatch *watch);

void j_watch_set_private(JWatch *watch, void *priv_data);
JBool j_watch_set_conn_ttd(JWatch *watch, int milli_sec);

char *j_watch_get_host(JWatch *watch, char *ip);
char *j_watch_get_peer(JWatch *watch, char *ip);

#ifdef __cplusplus
}
#endif

#endif  //__J_WATCH_H__
