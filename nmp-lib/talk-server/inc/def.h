
#ifndef __DEF_H__
#define __DEF_H__

#include <stdint.h>
#include "jlib.h"

typedef JMutex *LOCK_T;
#define LOCK_NEW j_mutex_new
#define LOCK_DEL j_mutex_free
#define AQUIRE_LOCK j_mutex_lock
#define RELEASE_LOCK j_mutex_unlock

#define TR_FATAL_ERROR_EXIT			exit(128)
#define __u8_str(p) ((uint8_t*)(p))
#define __str(p) ((char*)(p))

#endif
