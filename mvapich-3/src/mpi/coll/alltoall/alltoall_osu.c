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
#include <math.h>
#include <unistd.h>
#include "mvp_coll_shmem.h"
#include "mvp_common_tuning.h"
#include "alltoall_tuning.h"

/*
=== BEGIN_MPI_T_MVP_CVAR_INFO_BLOCK ===

categories:
    - name : COLLECTIVE
      description: INHERITED

cvars:
   - name        : MVP_ALLTOALL_MEDIUM_MSG
     category    : COLLECTIVE
     type        : int
     default     : 16384
     class       : none
     verbosity   : MPI_T_VERBOSITY_USER_BASIC
     scope       : MPI_T_SCOPE_ALL_EQ
     description : >-
       TODO-DESC

   - name        : MVP_ALLTOALL_SMALL_MSG
     category    : COLLECTIVE
     type        : int
     default     : 2048
     class       : none
     verbosity   : MPI_T_VERBOSITY_USER_BASIC
     scope       : MPI_T_SCOPE_ALL_EQ
     description : >-
       TODO-DESC

   - name        : MVP_ALLTOALL_THROTTLE_FACTOR
     category    : COLLECTIVE
     type        : int
     default     : 32
     class       : none
     verbosity   : MPI_T_VERBOSITY_USER_BASIC
     scope       : MPI_T_SCOPE_ALL_EQ
     description : >-
       TODO-DESC

   - name        : MVP_ALLTOALL_INTRA_THROTTLE_FACTOR
     category    : COLLECTIVE
     type        : int
     default     : 32
     class       : none
     verbosity   : MPI_T_VERBOSITY_USER_BASIC
     scope       : MPI_T_SCOPE_ALL_EQ
     description : >-
       TODO-DESC

   - name        : MVP_ALLTOALL_LARGE_MSG_THROTTLE_FACTOR
     category    : COLLECTIVE
     type        : int
     default     : 4
     class       : none
     verbosity   : MPI_T_VERBOSITY_USER_BASIC
     scope       : MPI_T_SCOPE_ALL_EQ
     description : >-
       TODO-DESC

   - name        : MVP_USE_XOR_ALLTOALL
     category    : COLLECTIVE
     type        : int
     default     : 1
     class       : none
     verbosity   : MPI_T_VERBOSITY_USER_BASIC
     scope       : MPI_T_SCOPE_ALL_EQ
     description : >-
       TODO-DESC

   -  name        : MVP_ALLTOALL_COLLECTIVE_ALGORITHM
      alias       : MVP_ALLTOALL_TUNING
      category    : COLLECTIVE
      type        : enum
      default     : UNSET
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : |-
        This CVAR selects proper collective algorithm for the all-to-all
        operation.
        UNSET           - No algorithm selected.
        BRUCK           - Uses an algorithm based on Bruck's algorithm, which
                        exchanges messages between neighboring processes in a
                        hypercube-like topology.
        RD              - Performs a recurisve doubling alltoall, where each
                        process sends all its data at each step along with data
                        it received in previous steps.
        SCATTER_DEST    - Uses a scatter and gather approach, where each process
                        scatters its data to all other processes, with each
                        process specifying the destination buffer to scatter to.
        PAIRWISE        - Uses pairwise communication between processes, where
                        each process exchanges data with every other process
                        using point-to-point communication.
        INPLACE         - Performs an in-place all-to-all operation, where each
                        process sends its data to every other process and
                        receives data from every other process in the same
                        buffer.

=== END_MPI_T_MVP_CVAR_INFO_BLOCK ===
*/

/* This is the default implementation of alltoall. The algorithm is:

   Algorithm: MPI_Alltoall

   We use four algorithms for alltoall. For short messages and
   (comm_size >= 8), we use the algorithm by Jehoshua Bruck et al,
   IEEE TPDS, Nov. 1997. It is a store-and-forward algorithm that
   takes lgp steps. Because of the extra communication, the bandwidth
   requirement is (n/2).lgp.beta.

   Cost = lgp.alpha + (n/2).lgp.beta

   where n is the total amount of data a process needs to send to all
   other processes.

   For medium size messages and (short messages for comm_size < 8), we
   use an algorithm that posts all irecvs and isends and then does a
   waitall. We scatter the order of sources and destinations among the
   processes, so that all processes don't try to send/recv to/from the
   same process at the same time.

   For long messages and power-of-two number of processes, we use a
   pairwise exchange algorithm, which takes p-1 steps. We
   calculate the pairs by using an exclusive-or algorithm:
           for (i=1; i<comm_size; i++)
               dest = rank ^ i;
   This algorithm doesn't work if the number of processes is not a power of
   two. For a non-power-of-two number of processes, we use an
   algorithm in which, in step i, each process  receives from (rank-i)
   and sends to (rank+i).

   Cost = (p-1).alpha + n.beta

   where n is the total amount of data a process needs to send to all
   other processes.

   Possible improvements:

   End Algorithm: MPI_Alltoall
*/

