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
/* TODO: I don't think this applies here, should be removed */
#define _MVP_INTERNAL_DISABLE_OVERRIDES_
#include "mvp_util.h"
#include "mvp_smp_impl.h"

/* base inline definitions of the MPIDI_NM_mpi_send function */
/* TODO: do we need others? */
#ifdef HAVE_CH4_NETMOD_OFI
#include "../ofi/ofi_send.h"
#elif HAVE_CH4_NETMOD_UCX
#include "../ucx/ucx_send.h"
#endif

#define MAX_PROGRESS_HOOKS 4
extern mvp_smp_info_t mvp_smp_info;

int MPIDI_MVP_Progress(int is_blocking, MPID_Progress_state *state);
int MPIDI_MVP_Progress_test();
int MPIDI_MVP_SMP_read_progress();
int MPIDI_MVP_SMP_write_progress();
void MPIDI_MVP_MRAILI_Process_rndv();
#define MPIDI_MVP_Progress_wait(state) MPIDI_MVP_Progress(TRUE, state)
int MPIDI_MVP_smp_progress_wait(int is_blocking, MPID_Progress_state *state);
int MPIDI_MVP_smp_progress_test();

int MPIDI_MVP_progress(int vci, int blocking)
{
    int mpi_errno = MPI_SUCCESS;

    /* TODO: Remove this fake state and get a real one!!!! */
    MPID_Progress_state *state = NULL;

    /* MPICH channel level inter-node progress - do this first */
    MPIDI_MVP_NETWORK_PATH_ENTER
#ifdef HAVE_CH4_NETMOD_OFI
    mpi_errno = MPIDI_OFI_progress(vci, blocking);
#endif
#ifdef HAVE_CH4_NETMOD_UCX
    mpi_errno = MPIDI_UCX_progress(vci, blocking);
#endif
    MPIR_ERR_CHECK(mpi_errno);
    MPIDI_MVP_NETWORK_PATH_EXIT

    /* MVAPICH intra-node progress */
    /* TODO: combine functions */
    if (MVP_USE_PT2PT_SHMEM) {
        if (blocking) {
            mpi_errno = MPIDI_MVP_smp_progress_wait(blocking, state);
        } else {
            mpi_errno = MPIDI_MVP_smp_progress_test();
        }
        MPIR_ERR_CHECK(mpi_errno);
    }

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
