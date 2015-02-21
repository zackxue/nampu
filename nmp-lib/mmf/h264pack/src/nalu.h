#ifndef __nalu_h__
#define __nalu_h__

#include "h264pack.h"
#include "media_structs.h"

int nalu_find (node_t *nalus, int *count, char *bitst, int st_size);
int nalu_get_sps_pps(char *bitst, int st_size, pic_parm_t *parm);

#endif //__nalu_h__
