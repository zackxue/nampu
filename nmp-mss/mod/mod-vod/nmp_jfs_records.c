#include <string.h>
#include "nmp_jfs_records.h"
#include "nmp_errno.h"
#include "nmp_debug.h"
#include "nmp_utility.h"
#include "stor_api.h"
#include "stream_api.h"
#include "nmp_sdl.h"
#include <fcntl.h>
#include "rtp.h"

#define PROPERTY_FORMAT		"g=%d:d=%d:p=%d:f=%d:ds=%d:de=%d:is=%d:ie=%d"
#define MAX_BUF_LEN			(16*1024)

typedef struct __NmpQueryBlock NmpQueryBlock;
struct __NmpQueryBlock
{
	gint	row;
	gint	length;

	gint	entries;

	NmpRecordList *l;
};


static gint
nmp_jfs_foreach_file(stor_file_query_t *handle, stor_fragment_t *fragment)
{
	NmpQueryBlock *qb = (NmpQueryBlock*)handle->user_arg;
	gint list_len;
	gchar *property;

	++qb->entries;

	if (qb->entries - 1 < qb->row)
		return 0;

	list_len = nmp_record_list_length(qb->l);
	if (list_len >= qb->length)
		return 0;

	property = g_strdup_printf(
		PROPERTY_FORMAT,
		fragment->group_id,
		fragment->disk_no,
		fragment->part_no,
		fragment->file_no,
		fragment->data_start,
		fragment->data_end,
		fragment->index_start,
		fragment->index_end
	);

	nmp_record_list_add(qb->l, fragment->start_time, fragment->end_time,
		fragment->data_end - fragment->data_start, fragment->tags, "jfs-segments", property);

	g_free(property);
	return 0;
}


static gint
nmp_jfs_records_get_list(NmpRecords *rs, NmpRecordList *l, gchar *ch,
	time_t start, time_t end, gint row, gint length, guint flags)
{
	stor_file_query_t query;
	gint ret;
	NmpQueryBlock qb;

	if (length < 0)
	{
		nmp_warning(
			"jfs get '%s' record list failed, length < 0.", ch
		);
		return -1;
	}

	qb.row = row;
	qb.length = length;
	qb.entries = 0;
	qb.l = l;

	memset(&query, 0, sizeof(query));
	strncpy(query.ch_name, ch, STOR_CH_NAME_LEN - 1);
	query.group_id = 1;
	query.start_time = (gint)start;
	query.end_time = (gint)end;
	query.tags = flags;	/* User tags, RC_TYPE_AUTO | RC_TYPE_ALARM etc. */
	query.user_arg = &qb;
	query.callback = nmp_jfs_foreach_file;

	ret = stor_file_query(&query);
	if (ret)
	{
		nmp_warning(
			"jfs get '%s' record list failed.", ch
		);
		return -1;
	}

	nmp_print(
		"jpf get '%s' record list ok, row:%d, length:%d, total:%d.",
		ch, row, length, qb.entries
	);
	return qb.entries;
}


static gint
nmp_jfs_file_probe(const gchar *mrl)
{
	return 0;
}


static STOR_EACH_SINDEX_CBRET_E
nmp_jfs_seek_set(struct _stor_each_sindex_s *handle, stor_frame_sindex_t *psindx)
{
	nmp_print("in seek callback!");
	return STOR_EACH_SINDEX_CBRET_SEEKTO;
}


static gint
nmp_jfs_seek_to_begin(stor_fread_handle_t *h)
{
	stor_each_sindex_t si;
	gint ret;

	si.dir = STOR_EACH_SINDEX_DIR_RESTART;
	si.user_arg = NULL;
	si.callback = nmp_jfs_seek_set;

	ret = stor_file_lseek(h, &si);
	return ret;
}


