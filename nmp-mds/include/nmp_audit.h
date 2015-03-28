#ifndef __NMP_AUDIT_H__
#define __NMP_AUDIT_H__

#include <glib.h>

typedef enum
{
	AUDIT_BRATE_SRC,
	AUDIT_BRATE_SINKER,
	AUDIT_CNT_MEDIA_SRC,
	AUDIT_CNT_MEDIA_SNK,
	AUDIT_CNT_STREAM_SRC,
	AUDIT_CNT_STREAM_SNK
}NmpAuditType;

typedef struct _NmpAuditContext NmpAuditContext;

void nmp_audit_init( void );
void nmp_audit_inc(NmpAuditType type, gint v);
void nmp_audit_dec(NmpAuditType type, gint v);
void nmp_audit_zero(NmpAuditContext *ctx);

#endif	/* __NMP_AUDIT_H__ */
