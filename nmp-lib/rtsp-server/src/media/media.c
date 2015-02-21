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

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <glib.h>

#include "demuxer.h"
#include "mediaparser.h"
#include "rtp.h"
#include "rc_log.h"

#define RESOURCE_ROOT	"/"

/**
 * @brief Free a resource object
 *
 * @param resource The resource to free
 * @param user_data Unused
 *
 * @internal Please note that this function will execute the freeing
 *           in the currently running thread, synchronously. It should
 *           thus only be called by the functions that wouldn't block
 *           the mainloop.
 *
 * @note Nobody may hold the @ref Resource::lock mutex when calling
 *       this function.
 *
 * @note This function uses two parameters because it's used as
 *       interface for the closing thread pool.
 */
static void r_free_cb(gpointer resource_p, gpointer user_data)
{
    Resource *resource = (Resource *)resource_p;
    if (!resource)
        return;

    if (resource->lock)
        g_mutex_free(resource->lock);

    g_free(resource->info->mrl);
    g_free(resource->info->name);
    g_slice_free(ResourceInfo, resource->info);

    if (resource->tracks) {
        g_list_foreach(resource->tracks, free_track, resource);
        g_list_free(resource->tracks);
    }

    resource->info = NULL;
    resource->demuxer->uninit(resource);

    g_slice_free(Resource, resource);
}


static __inline__ gint
get_path_ch_and_level(const gchar *path, gint *chp, gint *lp)
{
	gint ch = -1, level = -1;
	gchar *v;

	v = get_param_string(path, "channel=\\d+");
	if (v)
	{
		sscanf(v, "channel=%d", &ch);
		g_free(v);
	}

	v = get_param_string(path, "level=\\d+");
	if (v)
	{
		sscanf(v, "level=%d", &level);
		g_free(v);
	}

	if (ch >= 0 && level >= 0)
	{
		*chp = ch;
		*lp = level;
		return 0;
	}

	return -1;
}


static __inline__ gchar *
get_media_path_live(const gchar *path, gint *ls)
{
	gchar *v,*new_path;
	gint ch, level;

	if (get_path_ch_and_level(path, &ch, &level))
		return g_strdup(path);

	v = get_param_string(path, "dev=[A-Z]{3}-[A-Z]{3}-\\d{8}");
	if (v)
	{
		new_path = g_strdup_printf("/%s/media=0/channel=%d&level=%d/seq=0", v, ch, level);
		g_free(v);
		*ls = 1;
		return new_path;
	}

	return g_strdup(path);
}


static __inline__ gchar *
get_media_path_stored(const gchar *path, gint type, gint *ls)
{
	static gint seq = 0;
	gchar *v, *v2, *new_path;
	gint ch, level;

	if (get_path_ch_and_level(path, &ch, &level))
		return g_strdup(path);

	v = get_param_string(path, "dev=[A-Z]{3}-[A-Z]{3}-\\d{8}");	
	if (!v)
	{/* dev=MSS-ID, returned */
		return g_strdup(path);
	}

	v2 = get_param_string(path, "recordType=\\d+&startTime=\\d{14}+&endTime=\\d{14}+");
	if (!v2)
	{
		g_free(v);
		return g_strdup(path);
	}

	new_path = g_strdup_printf("/%s/media=%d/channel=%d&level=%d/seq=%d/%s",
		v, type, ch, level, ++seq, v2);
	g_free(v);
	g_free(v2);
	*ls = 1;
	return new_path;
}



static __inline__ gchar *
get_media_path(const gchar *path, gint *ls, gint *ls_store)
{
	gint media_type;
	gchar *v;

	*ls = 0;

	if (strchr(path, '.'))	/* .mp3 .mp4 ?*/
		return g_strdup(path);

	v = get_param_string(path, "media=\\d+");
	if (v)
	{
		media_type = atoi(v + 6);
		g_free(v);

		switch (media_type)
		{
		case 0:		/* media=0 */
			*ls_store = 0;
			return get_media_path_live(path, ls);

		default:
			*ls_store = 1;
			return get_media_path_stored(path, media_type, ls);		
		}
	}

	return g_strdup(path);
}


