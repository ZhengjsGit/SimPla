//
// Created by salmon on 16-7-6.
//

#ifndef SIMPLA_SPPARALLEL_H
#define SIMPLA_SPPARALLEL_H

#include "sp_lite_def.h"

void spParallelInitialize(int argc, char **argv);

void spParallelFinalize();


#ifdef HAS_CUDA
#   include "spParallelCUDA.h"
#else
#   include "spParallelCPU.h"
#endif


#define MPI_ERROR(_CMD_)                                                   \
{                                                                          \
    int _mpi_error_code_ = _CMD_;                                          \
    if (_mpi_error_code_ != MPI_SUCCESS)                                   \
    {                                                                      \
        char _error_msg[MPI_MAX_ERROR_STRING];                             \
        MPI_Error_string(_mpi_error_code_, _error_msg, NULL);           \
        ERROR(_error_msg);                                                 \
    }                                                                      \
}


int spMPIDataTypeCreate(int type_tag, int type_size_in_byte, MPI_Datatype *new_type);

int spMPINeighborAllToAllCart(const void *send_buffer,
                              const int *send_counts,
                              const MPI_Aint *send_displs,
                              MPI_Datatype const *send_types,
                              void *recv_buffer,
                              const int *recv_counts,
                              const MPI_Aint *recv_displs,
                              MPI_Datatype const *recv_types,
                              MPI_Comm comm);

int spMPIUpdateNdArrayHalo(void *buffer,
                           int ndims,
                           const size_type *dims,
                           const size_type *start,
                           const size_type *,
                           const size_type *count,
                           const size_type *,
                           MPI_Datatype ele_type,
                           MPI_Comm comm);

#endif //SIMPLA_SPPARALLEL_H
