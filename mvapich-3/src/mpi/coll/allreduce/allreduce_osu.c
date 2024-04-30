/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
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
#include <stdio.h>
#include "mpiimpl.h"
#include "mvp_coll_shmem.h"
#include "mvp_common_tuning.h"
#include "allreduce_tuning.h"
#include "bcast_tuning.h"
#include "reduce_tuning.h"
#if defined(CKPT)
#include <cr.h>
#endif

#if defined(_SHARP_SUPPORT_)
#include "mvp_sharp.h"
#endif

/*
=== BEGIN_MPI_T_MVP_CVAR_INFO_BLOCK ===

cvars:
    - name        : MVP_ALLREDUCE_2LEVEL_MSG
      category    : COLLECTIVE
      type        : int
      default     : (1 << 18)
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This parameter can be used to determine the threshold for the
        2-level Allreduce algorithm. We now use the
        shared-memory-based algorithm for messages smaller than the
        MVP_SHMEM_ALLREDUCE_MSG threshold, the 2-level algorithm for
        medium sized messages up to the threshold defined by this
        parameter. We use the default point-to-point algorithms
        messages larger than this threshold.

    - name        : MVP_ALLREDUCE_SHORT_MSG
      category    : COLLECTIVE
      type        : int
      default     : 2048
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_SHMEM_ALLREDUCE_MSG
      category    : COLLECTIVE
      type        : int
      default     : (1<<15)
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        The SHMEM AllReduce is used for messages less than this
        threshold.

    - name        : MVP_USE_SHMEM_ALLREDUCE
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This parameter can be used to turn off shared memory based
        MPI_Allreduce for OFA-IB-CH3 over IBA by setting this to 0.

    - name        : MVP_USE_SOCKET_AWARE_ALLREDUCE
      category    : COLLECTIVE
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_SOCKET_AWARE_SHARP_ALLREDUCE
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_SOCKET_AWARE_ALLREDUCE_MAX_MSG
      category    : COLLECTIVE
      type        : int
      default     : 2048
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_SOCKET_AWARE_ALLREDUCE_MIN_MSG
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_ALLREDUCE_COLLECTIVE_ALGORITHM
      category    : COLLECTIVE
      type        : enum
      default     : UNSET
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : |-
        This CVAR selects proper collective algorithm for inter and intra node
        allreduce operations. To set this operation to a two level operation
        then you must seperately set MVP_ALLREDUCE_TUNING_IS_TWO_LEVEL to 1.
        (NOTE: This will override MVP_ALLREDUCE_INTER_NODE_TUNING_ALGO and
        MVP_ALLREDUCE_INTRA_NODE_TUNING_ALGO).
        UNSET       - No algorithm selected, will default to algo selected by
                    CVARs MVP_ALLREDUCE_INTER_NODE_TUNING_ALGO and
                    MVP_ALLREDUCE_INTRA_NODE_TUNING_ALGO.

    - name        : MVP_ALLREDUCE_INTER_NODE_TUNING_ALGO
      alias       : MVP_INTER_ALLREDUCE_TUNING
      category    : COLLECTIVE
      type        : enum
      default     : UNSET
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : |-
        Variable to select the inter node allreduce algorithm. To use a two
        level operation you still must seperately set
        MVP_ALLREDUCE_TUNING_IS_TWO_LEVEL=1. To use an MCAST algorithm, you must
        also set MVP_USE_MCAST_ALLREDUCE=1 and MVP_USE_MCAST=1. (This CVAR can
        be overriden by MVP_ALLREDUCE_COLLECTIVE_ALGORITHM)
        UNSET       - Default to internal algorithm selection
        P2P_RD      - This algorithm uses a peer-to-peer recursive doubling
                    approach to perform the allreduce operation among nodes.
                    The process starts with each node sending its data to its
                    "peer" node, which then combines the data and sends it to
                    its peer, and so on until all nodes have received the
                    combined data.
        P2P_RSA     - This algorithm uses a peer-to-peer recursive scatter and
                    allgather approach to perform the allreduce operation among
                    nodes. The process starts with each node recursively
                    splitting its data into smaller subsets and sending them to
                    its peers, and then receiving and combining the subsets
                    received from peers until all nodes have the combined data.
        MCAST_2LVL  - This algorithm uses a two-level multicast approach to
                    perform the allreduce operation among nodes. The process
                    starts with a root node sending the data to two intermediate
                    nodes, which then multicast the data to the remaining nodes
                    in a tree-like structure.
        MCAST_RSA   - This algorithm uses a recursive scatter and allgather
                    approach to perform the multicast and allreduce operations
                    among nodes. The process starts with each node recursively
                    splitting its data into smaller subsets and sending them to
                    its peers, and then receiving and combining the subsets
                    received from peers until all nodes have the combined data.
        RSA         - This is a flat reduce-scatter-allgather allreduce using
                    MPR_reduce_scatter and MPR_Allgather instead of uisng
                    Send/Recv like other functions. This function uses
                    reduce_scatter and allgather colls.
        RING        - This algorithm uses a ring topology to perform the
                    allreduce operation among nodes. The process starts with
                    each node sending its data to its "neighbor" node on the
                    ring, which then combines the data and sends it to its
                    neighbor, and so on until all nodes have the combined data.
        SOCK_AWARE  - This algorithm optimizes performance on systems with
                    multiple sockets by taking into account the placement of
                    MPI processes on the sockets during the allreduce operation
                    among nodes. The algorithm uses a combination of recursive
                    doubling and ring-based allreduce to minimize the amount of
                    inter-socket communication required.

    - name        : MVP_ALLREDUCE_INTRA_NODE_TUNING_ALGO
      alias       : MVP_INTRA_ALLREDUCE_TUNING
      category    : COLLECTIVE
      type        : enum
      default     : UNSET
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : |-
        Variable to select the intra node allreduce algorithm. (This can be
        overriden by MVP_ALLREDUCE_COLLECTIVE_ALGORITHM)
        UNSET       - Default to internal algorithm selection.
        P2P         - Intra-node p2p reduce as the first reduce in allreduce.
        SHMEM       - Intra-node shm reduce as the first reduce in allreduce.
        P2P_RD      - This algorithm uses a peer-to-peer recursive doubling
                    approach to perform the allreduce operation among nodes.
                    The process starts with each node sending its data to its
                    "peer" node, which then combines the data and sends it to
                    its peer, and so on until all nodes have received the
                    combined data.
        P2P_RSA     - This algorithm uses a peer-to-peer recursive scatter and
                    allgather approach to perform the allreduce operation among
                    nodes. The process starts with each node recursively
                    splitting its data into smaller subsets and sending them to
                    its peers, and then receiving and combining the subsets
                    received from peers until all nodes have the combined data.

    - name        : MVP_ALLREDUCE_TUNING_IS_TWO_LEVEL
      alias       : MVP_INTER_ALLREDUCE_TUNING_TWO_LEVEL
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

MVP_load_imbalance_time mvp_allreduce_load_imbalance;

extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allreduce_subcomm;
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_subcomm;

extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_count_recv;

#define RING_ALLREDUCE_FALLBACK(_sendbuf, _recvbuf, _count, _datatype, _op,    \
                                _comm_ptr, _errflag, _nbytes, _comm_size)      \
    do {                                                                       \
        if (64 * 1024 <= _nbytes / _comm_size) {                               \
            mpi_errno = MPIR_Allreduce_pt2pt_ring_wrapper_MVP(                 \
                _sendbuf, _recvbuf, _count, _datatype, _op, _comm_ptr,         \
                _errflag);                                                     \
        } else {                                                               \
            mpi_errno = MPIR_Allreduce_pt2pt_rs_MVP(_sendbuf, _recvbuf,        \
                                                    _count, _datatype, _op,    \
                                                    _comm_ptr, _errflag);      \
        }                                                                      \
    } while (0);

#define ALLREDUCE_SKIP_SMALL_MESSAGE_TUNING_TABLES                             \
    do {                                                                       \
        /* skip the tables for small messages using topo aware collectives  */ \
        if (nbytes <= MVP_TOPO_AWARE_ALLREDUCE_MAX_MSG &&                      \
            nbytes >= MVP_TOPO_AWARE_ALLREDUCE_MIN_MSG &&                      \
            MVP_ENABLE_TOPO_AWARE_COLLECTIVES &&                               \
            MVP_USE_TOPO_AWARE_ALLREDUCE &&                                    \
            comm_ptr->dev.ch.topo_coll_ok == 1 && is_commutative) {            \
            if (local_size < mvp_topo_aware_allreduce_ppn_threshold) {         \
                goto use_tables;                                               \
            }                                                                  \
            mpi_errno = MPIR_Allreduce_topo_aware_hierarchical_MVP(            \
                sendbuf, recvbuf, count, datatype, op, comm_ptr, errflag);     \
            return mpi_errno;                                                  \
        }                                                                      \
        /* skip the tables for small messages using sock aware collectives  */ \
        if (nbytes <= MVP_SOCKET_AWARE_ALLREDUCE_MAX_MSG &&                    \
            nbytes >= MVP_SOCKET_AWARE_ALLREDUCE_MIN_MSG &&                    \
            MVP_ENABLE_SOCKET_AWARE_COLLECTIVES &&                             \
            MVP_USE_SOCKET_AWARE_ALLREDUCE &&                                  \
            comm_ptr->dev.ch.use_intra_sock_comm == 1 && is_commutative) {     \
            if (local_size < MVP_SOCKET_AWARE_ALLREDUCE_PPN_THRESHOLD) {       \
                goto use_tables;                                               \
            }                                                                  \
            mpi_errno = MPIR_Allreduce_socket_aware_two_level_MVP(             \
                sendbuf, recvbuf, count, datatype, op, comm_ptr, errflag);     \
            return mpi_errno;                                                  \
        }                                                                      \
                                                                               \
        /* skip the tables for small messages using single leader shmem        \
         * collectives */                                                      \
        if (likely(MVP_USE_SHMEM_ALLREDUCE &&                                  \
                   MVP_ENABLE_SKIP_TUNING_TABLE_SEARCH &&                      \
                   (nbytes <= MVP_COLL_SKIP_TABLE_THRESHOLD))) {               \
            /* for small messages, force Shmem + RD */                         \
            MVP_Allreduce_intra_function = MPIR_Allreduce_reduce_shmem_MVP;    \
            MVP_Allreduce_function = MPIR_Allreduce_pt2pt_rd_MVP;              \
            is_two_level = 1;                                                  \
            goto skip_tuning_tables;                                           \
        }                                                                      \
                                                                               \
    } while (0);

