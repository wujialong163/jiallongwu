#include "allreduce_tuning.h"
#include "bcast_tuning.h"
#include "mpiimpl.h"

extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allreduce_shm_intra;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allreduce_intra_p2p;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allreduce_2lvl;

extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_shm_intra;
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_intra_p2p;
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_2lvl;

MVP_Allreduce_fn_t MVP_Allreduce_function = NULL;
MVP_Allreduce_fn_t MVP_Allreduce_intra_function = NULL;

/* intra-node shm reduce as the first reduce in allreduce */
int MPIR_Allreduce_reduce_shmem_MVP(const void *sendbuf, void *recvbuf,
                                    int count, MPI_Datatype datatype, MPI_Op op,
                                    MPIR_Comm *comm_ptr,
                                    MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, allreduce, shm_intra);
    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_allreduce_shm_intra, 1);
    int mpi_errno = MPI_SUCCESS;
    int i = 0, is_commutative = 0;
    MPI_Aint true_lb, true_extent, extent;
    MPI_User_function *uop;
#ifdef HAVE_CXX_BINDING
    int is_cxx_uop = 0;
#endif
    char *shmem_buf = NULL;
    MPI_Comm shmem_comm = MPI_COMM_NULL;
    MPIR_Comm *shmem_commptr = NULL;
    int local_rank = -1, local_size = 0;
    void *local_buf = NULL;
    int stride = 0;
    is_commutative = 0;
    int shmem_comm_rank;

    if (count == 0) {
        MPIR_TIMER_END(coll, allreduce, shm_intra);
        return MPI_SUCCESS;
    }

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_num_shmem_coll_calls, 1);
    MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);
    MPIR_Datatype_get_extent_macro(datatype, extent);
    stride = count * MPL_MAX(extent, true_extent);

    shmem_comm = comm_ptr->dev.ch.shmem_comm;
    MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
    local_size = MPIR_Comm_size(shmem_commptr);
    if (count * (MPL_MAX(extent, true_extent)) >= MVP_SHMEM_COLL_MAX_MSG_SIZE) {
        mpi_errno = MPIR_Reduce_intra_smp(sendbuf, recvbuf, count, datatype, op,
                                          0, shmem_commptr, errflag);
        goto fn_exit;
    }

    /* Get the operator and check whether it is commutative or not */
    MPIR_get_op_ptr(&uop, &is_commutative,
#ifdef HAVE_CXX_BINDING
                    &is_cxx_uop,
#endif
                    op);

    local_rank = shmem_commptr->rank;
    local_size = shmem_commptr->local_size;
    shmem_comm_rank = shmem_commptr->dev.ch.shmem_comm_rank;

    /* Doing the shared memory gather and reduction by the leader */
    if (local_rank == 0) {
        /* Message size is smaller than the shmem_reduce threshold.
         * The intra-node communication is done through shmem */
        if (local_size > 1) {
            /* Node leader waits till all the non-leaders have written
             * the data into the shmem buffer */
            MPIR_MVP_SHMEM_COLL_GetShmemBuf(
                local_size, local_rank, shmem_comm_rank, (void *)&shmem_buf);
            if (is_commutative) {
                for (i = 1; i < local_size; i++) {
                    local_buf = (char *)shmem_buf + stride * i;

                    MPIR_MVP_Reduce_local(local_buf, recvbuf, (MPI_Aint)count,
                                          datatype, uop
#ifdef HAVE_CXX_BINDING
                                          ,
                                          is_cxx_uop
#endif
                    );
                }
                MPIR_MVP_SHMEM_COLL_SetGatherComplete(local_size, local_rank,
                                                      shmem_comm_rank);
            }
        }
    } else {
        MPIR_MVP_SHMEM_COLL_GetShmemBuf(local_size, local_rank, shmem_comm_rank,
                                        (void *)&shmem_buf);
        local_buf = (char *)shmem_buf + stride * local_rank;
        if (sendbuf != MPI_IN_PLACE) {
            mpi_errno = MPIR_Localcopy(sendbuf, count, datatype, local_buf,
                                       count, datatype);
        } else {
            mpi_errno = MPIR_Localcopy(recvbuf, count, datatype, local_buf,
                                       count, datatype);
        }
        MPIR_ERR_CHKANDJUMP((mpi_errno), mpi_errno, MPI_ERR_OTHER, "**fail");
        MPIR_MVP_SHMEM_COLL_SetGatherComplete(local_size, local_rank,
                                              shmem_comm_rank);
    }

fn_exit:
    MPIR_TIMER_END(coll, allreduce, shm_intra);
    return (mpi_errno);

fn_fail:
    goto fn_exit;
}

