#include <string.h>

#include "nmplib.h"

#include "nmp_proxy_log.h"
#include "nmp_service_template.h"

typedef struct basic_service_manager basic_service_manager_t;

struct basic_service_manager
{
    nmp_mutex_t *lock;
    nmp_list_t  *list;   /* link all basic service object*/
};


/* Manager all basic service object, when none left, free self. */
static basic_service_manager_t *g_basic_service_manager = NULL;



static __inline__ void 
__add_service_template_unlock(basic_service_manager_t *bs_manager, 
    service_template_t *srv_tm)
{
    bs_manager->list = nmp_list_add_tail(bs_manager->list, srv_tm);
}

int add_service_template(service_template_t *srv_tm)
{
    basic_service_manager_t *bs_manager = g_basic_service_manager;
    NMP_ASSERT(srv_tm);

    if (!bs_manager)
    {
        bs_manager = (basic_service_manager_t*)nmp_new(basic_service_manager_t, 1);
        bs_manager->lock = nmp_mutex_new();
        bs_manager->list = NULL;
        g_basic_service_manager = bs_manager;
    }

    if (srv_tm->init)
    {
        if(!(*srv_tm->init)(srv_tm))
        {
            nmp_mutex_lock(bs_manager->lock);
            __add_service_template_unlock(bs_manager, srv_tm);
            nmp_mutex_unlock(bs_manager->lock);
        }
        else
        {
            show_warn("Init service template[%s] failure!\n", 
                srv_tm->service_name);
			goto INIT_FAIL;
        }
    }
    else
    {
        show_warn("Service template[%s] is without functio[init]!\n", 
            srv_tm->service_name);
		goto INIT_FAIL;
    }

    return 0;

INIT_FAIL:
    show_info("[%s]Init fail, exit.\nBye!\n", srv_tm->service_name);
    exit(0);
}

static __inline__ void 
__rm_service_template_unlock(basic_service_manager_t *bs_manager, 
    service_template_t *srv_tm)
{
    bs_manager->list = nmp_list_remove(bs_manager->list, srv_tm);
    if (srv_tm->cleanup)
    {
        (*srv_tm->cleanup)(srv_tm);
    }
}

int rm_service_template(service_template_t *srv_tm)
{
    basic_service_manager_t *bs_manager = g_basic_service_manager;
    NMP_ASSERT(srv_tm);

    if (!bs_manager)
        return -1;

    nmp_mutex_lock(bs_manager->lock);
    __rm_service_template_unlock(bs_manager, srv_tm);
    if (!bs_manager->list)
    {
        nmp_mutex_unlock(bs_manager->lock);
        nmp_mutex_free(bs_manager->lock);
        nmp_del(bs_manager, basic_service_manager_t, 1);
        g_basic_service_manager = NULL;
    }
    else
        nmp_mutex_unlock(bs_manager->lock);

    return 0;
}

static int compare_service_name(void *orig, void *custom)
{
    return strcmp(custom, ((service_template_t*)orig)->service_name);
}

static __inline__ service_template_t *
__find_service_template_by_name_unlock(
    basic_service_manager_t *bs_manager, const char *srv_name)
{
    nmp_list_t *list;
    service_template_t *srv_tm = NULL;

    list = nmp_list_find_custom(nmp_list_first(bs_manager->list), 
                (void*)srv_name, compare_service_name);
    if (list)
    {
        srv_tm = (service_template_t*)nmp_list_data(list);
    }

    return srv_tm;
}

service_template_t *
find_service_template_by_name(const char *srv_name)
{
    service_template_t *retval = NULL;
    basic_service_manager_t *bs_manager = g_basic_service_manager;
    NMP_ASSERT(srv_name);

    if (bs_manager)
    {
        nmp_mutex_lock(bs_manager->lock);
        retval = __find_service_template_by_name_unlock(bs_manager, srv_name);
        nmp_mutex_unlock(bs_manager->lock);
    }

    return retval;
}

static void foreach_service_basic(void *orig, void *custom)
{
    service_template_t *srv_tm = (service_template_t*)orig;
    basic_service_manager_t *bs_manager = (basic_service_manager_t*)custom;

    if (srv_tm->cleanup)
    {
        (*srv_tm->cleanup)(srv_tm);
    }

    __rm_service_template_unlock(bs_manager, srv_tm);
}

void cleanup_all_service_template()
{
    basic_service_manager_t *bs_manager = g_basic_service_manager;
    if (!bs_manager)
        return ;

    nmp_mutex_lock(bs_manager->lock);
    nmp_list_foreach(bs_manager->list, foreach_service_basic, (void*)bs_manager);
    nmp_mutex_unlock(bs_manager->lock);
}

