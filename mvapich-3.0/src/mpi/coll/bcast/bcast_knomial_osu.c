#include "bcast_tuning.h"

int MPIR_Bcast_binomial_MVP(void *buffer, int count, MPI_Datatype datatype,
                            int root, MPIR_Comm *comm_ptr,
                            MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, bcast, binomial);

    int rank, comm_size, src, dst;
    int relative_rank, mask;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    intptr_t nbytes = 0;
    int is_contig, is_homogeneous;
    MPI_Aint type_size;
    MPI_Aint position;
    void *tmp_buf = NULL;
    MPIR_Datatype *dtp;
    MPIR_CHKLMEM_DECL(1);

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_bcast_binomial, 1);
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

    nbytes = (intptr_t)(count) * (type_size);

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

    relative_rank = (rank >= root) ? rank - root : rank - root + comm_size;

    /* Use short message algorithm, namely, binomial tree */

    /* Algorithm:
       This uses a fairly basic recursive subdivision algorithm.
       The root sends to the process comm_size/2 away; the receiver becomes
       a root for a subtree and applies the same process.

       So that the new root can easily identify the size of its
       subtree, the (subtree) roots are all powers of two (relative
       to the root) If m = the first power of 2 such that 2^m >= the
       size of the communicator, then the subtree at root at 2^(m-k)
       has size 2^k (with special handling for subtrees that aren't
       a power of two in size).

       Do subdivision.  There are two phases:
       1. Wait for arrival of data.  Because of the power of two nature
       of the subtree roots, the source of this message is always the
       process whose relative rank has the least significant 1 bit CLEARED.
       That is, process 4 (100) receives from process 0, process 7 (111)
       from process 6 (110), etc.
       2. Forward to my subtree

       Note that the process that is the tree root is handled automatically
       by this code, since it has no bits set.  */

    mask = 0x1;
    while (mask < comm_size) {
        if (relative_rank & mask) {
            src = rank - mask;
            if (src < 0)
                src += comm_size;
            if (!is_contig || !is_homogeneous) {
                MPIR_PVAR_INC(bcast, binomial, recv, nbytes, MPI_BYTE);
                mpi_errno =
                    MPIC_Recv(tmp_buf, nbytes, MPI_BYTE, src, MPIR_BCAST_TAG,
                              comm_ptr, MPI_STATUS_IGNORE, errflag);
            } else {
                MPIR_PVAR_INC(bcast, binomial, recv, count, datatype);
                mpi_errno =
                    MPIC_Recv(buffer, count, datatype, src, MPIR_BCAST_TAG,
                              comm_ptr, MPI_STATUS_IGNORE, errflag);
            }
            if (mpi_errno) {
                /* for communication errors,
                 * just record the error but continue */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
            break;
        }
        mask <<= 1;
    }

    /* This process is responsible for all processes that have bits
       set from the LSB up to (but not including) mask.  Because of
       the "not including", we start by shifting mask back down one.

       We can easily change to a different algorithm at any power of two
       by changing the test (mask > 1) to (mask > block_size)

       One such version would use non-blocking operations for the last 2-4
       steps (this also bounds the number of MPI_Requests that would
       be needed).  */

    mask >>= 1;
    while (mask > 0) {
        if (relative_rank + mask < comm_size) {
            dst = rank + mask;
            if (dst >= comm_size)
                dst -= comm_size;
            if (!is_contig || !is_homogeneous) {
                MPIR_PVAR_INC(bcast, binomial, send, nbytes, MPI_BYTE);
                mpi_errno = MPIC_Send(tmp_buf, nbytes, MPI_BYTE, dst,
                                      MPIR_BCAST_TAG, comm_ptr, errflag);
            } else {
                MPIR_PVAR_INC(bcast, binomial, send, count, datatype);
                mpi_errno = MPIC_Send(buffer, count, datatype, dst,
                                      MPIR_BCAST_TAG, comm_ptr, errflag);
            }
            if (mpi_errno) {
                /* for communication errors,
                 * just record the error but continue */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
        }
        mask >>= 1;
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

    MPIR_TIMER_END(coll, bcast, binomial);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIR_Knomial_Bcast_inter_node_MVP(void *buffer, int count,
                                      MPI_Datatype datatype, int root,
                                      int knomial_factor, MPIR_Comm *comm_ptr,
                                      MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, bcast, knomial_internode);
    MPI_Comm shmem_comm, leader_comm;
    MPIR_Comm *shmem_commptr = NULL, *leader_commptr = NULL;
    int local_rank = 0;
    int comm_size = 0, rank = 0;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    MPIR_Request **reqarray = NULL;
    MPI_Status *starray = NULL;
    int src, dst, mask, relative_rank;
    int k;

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_bcast_knomial_internode, 1);
    shmem_comm = comm_ptr->dev.ch.shmem_comm;
    leader_comm = comm_ptr->dev.ch.leader_comm;
    MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
    MPIR_Comm_get_ptr(leader_comm, leader_commptr);
    local_rank = shmem_commptr->rank;

    comm_size = leader_commptr->local_size;
    rank = leader_commptr->rank;

    MPIR_CHKLMEM_DECL(2);

    MPIR_CHKLMEM_MALLOC(reqarray, MPIR_Request **,
                        2 * knomial_factor * sizeof(MPIR_Request *), mpi_errno,
                        "reqarray", MPL_MEM_COLL);

    MPIR_CHKLMEM_MALLOC(starray, MPI_Status *,
                        2 * knomial_factor * sizeof(MPI_Status), mpi_errno,
                        "starray", MPL_MEM_COLL);
    if (local_rank == 0) {
        /* inter-node k-nomial bcast  */
        if (comm_size > 1) {
            relative_rank =
                (rank >= root) ? rank - root : rank - root + comm_size;
            mask = 0x1;

            while (mask < comm_size) {
                if (relative_rank % (knomial_factor * mask)) {
                    src = relative_rank / (knomial_factor * mask) *
                              (knomial_factor * mask) +
                          root;
                    if (src >= comm_size) {
                        src -= comm_size;
                    }

                    MPIR_PVAR_INC(bcast, knomial_internode, recv, count,
                                  datatype);
                    mpi_errno =
                        MPIC_Recv(buffer, count, datatype, src, MPIR_BCAST_TAG,
                                  leader_commptr, MPI_STATUS_IGNORE, errflag);
                    if (mpi_errno) {
                        /* for communication errors,
                         * just record the error but continue */
                        *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                        MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                    }
                    break;
                }
                mask *= knomial_factor;
            }

            mask /= knomial_factor;

            while (mask > 0) {
                int reqs = 0;
                for (k = 1; k < knomial_factor; k++) {
                    if (relative_rank + mask * k < comm_size) {
                        dst = rank + mask * k;
                        if (dst >= comm_size) {
                            dst -= comm_size;
                        }
                        MPIR_PVAR_INC(bcast, knomial_internode, send, count,
                                      datatype);
                        mpi_errno = MPIC_Isend(buffer, count, datatype, dst,
                                               MPIR_BCAST_TAG, leader_commptr,
                                               &reqarray[reqs++], errflag);
                        if (mpi_errno) {
                            /* for communication errors,
                             * just record the error but continue */
                            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                        }
                    }
                }
                mpi_errno = MPIC_Waitall(reqs, reqarray, starray, errflag);
                MPIR_ERR_CHECK(mpi_errno);

                /* --BEGIN ERROR HANDLING-- */
                if (mpi_errno == MPI_ERR_IN_STATUS) {
                    int j;
                    for (j = 0; j < reqs; j++) {
                        if (starray[j].MPI_ERROR != MPI_SUCCESS) {
                            mpi_errno = starray[j].MPI_ERROR;
                            if (mpi_errno) {
                                /* for communication errors,
                                 * just record the error but continue */
                                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER,
                                             "**fail");
                                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                            }
                        }
                    }
                }
                mask /= knomial_factor;
            }
        }
        if (!MVP_USE_OLD_BCAST) {
            /* Start the shmem-bcast before
             * we send the data across the network */
            mpi_errno = MVP_Bcast_intra_node_function(buffer, count, datatype,
                                                      INTRA_NODE_ROOT,
                                                      shmem_commptr, errflag);
        } else {
            MPI_Aint type_size = 0;
            MPIR_Datatype_get_size_macro(datatype, type_size);
            intptr_t nbytes;

            nbytes = (intptr_t)(count) * (type_size);
            if (nbytes <= MVP_KNOMIAL_INTRA_NODE_THRESHOLD) {
                mpi_errno = MPIR_Shmem_Bcast_MVP(buffer, count, datatype,
                                                 INTRA_NODE_ROOT, shmem_commptr,
                                                 errflag);
            } else {
                mpi_errno = MPIR_Knomial_Bcast_intra_node_MVP(
                    buffer, count, datatype, INTRA_NODE_ROOT, shmem_commptr,
                    errflag);
            }
        }
        comm_ptr->dev.ch.intra_node_done = 1;
    }
fn_fail:

    MPIR_CHKLMEM_FREEALL();
    MPIR_TIMER_END(coll, bcast, knomial_internode);
    return mpi_errno;
}

