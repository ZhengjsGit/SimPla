//
// Created by salmon on 16-7-20.
//

#ifndef SIMPLA_SPPARALLELCPU_H
#define SIMPLA_SPPARALLELCPU_H

#include <stdlib.h>
#include <string.h>
#include "../../sp_lite_def.h"

typedef struct { float x, y, z; } float3;

typedef struct { size_type x, y, z; } dim3;

#define INLINE  static inline
#define __device__
#define __host__
#define __constant__
#define __shared__




#define SP_DEVICE_DECLARE_KERNEL(_FUN_, ...)   void _FUN_ (dim3 gridDim,dim3 blockDim,dim3 blockIdx, dim3 threadIdx,__VA_ARGS__)

//_Pragma("omp parallel for shared(blockIdx)")
#define OMP_PARALLEL_INTERNAL _Pragma("omp parallel for default(none)")
#define SP_DEVICE_CALL_KERNEL(_FUN_, gridDim, blockDim, ...)         \
{                                                                    \
  dim3   blockIdx;                                                   \
  for (blockIdx.x = 0; blockIdx.x < gridDim.x; ++blockIdx.x)         \
  for (blockIdx.y = 0; blockIdx.y < gridDim.y; ++blockIdx.y)         \
  for (blockIdx.z = 0; blockIdx.z < gridDim.z; ++blockIdx.z)         \
  for (int j = 0; j < blockDim.y; ++j)                               \
  for (int k = 0; k < blockDim.z; ++k)                               \
  {                                                                  \
      \
    for(int i=0; i < blockDim.x; ++i)                                \
    {  dim3 threadIdx = {i,j,k};                                     \
       _FUN_(gridDim, blockDim, blockIdx, threadIdx, __VA_ARGS__);   \
    }                                                                \
  }                                                                  \
}

#define SP_DEVICE_CALL(_CMD_)  SP_CALL(_CMD_)


#endif //SIMPLA_SPPARALLELCPU_H