/*
 * nmp_fragments.h
 *
 * Data structure for packet defragmentation. 
 *
 * Copyright(c) by HiMickey, 2010~2014
 * Author:  Author:
 *
*/

#ifndef __JPF_FRAGMENT_H__
#define __JPF_FRAGMENT_H__

#include "nmp_netproto.h"

#ifdef __cplusplus
extern "C" {
#endif

static __inline__ void j_net_packet_release_npi(JNetPackInfo *npi){};
static __inline__ JNetPackInfo *j_net_packet_defrag(JNetPackInfo *net_pack){return net_pack;};

#ifdef __cplusplus
}
#endif

#endif  //__JPF_FRAGMENT_H__
