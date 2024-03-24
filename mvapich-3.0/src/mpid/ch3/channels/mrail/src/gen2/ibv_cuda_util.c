/* Copyright (c) 2001-2024, The Ohio State University. All rights
 * reserved.
 *
 * This file is part of the MVAPICH software package developed by the
 * team members of The Ohio State University's Network-Based Computing
 * Laboratory (NBCL), headed by Professor Dhabaleswar K. (DK) Panda.
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level MVAPICH directory.
 *
 */

#include "rdma_impl.h"
#include "ibv_param.h"
#include "mvp_utils.h"
#include "ibv_cuda_util.h"

/*
=== BEGIN_MPI_T_MVP_CVAR_INFO_BLOCK ===

cvars:
    - name        : MVP_EAGER_CUDAHOST_REG
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_CUDA_VECTOR_OPT
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_CUDA_KERNEL_OPT
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_CUDA_KERNEL_VECTOR_TIDBLK_SIZE
      category    : CH3
      type        : int
      default     : 1024
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This controls the number of CUDA threads per block in pack/unpack
        kernels for MPI vector datatype in communication involving GPU device
        buffers.

    - name        : MVP_CUDA_KERNEL_VECTOR_YSIZE
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This controls the y-dimension of a thread block in pack/unpack kernels
        for MPI vector datatype in communication involving GPU device buffers.
        It controls the number of threads operating on each block of data in a
        vector.

    - name        : MVP_CUDA_KERNEL_SUBARR_TIDBLK_SIZE
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_CUDA_KERNEL_SUBARR_XDIM
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_CUDA_KERNEL_SUBARR_YDIM
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_CUDA_KERNEL_SUBARR_ZDIM
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_CUDA_NUM_EVENTS
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_CUDA_INIT_CONTEXT
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_CUDA_USE_NAIVE
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_CUDA_ALLTOALL_DYNAMIC
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_CUDA_REGISTER_NAIVE_BUF
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_CUDA_GATHER_NAIVE_LIMIT
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_CUDA_SCATTER_NAIVE_LIMIT
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_CUDA_ALLTOALL_NAIVE_LIMIT
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_CUDA_ALLTOALLV_NAIVE_LIMIT
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_CUDA_ALLGATHER_NAIVE_LIMIT
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_CUDA_ALLGATHERV_NAIVE_LIMIT
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_CUDA_BCAST_NAIVE_LIMIT
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_CUDA_GATHERV_NAIVE_LIMIT
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_CUDA_SCATTERV_NAIVE_LIMIT
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_CUDA_ALLGATHER_RD_LIMIT
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_CUDA_ALLGATHER_FGP
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_CUDA_IPC
      category    : CH3
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This enables intra-node GPU-GPU communication using IPC feature
        available from CUDA 4.1

    - name        : MVP_CUDA_IPC_SHARE_GPU
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_CUDA_SMP_IPC
      category    : CH3
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This enables an optimization for short message GPU device-to-device
        communication using IPC feature available from CUDA 4.1

    - name        : MVP_CUDA_ENABLE_IPC_CACHE
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_CUDA_IPC_MAX_CACHE_ENTRIES
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_CUDA_IPC_NUM_STAGE_BUFFERS
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_CUDA_IPC_STAGE_BUF_SIZE
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_CUDA_IPC_BUFFERED
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_CUDA_IPC_THRESHOLD
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_CUDA_IPC_BUFFERED_LIMIT
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_CUDA_IPC_SYNC_LIMIT
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_CUDA_DYNAMIC_INIT
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_CUDA_NONBLOCKING_STREAMS
      category    : CH3
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This controls the use of non-blocking streams for asynchronous CUDA
        memory copies in communication involving GPU memory.

=== END_MPI_T_MVP_CVAR_INFO_BLOCK ===
*/
#if defined(_ENABLE_CUDA_)
#define CUDA_DEBUG             0
#define INITIAL_PACKUNPACK_OPT -1
#define SUCCESS_PACKUNPACK_OPT 0
#define FAILURE_PACKUNPACK_OPT 1

extern unsigned long PVAR_COUNTER_mvp_vbuf_allocated;
extern unsigned long PVAR_COUNTER_mvp_vbuf_freed;
extern unsigned long PVAR_LEVEL_mvp_vbuf_available;
extern unsigned long PVAR_COUNTER_mvp_ud_vbuf_allocated;
extern unsigned long PVAR_COUNTER_mvp_ud_vbuf_freed;
extern unsigned long PVAR_LEVEL_mvp_ud_vbuf_available;

static int can_use_cuda = 0;
int cudaipc_init = 0;
static int cudaipc_init_global = 0;
static CUcontext mvp_cuda_context = NULL;
static CUcontext mvp_save_cuda_context = NULL;

void MVP_MPII_IOV_pack_cuda(void *buf, struct iovec *iov, int n_iov,
                            int position, cudaStream_t stream)
{
    int i;
    void *ptr;
    ptr = (char *)buf + position;
    for (i = 0; i < n_iov; i++) {
        MVP_MPIDI_Memcpy_Device_Async(ptr, iov[i].iov_base, iov[i].iov_len,
                                      cudaMemcpyDefault, stream);
        ptr = (char *)ptr + iov[i].iov_len;
    }
    PRINT_DEBUG(CUDA_DEBUG, "CUDA pack: buf:%p n_iov: %d\n", buf, n_iov);
}

void MVP_MPII_IOV_unpack_cuda(void *buf, struct iovec *iov, int n_iov,
                              int position, int *bytes_unpacked,
                              cudaStream_t stream)
{
    int i = 0;
    void *ptr = buf + position;
    int total_len = 0;

    for (i = 0; i < n_iov; i++) {
        MVP_MPIDI_Memcpy_Device_Async(iov[i].iov_base, ptr, iov[i].iov_len,
                                      cudaMemcpyDefault, stream);
        ptr = (char *)ptr + iov[i].iov_len;
        total_len += iov[i].iov_len;
    }
    PRINT_DEBUG(CUDA_DEBUG, "CUDA unpack: buf:%p n_iov: %d total_len:%d \n",
                buf, n_iov, total_len);
    *bytes_unpacked = total_len;
}

void vector_pack_cudabuf(void *buf, struct iovec *iov, int size,
                         cudaStream_t stream)
{
    cudaError_t cerr = cudaSuccess;
    cerr = cudaMemcpy2DAsync(
        buf, iov[0].iov_len, iov[0].iov_base,
        (size_t)(iov[1].iov_base) - (size_t)(iov[0].iov_base), iov[0].iov_len,
        size / iov[0].iov_len, cudaMemcpyDeviceToDevice, stream);
    if (cerr != cudaSuccess) {
        PRINT_INFO(1, "Error in cudaMemcpy2D\n");
    }
    PRINT_DEBUG(CUDA_DEBUG, "cuda vector pack with cudaMemcpy2D\n");
}

void vector_unpack_cudabuf(void *buf, struct iovec *iov, int size,
                           cudaStream_t stream)
{
    cudaError_t cerr = cudaSuccess;
    cerr = cudaMemcpy2DAsync(
        iov[0].iov_base, (size_t)(iov[1].iov_base) - (size_t)(iov[0].iov_base),
        buf, iov[0].iov_len, iov[0].iov_len, size / iov[0].iov_len,
        cudaMemcpyDeviceToDevice, stream);
    if (cerr != cudaSuccess) {
        PRINT_INFO(1, "Error in cudaMemcpy2D\n");
    }
    PRINT_DEBUG(CUDA_DEBUG, "cuda vector unpack with cudaMemcpy2D\n");
}

