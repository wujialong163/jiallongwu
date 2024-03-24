#include "alltoall_tuning.h"

int MPIR_Alltoall_bruck_MVP(const void *sendbuf, int sendcount,
                            MPI_Datatype sendtype, void *recvbuf, int recvcount,
                            MPI_Datatype recvtype, MPIR_Comm *comm_ptr,
                            MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, alltoall, bruck);
    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_alltoall_bruck, 1);
    int comm_size, i, pof2;
    MPI_Aint sendtype_extent, recvtype_extent;
    MPI_Aint recvtype_true_extent, recvbuf_extent, recvtype_true_lb;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    int src, dst, rank;
    int block, *displs, count;
    MPI_Aint pack_size, position;
    MPI_Datatype newtype = MPI_DATATYPE_NULL;
    void *tmp_buf;

    if (recvcount == 0) {
        MPIR_TIMER_END(coll, alltoall, bruck);
        return MPI_SUCCESS;
    }

    comm_size = comm_ptr->local_size;
    rank = comm_ptr->rank;

    /* Get extent of send and recv types */
    MPIR_Datatype_get_extent_macro(recvtype, recvtype_extent);
    MPIR_Datatype_get_extent_macro(sendtype, sendtype_extent);

    /* use the indexing algorithm by Jehoshua Bruck et al,
     * IEEE TPDS, Nov. 97 */

    /* allocate temporary buffer */
    MPIR_Pack_size_impl(recvcount * comm_size, recvtype, &pack_size);
    tmp_buf = MPL_malloc(pack_size, MPL_MEM_COLL);
    /* --BEGIN ERROR HANDLING-- */
    if (!tmp_buf) {
        mpi_errno =
            MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, __func__,
                                 __LINE__, MPI_ERR_OTHER, "**nomem", 0);

        MPIR_TIMER_END(coll, alltoall, bruck);
        return mpi_errno;
    }
    /* --END ERROR HANDLING-- */

    /* Do Phase 1 of the algorithim. Shift the data blocks on process i
     * upwards by a distance of i blocks. Store the result in recvbuf. */
    mpi_errno =
        MPIR_Localcopy((char *)sendbuf + rank * sendcount * sendtype_extent,
                       (comm_size - rank) * sendcount, sendtype, recvbuf,
                       (comm_size - rank) * recvcount, recvtype);
    MPIR_ERR_CHECK(mpi_errno);
    mpi_errno = MPIR_Localcopy(
        sendbuf, rank * sendcount, sendtype,
        (char *)recvbuf + (comm_size - rank) * recvcount * recvtype_extent,
        rank * recvcount, recvtype);
    MPIR_ERR_CHECK(mpi_errno);
    /* Input data is now stored in recvbuf with datatype recvtype */

    /* Now do Phase 2, the communication phase. It takes
     ceiling(lg p) steps. In each step i, each process sends to rank+2^i
     and receives from rank-2^i, and exchanges all data blocks
     whose ith bit is 1. */

    /* allocate displacements array for indexed datatype used in
     communication */

    displs = MPL_malloc(comm_size * sizeof(int), MPL_MEM_COLL);
    /* --BEGIN ERROR HANDLING-- */
    if (!displs) {
        mpi_errno =
            MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, __func__,
                                 __LINE__, MPI_ERR_OTHER, "**nomem", 0);

        MPIR_TIMER_END(coll, alltoall, bruck);
        return mpi_errno;
    }
    /* --END ERROR HANDLING-- */

    pof2 = 1;
    while (pof2 < comm_size) {
        dst = (rank + pof2) % comm_size;
        src = (rank - pof2 + comm_size) % comm_size;

        /* Exchange all data blocks whose ith bit is 1 */
        /* Create an indexed datatype for the purpose */

        count = 0;
        for (block = 1; block < comm_size; block++) {
            if (block & pof2) {
                displs[count] = block * recvcount;
                count++;
            }
        }

        mpi_errno = MPIR_Type_create_indexed_block_impl(
            count, recvcount, displs, recvtype, &newtype);
        MPIR_ERR_CHECK(mpi_errno);

        mpi_errno = MPIR_Type_commit_impl(&newtype);
        MPIR_ERR_CHECK(mpi_errno);

        position = 0;
        mpi_errno = MPIR_Typerep_pack(recvbuf, 1, newtype, position, tmp_buf,
                                      pack_size, &position);
        MPIR_ERR_CHECK(mpi_errno);

        MPIR_PVAR_INC(alltoall, bruck, send, position, MPI_PACKED);
        MPIR_PVAR_INC(alltoall, bruck, recv, 1, newtype);
        mpi_errno =
            MPIC_Sendrecv(tmp_buf, position, MPI_PACKED, dst, MPIR_ALLTOALL_TAG,
                          recvbuf, 1, newtype, src, MPIR_ALLTOALL_TAG, comm_ptr,
                          MPI_STATUS_IGNORE, errflag);
        if (mpi_errno) {
            /* for communication errors, just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }

        MPIR_Type_free_impl(&newtype);

        pof2 *= 2;
    }

    MPL_free(displs);
    MPL_free(tmp_buf);

    /* Rotate blocks in recvbuf upwards by (rank + 1) blocks. Need
     * a temporary buffer of the same size as recvbuf. */

    /* get true extent of recvtype */
    MPIR_Type_get_true_extent_impl(recvtype, &recvtype_true_lb,
                                   &recvtype_true_extent);
    recvbuf_extent = recvcount * comm_size *
                     (MPL_MAX(recvtype_true_extent, recvtype_extent));
    tmp_buf = MPL_malloc(recvbuf_extent, MPL_MEM_COLL);
    /* --BEGIN ERROR HANDLING-- */
    if (!tmp_buf) {
        mpi_errno =
            MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, __func__,
                                 __LINE__, MPI_ERR_OTHER, "**nomem", 0);

        MPIR_TIMER_END(coll, alltoall, bruck);
        return mpi_errno;
    }
    /* --END ERROR HANDLING-- */
    /* adjust for potential negative lower bound in datatype */
    tmp_buf = (void *)((char *)tmp_buf - recvtype_true_lb);

    mpi_errno = MPIR_Localcopy(
        (char *)recvbuf + (rank + 1) * recvcount * recvtype_extent,
        (comm_size - rank - 1) * recvcount, recvtype, tmp_buf,
        (comm_size - rank - 1) * recvcount, recvtype);
    MPIR_ERR_CHECK(mpi_errno);
    mpi_errno = MPIR_Localcopy(
        recvbuf, (rank + 1) * recvcount, recvtype,
        (char *)tmp_buf + (comm_size - rank - 1) * recvcount * recvtype_extent,
        (rank + 1) * recvcount, recvtype);
    MPIR_ERR_CHECK(mpi_errno);

    /* Blocks are in the reverse order now (comm_size-1 to 0).
     * Reorder them to (0 to comm_size-1) and store them in recvbuf. */

    for (i = 0; i < comm_size; i++)
        MPIR_Localcopy((char *)tmp_buf + i * recvcount * recvtype_extent,
                       recvcount, recvtype,
                       (char *)recvbuf +
                           (comm_size - i - 1) * recvcount * recvtype_extent,
                       recvcount, recvtype);

    void *tmp = (void *)(tmp_buf + recvtype_true_lb);
    MPL_free(tmp);

fn_exit:
    MPIR_TIMER_END(coll, alltoall, bruck);
    return (mpi_errno);
fn_fail:
    if (newtype != MPI_DATATYPE_NULL) {
        MPIR_Type_free_impl(&newtype);
    }
    goto fn_exit;
}
