
#ifndef __MEDIA_SET_H__
#define __MEDIA_SET_H__

#include "def.h"
#include "nmplib.h"
#include "list.h"
#include "client.h"

// 管理相同media的client_node_t
typedef struct _media_node
{
    struct list_head list;
    int count;                 /* 打开客户端的个数 */
    atomic_t ref_count;          /* 计录客户端的个数,当ref_count为空即客户端为0时，在media_set释放media_node */
    char pu_id[32];            
    int channel;
    LOCK_T lock;
    client_node_t  cn_list;    /* 抽象的客户端链表 sinkers*/  
    void *user_data;

    POINTER_BACK_TO(media_set_t);
}media_node_t;

media_node_t *get_media_node(char *pu_id, int channel);

media_node_t *media_node_new(char* pu_id, int channel);
void media_node_del(media_node_t *mn);

int media_node_get_count(media_node_t *mn);

int media_node_add_client(media_node_t *mn, client_node_t *cn);
int media_node_del_client(media_node_t *mn, client_node_t *cn);

// 管理不同的pu_id, channel的media
typedef struct _media_set
{
    int count;                 /* 打开的媒体个数 */

    LOCK_T lock;
    media_node_t    mn_list;   /* 抽象的媒体链表 */
}media_set_t;

media_set_t *media_set_new();
void media_set_del(media_set_t *ms);

int media_set_add_node(media_set_t *ms, media_node_t *mn);
int media_set_del_node(media_set_t *ms, media_node_t *mn);

#endif