#if defined(USE_GPU_KERNEL)
int hindexed_pack_cudabuf(void *dst, struct iovec *iov, MPIR_Datatype *dtp,
                          int size, cudaStream_t stream)
{
    int i, builtin_element_size;
    int struct_sz = sizeof(MPIR_Datatype_contents);
    int types_sz = dtp->contents->nr_types * sizeof(MPI_Datatype);
    int ints_sz = dtp->contents->nr_ints * sizeof(int);
    int align_sz = 8, epsilon;

    int subarray_struct_sz = sizeof(MPIR_Datatype_contents);
    int subarray_types_sz;
    int *subarray_array_of_ints;

    int array_numb = dtp->contents->nr_aints;

    MPIR_Datatype *old_dtp, *sub_dtp;
    MPI_Datatype *array_of_types, *sub_array_of_types;
    MPI_Aint *array_of_aints;
    MPI_Aint base_displs, array_displs[array_numb];

    MPI_Address(iov[0].iov_base, &base_displs);

    if ((epsilon = struct_sz % align_sz)) {
        struct_sz += align_sz - epsilon;
    }

    if ((epsilon = types_sz % align_sz)) {
        types_sz += align_sz - epsilon;
    }

    if ((epsilon = ints_sz % align_sz)) {
        ints_sz += align_sz - epsilon;
    }

    array_of_aints =
        (MPI_Aint *)((char *)dtp->contents + struct_sz + types_sz + ints_sz);
    array_of_types = (MPI_Datatype *)((char *)dtp->contents + struct_sz);

    for (i = 0; i < dtp->contents->nr_types; i++) {
        if (HANDLE_GET_KIND(array_of_types[i]) != HANDLE_KIND_BUILTIN) {
            MPIR_Datatype_get_ptr(array_of_types[i], old_dtp);
            if (old_dtp->contents->combiner != MPI_COMBINER_SUBARRAY ||
                old_dtp->contents->nr_ints != 11) {
                /* only handle subarray type and nr_ints must be 11 for 3
                 * ndims*/
                break;
            } else {
                continue;
            }
        } else {
            return FAILURE_PACKUNPACK_OPT;
        }
    }

    if (i != dtp->contents->nr_types) {
        return FAILURE_PACKUNPACK_OPT;
    }

    void *src;

    if (HANDLE_GET_KIND(array_of_types[0]) != HANDLE_KIND_BUILTIN) {
        MPIR_Datatype_get_ptr(array_of_types[0], old_dtp);

        if (old_dtp->contents->combiner == MPI_COMBINER_SUBARRAY) {
            subarray_types_sz =
                old_dtp->contents->nr_types * sizeof(MPI_Datatype);
            if ((epsilon = subarray_struct_sz % align_sz)) {
                subarray_struct_sz += align_sz - epsilon;
            }
            if ((epsilon = subarray_types_sz % align_sz)) {
                subarray_types_sz += align_sz - epsilon;
            }

            sub_array_of_types = (MPI_Datatype *)((char *)old_dtp->contents +
                                                  subarray_struct_sz);
            builtin_element_size = old_dtp->builtin_element_size;
            if (HANDLE_GET_KIND(sub_array_of_types[0]) != HANDLE_KIND_BUILTIN) {
                int cont_types_sz;
                MPI_Datatype *cont_array_of_types;

                MPIR_Datatype_get_ptr(sub_array_of_types[0], sub_dtp);

                if (sub_dtp->contents->combiner == MPI_COMBINER_CONTIGUOUS) {
                    cont_types_sz =
                        sub_dtp->contents->nr_types * sizeof(MPI_Datatype);
                    if ((epsilon = cont_types_sz % align_sz)) {
                        cont_types_sz += align_sz - epsilon;
                    }

                    cont_array_of_types =
                        (MPI_Datatype *)((char *)sub_dtp->contents +
                                         subarray_struct_sz);

                    if (HANDLE_GET_KIND(cont_array_of_types[0]) ==
                        HANDLE_KIND_BUILTIN) {
                        builtin_element_size = sub_dtp->n_builtin_elements *
                                               MPIR_Datatype_get_basic_size(
                                                   cont_array_of_types[0]);
                    } else {
                        return FAILURE_PACKUNPACK_OPT;
                    }
                } else {
                    return FAILURE_PACKUNPACK_OPT;
                }
            }

            subarray_array_of_ints =
                (int *)((char *)old_dtp->contents + subarray_struct_sz +
                        subarray_types_sz);

            int ndims = subarray_array_of_ints[0];
            int order = subarray_array_of_ints[10];

            if (ndims == 3 && order == MPI_ORDER_FORTRAN) {
                int array_of_sizes[ndims];
                int array_of_subsizes[ndims];
                int array_of_starts[ndims];

                array_of_sizes[0] = subarray_array_of_ints[1];
                array_of_sizes[1] = subarray_array_of_ints[2];
                array_of_sizes[2] = subarray_array_of_ints[3];
                array_of_subsizes[0] = subarray_array_of_ints[4];
                array_of_subsizes[1] = subarray_array_of_ints[5];
                array_of_subsizes[2] = subarray_array_of_ints[6];
                array_of_starts[0] = subarray_array_of_ints[7];
                array_of_starts[1] = subarray_array_of_ints[8];
                array_of_starts[2] = subarray_array_of_ints[9];

                base_displs -= (array_of_starts[0] +
                                array_of_sizes[0] * array_of_starts[1] +
                                array_of_sizes[0] * array_of_sizes[1] *
                                    array_of_starts[2]) *
                               builtin_element_size;

                for (i = 0; i < array_numb; i++) {
                    array_displs[i] = base_displs + array_of_aints[i];
                    src = (void *)array_displs[i];

                    if (builtin_element_size == 1 ||
                        builtin_element_size == 4 ||
                        builtin_element_size == 8) {
                        pack_subarray(dst, src, ndims, array_of_sizes[0],
                                      array_of_sizes[1], array_of_sizes[2],
                                      array_of_subsizes[0],
                                      array_of_subsizes[1],
                                      array_of_subsizes[2], array_of_starts[0],
                                      array_of_starts[1], array_of_starts[2],
                                      order, builtin_element_size, stream);
                        dst += array_of_subsizes[0] * array_of_subsizes[1] *
                               array_of_subsizes[2] * builtin_element_size;
                    } else {
                        return FAILURE_PACKUNPACK_OPT;
                    }
                }
            } else {
                return FAILURE_PACKUNPACK_OPT;
            }

        } else {
            return FAILURE_PACKUNPACK_OPT;
        }
    } else {
        return FAILURE_PACKUNPACK_OPT;
    }
    return SUCCESS_PACKUNPACK_OPT;
}

