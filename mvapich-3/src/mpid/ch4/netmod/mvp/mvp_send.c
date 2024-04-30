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

/* base inline definitions of the MPIDI_NM_mpi_recv function */
#ifdef HAVE_CH4_NETMOD_OFI
#include "../ofi/ofi_send.h"
#elif HAVE_CH4_NETMOD_UCX
#include "../ucx/ucx_send.h"
#endif

int MPIDI_MVP_smp_mpi_send(const void *buf, MPI_Aint count,
                           MPI_Datatype datatype, int rank, int tag,
                           MPIR_Comm *comm, int context_offset,
                           MPIDI_av_entry_t *addr, MPIR_Request **request);
int MPIDI_MVP_mpi_send(const void *buf, MPI_Aint count, MPI_Datatype datatype,
                       int rank, int tag, MPIR_Comm *comm, int context_offset,
                       MPIDI_av_entry_t *addr, MPIR_Request **request);
int MPIDI_MVP_mpi_isend(const void *buf, MPI_Aint count, MPI_Datatype datatype,
                        int rank, int tag, MPIR_Comm *comm, int context_offset,
                        MPIDI_av_entry_t *addr, MPIR_Request **request);

int MPIDI_MVP_mpi_send_coll(const void *buf, MPI_Aint count, MPI_Datatype datatype,
                       int rank, int tag, MPIR_Comm *comm, int context_offset,
                       MPIDI_av_entry_t *addr, MPIR_Request **request, 
                       MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS; 

    mpi_errno = MPIDI_MVP_mpi_send(buf, count, datatype, rank, tag, comm, context_offset,
            addr, request);
    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIDI_MVP_mpi_isend_coll(const void *buf, MPI_Aint count,
                             MPI_Datatype datatype, int rank, int tag,
                             MPIR_Comm *comm, int context_offset,
                             MPIDI_av_entry_t *addr, MPIR_Request **request,
                             MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_MPI_ISEND_COLL);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_MPI_ISEND_COLL);

    mpi_errno = MPIDI_MVP_mpi_isend(buf, count, datatype, rank, tag, comm, context_offset,
            addr, request);
    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_MPI_ISEND_COLL);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIDI_MVP_mpi_send(const void *buf, MPI_Aint count, MPI_Datatype datatype,
                       int rank, int tag, MPIR_Comm *comm, int context_offset,
                       MPIDI_av_entry_t *addr, MPIR_Request **request)
{
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_MPI_SEND);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_MPI_SEND);

    /* inter-node send: use netmod level send (or shmem disabled) */
    if (!MPIDI_rank_is_local(rank, comm) || !MVP_USE_PT2PT_SHMEM ||
        !MVP_USE_SHARED_MEM) {
        mpi_errno = MPIDI_NM_mpi_send(buf, count, datatype, rank, tag, comm,
                                      context_offset, addr, request);
    } else {
        /* intra-node send: MVAPICH smp send */
        mpi_errno = MPIDI_MVP_smp_mpi_send(buf, count, datatype, rank, tag,
                                           comm, context_offset, addr, request);
    }
    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_MPI_SEND);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIDI_MVP_mpi_isend(const void *buf, MPI_Aint count, MPI_Datatype datatype,
                        int rank, int tag, MPIR_Comm *comm, int context_offset,
                        MPIDI_av_entry_t *addr, MPIR_Request **request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_MPI_ISEND);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_MPI_ISEND);

    mpi_errno = MPIDI_MVP_mpi_send(buf, count, datatype, rank, tag, comm,
                                   context_offset, addr, request);
    MPIR_ERR_CHECK(mpi_errno);
    if (*request == NULL) {
        *request = MPIR_Request_create_complete(MPIR_REQUEST_KIND__SEND);
    }

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_MPI_ISEND);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
