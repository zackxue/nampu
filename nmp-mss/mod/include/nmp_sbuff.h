#ifndef __NMP_STREAM_BUFF_H__
#define __NMP_STREAM_BUFF_H__


#include <glib.h>
#include "nmp_guid.h"

G_BEGIN_DECLS

typedef struct __NmpSBuff NmpSBuff;
typedef struct __NmpSBuffOps NmpSBuffOps;

struct __NmpSBuffOps
{
	gsize sb_size;
	gint (*sb_init)(NmpSBuff *sb);
	gint (*sb_pause)(NmpSBuff *sb);
	gint (*sb_write)(NmpSBuff *sb, gchar *data, gsize size, gint flags);
	gint (*sb_flush)(NmpSBuff *sb);
	void (*sb_kill)(NmpSBuff *sb);
	void (*sb_fin)(NmpSBuff *sb);
};

struct __NmpSBuff
{
	NmpGuid	    guid;
	NmpSBuffOps *ops;
	gint        ref_count;
	gint		pending;
	gint		action;
	guint       flags;
	guint		local_tags;
};


NmpSBuff *nmp_sbuff_new(NmpGuid *guid, gint flags, gint local_tags);
NmpSBuff *nmp_sbuff_ref(NmpSBuff *sb);
void nmp_sbuff_unref(NmpSBuff *sb);
void nmp_sbuff_kill_unref(NmpSBuff *sb);

gint nmp_sbuff_write(NmpSBuff *sb, gchar *data, gsize size, gint flags);
gint nmp_sbuff_flush(NmpSBuff *sb);
gint nmp_sbuff_pause(NmpSBuff *sb);
gint nmp_sbuff_flush_pending(NmpSBuff *sb);

G_END_DECLS

#endif	/* __NMP_STREAM_BUFF_H__ */
