/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/


#ifndef __TINY_RAIN_TIMER_H__
#define __TINY_RAIN_TIMER_H__

#include "def.h"

BEGIN_NAMESPACE

#define TIMER_CONT		0
#define TIMER_EXIT		1

typedef struct __Timer Timer;
typedef int32_t (*OnTimer)(Timer *self, void *data);
typedef void (*OnDel)(void *data);

int32_t init_timer_facility( void );
Timer* set_timer(int32_t timeout, OnTimer on_timer, OnDel on_del, void *data);
void mod_timer(Timer *timer, int32_t milli_secs);
void del_timer(Timer *timer);

END_NAMESPACE

#endif	//__TINY_RAIN_TIMER_H__
