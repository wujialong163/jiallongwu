#include "gather_tuning.h"

int MPIR_Gather_MVP_Direct(const void *sendbuf, int sendcnt,
                           MPI_Datatype sendtype, void *recvbuf, int recvcnt,
                           MPI_Datatype recvtype, int root, MPIR_Comm *comm_ptr,
                           MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, gather, direct);
    int comm_size, rank;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    MPI_Aint extent = 0; /* Datatype extent */
    int reqs = 0, i = 0;
    MPIR_Request **reqarray;
    MPI_Status *starray;
    MPIR_CHKLMEM_DECL(2);

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_gather_direct, 1);

    comm_size = comm_ptr->local_size;
    rank = comm_ptr->rank;

    if (((rank == root) && (recvcnt == 0)) ||
        ((rank != root) && (sendcnt == 0))) {
        MPIR_TIMER_END(coll, gather, direct);
        return MPI_SUCCESS;
    }

    if (root == rank) {
        comm_size = comm_ptr->local_size;

        MPIR_Datatype_get_extent_macro(recvtype, extent);

        MPIR_CHKLMEM_MALLOC(reqarray, MPIR_Request **,
                            comm_size * sizeof(MPIR_Request *), mpi_errno,
                            "reqarray", MPL_MEM_COLL);
        MPIR_CHKLMEM_MALLOC(starray, MPI_Status *,
                            comm_size * sizeof(MPI_Status), mpi_errno,
                            "starray", MPL_MEM_COLL);

        reqs = 0;
        for (i = 0; i < comm_size; i++) {
            if (i == rank) {
                if (sendbuf != MPI_IN_PLACE) {
                    mpi_errno = MPIR_Localcopy(
                        sendbuf, sendcnt, sendtype,
                        ((char *)recvbuf + rank * recvcnt * extent), recvcnt,
                        recvtype);
                }
            } else {
                MPIR_PVAR_INC(gather, direct, recv, recvcnt, recvtype);
                mpi_errno = MPIC_Irecv(((char *)recvbuf + i * recvcnt * extent),
                                       recvcnt, recvtype, i, MPIR_GATHER_TAG,
                                       comm_ptr, &reqarray[reqs++]);
            }
            /* --BEGIN ERROR HANDLING-- */
            if (mpi_errno) {
                mpi_errno = MPIR_Err_create_code(
                    mpi_errno, MPIR_ERR_RECOVERABLE, __func__, __LINE__,
                    MPI_ERR_OTHER, "**fail", 0);
                return mpi_errno;
            }
            /* --END ERROR HANDLING-- */
        }
        /* ... then wait for *all* of them to finish: */
        mpi_errno = MPIC_Waitall(reqs, reqarray, starray, errflag);
        /* --BEGIN ERROR HANDLING-- */
        if (mpi_errno == MPI_ERR_IN_STATUS) {
            for (i = 0; i < reqs; i++) {
                if (starray[i].MPI_ERROR != MPI_SUCCESS) {
                    mpi_errno = starray[i].MPI_ERROR;
                    if (mpi_errno) {
                        /* for communication errors, just record
                           the error but continue */
                        *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                        MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                    }
                }
            }
        }
        /* --END ERROR HANDLING-- */
    } else if (root != rank) { /* non-root nodes processes */
        if (sendcnt) {
            comm_size = comm_ptr->local_size;
            if (sendbuf != MPI_IN_PLACE) {
                MPIR_PVAR_INC(gather, direct, send, sendcnt, sendtype);
                mpi_errno = MPIC_Send(sendbuf, sendcnt, sendtype, root,
                                      MPIR_GATHER_TAG, comm_ptr, errflag);
            } else {
                MPIR_PVAR_INC(gather, direct, send, sendcnt, sendtype);
                mpi_errno = MPIC_Send(recvbuf, sendcnt, sendtype, root,
                                      MPIR_GATHER_TAG, comm_ptr, errflag);
            }
            if (mpi_errno) {
                /* for communication errors,
                 * just record the error but continue */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
        }
    }
fn_fail:
    /* check if multiple threads are calling this collective function */
    MPIR_CHKLMEM_FREEALL();

    MPIR_TIMER_END(coll, gather, direct);
    return (mpi_errno);
}

int MPIR_Gather_MVP_Direct_Blk(const void *sendbuf, int sendcnt,
                               MPI_Datatype sendtype, void *recvbuf,
                               int recvcnt, MPI_Datatype recvtype, int root,
                               MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, gather, direct_blk);
    int comm_size, rank;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    MPI_Aint extent = 0; /* Datatype extent */
    int i = 0;

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_gather_direct_blk, 1);

    comm_size = comm_ptr->local_size;
    rank = comm_ptr->rank;

    if (((rank == root) && (recvcnt == 0)) ||
        ((rank != root) && (sendcnt == 0))) {
        MPIR_TIMER_END(coll, gather, direct_blk);
        return MPI_SUCCESS;
    }

    if (root == rank) {
        comm_size = comm_ptr->local_size;

        MPIR_Datatype_get_extent_macro(recvtype, extent);

        for (i = 0; i < comm_size; i++) {
            if (i == rank) {
                if (sendbuf != MPI_IN_PLACE) {
                    mpi_errno = MPIR_Localcopy(
                        sendbuf, sendcnt, sendtype,
                        ((char *)recvbuf + rank * recvcnt * extent), recvcnt,
                        recvtype);
                    MPIR_ERR_CHECK(mpi_errno);
                }
            } else {
                MPIR_PVAR_INC(gather, direct_blk, recv, recvcnt, recvtype);
                mpi_errno = MPIC_Recv(((char *)recvbuf + i * recvcnt * extent),
                                      recvcnt, recvtype, i, MPIR_GATHER_TAG,
                                      comm_ptr, MPI_STATUS_IGNORE, errflag);
            }
            /* --BEGIN ERROR HANDLING-- */
            if (mpi_errno) {
                mpi_errno = MPIR_Err_create_code(
                    mpi_errno, MPIR_ERR_RECOVERABLE, __func__, __LINE__,
                    MPI_ERR_OTHER, "**fail", 0);

                MPIR_TIMER_END(coll, gather, direct_blk);
                return mpi_errno;
            }
            /* --END ERROR HANDLING-- */
        }
    } else if (root != rank) { /* non-root nodes processes */
        if (sendcnt) {
            comm_size = comm_ptr->local_size;
            if (sendbuf != MPI_IN_PLACE) {
                MPIR_PVAR_INC(gather, direct_blk, send, sendcnt, sendtype);
                mpi_errno = MPIC_Send(sendbuf, sendcnt, sendtype, root,
                                      MPIR_GATHER_TAG, comm_ptr, errflag);
            } else {
                MPIR_PVAR_INC(gather, direct_blk, send, sendcnt, sendtype);
                mpi_errno = MPIC_Send(recvbuf, sendcnt, sendtype, root,
                                      MPIR_GATHER_TAG, comm_ptr, errflag);
            }
            if (mpi_errno) {
                /* for communication errors,
                 * just record the error but continue */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
        }
    }

fn_exit:
    MPIR_TIMER_END(coll, gather, direct_blk);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
