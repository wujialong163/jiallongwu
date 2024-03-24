#include "reduce_scatter_tuning.h"

/* Implements the "mirror permutation" of "bits" bits of an integer "x".
 * positions 76543210, bits==3 yields 76543012.
 * This function could/should be moved to a common utility location for use in
 * other collectives as well. */

ATTRIBUTE((const))
/* tells the compiler that this func only depends on its args
 * and may be optimized much more aggressively, similar to "pure" */
static inline int mirror_permutation(unsigned int x, int bits)
{
    /* a mask for the high order bits that should be copied as-is */
    int high_mask = ~((0x1 << bits) - 1);
    int retval = x & high_mask;
    int i;

    for (i = 0; i < bits; ++i) {
        unsigned int bitval = (x & (0x1 << i)) >> i; /* 0x1 or 0x0 */
        retval |= bitval << ((bits - i) - 1);
    }

    return retval;
}

/* Implements the reduce-scatter butterfly algorithm described in J. L. Traff's
 * "An Improved Algorithm for (Non-commutative) Reduce-Scatter
 * with an Application" from EuroPVM/MPI 2005.
 * This function currently only implements support for
 * the power-of-2, block-regular case (all receive counts are equal). */
int MPIR_Reduce_scatter_noncomm_MVP(const void *sendbuf, void *recvbuf,
                                    const int *recvcnts, MPI_Datatype datatype,
                                    MPI_Op op, MPIR_Comm *comm_ptr,
                                    MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, reduce_scatter, noncomm);
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    int comm_size = comm_ptr->local_size;
    int rank = comm_ptr->rank;
    int pof2;
    int log2_comm_size;
    int i, k;
    int recv_offset, send_offset;
    int block_size, total_count, size;
    MPI_Aint true_extent, true_lb;
    int buf0_was_inout;
    void *tmp_buf0;
    void *tmp_buf1;
    void *result_ptr;
    MPI_User_function *uop;
    int is_commutative;
#ifdef HAVE_CXX_BINDING
    int is_cxx_uop = 0;
#endif
    MPIR_CHKLMEM_DECL(3);

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_reduce_scatter_noncomm, 1);

    MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);

    /* Get func ptr for the reduction op
     * and initialize is_commutative and is_cxx_uop */
    MPIR_get_op_ptr(&uop, &is_commutative,
#ifdef HAVE_CXX_BINDING
                    &is_cxx_uop,
#endif
                    op);

    pof2 = 1;
    log2_comm_size = 0;
    while (pof2 < comm_size) {
        pof2 <<= 1;
        ++log2_comm_size;
    }

    /* begin error checking */
    MPIR_Assert(pof2 == comm_size);
    /* FIXME this version only works for power of 2 procs */

    for (i = 0; i < (comm_size - 1); ++i) {
        MPIR_Assert(recvcnts[i] == recvcnts[i + 1]);
    }
    /* end error checking */

    /* size of a block (count of datatype per block, NOT bytes per block) */
    block_size = recvcnts[0];
    total_count = block_size * comm_size;

    MPIR_CHKLMEM_MALLOC(tmp_buf0, void *, true_extent *total_count, mpi_errno,
                        "tmp_buf0", MPL_MEM_COLL);
    MPIR_CHKLMEM_MALLOC(tmp_buf1, void *, true_extent *total_count, mpi_errno,
                        "tmp_buf1", MPL_MEM_COLL);
    /* adjust for potential negative lower bound in datatype */
    tmp_buf0 = (void *)((char *)tmp_buf0 - true_lb);
    tmp_buf1 = (void *)((char *)tmp_buf1 - true_lb);

    /* Copy our send data to tmp_buf0.  We do this one block at a time and
       permute the blocks as we go according to the mirror permutation. */
    for (i = 0; i < comm_size; ++i) {
        mpi_errno = MPIR_Localcopy(
            (char *)(sendbuf == MPI_IN_PLACE ? recvbuf : sendbuf) +
                (i * true_extent * block_size),
            block_size, datatype,
            (char *)tmp_buf0 + (mirror_permutation(i, log2_comm_size) *
                                true_extent * block_size),
            block_size, datatype);
        MPIR_ERR_CHECK(mpi_errno);
    }
    buf0_was_inout = 1;

    send_offset = 0;
    recv_offset = 0;
    size = total_count;
    for (k = 0; k < log2_comm_size; ++k) {
        /* use a double-buffering scheme to avoid local copies */
        char *incoming_data = (buf0_was_inout ? tmp_buf1 : tmp_buf0);
        char *outgoing_data = (buf0_was_inout ? tmp_buf0 : tmp_buf1);
        int peer = rank ^ (0x1 << k);
        size /= 2;

        if (rank > peer) {
            /* we have the higher rank: send top half,
             * recv bottom half */
            recv_offset += size;
        } else {
            /* we have the lower rank: recv top half,
             * send bottom half */
            send_offset += size;
        }

        MPIR_PVAR_INC(reduce_scatter, noncomm, send, size, datatype);
        MPIR_PVAR_INC(reduce_scatter, noncomm, recv, size, datatype);
        mpi_errno = MPIC_Sendrecv(outgoing_data + send_offset * true_extent,
                                  size, datatype, peer, MPIR_REDUCE_SCATTER_TAG,
                                  incoming_data + recv_offset * true_extent,
                                  size, datatype, peer, MPIR_REDUCE_SCATTER_TAG,
                                  comm_ptr, MPI_STATUS_IGNORE, errflag);
        if (mpi_errno) {
            /* for communication errors,
             * just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
        /* always perform the reduction at recv_offset,
                 * the data at send_offset
           is now our peer's responsibility */
        if (rank > peer) {
            /* higher ranked value
             * so need to call op(received_data, my_data) */
            MPIR_MVP_Reduce_local(incoming_data + recv_offset * true_extent,
                                  outgoing_data + recv_offset * true_extent,
                                  (MPI_Aint)size, datatype, uop
#ifdef HAVE_CXX_BINDING
                                  ,
                                  is_cxx_uop
#endif
            );
            buf0_was_inout = buf0_was_inout;
        } else {
            /* lower ranked value
             * so need to call op(my_data, received_data) */
            MPIR_MVP_Reduce_local(outgoing_data + recv_offset * true_extent,
                                  incoming_data + recv_offset * true_extent,
                                  (MPI_Aint)size, datatype, uop
#ifdef HAVE_CXX_BINDING
                                  ,
                                  is_cxx_uop
#endif
            );
            buf0_was_inout = !buf0_was_inout;
        }

        /* the next round of send/recv needs to happen
         * within the block (of size "size")
         * that we just received and reduced */
        send_offset = recv_offset;
    }

    MPIR_Assert(size == recvcnts[rank]);

    /* copy the reduced data to the recvbuf */
    result_ptr = (char *)(buf0_was_inout ? tmp_buf0 : tmp_buf1) +
                 recv_offset * true_extent;
    mpi_errno =
        MPIR_Localcopy(result_ptr, size, datatype, recvbuf, size, datatype);
fn_exit:
    MPIR_CHKLMEM_FREEALL();
    if (mpi_errno_ret)
        mpi_errno = mpi_errno_ret;
    else if (*errflag)
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**coll_fail");
    MPIR_TIMER_END(coll, reduce_scatter, noncomm);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