int hindexed_unpack_cudabuf(void *src, struct iovec *iov, MPIR_Datatype *dtp,
                            int size, cudaStream_t stream)
{
    int i, builtin_element_size;
    int struct_sz = sizeof(MPIR_Datatype_contents);
    int types_sz = dtp->contents->nr_types * sizeof(MPI_Datatype);
    int ints_sz = dtp->contents->nr_ints * sizeof(int);
    int align_sz = 8, epsilon;

    int subarray_struct_sz = sizeof(MPIR_Datatype_contents);
    int subarray_types_sz;
    int *subarray_array_of_ints;

    int array_numb = dtp->contents->nr_aints;

    MPIR_Datatype *old_dtp, *sub_dtp;
    MPI_Datatype *array_of_types, *sub_array_of_types;
    MPI_Aint *array_of_aints;
    MPI_Aint base_displs, array_displs[array_numb];

    MPI_Address(iov[0].iov_base, &base_displs);

    if ((epsilon = struct_sz % align_sz)) {
        struct_sz += align_sz - epsilon;
    }

    if ((epsilon = types_sz % align_sz)) {
        types_sz += align_sz - epsilon;
    }

    if ((epsilon = ints_sz % align_sz)) {
        ints_sz += align_sz - epsilon;
    }

    array_of_aints =
        (MPI_Aint *)((char *)dtp->contents + struct_sz + types_sz + ints_sz);
    array_of_types = (MPI_Datatype *)((char *)dtp->contents + struct_sz);

    for (i = 0; i < dtp->contents->nr_types; i++) {
        if (HANDLE_GET_KIND(array_of_types[i]) != HANDLE_KIND_BUILTIN) {
            MPIR_Datatype_get_ptr(array_of_types[i], old_dtp);
            if (old_dtp->contents->combiner != MPI_COMBINER_SUBARRAY ||
                old_dtp->contents->nr_ints != 11) {
                /* only handle subarray type and nr_ints must be 11 for 3
                 * ndims*/
                break;
            } else {
                continue;
            }
        } else {
            return FAILURE_PACKUNPACK_OPT;
        }
    }

    if (i != dtp->contents->nr_types) {
        return FAILURE_PACKUNPACK_OPT;
    }

    void *dst;

    if (HANDLE_GET_KIND(array_of_types[0]) != HANDLE_KIND_BUILTIN) {
        MPIR_Datatype_get_ptr(array_of_types[0], old_dtp);

        if (old_dtp->contents->combiner == MPI_COMBINER_SUBARRAY) {
            subarray_types_sz =
                old_dtp->contents->nr_types * sizeof(MPI_Datatype);
            if ((epsilon = subarray_struct_sz % align_sz)) {
                subarray_struct_sz += align_sz - epsilon;
            }
            if ((epsilon = subarray_types_sz % align_sz)) {
                subarray_types_sz += align_sz - epsilon;
            }

            sub_array_of_types = (MPI_Datatype *)((char *)old_dtp->contents +
                                                  subarray_struct_sz);
            builtin_element_size = old_dtp->builtin_element_size;
            if (HANDLE_GET_KIND(sub_array_of_types[0]) != HANDLE_KIND_BUILTIN) {
                int cont_types_sz;
                MPI_Datatype *cont_array_of_types;

                MPIR_Datatype_get_ptr(sub_array_of_types[0], sub_dtp);

                if (sub_dtp->contents->combiner == MPI_COMBINER_CONTIGUOUS) {
                    cont_types_sz =
                        sub_dtp->contents->nr_types * sizeof(MPI_Datatype);
                    if ((epsilon = cont_types_sz % align_sz)) {
                        cont_types_sz += align_sz - epsilon;
                    }

                    cont_array_of_types =
                        (MPI_Datatype *)((char *)sub_dtp->contents +
                                         subarray_struct_sz);

                    if (HANDLE_GET_KIND(cont_array_of_types[0]) ==
                        HANDLE_KIND_BUILTIN) {
                        builtin_element_size = sub_dtp->n_builtin_elements *
                                               MPIR_Datatype_get_basic_size(
                                                   cont_array_of_types[0]);
                    } else {
                        return FAILURE_PACKUNPACK_OPT;
                    }
                } else {
                    return FAILURE_PACKUNPACK_OPT;
                }
            }

            subarray_array_of_ints =
                (int *)((char *)old_dtp->contents + subarray_struct_sz +
                        subarray_types_sz);

            int ndims = subarray_array_of_ints[0];
            int order = subarray_array_of_ints[10];

            if (ndims == 3 && order == MPI_ORDER_FORTRAN) {
                int array_of_sizes[ndims];
                int array_of_subsizes[ndims];
                int array_of_starts[ndims];

                array_of_sizes[0] = subarray_array_of_ints[1];
                array_of_sizes[1] = subarray_array_of_ints[2];
                array_of_sizes[2] = subarray_array_of_ints[3];
                array_of_subsizes[0] = subarray_array_of_ints[4];
                array_of_subsizes[1] = subarray_array_of_ints[5];
                array_of_subsizes[2] = subarray_array_of_ints[6];
                array_of_starts[0] = subarray_array_of_ints[7];
                array_of_starts[1] = subarray_array_of_ints[8];
                array_of_starts[2] = subarray_array_of_ints[9];

                base_displs -= (array_of_starts[0] +
                                array_of_sizes[0] * array_of_starts[1] +
                                array_of_sizes[0] * array_of_sizes[1] *
                                    array_of_starts[2]) *
                               builtin_element_size;

                for (i = 0; i < array_numb; i++) {
                    array_displs[i] = base_displs + array_of_aints[i];
                    dst = (void *)array_displs[i];

                    if (builtin_element_size == 1 ||
                        builtin_element_size == 4 ||
                        builtin_element_size == 8) {
                        unpack_subarray(
                            dst, src, ndims, array_of_sizes[0],
                            array_of_sizes[1], array_of_sizes[2],
                            array_of_subsizes[0], array_of_subsizes[1],
                            array_of_subsizes[2], array_of_starts[0],
                            array_of_starts[1], array_of_starts[2], order,
                            builtin_element_size, stream);
                        src += array_of_subsizes[0] * array_of_subsizes[1] *
                               array_of_subsizes[2] * builtin_element_size;
                    } else {
                        return FAILURE_PACKUNPACK_OPT;
                    }
                }

            } else {
                return FAILURE_PACKUNPACK_OPT;
            }
        } else {
            return FAILURE_PACKUNPACK_OPT;
        }
    } else {
        return FAILURE_PACKUNPACK_OPT;
    }
    return SUCCESS_PACKUNPACK_OPT;
}
#endif

int MPIDI_CH3_ReqHandler_pack_device(MPIDI_VC_t *vc, MPIR_Request *rreq,
                                     int *complete)
{
    return MPIDI_CH3_ReqHandler_pack_device_stream(vc, rreq, complete, NULL);
}

int MPIDI_CH3_ReqHandler_pack_device_stream(MPIDI_VC_t *vc ATTRIBUTE((unused)),
                                            MPIR_Request *req,
                                            int *complete ATTRIBUTE((unused)),
                                            void *stream)
{
    MPI_Aint last;
    int iov_n;
    struct iovec iov[MPL_IOV_LIMIT] = {0};
    cudaStream_t stream_passed = 0;

    if (stream != NULL) {
        stream_passed = (cudaStream_t)stream;
    }

    req->dev.segment_first = 0;
    do {
        last = req->dev.segment_size;
        iov_n = MPL_IOV_LIMIT;
        MPIR_Assert(req->dev.segment_first < last);
        MPIR_Assert(last > 0);
        MPIR_Assert(iov_n > 0 && iov_n <= MPL_IOV_LIMIT);

        MPID_Segment_pack_vector(req->dev.segment_ptr, req->dev.segment_first,
                                 &last, iov, &iov_n);
        if (req->dev.datatype_ptr->contents->combiner == MPI_COMBINER_VECTOR &&
            (req->dev.segment_ptr->builtin_loop.loop_params.count == 1) &&
            (rdma_cuda_vector_dt_opt || rdma_cuda_kernel_dt_opt)) {
#if defined(USE_GPU_KERNEL)
            if (rdma_cuda_kernel_dt_opt) {
                pack_unpack_vector_kernel(
                    req->dev.tmpbuf, iov[0].iov_len, iov[0].iov_base,
                    iov[1].iov_base - iov[0].iov_base, iov[0].iov_len,
                    req->dev.segment_size / iov[0].iov_len, stream_passed);
            } else
#endif
            {
                vector_pack_cudabuf(req->dev.tmpbuf, iov, req->dev.segment_size,
                                    stream_passed);
            }
            last = req->dev.segment_size;
        } else if (req->dev.datatype_ptr->contents->combiner ==
                       MPI_COMBINER_SUBARRAY &&
                   (req->dev.segment_ptr->builtin_loop.loop_params.count ==
                    1) &&
                   rdma_cuda_kernel_dt_opt) {
#if defined(USE_GPU_KERNEL)
            MPIR_Datatype *dtptr = req->dev.datatype_ptr;
            int struct_size = sizeof(MPIR_Datatype_contents);
            int types_size = dtptr->contents->nr_types * sizeof(MPI_Datatype);
            int *arr_ints =
                (int *)((char *)dtptr->contents + struct_size + types_size);
            MPI_Aint base_addr;
            int subarr_dims = arr_ints[1];

            if (subarr_dims < 4) {
                int subarr_order = arr_ints[2 + 3 * subarr_dims];
                int arr_of_bigsizes[3] = {1, 1, 1};
                int arr_of_subsizes[3] = {1, 1, 1};
                int arr_of_offsets[3] = {0, 0, 0};
                int idx;
                int elem_sz = dtptr->builtin_element_size;
                MPI_Address(iov[0].iov_base, &base_addr);

                for (idx = 0; idx < subarr_dims; idx++) {
                    arr_of_bigsizes[idx] = arr_ints[2 + idx];
                    arr_of_subsizes[idx] = arr_ints[2 + 1 * subarr_dims + idx];
                    arr_of_offsets[idx] = arr_ints[2 + 2 * subarr_dims + idx];
                }

                if (MPI_ORDER_C == subarr_order) {
                    base_addr -= (arr_of_offsets[0] * arr_of_bigsizes[1] *
                                      arr_of_bigsizes[2] +
                                  arr_of_offsets[1] * arr_of_bigsizes[2] +
                                  arr_of_offsets[2]) *
                                 elem_sz;
                } else if (MPI_ORDER_FORTRAN == subarr_order) {
                    base_addr -= (arr_of_offsets[0] +
                                  arr_of_offsets[1] * arr_of_bigsizes[0] +
                                  arr_of_offsets[2] * arr_of_bigsizes[0] *
                                      arr_of_bigsizes[1]) *
                                 elem_sz;
                }

                void *src = (void *)base_addr;

                pack_subarray(
                    req->dev.tmpbuf, src, subarr_dims, arr_of_bigsizes[0],
                    arr_of_bigsizes[1], arr_of_bigsizes[2], arr_of_subsizes[0],
                    arr_of_subsizes[1], arr_of_subsizes[2], arr_of_offsets[0],
                    arr_of_offsets[1], arr_of_offsets[2], subarr_order, elem_sz,
                    stream_passed);
                last = req->dev.segment_size;
            } else {
                MVP_MPII_IOV_pack_cuda(req->dev.tmpbuf, iov, iov_n,
                                       req->dev.segment_first, stream_passed);
            }
#endif

#if defined(USE_GPU_KERNEL)
        } else if (req->dev.datatype_ptr->contents->combiner ==
                       MPI_COMBINER_HINDEXED &&
                   rdma_cuda_kernel_dt_opt) {
            int return_hindexed_pack = INITIAL_PACKUNPACK_OPT;
            return_hindexed_pack = hindexed_pack_cudabuf(
                (void *)req->dev.tmpbuf, iov, req->dev.datatype_ptr,
                req->dev.segment_size, stream_passed);

            if (return_hindexed_pack == SUCCESS_PACKUNPACK_OPT) {
                last = req->dev.segment_size;
            } else if (return_hindexed_pack == FAILURE_PACKUNPACK_OPT) {
                MVP_MPII_IOV_pack_cuda(req->dev.tmpbuf, iov, iov_n,
                                       req->dev.segment_first, stream_passed);
            } else {
                MVP_MPII_IOV_pack_cuda(req->dev.tmpbuf, iov, iov_n,
                                       req->dev.segment_first, stream_passed);
            }
#endif
        } else {
            MVP_MPII_IOV_pack_cuda(req->dev.tmpbuf, iov, iov_n,
                                   req->dev.segment_first, stream_passed);
        }

        req->dev.segment_first = last;
        PRINT_INFO(CUDA_DEBUG, "paked :%d start:%lu last:%lu\n", iov_n,
                   req->dev.segment_first, last);
    } while (last != req->dev.segment_size);
    /* Synchronize on the stream to make sure pack is complete*/
    cudaStreamSynchronize(stream_passed);

    return MPI_SUCCESS;
}