#define ALLREDUCE_SKIP_LARGE_MESSAGE_TUNING_TABLES                             \
    do {                                                                       \
        /* skip the tables for large messages with low PPN using ring          \
         * collective */                                                       \
        if (MVP_ALLRED_USE_RING &&                                             \
            MVP_ALLREDUCE_RING_ALGO_THRESHOLD <= nbytes &&                     \
            local_size <= MVP_ALLREDUCE_RING_ALGO_PPN_THRESHOLD) {             \
            /* flat ring-based Allreduce */                                    \
            return MPIR_Allreduce_pt2pt_ring_wrapper_MVP(                      \
                sendbuf, recvbuf, count, datatype, op, comm_ptr, errflag);     \
        }                                                                      \
                                                                               \
        /* skip the tables for large messages with high PPN using ring-based   \
         * reduce_scatter followed by Allgather  collective */                 \
        if ((comm_ptr->dev.ch.allgather_comm_ok != 0 &&                        \
             comm_ptr->dev.ch.is_blocked == 0 &&                               \
             MVP_ALLREDUCE_CYCLIC_ALGO_THRESHOLD <= nbytes) ||                 \
            (MVP_ALLREDUCE_RED_SCAT_ALLGATHER_ALGO_THRESHOLD <= nbytes &&      \
             local_size >                                                      \
                 MVP_ALLREDUCE_RED_SCAT_ALLGATHER_ALGO_PPN_THRESHOLD)) {       \
            /* for large messages or cyclic hostfiles for medium messages, use \
             * red-scat-allgather algorithm  */                                \
            return MPIR_Allreduce_pt2pt_reduce_scatter_allgather_MVP(          \
                sendbuf, recvbuf, count, datatype, op, comm_ptr, errflag);     \
        }                                                                      \
    } while (0);

