#ifndef __NMP_GUID_H__
#define __NMP_GUID_H__

#include <glib.h>
#include "nmp_message.h"


G_BEGIN_DECLS


typedef struct __NmpGuid NmpGuid;
struct __NmpGuid
{
	gchar	domain_id[DOMAIN_ID_LEN];
	gchar	guid[MAX_ID_LEN];
};


typedef struct __NmpGuidSet NmpGuidSet;
struct __NmpGuidSet
{
	gint	capacity;
	gint	size;

	NmpGuid	*arr;
};


typedef struct __NmpDiffSet NmpDiffSet;
struct __NmpDiffSet
{
	NmpGuidSet *add;
	NmpGuidSet *del;
};



gint nmp_guid_equal(NmpGuid *guid_1, NmpGuid *guid_2);
void nmp_guid_assign(NmpGuid *new, NmpGuid *old);
void nmp_guid_generate(NmpGuid *new, gchar *domain, gchar *id);

NmpGuidSet *nmp_guid_set_new( void );
void nmp_guid_set_delete(NmpGuidSet *set);
gint nmp_guid_set_add(NmpGuidSet *set, gchar *domain, gchar *guid);
NmpGuidSet *nmp_guid_set_diff(NmpGuidSet *set1, NmpGuidSet *set2);
gint nmp_guid_set_empty(NmpGuidSet *set);
void nmp_guid_set_foreach(NmpGuidSet *set, void (*fun)(NmpGuid *guid, void *parm),
	void *parm);
void nmp_guid_set_print(NmpGuidSet *set);

NmpDiffSet *nmp_diff_set_new( void );
void nmp_diff_set_delete(NmpDiffSet *set);
gint nmp_diff_set_empty(NmpDiffSet *set);

G_END_DECLS

#endif	/* __NMP_GUID_H__ */
