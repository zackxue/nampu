#ifndef __pack_h__
#define __pack_h__

#include "ctx.h"

int pack_snp(pack_ctx_t *ctx, node_t *nal, node_array_t **pkt, void *_uargs);

int pack_fu_a(pack_ctx_t *ctx, node_t *nal, node_array_t **pkt, void *_uargs);
int pack_fu_b(pack_ctx_t *ctx, node_t *nal, node_array_t **pkt, void *_uargs);

int pack_stap_a(pack_ctx_t *ctx, node_t *nal, node_array_t **pkt, void *_uargs);
int pack_stap_b(pack_ctx_t *ctx, node_t *nal, node_array_t **pkt, void *_uargs);

int pack_mtap(pack_ctx_t *ctx, node_t *nal, node_array_t **pkt, void *_uargs);

int pack_audio(pack_ctx_t *ctx, node_t *nal, node_array_t **pkt, void *_uargs);
int pack_eof(pack_ctx_t *ctx, node_t *nal, node_array_t **pkt, void *_uargs);


#endif //__pack_h__
