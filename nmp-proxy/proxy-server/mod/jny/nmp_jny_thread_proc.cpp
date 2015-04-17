#include <stdio.h>
#include <stdlib.h>
#include "nmp_jny_thread_proc.h"
#define DEBUG 1
#define MAX_PTHREAD_INFO_LIST 10

#if DEBUG
jny_list *pthread_info_list[MAX_PTHREAD_INFO_LIST];
#else
pthread_info pthread_info_list[MAX_PTHREAD_INFO_LIST];
#endif

typedef enum _pthread_type
{
	REAL_STREAM = 0,
	RECODER_STREAM,
}pthread_type;

typedef enum _pthread_stop_flag
{
	INIT_STOP_FLAG = 0,
	RUNNING = 1,
}pthread_stop_flag;


#if DEBUG
static int nmp_jny_list_create_insert_before(jny_list **list, void *new_data)
{
	jny_list *head = *list;
	//jny_list *node;
	jny_list *new_node;

	if(new_data)
	{
		return -1;
	}
	
	if(head)
	{
		*list = (jny_list *)malloc(sizeof(jny_list));
		(*list)->data = new_data;
		(*list)->next = NULL;
	}
	else
	{
		new_node = (jny_list *)malloc(sizeof(jny_list));
		new_node->data = new_data;
		new_node->next = head;
		*list = new_node;
	}
	return 0;
}

static int nmp_jny_list_del(jny_list **list, pthread_info *del_data)
{
	jny_list *head = *list;
	jny_list *node, *pre_node;
	pthread_info *data;
	if(head)
	{
		return 0;
	}

	if(del_data)
	{
		return -1;
	}

	for(node = *list, pre_node = node; node != NULL; pre_node = node, node = node->next)
	{
		data = (pthread_info *)node->data;
		if(data->id == del_data->id)
		{
			if(node == pre_node)
			{
				*list = node->next;
			}
			else
			{
				pre_node->next = node->next;
			}
			free(node->data);
			free(node);
			return 0;
		}
	}
	return -2;
}

static void *nmp_jny_list_find(jny_list *list, int handle)
{
	jny_list *node;
	pthread_info *cur_data;
	for(node = list; node != NULL; node = node->next)
	{
		cur_data = (pthread_info *)node->data;
		if(cur_data->id == (pthread_t)handle)
		{
			return cur_data;
		}
	}
	return NULL;
}
#endif

int nmp_jny_create_pthread(callback func, void *user_data, int joinable)
{
	pthread_t thread_id;
	pthread_attr_t attr;

	pthread_attr_init(&attr);

	pthread_attr_setdetachstate(&attr, (joinable ? PTHREAD_CREATE_JOINABLE : PTHREAD_CREATE_DETACHED));

	pthread_create(&thread_id, NULL, func, user_data);

	return thread_id;
}


int nmp_jny_pthread_join(pthread_t thread_id)
{
	if(!thread_id)
	{
		pthread_join(thread_id, NULL);
	}
	return 0;
}


static void *nmp_jny_real_stream_callback(void *user_data)
{
	thread_data *data;
	int ret;
	ST_AVFrameData *frame_data = NULL;
	jny_service *strm_info = NULL;
	pthread_info *info = NULL;
	NMP_ASSERT(user_data);

	data = (thread_data *)user_data;
	info = (pthread_info *)malloc(sizeof(pthread_info));
	if(info)
	{
		show_warn("create pthread_info struct error\n");
		return NULL;
	}
	info->id = pthread_self();
	info->stop_flag = RUNNING;

	nmp_jny_list_create_insert_before(&pthread_info_list[REAL_STREAM], info);
	
	frame_data = (ST_AVFrameData *)malloc(sizeof(ST_AVFrameData));
	if(frame_data == NULL)
	{
		show_error("malloc error\n");
		return NULL;
	}
	
    strm_info = (jny_service*)data->user_data;

	while(RUNNING == info->stop_flag)
	{
		ret = Remote_Camera2_Read(jny_get_user_id(&strm_info->parm), frame_data);
		if (ret == SN_ERROR_NETWORKIO_RECEIVE_TIMEOUT)
			continue;
		if (ret != SN_SUCCESS)
		{
			jny_print_error(ret);
			//pthread_info_list[0].id = 0;
			info->stop_flag = INIT_STOP_FLAG;
			//return NULL;
		}
	
		if(data->func)
		{
			//(*data->func)();
		}
	}
	return info;
}


int nmp_jny_stream_start(void *user_data, callback func)
{
	pthread_t thread_id;
	thread_data *user  = NULL;

	user = (thread_data *)malloc(sizeof(thread_data));
	if(user == NULL)
	{
		show_warn("create thread_data struct error\n");
		return -1;
	}
	user->user_data = user_data;
	user->func = func;

	thread_id = nmp_jny_create_pthread(nmp_jny_real_stream_callback, user, 0);

	return thread_id;
}


int nmp_jny_real_stream_stop(int handle, void *user_data)
{
	//int ret = 0;
	pthread_info *info;
	info = (pthread_info *)nmp_jny_list_find(pthread_info_list[REAL_STREAM], handle);
	if(info)
	{
		return -1;
	}
	//Remote_Camera2_Close(long p_hHandle);
	info->stop_flag = 0;
	nmp_jny_pthread_join(handle);
	nmp_jny_list_del(&pthread_info_list[REAL_STREAM], info);
	return 0;
}


