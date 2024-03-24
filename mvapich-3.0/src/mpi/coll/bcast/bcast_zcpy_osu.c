#include "bcast_tuning.h"

int MPIR_Knomial_Bcast_inter_node_trace_MVP(
    int root, int mvp_bcast_knomial_factor, int *src, int *expected_send_count,
    int *expected_recv_count, int **dst_array, MPIR_Comm *comm_ptr)
{
    int mask = 0x1, k, local_size, dst, rank, relative_rank;
    int orig_mask = 0x1;
    int recv_iter = 0, send_iter = 0;
    int *knomial_bcast_dst_array = NULL;
    rank = comm_ptr->rank;
    local_size = comm_ptr->local_size;

    relative_rank = (rank >= root) ? rank - root : rank - root + local_size;

    while (mask < local_size) {
        if (relative_rank % (mvp_bcast_knomial_factor * mask)) {
            *src = relative_rank / (mvp_bcast_knomial_factor * mask) *
                       (mvp_bcast_knomial_factor * mask) +
                   root;
            if (*src >= local_size) {
                *src -= local_size;
            }
            recv_iter++;
            break;
        }
        mask *= mvp_bcast_knomial_factor;
    }
    mask /= mvp_bcast_knomial_factor;

    orig_mask = mask;
    while (mask > 0) {
        for (k = 1; k < mvp_bcast_knomial_factor; k++) {
            if (relative_rank + mask * k < local_size) {
                send_iter++;
            }
        }
        mask /= mvp_bcast_knomial_factor;
    }

    /* Finally, fill up the dst array */
    if (send_iter > 0) {
        knomial_bcast_dst_array =
            MPL_malloc(sizeof(int) * send_iter, MPL_MEM_COLL);
    }

    mask = orig_mask;
    send_iter = 0;
    while (mask > 0) {
        for (k = 1; k < mvp_bcast_knomial_factor; k++) {
            if (relative_rank + mask * k < local_size) {
                dst = rank + mask * k;
                if (dst >= local_size) {
                    dst -= local_size;
                }
                knomial_bcast_dst_array[send_iter++] = dst;
            }
        }
        mask /= mvp_bcast_knomial_factor;
    }

    *expected_recv_count = recv_iter;
    *expected_send_count = send_iter;
    *dst_array = knomial_bcast_dst_array;
    return 0;
}

#ifdef CHANNEL_MRAIL_GEN2
int MPIR_Shmem_Bcast_Zcpy_MVP(void *buffer, int count, MPI_Datatype datatype,
                              int root, int src, int expected_recv_count,
                              int *dst_array, int expected_send_count,
                              int knomial_factor, MPIR_Comm *comm_ptr,
                              MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, bcast, shmem_zcpy);
    int mpi_errno = MPI_SUCCESS;
    MPI_Aint type_size;
    intptr_t nbytes;
    MPI_Comm shmem_comm;
    MPIR_Comm *shmem_commptr = NULL;

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_bcast_shmem_zcpy, 1);

    MPIR_Datatype_get_size_macro(datatype, type_size);
    nbytes = (intptr_t)(count) * (type_size);
    shmem_comm = comm_ptr->dev.ch.shmem_comm;
    MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);

    MPIR_Assert(MVP_USE_ZCOPY_BCAST && MVP_USE_SLOT_SHMEM_COLL);
    if (count == 0) {
        goto fn_exit;
    }

    if (MVP_USE_SLOT_SHMEM_COLL && MVP_USE_SLOT_SHMEM_BCAST) {
        char *buf;
        int len;
        intptr_t pos;
        MPI_Aint extent;
        MPI_Aint true_lb, true_extent;
        MPIR_Datatype_get_extent_macro(datatype, extent);
        MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);
        nbytes = count * extent;
        for (pos = 0; pos < nbytes; pos += MVP_SHMEM_COLL_SLOT_LEN) {
            buf = (char *)buffer + true_lb + pos;
            len = MIN(nbytes - pos, MVP_SHMEM_COLL_SLOT_LEN);
            mpi_errno = mvp_shm_zcpy_bcast(
                shmem_commptr->dev.ch.shmem_info, buf, len, root, src,
                expected_recv_count, dst_array, expected_send_count,
                knomial_factor, comm_ptr);
            MPIR_ERR_CHECK(mpi_errno);
        }
        goto fn_exit;
    }

