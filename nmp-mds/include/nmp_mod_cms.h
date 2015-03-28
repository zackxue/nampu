#ifndef __NMP_MOD_CMS_H__
#define __NMP_MOD_CMS_H__

#include <glib.h>

typedef struct __NmpModCms NmpModCms;

NmpModCms *nmp_mod_cms_new( void );
gint nmp_mod_cms_rtsp_port(NmpModCms *mod);
gint nmp_mod_cms_pu_port(NmpModCms *mod);
gint nmp_mds_cms_state(NmpModCms *mod);

#endif	/* __NMP_MOD_CMS_H__ */
