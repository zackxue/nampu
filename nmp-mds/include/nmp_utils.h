#ifndef __NMP_UTILS_H__
#define __NMP_UTILS_H__

#include <fcntl.h>
#include <glib.h>

G_BEGIN_DECLS

gint set_fd_flags(gint fd, gint flgs);

G_END_DECLS

#endif /* __NMP_UTILS_H__ */
