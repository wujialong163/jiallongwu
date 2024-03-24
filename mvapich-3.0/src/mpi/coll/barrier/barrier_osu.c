/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *
 *  (C) 2001 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

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
#include "mvp_coll_shmem.h"
#include "barrier_tuning.h"
#if defined(CKPT)
#include <cr.h>
#endif

/*
=== BEGIN_MPI_T_MVP_CVAR_INFO_BLOCK ===

cvars:
    - name        : MVP_USE_SHMEM_BARRIER
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This parameter can be used to turn off shared memory based
        MPI_Barrier for OFA-IB-CH3 over IBA by setting this to 0.

    - name        : MVP_USE_SOCKET_AWARE_BARRIER
      category    : COLLECTIVE
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

=== END_MPI_T_MVP_CVAR_INFO_BLOCK ===
*/

/* This is the default implementation of the barrier operation.  The
   algorithm is:

   Algorithm: MPI_Barrier

   We use pairwise exchange with recursive doubling algorithm
   described in:
   R. Gupta, V. Tipparaju, J. Nieplocha and D.K. Panda,
   "Efficient Barrier using Remote Memory Operations on VIA-Based Clusters",
   IEEE Cluster Computing, 2002

   Possible improvements:

   End Algorithm: MPI_Barrier

   This is an intracommunicator barrier only!
*/

/* not declared static because it is called in ch3_comm_connect/accept */
int MPIR_Barrier_intra_MVP(MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int size;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;

    size = comm_ptr->local_size;
    /* Trivial barriers return immediately */
    if (size == 1)
        return MPI_SUCCESS;

    if (MVP_USE_SHARED_MEM && MVP_USE_SHMEM_BARRIER &&
        comm_ptr->dev.ch.shmem_coll_ok == 1) {
        if (MVP_ENABLE_TOPO_AWARE_COLLECTIVES && MVP_USE_TOPO_AWARE_BARRIER &&
            comm_ptr->dev.ch.topo_coll_ok == 1) {
            mpi_errno = MPIR_topo_aware_shmem_barrier_MVP(comm_ptr, errflag);
        } else if (MVP_USE_SOCKET_AWARE_BARRIER &&
                   comm_ptr->dev.ch.use_intra_sock_comm == 1) {
            mpi_errno = MPIR_socket_aware_shmem_barrier_MVP(comm_ptr, errflag);
        } else {
            mpi_errno = MPIR_shmem_barrier_MVP(comm_ptr, errflag);
        }
    } else {
        mpi_errno = MPIR_Pairwise_Barrier_MVP(comm_ptr, errflag);
    }

    if (mpi_errno) {
        /* for communication errors, just record the error but continue */
        *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
        MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
    }

    return mpi_errno;
}

int MPIR_Barrier_MVP(MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_T_PVAR_COMM_COUNTER_INC(MVP, mvp_coll_barrier_subcomm, 1, comm_ptr);
    MPIR_T_PVAR_COMM_TIMER_START(MVP, mvp_coll_timer_barrier_subcomm, comm_ptr);
    mpi_errno = MPIR_Barrier_intra_MVP(comm_ptr, errflag);
    MPIR_ERR_CHECK(mpi_errno);
fn_exit:
    MPIR_T_PVAR_COMM_TIMER_END(MVP, mvp_coll_timer_barrier_subcomm, comm_ptr);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
