#include <string.h>
#include <glib.h>
//#include "nmp_debug.h"
//#include "nmp_errno.h"

#include "ports.h"

#define BUG_ON(x) g_assert(!(x))
#define BUG()	g_assert(0)
#define G_ASSERT g_assert
#define jpf_error(...)
#define jpf_warning(...)

#define DEFAULT_RANGE_LOWER		20000
#define DEFAULT_RANGE_UPPER		30000
	
#define ALIGN(size, align)	((size + align - 1)&(~(align - 1)))
#define BIT_BYTES(size)		(ALIGN((size), sizeof(gchar))/sizeof(gchar));

#define __require_lock(pc)	g_static_mutex_lock(&(pc)->lock)
#define __release_lock(pc)	g_static_mutex_unlock(&(pc)->lock)

/* 位图操作 */
typedef struct _BITMAP BITMAP;
struct _BITMAP
{
	gint	bits;
	gchar	map[0];
};


/*
 * 端口管理：数据结构及规则.
*/ 

typedef struct _JpfPortsCtl JpfPortsCtl;
struct _JpfPortsCtl
{
	GStaticMutex lock;
	gint		min;		/* 最小端口号 */
	gint		max;		/* 最大端口号 + 1*/

	gint		used;
	gint		next;
	gint		reserved;

	BITMAP		*bitmap;		/* 使用位图 */
};


static JpfPortsCtl jpf_rtsp_media_server_ports = {
	G_STATIC_MUTEX_INIT, 0, 0, 0, 0, 0, NULL
};


static __inline__ BITMAP*
jpf_bitmap_alloc(gsize size)
{
	gsize bytes;
	gchar *maps;

	bytes = BIT_BYTES(size);
	bytes += sizeof(BITMAP);

	maps = g_malloc(bytes);
	memset(maps, 0, bytes);

	((BITMAP*)maps)->bits = size;

	return (BITMAP*)maps;
}


static __inline__ void
jpf_bitmap_free(BITMAP *bitmap)
{
	g_free(bitmap);
}


static __inline__ void
jpf_bitmap_zero(BITMAP *bitmap)
{
	gint bytes;

	bytes = BIT_BYTES(bitmap->bits);
	memset(bitmap->map, 0, bytes);
}


/* 位图操作: 置位 */
static __inline__ gint
jpf_bitmap_test_and_set(BITMAP *bitmap, gint bit)
{
	gint old, offset;

	BUG_ON(bit >= bitmap->bits);

	old = bitmap->map[bit / sizeof(gchar)];
	offset = (bit & (sizeof(gchar) - 1));
	bitmap->map[bit/sizeof(gchar)] |= 1 << offset;

	return old & (1 << offset);
}


/* 位图操作: 复位*/
static __inline__ void
jpf_bitmap_reset(BITMAP *bitmap, gint bit)
{
	gint old, offset;

	BUG_ON(bit >= bitmap->bits);

	old = bitmap->map[bit / sizeof(gchar)];
	offset = (bit & (sizeof(gchar) - 1));

	if (!(old & (1 << offset)))
	{
		jpf_error(
			"Bit '%d' was reset twice in bitmap '%p'.",
			bit, bitmap
		);

		BUG();
	}

	bitmap->map[bit/sizeof(gchar)] &= ~(1 << offset);
}


static __inline__ void
jpf_bitmap_set_bits(BITMAP *bitmap, gint bits)
{
	BUG_ON(bitmap->bits < bits);
	bitmap->bits = bits;
}


/* 初始化管理结构 */
static __inline__ void
jpf_ports_init(JpfPortsCtl *pc)
{
	memset(pc, 0, sizeof(*pc));
	g_static_mutex_init(&pc->lock);
}


static __inline__ gint
__jpf_ports_set_range(JpfPortsCtl *pc, gint low, gint hi)
{
	gint size;
	BITMAP *new_map = NULL;

	size = hi - low;

	if (size > pc->max - pc->min)
	{
		new_map = jpf_bitmap_alloc(size);
		if (!new_map)
		{
			jpf_warning(
				"Alloc ports bitmap failed."
			);
			return -E_NOMEM;
		}

		jpf_bitmap_free(pc->bitmap);
		pc->bitmap = new_map;
	}
	else
	{
		jpf_bitmap_set_bits(pc->bitmap, size);
		jpf_bitmap_zero(pc->bitmap);
	}

	pc->min = low;
	pc->max = hi;
	pc->used = 0;
	pc->next = pc->min;
	pc->reserved = 0;

	return 0;
}


/* 设置端口范围 */
static __inline__ gint
jpf_ports_set_range(JpfPortsCtl *pc, gint low, gint hi)
{
	gint rc;
	G_ASSERT(pc != NULL && hi > low && low > 0);
 
 	__require_lock(pc);
 	rc = __jpf_ports_set_range(pc, low, hi);
 	__release_lock(pc);

	return 	rc;
}


static __inline__ void
__jpf_ports_set_reserved(JpfPortsCtl *pc, gint port)
{
	if (port >= pc->max || port < pc->min)
		return;

	if (!jpf_bitmap_test_and_set(pc->bitmap, 
		port - pc->min))
	{
		++pc->reserved;
	}
}


/* 保留一个端口 */
static __inline__ void
jpf_ports_set_reserved(JpfPortsCtl *pc, gint port)
{
	G_ASSERT(pc != NULL);

	__require_lock(pc);
	__jpf_ports_set_reserved(pc, port);
	__release_lock(pc);
}


