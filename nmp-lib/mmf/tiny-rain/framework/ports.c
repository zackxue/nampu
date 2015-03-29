#include <string.h>
#include <assert.h>
#include "ports.h"

//#define BUG_ON(x) g_assert(!(x))
//#define BUG()	g_assert(0)
#define TR_ASSERT assert
#define tr_error(...)
#define tr_warning(...)

#define DEFAULT_RANGE_LOWER		25000
#define DEFAULT_RANGE_UPPER		30000

//#define ALIGN(size, align)	((size + align - 1)&(~(align - 1)))
#define BIT_BYTES(size)		(ALIGN((size), sizeof(uint8_t))/sizeof(uint8_t));

#define __require_lock(pc)	AQUIRE_LOCK((pc)->lock)
#define __release_lock(pc)	RELEASE_LOCK((pc)->lock)

/* 位图操作 */
typedef struct _BITMAP BITMAP;
struct _BITMAP
{
	int32_t	bits;
	uint8_t	map[0];
};


/*
 * 端口管理：数据结构及规则.
*/ 

typedef struct _TrPortsCtl TrPortsCtl;
struct _TrPortsCtl
{
	LOCK_T		lock;
	int32_t		min;		/* 最小端口号 */
	int32_t		max;		/* 最大端口号 + 1*/

	int32_t		used;
	int32_t		next;
	int32_t		reserved;

	BITMAP		*bitmap;		/* 使用位图 */
};


static TrPortsCtl tr_ports = {
	0, 0, 0, 0, 0, 0, NULL
};


static __inline__ BITMAP*
tr_bitmap_alloc(uint32_t size)
{
	uint32_t bytes;
	uint8_t *maps;

	bytes = BIT_BYTES(size);
	bytes += sizeof(BITMAP);

	maps = malloc(bytes);
	memset(maps, 0, bytes);

	((BITMAP*)maps)->bits = size;

	return (BITMAP*)maps;
}


static __inline__ void
tr_bitmap_free(BITMAP *bitmap)
{
	free(bitmap);
}


static __inline__ void
tr_bitmap_zero(BITMAP *bitmap)
{
	int32_t bytes;

	bytes = BIT_BYTES(bitmap->bits);
	memset(bitmap->map, 0, bytes);
}


/* 位图操作: 置位 */
static __inline__ int32_t
tr_bitmap_test_and_set(BITMAP *bitmap, int32_t bit)
{
	int32_t old, offset;

	BUG_ON(bit >= bitmap->bits);

	old = bitmap->map[bit / sizeof(uint8_t)];
	offset = (bit & (sizeof(uint8_t) - 1));
	bitmap->map[bit/sizeof(uint8_t)] |= 1 << offset;

	return old & (1 << offset);
}


/* 位图操作: 复位*/
static __inline__ void
tr_bitmap_reset(BITMAP *bitmap, int32_t bit)
{
	int32_t old, offset;

	BUG_ON(bit >= bitmap->bits);

	old = bitmap->map[bit / sizeof(uint8_t)];
	offset = (bit & (sizeof(uint8_t) - 1));

	if (!(old & (1 << offset)))
	{
		tr_error(
			"Bit '%d' was reset twice in bitmap '%p'.",
			bit, bitmap
		);

		BUG();
	}

	bitmap->map[bit/sizeof(uint8_t)] &= ~(1 << offset);
}


static __inline__ void
tr_bitmap_set_bits(BITMAP *bitmap, int32_t bits)
{
	BUG_ON(bitmap->bits < bits);
	bitmap->bits = bits;
}


/* 初始化管理结构 */
static __inline__ void
__tr_ports_init(TrPortsCtl *pc)
{
	memset(pc, 0, sizeof(*pc));
	pc->lock = LOCK_NEW();
}


void
tr_ports_init( void )
{
	__tr_ports_init(&tr_ports);
}