MVP_Alltoall_fn_t MVP_Alltoall_function = NULL;

int MPIR_Alltoall_RD_MVP(const void *sendbuf, int sendcount,
                         MPI_Datatype sendtype, void *recvbuf, int recvcount,
                         MPI_Datatype recvtype, MPIR_Comm *comm_ptr,
                         MPIR_Errflag_t *errflag)
{
    if ((HANDLE_GET_KIND(sendtype) == HANDLE_KIND_BUILTIN) &&
        (HANDLE_GET_KIND(recvtype) == HANDLE_KIND_BUILTIN)) {
        return MPIR_Alltoall_ALG_MVP(sendbuf, sendcount, sendtype, recvbuf,
                                     recvcount, recvtype, comm_ptr, errflag);
    } else {
        return MPIR_Alltoall_pairwise_MVP(sendbuf, sendcount, sendtype, recvbuf,
                                          recvcount, recvtype, comm_ptr,
                                          errflag);
    }
}

int MPIR_Alltoall_index_tuned_intra_MVP(const void *sendbuf, int sendcount,
                                        MPI_Datatype sendtype, void *recvbuf,
                                        int recvcount, MPI_Datatype recvtype,
                                        MPIR_Comm *comm_ptr,
                                        MPIR_Errflag_t *errflag)
{
    int sendtype_size, recvtype_size, comm_size;
    MPI_Aint nbytes;
    char *tmp_buf = NULL;
    int mpi_errno = MPI_SUCCESS;
    int partial_sub_ok = 0;
    int conf_index = 0;
    int local_size = -1;
    int comm_size_index = 0;
    int inter_node_algo_index = 0;
    int table_min_comm_size = 0;
    int table_max_comm_size = 0;
    int table_min_inter_size = 0;
    int table_max_inter_size = 0;
    int last_inter;
    int lp2ltn; // largest power of 2 less than n
    int lp2ltn_min;
    MPI_Comm shmem_comm;
    MPIR_Comm *shmem_commptr = NULL;
    comm_size = comm_ptr->local_size;

    MPIR_Datatype_get_size_macro(sendtype, sendtype_size);
    MPIR_Datatype_get_size_macro(recvtype, recvtype_size);
    nbytes = (MPI_Aint)sendtype_size * sendcount;

#ifdef CHANNEL_PSM
    /* To avoid picking up algorithms like recursive doubling alltoall
     * if psm is used */
    if (nbytes >= ipath_max_transfer_size) {
        MVP_Alltoall_function = MPIR_Alltoall_pairwise_MVP;
        goto psm_a2a_bypass;
    }
#endif

    /* check if safe to use partial subscription mode */
    if (comm_ptr->dev.ch.shmem_coll_ok == 1 && comm_ptr->dev.ch.is_uniform) {
        shmem_comm = comm_ptr->dev.ch.shmem_comm;
        MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
        local_size = shmem_commptr->local_size;
        if (mvp_alltoall_indexed_table_ppn_conf[0] == -1) {
            /* Indicating user defined tuning */
            conf_index = 0;
            goto conf_check_end;
        }
        if (likely(MVP_ENABLE_SKIP_TUNING_TABLE_SEARCH &&
                   (nbytes <= MVP_COLL_SKIP_TABLE_THRESHOLD))) {
            /* for small messages, force Bruck or RD */
            if (comm_size * nbytes < MVP_ALLTOALL_RD_MAX_MSG_SIZE &&
                local_size < 16 && nbytes < 32) {
                MVP_Alltoall_function = MPIR_Alltoall_RD_MVP;
            } else {
                MVP_Alltoall_function = MPIR_Alltoall_bruck_MVP;
            }
            goto skip_tuning_tables;
        }

        FIND_PPN_INDEX(alltoall, local_size, conf_index, partial_sub_ok)
    }

    if (partial_sub_ok != 1) {
        conf_index = mvp_alltoall_indexed_num_ppn_conf / 2;
    }

conf_check_end:

    /* Search for the corresponding system size inside the tuning table */
    /*
     * Comm sizes progress in powers of 2.
     * Therefore comm_size can just be indexed instead
     */
    table_min_comm_size =
        mvp_alltoall_indexed_thresholds_table[conf_index][0].numproc;
    table_max_comm_size =
        mvp_alltoall_indexed_thresholds_table
            [conf_index][mvp_size_alltoall_indexed_tuning_table[conf_index] - 1]
                .numproc;

    if (comm_size < table_min_comm_size) {
        /* Comm size smaller than smallest configuration in table: use smallest
         * available */
        comm_size_index = 0;
    } else if (comm_size > table_max_comm_size) {
        /* Comm size larger than largest configuration in table: use largest
         * available */
        comm_size_index =
            mvp_size_alltoall_indexed_tuning_table[conf_index] - 1;
    } else {
        /* Comm size in between smallest and largest configuration: find closest
         * match */
        lp2ltn_min = pow(2, (int)log2(table_min_comm_size));
        if (comm_ptr->dev.ch.is_pof2) {
            comm_size_index = log2(comm_size / lp2ltn_min);
        } else {
            lp2ltn = pow(2, (int)log2(comm_size));
            comm_size_index =
                (lp2ltn < lp2ltn_min) ? 0 : log2(lp2ltn / lp2ltn_min);
        }
    }

    last_inter =
        mvp_alltoall_indexed_thresholds_table[conf_index][comm_size_index]
            .size_table -
        1;
    table_min_inter_size =
        mvp_alltoall_indexed_thresholds_table[conf_index][comm_size_index]
            .algo_table[0]
            .msg_sz;
    table_max_inter_size =
        mvp_alltoall_indexed_thresholds_table[conf_index][comm_size_index]
            .algo_table[last_inter]
            .msg_sz;

    if (nbytes < table_min_inter_size) {
        /* Msg size smaller than smallest configuration in table: use smallest
         * available */
        inter_node_algo_index = 0;
    } else if (nbytes > table_max_inter_size) {
        /* Msg size larger than largest configuration in table: use largest
         * available */
        inter_node_algo_index = last_inter;
    } else {
        /* Msg size in between smallest and largest configuration: find closest
         * match */
        if (pow(2, (int)log2(nbytes)) == nbytes) {
            inter_node_algo_index = log2(nbytes / table_min_inter_size);
        } else {
            lp2ltn = pow(2, (int)log2(nbytes));
            inter_node_algo_index = (lp2ltn < table_min_inter_size) ?
                                        0 :
                                        log2(lp2ltn / table_min_inter_size);
        }
    }

    MVP_Alltoall_function =
        mvp_alltoall_indexed_thresholds_table[conf_index][comm_size_index]
            .algo_table[inter_node_algo_index]
            .alltoall_fn;

#ifdef _MVP_CH4_OVERRIDE_
psm_a2a_bypass:
#endif
skip_tuning_tables:

    if (sendbuf != MPI_IN_PLACE) {
        mpi_errno =
            MVP_Alltoall_function(sendbuf, sendcount, sendtype, recvbuf,
                                  recvcount, recvtype, comm_ptr, errflag);
    } else {
        if (mvp_alltoall_indexed_thresholds_table[conf_index][comm_size_index]
                .in_place_algo_table[inter_node_algo_index] == 0) {
            tmp_buf = (char *)MPL_malloc(comm_size * recvcount * recvtype_size,
                                         MPL_MEM_COLL);
            mpi_errno = MPIR_Localcopy((char *)recvbuf, comm_size * recvcount,
                                       recvtype, (char *)tmp_buf,
                                       comm_size * recvcount, recvtype);

            mpi_errno =
                MVP_Alltoall_function(tmp_buf, recvcount, recvtype, recvbuf,
                                      recvcount, recvtype, comm_ptr, errflag);
            MPL_free(tmp_buf);
        } else {
            mpi_errno = MPIR_Alltoall_inplace_MVP(sendbuf, sendcount, sendtype,
                                                  recvbuf, recvcount, recvtype,
                                                  comm_ptr, errflag);
        }
    }
    return (mpi_errno);
}

int MPIR_Alltoall_MVP(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                      void *recvbuf, int recvcount, MPI_Datatype recvtype,
                      MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Alltoall_index_tuned_intra_MVP(
        sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm_ptr,
        errflag);

    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
