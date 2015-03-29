/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/


#ifndef __TINY_RAIN_DEF_H__
#define __TINY_RAIN_DEF_H__

#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

#include "nmplib.h"

#define tr_assert(expr) assert(expr)

#define BEGIN_NAMESPACE
#define END_NAMESPACE

//#define TR_DEBUG					1
#define TR_FATAL_ERROR_EXIT			exit(128)

#ifdef _TI36X_IPNC_
#define MAX_FRAME_SIZE				(4*1024*1024)
#else
#define MAX_FRAME_SIZE				(8*1024*1024)
#endif

#define MAX_NALS_COUNT				(32)
#define DEFAULT_PW_WINSIZE			(25)

#ifdef __TR_DEBUG_NO_LOCK
# warning "Tiny-Rain Uses No Lock!!!!!!!!!!!!!!!!!"
# define LOCK_T uint32_t
# define LOCK_NEW() 0
# define LOCK_DEL(t)
# define AQUIRE_LOCK(t) do{}while(0)
# define RELEASE_LOCK(t) do{}while(0)
#else
typedef JMutex *LOCK_T;
# define LOCK_NEW j_mutex_new
# define LOCK_DEL j_mutex_free
# define AQUIRE_LOCK j_mutex_lock
# define RELEASE_LOCK j_mutex_unlock
#endif


#define __u8_str(p) ((uint8_t*)(p))
#define __str(p) ((char*)(p))

typedef void *__super_ptr;

#define j_event_u(ev) (((JEvent*)ev)->user_data)	//TODO, remove!

#define EUSER		255
#define EKILLED		(EUSER + 1)
#define ESESSION	(EUSER + 2)

#endif	//__TINY_RAIN_DEF_H__
