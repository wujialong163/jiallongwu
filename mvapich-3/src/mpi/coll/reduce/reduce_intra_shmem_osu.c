#include "reduce_tuning.h"

int MPIR_Reduce_tree_shmem_MVP_optrels(const void *sendbuf, void *recvbuf,
                                       int count, MPI_Datatype datatype,
                                       MPI_Op op, int root, MPIR_Comm *comm_ptr,
                                       MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, reduce, shmem);
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    int i, stride, local_rank, local_size, shmem_comm_rank;
    MPI_User_function *uop;
    int is_commutative;
    char *shmem_buf = NULL;
    void *local_buf = NULL;
    char *tmp_buf = NULL;
    int buf_allocated = 0;
    int parent;
    MPI_Aint true_lb, true_extent, extent;
#ifdef HAVE_CXX_BINDING
    int is_cxx_uop = 0;
#endif

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_reduce_shmem, 1);

    MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);
    MPIR_Datatype_get_extent_macro(datatype, extent);
    stride = count * MPL_MAX(extent, true_extent);

    local_rank = comm_ptr->rank;
    local_size = comm_ptr->local_size;
    shmem_comm_rank = comm_ptr->dev.ch.shmem_comm_rank;
    if (local_size < MVP_USE_SHMEM_NUM_TREES) {
        MVP_USE_SHMEM_NUM_TREES = 1;
    }

    /* For the processes that are doing the computation,
     * check if recvbuf is valid */
    if (sendbuf != MPI_IN_PLACE && (local_rank < MVP_USE_SHMEM_NUM_TREES)) {
        /* if local_rank == 0 and not root then the recvbuf may not be valid*/
        if (sendbuf == recvbuf || recvbuf == NULL) {
            tmp_buf = recvbuf;
            recvbuf =
                MPL_malloc(count * MPL_MAX(extent, true_extent), MPL_MEM_COLL);
            buf_allocated = 1;
        }
        mpi_errno =
            MPIR_Localcopy(sendbuf, count, datatype, recvbuf, count, datatype);
        MPIR_ERR_CHECK(mpi_errno);
    }

    if (local_size == 0) {
        /* Only one process. So, return */
        goto fn_exit;
    }

    /* Get func ptr for the reduction op
     * and initialize is_commutative and is_cxx_uop */
    MPIR_get_op_ptr(&uop, &is_commutative,
#ifdef HAVE_CXX_BINDING
                    &is_cxx_uop,
#endif
                    op);

    parent = local_rank % MVP_USE_SHMEM_NUM_TREES;
    if (local_rank < MVP_USE_SHMEM_NUM_TREES) {
        for (i = local_rank + MVP_USE_SHMEM_NUM_TREES; i < local_size;
             i = i + MVP_USE_SHMEM_NUM_TREES) {
            MPIR_MVP_SHMEM_TREE_COLL_GetShmemBuf_optrels(
                local_size, local_rank, shmem_comm_rank, i, (void *)&shmem_buf,
                local_rank);
            local_buf = (char *)shmem_buf + stride * i;
            MPIR_MVP_Reduce_local(local_buf, recvbuf, (MPI_Aint)count, datatype,
                                  uop
#ifdef HAVE_CXX_BINDING
                                  ,
                                  is_cxx_uop
#endif
            );

            MPIR_MVP_SHMEM_TREE_COLL_SetGatherComplete_optrels(
                local_size, local_rank, shmem_comm_rank, i, local_rank);
        }
    } else {
        MPIR_MVP_SHMEM_TREE_COLL_GetShmemBuf_optrels(
            local_size, local_rank, shmem_comm_rank, 0, (void *)&shmem_buf,
            parent);
        local_buf = (char *)shmem_buf + stride * local_rank;
        mpi_errno = MPIR_Localcopy(sendbuf, count, datatype, local_buf, count,
                                   datatype);
        MPIR_ERR_CHECK(mpi_errno);
        MPIR_MVP_SHMEM_TREE_COLL_SetGatherComplete_optrels(
            local_size, local_rank, shmem_comm_rank, 0, parent);
    }

    /* now, process 0 does the reduction between parents */
    if (local_rank == 0) {
        for (i = 1; i < MVP_USE_SHMEM_NUM_TREES; i++) {
            MPIR_MVP_SHMEM_TREE_COLL_GetShmemBuf_optrels(local_size, local_rank,
                                                         shmem_comm_rank, i,
                                                         (void *)&shmem_buf, 0);
            local_buf = (char *)shmem_buf + stride * i;
            MPIR_MVP_Reduce_local(local_buf, recvbuf, (MPI_Aint)count, datatype,
                                  uop
#ifdef HAVE_CXX_BINDING
                                  ,
                                  is_cxx_uop
#endif
            );

            MPIR_MVP_SHMEM_TREE_COLL_SetGatherComplete_optrels(
                local_size, local_rank, shmem_comm_rank, i, 0);
        }
    } else if (local_rank < MVP_USE_SHMEM_NUM_TREES) {
        MPIR_MVP_SHMEM_TREE_COLL_GetShmemBuf_optrels(
            local_size, local_rank, shmem_comm_rank, 0, (void *)&shmem_buf, 0);
        local_buf = (char *)shmem_buf + stride * local_rank;
        mpi_errno = MPIR_Localcopy(recvbuf, count, datatype, local_buf, count,
                                   datatype);
        MPIR_ERR_CHECK(mpi_errno);
        MPIR_MVP_SHMEM_TREE_COLL_SetGatherComplete_optrels(
            local_size, local_rank, shmem_comm_rank, 0, 0);
    }

    if (buf_allocated) {
        MPL_free(recvbuf);
        recvbuf = tmp_buf;
    }
