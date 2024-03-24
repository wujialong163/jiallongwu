#include "reduce_tuning.h"

int MPIR_Reduce_allreduce_MVP(const void *sendbuf, void *recvbuf, int count,
                              MPI_Datatype datatype, MPI_Op op, int root,
                              MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, reduce, allreduce);
    MPIR_RECURSION_GUARD_DECL(MVP_Reduce_fn_t, &MPIR_Reduce_binomial_MVP);
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    int rank;
    MPI_Aint true_lb, true_extent, extent;
    void *tmp_buf = NULL;
    size_t recvbuf_size = 0;
    int using_comm_buf = 0;

    MPIR_RECURSION_GUARD_ENTER(sendbuf, recvbuf, count, datatype, op, root,
                               comm_ptr, errflag);

    if (count == 0) {
        MPIR_TIMER_END(coll, reduce, allreduce);
        return MPI_SUCCESS;
    }

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_reduce_allreduce, 1);

    rank = comm_ptr->rank;

    MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);
    MPIR_Datatype_get_extent_macro(datatype, extent);

    recvbuf_size = count * MPL_MAX(extent, true_extent);

    /* Call allreduce and discard the result on all non-root processes */
    if (comm_ptr->dev.ch.shmem_coll_ok == 1 &&
        recvbuf_size <= MVP_COLL_TMP_BUF_SIZE) {
        tmp_buf = comm_ptr->dev.ch.coll_tmp_buf;
        using_comm_buf = 1;
    } else {
        if (rank != root) {
            tmp_buf = MPL_malloc(recvbuf_size, MPL_MEM_BUFFER);
        }
    }

    if (rank != root) {
        mpi_errno = MPIR_Allreduce_MVP(sendbuf, tmp_buf, count, datatype, op,
                                       comm_ptr, errflag);
    } else {
        mpi_errno = MPIR_Allreduce_MVP(sendbuf, recvbuf, count, datatype, op,
                                       comm_ptr, errflag);
    }

    MPIR_ERR_CHECK(mpi_errno);

    if (!using_comm_buf && rank != root) {
        MPL_free(tmp_buf);
    }

fn_exit:
    MPIR_RECURSION_GUARD_EXIT;
    if (mpi_errno_ret)
        mpi_errno = mpi_errno_ret;
    else if (*errflag)
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**coll_fail");

    MPIR_TIMER_END(coll, reduce, allreduce);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
