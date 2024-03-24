#include "barrier_tuning.h"
#if defined(_SHARP_SUPPORT_)
#include "api/sharp_coll.h"
#include "mvp_sharp.h"
#endif

int MPIR_socket_aware_shmem_barrier_old_MVP(MPIR_Comm *comm_ptr,
                                            MPIR_Errflag_t *errflag)
{
    MPIR_Assert(comm_ptr->dev.ch.use_intra_sock_comm &&
                comm_ptr->dev.ch.shmem_coll_ok == 1);
    int mpi_errno = MPI_SUCCESS;
    MPI_Comm shmem_comm = MPI_COMM_NULL, leader_comm = MPI_COMM_NULL;
    MPIR_Comm *shmem_commptr = NULL, *leader_commptr = NULL;
    int local_rank = -1, local_size = 0;
    shmem_comm = comm_ptr->dev.ch.shmem_comm;
    PMPI_Comm_size(shmem_comm, &local_size);
    MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
    local_rank = shmem_commptr->rank;
    local_size = shmem_commptr->local_size;

    leader_comm = comm_ptr->dev.ch.leader_comm;
    MPIR_Comm_get_ptr(leader_comm, leader_commptr);

    int intra_sock_rank = -1, intra_sock_size;
    MPIR_Comm *intra_sock_commptr = NULL;
    MPIR_Comm_get_ptr(shmem_commptr->dev.ch.intra_sock_comm,
                      intra_sock_commptr);
    intra_sock_rank = MPIR_Comm_rank(intra_sock_commptr);
    intra_sock_size = MPIR_Comm_size(intra_sock_commptr);

    int shmem_comm_rank = intra_sock_commptr->dev.ch.shmem_comm_rank;

    if (local_size > 1) {
        MPIR_MVP_SHMEM_COLL_Barrier_gather(intra_sock_size, intra_sock_rank,
                                           shmem_comm_rank);
    }

    if (intra_sock_rank == 0) {
        MPIR_Comm *shmem_leader_commptr = NULL;
        MPIR_Comm_get_ptr(shmem_commptr->dev.ch.intra_sock_leader_comm,
                          shmem_leader_commptr);
        MPIR_Comm *global_sock_leader_ptr = NULL;
        MPIR_Comm_get_ptr(comm_ptr->dev.ch.global_sock_leader_comm,
                          global_sock_leader_ptr);

        if (shmem_leader_commptr->dev.ch.shmem_coll_ok != 1) {
            /* Fall back to pt2pt pairwise algorithm */
            mpi_errno =
                MPIR_Pairwise_Barrier_MVP(global_sock_leader_ptr, errflag);
        } else {
            MPIR_Comm *shmem_leader_shmemcomm = NULL;
            MPIR_Comm_get_ptr(shmem_leader_commptr->dev.ch.shmem_comm,
                              shmem_leader_shmemcomm);
            int leader_shmem_comm_rank = 0, leader_size = 0, leader_rank = 0;

            if (shmem_leader_commptr->local_size > 1) {
                leader_shmem_comm_rank =
                    shmem_leader_shmemcomm->dev.ch.shmem_comm_rank;
                leader_size = shmem_leader_commptr->local_size;
                leader_rank = shmem_leader_commptr->rank;

                MPIR_MVP_SHMEM_COLL_Barrier_gather(leader_size, leader_rank,
                                                   leader_shmem_comm_rank);
            }

            if (local_rank == 0) {
                mpi_errno = MPIR_Pairwise_Barrier_MVP(leader_commptr, errflag);
            }

            if (shmem_leader_commptr->local_size > 1) {
                MPIR_MVP_SHMEM_COLL_Barrier_bcast(leader_size, leader_rank,
                                                  leader_shmem_comm_rank);
            }
        }
    }

    if (local_size > 1) {
        MPIR_MVP_SHMEM_COLL_Barrier_bcast(intra_sock_size, intra_sock_rank,
                                          shmem_comm_rank);
    }
    return mpi_errno;
}

