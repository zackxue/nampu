#ifndef __NMP_REC_H__
#define __NMP_REC_H__


enum
{
	REC_TYPE_AUTO = 1 << 0,			/* 定时录像类型 */
	REC_TYPE_ALRM = 1 << 4,			/* 报警录像类型 */
	REC_TYPE_MANU = 1 << 12			/* 手动录像类型 */
};


#define REC_IFRAME_TAG 0x80000000

#endif	/* __NMP_REC_H__ */
