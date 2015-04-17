/*
 *          file: nmp_proxy_error.h
 *          description:定义以及描述代理服务器所有错误码信息
 *
 *          hegui,heguijiss@gmail.com
 *          May 16th, 2013
 */

#ifndef __PROXY_ERROR_H__
#define __PROXY_ERROR_H__


#define JPROXY_SERVER_ERROR_BASE        9000     /* 代理服务器错误码基准*/
#define JEBASE JPROXY_SERVER_ERROR_BASE


#define E_PRITYPE               (JEBASE + 1)     /* 私有数据类型无效 */
#define E_NO_PRIDATA            (JEBASE + 2)     /* 无私有数据 */
#define E_INIT_DEVICE           (JEBASE + 3)     /* 初始化代理设备失败 */


#define E_CONNECT_TIMEOUT       (JEBASE + 10)     /* 连接cms 超时 */
#define E_REGISTER_TIMEOUT      (JEBASE + 11)     /* 注册cms 超时 */
#define E_ONLINE_TIMEOUT        (JEBASE + 12)     /* 在线 超时 */



#endif  /* __PROXY_ERROR_H__ */

