#include "barrier_tuning.h"
#if defined(_SHARP_SUPPORT_)
#include "api/sharp_coll.h"
#include "mvp_sharp.h"
#endif

int MPIR_topo_aware_shmem_barrier_MVP(MPIR_Comm *comm_ptr,
                                      MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, barrier, topo_aware_shmem);
    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_barrier_topo_aware_shmem, 1);
    MPIR_Assert(comm_ptr->dev.ch.topo_coll_ok == 1 &&
                comm_ptr->dev.ch.shmem_coll_ok == 1);
    int mpi_errno = MPI_SUCCESS;
#if defined(_SHARP_SUPPORT_)
    int mpi_errno_ret = MPI_SUCCESS;
#endif
    MPI_Comm shmem_comm = MPI_COMM_NULL, leader_comm = MPI_COMM_NULL;
    MPIR_Comm *shmem_commptr = NULL, *leader_commptr = NULL,
              *topo_comm_ptr = NULL;
    int local_rank = -1, local_size = 0, i = 0;
    shmem_comm = comm_ptr->dev.ch.shmem_comm;
    MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
    local_rank = shmem_commptr->rank;
    local_size = shmem_commptr->local_size;

    leader_comm = comm_ptr->dev.ch.leader_comm;
    MPIR_Comm_get_ptr(leader_comm, leader_commptr);

    if (local_size > 1) {
        for (i = 0; i <= mvp_num_intra_node_comm_levels; ++i) {
            /* Get next topo_comm pointer */
            MPIR_Comm_get_ptr(shmem_commptr->dev.ch.topo_comm[i],
                              topo_comm_ptr);
            if (topo_comm_ptr && topo_comm_ptr->local_size > 1) {
                mvp_shm_barrier_gather(topo_comm_ptr->dev.ch.shmem_info);
            }
        }
    }

    if (local_rank == 0) {
#if defined(_SHARP_SUPPORT_)
        if (comm_ptr->dev.ch.is_sharp_ok == 1 && MVP_ENABLE_SHARP == 1 &&
            MVP_ENABLE_SHARP_BARRIER) {
            mpi_errno = MPIR_Sharp_Barrier_MVP(comm_ptr, errflag);
            if (mpi_errno != MPI_SUCCESS) {
                /* fall back to Pairwise algorithm if SHArP is not supported */
                mpi_errno = MPIR_Pairwise_Barrier_MVP(leader_commptr, errflag);
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
            mpi_errno = MPIR_Pairwise_Barrier_MVP(leader_commptr, errflag);
    }

    if (local_size > 1) {
        for (i = mvp_num_intra_node_comm_levels; i >= 0; --i) {
            MPIR_Comm_get_ptr(shmem_commptr->dev.ch.topo_comm[i],
                              topo_comm_ptr);
            if (topo_comm_ptr && topo_comm_ptr->local_size > 1) {
                mvp_shm_barrier_bcast(topo_comm_ptr->dev.ch.shmem_info);
            }
        }
    }

fn_exit:
    MPIR_TIMER_END(coll, barrier, topo_aware_shmem);
    return mpi_errno;

fn_fail:
    goto fn_exit;
}
