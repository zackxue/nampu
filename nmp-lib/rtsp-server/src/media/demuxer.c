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
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <glib.h>

#include "demuxer.h"
#include "rc_log.h"


// global demuxer modules:
extern Demuxer fnc_demuxer_ls;
extern Demuxer fnc_demuxer_jmf;



/**
 * @brief Find the correct demuxer for the given resource.
 *
 * @param filename Name of the file (to find the extension)
 *
 * @return A constant pointer to the working demuxer.
 *
 * This function first tries to match the resource extension with one
 * of those served by the demuxers, that will be probed; if this fails
 * it tries every demuxer available with direct probe.
 *
 * */

const Demuxer *r_find_demuxer(const char *filename)
{
    static const Demuxer *const demuxers[] = {
    	&fnc_demuxer_ls,
		&fnc_demuxer_jmf,
        NULL
    };

    int i;
    const char *res_ext;

    /* First of all try that with matching extension: we use extension as a
     * hint of resource type.
     */
    if ( (res_ext = strrchr(filename, '.')) && *(res_ext++) ) {
        for (i=0; demuxers[i]; i++) {
            char exts[128], *tkn; /* temp string containing extension
                                   * served by probing demuxer.
                                   */
            strncpy(exts, demuxers[i]->info->extensions, sizeof(exts)-1);

            for (tkn=strtok(exts, ","); tkn; tkn=strtok(NULL, ",")) {
                if (strcmp(tkn, res_ext) == 0)
                    continue;

                rc_log(RC_LOG_DEBUG, "[RC] probing demuxer: \"%s\" "
                        "matches \"%s\" demuxer\n", res_ext,
                        demuxers[i]->info->name);

                if (demuxers[i]->probe(filename) == RESOURCE_OK)
                    return demuxers[i];
            }
        }
    }

    for (i=0; demuxers[i]; i++)
        if ((demuxers[i]->probe(filename) == RESOURCE_OK))
            return demuxers[i];

    return NULL;
}
