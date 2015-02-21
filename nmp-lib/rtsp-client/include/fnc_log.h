#ifndef __FNC_LOG_H__
#define __FNC_LOG_H__

#include <stdlib.h>
#include <stdio.h>

#define RET_BUG_EXIT		0x0A

#define FNC_LOG_ERR 	3

#define fnc_log(x, ...) fprintf(stderr, __VA_ARGS__)


#define BUG() \
do {\
	fnc_log(FNC_LOG_ERR, "BUG AT FILE:%s LINE:%d.", __FILE__, __LINE__); \
	exit (RET_BUG_EXIT); \
} while (0)

#endif	/* __FNC_LOG_H__ */
