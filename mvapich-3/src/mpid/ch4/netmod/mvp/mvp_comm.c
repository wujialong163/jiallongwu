/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda. For detailed
 * copyright and licensing information, please refer to the copyright file
 * COPYRIGHT in the top level MVAPICH directory.
 */

#include "mpidimpl.h"
#include "mvp_coll_shmem.h"

/* This function initializes MVP-specific variables before a communicator is
 * given to the application */
int MPIDI_MVP_mpi_comm_commit_post_hook(MPIR_Comm *comm)
{
    int mpi_errno = MPI_SUCCESS;

    /*
     * these don't always do anything but incase they do eventually we should
     * call them first
     */
#ifdef HAVE_CH4_NETMOD_OFI
    mpi_errno = MPIDI_OFI_mpi_comm_commit_post_hook(comm);
#endif
#ifdef HAVE_CH4_NETMOD_UCX
    mpi_errno = MPIDI_UCX_mpi_comm_commit_post_hook(comm);
#endif
    MPIR_ERR_CHECK(mpi_errno);

    /* Initialize pof2 variables for every communicator. This is necessary for
     * many collective algorithms to work correctly */
    MPIR_pof2_comm(comm, comm->local_size, comm->rank);
    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
