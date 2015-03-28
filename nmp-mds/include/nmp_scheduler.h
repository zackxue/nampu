#ifndef __NMP_SCHEDULER_H__
#define __NMP_SCHEDULER_H__

#define WEIGHT_STREAM_OUT			1
#define	WEIGHT_STREAM_IN			5

void nmp_scheduler_init( void );
void nmp_scheduler_add(GSource *source, guint weight);
void nmp_scheduler_del(GSource *source);

#endif	/* __NMP_SCHEDULER_H__ */
