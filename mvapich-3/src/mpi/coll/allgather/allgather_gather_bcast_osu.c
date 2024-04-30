#include "allgather_tuning.h"

extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allgather_gather_bcast;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_gather_bcast;

/* executes an allgather as a gather followed by a broadcast */
int MPIR_Allgather_gather_bcast_MVP(const void *sendbuf, int sendcount,
                                    MPI_Datatype sendtype, void *recvbuf,
                                    int recvcount, MPI_Datatype recvtype,
                                    MPIR_Comm *comm_ptr,
                                    MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, allgather, gather_bcast);
    int comm_size;
    int mpi_errno = MPI_SUCCESS;
    int gather_bcast_root = -1;

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_allgather_gather_bcast, 1);

    comm_size = comm_ptr->local_size;

    MPIR_Assert(comm_ptr->dev.ch.shmem_coll_ok == 1);

    // Call gather (Calling mvp version so that gather is tuned)
    // If IN_PLACE is used, gather is expected to handle it
    gather_bcast_root = 0;
    mpi_errno =
        MPIR_Gather_MVP(sendbuf, sendcount, sendtype, recvbuf, recvcount,
                        recvtype, gather_bcast_root, comm_ptr, errflag);
    MPIR_ERR_CHECK(mpi_errno);

    // gather_bcast_root has all data at this point

    // call bcast on the receive buffer
    mpi_errno = MPIR_Bcast_MVP(recvbuf, recvcount * comm_size, recvtype,
                               gather_bcast_root, comm_ptr, errflag);
    MPIR_ERR_CHECK(mpi_errno);

fn_fail:
    MPIR_TIMER_END(coll, allgather, gather_bcast);
    return (mpi_errno);
}
