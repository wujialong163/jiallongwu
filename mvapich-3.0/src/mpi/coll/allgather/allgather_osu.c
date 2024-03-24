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

#include "datatype.h"
#include "mvp_common_tuning.h"
#include "mvp_coll_shmem.h"
#include "allgather_tuning.h"

/*
=== BEGIN_MPI_T_MVP_CVAR_INFO_BLOCK ===

cvars:
    - name        : MVP_ALLGATHER_BRUCK_THRESHOLD
      category    : COLLECTIVE
      type        : int
      default     : 524288
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_ALLGATHER_RD_THRESHOLD
      category    : COLLECTIVE
      type        : int
      default     : 81920
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_ALLGATHER_REVERSE_RANKING
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_ALLGATHER_COLLECTIVE_ALGORITHM
      category    : COLLECTIVE
      type        : enum
      default     : UNSET
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : |-
        Variable to select the allgather collective algorithm.
        UNSET                   - Internal algorithm selection.
        RD_ALLGATHER_COMM       - Performs data exchange by recursively dividing
                                the group into halves and exchanging data
                                between neighboring processes, then reordering
                                the received data in a final step.
        RD                      - Processes are divided into groups, and each
                                group exchanges data with its neighbor groups in
                                a recursive manner until all processes have
                                received the complete data set.
        BRUCK                   - Uses a combination of recursive doubling and a
                                tree-based approach to perform an allgather
                                operation. It is often used in situations where
                                the network has low latency and high bandwidth.
        RING                    - Uses a combination of recursive doubling and a
                                tree-based approach.
        DIRECT                  - A variant of the Two-Level Non-Blocking
                                algorithm that uses a direct-exchange approach
                                for the first level of communication.
        DIRECTSPREAD            - A variant of the Direct algorithm that uses a
                                scatter-gather approach to reduce the amount of
                                data that each process needs to send and
                                receive.
        GATHER_BCAST            -  Involves first gathering all data onto a
                                single process and then broadcasting the result
                                to all processes. This algorithm is often used
                                in situations where the network has low
                                bandwidth and high latency.
        2LVL_NONBLOCKED         - A variant of the Gather-Broadcast algorithm
                                that uses a two-level tree-based approach to
                                reduce the amount of data that needs to be
                                transmitted.
        2LVL_RING_NONBLOCKED    - A variant of the Two-Level Non-Blocking
                                algorithm that uses a ring-based approach for
                                the first level of communication.
        2LVL_DIRECT             - A variant of the Two-Level Non-Blocking
                                algorithm that uses a direct-exchange approach
                                for the first level of communication.
        2LVL_RING               - A variant of the Two-Level Non-Blocking
                                algorithm that uses a direct-exchange approach
                                for the first level of communication.

    - name        : MVP_ALLGATHER_TUNING_IS_TWO_LEVEL
      alias       : MVP_INTER_ALLGATHER_TUNING_TWO_LEVEL
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

extern int allgather_tuning_algo;
extern int allgather_algo_num;

extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_rd_allgather_comm;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_count_recv;

#define FGP_SWITCH_FACTOR                                                      \
    4 /*Used to determine switch between naive and FGP                         \
         design */

MVP_Allgather_fn_t MVP_Allgather_function = NULL;

