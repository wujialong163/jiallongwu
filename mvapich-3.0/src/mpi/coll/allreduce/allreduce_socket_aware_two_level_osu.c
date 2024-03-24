#include "allreduce_tuning.h"
#include "reduce_tuning.h"
#include "bcast_tuning.h"
#include "mvp_coll_shmem.h"

#if defined(_SHARP_SUPPORT_)
#include "mvp_sharp.h"
#endif

int MPIR_Allreduce_socket_aware_two_level_old_MVP(
    const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
    MPI_Op op, MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    MPIR_Assert(comm_ptr->dev.ch.use_intra_sock_comm &&
                comm_ptr->dev.ch.shmem_coll_ok == 1);
    /*MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_allreduce_2lvl, 1);*/
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    MPI_Aint true_lb, true_extent;
    MPI_Comm shmem_comm = MPI_COMM_NULL, leader_comm = MPI_COMM_NULL;
    MPIR_Comm *shmem_commptr = NULL, *leader_commptr = NULL;
    int local_rank = -1;

    if (count == 0) {
        return MPI_SUCCESS;
    }
    MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);

    shmem_comm = comm_ptr->dev.ch.shmem_comm;
    MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
    local_rank = shmem_commptr->rank;

    leader_comm = comm_ptr->dev.ch.leader_comm;
    MPIR_Comm_get_ptr(leader_comm, leader_commptr);

    int intra_sock_rank = -1;

    MPIR_Comm *intra_sock_commptr;
    MPIR_Comm_get_ptr(shmem_commptr->dev.ch.intra_sock_comm,
                      intra_sock_commptr);
    intra_sock_rank = intra_sock_commptr->rank;

    /* Step 1. Socket leaders do an intra-socket reduce using shared memory*/

    if (intra_sock_rank == 0) {
        /* root ranks */
        if (MVP_USE_OPTIMIZED_RELEASE_ALLREDUCE) {
            mpi_errno = MPIR_Reduce_shmem_MVP_optrels(
                sendbuf, recvbuf, count, datatype, op, 0, intra_sock_commptr,
                errflag);
        } else {
            mpi_errno =
                MPIR_Reduce_shmem_MVP(sendbuf, recvbuf, count, datatype, op, 0,
                                      intra_sock_commptr, errflag);
        }
        if (mpi_errno) {
            /* for communication errors, just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }

    } else {
        /* non-root ranks */
        if (sendbuf != MPI_IN_PLACE) {
            if (MVP_USE_OPTIMIZED_RELEASE_ALLREDUCE) {
                mpi_errno = MPIR_Reduce_shmem_MVP_optrels(
                    sendbuf, recvbuf, count, datatype, op, 0,
                    intra_sock_commptr, errflag);
            } else {
                mpi_errno =
                    MPIR_Reduce_shmem_MVP(sendbuf, recvbuf, count, datatype, op,
                                          0, intra_sock_commptr, errflag);
            }
        } else {
            if (MVP_USE_OPTIMIZED_RELEASE_ALLREDUCE) {
                mpi_errno = MPIR_Reduce_shmem_MVP_optrels(
                    recvbuf, NULL, count, datatype, op, 0, intra_sock_commptr,
                    errflag);
            } else {
                mpi_errno =
                    MPIR_Reduce_shmem_MVP(recvbuf, NULL, count, datatype, op, 0,
                                          intra_sock_commptr, errflag);
            }
        }

        if (mpi_errno) {
            /* for communication errors, just record the error but continue */
            *errflag = TRUE;
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
    }

    /* Step 2. Socket level leaders within the node do an intra-node reduce to
     * rank 0*/

    MPIR_Comm *shmem_leader_commptr = NULL;
    MPIR_Comm_get_ptr(shmem_commptr->dev.ch.intra_sock_leader_comm,
                      shmem_leader_commptr);

    if (intra_sock_rank == 0 && shmem_leader_commptr->local_size > 1) {
        // Fall back to binomial if shmem coll not ok for shmem leaders
        if (shmem_leader_commptr->dev.ch.shmem_coll_ok != 1) {
            mpi_errno =
                MPIR_Reduce_binomial_MVP(MPI_IN_PLACE, recvbuf, count, datatype,
                                         op, 0, shmem_leader_commptr, errflag);
            if (mpi_errno) {
                /* for communication errors, just record the error but continue
                 */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
        } else {
            MPIR_Comm *shmem_leader_shmemcomm = NULL;
            MPIR_Comm_get_ptr(shmem_leader_commptr->dev.ch.shmem_comm,
                              shmem_leader_shmemcomm);
            int i, stride, leader_rank, leader_size, leader_shmem_comm_rank;
            MPI_User_function *uop;
            int is_commutative;
            char *shmem_buf = NULL;
            void *local_buf = NULL;
            MPI_Aint true_lb, true_extent, extent;
#ifdef HAVE_CXX_BINDING
            int is_cxx_uop = 0;
#endif

            MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);
            MPIR_Datatype_get_extent_macro(datatype, extent);
            stride = count * MPL_MAX(extent, true_extent);

            leader_rank = shmem_leader_shmemcomm->rank;
            leader_size = shmem_leader_shmemcomm->local_size;
            leader_shmem_comm_rank =
                shmem_leader_shmemcomm->dev.ch.shmem_comm_rank;

            /* Get the operator and check whether it is commutative or not */
            MPIR_get_op_ptr(&uop, &is_commutative,
#ifdef HAVE_CXX_BINDING
                            &is_cxx_uop,
#endif
                            op);

            if (leader_rank == 0) {
                MPIR_MVP_SHMEM_COLL_GetShmemBuf(leader_size, leader_rank,
                                                leader_shmem_comm_rank,
                                                (void *)&shmem_buf);
                for (i = 1; i < leader_size; i++) {
                    local_buf = (char *)shmem_buf + stride * i;

                    MPIR_MVP_Reduce_local(local_buf, recvbuf, (MPI_Aint)count,
                                          datatype, uop
#ifdef HAVE_CXX_BINDING
                                          ,
                                          is_cxx_uop
#endif
                    );
                }
                MPIR_MVP_SHMEM_COLL_SetGatherComplete(leader_size, leader_rank,
                                                      leader_shmem_comm_rank);
            } else {
                MPIR_MVP_SHMEM_COLL_GetShmemBuf(leader_size, leader_rank,
                                                leader_shmem_comm_rank,
                                                (void *)&shmem_buf);
                local_buf = (char *)shmem_buf + stride * leader_rank;
                mpi_errno = MPIR_Localcopy(recvbuf, count, datatype, local_buf,
                                           count, datatype);
                MPIR_ERR_CHECK(mpi_errno);
                MPIR_MVP_SHMEM_COLL_SetGatherComplete(leader_size, leader_rank,
                                                      leader_shmem_comm_rank);
            }
        }
    }

    /* Step 3. Leaders across nodes do an inter-node allreduce */

    if (local_rank == 0 && leader_commptr->local_size > 1) {
        mpi_errno =
            MPIR_Allreduce_pt2pt_rd_MVP(MPI_IN_PLACE, recvbuf, count, datatype,
                                        op, leader_commptr, errflag);
        if (mpi_errno) {
            /* for communication errors, just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
    }

    /* Step 4. Intra-node Bcast on shmem comm from root (rank 0) */

    mpi_errno = MPIR_Shmem_Bcast_MVP(recvbuf, count, datatype, 0, shmem_commptr,
                                     errflag);

    if (mpi_errno) {
        *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
        MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
    }

fn_exit:
    return (mpi_errno);

fn_fail:
    goto fn_exit;
}

int MPIR_Allreduce_socket_aware_two_level_MVP(const void *sendbuf,
                                              void *recvbuf, int count,
                                              MPI_Datatype datatype, MPI_Op op,
                                              MPIR_Comm *comm_ptr,
                                              MPIR_Errflag_t *errflag)
{
    MPIR_Assert(comm_ptr->dev.ch.use_intra_sock_comm &&
                comm_ptr->dev.ch.shmem_coll_ok == 1);

    if (!MVP_USE_SLOT_SHMEM_COLL) {
        return MPIR_Allreduce_socket_aware_two_level_old_MVP(
            sendbuf, recvbuf, count, datatype, op, comm_ptr, errflag);
    }

    /*MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_allreduce_2lvl, 1);*/
    shmem_info_t *shmem = NULL;
    int mpi_errno = MPI_SUCCESS, mpi_errno_ret = MPI_SUCCESS;
    MPI_Aint true_lb = 0, true_extent = 0, extent = 0;
    void *in_buf = NULL, *buf = NULL;
    int local_rank = -1, is_cxx_uop = 0, rindex = 0, len = 0;
    int is_commutative;
    MPI_User_function *uop = NULL;
    MPI_Comm shmem_comm = MPI_COMM_NULL, leader_comm = MPI_COMM_NULL;
    MPIR_Comm *shmem_commptr = NULL, *leader_commptr = NULL;
    MPIR_Comm *intra_sock_commptr = NULL, *shmem_leader_commptr = NULL;

    MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);
    MPIR_Datatype_get_extent_macro(datatype, extent);
    len = count * MPL_MAX(extent, true_extent);

    if (count == 0) {
        return MPI_SUCCESS;
    }
    /* Get details shmem_comm */
    shmem_comm = comm_ptr->dev.ch.shmem_comm;
    MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
    local_rank = shmem_commptr->rank;
    /* Get details leader_comm */
    leader_comm = comm_ptr->dev.ch.leader_comm;
    MPIR_Comm_get_ptr(leader_comm, leader_commptr);
    /* Get details intra_sock_comm */
    MPIR_Comm_get_ptr(shmem_commptr->dev.ch.intra_sock_comm,
                      intra_sock_commptr);

    /* Get the operator and check whether it is commutative or not */
    MPIR_get_op_ptr(&uop, &is_commutative,
#ifdef HAVE_CXX_BINDING
                    &is_cxx_uop,
#endif
                    op);

    /* Get details of the datatype */
    MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);
    MPIR_Datatype_get_extent_macro(datatype, extent);

    if (sendbuf != MPI_IN_PLACE) {
        in_buf = (void *)sendbuf;
    } else {
        in_buf = recvbuf;
    }

    /* Step 1. intra-socket reduce using shared memory */
    /* Get shmem region */
    shmem = intra_sock_commptr->dev.ch.shmem_info;
    /* Get rindex */
    rindex = shmem->read % MVP_SHMEM_COLL_WINDOW_SIZE;
    buf = shmem->queue[shmem->local_rank].shm_slots[rindex]->buf;
    mvp_shm_tree_reduce(shmem, in_buf, len, count, 0, uop, datatype,
                        is_cxx_uop);
    shmem->write++;
    shmem->read++;
    if (IS_SHMEM_WINDOW_FULL(shmem->write, shmem->tail)) {
        mvp_shm_barrier(shmem);
        shmem->tail = shmem->read;
    }
    /* in_buf contains the result of the operation */
    in_buf = buf;

    /* Step 2. Socket level leaders within the node
     * do an intra-node reduce to rank 0*/
    MPIR_Comm_get_ptr(shmem_commptr->dev.ch.intra_sock_leader_comm,
                      shmem_leader_commptr);

    if (intra_sock_commptr->rank == 0 && shmem_leader_commptr != NULL &&
        shmem_leader_commptr->local_size > 1) {
        /* Fall back to binomial if shmem coll not ok for shmem leaders */
        if (shmem_leader_commptr->dev.ch.shmem_coll_ok != 1) {
            mpi_errno =
                MPIR_Reduce_binomial_MVP(MPI_IN_PLACE, in_buf, count, datatype,
                                         op, 0, shmem_leader_commptr, errflag);
            if (mpi_errno) {
                /* for communication errors,
                 * just record the error but continue */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
        } else {
            /* Get shmem region */
            shmem = shmem_leader_commptr->dev.ch.shmem_info;
            /* Get rindex */
            rindex = shmem->read % MVP_SHMEM_COLL_WINDOW_SIZE;
            buf = shmem->queue[shmem->local_rank].shm_slots[rindex]->buf;
            mvp_shm_tree_reduce(shmem, in_buf, len, count, 0, uop, datatype,
                                is_cxx_uop);
            shmem->write++;
            shmem->read++;
            if (IS_SHMEM_WINDOW_FULL(shmem->write, shmem->tail)) {
                mvp_shm_barrier(shmem);
                shmem->tail = shmem->read;
            }
            in_buf = buf;
        }
    }

    /* Step 3. Leaders across nodes do an inter-node allreduce */
    if (local_rank == 0 && leader_commptr->local_size > 1) {
#if defined(_SHARP_SUPPORT_)
        MPI_Aint type_size = 0;
        /* Get size of data */
        MPIR_Datatype_get_size_macro(datatype, type_size);
        intptr_t nbytes = (intptr_t)(count) * (type_size);
        if (comm_ptr->dev.ch.is_sharp_ok == 1 &&
            nbytes <= MVP_SHARP_MAX_MSG_SIZE && MVP_ENABLE_SHARP == 1 &&
            MVP_ENABLE_SHARP_ALLREDUCE) {
            mpi_errno = MPIR_Sharp_Allreduce_MVP(
                MPI_IN_PLACE, in_buf, count, datatype, op, comm_ptr, errflag);
            if (mpi_errno != MPI_SUCCESS) {
                /* fall back to RD algorithm if SHArP is not supported */
                mpi_errno = MPIR_Allreduce_pt2pt_rd_MVP(
                    MPI_IN_PLACE, in_buf, count, datatype, op, leader_commptr,
                    errflag);
                if (mpi_errno) {
                    /* for communication errors,
                     * just record the error but continue */
                    *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                    MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                }
            }
        } else
#endif /* #if defined (_SHARP_SUPPORT_) */
        {
            mpi_errno = MPIR_Allreduce_pt2pt_rd_MVP(MPI_IN_PLACE, in_buf, count,
                                                    datatype, op,
                                                    leader_commptr, errflag);
            if (mpi_errno) {
                /* for communication errors,
                 * just record the error but continue */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
        }
    }

    /* Step 4. Inter-socket-leader Bcast on shmem_leader_comm */
    if (intra_sock_commptr->rank == 0 && shmem_leader_commptr != NULL &&
        shmem_leader_commptr->local_size > 1) {
        /* Fall back to binomial if shmem coll not ok for shmem leaders */
        if (shmem_leader_commptr->dev.ch.shmem_coll_ok != 1) {
            mpi_errno = MPIR_Bcast_binomial_MVP(in_buf, count, datatype, 0,
                                                shmem_leader_commptr, errflag);
            if (mpi_errno) {
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
        } else {
            MPIR_Shmem_Bcast_MVP(in_buf, count, datatype, 0,
                                 shmem_leader_commptr, errflag);
        }
    }

    /* Step 5. Intra-socket Bcast on intra_sock_comm */
    MPIR_Shmem_Bcast_MVP(in_buf, count, datatype, 0, intra_sock_commptr,
                         errflag);

    MPIR_Localcopy(in_buf, count, datatype, recvbuf, count, datatype);

fn_exit:
    return (mpi_errno);

fn_fail:
    goto fn_exit;
}
