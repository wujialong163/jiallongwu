#include "allgather_tuning.h"

extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allgather_direct;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allgather_directspread;

extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_direct;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_directspread;

extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_direct_bytes_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_allgather_directspread_bytes_send;

extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_direct_bytes_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_allgather_directspread_bytes_recv;

extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_direct_count_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_allgather_directspread_count_send;

extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_direct_count_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_allgather_directspread_count_recv;

/* This implements an allgather via direct method, in which each
 * process sends directly to every other process.  All processes
 * start sending to rank 0 and work up in order. This is meant as
 * a base case */
int MPIR_Allgather_Direct_MVP(const void *sendbuf, int sendcnt,
                              MPI_Datatype sendtype, void *recvbuf, int recvcnt,
                              MPI_Datatype recvtype, MPIR_Comm *comm_ptr,
                              MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, allgather, direct);
    int i;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    MPIR_CHKLMEM_DECL(2);

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_allgather_direct, 1);

    if (recvcnt == 0) {
        return MPI_SUCCESS;
    }

    /* get our rank and the size of this communicator */
    int rank = comm_ptr->rank;
    int size = comm_ptr->local_size;

    /* get extent of receive type */
    MPI_Aint recvtype_extent;
    MPIR_Datatype_get_extent_macro(recvtype, recvtype_extent);

    /* allocate an array of request objects */
    MPIR_Request **reqarray = NULL;
    MPIR_CHKLMEM_MALLOC(reqarray, MPIR_Request **,
                        2 * size * sizeof(MPIR_Request *), mpi_errno,
                        "reqarray", MPL_MEM_COLL);

    /* allocate an array of status objects */
    MPI_Status *starray = NULL;
    MPIR_CHKLMEM_MALLOC(starray, MPI_Status *, 2 * size * sizeof(MPI_Status),
                        mpi_errno, "starray", MPL_MEM_COLL);

    /* initialize our active request counter to 0 */
    int reqs = 0;

    /* copy our data to our receive buffer if needed */
    if (sendbuf != MPI_IN_PLACE) {
        /* compute location in receive buffer for our data */
        void *rbuf =
            (void *)((char *)recvbuf + rank * recvcnt * recvtype_extent);

        /* copy data from send buffer to receive buffer */
        mpi_errno =
            MPIR_Localcopy(sendbuf, sendcnt, sendtype, rbuf, recvcnt, recvtype);
        MPIR_ERR_CHECK(mpi_errno);
    }

    /* post receives */
    for (i = 0; i < size; i++) {
        /* our data is already in the receive buffer */
        if (i == rank) {
            continue;
        }

        /* compute pointer in receive buffer for incoming data from this rank */
        void *rbuf = (void *)((char *)recvbuf + i * recvcnt * recvtype_extent);

        /* post receive for data from this rank */
        MPIR_PVAR_INC(allgather, direct, recv, recvcnt, recvtype);
        mpi_errno = MPIC_Irecv(rbuf, recvcnt, recvtype, i, MPIR_ALLGATHER_TAG,
                               comm_ptr, &reqarray[reqs++]);
        if (mpi_errno) {
            /* for communication errors, just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
    }

    /* TODO: consider placing a barrier here to ensure
     * receives are posted before sends, especially for large messages */
    // MPIR_Barrier_impl(comm_ptr);

    /* get parameters for sending data */
    const void *sbuf = sendbuf;
    int scnt = sendcnt;
    MPI_Datatype stype = sendtype;
    if (sendbuf == MPI_IN_PLACE) {
        /* use receive params if IN_PLACE */
        sbuf = (void *)((char *)recvbuf + rank * recvcnt * recvtype_extent);
        scnt = recvcnt;
        stype = recvtype;
    }

    /* post sends */
    for (i = 0; i < size; i++) {
        /* no need to send to ourself */
        if (i == rank) {
            continue;
        }

        /* send data to this rank */
        MPIR_PVAR_INC(allgather, direct, send, scnt, stype);
        mpi_errno = MPIC_Isend(sbuf, scnt, stype, i, MPIR_ALLGATHER_TAG,
                               comm_ptr, &reqarray[reqs++], errflag);
        if (mpi_errno) {
            /* for communication errors, just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
    }

    /* wait for all outstanding requests to complete */
    mpi_errno = MPIC_Waitall(reqs, reqarray, starray, errflag);
    MPIR_ERR_CHECK(mpi_errno);

    /* --BEGIN ERROR HANDLING-- */
    if (mpi_errno == MPI_ERR_IN_STATUS) {
        for (i = 0; i < reqs; i++) {
            if (starray[i].MPI_ERROR != MPI_SUCCESS) {
                mpi_errno = starray[i].MPI_ERROR;
                if (mpi_errno) {
                    /* for communication errors,
                     * just record the error but continue */
                    *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                    MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                }
            }
        }
    }

    MPIR_CHKLMEM_FREEALL();
fn_fail:
    MPIR_TIMER_END(coll, allgather, direct);
    return (mpi_errno);
}

/* This implements an allgather via direct method, in which each
 * process sends directly to every other process.  To spread the
 * load and avoid hot spots, processes starting by sending to the
 * rank one higher than their own.  This is meant as a base case
 * allgather, but it may actually be the fastest method in some cases. */
int MPIR_Allgather_DirectSpread_MVP(const void *sendbuf, int sendcnt,
                                    MPI_Datatype sendtype, void *recvbuf,
                                    int recvcnt, MPI_Datatype recvtype,
                                    MPIR_Comm *comm_ptr,
                                    MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, allgather, directspread);
    int i;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    MPIR_CHKLMEM_DECL(2);

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_allgather_directspread, 1);

    if (recvcnt == 0) {
        return MPI_SUCCESS;
    }

    /* get our rank and the size of this communicator */
    int rank = comm_ptr->rank;
    int size = comm_ptr->local_size;

    /* get extent of receive type */
    MPI_Aint recvtype_extent;
    MPIR_Datatype_get_extent_macro(recvtype, recvtype_extent);

    /* allocate an array of request objects */
    MPIR_Request **reqarray = NULL;
    MPIR_CHKLMEM_MALLOC(reqarray, MPIR_Request **,
                        2 * size * sizeof(MPIR_Request *), mpi_errno,
                        "reqarray", MPL_MEM_COLL);

    /* allocate an array of status objects */
    MPI_Status *starray = NULL;
    MPIR_CHKLMEM_MALLOC(starray, MPI_Status *, 2 * size * sizeof(MPI_Status),
                        mpi_errno, "starray", MPL_MEM_COLL);

    /* initialize our active request counter to 0 */
    int reqs = 0;

    /* copy our data to our receive buffer if needed */
    if (sendbuf != MPI_IN_PLACE) {
        /* compute location in receive buffer for our data */
        void *rbuf =
            (void *)((char *)recvbuf + rank * recvcnt * recvtype_extent);

        /* copy data from send buffer to receive buffer */
        mpi_errno =
            MPIR_Localcopy(sendbuf, sendcnt, sendtype, rbuf, recvcnt, recvtype);
        MPIR_ERR_CHECK(mpi_errno);
    }

    /* post receives */
    for (i = 1; i < size; i++) {
        /* compute source rank sending to us in this step */
        int src = rank - i;
        if (src < 0) {
            src += size;
        }

        /* get pointer to receive buffer for this rank */
        void *rbuf =
            (void *)((char *)recvbuf + src * recvcnt * recvtype_extent);

        /* post receive */
        MPIR_PVAR_INC(allgather, directspread, recv, recvcnt, recvtype);
        mpi_errno = MPIC_Irecv(rbuf, recvcnt, recvtype, src, MPIR_ALLGATHER_TAG,
                               comm_ptr, &reqarray[reqs++]);
        if (mpi_errno) {
            /* for communication errors, just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
    }

    /* TODO: consider placing a barrier here to ensure
     * receives are posted before sends, especially for large messages */
    // MPIR_Barrier_impl(comm_ptr);

    /* get parameters for sending data */
    const void *sbuf = sendbuf;
    int scnt = sendcnt;
    MPI_Datatype stype = sendtype;
    if (sendbuf == MPI_IN_PLACE) {
        /* use receive params if IN_PLACE */
        sbuf = (void *)((char *)recvbuf + rank * recvcnt * recvtype_extent);
        scnt = recvcnt;
        stype = recvtype;
    }

    for (i = 1; i < size; i++) {
        /* compute destination rank for this step */
        int dst = rank + i;
        if (dst >= size) {
            dst -= size;
        }

        /* post send to this destination rank */
        MPIR_PVAR_INC(allgather, directspread, send, scnt, stype);
        mpi_errno = MPIC_Isend(sbuf, scnt, stype, dst, MPIR_ALLGATHER_TAG,
                               comm_ptr, &reqarray[reqs++], errflag);
        if (mpi_errno) {
            /* for communication errors, just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
    }

    /* wait on all outstanding requests */
    mpi_errno = MPIC_Waitall(reqs, reqarray, starray, errflag);
    MPIR_ERR_CHECK(mpi_errno);

    /* --BEGIN ERROR HANDLING-- */
    if (mpi_errno == MPI_ERR_IN_STATUS) {
        for (i = 0; i < reqs; i++) {
            if (starray[i].MPI_ERROR != MPI_SUCCESS) {
                mpi_errno = starray[i].MPI_ERROR;
                if (mpi_errno) {
                    /* for communication errors,
                     * just record the error but continue */
                    *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                    MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                }
            }
        }
    }

    MPIR_CHKLMEM_FREEALL();
fn_fail:
    MPIR_TIMER_END(coll, allgather, directspread);
    return (mpi_errno);
}
