/* *
 * This file is part of rtsp-server
 *
 * Copyright (C) 2012 fangyi <fangyi@szjxj.net>
 * See COPYING for more details
 *
 * rtsp-server is a stream transporting and controlling module using
 * RTP/RTCP/RTSP protocols, which is designed for jxj platform servers
 * such as mss, proxy-server etc.
 *
 * */

#ifndef __MACRO_H__
#define __MACRO_H__

#include "nmpev.h"

#define POINTER_BACK_TO(x) \
	gpointer	owner;

#define SET_OWNER(x, o) \
	(x)->owner = o

#define GET_OWNER(x, TYPE) \
	(TYPE*)((x)->owner)

#define __OBJECT_REF(x) \
	REF_DEBUG_TEST(x); \
	g_atomic_int_add(&(x)->ref_count, 1); \
	return x;

#define __OBJECT_UNREF(x, FUNC_PREFIX) \
	REF_DEBUG_TEST(x); \
	if (g_atomic_int_dec_and_test(&(x)->ref_count)) \
	{\
		FUNC_PREFIX##_finalize(x);\
	}

#define SDP_TRACK_SEPARATOR		"stream="
#define SDP_TRACK_URI_SEPARATOR	"/"SDP_TRACK_SEPARATOR

#endif	/* __MACRO_H__ */
