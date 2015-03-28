#include <string.h>
#include "nmp_debug.h"
#include "nmp_record_list.h"


NmpRecordList *
nmp_record_list_new( void )
{
	NmpRecordList *list;

	list = g_new0(NmpRecordList, 1);
	return list;
}


static void
__nmp_record_free(NmpRecord *r)
{
	g_free(r);
}


void
nmp_record_list_free(NmpRecordList *list)
{
	G_ASSERT(list != NULL);

	g_list_foreach(list->list, (GFunc)__nmp_record_free, NULL);
	g_list_free(list->list);
	g_free(list);
}


void
nmp_record_list_add(NmpRecordList *list, time_t start,
	time_t end, guint size, guint flags, gchar *name, gchar *property)
{
	NmpRecord *r;

	r = g_new0(NmpRecord, 1);
	r->start = start;
	r->end = end;
	r->size = size;
	r->flags = flags;
	strncpy(r->name, name, RECORD_NAME_SIZE - 1);
	strncpy(r->property, property, RECORD_PROPERTY_SIZE - 1);

	++list->count;
	list->list = g_list_append(list->list, r);
}


gint
nmp_record_list_length(NmpRecordList *list)
{
	return list->count;
}


//:~ End
