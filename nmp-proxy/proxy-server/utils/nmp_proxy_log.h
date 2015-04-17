
#ifndef __SHOW_LOG_H__
#define __SHOW_LOG_H__

/*********************************************************************************

file:nmp_proxy_log.h
description:
    这个头文件规范化我们程序输出调试、警告、错误等信息的方式。
    现在信息分为五个级别，Linux提供了七个级别，我们不使用最高的两个级别。
    我们程序中可根据需要用。

    五个级别对应的宏函数如下：(使用方式与printf相同)
    show_debug
    show_info
    show_warn
    show_error
    show_critical

    其中，show_error和show_critical只应在系统的errno有值的时候用，
    例如打开文件找不到，系统会设置errno的值。
    show_error和show_critical的时候，除了输出你指定的信息，还会附带输出
    errno对应的信息，这样可方便你分析出错条件。

*********************************************************************************/

#if _DEBUG_

#define show_debug(fmt, ...) \
    do {\
        printf("DEBUG: %s(%d) "fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
    }while (0)

#define show_info(fmt, ...) \
    do {\
        printf("INFO: %s(%d) "fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
    }while (0)

#define show_warn(fmt, ...) \
    do {\
        printf("WARN: %s(%d) "fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
    }while (0)

#define show_error(fmt, ...) \
    do {\
        printf("ERROR: %s(%d) [%m] "fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
    }while (0)

#define show_critical(fmt, ...) \
    do {\
        printf("CRITICAL: %s(%d) [%m] "fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
    }while (0)

#else

#include "nmp_debug.h"

#define show_debug nmp_print
#define show_info nmp_print
#define show_warn nmp_warning
#define show_error nmp_error
#define show_critical nmp_error

#endif



#endif //__SHOW_LOG_H__