int MPIDI_CH3_ReqHandler_unpack_device(MPIDI_VC_t *vc, MPIR_Request *req,
                                       int *complete)
{
    return MPIDI_CH3_ReqHandler_unpack_device_stream(vc, req, complete, NULL);
}

int MPIDI_CH3_ReqHandler_unpack_device_stream(
    MPIDI_VC_t *vc ATTRIBUTE((unused)), MPIR_Request *req, int *complete,
    void *stream)
{
    MPI_Aint last;
    int iov_n, bytes_copied;
    struct iovec iov[MPL_IOV_LIMIT] = {0};
    cudaStream_t stream_passed = 0;

    if (stream != NULL) {
        stream_passed = (cudaStream_t)stream;
    }

    req->dev.segment_first = 0;
    do {
        last = req->dev.segment_size;
        iov_n = MPL_IOV_LIMIT;
        MPIR_Assert(req->dev.segment_first < last);
        MPIR_Assert(last > 0);
        MPIR_Assert(iov_n > 0 && iov_n <= MPL_IOV_LIMIT);

        MPID_Segment_unpack_vector(req->dev.segment_ptr, req->dev.segment_first,
                                   &last, iov, &iov_n);

        if (req->dev.datatype_ptr->contents->combiner == MPI_COMBINER_VECTOR &&
            (req->dev.segment_ptr->builtin_loop.loop_params.count == 1) &&
            (rdma_cuda_vector_dt_opt || rdma_cuda_kernel_dt_opt)) {
#if defined(USE_GPU_KERNEL)
            if (rdma_cuda_kernel_dt_opt) {
                pack_unpack_vector_kernel(
                    iov[0].iov_base, iov[1].iov_base - iov[0].iov_base,
                    req->dev.tmpbuf, iov[0].iov_len, iov[0].iov_len,
                    req->dev.segment_size / iov[0].iov_len, stream_passed);
            } else
#endif
            {
                vector_unpack_cudabuf(req->dev.tmpbuf, iov,
                                      req->dev.segment_size, stream_passed);
            }
            last = bytes_copied = req->dev.segment_size;
        } else if (req->dev.datatype_ptr->contents->combiner ==
                       MPI_COMBINER_SUBARRAY &&
                   (req->dev.segment_ptr->builtin_loop.loop_params.count ==
                    1) &&
                   rdma_cuda_kernel_dt_opt) {
#if defined(USE_GPU_KERNEL)
            MPIR_Datatype *dtptr = req->dev.datatype_ptr;
            int struct_size = sizeof(MPIR_Datatype_contents);
            int types_size = dtptr->contents->nr_types * sizeof(MPI_Datatype);
            int *arr_ints =
                (int *)((char *)dtptr->contents + struct_size + types_size);
            MPI_Aint base_addr;
            int subarr_dims = arr_ints[1];

            if (subarr_dims < 4) {
                int subarr_order = arr_ints[2 + 3 * subarr_dims];
                int arr_of_bigsizes[3] = {1, 1, 1};
                int arr_of_subsizes[3] = {1, 1, 1};
                int arr_of_offsets[3] = {0, 0, 0};
                int idx;
                int elem_sz = dtptr->builtin_element_size;
                MPI_Address(iov[0].iov_base, &base_addr);

                for (idx = 0; idx < subarr_dims; idx++) {
                    arr_of_bigsizes[idx] = arr_ints[2 + idx];
                    arr_of_subsizes[idx] = arr_ints[2 + 1 * subarr_dims + idx];
                    arr_of_offsets[idx] = arr_ints[2 + 2 * subarr_dims + idx];
                }

                if (MPI_ORDER_C == subarr_order) {
                    base_addr -= (arr_of_offsets[0] * arr_of_bigsizes[1] *
                                      arr_of_bigsizes[2] +
                                  arr_of_offsets[1] * arr_of_bigsizes[2] +
                                  arr_of_offsets[2]) *
                                 elem_sz;
                } else if (MPI_ORDER_FORTRAN == subarr_order) {
                    base_addr -= (arr_of_offsets[0] +
                                  arr_of_offsets[1] * arr_of_bigsizes[0] +
                                  arr_of_offsets[2] * arr_of_bigsizes[0] *
                                      arr_of_bigsizes[1]) *
                                 elem_sz;
                }

                void *dst = (void *)base_addr;

                unpack_subarray(
                    dst, req->dev.tmpbuf, subarr_dims, arr_of_bigsizes[0],
                    arr_of_bigsizes[1], arr_of_bigsizes[2], arr_of_subsizes[0],
                    arr_of_subsizes[1], arr_of_subsizes[2], arr_of_offsets[0],
                    arr_of_offsets[1], arr_of_offsets[2], subarr_order, elem_sz,
                    stream_passed);
                last = req->dev.segment_size;
                bytes_copied = last - req->dev.segment_first;
            } else {
                MVP_MPII_IOV_unpack_cuda(req->dev.tmpbuf, iov, iov_n,
                                         req->dev.segment_first, &bytes_copied,
                                         stream_passed);
            }
#endif

#if defined(USE_GPU_KERNEL)
        } else if (req->dev.datatype_ptr->contents->combiner ==
                       MPI_COMBINER_HINDEXED &&
                   rdma_cuda_kernel_dt_opt) {
            int return_hindexed_unpack = INITIAL_PACKUNPACK_OPT;
            return_hindexed_unpack = hindexed_unpack_cudabuf(
                req->dev.tmpbuf, iov, req->dev.datatype_ptr,
                req->dev.segment_size, stream_passed);

            if (return_hindexed_unpack == SUCCESS_PACKUNPACK_OPT) {
                last = bytes_copied = req->dev.segment_size;
            } else if (return_hindexed_unpack == FAILURE_PACKUNPACK_OPT) {
                MVP_MPII_IOV_unpack_cuda(req->dev.tmpbuf, iov, iov_n,
                                         req->dev.segment_first, &bytes_copied,
                                         stream_passed);
            } else {
                MVP_MPII_IOV_unpack_cuda(req->dev.tmpbuf, iov, iov_n,
                                         req->dev.segment_first, &bytes_copied,
                                         stream_passed);
            }
#endif
        } else {
            MVP_MPII_IOV_unpack_cuda(req->dev.tmpbuf, iov, iov_n,
                                     req->dev.segment_first, &bytes_copied,
                                     stream_passed);
        }

        MPIR_Assert(bytes_copied == (last - req->dev.segment_first));
        req->dev.segment_first = last;
        PRINT_INFO(CUDA_DEBUG, "unpaked :%d start:%lu last:%lu\n", iov_n,
                   req->dev.segment_first, last);
    } while (last != req->dev.segment_size);

    /* Synchronize on the stream to make sure unpack is complete*/
    cudaStreamSynchronize(stream_passed);

    MPID_Request_complete(req);
    *complete = TRUE;

    return MPI_SUCCESS;
}

