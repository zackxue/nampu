/*
 * nmp_timer.h
 *
 * This file declares interfaces for timer.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#ifndef __NMP_TIMER_H__
#define __NMP_TIMER_H__

#include "nmplib.h"

typedef int (*nmp_on_timer)(void *data);

typedef struct _nmp_timer nmp_timer_t;
struct _nmp_timer
{
	nmp_event_t     base;
	nmp_on_timer    on_timer;
	void            *data;
};

#ifdef __cplusplus
extern "C" {
#endif

nmp_timer_t *nmp_timer_new(int timeout, nmp_on_timer on_timer, void *data);
int nmp_timer_attach(nmp_event_loop_t *loop, nmp_timer_t *timer);
void nmp_timer_del(nmp_timer_t *timer);

#ifdef __cplusplus
}
#endif


#endif	//__NMP_TIMER_H__
