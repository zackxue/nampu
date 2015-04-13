/*
 * nmp_fragments.h
 *
 * Data structure for packet defragmentation. 
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:  Author:
 *
*/

#ifndef __NMP_FRAGMENT_H__
#define __NMP_FRAGMENT_H__

#include "nmp_netproto.h"

#ifdef __cplusplus
extern "C" {
#endif

static __inline__ void nmp_net_packet_release_npi(nmp_net_packinfo_t *npi){};
static __inline__ nmp_net_packinfo_t *nmp_net_packet_defrag(nmp_net_packinfo_t *net_pack){return net_pack;};

#ifdef __cplusplus
}
#endif

#endif  //__NMP_FRAGMENT_H__