/**
 * @brief Open a new resource and create a new instance
 *
 * @param srv The server object for the vhost requesting the resource
 * @param inner_path The path, relative to the avroot, for the
 *                   resource
 *
 * @return A new Resource object
 * @retval NULL Error while opening resource
 */
Resource *r_open(const char *inner_path, OpenMode mode)
{
    Resource *r;
    const Demuxer *dmx;
    gchar *mrl, *path;
	gint ls, ls_store;		/* ls: use demuxer_ls; ls_store: demuxer_ls with history streams */

	path = get_media_path(inner_path, &ls, &ls_store);
	mrl = g_strdup_printf("%s%s", RESOURCE_ROOT, path);
	g_free(path);

    if ((dmx = r_find_demuxer(mrl)) == NULL)
    {
        rc_log(RC_LOG_DEBUG,
                "[MT] Could not find a valid demuxer for resource %s\n",
                mrl);
        goto error;
    }

    rc_log(RC_LOG_DEBUG, "[MT] registrered demuxer \"%s\" for resource"
                               "\"%s\"\n", dmx->info->name, mrl);

    r = g_slice_new0(Resource);
    r->info = g_slice_new0(ResourceInfo);
    r->info->mrl = mrl;
    r->info->mtime = 0;
    r->info->name = g_path_get_basename(mrl);
    r->info->seekable = (dmx->seek != NULL);

    if (ls && !ls_store)
		r->info->seekable = FALSE;

    r->demuxer = dmx;
	r->mode = mode;

    if (r->demuxer->init(r))
    {
        r_free_cb(r, NULL);
        return NULL;
    }

    /* Now that we have opened the actual resource we can proceed with
     * the extras */

    r->lock = g_mutex_new();

#ifdef HAVE_METADATA
    cpd_find_request(srv, r, filename);
#endif

    return r;
 error:
    g_free(mrl);
    return NULL;
}



gint r_ctrl(const char *inner_path, gint cmd, void *value)
{
    const Demuxer *dmx;
    gchar *mrl, *path;
	gint ls, ls_store, ret = -1;

	path = get_media_path(inner_path, &ls, &ls_store);
	mrl = g_strdup_printf("%s%s", RESOURCE_ROOT, path);
	g_free(path);

	if (ls)
	{
	    if ((dmx = r_find_demuxer(mrl)) == NULL)
	    {
	        rc_log(RC_LOG_DEBUG,
	                "[MT] Could not find a valid demuxer for '%s'\n",
	                mrl);
	        g_free(mrl);
	        return -1;
	    }

		if (dmx->ctrl_track)
			ret = (*dmx->ctrl_track)(mrl, cmd, value);
		g_free(mrl);
		return ret;
	}

	g_free(mrl);
	return -1;
}


static void play_track(gpointer element, gpointer user_data)
{
    Track *track = (Track*)element;
    Resource *r = (Resource*)user_data;
	const Demuxer *dmx;

	dmx = r->demuxer;
	if (dmx && dmx->play_track)
	{
		if (track->producer)
		{//Resource for desc may not have a producer.
			(*dmx->play_track)(track->producer, track->info->mrl);
		}
	}
}


static void pause_track(gpointer element, gpointer user_data)
{
    Track *track = (Track*)element;
    Resource *r = (Resource*)user_data;
	const Demuxer *dmx;

	dmx = r->demuxer;
	if (dmx && dmx->pause_track)
	{
		if (track->producer)
		{//Resource for desc may not have a producer.
			(*dmx->pause_track)(track->producer, track->info->mrl);
		}
	}
}


gint r_play(Resource *r)
{
	if (r)
	{
		g_list_foreach(r->tracks, play_track, r);
	}
	return 0;
}


gint r_pause(Resource *r)
{
	if (r)
	{
		g_list_foreach(r->tracks, pause_track, r);
	}
	return 0;
}


/**
 * @brief Comparison function to compare a Track to a name
 *
 * @param a Track pointer to the element in the list
 * @param b String with the name to compare to
 *
 * @return The strcmp() result between the correspondent Track name
 *         and the given name.
 *
 * @internal This function should _only_ be used by @ref r_find_track.
 */
static gint r_find_track_cmp_name(gconstpointer a, gconstpointer b)
{
    return strcmp( ((Track *)a)->info->name, (const char *)b);
}

