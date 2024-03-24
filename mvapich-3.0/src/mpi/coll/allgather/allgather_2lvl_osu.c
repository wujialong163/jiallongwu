#include "allgather_tuning.h"

extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allgather_2lvl;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allgather_2lvl_nonblocked;
extern MPIR_T_pvar_timer_t
    PVAR_TIMER_mvp_coll_timer_allgather_2lvl_ring_nonblocked;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allgather_2lvl_direct;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allgather_2lvl_ring;

extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_2lvl;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_2lvl_nonblocked;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_2lvl_ring_nonblocked;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_2lvl_direct;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_2lvl_ring;

extern unsigned long long
    PVAR_COUNTER_mvp_coll_allgather_2lvl_ring_nonblocked_bytes_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_allgather_2lvl_direct_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_2lvl_ring_bytes_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_allgather_2lvl_ring_nonblocked_bytes_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_allgather_2lvl_direct_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_2lvl_ring_bytes_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_allgather_2lvl_ring_nonblocked_count_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_allgather_2lvl_direct_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_2lvl_ring_count_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_allgather_2lvl_ring_nonblocked_count_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_allgather_2lvl_direct_count_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_2lvl_ring_count_recv;

int MPIR_2lvl_Allgather_MVP(const void *sendbuf, int sendcnt,
                            MPI_Datatype sendtype, void *recvbuf, int recvcnt,
                            MPI_Datatype recvtype, MPIR_Comm *comm_ptr,
                            MPIR_Errflag_t *errflag)
{
    int rank, size;
    int local_rank, local_size;
    int leader_comm_size = 0;
    int mpi_errno = MPI_SUCCESS;
    MPI_Aint recvtype_extent = 0; /* Datatype extent */
    MPI_Comm shmem_comm, leader_comm;
    MPIR_Comm *shmem_commptr = NULL, *leader_commptr = NULL;

    if (recvcnt == 0) {
        return MPI_SUCCESS;
    }
    MPIR_TIMER_START(coll, allgather, 2lvl);
    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_allgather_2lvl, 1);

    rank = comm_ptr->rank;
    size = comm_ptr->local_size;

    /* extract the rank,size information for the intra-node
     * communicator */
    MPIR_Datatype_get_extent_macro(recvtype, recvtype_extent);

    shmem_comm = comm_ptr->dev.ch.shmem_comm;
    MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
    local_rank = shmem_commptr->rank;
    local_size = shmem_commptr->local_size;

    if (local_rank == 0) {
        /* Node leader. Extract the rank, size information for the leader
         * communicator */
        leader_comm = comm_ptr->dev.ch.leader_comm;
        MPIR_Comm_get_ptr(leader_comm, leader_commptr);
        leader_comm_size = leader_commptr->local_size;
    }

    /*If there is just one node, after gather itself,
     * root has all the data and it can do bcast*/
    if (local_rank == 0) {
        mpi_errno = MPIR_Gather_impl(
            sendbuf, sendcnt, sendtype,
            (void *)((char *)recvbuf + (rank * recvcnt * recvtype_extent)),
            recvcnt, recvtype, 0, shmem_commptr, errflag);
    } else {
        /*Since in allgather all the processes could have
         * its own data in place*/
        if (sendbuf == MPI_IN_PLACE) {
            mpi_errno = MPIR_Gather_impl(
                (void *)((char *)recvbuf + (rank * recvcnt * recvtype_extent)),
                recvcnt, recvtype, recvbuf, recvcnt, recvtype, 0, shmem_commptr,
                errflag);
        } else {
            mpi_errno =
                MPIR_Gather_impl(sendbuf, sendcnt, sendtype, recvbuf, recvcnt,
                                 recvtype, 0, shmem_commptr, errflag);
        }
    }

    MPIR_ERR_CHECK(mpi_errno);

    /* Exchange the data between the node leaders*/
    if (local_rank == 0 && (leader_comm_size > 1)) {
        /*When data in each socket is different*/
        if (comm_ptr->dev.ch.is_uniform != 1) {
            int *displs = NULL;
            int *recvcnts = NULL;
            int *node_sizes;
            int i = 0;

            node_sizes = comm_ptr->dev.ch.node_sizes;

            displs = MPL_malloc(sizeof(int) * leader_comm_size, MPL_MEM_COLL);
            recvcnts = MPL_malloc(sizeof(int) * leader_comm_size, MPL_MEM_COLL);
            if (!displs || !recvcnts) {
                mpi_errno = MPIR_Err_create_code(
                    MPI_SUCCESS, MPIR_ERR_RECOVERABLE, __func__, __LINE__,
                    MPI_ERR_OTHER, "**nomem", 0);
                goto fn_fail;
            }
            recvcnts[0] = node_sizes[0] * recvcnt;
            displs[0] = 0;

            for (i = 1; i < leader_comm_size; i++) {
                displs[i] = displs[i - 1] + node_sizes[i - 1] * recvcnt;
                recvcnts[i] = node_sizes[i] * recvcnt;
            }

            mpi_errno = MPIR_Allgatherv(MPI_IN_PLACE, (recvcnt * local_size),
                                        recvtype, recvbuf, recvcnts, displs,
                                        recvtype, leader_commptr, errflag);
            MPL_free(displs);
            MPL_free(recvcnts);
        } else {
            mpi_errno = MVP_Allgather_function(
                MPI_IN_PLACE, (recvcnt * local_size), recvtype, recvbuf,
                (recvcnt * local_size), recvtype, leader_commptr, errflag);
        }

        MPIR_ERR_CHECK(mpi_errno);
    }

    /*Bcast the entire data from node leaders to all other cores*/
    mpi_errno = MPIR_Bcast_impl(recvbuf, recvcnt * size, recvtype, 0,
                                shmem_commptr, errflag);
    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    MPIR_TIMER_END(coll, allgather, 2lvl);
    return (mpi_errno);

