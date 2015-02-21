#ifndef __FRAME_DEFINITION_FOR_JXJ_PLATFORM_H__
#define __FRAME_DEFINITION_FOR_JXJ_PLATFORM_H__

#include <stdint.h>

/* 通用数据帧定义 */

typedef enum                    /* 数据帧类型 */
{
    F_TYPE_NULL = 0,
    F_TYPE_PICTURE,
    F_TYPE_AUDIO,
    F_TYPE_VIDEO_I_FRAME,
    F_TYPE_VIDEO_P_FRAME,
    F_TYPE_VIDEO_B_FRAME
}frame_type_t;


typedef enum                    /* 设备类型 */
{
    F_DEV_HIK
}frame_dev_t;


typedef struct _standard_frame_t
{
    uint32_t    magic;          /* 0x6a786a2d */	
    uint8_t     frame_type;     /* 数据帧类型 */
    uint8_t     frame_ext;      /* 附加数据类型，0为无附加数据 */
    uint8_t     padding[2];     /* 填充 */
    uint32_t    frame_dev;      /* 产生数据的设备类型 */
    uint32_t    frame_size;     /* 数据总长度，含头 */
    uint32_t    time_stamp;     /* 时间戳 */
    uint8_t     data[0];        /* 数据，或附加数据起始 */
}standard_frame_t;


typedef struct _frame_ext_t frame_ext_t;
struct _frame_ext_t
{
    uint32_t        ext_size;       /* 附加数据长度 */
    uint32_t        data[0];        /* 附加数据，4字节对齐 */
};


/* RTP封包规则:
        (1) PT:99 Encoding-Name:jpf-generic, Clock-Rate:90000. 例如：m段，a=rtpmap:99 jframe-generic/90000
        (2) 使用frame封装数据时，不使用rtp扩展，单track.
        (3) 若解码需要辅助数据，在media的属性部分指定priv属性，decode-header字段，例如: m段，a=priv:99 decode-header=AAAAAAAAAAABAAAAQgAAAFAW/ggDAAAAAAAAAAAAAAAATv4IQgAAAA==
        (4) 格式：|RTP-Header|EL|Frame-Header|Ext|Payload|

            struct EL                   //@{Encapsulation Layer for network transmission}
            {
                uint8_t magic;          //数据检测的幻数：0xAA
                uint8_t frame_no;       //同一帧拆开的片具有相同的frame_no
                uint8_t total_frags;    //总共分成了几片
                uint8_t frag_no;        //当前是第几片,从1开始
            };

*/


#endif  /* __FRAME_DEFINITION_FOR_JXJ_PLATFORM_H__ */
