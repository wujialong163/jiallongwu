#include "scatter_tuning.h"

int MPIR_Scatter_MVP_Binomial(const void *sendbuf, int sendcnt,
                              MPI_Datatype sendtype, void *recvbuf, int recvcnt,
                              MPI_Datatype recvtype, int root,
                              MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, scatter, binomial);
    MPI_Status status;
    MPI_Aint extent = 0, curr_cnt;
    int rank, comm_size, is_homogeneous, sendtype_size;
    int relative_rank;
    MPI_Aint nbytes, send_subtree_cnt;
    int mask, recvtype_size = 0, src, dst;
#ifdef MPID_HAS_HETERO
    int position;
#endif /* MPID_HAS_HETERO */
    int tmp_buf_size = 0;
    void *tmp_buf = NULL;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_scatter_binomial, 1);

    comm_size = comm_ptr->local_size;
    rank = comm_ptr->rank;

    if (((rank == root) && (sendcnt == 0)) ||
        ((rank != root) && (recvcnt == 0))) {
        MPIR_TIMER_END(coll, scatter, binomial);
        return MPI_SUCCESS;
    }

    is_homogeneous = 1;
#ifdef MPID_HAS_HETERO
    if (comm_ptr->is_hetero) {
        is_homogeneous = 0;
    }
