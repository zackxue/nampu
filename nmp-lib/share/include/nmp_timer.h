/*
 * hm_timer.h
 *
 * This file describes timer
 *
 * Copyright(c) by HiMickey, 2010~2014
 * Author:
*/

#ifndef __HM_TIMER_H__
#define __HM_TIMER_H__

#include <glib.h>

G_BEGIN_DECLS

typedef gboolean (*HmTimerFun)(gpointer user_data);
typedef struct _HmTimerEngine HmTimerEngine;


HmTimerEngine *hm_timer_engine_new( void );
void hm_timer_engine_release(HmTimerEngine *eng);

guint hm_timer_engine_set_timer(HmTimerEngine *eng, guint interval,
	HmTimerFun fun, gpointer data);

void hm_timer_engine_del_timer(HmTimerEngine *eng, gint id);

/* global timer setup */

guint hm_set_timer(guint interval, HmTimerFun fun, gpointer data);

void hm_del_timer(guint id);

G_END_DECLS

#endif	//__HM_TIMER_H__
