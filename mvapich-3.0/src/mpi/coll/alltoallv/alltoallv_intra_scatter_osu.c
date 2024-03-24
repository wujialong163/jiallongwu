#include "alltoallv_tuning.h"

/* begin:nested */
/* not declared static because a machine-specific function may call this one in
 * some cases */

int MPIR_Alltoallv_intra_scatter_MVP(const void *sendbuf, const int *sendcnts,
                                     const int *sdispls, MPI_Datatype sendtype,
                                     void *recvbuf, const int *recvcnts,
                                     const int *rdispls, MPI_Datatype recvtype,
                                     MPIR_Comm *comm_ptr,
                                     MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, alltoallv, intra_scatter);
    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_alltoallv_intra_scatter, 1);
    int comm_size, i, j;
    MPI_Aint send_extent, recv_extent;
    int mpi_errno = MPI_SUCCESS;
    int src, dst, rank;
    int ii, ss, bblock;
    int sreq_cnt, rreq_cnt;
    size_t sent_bytes = 0;
    size_t max_bytes = MVP_ALLTOALLV_INTERMEDIATE_WAIT_THRESHOLD;
    MPI_Status *sstarray = NULL;
    MPIR_Request **sreqarray = NULL;
    MPI_Status *rstarray = NULL;
    MPIR_Request **rreqarray = NULL;
    int total_count = 0;
    const void *sendbuf_tmp = NULL;
    MPI_Aint true_extent, true_lb;
    const int *sendcnt_tmp = NULL;
    const int *sdispls_tmp = NULL;
    MPI_Datatype sendtype_tmp;

    comm_size = comm_ptr->local_size;
    rank = comm_ptr->rank;

    /* Get extent of send and recv types */
    MPIR_Datatype_get_extent_macro(recvtype, recv_extent);
    MPIR_Datatype_get_extent_macro(recvtype, send_extent);

    /* If MPI_IN_PLACE is passed to the send buffer, copy the receive buffer to
     * sendbuf_tmp and then run as normal */
    if (sendbuf == MPI_IN_PLACE) {
        MPIR_Type_get_true_extent_impl(recvtype, &true_lb, &true_extent);

        total_count = rdispls[comm_size - 1] + recvcnts[comm_size - 1];

        sendbuf_tmp = (char *)MPL_malloc(
            total_count * (MPL_MAX(recv_extent, true_extent)), MPL_MEM_COLL);

        /* adjust for potential negative lower bound in datatype */
        sendbuf_tmp = (void *)((char *)sendbuf_tmp - true_lb);

        mpi_errno =
            MPIR_Localcopy(((char *)recvbuf), total_count, recvtype,
                           ((char *)sendbuf_tmp), total_count, recvtype);

        /* in th MPI_IN_PLACE case, all the following for sender could be NULL,
         * use the receiver info instead of sender info  */
        sendcnt_tmp = recvcnts;
        sdispls_tmp = rdispls;
        sendtype_tmp = recvtype;

        MPIR_Datatype_get_extent_macro(sendtype_tmp, send_extent);

    } else {
        sendbuf_tmp = sendbuf;
        sendcnt_tmp = sendcnts;
        sdispls_tmp = sdispls;
        sendtype_tmp = sendtype;

        MPIR_Datatype_get_extent_macro(sendtype, send_extent);
    }

    MPIR_CHKLMEM_DECL(4);
    bblock = MVP_ALLTOALL_THROTTLE_FACTOR;

    if (bblock >= comm_size) {
        bblock = comm_size;
    }
    /* If throttle_factor is n, each process posts n pairs of isend/irecv
     * in each iteration. */

    /* FIXME: This should use the memory macros (there are storage
     * leaks here if there is an error, for example) */
    MPIR_CHKLMEM_MALLOC(sreqarray, MPIR_Request **,
                        bblock * sizeof(MPIR_Request *), mpi_errno, "sreqarray",
                        MPL_MEM_COLL);

    MPIR_CHKLMEM_MALLOC(sstarray, MPI_Status *, bblock * sizeof(MPI_Status),
                        mpi_errno, "sstarray", MPL_MEM_COLL);

    MPIR_CHKLMEM_MALLOC(rreqarray, MPIR_Request **,
                        bblock * sizeof(MPIR_Request *), mpi_errno, "rreqarray",
                        MPL_MEM_COLL);

    MPIR_CHKLMEM_MALLOC(rstarray, MPI_Status *, bblock * sizeof(MPI_Status),
                        mpi_errno, "rstarray", MPL_MEM_COLL);

    mpi_errno =
        MPIR_Localcopy(((char *)sendbuf_tmp + sdispls_tmp[rank] * send_extent),
                       sendcnt_tmp[rank], sendtype_tmp,
                       ((char *)recvbuf + rdispls[rank] * recv_extent),
                       recvcnts[rank], recvtype);
    if (mpi_errno) {
        mpi_errno =
            MPIR_Err_create_code(mpi_errno, MPIR_ERR_RECOVERABLE, __func__,
                                 __LINE__, MPI_ERR_OTHER, "**fail", 0);
        goto fn_fail;
    }

    /* Do the exchanges */
    for (i = 0, ii = 0; ii < comm_size; ii += bblock) {
        sreq_cnt = rreq_cnt = 0;
        ss = comm_size - ii < bblock ? comm_size - ii : bblock;
        /* do the communication -- post ss receives: */
        for (i = 0; i < ss; i++) {
            src = (rank + i + ii) % comm_size;
            MPIR_PVAR_INC(alltoallv, intra, recv, recvcnts[src], recvtype);
            MPIR_PVAR_INC(alltoallv, intra_scatter, recv, recvcnts[src],
                          recvtype);
            mpi_errno =
                MPIC_Irecv(((char *)recvbuf + rdispls[src] * recv_extent),
                           recvcnts[src], recvtype, src, MPIR_ALLTOALLV_TAG,
                           comm_ptr, &rreqarray[rreq_cnt]);
            MPIR_ERR_CHECK(mpi_errno);
            rreq_cnt++;
        }
        /* do the communication -- post ss sends : */
        for (i = 0; i < ss; i++) {
            dst = (rank - i - ii + comm_size) % comm_size;
            MPIR_PVAR_INC(alltoallv, intra, send, sendcnt_tmp[dst],
                          sendtype_tmp);
            MPIR_PVAR_INC(alltoallv, intra_scatter, send, sendcnt_tmp[dst],
                          sendtype_tmp);
            mpi_errno = MPIC_Isend(
                ((char *)sendbuf_tmp + sdispls_tmp[dst] * send_extent),
                sendcnt_tmp[dst], sendtype_tmp, dst, MPIR_ALLTOALLV_TAG,
                comm_ptr, &sreqarray[sreq_cnt], errflag);
            MPIR_ERR_CHECK(mpi_errno);
            sreq_cnt++;

            /* Throttle sends dynamically if pushing large amount of data */
            sent_bytes += send_extent * sendcnt_tmp[dst];
            if (max_bytes && sent_bytes >= max_bytes) {
                mpi_errno =
                    MPIC_Waitall(sreq_cnt, sreqarray, sstarray, errflag);
                MPIR_ERR_CHECK(mpi_errno);
                sreq_cnt = 0;
            }
        }

        /* wait for recv to complete then wait for remaining sends*/
        mpi_errno = MPIC_Waitall(rreq_cnt, rreqarray, rstarray, errflag);
        MPIR_ERR_CHECK(mpi_errno);

        mpi_errno = MPIC_Waitall(sreq_cnt, sreqarray, sstarray, errflag);
        MPIR_ERR_CHECK(mpi_errno);

        /* --BEGIN ERROR HANDLING-- */
        if (mpi_errno == MPI_ERR_IN_STATUS) {
            for (j = 0; j < rreq_cnt; j++) {
                if (rstarray[j].MPI_ERROR != MPI_SUCCESS) {
                    mpi_errno = rstarray[j].MPI_ERROR;
                }
            }
            for (j = 0; j < sreq_cnt; j++) {
                if (sstarray[j].MPI_ERROR != MPI_SUCCESS) {
                    mpi_errno = sstarray[j].MPI_ERROR;
                }
            }
        }
    }

    /* --END ERROR HANDLING-- */
    MPIR_CHKLMEM_FREEALL();
    if (sendbuf == MPI_IN_PLACE) {
        MPL_free((void *)sendbuf_tmp);
    }

fn_exit:
    MPIR_TIMER_END(coll, alltoallv, intra_scatter);
    return (mpi_errno);

fn_fail:
    goto fn_exit;
}
