/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda. For detailed
 * copyright and licensing information, please refer to the copyright file
 * COPYRIGHT in the top level MVAPICH directory.
 */

/*
 * define this to avoid breaking the old implementations when we include the
 * implmentation headers that include our overrides
 */
#define _MVP_INTERNAL_DISABLE_OVERRIDES_
#include "mpidimpl.h"
#include "mvp_util.h"
/* base inline definitions of the MPIDI_NM_mpi_recv function */
#ifdef HAVE_CH4_NETMOD_OFI
#include "../ofi/ofi_recv.h"
#elif HAVE_CH4_NETMOD_UCX
#include "../ucx/ucx_recv.h"
#endif

/* forward declarations for this file only */
int MPIDI_MVP_smp_mpi_recv(void *buf, MPI_Aint count, MPI_Datatype datatype,
                           int rank, int tag, MPIR_Comm *comm,
                           int context_offset, MPIDI_av_entry_t *addr,
                           MPI_Status *status, MPIR_Request **request);

int MPIDI_MVP_smp_mpi_imrecv(void *buf, MPI_Aint count, MPI_Datatype datatype,
                             MPIR_Request *message);

int MPIDI_MVP_smp_mpi_cancel_recv(MPIR_Request *rreq);

MPL_STATIC_INLINE_PREFIX int MPIDI_MVP_anysrc_irecv(
    void *buf, MPI_Aint count, MPI_Datatype datatype, int rank, int tag,
    MPIR_Comm *comm, int context_offset, MPIDI_av_entry_t *addr,
    MPI_Status *status, MPIR_Request **request)
{
    int flag = 0;
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_ANYSRC_IRECV);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_ANYSRC_IRECV);

    /* only applicable for anysrc recv */
    MPIR_Assert(MPI_ANY_SOURCE == rank);

    /* check if the network already recieved this request */
    mpi_errno = MPIDI_NM_mpi_improbe(rank, tag, comm, context_offset, addr,
                                     &flag, request, status);
    MPIR_ERR_CHECK(mpi_errno);

    /* message has been recieved on the network */
    if (flag) {
        MPIR_Assert((*request)->kind == MPIR_REQUEST_KIND__MPROBE);
        (*request)->kind = MPIR_REQUEST_KIND__RECV;
        mpi_errno = MPIDI_NM_mpi_imrecv(buf, count, datatype, *request);
        MPIR_ERR_CHECK(mpi_errno);
        MPIR_Assert(*request);
        goto fn_exit;
    } else {
        /* post this to our shmem recvq */
        mpi_errno =
            MPIDI_MVP_smp_mpi_recv(buf, count, datatype, rank, tag, comm,
                                   context_offset, addr, status, request);
        MPIR_ERR_CHECK(mpi_errno);
        MPIR_Assert(*request);
    }

fn_exit:
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_ANYSRC_IRECV);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIDI_MVP_mpi_imrecv(void *buf, MPI_Aint count, MPI_Datatype datatype,
                         MPIR_Request *message)
{
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_MPI_IMRECV);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_MPI_IMRECV);

    if (!MPIDI_MVP_REQUEST_FROM_MPICH(message) || !MVP_USE_PT2PT_SHMEM ||
        !MVP_USE_SHARED_MEM) {
        /* inter-node recv: use netmod level recv (or shmem disabled) */
        mpi_errno = MPIDI_NM_mpi_imrecv(buf, count, datatype, message);
    } else {
        /* intra-node recv: use MVAPICH smp recv */
        mpi_errno = MPIDI_MVP_smp_mpi_imrecv(buf, count, datatype, message);
    }
    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_MPI_IMRECV);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIDI_MVP_mpi_recv(void *buf, MPI_Aint count, MPI_Datatype datatype,
                       int rank, int tag, MPIR_Comm *comm,
                       int context_offset, MPIDI_av_entry_t *addr,
                       MPI_Status *status, MPIR_Request **request)
{
    /* TODO: Replace or reimplement these bucket macros */

    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_MPI_RECV);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_MPI_RECV);

    /* MPIR_T_PVAR_COUNTER_BUCKET_INC(MVP,mvp_pt2pt_mpid_recv,count,datatype);
     */
    *request = NULL;

    if (unlikely(rank == MPI_ANY_SOURCE) && mvp_network_init &&
        (MVP_USE_PT2PT_SHMEM || MVP_USE_SHARED_MEM)) {
        mpi_errno =
            MPIDI_MVP_anysrc_irecv(buf, count, datatype, rank, tag, comm,
                                   context_offset, addr, status, request);
    } else if (!MPIDI_rank_is_local(rank, comm) || !MVP_USE_PT2PT_SHMEM ||
               !MVP_USE_SHARED_MEM) {
        /* inter-node recv: use netmod level recv (or shmem disabled) */
        mpi_errno = MPIDI_NM_mpi_recv(buf, count, datatype, rank, tag, comm,
                                      context_offset, addr, status, request);
    } else {
        /* intra-node recv: use MVAPICH smp recv */
        mpi_errno =
            MPIDI_MVP_smp_mpi_recv(buf, count, datatype, rank, tag, comm,
                                   context_offset, addr, status, request);
    }
    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_MPI_RECV);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIDI_MVP_mpi_irecv(void *buf, MPI_Aint count, MPI_Datatype datatype,
                       int rank, int tag, MPIR_Comm *comm, int context_offset,
                       MPIDI_av_entry_t *addr, MPIR_Request **request,
                       MPIR_Request *partner)
{
    int mpi_errno = MPI_SUCCESS;
    MPI_Status status;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_MPI_IRECV);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_MPI_IRECV);

    mpi_errno = MPIDI_MVP_mpi_recv(buf, count, datatype, rank, tag, comm,
                                   context_offset, addr, &status, request);
    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_MPI_IRECV);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIDI_MVP_mpi_cancel_recv(MPIR_Request *rreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_MPI_CANCEL_RECV);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_MPI_CANCEL_RECV);

    if (!MPIR_Request_is_complete(rreq)) {
        /* not sure if the persistent request check is required, but keeping it
         * for safety since it would make our req struct garbage */
        if (rreq->kind == MPIR_REQUEST_KIND__PREQUEST_RECV ||
            NULL == MPIDI_MVP_REQUEST_FROM_MPICH(rreq) ||
            !MVP_USE_PT2PT_SHMEM || !MVP_USE_SHARED_MEM) {
            mpi_errno = MPIDI_NM_mpi_cancel_recv(rreq);
        } else {
            mpi_errno = MPIDI_MVP_smp_mpi_cancel_recv(rreq);
        }
        MPIR_ERR_CHECK(mpi_errno);
    }

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_MPI_CANCEL_RECV);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
