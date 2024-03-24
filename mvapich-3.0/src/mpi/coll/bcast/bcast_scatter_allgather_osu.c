#include "bcast_tuning.h"

/* FIXME it would be nice if we could refactor things to minimize
 * duplication between this and MPIR_Scatter_intra and friends.  We can't use
 * MPIR_Scatter_intra as is without inducing an extra copy
 * in the noncontaig case. */
/* There are additional arguments included here that are unused because we
 * always assume that the noncontig case has been packed into a contig case by
 * the caller for now.  Once we start handling noncontig data at the upper level
 * we can start handling it here.
 * At the moment this function always scatters a buffer of nbytes starting at
 * tmp_buf address. */
static int scatter_for_bcast_MVP(void *buffer ATTRIBUTE((unused)),
                                 int count ATTRIBUTE((unused)),
                                 MPI_Datatype datatype ATTRIBUTE((unused)),
                                 int root, MPIR_Comm *comm_ptr, intptr_t nbytes,
                                 void *tmp_buf, int is_contig,
                                 int is_homogeneous, MPIR_Errflag_t *errflag)
{
    MPI_Status status;
    int rank, comm_size, src, dst;
    int relative_rank, mask;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    intptr_t scatter_size = 0, recv_size = 0, send_size = 0;
    // intptr_t curr_size = 0;
    MPI_Count byte_count, curr_size = 0;

    comm_size = comm_ptr->local_size;
    rank = comm_ptr->rank;
    relative_rank = (rank >= root) ? rank - root : rank - root + comm_size;

    /* use long message algorithm: binomial tree scatter followed by an
     * allgather */

    /* The scatter algorithm divides the buffer into nprocs pieces and
       scatters them among the processes. Root gets the first piece,
       root+1 gets the second piece, and so forth. Uses the same binomial
       tree algorithm as above. Ceiling division
       is used to compute the size of each piece. This means some
       processes may not get any data. For example if bufsize = 97 and
       nprocs = 16, ranks 15 and 16 will get 0 data. On each process, the
       scattered data is stored at the same offset in the buffer as it is
       on the root process. */

    scatter_size = (nbytes + comm_size - 1) / comm_size; /* ceiling division */
    curr_size = (rank == root) ? nbytes : 0; /* root starts with all the
                                                   data */

    mask = 0x1;
    while (mask < comm_size) {
        if (relative_rank & mask) {
            src = rank - mask;
            if (src < 0)
                src += comm_size;
            recv_size = nbytes - relative_rank * scatter_size;
            /* recv_size is larger than what might actually be sent by the
               sender. We don't need compute the exact value because MPI
               allows you to post a larger recv. */
            if (recv_size <= 0) {
                curr_size = 0; /* this process doesn't receive any data
                                  because of uneven division */
            } else {
                MPIR_PVAR_INC(bcast, scatter_for_bcast, recv, recv_size,
                              MPI_BYTE);
                mpi_errno = MPIC_Recv(
                    ((char *)tmp_buf + relative_rank * scatter_size), recv_size,
                    MPI_BYTE, src, MPIR_BCAST_TAG, comm_ptr, &status, errflag);
                if (mpi_errno) {
                    /* for communication errors,
                     * just record the error but continue */
                    *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                    MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                    curr_size = 0;
                } else
                    /* query actual size of data received */
                    byte_count = MPIR_STATUS_GET_COUNT(status);
                MPIR_Get_elements_x_impl(&byte_count, MPI_BYTE, &curr_size);
            }
            break;
        }
        mask <<= 1;
    }

    /* This process is responsible for all processes that have bits
       set from the LSB up to (but not including) mask.  Because of
       the "not including", we start by shifting mask back down
       one. */

    mask >>= 1;
    while (mask > 0) {
        if (relative_rank + mask < comm_size) {
            send_size = curr_size - scatter_size * mask;
            /* mask is also the size of this process's subtree */

            if (send_size > 0) {
                dst = rank + mask;
                if (dst >= comm_size)
                    dst -= comm_size;
                MPIR_PVAR_INC(bcast, scatter_for_bcast, send, send_size,
                              MPI_BYTE);
                mpi_errno = MPIC_Send(
                    ((char *)tmp_buf + scatter_size * (relative_rank + mask)),
                    send_size, MPI_BYTE, dst, MPIR_BCAST_TAG, comm_ptr,
                    errflag);
                if (mpi_errno) {
                    /* for communication errors, just record the error but
                     * continue */
                    *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                    MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                }
                curr_size -= send_size;
            }
        }
        mask >>= 1;
    }

    if (mpi_errno_ret)
        mpi_errno = mpi_errno_ret;
    else if (*errflag)
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**coll_fail");
    return mpi_errno;
}

