#ifndef __CMS_MSG_H__
#define __CMS_MSG_H__

gint nmp_mos_cms_make_query_guids_msg(NmpModCms *self, NmpSysMsg **pmsg,
	gint start_row, gint num);

gint nmp_mos_cms_make_query_mds_msg(NmpModCms *self, NmpSysMsg **pmsg,
	gint start_row, gint num);

#endif	/* __CMS_MSG_H__ */
