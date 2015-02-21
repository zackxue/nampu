#ifndef __RC_LOG_H__
#define __RC_LOG_H__

#include <stdio.h>

#define RC_LOG_FATAL	1
#define RC_LOG_ERR		1
#define RC_LOG_WARN		2
#define RC_LOG_DEBUG	3
#define RC_LOG_INFO		4
#define RC_LOG_VERBOSE 	5

#define FNC_LOG_FATAL	1
#define FNC_LOG_ERR		1
#define FNC_LOG_WARN	2
#define FNC_LOG_DEBUG	3
#define FNC_LOG_INFO	4
#define FNC_LOG_VERBOSE 5

#define EC_BIND			1
#define EC_LISTEN		2
#define EC_ARGS			3


#define rc_log(level, ...) do{}while(0)	//fprintf(stderr, __VA_ARGS__)
#define fnc_log(level, ...) do{}while(0)	//fprintf(stderr, __VA_ARGS__)

#endif	/* __RC_LOG_H__ */
