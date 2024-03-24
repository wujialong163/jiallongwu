#include "allgather_tuning.h"

extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allgather_ring;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_ring;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_ring_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_ring_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_ring_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_ring_count_recv;

int MPIR_Allgather_Ring_MVP(const void *sendbuf, int sendcount,
                            MPI_Datatype sendtype, void *recvbuf, int recvcount,
                            MPI_Datatype recvtype, MPIR_Comm *comm_ptr,
                            MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, allgather, ring);
    int comm_size, rank;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    MPI_Aint recvtype_extent;
    int j, i;
    int left, right, jnext;

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_allgather_ring, 1);

    comm_size = comm_ptr->local_size;
    rank = comm_ptr->rank;

    MPIR_Datatype_get_extent_macro(recvtype, recvtype_extent);

    /* First, load the "local" version in the recvbuf. */
    if (sendbuf != MPI_IN_PLACE) {
        mpi_errno = MPIR_Localcopy(
            sendbuf, sendcount, sendtype,
            ((char *)recvbuf + rank * recvcount * recvtype_extent), recvcount,
            recvtype);
        MPIR_ERR_CHECK(mpi_errno);
    }

    /*
     * Now, send left to right.  This fills in the receive area in
     * reverse order.
     */
    left = (comm_size + rank - 1) % comm_size;
    right = (rank + 1) % comm_size;

    j = rank;
    jnext = left;
    for (i = 1; i < comm_size; i++) {
        MPIR_PVAR_INC(allgather, ring, send, recvcount, recvtype);
        MPIR_PVAR_INC(allgather, ring, recv, recvcount, recvtype);
        mpi_errno = MPIC_Sendrecv(
            ((char *)recvbuf + j * recvcount * recvtype_extent), recvcount,
            recvtype, right, MPIR_ALLGATHER_TAG,
            ((char *)recvbuf + jnext * recvcount * recvtype_extent), recvcount,
            recvtype, left, MPIR_ALLGATHER_TAG, comm_ptr, MPI_STATUS_IGNORE,
            errflag);
        if (mpi_errno) {
            /* for communication errors, just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
        j = jnext;
        jnext = (comm_size + jnext - 1) % comm_size;
    }

fn_fail:
    MPIR_TIMER_END(coll, allgather, ring);
    return (mpi_errno);
}
