/*
 *          file: nmp_proxy_rw_file.h
 *          description:抽象文件读写接口
 *
 *          May 21th, 2013
 */

#ifndef __NMP_PROXY_RW_FILE_H__
#define __NMP_PROXY_RW_FILE_H__

#include <stdio.h>


#ifdef __cplusplus
extern "C" {
#endif


int read_file(char **buffer, const char *file_path);
int write_file(char *buffer, size_t size, const char *file_path);


#ifdef __cplusplus
    }
#endif

#endif  /* __NMP_PROXY_RW_FILE_H__ */

