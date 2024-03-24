#include "gather_tuning.h"

#if defined(_SMP_LIMIC_)
int num_scheme;
extern int limic_fd;
extern int MPIR_Limic_Gather_OSU(void *sendbuf, int sendbytes, void *recvbuf,
                                 int recvbytes, MPIR_Comm *shmem_comm_ptr);
#endif /*#if defined(_SMP_LIMIC_)*/

/* sendbuf           - (in) sender's buffer
 * sendcnt           - (in) sender's element count
 * sendtype          - (in) sender's data type
 * recvbuf           - (in) receiver's buffer
 * recvcnt           - (in) receiver's element count
 * recvtype          - (in) receiver's data type
 * root              - (in)root for the gather operation
 * rank              - (in) global rank(rank in the global comm)
 * tmp_buf           - (out/in) tmp_buf into which intra node
 *                     data is gathered
 * is_data_avail     - (in) based on this, tmp_buf acts
 *                     as in/out parameter.
 *                     1 - tmp_buf acts as in parameter
 *                     0 - tmp_buf acts as out parameter
 * comm_ptr          - (in) pointer to the communicator
 *                     (shmem_comm or intra_sock_comm or
 *                     inter-sock_leader_comm)
 * intra_node_fn_ptr - (in) Function ptr to choose the
 *                      intra node gather function
 * errflag           - (out) to record errors
 */
int MPIR_pt_pt_intra_gather(const void *sendbuf, int sendcnt,
                            MPI_Datatype sendtype, void *recvbuf, int recvcnt,
                            MPI_Datatype recvtype, int root, int rank,
                            void *tmp_buf, int nbytes, int is_data_avail,
                            MPIR_Comm *comm_ptr,
                            MVP_Gather_fn_t intra_node_fn_ptr,
                            MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, gather, pt2pt);
    int mpi_errno = MPI_SUCCESS;
    MPI_Aint recvtype_extent = 0; /* Datatype extent */
    MPI_Aint true_lb, sendtype_true_extent, recvtype_true_extent;

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_gather_pt2pt, 1);
    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_num_shmem_coll_calls, 1);

    if (sendtype != MPI_DATATYPE_NULL) {
        MPIR_Type_get_true_extent_impl(sendtype, &true_lb,
                                       &sendtype_true_extent);
    }
    if (recvtype != MPI_DATATYPE_NULL) {
        MPIR_Datatype_get_extent_macro(recvtype, recvtype_extent);
        MPIR_Type_get_true_extent_impl(recvtype, &true_lb,
                                       &recvtype_true_extent);
    }

    /* Special case, when tmp_buf itself has data */
    if (rank == root && sendbuf == MPI_IN_PLACE && is_data_avail) {
        mpi_errno = intra_node_fn_ptr(MPI_IN_PLACE, sendcnt, sendtype, tmp_buf,
                                      nbytes, MPI_BYTE, 0, comm_ptr, errflag);

    } else if (rank == root && sendbuf == MPI_IN_PLACE) {
        mpi_errno = intra_node_fn_ptr(
            recvbuf + rank * recvcnt * recvtype_extent, recvcnt, recvtype,
            tmp_buf, nbytes, MPI_BYTE, 0, comm_ptr, errflag);
    } else {
        mpi_errno = intra_node_fn_ptr(sendbuf, sendcnt, sendtype, tmp_buf,
                                      nbytes, MPI_BYTE, 0, comm_ptr, errflag);
    }

    MPIR_TIMER_END(coll, gather, pt2pt);
    return mpi_errno;
}