/**
 * @brief Find the given track name in the resource
 *
 * @param resource The Resource object to look into
 * @param track_name The name of the track to look for
 *
 * @return A pointer to the Track object for the requested track.
 *
 * @retval NULL No track in the resource corresponded to the given
 *              parameters.
 *
 * @todo This only returns the first resource corresponding to the
 *       parameters.
 * @todo Capabilities aren't used yet.
 */
Track *r_find_track(Resource *resource, const char *track_name) {
    GList *track = g_list_find_custom(resource->tracks,
                                      track_name,
                                      r_find_track_cmp_name);

    if ( !track ) {
        rc_log(RC_LOG_DEBUG, "Track %s not present in resource %s\n",
                track_name, resource->info->mrl);
        return NULL;
    }

    return track->data;
}

/**
 * @brief Resets the BufferQueue producer queue for a given track
 *
 * @param element The Track element from the list
 * @param user_data Unused, for compatibility with g_list_foreach().
 *
 * @see bq_producer_reset_queue
 */
static void r_track_producer_reset_queue(gpointer element,  gpointer user_data)
{
    Track *t = (Track*)element;

	if (t->producer)
	{
    	bq_producer_reset_queue(t->producer);
    }
}

/**
 * @brief Seek a resource to a given time in stream
 *
 * @param resource The Resource to seek
 * @param time The time in seconds within the stream to seek to
 *
 * @return The value returned by @ref Demuxer::seek
 *
 * @note This function will lock the @ref Resource::lock mutex.
 */
int r_seek(Resource *resource, double time) {
    int res;

    g_mutex_lock(resource->lock);

    res = resource->demuxer->seek(resource, time);

    g_list_foreach(resource->tracks, r_track_producer_reset_queue, NULL);

    g_mutex_unlock(resource->lock);

    return res;
}

/**
 * @brief Read data from a resource (unlocked)
 *
 * @param resouce The resource to read from
 *
 * @return The same return value as read_packet
 */
int r_read(Resource *resource)
{
    int ret = RESOURCE_EOF;

    g_mutex_lock(resource->lock);
    if (!resource->eor)
        switch( (ret = resource->demuxer->read_packet(resource)) ) {
        case RESOURCE_OK:
            break;
        case RESOURCE_EOF:
            rc_log(RC_LOG_INFO,
                    "r_read_unlocked: %s read_packet() end of file.",
                    resource->info->mrl);
            resource->eor = TRUE;
            break;
        default:
            rc_log(RC_LOG_FATAL,
                    "r_read_unlocked: %s read_packet() error.",
                    resource->info->mrl);
            break;
        }

    g_mutex_unlock(resource->lock);

    return ret;
}

/**
 * @brief Request closing of a resource
 *
 * @param resource The resource to close
 *
 * This function only pushes a close request on the resource closing
 * pool; the actual closing will happen asynchronously.
 *
 * @see r_free_cb
 */
void r_close(Resource *resource)
{
    /* This will never be freed but we don't care since we're going to
     * need it till the very end of the process.
     */
    static GThreadPool *closing_pool;

    static gsize created_pool = FALSE;
    if ( g_once_init_enter(&created_pool) ) {
        closing_pool = g_thread_pool_new(r_free_cb, NULL,
                                         -1, FALSE, NULL);

        g_once_init_leave(&created_pool, TRUE);
    }

    g_thread_pool_push(closing_pool, resource, NULL);
}


void r_start_track(Resource *r, BufferQueue_Producer *b, gchar *uri)
{//ls->open()
	g_assert(r != NULL);

	if (r->demuxer->start_track)
		r->demuxer->start_track(b, uri);
}


void r_stop_track(Resource *r, BufferQueue_Producer *b, gchar *uri)
{//ls->close()
	g_assert(r != NULL);

	if (r->mode == OM_SETUP && r->demuxer->stop_track)
		r->demuxer->stop_track(b, uri);
}


static void free_sdp_field(sdp_field *sdp, void *unused)
{
    if (!sdp)
        return;

    g_free(sdp->field);
    g_slice_free(sdp_field, sdp);
}

static void sdp_fields_free(GSList *fields)
{
    if ( fields == NULL )
        return;

    g_slist_foreach(fields, (GFunc)free_sdp_field, NULL);
    g_slist_free(fields);
}

