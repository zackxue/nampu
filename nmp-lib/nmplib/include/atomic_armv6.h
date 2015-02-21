#ifndef __atomic_armv6_h__
#define __atomic_armv6_h__


#define ATOMIC_INIT			{0}

typedef struct { volatile int counter; } atomic_t;

static inline void _atomic_set(atomic_t *v, int i)
{
	unsigned long tmp;

	__asm__ __volatile__("@ atomic_set\n"
"1:	ldrex	%0, [%1]\n"
"	strex	%0, %2, [%1]\n"
"	teq	%0, #0\n"
"	bne	1b"
	: "=&r" (tmp)
	: "r" (&v->counter), "r" (i)
	: "cc");
}

static inline int _atomic_add_return(int i, atomic_t *v)
{
	unsigned long tmp;
	int result;

	__asm__ __volatile__("@ atomic_add_return\n"
"1:	ldrex	%0, [%2]\n"
"	add	%0, %0, %3\n"
"	strex	%1, %0, [%2]\n"
"	teq	%1, #0\n"
"	bne	1b"
	: "=&r" (result), "=&r" (tmp)
	: "r" (&v->counter), "Ir" (i)
	: "cc");

	return result;
}

static inline int _atomic_sub_return(int i, atomic_t *v)
{
	unsigned long tmp;
	int result;

	__asm__ __volatile__("@ atomic_sub_return\n"
"1:	ldrex	%0, [%2]\n"
"	sub	%0, %0, %3\n"
"	strex	%1, %0, [%2]\n"
"	teq	%1, #0\n"
"	bne	1b"
	: "=&r" (result), "=&r" (tmp)
	: "r" (&v->counter), "Ir" (i)
	: "cc");

	return result;
}

static inline void _atomic_clear_mask(unsigned long mask, unsigned long *addr)
{
	unsigned long tmp, tmp2;

	__asm__ __volatile__("@ atomic_clear_mask\n"
"1:	ldrex	%0, %2\n"
"	bic	%0, %0, %3\n"
"	strex	%1, %0, %2\n"
"	teq	%1, #0\n"
"	bne	1b"
	: "=&r" (tmp), "=&r" (tmp2)
	: "r" (addr), "Ir" (mask)
	: "cc");
}

#define _atomic_add(i, v)	_atomic_add_return(i, v)
#define _atomic_inc(v)		_atomic_add_return(1, v)
#define _atomic_sub(i, v)	_atomic_sub_return(i, v)
#define _atomic_dec(v)		_atomic_sub_return(1, v)

#define _atomic_inc_and_test(v)	(_atomic_add_return(1, v) == 0)
#define _atomic_dec_and_test(v)	(_atomic_sub_return(1, v) == 0)
#define _atomic_inc_return(v)    (_atomic_add_return(1, v))
#define _atomic_dec_return(v)    (_atomic_sub_return(1, v))
#define _atomic_sub_and_test(i, v) (_atomic_sub_return(i, v) == 0)

#endif //__atomic_armv6_h__
