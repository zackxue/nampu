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

#define NMP_TYPE_MODTW		(nmp_mod_tw_get_type())
#define NMP_IS_MODTW(o)		(G_TYPE_CHECK_INSTANCE_TYPE((o), NMP_TYPE_MODTW))
#define NMP_IS_MODTW_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), NMP_TYPE_MODTW))
#define NMP_MODTW(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), NMP_TYPE_MODTW, NmpModTw))
#define NMP_MODTW_CLASS(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), NMP_TYPE_MODTW, NmpModTwClass))
#define NMP_MODTW_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), NMP_TYPE_MODTW, NmpModTwClass))

#define nmp_mod_tw_log(fmt, args ...) do {	\
	if (1)	\
		printf("<nmp_mod_tw_log> %s[%d]:"fmt, __FUNCTION__, __LINE__, ##args);	\
} while (0)

#define nmp_mod_tw_func_begin(fmt, args ...) do {	\
	if (1)	\
		printf("<b> %s[%d]__ _"fmt, __FUNCTION__, __LINE__, ##args);	\
} while (0)


typedef struct _NmpModTw NmpModTw;
struct _NmpModTw
{
	NmpAppMod		parent_object;
};

typedef struct _NmpModTwClass NmpModTwClass;
struct _NmpModTwClass
{
	NmpAppModClass		parent_class;
};


GType nmp_mod_tw_get_type(void);


gint nmp_mod_tw_event_handler(TW_INFO_TYPE cmd, void *in_parm, void *out_parm);


G_END_DECLS

#endif

