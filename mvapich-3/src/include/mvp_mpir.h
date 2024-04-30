/* Copyright (c) 2001-2021, The Ohio State University. All rights
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

/*
 * This file contains the MVP extensions to the MPIR_ interface used by MPICH
 * most of these formerly resided in mpiimpl.h but after MPICH refactored that
 * file it makes more sense to keep these in a seperate file.
 */

#ifndef MVP_MPIR_H_INCLUDED
#define MVP_MPIR_H_INCLUDED

#include "mvp_err.h"
#include "mvp_arch_hca.h"
/*****************************************************************************/
/***************************** mvp_mpir_coll.h *******************************/
/*****************************************************************************/

/*
 * this section defines several collective related values used by MVAPICH
 * formerlly residing in mpiimpl.h
 */

/* max no. of irecvs/isends posted at a
time in some alltoall algorithms. Setting
it to 0 causes all irecvs/isends to be
posted at once. */
#define MVP_ALLTOALL_LARGE_MSG 64 * 1024

#define MPIR_BCAST_LONG_MSG  524288
#define MPIR_BCAST_MIN_PROCS 8

#ifdef _OSU_MVAPICH_
#define MPIR_ALLTOALL_SMALL_SYSTEM_SIZE 256
#define MPIR_BCAST_LARGE_MSG            512 * 1024
#endif /* _OSU_MVAPICH_ */
#define MVP_ALLGATHER_SHORT_MSG 1024
#define MVP_ALLGATHER_LONG_MSG  1024
#define MPIR_GATHER_VSMALL_MSG  1024
#define MPIR_SCATTER_SHORT_MSG  2048 /* for intercommunicator scatter */
#define MPIR_GATHER_SHORT_MSG   2048 /* for intercommunicator scatter */
#define MPIR_GATHERV_MIN_PROCS  32

#ifdef _OSU_MVAPICH_

int MPIR_MVP_Wait(MPIR_Request *request_ptr, MPI_Status *status);

int MPIR_Allgather_MVP(const void *sendbuf, int sendcount,
                       MPI_Datatype sendtype, void *recvbuf, int recvcount,
                       MPI_Datatype recvtype, MPIR_Comm *comm_ptr,
                       MPIR_Errflag_t *errflag);
int MPIR_Allgatherv_MVP(const void *sendbuf, int sendcount,
                        MPI_Datatype sendtype, void *recvbuf,
                        const int *recvcounts, const int *displs,
                        MPI_Datatype recvtype, MPIR_Comm *comm_ptr,
                        MPIR_Errflag_t *errflag);
int MPIR_Allreduce_MVP(const void *sendbuf, void *recvbuf, int count,
                       MPI_Datatype datatype, MPI_Op op, MPIR_Comm *comm_ptr,
                       MPIR_Errflag_t *errflag);
int MPIR_Alltoall_MVP(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                      void *recvbuf, int recvcount, MPI_Datatype recvtype,
                      MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);
int MPIR_Alltoallv_MVP(const void *sendbuf, const int *sendcnts,
                       const int *sdispls, MPI_Datatype sendtype, void *recvbuf,
                       const int *recvcnts, const int *rdispls,
                       MPI_Datatype recvtype, MPIR_Comm *comm_ptr,
                       MPIR_Errflag_t *errflag);
int MPIR_Alltoallv_intra_MVP(const void *sendbuf, const int *sendcnts,
                             const int *sdispls, MPI_Datatype sendtype,
                             void *recvbuf, const int *recvcnts,
                             const int *rdispls, MPI_Datatype recvtype,
                             MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);
int MPIR_Bcast_MVP(void *buffer, int count, MPI_Datatype datatype, int root,
                   MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);
int MPIR_Gather_MVP(const void *sendbuf, int sendcnt, MPI_Datatype sendtype,
                    void *recvbuf, int recvcnt, MPI_Datatype recvtype, int root,
                    MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);
int MPIR_Gatherv_MVP(const void *sendbuf, int sendcnt, MPI_Datatype sendtype,
                     void *recvbuf, const int *recvcnts, const int *displs,
                     MPI_Datatype recvtype, int root, MPIR_Comm *comm_ptr,
                     MPIR_Errflag_t *errflag);
int MPIR_Reduce_MVP(const void *sendbuf, void *recvbuf, int count,
                    MPI_Datatype datatype, MPI_Op op, int root,
                    MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);
