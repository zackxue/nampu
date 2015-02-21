/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/


#ifndef __TINY_RAIN_SIP_CLIENT_H__
#define __TINY_RAIN_SIP_CLIENT_H__

#include "network_client.h"

BEGIN_NAMESPACE

typedef struct __sip_client sip_client;
typedef struct __sip_client_ops sip_client_ops;

struct __sip_client
{
	network_client __super;
	sip_client_ops *ops;
};

struct __sip_client_ops
{
};

END_NAMESPACE

#endif	//__TINY_RAIN_SIP_CLIENT_H__

