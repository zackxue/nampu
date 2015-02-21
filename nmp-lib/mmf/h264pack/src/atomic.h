#ifndef __atomic_h__
#define __atomic_h__

#if defined _X86_
#warning "CPU Type: _X86_"
#if defined (__GNUC__) && (__GNUC__ >= 4) /* since 4.1.2 */
#warning "__GNUC__ >= 4"
typedef unsigned int atomic_t;
#define atomic_set(p, val) ((*(p)) = (val))
#define atomic_get(p) __sync_add_and_fetch(p, 0)
#define atomic_add(p, val)  __sync_add_and_fetch(p, val)
#define atomic_sub(p, val)  __sync_sub_and_fetch(p, val)
#define atomic_inc(p) atomic_add(p, 1)
#define atomic_dec(p) atomic_sub(p, 1)
#define atomic_dec_and_test_zero(p) (__sync_sub_and_fetch(p, 1) == 0)
#else
#warning "__GNUC__ < 4"
typedef unsigned int atomic_t;
#define atomic_set(p, val) ((*(p)) = (val))
#define atomic_get(p) (*(p))
#define atomic_add(p, val) ((*(p)) += (val))
#define atomic_sub(p, val) ((*(p)) -= (val))
#define atomic_inc(p) atomic_add(p, 1)
#define atomic_dec(p) atomic_sub(p, 1)
#define atomic_dec_and_test_zero(p) (atomic_sub(p, 1), !*(p))
#endif
#endif //_X86_

#if defined _ARM_
#warning "CPU Type: _ARM_"
#if defined (__GNUC__) && (__GNUC__ >= 4) /* since 4.1.2 */
#warning "__GNUC__ >= 4"
typedef unsigned int atomic_t;
#define atomic_set(p, val) ((*(p)) = (val))
#define atomic_get(p) __sync_add_and_fetch(p, 0)
#define atomic_add(p, val)  __sync_add_and_fetch(p, val)
#define atomic_sub(p, val)  __sync_sub_and_fetch(p, val)
#define atomic_inc(p) atomic_add(p, 1)
#define atomic_dec(p) atomic_sub(p, 1)
#define atomic_dec_and_test_zero(p) (__sync_sub_and_fetch(p, 1) == 0)
#else
#warning "__GNUC__ < 4"
#include "atomic_armv6.h"
typedef _atomic_t atomic_t;
#define atomic_set(p, val) _atomic_set(p, val)
#define atomic_get(p) _atomic_add(0, p)
#define atomic_add(p, val)  _atomic_add(val, p)
#define atomic_sub(p, val)  _atomic_sub(val, p)
#define atomic_inc(p) _atomic_inc(p)
#define atomic_dec(p) _atomic_dec(p)
#define atomic_dec_and_test_zero(p) _atomic_sub_and_test(1, p)
#endif
#endif //_ARM_

#endif //__atomic_h__
