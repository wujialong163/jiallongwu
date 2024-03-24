#include "scatter_tuning.h"

int MPIR_Scatter_MVP_two_level_Binomial(const void *sendbuf, int sendcnt,
                                        MPI_Datatype sendtype, void *recvbuf,
                                        int recvcnt, MPI_Datatype recvtype,
                                        int root, MPIR_Comm *comm_ptr,
                                        MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, scatter, two_level_binomial);
    int comm_size, rank;
    int local_rank, local_size;
    int leader_comm_rank, leader_comm_size;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    MPI_Aint recvtype_size, sendtype_size, nbytes;
    void *tmp_buf = NULL;
    void *leader_scatter_buf = NULL;
    MPI_Status status;
    int leader_root = -1, leader_of_root = -1;
    MPI_Comm shmem_comm, leader_comm;
    MPIR_Comm *shmem_commptr, *leader_commptr = NULL;

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_scatter_two_level_binomial, 1);
    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_num_shmem_coll_calls, 1);
    comm_size = comm_ptr->local_size;
    rank = comm_ptr->rank;

    if (((rank == root) && (sendcnt == 0)) ||
        ((rank != root) && (recvcnt == 0))) {
        MPIR_TIMER_END(coll, scatter, two_level_binomial);
        return MPI_SUCCESS;
    }
    /* extract the rank,size information for the intra-node
     * communicator */
    shmem_comm = comm_ptr->dev.ch.shmem_comm;
    MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
    local_rank = MPIR_Comm_rank(shmem_commptr);
    local_size = MPIR_Comm_size(shmem_commptr);

    if (local_rank == 0) {
        /* Node leader. Extract the rank, size information for the leader
         * communicator */
        leader_comm = comm_ptr->dev.ch.leader_comm;
        MPIR_Comm_get_ptr(leader_comm, leader_commptr);
        leader_comm_rank = MPIR_Comm_rank(leader_commptr);
        leader_comm_size = MPIR_Comm_size(leader_commptr);
    }

    if (local_size == comm_size) {
        /* purely intra-node scatter.
         * Just use the direct algorithm and we are done */
        mpi_errno =
            MPIR_Scatter_MVP_Direct(sendbuf, sendcnt, sendtype, recvbuf,
                                    recvcnt, recvtype, root, comm_ptr, errflag);
        if (mpi_errno) {
            /* for communication errors, just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
    } else {
        MPIR_Datatype_get_size_macro(recvtype, recvtype_size);
        MPIR_Datatype_get_size_macro(sendtype, sendtype_size);

        if (rank == root) {
            nbytes = sendcnt * sendtype_size;
        } else {
            nbytes = recvcnt * recvtype_size;
        }

        if (local_rank == 0) {
            /* Node leader, allocate tmp_buffer */
            tmp_buf = MPL_malloc(nbytes * local_size, MPL_MEM_COLL);
        }

        leader_of_root = comm_ptr->dev.ch.leader_map[root];
        /* leader_of_root is the global rank of the leader of the root */
        leader_root = comm_ptr->dev.ch.leader_rank[leader_of_root];
        /* leader_root is the rank of the leader of the root in leader_comm.
         * leader_root is to be used as the root of the inter-leader gather ops
         */

        if ((local_rank == 0) && (root != rank) && (leader_of_root == rank)) {
            /* The root of the scatter operation is not the node leader. Recv
             * data from the node leader */
            leader_scatter_buf = MPL_malloc(nbytes * comm_size, MPL_MEM_COLL);
            MPIR_PVAR_INC(scatter, two_level_binomial, recv, nbytes * comm_size,
                          MPI_BYTE);
            mpi_errno =
                MPIC_Recv(leader_scatter_buf, nbytes * comm_size, MPI_BYTE,
                          root, MPIR_SCATTER_TAG, comm_ptr, &status, errflag);
            if (mpi_errno) {
                /* for communication errors,
                 * just record the error but continue */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
        }

        if (rank == root && local_rank != 0) {
            /* The root of the scatter operation is not the node leader. Send
             * data to the node leader */
            MPIR_PVAR_INC(scatter, two_level_binomial, send,
                          sendcnt * comm_size, sendtype);
            mpi_errno =
                MPIC_Send(sendbuf, sendcnt * comm_size, sendtype,
                          leader_of_root, MPIR_SCATTER_TAG, comm_ptr, errflag);
            if (mpi_errno) {
                /* for communication errors,
                 * just record the error but continue */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
        }

        if (leader_comm_size > 1 && local_rank == 0) {
            if (comm_ptr->dev.ch.is_uniform != 1) {
                int *displs = NULL;
                int *sendcnts = NULL;
                int *node_sizes;
                int i = 0;
                node_sizes = comm_ptr->dev.ch.node_sizes;

                if (root != leader_of_root) {
                    if (leader_comm_rank == leader_root) {
                        displs = MPL_malloc(sizeof(int) * leader_comm_size,
                                            MPL_MEM_COLL);
                        sendcnts = MPL_malloc(sizeof(int) * leader_comm_size,
                                              MPL_MEM_COLL);
                        sendcnts[0] = node_sizes[0] * nbytes;
                        displs[0] = 0;

                        for (i = 1; i < leader_comm_size; i++) {
                            displs[i] =
                                displs[i - 1] + node_sizes[i - 1] * nbytes;
                            sendcnts[i] = node_sizes[i] * nbytes;
                        }
                    }
                    mpi_errno = MPIR_Scatterv(
                        leader_scatter_buf, sendcnts, displs, MPI_BYTE, tmp_buf,
                        nbytes * local_size, MPI_BYTE, leader_root,
                        leader_commptr, errflag);
                } else {
                    if (leader_comm_rank == leader_root) {
                        displs = MPL_malloc(sizeof(int) * leader_comm_size,
                                            MPL_MEM_COLL);
                        sendcnts = MPL_malloc(sizeof(int) * leader_comm_size,
                                              MPL_MEM_COLL);
                        sendcnts[0] = node_sizes[0] * sendcnt;
                        displs[0] = 0;

                        for (i = 1; i < leader_comm_size; i++) {
                            displs[i] =
                                displs[i - 1] + node_sizes[i - 1] * sendcnt;
                            sendcnts[i] = node_sizes[i] * sendcnt;
                        }
                    }
                    mpi_errno =
                        MPIR_Scatterv(sendbuf, sendcnts, displs, sendtype,
                                      tmp_buf, nbytes * local_size, MPI_BYTE,
                                      leader_root, leader_commptr, errflag);
                }
                if (mpi_errno) {
                    /* for communication errors,
                     * just record the error but continue */
                    *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                    MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                }
                if (leader_comm_rank == leader_root) {
                    MPL_free(displs);
                    MPL_free(sendcnts);
                }
            } else {
                if (leader_of_root != root) {
                    mpi_errno = MPIR_Scatter_MVP_Binomial(
                        leader_scatter_buf, nbytes * local_size, MPI_BYTE,
                        tmp_buf, nbytes * local_size, MPI_BYTE, leader_root,
                        leader_commptr, errflag);
                } else {
                    mpi_errno = MPIR_Scatter_MVP_Binomial(
                        sendbuf, sendcnt * local_size, sendtype, tmp_buf,
                        nbytes * local_size, MPI_BYTE, leader_root,
                        leader_commptr, errflag);
                }
                if (mpi_errno) {
                    /* for communication errors,
                     * just record the error but continue */
                    *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                    MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                }
            }
        }
        /* The leaders are now done with the inter-leader part.
         * Scatter the data within the nodes */

        if (rank == root && recvbuf == MPI_IN_PLACE) {
            mpi_errno = MVP_Scatter_intra_function(
                tmp_buf, nbytes, MPI_BYTE, (void *)sendbuf, sendcnt, sendtype,
                0, shmem_commptr, errflag);
        } else {
            mpi_errno = MVP_Scatter_intra_function(tmp_buf, nbytes, MPI_BYTE,
                                                   recvbuf, recvcnt, recvtype,
                                                   0, shmem_commptr, errflag);
        }
        if (mpi_errno) {
            /* for communication errors, just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }

        MPIR_ERR_CHECK(mpi_errno);
    }

fn_fail:
    /* check if multiple threads are calling this collective function */
    if (comm_size != local_size && local_rank == 0) {
        MPL_free(tmp_buf);
        if (leader_of_root == rank && root != rank) {
            MPL_free(leader_scatter_buf);
        }
    }

    MPIR_TIMER_END(coll, scatter, two_level_binomial);
    return (mpi_errno);
}

int MPIR_Scatter_MVP_two_level_Direct(const void *sendbuf, int sendcnt,
                                      MPI_Datatype sendtype, void *recvbuf,
                                      int recvcnt, MPI_Datatype recvtype,
                                      int root, MPIR_Comm *comm_ptr,
                                      MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, scatter, two_level_direct);
    int comm_size, rank;
    int local_rank, local_size;
    int leader_comm_rank, leader_comm_size;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    MPI_Aint recvtype_size, sendtype_size, nbytes;
    void *tmp_buf = NULL;
    void *leader_scatter_buf = NULL;
    MPI_Status status;
    int leader_root, leader_of_root = -1;
    MPI_Comm shmem_comm, leader_comm;
    MPIR_Comm *shmem_commptr, *leader_commptr = NULL;

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_scatter_two_level_direct, 1);
    comm_size = comm_ptr->local_size;
    rank = comm_ptr->rank;

    if (((rank == root) && (sendcnt == 0)) ||
        ((rank != root) && (recvcnt == 0))) {
        MPIR_TIMER_END(coll, scatter, two_level_direct);
        return MPI_SUCCESS;
    }

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_num_shmem_coll_calls, 1);

    /* extract the rank,size information for the intra-node
     * communicator */
    shmem_comm = comm_ptr->dev.ch.shmem_comm;
    MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
    local_rank = MPIR_Comm_rank(shmem_commptr);
    local_size = MPIR_Comm_size(shmem_commptr);

    if (local_rank == 0) {
        /* Node leader. Extract the rank, size information for the leader
         * communicator */
        leader_comm = comm_ptr->dev.ch.leader_comm;
        MPIR_Comm_get_ptr(leader_comm, leader_commptr);
        leader_comm_rank = MPIR_Comm_rank(leader_commptr);
        leader_comm_size = MPIR_Comm_size(leader_commptr);
    }

    if (local_size == comm_size) {
        /* purely intra-node scatter.
         * Just use the direct algorithm and we are done */
        mpi_errno =
            MPIR_Scatter_MVP_Direct(sendbuf, sendcnt, sendtype, recvbuf,
                                    recvcnt, recvtype, root, comm_ptr, errflag);
        MPIR_ERR_CHECK(mpi_errno);
    } else {
        MPIR_Datatype_get_size_macro(recvtype, recvtype_size);
        MPIR_Datatype_get_size_macro(sendtype, sendtype_size);

        if (rank == root) {
            nbytes = sendcnt * sendtype_size;
        } else {
            nbytes = recvcnt * recvtype_size;
        }

        if (local_rank == 0) {
            /* Node leader, allocate tmp_buffer */
            tmp_buf = MPL_malloc(nbytes * local_size, MPL_MEM_COLL);
        }

        leader_of_root = comm_ptr->dev.ch.leader_map[root];
        /* leader_of_root is the global rank of the leader of the root */
        leader_root = comm_ptr->dev.ch.leader_rank[leader_of_root];
        /* leader_root is the rank of the leader of the root in leader_comm.
         * leader_root is to be used as the root of the inter-leader gather ops
         */

        if ((local_rank == 0) && (root != rank) && (leader_of_root == rank)) {
            /* The root of the scatter operation is not the node leader. Recv
             * data from the node leader */
            leader_scatter_buf = MPL_malloc(nbytes * comm_size, MPL_MEM_COLL);
            MPIR_PVAR_INC(scatter, two_level_direct, recv, nbytes * comm_size,
                          MPI_BYTE);
            mpi_errno =
                MPIC_Recv(leader_scatter_buf, nbytes * comm_size, MPI_BYTE,
                          root, MPIR_SCATTER_TAG, comm_ptr, &status, errflag);
            if (mpi_errno) {
                /* for communication errors,
                 * just record the error but continue */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
        }

        if (rank == root && local_rank != 0) {
            /* The root of the scatter operation is not the node leader. Send
             * data to the node leader */
            MPIR_PVAR_INC(scatter, two_level_direct, send, sendcnt * comm_size,
                          sendtype);
            mpi_errno =
                MPIC_Send(sendbuf, sendcnt * comm_size, sendtype,
                          leader_of_root, MPIR_SCATTER_TAG, comm_ptr, errflag);
            if (mpi_errno) {
                /* for communication errors,
                 * just record the error but continue */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
        }

        if (leader_comm_size > 1 && local_rank == 0) {
            if (comm_ptr->dev.ch.is_uniform != 1) {
                int *displs = NULL;
                int *sendcnts = NULL;
                int *node_sizes;
                int i = 0;
                node_sizes = comm_ptr->dev.ch.node_sizes;

                if (root != leader_of_root) {
                    if (leader_comm_rank == leader_root) {
                        displs = MPL_malloc(sizeof(int) * leader_comm_size,
                                            MPL_MEM_COLL);
                        sendcnts = MPL_malloc(sizeof(int) * leader_comm_size,
                                              MPL_MEM_COLL);
                        sendcnts[0] = node_sizes[0] * nbytes;
                        displs[0] = 0;

                        for (i = 1; i < leader_comm_size; i++) {
                            displs[i] =
                                displs[i - 1] + node_sizes[i - 1] * nbytes;
                            sendcnts[i] = node_sizes[i] * nbytes;
                        }
                    }
                    mpi_errno = MPIR_Scatterv(
                        leader_scatter_buf, sendcnts, displs, MPI_BYTE, tmp_buf,
                        nbytes * local_size, MPI_BYTE, leader_root,
                        leader_commptr, errflag);
                } else {
                    if (leader_comm_rank == leader_root) {
                        displs = MPL_malloc(sizeof(int) * leader_comm_size,
                                            MPL_MEM_COLL);
                        sendcnts = MPL_malloc(sizeof(int) * leader_comm_size,
                                              MPL_MEM_COLL);
                        sendcnts[0] = node_sizes[0] * sendcnt;
                        displs[0] = 0;

                        for (i = 1; i < leader_comm_size; i++) {
                            displs[i] =
                                displs[i - 1] + node_sizes[i - 1] * sendcnt;
                            sendcnts[i] = node_sizes[i] * sendcnt;
                        }
                    }
                    mpi_errno =
                        MPIR_Scatterv(sendbuf, sendcnts, displs, sendtype,
                                      tmp_buf, nbytes * local_size, MPI_BYTE,
                                      leader_root, leader_commptr, errflag);
                }
                if (mpi_errno) {
                    /* for communication errors,
                     * just record the error but continue */
                    *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                    MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                }
                if (leader_comm_rank == leader_root) {
                    MPL_free(displs);
                    MPL_free(sendcnts);
                }
            } else {
                if (leader_of_root != root) {
                    mpi_errno = MPIR_Scatter_MVP_Direct(
                        leader_scatter_buf, nbytes * local_size, MPI_BYTE,
                        tmp_buf, nbytes * local_size, MPI_BYTE, leader_root,
                        leader_commptr, errflag);
                } else {
                    mpi_errno = MPIR_Scatter_MVP_Direct(
                        sendbuf, sendcnt * local_size, sendtype, tmp_buf,
                        nbytes * local_size, MPI_BYTE, leader_root,
                        leader_commptr, errflag);
                }
                if (mpi_errno) {
                    /* for communication errors,
                     * just record the error but continue */
                    *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                    MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                }
            }
        }
        /* The leaders are now done with the inter-leader part.
         * Scatter the data within the nodes */

        if (rank == root && recvbuf == MPI_IN_PLACE) {
            mpi_errno = MVP_Scatter_intra_function(
                tmp_buf, nbytes, MPI_BYTE, (void *)sendbuf, sendcnt, sendtype,
                0, shmem_commptr, errflag);
        } else {
            mpi_errno = MVP_Scatter_intra_function(tmp_buf, nbytes, MPI_BYTE,
                                                   recvbuf, recvcnt, recvtype,
                                                   0, shmem_commptr, errflag);
        }
        if (mpi_errno) {
            /* for communication errors, just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
    }

fn_fail:
    /* check if multiple threads are calling this collective function */
    if (comm_size != local_size && local_rank == 0) {
        MPL_free(tmp_buf);
        if (leader_of_root == rank && root != rank) {
            MPL_free(leader_scatter_buf);
        }
    }

    MPIR_TIMER_END(coll, scatter, two_level_direct);
    return (mpi_errno);
}
