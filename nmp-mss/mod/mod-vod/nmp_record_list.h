#ifndef __NMP_RECORD_LIST_H__
#define __NMP_RECORD_LIST_H__

#include <time.h>
#include <glib.h>

#define RECORD_PROPERTY_SIZE		128
#define RECORD_NAME_SIZE			32

typedef struct __NmpRecord NmpRecord;
struct __NmpRecord
{
	time_t			start;
	time_t			end;
	guint			flags;	/* type etc. */
	guint           size;
	gchar           name[RECORD_NAME_SIZE];
	gchar			property[RECORD_PROPERTY_SIZE];
};


typedef struct __NmpRecordList NmpRecordList;
struct __NmpRecordList
{
	GList			*list;
	gint			count;
};


NmpRecordList *nmp_record_list_new( void );
void nmp_record_list_free(NmpRecordList *list);

void nmp_record_list_add(NmpRecordList *list, time_t start,
	time_t end, guint size, guint flags, gchar *name, gchar *property);

gint nmp_record_list_length(NmpRecordList *list);


#endif	/* __NMP_RECORD_LIST_H__ */