#if defined(_SMP_LIMIC_)
static int MPIR_Limic_Gather_Scheme_PT_PT(const void *sendbuf, int sendcnt,
                                          MPI_Datatype sendtype, void *recvbuf,
                                          int recvcnt, MPI_Datatype recvtype,
                                          int root, MPIR_Comm *comm_ptr,
                                          MVP_Gather_fn_t intra_node_fn_ptr,
                                          MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, gather, limic_scheme_pt_pt);
    void *intra_tmp_buf = NULL;
    int rank;
    int local_size;
    int mpi_errno = MPI_SUCCESS;
    MPI_Aint recvtype_size = 0, sendtype_size = 0, nbytes = 0;
    int sendtype_iscontig;
    int intra_sock_rank = 0, intra_sock_comm_size = 0;
    int intra_node_leader_rank = 0, intra_node_leader_comm_size = 0;
    MPI_Aint sendtype_extent = 0, recvtype_extent = 0; /* Datatype extent */
    MPI_Aint true_lb, sendtype_true_extent, recvtype_true_extent;
    MPI_Comm shmem_comm;
    MPIR_Comm *shmem_commptr;
    MPIR_Comm *intra_sock_commptr = NULL, *intra_node_leader_commptr = NULL;

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_gather_limic_scheme_pt_pt, 1);
    rank = comm_ptr->rank;

    if (((rank == root) && (recvcnt == 0)) ||
        ((rank != root) && (sendcnt == 0))) {
        MPIR_TIMER_END(coll, gather, limic_scheme_pt_pt);
        return MPI_SUCCESS;
    }

    if (sendtype != MPI_DATATYPE_NULL) {
        MPIR_Datatype_iscontig(sendtype, &sendtype_iscontig);
        MPIR_Datatype_get_extent_macro(sendtype, sendtype_extent);
        MPIR_Datatype_get_size_macro(sendtype, sendtype_size);
        MPIR_Type_get_true_extent_impl(sendtype, &true_lb,
                                       &sendtype_true_extent);
    }
    if (recvtype != MPI_DATATYPE_NULL) {
        MPIR_Datatype_get_extent_macro(recvtype, recvtype_extent);
        MPIR_Datatype_get_size_macro(recvtype, recvtype_size);
        MPIR_Type_get_true_extent_impl(recvtype, &true_lb,
                                       &recvtype_true_extent);
    }

    /* extract the rank,size information for the intra-node
     * communicator */
    shmem_comm = comm_ptr->dev.ch.shmem_comm;
    MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
    local_size = shmem_commptr->local_size;

    if (rank == root) {
        nbytes = recvcnt * recvtype_size;

    } else {
        nbytes = sendcnt * sendtype_size;
    }

    if (shmem_commptr->dev.ch.use_intra_sock_comm == 1) {
        MPIR_Comm_get_ptr(shmem_commptr->dev.ch.intra_sock_comm,
                          intra_sock_commptr);
        MPIR_Comm_get_ptr(shmem_commptr->dev.ch.intra_sock_leader_comm,
                          intra_node_leader_commptr);

        intra_sock_rank = intra_sock_commptr->rank;
        intra_sock_comm_size = intra_sock_commptr->local_size;
        if (intra_sock_rank == 0) {
            intra_node_leader_rank = intra_node_leader_commptr->rank;
            intra_node_leader_comm_size = intra_node_leader_commptr->local_size;
        }
    }
    if (intra_sock_rank == 0) {
        if (intra_node_leader_rank == 0) {
            /* Node leaders, allocate large buffers which is used to gather
             * data for the entire node. The same buffer is used for inter-node
             * gather as well. This saves us a memcpy operation*/
            if (rank == root) {
                intra_tmp_buf = MPL_malloc(
                    recvcnt * MPL_MAX(recvtype_extent, recvtype_true_extent) *
                        local_size,
                    MPL_MEM_COLL);
            } else {
                intra_tmp_buf = MPL_malloc(
                    sendcnt * MPL_MAX(sendtype_extent, sendtype_true_extent) *
                        local_size,
                    MPL_MEM_COLL);
            }
        } else {
            /* Socket leader, allocate tmp_buffer */
            if (rank == root) {
                intra_tmp_buf = MPL_malloc(
                    recvcnt * MPL_MAX(recvtype_extent, recvtype_true_extent) *
                        intra_sock_comm_size,
                    MPL_MEM_COLL);
            } else {
                intra_tmp_buf = MPL_malloc(
                    sendcnt * MPL_MAX(sendtype_extent, sendtype_true_extent) *
                        intra_sock_comm_size,
                    MPL_MEM_COLL);
            }
        }
        if (intra_tmp_buf == NULL) {
            mpi_errno = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE,
                                             __func__, __LINE__, MPI_ERR_OTHER,
                                             "**nomem", 0);

            MPIR_TIMER_END(coll, gather, limic_scheme_pt_pt);
            return mpi_errno;
        }
    }

    /*Intra socket gather*/
    /*We are gathering the data into intra_tmp_buf and the output
     * will be of MPI_BYTE datatype. Since the tmp_buf has no
     * local data, we pass is_data_avail = TEMP_BUF_HAS_NO_DATA*/
    mpi_errno = MPIR_pt_pt_intra_gather(
        sendbuf, sendcnt, sendtype, recvbuf, recvcnt, recvtype, root, rank,
        intra_tmp_buf, nbytes, TEMP_BUF_HAS_NO_DATA, intra_sock_commptr,
        intra_node_fn_ptr, errflag);
    MPIR_ERR_CHECK(mpi_errno);

    /*Inter socket gather*/
    if (intra_sock_rank == 0) {
        /*When data in each socket is different*/
        if (shmem_commptr->dev.ch.is_socket_uniform != 1) {
            int *displs = NULL;
            int *recvcnts = NULL;
            int *socket_sizes;
            int i = 0;
            socket_sizes = shmem_commptr->dev.ch.socket_size;

            if (intra_node_leader_rank == 0) {
                tmp_buf = intra_tmp_buf;

                displs = MPL_malloc(sizeof(int) * intra_node_leader_comm_size,
                                    MPL_MEM_COLL);
                recvcnts = MPL_malloc(sizeof(int) * intra_node_leader_comm_size,
                                      MPL_MEM_COLL);
                if (!displs || !recvcnts) {
                    mpi_errno = MPIR_Err_create_code(
                        MPI_SUCCESS, MPIR_ERR_RECOVERABLE, __func__, __LINE__,
                        MPI_ERR_OTHER, "**nomem", 0);

                    MPIR_TIMER_END(coll, gather, limic_scheme_pt_pt);
                    return mpi_errno;
                }

                recvcnts[0] = socket_sizes[0] * nbytes;
                displs[0] = 0;

                for (i = 1; i < intra_node_leader_comm_size; i++) {
                    displs[i] = displs[i - 1] + socket_sizes[i - 1] * nbytes;
                    recvcnts[i] = socket_sizes[i] * nbytes;
                }

                mpi_errno =
                    MPIR_Gatherv(MPI_IN_PLACE, intra_sock_comm_size * nbytes,
                                 MPI_BYTE, tmp_buf, recvcnts, displs, MPI_BYTE,
                                 0, intra_node_leader_commptr, errflag);

                /*Free the displacement and recvcnts buffer*/
                MPL_free(displs);
                MPL_free(recvcnts);
            } else {
                mpi_errno =
                    MPIR_Gatherv(intra_tmp_buf, intra_sock_comm_size * nbytes,
                                 MPI_BYTE, tmp_buf, recvcnts, displs, MPI_BYTE,
                                 0, intra_node_leader_commptr, errflag);
            }

        } else {
            if (intra_node_leader_rank == 0) {
                tmp_buf = intra_tmp_buf;

                /*We have now completed the intra_sock gather and all the
                 * socket level leaders have data in their tmp_buf. So we
                 * set sendbuf = MPI_IN_PLACE and also explicitly set the
                 * is_data_avail= TEMP_BUF_HAS_DATA*/
                mpi_errno = MPIR_pt_pt_intra_gather(
                    MPI_IN_PLACE, (nbytes * intra_sock_comm_size), MPI_BYTE,
                    recvbuf, recvcnt, recvtype, root, rank, tmp_buf,
                    (nbytes * intra_sock_comm_size), TEMP_BUF_HAS_DATA,
                    intra_node_leader_commptr, intra_node_fn_ptr, errflag);
            } else {
                /*After the intra_sock gather, all the node level leaders
                 * have the data in intra_tmp_buf(sendbuf)
                 * and this is gathered into tmp_buf.
                 * Since the tmp_buf(in non-root processes) does not have
                 * the data in tmp_buf is_data_avail = TEMP_BUF_HAS_NO_DATA*/
                mpi_errno = MPIR_pt_pt_intra_gather(
                    intra_tmp_buf, (nbytes * intra_sock_comm_size), MPI_BYTE,
                    recvbuf, recvcnt, recvtype, root, rank, tmp_buf,
                    (nbytes * intra_sock_comm_size), TEMP_BUF_HAS_NO_DATA,
                    intra_node_leader_commptr, intra_node_fn_ptr, errflag);
            }
        }

        MPIR_ERR_CHECK(mpi_errno);
    }
