/*
 * nmp_hlio.h
 *
 * This file declares high level io, payload layer.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#ifndef __NMP_HLIO_H__
#define __NMP_HLIO_H__

#include "nmp_io.h"

typedef struct _nmp_hlio nmp_hlio_t;

struct _nmp_hlio		/* High level IO, payload layer, for example: XML */
{
	nmpio_t			io;

	nmp_payload_proto_t	*proto;
};

#ifdef __cplusplus
extern "C" {
#endif

nmp_hlio_t *nmp_hlio_io_new(nmp_conn_t *conn, nmp_packet_proto_t *ll_proto,
	nmp_payload_proto_t *hl_proto);

nmp_hlio_t *nmp_hlio_listen_io_new(nmp_conn_t *conn, nmp_packet_proto_t *ll_proto,
	nmp_payload_proto_t *hl_proto);

#ifdef __cplusplus
}
#endif

#endif	//__NMP_HLIO_H__