fn_exit:
    MPIR_TIMER_END(coll, bcast, shmem_zcpy);
    return mpi_errno;

fn_fail:
    goto fn_fail;
}

int MPIR_Pipelined_Bcast_Zcpy_MVP(void *buffer, int count,
                                  MPI_Datatype datatype, int root,
                                  MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, bcast, pipelined_zcpy);
    MPI_Comm shmem_comm;
    MPIR_Comm *shmem_commptr = NULL;
    int local_rank = 0, rank = 0;
    int mpi_errno = MPI_SUCCESS;
    int new_root = 0;
    ;
    intptr_t nbytes = 0;
    int leader_of_root = 0, leader_root = 0;
    intptr_t rem_count = 0, bcast_curr_count = 0;
    int bcast_segment_count = 0;
    int src, expected_send_count = -1, expected_recv_count = -1;
    int *dst_array = NULL;
    MPI_Aint extent;
    MPI_Aint true_extent, true_lb;
    void *tmp_buf = NULL;
    static int fn_call = 0;
    MPIR_Request *prev_request = NULL, *next_request = NULL;
    MPI_Status prev_status, next_status;

    MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);
    /* even though we always call this algorithm with contiguous buffer, still,
     * the datatype might have some holes in the beginning. Therefore, true_lb
     * might be non zero */
    tmp_buf = buffer + true_lb;

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_bcast_pipelined_zcpy, 1);

    rank = comm_ptr->rank;
    shmem_comm = comm_ptr->dev.ch.shmem_comm;
    MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
    MPIR_Datatype_get_extent_macro(datatype, extent);

    MPIR_Assert(MVP_USE_ZCOPY_BCAST && MVP_USE_SLOT_SHMEM_COLL);
    local_rank = shmem_commptr->rank;
    nbytes = count * extent;
    rem_count = nbytes;
    bcast_segment_count = MIN(rem_count, MVP_BCAST_SEGMENT_SIZE);

    leader_of_root = comm_ptr->dev.ch.leader_map[new_root];
    leader_root = comm_ptr->dev.ch.leader_rank[leader_of_root];

    if (local_rank == 0) {
        MPIR_Comm *leader_commptr = NULL;
        shmem_info_t *shmem_info = NULL;
        MPI_Comm leader_comm;
        leader_comm = comm_ptr->dev.ch.leader_comm;
        MPIR_Comm_get_ptr(leader_comm, leader_commptr);

        shmem_info = comm_ptr->dev.ch.shmem_info;
        /* If the knomial_factor requested for this specific bcast
         * is the same as the one that we have used before, the communication
         * tree is already setup and cached. No need to do it again */
        if ((shmem_info)->bcast_knomial_factor !=
            MVP_PIPELINED_ZCPY_BCAST_KNOMIAL_FACTOR) {
            MPIR_Knomial_Bcast_inter_node_trace_MVP(
                leader_root, MVP_PIPELINED_ZCPY_BCAST_KNOMIAL_FACTOR, &src,
                &expected_send_count, &expected_recv_count, &dst_array,
                leader_commptr);
            (shmem_info)->bcast_exchange_rdma_keys = 1;
        }
    }

    /* If root is not 0, send the data the rank0. This
     * is because we are re-using the communication tree
     * that we have already set up */
    if (rank == root && rank != 0) {
        MPIR_PVAR_INC(bcast, pipelined_zcpy, send, bcast_segment_count,
                      MPI_BYTE);
        mpi_errno = MPIC_Isend(
            (char *)tmp_buf + bcast_curr_count, bcast_segment_count, MPI_BYTE,
            new_root, MPIR_BCAST_TAG, comm_ptr, &prev_request, errflag);
        MPIR_ERR_CHECK(mpi_errno);
    }

    if (rank == 0 && rank != root) {
        MPIR_PVAR_INC(bcast, pipelined_zcpy, recv, bcast_segment_count,
                      MPI_BYTE);
        mpi_errno =
            MPIC_Irecv((char *)tmp_buf + bcast_curr_count, bcast_segment_count,
                       MPI_BYTE, root, MPIR_BCAST_TAG, comm_ptr, &prev_request);
        MPIR_ERR_CHECK(mpi_errno);
    }

    while (bcast_curr_count < nbytes) {
        comm_ptr->dev.ch.intra_node_done = 0;

        if (rank == root && rank != 0) {
            int bcast_next_segment_count = 0;
            bcast_next_segment_count =
                MIN(rem_count - bcast_segment_count, bcast_segment_count);
            if (bcast_curr_count + bcast_segment_count < nbytes) {
                MPIR_PVAR_INC(bcast, pipelined_zcpy, send,
                              bcast_next_segment_count, MPI_BYTE);
                mpi_errno = MPIC_Isend(
                    (char *)tmp_buf + bcast_curr_count + bcast_segment_count,
                    bcast_next_segment_count, MPI_BYTE, new_root,
                    MPIR_BCAST_TAG, comm_ptr, &next_request, errflag);
                MPIR_ERR_CHECK(mpi_errno);
            }
        }

        if (rank == 0 && rank != root) {
            int bcast_next_segment_count = 0;
            bcast_next_segment_count =
                MIN(rem_count - bcast_segment_count, bcast_segment_count);
            if (bcast_curr_count + bcast_segment_count < nbytes) {
                MPIR_PVAR_INC(bcast, pipelined_zcpy, recv,
                              bcast_next_segment_count, MPI_BYTE);
                mpi_errno = MPIC_Irecv((char *)tmp_buf + bcast_curr_count +
                                           bcast_segment_count,
                                       bcast_next_segment_count, MPI_BYTE, root,
                                       MPIR_BCAST_TAG, comm_ptr, &next_request);
                MPIR_ERR_CHECK(mpi_errno);
            }
        }

        if ((rank == root && rank != 0) || (rank == 0 && rank != root)) {
            mpi_errno = MPIC_Waitall(1, &prev_request, &prev_status, errflag);
            prev_request = next_request;
            prev_status = next_status;
        }

        mpi_errno = MPIR_Shmem_Bcast_Zcpy_MVP(
            (char *)tmp_buf + bcast_curr_count, bcast_segment_count, MPI_BYTE,
            leader_root, src, expected_recv_count, dst_array,
            expected_send_count, MVP_PIPELINED_ZCPY_BCAST_KNOMIAL_FACTOR,
            comm_ptr, errflag);
        MPIR_ERR_CHECK(mpi_errno);
        bcast_curr_count += bcast_segment_count;
        rem_count -= bcast_segment_count;
        bcast_segment_count = MIN(rem_count, bcast_segment_count);
    }

    comm_ptr->dev.ch.intra_node_done = 1;
    if (dst_array != NULL) {
        MPL_free(dst_array);
    }
    fn_call++;

fn_exit:
    MPIR_TIMER_END(coll, bcast, pipelined_zcpy);
    return mpi_errno;

fn_fail:
    goto fn_exit;
}
#else
int MPIR_Pipelined_Bcast_Zcpy_MVP(void *buffer, int count,
                                  MPI_Datatype datatype, int root,
                                  MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    return MPIR_Bcast_binomial_MVP(buffer, count, datatype, root, comm_ptr,
                                   errflag);
}
#endif /* CHANNEL_MRAIL_GEN2 */
