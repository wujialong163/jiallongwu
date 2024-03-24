#include "bcast_tuning.h"

#if defined(_SHARP_SUPPORT_)
#include "mvp_sharp.h"
extern int MPIR_Sharp_Bcast_MVP(void *buffer, int count, MPI_Datatype datatype,
                                int root, MPIR_Comm *comm_ptr,
                                MPIR_Errflag_t *errflag);

#endif

int MPIR_Bcast_topo_aware_inter_node_helper_MVP(void *buffer, int count,
                                                MPI_Datatype datatype, int root,
                                                MPIR_Comm *comm_ptr,
                                                MPIR_Errflag_t *errflag)
{
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
        MVP_ENABLE_SHARP == 1 && MVP_ENABLE_SHARP_BCAST) {
        mpi_errno = MPIR_Sharp_Bcast_MVP(buffer, count, datatype, root,
                                         comm_ptr, errflag);
        if (mpi_errno != MPI_SUCCESS) {
            /* fall back to binomial algorithm if SHArP is not supported */
            mpi_errno = MPIR_Bcast_binomial_MVP(buffer, count, datatype, root,
                                                leader_commptr, errflag);
            MPIR_ERR_CHECK(mpi_errno);
        }
    } else
#endif /* #if defined (_SHARP_SUPPORT_) */
    {
        mpi_errno = MPIR_Bcast_binomial_MVP(buffer, count, datatype, root,
                                            leader_commptr, errflag);
        MPIR_ERR_CHECK(mpi_errno);
    }

fn_exit:
    return (mpi_errno);

fn_fail:
    goto fn_exit;
}

int MPIR_Bcast_topo_aware_hierarchical_MVP(void *buffer, int count,
                                           MPI_Datatype datatype, int root,
                                           MPIR_Comm *comm_ptr,
                                           MPIR_Errflag_t *errflag)
{
    MPIR_Assert(comm_ptr->dev.ch.topo_coll_ok == 1 &&
                comm_ptr->dev.ch.shmem_coll_ok == 1);
    MPIR_TIMER_START(coll, bcast, topo_aware_hierarchical);
    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_bcast_topo_aware_hierarchical, 1);
    int i = 0;
    int mpi_errno = MPI_SUCCESS;
    MPI_Comm shmem_comm = MPI_COMM_NULL, leader_comm = MPI_COMM_NULL;
    MPIR_Comm *shmem_commptr = NULL, *leader_commptr = NULL,
              *topo_comm_ptr = NULL;
    MPI_Status status;
    int rank = comm_ptr->rank;
    int local_rank;

    if (rank == root && rank != 0) {
        mpi_errno = MPIC_Send(buffer, count, datatype, 0, MPIR_BCAST_TAG,
                              comm_ptr, errflag);
        MPIR_ERR_CHECK(mpi_errno);
    }

    if (rank == 0 && rank != root) {
        mpi_errno = MPIC_Recv(buffer, count, datatype, root, MPIR_BCAST_TAG,
                              comm_ptr, &status, errflag);
        MPIR_ERR_CHECK(mpi_errno);
    }

    shmem_comm = comm_ptr->dev.ch.shmem_comm;
    MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
    local_rank = shmem_commptr->rank;
    leader_comm = comm_ptr->dev.ch.leader_comm;
    MPIR_Comm_get_ptr(leader_comm, leader_commptr);

    /* Step 1. Inter-node Bcast */
    if (local_rank == 0 && leader_commptr->local_size > 1) {
        mpi_errno = MPIR_Bcast_topo_aware_inter_node_helper_MVP(
            buffer, count, datatype, 0, comm_ptr, errflag);
        MPIR_ERR_CHECK(mpi_errno);
    }

    /* Step 2. Intra-node Bcast */
    for (i = mvp_num_intra_node_comm_levels; i >= 0; --i) {
        MPIR_Comm_get_ptr(shmem_commptr->dev.ch.topo_comm[i], topo_comm_ptr);
        if (topo_comm_ptr && topo_comm_ptr->local_size > 1) {
            mpi_errno = MPIR_Shmem_Bcast_MVP(buffer, count, datatype, 0,
                                             topo_comm_ptr, errflag);
            MPIR_ERR_CHECK(mpi_errno);
        }
    }
fn_exit:
    MPIR_TIMER_END(coll, bcast, topo_aware_hierarchical);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
