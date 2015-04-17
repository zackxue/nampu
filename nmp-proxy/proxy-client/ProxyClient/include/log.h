
#ifndef __LOG_H__
#define __LOG_H__

void log_out(const char *fmt, ...);

#define LOG(...) \
	log_out(## __VA__ARGS__);

#endif