fn_fail:
    /*Free the intra socket leader buffers*/
    if (intra_sock_rank == 0) {
        if ((intra_node_leader_rank != 0) && (intra_tmp_buf != NULL)) {
            MPL_free(intra_tmp_buf);
        }
    }

    MPIR_TIMER_END(coll, gather, limic_scheme_pt_pt);
    return (mpi_errno);
}

static int MPIR_Limic_Gather_Scheme_PT_LINEAR(
    const void *sendbuf, int sendcnt, MPI_Datatype sendtype, void *recvbuf,
    int recvcnt, MPI_Datatype recvtype, int root, MPIR_Comm *comm_ptr,
    MVP_Gather_fn_t intra_node_fn_ptr, MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, gather, limic_scheme_pt_linear);
    void *intra_tmp_buf = NULL;
    void *local_sendbuf = NULL;
    int rank;
    int local_rank, local_size;
    int mpi_errno = MPI_SUCCESS;
    MPI_Aint recvtype_size = 0, nbytes = 0;
    int sendtype_iscontig;
    int intra_sock_rank = 0, intra_sock_comm_size = 0;
    int intra_node_leader_rank = 0, intra_node_leader_comm_size = 0;
    MPI_Aint send_nbytes = 0;
    MPI_Aint recvtype_extent = 0; /* Datatype extent */
    MPI_Aint true_lb, sendtype_true_extent, recvtype_true_extent;
    MPI_Comm shmem_comm;
    MPIR_Comm *shmem_commptr;
    MPIR_Comm *intra_sock_commptr = NULL, *intra_node_leader_commptr = NULL;
    MPI_Aint position = 0;
    MPI_Aint sendtype_size = 0;
    MPIR_CHKLMEM_DECL(1);

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_gather_limic_scheme_pt_linear, 1);
    rank = comm_ptr->rank;

    if (((rank == root) && (recvcnt == 0)) ||
        ((rank != root) && (sendcnt == 0))) {
        MPIR_TIMER_END(coll, gather, limic_scheme_pt_linear);
        return MPI_SUCCESS;
    }

    if (sendtype != MPI_DATATYPE_NULL) {
        MPIR_Datatype_iscontig(sendtype, &sendtype_iscontig);
        MPIR_Datatype_get_size_macro(sendtype, sendtype_size);
        MPIR_Type_get_true_extent_impl(sendtype, &true_lb,
                                       &sendtype_true_extent);
    }
    if (recvtype != MPI_DATATYPE_NULL) {
        MPIR_Datatype_get_extent_macro(recvtype, recvtype_extent);
        MPIR_Datatype_get_size_macro(recvtype, recvtype_size);
        MPIR_Type_get_true_extent_impl(recvtype, &true_lb,
                                       &recvtype_true_extent);
    }

    /* extract the rank,size information for the intra-node
     * communicator */
    shmem_comm = comm_ptr->dev.ch.shmem_comm;
    MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
    local_rank = shmem_commptr->rank;
    local_size = shmem_commptr->local_size;

    if (rank == root) {
        nbytes = recvcnt * recvtype_size;

    } else {
        nbytes = sendcnt * sendtype_size;
    }

    if (shmem_commptr->dev.ch.use_intra_sock_comm == 1) {
        MPIR_Comm_get_ptr(shmem_commptr->dev.ch.intra_sock_comm,
                          intra_sock_commptr);
        MPIR_Comm_get_ptr(shmem_commptr->dev.ch.intra_sock_leader_comm,
                          intra_node_leader_commptr);

        intra_sock_rank = intra_sock_commptr->rank;
        intra_sock_comm_size = intra_sock_commptr->local_size;
        if (intra_sock_rank == 0) {
            intra_node_leader_rank = intra_node_leader_commptr->rank;
            intra_node_leader_comm_size = intra_node_leader_commptr->local_size;
        }
    }
    /*Pack data for non-contiguous buffer*/
    if ((!sendtype_iscontig) && (sendbuf != MPI_IN_PLACE)) {
        MPIR_Pack_size_impl(1, sendtype, &sendtype_size);
        send_nbytes = sendcnt * sendtype_size;
        MPIR_CHKLMEM_MALLOC(local_sendbuf, void *, send_nbytes, mpi_errno,
                            "local_sendbuf", MPL_MEM_COLL);
        MPIR_Typerep_pack(sendbuf, sendcnt, sendtype, position, local_sendbuf,
                          send_nbytes, &position);
    } else {
        local_sendbuf = (void *)sendbuf;
        send_nbytes = nbytes;
    }

    if (intra_sock_rank == 0) {
        if (intra_node_leader_rank == 0) {
            /* Node leaders, allocate large buffers which is used to gather
             * data for the entire node. The same buffer is used for inter-node
             * gather as well. This saves us a memcpy operation*/
            if (rank == root) {
                intra_tmp_buf = MPL_malloc(
                    recvcnt * MPL_MAX(recvtype_extent, recvtype_true_extent) *
                        local_size,
                    MPL_MEM_COLL);
            } else {
                intra_tmp_buf =
                    MPL_malloc(send_nbytes * local_size, MPL_MEM_COLL);
            }

        } else {
            /* Socket leader, allocate tmp_buffer */
            if (rank == root) {
                intra_tmp_buf = MPL_malloc(
                    recvcnt * MPL_MAX(recvtype_extent, recvtype_true_extent) *
                        intra_sock_comm_size,
                    MPL_MEM_COLL);
            } else {
                intra_tmp_buf = MPL_malloc(send_nbytes * intra_sock_comm_size,
                                           MPL_MEM_COLL);
            }
        }

        if (intra_tmp_buf == NULL) {
            mpi_errno = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE,
                                             __func__, __LINE__, MPI_ERR_OTHER,
                                             "**nomem", 0);

            MPIR_TIMER_END(coll, gather, limic_scheme_pt_linear);
            return mpi_errno;
        }

        /*Local copy of buffer*/
        if (sendbuf != MPI_IN_PLACE) {
            MPIR_Memcpy(intra_tmp_buf, local_sendbuf, send_nbytes);
        } else {
            MPIR_Localcopy(((char *)recvbuf + rank * recvcnt * recvtype_extent),
                           recvcnt, recvtype, intra_tmp_buf, send_nbytes,
                           MPI_BYTE);
        }
    }

    if (local_rank != 0 && sendbuf == MPI_IN_PLACE) {
        mpi_errno = MPIR_Limic_Gather_OSU(
            intra_tmp_buf, (intra_sock_comm_size * send_nbytes),
            (recvbuf + (rank * nbytes)), nbytes, intra_sock_commptr);
    } else {
        mpi_errno = MPIR_Limic_Gather_OSU(
            intra_tmp_buf, (intra_sock_comm_size * send_nbytes), local_sendbuf,
            send_nbytes, intra_sock_commptr);
    }
    MPIR_ERR_CHECK(mpi_errno);

    /*Inter socket gather*/
    if (intra_sock_rank == 0) {
        /*When data in each socket is different*/
        if (shmem_commptr->dev.ch.is_socket_uniform != 1) {
            int *displs = NULL;
            int *recvcnts = NULL;
            int *socket_sizes;
            int i = 0;
            socket_sizes = shmem_commptr->dev.ch.socket_size;

            if (intra_node_leader_rank == 0) {
                tmp_buf = intra_tmp_buf;

                displs = MPL_malloc(sizeof(int) * intra_node_leader_comm_size,
                                    MPL_MEM_COLL);
                recvcnts = MPL_malloc(sizeof(int) * intra_node_leader_comm_size,
                                      MPL_MEM_COLL);
                if (!displs || !recvcnts) {
                    mpi_errno = MPIR_Err_create_code(
                        MPI_SUCCESS, MPIR_ERR_RECOVERABLE, __func__, __LINE__,
                        MPI_ERR_OTHER, "**nomem", 0);

                    MPIR_TIMER_END(coll, gather, limic_scheme_pt_linear);
                    return mpi_errno;
                }

                recvcnts[0] = socket_sizes[0] * nbytes;
                displs[0] = 0;

                for (i = 1; i < intra_node_leader_comm_size; i++) {
                    displs[i] = displs[i - 1] + socket_sizes[i - 1] * nbytes;
                    recvcnts[i] = socket_sizes[i] * nbytes;
                }

                mpi_errno =
                    MPIR_Gatherv(MPI_IN_PLACE, intra_sock_comm_size * nbytes,
                                 MPI_BYTE, tmp_buf, recvcnts, displs, MPI_BYTE,
                                 0, intra_node_leader_commptr, errflag);

                /*Free the displacement and recvcnts buffer*/
                MPL_free(displs);
                MPL_free(recvcnts);

            } else {
                mpi_errno =
                    MPIR_Gatherv(intra_tmp_buf, intra_sock_comm_size * nbytes,
                                 MPI_BYTE, tmp_buf, recvcnts, displs, MPI_BYTE,
                                 0, intra_node_leader_commptr, errflag);
            }
        } else {
            if (intra_node_leader_rank == 0) {
                tmp_buf = intra_tmp_buf;

                /*We have now completed the intra_sock gather and all the
                 * socket level leaders have data in their tmp_buf. So we
                 * set sendbuf = MPI_IN_PLACE and also explicitly set the
                 * is_data_avail= TEMP_BUF_HAS_DATA*/
                mpi_errno = MPIR_pt_pt_intra_gather(
                    MPI_IN_PLACE, (send_nbytes * intra_sock_comm_size),
                    MPI_BYTE, recvbuf, recvcnt, recvtype, root, rank, tmp_buf,
                    (send_nbytes * intra_sock_comm_size), TEMP_BUF_HAS_DATA,
                    intra_node_leader_commptr, intra_node_fn_ptr, errflag);
            } else {
                /*After the intra_sock gather, all the node level leaders
                 * have the data in intra_tmp_buf(sendbuf)
                 * and this is gathered into
                 * tmp_buf.
                 * Since the tmp_buf(in non-root processes) does not have
                 * the data in tmp_buf is_data_avail = TEMP_BUF_HAS_NO_DATA*/
                mpi_errno = MPIR_pt_pt_intra_gather(
                    intra_tmp_buf, (send_nbytes * intra_sock_comm_size),
                    MPI_BYTE, recvbuf, recvcnt, recvtype, root, rank, tmp_buf,
                    (send_nbytes * intra_sock_comm_size), TEMP_BUF_HAS_NO_DATA,
                    intra_node_leader_commptr, intra_node_fn_ptr, errflag);
            }
        }

        MPIR_ERR_CHECK(mpi_errno);
    }
