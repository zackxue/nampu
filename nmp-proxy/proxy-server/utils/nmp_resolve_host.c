
#include <ifaddrs.h>

#include "nmplib.h"
#include "nmp_net.h"

#include "nmp_proxy_log.h"
#include "nmp_proxy_info.h"
#include "nmp_resolve_host.h"
#include "nmp_proxy_server.h"


#define MAX_HOST_RESOLVE_TIMES      3
#define MAX_HOST_IN_VALID_TIME      (2*60)


enum
{
    HOST_INVALID,
    HOST_RESOLVING,
    HOST_SUCCESS
};

typedef struct proxy_host proxy_host_t;

struct proxy_host
{
    nmp_mutex_t  *lock;
    //atomic_t ref_count;
    int      state;
    int      state_timer;
    int      resolve_times;     //事不过三, 解析错误三次自动放弃

    char     host[MAX_HOST_LEN];
    char     addr[MAX_IP_LEN];
};

typedef struct proxy_host_list proxy_hlist_t;

struct proxy_host_list
{
    nmp_mutex_t *lock;
    nmp_list_t  *list;

    void *timer;
};

static proxy_hlist_t *hlist_manager = NULL;


static void free_host_node(void *node, size_t size)
{//pretend
    return ;
}

static void add_host_node(proxy_host_t *host)
{
    proxy_hlist_t *host_list;
    if (!hlist_manager)
        return ;
    else
        host_list = hlist_manager;

    nmp_mutex_lock(host_list->lock);
    host_list->list = nmp_list_add_tail(host_list->list, host);
    nmp_mutex_unlock(host_list->lock);
}

static void foreach_host_node(void *orig, void *custom)
{
    int flag = -1;
    proxy_task_t *task = NULL;
    proxy_host_t *node = (proxy_host_t*)orig;

    nmp_mutex_lock(node->lock);
    switch (node->state)
    {
        case HOST_INVALID:
            node->state = HOST_RESOLVING;
            if (MAX_HOST_RESOLVE_TIMES > node->resolve_times++)
                flag = 0;
            else
                node->state_timer = 0;      //Die
            break;
        case HOST_RESOLVING:
            break;
        case HOST_SUCCESS:
            node->state_timer--;
            break;
    }
    nmp_mutex_unlock(node->lock);

    if (!flag)
    {
        task = proxy_new_task(RESOLVE_HOST, node, 
                sizeof(proxy_host_t), free_host_node, NULL);
        if (task)
            proxy_thread_pool_push(task);
    }
}

static int check_host_node_timeout(void *orig, void *custom)
{
    int retval;
    proxy_host_t *node = (proxy_host_t*)orig;

    nmp_mutex_lock(node->lock);
    if (0 >= node->state_timer)
        retval = 0;
    else
        retval = -1;
    nmp_mutex_unlock(node->lock);

    return retval;
}

static proxy_host_t *
reap_timeout_host_node(proxy_hlist_t *hlist)
{
    nmp_list_t *link;
    proxy_host_t *node = NULL;

    link = nmp_list_find_custom(hlist->list, 
            (void*)NULL, check_host_node_timeout);
    if (link)
    {
        node = (proxy_host_t*)nmp_list_data(link);
        hlist->list = nmp_list_delete_link(hlist->list, link);
    }

    return node;
}

static int manage_host_list_timer(void *data)
{
    proxy_host_t *node;
    proxy_hlist_t *host_list = (proxy_hlist_t*)data;

    nmp_mutex_lock(host_list->lock);
    nmp_list_foreach(host_list->list, foreach_host_node, (void*)host_list);

    for (;;)
    {
        node = reap_timeout_host_node(host_list);
        if (!node)
            break;

        show_info("delete one host node[%s]!!!!!!!!!!!!!!!\n", node->host);
        nmp_mutex_free(node->lock);
        nmp_del(node, proxy_host_t, 1);
    }
    nmp_mutex_unlock(host_list->lock);

    return 0;
}

static int compare_domain_name(void *orig, void *custom)
{
    int retval;
    proxy_host_t *node = (proxy_host_t*)orig;

    nmp_mutex_lock(node->lock);
    if (!strcmp(custom, node->host))
        retval = 0;
    else
        retval = -1;
    nmp_mutex_unlock(node->lock);

    return retval;
}

