#include "allreduce_tuning.h"
#include "bcast_tuning.h"
#include "reduce_tuning.h"

extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allreduce_shmem;

extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_shmem;

int MPIR_Allreduce_shmem_MVP(const void *sendbuf, void *recvbuf, int count,
                             MPI_Datatype datatype, MPI_Op op,
                             MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, allreduce, shmem);
    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_allreduce_shmem, 1);
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    int i = 0, is_commutative = 0;
    MPI_Aint true_lb, true_extent, extent;
    MPI_User_function *uop;
#ifdef HAVE_CXX_BINDING
    int is_cxx_uop = 0;
#endif
    char *shmem_buf = NULL;
    MPI_Comm shmem_comm = MPI_COMM_NULL, leader_comm = MPI_COMM_NULL;
    MPIR_Comm *shmem_commptr = NULL, *leader_commptr = NULL;
    int local_rank = -1, local_size = 0;
    void *local_buf = NULL;
    int stride = 0;
    is_commutative = 0;
    int total_size, shmem_comm_rank;

    if (count == 0) {
        return MPI_SUCCESS;
    }

    MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);
    MPIR_Datatype_get_extent_macro(datatype, extent);
    stride = count * MPL_MAX(extent, true_extent);

    /* Get the operator and check whether it is commutative or not */
    MPIR_get_op_ptr(&uop, &is_commutative,
#ifdef HAVE_CXX_BINDING
                    &is_cxx_uop,
#endif
                    op);

    total_size = comm_ptr->local_size;
    shmem_comm = comm_ptr->dev.ch.shmem_comm;
    MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
    local_rank = shmem_commptr->rank;
    local_size = shmem_commptr->local_size;
    shmem_comm_rank = shmem_commptr->dev.ch.shmem_comm_rank;

    leader_comm = comm_ptr->dev.ch.leader_comm;
    MPIR_Comm_get_ptr(leader_comm, leader_commptr);

    if (local_rank == 0) {
        if (sendbuf != MPI_IN_PLACE) {
            mpi_errno = MPIR_Localcopy(sendbuf, count, datatype, recvbuf, count,
                                       datatype);
            MPIR_ERR_CHKANDJUMP((mpi_errno), mpi_errno, MPI_ERR_OTHER,
                                "**fail");
        }
    }

    /* Doing the shared memory gather and reduction by the leader */
    if (local_rank == 0) {
        if (stride <= MVP_SHMEM_ALLREDUCE_MSG) {
            /* Message size is smaller than the shmem_reduce threshold.
             * The intra-node communication is done through shmem */
            if (local_size > 1) {
                /* Node leader waits till all the non-leaders have written
                 * the data into the shmem buffer */
                MPIR_MVP_SHMEM_COLL_GetShmemBuf(local_size, local_rank,
                                                shmem_comm_rank,
                                                (void *)&shmem_buf);
                if (is_commutative) {
                    for (i = 1; i < local_size; i++) {
                        local_buf = (char *)shmem_buf + stride * i;

                        MPIR_MVP_Reduce_local(local_buf, recvbuf,
                                              (MPI_Aint)count, datatype, uop
#ifdef HAVE_CXX_BINDING
                                              ,
                                              is_cxx_uop
#endif
                        );
                    }
                    MPIR_MVP_SHMEM_COLL_SetGatherComplete(
                        local_size, local_rank, shmem_comm_rank);
                }
            }
        } else {
            /* Message size is larger than the shmem_reduce threshold.
             * The leader will spend too much time doing the math operation
             * for messages that are larger. So, we use a point-to-point
             * based reduce to balance the load across all the processes within
             * the same node*/
            mpi_errno = MPIR_Reduce_MVP(sendbuf, recvbuf, count, datatype, op,
                                        0, shmem_commptr, errflag);
            if (mpi_errno) {
                /* for communication errors,
                 * just record the error but continue */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
        }
        if (local_size != total_size) {
            mpi_errno =
                MPIR_Allreduce_MVP(MPI_IN_PLACE, recvbuf, count, datatype, op,
                                   leader_commptr, errflag);
            if (mpi_errno) {
                /* for communication errors,
                 * just record the error but continue */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
        }
    } else {
        if (stride <= MVP_SHMEM_ALLREDUCE_MSG) {
            MPIR_MVP_SHMEM_COLL_GetShmemBuf(
                local_size, local_rank, shmem_comm_rank, (void *)&shmem_buf);
            local_buf = (char *)shmem_buf + stride * local_rank;
            if (sendbuf != MPI_IN_PLACE) {
                mpi_errno = MPIR_Localcopy(sendbuf, count, datatype, local_buf,
                                           count, datatype);
            } else {
                mpi_errno = MPIR_Localcopy(recvbuf, count, datatype, local_buf,
                                           count, datatype);
            }
            MPIR_ERR_CHKANDJUMP((mpi_errno), mpi_errno, MPI_ERR_OTHER,
                                "**fail");
            MPIR_MVP_SHMEM_COLL_SetGatherComplete(local_size, local_rank,
                                                  shmem_comm_rank);
        } else {
            if (sendbuf != MPI_IN_PLACE) {
                mpi_errno = MPIR_Reduce_MVP(sendbuf, recvbuf, count, datatype,
                                            op, 0, shmem_commptr, errflag);
            } else {
                /* MPI_Allreduce was called with MPI_IN_PLACE as the sendbuf.
                 * Since we are doing Reduce now, we need to be careful. In
                 * MPI_Reduce, only the root can use MPI_IN_PLACE as sendbuf.
                 * Also, the recvbuf is not relevant at all non-root processes*/
                mpi_errno = MPIR_Reduce_MVP(recvbuf, NULL, count, datatype, op,
                                            0, shmem_commptr, errflag);
            }
            if (mpi_errno) {
                /* for communication errors,
                 * just record the error but continue */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
        }
    }

    /* Broadcasting the message from leader to the rest */
    /* Note: shared memory broadcast could improve the performance */
    if (local_size > 1) {
        MPIR_Bcast_MVP(recvbuf, count, datatype, 0, shmem_commptr, errflag);
        if (mpi_errno) {
            /* for communication errors, just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
    }

fn_exit:
    MPIR_TIMER_END(coll, allreduce, shmem);
    return (mpi_errno);

fn_fail:
    goto fn_exit;
}
