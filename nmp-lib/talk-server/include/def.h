
#ifndef __DEF_H__
#define __DEF_H__

#include <stdint.h>
#include "nmplib.h"

typedef nmp_mutex_t *LOCK_T;
#define LOCK_NEW nmp_mutex_new
#define LOCK_DEL nmp_mutex_free
#define AQUIRE_LOCK nmp_mutex_lock
#define RELEASE_LOCK nmp_mutex_unlock

#define TR_FATAL_ERROR_EXIT			exit(128)
#define __u8_str(p) ((uint8_t*)(p))
#define __str(p) ((char*)(p))

#endif
