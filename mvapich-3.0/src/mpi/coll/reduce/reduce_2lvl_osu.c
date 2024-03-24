#include "reduce_tuning.h"

int MPIR_Reduce_two_level_helper_MVP(const void *sendbuf, void *recvbuf,
                                     int count, MPI_Datatype datatype,
                                     MPI_Op op, int root, MPIR_Comm *comm_ptr,
                                     MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, reduce, two_level_helper);
    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_reduce_two_level_helper, 1);
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    int my_rank, total_size, local_rank, local_size;
    int leader_comm_rank = -1, leader_comm_size = 0;
    MPI_Comm shmem_comm, leader_comm;
    int leader_root, leader_of_root;
    MPIR_Comm *shmem_commptr = NULL, *leader_commptr = NULL;
    void *in_buf = NULL, *out_buf = NULL, *tmp_buf = NULL;
    MPI_Aint true_lb, true_extent, extent;
    int is_commutative = 0, stride = 0;
    int intra_node_root = 0;
    MPI_User_function *uop;
#ifdef HAVE_CXX_BINDING
    int is_cxx_uop = 0;
#endif
    MPIR_CHKLMEM_DECL(1);

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_num_shmem_coll_calls, 1);
    my_rank = comm_ptr->rank;
    total_size = comm_ptr->local_size;
    shmem_comm = comm_ptr->dev.ch.shmem_comm;

    MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
    local_rank = shmem_commptr->rank;
    local_size = shmem_commptr->local_size;

    leader_of_root = comm_ptr->dev.ch.leader_map[root];
    leader_root = comm_ptr->dev.ch.leader_rank[leader_of_root];

    /* Get func ptr for the reduction op
     * and initialize is_commutative and is_cxx_uop */
    MPIR_get_op_ptr(&uop, &is_commutative,
#ifdef HAVE_CXX_BINDING
                    &is_cxx_uop,
#endif
                    op);

    MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);
    MPIR_Datatype_get_extent_macro(datatype, extent);
    stride = count * MPL_MAX(extent, true_extent);

    if (local_size == total_size) {
        /* First handle the case where there is only one node */
        if (comm_ptr->dev.ch.shmem_coll_ok == 1 &&
            stride <= MVP_INTRA_SHMEM_REDUCE_MSG && MVP_USE_SHMEM_REDUCE &&
            is_commutative == 1) {
            if (local_rank == 0) {
                MPIR_CHKLMEM_MALLOC(tmp_buf, void *,
                                    count *(MPL_MAX(extent, true_extent)),
                                    mpi_errno, "receive buffer", MPL_MEM_COLL);
                tmp_buf = (void *)((char *)tmp_buf - true_lb);
            }

            if (sendbuf != MPI_IN_PLACE) {
                in_buf = (void *)sendbuf;
            } else {
                in_buf = recvbuf;
            }

            if (local_rank == 0) {
                if (my_rank != root) {
                    out_buf = tmp_buf;
                } else {
                    out_buf = recvbuf;
                    if (in_buf == out_buf) {
                        in_buf = MPI_IN_PLACE;
                        out_buf = recvbuf;
                    }
                }
            } else {
                if (my_rank != root) {
                    in_buf = (void *)sendbuf;
                    out_buf = NULL;
                } else {
                    if (sendbuf != MPI_IN_PLACE) {
                        in_buf = (void *)sendbuf;
                        out_buf = (void *)recvbuf;
                    } else {
                        in_buf = (void *)recvbuf;
                        out_buf = (void *)recvbuf;
                    }
                }
            }

            if (count * (MPL_MAX(extent, true_extent)) <
                MVP_SHMEM_COLL_MAX_MSG_SIZE) {
                mpi_errno =
                    MPIR_Reduce_shmem_MVP(in_buf, out_buf, count, datatype, op,
                                          0, shmem_commptr, errflag);
            } else {
                mpi_errno = MPIR_Reduce_intra_knomial_wrapper_MVP(
                    in_buf, out_buf, count, datatype, op, 0, shmem_commptr,
                    errflag);
            }

            if (local_rank == 0 && root != my_rank) {
                MPIR_PVAR_INC(reduce, two_level_helper, send, count, datatype);
                mpi_errno = MPIC_Send(out_buf, count, datatype, root,
                                      MPIR_REDUCE_TAG, comm_ptr, errflag);
            }
            if ((local_rank != 0) && (root == my_rank)) {
                MPIR_PVAR_INC(reduce, two_level_helper, recv, count, datatype);
                mpi_errno = MPIC_Recv(recvbuf, count, datatype, leader_of_root,
                                      MPIR_REDUCE_TAG, comm_ptr,
                                      MPI_STATUS_IGNORE, errflag);
            }
        } else {
            if (MVP_USE_KNOMIAL_REDUCE) {
                reduce_fn = &MPIR_Reduce_intra_knomial_wrapper_MVP;
            } else {
                reduce_fn = &MPIR_Reduce_binomial_MVP;
            }
            mpi_errno = reduce_fn(sendbuf, recvbuf, count, datatype, op, root,
                                  comm_ptr, errflag);
        }
        if (mpi_errno) {
            /* for communication errors, just record the error but
             * continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
        /* We are done */
        goto fn_exit;
    }

#ifdef CHANNEL_MRAIL_GEN2
    if (MVP_USE_SLOT_SHMEM_COLL && MVP_USE_ZCOPY_REDUCE &&
        stride <= MVP_SHMEM_COLL_SLOT_LEN &&
        comm_ptr->dev.ch.shmem_coll_ok == 1 && MVP_USE_SHMEM_REDUCE &&
        is_commutative == 1) {
        mpi_errno = MPIR_Reduce_Zcpy_MVP(sendbuf, recvbuf, count, datatype, op,
                                         root, comm_ptr, errflag);
        if (mpi_errno) {
            /* for communication errors, just record the error but
             * continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
        /* We are done */
        goto fn_exit;
    }
#endif /* CHANNEL_MRAIL_GEN2 */

    if (local_rank == 0) {
        leader_comm = comm_ptr->dev.ch.leader_comm;
        MPIR_Comm_get_ptr(leader_comm, leader_commptr);
        leader_comm_rank = leader_commptr->rank;
        leader_comm_size = leader_commptr->local_size;
        MPIR_CHKLMEM_MALLOC(tmp_buf, void *,
                            count *(MPL_MAX(extent, true_extent)), mpi_errno,
                            "receive buffer", MPL_MEM_COLL);
        tmp_buf = (void *)((char *)tmp_buf - true_lb);
    }
    if (sendbuf != MPI_IN_PLACE) {
        in_buf = (void *)sendbuf;
    } else {
        in_buf = recvbuf;
    }
    if (local_rank == 0) {
        out_buf = tmp_buf;
    } else {
        out_buf = NULL;
    }

    if (local_size > 1) {
        /* Lets do the intra-node reduce operations, if we have more than one
         * process in the node */

        /*Fix the input and outbuf buffers for the intra-node reduce.
         *Node leaders will have the reduced data in tmp_buf after
         *this step*/
        if (MVP_Reduce_intra_function == &MPIR_Reduce_shmem_MVP) {
            if (comm_ptr->dev.ch.shmem_coll_ok == 1 && MVP_USE_SHMEM_REDUCE &&
                is_commutative == 1 &&
                (count * (MPL_MAX(extent, true_extent)) <
                 MVP_SHMEM_COLL_MAX_MSG_SIZE)) {
                mpi_errno = MVP_Reduce_intra_function(
                    in_buf, out_buf, count, datatype, op, intra_node_root,
                    shmem_commptr, errflag);
            } else {
                mpi_errno = MPIR_Reduce_intra_knomial_wrapper_MVP(
                    in_buf, out_buf, count, datatype, op, intra_node_root,
                    shmem_commptr, errflag);
            }
        } else {
            mpi_errno = MVP_Reduce_intra_function(in_buf, out_buf, count,
                                                  datatype, op, intra_node_root,
                                                  shmem_commptr, errflag);
        }
        if (mpi_errno) {
            /* for communication errors, just record the error but
             * continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
    } else {
        tmp_buf = in_buf;
    }

    /* Now work on the inter-leader phase. Data is in tmp_buf */
    if (local_rank == 0 && leader_comm_size > 1) {
        /*The leader of root will have the global reduced data in tmp_buf
           or recv_buf
           at the end of the reduce */
        if (leader_comm_rank == leader_root) {
            if (my_rank == root) {
                /* I am the root of the leader-comm, and the
                 * root of the reduce op. So, I will write the
                 * final result directly into my recvbuf */
                if (tmp_buf != recvbuf) {
                    in_buf = tmp_buf;
                    out_buf = recvbuf;
                } else {
                    in_buf = MPI_IN_PLACE;
                    out_buf = recvbuf;
                }
            } else {
                in_buf = MPI_IN_PLACE;
                out_buf = tmp_buf;
            }
        } else {
            in_buf = tmp_buf;
            out_buf = NULL;
        }

        /* inter-leader communication  */
        mpi_errno = MVP_Reduce_function(in_buf, out_buf, count, datatype, op,
                                        leader_root, leader_commptr, errflag);
        if (mpi_errno) {
            /* for communication errors, just record the error
             * but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
    }

    if (local_size > 1) {
        /* Send the message to the root if the leader is not the
         * root of the reduce operation. The reduced data is in tmp_buf */
        if ((local_rank == 0) && (root != my_rank) &&
            (leader_root == leader_comm_rank)) {
            MPIR_PVAR_INC(reduce, two_level_helper, send, count, datatype);
            mpi_errno = MPIC_Send(tmp_buf, count, datatype, root,
                                  MPIR_REDUCE_TAG, comm_ptr, errflag);
            if (mpi_errno) {
                /* for communication errors, just record the error
                 * but continue */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
        }

        if ((local_rank != 0) && (root == my_rank)) {
            MPIR_PVAR_INC(reduce, two_level_helper, recv, count, datatype);
            mpi_errno = MPIC_Recv(recvbuf, count, datatype, leader_of_root,
                                  MPIR_REDUCE_TAG, comm_ptr, MPI_STATUS_IGNORE,
                                  errflag);
            if (mpi_errno) {
                /* for communication errors, just record the error but
                 * continue */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
        }
    }

fn_exit:
    MPIR_CHKLMEM_FREEALL();
    MPIR_TIMER_END(coll, reduce, two_level_helper);
    if (mpi_errno_ret) {
        mpi_errno = mpi_errno_ret;
    } else if (*errflag) {
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**coll_fail");
    }
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
