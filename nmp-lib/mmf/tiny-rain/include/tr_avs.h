/*
 * RTSP/SIP服务端设备对接接口(Ver 1.0)
*/

#ifndef __TINY_RAIN_AV_SERVER_H__
#define __TINY_RAIN_AV_SERVER_H__

#include "media_structs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CTL_CMD_IFRAME          1

#define AVS_API extern
typedef struct __avs_media avs_media;
typedef void (*exp_cb)(void *u, int32_t err);       /* 启动流服务结果回调，err->0代表成功，其它值代表连接失败 */
typedef void (*log_handler)(uint8_t *message);      /* 日志信息回调 */

/* 流句柄操作接口 */
AVS_API void avs_media_set_u(avs_media *m, void *u);                        /* 设置用户参数 */
AVS_API void *avs_media_get_u(avs_media *m);                                /* 获取用户参数 */
AVS_API int32_t avs_media_fill(avs_media *m, frame_t *frm);                 /* 实时流推送 */
AVS_API void avs_media_kill(avs_media *m);                                  /* 故障通告AVS */
AVS_API avs_media *avs_media_ref(avs_media *m);                             /* 增对对句柄的引用 */
AVS_API void avs_media_unref(avs_media *m);                                 /* 减少对句柄的引用 */

/* 帧分配接口，用于历史流pull */
AVS_API frame_t *avs_alloc_frame(uint32_t data_size, uint32_t nal_count);   /* 分配一帧的空间 */
AVS_API void avs_free_frame(frame_t *frame);                                /* 释放一帧空间，当填充帧数据失败时使用 */

/* 实时流接口 */
typedef struct __ls_avs_ops ls_avs_ops;
struct __ls_avs_ops
{
    int32_t (*probe)(int32_t channel, int32_t level, media_info_t *mi);     /* 获取媒体信息,可能被反复调用 */
    int32_t (*open)(avs_media *m, int32_t channel, int32_t level);          /* 初始化 */
    int32_t (*play)(avs_media *m);                                          /* 就绪，之后可以使用avs_media_fill()发送实时流 */
    int32_t (*ctrl)(avs_media *m, uint32_t cmd, void *data);                /* 控制：cmd=1,data=NULL 强制I帧; */
    void    (*close)(avs_media *m);                                         /* 关闭 */
};

/* 历史流接口 */
typedef struct __hs_avs_ops hs_avs_ops;
struct __hs_avs_ops
{
    int32_t (*probe)(int32_t channel, int32_t level, int32_t type,       
        uint8_t *start_time, uint8_t *end_time, uint8_t *property,          /* 时间格式:YYYYMMDDHHMMSS */
        media_info_t *mi);                                                  /* 获取媒体信息,可能被反复调用 */
    int32_t (*open)(avs_media *m, int32_t channel, int32_t level,
        int32_t type, uint8_t *start_time, uint8_t *end_time, uint8_t *property); /* 初始化 */
    int32_t (*play)(avs_media *m);                                          /* SDK即将开始回调.pull(), 辅助user实现相关策略（如预读等），可为NULL */
    int32_t (*pause)(avs_media *m);                                         /* SDK即将暂停回调.pull(), 辅助user实现相关策略（如预读等），可为NULL */
    int32_t (*lseek)(avs_media *m, uint32_t ts);                            /* 定位 */
    void    (*close)(avs_media *m);                                         /* 关闭 */
    int32_t (*pull)(avs_media *m, frame_t **frm);                           /* SDK获取历史视频流HOOK */
};

AVS_API int32_t avs_init( void );                                           /* 初始化 */
AVS_API int32_t avs_get_version(uint32_t *major, uint32_t *minor);          /* 获取AVS版本号 */
AVS_API int32_t avs_register_ops(ls_avs_ops *lso, hs_avs_ops *hso);         /* 注册回调 */
AVS_API int32_t avs_set_stream_ports_range(uint16_t low, uint16_t hi);      /* 设置流端口范围，用于支持DMZ或其它路由端口映射功能，默认：[15000,20000) */
AVS_API int32_t avs_start(uint16_t port);                                   /* RTSP 服务端口，默认：7554 */
AVS_API void    avs_stop( void );                                           /* 停止服务 */

/* 平台流服务 */
AVS_API int32_t avs_start_pf_service(uint8_t *mds_ip, uint16_t port, int32_t pu_type,
    uint8_t *puid, int32_t l4_proto, int32_t ka, exp_cb exp, void *u);      /* 启动平台流服务 */
AVS_API int32_t avs_stop_pf_service( void );                                /* 停止平台流服务 */


/* 日志对接*/
AVS_API int32_t avs_log_set_handler(log_handler lf);
AVS_API int32_t avs_log_set_verbose(int8_t level);

#ifdef __cplusplus
}
#endif

#endif  /* __TINY_RAIN_AV_SERVER_H__ */
