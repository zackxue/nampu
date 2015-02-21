#ifndef __buf_pool_h__
#define __buf_pool_h__

enum BUF_FLAG_E {
     BUF_FLAG_GROWTH = 0x01
    ,BUF_FLAG_MUTEX  = 0x02
};

typedef void buf_pool_t;

buf_pool_t*
     buf_pool_new(int num, int size, int flags);
buf_pool_t*
     buf_pool_ref(buf_pool_t* pl);
void buf_pool_unref(buf_pool_t *pl);

void *buf_new(buf_pool_t* pl);
void *buf_ref(void *p);
void  buf_unref(void *p);

#endif //__buf_pool_h__
