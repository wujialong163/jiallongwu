#include "allgatherv_tuning.h"

#undef FUNCNAME
#define FUNCNAME MPIR_Allgatherv_Bruck_MVP
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPIR_Allgatherv_Bruck_MVP(const void *sendbuf, int sendcount,
                              MPI_Datatype sendtype, void *recvbuf,
                              const int *recvcounts, const int *displs,
                              MPI_Datatype recvtype, MPIR_Comm *comm_ptr,
                              MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, allgatherv, bruck);
    int comm_size, rank, j, i;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    MPI_Status status;
    MPI_Aint recvbuf_extent, recvtype_extent, recvtype_true_extent,
        recvtype_true_lb, recv_cnt;
    int curr_cnt, send_cnt, dst, total_count, pof2, src, rem;
    void *tmp_buf;

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_allgatherv_bruck, 1);
    MPIR_CHKLMEM_DECL(1);

    comm_size = comm_ptr->local_size;
    rank = comm_ptr->rank;

    total_count = 0;
    for (i = 0; i < comm_size; i++)
        total_count += recvcounts[i];

    if (total_count == 0)
        goto fn_exit;

    MPIR_Datatype_get_extent_macro(recvtype, recvtype_extent);

    /* allocate a temporary buffer of the same size as recvbuf. */
    /* get true extent of recvtype */
    MPIR_Type_get_true_extent_impl(recvtype, &recvtype_true_lb,
                                   &recvtype_true_extent);

    recvbuf_extent =
        total_count * (MPL_MAX(recvtype_true_extent, recvtype_extent));

    MPIR_CHKLMEM_MALLOC(tmp_buf, void *, recvbuf_extent, mpi_errno, "tmp_buf",
                        MPL_MEM_COLL);

    /* adjust for potential negative lower bound in datatype */
    tmp_buf = (void *)((char *)tmp_buf - recvtype_true_lb);

    /* copy local data to the top of tmp_buf */
    if (sendbuf != MPI_IN_PLACE) {
        mpi_errno = MPIR_Localcopy(sendbuf, sendcount, sendtype, tmp_buf,
                                   recvcounts[rank], recvtype);
        MPIR_ERR_CHECK(mpi_errno);
    } else {
        mpi_errno = MPIR_Localcopy(
            ((char *)recvbuf + displs[rank] * recvtype_extent),
            recvcounts[rank], recvtype, tmp_buf, recvcounts[rank], recvtype);
        MPIR_ERR_CHECK(mpi_errno);
    }

    /* do the first \floor(\lg p) steps */

    curr_cnt = recvcounts[rank];
    pof2 = 1;
    while (pof2 <= comm_size / 2) {
        src = (rank + pof2) % comm_size;
        dst = (rank - pof2 + comm_size) % comm_size;

        MPIR_PVAR_INC(allgatherv, bruck, send, curr_cnt, recvtype);
        MPIR_PVAR_INC(allgatherv, bruck, recv, total_count - curr_cnt,
                      recvtype);
        mpi_errno =
            MPIC_Sendrecv(tmp_buf, curr_cnt, recvtype, dst, MPIR_ALLGATHERV_TAG,
                          ((char *)tmp_buf + curr_cnt * recvtype_extent),
                          total_count - curr_cnt, recvtype, src,
                          MPIR_ALLGATHERV_TAG, comm_ptr, &status, errflag);
        if (mpi_errno) {
            /* for communication errors, just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            recv_cnt = 0;
        } else
            MPIR_Get_count_impl(&status, recvtype, &recv_cnt);
        curr_cnt += recv_cnt;

        pof2 *= 2;
    }

    /* if comm_size is not a power of two, one more step is needed */

    rem = comm_size - pof2;
    if (rem) {
        src = (rank + pof2) % comm_size;
        dst = (rank - pof2 + comm_size) % comm_size;

        send_cnt = 0;
        for (i = 0; i < rem; i++)
            send_cnt += recvcounts[(rank + i) % comm_size];

        MPIR_PVAR_INC(allgatherv, bruck, send, send_cnt, recvtype);
        MPIR_PVAR_INC(allgatherv, bruck, recv, total_count - curr_cnt,
                      recvtype);
        mpi_errno = MPIC_Sendrecv(
            tmp_buf, send_cnt, recvtype, dst, MPIR_ALLGATHERV_TAG,
            ((char *)tmp_buf + curr_cnt * recvtype_extent),
            total_count - curr_cnt, recvtype, src, MPIR_ALLGATHERV_TAG,
            comm_ptr, MPI_STATUS_IGNORE, errflag);
        if (mpi_errno) {
            /* for communication errors, just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
    }

    /* Rotate blocks in tmp_buf down by (rank) blocks and store
     * result in recvbuf. */

    send_cnt = 0;
    for (i = 0; i < (comm_size - rank); i++) {
        j = (rank + i) % comm_size;
        mpi_errno = MPIR_Localcopy(
            (char *)tmp_buf + send_cnt * recvtype_extent, recvcounts[j],
            recvtype, (char *)recvbuf + displs[j] * recvtype_extent,
            recvcounts[j], recvtype);
        MPIR_ERR_CHECK(mpi_errno);
        send_cnt += recvcounts[j];
    }

    for (i = 0; i < rank; i++) {
        mpi_errno = MPIR_Localcopy(
            (char *)tmp_buf + send_cnt * recvtype_extent, recvcounts[i],
            recvtype, (char *)recvbuf + displs[i] * recvtype_extent,
            recvcounts[i], recvtype);
        MPIR_ERR_CHECK(mpi_errno);
        send_cnt += recvcounts[i];
    }
fn_exit:
    MPIR_CHKLMEM_FREEALL();
    if (mpi_errno_ret)
        mpi_errno = mpi_errno_ret;
    else if (*errflag)
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**coll_fail");

    MPIR_TIMER_END(coll, allgatherv, bruck);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