/* This is the default implementation of allreduce. The algorithm is:

   Algorithm: MPI_Allreduce

   For the heterogeneous case, we call MPI_Reduce followed by MPI_Bcast
   in order to meet the requirement that all processes must have the
   same result. For the homogeneous case, we use the following algorithms.

   For long messages and for builtin ops and if count >= pof2 (where
   pof2 is the nearest power-of-two less than or equal to the number
   of processes), we use Rabenseifner's algorithm (see
   http://www.hlrs.de/organization/par/services/models/mpi/myreduce.html ).
   This algorithm implements the allreduce in two steps: first a
   reduce-scatter, followed by an allgather. A recursive-halving
   algorithm (beginning with processes that are distance 1 apart) is
   used for the reduce-scatter, and a recursive doubling
   algorithm is used for the allgather. The non-power-of-two case is
   handled by dropping to the nearest lower power-of-two: the first
   few even-numbered processes send their data to their right neighbors
   (rank+1), and the reduce-scatter and allgather happen among the remaining
   power-of-two processes. At the end, the first few even-numbered
   processes get the result from their right neighbors.

   For the power-of-two case, the cost for the reduce-scatter is
   lgp.alpha + n.((p-1)/p).beta + n.((p-1)/p).gamma. The cost for the
   allgather lgp.alpha + n.((p-1)/p).beta. Therefore, the
   total cost is:
   Cost = 2.lgp.alpha + 2.n.((p-1)/p).beta + n.((p-1)/p).gamma

   For the non-power-of-two case,
   Cost = (2.floor(lgp)+2).alpha + (2.((p-1)/p) + 2).n.beta +
   n.(1+(p-1)/p).gamma


   For short messages, for user-defined ops, and for count < pof2
   we use a recursive doubling algorithm (similar to the one in
   MPI_Allgather). We use this algorithm in the case of user-defined ops
   because in this case derived datatypes are allowed, and the user
   could pass basic datatypes on one process and derived on another as
   long as the type maps are the same. Breaking up derived datatypes
   to do the reduce-scatter is tricky.

   Cost = lgp.alpha + n.lgp.beta + n.lgp.gamma

   Possible improvements:

   End Algorithm: MPI_Allreduce
*/

int MPIR_Allreduce_mcst_reduce_two_level_helper_MVP(
    const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
    MPI_Op op, MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    return 0;
}

int MPIR_Allreduce_mcst_reduce_redscat_gather_MVP(
    const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
    MPI_Op op, MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    return 0;
}

/* We can remove this old one
 * It does a reduce then bcast */
/* not declared static because a machine-specific function may call this one
   in some cases */
