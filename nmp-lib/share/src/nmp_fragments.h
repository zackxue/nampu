/*
 * jpf_fragments.h
 *
 * Data structure for packet defragmentation. 
 *
 * Copyright(c) by HiMickey, 2010~2014
 * Author:  Author:
 *
*/

#ifndef __HM_FRAGMENT_H__
#define __HM_FRAGMENT_H__

#include <glib.h>
#include "nmp_netproto.h"

G_BEGIN_DECLS

void jpf_net_packet_release_npi(JpfNetPackInfo *npi);
JpfNetPackInfo *jpf_net_packet_defrag(JpfNetPackInfo *net_pack);

G_END_DECLS

#endif  //__HM_FRAGMENT_H__