void MPID_Segment_pack_device(DLOOP_Segment *segp, DLOOP_Offset first,
                              DLOOP_Offset *lastp, MPIR_Datatype *dt_ptr,
                              void *streambuf)
{
    int iov_n;
    int device_pack_buf = 1;
    int buff_off = 0;
    void *tmpbuf = NULL;
    struct iovec iov[MPL_IOV_LIMIT] = {0};
    DLOOP_Offset segment_first, segment_last;
    int segment_size;
    int sbuf_isdev = 0;
    int sbuf_isdev_check = 0;

    /* allocate temp device pack buffer */
    if (!is_device_buffer(streambuf)) {
        MVP_MPIDI_Malloc_Device(tmpbuf, *lastp);
        device_pack_buf = 0;
    } else {
        tmpbuf = streambuf;
    }

    segment_first = first;
    segment_size = *lastp - segment_first;
    do {
        segment_last = *lastp;
        iov_n = MPL_IOV_LIMIT;
        MPIR_Assert(segment_first < segment_last);
        MPIR_Assert(segment_last > 0);
        MPIR_Assert(iov_n > 0 && iov_n <= MPL_IOV_LIMIT);

        MPID_Segment_pack_vector(segp, segment_first, &segment_last, iov,
                                 &iov_n);

        if (!sbuf_isdev_check) {
            if (is_device_buffer(iov[0].iov_base)) {
                sbuf_isdev = 1;
            }
            sbuf_isdev_check = 1;
        }

        if (sbuf_isdev && dt_ptr->contents->combiner == MPI_COMBINER_VECTOR &&
            (segp->builtin_loop.loop_params.count == 1) &&
            (rdma_cuda_vector_dt_opt || rdma_cuda_kernel_dt_opt)) {
#if defined(USE_GPU_KERNEL)
            if (rdma_cuda_kernel_dt_opt) {
                pack_unpack_vector_kernel(
                    tmpbuf, iov[0].iov_len, iov[0].iov_base,
                    iov[1].iov_base - iov[0].iov_base, iov[0].iov_len,
                    segment_size / iov[0].iov_len, stream_kernel);
            } else
#endif
            {
                vector_pack_cudabuf(tmpbuf, iov, segment_size, stream_kernel);
            }
            segment_last = *lastp;
#if defined(USE_GPU_KERNEL)
        } else if (sbuf_isdev &&
                   dt_ptr->contents->combiner == MPI_COMBINER_HINDEXED &&
                   rdma_cuda_kernel_dt_opt) {
            int return_hindexed_pack = INITIAL_PACKUNPACK_OPT;
            return_hindexed_pack = hindexed_pack_cudabuf(
                tmpbuf, iov, dt_ptr, segment_size, stream_kernel);

            if (return_hindexed_pack == SUCCESS_PACKUNPACK_OPT) {
                segment_last = *lastp;
            } else if (return_hindexed_pack == FAILURE_PACKUNPACK_OPT) {
                MVP_MPII_IOV_pack_cuda((char *)tmpbuf, iov, iov_n, buff_off,
                                       stream_kernel);
            } else {
                MVP_MPII_IOV_pack_cuda((char *)tmpbuf, iov, iov_n, buff_off,
                                       stream_kernel);
            }
#endif
        } else {
            MVP_MPII_IOV_pack_cuda((char *)tmpbuf, iov, iov_n, buff_off,
                                   stream_kernel);
        }

        buff_off += (segment_last - segment_first);
        segment_first = segment_last;

    } while (segment_last != *lastp);

    /* This is just a pack function. synchronize for kernel to complete before
     * the following copy */
    CUDA_CHECK(cudaStreamSynchronize(stream_kernel));

    /* copy to device pack buffer to host pack buffer */
    if (!device_pack_buf) {
        MVP_MPIDI_Memcpy_Device(streambuf, tmpbuf, *lastp,
                                cudaMemcpyDeviceToHost);
        MVP_MPIDI_Free_Device(tmpbuf);
    }
}

void MPID_Segment_unpack_device(DLOOP_Segment *segp, DLOOP_Offset first,
                                DLOOP_Offset *lastp, MPIR_Datatype *dt_ptr,
                                void *inbuf)
{
    int iov_n;
    int bytes_unpacked;
    int device_unpack_buf = 1;
    int buff_off = 0;
    void *tmpbuf;
    struct iovec iov[MPL_IOV_LIMIT] = {0};
    DLOOP_Offset segment_first, segment_last;
    int segment_size;
    int rbuf_isdev = 0;
    int rbuf_isdev_check = 0;

    /* allocate temp device unpack buffer */
    if (!is_device_buffer(inbuf)) {
        device_unpack_buf = 0;
        MVP_MPIDI_Malloc_Device(tmpbuf, *lastp);
        MVP_MPIDI_Memcpy_Device(tmpbuf, inbuf, *lastp, cudaMemcpyHostToDevice);
    } else {
        tmpbuf = inbuf;
    }

    segment_first = first;
    segment_size = *lastp - segment_first;

    do {
        segment_last = *lastp;
        iov_n = MPL_IOV_LIMIT;
        MPIR_Assert(segment_first < segment_last);
        MPIR_Assert(segment_last > 0);
        MPIR_Assert(iov_n > 0 && iov_n <= MPL_IOV_LIMIT);

        MPID_Segment_unpack_vector(segp, segment_first, &segment_last, iov,
                                   &iov_n);

        if (!rbuf_isdev_check) {
            if (is_device_buffer(iov[0].iov_base)) {
                rbuf_isdev = 1;
            }
            rbuf_isdev_check = 1;
        }

        if (rbuf_isdev && dt_ptr->contents->combiner == MPI_COMBINER_VECTOR &&
            (segp->builtin_loop.loop_params.count == 1) &&
            (rdma_cuda_vector_dt_opt || rdma_cuda_kernel_dt_opt)) {
#if defined(USE_GPU_KERNEL)
            if (rdma_cuda_kernel_dt_opt) {
                pack_unpack_vector_kernel(
                    iov[0].iov_base, iov[1].iov_base - iov[0].iov_base, tmpbuf,
                    iov[0].iov_len, iov[0].iov_len,
                    segment_size / iov[0].iov_len, stream_kernel);
            } else
#endif
            {
                vector_unpack_cudabuf(tmpbuf, iov, segment_size, stream_kernel);
            }
            segment_last = *lastp;
            bytes_unpacked = segment_last - segment_first;
#if defined(USE_GPU_KERNEL)
        } else if (rbuf_isdev &&
                   dt_ptr->contents->combiner == MPI_COMBINER_HINDEXED &&
                   rdma_cuda_kernel_dt_opt) {
            int return_hindexed_unpack = INITIAL_PACKUNPACK_OPT;
            return_hindexed_unpack = hindexed_unpack_cudabuf(
                tmpbuf, iov, dt_ptr, segment_size, stream_kernel);

            if (return_hindexed_unpack == SUCCESS_PACKUNPACK_OPT) {
                segment_last = *lastp;
                bytes_unpacked = segment_last - segment_first;
            } else if (return_hindexed_unpack == FAILURE_PACKUNPACK_OPT) {
                MVP_MPII_IOV_unpack_cuda(tmpbuf, iov, iov_n, buff_off,
                                         &bytes_unpacked, stream_kernel);
            } else {
                MVP_MPII_IOV_unpack_cuda(tmpbuf, iov, iov_n, buff_off,
                                         &bytes_unpacked, stream_kernel);
            }
#endif
        } else {
            MVP_MPII_IOV_unpack_cuda(tmpbuf, iov, iov_n, buff_off,
                                     &bytes_unpacked, stream_kernel);
        }

        MPIR_Assert(bytes_unpacked == (segment_last - segment_first));
        segment_first = segment_last;
        buff_off += bytes_unpacked;

    } while (segment_last != *lastp);

    cudaStreamSynchronize(stream_kernel);

    if (!device_unpack_buf) {
        MVP_MPIDI_Free_Device(tmpbuf);
    }
}