int MPIR_Allreduce_pt2pt_old_MVP(const void *sendbuf, void *recvbuf, int count,
                                 MPI_Datatype datatype, MPI_Op op,
                                 MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
#ifdef MPID_HAS_HETERO
    int rc;
    int is_homogeneous = 1;
#endif
    int comm_size, rank;
    MPI_Aint type_size;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    int mask, dst, is_commutative, pof2, newrank = 0, rem, newdst, i, send_idx,
                                         recv_idx, last_idx, send_cnt, recv_cnt,
                                         *cnts, *disps;
    MPI_Aint true_lb, true_extent, extent;
    void *tmp_buf;
    MPI_User_function *uop;
#ifdef HAVE_CXX_BINDING
    int is_cxx_uop = 0;
#endif
    MPIR_CHKLMEM_DECL(3);

    if (count == 0) {
        return MPI_SUCCESS;
    }

#ifdef MPID_HAS_HETERO
    if (comm_ptr->is_hetero) {
        is_homogeneous = 0;
    }

    if (!is_homogeneous) {
        /* heterogeneous. To get the same result on all processes, we
           do a reduce to 0 and then broadcast. */
        mpi_errno = MPIR_Reduce_MVP(sendbuf, recvbuf, count, datatype, op, 0,
                                    comm_ptr, errflag);
        /*
           FIXME: mpi_errno is error CODE, not necessarily the error
           class MPI_ERR_OP.  In MPICH2, we can get the error class
           with errorclass = mpi_errno & ERROR_CLASS_MASK;
         */
        if (mpi_errno == MPI_ERR_OP || mpi_errno == MPI_SUCCESS) {
            /* Allow MPI_ERR_OP since we can continue from this error */
            rc =
                MPIR_Bcast_impl(recvbuf, count, datatype, 0, comm_ptr, errflag);
            if (rc)
                mpi_errno = rc;
        }
    } else
#endif /* MPID_HAS_HETERO */
    {
        /* homogeneous */

        comm_size = comm_ptr->local_size;
        rank = comm_ptr->rank;

        /* Get func ptr for the reduction op
         * and initialize is_commutative and is_cxx_uop */
        MPIR_get_op_ptr(&uop, &is_commutative,
#ifdef HAVE_CXX_BINDING
                        &is_cxx_uop,
#endif
                        op);

        /* need to allocate temporary buffer to store incoming data */
        MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);
        MPIR_Datatype_get_extent_macro(datatype, extent);

        MPIR_CHKLMEM_MALLOC(tmp_buf, void *,
                            count *(MPL_MAX(extent, true_extent)), mpi_errno,
                            "temporary buffer", MPL_MEM_COLL);

        /* adjust for potential negative lower bound in datatype */
        tmp_buf = (void *)((char *)tmp_buf - true_lb);

        /* copy local data into recvbuf */
        if (sendbuf != MPI_IN_PLACE) {
            mpi_errno = MPIR_Localcopy(sendbuf, count, datatype, recvbuf, count,
                                       datatype);
            MPIR_ERR_CHKANDJUMP((mpi_errno), mpi_errno, MPI_ERR_OTHER,
                                "**fail");
        }

        MPIR_Datatype_get_size_macro(datatype, type_size);

        /* find nearest power-of-two less than or equal to comm_size */
        pof2 = comm_ptr->dev.ch.gpof2;

        rem = comm_size - pof2;

        /* In the non-power-of-two case, all even-numbered
           processes of rank < 2*rem send their data to
           (rank+1). These even-numbered processes no longer
           participate in the algorithm until the very end. The
           remaining processes form a nice power-of-two. */

        if (rank < 2 * rem) {
            if (rank % 2 == 0) {
                /* even */
                mpi_errno = MPIC_Send(recvbuf, count, datatype, rank + 1,
                                      MPIR_ALLREDUCE_TAG, comm_ptr, errflag);
                if (mpi_errno) {
                    /* for communication errors, just record the error but
                     * continue */
                    *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                    MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                }

                /* temporarily set the rank to -1 so that this
                   process does not pariticipate in recursive
                   doubling */
                newrank = -1;
            } else {
                /* odd */
                mpi_errno = MPIC_Recv(tmp_buf, count, datatype, rank - 1,
                                      MPIR_ALLREDUCE_TAG, comm_ptr,
                                      MPI_STATUS_IGNORE, errflag);
                if (mpi_errno) {
                    /* for communication errors, just record the error but
                     * continue */
                    *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                    MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                }

                /* do the reduction on received data. since the
                   ordering is right, it doesn't matter whether
                   the operation is commutative or not. */
                MPIR_MVP_Reduce_local(tmp_buf, recvbuf, (MPI_Aint)count,
                                      datatype, uop
#ifdef HAVE_CXX_BINDING
                                      ,
                                      is_cxx_uop
#endif
                );

                /* change the rank */
                newrank = rank / 2;
            }
        } else { /* rank >= 2*rem */
            newrank = rank - rem;
        }

        /* If op is user-defined or count is less than pof2, use
           recursive doubling algorithm. Otherwise do a reduce-scatter
           followed by allgather. (If op is user-defined,
           derived datatypes are allowed and the user could pass basic
           datatypes on one process and derived on another as long as
           the type maps are the same. Breaking up derived
           datatypes to do the reduce-scatter is tricky, therefore
           using recursive doubling in that case.) */

        if (newrank != -1) {
            if ((count * type_size <= MVP_ALLREDUCE_SHORT_MSG) ||
                (HANDLE_GET_KIND(op) != HANDLE_KIND_BUILTIN) ||
                (count < pof2)) { /* use recursive doubling */
                mask = 0x1;
                while (mask < pof2) {
                    newdst = newrank ^ mask;
                    /* find real rank of dest */
                    dst = (newdst < rem) ? newdst * 2 + 1 : newdst + rem;

                    /* Send the most current data, which is in recvbuf. Recv
                       into tmp_buf */
                    mpi_errno = MPIC_Sendrecv(
                        recvbuf, count, datatype, dst, MPIR_ALLREDUCE_TAG,
                        tmp_buf, count, datatype, dst, MPIR_ALLREDUCE_TAG,
                        comm_ptr, MPI_STATUS_IGNORE, errflag);

                    if (mpi_errno) {
                        /* for communication errors, just record the error but
                         * continue */
                        *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                        MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                    }

                    /* tmp_buf contains data received in this step.
                       recvbuf contains data accumulated so far */

                    if (is_commutative || (dst < rank)) {
                        /* op is commutative OR the order is already right */
                        MPIR_MVP_Reduce_local(tmp_buf, recvbuf, (MPI_Aint)count,
                                              datatype, uop
#ifdef HAVE_CXX_BINDING
                                              ,
                                              is_cxx_uop
#endif
                        );
                    } else {
                        /* op is noncommutative and the order is not right */
                        MPIR_MVP_Reduce_local(recvbuf, tmp_buf, (MPI_Aint)count,
                                              datatype, uop
#ifdef HAVE_CXX_BINDING
                                              ,
                                              is_cxx_uop
#endif
                        );

                        /* copy result back into recvbuf */
                        mpi_errno = MPIR_Localcopy(tmp_buf, count, datatype,
                                                   recvbuf, count, datatype);
                        MPIR_ERR_CHKANDJUMP((mpi_errno), mpi_errno,
                                            MPI_ERR_OTHER, "**fail");
                    }
                    mask <<= 1;
                }
            } else {
                /* do a reduce-scatter followed by allgather */

                /* for the reduce-scatter, calculate the count that
                   each process receives and the displacement within
                   the buffer */

                MPIR_CHKLMEM_MALLOC(cnts, int *, pof2 * sizeof(int), mpi_errno,
                                    "counts", MPL_MEM_COLL);
                MPIR_CHKLMEM_MALLOC(disps, int *, pof2 * sizeof(int), mpi_errno,
                                    "displacements", MPL_MEM_COLL);

                for (i = 0; i < (pof2 - 1); i++) {
                    cnts[i] = count / pof2;
                }
                cnts[pof2 - 1] = count - (count / pof2) * (pof2 - 1);

                disps[0] = 0;
                for (i = 1; i < pof2; i++) {
                    disps[i] = disps[i - 1] + cnts[i - 1];
                }

                mask = 0x1;
                send_idx = recv_idx = 0;
                last_idx = pof2;
                while (mask < pof2) {
                    newdst = newrank ^ mask;
                    /* find real rank of dest */
                    dst = (newdst < rem) ? newdst * 2 + 1 : newdst + rem;

                    send_cnt = recv_cnt = 0;
                    if (newrank < newdst) {
                        send_idx = recv_idx + pof2 / (mask * 2);
                        for (i = send_idx; i < last_idx; i++)
                            send_cnt += cnts[i];
                        for (i = recv_idx; i < send_idx; i++)
                            recv_cnt += cnts[i];
                    } else {
                        recv_idx = send_idx + pof2 / (mask * 2);
                        for (i = send_idx; i < recv_idx; i++)
                            send_cnt += cnts[i];
                        for (i = recv_idx; i < last_idx; i++)
                            recv_cnt += cnts[i];
                    }

                    /* Send data from recvbuf. Recv into tmp_buf */
                    mpi_errno = MPIC_Sendrecv(
                        (char *)recvbuf + disps[send_idx] * extent, send_cnt,
                        datatype, dst, MPIR_ALLREDUCE_TAG,
                        (char *)tmp_buf + disps[recv_idx] * extent, recv_cnt,
                        datatype, dst, MPIR_ALLREDUCE_TAG, comm_ptr,
                        MPI_STATUS_IGNORE, errflag);
                    if (mpi_errno) {
                        /* for communication errors, just record the error but
                         * continue */
                        *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                        MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                    }

                    /* tmp_buf contains data received in this step.
                       recvbuf contains data accumulated so far */

                    /* This algorithm is used only for predefined ops
                       and predefined ops are always commutative. */

                    (*uop)((char *)tmp_buf + disps[recv_idx] * extent,
                           (char *)recvbuf + disps[recv_idx] * extent,
                           &recv_cnt, &datatype);

                    /* update send_idx for next iteration */
                    send_idx = recv_idx;
                    mask <<= 1;

                    /* update last_idx, but not in last iteration
                       because the value is needed in the allgather
                       step below. */
                    if (mask < pof2)
                        last_idx = recv_idx + pof2 / mask;
                }

                /* now do the allgather */

                mask >>= 1;
                while (mask > 0) {
                    newdst = newrank ^ mask;
                    /* find real rank of dest */
                    dst = (newdst < rem) ? newdst * 2 + 1 : newdst + rem;

                    send_cnt = recv_cnt = 0;
                    if (newrank < newdst) {
                        /* update last_idx except on first iteration */
                        if (mask != pof2 / 2) {
                            last_idx = last_idx + pof2 / (mask * 2);
                        }

                        recv_idx = send_idx + pof2 / (mask * 2);
                        for (i = send_idx; i < recv_idx; i++) {
                            send_cnt += cnts[i];
                        }
                        for (i = recv_idx; i < last_idx; i++) {
                            recv_cnt += cnts[i];
                        }
                    } else {
                        recv_idx = send_idx - pof2 / (mask * 2);
                        for (i = send_idx; i < last_idx; i++) {
                            send_cnt += cnts[i];
                        }
                        for (i = recv_idx; i < send_idx; i++) {
                            recv_cnt += cnts[i];
                        }
                    }

                    mpi_errno = MPIC_Sendrecv(
                        (char *)recvbuf + disps[send_idx] * extent, send_cnt,
                        datatype, dst, MPIR_ALLREDUCE_TAG,
                        (char *)recvbuf + disps[recv_idx] * extent, recv_cnt,
                        datatype, dst, MPIR_ALLREDUCE_TAG, comm_ptr,
                        MPI_STATUS_IGNORE, errflag);
                    if (mpi_errno) {
                        /* for communication errors, just record the error but
                         * continue */
                        *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                        MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                    }

                    if (newrank > newdst) {
                        send_idx = recv_idx;
                    }

                    mask >>= 1;
                }
            }
        }

        /* In the non-power-of-two case, all odd-numbered
           processes of rank < 2*rem send the result to
           (rank-1), the ranks who didn't participate above. */
        if (rank < 2 * rem) {
            if (rank % 2) { /* odd */
                mpi_errno = MPIC_Send(recvbuf, count, datatype, rank - 1,
                                      MPIR_ALLREDUCE_TAG, comm_ptr, errflag);
            } else { /* even */

                mpi_errno = MPIC_Recv(recvbuf, count, datatype, rank + 1,
                                      MPIR_ALLREDUCE_TAG, comm_ptr,
                                      MPI_STATUS_IGNORE, errflag);
            }
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
    MPIR_CHKLMEM_FREEALL();
    return (mpi_errno);

fn_fail:
    goto fn_exit;
}

int MPIR_Allreduce_new_MVP(const void *sendbuf, void *recvbuf, int count,
                           MPI_Datatype datatype, MPI_Op op,
                           MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
#ifdef MPID_HAS_HETERO
    int rc;
    int is_homogeneous = 1;
#endif

    int mpi_errno = MPI_SUCCESS;
    int rank = 0, comm_size = 0;

    comm_size = MPIR_Comm_size(comm_ptr);
    rank = MPIR_Comm_rank(comm_ptr);

    if (count == 0) {
        return MPI_SUCCESS;
    }

    MPI_Aint sendtype_size = 0;
    MPI_Aint nbytes = 0;
    int range = 0, range_threshold = 0, range_threshold_intra = 0;
    int is_two_level = 0;
    int is_commutative = 0;
    MPI_Aint true_lb, true_extent;

    MPIR_Datatype_get_size_macro(datatype, sendtype_size);
    nbytes = count * sendtype_size;

    MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);
    MPIR_Op *op_ptr;

    if (HANDLE_GET_KIND(op) == HANDLE_KIND_BUILTIN) {
        is_commutative = 1;
    } else {
        MPIR_Op_get_ptr(op, op_ptr);
        if (op_ptr->kind == MPIR_OP_KIND__USER_NONCOMMUTE) {
            is_commutative = 0;
        } else {
            is_commutative = 1;
        }
    }

#ifdef MPID_HAS_HETERO
    if (comm_ptr->is_hetero) {
        is_homogeneous = 0;
    }

    if (!is_homogeneous) {
        /* heterogeneous. To get the same result on all processes, we
           do a reduce to 0 and then broadcast. */
        mpi_errno = MPIR_Reduce_MVP(sendbuf, recvbuf, count, datatype, op, 0,
                                    comm_ptr, errflag);
        /*
           FIXME: mpi_errno is error CODE, not necessarily the error
           class MPI_ERR_OP.  In MPICH2, we can get the error class
           with errorclass = mpi_errno & ERROR_CLASS_MASK;
         */
        if (mpi_errno == MPI_ERR_OP || mpi_errno == MPI_SUCCESS) {
            /* Allow MPI_ERR_OP since we can continue from this error */
            rc =
                MPIR_Bcast_impl(recvbuf, count, datatype, 0, comm_ptr, errflag);
            if (rc)
                mpi_errno = rc;
        }
    } else
#endif /* MPID_HAS_HETERO */
    {
        /* Search for the corresponding system size inside the tuning table */
        while ((range < (mvp_size_allreduce_tuning_table - 1)) &&
               (comm_size > mvp_allreduce_thresholds_table[range].numproc)) {
            range++;
        }
        /* Search for corresponding inter-leader function */
        /* skip mcast pointers if mcast is not available */
        if (mvp_allreduce_thresholds_table[range].mcast_enabled != 1) {
            while ((range_threshold <
                    (mvp_allreduce_thresholds_table[range].size_inter_table -
                     1)) &&
                   ((mvp_allreduce_thresholds_table[range]
                         .inter_leader[range_threshold]
                         .allreduce_fn ==
                     &MPIR_Allreduce_mcst_reduce_redscat_gather_MVP) ||
                    (mvp_allreduce_thresholds_table[range]
                         .inter_leader[range_threshold]
                         .allreduce_fn ==
                     &MPIR_Allreduce_mcst_reduce_two_level_helper_MVP))) {
                range_threshold++;
            }
        }
        while ((range_threshold <
                (mvp_allreduce_thresholds_table[range].size_inter_table - 1)) &&
               (nbytes > mvp_allreduce_thresholds_table[range]
                             .inter_leader[range_threshold]
                             .max) &&
               (mvp_allreduce_thresholds_table[range]
                    .inter_leader[range_threshold]
                    .max != -1)) {
            range_threshold++;
        }
        if (mvp_allreduce_thresholds_table[range]
                .is_two_level_allreduce[range_threshold] == 1) {
            is_two_level = 1;
        }
        /* Search for corresponding intra-node function */
        while ((range_threshold_intra <
                (mvp_allreduce_thresholds_table[range].size_intra_table - 1)) &&
               (nbytes > mvp_allreduce_thresholds_table[range]
                             .intra_node[range_threshold_intra]
                             .max) &&
               (mvp_allreduce_thresholds_table[range]
                    .intra_node[range_threshold_intra]
                    .max != -1)) {
            range_threshold_intra++;
        }

        MVP_Allreduce_function = mvp_allreduce_thresholds_table[range]
                                     .inter_leader[range_threshold]
                                     .allreduce_fn;

        MVP_Allreduce_intra_function = mvp_allreduce_thresholds_table[range]
                                           .intra_node[range_threshold_intra]
                                           .allreduce_fn;

        /* check if mcast is ready, otherwise replace mcast with other algorithm
         */
        if ((MVP_Allreduce_function ==
             &MPIR_Allreduce_mcst_reduce_redscat_gather_MVP) ||
            (MVP_Allreduce_function ==
             &MPIR_Allreduce_mcst_reduce_two_level_helper_MVP)) {
#if defined(_MCST_SUPPORT_)
            /* fall back to RD algorithm if:
             *  1) two level is disabled
             *  2) mcast is not ready or supported */
            if (is_two_level != 1 || !MVP_USE_MCAST_ALLREDUCE ||
                comm_ptr->dev.ch.is_mcast_ok != 1 ||
                comm_ptr->dev.ch.shmem_coll_ok != 1)
#endif /* #if defined(_MCST_SUPPORT_) */
            {
                MVP_Allreduce_function = &MPIR_Allreduce_pt2pt_rd_MVP;
            }
        }

        if (is_two_level == 1) {
#if defined(_MCST_SUPPORT_)
            if ((MVP_Allreduce_function ==
                 &MPIR_Allreduce_mcst_reduce_redscat_gather_MVP) ||
                (MVP_Allreduce_function ==
                 &MPIR_Allreduce_mcst_reduce_two_level_helper_MVP)) {
                mpi_errno = MPIR_Allreduce_mcst_MVP(
                    sendbuf, recvbuf, count, datatype, op, comm_ptr, errflag);
            } else
#endif /* #if defined(_MCST_SUPPORT_) */
            {
                /* check if shm is ready, if not use other algorithm first */
                if ((comm_ptr->dev.ch.shmem_coll_ok == 1) &&
                    (MVP_USE_SHMEM_ALLREDUCE) && (is_commutative) &&
                    (MVP_USE_SHARED_MEM)) {
                    mpi_errno = MPIR_Allreduce_two_level_MVP(
                        sendbuf, recvbuf, count, datatype, op, comm_ptr,
                        errflag);
                } else {
                    mpi_errno = MPIR_Allreduce_pt2pt_rd_MVP(sendbuf, recvbuf,
                                                            count, datatype, op,
                                                            comm_ptr, errflag);
                }
            }
        } else {
            mpi_errno = MVP_Allreduce_function(sendbuf, recvbuf, count,
                                               datatype, op, comm_ptr, errflag);
        }
    }

    comm_ptr->dev.ch.intra_node_done = 0;

    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    return (mpi_errno);

fn_fail:
    goto fn_exit;
}

