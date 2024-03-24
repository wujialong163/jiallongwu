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
#include <unistd.h>
#include "mvp_coll_shmem.h"
#include "reduce_scatter_tuning.h"
#include <unistd.h>

int (*MVP_Red_scat_function)(const void *sendbuf, void *recvbuf,
                             const int *recvcnts, MPI_Datatype datatype,
                             MPI_Op op, MPIR_Comm *comm_ptr,
                             MPIR_Errflag_t *errflag);

static int MPIR_Reduce_scatter_non_comm_MVP(const void *sendbuf, void *recvbuf,
                                            const int *recvcnts,
                                            MPI_Datatype datatype, MPI_Op op,
                                            MPIR_Comm *comm_ptr,
                                            MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, reduce_scatter, non_comm);
    int rank, comm_size, i;
    MPI_Aint extent, true_extent, true_lb;
    int *disps;
    void *tmp_recvbuf, *tmp_results;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    int dis[2], blklens[2], total_count, dst;
    int mask, dst_tree_root, my_tree_root, j, k;
    int pof2, received;
    MPI_Datatype sendtype, recvtype;
    int nprocs_completed, tmp_mask, tree_root, is_commutative;
    MPI_User_function *uop;
#ifdef HAVE_CXX_BINDING
    int is_cxx_uop = 0;