int is_device_buffer(const void *buffer)
{
    int memory_type, is_dev = 0;
    cudaError_t cuda_err = cudaSuccess;
    struct cudaPointerAttributes attributes;
    CUresult cu_err = CUDA_SUCCESS;

    if (!rdma_enable_cuda || buffer == NULL || buffer == MPI_IN_PLACE ||
        !can_use_cuda) {
        return 0;
    }

    if (mvp_device_dynamic_init && (mvp_save_cuda_context == NULL)) {
        cu_err = cuCtxGetCurrent(&mvp_save_cuda_context);
        if (cu_err != CUDA_SUCCESS || mvp_save_cuda_context == NULL) {
            return 0;
        }
    }

    cu_err = cuPointerGetAttribute(
        &memory_type, CU_POINTER_ATTRIBUTE_MEMORY_TYPE, (CUdeviceptr)buffer);
    if (cu_err != CUDA_SUCCESS) {
        if (mvp_device_check_attribute) {
            cuda_err = cudaPointerGetAttributes(&attributes, buffer);
            if (cuda_err == cudaSuccess) {
#if CUDA_VERSION >= 11000
                is_dev = (attributes.type == cudaMemoryTypeDevice) ? 1 : 0;
#else
                is_dev =
                    (attributes.memoryType == cudaMemoryTypeDevice) ? 1 : 0;
#endif
            }
        }
    } else {
        is_dev = (memory_type == CU_MEMORYTYPE_DEVICE) ? 1 : 0;
    }

    if (is_dev && mvp_device_dynamic_init && !mvp_device_initialized) {
        device_init_dynamic(MPIDI_Process.my_pg);
    }

    return is_dev;
}

void ibv_device_register(void *ptr, size_t size)
{
    cudaError_t cuerr = cudaSuccess;

    if (ptr == NULL || (mvp_device_dynamic_init && !mvp_device_initialized)) {
        return;
    }

    cuerr = cudaHostRegister(ptr, size, cudaHostRegisterPortable);
    if (cuerr != cudaSuccess) {
        ibv_error_abort(GEN_EXIT_ERR, "cudaHostRegister Failed");
    }
    PRINT_DEBUG(DEBUG_CUDA_verbose,
                "cudaHostRegister success ptr:%p size:%lu\n", ptr, size);
}

void ibv_device_unregister(void *ptr)
{
    cudaError_t cuerr = cudaSuccess;
    if (ptr == NULL || (mvp_device_dynamic_init && !mvp_device_initialized)) {
        return;
    }
    cuerr = cudaHostUnregister(ptr);
    if (cuerr != cudaSuccess) {
        ibv_error_abort(GEN_EXIT_ERR, "cudaHostUnegister Failed");
    }
    PRINT_DEBUG(DEBUG_CUDA_verbose, "cudaHostUnregister success ptr:%p\n", ptr);
}

void cuda_get_user_parameters()
{
    char *value = NULL;

    if (MVP_EAGER_CUDAHOST_REG != -1) {
        rdma_eager_devicehost_reg = MVP_EAGER_CUDAHOST_REG;
    }

    if (MVP_CUDA_VECTOR_OPT != -1) {
        rdma_cuda_vector_dt_opt = MVP_CUDA_VECTOR_OPT;
    }

#if defined(USE_GPU_KERNEL)
    if (MVP_CUDA_KERNEL_OPT != -1) {
        rdma_cuda_kernel_dt_opt = MVP_CUDA_KERNEL_OPT;
    }
#else
    {
        rdma_cuda_kernel_dt_opt = 0;
    }
#endif

    if (MVP_CUDA_KERNEL_VECTOR_TIDBLK_SIZE != -1) {
        rdma_cuda_vec_thread_blksz = MVP_CUDA_KERNEL_VECTOR_TIDBLK_SIZE;
    }

    if (MVP_CUDA_KERNEL_VECTOR_YSIZE != -1) {
        rdma_cuda_vec_thread_ysz = MVP_CUDA_KERNEL_VECTOR_YSIZE;
    }

    if (MVP_CUDA_KERNEL_SUBARR_TIDBLK_SIZE != -1) {
        rdma_cuda_subarr_thread_blksz = MVP_CUDA_KERNEL_SUBARR_TIDBLK_SIZE;
    }

    if (MVP_CUDA_KERNEL_SUBARR_XDIM != -1) {
        rdma_cuda_subarr_thread_xdim = MVP_CUDA_KERNEL_SUBARR_XDIM;
    }

    if (MVP_CUDA_KERNEL_SUBARR_YDIM != -1) {
        rdma_cuda_subarr_thread_ydim = MVP_CUDA_KERNEL_SUBARR_YDIM;
    }

    if (MVP_CUDA_KERNEL_SUBARR_ZDIM != -1) {
        rdma_cuda_subarr_thread_zdim = MVP_CUDA_KERNEL_SUBARR_ZDIM;
    }

    if (MVP_CUDA_NUM_EVENTS != -1) {
        mvp_device_event_count = MVP_CUDA_NUM_EVENTS;
    }

    if (MVP_CUDA_INIT_CONTEXT != -1) {
        mvp_device_init_context = MVP_CUDA_INIT_CONTEXT;
    }

    if (MVP_CHECK_CUDA_ATTRIBUTE != -1) {
        mvp_device_check_attribute = MVP_CHECK_CUDA_ATTRIBUTE;
    }

    if (MVP_CUDA_USE_NAIVE != -1) {
        mvp_device_coll_use_stage = MVP_CUDA_USE_NAIVE;
    }

    if (MVP_CUDA_ALLTOALL_DYNAMIC != -1) {
        mvp_device_alltoall_dynamic = MVP_CUDA_ALLTOALL_DYNAMIC;
    }

    if (MVP_CUDA_REGISTER_NAIVE_BUF != -1) {
        mvp_device_coll_register_stage_buf_threshold =
            MVP_CUDA_REGISTER_NAIVE_BUF;
    }

    if (MVP_CUDA_GATHER_NAIVE_LIMIT != -1) {
        mvp_device_gather_stage_limit = MVP_CUDA_GATHER_NAIVE_LIMIT;
    }

    if (MVP_CUDA_SCATTER_NAIVE_LIMIT != -1) {
        mvp_device_scatter_stage_limit = MVP_CUDA_SCATTER_NAIVE_LIMIT;
    }

    if (MVP_CUDA_ALLTOALL_NAIVE_LIMIT != -1) {
        mvp_device_alltoall_stage_limit = MVP_CUDA_ALLTOALL_NAIVE_LIMIT;
    }

    if (MVP_CUDA_ALLTOALLV_NAIVE_LIMIT != -1) {
        mvp_device_alltoallv_stage_limit = MVP_CUDA_ALLTOALLV_NAIVE_LIMIT;
    }

    if (MVP_CUDA_ALLGATHER_NAIVE_LIMIT != -1) {
        mvp_device_allgather_stage_limit = MVP_CUDA_ALLGATHER_NAIVE_LIMIT;
    }

    if (MVP_CUDA_ALLGATHERV_NAIVE_LIMIT != -1) {
        mvp_device_allgatherv_stage_limit = MVP_CUDA_ALLGATHERV_NAIVE_LIMIT;
    }

    if (MVP_CUDA_BCAST_NAIVE_LIMIT != -1) {
        mvp_device_bcast_stage_limit = MVP_CUDA_BCAST_NAIVE_LIMIT;
    }

    if (MVP_CUDA_GATHERV_NAIVE_LIMIT != -1) {
        mvp_device_gatherv_stage_limit = MVP_CUDA_GATHERV_NAIVE_LIMIT;
    }

    if (MVP_CUDA_SCATTERV_NAIVE_LIMIT != -1) {
        mvp_device_scatterv_stage_limit = MVP_CUDA_SCATTERV_NAIVE_LIMIT;
    }

    if (MVP_CUDA_ALLGATHER_RD_LIMIT != -1) {
        mvp_device_allgather_rd_limit = MVP_CUDA_ALLGATHER_RD_LIMIT;
    }

    if (MVP_CUDA_ALLGATHER_FGP != -1) {
        mvp_device_use_allgather_fgp = MVP_CUDA_ALLGATHER_FGP;
    }

#if defined(HAVE_CUDA_IPC)
    if (MVP_CUDA_IPC != -1) {
        mvp_device_use_ipc = MVP_CUDA_IPC;
    }

    if (MVP_CUDA_IPC_SHARE_GPU != -1) {
        rdma_enable_ipc_share_gpu = MVP_CUDA_IPC_SHARE_GPU;
    }

    if (MVP_CUDA_SMP_IPC != -1) {
        mvp_device_use_smp_eager_ipc = MVP_CUDA_SMP_IPC;
    }
    if (!mvp_device_use_ipc) {
        mvp_device_use_smp_eager_ipc = 0;
    }

    if (mvp_device_use_ipc && CUDART_VERSION < 4010) {
        PRINT_DEBUG(DEBUG_CUDA_verbose > 1,
                    "IPC is available only"
                    "from version 4.1 or later, version available : %d",
                    CUDART_VERSION);
        mvp_device_use_ipc = 0;
        mvp_device_use_smp_eager_ipc = 0;
    }

    if (MVP_CUDA_ENABLE_IPC_CACHE != -1) {
        mvp_device_enable_ipc_cache = MVP_CUDA_ENABLE_IPC_CACHE;
    }

    if (MVP_CUDA_IPC_MAX_CACHE_ENTRIES != -1) {
        mvp_device_ipc_cache_max_entries = MVP_CUDA_IPC_MAX_CACHE_ENTRIES;
    }

    if (MVP_CUDA_IPC_NUM_STAGE_BUFFERS != -1) {
        cudaipc_num_stage_buffers = MVP_CUDA_IPC_NUM_STAGE_BUFFERS;
    }
    if (MVP_CUDA_IPC_STAGE_BUF_SIZE != -1) {
        cudaipc_stage_buffer_size = MVP_CUDA_IPC_STAGE_BUF_SIZE;
    }
    if (MVP_CUDA_IPC_BUFFERED != -1) {
        mvp_device_use_ipc_stage_buffer = MVP_CUDA_IPC_BUFFERED;
    }
    if (mvp_device_use_ipc_stage_buffer) {
        mvp_device_ipc_threshold = 0;
    }
    if (MVP_CUDA_IPC_THRESHOLD != -1) {
        mvp_device_ipc_threshold = MVP_CUDA_IPC_THRESHOLD;
    }
    if (MVP_CUDA_IPC_BUFFERED_LIMIT != -1) {
        mvp_device_ipc_stage_buffer_limit = MVP_CUDA_IPC_BUFFERED_LIMIT;
    }
    if (MVP_CUDA_IPC_SYNC_LIMIT != -1) {
        cudaipc_sync_limit = MVP_CUDA_IPC_SYNC_LIMIT;
    }
    MPIR_Assert(mvp_device_ipc_stage_buffer_limit >= mvp_device_ipc_threshold);

    if (MVP_CUDA_DYNAMIC_INIT != -1) {
        mvp_device_dynamic_init = MVP_CUDA_DYNAMIC_INIT;
    }

    if (MVP_CUDA_NONBLOCKING_STREAMS != -1) {
        mvp_device_nonblocking_streams = MVP_CUDA_NONBLOCKING_STREAMS;
    }

    /*TODO: remove this dependency*/
    /*disabling cuda_smp_ipc (disabled by default) when dynamic initialization
     * us used*/
    if (mvp_device_dynamic_init) {
        mvp_device_use_smp_eager_ipc = 0;
    }
#endif
}

