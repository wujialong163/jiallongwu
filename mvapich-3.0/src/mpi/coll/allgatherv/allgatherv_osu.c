/* -*- Mode: C; c-basic-offset:4 ; -*- */
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
/*
 *
 *  (C) 2001 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mvp_coll_shmem.h"
#include "allgatherv_tuning.h"

/* This is the default implementation of allgatherv. The algorithm is:

   Algorithm: MPI_Allgatherv

   For short messages and non-power-of-two no. of processes, we use
   the algorithm from the Jehoshua Bruck et al IEEE TPDS Nov 97
   paper. It is a variant of the disemmination algorithm for
   barrier. It takes ceiling(lg p) steps.

   Cost = lgp.alpha + n.((p-1)/p).beta
   where n is total size of data gathered on each process.

   For short or medium-size messages and power-of-two no. of
   processes, we use the recursive doubling algorithm.

   Cost = lgp.alpha + n.((p-1)/p).beta

   TODO: On TCP, we may want to use recursive doubling instead of the Bruck
   algorithm in all cases because of the pairwise-exchange property of
   recursive doubling (see Benson et al paper in Euro PVM/MPI
   2003).

   For long messages or medium-size messages and non-power-of-two
   no. of processes, we use a ring algorithm. In the first step, each
   process i sends its contribution to process i+1 and receives
   the contribution from process i-1 (with wrap-around). From the
   second step onwards, each process i forwards to process i+1 the
   data it received from process i-1 in the previous step. This takes
   a total of p-1 steps.

   Cost = (p-1).alpha + n.((p-1)/p).beta

   Possible improvements:

   End Algorithm: MPI_Allgatherv
*/

/*
=== BEGIN_MPI_T_MVP_CVAR_INFO_BLOCK ===

cvars:
    - name        : MVP_ALLGATHERV_COLLECTIVE_ALGORITHM
      alias       : MVP_INTER_ALLGATHERV_TUNING
      category    : COLLECTIVE
      type        : enum
      default     : UNSET
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : |-
        Variable to select the allgatherv collective algorithm.
        UNSET           - Internal algorithm selection.
        RD              - Pairs of processes exchange data in a doubling
                        pattern.
        BRUCK           - A communication-efficient algorithm based on a
                        generalized butterfly network.
        RING            - Data is passed around a virtual ring in a fixed
                        pattern.
        RING_CYCLIC     - Data is passed around a virtual ring in a cyclic
                        pattern.

=== END_MPI_T_MVP_CVAR_INFO_BLOCK ===
*/

MVP_Allgatherv_fn_t MVP_Allgatherv_function = NULL;

/* MPIR_Allgatherv performs an allgatherv using point-to-point
   messages.  This is intended to be used by device-specific
   implementations of allgatherv.  In all other cases
   MPIR_Allgatherv_impl should be used. */
#undef FUNCNAME
#define FUNCNAME MPIR_Allgatherv_MVP
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPIR_Allgatherv_MVP(const void *sendbuf, int sendcount,
                        MPI_Datatype sendtype, void *recvbuf,
                        const int *recvcounts, const int *displs,
                        MPI_Datatype recvtype, MPIR_Comm *comm_ptr,
                        MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;
    int range = 0, comm_size, total_count, recvtype_size, i;
    int range_threshold = 0;
    MPI_Aint nbytes = 0;

    comm_size = comm_ptr->local_size;
    total_count = 0;
    for (i = 0; i < comm_size; i++)
        total_count += recvcounts[i];

    if (total_count == 0)
        goto fn_exit;

    MPIR_Datatype_get_size_macro(recvtype, recvtype_size);
    nbytes = (MPI_Aint)total_count * recvtype_size;

    /* Search for the corresponding system size inside the tuning table */
    while ((range < (mvp_size_allgatherv_tuning_table - 1)) &&
           (comm_size > mvp_allgatherv_thresholds_table[range].numproc)) {
        range++;
    }
    /* Search for corresponding inter-leader function */
    while ((range_threshold <
            (mvp_allgatherv_thresholds_table[range].size_inter_table - 1)) &&
           (nbytes > comm_size * mvp_allgatherv_thresholds_table[range]
                                     .inter_leader[range_threshold]
                                     .max) &&
           (mvp_allgatherv_thresholds_table[range]
                .inter_leader[range_threshold]
                .max != -1)) {
        range_threshold++;
    }
    /* Set inter-leader pt */
    MVP_Allgatherv_function = mvp_allgatherv_thresholds_table[range]
                                  .inter_leader[range_threshold]
                                  .allgatherv_fn;

    if (MVP_Allgatherv_function == &MPIR_Allgatherv_Rec_Doubling_MVP) {
        if (!(comm_size & (comm_size - 1))) {
            mpi_errno = MPIR_Allgatherv_Rec_Doubling_MVP(
                sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs,
                recvtype, comm_ptr, errflag);
        } else {
            mpi_errno = MPIR_Allgatherv_Bruck_MVP(sendbuf, sendcount, sendtype,
                                                  recvbuf, recvcounts, displs,
                                                  recvtype, comm_ptr, errflag);
        }
    } else {
        mpi_errno = MVP_Allgatherv_function(sendbuf, sendcount, sendtype,
                                            recvbuf, recvcounts, displs,
                                            recvtype, comm_ptr, errflag);
    }

    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
