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
#include "mvp_common_tuning.h"
#include "bcast_tuning.h"
#include "scatter_tuning.h"

#if defined(_SHARP_SUPPORT_)
#include "mvp_sharp.h"
#endif

/*
=== BEGIN_MPI_T_MVP_CVAR_INFO_BLOCK ===

cvars:
    - name        : MVP_RED_SCAT_LARGE_MSG
      category    : COLLECTIVE
      type        : int
      default     : (512*1024)
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_RED_SCAT_SHORT_MSG
      category    : COLLECTIVE
      type        : int
      default     : 64
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_SCATTER_MEDIUM_MSG
      category    : COLLECTIVE
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        When the system size is lower than 512 cores, we use the
        "2-level" algorithm for medium message sizes. This allows
        the user to set the threshold for medium messages. -1
        indicates allowing the default values to be used.

    - name        : MVP_SCATTER_SMALL_MSG
      category    : COLLECTIVE
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        When the system size is lower than 512 cores, we use the
        "Binomial"algorithm for small message sizes. This allows the
        user to set the threshold for small messgaes. -1 indicates
        allowing the default values to be used.

    - name        : MVP_USE_DIRECT_SCATTER
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Use the "Direct" algorithm for the MPI_Scatter operation. If
        this parameter is set to 0 at runtime, the "Direct" algorithm
        will not be invoked.

    - name        : MVP_USE_SCATTER_RD_INTER_LEADER_BCAST
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_SCATTER_RING_INTER_LEADER_BCAST
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_TWO_LEVEL_SCATTER
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Use the two-level multi-core-aware algorithm for the
        MPI_Scatter operation. If this parameter is set to 0 at
        run-time, the two-level algorithm will not be invoked.

    - name        : MVP_SCATTER_COLLECTIVE_ALGORITHM
      category    : COLLECTIVE
      type        : enum
      default     : UNSET
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : |-
        This CVAR selects proper collective algorithm for inter and intra node
        scatter operations. (NOTE: This will override
        MVP_SCATTER_INTER_NODE_TUNING_ALGO and
        MVP_SCATTER_INTRA_NODE_TUNING_ALGO).
        UNSET       - No algorithm selected, will default to algo selected by
                    CVARs MVP_SCATTER_INTER_NODE_TUNING_ALGO and
                    MVP_SCATTER_INTRA_NODE_TUNING_ALGO.
        BINOMIAL    - Set both inter and intra node scatter tuning to binomial.
        DIRECT      - Set both inter and intra node scatter tuning to binomial.

    - name        : MVP_SCATTER_INTER_NODE_TUNING_ALGO
      alias       : MVP_INTER_SCATTER_TUNING
      category    : COLLECTIVE
      type        : enum
      default     : UNSET
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : |-
        Variable to select the inter node scatter algorithm. To use the two
        level algorithms MVP_USE_TWO_LEVEL_SCATTER must be set to 1 (default).
        (This CVAR can be overriden by MVP_SCATTER_COLLECTIVE_ALGORITHM).
        UNSET           - Default to internal algorithm selection.
        BINOMIAL        - Scatter data using a binomial tree algorithm that
                        divides data equally among processes.
        DIRECT          - Scatter data by directly sending it from the root to
                        all processes.
        2LVL_BINOMIAL   - Scatter data using a two-level binomial tree, where
                        processes within each node communicate with each other
                        before communicating across nodes.
        2LVL_DIRECT     - Scatter data using a two-level direct send-receive,
                        where processes within each node communicate with each
                        other before communicating across nodes.
        MCAST           - Scatter data using a multicast algorithm that sends
                        data to all processes at once. Also make sure to set
                        MVP_USE_MCAST_SCATTER=1 MVP_USE_MCAST_PIPELINE_SHM=1
                        MVP_USE_MCAST=1.

    - name        : MVP_SCATTER_INTRA_NODE_TUNING_ALGO
      alias       : MVP_INTRA_SCATTER_TUNING
      category    : COLLECTIVE
      type        : enum
      default     : UNSET
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : |-
        Variable to select the intra node scatter algorithm. (This CVAR can be
        overriden by MVP_SCATTER_COLLECTIVE_ALGORITHM).
        UNSET       - Default to internal algorithm selection.
        DIRECT      - Scatter data by directly sending it from the root to all
                    processes within a node.
        BINOMIAL    - Scatter data using a binomial tree algorithm that divides
                    data equally among processes within a node

=== END_MPI_T_MVP_CVAR_INFO_BLOCK ===
*/

