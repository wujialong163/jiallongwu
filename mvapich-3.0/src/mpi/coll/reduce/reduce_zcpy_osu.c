#include "reduce_tuning.h"

#ifdef CHANNEL_MRAIL_GEN2
int MPIR_Reduce_Zcpy_MVP(const void *sendbuf, void *recvbuf, int count,
                         MPI_Datatype datatype, MPI_Op op, int root,
                         MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, reduce, zcpy);
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    int my_rank, local_rank;
    MPI_Comm shmem_comm, leader_comm;
    MPIR_Comm *shmem_commptr = NULL, *leader_commptr = NULL;
    void *in_buf = NULL, *out_buf = NULL;
    MPI_Aint true_lb, true_extent, extent;
    int stride = 0;
    int dst, expected_send_count, expected_recv_count;
    int *src_array = NULL;
    int pseudo_root = 0;
    static int fn_call = 0;
    MPI_Status status;

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_reduce_zcpy, 1);

    fn_call++;

    my_rank = comm_ptr->rank;
    shmem_comm = comm_ptr->dev.ch.shmem_comm;

    MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
    local_rank = shmem_commptr->rank;

    MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);
    MPIR_Datatype_get_extent_macro(datatype, extent);
    stride = count * MPL_MAX(extent, true_extent);

    if (local_rank == 0) {
        int leader_of_psuedo_root, leader_psuedo_root;
        shmem_info_t *shmem_info = NULL;
        leader_comm = comm_ptr->dev.ch.leader_comm;
        MPIR_Comm_get_ptr(leader_comm, leader_commptr);

        leader_of_psuedo_root = comm_ptr->dev.ch.leader_map[pseudo_root];
        leader_psuedo_root =
            comm_ptr->dev.ch.leader_rank[leader_of_psuedo_root];

        shmem_info = comm_ptr->dev.ch.shmem_info;
        /* If the knomial_factor requested for this specific bcast
         * is the same as the one that we have used before, the communication
         * tree is already setup and cached. No need to do it again
         */
        if ((shmem_info)->reduce_knomial_factor !=
            MVP_REDUCE_ZCOPY_INTER_KNOMIAL_FACTOR) {
            MPIR_Reduce_knomial_trace(
                leader_psuedo_root, MVP_REDUCE_ZCOPY_INTER_KNOMIAL_FACTOR,
                leader_commptr, &dst, &expected_send_count,
                &expected_recv_count, &src_array);
            (shmem_info)->reduce_exchange_rdma_keys = 1;
        }
    }

    if (sendbuf != MPI_IN_PLACE) {
        in_buf = (void *)sendbuf;
    } else {
        in_buf = recvbuf;
    }

    mpi_errno = mvp_shm_zcpy_reduce(
        shmem_commptr->dev.ch.shmem_info, in_buf, &out_buf, count, stride,
        datatype, op, root, expected_recv_count, src_array, expected_send_count,
        dst, MVP_REDUCE_ZCOPY_INTER_KNOMIAL_FACTOR, comm_ptr, errflag);
    if (mpi_errno) {
        /* for communication errors, just record the error
         * but continue */
        *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
        MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
    }

    if (my_rank == 0 && root == my_rank) {
        MPIR_Memcpy(recvbuf, out_buf, stride);
    } else {
        /* Send the message to the root if the root is not rank0 */
        if ((my_rank == 0) && (root != my_rank)) {
            MPIR_PVAR_INC(reduce, zcpy, send, count, datatype);
            mpi_errno = MPIC_Send(out_buf, count, datatype, root,
                                  MPIR_REDUCE_TAG, comm_ptr, errflag);
            if (mpi_errno) {
                /* for communication errors, just record the error
                 * but continue */
                fprintf(stderr, "%d send to %d failed, mpi_errno %d\n",
                        comm_ptr->rank, root, mpi_errno);

                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
        }

        if ((my_rank != 0) && (root == my_rank)) {
            MPIR_PVAR_INC(reduce, zcpy, recv, count, datatype);
            mpi_errno = MPIC_Recv(recvbuf, count, datatype, pseudo_root,
                                  MPIR_REDUCE_TAG, comm_ptr, &status, errflag);
            if (mpi_errno) {
                /* for communication errors, just record the error but
                 * continue */
                fprintf(stderr, "%d send to %d failed, mpi_errno %d\n",
                        comm_ptr->rank, root, mpi_errno);
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
        }
    }

    if (mpi_errno_ret)
        mpi_errno = mpi_errno_ret;
    else if (*errflag)
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**coll_fail");

    MPIR_TIMER_END(coll, reduce, zcpy);
    return mpi_errno;
}
#else
int MPIR_Reduce_Zcpy_MVP(const void *sendbuf, void *recvbuf, int count,
                         MPI_Datatype datatype, MPI_Op op, int root,
                         MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    return MPIR_Reduce_inter_knomial_wrapper_MVP(
        sendbuf, recvbuf, count, datatype, op, root, comm_ptr, errflag);
}
#endif /* CHANNEL_MRAIL_GEN2 */
