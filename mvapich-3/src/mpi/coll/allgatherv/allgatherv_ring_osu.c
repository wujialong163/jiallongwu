#include "allgatherv_tuning.h"

#undef FUNCNAME
#define FUNCNAME MPIR_Allgatherv_Ring_MVP
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPIR_Allgatherv_Ring_MVP(const void *sendbuf, int sendcount,
                             MPI_Datatype sendtype, void *recvbuf,
                             const int *recvcounts, const int *displs,
                             MPI_Datatype recvtype, MPIR_Comm *comm_ptr,
                             MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, allgatherv, ring);
    int comm_size, rank, i, left, right, total_count;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    MPI_Status status;
    MPI_Aint recvtype_extent;

    /* User has not forced algorthitm selection and non-block allocation is used
     */
    if (MVP_ALLGATHERV_COLLECTIVE_ALGORITHM_UNSET ==
            MVP_ALLGATHERV_COLLECTIVE_ALGORITHM &&
        comm_ptr->dev.ch.rank_list != NULL &&
        comm_ptr->dev.ch.is_blocked != 1) {
        return MPIR_Allgatherv_Ring_Cyclic_MVP(sendbuf, sendcount, sendtype,
                                               recvbuf, recvcounts, displs,
                                               recvtype, comm_ptr, errflag);
    }

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_allgatherv_ring, 1);

    comm_size = comm_ptr->local_size;
    rank = comm_ptr->rank;

    total_count = 0;
    for (i = 0; i < comm_size; i++)
        total_count += recvcounts[i];

    if (total_count == 0)
        goto fn_exit;

    MPIR_Datatype_get_extent_macro(recvtype, recvtype_extent);

    char *sbuf = NULL, *rbuf = NULL;
    int soffset, roffset;
    int torecv, tosend, min;
    int sendnow, recvnow;
    int sindex, rindex;

    if (sendbuf != MPI_IN_PLACE) {
        /* First, load the "local" version in the recvbuf. */
        mpi_errno =
            MPIR_Localcopy(sendbuf, sendcount, sendtype,
                           ((char *)recvbuf + displs[rank] * recvtype_extent),
                           recvcounts[rank], recvtype);
        MPIR_ERR_CHECK(mpi_errno);
    }

    left = (comm_size + rank - 1) % comm_size;
    right = (rank + 1) % comm_size;

    torecv = total_count - recvcounts[rank];
    tosend = total_count - recvcounts[right];

    min = recvcounts[0];
    for (i = 1; i < comm_size; i++)
        if (min > recvcounts[i])
            min = recvcounts[i];
    if (min * recvtype_extent < MPIR_CVAR_ALLGATHERV_PIPELINE_MSG_SIZE)
        min = MPIR_CVAR_ALLGATHERV_PIPELINE_MSG_SIZE / recvtype_extent;
    /* Handle the case where the datatype extent is larger than
     * the pipeline size. */
    if (!min)
        min = 1;

    sindex = rank;
    rindex = left;
    soffset = 0;
    roffset = 0;
    while (tosend || torecv) { /* While we have data to send or receive */
        sendnow = ((recvcounts[sindex] - soffset) > min) ?
                      min :
                      (recvcounts[sindex] - soffset);
        recvnow = ((recvcounts[rindex] - roffset) > min) ?
                      min :
                      (recvcounts[rindex] - roffset);
        sbuf = (char *)recvbuf + ((displs[sindex] + soffset) * recvtype_extent);
        rbuf = (char *)recvbuf + ((displs[rindex] + roffset) * recvtype_extent);

        /* Protect against wrap-around of indices */
        if (!tosend)
            sendnow = 0;
        if (!torecv)
            recvnow = 0;

        /* Communicate */
        if (!sendnow && !recvnow) {
            /* Don't do anything. This case is possible if two
             * consecutive processes contribute 0 bytes each. */
        } else if (!sendnow) {
            /* If there's no data to send, just do a recv call */
            MPIR_PVAR_INC(allgatherv, ring, recv, recvnow, recvtype);
            mpi_errno =
                MPIC_Recv(rbuf, recvnow, recvtype, left, MPIR_ALLGATHERV_TAG,
                          comm_ptr, &status, errflag);
            if (mpi_errno) {
                /* for communication errors,
                 * just record the error but continue */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
            torecv -= recvnow;
        } else if (!recvnow) {
            /* If there's no data to receive, just do a send call */
            MPIR_PVAR_INC(allgatherv, ring, send, sendnow, recvtype);
            mpi_errno = MPIC_Send(sbuf, sendnow, recvtype, right,
                                  MPIR_ALLGATHERV_TAG, comm_ptr, errflag);
            if (mpi_errno) {
                /* for communication errors,
                 * just record the error but continue */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
            tosend -= sendnow;
        } else { /* There's data to be sent and received */
            MPIR_PVAR_INC(allgatherv, ring, send, sendnow, recvtype);
            MPIR_PVAR_INC(allgatherv, ring, recv, recvnow, recvtype);
            mpi_errno = MPIC_Sendrecv(sbuf, sendnow, recvtype, right,
                                      MPIR_ALLGATHERV_TAG, rbuf, recvnow,
                                      recvtype, left, MPIR_ALLGATHERV_TAG,
                                      comm_ptr, &status, errflag);
            if (mpi_errno) {
                /* for communication errors,
                 * just record the error but continue */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
            tosend -= sendnow;
            torecv -= recvnow;
        }

        soffset += sendnow;
        roffset += recvnow;
        if (soffset == recvcounts[sindex]) {
            soffset = 0;
            sindex = (sindex + comm_size - 1) % comm_size;
        }
        if (roffset == recvcounts[rindex]) {
            roffset = 0;
            rindex = (rindex + comm_size - 1) % comm_size;
        }
    }
fn_exit:
    if (mpi_errno_ret)
        mpi_errno = mpi_errno_ret;
    else if (*errflag)
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**coll_fail");

    MPIR_TIMER_END(coll, allgatherv, ring);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

/* Execute an allgatherv by forwarding data through a ring of
 * processes.  This implementation uses the two-level data
 * structures to account for how procs are assigned to nodes
 * to ensure data is only sent into and out of each node once. */
#undef FUNCNAME
#define FUNCNAME MPIR_Allgatherv_Ring_Cyclic_MVP
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPIR_Allgatherv_Ring_Cyclic_MVP(const void *sendbuf, int sendcount,
                                    MPI_Datatype sendtype, void *recvbuf,
                                    const int *recvcounts, const int *displs,
                                    MPI_Datatype recvtype, MPIR_Comm *comm_ptr,
                                    MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, allgatherv, ring_cyclic);
    int comm_size, rank, i, total_count;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    MPI_Status status;
    MPI_Aint recvtype_extent;

    /* rank_list is not initialized until create_2level_comm is called */
    if (!comm_ptr->dev.ch.rank_list) {
        return MPIR_Allgatherv_Ring_MVP(sendbuf, sendcount, sendtype, recvbuf,
                                        recvcounts, displs, recvtype, comm_ptr,
                                        errflag);
    }

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_allgatherv_ring_cyclic, 1);

    comm_size = comm_ptr->local_size;
    rank = comm_ptr->rank;

    total_count = 0;
    for (i = 0; i < comm_size; i++)
        total_count += recvcounts[i];

    if (total_count == 0)
        goto fn_exit;

    MPIR_Datatype_get_extent_macro(recvtype, recvtype_extent);

    char *sbuf = NULL, *rbuf = NULL;
    int soffset, roffset;
    int torecv, tosend, min;
    int sendnow, recvnow;

    if (sendbuf != MPI_IN_PLACE) {
        /* First, load the "local" version in the recvbuf. */
        mpi_errno =
            MPIR_Localcopy(sendbuf, sendcount, sendtype,
                           ((char *)recvbuf + displs[rank] * recvtype_extent),
                           recvcounts[rank], recvtype);
        MPIR_ERR_CHECK(mpi_errno);
    }

    /* lookup our index in the rank list */
    int rank_index = comm_ptr->dev.ch.rank_list_index;

    /* compute the left and right neighbor ranks in the rank_list */
    int left_index = (comm_size + rank_index - 1) % comm_size;
    int right_index = (comm_size + rank_index + 1) % comm_size;
    int left = comm_ptr->dev.ch.rank_list[left_index];
    int right = comm_ptr->dev.ch.rank_list[right_index];

    /* execute ring exchange, start by sending our own data to the right
     * and receiving the data from the rank to our left */
    int send_index = rank_index;
    int recv_index = left_index;

    torecv = total_count - recvcounts[rank];
    tosend = total_count - recvcounts[right];

    min = recvcounts[0];
    for (i = 1; i < comm_size; i++)
        if (min > recvcounts[i])
            min = recvcounts[i];
    if (min * recvtype_extent < MPIR_CVAR_ALLGATHERV_PIPELINE_MSG_SIZE)
        min = MPIR_CVAR_ALLGATHERV_PIPELINE_MSG_SIZE / recvtype_extent;
    /* Handle the case where the datatype extent is larger than
     * the pipeline size. */
    if (!min)
        min = 1;

    /* execute ring exchange, start by sending our own data to the right
     * and receiving the data from the rank to our left */
    soffset = 0;
    roffset = 0;
    while (tosend || torecv) { /* While we have data to send or receive */

        /* compute ranks whose data we'll send and receive in this step */
        int send_rank = comm_ptr->dev.ch.rank_list[send_index];
        int recv_rank = comm_ptr->dev.ch.rank_list[recv_index];

        sendnow = ((recvcounts[send_rank] - soffset) > min) ?
                      min :
                      (recvcounts[send_rank] - soffset);
        recvnow = ((recvcounts[recv_rank] - roffset) > min) ?
                      min :
                      (recvcounts[recv_rank] - roffset);
        sbuf =
            (char *)recvbuf + ((displs[send_rank] + soffset) * recvtype_extent);
        rbuf =
            (char *)recvbuf + ((displs[recv_rank] + roffset) * recvtype_extent);

        /* Protect against wrap-around of indices */
        if (!tosend)
            sendnow = 0;
        if (!torecv)
            recvnow = 0;

        /* Communicate */
        if (!sendnow && !recvnow) {
            /* Don't do anything. This case is possible if two
             * consecutive processes contribute 0 bytes each. */
        } else if (!sendnow) {
            /* If there's no data to send, just do a recv call */
            mpi_errno =
                MPIC_Recv(rbuf, recvnow, recvtype, left, MPIR_ALLGATHERV_TAG,
                          comm_ptr, &status, errflag);
            if (mpi_errno) {
                /* for communication errors,
                 * just record the error but continue */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
            torecv -= recvnow;
        } else if (!recvnow) {
            /* If there's no data to receive, just do a send call */
            mpi_errno = MPIC_Send(sbuf, sendnow, recvtype, right,
                                  MPIR_ALLGATHERV_TAG, comm_ptr, errflag);
            if (mpi_errno) {
                /* for communication errors,
                 * just record the error but continue */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
            tosend -= sendnow;
        } else {
            /* There's data to be sent and received */
            mpi_errno = MPIC_Sendrecv(sbuf, sendnow, recvtype, right,
                                      MPIR_ALLGATHERV_TAG, rbuf, recvnow,
                                      recvtype, left, MPIR_ALLGATHERV_TAG,
                                      comm_ptr, &status, errflag);
            if (mpi_errno) {
                /* for communication errors,
                 * just record the error but continue */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
            tosend -= sendnow;
            torecv -= recvnow;
        }

        soffset += sendnow;
        roffset += recvnow;
        if (soffset == recvcounts[send_rank]) {
            soffset = 0;
            send_index = (send_index + comm_size - 1) % comm_size;
        }
        if (roffset == recvcounts[recv_rank]) {
            roffset = 0;
            recv_index = (recv_index + comm_size - 1) % comm_size;
        }
    }
fn_exit:
    if (mpi_errno_ret)
        mpi_errno = mpi_errno_ret;
    else if (*errflag)
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**coll_fail");

    MPIR_TIMER_END(coll, allgatherv, ring_cyclic);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