fn_fail:
    /*Free the intra socket leader buffers*/
    if (intra_sock_rank == 0) {
        if ((intra_node_leader_rank != 0) && (intra_tmp_buf != NULL)) {
            MPL_free(intra_tmp_buf);
        }
    }
    MPIR_CHKLMEM_FREEALL();
    MPIR_TIMER_END(coll, gather, limic_scheme_pt_linear);
    return (mpi_errno);
}

static int MPIR_Limic_Gather_Scheme_LINEAR_PT(
    const void *sendbuf, int sendcnt, MPI_Datatype sendtype, void *recvbuf,
    int recvcnt, MPI_Datatype recvtype, int root, MPIR_Comm *comm_ptr,
    MVP_Gather_fn_t intra_node_fn_ptr, MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, gather, limic_scheme_linear_pt);
    void *intra_tmp_buf = NULL;
    int rank;
    int local_size;
    int mpi_errno = MPI_SUCCESS;
    MPI_Aint recvtype_size = 0, sendtype_size = 0, nbytes = 0;
    int sendtype_iscontig;
    int intra_sock_rank = 0, intra_sock_comm_size = 0;
    int intra_node_leader_rank = 0;
    MPI_Aint sendtype_extent = 0, recvtype_extent = 0; /* Datatype extent */
    MPI_Aint true_lb, sendtype_true_extent, recvtype_true_extent;
    MPI_Comm shmem_comm;
    MPIR_Comm *shmem_commptr;
    MPIR_Comm *intra_sock_commptr = NULL, *intra_node_leader_commptr = NULL;

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_gather_limic_scheme_linear_pt, 1);
    rank = comm_ptr->rank;

    if (((rank == root) && (recvcnt == 0)) ||
        ((rank != root) && (sendcnt == 0))) {
        MPIR_TIMER_END(coll, gather, limic_scheme_linear_pt);
        return MPI_SUCCESS;
    }

    if (sendtype != MPI_DATATYPE_NULL) {
        MPIR_Datatype_iscontig(sendtype, &sendtype_iscontig);
        MPIR_Datatype_get_extent_macro(sendtype, sendtype_extent);
        MPIR_Datatype_get_size_macro(sendtype, sendtype_size);
        MPIR_Type_get_true_extent_impl(sendtype, &true_lb,
                                       &sendtype_true_extent);
    }
    if (recvtype != MPI_DATATYPE_NULL) {
        MPIR_Datatype_get_extent_macro(recvtype, recvtype_extent);
        MPIR_Datatype_get_size_macro(recvtype, recvtype_size);
        MPIR_Type_get_true_extent_impl(recvtype, &true_lb,
                                       &recvtype_true_extent);
    }

    /* extract the rank,size information for the intra-node
     * communicator */
    shmem_comm = comm_ptr->dev.ch.shmem_comm;
    MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
    local_size = shmem_commptr->local_size;

    if (rank == root) {
        nbytes = recvcnt * recvtype_size;

    } else {
        nbytes = sendcnt * sendtype_size;
    }

    if (shmem_commptr->dev.ch.use_intra_sock_comm == 1) {
        MPIR_Comm_get_ptr(shmem_commptr->dev.ch.intra_sock_comm,
                          intra_sock_commptr);
        MPIR_Comm_get_ptr(shmem_commptr->dev.ch.intra_sock_leader_comm,
                          intra_node_leader_commptr);

        intra_sock_rank = intra_sock_commptr->rank;
        intra_sock_comm_size = intra_sock_commptr->local_size;
        if (intra_sock_rank == 0) {
            intra_node_leader_rank = intra_node_leader_commptr->rank;
        }
    }

    if (intra_sock_rank == 0) {
        if (intra_node_leader_rank == 0) {
            /* Node leaders, allocate large buffers which is used to gather
             * data for the entire node. The same buffer is used for inter-node
             * gather as well. This saves us a memcpy operation*/
            if (rank == root) {
                intra_tmp_buf = MPL_malloc(
                    recvcnt * MPL_MAX(recvtype_extent, recvtype_true_extent) *
                        local_size,
                    MPL_MEM_COLL);
            } else {
                intra_tmp_buf = MPL_malloc(
                    sendcnt * MPL_MAX(sendtype_extent, sendtype_true_extent) *
                        local_size,
                    MPL_MEM_COLL);
            }
        } else {
            /* Socket leader, allocate tmp_buffer */
            if (rank == root) {
                intra_tmp_buf = MPL_malloc(
                    recvcnt * MPL_MAX(recvtype_extent, recvtype_true_extent) *
                        intra_sock_comm_size,
                    MPL_MEM_COLL);
            } else {
                intra_tmp_buf = MPL_malloc(
                    sendcnt * MPL_MAX(sendtype_extent, sendtype_true_extent) *
                        intra_sock_comm_size,
                    MPL_MEM_COLL);
            }
        }
        if (intra_tmp_buf == NULL) {
            mpi_errno = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE,
                                             __func__, __LINE__, MPI_ERR_OTHER,
                                             "**nomem", 0);

            MPIR_TIMER_END(coll, gather, limic_scheme_linear_pt);
            return mpi_errno;
        }
    }

    /*Intra socket gather*/
    /*We are gathering the data into intra_tmp_buf and the output
     * will be of MPI_BYTE datatype. Since the tmp_buf has no
     * local data, we pass is_data_avail = TEMP_BUF_HAS_NO_DATA*/
    mpi_errno = MPIR_pt_pt_intra_gather(
        sendbuf, sendcnt, sendtype, recvbuf, recvcnt, recvtype, root, rank,
        intra_tmp_buf, nbytes, TEMP_BUF_HAS_NO_DATA, intra_sock_commptr,
        intra_node_fn_ptr, errflag);

    MPIR_ERR_CHECK(mpi_errno);

    /*Inter socket gather*/
    if (intra_sock_rank == 0) {
        if (intra_node_leader_rank == 0) {
            tmp_buf = intra_tmp_buf;
        }
        mpi_errno = MPIR_Limic_Gather_OSU(
            tmp_buf, (local_size * nbytes), intra_tmp_buf,
            (intra_sock_comm_size * nbytes), intra_node_leader_commptr);
    }

    MPIR_ERR_CHECK(mpi_errno);
