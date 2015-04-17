/*
 *          file: nmp_msg_impl.h
 *          description:
 *
 *          May 16th, 2013
 */

#ifndef __MSG_IMPL_H__
#define __MSG_IMPL_H__

#include "nmplib.h"
#include "nmp_net.h"
#include "nmp_io.h"

#define MAX_MSG_SIZE                256


enum
{
    RET_ACCEPTED = 0x1000,
    RET_BACK,
    RET_DROP
};

typedef struct message msg_t;

typedef void (*msg_fin_t)(void *data, size_t size);
typedef int (*msg_handler_t)(void *owner, msg_t *msg, int parm_id);

typedef struct msg_owner msg_owner_t;

struct msg_owner
{
    void *object;
    void*(*ref)(void*);
    void (*unref)(void*);
};

struct message
{
    int     id;
    int     seq;        /* Message Sequence Number. */

    void  *data;
    size_t size;

    nmpio_t     *io;     /* Where`s the message come from? */
    msg_owner_t owner;

    msg_fin_t fin;
};

typedef struct entry entry_t;

struct entry
{
    int           msg_id;
    int           parm_id;
    msg_handler_t handler;
};

typedef struct msg_map msg_map_t;

struct msg_map
{
    int count;
    entry_t entries[MAX_MSG_SIZE];
};

typedef struct msg_engine msg_engine_t;

struct msg_engine
{
    msg_map_t   *msg_map;       /* Message map. */
    nmp_bool_t immediate;            /*TRUE: Direct execution, does not produce the thread pool. */
                                /* FALSE: Message will delivery into the thread pool to execute. */
    nmp_threadpool_t *th_pool;       /* The thread pool to process the message. */
};



#define MSG_ID(msg)         ((msg)->id)
#define MSG_IO(msg)         ((msg)->io)
#define MSG_SEQ(msg)        ((msg)->seq)
#define MSG_DATA(msg)       ((msg)->data)
#define MSG_DATA_SIZE(msg)  ((msg)->size)


#define strcpy_connect_info0(pvalue_des, pvalue_src) \
        { \
        strcpy((pvalue_des)->session_id, (pvalue_src)->session_id); \
        strcpy((pvalue_des)->domain_id, (pvalue_src)->domain_id); \
        strcpy((pvalue_des)->pu_id, (pvalue_src)->pu_or_gu_id); \
        }

#define strcpy_connect_info1(pvalue_des, pvalue_src) \
        { \
        strcpy((pvalue_des)->session_id, (pvalue_src)->session_id); \
        strcpy((pvalue_des)->domain_id, (pvalue_src)->domain_id); \
        strcpy((pvalue_des)->pu_or_gu_id, (pvalue_src)->pu_id); \
        }

#define strcpy_connect_info2(pvalue_des, pvalue_src) \
        { \
        strcpy((pvalue_des)->session_id, (pvalue_src)->session_id); \
        strcpy((pvalue_des)->domain_id, (pvalue_src)->domain_id); \
        strcpy((pvalue_des)->gu_id, (pvalue_src)->pu_or_gu_id); \
        }

#define strcpy_connect_info3(pvalue_des, pvalue_src) \
        { \
        strcpy((pvalue_des)->session_id, (pvalue_src)->session_id); \
        strcpy((pvalue_des)->domain_id, (pvalue_src)->domain_id); \
        strcpy((pvalue_des)->pu_or_gu_id, (pvalue_src)->gu_id); \
        }


#ifdef __cplusplus
extern "C" {
#endif

msg_t *alloc_new_msg(int id, void *data, size_t size, int seq);
msg_t *alloc_new_msg_2(int id, void *data, size_t size, int seq, msg_fin_t fin);

void dealloc_msg_data(void *data, size_t size);

msg_t *attach_msg_io(msg_t *msg, nmpio_t *io);
msg_t *attach_msg_owner(msg_t *msg, msg_owner_t *owner);

#define set_msg_id(msg, msg_id) ((msg)->id = msg_id)
void set_msg_data(msg_t *msg, void *data, size_t size);
void set_msg_data_2(msg_t *msg, void *data, size_t size, msg_fin_t fin);

void free_msg(msg_t *_msg);
void free_msg_2(msg_t *_msg);


msg_engine_t *create_msg_engine(nmp_bool_t immediate, int max_thread_num);
void destory_msg_engine(msg_engine_t *me);

int register_msg_handler(msg_engine_t *me, int msg_id, 
        int parm_id, msg_handler_t handler);
void deliver_msg(msg_engine_t *me, msg_t *msg);


#ifdef __cplusplus
    }
#endif


#endif  //__MSG_IMPL_H__

