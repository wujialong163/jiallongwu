#include "mpiimpl.h"
#include "allreduce_tuning.h"

extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allreduce_shm_rd;

extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_shm_rd;

extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_pt2pt_rd_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_pt2pt_rd_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_pt2pt_rd_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_pt2pt_rd_count_recv;

int MPIR_Allreduce_pt2pt_rd_MVP(const void *sendbuf, void *recvbuf, int count,
                                MPI_Datatype datatype, MPI_Op op,
                                MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, allreduce, shm_rd);
    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_allreduce_shm_rd, 1);
    int comm_size, rank;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    int mask, dst, is_commutative, pof2, newrank = 0, rem, newdst;
    MPI_Aint true_lb, true_extent, extent;
    void *tmp_buf;
    MPI_User_function *uop;
#ifdef HAVE_CXX_BINDING
    int is_cxx_uop = 0;
#endif
    MPIR_CHKLMEM_DECL(3);

    if (count == 0) {
        MPIR_TIMER_END(coll, allreduce, shm_rd);
        return MPI_SUCCESS;
    }

    /* homogeneous */

    comm_size = comm_ptr->local_size;
    rank = comm_ptr->rank;

    /* Get func ptr for the reduction op
     * and initialize is_commutative and is_cxx_uop */
    MPIR_get_op_ptr(&uop, &is_commutative,
#ifdef HAVE_CXX_BINDING
                    &is_cxx_uop,
#endif
                    op);

    /* need to allocate temporary buffer to store incoming data */
    MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);
    MPIR_Datatype_get_extent_macro(datatype, extent);

    if ((comm_ptr->dev.ch.shmem_coll_ok == 1) &&
        ((count * (MPL_MAX(extent, true_extent)) < MVP_COLL_TMP_BUF_SIZE))) {
        tmp_buf = (void *)((char *)comm_ptr->dev.ch.coll_tmp_buf);
    } else {
        MPIR_CHKLMEM_MALLOC(tmp_buf, void *,
                            count *(MPL_MAX(extent, true_extent)), mpi_errno,
                            "temporary buffer", MPL_MEM_COLL);
    }
    /* adjust for potential negative lower bound in datatype */
    tmp_buf = (void *)((char *)tmp_buf - true_lb);

    /* copy local data into recvbuf */
    if (sendbuf != MPI_IN_PLACE) {
        mpi_errno =
            MPIR_Localcopy(sendbuf, count, datatype, recvbuf, count, datatype);
        MPIR_ERR_CHKANDJUMP((mpi_errno), mpi_errno, MPI_ERR_OTHER, "**fail");
    }

    /* find nearest power-of-two less than or equal to comm_size */
    pof2 = comm_ptr->dev.ch.gpof2;

    rem = comm_size - pof2;

    /* In the non-power-of-two case, all even-numbered
     *  processes of rank < 2*rem send their data to
     *  (rank+1). These even-numbered processes no longer
     *  participate in the algorithm until the very end. The
     *  remaining processes form a nice power-of-two. */

    if (rank < 2 * rem) {
        if (rank % 2 == 0) {
            /* even */
            MPIR_PVAR_INC(allreduce, pt2pt_rd, send, count, datatype);
            mpi_errno = MPIC_Send(recvbuf, count, datatype, rank + 1,
                                  MPIR_ALLREDUCE_TAG, comm_ptr, errflag);
            if (mpi_errno) {
                /* for communication errors,
                 * just record the error but continue */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }

            /* temporarily set the rank to -1 so that this
             * process does not pariticipate in recursive
             * doubling */
            newrank = -1;
        } else {
            /* odd */
            MPIR_PVAR_INC(allreduce, pt2pt_rd, recv, count, datatype);
            mpi_errno = MPIC_Recv(tmp_buf, count, datatype, rank - 1,
                                  MPIR_ALLREDUCE_TAG, comm_ptr,
                                  MPI_STATUS_IGNORE, errflag);
            if (mpi_errno) {
                /* for communication errors,
                 * just record the error but continue */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }

            /* do the reduction on received data. since the
             * ordering is right, it doesn't matter whether
             * the operation is commutative or not. */
            MPIR_MVP_Reduce_local(tmp_buf, recvbuf, (MPI_Aint)count, datatype,
                                  uop
#ifdef HAVE_CXX_BINDING
                                  ,
                                  is_cxx_uop
#endif
            );

            /* change the rank */
            newrank = rank / 2;
        }
    } else { /* rank >= 2*rem */
        newrank = rank - rem;
    }

    /* If op is user-defined or count is less than pof2, use
     * recursive doubling algorithm. Otherwise do a reduce-scatter
     * followed by allgather. (If op is user-defined,
     * derived datatypes are allowed and the user could pass basic
     * datatypes on one process and derived on another
     * as long as the type maps are the same. Breaking up derived
     * datatypes to do the reduce-scatter is tricky, therefore
     * using recursive doubling in that case.) */

    if (newrank != -1) {
        mask = 0x1;
        while (mask < pof2) {
            newdst = newrank ^ mask;
            /* find real rank of dest */
            dst = (newdst < rem) ? newdst * 2 + 1 : newdst + rem;

            /* Send the most current data, which is in recvbuf.
             * Recv into tmp_buf */
            MPIR_PVAR_INC(allreduce, pt2pt_rd, send, count, datatype);
            MPIR_PVAR_INC(allreduce, pt2pt_rd, recv, count, datatype);
            mpi_errno =
                MPIC_Sendrecv(recvbuf, count, datatype, dst, MPIR_ALLREDUCE_TAG,
                              tmp_buf, count, datatype, dst, MPIR_ALLREDUCE_TAG,
                              comm_ptr, MPI_STATUS_IGNORE, errflag);

            if (mpi_errno) {
                /* for communication errors,
                 * just record the error but continue */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }

            /* tmp_buf contains data received in this step.
             * recvbuf contains data accumulated so far */

            if (is_commutative || (dst < rank)) {
                /* op is commutative OR the order is already right */
                MPIR_MVP_Reduce_local(tmp_buf, recvbuf, (MPI_Aint)count,
                                      datatype, uop
#ifdef HAVE_CXX_BINDING
                                      ,
                                      is_cxx_uop
#endif
                );
            } else {
                /* op is noncommutative and the order is not right */
                MPIR_MVP_Reduce_local(recvbuf, tmp_buf, (MPI_Aint)count,
                                      datatype, uop
#ifdef HAVE_CXX_BINDING
                                      ,
                                      is_cxx_uop
#endif
                );

                /* copy result back into recvbuf */
                mpi_errno = MPIR_Localcopy(tmp_buf, count, datatype, recvbuf,
                                           count, datatype);
                MPIR_ERR_CHKANDJUMP((mpi_errno), mpi_errno, MPI_ERR_OTHER,
                                    "**fail");
            }
            mask <<= 1;
        }
    }

    /* In the non-power-of-two case, all odd-numbered
     * processes of rank < 2*rem send the result to
     * (rank-1), the ranks who didn't participate above. */
    if (rank < 2 * rem) {
        if (rank % 2) { /* odd */
            MPIR_PVAR_INC(allreduce, pt2pt_rd, send, count, datatype);
            mpi_errno = MPIC_Send(recvbuf, count, datatype, rank - 1,
                                  MPIR_ALLREDUCE_TAG, comm_ptr, errflag);
        } else { /* even */
            MPIR_PVAR_INC(allreduce, pt2pt_rd, recv, count, datatype);
            mpi_errno = MPIC_Recv(recvbuf, count, datatype, rank + 1,
                                  MPIR_ALLREDUCE_TAG, comm_ptr,
                                  MPI_STATUS_IGNORE, errflag);
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
    MPIR_TIMER_END(coll, allreduce, shm_rd);
    return (mpi_errno);

fn_fail:
    goto fn_exit;
}
