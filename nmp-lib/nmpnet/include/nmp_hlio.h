/*
 * j_hlio.h
 *
 * This file declares high level io, payload layer.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#ifndef __NMP_HLIO_H__
#define __NMP_HLIO_H__

#include "nmp_io.h"

typedef struct _JHlIO JHlIO;

struct _JHlIO		/* High level IO, payload layer, for example: XML */
{
	JIO			io;

	JPayloadProto	*proto;
};

#ifdef __cplusplus
extern "C" {
#endif

JHlIO *j_hl_io_new(JConnection *conn, JPacketProto *ll_proto,
	JPayloadProto *hl_proto);

JHlIO *j_hl_listen_io_new(JConnection *conn, JPacketProto *ll_proto,
	JPayloadProto *hl_proto);

#ifdef __cplusplus
}
#endif

#endif	//__NMP_HLIO_H__