#undef FUNCNAME
#define FUNCNAME MPIR_Allgather_RD_Allgather_Comm_MVP
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPIR_Allgather_RD_Allgather_Comm_MVP(const void *sendbuf, int sendcount,
                                         MPI_Datatype sendtype, void *recvbuf,
                                         int recvcount, MPI_Datatype recvtype,
                                         MPIR_Comm *comm_ptr,
                                         MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;
    int i, rank, comm_size;
    MPI_Aint recvtype_extent, recvtype_size, nbytes = 0;

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_allgather_rd_allgather_comm, 1);

    rank = comm_ptr->rank;
    comm_size = comm_ptr->local_size;
    MPIR_Datatype_get_extent_macro(recvtype, recvtype_extent);

    MPIR_Datatype_get_size_macro(recvtype, recvtype_size);
    nbytes = recvtype_size * recvcount;

    if (comm_ptr->dev.ch.allgather_comm_ok == 1) {
        int sendtype_iscontig = 0, recvtype_iscontig = 0;
        void *tmp_recv_buf = NULL;
        MPIR_T_PVAR_COUNTER_INC(MVP, mvp_num_shmem_coll_calls, 1);
        if (sendtype != MPI_DATATYPE_NULL && recvtype != MPI_DATATYPE_NULL) {
            MPIR_Datatype_iscontig(sendtype, &sendtype_iscontig);
            MPIR_Datatype_iscontig(recvtype, &recvtype_iscontig);
        }

        MPIR_Comm *allgather_comm_ptr;
        MPIR_Comm_get_ptr(comm_ptr->dev.ch.allgather_comm, allgather_comm_ptr);

        /*creation of a temporary recvbuf */
        tmp_recv_buf =
            MPL_malloc(recvcount * comm_size * recvtype_extent, MPL_MEM_OTHER);
        if (!tmp_recv_buf) {
            mpi_errno =
                MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, FCNAME,
                                     __LINE__, MPI_ERR_OTHER, "**nomem", 0);
            return mpi_errno;
        }
        /* Calling Allgather with temporary buffer and allgather communicator */
        if (sendbuf != MPI_IN_PLACE) {
            mpi_errno = MPIR_Allgather_RD_MVP(sendbuf, sendcount, sendtype,
                                              tmp_recv_buf, recvcount, recvtype,
                                              allgather_comm_ptr, errflag);
        } else {
            mpi_errno = MPIR_Allgather_RD_MVP(
                recvbuf + rank * recvcount * recvtype_extent, recvcount,
                recvtype, tmp_recv_buf, recvcount, recvtype, allgather_comm_ptr,
                errflag);
        }

        if (mpi_errno) {
            MPIR_ERR_POP(mpi_errno);
        }
        /* Reordering data into recvbuf */
        if (sendtype_iscontig == 1 && recvtype_iscontig == 1
#if defined(_ENABLE_CUDA_)
            && mvp_enable_device == 0
#endif
        ) {
            for (i = 0; i < comm_size; i++) {
                MPIR_Memcpy((void *)((char *)recvbuf +
                                     (comm_ptr->dev.ch.allgather_new_ranks[i]) *
                                         nbytes),
                            (char *)tmp_recv_buf + i * nbytes, nbytes);
            }
        } else {
            for (i = 0; i < comm_size; i++) {
                mpi_errno = MPIR_Localcopy(
                    (void *)((char *)tmp_recv_buf +
                             i * recvcount * recvtype_extent),
                    recvcount, recvtype,
                    (void *)((char *)recvbuf +
                             (comm_ptr->dev.ch.allgather_new_ranks[i]) *
                                 recvcount * recvtype_extent),
                    recvcount, recvtype);
                if (mpi_errno) {
                    MPIR_ERR_POP(mpi_errno);
                }
            }
        }
        MPL_free(tmp_recv_buf);
    } else {
        mpi_errno =
            MPIR_Allgather_RD_MVP(sendbuf, sendcount, sendtype, recvbuf,
                                  recvcount, recvtype, comm_ptr, errflag);
        if (mpi_errno) {
            MPIR_ERR_POP(mpi_errno);
        }
    }

fn_fail:
    return mpi_errno;
}

int allgather_tuning(int comm_size)
{
    if (comm_size <= MVP_ALLGATHER_SMALL_SYSTEM_SIZE) {
        return mvp_tuning_table[ALLGATHER_ID][SMALL];
    } else if (comm_size > MVP_ALLGATHER_SMALL_SYSTEM_SIZE &&
               comm_size <= MVP_ALLGATHER_MEDIUM_SYSTEM_SIZE) {
        return mvp_tuning_table[ALLGATHER_ID][MEDIUM];
    } else {
        return mvp_tuning_table[ALLGATHER_ID][LARGE];
    }
}

/* This is the default implementation of allgather. The algorithm is:

Algorithm: MPI_Allgather

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

It is interesting to note that either of the above algorithms for
MPI_Allgather has the same cost as the tree algorithm for MPI_Gather!

For long messages or medium-size messages and non-power-of-two
no. of processes, we use a ring algorithm. In the first step, each
process i sends its contribution to process i+1 and receives
the contribution from process i-1 (with wrap-around). From the
second step onwards, each process i forwards to process i+1 the
data it received from process i-1 in the previous step. This takes
a total of p-1 steps.

Cost = (p-1).alpha + n.((p-1)/p).beta

We use this algorithm instead of recursive doubling for long
messages because we find that this communication pattern (nearest
neighbor) performs twice as fast as recursive doubling for long
messages (on Myrinet and IBM SP).

Possible improvements:

End Algorithm: MPI_Allgather
*/
/* begin:nested */
/* not declared static because a machine-specific function may call this
one in some cases */

