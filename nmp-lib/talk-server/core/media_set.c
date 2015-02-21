
#include "log.h"
#include "media_set.h"

void __media_node_finalize(media_node_t *mn)
{//@{fixme}
    LOG_I("del media_node:%p pu_is:%s channel:%d", mn, mn->pu_id, mn->channel);

    // µ±media_nodeÖÐµÄcn_listÁ´±í½áµãÈ«²¿É¾³ýºó£¬±ã¿É´Ómedia_setÖÐÉ¾³ýmedia_node½áµã
    media_set_del_node(media_set_new(), mn);  // É¾³ýmedia_node_t½áµã
    media_node_del(mn);
}

media_node_t *__media_node_ref(media_node_t *mn)
{
    __OBJECT_REF(mn);      
}

void __media_node_unref(media_node_t *mn)
{
    __OBJECT_UNREF(mn, __media_node);
}

media_node_t *get_media_node(char *pu_id, int channel)
{
    if (!pu_id)
    {
        return NULL;
    }
    
    return media_node_new(pu_id, channel);
}

media_node_t *__media_node_alloc(char *pu_id, int channel)
{
    media_node_t *mn;
    if (!pu_id)
    {
        goto _error;
    }

    mn = (media_node_t *)calloc(1, sizeof(media_node_t));
    if (!mn)
    {
        goto _error;
    }
    
    INIT_LIST_HEAD(&mn->cn_list.list);
    mn->count = 0;
    mn->ref_count = 0;
    mn->lock = LOCK_NEW();
    strcpy(mn->pu_id, pu_id);
    mn->channel = channel;

    return mn;
_error:
    assert(-1);
    return NULL;
}

media_node_t *media_node_new(char* pu_id, int channel)
{
    media_node_t *mn, *mn_next;
    media_set_t *ms = media_set_new();
    if (!ms)
    {
        goto _error;
    }
    
    extern media_node_t *__find_media_node_from_media_set(media_set_t *ms, char *pu_id, int channel);
    if (mn = __find_media_node_from_media_set(ms, pu_id, channel))
    {
        return mn;
    }
    
    // ²»´æÔÚ£¬new media_node_t
    mn = __media_node_alloc(pu_id, channel);
    if (!mn)
    {
        goto _error;
    }
    
    media_set_add_node(ms, mn);  // ½«meida_node_t ¼ÓÈëmedia_set_t

    return mn;
_error:

    return NULL;
}

void media_node_del(media_node_t *mn)
{
    if (mn)
    {
        mn->count = 0;
        LOCK_DEL(mn->lock);
    
        free(mn);
    }
}

int media_node_get_count(media_node_t *mn)
{
    int count;
    if (!mn)
    {
        assert(-1);
        return -1;
    }

    AQUIRE_LOCK(mn->lock);
    count = mn->count;
    RELEASE_LOCK(mn->lock);

    return count;
}

void __media_node_print_client(media_node_t *mn)
{
    int i = 0;
    client_node_t *cn, *cn_next;

    LOG_I("media_node:%p client_node count:%d pu_id:%s channel:%d", mn, mn->count, mn->pu_id, mn->channel);
    list_for_each_entry_safe(cn, cn_next, &mn->cn_list.list, list)
    {
        LOG_I("client[%d]:%p fd:%d", i++, cn->c, cn->c->fd);
    }
}

int media_node_add_client(media_node_t *mn, client_node_t *cn)
{
    if (!mn || !cn)
    {
        return -1;
    }

    __media_node_ref(mn);
    
    AQUIRE_LOCK(mn->lock);
    mn->count++;
    list_add_tail(&cn->list, &mn->cn_list.list);
  
    LOG_I("[INFO] media_node:%p client_node count:%d add client_node:%p fd:%d", mn, mn->count, cn, cn->c->fd);
    
    if (1)
    {
        __media_node_print_client(mn);
    }
    RELEASE_LOCK(mn->lock);

    return 0;
}

int media_node_del_client(media_node_t *mn, client_node_t *cn)
{
    if (!mn || !cn)
    {
        return -1;
    }
    
    AQUIRE_LOCK(mn->lock);
    mn->count--;
    list_del(&cn->list);

    LOG_I("[INFO] media_node:%p client_node count:%d del client_node:%p fd:%d", mn, mn->count, cn, cn->c->fd);

    if (1)
    {
        __media_node_print_client(mn);
    }
    
    __media_node_unref(mn);
    
    RELEASE_LOCK(mn->lock);

    return 0;
}

media_set_t *media_set_new()
{
    static media_set_t *g_media_set = NULL;
    if (!g_media_set)
    {
        g_media_set = (media_set_t *)calloc(1, sizeof(media_set_t));
        g_media_set->count = 0;
        g_media_set->lock = LOCK_NEW();
        INIT_LIST_HEAD(&g_media_set->mn_list.list);
    }

    return g_media_set;
}

void media_set_del(media_set_t *ms)
{
    if (!ms)
    {
        return;
    }
    
    ms->count = 0;
    LOCK_DEL(ms->lock);
    free(ms);
}

void __media_set_print_media_node(media_set_t *ms)
{
    int i = 0;
    media_node_t *mn, *mn_next;

    LOG_I("media_set:%p meida_node count:%d", ms, ms->count);
    list_for_each_entry_safe(mn, mn_next, &ms->mn_list.list, list)
    {
        LOG_I("meida_node[%d]:%p pu_id:%s channel:%d", i++, mn, mn->pu_id, mn->channel);
    }
}

int media_set_add_node(media_set_t *ms, media_node_t *mn)
{
    if (!ms || !mn)
    {
        assert(-1);
        return -1;
    }
    
    AQUIRE_LOCK(ms->lock);
    ms->count++;
    list_add_tail(&mn->list, &ms->mn_list.list);

    LOG_I("[INFO] media_set:%p media_node count:%d add media_node:%p pu_id:%s channel:%d", 
        ms, ms->count, mn, mn->pu_id, mn->channel);
        
    if (1)
    {
        __media_set_print_media_node(ms);
    }

    RELEASE_LOCK(ms->lock);

    return 0;
}

int media_set_del_node(media_set_t *ms, media_node_t *mn)
{
    if (!ms || !mn)
    {
        assert(-1);
        return -1;
    }
    
    AQUIRE_LOCK(ms->lock);
    ms->count--;
    list_del(&mn->list);

    LOG_I("[INFO] media_set:%p media_node count:%d del media_node:%p pu_id:%s channel:%d", 
        ms, ms->count, mn, mn->pu_id, mn->channel);

    if (1)
    {
        __media_set_print_media_node(ms);
    }

    RELEASE_LOCK(ms->lock);

    return 0;
}

media_node_t *__find_media_node_from_media_set(media_set_t *ms, char *pu_id, int channel)
{
    media_node_t *mn, *mn_next;
    if (!ms)
    {
        assert(-1);
        return NULL;
    }
    
    AQUIRE_LOCK(ms->lock);
    list_for_each_entry_safe(mn, mn_next, &ms->mn_list.list, list)
    {
        if ((!memcmp(mn->pu_id, pu_id, strlen(pu_id))) &&
            (mn->channel == channel))
        { // Èç¹ûÏàÍ¬µÄmedia_node_tÒÑ¾­´æÔÚ£ ·µ»Ø¸Ãmedia_node_t
            RELEASE_LOCK(ms->lock);
            return mn;
        }
    }
    RELEASE_LOCK(ms->lock);
    return NULL;
}

