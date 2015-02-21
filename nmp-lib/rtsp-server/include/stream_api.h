#ifndef __STREAM_API_H__
#define __STREAM_API_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>

typedef gint (*f_stream_init)(gchar *device, gint type, gint channel, gint level, gchar *pri, gchar **media_info, gsize *gsize, gdouble *length);
typedef gint (*f_stream_open)(gpointer stream, gchar *device, gint type, gint channel, gint level, gchar *pri);
typedef void (*f_stream_play)(gpointer stream);
typedef void (*f_stream_pause)(gpointer stream);
typedef void (*f_stream_close)(gpointer stream);
typedef gint (*f_stream_seek)(gpointer stream, gdouble ts);
typedef gint (*f_stream_ctrl)(gchar *device, gint channel, gint level, gint cmd, void *value);

typedef struct _Stream_operation Stream_operation;
struct _Stream_operation
{
	f_stream_init init;
	f_stream_open open;
	f_stream_play play;
	f_stream_pause pause;
	f_stream_close close;
	f_stream_seek seek;
	f_stream_ctrl ctrl;
};

void register_stream_operations(Stream_operation *ops);

void set_stream_user_data(gpointer stream, void *u);
void *get_stream_user_data(gpointer stream);
gint test_stream_blockable(gpointer stream, gsize size);	/* for downloading */
gpointer stream_handle_ref(gpointer stream);
void stream_handle_unref(gpointer stream);

gint write_stream_data(gpointer stream,
                       gulong timestamp,
                       gulong duration,
                       guint8 *data, gsize data_size, guint8 data_type,
                       guint8 *ext, gsize ext_size, guint8 ext_type);


enum
{
	FILE_STM_PUB,
	FILE_STM_PRI
};

enum
{
	STM_VIDEO,
	STM_AUDIO
};

struct file_packet
{
	gint stm_index;
	gdouble pts;

	gchar *data;
	gsize size;
};

struct file_stream
{
	gint	stm_type;	/* STM_VIDEO/STM_AUDIO */
	gchar	*codec;
	gchar	codec_size;
};

struct file_ctx
{
	gint		file_type;		/* public/private stream */
	gint		streams;		/* count of streams */
	guint		flags;
	gdouble 	ts_base;
	gdouble		length;
	struct file_stream stms[2];
	gchar		*priv_data;
};

struct file_ops
{
	gint (*probe)(const gchar *mrl);
	gint (*open)(const gchar *mrl, struct file_ctx *ctx);
	gint (*read)(struct file_ctx *ctx, struct file_packet *pkt);
	void (*free_packet)(struct file_packet *pkt);
	void (*close)(struct file_ctx *ctx);
};


gint register_file_ops(struct file_ops *ops);



#ifdef __cplusplus
	}
#endif

#endif	/* __STREAM_API_H__ */
