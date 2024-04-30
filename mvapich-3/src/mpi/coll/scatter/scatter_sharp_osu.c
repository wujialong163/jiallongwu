#include "scatter_tuning.h"

#if defined(_SHARP_SUPPORT_)
#include "api/sharp_coll.h"
#include "mvp_sharp.h"
#endif

#if defined(_SHARP_SUPPORT_)
int MPIR_Sharp_Scatter_MVP(const void *sendbuf, int sendcount,
                           MPI_Datatype sendtype, void *recvbuf, int recvcount,
                           MPI_Datatype recvtype, int root, MPIR_Comm *comm_ptr,
                           MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, scatter, sharp);
    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_scatter_sharp, 1);
    int mpi_errno = MPI_SUCCESS;
    MPI_Aint type_size = 0;
    /* Get size of data */
    MPIR_Datatype_get_size_macro(sendtype, type_size);
    intptr_t nbytes = (intptr_t)sendcount * (type_size);
    int rank = comm_ptr->rank;
    int size = comm_ptr->local_size;
    void *buffer = NULL;
    int copy_offset;

    if (rank == root) {
        buffer = (void *)sendbuf;
    } else {
        buffer = (void *)comm_ptr->dev.ch.coll_tmp_buf;
    }

    if (nbytes * size > MVP_COLL_TMP_BUF_SIZE) {
        mpi_errno = MPI_ERR_NO_MEM;
        PRINT_DEBUG(DEBUG_Sharp_verbose,
                    "coll_tmp_buf out of mem (%d), "
                    "need %d, continue without SHARP\n",
                    MVP_COLL_TMP_BUF_SIZE, nbytes * size);
        MPIR_ERR_SETANDJUMP2(mpi_errno, MPI_ERR_INTERN, "**sharpcoll",
                             "coll_tmp_buf out of mem (%d), need %d, "
                             "continue without SHARP\n",
                             MVP_COLL_TMP_BUF_SIZE, nbytes * size);
    }

    mpi_errno = MPIR_Bcast_MVP(buffer, sendcount * size, sendtype, root,
                               comm_ptr, errflag);
    if (mpi_errno) {
        MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_INTERN, "**sharpcoll");
    }

    if (rank == root) {
        if (recvbuf != MPI_IN_PLACE) {
            copy_offset = type_size * sendcount * rank;
            mpi_errno = MPIR_Localcopy((char *)buffer + copy_offset, sendcount,
                                       sendtype, recvbuf, recvcount, recvtype);
        }
    } else {
        copy_offset = type_size * recvcount * rank;
        mpi_errno = MPIR_Localcopy((char *)buffer + copy_offset, nbytes,
                                   MPI_BYTE, recvbuf, recvcount, recvtype);
    }

fn_exit:
    MPIR_TIMER_END(coll, scatter, sharp);
    return mpi_errno;

fn_fail:
    goto fn_exit;
}
#endif