void device_init(MPIDI_PG_t *pg)
{
#if defined(HAVE_CUDA_IPC)
    MPIR_Errflag_t errflag = MPIR_ERR_NONE;
    int mpi_errno = MPI_SUCCESS;
    int i, num_processes, my_rank, has_cudaipc_peer = 0;
    int my_local_rank, dev_count, my_dev_id;
    int *device = NULL;
    MPIR_Comm *comm_world = NULL;
    MPIDI_VC_t *vc = NULL;
    cudaError_t cudaerr = cudaSuccess;
    CUresult curesult = CUDA_SUCCESS;
    CUdevice cudevice;

    comm_world = MPIR_Process.comm_world;
    num_processes = comm_world->local_size;
    my_rank = comm_world->rank;

    curesult = cuCtxGetCurrent(&mvp_save_cuda_context);
    if (curesult != CUDA_SUCCESS || mvp_save_cuda_context == NULL) {
        if (mvp_device_init_context) {
            /*use has not selected a device or not created a context,
             *select device internally*/
            my_local_rank = MPIDI_Process.my_pg->ch.local_process_id;
            curesult = cuInit(0);
            if (curesult != CUDA_SUCCESS) {
                ibv_error_abort(GEN_EXIT_ERR, "cuInit failed\n");
            }
            cudaerr = cudaGetDeviceCount(&dev_count);
            if (cudaerr != cudaSuccess) {
                ibv_error_abort(GEN_EXIT_ERR, "get device count failed \n");
            }
            my_dev_id = my_local_rank % dev_count;
            curesult = cuDeviceGet(&cudevice, my_dev_id);
            if (curesult != CUDA_SUCCESS) {
                ibv_error_abort(GEN_EXIT_ERR, "cuDeviceGet failed \n");
            }
            curesult = cuCtxCreate(&mvp_cuda_context, 0, cudevice);
            if (curesult != CUDA_SUCCESS) {
                ibv_error_abort(GEN_EXIT_ERR, "cuCtxCreate failed \n");
            }
        } else {
            ibv_error_abort(GEN_EXIT_ERR, "Active CUDA context not detected.\
                 Select a device and create context before MPI_Init.\n");
        }
        mvp_save_cuda_context = mvp_cuda_context;
    }

    device = (int *)MPL_malloc(sizeof(int) * num_processes);
    if (device == NULL) {
        ibv_error_abort(GEN_EXIT_ERR, "memory allocation failed");
    }
    MPIR_Memset(device, 0, sizeof(int) * num_processes);

    cudaerr = cudaGetDevice(&device[my_rank]);
    if (cudaerr != cudaSuccess) {
        ibv_error_abort(GEN_EXIT_ERR, "cudaGetDevice failed");
    }

    mpi_errno =
        MPIR_Allgather_impl(MPI_IN_PLACE, 0, MPI_DATATYPE_NULL, device,
                            sizeof(int), MPI_BYTE, comm_world, &errflag);
    if (mpi_errno != MPI_SUCCESS) {
        ibv_error_abort(GEN_EXIT_ERR, "MPIR_Allgather_impl returned error");
    }

    for (i = 0; i < num_processes; i++) {
        if (i == my_rank)
            continue;
        MPIDI_Comm_get_vc(comm_world, i, &vc);
        vc->smp.can_access_peer = MVP_DEVICE_IPC_UNINITIALIZED;
        if (vc->smp.local_rank != -1) {
            if (rdma_enable_ipc_share_gpu) {
                /*if both processes are using the same device, IPC works
                    but cudaDeviceCanAccessPeer returns 0, or
                    else decide based on result of cudaDeviceCanAccessPeer*/
                if (device[my_rank] == device[i]) {
                    vc->smp.can_access_peer = MVP_DEVICE_IPC_ENABLED;
                } else {
                    cudaerr =
                        cudaDeviceCanAccessPeer((int *)&vc->smp.can_access_peer,
                                                device[my_rank], device[i]);
                    if (cudaerr != cudaSuccess) {
                        ibv_error_abort(GEN_EXIT_ERR,
                                        "cudaDeviceCanAccessPeer failed");
                    }
                    vc->smp.can_access_peer = (vc->smp.can_access_peer == 0) ?
                                                  MVP_DEVICE_IPC_DISABLED :
                                                  MVP_DEVICE_IPC_ENABLED;
                }
            } else {
                cudaerr =
                    cudaDeviceCanAccessPeer((int *)&vc->smp.can_access_peer,
                                            device[my_rank], device[i]);
                if (cudaerr != cudaSuccess) {
                    ibv_error_abort(GEN_EXIT_ERR,
                                    "cudaDeviceCanAccessPeer failed");
                }
                vc->smp.can_access_peer = (vc->smp.can_access_peer == 0) ?
                                              MVP_DEVICE_IPC_DISABLED :
                                              MVP_DEVICE_IPC_ENABLED;
            }
            if (vc->smp.can_access_peer == MVP_DEVICE_IPC_ENABLED) {
                has_cudaipc_peer = 1;
            }
        } else {
            vc->smp.can_access_peer = MVP_DEVICE_IPC_DISABLED;
        }
    }

    deviceipc_num_local_procs = MPIDI_MVP_Num_local_processes(pg);
    deviceipc_my_local_id = MPIDI_MVP_Get_local_process_id(pg);

    mpi_errno = MPIR_Allreduce_impl(&has_cudaipc_peer, &cudaipc_init, 1,
                                    MPI_INT, MPI_SUM, comm_world, &errflag);
    if (mpi_errno) {
        ibv_error_abort(GEN_EXIT_ERR, "MPIR_Allgather_impl returned error");
    }
    cudaipc_init_global = cudaipc_init;

    if (mvp_device_use_ipc && mvp_device_use_ipc_stage_buffer) {
        if (cudaipc_init) {
            device_ipc_initialize(pg, num_processes, my_rank);
        }
    }

    if (mvp_device_use_ipc && !mvp_device_use_ipc_stage_buffer &&
        mvp_device_enable_ipc_cache && has_cudaipc_peer) {
        device_ipc_initialize_cache();
    }
    MPL_free(device);
#endif

    can_use_cuda = 1;
    mvp_device_initialized = 1;

    if (stream_d2h == 0 && stream_h2d == 0) {
        allocate_cuda_rndv_streams();
    }

    if (SMP_INIT) {
        MPIDI_CH3I_CUDA_SMP_device_init(pg);
    }
}