#endif
    MPIR_CHKLMEM_DECL(5);

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_reduce_scatter_non_comm, 1);

    comm_size = comm_ptr->local_size;
    rank = comm_ptr->rank;

    MPIR_Datatype_get_extent_macro(datatype, extent);
    MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);

    /* Get func ptr for the reduction op
     * and initialize is_commutative and is_cxx_uop */
    MPIR_get_op_ptr(&uop, &is_commutative,
#ifdef HAVE_CXX_BINDING
                    &is_cxx_uop,
#endif
                    op);

    MPIR_CHKLMEM_MALLOC(disps, int *, comm_size * sizeof(int), mpi_errno,
                        "disps", MPL_MEM_COLL);

    total_count = 0;
    for (i = 0; i < comm_size; i++) {
        disps[i] = total_count;
        total_count += recvcnts[i];
    }

    if (total_count == 0) {
        goto fn_exit;
    }

    if (!is_commutative) {
        int is_block_regular = 1;
        for (i = 0; i < (comm_size - 1); ++i) {
            if (recvcnts[i] != recvcnts[i + 1]) {
                is_block_regular = 0;
                break;
            }
        }

        /* slightly retask pof2 to mean pof2 equal or greater,
         * not always greater as it is above */
        pof2 = 1;
        while (pof2 < comm_size)
            pof2 <<= 1;

        if (pof2 == comm_size && is_block_regular) {
            /* noncommutative, pof2 size, and block regular */
            mpi_errno = MPIR_Reduce_scatter_noncomm_MVP(
                sendbuf, recvbuf, recvcnts, datatype, op, comm_ptr, errflag);
            if (mpi_errno) {
                /* for communication errors, just record the error but continue
                 */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
        } else {
            /* noncommutative and (non-pof2 or block irregular), use recursive
             * doubling. */

            /* need to allocate temporary buffer to receive incoming data */
            MPIR_CHKLMEM_MALLOC(tmp_recvbuf, void *,
                                total_count *(MPL_MAX(true_extent, extent)),
                                mpi_errno, "tmp_recvbuf", MPL_MEM_COLL);
            /* adjust for potential negative lower bound in datatype */
            tmp_recvbuf = (void *)((char *)tmp_recvbuf - true_lb);

            /* need to allocate another temporary buffer to accumulate
               results */
            MPIR_CHKLMEM_MALLOC(tmp_results, void *,
                                total_count *(MPL_MAX(true_extent, extent)),
                                mpi_errno, "tmp_results", MPL_MEM_COLL);
            /* adjust for potential negative lower bound in datatype */
            tmp_results = (void *)((char *)tmp_results - true_lb);

            /* copy sendbuf into tmp_results */
            if (sendbuf != MPI_IN_PLACE)
                mpi_errno = MPIR_Localcopy(sendbuf, total_count, datatype,
                                           tmp_results, total_count, datatype);
            else
                mpi_errno = MPIR_Localcopy(recvbuf, total_count, datatype,
                                           tmp_results, total_count, datatype);

            MPIR_ERR_CHECK(mpi_errno);

            mask = 0x1;
            i = 0;
            while (mask < comm_size) {
                dst = rank ^ mask;

                dst_tree_root = dst >> i;
                dst_tree_root <<= i;

                my_tree_root = rank >> i;
                my_tree_root <<= i;

                /* At step 1, processes exchange (n-n/p) amount of
                    adata; at step 2, (n-2n/p) amount of data; at step 3,
                   (n-4n/p) amount of data, and so forth. We use derived
                   datatypes for this.

                    At each step, a process does not need to send data
                    indexed from my_tree_root to
                    my_tree_root+mask-1. Similarly, a process won't receive
                    data indexed from dst_tree_root to dst_tree_root+mask-1. */

                /* calculate sendtype */
                blklens[0] = blklens[1] = 0;
                for (j = 0; j < my_tree_root; j++)
                    blklens[0] += recvcnts[j];
                for (j = my_tree_root + mask; j < comm_size; j++)
                    blklens[1] += recvcnts[j];

                dis[0] = 0;
                dis[1] = blklens[0];
                for (j = my_tree_root;
                     (j < my_tree_root + mask) && (j < comm_size); j++)
                    dis[1] += recvcnts[j];

                mpi_errno = MPIR_Type_indexed_impl(2, blklens, dis, datatype,
                                                   &sendtype);
                MPIR_ERR_CHECK(mpi_errno);

                mpi_errno = MPIR_Type_commit_impl(&sendtype);
                MPIR_ERR_CHECK(mpi_errno);

                /* calculate recvtype */
                blklens[0] = blklens[1] = 0;
                for (j = 0; j < dst_tree_root && j < comm_size; j++)
                    blklens[0] += recvcnts[j];
                for (j = dst_tree_root + mask; j < comm_size; j++)
                    blklens[1] += recvcnts[j];

                dis[0] = 0;
                dis[1] = blklens[0];
                for (j = dst_tree_root;
                     (j < dst_tree_root + mask) && (j < comm_size); j++)
                    dis[1] += recvcnts[j];

                mpi_errno = MPIR_Type_indexed_impl(2, blklens, dis, datatype,
                                                   &recvtype);
                MPIR_ERR_CHECK(mpi_errno);

                mpi_errno = MPIR_Type_commit_impl(&recvtype);
                MPIR_ERR_CHECK(mpi_errno);

                received = 0;
                if (dst < comm_size) {
                    /* tmp_results contains data to be sent in each step. Data
                       is received in tmp_recvbuf and then accumulated into
                       tmp_results. accumulation is done later below.   */

                    MPIR_PVAR_INC(reduce_scatter, non_comm, send, 1, sendtype);
                    MPIR_PVAR_INC(reduce_scatter, non_comm, recv, 1, recvtype);
                    mpi_errno = MPIC_Sendrecv(
                        tmp_results, 1, sendtype, dst, MPIR_REDUCE_SCATTER_TAG,
                        tmp_recvbuf, 1, recvtype, dst, MPIR_REDUCE_SCATTER_TAG,
                        comm_ptr, MPI_STATUS_IGNORE, errflag);
                    received = 1;
                    if (mpi_errno) {
                        /* for communication errors, just record the error but
                         * continue */
                        *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                        MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                    }
                }

                /* if some processes in this process's subtree in this step
                   did not have any destination process to communicate with
                   because of non-power-of-two, we need to send them the
                   result. We use a logarithmic recursive-halfing algorithm
                   for this. */

                if (dst_tree_root + mask > comm_size) {
                    nprocs_completed = comm_size - my_tree_root - mask;
                    /* nprocs_completed is the number of processes in this
                subtree that have all the data. Send data to others
                in a tree fashion. First find root of current tree
                that is being divided into two. k is the number of
                least-significant bits in this process's rank that
                must be zeroed out to find the rank of the root */
                    j = mask;
                    k = 0;
                    while (j) {
                        j >>= 1;
                        k++;
                    }
                    k--;

                    tmp_mask = mask >> 1;
                    while (tmp_mask) {
                        dst = rank ^ tmp_mask;

                        tree_root = rank >> k;
                        tree_root <<= k;

                        /* send only if this proc has data and destination
                            doesn't have data. at any step, multiple processes
                            can send if they have the data */
                        if ((dst > rank) &&
                            (rank < tree_root + nprocs_completed) &&
                            (dst >= tree_root + nprocs_completed)) {
                            /* send the current result */
                            MPIR_PVAR_INC(reduce_scatter, non_comm, send, 1,
                                          recvtype);
                            mpi_errno = MPIC_Send(tmp_recvbuf, 1, recvtype, dst,
                                                  MPIR_REDUCE_SCATTER_TAG,
                                                  comm_ptr, errflag);
                            if (mpi_errno) {
                                /* for communication errors,
                                 * just record the error but continue */
                                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER,
                                             "**fail");
                                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                            }
                        }
                        /* recv only if this proc. doesn't have data and sender
                            has data */
                        else if ((dst < rank) &&
                                 (dst < tree_root + nprocs_completed) &&
                                 (rank >= tree_root + nprocs_completed)) {
                            MPIR_PVAR_INC(reduce_scatter, non_comm, recv, 1,
                                          recvtype);
                            mpi_errno =
                                MPIC_Recv(tmp_recvbuf, 1, recvtype, dst,
                                          MPIR_REDUCE_SCATTER_TAG, comm_ptr,
                                          MPI_STATUS_IGNORE, errflag);
                            received = 1;
                            if (mpi_errno) {
                                /* for communication errors,
                                 * just record the error but continue */
                                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER,
                                             "**fail");
                                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                            }
                        }
                        tmp_mask >>= 1;
                        k--;
                    }
                }

                /* The following reduction is done here instead of after
                the MPIC_Sendrecv or MPIC_Recv above. This is
                because to do it above, in the noncommutative
                case, we would need an extra temp buffer so as not to
                overwrite temp_recvbuf, because temp_recvbuf may have
                to be communicated to other processes in the
                non-power-of-two case. To avoid that extra allocation,
                we do the reduce here. */
                if (received) {
                    if (is_commutative || (dst_tree_root < my_tree_root)) {
                        MPIR_MVP_Reduce_local(tmp_recvbuf, tmp_results,
                                              (MPI_Aint)blklens[0], datatype,
                                              uop
#ifdef HAVE_CXX_BINDING
                                              ,
                                              is_cxx_uop
#endif
                        );
                        MPIR_MVP_Reduce_local(
                            ((char *)tmp_recvbuf + dis[1] * extent),
                            ((char *)tmp_results + dis[1] * extent),
                            (MPI_Aint)blklens[1], datatype, uop
#ifdef HAVE_CXX_BINDING
                            ,
                            is_cxx_uop
#endif
                        );
                    } else {
                        MPIR_MVP_Reduce_local(tmp_results, tmp_recvbuf,
                                              (MPI_Aint)blklens[0], datatype,
                                              uop
#ifdef HAVE_CXX_BINDING
                                              ,
                                              is_cxx_uop
#endif
                        );
                        MPIR_MVP_Reduce_local(
                            ((char *)tmp_results + dis[1] * extent),
                            ((char *)tmp_recvbuf + dis[1] * extent),
                            (MPI_Aint)blklens[1], datatype, uop
#ifdef HAVE_CXX_BINDING
                            ,
                            is_cxx_uop
#endif
                        );

                        /* copy result back into tmp_results */
                        mpi_errno = MPIR_Localcopy(tmp_recvbuf, 1, recvtype,
                                                   tmp_results, 1, recvtype);
                        MPIR_ERR_CHECK(mpi_errno);
                    }
                }

                MPIR_Type_free_impl(&sendtype);
                MPIR_Type_free_impl(&recvtype);

                mask <<= 1;
                i++;
            }

            /* now copy final results from tmp_results to recvbuf */
            mpi_errno = MPIR_Localcopy(
                ((char *)tmp_results + disps[rank] * extent), recvcnts[rank],
                datatype, recvbuf, recvcnts[rank], datatype);
            MPIR_ERR_CHECK(mpi_errno);
        }
    }

