#include "scatter_tuning.h"
#include "bcast_tuning.h"
#if defined(_MCST_SUPPORT_)
#include "ibv_mcast.h"

int MPIR_Scatter_mcst_MVP(const void *sendbuf, int sendcnt,
                          MPI_Datatype sendtype, void *recvbuf, int recvcnt,
                          MPI_Datatype recvtype, int root, MPIR_Comm *comm_ptr,
                          MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, scatter, mcast);
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    int rank = comm_ptr->rank, local_rank;
    int comm_size = comm_ptr->local_size;
    int sendtype_size, recvtype_size;
    MPI_Aint sendtype_extent = 0, recvtype_extent = 0;
    MPI_Aint nbytes, copy_offset;
    int tmp_buf_size = 0, intra_node_root = 0;
    int leader_of_root, sendtype_contig;
    void *mcast_scatter_buf = NULL;
    void *in_buf = NULL;
    int in_count = 0;
    MPI_Datatype in_type;
    MPI_Comm shmem_comm;
    MPIR_Comm *shmem_commptr;
    MPI_Status status;
    MPIR_Datatype *dtp;

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_scatter_mcast, 1);

    MPIR_CHKLMEM_DECL(1);

    if (rank == root) {
        MPIR_Datatype_get_size_macro(sendtype, sendtype_size);
        MPIR_Datatype_get_extent_macro(sendtype, sendtype_extent);
        nbytes = sendcnt * sendtype_size;
        if (HANDLE_GET_KIND(sendtype) == HANDLE_KIND_BUILTIN) {
            sendtype_contig = 1;
        } else {
            MPIR_Datatype_get_ptr(sendtype, dtp);
            sendtype_contig = dtp->is_contig;
        }

        if (sendtype_contig == 0) {
            tmp_buf_size = sendtype_extent * comm_size;
        }
    } else {
        MPIR_Datatype_get_extent_macro(recvtype, recvtype_extent);
        MPIR_Datatype_get_size_macro(recvtype, recvtype_size);
        nbytes = recvcnt * recvtype_size;
        tmp_buf_size = recvcnt * recvtype_extent * comm_size;
    }

    /* Allocate tmp buffer space if required */
    if (tmp_buf_size > 0) {
        MPIR_CHKLMEM_MALLOC(mcast_scatter_buf, void *, tmp_buf_size, mpi_errno,
                            "temporary buffer", MPL_MEM_COLL);
    }

    if ((rank == root && sendtype_contig == 0) || (rank != root)) {
        in_buf = mcast_scatter_buf;
        in_count = nbytes * comm_size;
        in_type = MPI_BYTE;
    } else {
        in_buf = (void *)sendbuf;
        in_count = sendcnt * comm_size;
        in_type = sendtype;
    }

    /* Before we do the 2-level mcast, we need to see if the root
     * is also the node-level leader. If not, we need to transfer the
     * data from the root to its leader */

    shmem_comm = comm_ptr->dev.ch.shmem_comm;
    MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
    local_rank = shmem_commptr->rank;

    leader_of_root = comm_ptr->dev.ch.leader_map[root];
    /* leader_of_root is the global rank of the leader of the root */

    if ((local_rank == 0) && (root != rank) && (leader_of_root == rank)) {
        /* The root of the scatter operation is not the node leader. Recv
         * data from the node leader, as bytes, so that the data is ready
         * for the mcast */
        MPIR_PVAR_INC(scatter, mcast, recv, nbytes * comm_size, MPI_BYTE);
        mpi_errno =
            MPIC_Recv(mcast_scatter_buf, nbytes * comm_size, MPI_BYTE, root,
                      MPIR_SCATTER_TAG, comm_ptr, &status, errflag);
        if (mpi_errno) {
            /* for communication errors, just record the error but continue
             */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
    }

    if (rank == root && local_rank != 0) {
        /* The root of the scatter operation is not the node leader. Send
         * data to the node leader */
        MPIR_PVAR_INC(scatter, mcast, send, sendcnt * comm_size, sendtype);
        mpi_errno =
            MPIC_Send(sendbuf, sendcnt * comm_size, sendtype, leader_of_root,
                      MPIR_SCATTER_TAG, comm_ptr, errflag);
        if (mpi_errno) {
            /* for communication errors, just record the error but continue
             */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
    }

    if (rank == root && local_rank == 0 && sendtype_contig == 0) {
        /* I am the root and the node-leader. My data is non-contig and I need
         * to pack before calling mcst-bcast  */
        mpi_errno =
            MPIR_Localcopy((char *)sendbuf, sendcnt * comm_size, sendtype,
                           mcast_scatter_buf, nbytes * comm_size, MPI_BYTE);
    }

    mpi_errno = MPIR_Mcast_inter_node_MVP(in_buf, in_count, in_type,
                                          leader_of_root, comm_ptr, errflag);

    if (mpi_errno) {
        /* for communication errors, just record the error but continue */
        *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
        MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
    }

    if (comm_ptr->dev.ch.intra_node_done == 0) {
        mpi_errno = MPIR_Shmem_Bcast_MVP(
            in_buf, in_count, in_type, intra_node_root, shmem_commptr, errflag);
        if (mpi_errno) {
            /* for communication errors, just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
    }

    if (rank == root) {
        if (recvbuf != MPI_IN_PLACE) {
            copy_offset = sendtype_extent * sendcnt * rank;
            mpi_errno = MPIR_Localcopy((char *)sendbuf + copy_offset, sendcnt,
                                       sendtype, recvbuf, recvcnt, recvtype);
        }
    } else {
        copy_offset = recvtype_extent * recvcnt * rank;
        mpi_errno =
            MPIR_Localcopy((char *)mcast_scatter_buf + copy_offset, nbytes,
                           MPI_BYTE, recvbuf, recvcnt, recvtype);
    }

    /* --BEGIN ERROR HANDLING-- */
    if (mpi_errno) {
        mpi_errno =
            MPIR_Err_create_code(mpi_errno, MPIR_ERR_RECOVERABLE, __func__,
                                 __LINE__, MPI_ERR_OTHER, "**fail", 0);
        return mpi_errno;
    }

fn_exit:
    MPIR_CHKLMEM_FREEALL();
    MPIR_TIMER_END(coll, scatter, mcast);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
#endif /* #if defined(_MCST_SUPPORT_) */

int MPIR_Scatter_mcst_wrap_MVP(const void *sendbuf, int sendcnt,
                               MPI_Datatype sendtype, void *recvbuf,
                               int recvcnt, MPI_Datatype recvtype, int root,
                               MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    return 0;
}