void device_preinit(MPIDI_PG_t *pg)
{
    int dev_count;
    cudaError_t cuda_err = CUDA_SUCCESS;
#if defined(HAVE_CUDA_IPC)
    MPIR_Errflag_t errflag = MPIR_ERR_NONE;
    int mpi_errno = MPI_SUCCESS;
    int i, num_processes, my_rank;
    MPIR_Comm *comm_world = NULL;
    MPIDI_VC_t *vc = NULL;
#endif

    cuda_err = cudaGetDeviceCount(&dev_count);
    if (cuda_err == cudaSuccess && dev_count > 0) {
        can_use_cuda = 1;
    }

#if defined(HAVE_CUDA_IPC)
    if (mvp_device_use_ipc) {
        comm_world = MPIR_Process.comm_world;
        num_processes = comm_world->local_size;
        my_rank = comm_world->rank;

        deviceipc_num_local_procs = MPIDI_MVP_Num_local_processes(pg);
        deviceipc_my_local_id = MPIDI_MVP_Get_local_process_id(pg);

        /*check if any one of the nodes have multiple processes/node and a GPU*/
        cudaipc_init =
            (deviceipc_num_local_procs > 1 && can_use_cuda == 1) ? 1 : 0;
        mpi_errno = MPIR_Allreduce_impl(&cudaipc_init, &cudaipc_init_global, 1,
                                        MPI_INT, MPI_SUM, comm_world, &errflag);

        if (mpi_errno != MPI_SUCCESS) {
            ibv_error_abort(GEN_EXIT_ERR, "MPIR_Allreduce_impl returned error");
        }

        if (cudaipc_init) {
            /*set can_access_peer to pre-setup value*/
            for (i = 0; i < num_processes; i++) {
                MPIDI_Comm_get_vc(comm_world, i, &vc);
                vc->smp.can_access_peer = MVP_DEVICE_IPC_UNINITIALIZED;
            }
        } else {
            /*set can_access_peer to disabled value*/
            for (i = 0; i < num_processes; i++) {
                MPIDI_Comm_get_vc(comm_world, i, &vc);
                vc->smp.can_access_peer = MVP_DEVICE_IPC_DISABLED;
            }
        }

        if (cudaipc_init_global) {
            /*allocate shared memory region to exchange ipc info*/
            cudaipc_allocate_shared_region(pg, num_processes, my_rank);
        }
    }
#endif
}

void device_init_dynamic(MPIDI_PG_t *pg)
{
    int i, num_processes, my_rank;
    MPIR_Comm *comm_world = NULL;
    MPIDI_VC_t *vc = NULL;

    mvp_device_initialized = 1;

    /*allocate CUDA streams*/
    MPIR_Assert(stream_d2h == 0 && stream_h2d == 0);
    allocate_cuda_rndv_streams();

    /*initialize and register shared memory buffers*/
    if (SMP_INIT) {
        MPIDI_CH3I_CUDA_SMP_device_init(pg);
    }

    /*register vbufs*/
    register_cuda_vbuf_regions();

    /*register rdma fastpath buffers*/
    comm_world = MPIR_Process.comm_world;
    num_processes = comm_world->local_size;
    my_rank = comm_world->rank;

    for (i = 0; i < num_processes; i++) {
        MPIDI_Comm_get_vc(comm_world, i, &vc);
        if (vc->mrail.rfp.RDMA_send_buf_DMA) {
            if (rdma_eager_devicehost_reg) {
                ibv_device_register(vc->mrail.rfp.RDMA_send_buf_DMA,
                                    num_rdma_buffer * rdma_fp_buffer_size);
            }
        }
    }

    if (mvp_device_use_ipc && cudaipc_init) {
        if (mvp_device_use_ipc_stage_buffer) {
            /*allocate ipc region locally*/
            cudaipc_allocate_ipc_region(pg, num_processes, my_rank);
        } else {
            if (mvp_device_enable_ipc_cache) {
                /*set device info in shared region*/
                cudaipc_share_device_info();
                /*initialize ipc cache*/
                device_ipc_initialize_cache();
            }
        }
    }
}

void device_cleanup()
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Errflag_t errflag = MPIR_ERR_NONE;

    /* Global Barrier - All processes must participate even if no active
     * contexts exist. Otherwise it will lead to a hang in case of some
     * processes calling Finalize without making CUDA calls */

    /*TODO: this synchronization can be made local to the node*/
#if defined(HAVE_CUDA_IPC)
    if (mvp_device_use_ipc && mvp_device_use_ipc_stage_buffer) {
        MPIR_Barrier_impl(MPIR_Process.comm_world, &errflag);
        if (MPI_SUCCESS != mpi_errno) {
            ibv_error_abort(GEN_EXIT_ERR,
                            "MPI_Barrier failed in cuda_cleanup \n");
        }
    }
#endif

    deallocate_cuda_events();
    deallocate_cuda_rndv_streams();

#if defined(HAVE_CUDA_IPC)
    if (mvp_device_use_ipc) {
        cudaipc_finalize();
    }

    if (mvp_device_use_ipc && cudaipc_cache_list != NULL) {
        int i;
        for (i = 0; i < deviceipc_num_local_procs; i++) {
            cudaipc_flush_regcache(i, num_cudaipc_cache_entries[i]);
        }
        MPL_free(cudaipc_cache_list);
        MPL_free(num_cudaipc_cache_entries);
    }
#endif

    if (SMP_INIT) {
        MPIDI_CH3I_CUDA_SMP_device_finalize(MPIDI_Process.my_pg);
    }

    if (mvp_cuda_context != NULL) {
        CU_CHECK(cuCtxDestroy(mvp_cuda_context));
    }
}

void device_init_thread_context()
{
    CUresult curesult = CUDA_SUCCESS;
    curesult = cuCtxSetCurrent(mvp_save_cuda_context);
    if (curesult != CUDA_SUCCESS) {
        ibv_error_abort(GEN_EXIT_ERR,
                        "Error in setting cuda context in a thread\n");
    }
}
#endif

int MPIX_Query_cuda_support()
{
#if defined(_ENABLE_CUDA_)
    /* NOTE: Ideally, we may want to return 'rdma_enable_cuda'.
     * However, this function can be called before performing MPI_Init,
     * where we initialize rdma_enable_cuda based on MVP_USE_CUDA. */
    return 1;
#else
    return 0;
#endif
}