fn_exit:
    if (mpi_errno_ret) {
        mpi_errno = mpi_errno_ret;
    } else if (*errflag) {
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**coll_fail");
    }

    MPIR_TIMER_END(coll, reduce, shmem);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIR_Reduce_shmem_MVP_optrels(const void *sendbuf, void *recvbuf, int count,
                                  MPI_Datatype datatype, MPI_Op op, int root,
                                  MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, reduce, shmem);
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    int i, stride, local_rank, local_size, shmem_comm_rank;
    MPI_User_function *uop;
    int is_commutative;
    char *shmem_buf = NULL;
    void *local_buf = NULL;
    char *tmp_buf = NULL;
    int buf_allocated = 0;
    MPI_Aint true_lb, true_extent, extent;
#ifdef HAVE_CXX_BINDING
    int is_cxx_uop = 0;
#endif

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_reduce_shmem, 1);

    MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);
    MPIR_Datatype_get_extent_macro(datatype, extent);
    stride = count * MPL_MAX(extent, true_extent);

    local_rank = comm_ptr->rank;
    local_size = comm_ptr->local_size;
    shmem_comm_rank = comm_ptr->dev.ch.shmem_comm_rank;

    if (sendbuf != MPI_IN_PLACE && local_rank == 0) {
        /* if local_rank == 0 and not root then the recvbuf may not be valid*/
        if (sendbuf == recvbuf || recvbuf == NULL) {
            tmp_buf = recvbuf;
            recvbuf =
                MPL_malloc(count * MPL_MAX(extent, true_extent), MPL_MEM_COLL);
            buf_allocated = 1;
        }
        mpi_errno =
            MPIR_Localcopy(sendbuf, count, datatype, recvbuf, count, datatype);
        MPIR_ERR_CHECK(mpi_errno);
    }

    if (local_size == 0) {
        /* Only one process. So, return */
        goto fn_exit;
    }

    /* Get the operator and check whether it is commutative or not */
    MPIR_get_op_ptr(&uop, &is_commutative,
#ifdef HAVE_CXX_BINDING
                    &is_cxx_uop,
#endif
                    op);

    if (local_rank == 0) {
        for (i = 1; i < local_size; i++) {
            MPIR_MVP_SHMEM_COLL_GetShmemBuf_optrels(
                local_size, local_rank, shmem_comm_rank, i, (void *)&shmem_buf);
            local_buf = (char *)shmem_buf + stride * i;
            MPIR_MVP_Reduce_local(local_buf, recvbuf, (MPI_Aint)count, datatype,
                                  uop
#ifdef HAVE_CXX_BINDING
                                  ,
                                  is_cxx_uop
#endif
            );

            MPIR_MVP_SHMEM_COLL_SetGatherComplete_optrels(
                local_size, local_rank, shmem_comm_rank, i);
        }
    } else {
        MPIR_MVP_SHMEM_COLL_GetShmemBuf_optrels(
            local_size, local_rank, shmem_comm_rank, 0, (void *)&shmem_buf);
        local_buf = (char *)shmem_buf + stride * local_rank;
        mpi_errno = MPIR_Localcopy(sendbuf, count, datatype, local_buf, count,
                                   datatype);
        MPIR_ERR_CHECK(mpi_errno);
        MPIR_MVP_SHMEM_COLL_SetGatherComplete_optrels(local_size, local_rank,
                                                      shmem_comm_rank, 0);
    }

    if (buf_allocated) {
        MPL_free(recvbuf);
        recvbuf = tmp_buf;
    }
