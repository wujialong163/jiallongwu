#include "reduce_tuning.h"

/* This function implements a binomial tree reduce.
 * Cost = lgp.alpha + n.lgp.beta + n.lgp.gamma
 */
int MPIR_Reduce_binomial_MVP(const void *sendbuf, void *recvbuf, int count,
                             MPI_Datatype datatype, MPI_Op op, int root,
                             MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, reduce, binomial);
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    MPI_Status status;
    int comm_size, rank, is_commutative;
    int root_rank_list_index, i;
    int mask, relrank, source, lroot;
    MPI_User_function *uop;
    MPI_Aint true_lb, true_extent, extent;
    void *tmp_buf = NULL, *tmp_rcv_buf = NULL;
#ifdef HAVE_CXX_BINDING
    int is_cxx_uop = 0;
#endif
    MPIR_CHKLMEM_DECL(2);

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_reduce_binomial, 1);

    if (count == 0) {
        MPIR_TIMER_END(coll, reduce, binomial);
        return MPI_SUCCESS;
    }

    comm_size = comm_ptr->local_size;

    /* Create a temporary buffer */

    MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);
    MPIR_Datatype_get_extent_macro(datatype, extent);

    /* Get func ptr for the reduction op
     * and initialize is_commutative and is_cxx_uop */
    MPIR_get_op_ptr(&uop, &is_commutative,
#ifdef HAVE_CXX_BINDING
                    &is_cxx_uop,
#endif
                    op);

    if (comm_ptr->dev.ch.rank_list == NULL || is_commutative != 1 ||
        comm_ptr->dev.ch.is_blocked == 1) {
        rank = comm_ptr->rank;
        MPIR_Rank_list_mapper = &Bunch_Rank_list_mapper;
        root_rank_list_index = root;
    } else {
        /* my index on rank_list */
        rank = comm_ptr->dev.ch.rank_list_index;
        MPIR_Assert(rank >= 0);
        MPIR_Rank_list_mapper = &Cyclic_Rank_list_mapper;

        /* now find the root's index on rank_list */
        for (i = 0; i < comm_size; i++) {
            if (comm_ptr->dev.ch.rank_list[i] == root)
                break;
        }
        root_rank_list_index = i;
    }

    MPIR_CHKLMEM_MALLOC(tmp_buf, void *,
                        count *(MPL_MAX(extent, true_extent)) * 2, mpi_errno,
                        "temporary buffer", MPL_MEM_COLL);

    /* If I'm not the root, then my recvbuf may not be valid, therefore
     * use a temporary buffer */
    if (rank != root_rank_list_index) {
        if (sendbuf == MPI_IN_PLACE) {
            tmp_rcv_buf = recvbuf;
        }
        recvbuf = (void *)((char *)(tmp_buf +
                                    count * (MPL_MAX(extent, true_extent))) -
                           true_lb);
        if (sendbuf == MPI_IN_PLACE) {
            mpi_errno = MPIR_Localcopy(tmp_rcv_buf, count, datatype, recvbuf,
                                       count, datatype);
            MPIR_ERR_CHECK(mpi_errno);
        }
    }

    /* adjust for potential negative lower bound in datatype */
    tmp_buf = (void *)((char *)tmp_buf - true_lb);

    if ((sendbuf != MPI_IN_PLACE) && (sendbuf != recvbuf)) {
        mpi_errno =
            MPIR_Localcopy(sendbuf, count, datatype, recvbuf, count, datatype);
        MPIR_ERR_CHECK(mpi_errno);
    }

    /* This code is from MPICH-1. */

    /* Here's the algorithm.  Relative to the root, look at the bit pattern in
       my rank.  Starting from the right (lsb), if the bit is 1, send to
       the node with that bit zero and exit; if the bit is 0, receive from the
       node with that bit set and combine (as long as that node is within the
       group)

       Note that by receiving with source selection, we guarentee that we get
       the same bits with the same input.  If we allowed the parent to receive
       the children in any order, then timing differences could cause different
       results (roundoff error, over/underflows in some cases, etc).

       Because of the way these are ordered, if root is 0, then this is correct
       for both commutative and non-commutitive operations.  If root is not
       0, then for non-commutitive, we use a root of zero and then send
       the result to the root.  To see this, note that the ordering is
       mask = 1: (ab)(cd)(ef)(gh)            (odds send to evens)
       mask = 2: ((ab)(cd))((ef)(gh))        (3,6 send to 0,4)
       mask = 4: (((ab)(cd))((ef)(gh)))      (4 sends to 0)

       Comments on buffering.
       If the datatype is not contiguous, we still need to pass contiguous
       data to the user routine.
       In this case, we should make a copy of the data in some format,
       and send/operate on that.

       In general, we can't use MPI_PACK, because the alignment of that
       is rather vague, and the data may not be re-usable.  What we actually
       need is a "squeeze" operation that removes the skips.
     */
    mask = 0x1;
    if (is_commutative)
        lroot = root_rank_list_index;
    else
        lroot = 0;
    relrank = (rank - lroot + comm_size) % comm_size;

    while (/*(mask & relrank) == 0 && */ mask < comm_size) {
        /* Receive */
        if ((mask & relrank) == 0) {
            source = (relrank | mask);
            if (source < comm_size) {
                source = (source + lroot) % comm_size;
                MPIR_PVAR_INC(reduce, binomial, recv, count, datatype);
                mpi_errno =
                    MPIC_Recv(tmp_buf, count, datatype,
                              MPIR_Rank_list_mapper(comm_ptr, source),
                              MPIR_REDUCE_TAG, comm_ptr, &status, errflag);
                if (mpi_errno) {
                    /* for communication errors,
                     * just record the error but continue */
                    *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                    MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                }

                /* The sender is above us, so the received buffer must be
                   the second argument (in the noncommutative case). */
                if (is_commutative) {
                    MPIR_MVP_Reduce_local(tmp_buf, recvbuf, (MPI_Aint)count,
                                          datatype, uop
#ifdef HAVE_CXX_BINDING
                                          ,
                                          is_cxx_uop
#endif
                    );

                } else {
                    MPIR_MVP_Reduce_local(recvbuf, tmp_buf, (MPI_Aint)count,
                                          datatype, uop
#ifdef HAVE_CXX_BINDING
                                          ,
                                          is_cxx_uop
#endif
                    );

                    mpi_errno = MPIR_Localcopy(tmp_buf, count, datatype,
                                               recvbuf, count, datatype);
                    MPIR_ERR_CHECK(mpi_errno);
                }
            }
        } else {
            /* I've received all that I'm going to.  Send my result to
               my parent */
            source = ((relrank & (~mask)) + lroot) % comm_size;
            MPIR_PVAR_INC(reduce, binomial, send, count, datatype);
            mpi_errno = MPIC_Send(recvbuf, count, datatype,
                                  MPIR_Rank_list_mapper(comm_ptr, source),
                                  MPIR_REDUCE_TAG, comm_ptr, errflag);
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

    if (!is_commutative && (root_rank_list_index != 0)) {
        if (rank == 0) {
            MPIR_PVAR_INC(reduce, binomial, send, count, datatype);
            mpi_errno =
                MPIC_Send(recvbuf, count, datatype,
                          MPIR_Rank_list_mapper(comm_ptr, root_rank_list_index),
                          MPIR_REDUCE_TAG, comm_ptr, errflag);
        } else if (rank == root_rank_list_index) {
            MPIR_PVAR_INC(reduce, binomial, recv, count, datatype);
            mpi_errno = MPIC_Recv(recvbuf, count, datatype,
                                  MPIR_Rank_list_mapper(comm_ptr, 0),
                                  MPIR_REDUCE_TAG, comm_ptr, &status, errflag);
        }
        if (mpi_errno) {
            /* for communication errors, just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
    }

fn_exit:
    MPIR_CHKLMEM_FREEALL();
    if (mpi_errno_ret)
        mpi_errno = mpi_errno_ret;
    else if (*errflag)
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**coll_fail");

    MPIR_TIMER_END(coll, reduce, binomial);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIR_Reduce_kinomial_trace
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPIR_Reduce_knomial_trace(int root, int mvp_reduce_knomial_factor,
                              MPIR_Comm *comm_ptr, int *dst,
                              int *expected_send_count,
                              int *expected_recv_count, int **src_array)
{
    int mask = 0x1, k, comm_size, src, rank, relative_rank, lroot = 0;
    int orig_mask = 0x1;
    int recv_iter = 0, send_iter = 0;
    int *knomial_reduce_src_array = NULL;
    rank = comm_ptr->rank;
    comm_size = comm_ptr->local_size;

    lroot = root;
    relative_rank = (rank - lroot + comm_size) % comm_size;

    /* First compute to whom we need to send data */
    while (mask < comm_size) {
        if (relative_rank % (mvp_reduce_knomial_factor * mask)) {
            *dst = relative_rank / (mvp_reduce_knomial_factor * mask) *
                       (mvp_reduce_knomial_factor * mask) +
                   root;
            if (*dst >= comm_size) {
                *dst -= comm_size;
            }
            send_iter++;
            break;
        }
        mask *= mvp_reduce_knomial_factor;
    }
    mask /= mvp_reduce_knomial_factor;

    /* Now compute how many children we have in the knomial-tree */
    orig_mask = mask;
    while (mask > 0) {
        for (k = 1; k < mvp_reduce_knomial_factor; k++) {
            if (relative_rank + mask * k < comm_size) {
                recv_iter++;
            }
        }
        mask /= mvp_reduce_knomial_factor;
    }

    /* Finally, fill up the src array */
    if (recv_iter > 0) {
        knomial_reduce_src_array =
            MPL_malloc(sizeof(int) * recv_iter, MPL_MEM_COLL);
    }

    mask = orig_mask;
    recv_iter = 0;
    while (mask > 0) {
        for (k = 1; k < mvp_reduce_knomial_factor; k++) {
            if (relative_rank + mask * k < comm_size) {
                src = rank + mask * k;
                if (src >= comm_size) {
                    src -= comm_size;
                }
                knomial_reduce_src_array[recv_iter++] = src;
            }
        }
        mask /= mvp_reduce_knomial_factor;
    }

    *expected_recv_count = recv_iter;
    *expected_send_count = send_iter;
    *src_array = knomial_reduce_src_array;
    return 0;
}

int MPIR_Reduce_knomial_MVP(const void *sendbuf, void *recvbuf, int count,
                            MPI_Datatype datatype, MPI_Op op, int root,
                            int mvp_reduce_knomial_factor, MPIR_Comm *comm_ptr,
                            MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, reduce, knomial);
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    int rank, is_commutative;
    int src, k;
    MPI_User_function *uop;
    MPIR_Request *send_request = NULL;
    int index = 0;
    MPI_Aint true_lb, true_extent, extent;
    MPI_Status status;
    int recv_iter = 0, dst, expected_send_count, expected_recv_count;
    int *src_array = NULL;
    void **tmp_buf = NULL;
    MPIR_Request **requests = NULL;
    MPI_Request *mpi_reqs = NULL;
#ifdef HAVE_CXX_BINDING
    int is_cxx_uop = 0;
#endif
    MPIR_CHKLMEM_DECL(1);

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_reduce_knomial, 1);

    if (count == 0) {
        MPIR_TIMER_END(coll, reduce, knomial);
        return MPI_SUCCESS;
    }

    rank = comm_ptr->rank;

    /* Create a temporary buffer */

    MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);
    MPIR_Datatype_get_extent_macro(datatype, extent);

    /* Get func ptr for the reduction op
     * and initialize is_commutative and is_cxx_uop */
    MPIR_get_op_ptr(&uop, &is_commutative,
#ifdef HAVE_CXX_BINDING
                    &is_cxx_uop,
#endif
                    op);

    if (rank != root && sendbuf != MPI_IN_PLACE) {
        MPIR_CHKLMEM_MALLOC(recvbuf, void *,
                            count *(MPL_MAX(extent, true_extent)), mpi_errno,
                            "receive buffer", MPL_MEM_COLL);
        recvbuf = (void *)((char *)recvbuf - true_lb);
    }

    if (sendbuf != MPI_IN_PLACE) {
        mpi_errno =
            MPIR_Localcopy(sendbuf, count, datatype, recvbuf, count, datatype);
        MPIR_ERR_CHECK(mpi_errno);
    }

    MPIR_Reduce_knomial_trace(root, mvp_reduce_knomial_factor, comm_ptr, &dst,
                              &expected_send_count, &expected_recv_count,
                              &src_array);

    if (expected_recv_count > 0) {
        tmp_buf =
            MPL_malloc(sizeof(void *) * expected_recv_count, MPL_MEM_COLL);
        requests = (MPIR_Request **)MPL_malloc(
            sizeof(MPIR_Request *) * expected_recv_count, MPL_MEM_COLL);
        mpi_reqs = (MPI_Request *)MPL_malloc(
            sizeof(MPI_Request) * expected_recv_count, MPL_MEM_COLL);
        for (k = 0; k < expected_recv_count; k++) {
            tmp_buf[k] = MPL_malloc(count * (MPL_MAX(extent, true_extent)),
                                    MPL_MEM_COLL);
            tmp_buf[k] = (void *)((char *)tmp_buf[k] - true_lb);
        }

        while (recv_iter < expected_recv_count) {
            src = src_array[expected_recv_count - (recv_iter + 1)];

            MPIR_PVAR_INC(reduce, knomial, recv, count, datatype);
            mpi_errno =
                MPIC_Irecv(tmp_buf[recv_iter], count, datatype, src,
                           MPIR_REDUCE_TAG, comm_ptr, &requests[recv_iter]);
            /* Convert the MPIR_Request objects to MPI_Request objects */
            mpi_reqs[recv_iter] = requests[recv_iter]->handle;
            recv_iter++;

            if (mpi_errno) {
                /* for communication errors,
                 * just record the error but continue*/
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
        }

        recv_iter = 0;
        while (recv_iter < expected_recv_count) {
            mpi_errno =
                PMPI_Waitany(expected_recv_count, mpi_reqs, &index, &status);
            recv_iter++;
            if (mpi_errno) {
                /* for communication errors, just record the error but
                 * continue*/
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
            if (is_commutative) {
                MPIR_MVP_Reduce_local(tmp_buf[index], recvbuf, (MPI_Aint)count,
                                      datatype, uop
#ifdef HAVE_CXX_BINDING
                                      ,
                                      is_cxx_uop
#endif
                );
            }
        }

        for (k = 0; k < expected_recv_count; k++) {
            MPL_free(tmp_buf[k]);
        }
        MPL_free(tmp_buf);
        MPL_free(requests);
        MPL_free(mpi_reqs);
    }

    if (src_array != NULL) {
        MPL_free(src_array);
    }

    if (rank != root) {
        MPIR_PVAR_INC(reduce, knomial, send, count, datatype);
        mpi_errno = MPIC_Isend(recvbuf, count, datatype, dst, MPIR_REDUCE_TAG,
                               comm_ptr, &send_request, errflag);
        if (mpi_errno) {
            /* for communication errors, just record the error but continue
             * */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
        mpi_errno = MPIC_Waitall(1, &send_request, &status, errflag);
    }

fn_exit:
    MPIR_CHKLMEM_FREEALL();
    if (mpi_errno_ret)
        mpi_errno = mpi_errno_ret;
    else if (*errflag)
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**coll_fail");

    MPIR_TIMER_END(coll, reduce, knomial);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIR_Reduce_inter_knomial_wrapper_MVP(const void *sendbuf, void *recvbuf,
                                          int count, MPI_Datatype datatype,
                                          MPI_Op op, int root,
                                          MPIR_Comm *comm_ptr,
                                          MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;
    mpi_errno = MPIR_Reduce_knomial_MVP(sendbuf, recvbuf, count, datatype, op,
                                        root, MVP_REDUCE_INTER_KNOMIAL_FACTOR,
                                        comm_ptr, errflag);
    MPIR_ERR_CHECK(mpi_errno);

fn_fail:
    return mpi_errno;
}

int MPIR_Reduce_intra_knomial_wrapper_MVP(const void *sendbuf, void *recvbuf,
                                          int count, MPI_Datatype datatype,
                                          MPI_Op op, int root,
                                          MPIR_Comm *comm_ptr,
                                          MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;
    mpi_errno = MPIR_Reduce_knomial_MVP(sendbuf, recvbuf, count, datatype, op,
                                        root, MVP_REDUCE_INTER_KNOMIAL_FACTOR,
                                        comm_ptr, errflag);
    MPIR_ERR_CHECK(mpi_errno);

fn_fail:
    return mpi_errno;
}
