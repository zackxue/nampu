/*
 * hm_fragments.h
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

void hm_net_packet_release_npi(HmNetPackInfo *npi);
HmNetPackInfo *hm_net_packet_defrag(HmNetPackInfo *net_pack);

G_END_DECLS

#endif  //__HM_FRAGMENT_H__
