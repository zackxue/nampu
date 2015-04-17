#ifndef __J_JNY_THREAD_PROC_H__
#define __J_JNY_THREAD_PROC_H__

#include <pthread.h>
#include "NvdcDll.h"
#include "nmp_jny_service.h"


typedef struct _jny_list
{
	void *data;
	struct _jny_list *next;
}jny_list;

typedef void *(*callback)(void *user_data);

typedef struct _pthread_info
{
	int stop_flag;		//0:结束， 1:线程运行
	pthread_t id;
	//struct _pthread_info *next;
}pthread_info;

typedef struct _thread_data
{
	void *user_data;
	callback func;
}thread_data;



#endif