static __inline__ gint
__jpf_ports_get_one(JpfPortsCtl *pc, gint *port)
{
	gint total;

	total = pc->max - pc->min;

	if (pc->used + pc->reserved >= total)
		return -E_OUTOFPORTS;

	for (;;)
	{
		if (--total < 0)
			break;

		if (!jpf_bitmap_test_and_set(pc->bitmap,
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
static __inline__ gint
jpf_ports_get_one(JpfPortsCtl *pc, gint *port)
{
	gint rc;
	G_ASSERT(pc != NULL && port != NULL);

	__require_lock(pc);
	rc = __jpf_ports_get_one(pc, port);
	__release_lock(pc);

	return rc;
}


static __inline__ void
__jpf_ports_put_one(JpfPortsCtl *pc, gint port)
{
	if (port >= pc->max || port < pc->min)
	{
		jpf_error(
			"Put port '%d' failed, Not in range [%d, %d).",
			port, pc->min, pc->max
		);
		BUG();
	}

	jpf_bitmap_reset(pc->bitmap, port - pc->min);
	--pc->used;

	BUG_ON(pc->used < 0);
}


/* 释放一个端口 */
static __inline__ void
jpf_ports_put_one(JpfPortsCtl *pc, gint port)
{
	G_ASSERT(pc != NULL);

	__require_lock(pc);	
	__jpf_ports_put_one(pc, port);
	__release_lock(pc);
}


static __inline__ gint
__jpf_ports_get_pair(JpfPortsCtl *pc, gint *p_low, gint *p_hi)
{
	gint low, hi, total;

	total = pc->max - pc->min;

	if (pc->used + pc->reserved + 2 > total)
		return -E_OUTOFPORTS;

	for (;;)
	{
		for (;;)
		{
			if (--total < 0)
				return -E_OUTOFPORTS;

			if (__jpf_ports_get_one(pc, &low))
				return -E_OUTOFPORTS;

			if (low & 0x1)
			{
				__jpf_ports_put_one(pc, low);
				continue;
			}

			break;
		}

		for (;;)
		{
			if (__jpf_ports_get_one(pc, &hi))
			{
				__jpf_ports_put_one(pc, low);
				return -E_OUTOFPORTS;
			}

			if (hi == low + 1)	/* got it! */
			{
				*p_low = low;
				*p_hi = hi;
				return 0;
			}

			__jpf_ports_put_one(pc, low);

			if (hi & 0x1)
			{
				__jpf_ports_put_one(pc, hi);
				break;
			}
			else
			{
				if (--total < 0)
				{
					__jpf_ports_put_one(pc, hi);
					return -E_OUTOFPORTS;
				}

				low = hi;
			}
		}
	}

	return -E_OUTOFPORTS;
}


/* 获得一对端口: n(偶数), n+1 */
static __inline__ gint
jpf_ports_get_pair(JpfPortsCtl *pc, gint *p_low, gint *p_hi)
{
	gint rc;
	G_ASSERT(pc != NULL && p_low != NULL && p_hi != NULL);

	__require_lock(pc);
	rc = __jpf_ports_get_pair(&jpf_rtsp_media_server_ports, p_low, p_hi);
	__release_lock(pc);

	return rc;
}


static __inline__ void
__jpf_ports_put_pair(JpfPortsCtl *pc, gint low, gint hi)
{
	__jpf_ports_put_one(pc, low);
	__jpf_ports_put_one(pc, hi);
}


/* 释放一对端口 */
static __inline__ void
jpf_ports_put_pair(JpfPortsCtl *pc, gint low, gint hi)
{
	G_ASSERT(pc != NULL);

	__require_lock(pc);
	__jpf_ports_put_pair(&jpf_rtsp_media_server_ports, low, hi);
	__release_lock(pc);
}


static __inline__ gint
__jpf_ports_get_range(JpfPortsCtl *pc, gint *p_low, gint *p_hi)
{
	if (pc->min >= pc->max)
		return -E_OUTOFPORTS;

	*p_low = pc->min;
	*p_hi = pc->max;

	return 0;
}


/* 获得端口范围 */
static __inline__ gint
jpf_ports_get_range(JpfPortsCtl *pc, gint *p_low, gint *p_hi)
{
	gint rc;
	G_ASSERT(pc != NULL && p_low != NULL && p_hi != NULL);

	__require_lock(pc);
	rc = __jpf_ports_get_range(pc, p_low, p_hi);
	__release_lock(pc);

	return rc;
}


/* 导出函数：流媒体端口操作集 */

__export gint
jpf_media_ports_set_range(gint low, gint hi)
{
	return jpf_ports_set_range(&jpf_rtsp_media_server_ports, low, hi);
}


__export gint
jpf_media_ports_set_default_range( void )
{
	return jpf_media_ports_set_range(DEFAULT_RANGE_LOWER,
		DEFAULT_RANGE_UPPER);
}


__export void
jpf_media_ports_set_reserved(gint port)
{
	jpf_ports_set_reserved(&jpf_rtsp_media_server_ports, port);
}

__export gint
jpf_media_ports_get_one(gint *p_port)
{
	return jpf_ports_get_one(&jpf_rtsp_media_server_ports, p_port);
}


__export void
jpf_media_ports_put_one(gint port)
{
	jpf_ports_put_one(&jpf_rtsp_media_server_ports, port);
}


__export gint
jpf_media_ports_get_pair(gint *p_low, gint *p_hi)
{
	return jpf_ports_get_pair(&jpf_rtsp_media_server_ports, p_low, p_hi);
}


__export void
jpf_media_ports_put_pair(gint low, gint hi)
{
	jpf_ports_put_pair(&jpf_rtsp_media_server_ports, low, hi);
}


__export gint
jpf_media_ports_get_range(gint *p_low, gint *p_hi)
{
	return jpf_ports_get_range(&jpf_rtsp_media_server_ports, p_low, p_hi);
}

//:~ End
