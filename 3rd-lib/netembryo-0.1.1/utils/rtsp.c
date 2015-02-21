/* *
 * * This file is part of NetEmbryo
 *
 * Copyright (C) 2009 by LScube team <team@lscube.org>
 * See AUTHORS for more details
 *
 * NetEmbryo is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * NetEmbryo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with NetEmbryo; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * */

#include "netembryo/rtsp.h"

#include <stddef.h>
#include <assert.h>

/**
 * @brief Table of default response phrases for RTSP
 *
 * This table contains the default response for a given code for RTSP.
 *
 * It is indexed first by the three-digits response code.
 *
 * The list of response phrases is available in RFC 2326 section 7.1.1
 *
 * @todo Provide a size-optimised alternative.
 */
static const char *const rtsp_responses[1000] = {
    [100] = "Continue",
    [200] = "OK",
    [201] = "Created",
    [202] = "Accepted",
    [400] = "Bad Request",
    [403] = "Forbidden",
    [404] = "Not Found",
    [406] = "Not Acceptable",
    [415] = "Unsupported Media Type",
    [451] = "Parameter Not Understood",
    [453] = "Not Enough Bandwith",
    [454] = "Session Not Found",
    [455] = "Method Not Valid In This State",
    [456] = "Header Field Not Valid for Resource",
    [457] = "Invalid Range",
    [459] = "Aggregate Operation Not Allowed",
    [460] = "Only Aggregate Operation Allowed",
    [461] = "Unsupported Transport",
    [500] = "Internal Server Error",
    [501] = "Not Implemented",
    [503] = "Service Unavailable",
    [505] = "RTSP Version Not Supported",
    [551] = "Option not supported",
    [999] = NULL
};

/**
 * @brief Takes care of mapping a numerical response code for RTSP in a phrase
 *
 * @param code The numerical response ID from RFC 2326
 * @return The response phrase connected to the code, or NULL if unknown.
 *
 * @todo This function should allow internationalisation of the response
 *       phrases.
 */
const char *rtsp_reason_phrase(RTSP_ResponseCode code)
{
    /* make sure the code is valid and we don't exceed our boundaries. */
    assert(100 <= code && code <= 999);

    return rtsp_responses[code];
}
