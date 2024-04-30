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

#include "ireduce_scatter_tuning.h"

int (*MVP_Ireduce_scatter_function)(const void *sendbuf, void *recvbuf,
                                    const int *recvcount, MPI_Datatype datatype,
                                    MPI_Op op, MPIR_Comm *comm_ptr,
                                    MPIR_Sched_t s) = NULL;

int (*MVP_Ireduce_scatter_intra_node_function)(const void *sendbuf,
                                               void *recvbuf,
                                               const int *recvcount,
                                               MPI_Datatype datatype, MPI_Op op,
                                               MPIR_Comm *comm_ptr,
                                               MPIR_Sched_t s) = NULL;
int MPIR_Ireduce_scatter_noncomm(const void *sendbuf, void *recvbuf,
                                 const int recvcounts[], MPI_Datatype datatype,
                                 MPI_Op op, MPIR_Comm *comm_ptr, MPIR_Sched_t s)
{
    int mpi_errno = MPI_SUCCESS;
    int comm_size = comm_ptr->local_size;
    int rank = comm_ptr->rank;
    int pof2;
    int log2_comm_size;
    int i, k;
    int recv_offset, send_offset;
    int block_size, total_count, size;
    MPI_Aint true_extent, true_lb;
    int buf0_was_inout;
    void *tmp_buf0;
    void *tmp_buf1;
    void *result_ptr;
    MPIR_SCHED_CHKPMEM_DECL(3);

    MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);

    pof2 = 1;
    log2_comm_size = 0;
    while (pof2 < comm_size) {
        pof2 <<= 1;
        ++log2_comm_size;
    }

    /* begin error checking */
    MPIR_Assert(pof2 == comm_size); /* FIXME this version only works for power
                                       of 2 procs */

    for (i = 0; i < (comm_size - 1); ++i) {
        MPIR_Assert(recvcounts[i] == recvcounts[i + 1]);
    }
    /* end error checking */

    /* size of a block (count of datatype per block, NOT bytes per block) */
    block_size = recvcounts[0];
    total_count = block_size * comm_size;

    MPIR_SCHED_CHKPMEM_MALLOC(tmp_buf0, void *, true_extent *total_count,
                              mpi_errno, "tmp_buf0", MPL_MEM_BUFFER);
    MPIR_SCHED_CHKPMEM_MALLOC(tmp_buf1, void *, true_extent *total_count,
                              mpi_errno, "tmp_buf1", MPL_MEM_BUFFER);
    /* adjust for potential negative lower bound in datatype */
    tmp_buf0 = (void *)((char *)tmp_buf0 - true_lb);
    tmp_buf1 = (void *)((char *)tmp_buf1 - true_lb);

    /* Copy our send data to tmp_buf0.  We do this one block at a time and
       permute the blocks as we go according to the mirror permutation. */
    for (i = 0; i < comm_size; ++i) {
        mpi_errno = MPIR_Sched_copy(
            ((char *)(sendbuf == MPI_IN_PLACE ? (const void *)recvbuf :
                                                sendbuf) +
             (i * true_extent * block_size)),
            block_size, datatype,
            ((char *)tmp_buf0 + (MPL_mirror_permutation(i, log2_comm_size) *
                                 true_extent * block_size)),
            block_size, datatype, s);
        MPIR_ERR_CHECK(mpi_errno);
        MPIR_SCHED_BARRIER(s);
    }
    buf0_was_inout = 1;

    send_offset = 0;
    recv_offset = 0;
    size = total_count;
    for (k = 0; k < log2_comm_size; ++k) {
        /* use a double-buffering scheme to avoid local copies */
        char *incoming_data = (buf0_was_inout ? tmp_buf1 : tmp_buf0);
        char *outgoing_data = (buf0_was_inout ? tmp_buf0 : tmp_buf1);
        int peer = rank ^ (0x1 << k);
        size /= 2;

        if (rank > peer) {
            /* we have the higher rank: send top half, recv bottom half */
            recv_offset += size;
        } else {
            /* we have the lower rank: recv top half, send bottom half */
            send_offset += size;
        }

        mpi_errno = MPIR_Sched_send((outgoing_data + send_offset * true_extent),
                                    size, datatype, peer, comm_ptr, s);
        MPIR_ERR_CHECK(mpi_errno);
        mpi_errno = MPIR_Sched_recv((incoming_data + recv_offset * true_extent),
                                    size, datatype, peer, comm_ptr, s);
        MPIR_ERR_CHECK(mpi_errno);
        MPIR_SCHED_BARRIER(s);

        /* always perform the reduction at recv_offset, the data at send_offset
           is now our peer's responsibility */
        if (rank > peer) {
            /* higher ranked value so need to call op(received_data, my_data) */
            mpi_errno =
                MPIR_Sched_reduce((incoming_data + recv_offset * true_extent),
                                  (outgoing_data + recv_offset * true_extent),
                                  size, datatype, op, s);
            MPIR_ERR_CHECK(mpi_errno);
        } else {
            /* lower ranked value so need to call op(my_data, received_data) */
            mpi_errno =
                MPIR_Sched_reduce((outgoing_data + recv_offset * true_extent),
                                  (incoming_data + recv_offset * true_extent),
                                  size, datatype, op, s);
            MPIR_ERR_CHECK(mpi_errno);
            buf0_was_inout = !buf0_was_inout;
        }
        MPIR_SCHED_BARRIER(s);

        /* the next round of send/recv needs to happen within the block (of size
           "size") that we just received and reduced */
        send_offset = recv_offset;
    }

    MPIR_Assert(size == recvcounts[rank]);

    /* copy the reduced data to the recvbuf */
    result_ptr = (char *)(buf0_was_inout ? tmp_buf0 : tmp_buf1) +
                 recv_offset * true_extent;
    mpi_errno =
        MPIR_Sched_copy(result_ptr, size, datatype, recvbuf, size, datatype, s);
    MPIR_ERR_CHECK(mpi_errno);
    MPIR_SCHED_CHKPMEM_COMMIT(s);
