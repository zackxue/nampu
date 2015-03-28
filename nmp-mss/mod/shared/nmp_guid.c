#include <stdio.h>
#include "nmp_errno.h"
#include "nmp_guid.h"

#define __GUID_SET_INCREMENT__	32


gint
nmp_guid_equal(NmpGuid *guid_1, NmpGuid *guid_2)
{
	G_ASSERT(guid_1 && guid_2);

	return !strcmp(guid_1->domain_id, guid_2->domain_id)
		&& !strcmp(guid_1->guid, guid_2->guid);
}


void
nmp_guid_assign(NmpGuid *new, NmpGuid *old)
{
	G_ASSERT(new && old);

	*new = *old;
}


void
nmp_guid_generate(NmpGuid *new, gchar *domain, gchar *id)
{
	G_ASSERT(new);

	memset(new, 0, sizeof(*new));
	strncpy(new->domain_id, domain, DOMAIN_ID_LEN - 1);
	strncpy(new->guid, id, MAX_ID_LEN - 1);
}


NmpGuidSet *
nmp_guid_set_new( void )
{
	NmpGuidSet *set;

	set = g_new0(NmpGuidSet, 1);
	set->capacity = __GUID_SET_INCREMENT__;
	set->arr = g_new0(NmpGuid, set->capacity);
	return set;
}


void
nmp_guid_set_delete(NmpGuidSet *set)
{
	g_free(set->arr);
	g_free(set);
}


static __inline__ void
nmp_guid_set_increase(NmpGuidSet *set)
{
	gint new_capacity;
	NmpGuid *g;

	new_capacity = set->capacity + __GUID_SET_INCREMENT__;
	g = g_new0(NmpGuid, new_capacity);
	memcpy(g, set->arr, set->capacity * sizeof(NmpGuid));
	g_free(set->arr);
	set->arr = g;
	set->capacity = new_capacity;
}


gint
nmp_guid_set_find(NmpGuidSet *set, gchar *domain, gchar *guid)
{
	gint index;
	NmpGuid *g;

	for (index = 0; index < set->size; ++index)
	{
		g = &set->arr[index];
		if (!strcmp(g->domain_id, domain) && !strcmp(g->guid, guid))
			return 1;
	}

	return 0;
}


gint
nmp_guid_set_add(NmpGuidSet *set, gchar *domain, gchar *guid)
{
	NmpGuid *g;

	if (nmp_guid_set_find(set, domain, guid))
		return -EEXIST;

	if (set->size >= set->capacity)
		nmp_guid_set_increase(set);

	g = &set->arr[set->size];
	++set->size;

	memset(g, 0, sizeof(*g));
	strncpy(g->domain_id, domain, DOMAIN_ID_LEN - 1);
	strncpy(g->guid, guid, MAX_ID_LEN - 1);

	return 0;
}


static __inline__ void
__nmp_guid_set_diff(NmpGuidSet *delta, NmpGuidSet *set1, NmpGuidSet *set2)
{
	gint index;
	NmpGuid *g;

	for (index = 0; index < set1->size; ++index)
	{
		g = &set1->arr[index];
		if (!nmp_guid_set_find(set2, g->domain_id, g->guid))
			nmp_guid_set_add(delta, g->domain_id, g->guid);
	}
}


NmpGuidSet *
nmp_guid_set_diff(NmpGuidSet *set1, NmpGuidSet *set2)
{
	NmpGuidSet *delta;

	delta = nmp_guid_set_new();
	__nmp_guid_set_diff(delta, set1, set2);

	return delta;
}


gint
nmp_guid_set_empty(NmpGuidSet *set)
{
	G_ASSERT(set != NULL);

	return set->size == 0;
}


void
nmp_guid_set_foreach(NmpGuidSet *set, void (*fun)(NmpGuid *guid, void *parm),
	void *parm)
{
	gint index;
	G_ASSERT(set != NULL);

	for (index = 0; index < set->size; ++index)
	{
		fun(&set->arr[index], parm);
	}
}


void
nmp_guid_set_print(NmpGuidSet *set)
{
	gint index;

	if (!set)
		return;

	for (index = 0; index < set->size; ++index)
		printf("set:%p, %s, %s\n", set, set->arr[index].domain_id, set->arr[index].guid);
}


NmpDiffSet *
nmp_diff_set_new( void )
{
	NmpDiffSet *set;

	set = g_new0(NmpDiffSet, 1);
	return set;
}


void
nmp_diff_set_delete(NmpDiffSet *set)
{
	if (set->add)
		nmp_guid_set_delete(set->add);

	if (set->del)
		nmp_guid_set_delete(set->del);

	g_free(set);
}


gint
nmp_diff_set_empty(NmpDiffSet *set)
{
	G_ASSERT(set != NULL);

	if (set->add && !nmp_guid_set_empty(set->add))
		return 0;

	if (set->del && !nmp_guid_set_empty(set->del))
		return 0;

	return 1;
}


//:~ End