static __inline__ int32_t
__tr_ports_set_range(TrPortsCtl *pc, int32_t low, int32_t hi)
{
	int32_t size;
	BITMAP *new_map = NULL;

	size = hi - low;

	if (size > pc->max - pc->min)
	{
		new_map = tr_bitmap_alloc(size);
		if (!new_map)
		{
			tr_warning(
				"Alloc ports bitmap failed."
			);
			return -E_NOMEM;
		}

		tr_bitmap_free(pc->bitmap);
		pc->bitmap = new_map;
	}
	else
	{
		tr_bitmap_set_bits(pc->bitmap, size);
		tr_bitmap_zero(pc->bitmap);
	}

	pc->min = low;
	pc->max = hi;
	pc->used = 0;
	pc->next = pc->min;
	pc->reserved = 0;

	return 0;
}


/* 设置端口范围 */
static __inline__ int32_t
_tr_ports_set_range(TrPortsCtl *pc, int32_t low, int32_t hi)
{
	int32_t rc;
	TR_ASSERT(pc != NULL && hi > low && low > 0);
 
 	__require_lock(pc);
 	rc = __tr_ports_set_range(pc, low, hi);
 	__release_lock(pc);

	return 	rc;
}


static __inline__ void
__tr_ports_set_reserved(TrPortsCtl *pc, int32_t port)
{
	if (port >= pc->max || port < pc->min)
		return;

	if (!tr_bitmap_test_and_set(pc->bitmap, 
		port - pc->min))
	{
		++pc->reserved;
	}
}


/* 保留一个端口 */
static __inline__ void
_tr_ports_set_reserved(TrPortsCtl *pc, int32_t port)
{
	TR_ASSERT(pc != NULL);

	__require_lock(pc);
	__tr_ports_set_reserved(pc, port);
	__release_lock(pc);
}


static __inline__ int32_t
__tr_ports_get_one(TrPortsCtl *pc, int32_t *port)
{
	int32_t total;

	total = pc->max - pc->min;

	if (pc->used + pc->reserved >= total)
		return -E_OUTOFPORTS;

	for (;;)
	{
		if (--total < 0)
			break;

		if (!tr_bitmap_test_and_set(pc->bitmap,
				pc->next - pc->min))
		{
			*port = pc->next;

			++pc->used;
			++pc->next;

			if (pc->next >= pc->max)
				pc->next = pc->min;

			return 0;
		}

		if (++pc->next >= pc->max)
			pc->next = pc->min;
	}

	return -E_OUTOFPORTS;
}


/* 分配一个端口 */
static __inline__ int32_t
_tr_ports_get_one(TrPortsCtl *pc, int32_t *port)
{
	int32_t rc;
	TR_ASSERT(pc != NULL && port != NULL);

	__require_lock(pc);
	rc = __tr_ports_get_one(pc, port);
	__release_lock(pc);

	return rc;
}


static __inline__ void
__tr_ports_put_one(TrPortsCtl *pc, int32_t port)
{
	if (port >= pc->max || port < pc->min)
	{
		tr_error(
			"Put port '%d' failed, Not in range [%d, %d).",
			port, pc->min, pc->max
		);
		BUG();
	}

	tr_bitmap_reset(pc->bitmap, port - pc->min);
	--pc->used;

	BUG_ON(pc->used < 0);
}


/* 释放一个端口 */
static __inline__ void
_tr_ports_put_one(TrPortsCtl *pc, int32_t port)
{
	TR_ASSERT(pc != NULL);

	__require_lock(pc);	
	__tr_ports_put_one(pc, port);
	__release_lock(pc);
}


