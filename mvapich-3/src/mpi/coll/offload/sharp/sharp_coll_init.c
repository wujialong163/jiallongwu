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

#include "mpiimpl.h"
#if defined(_SHARP_SUPPORT_)
#include "mvp_sharp.h"

int MPIR_MVP_sharp_comm_setup_allreduce(MPIR_Comm *comm)
{
    int mpi_errno = MPI_SUCCESS;

    comm->dev.ch.allreduce_coll_count++;
    if (MVP_ENABLE_SHARP && MVP_ENABLE_SHARP_ALLREDUCE &&
        (comm->dev.ch.is_sharp_ok == 0) && (comm->dev.ch.shmem_coll_ok == 1) &&
        (comm->dev.ch.allreduce_coll_count >= MVP_TWO_LEVEL_COMM_THRESHOLD)) {
        disable_split_comm(pthread_self());
        mpi_errno =
            mvp_create_sharp_comm(comm->handle, comm->local_size, comm->rank);
        MPIR_ERR_CHECK(mpi_errno);
        enable_split_comm(pthread_self());
    }
fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIR_MVP_sharp_comm_setup_reduce(MPIR_Comm *comm)
{
    int mpi_errno = MPI_SUCCESS;

    comm->dev.ch.reduce_coll_count++;
    if (MVP_ENABLE_SHARP && MVP_ENABLE_SHARP_REDUCE &&
        (comm->dev.ch.is_sharp_ok == 0) && (comm->dev.ch.shmem_coll_ok == 1) &&
        (comm->dev.ch.reduce_coll_count >= MVP_TWO_LEVEL_COMM_THRESHOLD)) {
        disable_split_comm(pthread_self());
        mpi_errno =
            mvp_create_sharp_comm(comm->handle, comm->local_size, comm->rank);
        MPIR_ERR_CHECK(mpi_errno);
        enable_split_comm(pthread_self());
    }
fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIR_MVP_sharp_comm_setup_scatter(MPIR_Comm *comm)
{
    int mpi_errno = MPI_SUCCESS;

    comm->dev.ch.scatter_coll_count++;
    if (MVP_ENABLE_SHARP && MVP_ENABLE_SHARP_SCATTER &&
        (comm->dev.ch.is_sharp_ok == 0) && (comm->dev.ch.shmem_coll_ok == 1) &&
        (comm->dev.ch.scatter_coll_count >= MVP_TWO_LEVEL_COMM_THRESHOLD)) {
        disable_split_comm(pthread_self());
        mpi_errno =
            mvp_create_sharp_comm(comm->handle, comm->local_size, comm->rank);
        MPIR_ERR_CHECK(mpi_errno);
        enable_split_comm(pthread_self());
    }
fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIR_MVP_sharp_comm_setup_scatterv(MPIR_Comm *comm)
{
    int mpi_errno = MPI_SUCCESS;

    comm->dev.ch.scatterv_coll_count++;
    if (MVP_ENABLE_SHARP && MVP_ENABLE_SHARP_SCATTERV &&
        (comm->dev.ch.is_sharp_ok == 0) && (comm->dev.ch.shmem_coll_ok == 1) &&
        (comm->dev.ch.scatterv_coll_count >= MVP_TWO_LEVEL_COMM_THRESHOLD)) {
        disable_split_comm(pthread_self());
        mpi_errno =
            mvp_create_sharp_comm(comm->handle, comm->local_size, comm->rank);
        MPIR_ERR_CHECK(mpi_errno);
        enable_split_comm(pthread_self());
    }
fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIR_MVP_sharp_comm_setup_bcast(MPIR_Comm *comm)
{
    int mpi_errno = MPI_SUCCESS;

    comm->dev.ch.bcast_coll_count++;
    if (MVP_ENABLE_SHARP && MVP_ENABLE_SHARP_BCAST &&
        (comm->dev.ch.is_sharp_ok == 0) && (comm->dev.ch.shmem_coll_ok == 1) &&
        (comm->dev.ch.bcast_coll_count >= MVP_TWO_LEVEL_COMM_THRESHOLD)) {
        disable_split_comm(pthread_self());
        mpi_errno =
            mvp_create_sharp_comm(comm->handle, comm->local_size, comm->rank);
        MPIR_ERR_CHECK(mpi_errno);
        enable_split_comm(pthread_self());
    }
fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIR_MVP_sharp_comm_setup_barrier(MPIR_Comm *comm)
{
    int mpi_errno = MPI_SUCCESS;

    comm->dev.ch.barrier_coll_count++;
    if (MVP_ENABLE_SHARP && MVP_ENABLE_SHARP_BARRIER &&
        (comm->dev.ch.is_sharp_ok == 0) && (comm->dev.ch.shmem_coll_ok == 1) &&
        (comm->dev.ch.barrier_coll_count >= MVP_TWO_LEVEL_COMM_THRESHOLD)) {
        disable_split_comm(pthread_self());
        mpi_errno =
            mvp_create_sharp_comm(comm->handle, comm->local_size, comm->rank);
        MPIR_ERR_CHECK(mpi_errno);
        enable_split_comm(pthread_self());
    }
fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIR_MVP_sharp_comm_setup_iallreduce(MPIR_Comm *comm)
{
    int mpi_errno = MPI_SUCCESS;

    comm->dev.ch.iallreduce_coll_count++;
    if (MVP_ENABLE_SHARP && MVP_ENABLE_SHARP_IALLREDUCE &&
        (comm->dev.ch.is_sharp_ok == 0) && (comm->dev.ch.shmem_coll_ok == 1) &&
        (comm->dev.ch.iallreduce_coll_count >= MVP_TWO_LEVEL_COMM_THRESHOLD)) {
        disable_split_comm(pthread_self());
        mpi_errno =
            mvp_create_sharp_comm(comm->handle, comm->local_size, comm->rank);
        MPIR_ERR_CHECK(mpi_errno);
        enable_split_comm(pthread_self());
    }
fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIR_MVP_sharp_comm_setup_ibarrier(MPIR_Comm *comm)
{
    int mpi_errno = MPI_SUCCESS;

    comm->dev.ch.ibarrier_coll_count++;
    if (MVP_ENABLE_SHARP && MVP_ENABLE_SHARP_IBARRIER &&
        (comm->dev.ch.is_sharp_ok == 0) && (comm->dev.ch.shmem_coll_ok == 1) &&
        (comm->dev.ch.ibarrier_coll_count >= MVP_TWO_LEVEL_COMM_THRESHOLD)) {
        disable_split_comm(pthread_self());
        mpi_errno =
            mvp_create_sharp_comm(comm->handle, comm->local_size, comm->rank);
        MPIR_ERR_CHECK(mpi_errno);
        enable_split_comm(pthread_self());
    }
fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIR_MVP_sharp_comm_setup_ibcast(MPIR_Comm *comm)
{
    int mpi_errno = MPI_SUCCESS;

    comm->dev.ch.ibcast_coll_count++;
    if (MVP_ENABLE_SHARP && MVP_ENABLE_SHARP_IBCAST &&
        (comm->dev.ch.is_sharp_ok == 0) && (comm->dev.ch.shmem_coll_ok == 1) &&
        (comm->dev.ch.ibcast_coll_count >= MVP_TWO_LEVEL_COMM_THRESHOLD)) {
        disable_split_comm(pthread_self());
        mpi_errno =
            mvp_create_sharp_comm(comm->handle, comm->local_size, comm->rank);
        MPIR_ERR_CHECK(mpi_errno);
        enable_split_comm(pthread_self());
    }
fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIR_MVP_sharp_comm_setup_ireduce(MPIR_Comm *comm)
{
    int mpi_errno = MPI_SUCCESS;

    comm->dev.ch.ireduce_coll_count++;
    if (MVP_ENABLE_SHARP && MVP_ENABLE_SHARP_IREDUCE &&
        (comm->dev.ch.is_sharp_ok == 0) && (comm->dev.ch.shmem_coll_ok == 1) &&
        (comm->dev.ch.ireduce_coll_count >= MVP_TWO_LEVEL_COMM_THRESHOLD)) {
        disable_split_comm(pthread_self());
        mpi_errno =
            mvp_create_sharp_comm(comm->handle, comm->local_size, comm->rank);
        MPIR_ERR_CHECK(mpi_errno);
        enable_split_comm(pthread_self());
    }
fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
#endif /*defined(_SHARP_SUPPORT_)*/