static proxy_host_t *
find_host_node_by_host(proxy_hlist_t *manager, const char *host, 
    char *addr, size_t size)
{
    nmp_list_t *link = NULL;
    proxy_host_t *node = NULL;

    NMP_ASSERT(host);

    if (!manager)
        return NULL;

    nmp_mutex_lock(manager->lock);
    link = nmp_list_find_custom(manager->list, 
                (void*)host, (nmp_compare_custom)compare_domain_name);
    if (link)
    {
        node = (proxy_host_t*)nmp_list_data(link);
        //node->state_timer = MAX_HOST_IN_VALID_TIME;

        if (HOST_SUCCESS == node->state)
            memcpy(addr, node->addr, size);
    }
    nmp_mutex_unlock(manager->lock);

    return node;
}

///////////////////////////////////////////////////////////////////////////////////////////
int proxy_resolve_host_init()
{
    hlist_manager = (proxy_hlist_t*)nmp_new(proxy_hlist_t, 1);
    hlist_manager->lock  = nmp_mutex_new();
    hlist_manager->list  = NULL;
    hlist_manager->timer = nmp_set_timer(1000, manage_host_list_timer, 
                            (void*)hlist_manager);
    return 0;
}

void proxy_resolve_host_cleanup()
{
    if (hlist_manager->timer)
        nmp_del_timer(hlist_manager->timer);

    if (hlist_manager->lock)
        nmp_mutex_free(hlist_manager->lock);

    nmp_del(hlist_manager, proxy_hlist_t, 1);
    hlist_manager = NULL;
}

void 
resolve_host_proxy(const void *host)
{
    int err;
    struct sockaddr_in socket;
    proxy_host_t *node = (proxy_host_t*)host;
    NMP_ASSERT(host);

    memset(&socket, 0, sizeof(struct sockaddr_in));
    err = nmp_resolve_host(&socket, node->host, 0);

    nmp_mutex_lock(node->lock);
    if (err)
    {
        node->state = HOST_INVALID;
        show_info("Resolve host '%s' failed, err:'%d'.\n", 
            node->host, err);
    }
    else
    {
        if (HOST_SUCCESS != node->state)
        {
            char ip_addr[MAX_IP_LEN];
            memset(ip_addr, 0, sizeof(ip_addr));
            inet_ntop(socket.sin_family, &socket.sin_addr, 
                ip_addr, sizeof(ip_addr));

            node->state = HOST_SUCCESS;
            node->state_timer = MAX_HOST_IN_VALID_TIME;
            strncpy(node->addr, ip_addr, sizeof(node->addr)-1);
            show_info("Resolve host '%s' to '%s'.\n", 
                node->host, node->addr);
        }
    }
    nmp_mutex_unlock(node->lock);
}

char *
proxy_resolve_host_immediate(const char *host, char *addr, size_t size)
{
    int err;
    struct sockaddr_in socket;
    NMP_ASSERT(host && addr && !strlen(addr));

    find_host_node_by_host(hlist_manager, host, addr, size);
    if (!strlen(addr))
    {
        memset(&socket, 0, sizeof(struct sockaddr_in));
        err = nmp_resolve_host(&socket, (char*)host, 0);
        if (err)
        {
            show_debug("nmp_resolve_host[%s] err: %d\n", host, err);
            return NULL;
        }

        inet_ntop(socket.sin_family, &socket.sin_addr, 
            addr, size);
    }
    else
        return NULL;

    return addr;
}

char *
proxy_resolve_host(const char *host, char *addr, size_t size)
{
    NMP_ASSERT(host && addr && !strlen(addr));

    if (!find_host_node_by_host(hlist_manager, host, addr, size))
    {
        proxy_host_t *node;
        node = (proxy_host_t*)nmp_new0(proxy_host_t, 1);
        node->lock = nmp_mutex_new();
        node->state = HOST_INVALID;
        node->state_timer = MAX_HOST_IN_VALID_TIME;
        node->resolve_times = 0;
        strncpy(node->host, host, sizeof(node->host)-1);

        add_host_node(node);
        return NULL;
    }

    if (strlen(addr))
        return addr;
    else
        return NULL;
}

void 
proxy_get_local_ip(local_ifs_t *ifs)
{
    int addr;
    struct sockaddr_in *addr_in;
    struct ifaddrs *ifaddr = NULL, *pifs = NULL;

    NMP_ASSERT(ifs);

    if (-1 == getifaddrs(&ifaddr))
        return ;

    pifs = ifaddr;
    memset(ifs, 0, sizeof(local_ifs_t));

    while (pifs)
    {
        if (AF_INET == pifs->ifa_addr->sa_family)
        {
            addr_in = (struct sockaddr_in*)pifs->ifa_addr;
            addr = addr_in->sin_addr.s_addr;

            ifs->ifa[ifs->count].addr = htonl(addr);
            ++ifs->count;
        }

        if (MAX_IP_COUNT <= ifs->count)
            break;

        pifs = pifs->ifa_next;
    }

    freeifaddrs(ifaddr);
}