static __inline__ int32_t
__tr_ports_get_pair(TrPortsCtl *pc, int32_t *p_low, int32_t *p_hi)
{
	int32_t low, hi, total;

	total = pc->max - pc->min;

	if (pc->used + pc->reserved + 2 > total)
		return -E_OUTOFPORTS;

	for (;;)
	{
		for (;;)
		{
			if (--total < 0)
				return -E_OUTOFPORTS;

			if (__tr_ports_get_one(pc, &low))
				return -E_OUTOFPORTS;

			if (low & 0x1)
			{
				__tr_ports_put_one(pc, low);
				continue;
			}

			break;
		}

		for (;;)
		{
			if (__tr_ports_get_one(pc, &hi))
			{
				__tr_ports_put_one(pc, low);
				return -E_OUTOFPORTS;
			}

			if (hi == low + 1)	/* got it! */
			{
				*p_low = low;
				*p_hi = hi;
				return 0;
			}

			__tr_ports_put_one(pc, low);

			if (hi & 0x1)
			{
				__tr_ports_put_one(pc, hi);
				break;
			}
			else
			{
				if (--total < 0)
				{
					__tr_ports_put_one(pc, hi);
					return -E_OUTOFPORTS;
				}

				low = hi;
			}
		}
	}

	return -E_OUTOFPORTS;
}


/* 获得一对端口: n(偶数), n+1 */
static __inline__ int32_t
_tr_ports_get_pair(TrPortsCtl *pc, int32_t *p_low, int32_t *p_hi)
{
	int32_t rc;
	TR_ASSERT(pc != NULL && p_low != NULL && p_hi != NULL);

	__require_lock(pc);
	rc = __tr_ports_get_pair(&tr_ports, p_low, p_hi);
	__release_lock(pc);

	return rc;
}


static __inline__ void
__tr_ports_put_pair(TrPortsCtl *pc, int32_t low, int32_t hi)
{
	__tr_ports_put_one(pc, low);
	__tr_ports_put_one(pc, hi);
}


/* 释放一对端口 */
static __inline__ void
_tr_ports_put_pair(TrPortsCtl *pc, int32_t low, int32_t hi)
{
	TR_ASSERT(pc != NULL);

	__require_lock(pc);
	__tr_ports_put_pair(&tr_ports, low, hi);
	__release_lock(pc);
}


static __inline__ int32_t
__tr_ports_get_range(TrPortsCtl *pc, int32_t *p_low, int32_t *p_hi)
{
	if (pc->min >= pc->max)
		return -E_OUTOFPORTS;

	*p_low = pc->min;
	*p_hi = pc->max;

	return 0;
}


/* 获得端口范围 */
static __inline__ int32_t
_tr_ports_get_range(TrPortsCtl *pc, int32_t *p_low, int32_t *p_hi)
{
	int32_t rc;
	TR_ASSERT(pc != NULL && p_low != NULL && p_hi != NULL);

	__require_lock(pc);
	rc = __tr_ports_get_range(pc, p_low, p_hi);
	__release_lock(pc);

	return rc;
}


/* 导出函数：流媒体端口操作集 */

__export int32_t
tr_ports_set_range(int32_t low, int32_t hi)
{
	return _tr_ports_set_range(&tr_ports, low, hi);
}


__export int32_t
tr_ports_set_default_range( void )
{
	return tr_ports_set_range(DEFAULT_RANGE_LOWER,
		DEFAULT_RANGE_UPPER);
}


__export void
tr_ports_set_reserved(int32_t port)
{
	_tr_ports_set_reserved(&tr_ports, port);
}

__export int32_t
tr_ports_get_one(int32_t *p_port)
{
	return _tr_ports_get_one(&tr_ports, p_port);
}


__export void
tr_ports_put_one(int32_t port)
{
	_tr_ports_put_one(&tr_ports, port);
}


__export int32_t
tr_ports_get_pair(int32_t *p_low, int32_t *p_hi)
{
	return _tr_ports_get_pair(&tr_ports, p_low, p_hi);
}


__export void
tr_ports_put_pair(int32_t low, int32_t hi)
{
	_tr_ports_put_pair(&tr_ports, low, hi);
}


__export int32_t
tr_ports_get_range(int32_t *p_low, int32_t *p_hi)
{
	return _tr_ports_get_range(&tr_ports, p_low, p_hi);
}

//:~ End
