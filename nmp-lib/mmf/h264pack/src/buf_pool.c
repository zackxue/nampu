#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "atomic.h"
#include "buf_pool.h"

#define _offsetof(type, f) ((size_t) \
		((char *)&((type *)0)->f - (char *)(type *)0))

#define _container_of(ptr, type, member) ({ \
		(type *)( (char *)(ptr) - _offsetof(type,member) );})


#define IS_SET(f) (pool->flags & (f))
#define GROWTH_NUM			32
#define MAX_CACHED_OBJS		128
//#define _DEBUG

typedef struct buf_s {
	struct buf_s *next;
	void *owner;
    atomic_t ref_cout;
    uint8_t  data[0];
}buf_t;

typedef struct _buf_pool_s {
	buf_t *head;
    int objs;
	int size;
    int flags;
    atomic_t ref_cout;
    pthread_mutex_t mutex;
}_buf_pool_t;


static __inline__ void __buf_clear(_buf_pool_t *pool)
{
	buf_t *pobj;

	while (pool->head)
	{
		pobj = pool->head;
		pool->head = pobj->next;
		--pool->objs;
		free(pobj);
	}
}


static __inline__ int __buf_fill(_buf_pool_t *pool, int num)
{
	buf_t *pobj;

	while (num-- > 0)
	{
		pobj = (buf_t*)malloc(pool->size);	// real_size = pool->size - sizeof(buf_t)
		if (!pobj)
		{
			printf("buf oom!\n");
			return -1;
		}
		pobj->next = pool->head;
		pool->head = pobj;
		++pool->objs;
	}
printf(">>>>>>>>>>>>>>>>>> __buf_fill!!\n");
	return 0;
}


buf_pool_t *buf_pool_new(int num, int size, int flags)
{
	int i;
    
    if(num < 0 || size <= 0)
    {
        return NULL;
    }

    _buf_pool_t *pool = (_buf_pool_t*)calloc(1, sizeof(_buf_pool_t));
    pool->head = NULL;
    pool->objs  = 0;
    pool->size = size;
    pool->flags= flags;
    atomic_set(&pool->ref_cout, 1);

	if (__buf_fill(pool, GROWTH_NUM))
	{
		printf("buf alloc failed\n");
		__buf_clear(pool);
		free(pool);
		return NULL;
	}

#ifdef _DEBUG
    if(IS_SET(BUF_FLAG_GROWTH)) printf("buf flags: BUF_FLAG_GROWTH\n");
    if(IS_SET(BUF_FLAG_MUTEX))  printf("buf flags: BUF_FLAG_MUTEX\n");
#endif
    if(IS_SET(BUF_FLAG_MUTEX))
	    pthread_mutex_init(&pool->mutex, NULL);	

    return pool;
}

static void buf_pool_del(buf_pool_t *pl)
{
    int i;
    _buf_pool_t *pool = (_buf_pool_t*)pl;

	__buf_clear(pool);
	if (pool->objs)
	{
		printf("buf pool: BUG()!!!!!!!\n");
	}

    if(IS_SET(BUF_FLAG_MUTEX))
        pthread_mutex_destroy(&pool->mutex);
    free(pool);

    return;
}



void buf_pool_unref(buf_pool_t *pl)
{
    _buf_pool_t *pool = (_buf_pool_t*)pl;
    
    if(atomic_dec_and_test_zero(&pool->ref_cout))
    {
        buf_pool_del(pl);
    }
}

buf_pool_t *buf_pool_ref(buf_pool_t* pl)
{
    _buf_pool_t *pool = (_buf_pool_t*)pl;
    if(pool)
    {
        atomic_inc(&pool->ref_cout);
    }
    return pool;
}


static __inline__ buf_t *__buf_new(_buf_pool_t *pool)
{
	buf_t *pobj;

	if (pool->head)
	{
		pobj = pool->head;
		pool->head = pobj->next;
		--pool->objs;
		atomic_set(&pobj->ref_cout,1);
		pobj->owner = pool;
		buf_pool_ref((buf_pool_t*)pool);

		return pobj;
	}

	if (__buf_fill(pool, GROWTH_NUM))
	{
		return NULL;
	}
	return __buf_new(pool);
}

void *buf_new(buf_pool_t* pl)
{
	buf_t *ptr;
    _buf_pool_t *pool = (_buf_pool_t*)pl;
 
   if(IS_SET(BUF_FLAG_MUTEX))
       pthread_mutex_lock(&pool->mutex);

  	ptr = __buf_new(pool);
    
    if(IS_SET(BUF_FLAG_MUTEX))
        pthread_mutex_unlock(&pool->mutex);

    return ptr ? ptr->data : NULL;
}

void *buf_ref(void *p)
{
    if(p == NULL) return NULL;
    
    buf_t *buf = _container_of(p, buf_t, data);
   
    atomic_inc(&buf->ref_cout);
    return buf;
}

static inline int buf_free(buf_t *buf)
{
	int i;
	void *temp;
    
    if(buf == NULL || buf->owner == NULL) return -1;
    
    _buf_pool_t *pool = (_buf_pool_t*)buf->owner;

	if (pool == NULL) return -1;

    if(IS_SET(BUF_FLAG_MUTEX))
        pthread_mutex_lock(&pool->mutex);

	if (pool->objs < MAX_CACHED_OBJS)
	{
		buf->next = pool->head;
		pool->head = buf;
		++pool->objs;
	}
	else
	{
		free(buf);
	}

    if(IS_SET(BUF_FLAG_MUTEX))
        pthread_mutex_unlock(&pool->mutex);

    buf_pool_unref(pool);
  
    return 0;
}

void  buf_unref(void *p)
{
    if(p == NULL) return;
   
    buf_t *buf = _container_of(p, buf_t, data);

    if(atomic_dec_and_test_zero(&buf->ref_cout))
    {
        buf_free(buf);
    }
    return;
}


