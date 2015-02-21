/* *
 * This file is part of rtsp-server
 *
 * Copyright (C) 2012 fangyi <fangyi@szjxj.net>
 * See COPYING for more details
 *
 * rtsp-server is a stream transporting and controlling module using
 * RTP/RTCP/RTSP protocols, which is designed for jxj platform servers
 * such as mss, proxy-server etc.
 *
 * */

#ifndef __MEDIA_H__
#define __MEDIA_H__

#include <glib.h>
#include <stdint.h>

#include "bufferqueue.h"

#define RESOURCE_OK					0
#define RESOURCE_NOT_FOUND			-1
#define RESOURCE_DAMAGED			-2
#define RESOURCE_TRACK_NOT_FOUND	-4
#define RESOURCE_NOT_PARSEABLE		-5
#define RESOURCE_EOF				-6

#define MAX_TRACKS					20
#define MAX_SEL_TRACKS				5

typedef enum
{
	MP_undef = -1,
	MP_audio,
	MP_video,
	MP_application,
	MP_data,
	MP_control
}MediaType;

typedef enum
{
	MS_stored=0,
	MS_live
}MediaSource;

typedef enum
{
	OM_DESC,
	OM_SETUP
}OpenMode;

//! typedefs that give convenient names to GLists used
typedef GList *TrackList;
typedef GList *SelList;

struct MediaParser;

typedef enum {empty_field, fmtp, rtpmap, pri} sdp_field_type;

typedef struct _sdp_field sdp_field;
struct _sdp_field
{
	sdp_field_type type;
	gchar *field;
};

typedef struct ResourceInfo_s {
    gchar *mrl;
    gchar *name;
    gchar *description;
    gchar *descrURI;
    gchar *email;
    gchar *phone;
    time_t mtime;
    gdouble duration;
    MediaSource media_source;
    gchar twin[256];
    gchar multicast[16];
    gint port;
    gchar ttl[4];

    /**
     * @brief Seekable resource flag
     *
     * Right now this is only false when the resource is a live
     * resource (@ref ResourceInfo::media_source set to @ref MS_live)
     * or when the demuxer provides no @ref Demuxer::seek function.
     *
     * In the future this can be extended to be set to false if the
     * user disable seeking in a particular stored (and otherwise
     * seekable) resource.
     */
    gboolean seekable;
} ResourceInfo;

typedef struct Resource {
    GMutex *lock;
    const struct _Demuxer *demuxer;
    ResourceInfo *info;
    // Metadata begin
#ifdef HAVE_METADATA
    void *metadata;
#endif
    // Metadata end

    /* Timescale fixer callback function for meta-demuxers */
    gdouble (*timescaler)(struct Resource *, gdouble);
    /* EDL specific data */
    struct Resource *edl;
    /* Multiformat related things */
    TrackList tracks;
    BufferQueue_Producer *vp;	/*打破现有机制，使用视频带动音频，不然无法同步送流 */
	void *audio_rtp;	/* 打破现有机制，使用视频带动音频，不然无法同步送流 */

	OpenMode mode;	/* what purpose does the resource open for? describe? setup? */
    gint num_tracks;
    gboolean eor;
    void *private_data; /* Demuxer private data */
} Resource;

typedef struct Trackinfo_s {
    gchar *mrl;
    gchar name[256];
    gint id; // should it more generic?
    gint rtp_port;
    //start CC
    gchar commons_deed[256];
    gchar rdf_page[256];
    gchar title[256];
    gchar author[256];
    //end CC
} TrackInfo;

typedef struct MediaProperties {
    gint bit_rate; /*!< average if VBR or -1 is not useful*/
    gint payload_type;
    guint clock_rate;
    gchar encoding_name[16];
    MediaType media_type;
    MediaSource media_source;
    gint codec_id; /*!< Codec ID as defined by ffmpeg */
    gint codec_sub_id; /*!< Subcodec ID as defined by ffmpeg */
    gdouble pts;             //time is in seconds
    gdouble dts;             //time is in seconds
    gdouble frame_duration;  //time is in seconds
    gfloat sample_rate;/*!< SamplingFrequency*/
    gfloat OutputSamplingFrequency;
    gint audio_channels;
    gint bit_per_sample;/*!< BitDepth*/
    gfloat frame_rate;
    gint FlagInterlaced;
    guint PixelWidth;
    guint PixelHeight;
    guint DisplayWidth;
    guint DisplayHeight;
    guint DisplayUnit;
    guint AspectRatio;
    uint8_t *ColorSpace;
    gfloat GammaValue;
    uint8_t *extradata;
    size_t extradata_len;
} MediaProperties;

typedef struct Track {
    GMutex *lock;
    TrackInfo *info;
    gdouble start_time;
    struct MediaParser *parser;

    BufferQueue_Producer *producer;
    Resource *parent;

    void *private_data; /* private data of media parser */

    GSList *sdp_fields;

    MediaProperties properties;

} Track;

// --- functions --- //

Resource *r_open(const gchar *inner_path, OpenMode mode);
gint r_read(Resource *resource);
gint r_seek(Resource *resource, gdouble time);
void r_close(Resource *resource);
gint r_ctrl(const char *inner_path, gint cmd, void *value);
gint r_play(Resource *r);
gint r_pause(Resource *r);

Track *r_find_track(Resource *, const gchar *);

// Tracks
Track *add_track(Resource *, TrackInfo *, MediaProperties *);
void free_track(gpointer element, gpointer user_data);
void track_add_sdp_field(Track *track, sdp_field_type type, gchar *value);

#endif	/* __MEDIA_H__ */
