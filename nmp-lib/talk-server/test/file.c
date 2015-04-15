#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>

#include "file.h"

int open_file(const char *file_path)
{
    int fd = open(file_path, O_RDONLY);
    if (fd < 0)
    {
        return -errno;
    }

    return fd;
}

int read_file(int fd, char *buf, int buf_size)
{
    int ret = 0;
    int read_len = 0;

    if (fd < 0 || !buf || buf_size <= 0)
    {
        return -1;
    }

    ret = read(fd, buf, buf_size);
    if (ret != buf_size)
    {
        if (lseek(fd, 0, SEEK_SET) < 0)
        {
            printf("lseek error\n");
        }
        return -1;
    }

    return ret;
}

int close_file(int fd)
{
    if (fd < 0)
    {
        return -1;
    }

    return close(fd);
}

