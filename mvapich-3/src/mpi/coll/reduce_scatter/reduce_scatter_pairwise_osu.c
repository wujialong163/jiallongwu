#include "reduce_scatter_tuning.h"

int MPIR_Reduce_scatter_Pair_Wise_MVP(const void *sendbuf, void *recvbuf,
                                      const int *recvcnts,
                                      MPI_Datatype datatype, MPI_Op op,
                                      MPIR_Comm *comm_ptr,
                                      MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, reduce_scatter, pairwise);
    int rank, comm_size, i;
    MPI_Aint extent, true_extent, true_lb;
    int *disps;
    void *tmp_recvbuf;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    int total_count, src, dst;
    int is_commutative;
    MPI_User_function *uop;
#ifdef HAVE_CXX_BINDING
    int is_cxx_uop = 0;
#endif
    MPIR_CHKLMEM_DECL(5);

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_reduce_scatter_pairwise, 1);

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

    /* commutative and long message, or noncommutative and long message.
       use (p-1) pairwise exchanges */

    if (sendbuf != MPI_IN_PLACE) {
        /* copy local data into recvbuf */
        mpi_errno = MPIR_Localcopy(((char *)sendbuf + disps[rank] * extent),
                                   recvcnts[rank], datatype, recvbuf,
                                   recvcnts[rank], datatype);
        MPIR_ERR_CHECK(mpi_errno);
    }

    /* allocate temporary buffer to store incoming data */
    MPIR_CHKLMEM_MALLOC(tmp_recvbuf, void *,
                        recvcnts[rank] * (MPL_MAX(true_extent, extent)) + 1,
                        mpi_errno, "tmp_recvbuf", MPL_MEM_COLL);
    /* adjust for potential negative lower bound in datatype */
    tmp_recvbuf = (void *)((char *)tmp_recvbuf - true_lb);

    for (i = 1; i < comm_size; i++) {
        src = (rank - i + comm_size) % comm_size;
        dst = (rank + i) % comm_size;

        /* send the data that dst needs. recv data that this process
           needs from src into tmp_recvbuf */
        if (sendbuf != MPI_IN_PLACE) {
            MPIR_PVAR_INC(reduce_scatter, pairwise, send, recvcnts[dst],
                          datatype);
            MPIR_PVAR_INC(reduce_scatter, pairwise, recv, recvcnts[rank],
                          datatype);
            mpi_errno = MPIC_Sendrecv(
                ((char *)sendbuf + disps[dst] * extent), recvcnts[dst],
                datatype, dst, MPIR_REDUCE_SCATTER_TAG, tmp_recvbuf,
                recvcnts[rank], datatype, src, MPIR_REDUCE_SCATTER_TAG,
                comm_ptr, MPI_STATUS_IGNORE, errflag);
        } else {
            MPIR_PVAR_INC(reduce_scatter, pairwise, send, recvcnts[dst],
                          datatype);
            MPIR_PVAR_INC(reduce_scatter, pairwise, recv, recvcnts[rank],
                          datatype);
            mpi_errno = MPIC_Sendrecv(
                ((char *)recvbuf + disps[dst] * extent), recvcnts[dst],
                datatype, dst, MPIR_REDUCE_SCATTER_TAG, tmp_recvbuf,
                recvcnts[rank], datatype, src, MPIR_REDUCE_SCATTER_TAG,
                comm_ptr, MPI_STATUS_IGNORE, errflag);
        }
        if (mpi_errno) {
            /* for communication errors,
             * just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }

        if (is_commutative || (src < rank)) {
            if (sendbuf != MPI_IN_PLACE) {
                MPIR_MVP_Reduce_local(tmp_recvbuf, recvbuf,
                                      (MPI_Aint)recvcnts[rank], datatype, uop
#ifdef HAVE_CXX_BINDING
                                      ,
                                      is_cxx_uop
#endif
                );
            } else {
                MPIR_MVP_Reduce_local(tmp_recvbuf,
                                      ((char *)recvbuf + disps[rank] * extent),
                                      (MPI_Aint)recvcnts[rank], datatype, uop
#ifdef HAVE_CXX_BINDING
                                      ,
                                      is_cxx_uop
#endif
                );

                /* we can't store the result at the beginning of
                 * recvbuf right here because
                 * there is useful data
                 * there that other process/processes need.
                 * at the end,
                 * we will copy back the result to the
                 * beginning of recvbuf. */
            }
        } else {
            if (sendbuf != MPI_IN_PLACE) {
                MPIR_MVP_Reduce_local(recvbuf, tmp_recvbuf,
                                      (MPI_Aint)recvcnts[rank], datatype, uop
#ifdef HAVE_CXX_BINDING
                                      ,
                                      is_cxx_uop
#endif
                );

                /* copy result back into recvbuf */
                mpi_errno =
                    MPIR_Localcopy(tmp_recvbuf, recvcnts[rank], datatype,
                                   recvbuf, recvcnts[rank], datatype);
                MPIR_ERR_CHECK(mpi_errno);
            } else {
                MPIR_MVP_Reduce_local(((char *)recvbuf + disps[rank] * extent),
                                      tmp_recvbuf, (MPI_Aint)recvcnts[rank],
                                      datatype, uop
#ifdef HAVE_CXX_BINDING
                                      ,
                                      is_cxx_uop
#endif
                );

                /* copy result back into recvbuf */
                mpi_errno =
                    MPIR_Localcopy(tmp_recvbuf, recvcnts[rank], datatype,
                                   ((char *)recvbuf + disps[rank] * extent),
                                   recvcnts[rank], datatype);
                MPIR_ERR_CHECK(mpi_errno);
            }
        }
    }

    /* if MPI_IN_PLACE, move output data to the beginning of
       recvbuf. already done for rank 0. */
    if ((sendbuf == MPI_IN_PLACE) && (rank != 0)) {
        mpi_errno = MPIR_Localcopy(((char *)recvbuf + disps[rank] * extent),
                                   recvcnts[rank], datatype, recvbuf,
                                   recvcnts[rank], datatype);
        MPIR_ERR_CHECK(mpi_errno);
    }

fn_exit:
    MPIR_CHKLMEM_FREEALL();

    if (mpi_errno_ret)
        mpi_errno = mpi_errno_ret;
    else if (*errflag)
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**coll_fail");

    MPIR_TIMER_END(coll, reduce_scatter, pairwise);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
