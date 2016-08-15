//
// Created by salmon on 16-7-28.
//
#include <stdlib.h>
#include <string.h>
#include "../../spParallel.h"
#include "spParallelCPU.h"

#ifdef _OPENMP
#include <omp.h>
#endif

int spParallelDeviceInitialize(int argc, char **argv) { return SP_SUCCESS; }

int spParallelDeviceFinalize() { return SP_SUCCESS; }

int spParallelDefaultNumOfThreads()
{

#ifdef _OPENMP
    return omp_get_num_procs();
#else
    return 1;
#endif
};

int spParallelDefaultNumOfBlocks() { return 1; };

int spParallelDeviceAlloc(void **p, size_type s)
{
    *p = malloc(s);
    return SP_SUCCESS;
}

int spParallelDeviceFree(void **p)
{
    if (*p != NULL)
    {
        free(*p);
        *p = NULL;
    }
    return SP_SUCCESS;
};

int spParallelMemcpy(void *dest, void const *src, size_type s)
{
    memcpy(dest, src, s);
    return SP_SUCCESS;
}

int spParallelMemcpyToCache(const void *dest, void const *src, size_type s)
{
    memcpy((void *) dest, src, s);
    return SP_SUCCESS;
}

int spParallelMemset(void *dest, int v, size_type s)
{
    memset(dest, v, s);
    return SP_SUCCESS;
}

int spParallelDeviceSync()
{
    SP_CALL(spParallelGlobalBarrier());
//    SP_CUDA_CALL(cudaDeviceSynchronize());
    return SP_SUCCESS;
}

int spParallelHostAlloc(void **p, size_type s)
{
    *p = malloc(s);
    return SP_SUCCESS;
};

int spParallelHostFree(void **p)
{
    if (*p != NULL)
    {
        free(*p);
        *p = NULL;
    }
    return SP_SUCCESS;
}

int spParallelDeviceFillInt(int *d, int v, size_type s)
{
//    SP_DEVICE_CALL_KERNEL(spParallelDeviceFillIntKernel, 16, 256, d, v, s);

    return SP_SUCCESS;
};


int spParallelDeviceFillReal(Real *d, Real v, size_type s)
{
//    SP_DEVICE_CALL_KERNEL(spParallelDeviceFillRealKernel, 16, 256, d, v, s);
    return SP_SUCCESS;
};


int spParallelAssign(size_type num_of_point, size_type *offset, Real *d, Real const *v)
{
//    SP_DEVICE_CALL_KERNEL(spParallelAssignKernel, 16, 256, num_of_point, offset, d, v);
    return SP_SUCCESS;
};


