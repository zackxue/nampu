/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/

#ifndef __TINY_RAIN_MSG_H__
#define __TINY_RAIN_MSG_H__

#include "obj.h"
#include "mem_block.h"

BEGIN_NAMESPACE

#define MT_MASK  0x03
#define GET_MSG_ADDR(m) ((msg*)((unsigned long)m&(~MT_MASK)))
#define SET_MSG_TYPE(m, mt) ((msg*)((void*)m + mt))

typedef enum
{
	MT_01 = 0x01,
	MT_02 = 0x02,
	MT_03 = 0x03
}MSG_TYPE;

typedef struct __msg msg;
typedef struct __msg_ops msg_ops;

struct __msg_ops
{
	msg *(*ref)(msg *m);
	void (*unref)(msg *m);
	msg *(*dup)(msg *m);
	uint32_t (*msg_size)(msg *m);
	mem_block *(*msg_to_mb)(msg *m);
};

int32_t register_msg_type(MSG_TYPE mt, msg_ops *ops); 

msg *msg_ref(msg *m);
void msg_unref(msg *m);

msg *msg_dup(msg  *m);
uint32_t msg_size(msg *m);

mem_block *msg_to_mb(msg *m);	//@{get msg data, and unref msg}

END_NAMESPACE

#endif	//__TINY_RAIN_MSG_H__