fn_fail:
    /*Free the intra socket leader buffers*/
    if (intra_sock_rank == 0) {
        if ((intra_node_leader_rank != 0) && (intra_tmp_buf != NULL)) {
            MPL_free(intra_tmp_buf);
        }
    }

    MPIR_TIMER_END(coll, gather, limic_scheme_linear_pt);
    return (mpi_errno);
}

static int MPIR_Limic_Gather_Scheme_LINEAR_LINEAR(
    const void *sendbuf, int sendcnt, MPI_Datatype sendtype, void *recvbuf,
    int recvcnt, MPI_Datatype recvtype, int root, MPIR_Comm *comm_ptr,
    MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, gather, limic_scheme_linear_linear);
    void *intra_tmp_buf = NULL;
    void *local_sendbuf = NULL;
    int rank;
    int local_rank, local_size;
    int mpi_errno = MPI_SUCCESS;
    MPI_Aint recvtype_size = 0, nbytes = 0;
    int sendtype_iscontig;
    int intra_sock_rank = 0, intra_sock_comm_size = 0;
    int intra_node_leader_rank = 0;
    MPI_Aint send_nbytes = 0;
    MPI_Aint recvtype_extent = 0; /* Datatype extent */
    MPI_Aint true_lb, sendtype_true_extent, recvtype_true_extent;
    MPI_Comm shmem_comm;
    MPIR_Comm *shmem_commptr;
    MPIR_Comm *intra_sock_commptr = NULL, *intra_node_leader_commptr = NULL;
    MPI_Aint sendtype_size = 0;
    MPI_Aint position = 0;
    MPIR_CHKLMEM_DECL(1);

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_gather_limic_scheme_linear_linear, 1);
    rank = comm_ptr->rank;

    if (((rank == root) && (recvcnt == 0)) ||
        ((rank != root) && (sendcnt == 0))) {
        MPIR_TIMER_END(coll, gather, limic_scheme_linear_linear);
        return MPI_SUCCESS;
    }

    if (sendtype != MPI_DATATYPE_NULL) {
        MPIR_Datatype_iscontig(sendtype, &sendtype_iscontig);
        MPIR_Datatype_get_size_macro(sendtype, sendtype_size);
        MPIR_Type_get_true_extent_impl(sendtype, &true_lb,
                                       &sendtype_true_extent);
    }
    if (recvtype != MPI_DATATYPE_NULL) {
        MPIR_Datatype_get_extent_macro(recvtype, recvtype_extent);
        MPIR_Datatype_get_size_macro(recvtype, recvtype_size);
        MPIR_Type_get_true_extent_impl(recvtype, &true_lb,
                                       &recvtype_true_extent);
    }

    /* extract the rank,size information for the intra-node
     * communicator */
    shmem_comm = comm_ptr->dev.ch.shmem_comm;
    MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
    local_rank = shmem_commptr->rank;
    local_size = shmem_commptr->local_size;

    if (rank == root) {
        nbytes = recvcnt * recvtype_size;

    } else {
        nbytes = sendcnt * sendtype_size;
    }

    if (shmem_commptr->dev.ch.use_intra_sock_comm == 1) {
        MPIR_Comm_get_ptr(shmem_commptr->dev.ch.intra_sock_comm,
                          intra_sock_commptr);
        MPIR_Comm_get_ptr(shmem_commptr->dev.ch.intra_sock_leader_comm,
                          intra_node_leader_commptr);

        intra_sock_rank = intra_sock_commptr->rank;
        intra_sock_comm_size = intra_sock_commptr->local_size;
        if (intra_sock_rank == 0) {
            intra_node_leader_rank = intra_node_leader_commptr->rank;
        }
    }

    /*Pack data for non-contiguous buffer*/
    if ((!sendtype_iscontig) && (sendbuf != MPI_IN_PLACE)) {
        MPIR_Pack_size_impl(1, sendtype, &sendtype_size);
        send_nbytes = sendcnt * sendtype_size;
        MPIR_CHKLMEM_MALLOC(local_sendbuf, void *, send_nbytes, mpi_errno,
                            "local_sendbuf", MPL_MEM_COLL);
        MPIR_Typerep_pack(sendbuf, sendcnt, sendtype, position, local_sendbuf,
                          send_nbytes, &position);
    } else {
        local_sendbuf = (void *)sendbuf;
        send_nbytes = nbytes;
    }

    if (intra_sock_rank == 0) {
        if (intra_node_leader_rank == 0) {
            /* Node leaders, allocate large buffers which is used to gather
             * data for the entire node. The same buffer is used for inter-node
             * gather as well. This saves us a memcpy operation*/
            if (rank == root) {
                intra_tmp_buf = MPL_malloc(
                    recvcnt * MPL_MAX(recvtype_extent, recvtype_true_extent) *
                        local_size,
                    MPL_MEM_COLL);
            } else {
                intra_tmp_buf =
                    MPL_malloc(send_nbytes * local_size, MPL_MEM_COLL);
            }

        } else {
            /* Socket leader, allocate tmp_buffer */
            if (rank == root) {
                intra_tmp_buf = MPL_malloc(
                    recvcnt * MPL_MAX(recvtype_extent, recvtype_true_extent) *
                        intra_sock_comm_size,
                    MPL_MEM_COLL);
            } else {
                intra_tmp_buf = MPL_malloc(send_nbytes * intra_sock_comm_size,
                                           MPL_MEM_COLL);
            }
        }
        if (intra_tmp_buf == NULL) {
            mpi_errno = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE,
                                             __func__, __LINE__, MPI_ERR_OTHER,
                                             "**nomem", 0);

            MPIR_TIMER_END(coll, gather, limic_scheme_linear_linear);
            return mpi_errno;
        }

        /*Local copy of buffer*/
        if (sendbuf != MPI_IN_PLACE) {
            MPIR_Memcpy(intra_tmp_buf, local_sendbuf, send_nbytes);
        } else {
            MPIR_Localcopy(((char *)recvbuf + rank * recvcnt * recvtype_extent),
                           recvcnt, recvtype, intra_tmp_buf, send_nbytes,
                           MPI_BYTE);
        }
    }

    if (local_rank != 0 && sendbuf == MPI_IN_PLACE) {
        mpi_errno = MPIR_Limic_Gather_OSU(
            intra_tmp_buf, (intra_sock_comm_size * send_nbytes),
            (recvbuf + (rank * nbytes)), nbytes, intra_sock_commptr);
    } else {
        mpi_errno = MPIR_Limic_Gather_OSU(
            intra_tmp_buf, (intra_sock_comm_size * send_nbytes), local_sendbuf,
            send_nbytes, intra_sock_commptr);
    }
    MPIR_ERR_CHECK(mpi_errno);

    /*Inter socket gather*/
    if (intra_sock_rank == 0) {
        if (intra_node_leader_rank == 0) {
            tmp_buf = intra_tmp_buf;
        }
        mpi_errno = MPIR_Limic_Gather_OSU(
            tmp_buf, (local_size * send_nbytes), intra_tmp_buf,
            (intra_sock_comm_size * send_nbytes), intra_node_leader_commptr);
    }

    MPIR_ERR_CHECK(mpi_errno);
