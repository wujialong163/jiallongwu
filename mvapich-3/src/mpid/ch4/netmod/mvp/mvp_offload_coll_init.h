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

#ifndef _MVP_OFFLOAD_COLL_INIT_H_
#define _MVP_OFFLOAD_COLL_INIT_H_

#if defined(_SHARP_SUPPORT_)
#include "mvp_sharp.h"
#endif

MPL_STATIC_INLINE_PREFIX int MPIDI_MVP_offload_comm_setup_allreduce(
    MPIR_Comm *comm)
{
    int mpi_errno = MPI_SUCCESS;
#if defined(_SHARP_SUPPORT_)
    mpi_errno = MPIR_MVP_sharp_comm_setup_allreduce(comm);
#endif /*defined(_SHARP_SUPPORT_)*/
    return mpi_errno;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_MVP_offload_comm_setup_barrier(
    MPIR_Comm *comm)
{
    int mpi_errno = MPI_SUCCESS;
#if defined(_SHARP_SUPPORT_)
    mpi_errno = MPIR_MVP_sharp_comm_setup_barrier(comm);
#endif /*defined(_SHARP_SUPPORT_)*/
    return mpi_errno;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_MVP_offload_comm_setup_bcast(MPIR_Comm *comm)
{
    int mpi_errno = MPI_SUCCESS;
#if defined(_SHARP_SUPPORT_)
    mpi_errno = MPIR_MVP_sharp_comm_setup_bcast(comm);
#endif /*defined(_SHARP_SUPPORT_)*/
    return mpi_errno;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_MVP_offload_comm_setup_scatter(
    MPIR_Comm *comm)
{
    int mpi_errno = MPI_SUCCESS;
#if defined(_SHARP_SUPPORT_)
    mpi_errno = MPIR_MVP_sharp_comm_setup_scatter(comm);
#endif /*defined(_SHARP_SUPPORT_)*/
    return mpi_errno;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_MVP_offload_comm_setup_scatterv(
    MPIR_Comm *comm)
{
    int mpi_errno = MPI_SUCCESS;
#if defined(_SHARP_SUPPORT_)
    mpi_errno = MPIR_MVP_sharp_comm_setup_scatterv(comm);
#endif /*defined(_SHARP_SUPPORT_)*/
    return mpi_errno;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_MVP_offload_comm_setup_reduce(
    MPIR_Comm *comm)
{
    int mpi_errno = MPI_SUCCESS;
#if defined(_SHARP_SUPPORT_)
    mpi_errno = MPIR_MVP_sharp_comm_setup_reduce(comm);
#endif /*defined(_SHARP_SUPPORT_)*/
    return mpi_errno;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_MVP_offload_comm_setup_iallreduce(
    MPIR_Comm *comm)
{
    int mpi_errno = MPI_SUCCESS;
#if defined(_SHARP_SUPPORT_)
    mpi_errno = MPIR_MVP_sharp_comm_setup_iallreduce(comm);
#endif /*defined(_SHARP_SUPPORT_)*/
    return mpi_errno;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_MVP_offload_comm_setup_ireduce(
    MPIR_Comm *comm)
{
    int mpi_errno = MPI_SUCCESS;
#if defined(_SHARP_SUPPORT_)
    mpi_errno = MPIR_MVP_sharp_comm_setup_ireduce(comm);
#endif /*defined(_SHARP_SUPPORT_)*/
    return mpi_errno;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_MVP_offload_comm_setup_ibarrier(
    MPIR_Comm *comm)
{
    int mpi_errno = MPI_SUCCESS;
#if defined(_SHARP_SUPPORT_)
    mpi_errno = MPIR_MVP_sharp_comm_setup_ibarrier(comm);
#endif /*defined(_SHARP_SUPPORT_)*/
    return mpi_errno;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_MVP_offload_comm_setup_ibcast(
    MPIR_Comm *comm)
{
    int mpi_errno = MPI_SUCCESS;
#if defined(_SHARP_SUPPORT_)
    mpi_errno = MPIR_MVP_sharp_comm_setup_ibcast(comm);
#endif /*defined(_SHARP_SUPPORT_)*/
    return mpi_errno;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_MVP_offload_coll_init()
{
    int mpi_errno = MPI_SUCCESS;

    /* Use abstractions to remove dependency on OFED */
#if defined(_SHARP_SUPPORT_)
    mpi_errno = mvp_sharp_dlopen_init();
    MPIR_ERR_CHECK(mpi_errno);
#endif /*defined(_SHARP_SUPPORT_)*/

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
#endif /* _MVP_OFFLOAD_COLL_INIT_H_ */
