/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/


#ifndef __TINY_RAIN_NETWORK_SINKER_H__
#define __TINY_RAIN_NETWORK_SINKER_H__

#include "media_sinker.h"
#include "media_stm.h"
#include "proto_watch.h"

BEGIN_NAMESPACE

typedef struct __sinker_param sinker_param;
struct __sinker_param
{
	int32_t factory;
	int32_t l4_proto;
	int32_t stms_type[ST_MAX];
};

typedef struct __network_sinker network_sinker;
typedef struct __network_sinker_ops network_sinker_ops;

struct __network_sinker
{
	media_sinker __super;

	int32_t factory;
	int32_t l4_proto;

	struct {
		int32_t stm_type;
		int32_t port;
		int32_t interleaved;
		proto_watch *stm_watch;
	}stms[ST_MAX];

	network_sinker_ops *ops;
};

struct __network_sinker_ops
{
	int32_t (*init)(network_sinker *ns, sinker_param *sp);
	void	(*fin)(network_sinker *ns);
	void	(*kill)(network_sinker *ns);
	int32_t (*consumable)(network_sinker *ns, proto_watch *pw, int32_t stm_i, uint32_t size);
	int32_t (*consume)(network_sinker *ns, proto_watch *pw, int32_t stm_i, msg *m, uint32_t seq);
	int32_t (*set_config)(network_sinker *ns, void *in, void *data);
	int32_t (*get_config)(network_sinker *ns, void *in, void *data);
};

network_sinker *network_sinker_alloc(uint32_t size, network_sinker_ops *ops,
	sinker_param *sp);

END_NAMESPACE

#endif	//__TINY_RAIN_NETWORK_SINKER_H__
