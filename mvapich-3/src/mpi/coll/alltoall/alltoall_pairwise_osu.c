#include "alltoall_tuning.h"

int MPIR_Alltoall_pairwise_MVP(const void *sendbuf, int sendcount,
                               MPI_Datatype sendtype, void *recvbuf,
                               int recvcount, MPI_Datatype recvtype,
                               MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, alltoall, pw);
    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_alltoall_pw, 1);
    int comm_size, i, pof2;
    MPI_Aint sendtype_extent, recvtype_extent;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    int src, dst, rank;
    MPI_Status status;
    double start,end;
    int size1,size2;

    if (recvcount == 0) {
        MPIR_TIMER_END(coll, alltoall, pw);
        return MPI_SUCCESS;
    }

    comm_size = comm_ptr->local_size;
    rank = comm_ptr->rank;

    /* Get extent of send and recv types */
    MPIR_Datatype_get_extent_macro(recvtype, recvtype_extent);
    MPIR_Datatype_get_extent_macro(sendtype, sendtype_extent);

    /* Long message. If comm_size is a power-of-two, do a pairwise
     exchange using exclusive-or to create pairs. Else send to
     rank+i, receive from rank-i. */

    /* Make local copy first */
    mpi_errno = MPIR_Localcopy(
        ((char *)sendbuf + rank * sendcount * sendtype_extent), sendcount,
        sendtype, ((char *)recvbuf + rank * recvcount * recvtype_extent),
        recvcount, recvtype);
    MPIR_ERR_CHECK(mpi_errno);

    /* Is comm_size a power-of-two? */
    i = 1;
    while (i < comm_size) {
        i *= 2;
    }
    if (i == comm_size && MVP_USE_XOR_ALLTOALL) {
        pof2 = 1;
    } else {
        pof2 = 0;
    }

    /* Do the pairwise exchanges */
    for (i = 1; i < comm_size; i++) {
        if (pof2 == 1) {
            /* use exclusive-or algorithm */
            src = dst = rank ^ i;
        } else {
            src = (rank - i + comm_size) % comm_size;
            dst = (rank + i) % comm_size;
        }
        MPIR_PVAR_INC(alltoall, pw, send, sendcount, sendtype);
        MPIR_PVAR_INC(alltoall, pw, recv, recvcount, recvtype);
        start=PMPI_Wtime();
        mpi_errno = MPIC_Sendrecv(
            ((char *)sendbuf + dst * sendcount * sendtype_extent), sendcount,
            sendtype, dst, MPIR_ALLTOALL_TAG,
            ((char *)recvbuf + src * recvcount * recvtype_extent), recvcount,
            recvtype, src, MPIR_ALLTOALL_TAG, comm_ptr, &status, errflag);
        end=PMPI_Wtime();
        MPIR_Datatype_get_size_macro(sendtype, size1);
        MPIR_Datatype_get_size_macro(recvtype, size2);
        MPI_PVAR_DETAIL_INFO_INC(MVP,MPI_T_Alltoall,mvp_coll_alltoall,dst,src
            ,1,start,end,sendrecv,sendcount*size1,recvcount*size2);
        if (mpi_errno) {
            /* for communication errors, just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
    }

fn_fail:
    MPIR_TIMER_END(coll, alltoall, pw);
    return (mpi_errno);
}

/* Inplace alltoall using pairwise */
int MPIR_Alltoall_inplace_MVP(const void *sendbuf, int sendcount,
                              MPI_Datatype sendtype, void *recvbuf,
                              int recvcount, MPI_Datatype recvtype,
                              MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, alltoall, inplace);
    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_alltoall_inplace, 1);
    int comm_size, i, j;
    MPI_Aint recvtype_extent;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    int rank;
    MPI_Status status;
    double start,end;
    int size1,size2;
    if (recvcount == 0) {
        MPIR_TIMER_END(coll, alltoall, inplace);
        return MPI_SUCCESS;
    }

    comm_size = comm_ptr->local_size;
    rank = comm_ptr->rank;

    /* Get extent of recv type */
    MPIR_Datatype_get_extent_macro(recvtype, recvtype_extent);

    if (sendbuf != MPI_IN_PLACE) {
        mpi_errno =
            MPIR_Localcopy((char *)sendbuf, recvcount * comm_size, recvtype,
                           (char *)recvbuf, recvcount * comm_size, recvtype);
    }

    /* We use pair-wise sendrecv_replace in order to conserve memory usage,
     * which is keeping with the spirit of the MPI-2.2 Standard.  But
     * because of this approach all processes must agree on the global
     * schedule of sendrecv_replace operations to avoid deadlock.
     *
     * Note that this is not an especially efficient algorithm in terms of
     * time and there will be multiple repeated malloc/free's rather than
     * maintaining a single buffer across the whole loop.  Something like
     * MADRE is probably the best solution for the MPI_IN_PLACE scenario. */
    for (i = 0; i < comm_size; ++i) {
        /* start inner loop at i to avoid re-exchanging data */
        for (j = i; j < comm_size; ++j) {
            if (rank == i) {
                /* also covers the (rank == i && rank == j) case */
                MPIR_PVAR_INC(alltoall, inplace, send, recvcount, recvtype);
                MPIR_PVAR_INC(alltoall, inplace, recv, recvcount, recvtype);
                start=PMPI_Wtime();
                mpi_errno = MPIC_Sendrecv_replace(
                    ((char *)recvbuf + j * recvcount * recvtype_extent),
                    recvcount, recvtype, j, MPIR_ALLTOALL_TAG, j,
                    MPIR_ALLTOALL_TAG, comm_ptr, &status, errflag);
                end=PMPI_Wtime();
                MPIR_Datatype_get_size_macro(recvtype, size1);
                MPIR_Datatype_get_size_macro(recvtype, size2);
                MPI_PVAR_DETAIL_INFO_INC(MVP,MPI_T_Alltoall,mvp_coll_alltoall,j,j
            ,1,start,end,sendrecv,recvcount*size1,recvcount*size2);
                if (mpi_errno) {
                    /* for communication errors,
                     * just record the error but continue */
                    *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                    MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                }
            } else if (rank == j) {
                /* same as above with i/j args reversed */
                MPIR_PVAR_INC(alltoall, inplace, send, recvcount, recvtype);
                MPIR_PVAR_INC(alltoall, inplace, recv, recvcount, recvtype);
                start=PMPI_Wtime();
                mpi_errno = MPIC_Sendrecv_replace(
                    ((char *)recvbuf + i * recvcount * recvtype_extent),
                    recvcount, recvtype, i, MPIR_ALLTOALL_TAG, i,
                    MPIR_ALLTOALL_TAG, comm_ptr, &status, errflag);
                end=PMPI_Wtime();
                MPIR_Datatype_get_size_macro(recvtype, size1);
                MPIR_Datatype_get_size_macro(recvtype, size2);
                MPI_PVAR_DETAIL_INFO_INC(MVP,MPI_T_Alltoall,mvp_coll_alltoall,i,i
            ,1,start,end,sendrecv,recvcount*size1,recvcount*size2);
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
    MPIR_TIMER_END(coll, alltoall, inplace);
    return (mpi_errno);
}
