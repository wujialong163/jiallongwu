#include "allgather_tuning.h"

extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allgather_rd;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_rd;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_rd_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_rd_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_rd_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_rd_count_recv;

int MPIR_Allgather_RD_MVP(const void *sendbuf, int sendcount,
                          MPI_Datatype sendtype, void *recvbuf, int recvcount,
                          MPI_Datatype recvtype, MPIR_Comm *comm_ptr,
                          MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, allgather, rd);
    int comm_size, rank;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    MPI_Aint recvtype_extent, last_recv_cnt = 0;
    int j, i;
    int curr_cnt, dst;
    MPI_Status status;
    int mask, dst_tree_root, my_tree_root, is_homogeneous, nprocs_completed, k,
        tmp_mask, tree_root;
    MPI_Aint send_offset, recv_offset, offset;
#ifdef MPID_HAS_HETERO
    MPI_Aint position, tmp_buf_size, nbytes;
#endif

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_allgather_rd, 1);

    comm_size = comm_ptr->local_size;
    rank = comm_ptr->rank;

    MPIR_Datatype_get_extent_macro(recvtype, recvtype_extent);

    is_homogeneous = 1;
#ifdef MPID_HAS_HETERO
    if (comm_ptr->is_hetero) {
        is_homogeneous = 0;
    }
#endif

    if (is_homogeneous) {
        /* homogeneous. no need to pack into tmp_buf on each node. copy
         * local data into recvbuf */
        if (sendbuf != MPI_IN_PLACE) {
            mpi_errno = MPIR_Localcopy(
                sendbuf, sendcount, sendtype,
                ((char *)recvbuf + rank * recvcount * recvtype_extent),
                recvcount, recvtype);
            MPIR_ERR_CHECK(mpi_errno);
        }

        curr_cnt = recvcount;

        mask = 0x1;
        i = 0;
        while (mask < comm_size) {
            dst = rank ^ mask;

            /* find offset into send and recv buffers. zero out
             * the least significant "i" bits of rank and dst to
             * find root of src and dst subtrees. Use ranks of
             * roots as index to send from and recv into buffer */

            dst_tree_root = dst >> i;
            dst_tree_root <<= i;

            my_tree_root = rank >> i;
            my_tree_root <<= i;

            send_offset = my_tree_root * recvcount * recvtype_extent;
            recv_offset = dst_tree_root * recvcount * recvtype_extent;

            if (dst < comm_size) {
                MPIR_PVAR_INC(allgather, rd, send, curr_cnt, recvtype);
                MPIR_PVAR_INC(allgather, rd, recv,
                              (comm_size - dst_tree_root) * recvcount,
                              recvtype);
                mpi_errno = MPIC_Sendrecv(
                    ((char *)recvbuf + send_offset), curr_cnt, recvtype, dst,
                    MPIR_ALLGATHER_TAG, ((char *)recvbuf + recv_offset),
                    (comm_size - dst_tree_root) * recvcount, recvtype, dst,
                    MPIR_ALLGATHER_TAG, comm_ptr, &status, errflag);
                if (mpi_errno) {
                    /* for communication errors, just record the error but
                       continue */
                    *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                    MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                    last_recv_cnt = 0;
                }

                MPIR_Get_count_impl(&status, recvtype, &last_recv_cnt);
                curr_cnt += last_recv_cnt;
            }

            /* if some processes in this process's subtree in this step
             * did not have any destination process to communicate with
             * because of non-power-of-two, we need to send them the
             * data that they would normally have received from those
             * processes. That is, the haves in this subtree must send to
             * the havenots. We use a logarithmic recursive-halfing algorithm
             * for this. */

            /* This part of the code will not currently be
             * executed because we are not using recursive
             * doubling for non power of two. Mark it as experimental
             * so that it doesn't show up as red in the coverage
             * tests. */

            /* --BEGIN EXPERIMENTAL-- */
            if (dst_tree_root + mask > comm_size) {
                nprocs_completed = comm_size - my_tree_root - mask;
                /* nprocs_completed is the number of processes in this
                 * subtree that have all the data. Send data to others
                 * in a tree fashion. First find root of current tree
                 * that is being divided into two. k is the number of
                 * least-significant bits in this process's rank that
                 * must be zeroed out to find the rank of the root */
                j = mask;
                k = 0;
                while (j) {
                    j >>= 1;
                    k++;
                }
                k--;

                offset = recvcount * (my_tree_root + mask) * recvtype_extent;
                tmp_mask = mask >> 1;

                while (tmp_mask) {
                    dst = rank ^ tmp_mask;

                    tree_root = rank >> k;
                    tree_root <<= k;

                    /* send only if this proc has data and destination
                     * doesn't have data. at any step, multiple processes
                     * can send if they have the data */
                    if ((dst > rank) && (rank < tree_root + nprocs_completed) &&
                        (dst >= tree_root + nprocs_completed)) {
                        MPIR_PVAR_INC(allgather, rd, send, last_recv_cnt,
                                      recvtype);
                        mpi_errno = MPIC_Send(
                            ((char *)recvbuf + offset), last_recv_cnt, recvtype,
                            dst, MPIR_ALLGATHER_TAG, comm_ptr, errflag);
                        /* last_recv_cnt was set in the previous
                         * receive. that's the amount of data to be
                         * sent now. */
                        if (mpi_errno) {
                            /* for communication errors, just record the error
                               but continue */
                            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                        }
                    }
                    /* recv only if this proc. doesn't have data and sender
                     * has data */
                    else if ((dst < rank) &&
                             (dst < tree_root + nprocs_completed) &&
                             (rank >= tree_root + nprocs_completed)) {
                        MPIR_PVAR_INC(allgather, rd, recv,
                                      (comm_size - (my_tree_root + mask)) *
                                          recvcount,
                                      recvtype);
                        mpi_errno = MPIC_Recv(
                            ((char *)recvbuf + offset),
                            (comm_size - (my_tree_root + mask)) * recvcount,
                            recvtype, dst, MPIR_ALLGATHER_TAG, comm_ptr,
                            &status, errflag);
                        /* nprocs_completed is also equal to the
                         * no. of processes whose data we don't have */
                        if (mpi_errno) {
                            /* for communication errors, just record the error
                               but continue */
                            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                            last_recv_cnt = 0;
                        }
                        MPIR_Get_count_impl(&status, recvtype, &last_recv_cnt);
                        curr_cnt += last_recv_cnt;
                    }
                    tmp_mask >>= 1;
                    k--;
                }
            }
            /* --END EXPERIMENTAL-- */

            mask <<= 1;
            i++;
        }
    }
