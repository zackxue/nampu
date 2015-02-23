/*
 * jpf_timer.h
 *
 * This file describes timer
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#ifndef __NMP_TIMER_H__
#define __NMP_TIMER_H__

#include <glib.h>

G_BEGIN_DECLS

typedef gboolean (*HmTimerFun)(gpointer user_data);
typedef struct _HmTimerEngine HmTimerEngine;


HmTimerEngine *jpf_timer_engine_new( void );
void jpf_timer_engine_release(HmTimerEngine *eng);

guint jpf_timer_engine_set_timer(HmTimerEngine *eng, guint interval,
	HmTimerFun fun, gpointer data);

void jpf_timer_engine_del_timer(HmTimerEngine *eng, gint id);

/* global timer setup */

guint jpf_set_timer(guint interval, HmTimerFun fun, gpointer data);

void jpf_del_timer(guint id);

G_END_DECLS

#endif	//__NMP_TIMER_H__
