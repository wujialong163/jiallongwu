#include "reduce_tuning.h"

#if defined(_SHARP_SUPPORT_)
#include "mvp_sharp.h"
#endif

int MPIR_Reduce_topo_aware_inter_node_helper_MVP(
    const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
    MPI_Op op, int root, MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, reduce, topo_aware_hierarchical);
    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_reduce_topo_aware_hierarchical, 1);
    int mpi_errno = MPI_SUCCESS;
    MPI_Comm leader_comm = MPI_COMM_NULL;
    MPIR_Comm *leader_commptr = NULL;
    leader_comm = comm_ptr->dev.ch.leader_comm;
    MPIR_Comm_get_ptr(leader_comm, leader_commptr);
#if defined(_SHARP_SUPPORT_)
    MPI_Aint type_size = 0;
    /* Get size of data */
    MPIR_Datatype_get_size_macro(datatype, type_size);
    intptr_t nbytes = (intptr_t)(count) * (type_size);
    if (comm_ptr->dev.ch.is_sharp_ok == 1 && nbytes <= MVP_SHARP_MAX_MSG_SIZE &&
        MVP_ENABLE_SHARP == 1 && MVP_ENABLE_SHARP_REDUCE) {
        mpi_errno = MPIR_Sharp_Reduce_MVP(sendbuf, recvbuf, count, datatype, op,
                                          root, comm_ptr, errflag);
        if (mpi_errno != MPI_SUCCESS) {
            /* fall back to binomial algorithm if SHArP is not supported */
            mpi_errno =
                MPIR_Reduce_binomial_MVP(sendbuf, recvbuf, count, datatype, op,
                                         root, leader_commptr, errflag);
            MPIR_ERR_CHECK(mpi_errno);
        }
    } else
#endif /* #if defined (_SHARP_SUPPORT_) */
    {
        mpi_errno = MPIR_Reduce_binomial_MVP(sendbuf, recvbuf, count, datatype,
                                             op, root, leader_commptr, errflag);
        MPIR_ERR_CHECK(mpi_errno);
    }

fn_exit:
    MPIR_TIMER_END(coll, reduce, topo_aware_hierarchical);
    return (mpi_errno);

fn_fail:
    goto fn_exit;
}

int MPIR_Reduce_topo_aware_hierarchical_MVP(const void *sendbuf, void *recvbuf,
                                            int count, MPI_Datatype datatype,
                                            MPI_Op op, int root,
                                            MPIR_Comm *comm_ptr,
                                            MPIR_Errflag_t *errflag)
{
    MPIR_Assert(comm_ptr->dev.ch.topo_coll_ok == 1 &&
                comm_ptr->dev.ch.shmem_coll_ok == 1);
    int i = 0;
    shmem_info_t *shmem = NULL;
    int mpi_errno = MPI_SUCCESS;
    MPI_Aint true_lb = 0, true_extent = 0, extent = 0;
    void *in_buf = NULL, *buf = NULL;
    int local_rank = -1, rindex = 0, len = 0;
    MPI_User_function *uop = NULL;
    int is_cxx_uop = 0, is_commutative;
    MPI_Comm shmem_comm = MPI_COMM_NULL, leader_comm = MPI_COMM_NULL;
    MPIR_Comm *shmem_commptr = NULL, *leader_commptr = NULL;
    MPIR_Comm *topo_comm_ptr = NULL;
    MPI_Status status;
    MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);
    MPIR_Datatype_get_extent_macro(datatype, extent);
    len = count * MPL_MAX(extent, true_extent);
    int rank = comm_ptr->rank;
    if (count == 0) {
        return MPI_SUCCESS;
    }

    /* Get leader and shared memory communicators/related attributes */
    shmem_comm = comm_ptr->dev.ch.shmem_comm;
    MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
    local_rank = shmem_commptr->rank;
    leader_comm = comm_ptr->dev.ch.leader_comm;
    MPIR_Comm_get_ptr(leader_comm, leader_commptr);

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

    /* Step 1. intra-node topo-aware reduce using shared memory */
    for (i = 0; i <= mvp_num_intra_node_comm_levels; ++i) {
        /* Get next topo_comm pointer */
        MPIR_Comm_get_ptr(shmem_commptr->dev.ch.topo_comm[i], topo_comm_ptr);
        if (topo_comm_ptr && topo_comm_ptr->local_size > 1) {
            /* Get shmem region */
            shmem = topo_comm_ptr->dev.ch.shmem_info;
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
        }
    }

    /* Step 2. Leaders across nodes do an inter-node reduce */
    if (local_rank == 0 && leader_commptr->local_size > 1) {
        /* in_buf == sendbuf if and only if no intra-node reduction occurs
         * (example : local size is one). This check exists to avoid overwriting
         * the application's sendbuf in the inter-node step*/
        if (in_buf == sendbuf) {
            mpi_errno = MPIR_Reduce_topo_aware_inter_node_helper_MVP(
                sendbuf, recvbuf, count, datatype, op, 0, comm_ptr, errflag);
        } else {
            mpi_errno = MPIR_Reduce_topo_aware_inter_node_helper_MVP(
                MPI_IN_PLACE, in_buf, count, datatype, op, 0, comm_ptr,
                errflag);
        }

        MPIR_ERR_CHECK(mpi_errno);
    }

    /* Copy in_buf into recvbuf only if it has been modified */
    if (in_buf != recvbuf && in_buf != sendbuf) {
        MPIR_Localcopy(in_buf, count, datatype, recvbuf, count, datatype);
    }

    if (rank == root && rank != 0) {
        mpi_errno = MPIC_Recv(recvbuf, count, datatype, 0, MPIR_REDUCE_TAG,
                              comm_ptr, &status, errflag);
        MPIR_ERR_CHECK(mpi_errno);
    }

    if (rank == 0 && rank != root) {
        mpi_errno = MPIC_Send(recvbuf, count, datatype, root, MPIR_REDUCE_TAG,
                              comm_ptr, errflag);
        MPIR_ERR_CHECK(mpi_errno);
    }

fn_exit:
    return (mpi_errno);

fn_fail:
    goto fn_exit;
}
