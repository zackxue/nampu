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

#include <string.h>

#include "mediaparser.h"
#include "demuxer.h"
#include "bufferqueue.h"
#include "rc_log.h"

// global media parsers modules:

//extern MediaParser fnc_mediaparser_h264;
//extern MediaParser fnc_mediaparser_aac;
//extern MediaParser fnc_mediaparser_mpa;
extern MediaParser fnc_mediaparser_jma;
extern MediaParser fnc_mediaparser_jmv;

/**
 * the following are not supported yet!
 *
 *	extern MediaParser fnc_mediaparser_mpv;
 *	extern MediaParser fnc_mediaparser_vorbis;
 *  extern MediaParser fnc_mediaparser_mp4ves;
 *	extern MediaParser fnc_mediaparser_theora;
 *	extern MediaParser fnc_mediaparser_speex;
 *	extern MediaParser fnc_mediaparser_mp2t;
 *	extern MediaParser fnc_mediaparser_h263;
 *	extern MediaParser fnc_mediaparser_amr;
*/

// static array containing all the available media parsers:
static MediaParser *media_parsers[] = {
    &fnc_mediaparser_jmv,
    &fnc_mediaparser_jma,
    NULL
};

MediaParser *mparser_find(const char *encoding_name)
{
    int i;

    for(i=0; media_parsers[i]; i++) {
        if ( !g_ascii_strcasecmp(encoding_name,
                                 media_parsers[i]->info->encoding_name) ) {
            rc_log(RC_LOG_DEBUG, "[MT] Found Media Parser for %s\n",
                    encoding_name);
            return media_parsers[i];
        }
    }

    rc_log(RC_LOG_DEBUG, "[MT] Media Parser for %s not found\n",
            encoding_name);
    return NULL;
}

/**
 *  Insert a rtp packet inside the track buffer queue
 *  @param tr track the packetized frames/samples belongs to
 *  @param presentation the actual packet presentation timestamp
 *         in fractional seconds, will be embedded in the rtp packet
 *  @param delivery the actual packet delivery timestamp
 *         in fractional seconds, will be used to calculate sending time
 *  @param duration the actual packet duration, a multiple of the frame/samples
 *  duration
 *  @param marker tell if we are handling a frame/sample fragment
 *  @param data actual packet data
 *  @param data_size actual packet data size
 */
void mparser_buffer_write(Track *tr,
                          double presentation,
                          double delivery,
                          double duration,
                          gboolean marker,
                          gint mtype,
                          guint8 *data, gsize data_size)
{

	if (tr->producer)
	{
	    MParserBuffer *buffer = g_malloc(sizeof(MParserBuffer) + data_size);
	
	    buffer->timestamp = presentation;
	    buffer->delivery = delivery;
	    buffer->duration = duration;
	    buffer->marker = marker;
	    buffer->mtype = mtype;
	    buffer->data_size = data_size;
	    memcpy(buffer->data, data, data_size);
	    bq_producer_put(tr->producer, buffer);
	}
}


/**
 *  Insert a rtp packet inside the track buffer queue
 *  @param tr track the packetized frames/samples belongs to
 *  @param presentation the actual packet presentation timestamp
 *         in fractional seconds, will be embedded in the rtp packet
 *  @param delivery the actual packet delivery timestamp
 *         in fractional seconds, will be used to calculate sending time
 *  @param duration the actual packet duration, a multiple of the frame/samples
 *  duration
 *  @param marker tell if we are handling a frame/sample fragment
 *  @param data actual packet data
 *  @param data_size actual packet data size
 */
MParserBuffer *alloc_mparser_buffer(double timestamp,
                                    double delivery,
                                    double duration,
                                    gboolean marker,
                                    gint mtype,
                                    guint8 *data, gsize data_size)
{
    MParserBuffer *buffer = g_malloc(sizeof(MParserBuffer) + data_size);
    buffer->timestamp = timestamp;
    buffer->delivery = delivery;
    buffer->duration = duration;
    buffer->marker = marker;
    buffer->mtype = mtype;
    buffer->data_size = data_size;
    memcpy(buffer->data, data, data_size);
    return buffer;
}

void free_mparser_buffer(MParserBuffer *buffer)
{
	if (!buffer)
		return;

	g_free(buffer);
}

gint mparser_buffer_write_buffer(BufferQueue_Producer *producer,
                                 MParserBuffer *buffer)
{
	bq_producer_put((BufferQueue_Producer*)producer, buffer);
	return 0;
}

gint mparser_buffer_write_eof(BufferQueue_Producer *producer)
{
	bq_producer_put_eof(producer);
	return 0;
}

//:~ End
