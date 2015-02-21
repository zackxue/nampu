#ifndef __J_SCHEDULER_H__
#define __J_SCHEDULER_H__

#include "nmp_netio.h"

typedef struct _JScheduler JScheduler;

#ifdef __cplusplus
extern "C" {
#endif

JScheduler *j_scheduler_new(int loop_count);
void j_scheduler_sched_io(JScheduler *sched, JNetIO *io);
void j_scheduler_release(JScheduler *sched);

void *j_scheduler_add_timer(int timeout, int (*)(void*), void *data);
void j_scheduler_del_timer(void *handle);

#ifdef __cplusplus
}
#endif

#endif
