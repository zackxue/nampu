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

/**
 * @file
 * @brief Functions to generate protocol responses (HTTP and RTSP)
 */

#ifndef NETEMBRYO_RTSP_H
#define NETEMBRYO_RTSP_H

/**
 * @brief Status codes for RTSP responses.
 *
 * The list is derived from the list in RFC2326, sections 7.1.1 and 11.
 */
typedef enum {
    RTSP_Continue = 100,
    RTSP_Ok = 200,
    RTSP_Created = 201,
    RTSP_Accepted = 202,
    RTSP_BadRequest = 400,
    RTSP_Forbidden = 403,
    RTSP_NotFound = 404,
    RTSP_NotAcceptable = 406,
    RTSP_UnsupportedMedia = 415,
    RTSP_ParameterNotUnderstood = 451,
    RTSP_NotEnoughBandwidth = 453,
    RTSP_SessionNotFound = 454,
    RTSP_InvalidMethodInState = 455,
    RTSP_HeaderFieldNotValidforResource = 456,
    RTSP_InvalidRange = 457,
    RTSP_AggregateNotAllowed = 459,
    RTSP_AggregateOnly = 460,
    RTSP_UnsupportedTransport = 461,
    RTSP_InternalServerError = 500,
    RTSP_NotImplemented = 501,
    RTSP_ServiceUnavailable = 503,
    RTSP_VersionNotSupported = 505,
    RTSP_OptionNotSupported = 551
} RTSP_ResponseCode;

const char *rtsp_reason_phrase(RTSP_ResponseCode code);

#endif
