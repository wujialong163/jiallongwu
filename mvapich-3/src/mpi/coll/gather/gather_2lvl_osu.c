#include "gather_tuning.h"

static void *tmp_buf = NULL;

int MPIR_Gather_MVP_two_level_Direct(const void *sendbuf, int sendcnt,
                                     MPI_Datatype sendtype, void *recvbuf,
                                     int recvcnt, MPI_Datatype recvtype,
                                     int root, MPIR_Comm *comm_ptr,
                                     MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, gather, two_level_direct);
    void *leader_gather_buf = NULL;
    int comm_size, rank;
    int local_rank, local_size;
    int leader_comm_rank = -1, leader_comm_size = 0;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    MPI_Aint recvtype_size = 0, sendtype_size = 0, nbytes = 0;
    int leader_root, leader_of_root;
    MPI_Status status;
    MPI_Aint sendtype_extent = 0, recvtype_extent = 0; /* Datatype extent */
    MPI_Aint true_lb, sendtype_true_extent, recvtype_true_extent;
    MPI_Comm shmem_comm, leader_comm;
    MPIR_Comm *shmem_commptr, *leader_commptr = NULL;

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_gather_two_level_direct, 1);
    comm_size = comm_ptr->local_size;
    rank = comm_ptr->rank;

    if (((rank == root) && (recvcnt == 0)) ||
        ((rank != root) && (sendcnt == 0))) {
        return MPI_SUCCESS;
    }

    if (sendtype != MPI_DATATYPE_NULL) {
        MPIR_Datatype_get_extent_macro(sendtype, sendtype_extent);
        MPIR_Datatype_get_size_macro(sendtype, sendtype_size);
        MPIR_Type_get_true_extent_impl(sendtype, &true_lb,
                                       &sendtype_true_extent);
    }
    if (recvtype != MPI_DATATYPE_NULL) {
        MPIR_Datatype_get_extent_macro(recvtype, recvtype_extent);
        MPIR_Datatype_get_size_macro(recvtype, recvtype_size);
        MPIR_Type_get_true_extent_impl(recvtype, &true_lb,
                                       &recvtype_true_extent);
    }

    /* extract the rank,size information for the intra-node
     * communicator */
    shmem_comm = comm_ptr->dev.ch.shmem_comm;
    MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
    local_rank = shmem_commptr->rank;
    local_size = shmem_commptr->local_size;

    if (local_rank == 0) {
        /* Node leader. Extract the rank, size information for the leader
         * communicator */
        leader_comm = comm_ptr->dev.ch.leader_comm;
        MPIR_Comm_get_ptr(leader_comm, leader_commptr);
        leader_comm_rank = leader_commptr->rank;
        leader_comm_size = leader_commptr->local_size;
    }

    if (rank == root) {
        nbytes = recvcnt * recvtype_size;

    } else {
        nbytes = sendcnt * sendtype_size;
    }

        if (local_rank == 0) {
            /* Node leader, allocate tmp_buffer */
            if (rank == root) {
                tmp_buf = MPL_malloc(
                    recvcnt * MPL_MAX(recvtype_extent, recvtype_true_extent) *
                        local_size,
                    MPL_MEM_COLL);
            } else {
                tmp_buf = MPL_malloc(
                    sendcnt * MPL_MAX(sendtype_extent, sendtype_true_extent) *
                        local_size,
                    MPL_MEM_COLL);
            }
            if (tmp_buf == NULL) {
                mpi_errno = MPIR_Err_create_code(
                    MPI_SUCCESS, MPIR_ERR_RECOVERABLE, __func__, __LINE__,
                    MPI_ERR_OTHER, "**nomem", 0);
                return mpi_errno;
            }
        }
        /* We are gathering the data into tmp_buf and the output
         * will be of MPI_BYTE datatype. Since the tmp_buf has no
         * local data, we pass is_data_avail = TEMP_BUF_HAS_NO_DATA
         */
        mpi_errno = MPIR_pt_pt_intra_gather(
            sendbuf, sendcnt, sendtype, recvbuf, recvcnt, recvtype, root, rank,
            tmp_buf, nbytes, TEMP_BUF_HAS_NO_DATA, shmem_commptr,
            MVP_Gather_intra_node_function, errflag);

        MPIR_ERR_CHECK(mpi_errno);

        leader_of_root = comm_ptr->dev.ch.leader_map[root];
        leader_root = comm_ptr->dev.ch.leader_rank[leader_of_root];
        /* leader_root is the rank of the leader of the root in leader_comm.
         * leader_root is to be used as the root of the inter-leader gather ops
         */
        if (comm_ptr->dev.ch.is_uniform != 1) {
            if (local_rank == 0) {
                int *displs = NULL;
                int *recvcnts = NULL;
                int *node_sizes;
                int i = 0;
                /* Node leaders have all the data. But, different nodes can have
                 * different number of processes. Do a Gather first to get the
                 * buffer lengths at each leader, followed by a Gatherv to move
                 * the actual data */

                if (leader_comm_rank == leader_root && root != leader_of_root) {
                    /* The root of the Gather operation is not a node-level
                     * leader and this process's rank in the leader_comm
                     * is the same as leader_root */
                    if (rank == root) {
                        leader_gather_buf = MPL_malloc(
                            recvcnt *
                                MPL_MAX(recvtype_extent, recvtype_true_extent) *
                                comm_size,
                            MPL_MEM_COLL);
                    } else {
                        leader_gather_buf = MPL_malloc(
                            sendcnt *
                                MPL_MAX(sendtype_extent, sendtype_true_extent) *
                                comm_size,
                            MPL_MEM_COLL);
                    }
                    if (leader_gather_buf == NULL) {
                        mpi_errno = MPIR_Err_create_code(
                            MPI_SUCCESS, MPIR_ERR_RECOVERABLE, __func__,
                            __LINE__, MPI_ERR_OTHER, "**nomem", 0);

                        MPIR_TIMER_END(coll, gather, two_level_direct);
                        return mpi_errno;
                    }
                }

                node_sizes = comm_ptr->dev.ch.node_sizes;

                if (leader_comm_rank == leader_root) {
                    displs = MPL_malloc(sizeof(int) * leader_comm_size,
                                        MPL_MEM_COLL);
                    recvcnts = MPL_malloc(sizeof(int) * leader_comm_size,
                                          MPL_MEM_COLL);
                    if (!displs || !recvcnts) {
                        mpi_errno = MPIR_Err_create_code(
                            MPI_SUCCESS, MPIR_ERR_RECOVERABLE, __func__,
                            __LINE__, MPI_ERR_OTHER, "**nomem", 0);

                        MPIR_TIMER_END(coll, gather, two_level_direct);
                        return mpi_errno;
                    }
                }

                if (root == leader_of_root) {
                    /* The root of the gather operation is also the node
                     * leader. Receive into recvbuf and we are done */
                    if (leader_comm_rank == leader_root) {
                        recvcnts[0] = node_sizes[0] * recvcnt;
                        displs[0] = 0;

                        for (i = 1; i < leader_comm_size; i++) {
                            displs[i] =
                                displs[i - 1] + node_sizes[i - 1] * recvcnt;
                            recvcnts[i] = node_sizes[i] * recvcnt;
                        }
                    }
                    mpi_errno =
                        MPIR_Gatherv(tmp_buf, local_size * nbytes, MPI_BYTE,
                                     recvbuf, recvcnts, displs, recvtype,
                                     leader_root, leader_commptr, errflag);
                } else {
                    /* The root of the gather operation is not the node leader.
                     * Receive into leader_gather_buf and then send
                     * to the root */
                    if (leader_comm_rank == leader_root) {
                        recvcnts[0] = node_sizes[0] * nbytes;
                        displs[0] = 0;

                        for (i = 1; i < leader_comm_size; i++) {
                            displs[i] =
                                displs[i - 1] + node_sizes[i - 1] * nbytes;
                            recvcnts[i] = node_sizes[i] * nbytes;
                        }
                    }
                    mpi_errno = MPIR_Gatherv(
                        tmp_buf, local_size * nbytes, MPI_BYTE,
                        leader_gather_buf, recvcnts, displs, MPI_BYTE,
                        leader_root, leader_commptr, errflag);
                }
                MPIR_ERR_CHECK(mpi_errno);
                if (leader_comm_rank == leader_root) {
                    MPL_free(displs);
                    MPL_free(recvcnts);
                }
            }
        } else {
            /* All nodes have the same number of processes.
             * Just do one Gather to get all
             * the data at the leader of the root process */
            if (local_rank == 0) {
                if (leader_comm_rank == leader_root && root != leader_of_root) {
                    /* The root of the Gather operation is not a node-level
                     * leader
                     */
                    leader_gather_buf =
                        MPL_malloc(nbytes * comm_size, MPL_MEM_COLL);
                    if (leader_gather_buf == NULL) {
                        mpi_errno = MPIR_Err_create_code(
                            MPI_SUCCESS, MPIR_ERR_RECOVERABLE, __func__,
                            __LINE__, MPI_ERR_OTHER, "**nomem", 0);

                        MPIR_TIMER_END(coll, gather, two_level_direct);
                        return mpi_errno;
                    }
                }
                if (root == leader_of_root) {
                    if (comm_ptr->dev.ch.is_global_block != 1 ||
                        comm_ptr->dev.ch.is_blocked != 1) {
                        /* cyclic process mapping */

                        MPIR_Assert(comm_ptr->dev.ch.rank_list != NULL);

                        void *reorder_tmp_buf = NULL;
                        if (leader_comm_rank == leader_root) {
                            reorder_tmp_buf =
                                MPL_malloc(nbytes * comm_size, MPL_MEM_COLL);
                        }

                        mpi_errno = MPIR_Gather_MVP_Direct(
                            tmp_buf, nbytes * local_size, MPI_BYTE,
                            reorder_tmp_buf, recvcnt * local_size, recvtype,
                            leader_root, leader_commptr, errflag);

                        if (leader_comm_rank == leader_root) {
                            /* now reorder and place the results in recvbuf */
                            void *src = NULL, *dst = NULL;
                            int i;
                            for (i = 0; i < comm_size; i++) {
                                src = reorder_tmp_buf + i * nbytes;
                                dst = recvbuf +
                                      comm_ptr->dev.ch.rank_list[i] * nbytes;
                                if (dst == NULL)
                                    PRINT_ERROR("recvbuf=%a \
                                    comm_ptr->dev.ch.rank_list[%d]=%d\n",
                                                recvbuf, i,
                                                comm_ptr->dev.ch.rank_list[i]);
                                MPIR_Memcpy(dst, src, nbytes);
                            }
                            MPL_free(reorder_tmp_buf);
                        }

                    } else {
                        /* blocked (bunch) process mapping */
                        mpi_errno = MPIR_Gather_MVP_Direct(
                            tmp_buf, nbytes * local_size, MPI_BYTE, recvbuf,
                            recvcnt * local_size, recvtype, leader_root,
                            leader_commptr, errflag);
                    }
                } else {
                    mpi_errno = MPIR_Gather_MVP_Direct(
                        tmp_buf, nbytes * local_size, MPI_BYTE,
                        leader_gather_buf, nbytes * local_size, MPI_BYTE,
                        leader_root, leader_commptr, errflag);
                }
                MPIR_ERR_CHECK(mpi_errno);
            }
        }
    if ((local_rank == 0) && (root != rank) && (leader_of_root == rank)) {
        MPIR_PVAR_INC(gather, two_level_direct, send, nbytes * comm_size,
                      MPI_BYTE);
        mpi_errno = MPIC_Send(leader_gather_buf, nbytes * comm_size, MPI_BYTE,
                              root, MPIR_GATHER_TAG, comm_ptr, errflag);
        if (mpi_errno) {
            /* for communication errors, just record the error
               but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
    }

    if (rank == root && local_rank != 0) {
        /* The root of the gather operation is not the node leader. Receive
         y* data from the node leader */
        MPIR_PVAR_INC(gather, two_level_direct, recv, recvcnt * comm_size,
                      recvtype);
        if (comm_ptr->dev.ch.is_global_block != 1 ||
            comm_ptr->dev.ch.is_blocked != 1) {
            /* cyclic process mapping */
            MPIR_Assert(comm_ptr->dev.ch.rank_list != NULL);
            void *reorder_tmp_buf =
                MPL_malloc(nbytes * comm_size, MPL_MEM_COLL);
            mpi_errno = MPIC_Recv(reorder_tmp_buf, recvcnt * comm_size,
                                  recvtype, leader_of_root, MPIR_GATHER_TAG,
                                  comm_ptr, &status, errflag);
            /* now reorder and place the results in recvbuf */
            void *src = NULL, *dst = NULL;
            int i;
            for (i = 0; i < comm_size; i++) {
                src = reorder_tmp_buf + i * nbytes;
                dst = recvbuf + comm_ptr->dev.ch.rank_list[i] * nbytes;
                if (dst == NULL)
                    PRINT_ERROR("recvbuf=%p\
                         comm_ptr->dev.ch.rank_list[%d]=%d\n",
                                recvbuf, i, comm_ptr->dev.ch.rank_list[i]);
                MPIR_Memcpy(dst, src, nbytes);
            }
            MPL_free(reorder_tmp_buf);
        } else {
            /* blocked (bunch) process mapping */
            mpi_errno = MPIC_Recv(recvbuf, recvcnt * comm_size, recvtype,
                                  leader_of_root, MPIR_GATHER_TAG, comm_ptr,
                                  &status, errflag);
        }
        if (mpi_errno) {
            /* for communication errors, just record the error but
               continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
    }

fn_fail:
    /* check if multiple threads are calling this collective function */
    if (local_rank == 0) {
        if (tmp_buf != NULL) {
            MPL_free(tmp_buf);
        }
        if (leader_gather_buf != NULL) {
            MPL_free(leader_gather_buf);
        }
    }

    MPIR_TIMER_END(coll, gather, two_level_direct);
    return (mpi_errno);
}
