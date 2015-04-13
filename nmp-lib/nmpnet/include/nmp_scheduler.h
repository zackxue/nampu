#ifndef __NMP_SCHEDULER_H__
#define __NMP_SCHEDULER_H__

#include "nmp_netio.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _nmp_scheduler nmp_scheduler_t;

nmp_scheduler_t *nmp_scheduler_new(int loop_count);
void nmp_scheduler_sched_io(nmp_scheduler_t *sched, nmp_netio_t *io);
void nmp_scheduler_release(nmp_scheduler_t *sched);

void *nmp_scheduler_add_timer(int timeout, int (*)(void*), void *data);
void nmp_scheduler_del_timer(void *handle);

#ifdef __cplusplus
}
#endif

#endif