fn_fail:
    goto fn_exit;
}
/* end:nested */

/* This implements an allgather via a gather to leader processes,
 * an allgather across leaders, and a broadcast from each leader.
 * First all procs on a node execute a gather to the leader process.
 * Then the leaders exchange data via allgather/allgatherv.
 * Finally, the leaders broadcast the data to all procs on the node.
 * In this version, ranks do not need to be ordered, because the
 * leader will unpack data from a temporary buffer to the receive
 * buffer in the correct order before the broadcast. */
int MPIR_2lvl_Allgather_nonblocked_MVP(const void *sendbuf, int sendcnt,
                                       MPI_Datatype sendtype, void *recvbuf,
                                       int recvcnt, MPI_Datatype recvtype,
                                       MPIR_Comm *comm_ptr,
                                       MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, allgather, 2lvl_nonblocked);
    int i;
    int mpi_errno = MPI_SUCCESS;

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_allgather_2lvl_nonblocked, 1);

    if (recvcnt == 0) {
        return MPI_SUCCESS;
    }

    /* get our rank and the size of this communicator */
    int rank = comm_ptr->rank;
    int size = comm_ptr->local_size;
    int *node_sizes = comm_ptr->dev.ch.node_sizes;

    /* get extent of receive type */
    MPI_Aint recvtype_extent;
    MPIR_Datatype_get_extent_macro(recvtype, recvtype_extent);

    /* get true extent of recvtype */
    MPI_Aint recvtype_true_lb, recvtype_true_extent;
    MPIR_Type_get_true_extent_impl(recvtype, &recvtype_true_lb,
                                   &recvtype_true_extent);

    /* get info about communicator for ranks on the same node */
    MPIR_Comm *shmem_commptr;
    MPI_Comm shmem_comm = comm_ptr->dev.ch.shmem_comm;
    MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
    int local_rank = shmem_commptr->rank;
    int local_size = shmem_commptr->local_size;

    /* TODO: if ranks are in order, we can avoid the temp buffer and memcpy */
    int need_temp = 1;

    /* get info about communicator across node leaders, allocate temp buffer */
    MPIR_Comm *leader_commptr = NULL;
    int leader_rank = -1;
    int leader_size = 0;
    void *tmpbuf = recvbuf;
    if (local_rank == 0) {
        /* Node leader. Extract the rank, size information for the leader
         * communicator */
        MPI_Comm leader_comm = comm_ptr->dev.ch.leader_comm;
        MPIR_Comm_get_ptr(leader_comm, leader_commptr);
        leader_rank = leader_commptr->rank;
        leader_size = leader_commptr->local_size;

        /* allocate a temporary buffer */
        if (need_temp) {
            tmpbuf = MPL_malloc(size * recvcnt * recvtype_extent, MPL_MEM_COLL);
            if (!tmpbuf) {
                mpi_errno = MPIR_Err_create_code(
                    MPI_SUCCESS, MPIR_ERR_RECOVERABLE, __func__, __LINE__,
                    MPI_ERR_OTHER, "**nomem", 0);
                return mpi_errno;
            }

            /* adjust for potential negative lower bound in datatype */
            tmpbuf = (void *)((char *)tmpbuf - recvtype_true_lb);
        }
    }

    /* ----------------------------------------------
     * Gather data from procs on same node to leader
     * ---------------------------------------------- */

    /* gather data to leaders on each node */
    if (local_rank == 0) {
        /* compute number of items to receive ahead of our spot in the buffer */
        MPI_Aint preceding_count = 0;
        for (i = 0; i < leader_rank; i++) {
            preceding_count += node_sizes[i] * recvcnt;
        }

        /* compute location to receive data from procs on our node */
        void *rbuf =
            (void *)((char *)tmpbuf + (preceding_count * recvtype_extent));

        /* gather data from procs on our node */
        if (sendbuf == MPI_IN_PLACE) {
            /* data is in our receive buffer indexed by our rank */
            void *sbuf =
                (void *)((char *)recvbuf + (rank * recvcnt * recvtype_extent));
            mpi_errno = MPIR_Gather_impl(sbuf, recvcnt, recvtype, rbuf, recvcnt,
                                         recvtype, 0, shmem_commptr, errflag);
        } else {
            mpi_errno =
                MPIR_Gather_impl(sendbuf, sendcnt, sendtype, rbuf, recvcnt,
                                 recvtype, 0, shmem_commptr, errflag);
        }
    } else {
        /* send data to leader on our node */
        if (sendbuf == MPI_IN_PLACE) {
            /* data is in our receive buffer indexed by our rank */
            void *sbuf =
                (void *)((char *)recvbuf + (rank * recvcnt * recvtype_extent));
            mpi_errno = MPIR_Gather_impl(sbuf, recvcnt, recvtype, NULL, recvcnt,
                                         recvtype, 0, shmem_commptr, errflag);
        } else {
            mpi_errno =
                MPIR_Gather_impl(sendbuf, sendcnt, sendtype, NULL, recvcnt,
                                 recvtype, 0, shmem_commptr, errflag);
        }
    }

    MPIR_ERR_CHECK(mpi_errno);

    /* ----------------------------------------------
     * Execute allgather across leaders
     * ---------------------------------------------- */

    /*If there is just one node, after gather itself,
     * root has all the data and it can do bcast*/

    /* Exchange the data between the node leaders */
    if (local_rank == 0 && (leader_size > 1)) {
        /* When data in each socket is different */
        if (comm_ptr->dev.ch.is_uniform != 1) {
            /* allocate memory for counts and displacements arrays */
            int *displs = MPL_malloc(sizeof(int) * leader_size, MPL_MEM_COLL);
            int *counts = MPL_malloc(sizeof(int) * leader_size, MPL_MEM_COLL);
            if (!displs || !counts) {
                mpi_errno = MPIR_Err_create_code(
                    MPI_SUCCESS, MPIR_ERR_RECOVERABLE, __func__, __LINE__,
                    MPI_ERR_OTHER, "**nomem", 0);
                return mpi_errno;
            }

            /* set values in our counts and displacements arrays */
            displs[0] = 0;
            counts[0] = node_sizes[0] * recvcnt;
            for (i = 1; i < leader_size; i++) {
                displs[i] = displs[i - 1] + node_sizes[i - 1] * recvcnt;
                counts[i] = node_sizes[i] * recvcnt;
            }

            /* execute allgatherv across leader ranks */
            mpi_errno = MPIR_Allgatherv_impl(
                MPI_IN_PLACE, (recvcnt * local_size), recvtype, tmpbuf, counts,
                displs, recvtype, leader_commptr, errflag);

            /* free counts and displacements arrays */
            MPL_free(displs);
            MPL_free(counts);
        } else {
            /* execute allgather across leader ranks */
            mpi_errno = MPIR_Allgather_impl(
                MPI_IN_PLACE, (recvcnt * local_size), recvtype, tmpbuf,
                (recvcnt * local_size), recvtype, leader_commptr, errflag);
        }

        MPIR_ERR_CHECK(mpi_errno);
    }

    /* ----------------------------------------------
     * Unpack data into receive buffer in correct order
     * ---------------------------------------------- */

    /* ----------------------------------------------
     * TODO: can avoid temp buffer and mem copy when in block
     * ---------------------------------------------- */
    /* ----------------------------------------------
     * TODO: faster memcpy available?
     * ---------------------------------------------- */

    /* leaders copy data from temp buffer to receive buffer in correct order */
    if (local_rank == 0 && need_temp) {
        /* point to start of temp buffer */
        char *sbuf = (char *)tmpbuf;

        /* copy data for each rank from temp buffer to receive buffer */
        for (i = 0; i < size; i++) {
            /* get next rank in list */
            int dstrank = comm_ptr->dev.ch.rank_list[i];

            /* compute position in receive buffer for this rank */
            void *rbuf =
                (void *)((char *)recvbuf + dstrank * recvcnt * recvtype_extent);

            /* copy data to its correct place */
            mpi_errno = MPIR_Localcopy(sbuf, recvcnt, recvtype, rbuf, recvcnt,
                                       recvtype);
            MPIR_ERR_CHECK(mpi_errno);

            /* update pointer to next spot in temp buffer */
            sbuf += recvcnt * recvtype_extent;
        }

        /* free the temporary buffer if we allocated one */
        tmpbuf = (void *)((char *)tmpbuf + recvtype_true_lb);
        MPL_free(tmpbuf);
    }

    /* ----------------------------------------------
     * Broadcast receive buffer from leader to all procs on the node
     * ---------------------------------------------- */

    /* Bcast the entire data from node leaders to other ranks on node */
    mpi_errno = MPIR_Bcast_impl(recvbuf, recvcnt * size, recvtype, 0,
                                shmem_commptr, errflag);
    MPIR_ERR_CHECK(mpi_errno);