int MPIR_Allreduce_index_tuned_intra_MVP(const void *sendbuf, void *recvbuf,
                                         int count, MPI_Datatype datatype,
                                         MPI_Op op, MPIR_Comm *comm_ptr,
                                         MPIR_Errflag_t *errflag)
{
#ifdef MPID_HAS_HETERO
    int rc;
    int is_homogeneous = 1;
#endif

    int mpi_errno = MPI_SUCCESS;
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
    int rank = 0, comm_size = 0;
    MPI_Aint sendtype_size = 0;
    MPI_Aint nbytes = 0;
    int is_two_level = 0;
    int is_commutative = 0;
    MPI_Aint true_lb, true_extent;

    comm_size = MPIR_Comm_size(comm_ptr);
    rank = MPIR_Comm_rank(comm_ptr);

    if (count == 0) {
        return MPI_SUCCESS;
    }

    MPIR_Datatype_get_size_macro(datatype, sendtype_size);
    nbytes = count * sendtype_size;

#if defined(_SHARP_SUPPORT_)
    int is_socket_aware = MVP_ENABLE_SOCKET_AWARE_COLLECTIVES &&
                          MVP_USE_SOCKET_AWARE_ALLREDUCE &&
                          MVP_USE_SOCKET_AWARE_SHARP_ALLREDUCE;

    // If is_socket_aware == 1 then Sharp allreduce will be called from within
    // the socket-aware allreduce function
    if (comm_ptr->dev.ch.is_sharp_ok == 1 && nbytes <= MVP_SHARP_MAX_MSG_SIZE &&
        MVP_ENABLE_SHARP == 2 && !is_socket_aware &&
        MVP_ENABLE_SHARP_ALLREDUCE) {
        /* Direct flat algorithm in which every process calls Sharp
         * MVP_ENABLE_SHARP should be set to 2 */
        mpi_errno = MPIR_Sharp_Allreduce_MVP(sendbuf, recvbuf, count, datatype,
                                             op, comm_ptr, errflag);
        if (mpi_errno == MPI_SUCCESS) {
            goto fn_exit;
        }
        /* SHArP collective is not supported, continue without using SHArP */
    }
#endif /* end of defined (_SHARP_SUPPORT_) */

    MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);
    MPIR_Op *op_ptr;

    if (HANDLE_GET_KIND(op) == HANDLE_KIND_BUILTIN) {
        is_commutative = 1;
    } else {
        MPIR_Op_get_ptr(op, op_ptr);
        if (op_ptr->kind == MPIR_OP_KIND__USER_NONCOMMUTE) {
            is_commutative = 0;
        } else {
            is_commutative = 1;
        }
    }

    if (comm_ptr->dev.ch.shmem_coll_ok == 1 && comm_ptr->dev.ch.is_uniform) {
        shmem_comm = comm_ptr->dev.ch.shmem_comm;
        MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
        local_size = shmem_commptr->local_size;
        if (mvp_allreduce_indexed_table_ppn_conf[0] == -1) {
            // Indicating user defined tuning
            conf_index = 0;
            goto conf_check_end;
        }

        if (MVP_ENABLE_ALLREDUCE_SKIP_SMALL_MESSAGE_TUNING_TABLE_SEARCH) {
            ALLREDUCE_SKIP_SMALL_MESSAGE_TUNING_TABLES
        }
        if (MVP_ENABLE_ALLREDUCE_SKIP_LARGE_MESSAGE_TUNING_TABLE_SEARCH) {
            ALLREDUCE_SKIP_LARGE_MESSAGE_TUNING_TABLES
        }

    use_tables:
        FIND_PPN_INDEX(allreduce, local_size, conf_index, partial_sub_ok)
    }

    if (partial_sub_ok != 1) {
        conf_index = mvp_allreduce_indexed_num_ppn_conf / 2;
    }

