/*
 * nmp_watch.h
 *
 * This file declares watch structure and interfaces.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#ifndef __NMP_WATCH_H__
#define __NMP_WATCH_H__

#include "nmplib.h"
#include "nmp_conn.h"
#include "nmp_netbuf.h"


#define nmp_error(...) fprintf(stderr, __VA_ARGS__);
#define nmp_print nmp_error
#define nmp_debug nmp_error
#define nmp_warning nmp_error


typedef struct _nmp_watch nmp_watch_t;
typedef struct _nmp_watch_funcs nmp_watch_funcs;

struct _nmp_watch_funcs
{
    nmp_watch_t *(*create)(nmp_watch_t *w, nmp_conn_t *conn);

    int (*recv)(nmp_watch_t *w, char *buf, size_t size);
    void (*error)(nmp_watch_t *w, int rw, int err);
    void (*close)(nmp_watch_t *w, int async);

    int (*format)(nmp_watch_t *w, void *msg, char buf[], size_t size);
};


struct _nmp_watch
{
    nmp_event_t        source;

    nmp_mutex_t        *lock;

    nmp_net_buf_t      *buffer;
    nmp_conn_t         *conn;
    nmp_watch_funcs    *funcs;

    nmp_timeval_t      next_timeout;       /* timeout point */

    int                w_pending;
    int                killed;

    void               *priv_data;
};

#ifdef __cplusplus
extern "C" {
#endif

nmp_watch_t *nmp_watch_create(nmp_conn_t *conn,
    nmp_watch_funcs *funcs, int size);

nmp_watch_t *j_listen_watch_create(nmp_conn_t *conn,
    nmp_watch_funcs *funcs, int size);

void nmp_watch_attach(nmp_watch_t *watch, nmp_event_loop_t *loop);

int nmp_watch_recv_message(nmp_watch_t *watch, void *msg);
int nmp_watch_write_message(nmp_watch_t *watch, void *msg);

void nmp_watch_kill(nmp_watch_t *watch);

void nmp_watch_ref(nmp_watch_t *watch);
void nmp_watch_unref(nmp_watch_t *watch);

void nmp_watch_set_private(nmp_watch_t *watch, void *priv_data);
nmp_bool_t nmp_watch_set_conn_ttd(nmp_watch_t *watch, int milli_sec);

char *nmp_watch_get_host(nmp_watch_t *watch, char *ip);
char *nmp_watch_get_peer(nmp_watch_t *watch, char *ip);

#ifdef __cplusplus
}
#endif

#endif  //__NMP_WATCH_H__
