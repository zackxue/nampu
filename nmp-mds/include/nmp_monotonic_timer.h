#ifndef __NMP_MONOTONIC_H__
#define __NMP_MONOTONIC_H__

#include <glib.h>

G_BEGIN_DECLS

void nmp_monotonic_timer_init( void );
guint nmp_get_monotonic_time( void );

G_END_DECLS

#endif	/* __NMP_MONOTONIC_H__ */