fn_fail:
    /*Free the intra socket leader buffers*/
    if (intra_sock_rank == 0) {
        if ((intra_node_leader_rank != 0) && (intra_tmp_buf != NULL)) {
            MPL_free(intra_tmp_buf);
        }
    }

    MPIR_CHKLMEM_FREEALL();
    MPIR_TIMER_END(coll, gather, limic_scheme_linear_linear);
    return (mpi_errno);
}

static int MPIR_Limic_Gather_Scheme_SINGLE_LEADER(
    const void *sendbuf, int sendcnt, MPI_Datatype sendtype, void *recvbuf,
    int recvcnt, MPI_Datatype recvtype, int root, MPIR_Comm *comm_ptr,
    MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, gather, limic_scheme_single_leader);
    void *local_sendbuf = NULL;
    int rank;
    int local_rank, local_size;
    int mpi_errno = MPI_SUCCESS;
    MPI_Aint recvtype_size = 0, nbytes = 0;
    int sendtype_iscontig;
    MPI_Aint send_nbytes = 0;
    MPI_Aint recvtype_extent = 0; /* Datatype extent */
    MPI_Aint true_lb, sendtype_true_extent, recvtype_true_extent;
    MPI_Comm shmem_comm;
    MPIR_Comm *shmem_commptr;
    MPI_Aint sendtype_size = 0;
    MPI_Aint position = 0;
    MPIR_CHKLMEM_DECL(1);

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_gather_limic_scheme_single_leader, 1);
    rank = comm_ptr->rank;

    if (((rank == root) && (recvcnt == 0)) ||
        ((rank != root) && (sendcnt == 0))) {
        MPIR_TIMER_END(coll, gather, limic_scheme_single_leader);
        return MPI_SUCCESS;
    }

    if (sendtype != MPI_DATATYPE_NULL) {
        MPIR_Datatype_iscontig(sendtype, &sendtype_iscontig);
        MPIR_Datatype_get_size_macro(sendtype, sendtype_size);
        MPIR_Type_get_true_extent_impl(sendtype, &true_lb,
                                       &sendtype_true_extent);
    }
    if (recvtype != MPI_DATATYPE_NULL) {
        MPIR_Datatype_get_extent_macro(recvtype, recvtype_extent);
        MPIR_Datatype_get_size_macro(recvtype, recvtype_size);
        MPIR_Type_get_true_extent_impl(recvtype, &true_lb,
                                       &recvtype_true_extent);
    }

    /* extract the rank,size information for the intra-node
     * communicator */
    shmem_comm = comm_ptr->dev.ch.shmem_comm;
    MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
    local_rank = shmem_commptr->rank;
    local_size = shmem_commptr->local_size;

    if (rank == root) {
        nbytes = recvcnt * recvtype_size;

    } else {
        nbytes = sendcnt * sendtype_size;
    }

    /*Pack data for non-contiguous buffer*/
    if ((!sendtype_iscontig) && (sendbuf != MPI_IN_PLACE)) {
        MPIR_Pack_size_impl(1, sendtype, &sendtype_size);
        send_nbytes = sendcnt * sendtype_size;
        MPIR_CHKLMEM_MALLOC(local_sendbuf, void *, send_nbytes, mpi_errno,
                            "local_sendbuf", MPL_MEM_COLL);
        MPIR_Typerep_pack(sendbuf, sendcnt, sendtype, position, local_sendbuf,
                          send_nbytes, &position);
    } else {
        local_sendbuf = (void *)sendbuf;
        send_nbytes = nbytes;
    }

    if (local_rank == 0) {
        /* Node leader, allocate tmp_buffer */
        if (rank == root) {
            tmp_buf = MPL_malloc(
                recvcnt * MPL_MAX(recvtype_extent, recvtype_true_extent) *
                    local_size,
                MPL_MEM_COLL);
        } else {
            tmp_buf = MPL_malloc(send_nbytes * local_size, MPL_MEM_COLL);
        }
        if (tmp_buf == NULL) {
            mpi_errno = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE,
                                             __func__, __LINE__, MPI_ERR_OTHER,
                                             "**nomem", 0);
            MPIR_TIMER_END(coll, gather, limic_scheme_single_leader);
            return mpi_errno;
        }

        /*Local copy of buffer*/
        if (sendbuf != MPI_IN_PLACE) {
            MPIR_Memcpy(tmp_buf, local_sendbuf, send_nbytes);
        } else {
            MPIR_Localcopy(((char *)recvbuf + rank * recvcnt * recvtype_extent),
                           recvcnt, recvtype, tmp_buf, send_nbytes, MPI_BYTE);
        }
    }

    if (local_rank != 0 && sendbuf == MPI_IN_PLACE) {
        mpi_errno = MPIR_Limic_Gather_OSU(tmp_buf, (local_size * send_nbytes),
                                          (recvbuf + (rank * nbytes)), nbytes,
                                          shmem_commptr);
    } else {
        mpi_errno = MPIR_Limic_Gather_OSU(tmp_buf, (local_size * send_nbytes),
                                          local_sendbuf, nbytes, shmem_commptr);
    }

    MPIR_ERR_CHECK(mpi_errno);