int MPIR_Scatter_MVP(const void *sendbuf, int sendcnt, MPI_Datatype sendtype,
                     void *recvbuf, int recvcnt, MPI_Datatype recvtype,
                     int root, MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);
int MPIR_Reduce_scatter_MVP(const void *sendbuf, void *recvbuf,
                            const int *recvcnts, MPI_Datatype datatype,
                            MPI_Op op, MPIR_Comm *comm_ptr,
                            MPIR_Errflag_t *errflag);
int MPIR_Reduce_scatter_block_MVP(const void *sendbuf, void *recvbuf,
                                  int recvcount, MPI_Datatype datatype,
                                  MPI_Op op, MPIR_Comm *comm_ptr,
                                  MPIR_Errflag_t *errflag);
int MPIR_Scan_MVP(const void *sendbuf, void *recvbuf, int count,
                  MPI_Datatype datatype, MPI_Op op, MPIR_Comm *comm_ptr,
                  MPIR_Errflag_t *errflag);
int MPIR_Exscan_MVP(const void *sendbuf, void *recvbuf, int count,
                    MPI_Datatype datatype, MPI_Op op, MPIR_Comm *comm_ptr,
                    MPIR_Errflag_t *errflag);
int MPIR_Barrier_MVP(MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);

#define MPIR_RECURSION_GUARD_DECL(function_pointer_type, fallback)             \
    static int in_algo = 0;                                                    \
    static function_pointer_type fallback_fn = fallback;
#define MPIR_RECURSION_GUARD_ENTER(...)                                        \
    if (in_algo) {                                                             \
        fallback_fn(__VA_ARGS__);                                              \
        goto fn_exit;                                                          \
    }                                                                          \
    in_algo = 1;
#define MPIR_RECURSION_GUARD_EXIT in_algo = 0;

#if defined(_SHARP_SUPPORT_)
/* include SHARP functions here */
//#include "mvp_sharp.h"
extern int MVP_ENABLE_SHARP;
#endif /*(_SHARP_SUPPORT_)*/

#endif /* _OSU_MVAPICH_ */

/*****************************************************************************/
/*************************** mvp_mpir_contextid.h ****************************/
/*****************************************************************************/

/*****************************************************************************/
/****************************** mvp_mpir_mem.h *******************************/
/*****************************************************************************/

/*
 * This file defines the MVAPICH specific extentions of MPICH's MPIR memory
 * interface
 */

#include <string.h>

#if defined(CHANNEL_MRAIL) || defined(_MVP_CH4_OVERRIDE_)

#if (!defined(NDEBUG) && defined(HAVE_ERROR_CHECKING))
/* May be used to perform sanity and range checking on memcpy and mempcy-like
   function calls.  This macro will bail out much like an MPIR_Assert if any of
   the checks fail. */
#define CHECK_MEMSET(dst_, c, len_)                                            \
    do {                                                                       \
        MPIR_Assert(len_ > 0);                                                 \
        MPL_VG_CHECK_MEM_IS_ADDRESSABLE((dst_), (len_));                       \
    } while (0)
#else
#define CHECK_MEMSET(dst_, src_, len_)                                         \
    do {                                                                       \
    } while (0)
#endif /* (!defined(NDEBUG) && defined(HAVE_ERROR_CHECKING)) */

/* Overriding memset:
 *  Devices and channels can override the default implementation of
 *  MPIR_Memset by defining the MPID_Memset macro.  The implementation
 *  can call MPIR_Memset for the default memset implementation.
 *  Note that MPIR_Memset returns void rather than a
 *  pointer to the destination buffer.  This is different from C89
 *  memset.
 */
#define MPIR_Memset(dst, c, len)                                               \
    do {                                                                       \
        CHECK_MEMSET((dst), (c), (len));                                       \
        memset((dst), (c), (len));                                             \
    } while (0)

#endif /* defined(CHANNEL_MRAIL) or defined(_MVP_CH4_OVERRIDE_) */

/*****************************************************************************/
/***************************** mvp_mpir_request.h ****************************/
/*****************************************************************************/

/*
 * this section defines several collective related values used by MVAPICH
 * formerly residing in mpiimpl.h
 */
/* MVP dummy request builtin type */
#if 0
#define MPIR_MVP_REQUEST_DUMMY (MPI_Request)0x6c000020
#undef MPIR_REQUEST_BUILTIN_COUNT
#define MPIR_REQUEST_BUILTIN_COUNT 0x21
#endif

#endif /* MVP_MPIR_H_INCLUDED */
