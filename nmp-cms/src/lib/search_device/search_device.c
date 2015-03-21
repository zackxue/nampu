/*
 * jpf-search-device:
 * Copyright (C) 2013 by hegui <heguijiss@163.com>
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>

#include "mb_api.h"
#include "search_device.h"


#define MAX_SEARCH_TIMEOUT          5  //s
#define MAX_INCREASE_SIZ            128

typedef struct _list
{
    void *data;
    int condition;
    struct _list *next;
}list_t;

#ifdef _DEBUG_
static void show_device_list(search_array_t *array)
{
    int i;
    printf("\n=========== show device list ===========\n");
    for (i=0; i<array->count; i++)
    {
        printf("dst_id: %s, %s\n", array->result[i].dst_id,
            array->result[i].nmp_srch.dev_info.dev_ip);
    }
    printf("\n");
}
#endif

static int check_destination_id(list_t *list, char *dst_id)
{
    int ret = 0;
    search_result_t *result;

    assert(dst_id);

    while (list)
    {
        result = (search_result_t*)list->data;
        if (!strcmp(dst_id, result->dst_id))
        {
            ret = 1;
            break;
        }
        list = list->next;
    }

    return ret;
}

static int search_notify(mb_cu_notify_t *notify_handle, mb_cu_parm_t *parm)
{
    unsigned int condition;
    int addr0, addr1, addr2, addr3;
    search_result_t *result;
    list_t *head, *list, *node;

    assert(notify_handle && parm);

    if (MB_NOTIFY_ERR_0 != parm->error)
    {
        printf("search error: %d, dst_id: %s\n", parm->error, parm->dst_id);
        return -1;
    }

    head = (list_t*)notify_handle->user_arg;

    if (check_destination_id(head->next, parm->dst_id))
        return -1;

#if _DEBUG_
    nmp_search_t *nmp_srch = (nmp_search_t*)parm->data;
    device_info_t *dev_info = &nmp_srch->dev_info;
    printf("===>dst_id: %s", parm->dst_id);
    printf("  ip: %s, id: %s\n", dev_info->dev_ip, dev_info->pu_id);
#endif

    result = (search_result_t*)malloc(sizeof(search_result_t));
    memcpy(result->dst_id, parm->dst_id, sizeof(result->dst_id));
    memcpy(&result->nmp_srch, parm->data, sizeof(nmp_search_t));

    sscanf(result->nmp_srch.dev_info.dev_ip, "%d.%d.%d.%d",
        &addr0, &addr1, &addr2, &addr3);
    condition = addr0<<24 | addr1<<16 | addr2<<8 | addr3;

    for (node=list=head; list->next; node=list->next)
    {
        if (node->condition > condition)
            break;
        list = node;
    }

    node = malloc(sizeof(list_t));
    node->condition = condition;
    node->data = result;
    node->next = NULL;

    node->next = list->next;
    list->next = node;

    return 0;
}

static int set_notify(mb_cu_notify_t *notify_handle, mb_cu_parm_t *parm)
{
    int *result;
    assert(notify_handle && parm);

#ifdef _DEBUG_
    printf("====>dst_id: %s\n", parm->dst_id);
#endif

    result = (int*)notify_handle->user_arg;
    *result = parm->error;

    return 0;
}


int search_device_init()
{
    int ret;

    ret = mb_cu_init(GROUP_IP, GROUP_PORT, GROUP_PORT+1);
    if (!ret)
    {
    }
    else
        printf("mb_cu_init failuer\n");

    return ret;
}

search_array_t *search_device()
{
    int total_size, capacity = 0;
    list_t head, *list, *node;
    search_array_t *array;

    mb_query_t *query_handle;
    mb_cu_notify_t notify_handle;

    memset(&head, 0, sizeof(list_t));
    notify_handle.user_arg = (void*)&head;
    notify_handle.callback = search_notify;

#ifdef _DEBUG_
    printf("====================== start search device ======================\n");
#endif

    query_handle = mb_query(N_MB_Nmp_Search_Id, &notify_handle);
    sleep(MAX_SEARCH_TIMEOUT);

#ifdef _DEBUG_
    printf("====================== stop search device ======================\n");
#endif

    mb_release(query_handle);

    for (list=&head; list->next; list=list->next)
        capacity++;
    printf("capacity: %d<!!!!!!!!!!!!!!!!!!!!!!!!!!\n", capacity);

    if (capacity)
    {
        total_size = sizeof(search_array_t) + capacity*sizeof(search_result_t);
        array = (search_array_t*)malloc(total_size);
        array->count = 0;
        array->capacity = capacity;

        for (list=head.next; (node=list); list=list->next)
        {
            memcpy(&array->result[array->count++],
                node->data, sizeof(search_result_t));
            free(node->data);
            free(node);
        }
    }

#ifdef _DEBUG_
    show_device_list(array);
#endif

    return array;
}

void destory_search_result(search_array_t *array)
{
    free(array);
}

int set_platform_info(char *dst_id, nmp_redirect_t *nmp_red)
{
    int timeout = 3;    //s
    int is_sync = 1;
    int err_no = 0;

    mb_cu_notify_t notify_handle;

    notify_handle.user_arg = &err_no;
    notify_handle.callback = set_notify;

    mb_cfg_set(N_MB_Nmp_Redirect_Id, dst_id,
        nmp_red->user_info.usr,
        nmp_red->user_info.pwd,
        timeout,
        sizeof(redirect_t),
        &nmp_red->redirect,
        &notify_handle, is_sync);

    return err_no;
}

int search_device_uninit()
{
    mb_cu_uinit();
    return 0;
}