fn_fail:
    MPIR_CHKLMEM_FREEALL();
    MPIR_TIMER_END(coll, gather, limic_scheme_single_leader);
    return (mpi_errno);
}

int MPIR_Intra_node_LIMIC_Gather_MVP(const void *sendbuf, int sendcnt,
                                     MPI_Datatype sendtype, void *recvbuf,
                                     int recvcnt, MPI_Datatype recvtype,
                                     int root, MPIR_Comm *comm_ptr,
                                     MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, gather, intra_node_limic);
    int mpi_errno = MPI_SUCCESS;
    MPI_Comm shmem_comm;
    MPIR_Comm *shmem_commptr;

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_gather_intra_node_limic, 1);
    /* extract the rank,size information for the intra-node
     * communicator */
    shmem_comm = comm_ptr->dev.ch.shmem_comm;
    MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);

    /*This case uses the PT-PT scheme with binomial
     * algorithm */
    if ((shmem_commptr->dev.ch.use_intra_sock_comm == 1) &&
        (num_scheme == USE_GATHER_PT_PT_BINOMIAL)) {
        mpi_errno = MPIR_Limic_Gather_Scheme_PT_PT(
            sendbuf, sendcnt, sendtype, recvbuf, recvcnt, recvtype, root,
            comm_ptr, MPIR_Gather_intra, errflag);
        MPIR_ERR_CHECK(mpi_errno);
    }
    /*This case uses the PT-PT scheme with DIRECT
     * algorithm */
    else if ((shmem_commptr->dev.ch.use_intra_sock_comm == 1) &&
             (num_scheme == USE_GATHER_PT_PT_DIRECT)) {
        mpi_errno = MPIR_Limic_Gather_Scheme_PT_PT(
            sendbuf, sendcnt, sendtype, recvbuf, recvcnt, recvtype, root,
            comm_ptr, MPIR_Gather_MVP_Direct, errflag);
        MPIR_ERR_CHECK(mpi_errno);
    }
    /*This case uses the PT-LINEAR scheme with binomial
     * algorithm */
    else if ((shmem_commptr->dev.ch.use_intra_sock_comm == 1) &&
             (num_scheme == USE_GATHER_PT_LINEAR_BINOMIAL)) {
        mpi_errno = MPIR_Limic_Gather_Scheme_PT_LINEAR(
            sendbuf, sendcnt, sendtype, recvbuf, recvcnt, recvtype, root,
            comm_ptr, MPIR_Gather_intra, errflag);
        MPIR_ERR_CHECK(mpi_errno);

    }
    /*This case uses the PT-LINEAR scheme with DIRECT
     * algorithm */
    else if ((shmem_commptr->dev.ch.use_intra_sock_comm == 1) &&
             (num_scheme == USE_GATHER_PT_LINEAR_DIRECT)) {
        mpi_errno = MPIR_Limic_Gather_Scheme_PT_LINEAR(
            sendbuf, sendcnt, sendtype, recvbuf, recvcnt, recvtype, root,
            comm_ptr, MPIR_Gather_MVP_Direct, errflag);
        MPIR_ERR_CHECK(mpi_errno);

    }
    /*This case uses the LINEAR-PT scheme with binomial
     * algorithm */
    else if ((shmem_commptr->dev.ch.use_intra_sock_comm == 1) &&
             (num_scheme == USE_GATHER_LINEAR_PT_BINOMIAL)) {
        mpi_errno = MPIR_Limic_Gather_Scheme_LINEAR_PT(
            sendbuf, sendcnt, sendtype, recvbuf, recvcnt, recvtype, root,
            comm_ptr, MPIR_Gather_intra, errflag);
        MPIR_ERR_CHECK(mpi_errno);

    }
    /*This case uses the LINEAR-PT scheme with DIRECT
     * algorithm */
    else if ((shmem_commptr->dev.ch.use_intra_sock_comm == 1) &&
             (num_scheme == USE_GATHER_LINEAR_PT_DIRECT)) {
        mpi_errno = MPIR_Limic_Gather_Scheme_LINEAR_PT(
            sendbuf, sendcnt, sendtype, recvbuf, recvcnt, recvtype, root,
            comm_ptr, MPIR_Gather_MVP_Direct, errflag);
        MPIR_ERR_CHECK(mpi_errno);

    } else if ((shmem_commptr->dev.ch.use_intra_sock_comm == 1) &&
               (num_scheme == USE_GATHER_LINEAR_LINEAR)) {
        mpi_errno = MPIR_Limic_Gather_Scheme_LINEAR_LINEAR(
            sendbuf, sendcnt, sendtype, recvbuf, recvcnt, recvtype, root,
            comm_ptr, errflag);
        MPIR_ERR_CHECK(mpi_errno);

    } else if (((comm_ptr->dev.ch.shmem_coll_ok == 1) ||
                (shmem_commptr->dev.ch.use_intra_sock_comm == 1)) &&
               (num_scheme == USE_GATHER_SINGLE_LEADER)) {
        mpi_errno = MPIR_Limic_Gather_Scheme_SINGLE_LEADER(
            sendbuf, sendcnt, sendtype, recvbuf, recvcnt, recvtype, root,
            comm_ptr, errflag);
        MPIR_ERR_CHECK(mpi_errno);
    } else {
        /*This is a invalid case, if we are in LIMIC Gather
         * the code flow should be in one of the if-else case*/
        mpi_errno =
            MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, __func__,
                                 __LINE__, MPI_ERR_OTHER, "**badcase", 0);
    }

fn_fail:
    MPIR_TIMER_END(coll, gather, intra_node_limic);
    return (mpi_errno);
}

#endif /*#if defined(_SMP_LIMIC_) */
