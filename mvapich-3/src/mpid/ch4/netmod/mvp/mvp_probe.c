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
#include "../ofi/ofi_probe.h"
#elif HAVE_CH4_NETMOD_UCX
#include "../ucx/ucx_probe.h"
#endif

int MPIDI_MVP_smp_mpi_iprobe(int source, int tag, MPIR_Comm *comm,
                             int context_offset, MPIDI_av_entry_t *addr,
                             int *flag, MPI_Status *status);
int MPIDI_MVP_smp_mpi_improbe(int source, int tag, MPIR_Comm *comm,
                              int context_offset, MPIDI_av_entry_t *addr,
                              int *flag, MPIR_Request **mesage,
                              MPI_Status *status);

int MPIDI_MVP_mpi_improbe(int source, int tag, MPIR_Comm *comm,
                          int context_offset, MPIDI_av_entry_t *addr, int *flag,
                          MPIR_Request **message, MPI_Status *status)
{
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_MPI_IMPROBE);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_MPI_IMPROBE);

    if (unlikely(MPI_ANY_SOURCE == source) && mvp_network_init &&
        (MVP_USE_PT2PT_SHMEM || MVP_USE_SHARED_MEM)) {
        /* anysrc probe - need to check both */
        mpi_errno = MPIDI_NM_mpi_improbe(source, tag, comm, context_offset,
                                         addr, flag, message, status);
        if (!*flag) {
            MPIR_ERR_CHECK(mpi_errno);
            mpi_errno = MPIDI_MVP_smp_mpi_improbe(
                source, tag, comm, context_offset, addr, flag, message, status);
        }
    } else if ((!MVP_USE_PT2PT_SHMEM || !MVP_USE_SHARED_MEM ||
                !MPIDI_av_is_local(addr)) &&
               mvp_network_init) {
        /* inter-node recv - probe the network via netmod */
        mpi_errno = MPIDI_NM_mpi_improbe(source, tag, comm, context_offset,
                                         addr, flag, message, status);
    } else {
        mpi_errno = MPIDI_MVP_smp_mpi_improbe(source, tag, comm, context_offset,
                                              addr, flag, message, status);
    }
    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_MPI_IMPROBE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIDI_MVP_mpi_iprobe(int source, int tag, MPIR_Comm *comm,
                         int context_offset, MPIDI_av_entry_t *addr, int *flag,
                         MPI_Status *status)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_MPI_IPROBE);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_MPI_IPROBE);

    if (unlikely(MPI_ANY_SOURCE == source) && mvp_network_init &&
        (MVP_USE_PT2PT_SHMEM || MVP_USE_SHARED_MEM)) {
        /* anysrc probe - need to check both */
        mpi_errno = MPIDI_NM_mpi_iprobe(source, tag, comm, context_offset, addr,
                                        flag, status);
        if (!*flag) {
            MPIR_ERR_CHECK(mpi_errno);
            mpi_errno = MPIDI_MVP_smp_mpi_iprobe(
                source, tag, comm, context_offset, addr, flag, status);
        }
    } else if ((!MVP_USE_PT2PT_SHMEM || !MVP_USE_SHARED_MEM ||
                !MPIDI_av_is_local(addr)) &&
               mvp_network_init) {
        /* inter-node recv - probe the network via netmod */
        mpi_errno = MPIDI_NM_mpi_iprobe(source, tag, comm, context_offset, addr,
                                        flag, status);
    } else {
        mpi_errno = MPIDI_MVP_smp_mpi_iprobe(source, tag, comm, context_offset,
                                             addr, flag, status);
    }
    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_MPI_IPROBE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
