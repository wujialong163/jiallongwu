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
#include "mvp_common_tuning.h"
#include "reduce_tuning.h"

/*
=== BEGIN_MPI_T_MVP_CVAR_INFO_BLOCK ===

cvars:
    - name        : MVP_INTRA_SHMEM_REDUCE_MSG
      category    : COLLECTIVE
      type        : int
      default     : (1<<11)
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_REDUCE_2LEVEL_MSG
      category    : COLLECTIVE
      type        : int
      default     : (1<<14)
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This parameter can be used to determine the threshold for the
        2-level reduce algorithm. We now use the shared-memory-based
        algorithm for messages smaller than the MVP_SHMEM_REDUCE_MSG,
        the 2-level algorithm for medium sized messages up to the
        threshold defined by this parameter. We use the default
        point-to-point algorithms messages larger than this threshold.

    - name        : MVP_REDUCE_SHORT_MSG
      category    : COLLECTIVE
      type        : int
      default     : 8192
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_SHMEM_REDUCE_MSG
      category    : COLLECTIVE
      type        : int
      default     : (1<<12)
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        The SHMEM reduce is used for messages less than this threshold.

    - name        : MVP_USE_SHMEM_REDUCE
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This parameter can be used to turn off shared memory based
        MPI_Reduce for OFA-IB-CH3 over IBA by setting this to 0.

    - name        : MVP_REDUCE_COLLECTIVE_ALGORITHM
      category    : COLLECTIVE
      type        : enum
      default     : UNSET
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : |-
        This CVAR selects proper collective algorithm for inter and intra node
        reduce operations. To set this operation to a two level operation then
        you must seperately set MVP_REDUCE_TUNING_IS_TWO_LEVEL to 1 (true).
        (NOTE: This will override MVP_REDUCE_INTER_NODE_TUNING_ALGO and
        MVP_REDUCE_INTRA_NODE_TUNING_ALGO).
        UNSET       - No algorithm selected, will default to algo selected by
                    CVARs MVP_REDUCE_INTER_NODE_TUNING_ALGO and
                    MVP_REDUCE_INTRA_NODE_TUNING_ALGO.
        BINOMIAL    - sets both inter and intra tuning algo to binomial
        KNOMIAL     - sets both inter and intra tuning algo to knomial

    - name        : MVP_REDUCE_INTER_NODE_TUNING_ALGO
      alias       : MVP_INTER_REDUCE_TUNING
      category    : COLLECTIVE
      type        : enum
      default     : UNSET
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : |-
        Variable to select the inter node reduce algorithm. (This can be
        overriden by MVP_REDUCE_COLLECTIVE_ALGORITHM)
        UNSET               - Default to internal algorithm selection
        BINOMIAL            - The reduce operation is performed using a
                            binomial tree algorithm. This algorithm is efficient
                            for small to medium-sized messages and is scalable
                            to a large number of nodes.
        KNOMIAL             - The reduce operation is performed using a k-nomial
                            tree algorithm. This algorithm is efficient for
                            medium to large-sized messages and is also scalable
                            to a large number of nodes.
        RED_SCAT_GATHER     - The reduce-scatter operation is performed first,
                            followed by a gather operation. This algorithm is
                            efficient for medium to large-sized messages and can
                            be used on systems where there is no direct
                            support for reduce operation.
        ALLREDUCE           - Uses an underlying allreduce algorithm to perform
                            the reduce operation.

    - name        : MVP_REDUCE_INTRA_NODE_TUNING_ALGO
      alias       : MVP_INTRA_REDUCE_TUNING
      category    : COLLECTIVE
      type        : enum
      default     : UNSET
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : |-
        Variable to select the intra node reduce algorithm. (This can be
        overriden by MVP_REDUCE_COLLECTIVE_ALGORITHM)
        UNSET       - Default to internal algorithm selection
        KNOMIAL     - This algorithm is a recursive, tree-based algorithm that
                    uses pairwise reduction operations to reduce data.
        SHMEM       - This algorithm is optimized for communication within a
                    single node and uses shared memory for data transfer.
        BINOMIAL    - This algorithm is optimized for communication within a
                    single node and uses shared memory for data transfer.

    - name        : MVP_REDUCE_TUNING_IS_TWO_LEVEL
      alias       : MVP_INTER_REDUCE_TUNING_TWO_LEVEL
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

MVP_Reduce_fn_t reduce_fn = NULL;
MVP_Reduce_fn_t MVP_Reduce_function = NULL;
MVP_Reduce_fn_t MVP_Reduce_intra_function = NULL;

int (*MPIR_Rank_list_mapper)(MPIR_Comm *, int) = NULL;

/* This is the default implementation of reduce. The algorithm is:

   Algorithm: MPI_Reduce

   For long messages and for builtin ops and if count >= pof2 (where
   pof2 is the nearest power-of-two less than or equal to the number
   of processes), we use Rabenseifner's algorithm (see
   http://www.hlrs.de/organization/par/services/models/mpi/myreduce.html ).
   This algorithm implements the reduce in two steps: first a
   reduce-scatter, followed by a gather to the root. A
   recursive-halving algorithm (beginning with processes that are
   distance 1 apart) is used for the reduce-scatter, and a binomial tree
   algorithm is used for the gather. The non-power-of-two case is
   handled by dropping to the nearest lower power-of-two: the first
   few odd-numbered processes send their data to their left neighbors
   (rank-1), and the reduce-scatter happens among the remaining
   power-of-two processes. If the root is one of the excluded
   processes, then after the reduce-scatter, rank 0 sends its result to
   the root and exits; the root now acts as rank 0 in the binomial tree
   algorithm for gather.

   For the power-of-two case, the cost for the reduce-scatter is
   lgp.alpha + n.((p-1)/p).beta + n.((p-1)/p).gamma. The cost for the
   gather to root is lgp.alpha + n.((p-1)/p).beta. Therefore, the
   total cost is:
   Cost = 2.lgp.alpha + 2.n.((p-1)/p).beta + n.((p-1)/p).gamma

   For the non-power-of-two case, assuming the root is not one of the
   odd-numbered processes that get excluded in the reduce-scatter,
   Cost = (2.floor(lgp)+1).alpha + (2.((p-1)/p) + 1).n.beta +
           n.(1+(p-1)/p).gamma

   For short messages, user-defined ops, and count < pof2, we use a
   binomial tree algorithm for both short and long messages.

   Cost = lgp.alpha + n.lgp.beta + n.lgp.gamma

   We use the binomial tree algorithm in the case of user-defined ops
   because in this case derived datatypes are allowed, and the user
   could pass basic datatypes on one process and derived on another as
   long as the type maps are the same. Breaking up derived datatypes
   to do the reduce-scatter is tricky.

   FIXME: Per the MPI-2.1 standard this case is not possible.  We
   should be able to use the reduce-scatter/gather approach as long as
   count >= pof2.  [goodell@ 2009-01-21]

   Possible improvements:

   End Algorithm: MPI_Reduce
*/

