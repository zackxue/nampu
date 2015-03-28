#include "nmp_errno.h"
#include "nmp_utils.h"
#include "nmp_debug.h"

gint
set_fd_flags(gint fd, gint flgs)
{
    gint old_flgs, err;

    old_flgs = fcntl(fd, F_GETFL, 0);
    if (G_UNLIKELY(old_flgs < 0))
    {
    	err = -errno;
    	nmp_warning(
    		"set_fd_flags()->fcntl(fd, F_GETFL, 0)"
    	);
        return err;
    }

    old_flgs |= flgs;

    if (fcntl(fd, F_SETFL, old_flgs) < 0)
    {
    	err = -errno;
    	nmp_warning(
    		"set_fd_flags()->fcntl(fd, F_SETFL, 0)"
    	);
        return err;
    }

    return 0;
}

//:~ End