fn_exit:
    return mpi_errno;
fn_fail:
    MPIR_SCHED_CHKPMEM_REAP(s);
    goto fn_exit;
}

static int MPIR_Ireduce_scatter_tune_helper_MVP(
    const void *sendbuf, void *recvbuf, const int *recvcount,
    MPI_Datatype datatype, MPI_Op op, MPIR_Comm *comm_ptr, MPIR_Sched_t s)
{
    int mpi_errno = MPI_SUCCESS;
    int is_homogeneous ATTRIBUTE((unused));
    int is_commutative;
    int total_count, i;
    int comm_size;

    MPIR_Assert(comm_ptr->comm_kind == MPIR_COMM_KIND__INTRACOMM);

    is_homogeneous = 1;
#ifdef MPID_HAS_HETERO
    if (comm_ptr->is_hetero)
        is_homogeneous = 0;
#endif
    MPIR_Assert(is_homogeneous);

    is_commutative = MPIR_Op_is_commutative(op);

    comm_size = comm_ptr->local_size;
    total_count = 0;
    for (i = 0; i < comm_size; i++) {
        total_count += recvcount[i];
    }
    if (total_count == 0) {
        goto fn_exit;
    }

    if (is_commutative) {
        mpi_errno = MVP_Ireduce_scatter_function(sendbuf, recvbuf, recvcount,
                                                 datatype, op, comm_ptr, s);
    } else /* (!is_commutative) */ {
        int is_block_regular = TRUE;
        for (i = 0; i < (comm_size - 1); ++i) {
            if (recvcount[i] != recvcount[i + 1]) {
                is_block_regular = FALSE;
                break;
            }
        }

        if (MPL_is_pof2(comm_size, NULL) && is_block_regular) {
            /* noncommutative, pof2 size, and block regular */
            mpi_errno = MPIR_Ireduce_scatter_intra_sched_noncommutative(
                sendbuf, recvbuf, recvcount, datatype, op, comm_ptr, s);
        } else {
            /* noncommutative and (non-pof2 or block irregular), use recursive
             * doubling. */
            mpi_errno = MPIR_Ireduce_scatter_intra_sched_recursive_doubling(
                sendbuf, recvbuf, recvcount, datatype, op, comm_ptr, s);
        }
    }

    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIR_Ireduce_scatter_intra_MVP(const void *sendbuf, void *recvbuf,
                                   const int *recvcount, MPI_Datatype datatype,
                                   MPI_Op op, MPIR_Comm *comm_ptr,
                                   MPIR_Sched_t s)
{
    int mpi_errno = MPI_SUCCESS;
    int comm_size, is_homogeneous ATTRIBUTE((unused)), i;
    MPI_Aint sendtype_size, nbytes;

    int two_level_ireduce_scatter = 1;
    int range = 0;
    int range_threshold = 0;
    int range_threshold_intra = 0;

    MPIR_Assert(comm_ptr->comm_kind == MPIR_COMM_KIND__INTRACOMM);

    is_homogeneous = 1;
#ifdef MPID_HAS_HETERO
    if (comm_ptr->is_hetero)
        is_homogeneous = 0;
#endif
    MPIR_Assert(is_homogeneous); /* we don't handle the hetero case right now */
    comm_size = comm_ptr->local_size;
    MPIR_Datatype_get_size_macro(datatype, sendtype_size);
    nbytes = 0;
    for (i = 0; i < comm_size; i++) {
        nbytes += recvcount[i] * sendtype_size;
    }

    // Search for some parameters regardless of whether subsequent selected
    // algorithm is 2-level or not

    // Search for the corresponding system size inside the tuning table
    while ((range < (mvp_size_ireduce_scatter_tuning_table - 1)) &&
           (comm_size > mvp_ireduce_scatter_thresholds_table[range].numproc)) {
        range++;
    }

    // Search for corresponding inter-leader function
    while (
        (range_threshold <
         (mvp_ireduce_scatter_thresholds_table[range].size_inter_table - 1)) &&
        (nbytes > mvp_ireduce_scatter_thresholds_table[range]
                      .inter_leader[range_threshold]
                      .max) &&
        (mvp_ireduce_scatter_thresholds_table[range]
             .inter_leader[range_threshold]
             .max != -1)) {
        range_threshold++;
    }

    // Search for corresponding intra-node function

    // Commenting this for the time being as none of
    // the algorithms are 2-level
    /*
    while ((range_threshold_intra <
            (mvp_ireduce_scatter_thresholds_table[range].size_intra_table - 1))
           && (nbytes >
               mvp_ireduce_scatter_thresholds_table[range].intra_node[range_threshold_intra].max)
           &&
    (mvp_ireduce_scatter_thresholds_table[range].intra_node[range_threshold_intra].max
    != -1)) { range_threshold_intra++;
    }
    */

    MVP_Ireduce_scatter_function = mvp_ireduce_scatter_thresholds_table[range]
                                       .inter_leader[range_threshold]
                                       .MVP_pt_Ireduce_scatter_function;

    MVP_Ireduce_scatter_intra_node_function =
        mvp_ireduce_scatter_thresholds_table[range]
            .intra_node[range_threshold_intra]
            .MVP_pt_Ireduce_scatter_function;

    /* There are currently no two-level nb-reduce_scatter functions hence
       setting to 0 by default */
    two_level_ireduce_scatter =
        mvp_ireduce_scatter_thresholds_table[range]
            .is_two_level_ireduce_scatter[range_threshold];
    if (1 != two_level_ireduce_scatter) {
        mpi_errno = MPIR_Ireduce_scatter_tune_helper_MVP(
            sendbuf, recvbuf, recvcount, datatype, op, comm_ptr, s);
    } else {
        /* Code path should not enter this with the current algorithms*/
    }

    return mpi_errno;
}

int MPIR_Ireduce_scatter_MVP(const void *sendbuf, void *recvbuf,
                             const int *recvcount, MPI_Datatype datatype,
                             MPI_Op op, MPIR_Comm *comm_ptr, MPIR_Sched_t s)
{
    int mpi_errno = MPI_SUCCESS;

    if (comm_ptr->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
#if defined(CHANNEL_MRAIL) || defined(_MVP_CH4_OVERRIDE_)
        mpi_errno = MPIR_Ireduce_scatter_intra_MVP(sendbuf, recvbuf, recvcount,
                                                   datatype, op, comm_ptr, s);
#else
        mpi_errno = MPIR_Ireduce_scatter_intra(sendbuf, recvbuf, recvcount,
                                               datatype, op, comm_ptr, s);
#endif /*#if defined(CHANNEL_MRAIL) || defined(_MVP_CH4_OVERRIDE_) */
    } else {
        mpi_errno = MPIR_Ireduce_scatter_inter_sched_auto(
            sendbuf, recvbuf, recvcount, datatype, op, comm_ptr, s);
    }

    return mpi_errno;
}
