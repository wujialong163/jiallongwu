#include "reduce_scatter_tuning.h"

int MPIR_Reduce_scatter_Rec_Halving_MVP(const void *sendbuf, void *recvbuf,
                                        const int *recvcnts,
                                        MPI_Datatype datatype, MPI_Op op,
                                        MPIR_Comm *comm_ptr,
                                        MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, reduce_scatter, rec_halving);
    int rank, comm_size, i;
    MPI_Aint extent, true_extent, true_lb;
    int *disps;
    void *tmp_recvbuf, *tmp_results;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    int total_count, dst;
    int mask;
    int *newcnts, *newdisps, rem, newdst, send_idx, recv_idx, last_idx,
        send_cnt, recv_cnt;
    int pof2, old_i, newrank;
    MPI_User_function *uop;
    int is_commutative;
#ifdef HAVE_CXX_BINDING
    int is_cxx_uop = 0;
#endif
    MPIR_CHKLMEM_DECL(5);

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_reduce_scatter_rec_halving, 1);

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

    /* commutative and short. use recursive halving algorithm */
    /* allocate temp. buffer to receive incoming data */
    MPIR_CHKLMEM_MALLOC(tmp_recvbuf, void *,
                        total_count *(MPL_MAX(true_extent, extent)), mpi_errno,
                        "tmp_recvbuf", MPL_MEM_COLL);
    /* adjust for potential negative lower bound in datatype */
    tmp_recvbuf = (void *)((char *)tmp_recvbuf - true_lb);

    /* need to allocate another temporary buffer to accumulate
       results because recvbuf may not be big enough */
    MPIR_CHKLMEM_MALLOC(tmp_results, void *,
                        total_count *(MPL_MAX(true_extent, extent)), mpi_errno,
                        "tmp_results", MPL_MEM_COLL);
    /* adjust for potential negative lower bound in datatype */
    tmp_results = (void *)((char *)tmp_results - true_lb);

    /* copy sendbuf into tmp_results */
    if (sendbuf != MPI_IN_PLACE)
        mpi_errno = MPIR_Localcopy(sendbuf, total_count, datatype, tmp_results,
                                   total_count, datatype);
    else
        mpi_errno = MPIR_Localcopy(recvbuf, total_count, datatype, tmp_results,
                                   total_count, datatype);

    MPIR_ERR_CHECK(mpi_errno);

    pof2 = comm_ptr->dev.ch.gpof2;

    rem = comm_size - pof2;

    /* In the non-power-of-two case, all even-numbered
       processes of rank < 2*rem send their data to
       (rank+1). These even-numbered processes no longer
       participate in the algorithm until the very end. The
       remaining processes form a nice power-of-two. */

    if (rank < 2 * rem) {
        if (rank % 2 == 0) { /* even */
            MPIR_PVAR_INC(reduce_scatter, rec_halving, send, total_count,
                          datatype);
            mpi_errno = MPIC_Send(tmp_results, total_count, datatype, rank + 1,
                                  MPIR_REDUCE_SCATTER_TAG, comm_ptr, errflag);
            if (mpi_errno) {
                /* for communication errors,
                 * just record the error but continue */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }

            /* temporarily set the rank to -1 so that this
               process does not pariticipate in recursive
               doubling */
            newrank = -1;
        } else { /* odd */
            MPIR_PVAR_INC(reduce_scatter, rec_halving, recv, total_count,
                          datatype);
            mpi_errno = MPIC_Recv(tmp_recvbuf, total_count, datatype, rank - 1,
                                  MPIR_REDUCE_SCATTER_TAG, comm_ptr,
                                  MPI_STATUS_IGNORE, errflag);
            if (mpi_errno) {
                /* for communication errors,
                 * just record the error but continue */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }

            /* do the reduction on received data. since the
               ordering is right, it doesn't matter whether
               the operation is commutative or not. */
            MPIR_MVP_Reduce_local(tmp_recvbuf, tmp_results,
                                  (MPI_Aint)total_count, datatype, uop
#ifdef HAVE_CXX_BINDING
                                  ,
                                  is_cxx_uop
#endif
            );

            /* change the rank */
            newrank = rank / 2;
        }
    } else /* rank >= 2*rem */
        newrank = rank - rem;

    if (newrank != -1) {
        /* recalculate the recvcnts and disps arrays because the
           even-numbered processes who no longer participate will
           have their result calculated by the process to their
           right (rank+1). */
        MPIR_Request *request = NULL;
        MPI_Status status;
        MPIR_CHKLMEM_MALLOC(newcnts, int *, pof2 * sizeof(int), mpi_errno,
                            "newcnts", MPL_MEM_COLL);
        MPIR_CHKLMEM_MALLOC(newdisps, int *, pof2 * sizeof(int), mpi_errno,
                            "newdisps", MPL_MEM_COLL);

        for (i = 0; i < pof2; i++) {
            /* what does i map to in the old ranking? */
            old_i = (i < rem) ? i * 2 + 1 : i + rem;
            if (old_i < 2 * rem) {
                /* This process has to also do
                 * its left neighbor's work */
                newcnts[i] = recvcnts[old_i] + recvcnts[old_i - 1];
            } else
                newcnts[i] = recvcnts[old_i];
        }

        newdisps[0] = 0;
        for (i = 1; i < pof2; i++)
            newdisps[i] = newdisps[i - 1] + newcnts[i - 1];

        mask = pof2 >> 1;
        send_idx = recv_idx = 0;
        last_idx = pof2;
        while (mask > 0) {
            newdst = newrank ^ mask;
            /* find real rank of dest */
            dst = (newdst < rem) ? newdst * 2 + 1 : newdst + rem;

            send_cnt = recv_cnt = 0;
            if (newrank < newdst) {
                send_idx = recv_idx + mask;
                for (i = send_idx; i < last_idx; i++)
                    send_cnt += newcnts[i];
                for (i = recv_idx; i < send_idx; i++)
                    recv_cnt += newcnts[i];
            } else {
                recv_idx = send_idx + mask;
                for (i = send_idx; i < recv_idx; i++)
                    send_cnt += newcnts[i];
                for (i = recv_idx; i < last_idx; i++)
                    recv_cnt += newcnts[i];
            }
            /* Send data from tmp_results. Recv into tmp_recvbuf */
            if ((send_cnt != 0) && (recv_cnt != 0)) {
                MPIR_PVAR_INC(reduce_scatter, rec_halving, send, send_cnt,
                              datatype);
                MPIR_PVAR_INC(reduce_scatter, rec_halving, recv, recv_cnt,
                              datatype);
                mpi_errno = MPIC_Sendrecv(
                    (char *)tmp_results + newdisps[send_idx] * extent, send_cnt,
                    datatype, dst, MPIR_REDUCE_SCATTER_TAG,
                    (char *)tmp_recvbuf + newdisps[recv_idx] * extent, recv_cnt,
                    datatype, dst, MPIR_REDUCE_SCATTER_TAG, comm_ptr,
                    MPI_STATUS_IGNORE, errflag);
            } else if ((send_cnt == 0) && (recv_cnt != 0)) {
                MPIR_PVAR_INC(reduce_scatter, rec_halving, recv, recv_cnt,
                              datatype);
                mpi_errno = MPIC_Irecv(
                    (char *)tmp_recvbuf + newdisps[recv_idx] * extent, recv_cnt,
                    datatype, dst, MPIR_REDUCE_SCATTER_TAG, comm_ptr, &request);
            } else if ((recv_cnt == 0) && (send_cnt != 0)) {
                MPIR_PVAR_INC(reduce_scatter, rec_halving, send, send_cnt,
                              datatype);
                mpi_errno = MPIC_Send(
                    (char *)tmp_results + newdisps[send_idx] * extent, send_cnt,
                    datatype, dst, MPIR_REDUCE_SCATTER_TAG, comm_ptr, errflag);
            }

            if (mpi_errno) {
                /* for communication errors,
                 * just record the error but continue */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }

            if ((send_cnt == 0) && (recv_cnt != 0)) {
                mpi_errno = MPIC_Waitall(1, &request, &status, errflag);
                if (mpi_errno) {
                    /* for communication errors,
                     * just record the error but continue */
                    *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                    MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                }
            }

            /* tmp_recvbuf contains data received in this step.
               tmp_results contains data accumulated so far */

            if (recv_cnt) {
                MPIR_MVP_Reduce_local(
                    (char *)tmp_recvbuf + newdisps[recv_idx] * extent,
                    (char *)tmp_results + newdisps[recv_idx] * extent,
                    (MPI_Aint)recv_cnt, datatype, uop
#ifdef HAVE_CXX_BINDING
                    ,
                    is_cxx_uop
#endif
                );
            }

            /* update send_idx for next iteration */
            send_idx = recv_idx;
            last_idx = recv_idx + mask;
            mask >>= 1;
        }

        /* copy this process's result from tmp_results to recvbuf */
        if (recvcnts[rank]) {
            mpi_errno = MPIR_Localcopy(
                (char *)tmp_results + disps[rank] * extent, recvcnts[rank],
                datatype, recvbuf, recvcnts[rank], datatype);
            MPIR_ERR_CHECK(mpi_errno);
        }
    }

    /* In the non-power-of-two case, all odd-numbered
       processes of rank < 2*rem send to (rank-1) the result they
       calculated for that process */
    if (rank < 2 * rem) {
        if (rank % 2) { /* odd */
            if (recvcnts[rank - 1]) {
                MPIR_PVAR_INC(reduce_scatter, rec_halving, send,
                              recvcnts[rank - 1], datatype);
                mpi_errno =
                    MPIC_Send((char *)tmp_results + disps[rank - 1] * extent,
                              recvcnts[rank - 1], datatype, rank - 1,
                              MPIR_REDUCE_SCATTER_TAG, comm_ptr, errflag);
                if (mpi_errno) {
                    /* for communication erroras,
                     * just record the error but continue */
                    *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                    MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                }
            }
        } else { /* even */
            if (recvcnts[rank]) {
                MPIR_PVAR_INC(reduce_scatter, rec_halving, recv, recvcnts[rank],
                              datatype);
                mpi_errno = MPIC_Recv(recvbuf, recvcnts[rank], datatype,
                                      rank + 1, MPIR_REDUCE_SCATTER_TAG,
                                      comm_ptr, MPI_STATUS_IGNORE, errflag);
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

fn_exit:
    MPIR_CHKLMEM_FREEALL();

    if (mpi_errno_ret)
        mpi_errno = mpi_errno_ret;
    else if (*errflag)
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**coll_fail");

    MPIR_TIMER_END(coll, reduce_scatter, rec_halving);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