/*
   Broadcast based on a scatter followed by an allgather.

   We first scatter the buffer using a binomial tree algorithm. This costs
   lgp.alpha + n.((p-1)/p).beta
   If the datatype is contiguous and the communicator is homogeneous,
   we treat the data as bytes and divide (scatter) it among processes
   by using ceiling division. For the noncontiguous or heterogeneous
   cases, we first pack the data into a temporary buffer by using
   MPI_Pack, scatter it as bytes, and unpack it after the allgather.

   We use a ring algorithm for the allgather, which takes p-1 steps.
   This may perform better than recursive doubling for long messages and
   medium-sized non-power-of-two messages.
   Total Cost = (lgp+p-1).alpha + 2.n.((p-1)/p).beta
*/
int MPIR_Bcast_scatter_ring_allgather_MVP(void *buffer, int count,
                                          MPI_Datatype datatype, int root,
                                          MPIR_Comm *comm_ptr,
                                          MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, bcast, scatter_ring_allgather);
    int rank, comm_size;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    intptr_t nbytes = 0, scatter_size = 0;
    int j, i, is_contig, is_homogeneous;
    MPI_Aint type_size, position;
    intptr_t *recvcnts = NULL, *displs = NULL;
    int left, right, jnext;
    void *tmp_buf;
    MPIR_Datatype *dtp;
    MPI_Aint true_extent, true_lb;
    MPIR_CHKLMEM_DECL(3);

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_bcast_scatter_ring_allgather, 1);
    comm_size = comm_ptr->local_size;
    rank = comm_ptr->rank;

    /* If there is only one process, return */
    if (comm_size == 1)
        goto fn_exit;

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

    nbytes = (intptr_t)(count)*type_size;

    if (is_contig && is_homogeneous) {
        /* contiguous and homogeneous. no need to pack. */
        MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);

        tmp_buf = (char *)buffer + true_lb;
    } else {
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

    scatter_size = (nbytes + comm_size - 1) / comm_size; /* ceiling division */

    mpi_errno =
        scatter_for_bcast_MVP(buffer, count, datatype, root, comm_ptr, nbytes,
                              tmp_buf, is_contig, is_homogeneous, errflag);
    if (mpi_errno) {
        /* for communication errors, just record the error but continue */
        *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
        MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
    }

    /* long-message allgather or medium-size but non-power-of-two. use ring
     * algorithm. */

    MPIR_CHKLMEM_MALLOC(recvcnts, intptr_t *, comm_size * sizeof(intptr_t),
                        mpi_errno, "recvcnts", MPL_MEM_COLL);
    MPIR_CHKLMEM_MALLOC(displs, intptr_t *, comm_size * sizeof(intptr_t),
                        mpi_errno, "displs", MPL_MEM_COLL);

    for (i = 0; i < comm_size; i++) {
        recvcnts[i] = nbytes - i * scatter_size;
        if (recvcnts[i] > scatter_size)
            recvcnts[i] = scatter_size;
        if (recvcnts[i] < 0)
            recvcnts[i] = 0;
    }

    displs[0] = 0;
    for (i = 1; i < comm_size; i++)
        displs[i] = displs[i - 1] + recvcnts[i - 1];

    left = (comm_size + rank - 1) % comm_size;
    right = (rank + 1) % comm_size;

    j = rank;
    jnext = left;
    for (i = 1; i < comm_size; i++) {
        MPIR_PVAR_INC(bcast, scatter_ring_allgather, send,
                      recvcnts[(j - root + comm_size) % comm_size], MPI_BYTE);
        MPIR_PVAR_INC(bcast, scatter_ring_allgather, recv,
                      recvcnts[(jnext - root + comm_size) % comm_size],
                      MPI_BYTE);
        mpi_errno = MPIC_Sendrecv(
            (char *)tmp_buf + displs[(j - root + comm_size) % comm_size],
            recvcnts[(j - root + comm_size) % comm_size], MPI_BYTE, right,
            MPIR_BCAST_TAG,
            (char *)tmp_buf + displs[(jnext - root + comm_size) % comm_size],
            recvcnts[(jnext - root + comm_size) % comm_size], MPI_BYTE, left,
            MPIR_BCAST_TAG, comm_ptr, MPI_STATUS_IGNORE, errflag);
        if (mpi_errno) {
            /* for communication errors, just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }

        j = jnext;
        jnext = (comm_size + jnext - 1) % comm_size;
    }

    if (!is_contig || !is_homogeneous) {
        if (rank != root) {
            position = 0;
            mpi_errno = MPIR_Typerep_unpack(tmp_buf, nbytes, buffer, count,
                                            datatype, position, &position);
            MPIR_ERR_CHECK(mpi_errno);
        }
    }

fn_exit:
    MPIR_CHKLMEM_FREEALL();
    if (mpi_errno_ret)
        mpi_errno = mpi_errno_ret;
    else if (*errflag)
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**coll_fail");
    MPIR_TIMER_END(coll, bcast, scatter_ring_allgather);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

/*
   This function is based on MPIR_Bcast_scatter_ring_allgather_MVP(),
   we overlap shared memory bcast with the allgather phase
*/
int MPIR_Bcast_scatter_ring_allgather_shm_MVP(void *buffer, int count,
                                              MPI_Datatype datatype, int root,
                                              MPIR_Comm *comm_ptr,
                                              MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, bcast, scatter_ring_allgather_shm);
    int rank, comm_size, local_rank;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    intptr_t nbytes = 0, scatter_size = 0;
    int j, i, is_contig = 1, is_homogeneous = 1;
    MPI_Aint type_size;
    int left = -1, right = -1, jnext;
    intptr_t *recvcnts = NULL, *displs = NULL;
    MPI_Aint true_extent, true_lb;
    void *tmp_buf = NULL;
    MPIR_CHKLMEM_DECL(3);

    comm_size = comm_ptr->local_size;
    rank = comm_ptr->rank;
    MPIR_Request *request[2];
    MPI_Status status[2];
    MPI_Comm shmem_comm;
    MPIR_Comm *shmem_commptr = NULL, *leader_commptr = NULL;
    shmem_comm = comm_ptr->dev.ch.shmem_comm;
    MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
    bcast_ring_allgather_shm_packet para_packet;

    intptr_t shmem_offset, shmem_nbytes;
    MPI_Comm leader_comm;
    leader_comm = comm_ptr->dev.ch.leader_comm;
    MPIR_Comm_get_ptr(leader_comm, leader_commptr);

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_bcast_scatter_ring_allgather_shm, 1);
    local_rank = shmem_commptr->rank;
    rank = comm_ptr->rank;
    if (local_rank == 0) {
        comm_size = leader_commptr->local_size;
        rank = leader_commptr->rank;
    }

    MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);
    /* even though we always call this algorithm with contiguous buffer, still,
     * the datatype might have some holes in the beginning. Therefore, true_lb
     * might be non zero */
    tmp_buf = buffer + true_lb;

    MPIR_Bcast_MVP(&comm_size, 1, MPI_INT, 0, shmem_commptr, errflag);
    if (comm_size == 1) {
        goto fn_exit;
    }

    if (local_rank == 0) {
        MPIR_Datatype_get_size_macro(datatype, type_size);
        nbytes = (intptr_t)(count) * (type_size);

        scatter_size = (nbytes + comm_size - 1) / comm_size;
        /* ceiling division */

        /* long-message allgather or medium-size but non-power-of-two. use ring
         * algorithm. */

        MPIR_CHKLMEM_MALLOC(recvcnts, intptr_t *, comm_size * sizeof(intptr_t),
                            mpi_errno, "recvcnts", MPL_MEM_COLL);
        MPIR_CHKLMEM_MALLOC(displs, intptr_t *, comm_size * sizeof(intptr_t),
                            mpi_errno, "displs", MPL_MEM_COLL);

        for (i = 0; i < comm_size; i++) {
            recvcnts[i] = nbytes - i * scatter_size;
            if (recvcnts[i] > scatter_size) {
                recvcnts[i] = scatter_size;
            }
            if (recvcnts[i] < 0) {
                recvcnts[i] = 0;
            }
        }

        displs[0] = 0;
        for (i = 1; i < comm_size; i++) {
            displs[i] = displs[i - 1] + recvcnts[i - 1];
        }
        left = (comm_size + rank - 1) % comm_size;
        right = (rank + 1) % comm_size;

        j = rank;
        jnext = left;

        /* parameters are packed up and broadcasted within the node,
         * therefore a leader pass the parameters to non-leaders
         */
        para_packet.j = j;
        para_packet.jnext = jnext;
        para_packet.root = root;
        para_packet.nbytes = nbytes;
        para_packet.scatter_size = scatter_size;

        MPIR_Bcast_MVP(&para_packet, sizeof(bcast_ring_allgather_shm_packet),
                       MPI_BYTE, 0, shmem_commptr, errflag);

        mpi_errno = scatter_for_bcast_MVP(tmp_buf, count, datatype, root,
                                          leader_commptr, nbytes, tmp_buf,
                                          is_contig, is_homogeneous, errflag);
        if (mpi_errno) {
            /* for communication errors, just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }

        /* one chunk is moving along the allgather ring,
         * node-leaders are involved*/
        MPIR_PVAR_INC(bcast, scatter_ring_allgather_shm, recv,
                      recvcnts[(jnext - root + comm_size) % comm_size],
                      MPI_BYTE);
        MPIC_Irecv((char *)tmp_buf +
                       displs[(jnext - root + comm_size) % comm_size],
                   recvcnts[(jnext - root + comm_size) % comm_size], MPI_BYTE,
                   left, MPIR_BCAST_TAG, leader_commptr, &request[0]);

        MPIR_PVAR_INC(bcast, scatter_ring_allgather_shm, send,
                      recvcnts[(j - root + comm_size) % comm_size], MPI_BYTE);
        MPIC_Isend((char *)tmp_buf + displs[(j - root + comm_size) % comm_size],
                   recvcnts[(j - root + comm_size) % comm_size], MPI_BYTE,
                   right, MPIR_BCAST_TAG, leader_commptr, &request[1], errflag);

        shmem_offset = displs[(j - root + comm_size) % comm_size];
        shmem_nbytes = recvcnts[(j - root + comm_size) % comm_size];

        mpi_errno =
            MPIR_Shmem_Bcast_MVP(tmp_buf + shmem_offset, shmem_nbytes, MPI_BYTE,
                                 INTRA_NODE_ROOT, shmem_commptr, errflag);

        mpi_errno = MPIC_Waitall(2, request, status, errflag);

        MPIR_ERR_CHECK(mpi_errno);

        if (mpi_errno) {
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
        j = jnext;
        jnext = (comm_size + jnext - 1) % comm_size;

        /* Leaders receive other chunks via allgather ring.
         * When a leader is getting
         * ith chunk from another leader,
         * it broadcast (i-1)th chunk to non-leaders
         * inside the node
         */
        for (i = 2; i < comm_size; i++) {
            MPIR_PVAR_INC(bcast, scatter_ring_allgather_shm, recv,
                          recvcnts[(jnext - root + comm_size) % comm_size],
                          MPI_BYTE);
            MPIC_Irecv((char *)tmp_buf +
                           displs[(jnext - root + comm_size) % comm_size],
                       recvcnts[(jnext - root + comm_size) % comm_size],
                       MPI_BYTE, left, MPIR_BCAST_TAG, leader_commptr,
                       &request[0]);

            MPIR_PVAR_INC(bcast, scatter_ring_allgather_shm, send,
                          recvcnts[(j - root + comm_size) % comm_size],
                          MPI_BYTE);
            MPIC_Isend(
                (char *)tmp_buf + displs[(j - root + comm_size) % comm_size],
                recvcnts[(j - root + comm_size) % comm_size], MPI_BYTE, right,
                MPIR_BCAST_TAG, leader_commptr, &request[1], errflag);

            shmem_offset = displs[(j - root + comm_size) % comm_size];
            shmem_nbytes = recvcnts[(j - root + comm_size) % comm_size];

            mpi_errno = MPIR_Shmem_Bcast_MVP(
                tmp_buf + shmem_offset, shmem_nbytes, MPI_BYTE, INTRA_NODE_ROOT,
                shmem_commptr, errflag);

            mpi_errno = MPIC_Waitall(2, request, status, errflag);

            MPIR_ERR_CHECK(mpi_errno);
            if (mpi_errno) {
                /* for communication errors,
                 * just record the error but continue */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }

            j = jnext;
            jnext = (comm_size + jnext - 1) % comm_size;
        }

        shmem_offset = displs[(j - root + comm_size) % comm_size];
        shmem_nbytes = recvcnts[(j - root + comm_size) % comm_size];
    }

    if (local_rank != 0) {
        MPIR_Bcast_MVP(&para_packet, sizeof(bcast_ring_allgather_shm_packet),
                       MPI_BYTE, 0, shmem_commptr, errflag);
        j = para_packet.j;
        jnext = para_packet.jnext;
        root = para_packet.root;
        nbytes = para_packet.nbytes;
        scatter_size = para_packet.scatter_size;
        MPIR_CHKLMEM_MALLOC(recvcnts, intptr_t *, comm_size * sizeof(intptr_t),
                            mpi_errno, "recvcnts", MPL_MEM_COLL);
        MPIR_CHKLMEM_MALLOC(displs, intptr_t *, comm_size * sizeof(intptr_t),
                            mpi_errno, "displs", MPL_MEM_COLL);

        for (i = 0; i < comm_size; i++) {
            recvcnts[i] = nbytes - i * scatter_size;
            if (recvcnts[i] > scatter_size) {
                recvcnts[i] = scatter_size;
            }
            if (recvcnts[i] < 0) {
                recvcnts[i] = 0;
            }
        }

        displs[0] = 0;
        for (i = 1; i < comm_size; i++) {
            displs[i] = displs[i - 1] + recvcnts[i - 1];
        }

        /* Each node-leader has one chunk already in the right place,
         * this chunk doesn't
         * require inter-node communication,
         * we broadcast this chunk to non-leaders in
         * the node
         */
        /* Non-leaders compute offset and count */
        shmem_offset = displs[(j - root + comm_size) % comm_size];
        shmem_nbytes = recvcnts[(j - root + comm_size) % comm_size];

        mpi_errno =
            MPIR_Shmem_Bcast_MVP(tmp_buf + shmem_offset, shmem_nbytes, MPI_BYTE,
                                 INTRA_NODE_ROOT, shmem_commptr, errflag);

        j = jnext;
        jnext = (comm_size + jnext - 1) % comm_size;

        /* Leaders receive other chunks via allgather ring.
         * When a leader is getting
         * ith chunk from another leader,
         * it broadcast (i-1)th chunk to non-leaders
         * inside the node
         */
        for (i = 2; i < comm_size; i++) {
            /* Non-leaders compute offset and count */
            shmem_offset = displs[(j - root + comm_size) % comm_size];
            shmem_nbytes = recvcnts[(j - root + comm_size) % comm_size];

            mpi_errno = MPIR_Shmem_Bcast_MVP(
                tmp_buf + shmem_offset, shmem_nbytes, MPI_BYTE, INTRA_NODE_ROOT,
                shmem_commptr, errflag);

            j = jnext;
            jnext = (comm_size + jnext - 1) % comm_size;
        }

        /* Non-leaders compute offset and count */
        shmem_offset = displs[(j - root + comm_size) % comm_size];
        shmem_nbytes = recvcnts[(j - root + comm_size) % comm_size];
    }

    mpi_errno =
        MPIR_Shmem_Bcast_MVP(tmp_buf + shmem_offset, shmem_nbytes, MPI_BYTE,
                             INTRA_NODE_ROOT, shmem_commptr, errflag);

    /* indicate that we have finished shared-memory bcast */
    comm_ptr->dev.ch.intra_node_done = 1;
    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_num_shmem_coll_calls, 1);

fn_exit:
    MPIR_CHKLMEM_FREEALL();
    if (mpi_errno_ret) {
        mpi_errno = mpi_errno_ret;
    } else if (*errflag) {
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**coll_fail");
    }
    MPIR_TIMER_END(coll, bcast, scatter_ring_allgather_shm);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