fn_exit:
    MPIR_CHKLMEM_FREEALL();

    if (mpi_errno_ret)
        mpi_errno = mpi_errno_ret;
    else if (*errflag)
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**coll_fail");

    MPIR_TIMER_END(coll, reduce_scatter, non_comm);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

/* MPIR_Reduce_Scatter performs an reduce_scatter using point-to-point
   messages.  This is intended to be used by device-specific
   implementations of reduce_scatter.  In all other cases
   MPIR_Reduce_Scatter_impl should be used. */

int MPIR_Reduce_scatter_MVP(const void *sendbuf, void *recvbuf,
                            const int *recvcnts, MPI_Datatype datatype,
                            MPI_Op op, MPIR_Comm *comm_ptr,
                            MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    int i = 0, comm_size = comm_ptr->local_size, total_count = 0, type_size = 0,
        nbytes = 0;
    int range = 0;
    int range_threshold = 0;
    int is_commutative = 0;
    MPIR_Op *op_ptr = NULL;
    int *disps = NULL;
    MPIR_CHKLMEM_DECL(1);

    if (HANDLE_GET_KIND(op) == HANDLE_KIND_BUILTIN) {
        is_commutative = 1;
    } else {
        MPIR_Op_get_ptr(op, op_ptr);
        if (op_ptr->kind == MPIR_OP_KIND__USER_NONCOMMUTE)
            is_commutative = 0;
        else
            is_commutative = 1;
    }

    MPIR_CHKLMEM_MALLOC(disps, int *, comm_size * sizeof(int), mpi_errno,
                        "disps", MPL_MEM_COLL);

    for (i = 0; i < comm_size; i++) {
        disps[i] = total_count;
        total_count += recvcnts[i];
    }

    if (total_count == 0) {
        goto fn_exit;
    }

    MPIR_Datatype_get_size_macro(datatype, type_size);
    nbytes = total_count * type_size;

    if (is_commutative) {
        if (mvp_red_scat_thresholds_table[0].numproc != 1 &&
            ((comm_ptr->dev.ch.allgather_comm_ok != 0 &&
              comm_ptr->dev.ch.is_blocked == 0 &&
              MVP_REDSCAT_CYCLIC_ALGO_THRESHOLD <= nbytes) ||
             MVP_RED_SCAT_RING_ALGO_THRESHOLD <= nbytes)) {
            /* make sure that user has not forced any algorithm and
             * then for large messages, use ring algorithm. Also, for cyclic
             * hostfile, use ring  */
            mpi_errno = MPIR_Reduce_scatter_ring_2lvl(
                sendbuf, recvbuf, recvcnts, datatype, op, comm_ptr, errflag);
            goto fn_exit;
        }

        /* Search for the corresponding system size inside the tuning table */
        while ((range < (mvp_size_red_scat_tuning_table - 1)) &&
               (comm_size > mvp_red_scat_thresholds_table[range].numproc)) {
            range++;
        }
        /* Search for corresponding inter-leader function */
        while ((range_threshold <
                (mvp_red_scat_thresholds_table[range].size_inter_table - 1)) &&
               (nbytes > mvp_red_scat_thresholds_table[range]
                             .inter_leader[range_threshold]
                             .max) &&
               (mvp_red_scat_thresholds_table[range]
                    .inter_leader[range_threshold]
                    .max != -1)) {
            range_threshold++;
        }

        /* Set inter-leader pt */
        MVP_Red_scat_function = mvp_red_scat_thresholds_table[range]
                                    .inter_leader[range_threshold]
                                    .MVP_pt_Red_scat_function;

        mpi_errno = MVP_Red_scat_function(sendbuf, recvbuf, recvcnts, datatype,
                                          op, comm_ptr, errflag);
    } else {
        mpi_errno = MPIR_Reduce_scatter_non_comm_MVP(
            sendbuf, recvbuf, recvcnts, datatype, op, comm_ptr, errflag);
    }
    MPIR_ERR_CHECK(mpi_errno);

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
