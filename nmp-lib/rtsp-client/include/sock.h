#ifndef __NMP_SOCK_H__
#define __NMP_SOCK_H__

#include <fcntl.h>
#include "rtsp_client.h"
#include "media.h"

gint unix_sock_bind(const gchar *host, gint port, L4_Proto l4);
gint unix_sock_connect(gint sock, const char *host, gint port);
gint rtp_session_sock_init(RTP_Session *rtp_s);
void rtp_session_sock_close(RTP_Session *rtp_s);
gint set_fd_flags(gint fd, gint flgs);

#endif	/* __NMP_SOCK_H__ */