/**
 * @brief Frees the resources of a Track object
 *
 * @param element Track to free
 * @param user_data Unused, for compatibility with g_list_foreach().
 */
void free_track(gpointer element, gpointer user_data)
{
    Track *track = (Track*)element;
    Resource *r = (Resource*)user_data;

    if (!track)
        return;

    g_mutex_free(track->lock);

	if (track->producer)
	{
		if (bq_producer_test_ref_and_remove(track->producer))	/* only us */
		{
			r_stop_track(r, track->producer, track->info->mrl);
		}
	    bq_producer_unref(track->producer);	/* reach 0, must be here, we have to assume all consumers freed!*/
	}

    g_free(track->info->mrl);
    g_slice_free(TrackInfo, track->info);

    sdp_fields_free(track->sdp_fields);

    if ( track->parser && track->parser->uninit )
        track->parser->uninit(track);

    g_slice_free(Track, track);
}


static void
free_payload(void *payload)
{
	if (payload != LS_EOF)
		g_free(payload);
}


/*! Add track to resource tree.  This function adds a new track data struct to
 * resource tree. It used by specific demuxer function in order to obtain the
 * struct to fill.
 * @param r pointer to resource.
 * @return pointer to newly allocated track struct.
 * */

Track *add_track(Resource *r, TrackInfo *info, MediaProperties *prop_hints)
{
    Track *t;
    gint new_producer;
    // TODO: search first of all in exclusive tracks

    if(r->num_tracks>=MAX_TRACKS)
        return NULL;

    t = g_slice_new0(Track);
    t->lock = g_mutex_new();
    t->parent = r;

    t->info = g_slice_new0(TrackInfo);
    memcpy(t->info, info, sizeof(TrackInfo));
    memcpy(&t->properties, prop_hints, sizeof(MediaProperties));

    switch (t->properties.media_source)
    {
    case MS_stored:	/* demuxer_jm */
		if (r->mode == OM_SETUP)
		{
			if (prop_hints->media_type == MP_video)
			{
		        if( !(t->producer = bq_producer_new(free_payload, NULL, &new_producer)) )
		        {
		            rc_log(RC_LOG_FATAL, "add_track()->bq_producer_new() failed!");
		            goto error;
		        }
				if (new_producer)
				{
					r_start_track(r, t->producer, t->info->mrl);
				}

				r->vp = t->producer;
			}
			else
			{
				if (r->vp)
				{
					t->producer = bq_producer_ref(r->vp);
				}
			}
	    }

        if ( !(t->parser = mparser_find(t->properties.encoding_name)) )
        {
            rc_log(RC_LOG_FATAL, "add_track()->mparser_find() failed!");
            goto error;
        }
        if (t->parser->init(t))
        {
            rc_log(RC_LOG_FATAL, "add_track()->parser->init(), for encoding:%s failed!",
            	t->properties.encoding_name);
            goto error;
        }
        t->properties.media_type = t->parser->info->media_type;
        break;

    case MS_live:	/* demuxer_ls */
		if (r->mode == OM_SETUP)
		{
	        if( !(t->producer = bq_producer_new(free_payload, t->info->mrl, &new_producer)) )
	        {
	            rc_log(RC_LOG_FATAL, "add_track()->bq_producer_new() failed!");
	            goto error;
	        }
			if (new_producer)
			{
				r_start_track(r, t->producer, t->info->mrl);
			}
		}
        break;

    default:
        rc_log(RC_LOG_FATAL, "add_track():Media source not supported!");
        break;
    }


    r->tracks = g_list_append(r->tracks, t);
    r->num_tracks++;

    return t;

 error:
    free_track(t, r);
    return NULL;
}


/**
 * @brief Append an SDP field to the track
 *
 * @param track The track to add the fields to
 * @param type The type of the field to add
 * @param value The value of the field to add (already duped)
 */
void track_add_sdp_field(Track *track, sdp_field_type type, char *value)
{
    sdp_field *field = g_slice_new(sdp_field);
    field->type = type;
    field->field = value;

    track->sdp_fields = g_slist_prepend(track->sdp_fields, field);
}

//:~ End
