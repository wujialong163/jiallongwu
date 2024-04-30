#include "reduce_scatter_tuning.h"

int MPIR_Reduce_scatter_ring(const void *sendbuf, void *recvbuf,
                             const int *recvcnts, MPI_Datatype datatype,
                             MPI_Op op, MPIR_Comm *comm_ptr,
                             MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, reduce_scatter, ring);
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    int comm_size = comm_ptr->local_size;
    int rank = comm_ptr->rank;

    MPIR_CHKLMEM_DECL(3);

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_reduce_scatter_ring, 1);

    /* get extent */
    MPI_Aint extent;
    MPIR_Datatype_get_extent_macro(datatype, extent);

    /* get true extent and lower bound of datatype */
    MPI_Aint true_extent, true_lb;
    MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);

    /* compute size of temporary buffers */
    size_t mpi_buf_size = 1024 * 1024;
    if (mpi_buf_size < true_extent) {
        /* ensure we allocate a temporary buffer
         * big enough to hold at least one element */
        mpi_buf_size = (size_t)true_extent;
    }

    /* allocate buffers to work with */
    void *tmp_sendbuf;
    void *tmp_recvbuf;
    MPIR_CHKLMEM_MALLOC(tmp_sendbuf, char *, mpi_buf_size, mpi_errno,
                        "tmp_sendbuf", MPL_MEM_COLL);
    MPIR_CHKLMEM_MALLOC(tmp_recvbuf, char *, mpi_buf_size, mpi_errno,
                        "tmp_recvbuf", MPL_MEM_COLL);

    /* adjust pointers for lower bounds */
    tmp_sendbuf -= true_lb;
    tmp_recvbuf -= true_lb;

    /* compute left and right neighbors */
    int rank_left = (rank - 1 + comm_size) % comm_size;
    int rank_right = (rank + 1 + comm_size) % comm_size;

    /* Reduce_scatter */
    MPIR_Request *request[2];
    MPI_Status status[2];

    const void *input_buf = sendbuf;
    void *output_buf = recvbuf;

    /* if the caller gave us MPI_IN_PLACE, pull the input data from the
     * receive buffer instead of the senf buffer.  We do not bother copying
     * the input data to a temporary buffer, because it will have been
     * read by the time we overwrite it with the result. */
    if (sendbuf == MPI_IN_PLACE) {
        input_buf = recvbuf;
    }

    /* allocate memory for displacement array */
    int *displs = NULL;
    MPIR_CHKLMEM_MALLOC(displs, int *, comm_size * sizeof(int), mpi_errno,
                        "displs", MPL_MEM_COLL);

    /* total count of data for each rank */
    int total_count = 0;
    int max_recv_count = 0;
    int i;
    for (i = 0; i < comm_size; i++) {
        displs[i] = total_count;
        total_count += recvcnts[i];

        if (recvcnts[i] > max_recv_count) {
            max_recv_count = recvcnts[i];
        }
    }

    /* max number of elements a rank will receive */
    size_t max_elem_per_rank = (size_t)max_recv_count;

    /* compute number of whole elements that can fit in the buffer */
    size_t elem_per_buffer = mpi_buf_size / true_extent;

    /* process data in chunks of size elem_per_buffer */
    size_t nread = 0;
    while (nread < max_elem_per_rank) {
        /* keep track of send and recv counts left to process */
        /* execute a lap through the ring */
        int dist;
        for (dist = comm_size - 1; dist >= 0; dist--) {
            /* compute rank of process whose data we're sending and rank
             * of process whose data we're receiving in this step */
            int send_rank = (rank + dist) % comm_size;
            int recv_rank = (rank + (dist - 1)) % comm_size;

            /* compute offset into input buffer to pull data for this chunk */
            unsigned long elem_offset = displs[send_rank] + nread;
            const char *buf = (const char *)input_buf + elem_offset * extent;

            /* compute the number of elements we're sending and receiving */
            int send_count = recvcnts[send_rank] - nread;
            int recv_count = recvcnts[recv_rank] - nread;

            if (send_count < 0)
                send_count = 0;
            if (recv_count < 0)
                recv_count = 0;

            if (send_count > elem_per_buffer)
                send_count = elem_per_buffer;
            if (recv_count > elem_per_buffer)
                recv_count = elem_per_buffer;

            /* copy next set of bytes for this chunk
             * from input buffer into sendbuf */
            MPIR_Localcopy(buf, send_count, datatype, tmp_sendbuf, send_count,
                           datatype);

            /* merge the blocks via reduce operation */
            if (dist < comm_size - 1) {
                MPIR_Reduce_local(tmp_recvbuf, tmp_sendbuf, send_count,
                                  datatype, op);
            }

            if (dist > 0) {
                MPIR_PVAR_INC(reduce_scatter, ring, send, send_count, datatype);
                MPIR_PVAR_INC(reduce_scatter, ring, recv, recv_count, datatype);
                /* exchange data with neighbors */
                MPIC_Irecv(tmp_recvbuf, recv_count, datatype, rank_left, 0,
                           comm_ptr, &request[0]);
                MPIC_Isend(tmp_sendbuf, send_count, datatype, rank_right, 0,
                           comm_ptr, &request[1], errflag);
                MPIC_Waitall(2, request, status, errflag);
            } else {
                /* write the result to the output buffer */
                char *buf = output_buf + nread * extent;
                MPIR_Localcopy(tmp_sendbuf, send_count, datatype, buf,
                               send_count, datatype);
            }
        }

        /* assume we send the max buffer count in each step,
         * this means that nread may exceed the max value,
         * but that will end the loop */
        nread += elem_per_buffer;
    }

    /* bump pointers back to start of buffers for free calls */
    tmp_sendbuf += true_lb;
    tmp_recvbuf += true_lb;

fn_exit:
    MPIR_CHKLMEM_FREEALL();
    /* --BEGIN ERROR HANDLING-- */
    if (mpi_errno_ret)
        mpi_errno = mpi_errno_ret;
    else if (*errflag != MPIR_ERR_NONE)
        MPIR_ERR_SET(mpi_errno, *errflag, "**coll_fail");
    /* --END ERROR HANDLING-- */

    MPIR_TIMER_END(coll, reduce_scatter, ring);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
