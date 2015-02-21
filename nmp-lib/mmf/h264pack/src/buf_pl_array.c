#include <assert.h>
#include <string.h>
#include <pthread.h>

#include "buf_pool.h"
#include "buf_pl_array.h"

#define MTU_MULT    (400)

typedef struct pl_array_s {
    int size;
    int ref_count;
    void *hdl;
}pl_array_t;

static pl_array_t pl_array[3] = {
     {1*MTU_MULT, 0, NULL}
    ,{4*MTU_MULT, 0, NULL}
    ,{26*MTU_MULT, 0, NULL}
};
static pthread_mutex_t pl_mutex = PTHREAD_MUTEX_INITIALIZER;

void *buf_pl_get(int mtu)
{
    void *pl;
    int idx = (mtu<=pl_array[0].size)
                    ?0:((mtu<=pl_array[1].size)
                            ?1:((mtu<=pl_array[2].size)?2:-1));
    if(idx < 0)
    {
        printf("%s >>>>>>>>>> err, BUF_POOL SUPPORT MAX_MTU:%d\n", __FUNCTION__, 26*MTU_MULT);
        return NULL;
    }

    pthread_mutex_lock(&pl_mutex);

    if(!pl_array[idx].hdl)
    {
        pl_array[idx].hdl = buf_pool_new(64, pl_array[idx].size, BUF_FLAG_GROWTH|BUF_FLAG_MUTEX);
        printf("%s >>>>>>>>>> buf_pool_new size:%d\n", __FUNCTION__, pl_array[idx].size);
    }
#if 0
    pl_array[idx].ref_count++;
#endif
    pthread_mutex_unlock(&pl_mutex);
 
    return pl_array[idx].hdl;
}

void buf_pl_rel(void *pl)
{
#if 0
    int i;
    assert(pl);
    
    pthread_mutex_lock(&pl_mutex);
    
    for(i = 0; i < 3; i++)
    {
        if(pl == pl_array[i].hdl)
        {
            if(--pl_array[i].ref_count == 0)
            {
                buf_pool_unref(pl_array[i].hdl);
                pl_array[i].hdl = NULL;
                printf("%s >>>>>>>>>> buf_pool_unref size:%d\n", __FUNCTION__, pl_array[i].size);
            }
            break;
        }
    }
    pthread_mutex_unlock(&pl_mutex);
#endif
}


