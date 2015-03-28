#ifndef __NMP_RECORDS_H__
#define __NMP_RECORDS_H__

#include <time.h>
#include "nmp_record_list.h"

typedef struct __NmpRecords NmpRecords;
typedef struct __NmpRecordsOps NmpRecordsOps;


struct __NmpRecordsOps
{
	gint (*get_list)(NmpRecords *rs, NmpRecordList *l, gchar *ch,
		time_t start, time_t end, gint row, gint length, guint flags);
};


struct __NmpRecords
{
	NmpRecordsOps *ops;
};


void
nmp_record_stor_init();

gint
nmp_records_get_list_2(NmpRecords *rs, NmpRecordList *l, gchar *ch,
	time_t start, time_t end, gint row, gint length, guint flags);

NmpRecords *nmp_records_new(NmpRecordsOps *ops);

void nmp_records_free(NmpRecords *rs); 

gint nmp_records_get_list(NmpRecords *rs, NmpRecordList *l, gchar *ch,
	time_t start, time_t end, gint row, gint length, guint flags);


#endif	/* __NMP_RECORDS_H__ */
