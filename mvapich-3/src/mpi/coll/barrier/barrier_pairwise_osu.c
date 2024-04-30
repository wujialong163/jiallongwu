#include "barrier_tuning.h"

int MPIR_Pairwise_Barrier_MVP(MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, barrier, pairwise);
    int size, rank;
    int d, dst, src;
    int mpi_errno = MPI_SUCCESS;

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_barrier_pairwise, 1);

    size = comm_ptr->local_size;
    /* Trivial barriers return immediately */
    if (size == 1)
        return MPI_SUCCESS;

    rank = comm_ptr->rank;

    /*  N2_prev = greatest power of two < size of Comm  */
    int N2_prev = comm_ptr->dev.ch.gpof2;
    int surfeit = size - N2_prev;

    /* Perform a combine-like operation */
    if (rank < N2_prev) {
        if (rank < surfeit) {
            /* get the fanin letter from the upper "half" process: */
            dst = N2_prev + rank;
            MPIR_PVAR_INC(barrier, pairwise, recv, 0, MPI_BYTE);
            mpi_errno = MPIC_Recv(NULL, 0, MPI_BYTE, dst, MPIR_BARRIER_TAG,
                                  comm_ptr, MPI_STATUS_IGNORE, errflag);
        }

        /* combine on embedded N2_prev power-of-two processes */
        for (d = 1; d < N2_prev; d <<= 1) {
            dst = (rank ^ d);
            MPIR_PVAR_INC(barrier, pairwise, send, 0, MPI_BYTE);
            MPIR_PVAR_INC(barrier, pairwise, recv, 0, MPI_BYTE);
            mpi_errno = MPIC_Sendrecv(NULL, 0, MPI_BYTE, dst, MPIR_BARRIER_TAG,
                                      NULL, 0, MPI_BYTE, dst, MPIR_BARRIER_TAG,
                                      comm_ptr, MPI_STATUS_IGNORE, errflag);
        }

        /* fanout data to nodes above N2_prev... */
        if (rank < surfeit) {
            dst = N2_prev + rank;
            MPIR_PVAR_INC(barrier, pairwise, send, 0, MPI_BYTE);
            mpi_errno = MPIC_Send(NULL, 0, MPI_BYTE, dst, MPIR_BARRIER_TAG,
                                  comm_ptr, errflag);
        }
    } else {
        /* fanin data to power of 2 subset */
        src = rank - N2_prev;
        MPIR_PVAR_INC(barrier, pairwise, send, 0, MPI_BYTE);
        MPIR_PVAR_INC(barrier, pairwise, recv, 0, MPI_BYTE);
        mpi_errno = MPIC_Sendrecv(NULL, 0, MPI_BYTE, src, MPIR_BARRIER_TAG,
                                  NULL, 0, MPI_BYTE, src, MPIR_BARRIER_TAG,
                                  comm_ptr, MPI_STATUS_IGNORE, errflag);
    }

    MPIR_TIMER_END(coll, barrier, pairwise);
    return mpi_errno;
}
