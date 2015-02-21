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

#include "netembryo/wsocket.h"
#include <stdio.h>
#include <glib.h>
#include "gtest-extra.h"

void test_local_hostname()
{
  char hostname_output[1024] = { 0, };
  char hostname_buffer[1024] = { 0, };

  FILE *hn_fd = popen("hostname", "r");
  size_t len = fread(hostname_output, 1, sizeof(hostname_output)-1, hn_fd);

  gte_fail_if( !feof(hn_fd),
               "Hostname too long for buffer");
  fclose(hn_fd);

  gte_fail_if( get_local_hostname(hostname_buffer, sizeof(hostname_buffer)-1),
               "Unable to get local hostname");

  /* Skip the last character read from hostname(1) as it's a newline */
  hostname_output[len-1] = '\0';

  g_assert_cmpstr(hostname_buffer, ==, hostname_output);
}
