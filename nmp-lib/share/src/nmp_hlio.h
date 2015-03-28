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

#include <glib.h>
#include "nmp_io.h"


G_BEGIN_DECLS

typedef struct _HmHlIO HmHlIO;

struct _HmHlIO		/* High level IO, payload layer, for example: XML */
{
	HmIO			io;

	NmpPayloadProto	*proto;
};


HmHlIO *nmp_hl_io_new(NmpConnection *conn, NmpPacketProto *ll_proto,
	NmpPayloadProto *hl_proto);

HmHlIO *nmp_hl_listen_io_new(NmpConnection *conn, NmpPacketProto *ll_proto,
	NmpPayloadProto *hl_proto);

G_END_DECLS

#endif	//__NMP_HLIO_H__