fn_fail:
    MPIR_TIMER_END(coll, allgather, 2lvl_nonblocked);
    return (mpi_errno);
}

/* Execute an allgather by forwarding data through a ring of
 * processes.  This implementation uses the two-level data
 * structures to account for how procs are assigned to nodes
 * to ensure data is only sent into and out of each node once. */
int MPIR_2lvl_Allgather_Ring_nonblocked_MVP(const void *sendbuf, int sendcount,
                                            MPI_Datatype sendtype,
                                            void *recvbuf, int recvcount,
                                            MPI_Datatype recvtype,
                                            MPIR_Comm *comm_ptr,
                                            MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, allgather, 2lvl_ring_nonblocked);
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    int i;

    /* get our rank and the size of this communicator */
    int rank = comm_ptr->rank;
    int size = comm_ptr->local_size;

    /* get extent of receive type */
    MPI_Aint recvtype_extent;
    MPIR_Datatype_get_extent_macro(recvtype, recvtype_extent);

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_allgather_2lvl_ring_nonblocked, 1);

    /* First, load the "local" version in the recvbuf. */
    if (sendbuf != MPI_IN_PLACE) {
        /* compute location in receive buffer for our data */
        void *rbuf =
            (void *)((char *)recvbuf + rank * recvcount * recvtype_extent);

        /* copy data from send buffer to receive buffer */
        mpi_errno = MPIR_Localcopy(sendbuf, sendcount, sendtype, rbuf,
                                   recvcount, recvtype);
        MPIR_ERR_CHECK(mpi_errno);
    }

    /* Now, send left to right. */

    /* lookup our index in the rank list */
    int rank_index = comm_ptr->dev.ch.rank_list_index;

    /* compute the left and right neighbor ranks in the rank_list */
    int left_index = (size + rank_index - 1) % size;
    int right_index = (size + rank_index + 1) % size;
    int left = comm_ptr->dev.ch.rank_list[left_index];
    int right = comm_ptr->dev.ch.rank_list[right_index];

    /* execute ring exchange, start by sending our own data to the right
     * and receiving the data from the rank to our left */
    int send_index = rank_index;
    int recv_index = left_index;
    for (i = 1; i < size; i++) {
        /* compute ranks whose data we'll send and receive in this step */
        int send_rank = comm_ptr->dev.ch.rank_list[send_index];
        int recv_rank = comm_ptr->dev.ch.rank_list[recv_index];

        /* compute position within buffer to send from and receive into */
        void *sbuf =
            (void *)((char *)recvbuf + send_rank * recvcount * recvtype_extent);
        void *rbuf =
            (void *)((char *)recvbuf + recv_rank * recvcount * recvtype_extent);

        /* exchange data with our neighbors in the ring */
        MPIR_PVAR_INC(allgather, 2lvl_ring_nonblocked, send, recvcount,
                      recvtype);
        MPIR_PVAR_INC(allgather, 2lvl_ring_nonblocked, recv, recvcount,
                      recvtype);
        mpi_errno =
            MPIC_Sendrecv(sbuf, recvcount, recvtype, right, MPIR_ALLGATHER_TAG,
                          rbuf, recvcount, recvtype, left, MPIR_ALLGATHER_TAG,
                          comm_ptr, MPI_STATUS_IGNORE, errflag);
        if (mpi_errno) {
            /* for communication errors, just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }

        /* update index values to account for data we just received */
        send_index = recv_index;
        recv_index = (size + recv_index - 1) % size;
    }

fn_fail:
    MPIR_TIMER_END(coll, allgather, 2lvl_ring_nonblocked);
    return (mpi_errno);
}

