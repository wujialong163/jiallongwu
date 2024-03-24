/* Copyright (c) 2001-2024, The Ohio State University. All rights
 * reserved.
 *
 * This file is part of the MVAPICH software package developed by the
 * team members of The Ohio State University's Network-Based Computing
 * Laboratory (NBCL), headed by Professor Dhabaleswar K. (DK) Panda.
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level MVAPICH directory.
 */
/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  (C) 2001 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpidi_ch3_impl.h"
#include "mpiimpl.h"
#include "mvp_ch3_shmem.h"

#define NULL_CONTEXT_ID -1

int MPIDI_CH3I_comm_create(MPIR_Comm *comm)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_TERSE_STATE_DECL(MPID_STATE_MPIDI_CH3I_COMM_CREATE);
    MPIR_FUNC_TERSE_ENTER(MPID_STATE_MPIDI_CH3I_COMM_CREATE);

    /* Use the VC's eager threshold by default if it is not set. */
    if (comm->hints[MPIR_COMM_HINT_EAGER_THRESH] == 0) {
        comm->hints[MPIR_COMM_HINT_EAGER_THRESH] = -1;
    }

    MPIR_pof2_comm(comm, comm->local_size, comm->rank);

    MPIR_FUNC_TERSE_EXIT(MPID_STATE_MPIDI_CH3I_COMM_CREATE);
    return mpi_errno;
}

int MPIDI_CH3I_comm_destroy(MPIR_Comm *comm)
{
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_TERSE_STATE_DECL(MPID_STATE_MPIDI_CH3I_COMM_DESTROY);
    MPIR_FUNC_TERSE_ENTER(MPID_STATE_MPIDI_CH3I_COMM_DESTROY);

    MPIR_FUNC_TERSE_EXIT(MPID_STATE_MPIDI_CH3I_COMM_DESTROY);
    return mpi_errno;
}
