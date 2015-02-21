/* *
 * * This file is part of NetEmbryo
 *
 * Copyright (C) 2008 by LScube team <team@streaming.polito.it>
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

#include "netembryo/url.h"
#include <glib.h>
#include "gtest-extra.h"

static void test_url(const char * url, const char * protocol,
                     const char * hostname, const char * port,
                     const char * path)
{
    Url turl;

    Url_init(&turl, url);

    if ( protocol && !turl.protocol )
        gte_fail("Expected protocol specified, but no protocol identified");
    else if ( !protocol && turl.protocol )
        gte_fail("No protocol expected, but protocol identified: '%s'", turl.protocol);
    else if ( protocol && protocol )
        g_assert_cmpstr(turl.protocol, ==, protocol);

    if ( hostname && !turl.hostname )
        gte_fail("Expected hostname specified, but no hostname identified");
    else if ( !hostname && turl.hostname )
        gte_fail("No hostname expected, but hostname identified: '%s'", turl.hostname);
    else if ( hostname && hostname )
        g_assert_cmpstr(turl.hostname, ==, hostname);

    if ( port && !turl.port )
        gte_fail("Expected port specified, but no port identified");
    else if ( !port && turl.port )
        gte_fail("No port expected, but port identified: '%s'", turl.port);
    else if ( port && port )
        g_assert_cmpstr(turl.port, ==, port);

    if ( path && !turl.path )
        gte_fail("Expected path specified, but no path identified");
    else if ( !path && turl.path )
        gte_fail("No path expected, but path identified: '%s'", turl.path);
    else if ( path && path )
        g_assert_cmpstr(turl.path, ==, path);

    Url_destroy(&turl);
}

void test_long_url()
{
    test_url("rtsp://this.is.a.very.long.url:this_should_be_the_port/this/is/a/path/to/file.wmv",
             "rtsp", "this.is.a.very.long.url", "this_should_be_the_port", "this/is/a/path/to/file.wmv");
}

void test_split_url()
{
    test_url("rtsp://this.is.the.host/this/is/the/path.avi", "rtsp", "this.is.the.host", NULL, "this/is/the/path.avi");
}

void test_no_proto()
{
    test_url("host:80/file.wmv", NULL, "host", "80", "file.wmv");
}

void test_no_path()
{
    test_url("host/file.wmv", NULL, "host", NULL, "file.wmv");
}

void test_just_host()
{
    test_url("host", NULL, "host", NULL, NULL);
}

void test_just_proto_host()
{
    test_url("rtsp://host", "rtsp", "host", NULL, NULL);
}

void test_just_proto_host_port()
{
    test_url("rtsp://host:port", "rtsp", "host", "port", NULL);
}
