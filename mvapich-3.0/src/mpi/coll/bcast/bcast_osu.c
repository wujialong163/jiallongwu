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
#include <unistd.h>
#include "mvp_coll_shmem.h"
#include <unistd.h>
#include "mvp_common_tuning.h"
#include "bcast_tuning.h"
#if defined(_MCST_SUPPORT_)
#include "ibv_mcast.h"
#endif

/*
=== BEGIN_MPI_T_MVP_CVAR_INFO_BLOCK ===

cvars:
    - name        : MVP_BCAST_TWO_LEVEL_SYSTEM_SIZE
      category    : COLLECTIVE
      type        : int
      default     : 64
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_KNOMIAL_2LEVEL_BCAST_MESSAGE_SIZE_THRESHOLD
      category    : COLLECTIVE
      type        : int
      default     : 2048
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_KNOMIAL_2LEVEL_BCAST_SYSTEM_SIZE_THRESHOLD
      category    : COLLECTIVE
      type        : int
      default     : 64
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_BCAST_SHORT_MSG
      category    : COLLECTIVE
      type        : int
      default     : 16384
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_KNOMIAL_2LEVEL_BCAST
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_KNOMIAL_INTER_LEADER_BCAST
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_SHMEM_BCAST
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This parameter can be used to turn off shared memory based
        MPI_Bcast for OFA-IB-CH3 over IBA by setting this to 0.

    - name        : MVP_BCAST_COLLECTIVE_ALGORITHM
      category    : COLLECTIVE
      type        : enum
      default     : UNSET
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : |-
        This CVAR selects the broadcast collective algorithm. In order to get a
        two level operation you must set MVP_BCAST_TUNING_IS_TWO_LEVEL=1.
        (NOTE: This will override MVP_BCAST_INTER_NODE_TUNING_ALGO and
        MVP_BCAST_INTRA_NODE_TUNING_ALGO).
        UNSET   - No algorithm selected
        KNOMIAL - Sets both inter and intra node algo to the Knomial algorithm

    - name        : MVP_BCAST_INTER_NODE_TUNING_ALGO
      alias       : MVP_INTER_BCAST_TUNING
      category    : COLLECTIVE
      type        : enum
      default     : UNSET
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : |-
        Variable to select the inter node broadcast algorithm. (This can be
        overriden by MVP_BCAST_COLLECTIVE_ALGORITHM)
        UNSET                       - Internal algorithm selection
        BINOMIAL                    - A binary tree-based broadcast algorithm
                                    that has low memory requirements but can be
                                    slow for large messages.
        SCATTER_DOUBLING_ALLGATHER  - A hybrid algorithm that first scatters the
                                    data using a recursive doubling algorithm
                                    and then gathers it using an allgather
                                    algorithm.
        SCATTER_RING_ALLGATHER      - A hybrid algorithm that first scatters the
                                    data using a ring-based algorithm and then
                                    gathers it using an allgather algorithm.
        SCATTER_RING_ALLGATHER_SHM  - A variant of SCATTER_RING_ALLGATHER that
                                    uses shared memory to reduce communication
                                    overhead.
        KNOMIAL                     - A knomial tree-based broadcast algorithm
                                    that can be faster than BINOMIAL for large
                                    messages.
        PIPELINED                   - A pipelined broadcast algorithm that
                                    overlaps communication and computation to
                                    reduce latency.

    - name        : MVP_BCAST_INTRA_NODE_TUNING_ALGO
      alias       : MVP_INTRA_BCAST_TUNING
      category    : COLLECTIVE
      type        : enum
      default     : UNSET
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : |-
        Variable to select the intra node broadcast algorithm. (This can be
        overriden by MVP_BCAST_COLLECTIVE_ALGORITHM)
        UNSET       - Internal algorithm selection
        SHMEM       - Uses shared memory to perform broadcast within a node.
        KNOMIAL     - Uses a k-ary tree algorithm to perform broadcast within a
                    node.

    - name        : MVP_BCAST_TUNING_IS_TWO_LEVEL
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

/* A binomial tree broadcast algorithm.  Good for short messages,
   Cost = lgp.alpha + n.lgp.beta */

MVP_Bcast_fn_t MVP_Bcast_function = NULL;
MVP_Bcast_fn_t MVP_Bcast_intra_node_function = NULL;