#endif /* MPID_HAS_HETERO */

    /* Use binomial tree algorithm */

    if (rank == root) {
        MPIR_Datatype_get_extent_macro(sendtype, extent);
    }

    relative_rank = (rank >= root) ? rank - root : rank - root + comm_size;

    if (is_homogeneous) {
        /* communicator is homogeneous */
        if (rank == root) {
            /* We separate the two cases (root and non-root) because
               in the event of recvbuf=MPI_IN_PLACE on the root,
               recvcnt and recvtype are not valid */
            MPIR_Datatype_get_size_macro(sendtype, sendtype_size);
            nbytes = sendtype_size * sendcnt;
        } else {
            MPIR_Datatype_get_size_macro(recvtype, recvtype_size);
            nbytes = recvtype_size * recvcnt;
        }

        curr_cnt = 0;

        /* all even nodes other than root need a temporary buffer to
           receive data of max size (nbytes*comm_size)/2 */
        if (relative_rank && !(relative_rank % 2)) {
            tmp_buf_size = (nbytes * comm_size) / 2;
            tmp_buf = MPL_malloc(tmp_buf_size, MPL_MEM_COLL);
            /* --BEGIN ERROR HANDLING-- */
            if (!tmp_buf) {
                mpi_errno = MPIR_Err_create_code(
                    MPI_SUCCESS, MPIR_ERR_RECOVERABLE, __func__, __LINE__,
                    MPI_ERR_OTHER, "**nomem", 0);
                return mpi_errno;
            }
            /* --END ERROR HANDLING-- */
        }

        /* if the root is not rank 0, we reorder the sendbuf in order of
           relative ranks and copy it into a temporary buffer, so that
           all the sends from the root are contiguous and in the right
           order. */
        if (rank == root) {
            if (root != 0) {
                tmp_buf_size = nbytes * comm_size;
                tmp_buf = MPL_malloc(tmp_buf_size, MPL_MEM_COLL);
                /* --BEGIN ERROR HANDLING-- */
                if (!tmp_buf) {
                    mpi_errno = MPIR_Err_create_code(
                        MPI_SUCCESS, MPIR_ERR_RECOVERABLE, __func__, __LINE__,
                        MPI_ERR_OTHER, "**nomem", 0);
                    return mpi_errno;
                }
                /* --END ERROR HANDLING-- */

                if (recvbuf != MPI_IN_PLACE) {
                    mpi_errno = MPIR_Localcopy(
                        ((char *)sendbuf + extent * sendcnt * rank),
                        sendcnt * (comm_size - rank), sendtype, tmp_buf,
                        nbytes * (comm_size - rank), MPI_BYTE);
                } else {
                    mpi_errno = MPIR_Localcopy(
                        ((char *)sendbuf + extent * sendcnt * (rank + 1)),
                        sendcnt * (comm_size - rank - 1), sendtype,
                        (char *)tmp_buf + nbytes,
                        nbytes * (comm_size - rank - 1), MPI_BYTE);
                }
                /* --BEGIN ERROR HANDLING-- */
                if (mpi_errno) {
                    mpi_errno = MPIR_Err_create_code(
                        mpi_errno, MPIR_ERR_RECOVERABLE, __func__, __LINE__,
                        MPI_ERR_OTHER, "**fail", 0);
                    return mpi_errno;
                }
                /* --END ERROR HANDLING-- */

                mpi_errno = MPIR_Localcopy(
                    sendbuf, sendcnt * rank, sendtype,
                    ((char *)tmp_buf + nbytes * (comm_size - rank)),
                    nbytes * rank, MPI_BYTE);
                /* --BEGIN ERROR HANDLING-- */
                if (mpi_errno) {
                    mpi_errno = MPIR_Err_create_code(
                        mpi_errno, MPIR_ERR_RECOVERABLE, __func__, __LINE__,
                        MPI_ERR_OTHER, "**fail", 0);
                    return mpi_errno;
                }
                /* --END ERROR HANDLING-- */

                curr_cnt = nbytes * comm_size;
            } else {
                curr_cnt = sendcnt * comm_size;
            }
        }

        /* root has all the data; others have zero so far */

        mask = 0x1;
        while (mask < comm_size) {
            if (relative_rank & mask) {
                src = rank - mask;
                if (src < 0)
                    src += comm_size;

                /* The leaf nodes receive directly into recvbuf because
                   they don't have to forward data to anyone. Others
                   receive data into a temporary buffer. */
                if (relative_rank % 2) {
                    MPIR_PVAR_INC(scatter, binomial, recv, recvcnt, recvtype);
                    mpi_errno =
                        MPIC_Recv(recvbuf, recvcnt, recvtype, src,
                                  MPIR_SCATTER_TAG, comm_ptr, &status, errflag);
                    if (mpi_errno) {
                        /* for communication errors,
                         * just record the error but continue */
                        *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                        MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                    }
                } else {
                    MPIR_PVAR_INC(scatter, binomial, recv, tmp_buf_size,
                                  MPI_BYTE);
                    mpi_errno =
                        MPIC_Recv(tmp_buf, tmp_buf_size, MPI_BYTE, src,
                                  MPIR_SCATTER_TAG, comm_ptr, &status, errflag);
                    if (mpi_errno) {
                        /* for communication errors,
                         * just record the error but continue */
                        *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                        MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                    }

                    /* the recv size is larger than what may be sent in
                       some cases. query amount of data actually received */
                    MPIR_Get_count_impl(&status, MPI_BYTE, &curr_cnt);
                }
                break;
            }
            mask <<= 1;
        }

        /* This process is responsible for all processes that have bits
           set from the LSB up to (but not including) mask.  Because of
           the "not including", we start by shifting mask back down
           one. */

        mask >>= 1;
        while (mask > 0) {
            if (relative_rank + mask < comm_size) {
                dst = rank + mask;
                if (dst >= comm_size)
                    dst -= comm_size;

                if ((rank == root) && (root == 0)) {
                    send_subtree_cnt = curr_cnt - sendcnt * mask;
                    /* mask is also the size of this process's subtree */
                    MPIR_PVAR_INC(scatter, binomial, send, send_subtree_cnt,
                                  sendtype);
                    mpi_errno =
                        MPIC_Send(((char *)sendbuf + extent * sendcnt * mask),
                                  send_subtree_cnt, sendtype, dst,
                                  MPIR_SCATTER_TAG, comm_ptr, errflag);
                } else {
                    /* non-zero root and others */
                    send_subtree_cnt = curr_cnt - nbytes * mask;
                    /* mask is also the size of this process's subtree */
                    MPIR_PVAR_INC(scatter, binomial, send, send_subtree_cnt,
                                  MPI_BYTE);
                    mpi_errno = MPIC_Send(((char *)tmp_buf + nbytes * mask),
                                          send_subtree_cnt, MPI_BYTE, dst,
                                          MPIR_SCATTER_TAG, comm_ptr, errflag);
                }
                if (mpi_errno) {
                    /* for communication errors,
                     * just record the error but continue */
                    *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                    MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                }

                curr_cnt -= send_subtree_cnt;
            }
            mask >>= 1;
        }

        if ((rank == root) && (root == 0) && (recvbuf != MPI_IN_PLACE)) {
            /* for root=0, put root's data in recvbuf if not MPI_IN_PLACE */
            mpi_errno = MPIR_Localcopy(sendbuf, sendcnt, sendtype, recvbuf,
                                       recvcnt, recvtype);
            /* --BEGIN ERROR HANDLING-- */
            if (mpi_errno) {
                mpi_errno = MPIR_Err_create_code(
                    mpi_errno, MPIR_ERR_RECOVERABLE, __func__, __LINE__,
                    MPI_ERR_OTHER, "**fail", 0);
                return mpi_errno;
            }
            /* --END ERROR HANDLING-- */
        } else if (!(relative_rank % 2) && (recvbuf != MPI_IN_PLACE)) {
            /* for non-zero root and non-leaf nodes, copy from tmp_buf
               into recvbuf */
            mpi_errno = MPIR_Localcopy(tmp_buf, nbytes, MPI_BYTE, recvbuf,
                                       recvcnt, recvtype);
            /* --BEGIN ERROR HANDLING-- */
            if (mpi_errno) {
                mpi_errno = MPIR_Err_create_code(
                    mpi_errno, MPIR_ERR_RECOVERABLE, __func__, __LINE__,
                    MPI_ERR_OTHER, "**fail", 0);
                return mpi_errno;
            }
            /* --END ERROR HANDLING-- */
        }

        if (tmp_buf != NULL)
            MPL_free(tmp_buf);
    }