int MPIR_Knomial_Bcast_intra_node_MVP(void *buffer, int count,
                                      MPI_Datatype datatype, int root,
                                      MPIR_Comm *comm_ptr,
                                      MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, bcast, knomial_intranode);
    int local_size = 0, rank;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    MPIR_Request **reqarray = NULL;
    MPI_Status *starray = NULL;
    int src, dst, mask, relative_rank;
    int k;

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_bcast_knomial_intranode, 1);
    local_size = comm_ptr->local_size;
    rank = comm_ptr->rank;
    MPIR_CHKLMEM_DECL(2);

    MPIR_CHKLMEM_MALLOC(reqarray, MPIR_Request **,
                        2 * MVP_KNOMIAL_INTRA_NODE_FACTOR *
                            sizeof(MPIR_Request *),
                        mpi_errno, "reqarray", MPL_MEM_COLL);

    MPIR_CHKLMEM_MALLOC(starray, MPI_Status *,
                        2 * MVP_KNOMIAL_INTRA_NODE_FACTOR * sizeof(MPI_Status),
                        mpi_errno, "starray", MPL_MEM_COLL);

    /* intra-node k-nomial bcast  */
    if (local_size > 1) {
        relative_rank = (rank >= root) ? rank - root : rank - root + local_size;
        mask = 0x1;

        while (mask < local_size) {
            if (relative_rank % (MVP_KNOMIAL_INTRA_NODE_FACTOR * mask)) {
                src = relative_rank / (MVP_KNOMIAL_INTRA_NODE_FACTOR * mask) *
                          (MVP_KNOMIAL_INTRA_NODE_FACTOR * mask) +
                      root;
                if (src >= local_size) {
                    src -= local_size;
                }

                MPIR_PVAR_INC(bcast, knomial_intranode, recv, count, datatype);
                mpi_errno =
                    MPIC_Recv(buffer, count, datatype, src, MPIR_BCAST_TAG,
                              comm_ptr, MPI_STATUS_IGNORE, errflag);
                if (mpi_errno) {
                    /* for communication errors,
                     * just record the error but continue */
                    *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                    MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                }
                break;
            }
            mask *= MVP_KNOMIAL_INTRA_NODE_FACTOR;
        }
        mask /= MVP_KNOMIAL_INTRA_NODE_FACTOR;

        while (mask > 0) {
            int reqs = 0;
            for (k = 1; k < MVP_KNOMIAL_INTRA_NODE_FACTOR; k++) {
                if (relative_rank + mask * k < local_size) {
                    dst = rank + mask * k;
                    if (dst >= local_size) {
                        dst -= local_size;
                    }
                    MPIR_PVAR_INC(bcast, knomial_intranode, send, count,
                                  datatype);
                    mpi_errno =
                        MPIC_Isend(buffer, count, datatype, dst, MPIR_BCAST_TAG,
                                   comm_ptr, &reqarray[reqs++], errflag);
                    if (mpi_errno) {
                        /* for communication errors,
                         * just record the error but continue */
                        *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                        MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                    }
                }
            }
            mpi_errno = MPIC_Waitall(reqs, reqarray, starray, errflag);
            MPIR_ERR_CHECK(mpi_errno);

            /* --BEGIN ERROR HANDLING-- */
            if (mpi_errno == MPI_ERR_IN_STATUS) {
                int j;
                for (j = 0; j < reqs; j++) {
                    if (starray[j].MPI_ERROR != MPI_SUCCESS) {
                        mpi_errno = starray[j].MPI_ERROR;
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
            mask /= MVP_KNOMIAL_INTRA_NODE_FACTOR;
        }
    }

fn_fail:
    MPIR_CHKLMEM_FREEALL();
    MPIR_TIMER_END(coll, bcast, knomial_intranode);
    return mpi_errno;
}

int MPIR_Knomial_Bcast_inter_node_wrapper_MVP(void *buffer, int count,
                                              MPI_Datatype datatype, int root,
                                              MPIR_Comm *comm_ptr,
                                              MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;
    int knomial_factor = 0;
    if (MVP_Bcast_function == &MPIR_Pipelined_Bcast_MVP) {
        knomial_factor = mvp_pipelined_knomial_factor;
    } else {
        knomial_factor = MVP_KNOMIAL_INTER_NODE_FACTOR;
    }
    mpi_errno = MPIR_Knomial_Bcast_inter_node_MVP(
        buffer, count, datatype, root, knomial_factor, comm_ptr, errflag);
    MPIR_ERR_CHECK(mpi_errno);

fn_fail:
    return mpi_errno;
}
