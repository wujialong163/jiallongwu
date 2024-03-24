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

#include "mpiimpl.h"
#include "datatype.h"
#include "mvp_common_tuning.h"
#include "gather_tuning.h"

/*
=== BEGIN_MPI_T_MVP_CVAR_INFO_BLOCK ===

cvars:
    - name        : MVP_GATHER_SWITCH_PT
      category    : COLLECTIVE
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        We use different algorithms depending on the system size. For small
        system sizes (up to 386 cores), we use the "2-level" algorithm
        followed by the "Direct" algorithm. For medium system sizes (up to 1k),
        we use the "Binomial" algorithm followed by the "Direct" algorithm.
        Users can set the switching point between algorithms using this runtime
        parameter.

    - name        : MVP_USE_DIRECT_GATHER
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Use the "Direct" algorithm for the MPI_Gather operation. If
        this parameter is set to 0 at run-time, the "Direct" algorithm
        will not be invoked.

    - name        : MVP_USE_DIRECT_GATHER_SYSTEM_SIZE_MEDIUM
      category    : COLLECTIVE
      type        : int
      default     : 1024
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_DIRECT_GATHER_SYSTEM_SIZE_SMALL
      category    : COLLECTIVE
      type        : int
      default     : 384
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_TWO_LEVEL_GATHER
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Use the two-level multi-core-aware algorithm for the
        MPI_Gather operation. If this parameter is set to 0 at
        run-time, the two-level algorithm will not be invoked.

    - name        : MVP_GATHER_COLLECTIVE_ALGORITHM
      category    : COLLECTIVE
      type        : enum
      default     : UNSET
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : |-
        This CVAR selects proper collective algorithm for inter and intra node
        gather operations. (This will override MVP_GATHER_INTER_NODE_TUNING_ALGO
        and MVP_GATHER_INTRA_NODE_TUNING_ALGO)
        UNSET       - No algorithm selected, will default to algo selected by
                    CVARs MVP_GATHER_INTER_NODE_TUNING_ALGO and
                    MVP_GATHER_INTRA_NODE_TUNING_ALGO.
        BINOMIAL    - Sets the inter and intra tuning algorithm to binomial.
        DIRECT      - Sets the inter and intra tuning algorithm to direct.

    - name        : MVP_GATHER_INTER_NODE_TUNING_ALGO
      alias       : MVP_INTER_GATHER_TUNING
      category    : COLLECTIVE
      type        : enum
      default     : UNSET
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : |-
        Variable to select the inter node gather algorithm. (This can be
        overriden by MVP_GATHER_COLLECTIVE_ALGORITHM)
        UNSET               - Internal algorithm selection.
        BINOMIAL            - A recursive algorithm that divides nodes into
                            pairs and exchanges data until all nodes have
                            the final result.
        DIRECT              - A recursive algorithm that divides nodes into
                            pairs and exchanges data until all nodes have the
                            final result.
        2LVL_DIRECT    - A recursive algorithm that divides nodes into
                            pairs and exchanges data until all nodes have the
                            final result.

    - name        : MVP_GATHER_INTRA_NODE_TUNING_ALGO
      alias       : MVP_INTRA_GATHER_TUNING
      category    : COLLECTIVE
      type        : enum
      default     : UNSET
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : |-
        Variable to select the intra node gather algorithm. (This can be
        overriden by MVP_GATHER_COLLECTIVE_ALGORITHM)
        UNSET       - Internal algorithm selection
        BINOMIAL    - A recursive algorithm that divides nodes into pairs and
                    exchanges data until all nodes have the  final result.
        DIRECT      - A recursive algorithm that divides nodes into pairs and
                    exchanges data until all nodes have the final result.

=== END_MPI_T_MVP_CVAR_INFO_BLOCK ===
*/

MVP_Gather_fn_t MVP_Gather_inter_leader_function = NULL;
MVP_Gather_fn_t MVP_Gather_intra_node_function = NULL;

