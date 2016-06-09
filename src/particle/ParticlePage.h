//
// Created by salmon on 16-6-9.
//

#ifndef SIMPLA_PARTICLEPAGE_H
#define SIMPLA_PARTICLEPAGE_H

#ifdef __cplusplus
extern "C" {

#include "SmallObjPool.h"

#endif

struct spPage;


int spParticleCopy(size_t key, size_t size_in_byte, struct spPage const *src_page, struct spPage **dest_page,
                   struct spPage **buffer);

int spParticleCopyN(size_t key, size_t size_in_byte, size_t src_num, struct spPage **src_page,
                    struct spPage **dest_page, struct spPage **buffer);
void spParticleClear(size_t key, size_t size_in_byte, struct spPage **pg, struct spPage **buffer);


#ifdef __cplusplus
}
#endif
#endif //SIMPLA_PARTICLEPAGE_H