int MPIR_Allgather_intra_MVP(const void *sendbuf, int sendcount,
                             MPI_Datatype sendtype, void *recvbuf,
                             int recvcount, MPI_Datatype recvtype,
                             MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int type_size;
    int comm_size;
    int msg_size;
    int threshold;

    if (((sendcount == 0) && (sendbuf != MPI_IN_PLACE)) || (recvcount == 0)) {
        return MPI_SUCCESS;
    }

    MPIR_Datatype_get_size_macro(recvtype, type_size);
    comm_size = comm_ptr->local_size;
    msg_size = recvcount * type_size;

    if (comm_ptr->dev.ch.is_pof2) {
        threshold = MVP_CVAR_IS_SET_BY_USER(MVP_ALLGATHER_RD_THRESHOLD) ?
                        MVP_ALLGATHER_RD_THRESHOLD :
                        allgather_tuning(comm_size);
        if (msg_size <= threshold) {
            /* Short or medium size message and power-of-two no. of processes.
             * Use recursive doubling algorithm */
            return MPIR_Allgather_RD_MVP(sendbuf, sendcount, sendtype, recvbuf,
                                         recvcount, recvtype, comm_ptr,
                                         errflag);
        }
    } else {
        threshold = MVP_CVAR_IS_SET_BY_USER(MVP_ALLGATHER_BRUCK_THRESHOLD) ?
                        MVP_ALLGATHER_BRUCK_THRESHOLD :
                        allgather_tuning(comm_size);
        if (msg_size <= threshold) {
            /* Short message and non-power-of-two no. of processes. Use Bruck
             * algorithm (see description above). */
            return MPIR_Allgather_Bruck_MVP(sendbuf, sendcount, sendtype,
                                            recvbuf, recvcount, recvtype,
                                            comm_ptr, errflag);
        }
    }
    /* Long message or medium-size message use ring algorithm. */
    return MPIR_Allgather_Ring_MVP(sendbuf, sendcount, sendtype, recvbuf,
                                   recvcount, recvtype, comm_ptr, errflag);
}

