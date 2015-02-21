/*
 *	author:	zyt
 *	time:	begin in 2012/7/24
 */
#ifndef __LOG_H_20120724__
#define __LOG_H_20120724__

#include <stdio.h>
#include <stddef.h>


//#define g_log_num (0x00)
#define g_log_num (0x01)

#define log_0(fmt, args ...) do {	\
	if (g_log_num & 0x01)	\
		printf("<log_0>   _________________________________________________________________ [error] %s[%d]:"	\
		fmt, __FUNCTION__, __LINE__, ##args);	\
} while (0)
	
#define log_1(fmt, args ...) do {	\
	if (g_log_num & 0x02)	\
		printf("<log_1> %s[%d]:"fmt, __FUNCTION__, __LINE__, ##args);	\
} while (0)
	
#define log_2(fmt, args ...) do {	\
	if (g_log_num & 0x04)	\
		printf("<log_2> %s[%d]:"fmt, __FUNCTION__, __LINE__, ##args);	\
} while (0)
	
#define log_3(fmt, args ...) do {	\
	if (g_log_num & 0x08)	\
		printf("<log_3> %s[%d]:"fmt, __FUNCTION__, __LINE__, ##args);	\
} while (0)

#define func_begin(fmt, args ...) do {	\
	if (g_log_num & 0x10)	\
		printf("<b> %s[%d]__ _"fmt, __FUNCTION__, __LINE__, ##args);		\
} while (0)


#define mem_error(fmt, args ...) do {	\
	if (1)	\
		printf("<mem_error> %s[%d]:"fmt, __FUNCTION__, __LINE__, ##args);	\
} while (0)
	

#define return_val_if_fail(expr, val) do {	\
	if (!(expr)) { \
		log_0("*** return_val_if_fail ***\n");	\
		return val; }	\
} while (0)

#define return_if_fail(expr) do {	\
	if (!(expr)) { \
		log_0("*** return_if_fail ***\n");	\
		return ; }	\
} while (0)

#define return_val_if_reached(val)	do {	\
	if (1) { \
		log_0("*** return_val_if_reached ***\n");	\
		return val; }	\
} while (0)


#define print printf

#define FUNCTION_BEGIN	func_begin("\n");


#endif /* __RTSP_LOG_H__ */

