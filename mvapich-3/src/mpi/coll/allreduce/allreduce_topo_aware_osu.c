#include "allreduce_tuning.h"
#include "bcast_tuning.h"

#if defined(_SHARP_SUPPORT_)
#include "mvp_sharp.h"
#endif

/* Given comm_ptr, MPIR_Allreduce_topo_aware_inter_node_helper_MVP performs an
 * inter-node Allreduce operation using SHARP/rd
 * over leader_comm (in comm_ptr) */
int MPIR_Allreduce_topo_aware_inter_node_helper_MVP(
    const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
    MPI_Op op, MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS, mpi_errno_ret = MPI_SUCCESS;
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
        MVP_ENABLE_SHARP == 1 && MVP_ENABLE_SHARP_ALLREDUCE) {
        mpi_errno = MPIR_Sharp_Allreduce_MVP(sendbuf, recvbuf, count, datatype,
                                             op, comm_ptr, errflag);
        if (mpi_errno != MPI_SUCCESS) {
            /* fall back to RD algorithm if SHArP is not supported */
            mpi_errno = MPIR_Allreduce_pt2pt_rd_MVP(
                sendbuf, recvbuf, count, datatype, op, leader_commptr, errflag);
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
        mpi_errno = MPIR_Allreduce_pt2pt_rd_MVP(
            sendbuf, recvbuf, count, datatype, op, leader_commptr, errflag);
        if (mpi_errno) {
            /* for communication errors, just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
    }

fn_exit:
    return (mpi_errno);

fn_fail:
    goto fn_exit;
}

int MPIR_Allreduce_topo_aware_hierarchical_MVP(const void *sendbuf,
                                               void *recvbuf, int count,
                                               MPI_Datatype datatype, MPI_Op op,
                                               MPIR_Comm *comm_ptr,
                                               MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, allreduce, topo_aware_hierarchical);
    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_allreduce_topo_aware_hierarchical, 1);
    MPIR_Assert(comm_ptr->dev.ch.topo_coll_ok == 1 &&
                comm_ptr->dev.ch.shmem_coll_ok == 1);
    int i = 0;
    shmem_info_t *shmem = NULL;
    int mpi_errno = MPI_SUCCESS, mpi_errno_ret = MPI_SUCCESS;
    MPI_Aint true_lb = 0, true_extent = 0, extent = 0;
    void *in_buf = NULL, *buf = NULL;
    int local_rank = -1, is_cxx_uop = 0, rindex = 0, len = 0;
    int is_commutative;
    MPI_User_function *uop = NULL;
    MPI_Comm shmem_comm = MPI_COMM_NULL, leader_comm = MPI_COMM_NULL;
    MPIR_Comm *shmem_commptr = NULL, *leader_commptr = NULL;
    MPIR_Comm *topo_comm_ptr = NULL;

    MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);
    MPIR_Datatype_get_extent_macro(datatype, extent);
    len = count * MPL_MAX(extent, true_extent);

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

    /* Step 2. Leaders across nodes do an inter-node allreduce */
    if (local_rank == 0 && leader_commptr->local_size > 1) {
        /* in_buf == sendbuf if and only if no intra-node reduction occurs
         * (example : local size is one). This check exists to avoid overwriting
         * the application's sendbuf in the inter-node step*/
        if (in_buf == sendbuf) {
            mpi_errno = MPIR_Allreduce_topo_aware_inter_node_helper_MVP(
                sendbuf, recvbuf, count, datatype, op, comm_ptr, errflag);
        } else {
            mpi_errno = MPIR_Allreduce_topo_aware_inter_node_helper_MVP(
                MPI_IN_PLACE, in_buf, count, datatype, op, comm_ptr, errflag);
        }

        if (mpi_errno) {
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
    }

    /* Step 3. Intra-node Bcast */
    for (i = mvp_num_intra_node_comm_levels; i >= 0; --i) {
        MPIR_Comm_get_ptr(shmem_commptr->dev.ch.topo_comm[i], topo_comm_ptr);
        if (topo_comm_ptr && topo_comm_ptr->local_size > 1) {
            mpi_errno = MPIR_Shmem_Bcast_MVP(in_buf, count, datatype, 0,
                                             topo_comm_ptr, errflag);
            if (mpi_errno) {
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
        }
    }

    /* Copy in_buf into recvbuf only if it has been modified */
    if (in_buf != recvbuf && in_buf != sendbuf) {
        MPIR_Localcopy(in_buf, count, datatype, recvbuf, count, datatype);
    }

fn_exit:
    MPIR_TIMER_END(coll, allreduce, topo_aware_hierarchical);
    return (mpi_errno);

fn_fail:
    goto fn_exit;
}
