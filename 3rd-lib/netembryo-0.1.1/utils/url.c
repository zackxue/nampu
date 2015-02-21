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

#include "url.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

/**
 * Creates an Url informations structure from a URI string
 *
 * @param url The url to initialize (Will not free the previous data,
 *            Url_destroy must be used to free it if we already initialized
 *            the structure before)
 * @param urlname The URI to parse to create the Url informations
 * @return Always 0, errors will be reported by setting to NULL the field of
 *         the Url structure that the function was not able to parse.
 */
int Url_init(Url * url, char * urlname)
{
    char * protocol_begin, * hostname_begin, * port_begin, * path_begin;
    size_t protocol_len, hostname_len, port_len, path_len;

    memset(url, 0, sizeof(Url));

    hostname_begin = strstr(urlname, "://");
    if (hostname_begin == NULL) {
        hostname_begin = urlname;
        protocol_begin = NULL;
        protocol_len = 0;
    } else {
        protocol_len = (size_t)(hostname_begin - urlname);
        hostname_begin = hostname_begin + 3;
        protocol_begin = urlname;
    }

    hostname_len = strlen(urlname) - ((size_t)(hostname_begin - urlname));

    path_begin = strstr(hostname_begin, "/");
    if (path_begin == NULL) {
        path_len = 0;
    } else {
        ++path_begin;
        hostname_len = (size_t)(path_begin - hostname_begin - 1);
        path_len = strlen(urlname) - ((size_t)(path_begin - urlname));
    }

    port_begin = strstr(hostname_begin, ":");
    if ((port_begin == NULL) ||
        ((port_begin >= path_begin) && (path_begin != NULL))) {
        port_len = 0;
        port_begin = NULL;
    } else {
        ++port_begin;
        if (path_len)
            port_len = (size_t)(path_begin - port_begin - 1);
        else
            port_len = strlen(urlname) - ((size_t)(port_begin - urlname));
        hostname_len = (size_t)(port_begin - hostname_begin - 1);
    }

    if (protocol_len) {
        url->protocol = (char*)malloc(protocol_len+1);
        strncpy(url->protocol, protocol_begin, protocol_len);
        url->protocol[protocol_len] = '\0';
    }

    if (port_len) {
        url->port = (char*)malloc(port_len+1);
        strncpy(url->port, port_begin, port_len);
        url->port[port_len] = '\0';
    }

    if (path_len) {
        url->path = (char*)malloc(path_len+1);
        strncpy(url->path, path_begin, path_len);
        url->path[path_len] = '\0';
    }

    url->hostname = (char*)malloc(hostname_len+1);
    strncpy(url->hostname, hostname_begin, hostname_len);
    url->hostname[hostname_len] = '\0';

    return 0;
}

/**
 * Will destroy the Url structure freeing the data contained in it
 *
 * @param url The Url structure to destroy
 */
void Url_destroy(Url * url)
{
    free(url->protocol);
    free(url->hostname);
    free(url->port);
    free(url->path);
}