conf_check_end:

#ifdef MPID_HAS_HETERO
    if (comm_ptr->is_hetero) {
        is_homogeneous = 0;
    }

    if (!is_homogeneous) {
        /* heterogeneous. To get the same result on all processes, we
           do a reduce to 0 and then broadcast. */
        mpi_errno = MPIR_Reduce_MVP(sendbuf, recvbuf, count, datatype, op, 0,
                                    comm_ptr, errflag);
        /*
           FIXME: mpi_errno is error CODE, not necessarily the error
           class MPI_ERR_OP.  In MPICH2, we can get the error class
           with errorclass = mpi_errno & ERROR_CLASS_MASK;
    */
        if (mpi_errno == MPI_ERR_OP || mpi_errno == MPI_SUCCESS) {
            /* Allow MPI_ERR_OP since we can continue from this error */
            rc =
                MPIR_Bcast_impl(recvbuf, count, datatype, 0, comm_ptr, errflag);
            if (rc)
                mpi_errno = rc;
        }
    } else
#endif /* MPID_HAS_HETERO */
    {

        /* Search for the corresponding system size inside the tuning table */
        /*
         * Comm sizes progress in powers of 2. Therefore comm_size can just be
         * indexed instead
         */
        table_min_comm_size =
            mvp_allreduce_indexed_thresholds_table[conf_index][0].numproc;
        table_max_comm_size =
            mvp_allreduce_indexed_thresholds_table
                [conf_index]
                [mvp_size_allreduce_indexed_tuning_table[conf_index] - 1]
                    .numproc;

        if (comm_size < table_min_comm_size) {
            /* Comm size smaller than smallest configuration in table: use
             * smallest available */
            comm_size_index = 0;
        } else if (comm_size > table_max_comm_size) {
            /* Comm size larger than largest configuration in table: use largest
             * available */
            comm_size_index =
                mvp_size_allreduce_indexed_tuning_table[conf_index] - 1;
        } else {
            /* Comm size in between smallest and largest configuration: find
             * closest match */
            lp2ltn_min = pow(2, (int)log2(table_min_comm_size));
            if (comm_ptr->dev.ch.is_pof2) {
                comm_size_index = log2(comm_size / lp2ltn_min);
            } else {
                lp2ltn = pow(2, (int)log2(comm_size));
                comm_size_index =
                    (lp2ltn < lp2ltn_min) ? 0 : log2(lp2ltn / lp2ltn_min);
            }
        }
        /* Search for corresponding inter-leader function */
        /* skip mcast pointers if mcast is not available */

        last_inter =
            mvp_allreduce_indexed_thresholds_table[conf_index][comm_size_index]
                .size_inter_table -
            1;
        table_min_inter_size =
            mvp_allreduce_indexed_thresholds_table[conf_index][comm_size_index]
                .inter_leader[0]
                .msg_sz;
        table_max_inter_size =
            mvp_allreduce_indexed_thresholds_table[conf_index][comm_size_index]
                .inter_leader[last_inter]
                .msg_sz;
        last_intra =
            mvp_allreduce_indexed_thresholds_table[conf_index][comm_size_index]
                .size_intra_table -
            1;
        table_min_intra_size =
            mvp_allreduce_indexed_thresholds_table[conf_index][comm_size_index]
                .intra_node[0]
                .msg_sz;
        table_max_intra_size =
            mvp_allreduce_indexed_thresholds_table[conf_index][comm_size_index]
                .intra_node[last_intra]
                .msg_sz;

        if (nbytes < table_min_inter_size) {
            /* Msg size smaller than smallest configuration in table: use
             * smallest available */
            inter_node_algo_index = 0;
        } else if (nbytes > table_max_inter_size) {
            /* Msg size larger than largest configuration in table: use largest
             * available */
            inter_node_algo_index = last_inter;
        } else {
            /* Msg size in between smallest and largest configuration: find
             * closest match */
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
            /* Msg size smaller than smallest configuration in table: use
             * smallest available */
            intra_node_algo_index = 0;
        } else if (nbytes > table_max_intra_size) {
            /* Msg size larger than largest configuration in table: use largest
             * available */
            intra_node_algo_index = last_intra;
        } else {
            /* Msg size in between smallest and largest configuration: find
             * closest match */
            if (pow(2, (int)log2(nbytes)) == nbytes) {
                intra_node_algo_index = log2(nbytes / table_min_intra_size);
            } else {
                lp2ltn = pow(2, (int)log2(nbytes));
                intra_node_algo_index = (lp2ltn < table_min_intra_size) ?
                                            0 :
                                            log2(lp2ltn / table_min_intra_size);
            }
        }

        if (mvp_allreduce_indexed_thresholds_table[conf_index][comm_size_index]
                .is_two_level_allreduce[inter_node_algo_index] == 1) {
            is_two_level = 1;
        }

        MVP_Allreduce_function =
            mvp_allreduce_indexed_thresholds_table[conf_index][comm_size_index]
                .inter_leader[inter_node_algo_index]
                .allreduce_fn;

        MVP_Allreduce_intra_function =
            mvp_allreduce_indexed_thresholds_table[conf_index][comm_size_index]
                .intra_node[intra_node_algo_index]
                .allreduce_fn;

    skip_tuning_tables:
        if (MVP_Allreduce_function ==
            &MPIR_Allreduce_topo_aware_hierarchical_MVP) {
            if (nbytes <= MVP_TOPO_AWARE_ALLREDUCE_MAX_MSG &&
                nbytes >= MVP_TOPO_AWARE_ALLREDUCE_MIN_MSG &&
                MVP_ENABLE_TOPO_AWARE_COLLECTIVES &&
                MVP_USE_TOPO_AWARE_ALLREDUCE &&
                comm_ptr->dev.ch.topo_coll_ok == 1 && is_commutative &&
                local_size >= mvp_topo_aware_allreduce_ppn_threshold) {
                return MPIR_Allreduce_topo_aware_hierarchical_MVP(
                    sendbuf, recvbuf, count, datatype, op, comm_ptr, errflag);
            } else {
                // Fall back to recursive doubling in case the above isn't
                // satisfied.
                return MPIR_Allreduce_pt2pt_rd_MVP(
                    sendbuf, recvbuf, count, datatype, op, comm_ptr, errflag);
            }
        }
        if (MVP_Allreduce_function ==
            &MPIR_Allreduce_socket_aware_two_level_MVP) {
            if (nbytes <= MVP_SOCKET_AWARE_ALLREDUCE_MAX_MSG &&
                nbytes >= MVP_SOCKET_AWARE_ALLREDUCE_MIN_MSG &&
                MVP_ENABLE_SOCKET_AWARE_COLLECTIVES &&
                MVP_USE_SOCKET_AWARE_ALLREDUCE &&
                comm_ptr->dev.ch.shmem_coll_ok == 1 &&
                comm_ptr->dev.ch.use_intra_sock_comm == 1 && is_commutative &&
                local_size >= MVP_SOCKET_AWARE_ALLREDUCE_PPN_THRESHOLD) {
                return MPIR_Allreduce_socket_aware_two_level_MVP(
                    sendbuf, recvbuf, count, datatype, op, comm_ptr, errflag);
            } else {
                return MPIR_Allreduce_pt2pt_rd_MVP(
                    sendbuf, recvbuf, count, datatype, op, comm_ptr, errflag);
            }
        }

        /* check if mcast is ready, otherwise replace mcast with other algorithm
         */
        if ((MVP_Allreduce_function ==
             &MPIR_Allreduce_mcst_reduce_redscat_gather_MVP) ||
            (MVP_Allreduce_function ==
             &MPIR_Allreduce_mcst_reduce_two_level_helper_MVP)) {
#if defined(_MCST_SUPPORT_)
            /* fall back to RD algorithm if:
             *  1) two level is disabled
             *  2) mcast is not ready or supported */
            if (is_two_level != 1 || MVP_USE_MCAST_ALLREDUCE ||
                comm_ptr->dev.ch.is_mcast_ok != 1 ||
                comm_ptr->dev.ch.shmem_coll_ok != 1)
#endif /* #if defined(_MCST_SUPPORT_) */
            {
                MVP_Allreduce_function = &MPIR_Allreduce_pt2pt_rd_MVP;
            }
        }
#if defined(_SHARP_SUPPORT_)
        if (comm_ptr->dev.ch.is_sharp_ok == 1 &&
            nbytes <= MVP_SHARP_MAX_MSG_SIZE && MVP_ENABLE_SHARP == 1 &&
            MVP_ENABLE_SHARP_ALLREDUCE) {
            is_two_level = 1;
            MVP_Allreduce_function = &MPIR_Sharp_Allreduce_MVP;
        }
#endif

        if (is_two_level == 1) {
#if defined(_MCST_SUPPORT_)
            if ((MVP_Allreduce_function ==
                 &MPIR_Allreduce_mcst_reduce_redscat_gather_MVP) ||
                (MVP_Allreduce_function ==
                 &MPIR_Allreduce_mcst_reduce_two_level_helper_MVP)) {
                mpi_errno = MPIR_Allreduce_mcst_MVP(
                    sendbuf, recvbuf, count, datatype, op, comm_ptr, errflag);
            } else
#endif /* #if defined(_MCST_SUPPORT_) */
            {
                /* check if shm is ready, if not use other algorithm first */
                if ((comm_ptr->dev.ch.shmem_coll_ok == 1) &&
                    (MVP_USE_SHMEM_ALLREDUCE) && (is_commutative) &&
                    (MVP_USE_SHARED_MEM)) {
                    mpi_errno = MPIR_Allreduce_two_level_MVP(
                        sendbuf, recvbuf, count, datatype, op, comm_ptr,
                        errflag);
                } else {
                    mpi_errno = MPIR_Allreduce_pt2pt_rd_MVP(sendbuf, recvbuf,
                                                            count, datatype, op,
                                                            comm_ptr, errflag);
                }
            }
        } else {
            mpi_errno = MVP_Allreduce_function(sendbuf, recvbuf, count,
                                               datatype, op, comm_ptr, errflag);
        }
    }

    comm_ptr->dev.ch.intra_node_done = 0;

    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    return (mpi_errno);

