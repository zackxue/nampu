/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/

#ifndef __TINY_RAIN_LOG_H__
#define __TINY_RAIN_LOG_H__

#include <stdio.h>
#include "def.h"

enum
{
	LVL_ERROR = 0,
	LVL_WARN,
	LVL_INFO,
	LVL_DEBUG
};

typedef void (*log_handler_t)(uint8_t *message);
void ___tr_log(int32_t level, char *file, int32_t line, const char *fmt, ...);

#define LOG_V(...) ___tr_log(LVL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_I(...) ___tr_log(LVL_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_W(...) ___tr_log(LVL_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_E(...) ___tr_log(LVL_ERROR, __FILE__, __LINE__, __VA_ARGS__)

void init_log_facility( void );
int32_t tr_log_set_verbose(int32_t level);
void tr_log_set_handler(log_handler_t handler);

#endif	//__TINY_RAIN_LOG_H__