/* not declared static because a machine-specific function may call this one
   in some cases */
int MPIR_Reduce_index_tuned_intra_MVP(const void *sendbuf, void *recvbuf,
                                      int count, MPI_Datatype datatype,
                                      MPI_Op op, int root, MPIR_Comm *comm_ptr,
                                      MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
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
    int is_commutative, pof2;
    MPIR_Op *op_ptr;
    int comm_size = 0;
    int nbytes = 0;
    int sendtype_size;
    int is_two_level = 0;
    MPIR_Comm *shmem_commptr = NULL;
    MPI_Comm shmem_comm;
    comm_size = comm_ptr->local_size;
    MPIR_Datatype_get_size_macro(datatype, sendtype_size);
    nbytes = count * sendtype_size;

    if (count == 0)
        return MPI_SUCCESS;

    if (HANDLE_GET_KIND(op) == HANDLE_KIND_BUILTIN) {
        is_commutative = 1;
        /* get the function by indexing into the op table */
    } else {
        MPIR_Op_get_ptr(op, op_ptr) if (op_ptr->kind ==
                                        MPIR_OP_KIND__USER_NONCOMMUTE)
        {
            is_commutative = 0;
        }
        else { is_commutative = 1; }
    }

    /* find nearest power-of-two less than or equal to comm_size */
    pof2 = comm_ptr->dev.ch.gpof2;

    /* check if safe to use partial subscription mode */
    if (comm_ptr->dev.ch.shmem_coll_ok == 1 && comm_ptr->dev.ch.is_uniform) {
        shmem_comm = comm_ptr->dev.ch.shmem_comm;
        MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
        local_size = shmem_commptr->local_size;
        if (mvp_reduce_indexed_table_ppn_conf[0] == -1) {
            /* Indicating user defined tuning */
            conf_index = 0;
            goto conf_check_end;
        }

        if (nbytes <= MVP_TOPO_AWARE_REDUCE_MAX_MSG &&
            nbytes >= MVP_TOPO_AWARE_REDUCE_MIN_MSG &&
            MVP_ENABLE_SKIP_TUNING_TABLE_SEARCH &&
            nbytes <= MVP_COLL_SKIP_TABLE_THRESHOLD &&
            MVP_ENABLE_TOPO_AWARE_COLLECTIVES && MVP_USE_TOPO_AWARE_REDUCE &&
            comm_ptr->dev.ch.topo_coll_ok == 1 && is_commutative &&
            local_size >= MVP_TOPO_AWARE_REDUCE_PPN_THRESHOLD &&
            MVP_TOPO_AWARE_REDUCE_NODE_THRESHOLD <=
                comm_ptr->dev.ch.leader_group_size) {
            MVP_Reduce_function = MPIR_Reduce_topo_aware_hierarchical_MVP;
            goto skip_tuning_tables;
        }

        if (likely(MVP_USE_SHMEM_REDUCE &&
                   MVP_ENABLE_SKIP_TUNING_TABLE_SEARCH &&
                   (nbytes <= MVP_COLL_SKIP_TABLE_THRESHOLD))) {
            /* for small messages, force shmem + binomial */
            MVP_Reduce_intra_function = MPIR_Reduce_shmem_MVP;
            MVP_Reduce_function = MPIR_Reduce_binomial_MVP;
            is_two_level = 1;
            goto skip_tuning_tables;
        }
        FIND_PPN_INDEX(reduce, local_size, conf_index, partial_sub_ok)
    }

    if (partial_sub_ok != 1) {
        conf_index = mvp_reduce_indexed_num_ppn_conf / 2;
    }

conf_check_end:
    /* Search for the corresponding system size inside the tuning table */
    /*
     * Comm sizes progress in powers of 2. Therefore comm_size can just be
     * indexed instead
     */
    table_min_comm_size =
        mvp_reduce_indexed_thresholds_table[conf_index][0].numproc;
    table_max_comm_size =
        mvp_reduce_indexed_thresholds_table
            [conf_index][mvp_size_reduce_indexed_tuning_table[conf_index] - 1]
                .numproc;

    if (comm_size < table_min_comm_size) {
        /* Comm size smaller than smallest configuration in table: use smallest
         * available */
        comm_size_index = 0;
    } else if (comm_size > table_max_comm_size) {
        /* Comm size larger than largest configuration in table: use largest
         * available */
        comm_size_index = mvp_size_reduce_indexed_tuning_table[conf_index] - 1;
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
        mvp_reduce_indexed_thresholds_table[conf_index][comm_size_index]
            .size_inter_table -
        1;
    table_min_inter_size =
        mvp_reduce_indexed_thresholds_table[conf_index][comm_size_index]
            .inter_leader[0]
            .msg_sz;
    table_max_inter_size =
        mvp_reduce_indexed_thresholds_table[conf_index][comm_size_index]
            .inter_leader[last_inter]
            .msg_sz;
    last_intra =
        mvp_reduce_indexed_thresholds_table[conf_index][comm_size_index]
            .size_intra_table -
        1;
    table_min_intra_size =
        mvp_reduce_indexed_thresholds_table[conf_index][comm_size_index]
            .intra_node[0]
            .msg_sz;
    table_max_intra_size =
        mvp_reduce_indexed_thresholds_table[conf_index][comm_size_index]
            .intra_node[last_intra]
            .msg_sz;

    if (nbytes < table_min_inter_size) {
        /* Msg size smaller than smallest configuration in table:
         * use smallest available */
        inter_node_algo_index = 0;
    } else if (nbytes > table_max_inter_size) {
        /* Msg size larger than largest configuration in table:
         * use largest available */
        inter_node_algo_index = last_inter;
    } else {
        /* Msg size in between smallest and largest configuration:
         * find closest match */
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
        /* Msg size smaller than smallest configuration in table:
         * use smallest available */
        intra_node_algo_index = 0;
    } else if (nbytes > table_max_intra_size) {
        /* Msg size larger than largest configuration in table:
         * use largest available */
        intra_node_algo_index = last_intra;
    } else {
        /* Msg size in between smallest and largest configuration:
         * find closest match */
        if (pow(2, (int)log2(nbytes)) == nbytes) {
            intra_node_algo_index = log2(nbytes / table_min_intra_size);
        } else {
            lp2ltn = pow(2, (int)log2(nbytes));
            intra_node_algo_index = (lp2ltn < table_min_intra_size) ?
                                        0 :
                                        log2(lp2ltn / table_min_intra_size);
        }
    }

    /* Set intra-node function pt for reduce_two_level */
    MVP_Reduce_intra_function =
        mvp_reduce_indexed_thresholds_table[conf_index][comm_size_index]
            .intra_node[intra_node_algo_index]
            .reduce_fn;
    /* Set inter-leader pt */
    MVP_Reduce_function =
        mvp_reduce_indexed_thresholds_table[conf_index][comm_size_index]
            .inter_leader[inter_node_algo_index]
            .reduce_fn;

    if (MVP_REDUCE_INTRA_KNOMIAL_FACTOR < 0) {
        MVP_REDUCE_INTRA_KNOMIAL_FACTOR =
            mvp_reduce_indexed_thresholds_table[conf_index][comm_size_index]
                .intra_k_degree;
    }
    if (MVP_REDUCE_INTER_KNOMIAL_FACTOR < 0) {
        MVP_REDUCE_INTER_KNOMIAL_FACTOR =
            mvp_reduce_indexed_thresholds_table[conf_index][comm_size_index]
                .inter_k_degree;
    }
    if (mvp_reduce_indexed_thresholds_table[conf_index][comm_size_index]
            .is_two_level_reduce[inter_node_algo_index] == 1) {
        is_two_level = 1;
    }

skip_tuning_tables:
#ifdef CHANNEL_MRAIL_GEN2
    if (MPIR_Reduce_Zcpy_MVP == MVP_Reduce_function) {
        if (MVP_USE_SLOT_SHMEM_COLL && MVP_USE_ZCOPY_REDUCE &&
            nbytes <= MVP_SHMEM_COLL_SLOT_LEN &&
            comm_ptr->dev.ch.shmem_coll_ok == 1 && MVP_USE_SHMEM_REDUCE &&
            is_commutative == 1) {
            /* do nothing and continue to use zcpy */
        } else {
            /*fall back to trusty algorithm because it's invalid to
             * use zcpy without the initializations. */
            MVP_Reduce_function = MPIR_Reduce_binomial_MVP;
        }
    }
#endif /* CHANNEL_MRAIL_GEN2 */

    /* We call Reduce function */
    if (is_two_level == 1) {
        if (comm_ptr->dev.ch.shmem_coll_ok == 1 && is_commutative == 1) {
            mpi_errno = MPIR_Reduce_two_level_helper_MVP(
                sendbuf, recvbuf, count, datatype, op, root, comm_ptr, errflag);
        } else {
            mpi_errno = MPIR_Reduce_binomial_MVP(
                sendbuf, recvbuf, count, datatype, op, root, comm_ptr, errflag);
        }
    } else if (MVP_Reduce_function == &MPIR_Reduce_inter_knomial_wrapper_MVP) {
        if (is_commutative == 1) {
            mpi_errno = MVP_Reduce_function(sendbuf, recvbuf, count, datatype,
                                            op, root, comm_ptr, errflag);
        } else {
            mpi_errno = MPIR_Reduce_binomial_MVP(
                sendbuf, recvbuf, count, datatype, op, root, comm_ptr, errflag);
        }
    } else if (MVP_Reduce_function == &MPIR_Reduce_redscat_gather_MVP) {
        if ((HANDLE_GET_KIND(op) == HANDLE_KIND_BUILTIN) && (count >= pof2)) {
            mpi_errno = MVP_Reduce_function(sendbuf, recvbuf, count, datatype,
                                            op, root, comm_ptr, errflag);
        } else {
            mpi_errno = MPIR_Reduce_binomial_MVP(
                sendbuf, recvbuf, count, datatype, op, root, comm_ptr, errflag);
        }
    } else {
        mpi_errno = MVP_Reduce_function(sendbuf, recvbuf, count, datatype, op,
                                        root, comm_ptr, errflag);
    }

    if (mpi_errno) {
        /* for communication errors, just record the error but continue */
        *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
        MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
    }
    MPIR_ERR_CHECK(mpi_errno);

    if (mpi_errno_ret) {
        mpi_errno = mpi_errno_ret;
    } else if (*errflag) {
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**coll_fail");
    }

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

/* not declared static because a machine-specific function may call this one
   in some cases */
int MPIR_Reduce_MVP(const void *sendbuf, void *recvbuf, int count,
                    MPI_Datatype datatype, MPI_Op op, int root,
                    MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;

    MPIR_T_PVAR_COMM_COUNTER_INC(MVP, mvp_coll_reduce_subcomm, 1, comm_ptr);

    mpi_errno = MPIR_Reduce_index_tuned_intra_MVP(
        sendbuf, recvbuf, count, datatype, op, root, comm_ptr, errflag);

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