int MPIR_Allgather_index_tuned_intra_MVP(const void *sendbuf, int sendcount,
                                         MPI_Datatype sendtype, void *recvbuf,
                                         int recvcount, MPI_Datatype recvtype,
                                         MPIR_Comm *comm_ptr,
                                         MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;
    MPI_Aint nbytes = 0, recvtype_size;
    int comm_size;
    int comm_size_index = 0;
    int inter_node_algo_index = 0;
    int local_size = 0;
    int partial_sub_ok = 0;
    int conf_index = 0;
    int table_min_comm_size = 0;
    int table_max_comm_size = 0;
    int table_min_inter_size = 0;
    int table_max_inter_size = 0;
    int last_inter;
    int lp2ltn; // largest power of 2 less than n
    int lp2ltn_min;
    MPI_Comm shmem_comm;
    MPIR_Comm *shmem_commptr = NULL;

    /* Get the size of the communicator */
    comm_size = comm_ptr->local_size;

    MPIR_Datatype_get_size_macro(recvtype, recvtype_size);
    nbytes = recvtype_size * recvcount;

    int i, rank;
    MPI_Aint recvtype_extent;
    MPIR_Datatype_get_extent_macro(recvtype, recvtype_extent);
    rank = MPIR_Comm_rank(comm_ptr);

    if (MVP_USE_OLD_ALLGATHER) {
        MPIR_Allgather_intra_MVP(sendbuf, sendcount, sendtype, recvbuf,
                                 recvcount, recvtype, comm_ptr, errflag);
        goto fn_exit;
    }

    /* check if safe to use partial subscription mode */
    if (comm_ptr->dev.ch.shmem_coll_ok == 1 && comm_ptr->dev.ch.is_uniform) {
        shmem_comm = comm_ptr->dev.ch.shmem_comm;
        MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
        local_size = shmem_commptr->local_size;
        i = 0;
        if (mvp_allgather_indexed_table_ppn_conf[0] == -1) {
            // Indicating user defined tuning
            conf_index = 0;
            goto conf_check_end;
        }
        if ((comm_ptr->dev.ch.allgather_comm_ok != 0) &&
            (comm_ptr->dev.ch.is_blocked == 0 &&
             (MVP_ALLGATHER_CYCLIC_ALGO_THRESHOLD <= nbytes ||
              MVP_ALLGATHER_RING_ALGO_THRESHOLD <= nbytes))) {
            /* for large messages or nonblocked hostfiles, use ring-allgather
             * algorithm. */
            mpi_errno = MPIR_2lvl_Allgather_Ring_nonblocked_MVP(
                sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype,
                comm_ptr, errflag);
            goto fn_exit;
        }

        FIND_PPN_INDEX(allgather, local_size, conf_index, partial_sub_ok)
    }

    if (partial_sub_ok != 1) {
        conf_index = mvp_allgather_indexed_num_ppn_conf / 2;
    }

conf_check_end:

    /* Search for the corresponding system size inside the tuning table */
    /*
     * Comm sizes progress in powers of 2. Therefore comm_size can just be
     * indexed instead
     */
    table_min_comm_size =
        mvp_allgather_indexed_thresholds_table[conf_index][0].numproc;
    table_max_comm_size =
        mvp_allgather_indexed_thresholds_table
            [conf_index]
            [mvp_size_allgather_indexed_tuning_table[conf_index] - 1]
                .numproc;

    if (comm_size < table_min_comm_size) {
        /* Comm size smaller than smallest configuration in table: use smallest
         * available */
        comm_size_index = 0;
    } else if (comm_size > table_max_comm_size) {
        /* Comm size larger than largest configuration in table: use largest
         * available */
        comm_size_index =
            mvp_size_allgather_indexed_tuning_table[conf_index] - 1;
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
        mvp_allgather_indexed_thresholds_table[conf_index][comm_size_index]
            .size_inter_table -
        1;
    table_min_inter_size =
        mvp_allgather_indexed_thresholds_table[conf_index][comm_size_index]
            .inter_leader[0]
            .msg_sz;
    table_max_inter_size =
        mvp_allgather_indexed_thresholds_table[conf_index][comm_size_index]
            .inter_leader[last_inter]
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

    /* Set inter-leader pt */
    MVP_Allgather_function =
        mvp_allgather_indexed_thresholds_table[conf_index][comm_size_index]
            .inter_leader[inter_node_algo_index]
            .allgather_fn;

    if (MVP_Allgather_function == &MPIR_Allgather_Bruck_MVP ||
        MVP_Allgather_function == &MPIR_Allgather_RD_MVP ||
        MVP_Allgather_function == &MPIR_Allgather_Ring_MVP ||
        MVP_Allgather_function == &MPIR_Allgather_Direct_MVP ||
        MVP_Allgather_function == &MPIR_Allgather_DirectSpread_MVP ||
        MVP_Allgather_function == &MPIR_Allgather_RD_Allgather_Comm_MVP) {
        mpi_errno =
            MVP_Allgather_function(sendbuf, sendcount, sendtype, recvbuf,
                                   recvcount, recvtype, comm_ptr, errflag);
    } else if ((comm_ptr->dev.ch.shmem_coll_ok == 1 &&
                comm_ptr->dev.ch.rank_list != NULL) &&
               sendbuf != MPI_IN_PLACE &&
               (MVP_Allgather_function == &MPIR_Allgather_gather_bcast_MVP ||
                MVP_Allgather_function == &MPIR_2lvl_Allgather_nonblocked_MVP ||
                MVP_Allgather_function ==
                    &MPIR_2lvl_Allgather_Ring_nonblocked_MVP ||
                MVP_Allgather_function == &MPIR_2lvl_Allgather_Direct_MVP ||
                MVP_Allgather_function == &MPIR_2lvl_Allgather_Ring_MVP)) {
        mpi_errno =
            MVP_Allgather_function(sendbuf, sendcount, sendtype, recvbuf,
                                   recvcount, recvtype, comm_ptr, errflag);
    } else {
        mpi_errno =
            MPIR_Allgather_allcomm_auto(sendbuf, sendcount, sendtype, recvbuf,
                                        recvcount, recvtype, comm_ptr, errflag);
    }
    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIR_Allgather_MVP(const void *sendbuf, int sendcount,
                       MPI_Datatype sendtype, void *recvbuf, int recvcount,
                       MPI_Datatype recvtype, MPIR_Comm *comm_ptr,
                       MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Allgather_index_tuned_intra_MVP(
        sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm_ptr,
        errflag);
    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
