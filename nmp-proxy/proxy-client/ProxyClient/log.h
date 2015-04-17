
#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>
#include <stdarg.h>

void log_out(const char *fmt, ...);

#define LOG(...) \
	log_out(## __VA_ARGS__);

#endif