int MPIR_socket_aware_shmem_barrier_MVP(MPIR_Comm *comm_ptr,
                                        MPIR_Errflag_t *errflag)
{
    MPIR_Assert(comm_ptr->dev.ch.use_intra_sock_comm &&
                comm_ptr->dev.ch.shmem_coll_ok == 1);
    if (!MVP_USE_SLOT_SHMEM_COLL) {
        return MPIR_socket_aware_shmem_barrier_old_MVP(comm_ptr, errflag);
    }
    int mpi_errno = MPI_SUCCESS;
#if defined(_SHARP_SUPPORT_)
    int mpi_errno_ret = MPI_SUCCESS;
#endif
    MPI_Comm shmem_comm = MPI_COMM_NULL, leader_comm = MPI_COMM_NULL;
    MPIR_Comm *shmem_commptr = NULL, *leader_commptr = NULL;
    int local_rank = -1, local_size = 0;
    shmem_comm = comm_ptr->dev.ch.shmem_comm;
    PMPI_Comm_size(shmem_comm, &local_size);
    MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
    local_rank = shmem_commptr->rank;
    local_size = shmem_commptr->local_size;

    leader_comm = comm_ptr->dev.ch.leader_comm;
    MPIR_Comm_get_ptr(leader_comm, leader_commptr);

    int intra_sock_rank = -1, intra_sock_size;
    MPIR_Comm *intra_sock_commptr = NULL;
    MPIR_Comm_get_ptr(shmem_commptr->dev.ch.intra_sock_comm,
                      intra_sock_commptr);
    intra_sock_rank = MPIR_Comm_rank(intra_sock_commptr);
    intra_sock_size = MPIR_Comm_size(intra_sock_commptr);

    if (local_size > 1) {
        mvp_shm_barrier_gather(intra_sock_commptr->dev.ch.shmem_info);
    }

    if (intra_sock_rank == 0) {
        MPIR_Comm *shmem_leader_commptr = NULL;
        MPIR_Comm_get_ptr(shmem_commptr->dev.ch.intra_sock_leader_comm,
                          shmem_leader_commptr);
        MPIR_Comm *global_sock_leader_ptr = NULL;
        MPIR_Comm_get_ptr(comm_ptr->dev.ch.global_sock_leader_comm,
                          global_sock_leader_ptr);

        if (shmem_leader_commptr->dev.ch.shmem_coll_ok != 1) {
            /* Fall back to pt2pt pairwise algorithm */
            mpi_errno =
                MPIR_Pairwise_Barrier_MVP(global_sock_leader_ptr, errflag);
        } else {
            MPIR_Comm *shmem_leader_shmemcomm = NULL;
            MPIR_Comm_get_ptr(shmem_leader_commptr->dev.ch.shmem_comm,
                              shmem_leader_shmemcomm);

            if (shmem_leader_commptr->local_size > 1) {
                mvp_shm_barrier_gather(
                    shmem_leader_shmemcomm->dev.ch.shmem_info);
            }

            if (local_rank == 0) {
#if defined(_SHARP_SUPPORT_)
                if (comm_ptr->dev.ch.is_sharp_ok == 1 &&
                    MVP_ENABLE_SHARP == 1 && MVP_ENABLE_SHARP_BARRIER) {
                    mpi_errno = MPIR_Sharp_Barrier_MVP(comm_ptr, errflag);
                    if (mpi_errno != MPI_SUCCESS) {
                        /* fall back to Pairwise algorithm
                         * if SHArP is not supported */
                        mpi_errno =
                            MPIR_Pairwise_Barrier_MVP(leader_commptr, errflag);
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
                    mpi_errno =
                        MPIR_Pairwise_Barrier_MVP(leader_commptr, errflag);
            }

            if (shmem_leader_commptr->local_size > 1) {
                mvp_shm_barrier_bcast(
                    shmem_leader_shmemcomm->dev.ch.shmem_info);
            }
        }
    }

    if (local_size > 1) {
        mvp_shm_barrier_bcast(intra_sock_commptr->dev.ch.shmem_info);
    }
    return mpi_errno;
}
