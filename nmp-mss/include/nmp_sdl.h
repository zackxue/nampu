/**
 * This file describe the data format that used to store stream
 * data.
 * SDL: Storage Data Layer.
*/
 

#ifndef __NMP_SDL_H__
#define __NMP_SDL_H__

#include <arpa/inet.h>
#include <glib.h>


#define SDL_MAGIC_NUM               0x21436587

#define SDL_SET_VALUE(sdl, mem, val) \
	((NmpSdl*)(sdl))->mem = htonl(val)
#define SDL_GET_VALUE(sdl, mem) \
	ntohl(((NmpSdl*)(sdl))->mem)

#define SDL_SET_MAGIC(sdl, magic_number) \
	SDL_SET_VALUE(sdl, magic, magic_number)
#define SDL_SET_VERSION(sdl, version) \
	SDL_SET_VALUE(sdl, ver, version)
#define SDL_SET_LEN(sdl, length) \
	SDL_SET_VALUE(sdl, len, length)

#define SDL_HEADER_INIT(sdl) \
do {\
	SDL_SET_MAGIC((sdl), SDL_MAGIC_NUM); \
	SDL_SET_VERSION((sdl), 0x01); \
} while (0)


typedef struct __NmpSdl NmpSdl;
struct __NmpSdl
{
	guint32     magic;
	guint32     ver;			/* SDL version */
	guint32     len;		    /* Total length */
};


#define SDL_V1_TAGS_IFRAME          0x01
#define SDL_V1_TAGS_AUDIO			0x02

#define SDL_V1_EXT_TYPE_SPS_PPS     0x01

#define SDL_V1_SET_VALUE(sdl, mem, val) \
	((NmpSdlV1*)sdl)->mem = htonl(val)
#define SDL_V1_GET_VALUE(sdl, mem) \
	ntohl(((NmpSdlV1*)sdl)->mem)

#define SDL_V1_GET_DATA(sdl) \
	((gchar*)(((NmpSdlV1*)(sdl))->data))

#define SDL_V1_SET_DEV(sdl, vdev) \
	SDL_V1_SET_VALUE(sdl, dev, vdev)
#define SDL_V1_SET_TAGS(sdl, vtags) \
	SDL_V1_SET_VALUE(sdl, tags, vtags)
#define SDL_V1_SET_EXT(sdl, ext) \
	SDL_V1_SET_VALUE(sdl, ext_type, ext)
#define SDL_V1_SET_EXTLEN(sdl, len) \
	SDL_V1_SET_VALUE(sdl, ext_len, len)
#define SDL_V1_SET_TS1(sdl, ts1) \
	SDL_V1_SET_VALUE(sdl, ts_1, ts1)
#define SDL_V1_SET_TS2(sds, ts2) \
	SDL_V1_SET_VALUE(sdl, ts_2, ts2)

#define SDL_V1_TAGS_IFRAME			0x01
#define SDL_V1_EXTS_SPS				0x01

typedef struct __NmpSdlV1 NmpSdlV1;			/* Version 1 */
struct __NmpSdlV1
{
	NmpSdl      header;
	guint32     dev;			/* stream type */
	guint32     tags;			/* Data tags: iframe etc. */
	guint32     ext_type;		/* 0 means no ext-data */
	guint32     ext_len;
	guint32     ts_1;			/* Timestamp 1 */
	guint32     ts_2;			/* Timestamp 2 */
	guint32     data[0];		/* Ext data or payload */
};


typedef struct __NmpSdlExtV1 NmpSdlExtV1;
struct __NmpSdlExtV1
{
	guint32		len1;			/* sps */
	guint32		len2;			/* pli */
	guint32		len3;			/* pri */
	guint32     data[0];
};


#endif	/* __NMP_SDL_H__ */