#ifdef MPID_HAS_HETERO
    else {
        /* heterogeneous. need to use temp. buffer. */

        MPIR_Pack_size_impl(recvcount * comm_size, recvtype, &tmp_buf_size);

        tmp_buf = MPL_malloc(tmp_buf_size, MPL_MEM_COLL);
        /* --BEGIN ERROR HANDLING-- */
        if (!tmp_buf) {
            mpi_errno = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE,
                                             __func__, __LINE__, MPI_ERR_OTHER,
                                             "**nomem", 0);
            return mpi_errno;
        }
        /* --END ERROR HANDLING-- */

        /* calculate the value of nbytes, the number of bytes in packed
         * representation that each process contributes. We can't simply divide
         * tmp_buf_size by comm_size because tmp_buf_size is an upper
         * bound on the amount of memory required. (For example, for
         * a single integer, MPICH-1 returns pack_size=12.) Therefore, we
         * actually pack some data into tmp_buf and see by how much
         * 'position' is incremented. */

        position = 0;
        MPIR_Typerep_pack(recvbuf, 1, recvtype, position, tmp_buf, tmp_buf_size,
                          &position);
        nbytes = position * recvcount;

        /* pack local data into right location in tmp_buf */
        position = rank * nbytes;
        if (sendbuf != MPI_IN_PLACE) {
            MPIR_Typerep_pack(sendbuf, sendcount, sendtype, position, tmp_buf,
                              tmp_buf_size, &position);
        } else {
            /* if in_place specified, local data is found in recvbuf */
            MPIR_Typerep_pack(((char *)recvbuf + recvtype_extent * rank),
                              recvcount, recvtype, position, tmp_buf,
                              tmp_buf_size, &position);
        }

        curr_cnt = nbytes;

        mask = 0x1;
        i = 0;
        while (mask < comm_size) {
            dst = rank ^ mask;

            /* find offset into send and recv buffers. zero out
             * the least significant "i" bits of rank and dst to
             * find root of src and dst subtrees. Use ranks of
             * roots as index to send from and recv into buffer. */

            dst_tree_root = dst >> i;
            dst_tree_root <<= i;

            my_tree_root = rank >> i;
            my_tree_root <<= i;

            send_offset = my_tree_root * nbytes;
            recv_offset = dst_tree_root * nbytes;

            if (dst < comm_size) {
                MPIR_PVAR_INC(allgather, rd, send, curr_cnt, MPI_BYTE);
                MPIR_PVAR_INC(allgather, rd, recv, (tmp_buf_size - recv_offset),
                              MPI_BYTE);
                mpi_errno = MPIC_Sendrecv(
                    ((char *)tmp_buf + send_offset), curr_cnt, MPI_BYTE, dst,
                    MPIR_ALLGATHER_TAG, ((char *)tmp_buf + recv_offset),
                    tmp_buf_size - recv_offset, MPI_BYTE, dst,
                    MPIR_ALLGATHER_TAG, comm_ptr, &status, errflag);
                if (mpi_errno) {
                    /* for communication errors, just record the error but
                       continue */
                    *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                    MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                    last_recv_cnt = 0;
                }

                MPIR_Get_count_impl(&status, MPI_BYTE, &last_recv_cnt);
                curr_cnt += last_recv_cnt;
            }

            /* if some processes in this process's subtree in this step
             * did not have any destination process to communicate with
             * because of non-power-of-two, we need to send them the
             * data that they would normally have received from those
             * processes. That is, the haves in this subtree must send to
             * the havenots. We use a logarithmic recursive-halfing
             * algorithm for this. */

            if (dst_tree_root + mask > comm_size) {
                nprocs_completed = comm_size - my_tree_root - mask;
                /* nprocs_completed is the number of processes in this
                 * subtree that have all the data. Send data to others
                 * in a tree fashion. First find root of current tree
                 * that is being divided into two. k is the number of
                 * least-significant bits in this process's rank that
                 * must be zeroed out to find the rank of the root */
                j = mask;
                k = 0;
                while (j) {
                    j >>= 1;
                    k++;
                }
                k--;

                offset = nbytes * (my_tree_root + mask);
                tmp_mask = mask >> 1;

                while (tmp_mask) {
                    dst = rank ^ tmp_mask;
                    tree_root = rank >> k;
                    tree_root <<= k;

                    /* send only if this proc has data and destination
                     * doesn't have data. at any step, multiple processes
                     * can send if they have the data */
                    if ((dst > rank) && (rank < tree_root + nprocs_completed) &&
                        (dst >= tree_root + nprocs_completed)) {
                        MPIR_PVAR_INC(allgather, rd, send, last_recv_cnt,
                                      MPI_BYTE);
                        mpi_errno = MPIC_Send(
                            ((char *)tmp_buf + offset), last_recv_cnt, MPI_BYTE,
                            dst, MPIR_ALLGATHER_TAG, comm_ptr, errflag);
                        /* last_recv_cnt was set in the previous
                         * receive. that's the amount of data to be
                         * sent now. */
                        if (mpi_errno) {
                            /* for communication errors, just record the error
                               but continue */
                            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                        }
                    }
                    /* recv only if this proc. doesn't have data and sender
                     * has data */
                    else if ((dst < rank) &&
                             (dst < tree_root + nprocs_completed) &&
                             (rank >= tree_root + nprocs_completed)) {
                        MPIR_PVAR_INC(allgather, rd, recv,
                                      (tmp_buf_size - offset), MPI_BYTE);
                        mpi_errno = MPIC_Recv(((char *)tmp_buf + offset),
                                              tmp_buf_size - offset, MPI_BYTE,
                                              dst, MPIR_ALLGATHER_TAG, comm_ptr,
                                              &status, errflag);
                        /* nprocs_completed is also equal to the
                         * no. of processes whose data we don't have */
                        if (mpi_errno) {
                            /* for communication errors, just record the error
                               but continue */
                            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                            last_recv_cnt = 0;
                        }
                        MPIR_Get_count_impl(&status, MPI_BYTE, &last_recv_cnt);
                        curr_cnt += last_recv_cnt;
                    }
                    tmp_mask >>= 1;
                    k--;
                }
            }
            mask <<= 1;
            i++;
        }

        position = 0;
        MPIR_Typerep_unpack(tmp_buf, tmp_buf_size, recvbuf,
                            recvcount * comm_size, recvtype, position,
                            &position);

        MPL_free(tmp_buf);
    }
#endif /* MPID_HAS_HETERO */

fn_fail:
    MPIR_TIMER_END(coll, allgather, rd);
    return (mpi_errno);
}
