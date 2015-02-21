#include "msg.h"

#define GET_MSG_OPS(msg) ops_array[(int32_t)msg&MT_MASK]
static msg_ops *ops_array[4] =  {NULL};


int32_t
register_msg_type(MSG_TYPE mt, msg_ops *ops)
{
	if (ops_array[mt])
		return -EEXIST;

	ops_array[mt] = ops;
	return 0;
}


msg *msg_ref(msg *m)
{
	msg_ops *ops;

	ops = GET_MSG_OPS(m);
	BUG_ON(!ops);

	return (*ops->ref)(m);
}


void msg_unref(msg *m)
{
	msg_ops *ops;

	ops = GET_MSG_OPS(m);
	BUG_ON(!ops);

	(*ops->unref)(m);
}


msg *msg_dup(msg  *m)
{
	msg_ops *ops;

	ops = GET_MSG_OPS(m);
	BUG_ON(!ops);

	return (*ops->dup)(m);
}


uint32_t msg_size(msg *m)
{
	msg_ops *ops;

	ops = GET_MSG_OPS(m);
	BUG_ON(!ops);

	return (*ops->msg_size)(m);
}


mem_block *msg_to_mb(msg *m)
{
	msg_ops *ops;

	ops = GET_MSG_OPS(m);
	BUG_ON(!ops);

	return (*ops->msg_to_mb)(m);
}

//:~ End
