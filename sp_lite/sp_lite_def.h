/**
 * sp_def.h
 *
 *  Created on: 2016年6月15日
 *      Author: salmon
 */

#ifndef SP_DEF_LITE_H_
#define SP_DEF_LITE_H_

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include "../src/sp_cwrap.h"

#define  AUTHOR " YU Zhi <yuzhi@ipp.ac.cn> "
#define  COPYRIGHT "All rights reserved. (2016 )"

typedef int8_t byte_type; // int8_t
typedef float Real;
typedef int Integral;
typedef int64_t id_type;
typedef int64_t index_type;
typedef uint64_t size_type;

#define SP_SUCCESS 0
#define SP_FAILED  1

#ifndef __CUDACC__
typedef struct { int x, y, z; } int3;
typedef struct { int x, y, z, w; } int4;
typedef struct { float x, y, z; } float3;
typedef struct { float x, y, z, w; } float4;
typedef struct { size_t x, y, z; } dim3;
#else  //__CUDACC__

#ifndef NUMBER_OF_THREADS_PER_BLOCK
#	define NUMBER_OF_THREADS_PER_BLOCK 128
#endif //NUMBER_OF_THREADS_PER_BLOCK

#if !defined(__CUDA_ARCH__)
#define MC_HOST_DEVICE
#define MC_HOST
#define MC_DEVICE
#define MC_SHARED
#define MC_CONSTANT static const
#define MC_GLOBAL
#else
#define MC_HOST_DEVICE __host__ __device__
#define MC_HOST __host__
#define MC_DEVICE  __device__
#define MC_SHARED __shared__
#define MC_CONSTANT __constant__
#define MC_GLOBAL  __global__

#endif

#	define CUDA_CHECK_RETURN(_CMD_) {											\
    cudaError_t _m_cudaStat = _CMD_;										\
    if (_m_cudaStat != cudaSuccess) {										\
        fprintf(stderr, "Error [code=0x%x] %s at line %d in file %s\n",					\
                _m_cudaStat,cudaGetErrorString(_m_cudaStat), __LINE__, __FILE__);		\
        exit(1);															\
    } }

//#define CUDA_CHECK_RETURN(_CMD_) _CMD_;

#if !defined(__CUDA_ARCH__)
#define CUDA_CHECK(_CMD_)  											\
         printf(  "[line %d in file %s]\n %s = %d \n",					\
                 __LINE__, __FILE__,__STRING(_CMD_),(_CMD_));
#else
#	define CUDA_CHECK(_CMD_) printf(  "[line %d in file %s : block=[%i,%i,%i] thread=[%i,%i,%i] ]\t %s = %x\n",					\
         __LINE__, __FILE__,blockIdx.x, blockIdx.y, blockIdx.z, threadIdx.x , threadIdx.y, threadIdx.z, __STRING(_CMD_),(_CMD_));
#endif
#define DONE	 	printf( "====== DONE ======\n" );
#define CHECK	 	printf( "[ line %d in file%s]====== CHECK ======\n", __LINE__, __FILE__ );

MC_HOST_DEVICE inline int sp_is_device_ptr(void const *p)
{
    cudaPointerAttributes attribute;
    CUDA_CHECK(cudaPointerGetAttributes(&attribute, p));
    return (attribute.device == cudaMemoryTypeDevice);

}
MC_HOST_DEVICE inline int sp_pointer_type(void const *p)
{
    cudaPointerAttributes attribute;
    CUDA_CHECK(cudaPointerGetAttributes(&attribute, p));
    return (attribute.device);

}


#endif //__CUDACC__
#endif /* SP_DEF_LITE_H_ */
