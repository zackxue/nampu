
#ifndef __CONFIG_H__
#define __CONFIG_H__


#ifdef WIN32

	#include "./../wind32/mxml.h"
	#pragma comment(lib, "./../wind32/mxmld.lib")

	#define __inline__		__inline

	#define snprintf(str, size, fmt, ...) \
			do {\
				_snprintf(str, size-1, fmt, ##__VA_ARGS__); \
				((char*)str)[size-1] = '\0'; \
			} while (0)

#endif


#endif	//__CONFIG_H__