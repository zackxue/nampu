
#ifndef __FILE_H__
#define __FILE_H__

int open_file(const char *file_path);
int read_file(int fd, char *buf, int buf_size);
int close_file(int fd);

#endif

