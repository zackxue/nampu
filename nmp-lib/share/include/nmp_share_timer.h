/*
 * nmp_share_timer.h
 *
 * This file describes timer
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#ifndef __NMP_SHARE_TIMER_H__
#define __NMP_SHARE_TIMER_H__

#include <glib.h>

G_BEGIN_DECLS

typedef gboolean (*HmTimerFun)(gpointer user_data);
typedef struct _HmTimerEngine HmTimerEngine;


HmTimerEngine *nmp_timer_engine_new( void );
void nmp_timer_engine_release(HmTimerEngine *eng);

guint nmp_timer_engine_set_timer(HmTimerEngine *eng, guint interval,
	HmTimerFun fun, gpointer data);

void nmp_timer_engine_del_timer(HmTimerEngine *eng, gint id);

/* global timer setup */

guint nmp_set_timer(guint interval, HmTimerFun fun, gpointer data);

void nmp_del_timer(guint id);

G_END_DECLS

#endif	//__NMP_TIMER_H__