#ifdef MPID_HAS_HETERO
    else { /* communicator is heterogeneous */
        if (rank == root) {
            MPIR_Pack_size_impl(sendcnt * comm_size, sendtype, &tmp_buf_size);
            tmp_buf = MPL_malloc(tmp_buf_size, MPL_MEM_COLL);
            /* --BEGIN ERROR HANDLING-- */
            if (!tmp_buf) {
                mpi_errno = MPIR_Err_create_code(
                    MPI_SUCCESS, MPIR_ERR_RECOVERABLE, __func__, __LINE__,
                    MPI_ERR_OTHER, "**nomem", 0);
                return mpi_errno;
            }
            /* --END ERROR HANDLING-- */

            /* calculate the value of nbytes, the number of bytes in packed
               representation that each process receives. We can't
               accurately calculate that from tmp_buf_size because
               MPI_Pack_size returns an upper bound on the amount of memory
               required. (For example, for a single integer, MPICH-1 returns
               pack_size=12.) Therefore, we actually pack some data into
               tmp_buf and see by how much 'position' is incremented. */

            position = 0;
            MPIR_Typerep_pack(sendbuf, 1, sendtype, position, tmp_buf,
                              tmp_buf_size, &position);
            nbytes = position * sendcnt;

            curr_cnt = nbytes * comm_size;

            if (root == 0) {
                if (recvbuf != MPI_IN_PLACE) {
                    position = 0;
                    MPIR_Typerep_pack(sendbuf, sendcnt * comm_size, sendtype,
                                      position, tmp_buf, tmp_buf_size,
                                      &position);
                } else {
                    position = nbytes;
                    MPIR_Typerep_pack(((char *)sendbuf + extent * sendcnt),
                                      sendcnt * (comm_size - 1), sendtype,
                                      position, tmp_buf, tmp_buf_size,
                                      &position);
                }
            } else {
                if (recvbuf != MPI_IN_PLACE) {
                    position = 0;
                    MPIR_Typerep_pack(
                        ((char *)sendbuf + extent * sendcnt * rank),
                        sendcnt * (comm_size - rank), sendtype, position,
                        tmp_buf, tmp_buf_size, &position);
                } else {
                    position = nbytes;
                    MPIR_Typerep_pack(
                        ((char *)sendbuf + extent * sendcnt * (rank + 1)),
                        sendcnt * (comm_size - rank - 1), sendtype, position,
                        tmp_buf, tmp_buf_size, &position);
                }
                MPIR_Typerep_pack(sendbuf, sendcnt * rank, sendtype, positioin,
                                  tmp_buf, tmp_buf_size, &position);
            }
        } else {
            MPIR_Typerep_pack_size(recvcnt * (comm_size / 2), recvtype,
                                   &tmp_buf_size);
            tmp_buf = MPL_malloc(tmp_buf_size, MPL_MEM_COLL);
            /* --BEGIN ERROR HANDLING-- */
            if (!tmp_buf) {
                mpi_errno = MPIR_Err_create_code(
                    MPI_SUCCESS, MPIR_ERR_RECOVERABLE, __func__, __LINE__,
                    MPI_ERR_OTHER, "**nomem", 0);
                return mpi_errno;
            }
            /* --END ERROR HANDLING-- */

            /* calculate nbytes */
            position = 0;
            MPIR_Typerep_pack(recvbuf, 1, recvtype, position, tmp_buf,
                              tmp_buf_size, &position);
            nbytes = position * recvcnt;

            curr_cnt = 0;
        }

        mask = 0x1;
        while (mask < comm_size) {
            if (relative_rank & mask) {
                src = rank - mask;
                if (src < 0)
                    src += comm_size;

                MPIR_PVAR_INC(scatter, binomial, recv, tmp_buf_size, MPI_BYTE);
                mpi_errno =
                    MPIC_Recv(tmp_buf, tmp_buf_size, MPI_BYTE, src,
                              MPIR_SCATTER_TAG, comm_ptr, &status, errflag);
                if (mpi_errno) {
                    /* for communication errors,
                     * just record the error but continue */
                    *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                    MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                }
                /* the recv size is larger than what may be sent in
                   some cases. query amount of data actually received */
                MPIR_Get_count_impl(&status, MPI_BYTE, &curr_cnt);
                break;
            }
            mask <<= 1;
        }

        /* This process is responsible for all processes that have bits
           set from the LSB up to (but not including) mask.  Because of
           the "not including", we start by shifting mask back down
           one. */

        mask >>= 1;
        while (mask > 0) {
            if (relative_rank + mask < comm_size) {
                dst = rank + mask;
                if (dst >= comm_size)
                    dst -= comm_size;

                send_subtree_cnt = curr_cnt - nbytes * mask;
                /* mask is also the size of this process's subtree */
                MPIR_PVAR_INC(scatter, binomial, send, send_subtree_cnt,
                              MPI_BYTE);
                mpi_errno = MPIC_Send(((char *)tmp_buf + nbytes * mask),
                                      send_subtree_cnt, MPI_BYTE, dst,
                                      MPIR_SCATTER_TAG, comm_ptr, errflag);
                if (mpi_errno) {
                    /* for communication errors,
                     * just record the error but continue */
                    *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                    MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                }

                curr_cnt -= send_subtree_cnt;
            }
            mask >>= 1;
        }

        /* copy local data into recvbuf */
        position = 0;
        if (recvbuf != MPI_IN_PLACE)
            MPIR_Typerep_unpack(tmp_buf, tmp_buf_size, &position, recvbuf,
                                recvcnt, recvtype, position, &position);
        MPL_free(tmp_buf);
    }
#endif /* MPID_HAS_HETERO */

    MPIR_TIMER_END(coll, scatter, binomial);
    return (mpi_errno);
}