int MPIR_Gather_index_tuned_intra_MVP(const void *sendbuf, int sendcnt,
                                      MPI_Datatype sendtype, void *recvbuf,
                                      int recvcnt, MPI_Datatype recvtype,
                                      int root, MPIR_Comm *comm_ptr,
                                      MPIR_Errflag_t *errflag)
{
    int comm_size_index = 0;
    int inter_node_algo_index = 0;
    int intra_node_algo_index = 0;
    int local_size = 0;
    int partial_sub_ok = 0;
    int conf_index = 0;
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
    MPIR_Comm *shmem_commptr = NULL;
    MPI_Comm shmem_comm;
    int mpi_errno = MPI_SUCCESS;
    int comm_size = 0;
    MPI_Aint nbytes = 0;
    MPI_Aint recvtype_size, sendtype_size;
    int rank = -1;

    comm_size = comm_ptr->local_size;
    rank = comm_ptr->rank;

    if (rank == root) {
        MPIR_Datatype_get_size_macro(recvtype, recvtype_size);
        nbytes = recvcnt * recvtype_size;
    } else {
        MPIR_Datatype_get_size_macro(sendtype, sendtype_size);
        nbytes = sendcnt * sendtype_size;
    }

    /* check if safe to use partial subscription mode */
    if (comm_ptr->dev.ch.shmem_coll_ok == 1 && comm_ptr->dev.ch.is_uniform) {
        shmem_comm = comm_ptr->dev.ch.shmem_comm;
        MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
        local_size = shmem_commptr->local_size;
        if (mvp_gather_indexed_table_ppn_conf[0] == -1) {
            // Indicating user defined tuning
            conf_index = 0;
            goto conf_check_end;
        }

        FIND_PPN_INDEX(gather, local_size, conf_index, partial_sub_ok)
    }

    if (partial_sub_ok != 1) {
        conf_index = mvp_gather_indexed_num_ppn_conf / 2;
    }

conf_check_end:

    /* Search for the corresponding system size inside the tuning table */
    /*
     * Comm sizes progress in powers of 2. Therefore comm_size can just be
     * indexed instead
     */
    table_min_comm_size =
        mvp_gather_indexed_thresholds_table[conf_index][0].numproc;
    table_max_comm_size =
        mvp_gather_indexed_thresholds_table
            [conf_index][mvp_size_gather_indexed_tuning_table[conf_index] - 1]
                .numproc;

    if (comm_size < table_min_comm_size) {
        /* Comm size smaller than smallest configuration in table: use smallest
         * available */
        comm_size_index = 0;
    } else if (comm_size > table_max_comm_size) {
        /* Comm size larger than largest configuration in table: use largest
         * available */
        comm_size_index = mvp_size_gather_indexed_tuning_table[conf_index] - 1;
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
        mvp_gather_indexed_thresholds_table[conf_index][comm_size_index]
            .size_inter_table -
        1;
    table_min_inter_size =
        mvp_gather_indexed_thresholds_table[conf_index][comm_size_index]
            .inter_leader[0]
            .msg_sz;
    table_max_inter_size =
        mvp_gather_indexed_thresholds_table[conf_index][comm_size_index]
            .inter_leader[last_inter]
            .msg_sz;
    last_intra =
        mvp_gather_indexed_thresholds_table[conf_index][comm_size_index]
            .size_intra_table -
        1;
    table_min_intra_size =
        mvp_gather_indexed_thresholds_table[conf_index][comm_size_index]
            .intra_node[0]
            .msg_sz;
    table_max_intra_size =
        mvp_gather_indexed_thresholds_table[conf_index][comm_size_index]
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

    if (comm_ptr->dev.ch.rank_list != NULL && MVP_USE_DIRECT_GATHER &&
        MVP_USE_TWO_LEVEL_GATHER && comm_ptr->dev.ch.shmem_coll_ok == 1 &&
        comm_ptr->dev.ch.is_blocked == 0 && comm_ptr->dev.ch.is_uniform == 1) {
        /* Set intra-node function pt for gather_two_level */
        MVP_Gather_intra_node_function =
            mvp_gather_indexed_thresholds_table[conf_index][comm_size_index]
                .intra_node[intra_node_algo_index]
                .gather_fn;
        /* Set inter-leader pt */
        MVP_Gather_inter_leader_function =
            mvp_gather_indexed_thresholds_table[conf_index][comm_size_index]
                .inter_leader[inter_node_algo_index]
                .gather_fn;
        /* We call Gather function */
        mpi_errno = MVP_Gather_inter_leader_function(sendbuf, sendcnt, sendtype,
                                                     recvbuf, recvcnt, recvtype,
                                                     root, comm_ptr, errflag);

    } else {
        mpi_errno = MPIR_Gather_intra_binomial(sendbuf, sendcnt, sendtype,
                                               recvbuf, recvcnt, recvtype, root,
                                               comm_ptr, errflag);
    }
    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIR_Gather_MVP(const void *sendbuf, int sendcnt, MPI_Datatype sendtype,
                    void *recvbuf, int recvcnt, MPI_Datatype recvtype, int root,
                    MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;
    MPI_Aint nbytes = 0;
    MPI_Aint recvtype_size, sendtype_size;
    int rank = -1;

    rank = comm_ptr->rank;

    if (rank == root) {
        MPIR_Datatype_get_size_macro(recvtype, recvtype_size);
        nbytes = recvcnt * recvtype_size;
    } else {
        MPIR_Datatype_get_size_macro(sendtype, sendtype_size);
        nbytes = sendcnt * sendtype_size;
    }

    mpi_errno = MPIR_Gather_index_tuned_intra_MVP(sendbuf, sendcnt, sendtype,
                                                  recvbuf, recvcnt, recvtype,
                                                  root, comm_ptr, errflag);

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

/* end:nested */
