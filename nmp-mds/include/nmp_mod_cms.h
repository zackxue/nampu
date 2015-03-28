#ifndef __NMP_MOD_CMS_H__
#define __NMP_MOD_CMS_H__

#include <glib.h>

typedef struct __JpfModCms JpfModCms;

JpfModCms *nmp_mod_cms_new( void );
gint nmp_mod_cms_rtsp_port(JpfModCms *mod);
gint nmp_mod_cms_pu_port(JpfModCms *mod);
gint nmp_mds_cms_state(JpfModCms *mod);

#endif	/* __NMP_MOD_CMS_H__ */
