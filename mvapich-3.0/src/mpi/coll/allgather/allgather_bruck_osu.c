#include "allgather_tuning.h"

extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allgather_bruck;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_bruck;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_bruck_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_bruck_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_bruck_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_bruck_count_recv;

int MPIR_Allgather_Bruck_MVP(const void *sendbuf, int sendcount,
                             MPI_Datatype sendtype, void *recvbuf,
                             int recvcount, MPI_Datatype recvtype,
                             MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, allgather, bruck);
    int comm_size, rank;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    MPI_Aint recvtype_extent;
    MPI_Aint recvtype_true_extent, recvbuf_extent, recvtype_true_lb;
    int src, rem;
    void *tmp_buf;
    int curr_cnt, dst;
    int pof2 = 0;

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_allgather_bruck, 1);

    comm_size = comm_ptr->local_size;
    rank = comm_ptr->rank;

    MPIR_Datatype_get_extent_macro(recvtype, recvtype_extent);

    /* get true extent of recvtype */
    MPIR_Type_get_true_extent_impl(recvtype, &recvtype_true_lb,
                                   &recvtype_true_extent);
    recvbuf_extent = recvcount * comm_size *
                     (MPL_MAX(recvtype_true_extent, recvtype_extent));
    /* allocate a temporary buffer of the same size as recvbuf. */
    tmp_buf = MPL_malloc(recvbuf_extent, MPL_MEM_COLL);
    /* --BEGIN ERROR HANDLING-- */
    if (!tmp_buf) {
        mpi_errno =
            MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, __func__,
                                 __LINE__, MPI_ERR_OTHER, "**nomem", 0);
        return mpi_errno;
    }
    /* --END ERROR HANDLING-- */

    /* adjust for potential negative lower bound in datatype */
    tmp_buf = (void *)((char *)tmp_buf - recvtype_true_lb);

    /* copy local data to the top of tmp_buf */
    if (sendbuf != MPI_IN_PLACE) {
        mpi_errno = MPIR_Localcopy(sendbuf, sendcount, sendtype, tmp_buf,
                                   recvcount, recvtype);
        MPIR_ERR_CHECK(mpi_errno);
    } else {
        mpi_errno = MPIR_Localcopy(
            ((char *)recvbuf + rank * recvcount * recvtype_extent), recvcount,
            recvtype, tmp_buf, recvcount, recvtype);
        MPIR_ERR_CHECK(mpi_errno);
    }

    /* do the first \floor(\lg p) steps */

    curr_cnt = recvcount;
    pof2 = 1;
    while (pof2 <= comm_size / 2) {
        src = (rank + pof2) % comm_size;
        dst = (rank - pof2 + comm_size) % comm_size;
        MPIR_PVAR_INC(allgather, bruck, send, curr_cnt, recvtype);
        MPIR_PVAR_INC(allgather, bruck, recv, curr_cnt, recvtype);
        mpi_errno = MPIC_Sendrecv(
            tmp_buf, curr_cnt, recvtype, dst, MPIR_ALLGATHER_TAG,
            ((char *)tmp_buf + curr_cnt * recvtype_extent), curr_cnt, recvtype,
            src, MPIR_ALLGATHER_TAG, comm_ptr, MPI_STATUS_IGNORE, errflag);
        if (mpi_errno) {
            /* for communication errors, just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
        curr_cnt *= 2;
        pof2 *= 2;
    }

    /* if comm_size is not a power of two, one more step is needed */

    rem = comm_size - pof2;
    if (rem) {
        src = (rank + pof2) % comm_size;
        dst = (rank - pof2 + comm_size) % comm_size;
        MPIR_PVAR_INC(allgather, bruck, send, rem * recvcount, recvtype);
        MPIR_PVAR_INC(allgather, bruck, recv, rem * recvcount, recvtype);
        mpi_errno = MPIC_Sendrecv(
            tmp_buf, rem * recvcount, recvtype, dst, MPIR_ALLGATHER_TAG,
            ((char *)tmp_buf + curr_cnt * recvtype_extent), rem * recvcount,
            recvtype, src, MPIR_ALLGATHER_TAG, comm_ptr, MPI_STATUS_IGNORE,
            errflag);
        if (mpi_errno) {
            /* for communication errors, just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
    }

    /* Rotate blocks in tmp_buf down by (rank) blocks and store
     * result in recvbuf. */

    mpi_errno =
        MPIR_Localcopy(tmp_buf, (comm_size - rank) * recvcount, recvtype,
                       (char *)recvbuf + rank * recvcount * recvtype_extent,
                       (comm_size - rank) * recvcount, recvtype);
    MPIR_ERR_CHECK(mpi_errno);

    if (rank) {
        mpi_errno = MPIR_Localcopy(
            (char *)tmp_buf + (comm_size - rank) * recvcount * recvtype_extent,
            rank * recvcount, recvtype, recvbuf, rank * recvcount, recvtype);
        MPIR_ERR_CHECK(mpi_errno);
    }

    void *tmp = (void *)(tmp_buf + recvtype_true_lb);
    MPL_free(tmp);

fn_fail:
    MPIR_TIMER_END(coll, allgather, bruck);
    return (mpi_errno);
}