fn_fail:
    goto fn_exit;
}

/* We can consider removing this old one */
int MPIR_Allreduce_old_MVP(const void *sendbuf, void *recvbuf, int count,
                           MPI_Datatype datatype, MPI_Op op,
                           MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;

    if (count == 0) {
        return MPI_SUCCESS;
    }

    int stride = 0, is_commutative = 0;
    MPI_Aint true_lb, true_extent, extent;
    MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);
    MPIR_Datatype_get_extent_macro(datatype, extent);
    stride = count * MPL_MAX(extent, true_extent);
    MPIR_Op *op_ptr;

    if (HANDLE_GET_KIND(op) == HANDLE_KIND_BUILTIN) {
        is_commutative = 1;
    } else {
        MPIR_Op_get_ptr(op, op_ptr);
        if (op_ptr->kind == MPIR_OP_KIND__USER_NONCOMMUTE) {
            is_commutative = 0;
        } else {
            is_commutative = 1;
        }
    }

#if defined(_MCST_SUPPORT_)
    if (comm_ptr->dev.ch.is_mcast_ok == 1 &&
        comm_ptr->dev.ch.shmem_coll_ok == 1 && MVP_USE_MCAST_ALLREDUCE &&
        stride >= MVP_MCAST_ALLREDUCE_SMALL_MSG_SIZE &&
        stride <= MVP_MCAST_ALLREDUCE_LARGE_MSG_SIZE) {
        mpi_errno = MPIR_Allreduce_mcst_MVP(sendbuf, recvbuf, count, datatype,
                                            op, comm_ptr, errflag);
    } else