static __inline__ gchar *
nmp_jfs_get_codes(stor_fragment_t *psf, gsize *psize, guint32 *ts)
{
	gchar buf[MAX_BUF_LEN] = {0};
	stor_fread_handle_t *h;
	gint ret, nread, rtp_len;
	guint total_len, ext_len;
	gchar *codec = NULL;

	h = stor_file_openR(psf, &ret);
	if (!h)
	{
		nmp_warning(
			"vod: open file failed, err:%d.", ret
		);
		return NULL;
	}

	for (;;)
	{
		nread = sizeof(NmpSdlV1);
		ret = stor_file_read(h, buf, nread);
		if (ret != nread)
		{
			nmp_warning(
				"vod: stor_file_read() failed."
			);
			goto FAILED_EXIT;
		}

		if (SDL_GET_VALUE(buf, magic) != SDL_MAGIC_NUM)
		{
			nmp_warning(
				"vod: magic(0x%X) check failed.",
				SDL_GET_VALUE(buf, magic)
			);
			goto FAILED_EXIT;			
		}

		ext_len = SDL_V1_GET_VALUE(buf, ext_len);
		if (ext_len >= MAX_BUF_LEN)
		{
			nmp_warning(
				"vod: ext-len(sps?) '%d' is invald.", ext_len
			);
			goto FAILED_EXIT;
		}

		if (!*ts)
			*ts = SDL_V1_GET_VALUE(buf, ts_2);

		if (ext_len != 0)
			break;

		total_len = SDL_GET_VALUE(buf, len);
		if (total_len < ext_len + sizeof(NmpSdlV1))
		{
			nmp_warning(
				"vod: sdl-len '%d' is invald, ext_len(%d).",
				total_len, ext_len
			);
			goto FAILED_EXIT;
		}

		rtp_len = total_len - ext_len - sizeof(NmpSdlV1);
		if (rtp_len > MAX_BUF_LEN)
		{
			nmp_warning(
				"vod: rtp_len ('%d') > MAX_BUF_LEN ('%d').",
				rtp_len, MAX_BUF_LEN
			);
			goto FAILED_EXIT;			
		}

		ret = stor_file_read(h, buf, rtp_len);
		if (ret != rtp_len)
		{
			nmp_warning(
				"vod: stor_file_read() failed."
			);
			goto FAILED_EXIT;
		}
	}

	ret = stor_file_read(h, buf, ext_len);
	if (ret != ext_len)
	{
		nmp_warning(
			"vod: stor_file_read() failed."
		);
		goto FAILED_EXIT;			
	}

	codec = g_memdup(buf, ext_len);
	*psize = ext_len;

FAILED_EXIT:
	stor_file_closeR(h);
	return codec;
}


static __inline__ gdouble
nmp_jfs_timestamp(guint32 ts)
{
	return (gdouble)ts;
}


static __inline__ void
nmp_jfs_set_context(struct file_ctx *ctx, gchar *codec, gsize codec_size,
	guint32 ts_base)
{
	NmpSdlExtV1 *codec_format = (NmpSdlExtV1*)codec;
	struct file_stream *stm;

	ctx->file_type = FILE_STM_PUB;
	ctx->streams = (gint)(codec_format->len1 || codec_format->len2) +
		(gint)(!!codec_format->len3);
	ctx->length = -1;
	ctx->ts_base = nmp_jfs_timestamp(ts_base);

	if (codec_format->len1 || codec_format->len2)
	{//@{H.264 rtp}
		stm = &ctx->stms[0];
		stm->stm_type = STM_VIDEO;
		stm->codec = codec;
		stm->codec_size = codec_size;

		if (codec_format->len3)
		{
			stm = &ctx->stms[1];
			stm->stm_type = STM_AUDIO;
			stm->codec = g_memdup(codec, codec_size);
			stm->codec_size = codec_size;
		}
	}
	else
	{//@{private rtp, only audio isn't permitted}
		stm = &ctx->stms[0];
		stm->stm_type = STM_VIDEO;
		stm->codec = codec;
		stm->codec_size = codec_size;
		ctx->file_type = FILE_STM_PRI;
	}
}