/* intra-node p2p reduce as the first reduce in allreduce */
int MPIR_Allreduce_reduce_p2p_MVP(const void *sendbuf, void *recvbuf, int count,
                                  MPI_Datatype datatype, MPI_Op op,
                                  MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, allreduce, intra_p2p);
    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_allreduce_intra_p2p, 1);
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    MPI_Aint true_lb, true_extent;
    MPI_Comm shmem_comm = MPI_COMM_NULL;
    MPIR_Comm *shmem_commptr = NULL;
    int local_rank = -1, local_size = 0;

    if (count == 0) {
        MPIR_TIMER_END(coll, allreduce, intra_p2p);
        return MPI_SUCCESS;
    }

    MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);

    shmem_comm = comm_ptr->dev.ch.shmem_comm;
    MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
    local_rank = shmem_commptr->rank;
    local_size = shmem_commptr->local_size;

    /* Doing the shared memory gather and reduction by the leader */
    if (local_rank == 0) {
        /* Message size is larger than the shmem_reduce threshold.
         * The leader will spend too much time doing the math operation
         * for messages that are larger. So, we use a point-to-point
         * based reduce to balance the load across all the
         * processes within the same node*/
        mpi_errno = MPIR_Reduce_MVP(sendbuf, recvbuf, count, datatype, op, 0,
                                    shmem_commptr, errflag);
        if (mpi_errno) {
            /* for communication errors, just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }

    } else {
        if (sendbuf != MPI_IN_PLACE) {
            mpi_errno = MPIR_Reduce_MVP(sendbuf, recvbuf, count, datatype, op,
                                        0, shmem_commptr, errflag);
        } else {
            /* MPI_Allreduce was called with MPI_IN_PLACE as the sendbuf.
             * Since we are doing Reduce now, we need to be careful. In
             * MPI_Reduce, only the root can use MPI_IN_PLACE as sendbuf.
             * Also, the recvbuf is not relevant at
             * all non-root processes*/
            mpi_errno = MPIR_Reduce_MVP(recvbuf, NULL, count, datatype, op, 0,
                                        shmem_commptr, errflag);
        }
        if (mpi_errno) {
            /* for communication errors, just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
    }

    MPIR_TIMER_END(coll, allreduce, intra_p2p);
    return (mpi_errno);
}

/* general two level allreduce helper function */
int MPIR_Allreduce_two_level_MVP(const void *sendbuf, void *recvbuf, int count,
                                 MPI_Datatype datatype, MPI_Op op,
                                 MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, allreduce, 2lvl);
    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_allreduce_2lvl, 1);
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    int total_size = 0;
    MPI_Aint true_lb, true_extent;
    MPI_Comm shmem_comm = MPI_COMM_NULL, leader_comm = MPI_COMM_NULL;
    MPIR_Comm *shmem_commptr = NULL, *leader_commptr = NULL;
    int local_rank = -1, local_size = 0;

    if (count == 0) {
        MPIR_TIMER_END(coll, allreduce, 2lvl);
        return MPI_SUCCESS;
    }

    MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);

    total_size = comm_ptr->local_size;
    shmem_comm = comm_ptr->dev.ch.shmem_comm;
    MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
    local_rank = shmem_commptr->rank;
    local_size = shmem_commptr->local_size;

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
        if ((MVP_Allreduce_intra_function ==
             &MPIR_Allreduce_reduce_shmem_MVP) ||
            (MVP_Allreduce_intra_function == &MPIR_Allreduce_reduce_p2p_MVP)) {
            mpi_errno = MVP_Allreduce_intra_function(
                sendbuf, recvbuf, count, datatype, op, comm_ptr, errflag);
        } else {
            mpi_errno = MVP_Allreduce_intra_function(
                sendbuf, recvbuf, count, datatype, op, shmem_commptr, errflag);
        }

        if (mpi_errno) {
            /* for communication errors, just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }

        if (local_size != total_size) {
            /* inter-node allreduce */

            if (MVP_Allreduce_function == &MPIR_Allreduce_pt2pt_rd_MVP) {
                mpi_errno = MPIR_Allreduce_pt2pt_rd_MVP(
                    MPI_IN_PLACE, recvbuf, count, datatype, op, leader_commptr,
                    errflag);
            } else if (MVP_Allreduce_function ==
                       &MPIR_Allreduce_pt2pt_reduce_scatter_allgather_MVP) {
                mpi_errno = MPIR_Allreduce_pt2pt_reduce_scatter_allgather_MVP(
                    MPI_IN_PLACE, recvbuf, count, datatype, op, leader_commptr,
                    errflag);
            } else
#if defined(_SHARP_SUPPORT_)
                if (MVP_Allreduce_function == &MPIR_Sharp_Allreduce_MVP) {
                mpi_errno =
                    MPIR_Sharp_Allreduce_MVP(MPI_IN_PLACE, recvbuf, count,
                                             datatype, op, comm_ptr, errflag);
                if (mpi_errno != MPI_SUCCESS) {
                    /* fall back to RD algorithm if SHArP is not supported */
                    mpi_errno = MPIR_Allreduce_pt2pt_rd_MVP(
                        MPI_IN_PLACE, recvbuf, count, datatype, op,
                        leader_commptr, errflag);
                }
            } else
#endif
            {
                mpi_errno = MPIR_Allreduce_pt2pt_rs_MVP(
                    MPI_IN_PLACE, recvbuf, count, datatype, op, leader_commptr,
                    errflag);
            }

            if (mpi_errno) {
                /* for communication errors,
                 * just record the error but continue */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
        }
    } else {
        /* insert the first reduce here */
        if ((MVP_Allreduce_intra_function ==
             &MPIR_Allreduce_reduce_shmem_MVP) ||
            (MVP_Allreduce_intra_function == &MPIR_Allreduce_reduce_p2p_MVP)) {
            mpi_errno = MVP_Allreduce_intra_function(
                sendbuf, recvbuf, count, datatype, op, comm_ptr, errflag);
        } else {
            mpi_errno = MVP_Allreduce_intra_function(
                sendbuf, recvbuf, count, datatype, op, shmem_commptr, errflag);
        }
        if (mpi_errno) {
            /* for communication errors,
             * just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
    }

    /* Broadcasting the message from leader to the rest */
    mpi_errno = MPIR_Shmem_Bcast_MVP(recvbuf, count, datatype, 0, shmem_commptr,
                                     errflag);
    if (mpi_errno) {
        /* for communication errors, just record the error but continue */
        *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
        MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
    }

fn_exit:
    MPIR_TIMER_END(coll, allreduce, 2lvl);
    return (mpi_errno);

fn_fail:
    goto fn_exit;
}
