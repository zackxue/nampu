/*
 * nmp_share_hlio.h
 *
 * This file declares high level io, payload layer.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#ifndef __NMP_HLIO_H__
#define __NMP_HLIO_H__

#include <glib.h>
#include "nmp_share_io.h"


G_BEGIN_DECLS

typedef struct _NmpHlIO NmpHlIO;

struct _NmpHlIO		/* High level IO, payload layer, for example: XML */
{
	NmpIO			io;

	NmpPayloadProto	*proto;
};


NmpHlIO *nmp_hl_io_new(NmpConnection *conn, NmpPacketProto *ll_proto,
	NmpPayloadProto *hl_proto);

NmpHlIO *nmp_hl_listen_io_new(NmpConnection *conn, NmpPacketProto *ll_proto,
	NmpPayloadProto *hl_proto);

G_END_DECLS

#endif	//__NMP_HLIO_H__