static int MPIR_Bcast_tune_inter_node_helper_MVP(void *buffer, int count,
                                                 MPI_Datatype datatype,
                                                 int root, MPIR_Comm *comm_ptr,
                                                 MPIR_Errflag_t *errflag)
{
    int rank;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    MPI_Aint type_size;
    intptr_t nbytes = 0;
    MPI_Comm shmem_comm, leader_comm;
    MPIR_Comm *shmem_commptr = NULL, *leader_commptr = NULL;
    int local_rank, local_size, global_rank = -1;
    int leader_root, leader_of_root;

    rank = comm_ptr->rank;

    shmem_comm = comm_ptr->dev.ch.shmem_comm;
    MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
    local_rank = shmem_commptr->rank;
    local_size = shmem_commptr->local_size;

    leader_comm = comm_ptr->dev.ch.leader_comm;
    MPIR_Comm_get_ptr(leader_comm, leader_commptr);

    if ((local_rank == 0) && (local_size > 1)) {
        global_rank = leader_commptr->rank;
    }

    leader_of_root = comm_ptr->dev.ch.leader_map[root];
    leader_root = comm_ptr->dev.ch.leader_rank[leader_of_root];
    MPIR_Datatype_get_size_macro(datatype, type_size);
    nbytes = (intptr_t)(count) * (type_size);

    if (local_size > 1) {
        if ((local_rank == 0) && (root != rank) &&
            (leader_root == global_rank)) {
            MPIR_PVAR_INC(bcast, tune_inter_node_helper, recv, count, datatype);
            mpi_errno = MPIC_Recv(buffer, count, datatype, root, MPIR_BCAST_TAG,
                                  comm_ptr, MPI_STATUS_IGNORE, errflag);
            if (mpi_errno) {
                /* for communication errors, just record the error but continue
                 */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
        }
        if ((local_rank != 0) && (root == rank)) {
            MPIR_PVAR_INC(bcast, tune_inter_node_helper, send, count, datatype);
            mpi_errno = MPIC_Send(buffer, count, datatype, leader_of_root,
                                  MPIR_BCAST_TAG, comm_ptr, errflag);
            if (mpi_errno) {
                /* for communication errors, just record the error but continue
                 */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
        }
    }
#if defined(_MCST_SUPPORT_)
    if (MVP_SELECT_MCAST_BASED_BCAST(comm_ptr, nbytes)) {
        mpi_errno = MPIR_Mcast_inter_node_MVP(buffer, count, datatype, root,
                                              comm_ptr, errflag);
        if (mpi_errno == MPI_SUCCESS) {
            goto fn_exit;
        } else {
            goto fn_fail;
        }
    }
#endif

    if (local_rank == 0) {
        leader_comm = comm_ptr->dev.ch.leader_comm;
        root = leader_root;
        MPIR_Comm_get_ptr(leader_comm, leader_commptr);
        rank = leader_commptr->rank;
    }

    if (MVP_Bcast_function == &MPIR_Pipelined_Bcast_MVP) {
        mpi_errno = MPIR_Pipelined_Bcast_MVP(buffer, count, datatype, root,
                                             comm_ptr, errflag);
        MPIR_ERR_CHECK(mpi_errno);
    } else if (MVP_Bcast_function ==
               &MPIR_Bcast_scatter_ring_allgather_shm_MVP) {
        mpi_errno = MPIR_Bcast_scatter_ring_allgather_shm_MVP(
            buffer, count, datatype, leader_root, comm_ptr, errflag);
        MPIR_ERR_CHECK(mpi_errno);
    } else {
        if (local_rank == 0) {
            if (MVP_Bcast_function ==
                &MPIR_Knomial_Bcast_inter_node_wrapper_MVP) {
                mpi_errno = MPIR_Knomial_Bcast_inter_node_wrapper_MVP(
                    buffer, count, datatype, root, comm_ptr, errflag);
            } else {
                mpi_errno = MVP_Bcast_function(buffer, count, datatype, root,
                                               leader_commptr, errflag);
            }
            MPIR_ERR_CHECK(mpi_errno);
        }
    }

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

static int MPIR_Bcast_inter_node_helper_MVP(void *buffer, int count,
                                            MPI_Datatype datatype, int root,
                                            MPIR_Comm *comm_ptr,
                                            MPIR_Errflag_t *errflag)
{
    int rank;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    MPI_Aint type_size;
    intptr_t nbytes = 0;
    MPI_Comm shmem_comm, leader_comm;
    MPIR_Comm *shmem_commptr = NULL, *leader_commptr = NULL;
    int local_rank, local_size, global_rank = -1;
    int leader_root, leader_of_root;

    rank = comm_ptr->rank;

    shmem_comm = comm_ptr->dev.ch.shmem_comm;
    MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
    local_rank = shmem_commptr->rank;
    local_size = shmem_commptr->local_size;

    leader_comm = comm_ptr->dev.ch.leader_comm;
    MPIR_Comm_get_ptr(leader_comm, leader_commptr);

    if ((local_rank == 0) && (local_size > 1)) {
        global_rank = leader_commptr->rank;
    }

    leader_of_root = comm_ptr->dev.ch.leader_map[root];
    leader_root = comm_ptr->dev.ch.leader_rank[leader_of_root];
    MPIR_Datatype_get_size_macro(datatype, type_size);
    nbytes = (intptr_t)(count) * (type_size);

    if (local_size > 1) {
        if ((local_rank == 0) && (root != rank) &&
            (leader_root == global_rank)) {
            MPIR_PVAR_INC(bcast, inter_node_helper, recv, count, datatype);
            mpi_errno = MPIC_Recv(buffer, count, datatype, root, MPIR_BCAST_TAG,
                                  comm_ptr, MPI_STATUS_IGNORE, errflag);
            if (mpi_errno) {
                /* for communication errors, just record the error but continue
                 */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
        }
        if ((local_rank != 0) && (root == rank)) {
            MPIR_PVAR_INC(bcast, inter_node_helper, send, count, datatype);
            mpi_errno = MPIC_Send(buffer, count, datatype, leader_of_root,
                                  MPIR_BCAST_TAG, comm_ptr, errflag);
            if (mpi_errno) {
                /* for communication errors, just record the error but continue
                 */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
        }
    }
#if defined(_MCST_SUPPORT_)
    if (MVP_SELECT_MCAST_BASED_BCAST(comm_ptr, nbytes)) {
        mpi_errno = MPIR_Mcast_inter_node_MVP(buffer, count, datatype, root,
                                              comm_ptr, errflag);
        if (mpi_errno == MPI_SUCCESS) {
            goto fn_exit;
        }
    }
#endif

    if (MVP_USE_PIPELINE_BCAST && nbytes > MVP_BCAST_SEGMENT_SIZE) {
        mpi_errno = MPIR_Pipelined_Bcast_MVP(buffer, count, datatype,
                                             leader_root, comm_ptr, errflag);
        MPIR_ERR_CHECK(mpi_errno);
    } else {
        if (local_rank == 0) {
            leader_comm = comm_ptr->dev.ch.leader_comm;
            root = leader_root;
            MPIR_Comm_get_ptr(leader_comm, leader_commptr);
            rank = leader_commptr->rank;
        }

        if (MVP_USE_KNOMIAL_INTER_LEADER_BCAST &&
            nbytes <= MVP_KNOMIAL_INTER_LEADER_THRESHOLD) {
            if (local_rank == 0) {
                mpi_errno = MPIR_Knomial_Bcast_inter_node_wrapper_MVP(
                    buffer, count, datatype, root, comm_ptr, errflag);
            }
        } else {
            if (MVP_USE_SCATTER_RING_INTER_LEADER_BCAST) {
                if (MVP_BCAST_SCATTER_RING_OVERLAP) {
                    if (nbytes <=
                            MVP_BCAST_SCATTER_RING_OVERLAP_MSG_UPPERBOUND &&
                        comm_ptr->local_size >=
                            MVP_BCAST_SCATTER_RING_OVERLAP_CORES_LOWERBOUND) {
                        mpi_errno = MPIR_Bcast_scatter_ring_allgather_shm_MVP(
                            buffer, count, datatype, leader_root, comm_ptr,
                            errflag);
                    } else if (local_rank == 0) {
                        mpi_errno = MPIR_Bcast_scatter_ring_allgather_MVP(
                            buffer, count, datatype, root, leader_commptr,
                            errflag);
                    }
                } else if (local_rank == 0) {
                    mpi_errno = MPIR_Bcast_scatter_ring_allgather_MVP(
                        buffer, count, datatype, root, leader_commptr, errflag);
                }

            } else if (local_rank == 0) {
                if (MVP_USE_SCATTER_RD_INTER_LEADER_BCAST) {
                    mpi_errno =
                        MPIR_Bcast_intra_scatter_recursive_doubling_allgather(
                            buffer, count, datatype, root, leader_commptr,
                            errflag);
                } else if (MVP_USE_KNOMIAL_INTER_LEADER_BCAST) {
                    mpi_errno = MPIR_Knomial_Bcast_inter_node_wrapper_MVP(
                        buffer, count, datatype, root, comm_ptr, errflag);
                } else {
                    mpi_errno = MPIR_Bcast_binomial_MVP(
                        buffer, count, datatype, root, leader_commptr, errflag);
                }
                MPIR_ERR_CHECK(mpi_errno);
            }
        }
    }

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIR_Bcast_intra_MVP(void *buffer, int count, MPI_Datatype datatype,
                         int root, MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    int comm_size, rank;
    int two_level_bcast = 1;
    intptr_t nbytes = 0;
    int is_homogeneous, is_contig;
    MPI_Aint type_size, position;
    void *tmp_buf = NULL;
    MPIR_Comm *shmem_commptr = NULL;
    MPI_Comm shmem_comm;
    MPIR_Datatype *dtp;

    MPIR_CHKLMEM_DECL(1);

    if (count == 0)
        goto fn_exit;

    comm_size = comm_ptr->local_size;
    rank = comm_ptr->rank;

    if (HANDLE_GET_KIND(datatype) == HANDLE_KIND_BUILTIN)
        is_contig = 1;
    else {
        MPIR_Datatype_get_ptr(datatype, dtp);
        is_contig = dtp->is_contig;
    }

    is_homogeneous = 1;
#ifdef MPID_HAS_HETERO
    if (comm_ptr->is_hetero)
        is_homogeneous = 0;
#endif

    /* MPI_Type_size() might not give the accurate size of the packed
     * datatype for heterogeneous systems (because of padding, encoding,
     * etc). On the other hand, MPI_Pack_size() can become very
     * expensive, depending on the implementation, especially for
     * heterogeneous systems. We want to use MPI_Type_size() wherever
     * possible, and MPI_Pack_size() in other places.
     */
    if (is_homogeneous) {
        MPIR_Datatype_get_size_macro(datatype, type_size);
    } else {
        MPIR_Pack_size_impl(1, datatype, &type_size);
    }
    nbytes = (intptr_t)(count) * (type_size);
    if (comm_size <= MVP_BCAST_TWO_LEVEL_SYSTEM_SIZE) {
        if (nbytes > MVP_BCAST_SHORT_MSG && nbytes < mvp_bcast_large_msg) {
            two_level_bcast = 1;
        } else {
            two_level_bcast = 0;
        }
    }

    if (comm_ptr->dev.ch.shmem_coll_ok == 1 && MVP_USE_SHMEM_BCAST &&
        (two_level_bcast == 1
#if defined(_MCST_SUPPORT_)
         || MVP_SELECT_MCAST_BASED_BCAST(comm_ptr, nbytes)
#endif
             )) {

        if (!is_contig || !is_homogeneous) {
            MPIR_CHKLMEM_MALLOC(tmp_buf, void *, nbytes, mpi_errno, "tmp_buf",
                                MPL_MEM_COLL);

            /* TODO: Pipeline the packing and communication */
            position = 0;
            if (rank == root) {
                mpi_errno = MPIR_Typerep_pack(buffer, count, datatype, position,
                                              tmp_buf, nbytes, &position);
                MPIR_ERR_CHECK(mpi_errno);
            }
        }

        shmem_comm = comm_ptr->dev.ch.shmem_comm;
        MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
        if (!is_contig || !is_homogeneous) {
            mpi_errno = MPIR_Bcast_inter_node_helper_MVP(
                tmp_buf, nbytes, MPI_BYTE, root, comm_ptr, errflag);
        } else {
            mpi_errno = MPIR_Bcast_inter_node_helper_MVP(
                buffer, count, datatype, root, comm_ptr, errflag);
        }
        if (mpi_errno) {
            /* for communication errors, just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }

        /* We are now done with the inter-node phase */
        if (comm_ptr->dev.ch.intra_node_done == 0) {
            if (nbytes <= MVP_KNOMIAL_INTRA_NODE_THRESHOLD) {
                if (!is_contig || !is_homogeneous) {
                    mpi_errno =
                        MPIR_Shmem_Bcast_MVP(tmp_buf, nbytes, MPI_BYTE, root,
                                             shmem_commptr, errflag);
                } else {
                    mpi_errno = MPIR_Shmem_Bcast_MVP(
                        buffer, count, datatype, root, shmem_commptr, errflag);
                }
            } else {
                if (!is_contig || !is_homogeneous) {
                    mpi_errno = MPIR_Knomial_Bcast_intra_node_MVP(
                        tmp_buf, nbytes, MPI_BYTE, INTRA_NODE_ROOT,
                        shmem_commptr, errflag);
                } else {
                    mpi_errno = MPIR_Knomial_Bcast_intra_node_MVP(
                        buffer, count, datatype, INTRA_NODE_ROOT, shmem_commptr,
                        errflag);
                }
            }
        }
        if (mpi_errno) {
            /* for communication errors, just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
        if (!is_contig || !is_homogeneous) {
            /* Finishing up... */
            if (rank != root) {
                position = 0;
                mpi_errno = MPIR_Typerep_unpack(tmp_buf, nbytes, buffer, count,
                                                datatype, position, &position);
            }
        }
    } else {
        if (nbytes <= MVP_BCAST_SHORT_MSG) {
            mpi_errno = MPIR_Bcast_binomial_MVP(buffer, count, datatype, root,
                                                comm_ptr, errflag);
        } else {
            if (MVP_USE_SCATTER_RD_INTER_LEADER_BCAST) {
                mpi_errno = MPIR_Bcast_scatter_ring_allgather_MVP(
                    buffer, count, datatype, root, comm_ptr, errflag);
            } else {
                mpi_errno =
                    MPIR_Bcast_intra_scatter_recursive_doubling_allgather(
                        buffer, count, datatype, root, comm_ptr, errflag);
            }
        }
        if (mpi_errno) {
            /* for communication errors, just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
    }

fn_exit:
    MPIR_CHKLMEM_FREEALL();
    if (mpi_errno_ret)
        mpi_errno = mpi_errno_ret;
    else if (*errflag)
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**coll_fail");
    return mpi_errno;

fn_fail:
    goto fn_exit;
}

int MPIR_Bcast_index_tuned_intra_MVP(void *buffer, int count,
                                     MPI_Datatype datatype, int root,
                                     MPIR_Comm *comm_ptr,
                                     MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    int comm_size, rank;
    int two_level_bcast = 1;
    intptr_t nbytes = 0;
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
    int is_homogeneous, is_contig;
    MPI_Aint type_size, position;
    void *tmp_buf = NULL;
    MPIR_Comm *shmem_commptr = NULL;
    MPI_Comm shmem_comm;
    MPIR_Datatype *dtp;

    MPIR_CHKLMEM_DECL(1);

    if (count == 0)
        goto fn_exit;

    comm_size = comm_ptr->local_size;
    rank = comm_ptr->rank;

    if (HANDLE_GET_KIND(datatype) == HANDLE_KIND_BUILTIN)
        is_contig = 1;
    else {
        MPIR_Datatype_get_ptr(datatype, dtp);
        is_contig = dtp->is_contig;
    }

    is_homogeneous = 1;
#ifdef MPID_HAS_HETERO
    if (comm_ptr->is_hetero)
        is_homogeneous = 0;
#endif

    /* MPI_Type_size() might not give the accurate size of the packed
     * datatype for heterogeneous systems (because of padding, encoding,
     * etc). On the other hand, MPI_Pack_size() can become very
     * expensive, depending on the implementation, especially for
     * heterogeneous systems. We want to use MPI_Type_size() wherever
     * possible, and MPI_Pack_size() in other places.
     */
    if (is_homogeneous) {
        MPIR_Datatype_get_size_macro(datatype, type_size);
    } else {
        MPIR_Pack_size_impl(1, datatype, &type_size);
    }
    nbytes = (intptr_t)(count) * (type_size);

    /* check if safe to use partial subscription mode */
    if (comm_ptr->dev.ch.shmem_coll_ok == 1 && comm_ptr->dev.ch.is_uniform) {
        shmem_comm = comm_ptr->dev.ch.shmem_comm;
        MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
        local_size = shmem_commptr->local_size;
        if (mvp_bcast_indexed_table_ppn_conf[0] == -1) {
            // Indicating user defined tuning
            conf_index = 0;
            goto conf_check_end;
        }
        if (nbytes <= MVP_TOPO_AWARE_BCAST_MAX_MSG &&
            nbytes >= MVP_TOPO_AWARE_BCAST_MIN_MSG &&
            MVP_ENABLE_SKIP_TUNING_TABLE_SEARCH &&
            nbytes <= MVP_COLL_SKIP_TABLE_THRESHOLD &&
            MVP_ENABLE_TOPO_AWARE_COLLECTIVES && MVP_USE_TOPO_AWARE_BCAST &&
            comm_ptr->dev.ch.topo_coll_ok == 1 &&
            local_size >= MVP_TOPO_AWARE_BCAST_PPN_THRESHOLD &&
            MVP_TOPO_AWARE_BCAST_NODE_THRESHOLD <=
                comm_ptr->dev.ch.leader_group_size) {
            MVP_Bcast_function = &MPIR_Bcast_topo_aware_hierarchical_MVP;
            goto skip_tuning_tables;
        }
        if (likely(MVP_USE_SHMEM_BCAST && MVP_ENABLE_SKIP_TUNING_TABLE_SEARCH &&
                   (nbytes <= MVP_COLL_SKIP_TABLE_THRESHOLD))) {
#if defined _MVP_CH4_OVERRIDE_
            MVP_Bcast_function = &MPIR_Bcast_binomial_MVP;
            MVP_Bcast_intra_node_function = &MPIR_Shmem_Bcast_MVP;
#endif
            two_level_bcast = 1;
            MVP_PIPELINED_ZCPY_BCAST_KNOMIAL_FACTOR = 8;
            MVP_KNOMIAL_INTER_NODE_FACTOR = 8;
            MVP_BCAST_SEGMENT_SIZE = 8192;
            goto skip_tuning_tables;
        }

        FIND_PPN_INDEX(bcast, local_size, conf_index, partial_sub_ok)
    }

    if (partial_sub_ok != 1) {
        conf_index = mvp_bcast_indexed_num_ppn_conf / 2;
    }

conf_check_end:

    /* Search for the corresponding system size inside the tuning table */
    /*
     * Comm sizes progress in powers of 2. Therefore comm_size can just be
     * indexed instead
     */
    table_min_comm_size =
        mvp_bcast_indexed_thresholds_table[conf_index][0].numproc;
    table_max_comm_size =
        mvp_bcast_indexed_thresholds_table
            [conf_index][mvp_size_bcast_indexed_tuning_table[conf_index] - 1]
                .numproc;

    if (comm_size < table_min_comm_size) {
        /* Comm size smaller than smallest configuration in table: use smallest
         * available */
        comm_size_index = 0;
    } else if (comm_size > table_max_comm_size) {
        /* Comm size larger than largest configuration in table: use largest
         * available */
        comm_size_index = mvp_size_bcast_indexed_tuning_table[conf_index] - 1;
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

    last_inter = mvp_bcast_indexed_thresholds_table[conf_index][comm_size_index]
                     .size_inter_table -
                 1;
    table_min_inter_size =
        mvp_bcast_indexed_thresholds_table[conf_index][comm_size_index]
            .inter_leader[0]
            .msg_sz;
    table_max_inter_size =
        mvp_bcast_indexed_thresholds_table[conf_index][comm_size_index]
            .inter_leader[last_inter]
            .msg_sz;
    last_intra = mvp_bcast_indexed_thresholds_table[conf_index][comm_size_index]
                     .size_intra_table -
                 1;
    table_min_intra_size =
        mvp_bcast_indexed_thresholds_table[conf_index][comm_size_index]
            .intra_node[0]
            .msg_sz;
    table_max_intra_size =
        mvp_bcast_indexed_thresholds_table[conf_index][comm_size_index]
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

    MVP_Bcast_function =
        mvp_bcast_indexed_thresholds_table[conf_index][comm_size_index]
            .inter_leader[inter_node_algo_index]
            .bcast_fn;

    MVP_Bcast_intra_node_function =
        mvp_bcast_indexed_thresholds_table[conf_index][comm_size_index]
            .intra_node[intra_node_algo_index]
            .bcast_fn;

    if (MVP_BCAST_INTRA_NODE_TUNING_ALGO_UNSET ==
            MVP_BCAST_INTRA_NODE_TUNING_ALGO &&
        MVP_Bcast_intra_node_function == &MPIR_Knomial_Bcast_intra_node_MVP &&
        nbytes < MVP_KNOMIAL_INTRA_NODE_THRESHOLD &&
        comm_ptr->dev.ch.shmem_coll_ok == 1) {
        MVP_Bcast_intra_node_function = &MPIR_Shmem_Bcast_MVP;
    }

    if (mvp_bcast_indexed_thresholds_table[conf_index][comm_size_index]
            .inter_leader[inter_node_algo_index]
            .zcpy_pipelined_knomial_factor > 0) {
        MVP_PIPELINED_ZCPY_BCAST_KNOMIAL_FACTOR =
            mvp_bcast_indexed_thresholds_table[conf_index][comm_size_index]
                .inter_leader[inter_node_algo_index]
                .zcpy_pipelined_knomial_factor;
    }


    /* If we use previous shmem scheme, fall back to previous threshold for
     * intra-node*/
    if (!MVP_USE_SLOT_SHMEM_COLL || !MVP_USE_SLOT_SHMEM_BCAST) {
        /* not depending on intra node tuning table with old shmem design */
        if (nbytes <= MVP_KNOMIAL_INTRA_NODE_THRESHOLD) {
            MVP_Bcast_intra_node_function = &MPIR_Shmem_Bcast_MVP;
        } else {
            MVP_Bcast_intra_node_function = &MPIR_Knomial_Bcast_intra_node_MVP;
        }
    } else if (MVP_Bcast_intra_node_function == NULL) {
        /* if tuning table do not have any intra selection, set func pointer to
        ** default one for mcast intra node */
        MVP_Bcast_intra_node_function = &MPIR_Shmem_Bcast_MVP;
    }

    /* Set value of pipeline segment size */
    MVP_BCAST_SEGMENT_SIZE =
        mvp_bcast_indexed_thresholds_table[conf_index][comm_size_index]
            .bcast_segment_size;

    /* Set value of inter node knomial factor */
    if (mvp_bcast_indexed_thresholds_table[conf_index][comm_size_index]
            .inter_node_knomial_factor > 0) {
        MVP_KNOMIAL_INTER_NODE_FACTOR =
            mvp_bcast_indexed_thresholds_table[conf_index][comm_size_index]
                .inter_node_knomial_factor;
    }
    /* Set value of intra node knomial factor */
    if (mvp_bcast_indexed_thresholds_table[conf_index][comm_size_index]
            .intra_node_knomial_factor > 0) {
        MVP_KNOMIAL_INTRA_NODE_FACTOR =
            mvp_bcast_indexed_thresholds_table[conf_index][comm_size_index]
                .intra_node_knomial_factor;
    }

    /* Check if we will use a two level algorithm or not */
    two_level_bcast =
        mvp_bcast_indexed_thresholds_table[conf_index][comm_size_index]
            .is_two_level_bcast[inter_node_algo_index];

    if (MVP_Bcast_function == &MPIR_Knomial_Bcast_inter_node_wrapper_MVP &&
        two_level_bcast != 1) {
        /* knomial inter node wrapper algorithm relies on leader_comm,
         * therefore, it most be called only on leader_comm */
        MVP_Bcast_function = &MPIR_Bcast_binomial_MVP;
    }

skip_tuning_tables:
    if (comm_ptr->dev.ch.shmem_coll_ok != 1) {
        if (nbytes < MPICH_LARGE_MSG_COLLECTIVE_SIZE) {
            mpi_errno = MPIR_Bcast_allcomm_auto(buffer, count, datatype, root,
                                                comm_ptr, errflag);
        } else {
            mpi_errno = MPIR_Bcast_scatter_ring_allgather_MVP(
                buffer, count, datatype, root, comm_ptr, errflag);
        }
    } else if (MVP_USE_SHMEM_BCAST && two_level_bcast) {
        if (!is_contig || !is_homogeneous) {
            MPIR_CHKLMEM_MALLOC(tmp_buf, void *, nbytes, mpi_errno, "tmp_buf",
                                MPL_MEM_COLL);

            /* TODO: Pipeline the packing and communication */
            position = 0;
            if (rank == root) {
                mpi_errno = MPIR_Typerep_pack(buffer, count, datatype, position,
                                              tmp_buf, nbytes, &position);
                MPIR_ERR_CHECK(mpi_errno);
            }
        }
            shmem_comm = comm_ptr->dev.ch.shmem_comm;
            MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
            if (!is_contig || !is_homogeneous) {
                mpi_errno = MPIR_Bcast_tune_inter_node_helper_MVP(
                    tmp_buf, nbytes, MPI_BYTE, root, comm_ptr, errflag);
            } else {
                mpi_errno = MPIR_Bcast_tune_inter_node_helper_MVP(
                    buffer, count, datatype, root, comm_ptr, errflag);
            }
            if (mpi_errno) {
                /* for communication errors, just record the error but continue
                 */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }

            /* We are now done with the inter-node phase */
            if (comm_ptr->dev.ch.intra_node_done == 0) {
                if (!is_contig || !is_homogeneous) {
                    mpi_errno = MVP_Bcast_intra_node_function(
                        tmp_buf, nbytes, MPI_BYTE, INTRA_NODE_ROOT,
                        shmem_commptr, errflag);
                } else {
                    mpi_errno = MVP_Bcast_intra_node_function(
                        buffer, count, datatype, INTRA_NODE_ROOT, shmem_commptr,
                        errflag);
                }
            }
        if (mpi_errno) {
            /* for communication errors, just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
        if (!is_contig || !is_homogeneous) {
            /* Finishing up... */
            if (rank != root) {
                position = 0;
                mpi_errno = MPIR_Typerep_unpack(tmp_buf, nbytes, buffer, count,
                                                datatype, position, &position);
            }
        }
    } else {
        /* We use Knomial for intra node */
        MVP_Bcast_intra_node_function = &MPIR_Knomial_Bcast_intra_node_MVP;
        if (!MVP_USE_SHMEM_BCAST) {
            /* Fall back to non-tuned version */
            MPIR_Bcast_intra_MVP(buffer, count, datatype, root, comm_ptr,
                                 errflag);
        } else {
            mpi_errno = MVP_Bcast_function(buffer, count, datatype, root,
                                           comm_ptr, errflag);
        }
    }

    if (mpi_errno) {
        /* for communication errors, just record the error but continue */
        *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
        MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
    }

fn_exit:
    MPIR_CHKLMEM_FREEALL();
    if (mpi_errno_ret)
        mpi_errno = mpi_errno_ret;
    else if (*errflag)
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**coll_fail");
    return mpi_errno;

fn_fail:
    goto fn_exit;
}

int MPIR_Bcast_tune_intra_MVP(void *buffer, int count, MPI_Datatype datatype,
                              int root, MPIR_Comm *comm_ptr,
                              MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    int comm_size, rank;
    int two_level_bcast = 1;
    intptr_t nbytes = 0;
    int range = 0;
    int range_threshold = 0;
    int range_threshold_intra = 0;
    int is_homogeneous, is_contig;
    MPI_Aint type_size, position;
    void *tmp_buf = NULL;
    MPIR_Comm *shmem_commptr = NULL;
    MPI_Comm shmem_comm;
    MPIR_Datatype *dtp;

    MPIR_CHKLMEM_DECL(1);

    if (count == 0)
        goto fn_exit;

    comm_size = comm_ptr->local_size;
    rank = comm_ptr->rank;

    if (HANDLE_GET_KIND(datatype) == HANDLE_KIND_BUILTIN)
        is_contig = 1;
    else {
        MPIR_Datatype_get_ptr(datatype, dtp);
        is_contig = dtp->is_contig;
    }

    is_homogeneous = 1;
#ifdef MPID_HAS_HETERO
    if (comm_ptr->is_hetero)
        is_homogeneous = 0;
#endif

    /* MPI_Type_size() might not give the accurate size of the packed
     * datatype for heterogeneous systems (because of padding, encoding,
     * etc). On the other hand, MPI_Pack_size() can become very
     * expensive, depending on the implementation, especially for
     * heterogeneous systems. We want to use MPI_Type_size() wherever
     * possible, and MPI_Pack_size() in other places.
     */
    if (is_homogeneous) {
        MPIR_Datatype_get_size_macro(datatype, type_size);
    } else {
        MPIR_Pack_size_impl(1, datatype, &type_size);
    }
    nbytes = (intptr_t)(count) * (type_size);

    /* Search for the corresponding system size inside the tuning table */
    while ((range < (mvp_size_bcast_tuning_table - 1)) &&
           (comm_size > mvp_bcast_thresholds_table[range].numproc)) {
        range++;
    }
    /* Search for corresponding inter-leader function */
    while (
        (range_threshold <
         (mvp_bcast_thresholds_table[range].size_inter_table - 1)) &&
        (nbytes >
         mvp_bcast_thresholds_table[range].inter_leader[range_threshold].max) &&
        (mvp_bcast_thresholds_table[range].inter_leader[range_threshold].max !=
         -1)) {
        range_threshold++;
    }

    /* Search for corresponding intra-node function */
    while ((range_threshold_intra <
            (mvp_bcast_thresholds_table[range].size_intra_table - 1)) &&
           (nbytes > mvp_bcast_thresholds_table[range]
                         .intra_node[range_threshold_intra]
                         .max) &&
           (mvp_bcast_thresholds_table[range]
                .intra_node[range_threshold_intra]
                .max != -1)) {
        range_threshold_intra++;
    }

    MVP_Bcast_function = mvp_bcast_thresholds_table[range]
                             .inter_leader[range_threshold]
                             .bcast_fn;

    MVP_Bcast_intra_node_function = mvp_bcast_thresholds_table[range]
                                        .intra_node[range_threshold_intra]
                                        .bcast_fn;

    if (MVP_BCAST_INTRA_NODE_TUNING_ALGO_UNSET ==
            MVP_BCAST_INTRA_NODE_TUNING_ALGO &&
        MVP_Bcast_intra_node_function == &MPIR_Knomial_Bcast_intra_node_MVP) {
        MVP_Bcast_intra_node_function = &MPIR_Shmem_Bcast_MVP;
    }

    if (mvp_bcast_thresholds_table[range]
            .inter_leader[range_threshold]
            .zcpy_pipelined_knomial_factor != -1) {
        MVP_PIPELINED_ZCPY_BCAST_KNOMIAL_FACTOR =
            mvp_bcast_thresholds_table[range]
                .inter_leader[range_threshold]
                .zcpy_pipelined_knomial_factor;
    }

    /* If we use previous shmem scheme, fall back to previous threshold for
     * intra-node*/
    if (!MVP_USE_SLOT_SHMEM_COLL || !MVP_USE_SLOT_SHMEM_BCAST) {
        /* not depending on intra node tuning table with old shmem design */
        if (nbytes <= MVP_KNOMIAL_INTRA_NODE_THRESHOLD) {
            MVP_Bcast_intra_node_function = &MPIR_Shmem_Bcast_MVP;
        } else {
            MVP_Bcast_intra_node_function = &MPIR_Knomial_Bcast_intra_node_MVP;
        }
    } else if (MVP_Bcast_intra_node_function == NULL) {
        /* if tuning table do not have any intra selection, set func pointer to
        ** default one for mcast intra node */
        MVP_Bcast_intra_node_function = &MPIR_Shmem_Bcast_MVP;
    }

    /* Set value of pipeline segment size */
    MVP_BCAST_SEGMENT_SIZE =
        mvp_bcast_thresholds_table[range].bcast_segment_size;

    /* Set value of inter node knomial factor */
    MVP_KNOMIAL_INTER_NODE_FACTOR =
        mvp_bcast_thresholds_table[range].inter_node_knomial_factor;

    /* Set value of intra node knomial factor */
    MVP_KNOMIAL_INTRA_NODE_FACTOR =
        mvp_bcast_thresholds_table[range].intra_node_knomial_factor;

    /* Check if we will use a two level algorithm or not */
    two_level_bcast =
        mvp_bcast_thresholds_table[range].is_two_level_bcast[range_threshold];
    if (comm_ptr->dev.ch.shmem_coll_ok != 1) {
        if (nbytes < MPICH_LARGE_MSG_COLLECTIVE_SIZE) {
            mpi_errno = MPIR_Bcast_allcomm_auto(buffer, count, datatype, root,
                                                comm_ptr, errflag);
        } else {
            mpi_errno = MPIR_Bcast_scatter_ring_allgather_MVP(
                buffer, count, datatype, root, comm_ptr, errflag);
        }
    } else if (MVP_USE_SHMEM_BCAST && two_level_bcast == 1) {
        if (!is_contig || !is_homogeneous) {
            MPIR_CHKLMEM_MALLOC(tmp_buf, void *, nbytes, mpi_errno, "tmp_buf",
                                MPL_MEM_COLL);

            /* TODO: Pipeline the packing and communication */
            position = 0;
            if (rank == root) {
                mpi_errno = MPIR_Typerep_pack(buffer, count, datatype, position,
                                              tmp_buf, nbytes, &position);
                MPIR_ERR_CHECK(mpi_errno);
            }
        }
            shmem_comm = comm_ptr->dev.ch.shmem_comm;
            MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
            if (!is_contig || !is_homogeneous) {
                mpi_errno = MPIR_Bcast_tune_inter_node_helper_MVP(
                    tmp_buf, nbytes, MPI_BYTE, root, comm_ptr, errflag);
            } else {
                mpi_errno = MPIR_Bcast_tune_inter_node_helper_MVP(
                    buffer, count, datatype, root, comm_ptr, errflag);
            }
            if (mpi_errno) {
                /* for communication errors, just record the error but continue
                 */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }

            /* We are now done with the inter-node phase */
            if (comm_ptr->dev.ch.intra_node_done == 0) {
                if (!is_contig || !is_homogeneous) {
                    mpi_errno = MVP_Bcast_intra_node_function(
                        tmp_buf, nbytes, MPI_BYTE, INTRA_NODE_ROOT,
                        shmem_commptr, errflag);
                } else {
                    mpi_errno = MVP_Bcast_intra_node_function(
                        buffer, count, datatype, INTRA_NODE_ROOT, shmem_commptr,
                        errflag);
                }
            }
        if (mpi_errno) {
            /* for communication errors, just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
        if (!is_contig || !is_homogeneous) {
            /* Finishing up... */
            if (rank != root) {
                position = 0;
                mpi_errno = MPIR_Typerep_unpack(tmp_buf, nbytes, buffer, count,
                                                datatype, position, &position);
            }
        }
    } else {
        /* We use Knomial for intra node */
        MVP_Bcast_intra_node_function = &MPIR_Knomial_Bcast_intra_node_MVP;
        if (!MVP_USE_SHMEM_BCAST) {
            /* Fall back to non-tuned version */
            MPIR_Bcast_intra_MVP(buffer, count, datatype, root, comm_ptr,
                                 errflag);
        } else {
            mpi_errno = MVP_Bcast_function(buffer, count, datatype, root,
                                           comm_ptr, errflag);
        }
    }

    if (mpi_errno) {
        /* for communication errors, just record the error but continue */
        *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
        MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
    }

fn_exit:
    MPIR_CHKLMEM_FREEALL();
    if (mpi_errno_ret)
        mpi_errno = mpi_errno_ret;
    else if (*errflag)
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**coll_fail");
    return mpi_errno;

fn_fail:
    goto fn_exit;
}

int MPIR_Bcast_MVP(void *buf, int count, MPI_Datatype datatype, int root,
                   MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_T_PVAR_COMM_COUNTER_INC(MVP, mvp_coll_bcast_subcomm, 1, comm_ptr);
    if (!MVP_USE_OLD_BCAST) {
        /* Use the new tuned bcast */
            mpi_errno = MPIR_Bcast_index_tuned_intra_MVP(
                buf, count, datatype, root, comm_ptr, errflag);
    } else {
        /* Use the previous tuned bcast */
        mpi_errno =
            MPIR_Bcast_intra_MVP(buf, count, datatype, root, comm_ptr, errflag);
    }
    comm_ptr->dev.ch.intra_node_done = 0;
    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