MVP_Scatter_fn_t MVP_Scatter_function = NULL;
MVP_Scatter_fn_t MVP_Scatter_intra_function = NULL;

/* This is the default implementation of scatter. The algorithm is:

   Algorithm: MPI_Scatter

   We use a binomial tree algorithm for both short and
   long messages. At nodes other than leaf nodes we need to allocate
   a temporary buffer to store the incoming message. If the root is
   not rank 0, we reorder the sendbuf in order of relative ranks by
   copying it into a temporary buffer, so that all the sends from the
   root are contiguous and in the right order. In the heterogeneous
   case, we first pack the buffer by using MPI_Pack and then do the
   scatter.

   Cost = lgp.alpha + n.((p-1)/p).beta
   where n is the total size of the data to be scattered from the root.

   Possible improvements:

   End Algorithm: MPI_Scatter
*/

int MPIR_Scatter_index_tuned_intra_MVP(const void *sendbuf, int sendcnt,
                                       MPI_Datatype sendtype, void *recvbuf,
                                       int recvcnt, MPI_Datatype recvtype,
                                       int root, MPIR_Comm *comm_ptr,
                                       MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    int rank, comm_size;
    MPI_Aint nbytes;
    MPI_Aint recvtype_size, sendtype_size;
    int partial_sub_ok = 0;
    int conf_index = 0;
    int local_size = -1;
    int comm_size_index = 0;
    int inter_node_algo_index = 0;
    int intra_node_algo_index = 0;
    int table_min_comm_size = 0;
    int table_max_comm_size = 0;
    int table_min_inter_size = 0;
    int table_max_inter_size = 0;
    int table_min_intra_size = 0;
    int table_max_intra_size = 0;
    int last_inter;
    int last_intra;
    int lp2ltn; // largest power of 2 less than n
    int lp2ltn_min;
    MPI_Comm shmem_comm;
    MPIR_Comm *shmem_commptr = NULL;

    comm_size = MPIR_Comm_size(comm_ptr);
    rank = MPIR_Comm_rank(comm_ptr);

    if (rank == root) {
        MPIR_Datatype_get_size_macro(sendtype, sendtype_size);
        nbytes = sendcnt * sendtype_size;
    } else {
        MPIR_Datatype_get_size_macro(recvtype, recvtype_size);
        nbytes = recvcnt * recvtype_size;
    }

#if defined(_SHARP_SUPPORT_)
    if (comm_ptr->dev.ch.is_sharp_ok == 1 &&
        nbytes <= MVP_SHARP_MAX_MSG_SIZE / 2 && MVP_ENABLE_SHARP == 1 &&
        MVP_ENABLE_SHARP_SCATTER) {
        /* Direct flat algorithm in which every process calls Sharp
         * MVP_ENABLE_SHARP should be set to 1 */
        mpi_errno =
            MPIR_Sharp_Scatter_MVP(sendbuf, sendcnt, sendtype, recvbuf, recvcnt,
                                   recvtype, root, comm_ptr, errflag);
        if (mpi_errno == MPI_SUCCESS) {
            return mpi_errno;
        }
        /* SHArP collective is not supported, continue without using SHArP */
    }
#endif /* end of defined (_SHARP_SUPPORT_) */

    /* check if safe to use partial subscription mode */
    if (comm_ptr->dev.ch.shmem_coll_ok == 1 && comm_ptr->dev.ch.is_uniform) {
        shmem_comm = comm_ptr->dev.ch.shmem_comm;
        MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
        local_size = shmem_commptr->local_size;
        if (mvp_scatter_indexed_table_ppn_conf[0] == -1) {
            /* Indicating user defined tuning */
            conf_index = 0;
            goto conf_check_end;
        }
        FIND_PPN_INDEX(scatter, local_size, conf_index, partial_sub_ok)
    }

    if (partial_sub_ok != 1) {
        conf_index = mvp_scatter_indexed_num_ppn_conf / 2;
    }

conf_check_end:

    /* Search for the corresponding system size inside the tuning table */
    /*
     * Comm sizes progress in powers of 2. Therefore comm_size can just be
     * indexed instead
     */
    table_min_comm_size =
        mvp_scatter_indexed_thresholds_table[conf_index][0].numproc;
    table_max_comm_size =
        mvp_scatter_indexed_thresholds_table
            [conf_index][mvp_size_scatter_indexed_tuning_table[conf_index] - 1]
                .numproc;

    if (comm_size < table_min_comm_size) {
        /* Comm size smaller than smallest configuration in table: use smallest
         * available */
        comm_size_index = 0;
    } else if (comm_size > table_max_comm_size) {
        /* Comm size larger than largest configuration in table: use largest
         * available */
        comm_size_index = mvp_size_scatter_indexed_tuning_table[conf_index] - 1;
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
        mvp_scatter_indexed_thresholds_table[conf_index][comm_size_index]
            .size_inter_table -
        1;
    table_min_inter_size =
        mvp_scatter_indexed_thresholds_table[conf_index][comm_size_index]
            .inter_leader[0]
            .msg_sz;
    table_max_inter_size =
        mvp_scatter_indexed_thresholds_table[conf_index][comm_size_index]
            .inter_leader[last_inter]
            .msg_sz;
    last_intra =
        mvp_scatter_indexed_thresholds_table[conf_index][comm_size_index]
            .size_intra_table -
        1;
    table_min_intra_size =
        mvp_scatter_indexed_thresholds_table[conf_index][comm_size_index]
            .intra_node[0]
            .msg_sz;
    table_max_intra_size =
        mvp_scatter_indexed_thresholds_table[conf_index][comm_size_index]
            .intra_node[last_intra]
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

    if (nbytes < table_min_intra_size) {
        /* Msg size smaller than smallest configuration in table: use smallest
         * available */
        intra_node_algo_index = 0;
    } else if (nbytes > table_max_intra_size) {
        /* Msg size larger than largest configuration in table: use largest
         * available */
        intra_node_algo_index = last_intra;
    } else {
        /* Msg size in between smallest and largest configuration: find closest
         * match */
        if (pow(2, (int)log2(nbytes)) == nbytes) {
            intra_node_algo_index = log2(nbytes / table_min_intra_size);
        } else {
            lp2ltn = pow(2, (int)log2(nbytes));
            intra_node_algo_index = (lp2ltn < table_min_intra_size) ?
                                        0 :
                                        log2(lp2ltn / table_min_intra_size);
        }
    }

    MVP_Scatter_function =
        mvp_scatter_indexed_thresholds_table[conf_index][comm_size_index]
            .inter_leader[inter_node_algo_index]
            .scatter_fn;

    if (MVP_Scatter_function == &MPIR_Scatter_mcst_wrap_MVP) {
#if defined(_MCST_SUPPORT_)
        if (comm_ptr->dev.ch.is_mcast_ok == 1 && MVP_USE_MCAST_SCATTER &&
            comm_ptr->dev.ch.shmem_coll_ok == 1) {
            MVP_Scatter_function = &MPIR_Scatter_mcst_MVP;
        } else
#endif /*#if defined(_MCST_SUPPORT_) */
        {
            if (mvp_scatter_indexed_thresholds_table
                    [conf_index][comm_size_index]
                        .inter_leader[inter_node_algo_index + 1]
                        .scatter_fn != NULL) {
                MVP_Scatter_function =
                    mvp_scatter_indexed_thresholds_table
                        [conf_index][comm_size_index]
                            .inter_leader[inter_node_algo_index + 1]
                            .scatter_fn;
            } else {
                /* Fallback! */
                MVP_Scatter_function = &MPIR_Scatter_MVP_Binomial;
            }
        }
    }

    if ((MVP_Scatter_function == &MPIR_Scatter_MVP_two_level_Direct) ||
        (MVP_Scatter_function == &MPIR_Scatter_MVP_two_level_Binomial)) {
        if (comm_ptr->dev.ch.shmem_coll_ok == 1 &&
            comm_ptr->dev.ch.is_global_block == 1) {
            MVP_Scatter_intra_function =
                mvp_scatter_indexed_thresholds_table
                    [conf_index][comm_size_index]
                        .intra_node[intra_node_algo_index]
                        .scatter_fn;

            mpi_errno = MVP_Scatter_function(sendbuf, sendcnt, sendtype,
                                             recvbuf, recvcnt, recvtype, root,
                                             comm_ptr, errflag);
        } else {
            mpi_errno = MPIR_Scatter_MVP_Binomial(sendbuf, sendcnt, sendtype,
                                                  recvbuf, recvcnt, recvtype,
                                                  root, comm_ptr, errflag);
        }
    } else {
        mpi_errno =
            MVP_Scatter_function(sendbuf, sendcnt, sendtype, recvbuf, recvcnt,
                                 recvtype, root, comm_ptr, errflag);
    }

    if (mpi_errno) {
        /* for communication errors, just record the error but continue */
        *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
        MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
    }

fn_fail:

    return (mpi_errno);
}

int MPIR_Scatter_tune_intra_MVP(const void *sendbuf, int sendcnt,
                                MPI_Datatype sendtype, void *recvbuf,
                                int recvcnt, MPI_Datatype recvtype, int root,
                                MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int range = 0, range_threshold = 0, range_threshold_intra = 0;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    int rank, comm_size;
    MPI_Aint nbytes;
    MPI_Aint recvtype_size, sendtype_size;
    int partial_sub_ok = 0;
    int conf_index = 0;
    int local_size = -1;
    int i;
    MPI_Comm shmem_comm;
    MPIR_Comm *shmem_commptr = NULL;

    rank = MPIR_Comm_rank(comm_ptr);
    comm_size = MPIR_Comm_size(comm_ptr);

    if (rank == root) {
        MPIR_Datatype_get_size_macro(sendtype, sendtype_size);
        nbytes = sendcnt * sendtype_size;
    } else {
        MPIR_Datatype_get_size_macro(recvtype, recvtype_size);
        nbytes = recvcnt * recvtype_size;
    }

    /* check if safe to use partial subscription mode */
    if (comm_ptr->dev.ch.shmem_coll_ok == 1 && comm_ptr->dev.ch.is_uniform) {
        shmem_comm = comm_ptr->dev.ch.shmem_comm;
        MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
        local_size = shmem_commptr->local_size;
        i = 0;
        if (mvp_scatter_table_ppn_conf[0] == -1) {
            /* Indicating user defined tuning */
            conf_index = 0;
            goto conf_check_end;
        }
        do {
            if (local_size == mvp_scatter_table_ppn_conf[i]) {
                conf_index = i;
                partial_sub_ok = 1;
                break;
            }
            i++;
        } while (i < mvp_scatter_num_ppn_conf);
    }

    if (partial_sub_ok != 1) {
        conf_index = mvp_scatter_num_ppn_conf / 2;
    }

conf_check_end:

    /* Search for the corresponding system size inside the tuning table */
    while (
        (range < (mvp_size_scatter_tuning_table[conf_index] - 1)) &&
        (comm_size > mvp_scatter_thresholds_table[conf_index][range].numproc)) {
        range++;
    }
    /* Search for corresponding inter-leader function */
    while ((range_threshold <
            (mvp_scatter_thresholds_table[conf_index][range].size_inter_table -
             1)) &&
           (nbytes > mvp_scatter_thresholds_table[conf_index][range]
                         .inter_leader[range_threshold]
                         .max) &&
           (mvp_scatter_thresholds_table[conf_index][range]
                .inter_leader[range_threshold]
                .max != -1)) {
        range_threshold++;
    }

    /* Search for corresponding intra-node function */
    while ((range_threshold_intra <
            (mvp_scatter_thresholds_table[conf_index][range].size_intra_table -
             1)) &&
           (nbytes > mvp_scatter_thresholds_table[conf_index][range]
                         .intra_node[range_threshold_intra]
                         .max) &&
           (mvp_scatter_thresholds_table[conf_index][range]
                .intra_node[range_threshold_intra]
                .max != -1)) {
        range_threshold_intra++;
    }

    MVP_Scatter_function = mvp_scatter_thresholds_table[conf_index][range]
                               .inter_leader[range_threshold]
                               .scatter_fn;

    if (MVP_Scatter_function == &MPIR_Scatter_mcst_wrap_MVP) {
#if defined(_MCST_SUPPORT_)
        if (comm_ptr->dev.ch.is_mcast_ok == 1 && MVP_USE_MCAST_SCATTER &&
            comm_ptr->dev.ch.shmem_coll_ok == 1) {
            MVP_Scatter_function = &MPIR_Scatter_mcst_MVP;
        } else
#endif /*#if defined(_MCST_SUPPORT_) */
        {
            if (mvp_scatter_thresholds_table[conf_index][range]
                    .inter_leader[range_threshold + 1]
                    .scatter_fn != NULL) {
                MVP_Scatter_function =
                    mvp_scatter_thresholds_table[conf_index][range]
                        .inter_leader[range_threshold + 1]
                        .scatter_fn;
            } else {
                /* Fallback! */
                MVP_Scatter_function = &MPIR_Scatter_MVP_Binomial;
            }
        }
    }

    if ((MVP_Scatter_function == &MPIR_Scatter_MVP_two_level_Direct) ||
        (MVP_Scatter_function == &MPIR_Scatter_MVP_two_level_Binomial)) {
        if (comm_ptr->dev.ch.shmem_coll_ok == 1 &&
            comm_ptr->dev.ch.is_global_block == 1) {
            MVP_Scatter_intra_function =
                mvp_scatter_thresholds_table[conf_index][range]
                    .intra_node[range_threshold_intra]
                    .scatter_fn;

            mpi_errno = MVP_Scatter_function(sendbuf, sendcnt, sendtype,
                                             recvbuf, recvcnt, recvtype, root,
                                             comm_ptr, errflag);
        } else {
            mpi_errno = MPIR_Scatter_MVP_Binomial(sendbuf, sendcnt, sendtype,
                                                  recvbuf, recvcnt, recvtype,
                                                  root, comm_ptr, errflag);
        }
    } else {
        mpi_errno =
            MVP_Scatter_function(sendbuf, sendcnt, sendtype, recvbuf, recvcnt,
                                 recvtype, root, comm_ptr, errflag);
    }

    if (mpi_errno) {
        /* for communication errors, just record the error but continue */
        *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
        MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
    }

fn_fail:

    return (mpi_errno);
}

int MPIR_Scatter_intra_MVP(const void *sendbuf, int sendcnt,
                           MPI_Datatype sendtype, void *recvbuf, int recvcnt,
                           MPI_Datatype recvtype, int root, MPIR_Comm *comm_ptr,
                           MPIR_Errflag_t *errflag)
{
    int range = 0;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    int rank, comm_size;
    MPI_Aint nbytes;
    MPI_Aint recvtype_size, sendtype_size;

    rank = MPIR_Comm_rank(comm_ptr);
    comm_size = MPIR_Comm_size(comm_ptr);

    if (rank == root) {
        MPIR_Datatype_get_size_macro(sendtype, sendtype_size);
        nbytes = sendcnt * sendtype_size;
    } else {
        MPIR_Datatype_get_size_macro(recvtype, recvtype_size);
        nbytes = recvcnt * recvtype_size;
    }

    while ((range < mvp_size_mvp_scatter_mvp_tuning_table) &&
           (comm_size > mvp_scatter_mvp_tuning_table[range].numproc)) {
        range++;
    }
#if defined(_MCST_SUPPORT_)
    if (comm_ptr->dev.ch.is_mcast_ok == 1 && MVP_USE_MCAST_SCATTER &&
        nbytes <= MVP_MCAST_SCATTER_MSG_SIZE &&
        comm_size >= MVP_MCAST_SCATTER_SMALL_SYS_SIZE &&
        comm_size <= MVP_MCAST_SCATTER_LARGE_SYS_SIZE) {
        mpi_errno =
            MPIR_Scatter_mcst_MVP(sendbuf, sendcnt, sendtype, recvbuf, recvcnt,
                                  recvtype, root, comm_ptr, errflag);
    } else
#endif /*#if defined(_MCST_SUPPORT_) */
    {
        if (MVP_USE_TWO_LEVEL_SCATTER || MVP_USE_DIRECT_SCATTER) {
            if (range < mvp_size_mvp_scatter_mvp_tuning_table) {
                if (nbytes < mvp_scatter_mvp_tuning_table[range].small) {
                    mpi_errno = MPIR_Scatter_MVP_Binomial(
                        sendbuf, sendcnt, sendtype, recvbuf, recvcnt, recvtype,
                        root, comm_ptr, errflag);
                } else if (nbytes > mvp_scatter_mvp_tuning_table[range].small &&
                           nbytes <
                               mvp_scatter_mvp_tuning_table[range].medium &&
                           comm_ptr->dev.ch.shmem_coll_ok == 1 &&
                           MVP_USE_TWO_LEVEL_SCATTER) {
                    mpi_errno = MPIR_Scatter_MVP_two_level_Direct(
                        sendbuf, sendcnt, sendtype, recvbuf, recvcnt, recvtype,
                        root, comm_ptr, errflag);

                } else {
                    mpi_errno = MPIR_Scatter_MVP_Direct(
                        sendbuf, sendcnt, sendtype, recvbuf, recvcnt, recvtype,
                        root, comm_ptr, errflag);
                }
            } else if (comm_size >
                           mvp_scatter_mvp_tuning_table[range - 1].numproc &&
                       comm_ptr->dev.ch.shmem_coll_ok == 1 &&
                       MVP_USE_TWO_LEVEL_SCATTER) {
                mpi_errno = MPIR_Scatter_MVP_two_level_Binomial(
                    sendbuf, sendcnt, sendtype, recvbuf, recvcnt, recvtype,
                    root, comm_ptr, errflag);
            } else {
                mpi_errno = MPIR_Scatter_MVP_Binomial(
                    sendbuf, sendcnt, sendtype, recvbuf, recvcnt, recvtype,
                    root, comm_ptr, errflag);
            }
        } else {
            mpi_errno = MPIR_Scatter_MVP_Binomial(sendbuf, sendcnt, sendtype,
                                                  recvbuf, recvcnt, recvtype,
                                                  root, comm_ptr, errflag);
        }
    }

    if (mpi_errno) {
        /* for communication errors, just record the error but continue */
        *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
        MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
    }

fn_fail:

    return (mpi_errno);
}

/* begin:nested */
/* not declared static because a machine-specific function may call this one in
 * some cases */
int MPIR_Scatter_inter_MVP(void *sendbuf, int sendcnt, MPI_Datatype sendtype,
                           void *recvbuf, int recvcnt, MPI_Datatype recvtype,
                           int root, MPIR_Comm *comm_ptr,
                           MPIR_Errflag_t *errflag)
{
    /*  Intercommunicator scatter.
        For short messages, root sends to rank 0 in remote group. rank 0
        does local intracommunicator scatter (binomial tree).
        Cost: (lgp+1).alpha + n.((p-1)/p).beta + n.beta

        For long messages, we use linear scatter to avoid the extra n.beta.
        Cost: p.alpha + n.beta
    */

    int rank, local_size, remote_size, mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    int i;
    MPI_Aint nbytes, sendtype_size, recvtype_size;
    MPI_Status status;
    MPI_Aint extent, true_extent, true_lb = 0;
    void *tmp_buf = NULL;
    MPIR_Comm *newcomm_ptr = NULL;

    if (root == MPI_PROC_NULL) {
        /* local processes other than root do nothing */
        return MPI_SUCCESS;
    }

    remote_size = comm_ptr->remote_size;
    local_size = comm_ptr->local_size;

    if (root == MPI_ROOT) {
        MPIR_Datatype_get_size_macro(sendtype, sendtype_size);
        nbytes = sendtype_size * sendcnt * remote_size;
    } else {
        /* remote side */
        MPIR_Datatype_get_size_macro(recvtype, recvtype_size);
        nbytes = recvtype_size * recvcnt * local_size;
    }

    if (nbytes < MPIR_SCATTER_SHORT_MSG) {
        if (root == MPI_ROOT) {
            /* root sends all data to rank 0 on remote group and returns */
            MPIR_PVAR_INC(scatter, inter, send, sendcnt * remote_size,
                          sendtype);
            mpi_errno = MPIC_Send(sendbuf, sendcnt * remote_size, sendtype, 0,
                                  MPIR_SCATTER_TAG, comm_ptr, errflag);
            if (mpi_errno) {
                /* for communication errors, just record the error but continue
                 */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
            goto fn_exit;
        } else {
            /* remote group. rank 0 receives data from root. need to
               allocate temporary buffer to store this data. */

            rank = comm_ptr->rank;

            if (rank == 0) {
                MPIR_Type_get_true_extent_impl(recvtype, &true_lb,
                                               &true_extent);
                MPIR_Datatype_get_extent_macro(recvtype, extent);
                tmp_buf = MPL_malloc(recvcnt * local_size *
                                         (MPL_MAX(extent, true_extent)),
                                     MPL_MEM_COLL);
                /* --BEGIN ERROR HANDLING-- */
                if (!tmp_buf) {
                    mpi_errno = MPIR_Err_create_code(
                        MPI_SUCCESS, MPIR_ERR_RECOVERABLE, __func__, __LINE__,
                        MPI_ERR_OTHER, "**nomem", 0);
                    return mpi_errno;
                }
                /* --END ERROR HANDLING-- */
                /* adjust for potential negative lower bound in datatype */
                tmp_buf = (void *)((char *)tmp_buf - true_lb);

                MPIR_PVAR_INC(scatter, inter, recv, recvcnt * local_size,
                              recvtype);
                mpi_errno =
                    MPIC_Recv(tmp_buf, recvcnt * local_size, recvtype, root,
                              MPIR_SCATTER_TAG, comm_ptr, &status, errflag);
                if (mpi_errno) {
                    /* for communication errors, just record the error but
                     * continue */
                    *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                    MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                }
            }

            /* Get the local intracommunicator */
            if (!comm_ptr->local_comm)
                MPII_Setup_intercomm_localcomm(comm_ptr);

            newcomm_ptr = comm_ptr->local_comm;

            /* now do the usual scatter on this intracommunicator */
            mpi_errno =
                MPIR_Scatter_MVP(tmp_buf, recvcnt, recvtype, recvbuf, recvcnt,
                                 recvtype, 0, newcomm_ptr, errflag);
            if (rank == 0) {
                void *tmp = (void *)(tmp_buf + true_lb);
                MPL_free(tmp);
            }
        }
    } else {
        /* long message. use linear algorithm. */
        if (root == MPI_ROOT) {
            MPIR_Datatype_get_extent_macro(sendtype, extent);
            for (i = 0; i < remote_size; i++) {
                MPIR_PVAR_INC(scatter, inter, send, sendcnt, sendtype);
                mpi_errno =
                    MPIC_Send(((char *)sendbuf + sendcnt * i * extent), sendcnt,
                              sendtype, i, MPIR_SCATTER_TAG, comm_ptr, errflag);
                if (mpi_errno) {
                    /* for communication errors, just record the error but
                     * continue */
                    *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                    MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                }
            }
        } else {
            MPIR_PVAR_INC(scatter, inter, recv, recvcnt, recvtype);
            mpi_errno = MPIC_Recv(recvbuf, recvcnt, recvtype, root,
                                  MPIR_SCATTER_TAG, comm_ptr, &status, errflag);
            if (mpi_errno) {
                /* for communication errors, just record the error but continue
                 */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
        }
    }

fn_exit:

    return mpi_errno;
}

int MPIR_Scatter_MVP(const void *sendbuf, int sendcnt, MPI_Datatype sendtype,
                     void *recvbuf, int recvcnt, MPI_Datatype recvtype,
                     int root, MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;

    if (MVP_USE_OLD_SCATTER) {
        mpi_errno =
            MPIR_Scatter_intra_MVP(sendbuf, sendcnt, sendtype, recvbuf, recvcnt,
                                   recvtype, root, comm_ptr, errflag);
    } else {
            mpi_errno = MPIR_Scatter_index_tuned_intra_MVP(
                sendbuf, sendcnt, sendtype, recvbuf, recvcnt, recvtype, root,
                comm_ptr, errflag);
    }

    comm_ptr->dev.ch.intra_node_done = 0;
    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    return mpi_errno;
fn_fail:

    goto fn_exit;
}