fn_exit:
    if (mpi_errno_ret)
        mpi_errno = mpi_errno_ret;
    else if (*errflag)
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**coll_fail");

    MPIR_TIMER_END(coll, reduce, shmem);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIR_Reduce_shmem_MVP(const void *sendbuf, void *recvbuf, int count,
                          MPI_Datatype datatype, MPI_Op op, int root,
                          MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, reduce, shmem);
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    int i, stride, local_rank, local_size, shmem_comm_rank;
    MPI_User_function *uop;
    int is_commutative;
    char *shmem_buf = NULL;
    void *local_buf = NULL;
    char *tmp_buf = NULL;
    int buf_allocated = 0;
    MPI_Aint true_lb, true_extent, extent;
#ifdef HAVE_CXX_BINDING
    int is_cxx_uop = 0;
#endif

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_reduce_shmem, 1);

    MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);
    MPIR_Datatype_get_extent_macro(datatype, extent);
    stride = count * MPL_MAX(extent, true_extent);

    local_rank = comm_ptr->rank;
    local_size = comm_ptr->local_size;
    shmem_comm_rank = comm_ptr->dev.ch.shmem_comm_rank;

    if (sendbuf != MPI_IN_PLACE && local_rank == 0) {
        /* if local_rank == 0 and not root then the recvbuf may not be valid*/
        if (sendbuf == recvbuf || recvbuf == NULL) {
            tmp_buf = recvbuf;
            recvbuf =
                MPL_malloc(count * MPL_MAX(extent, true_extent), MPL_MEM_COLL);
            buf_allocated = 1;
        }
        mpi_errno =
            MPIR_Localcopy(sendbuf, count, datatype, recvbuf, count, datatype);
        MPIR_ERR_CHECK(mpi_errno);
    }

    if (local_size == 0) {
        /* Only one process. So, return */
        goto fn_exit;
    }

    /* Get the operator and check whether it is commutative or not */
    MPIR_get_op_ptr(&uop, &is_commutative,
#ifdef HAVE_CXX_BINDING
                    &is_cxx_uop,
#endif
                    op);

    if (local_rank == 0) {
        MPIR_MVP_SHMEM_COLL_GetShmemBuf(local_size, local_rank, shmem_comm_rank,
                                        (void *)&shmem_buf);
        for (i = 1; i < local_size; i++) {
            local_buf = (char *)shmem_buf + stride * i;
            MPIR_MVP_Reduce_local(local_buf, recvbuf, (MPI_Aint)count, datatype,
                                  uop
#ifdef HAVE_CXX_BINDING
                                  ,
                                  is_cxx_uop
#endif
            );
        }
        MPIR_MVP_SHMEM_COLL_SetGatherComplete(local_size, local_rank,
                                              shmem_comm_rank);
    } else {
        MPIR_MVP_SHMEM_COLL_GetShmemBuf(local_size, local_rank, shmem_comm_rank,
                                        (void *)&shmem_buf);
        local_buf = (char *)shmem_buf + stride * local_rank;
        mpi_errno = MPIR_Localcopy(sendbuf, count, datatype, local_buf, count,
                                   datatype);
        MPIR_ERR_CHECK(mpi_errno);
        MPIR_MVP_SHMEM_COLL_SetGatherComplete(local_size, local_rank,
                                              shmem_comm_rank);
    }

    if (buf_allocated) {
        MPL_free(recvbuf);
        recvbuf = tmp_buf;
    }
fn_exit:
    if (mpi_errno_ret) {
        mpi_errno = mpi_errno_ret;
    } else if (*errflag) {
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**coll_fail");
    }

    MPIR_TIMER_END(coll, reduce, shmem);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
