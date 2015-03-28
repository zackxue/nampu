#include "nmp_records.h"
#include "nmp_debug.h"
#include "nmp_errno.h"



#define RECORD_STOR_NUM			(3)
#define RECORD_NODE_NAME_LEN		(128)
#define RECORD_NODE_UPDATE_TIME	(10 * 60)
#define RECORD_MAX_COUNT			(50000)


typedef struct _NmpRecordNode NmpRecordNode;
struct _NmpRecordNode
{
	gchar			name[RECORD_NODE_NAME_LEN];
	GTime			time;
	NmpRecordList		*record;
};

typedef struct _NmpRecordStor NmpRecordStor;
struct _NmpRecordStor
{
	GMutex			*mutex;
	NmpRecordNode	node[RECORD_STOR_NUM];
};

static NmpRecordStor g_record_stor;

#define RECORD_NODE_NEW(cur_time, node_time) \
	(cur_time >= node_time && cur_time - node_time <= RECORD_NODE_UPDATE_TIME)



static __inline__ void
__nmp_record_stor_init(NmpRecordStor *record_stor)
{
	memset(record_stor, 0, sizeof(NmpRecordStor));
	record_stor->mutex = g_mutex_new();
}

void
nmp_record_stor_init()
{
	__nmp_record_stor_init(&g_record_stor);
}


static void
nmp_record_stor_clean(NmpRecordStor *record_stor, gchar *node_name)
{
	NmpRecordNode *node;
	gint i;

	for (i = 0; i < RECORD_STOR_NUM; i++)
	{
		node = &record_stor->node[i];
		if (strcmp(node->name, node_name) == 0)
		{
			if (node->record)
				nmp_record_list_free(node->record);
			memset(node, 0, sizeof(NmpRecordNode));
		}
	}
}


static gint
nmp_record_stor_get_list(NmpRecordStor *record_stor, NmpRecordList *l, 
	gchar *node_name, gint row, gint length)
{
	NmpRecordNode *node;
	GTime cur_time;
	GList *list;
	NmpRecord *record, *new_record;
	gint cur_row = 0;
	gint i;

	cur_time = time(NULL);

	for (i = 0; i < RECORD_STOR_NUM; i++)
	{
		node = &record_stor->node[i];
		if (strcmp(node->name, node_name) == 0 && 
			RECORD_NODE_NEW(cur_time, node->time))
		{
			BUG_ON(!node->record);
			list = node->record->list;

			while (1)
			{
				if (!list)
					break;

				if (l->count >= length)
					break;

				if (++cur_row - 1 < row)
				{
					list = g_list_next(list);
					continue;
				}

				record = (NmpRecord *)list->data;
				new_record = g_new0(NmpRecord, 1);
				memcpy(new_record, record, sizeof(NmpRecord));

				++l->count;
				l->list = g_list_append(l->list, new_record);

				list = g_list_next(list);
			}

			return node->record->count;
		}
	}

	return -1;
}


static NmpRecordNode*
nmp_record_stor_get_free_node(NmpRecordStor *record_stor)
{
	NmpRecordNode *node, *free_node = NULL;
	gint i, oldest = -1;
	GTime oldest_time, cur_time;

	cur_time = time(NULL);

	for (i = 0; i < RECORD_STOR_NUM; i++)
	{
		node = &record_stor->node[i];
		if (!RECORD_NODE_NEW(cur_time, node->time))
		{
			free_node = node;
			break;
		}

		if (oldest == -1)
		{
			oldest_time = node->time;
			oldest = i;
		}
		else if (node->time < oldest_time)
		{
			oldest_time = node->time;
			oldest = i;
		}
	}

	if (!free_node)
	{
		free_node = &record_stor->node[oldest];
	}

	if (free_node->record)
	{
		nmp_record_list_free(free_node->record);
	}

	memset(free_node, 0, sizeof(NmpRecordNode));

	return free_node;
}


static __inline__ gint
__nmp_records_get_list_2(NmpRecordStor *record_stor, NmpRecords *rs, 
	NmpRecordList *l, gchar *ch, time_t start, time_t end, gint row, gint length, 
	guint flags)
{
	gchar node_name[RECORD_NODE_NAME_LEN] = {0};
	NmpRecordNode *fill_node;
	gint ret;

	snprintf(node_name, RECORD_NODE_NAME_LEN, "%s@%u@%lu@%lu", 
		ch, flags, start, end);

	if (row == 0)
	{
		nmp_record_stor_clean(record_stor, node_name);
	}
	else
	{
		ret = nmp_record_stor_get_list(record_stor, l, node_name, row, length);
		if (ret >= 0)
			return ret;
	}

	fill_node = nmp_record_stor_get_free_node(record_stor);
	memset(fill_node, 0, sizeof(NmpRecordNode));

	if (rs->ops && rs->ops->get_list)
	{
		fill_node->record = nmp_record_list_new();
	
		ret = (*rs->ops->get_list)(rs, fill_node->record, ch, start, end, 0,
			RECORD_MAX_COUNT, flags);
		if (ret < 0)
		{
			nmp_record_list_free(fill_node->record);
			fill_node->record = NULL;
			return ret;
		}

		strncpy(fill_node->name, node_name, RECORD_NODE_NAME_LEN - 1);
		fill_node->time = time(NULL);
	}
	else
	{
		nmp_warning("<NmpModVod> error:rs->ops=NULL or rs->ops->get_list=NULL!!!");
		return -998;
	}

	ret = nmp_record_stor_get_list(record_stor, l, node_name, row, length);
	if (ret >= 0)
		return ret;
	else
	{
		nmp_warning("<NmpModVod> error:nmp_record_stor_get_list failed!!!");
		return -999;
	}
}

static __inline__ gint
_nmp_records_get_list_2(NmpRecordStor *record_stor, NmpRecords *rs, 
	NmpRecordList *l, gchar *ch, time_t start, time_t end, gint row, gint length, 
	guint flags)
{
	gint ret;

	g_mutex_lock(record_stor->mutex);
	ret = __nmp_records_get_list_2(&g_record_stor, rs, l, ch, start, end, 
		row, length, flags);
	g_mutex_unlock(record_stor->mutex);

	return ret;
}

gint
nmp_records_get_list_2(NmpRecords *rs, NmpRecordList *l, gchar *ch,
	time_t start, time_t end, gint row, gint length, guint flags)
{
	G_ASSERT(rs != NULL);

	return _nmp_records_get_list_2(&g_record_stor, rs, l, ch, start, end, 
		row, length, flags);
}



NmpRecords *
nmp_records_new(NmpRecordsOps *ops)
{
	NmpRecords *rs;

	rs = g_new0(NmpRecords, 1);
	rs->ops = ops;
	return rs;
}


void
nmp_records_free(NmpRecords *rs)
{
	g_free(rs);
}


gint
nmp_records_get_list(NmpRecords *rs, NmpRecordList *l, gchar *ch,
	time_t start, time_t end, gint row, gint length, guint flags)
{
	gint ret = -EINVAL;
	G_ASSERT(rs != NULL);

	if (rs->ops && rs->ops->get_list)
	{
		ret = (*rs->ops->get_list)(rs, l, ch, start, end, row,
			length, flags);
	}

	return ret;
}

//:~ End
