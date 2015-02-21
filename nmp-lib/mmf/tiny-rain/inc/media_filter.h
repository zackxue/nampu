/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/


#ifndef __TINY_RAIN_MEDIA_FILTER_H__
#define __TINY_RAIN_MEDIA_FILTER_H__

#include "msg.h"

BEGIN_NAMESPACE

#define FILTER_MAX      		4

typedef struct __filter filter;
typedef struct __filter_ops filter_ops;

struct __filter
{
    obj __super;
    int32_t id;
    filter_ops *ops;
};

struct __filter_ops
{
    uint32_t filter_size;
    int32_t (*init)(filter *f);
    void    (*fin)(filter *f);
    int32_t (*cal)(filter *f, void *i, uint32_t i_size);
    int32_t (*fill)(filter *f, void *i, uint32_t i_size);
    int32_t (*pull)(filter *f, msg **o);
};

typedef struct __media_filter media_filter;
struct __media_filter
{
    filter *flts[FILTER_MAX];
};

media_filter *media_filter_new( void );
void media_filter_release(media_filter *mf);

int32_t media_filter_register(media_filter *mf, int32_t id,
    filter_ops *ops);

int32_t media_filter_cal_size(media_filter *mf, int32_t id,
	void *data, uint32_t size);
int32_t media_filter_fill(media_filter *mf, int32_t id,
    void *data, uint32_t size);
int32_t media_filter_pull(media_filter *mf, int32_t id, msg **o);

END_NAMESPACE

#endif  //__TINY_RAIN_MEDIA_FILTER_H__
