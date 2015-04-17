/*
 * nmp_share_fragments.h
 *
 * Data structure for packet defragmentation. 
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:  Author:
 *
*/

#ifndef __NMP_FRAGMENT_H__
#define __NMP_FRAGMENT_H__

#include <glib.h>
#include "nmp_share_netproto.h"

G_BEGIN_DECLS

void nmp_net_packet_release_npi(NmpNetPackInfo *npi);
NmpNetPackInfo *nmp_net_packet_defrag(NmpNetPackInfo *net_pack);

G_END_DECLS

#endif  //__NMP_FRAGMENT_H__