/* In this implementation, we "gather" data from all procs on a node
 * to their leader.  This is done with direct send/recv and we write
 * the data directly to the receive buffer on the leader process.
 * The leaders then execute an "allgather" by directly sending each
 * of these messages. Finally, we broadcast the final receive buffer
 * to the procs on the node. */
int MPIR_2lvl_Allgather_Direct_MVP(const void *sendbuf, int sendcnt,
                                   MPI_Datatype sendtype, void *recvbuf,
                                   int recvcnt, MPI_Datatype recvtype,
                                   MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, allgather, 2lvl_direct);
    int i, j;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_allgather_2lvl_direct, 1);

    if (recvcnt == 0) {
        return MPI_SUCCESS;
    }

    MPIR_CHKLMEM_DECL(2);

    /* get our rank and the size of this communicator */
    int rank = comm_ptr->rank;
    int size = comm_ptr->local_size;

    /* get extent of receive type */
    MPI_Aint recvtype_extent;
    MPIR_Datatype_get_extent_macro(recvtype, recvtype_extent);

    /* get info about communicator for ranks on the same node */
    MPIR_Comm *shmem_commptr;
    MPI_Comm shmem_comm = comm_ptr->dev.ch.shmem_comm;
    MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
    int local_rank = shmem_commptr->rank;
    int local_size = shmem_commptr->local_size;

    /* get info about communicator across node leaders, allocate temp buffer */
    MPIR_Comm *leader_commptr = NULL;
    int leader_rank = -1;
    int leader_size = 0;
    if (local_rank == 0) {
        /* Node leader. Extract the rank, size information for the leader
         * communicator */
        MPI_Comm leader_comm = comm_ptr->dev.ch.leader_comm;
        MPIR_Comm_get_ptr(leader_comm, leader_commptr);
        leader_rank = leader_commptr->rank;
        leader_size = leader_commptr->local_size;
    }

    int gather_msgs = 0;
    int allgather_msgs = 0;

    /* request/status object allocation is different for
     * leader vs. non-leader */
    if (local_rank == 0) {
        gather_msgs = local_size - 1;
        allgather_msgs = (local_size * (leader_size - 1)) + (size - local_size);
    } else {
        /* if non-leader only send one msg in gather step */
        gather_msgs = 1;
    }

    /* now get the max of these two because the gather
     * completes before the allgather to save space */
    int max = allgather_msgs;
    if (gather_msgs > max) {
        max = gather_msgs;
    }

    /* allocate memory for request objects */
    MPIR_Request **reqarray = NULL;
    MPIR_CHKLMEM_MALLOC(reqarray, MPIR_Request **, max * sizeof(MPIR_Request *),
                        mpi_errno, "reqarray", MPL_MEM_COLL);

    /* allocate memory for status objects */
    MPI_Status *starray = NULL;
    MPIR_CHKLMEM_MALLOC(starray, MPI_Status *, max * sizeof(MPI_Status),
                        mpi_errno, "starray", MPL_MEM_COLL);

    /****************************
     * Gather data to leaders using direct send/recv
     ****************************/

    /* track number of requests */
    int reqs = 0;

    /* gather data to leaders on each node */
    int rank_index = comm_ptr->dev.ch.rank_list_index;
    if (local_rank == 0) {
        /* post receives & point i at leader rank in the rank_list */
        for (i = 1; i < local_size; i++) {
            /* get global rank of incoming data */
            int dstrank = comm_ptr->dev.ch.rank_list[rank_index + i];

            /* compute pointer in receive buffer
             * for incoming data from this rank */
            void *rbuf =
                (void *)((char *)recvbuf + dstrank * recvcnt * recvtype_extent);

            /* post receive for data from this rank */
            MPIR_PVAR_INC(allgather, 2lvl_direct, recv, recvcnt, recvtype);
            mpi_errno =
                MPIC_Irecv(rbuf, recvcnt, recvtype, i, MPIR_ALLGATHER_TAG,
                           shmem_commptr, &reqarray[reqs++]);

            if (mpi_errno) {
                /* for communication errors, just record the error but continue
                 */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
        }

        /* copy our data to our receive buffer if needed */
        if (sendbuf != MPI_IN_PLACE) {
            /* compute location in receive buffer for our data */
            void *rbuf =
                (void *)((char *)recvbuf + rank * recvcnt * recvtype_extent);
            mpi_errno = MPIR_Localcopy(sendbuf, sendcnt, sendtype, rbuf,
                                       recvcnt, recvtype);
            MPIR_ERR_CHECK(mpi_errno);
        }
    } else {
        /* get parameters for sending data */
        const void *sbuf = sendbuf;
        int scnt = sendcnt;
        MPI_Datatype stype = sendtype;
        if (sendbuf == MPI_IN_PLACE) {
            /* use receive params if IN_PLACE */
            sbuf = (void *)((char *)recvbuf + rank * recvcnt * recvtype_extent);
            scnt = recvcnt;
            stype = recvtype;
        }

        /* send data to the leader process */
        MPIR_PVAR_INC(allgather, 2lvl_direct, send, scnt, stype);
        mpi_errno = MPIC_Isend(sbuf, scnt, stype, 0, MPIR_ALLGATHER_TAG,
                               shmem_commptr, &reqarray[reqs++], errflag);
        if (mpi_errno) {
            /* for communication errors, just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
    }

    /* wait for all outstanding requests to complete */
    mpi_errno = MPIC_Waitall(reqs, reqarray, starray, errflag);
    MPIR_ERR_CHECK(mpi_errno);

    /* --BEGIN ERROR HANDLING-- */
    if (mpi_errno == MPI_ERR_IN_STATUS) {
        for (i = 0; i < reqs; i++) {
            if (starray[i].MPI_ERROR != MPI_SUCCESS) {
                mpi_errno = starray[i].MPI_ERROR;
                if (mpi_errno) {
                    /* for communication errors,
                     * just record the error but continue */
                    *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                    MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                }
            }
        }
    }

    /****************************
     * Execute direct-send allgather across leaders
     ****************************/

    /*If there is just one node, after gather itself,
     * root has all the data and it can do bcast*/

    /* Exchange the data between the node leaders */
    if (local_rank == 0) {
        /* initialize the active request counter */
        reqs = 0;

        /* post receives */
        for (i = 1; i < leader_size; i++) {
            /* compute source rank sending to us in this step */
            int src = leader_rank - i;
            if (src < 0) {
                src += leader_size;
            }

            /* receive all messages from each rank on src node */
            int recv_count = comm_ptr->dev.ch.node_sizes[src];
            int recv_displ = comm_ptr->dev.ch.node_disps[src];
            for (j = 0; j < recv_count; j++) {
                /* get rank of incoming data */
                int srcrank = comm_ptr->dev.ch.rank_list[recv_displ + j];

                /* get pointer to receive buffer for this rank */
                void *rbuf = (void *)((char *)recvbuf +
                                      srcrank * recvcnt * recvtype_extent);

                /* post receive */
                MPIR_PVAR_INC(allgather, 2lvl_direct, recv, recvcnt, recvtype);
                mpi_errno =
                    MPIC_Irecv(rbuf, recvcnt, recvtype, src, MPIR_ALLGATHER_TAG,
                               leader_commptr, &reqarray[reqs++]);
                if (mpi_errno) {
                    /* for communication errors,
                     * just record the error but continue */
                    *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                    MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                }
            }
        }

        /* post sends */
        for (i = 1; i < leader_size; i++) {
            /* compute destination rank for this step */
            int dst = leader_rank + i;
            if (dst >= leader_size) {
                dst -= leader_size;
            }

            /* send all messages for this leader to each other leader rank */
            int send_count = comm_ptr->dev.ch.node_sizes[leader_rank];
            for (j = 0; j < send_count; j++) {
                /* get source rank for data we'll send */
                int dstrank = comm_ptr->dev.ch.rank_list[rank_index + j];

                /* get pointer into buffer for this rank */
                void *sbuf = (void *)((char *)recvbuf +
                                      dstrank * recvcnt * recvtype_extent);

                /* post send to this destination rank */
                MPIR_PVAR_INC(allgather, 2lvl_direct, send, sendcnt, sendtype);
                mpi_errno =
                    MPIC_Isend(sbuf, sendcnt, sendtype, dst, MPIR_ALLGATHER_TAG,
                               leader_commptr, &reqarray[reqs++], errflag);

                if (mpi_errno) {
                    /* for communication errors,
                     * just record the error but continue */
                    *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                    MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                }
            }
        }

        /* wait for all outstanding requests to complete */
        mpi_errno = MPIC_Waitall(reqs, reqarray, starray, errflag);
        MPIR_ERR_CHECK(mpi_errno);

        /* --BEGIN ERROR HANDLING-- */
        if (mpi_errno == MPI_ERR_IN_STATUS) {
            for (i = 0; i < reqs; i++) {
                if (starray[i].MPI_ERROR != MPI_SUCCESS) {
                    mpi_errno = starray[i].MPI_ERROR;
                    if (mpi_errno) {
                        /* for communication errors,
                         * just record the error but continue */
                        *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                        MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                    }
                }
            }
        }
    }

    /****************************
     * Broadcast data from leaders to procs on the node
     ****************************/

    /* Bcast the entire data from node leaders to other ranks on node */
    mpi_errno = MPIR_Bcast_impl(recvbuf, recvcnt * size, recvtype, 0,
                                shmem_commptr, errflag);
    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    MPIR_CHKLMEM_FREEALL();
    return (mpi_errno);

fn_fail:
    MPIR_TIMER_END(coll, allgather, 2lvl_direct);
    goto fn_exit;
}

/* In this implementation, we "gather" data from all procs on a node
 * to their leader.  This is done with direct send/recv and we write
 * the data directly to the receive buffer on the leader process.
 * The leaders then execute an "allgather" by directly sending each
 * of these messages.  We use a ring algorithm to forward data through
 * leaders.  Finally, we broadcast the final receive buffer to the
 * procs on the node. */
int MPIR_2lvl_Allgather_Ring_MVP(const void *sendbuf, int sendcnt,
                                 MPI_Datatype sendtype, void *recvbuf,
                                 int recvcnt, MPI_Datatype recvtype,
                                 MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, allgather, 2lvl_ring);
    int i, j;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_allgather_2lvl_ring, 1);

    if (recvcnt == 0) {
        return MPI_SUCCESS;
    }

    MPIR_CHKLMEM_DECL(2);

    /* get our rank and the size of this communicator */
    int rank = comm_ptr->rank;
    int size = comm_ptr->local_size;

    /* get extent of receive type */
    MPI_Aint recvtype_extent;
    MPIR_Datatype_get_extent_macro(recvtype, recvtype_extent);

    /* get info about communicator for ranks on the same node */
    MPIR_Comm *shmem_commptr;
    MPI_Comm shmem_comm = comm_ptr->dev.ch.shmem_comm;
    MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
    int local_rank = shmem_commptr->rank;
    int local_size = shmem_commptr->local_size;

    /* get info about communicator across node leaders */
    MPIR_Comm *leader_commptr = NULL;
    int leader_rank = -1;
    int leader_size = 0;
    if (local_rank == 0) {
        /* Node leader. Extract the rank, size information for the leader
         * communicator */
        MPI_Comm leader_comm = comm_ptr->dev.ch.leader_comm;
        MPIR_Comm_get_ptr(leader_comm, leader_commptr);
        leader_rank = leader_commptr->rank;
        leader_size = leader_commptr->local_size;
    }

    int gather_msgs = 0;
    int allgather_msgs = 0;
    int max_local_size = 0;
    int *node_sizes = comm_ptr->dev.ch.node_sizes;

    /* request/status object allocation is different for
     * leader vs. non-leader */
    if (local_rank == 0) {
        gather_msgs = local_size - 1;
        for (i = 0; i < leader_size; i++) {
            if (node_sizes[i] > max_local_size) {
                max_local_size = node_sizes[i];
            }
        }
        allgather_msgs = 2 * max_local_size;
    } else {
        /* if non-leader only send one msg in gather step */
        gather_msgs = 1;
    }

    /* now get the max of these two because the gather
     * completes before the allgather to save space */
    int max = allgather_msgs;
    if (gather_msgs > max) {
        max = gather_msgs;
    }

    /* allocate array of request objects */
    MPIR_Request **reqarray = NULL;
    MPIR_CHKLMEM_MALLOC(reqarray, MPIR_Request **, max * sizeof(MPIR_Request *),
                        mpi_errno, "reqarray", MPL_MEM_COLL);

    /* allocate array of status objects */
    MPI_Status *starray = NULL;
    MPIR_CHKLMEM_MALLOC(starray, MPI_Status *, max * sizeof(MPI_Status),
                        mpi_errno, "starray", MPL_MEM_COLL);

    /****************************
     * Gather data to leaders using direct send/recv
     ****************************/

    int reqs = 0;

    /* gather data to leaders on each node */
    int rank_index = comm_ptr->dev.ch.rank_list_index;
    if (local_rank == 0) {
        /* post receives for incoming data from procs on our node */
        for (i = 1; i < local_size; i++) {
            /* get global rank of this process */
            int srcrank = comm_ptr->dev.ch.rank_list[rank_index + i];

            /* compute pointer in receive buffer
             * for incoming data from this rank */
            void *rbuf =
                (void *)((char *)recvbuf + srcrank * recvcnt * recvtype_extent);

            /* post receive for data from this rank on shared mem comm */
            MPIR_PVAR_INC(allgather, 2lvl_ring, recv, recvcnt, recvtype);
            mpi_errno =
                MPIC_Irecv(rbuf, recvcnt, recvtype, i, MPIR_ALLGATHER_TAG,
                           shmem_commptr, &reqarray[reqs++]);
            if (mpi_errno) {
                /* for communication errors, just record the error but continue
                 */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
        }

        /* copy our data to our receive buffer if needed */
        if (sendbuf != MPI_IN_PLACE) {
            /* compute location in receive buffer for our data */
            void *rbuf =
                (void *)((char *)recvbuf + rank * recvcnt * recvtype_extent);
            mpi_errno = MPIR_Localcopy(sendbuf, sendcnt, sendtype, rbuf,
                                       recvcnt, recvtype);
            MPIR_ERR_CHECK(mpi_errno);
        }
    } else {
        /* get parameters for sending data */
        const void *sbuf = sendbuf;
        int scnt = sendcnt;
        MPI_Datatype stype = sendtype;
        if (sendbuf == MPI_IN_PLACE) {
            /* use receive params if IN_PLACE */
            sbuf = (void *)((char *)recvbuf + rank * recvcnt * recvtype_extent);
            scnt = recvcnt;
            stype = recvtype;
        }

        /* send data to leader of our node */
        MPIR_PVAR_INC(allgather, 2lvl_ring, send, scnt, stype);
        mpi_errno = MPIC_Isend(sbuf, scnt, stype, 0, MPIR_ALLGATHER_TAG,
                               shmem_commptr, &reqarray[reqs++], errflag);
        if (mpi_errno) {
            /* for communication errors, just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
    }

    /* wait for all outstanding requests to complete */
    mpi_errno = MPIC_Waitall(reqs, reqarray, starray, errflag);
    MPIR_ERR_CHECK(mpi_errno);

    /* --BEGIN ERROR HANDLING-- */
    if (mpi_errno == MPI_ERR_IN_STATUS) {
        for (i = 0; i < reqs; i++) {
            if (starray[i].MPI_ERROR != MPI_SUCCESS) {
                mpi_errno = starray[i].MPI_ERROR;
                if (mpi_errno) {
                    /* for communication errors,
                     * just record the error but continue */
                    *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                    MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                }
            }
        }
    }

    MPIR_ERR_CHECK(mpi_errno);

    /****************************
     * Execute ring-based allgather across leaders
     ****************************/

    /* Exchange the data between the node leaders */
    if (local_rank == 0 && leader_size > 1) {
        /* get our left and right ranks in our leader comm */
        int left = (leader_size + leader_rank - 1) % leader_size;
        int right = (leader_size + leader_rank + 1) % leader_size;

        /* start by sending our own data and receiving data
         * from the leader to our left */
        int send_index = leader_rank;
        int recv_index = left;
        for (i = 0; i < leader_size; i++) {
            /* initialize our request counter */
            reqs = 0;

            /* post receives for data coming from the left */
            int recv_count = comm_ptr->dev.ch.node_sizes[recv_index];
            int recv_displ = comm_ptr->dev.ch.node_disps[recv_index];
            for (j = 0; j < recv_count; j++) {
                /* get source rank for this message */
                int srcrank = comm_ptr->dev.ch.rank_list[recv_displ + j];

                /* compute pointer in receive buffer
                 * for incoming data from this rank */
                void *rbuf = (void *)((char *)recvbuf +
                                      srcrank * recvcnt * recvtype_extent);

                /* post receive for data from this rank */
                MPIR_PVAR_INC(allgather, 2lvl_ring, recv, recvcnt, recvtype);
                mpi_errno = MPIC_Irecv(rbuf, recvcnt, recvtype, left,
                                       MPIR_ALLGATHER_TAG, leader_commptr,
                                       &reqarray[reqs++]);
                if (mpi_errno) {
                    /* for communication errors,
                     * just record the error but continue */
                    *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                    MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                }
            }

            /* TODO: consider placing a barrier here to ensure
             * receives are posted before sends, especially for large messages
             */
            // MPIR_Barrier_impl(comm_ptr);

            /* post sends for data we're sending to the right */
            int send_count = comm_ptr->dev.ch.node_sizes[send_index];
            int send_displ = comm_ptr->dev.ch.node_disps[send_index];
            for (j = 0; j < send_count; j++) {
                /* get source rank for this message */
                int dstrank = comm_ptr->dev.ch.rank_list[send_displ + j];

                /* compute pointer in receive buffer
                 * for outgoing data from this rank */
                void *sbuf = (void *)((char *)recvbuf +
                                      dstrank * recvcnt * recvtype_extent);

                /* post send for data from this rank */
                MPIR_PVAR_INC(allgather, 2lvl_ring, send, recvcnt, recvtype);
                mpi_errno = MPIC_Isend(sbuf, recvcnt, recvtype, right,
                                       MPIR_ALLGATHER_TAG, leader_commptr,
                                       &reqarray[reqs++], errflag);
                if (mpi_errno) {
                    /* for communication errors,
                     * just record the error but continue */
                    *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                    MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                }
            }

            /* wait for all outstanding requests to complete */
            mpi_errno = MPIC_Waitall(reqs, reqarray, starray, errflag);
            MPIR_ERR_CHECK(mpi_errno);

            /* --BEGIN ERROR HANDLING-- */
            if (mpi_errno == MPI_ERR_IN_STATUS) {
                for (i = 0; i < reqs; i++) {
                    if (starray[i].MPI_ERROR != MPI_SUCCESS) {
                        mpi_errno = starray[i].MPI_ERROR;
                        if (mpi_errno) {
                            /* for communication errors,
                             * just record the error but continue */
                            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                        }
                    }
                }
            }

            /* update index values so in the next step, we send the data
             * that we just received */
            send_index = recv_index;
            recv_index = (leader_size + recv_index - 1) % leader_size;
        }
    }

    /****************************
     * Broadcast data from leaders to procs on the node
     ****************************/

    /* Bcast the entire data from node leaders to other ranks on node */
    mpi_errno = MPIR_Bcast_impl(recvbuf, recvcnt * size, recvtype, 0,
                                shmem_commptr, errflag);
    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    MPIR_CHKLMEM_FREEALL();
    return (mpi_errno);

fn_fail:
    MPIR_TIMER_END(coll, allgather, 2lvl_ring);
    goto fn_exit;
}