static gint
nmp_jfs_file_open(const gchar *mrl, struct file_ctx *ctx)
{
	stor_fragment_t sf;
	gchar *property;
	gint ret, group, disk, part;
	stor_fread_handle_t *h;
	gchar *codec;
	gsize codec_size;
	guint32 ts_base = 0;

	property = nmp_regex_string(mrl, "property=.*");
	if (!property)
	{
		nmp_print(
			"vod: file:%s not found, mrl property error.", mrl
		);
		return -EINVAL;
	}

	memset(&sf, 0, sizeof(sf));
	ret = sscanf(property + 9, PROPERTY_FORMAT,
		&group,
		&disk,
		&part,
		&sf.file_no,
		&sf.data_start,
		&sf.data_end,
		&sf.index_start,
		&sf.index_end
		);

	sf.group_id = group;
	sf.disk_no = disk;
	sf.part_no = part;

	g_free(property);

	if (ret != 8)
	{
		nmp_print(
			"vod: file:%s property format error '%s'.", mrl, property
		);
		return -EINVAL;
	}

	nmp_print(
		"vod: open file, property group:%d disk:%d part:%d file:%d "
		"dat_start:%d dat_end:%d idx_start:%d idx_end:%d",
		sf.group_id, sf.disk_no, sf.part_no, sf.file_no, sf.data_start,
		sf.data_end, sf.index_start, sf.index_end
	);

	codec = nmp_jfs_get_codes(&sf, &codec_size, &ts_base);
	if (!codec)
	{
		nmp_warning(
			"vod: can't get '%s' sps/pli/pri data.", mrl
		);
		return -EINVAL;
	}

	h = stor_file_openR(&sf, &ret);
	if (!h)
	{
		nmp_warning(
			"vod: open file:%s failed, err:%d.", mrl, ret
		);
		g_free(codec);
		return -EINVAL;
	}

	nmp_jfs_set_context(ctx, codec, codec_size, ts_base);
	ctx->priv_data = h;

	nmp_print(
		"vod: file '%s' open ok, ctx '%p'.", mrl, ctx
	);

	return 0;
}


static gint
nmp_jfs_file_read(struct file_ctx *ctx, struct file_packet *pkt)
{
	stor_fread_handle_t *h;
	gsize len, ext_len, rtp_len, ret;
	gchar buf[MAX_BUF_LEN] = {0};
	guint32 ts, tags;

	memset(pkt, 0, sizeof(*pkt));

	h = (stor_fread_handle_t*)ctx->priv_data;
	BUG_ON(!h);

	len =  sizeof(NmpSdlV1);
	ret = stor_file_read(h, buf, len);
	if (ret != len)
	{
		nmp_warning(
			"vod: stor_file_read() failed, len: %d, ret:%d",
			len, ret
		);
		return -EIO;
	}

	if (SDL_GET_VALUE(buf, magic) != SDL_MAGIC_NUM)
	{
		nmp_warning(
			"vod:  stor_file_read() magic(0x%X) check failed.",
			SDL_GET_VALUE(buf, magic)
		);
		return -EIO;				
	}

	len = SDL_GET_VALUE(buf, len);
	ext_len = SDL_V1_GET_VALUE(buf, ext_len);
	ts = SDL_V1_GET_VALUE(buf, ts_2);
	tags = SDL_V1_GET_VALUE(buf, tags);

	if (ext_len > 0)
	{
		ret = stor_file_read(h, buf, ext_len);
		if (ret != ext_len)
		{
			nmp_warning(
				"vod: stor_file_read() failed, len: %d, ret:%d",
				len, ret
			);
			return -EIO;
		}
	}

	rtp_len = len - sizeof(NmpSdlV1) - ext_len;

	if (rtp_len <= 0 || rtp_len > MAX_BUF_LEN)
	{
		nmp_warning(
			"vod: invalid rtp_len '%d'.", rtp_len
		);
		return -EIO;
	}

	ret = stor_file_read(h, buf, rtp_len);

	if (ret != rtp_len)
	{
		nmp_warning(
			"vod: stor_file_read() failed, len: %d, ret:%d.",
			rtp_len, ret
		);
		return -EIO;
	}

	pkt->stm_index = ctx->streams < 2 ? 0 : ((tags & SDL_V1_TAGS_AUDIO) ? 1 : 0);
	pkt->pts = nmp_jfs_timestamp(ts);
	pkt->data = g_memdup(buf, rtp_len);
	pkt->size = rtp_len;

	return 0;
}


static void
nmp_jfs_free_packet(struct file_packet *pkt)
{
	g_free(pkt->data);
}


static void
nmp_jfs_file_close(struct file_ctx *ctx)
{
	stor_fread_handle_t *h;
	gint i_stm = 0;

	h = (stor_fread_handle_t*)ctx->priv_data;
	stor_file_closeR(h);

	for (; i_stm < ctx->streams; ++i_stm)
	{
		g_free(ctx->stms[i_stm].codec);
	}
	nmp_print("vod: file closed, ctx '%p'.", ctx);
}


NmpRecordsOps nmp_advanced_records =
{
	.get_list = nmp_jfs_records_get_list
};


struct file_ops jfs_file_ops =
{
	.probe			= nmp_jfs_file_probe,
	.open			= nmp_jfs_file_open,
	.read			= nmp_jfs_file_read,
	.free_packet	= nmp_jfs_free_packet,
	.close			= nmp_jfs_file_close
};


//:~ End
