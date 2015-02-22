/*
 *	author:	zyt
 *	time:	begin in 2012/9/28
 */
#ifndef __NMP_MOD_TW_H__
#define __NMP_MOD_TW_H__

#include "nmp_mods.h"
#include "nmp_tw_interface.h"


G_BEGIN_DECLS


#define DEFAULT_HEART_SLICE		(3)

#define JPF_TYPE_MODTW		(jpf_mod_tw_get_type())
#define JPF_IS_MODTW(o)		(G_TYPE_CHECK_INSTANCE_TYPE((o), JPF_TYPE_MODTW))
#define JPF_IS_MODTW_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), JPF_TYPE_MODTW))
#define JPF_MODTW(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), JPF_TYPE_MODTW, JpfModTw))
#define JPF_MODTW_CLASS(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), JPF_TYPE_MODTW, JpfModTwClass))
#define JPF_MODTW_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), JPF_TYPE_MODTW, JpfModTwClass))

#define jpf_mod_tw_log(fmt, args ...) do {	\
	if (1)	\
		printf("<jpf_mod_tw_log> %s[%d]:"fmt, __FUNCTION__, __LINE__, ##args);	\
} while (0)

#define jpf_mod_tw_func_begin(fmt, args ...) do {	\
	if (1)	\
		printf("<b> %s[%d]__ _"fmt, __FUNCTION__, __LINE__, ##args);	\
} while (0)


typedef struct _JpfModTw JpfModTw;
struct _JpfModTw
{
	JpfAppMod		parent_object;
};

typedef struct _JpfModTwClass JpfModTwClass;
struct _JpfModTwClass
{
	JpfAppModClass		parent_class;
};


GType jpf_mod_tw_get_type(void);


gint jpf_mod_tw_event_handler(TW_INFO_TYPE cmd, void *in_parm, void *out_parm);


G_END_DECLS

#endif

