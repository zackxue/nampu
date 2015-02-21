#ifndef __JPF_PORTS_H__
#define __JPF_PORTS_H__

#include <glib.h>

//@{
#define E_NOMEM			1
#define E_OUTOFPORTS 	1
#define __export
//:}

/* 设置缺省端口范围 */
gint jpf_media_ports_set_default_range( void );

/* 设置端口范围 */
gint jpf_media_ports_set_range(gint low, gint hi);

/* 设置要排除的端口 */
void jpf_media_ports_set_reserved(gint port);

/* 获得一个端口 */
gint jpf_media_ports_get_one(gint *p_port);

/* 释放一个端口 */
void jpf_media_ports_put_one(gint port);

/* 获得一对端口: n(偶数), n+1 */
gint jpf_media_ports_get_pair(gint *p_low, gint *p_hi);

/* 释放一对端口 */
void jpf_media_ports_put_pair(gint low, gint hi);

/* 获得端口范围 */
gint jpf_media_ports_get_range(gint *p_low, gint *p_hi);

#endif	//__JPF_PORTS_H__
