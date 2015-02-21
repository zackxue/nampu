
/*
 * tinydump.h  
 *
 * @purpose: create a dump file to locate code when crash occrued   
 * @resource: from csdn -->>> http://blog.csdn.net/starlee/article/details/6630816
 * @date: 2013-4-22
 */

#ifndef __TINY_DUMP_H__
#define __TINY_DUMP_H__

#ifdef __cplusplus
extern "C" 
{
#endif

#define TD_STATIC_LIB

#ifdef TD_STATIC_LIB
#define TD_API
#else

#ifdef TD_EXPORTS
#define TD_API __declspec(dllexport)
#else
#define TD_API __declspec(dllimport)
#endif

#endif

TD_API void td_init(const char* dump_path, bool email_support, const char* recipient);

#ifdef __cplusplus
}
#endif

#endif