#endif /* #if defined(_MCST_SUPPORT_) */
    {
        if ((comm_ptr->dev.ch.shmem_coll_ok == 1) &&
            (stride < MVP_ALLREDUCE_2LEVEL_MSG) && (MVP_USE_SHMEM_ALLREDUCE) &&
            (is_commutative) && (MVP_USE_SHARED_MEM)) {
            mpi_errno = MPIR_Allreduce_shmem_MVP(
                sendbuf, recvbuf, count, datatype, op, comm_ptr, errflag);

        } else {
            mpi_errno = MPIR_Allreduce_pt2pt_old_MVP(
                sendbuf, recvbuf, count, datatype, op, comm_ptr, errflag);
        }
    }

    comm_ptr->dev.ch.intra_node_done = 0;

    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    return (mpi_errno);

fn_fail:
    goto fn_exit;
}

int MPIR_Allreduce_MVP(const void *sendbuf, void *recvbuf, int count,
                       MPI_Datatype datatype, MPI_Op op, MPIR_Comm *comm_ptr,
                       MPIR_Errflag_t *errflag)
{
    
    MPIR_TIMER_START(coll, allreduce, subcomm);
    MPIR_T_PVAR_COMM_COUNTER_INC(MVP, mvp_coll_allreduce_subcomm, 1, comm_ptr);
    // MPIR_T_PVAR_COMM_TIMER_START(MVP, mvp_coll_timer_allreduce_subcomm,
    //                              comm_ptr);
    
    int mpi_errno = MPI_SUCCESS;
    if (count == 0) {
        // MPIR_T_PVAR_COMM_TIMER_END(MVP, mvp_coll_timer_allreduce_subcomm,
        //                            comm_ptr);
        return MPI_SUCCESS;
    }

    if (MVP_REPORT_LOAD_IMBALANCE > 0) {
        MPII_MVP_LOAD_IMBALANCE_CHECK(mvp_allreduce_load_imbalance, mpi_errno,
                                      comm_ptr, errflag);
    }
    if (MVP_USE_OLD_ALLREDUCE) {
        mpi_errno = MPIR_Allreduce_old_MVP(sendbuf, recvbuf, count, datatype,
                                           op, comm_ptr, errflag);
    } else {
            mpi_errno = MPIR_Allreduce_index_tuned_intra_MVP(
                sendbuf, recvbuf, count, datatype, op, comm_ptr, errflag);
    }

    // MPIR_T_PVAR_COMM_TIMER_END(MVP, mvp_coll_timer_allreduce_subcomm, comm_ptr);
    MPIR_TIMER_END(coll, allreduce, subcomm);

fn_exit:
    return (mpi_errno);

fn_fail:
    goto fn_exit;
}
