/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/

#ifndef __TINY_RAIN_PORTS_RANGE_H__
#define __TINY_RAIN_PORTS_RANGE_H__

#include "def.h"

BEGIN_NAMESPACE

//@{
#define E_NOMEM			1
#define E_OUTOFPORTS 	1
#define __export
//:}

/* 初始化端口 */
void tr_ports_init( void );

/* 设置缺省端口范围 */
int32_t tr_ports_set_default_range( void );

/* 设置端口范围 */
int32_t tr_ports_set_range(int32_t low, int32_t hi);

/* 设置要排除的端口 */
void tr_ports_set_reserved(int32_t port);

/* 获得一个端口 */
int32_t tr_ports_get_one(int32_t *p_port);

/* 释放一个端口 */
void tr_ports_put_one(int32_t port);

/* 获得一对端口: n(偶数), n+1 */
int32_t tr_ports_get_pair(int32_t *p_low, int32_t *p_hi);

/* 释放一对端口 */
void tr_ports_put_pair(int32_t low, int32_t hi);

/* 获得端口范围 */
int32_t tr_ports_get_range(int32_t *p_low, int32_t *p_hi);

END_NAMESPACE

#endif	//__TINY_RAIN_PORTS_RANGE_H__
