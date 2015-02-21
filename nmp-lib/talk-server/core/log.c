#include <stdarg.h>
#include "jlib.h"
#include "macros.h"
#include "alloc.h"

#include "log.h"

#define LOG_TEXT_SIZE			256
#define LOG_MAX_BACKLOG			64

typedef struct __log_item log_item;
struct __log_item
{
	uint32_t err;
	uint8_t text[LOG_TEXT_SIZE];
};


static log_handler_t log_it = NULL; 
static int32_t log_verbose = LVL_DEBUG;
static atomic_t backlog = ATOMIC_INIT;
static JThreadPool *log_tp = NULL;		/* For log service task */


static void
log_service_op_fun(void *data, void *user_data)
{
	log_item *li = (log_item*)data;

	if (log_it)
	{
		(*log_it)(li->text);
		if (li->err)
		{
			TR_FATAL_ERROR_EXIT;
		}
	}
	else
	{
		if (li->err)
		{
			fprintf(stderr, "%s", li->text);
			TR_FATAL_ERROR_EXIT;
		}
		else
		{
			fprintf(stdout, "%s", li->text);
		}
	}

	tr_free(li, sizeof(*li));
	atomic_dec(&backlog);
}


void ___tr_log(int32_t level, char *file, int32_t line, const char *fmt, ...)
{
	va_list ap;
	log_item *li;

	if (!log_tp || level > log_verbose)
		return;
	if (atomic_get(&backlog) >= LOG_MAX_BACKLOG)
		return;
	atomic_inc(&backlog);

	li = tr_alloc(sizeof(*li));
	if (!li)
	{
		atomic_dec(&backlog);
		return;
	}

	strcpy(__str(li->text), "LOG: ");
	va_start(ap, fmt);
	vsnprintf(__str(li->text) + 5, sizeof(li->text) - 7, fmt, ap);	/* -6 */
	va_end(ap);
	strcat(__str(li->text), "\n");

	if (level == LVL_ERROR)
		li->err = 1;
	else
		li->err = 0;
	j_thread_pool_push(log_tp, li);
}


void init_log_facility( void )
{
	if (log_tp)
		return;

	log_tp = j_thread_pool_new(log_service_op_fun, NULL, 1, NULL);
	BUG_ON(!log_tp);
}


int32_t tr_log_set_verbose(int32_t level)
{
	if (level > LVL_DEBUG)
		level = LVL_DEBUG;
	if (level < LVL_ERROR)
		level = LVL_ERROR;
	log_verbose = level;
	return 0;
}


void tr_log_set_handler(log_handler_t handler)
{
	log_it = handler;
}

//:~ End
