/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda. For detailed
 * copyright and licensing information, please refer to the copyright file
 * COPYRIGHT in the top level MVAPICH directory.
 */

#include "mvp_smp_impl.h"
#include "mvp_vc.h"

int MPIDI_MVP_smp_init_vcs();
int MPIDI_MVP_smp_shmem_init();
int MPIDI_MVP_pt2pt_shmem_finalize();

MPIDI_MVP_ep_t **MPIDI_MVP_vc_table;

int MPIDI_MVP_smp_init()
{
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_SMP_INIT);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_INIT);

    mpi_errno = MPIDI_MVP_smp_init_vcs();
    MPIR_ERR_CHECK(mpi_errno);

    mpi_errno = MPIDI_MVP_smp_shmem_init();
    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_INIT);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIDI_MVP_smp_finalize()
{
    int mpi_errno = MPI_SUCCESS;
    int i;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_SMP_FINALIZE);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_FINALIZE);

    /* TODO: free anything or call finalize routines here */
    /* TODO: need to declare this */
    MPIDI_MVP_pt2pt_shmem_finalize();

    /* free all VCs last - TODO: offload to VC finalize */
    for (i = 0; i < MPIR_Process.size; ++i) {
        MPL_free(MPIDI_MVP_vc_table[i]);
    }
    MPL_free(MPIDI_MVP_vc_table);

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_FINALIZE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
