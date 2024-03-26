#include "mpiimpl.h"
#include "mvp_coll_shmem.h"
#include "allreduce_tuning.h"


/* This function is unnecessary, remove it in the future */
static int (*MPIR_Rank_list_mapper)(MPIR_Comm *, int) = NULL;

/* This is flat reduce-scatter-allgather allreduce (Ring Allreduce) */
/* TODO: add general MPIR_Allgather instead of p2p */
int MPIR_Allreduce_pt2pt_rs_MVP(const void *sendbuf, void *recvbuf, int count,
                                MPI_Datatype datatype, MPI_Op op,
                                MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, allreduce, shm_rs);
    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_allreduce_shm_rs, 1);     //算法调用次数统计
    double start=0,end=0,time;
    int s_r_count=0;
    int comm_size, rank;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    int mask, dst, is_commutative, pof2, newrank = 0, rem, newdst, i, send_idx,
                                         recv_idx, last_idx, send_cnt, recv_cnt,
                                         *cnts, *disps;
    MPI_Aint true_lb, true_extent, extent;
    void *tmp_buf;
    MPI_User_function *uop;
#ifdef HAVE_CXX_BINDING
    int is_cxx_uop = 0;
#endif
    MPIR_CHKLMEM_DECL(3);

    if (count == 0) {
        MPIR_TIMER_END(coll, allreduce, shm_rs);
        return MPI_SUCCESS;
    }

    /* homogeneous */

    comm_size = comm_ptr->local_size;

    /* Get func ptr for the reduction op
     * and initialize is_commutative and is_cxx_uop */
    MPIR_get_op_ptr(&uop, &is_commutative,
#ifdef HAVE_CXX_BINDING
                    &is_cxx_uop,
#endif
                    op);

    if (comm_ptr->dev.ch.rank_list == NULL || is_commutative != 1) {
        rank = comm_ptr->rank;
        MPIR_Rank_list_mapper = &Bunch_Rank_list_mapper;
    } else {
        /* my index on rank_list */
        rank = comm_ptr->dev.ch.rank_list_index;
        MPIR_Assert(rank >= 0);
        MPIR_Rank_list_mapper = &Cyclic_Rank_list_mapper;
    }

    /* need to allocate temporary buffer to store incoming data */
    MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);
    MPIR_Datatype_get_extent_macro(datatype, extent);

    MPIR_CHKLMEM_MALLOC(tmp_buf, void *, count *(MPL_MAX(extent, true_extent)),
                        mpi_errno, "temporary buffer", MPL_MEM_COLL);

    /* adjust for potential negative lower bound in datatype */
    tmp_buf = (void *)((char *)tmp_buf - true_lb);

    /* copy local data into recvbuf */
    if (sendbuf != MPI_IN_PLACE) {
        mpi_errno =
            MPIR_Localcopy(sendbuf, count, datatype, recvbuf, count, datatype);
        MPIR_ERR_CHKANDJUMP((mpi_errno), mpi_errno, MPI_ERR_OTHER, "**fail");
    }

    /* find nearest power-of-two less than or equal to comm_size */
    pof2 = comm_ptr->dev.ch.gpof2;

    rem = comm_size - pof2;

    /* In the non-power-of-two case, all even-numbered
     * processes of rank < 2*rem send their data to
     * (rank+1). These even-numbered processes no longer
     * participate in the algorithm until the very end. The
     * remaining processes form a nice power-of-two. */

    if (rank < 2 * rem) {
        if (rank % 2 == 0) {
            /* even */
            MPIR_PVAR_INC(allreduce, pt2pt_rs, send, count, datatype);
            printf("send_rank=%d recv_rank=%d \n",
            MPIR_Rank_list_mapper(comm_ptr, rank - 1),MPIR_Rank_list_mapper(comm_ptr, rank + 1));
            start=MPI_Wtime();
            mpi_errno = MPIC_Send(recvbuf, count, datatype,
                                  MPIR_Rank_list_mapper(comm_ptr, rank + 1),
                                  MPIR_ALLREDUCE_TAG, comm_ptr, errflag);
            end=MPI_Wtime();
            time=end-start;
            printf("time=%lf \n",end-start);
            MPI_PVAR_DETAIL_INFO_INC(MVP,mvp_coll_allreduce_pt2pt_rs,MPIR_Rank_list_mapper(comm_ptr, rank - 1),MPIR_Rank_list_mapper(comm_ptr, rank + 1)
            ,1,time,send);
            //save rank;data;time;

            if (mpi_errno) {
                /* for communication errors,
                 * just record the error but continue */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }

            /* temporarily set the rank to -1 so that this
             * process does not pariticipate in recursive
             * doubling */
            newrank = -1;
        } else {
            /* odd */
            MPIR_PVAR_INC(allreduce, pt2pt_rs, recv, count, datatype);
            s_r_count++;
            mpi_errno = MPIC_Recv(tmp_buf, count, datatype,
                                  MPIR_Rank_list_mapper(comm_ptr, rank - 1),
                                  MPIR_ALLREDUCE_TAG, comm_ptr,
                                  MPI_STATUS_IGNORE, errflag);
            if (mpi_errno) {
                /* for communication errors,
                 * just record the error but continue */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }

            /* do the reduction on received data. since the
             * ordering is right, it doesn't matter whether
             * the operation is commutative or not. */
            MPIR_MVP_Reduce_local(tmp_buf, recvbuf, (MPI_Aint)count, datatype,
                                  uop
#ifdef HAVE_CXX_BINDING
                                  ,
                                  is_cxx_uop
#endif
            );
            /* change the rank */
            newrank = rank / 2;
        }
    } else { /* rank >= 2*rem */
        newrank = rank - rem;
    }

    /* If op is user-defined or count is less than pof2, use
     * recursive doubling algorithm. Otherwise do a reduce-scatter
     * followed by allgather. (If op is user-defined,
     * derived datatypes are allowed and the user could pass basic
     * datatypes on one process and derived on another as long as
     * the type maps are the same. Breaking up derived
     * datatypes to do the reduce-scatter is tricky, therefore
     * using recursive doubling in that case.) */

    if (newrank != -1) {
        if ((HANDLE_GET_KIND(op) != HANDLE_KIND_BUILTIN) || (count < pof2)) {
            /* use recursive doubling */
            mask = 0x1;
            while (mask < pof2) {
                newdst = newrank ^ mask;
                /* find real rank of dest */
                dst = (newdst < rem) ? newdst * 2 + 1 : newdst + rem;

                /* Send the most current data, which is in recvbuf. Recv
                 * into tmp_buf */
                MPIR_PVAR_INC(allreduce, pt2pt_rs, send, count, datatype);
                MPIR_PVAR_INC(allreduce, pt2pt_rs, recv, count, datatype);
                start=MPI_Wtime();
                mpi_errno = MPIC_Sendrecv(
                    recvbuf, count, datatype,
                    MPIR_Rank_list_mapper(comm_ptr, dst), MPIR_ALLREDUCE_TAG,
                    tmp_buf, count, datatype,
                    MPIR_Rank_list_mapper(comm_ptr, dst), MPIR_ALLREDUCE_TAG,
                    comm_ptr, MPI_STATUS_IGNORE, errflag);
                end=MPI_Wtime();
                time=end-start;
                MPI_PVAR_DETAIL_INFO_INC(MVP,mvp_coll_allreduce_pt2pt_rs,MPIR_Rank_list_mapper(comm_ptr, dst),MPIR_Rank_list_mapper(comm_ptr, dst)
            ,1,time,send);
                if (mpi_errno) {
                    /* for communication errors,
                     * just record the error but continue */
                    *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                    MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                }

                /* tmp_buf contains data received in this step.
                 * recvbuf contains data accumulated so far */

                if (is_commutative || (dst < rank)) {
                    /* op is commutative OR the order is already right */
                    MPIR_MVP_Reduce_local(tmp_buf, recvbuf, (MPI_Aint)count,
                                          datatype, uop
#ifdef HAVE_CXX_BINDING
                                          ,
                                          is_cxx_uop
#endif
                    );
                } else {
                    /* op is noncommutative and the order is not right */
                    MPIR_MVP_Reduce_local(recvbuf, tmp_buf, (MPI_Aint)count,
                                          datatype, uop
#ifdef HAVE_CXX_BINDING
                                          ,
                                          is_cxx_uop
#endif
                    );

                    /* copy result back into recvbuf */
                    mpi_errno = MPIR_Localcopy(tmp_buf, count, datatype,
                                               recvbuf, count, datatype);
                    MPIR_ERR_CHKANDJUMP((mpi_errno), mpi_errno, MPI_ERR_OTHER,
                                        "**fail");
                }
                mask <<= 1;
            }
        } else {
            /* do a reduce-scatter followed by allgather */

            /* for the reduce-scatter, calculate the count that
             * each process receives and the displacement within
             * the buffer */

            MPIR_CHKLMEM_MALLOC(cnts, int *, pof2 * sizeof(int), mpi_errno,
                                "counts", MPL_MEM_COLL);
            MPIR_CHKLMEM_MALLOC(disps, int *, pof2 * sizeof(int), mpi_errno,
                                "displacements", MPL_MEM_COLL);

            for (i = 0; i < (pof2 - 1); i++) {
                cnts[i] = count / pof2;
            }
            cnts[pof2 - 1] = count - (count / pof2) * (pof2 - 1);

            disps[0] = 0;
            for (i = 1; i < pof2; i++) {
                disps[i] = disps[i - 1] + cnts[i - 1];
            }

            mask = 0x1;
            send_idx = recv_idx = 0;
            last_idx = pof2;
            while (mask < pof2) {
                newdst = newrank ^ mask;
                /* find real rank of dest */
                dst = (newdst < rem) ? newdst * 2 + 1 : newdst + rem;

                send_cnt = recv_cnt = 0;
                if (newrank < newdst) {
                    send_idx = recv_idx + pof2 / (mask * 2);
                    for (i = send_idx; i < last_idx; i++)
                        send_cnt += cnts[i];
                    for (i = recv_idx; i < send_idx; i++)
                        recv_cnt += cnts[i];
                } else {
                    recv_idx = send_idx + pof2 / (mask * 2);
                    for (i = send_idx; i < recv_idx; i++)
                        send_cnt += cnts[i];
                    for (i = recv_idx; i < last_idx; i++)
                        recv_cnt += cnts[i];
                }

                /* Send data from recvbuf. Recv into tmp_buf */
                MPIR_PVAR_INC(allreduce, pt2pt_rs, send, send_cnt, datatype);
                MPIR_PVAR_INC(allreduce, pt2pt_rs, recv, recv_cnt, datatype);
                start=MPI_Wtime();
                mpi_errno = MPIC_Sendrecv(
                    (char *)recvbuf + disps[send_idx] * extent, send_cnt,
                    datatype, MPIR_Rank_list_mapper(comm_ptr, dst),
                    MPIR_ALLREDUCE_TAG,
                    (char *)tmp_buf + disps[recv_idx] * extent, recv_cnt,
                    datatype, MPIR_Rank_list_mapper(comm_ptr, dst),
                    MPIR_ALLREDUCE_TAG, comm_ptr, MPI_STATUS_IGNORE, errflag);
                end=MPI_Wtime();
                time=end-start;
                MPI_PVAR_DETAIL_INFO_INC(MVP,mvp_coll_allreduce_pt2pt_rs,MPIR_Rank_list_mapper(comm_ptr, dst),MPIR_Rank_list_mapper(comm_ptr, dst)
            ,1,time,send);
                if (mpi_errno) {
                    /* for communication errors,
                     * just record the error but  continue */
                    *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                    MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                }

                /* tmp_buf contains data received in this step.
                 * recvbuf contains data accumulated so far */

                /* This algorithm is used only for predefined ops
                 * and predefined ops are always commutative. */

                (*uop)((char *)tmp_buf + disps[recv_idx] * extent,
                       (char *)recvbuf + disps[recv_idx] * extent, &recv_cnt,
                       &datatype);

                /* update send_idx for next iteration */
                send_idx = recv_idx;
                mask <<= 1;

                /* update last_idx, but not in last iteration
                 * because the value is needed in the allgather
                 * step below. */
                if (mask < pof2)
                    last_idx = recv_idx + pof2 / mask;
            }

            /* now do the allgather */

            mask >>= 1;
            while (mask > 0) {
                newdst = newrank ^ mask;
                /* find real rank of dest */
                dst = (newdst < rem) ? newdst * 2 + 1 : newdst + rem;

                send_cnt = recv_cnt = 0;
                if (newrank < newdst) {
                    /* update last_idx except on first iteration */
                    if (mask != pof2 / 2) {
                        last_idx = last_idx + pof2 / (mask * 2);
                    }

                    recv_idx = send_idx + pof2 / (mask * 2);
                    for (i = send_idx; i < recv_idx; i++) {
                        send_cnt += cnts[i];
                    }
                    for (i = recv_idx; i < last_idx; i++) {
                        recv_cnt += cnts[i];
                    }
                } else {
                    recv_idx = send_idx - pof2 / (mask * 2);
                    for (i = send_idx; i < last_idx; i++) {
                        send_cnt += cnts[i];
                    }
                    for (i = recv_idx; i < send_idx; i++) {
                        recv_cnt += cnts[i];
                    }
                }
                MPIR_PVAR_INC(allreduce, pt2pt_rs, send, send_cnt, datatype);
                MPIR_PVAR_INC(allreduce, pt2pt_rs, recv, recv_cnt, datatype);
                start=MPI_Wtime();
                mpi_errno = MPIC_Sendrecv(
                    (char *)recvbuf + disps[send_idx] * extent, send_cnt,
                    datatype, MPIR_Rank_list_mapper(comm_ptr, dst),
                    MPIR_ALLREDUCE_TAG,
                    (char *)recvbuf + disps[recv_idx] * extent, recv_cnt,
                    datatype, MPIR_Rank_list_mapper(comm_ptr, dst),
                    MPIR_ALLREDUCE_TAG, comm_ptr, MPI_STATUS_IGNORE, errflag);
                end=MPI_Wtime();
                time=end-start;
                MPI_PVAR_DETAIL_INFO_INC(MVP,mvp_coll_allreduce_pt2pt_rs,MPIR_Rank_list_mapper(comm_ptr, dst),MPIR_Rank_list_mapper(comm_ptr, dst)
            ,1,time,send);
                if (mpi_errno) {
                    /* for communication errors,
                     * just record the error but continue */
                    *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                    MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                }

                if (newrank > newdst) {
                    send_idx = recv_idx;
                }

                mask >>= 1;
            }
        }
    }

    /* In the non-power-of-two case, all odd-numbered
     * processes of rank < 2*rem send the result to
     * (rank-1), the ranks who didn't participate above. */
    if (rank < 2 * rem) {
        if (rank % 2) { /* odd */
            MPIR_PVAR_INC(allreduce, pt2pt_rs, send, count, datatype);
            start=MPI_Wtime();
            mpi_errno = MPIC_Send(recvbuf, count, datatype,
                                  MPIR_Rank_list_mapper(comm_ptr, rank - 1),
                                  MPIR_ALLREDUCE_TAG, comm_ptr, errflag);
            end=MPI_Wtime();
            time=end-start;
            MPI_PVAR_DETAIL_INFO_INC(MVP,mvp_coll_allreduce_pt2pt_rs,MPIR_Rank_list_mapper(comm_ptr,rank + 1),MPIR_Rank_list_mapper(comm_ptr,rank - 1)
            ,1,time,send);
        } else { /* even */
            MPIR_PVAR_INC(allreduce, pt2pt_rs, recv, count, datatype);
            mpi_errno = MPIC_Recv(recvbuf, count, datatype,
                                  MPIR_Rank_list_mapper(comm_ptr, rank + 1),
                                  MPIR_ALLREDUCE_TAG, comm_ptr,
                                  MPI_STATUS_IGNORE, errflag);
        }
        if (mpi_errno) {
            /* for communication errors, just record the error but continue */
            *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
        }
    }

fn_exit:
    MPIR_CHKLMEM_FREEALL();
    MPIR_TIMER_END(coll, allreduce, shm_rs);
    return (mpi_errno);

fn_fail:
    goto fn_exit;
}

/* This is flat ring-allreduce  allreduce */
/* TODO: this is similar to the func: MPIR_Allreduce_pt2pt_rs_MVP
 * And it is not implemented efficiently causing an additional local shifting */
int MPIR_Allreduce_pt2pt_ring_MVP(const void *sendbuf, void *recvbuf, int count,
                                  MPI_Datatype datatype, MPI_Op op,
                                  MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int comm_size, rank;
    int mpi_errno = MPI_SUCCESS;
    MPI_Aint true_lb, true_extent, extent;
    MPI_User_function *uop;
    int is_commutative;
#ifdef HAVE_CXX_BINDING
    int is_cxx_uop = 0;
#endif

    if (count == 0) {
        mpi_errno = MPI_SUCCESS;
        goto fn_exit;
    }

    /* homogeneous */

    comm_size = comm_ptr->local_size;
    rank = comm_ptr->rank;

    /* Get the operator and check whether it is commutative or not */
    MPIR_get_op_ptr(&uop, &is_commutative,
#ifdef HAVE_CXX_BINDING
                    &is_cxx_uop,
#endif
                    op);

    int context_id = (comm_ptr->comm_kind == MPIR_COMM_KIND__INTRACOMM) ?
                         MPIR_CONTEXT_INTRA_COLL :
                         MPIR_CONTEXT_INTER_COLL;

    MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);
    MPIR_Datatype_get_extent_macro(datatype, extent);

    MPI_Aint type_size = MPL_MAX(extent, true_extent);

    if (count % comm_size != 0 || sendbuf == MPI_IN_PLACE ||
        count < comm_size || !is_commutative) {
        return MPIR_Allreduce_pt2pt_rs_MVP(sendbuf, recvbuf, count, datatype,
                                           op, comm_ptr, errflag);
    }
    MPIR_TIMER_START(coll, allreduce, pt2pt_ring);
    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_allreduce_pt2pt_ring, 1);
    // memset(recvbuf, 0, type_size*count);

    MPIR_Assert((count * type_size) % comm_size == 0);
    MPI_Aint chunk_size = (count * type_size) / comm_size;
    int chunk_count = count / comm_size;

    {
        /* do a reduce-scatter followed by allgather */
        MPIR_Assert(sendbuf != MPI_IN_PLACE);

        int i = 0;
        int left = (rank - 1 + comm_size) % comm_size;
        int right = (rank + 1) % comm_size;
        void *recv_chunk = NULL, *send_chunk = NULL, *comp_chunk = NULL;
        MPIR_Request *recv_req_ptr = NULL, *send_req_ptr = NULL;

        for (i = 1; i < comm_size; i++) {
            MPIR_Request *recv_req_ptr = NULL, *send_req_ptr = NULL;

            /* iteration 1: read chunk murank - 1 from myrank - 1
             * and do compute locally, at the
             * same time, send chunk myrank + 1 to myrank + 1 */

            if (i == 1 && sendbuf != MPI_IN_PLACE)
                send_chunk =
                    (void *)sendbuf +
                    ((rank - (i - 1) + comm_size) % comm_size) * chunk_size;
            else
                send_chunk =
                    recvbuf +
                    ((rank - (i - 1) + comm_size) % comm_size) * chunk_size;

            recv_chunk =
                recvbuf + ((rank - i + comm_size) % comm_size) * chunk_size;

            comp_chunk = (void *)sendbuf +
                         ((rank - i + comm_size) % comm_size) * chunk_size;

            /*
            PRINT_DEBUG(1, "i = %d, sending chunk=%d to rank=%d
            and receiving chunk=%d from rank=%d
            and doing compute on chunk=%d \n",
                   i, ((rank - (i-1) + comm_size) % comm_size), right, ((rank -
                            i + comm_size) % comm_size), left, ((rank - i +
                                comm_size) % comm_size));
            */
            MPIR_PVAR_INC(allreduce, pt2pt_ring, recv, chunk_count, datatype);
            mpi_errno = MPID_Irecv(recv_chunk, chunk_count, datatype, left, 11,
                                   comm_ptr, context_id, &recv_req_ptr);
            MPIR_ERR_CHECK(mpi_errno);

            MPIR_PVAR_INC(allreduce, pt2pt_ring, send, chunk_count, datatype);
            mpi_errno = MPID_Isend(send_chunk, chunk_count, datatype, right, 11,
                                   comm_ptr, context_id, &send_req_ptr);
            MPIR_ERR_CHECK(mpi_errno);

            mpi_errno = MPIC_Wait(recv_req_ptr, errflag);
            MPIR_ERR_CHECK(mpi_errno);

            /* now do the computation with some MPI_Test (TODO) in between */
            MPIR_MVP_Reduce_local(comp_chunk, recv_chunk, (MPI_Aint)chunk_count,
                                  datatype, uop
#ifdef HAVE_CXX_BINDING
                                  ,
                                  is_cxx_uop
#endif
            );

            mpi_errno = MPIC_Wait(send_req_ptr, errflag);
            MPIR_ERR_CHECK(mpi_errno);

            MPIR_Request_free(send_req_ptr);
            MPIR_Request_free(recv_req_ptr);
        }

        /*
        if (rank == 0) {
        PRINT_DEBUG(1, "Final Results count = %d\n", count);
        for (i = 0; i < count; i++) {
            fprintf(stderr, "%d ", ((float*)recvbuf)[i]);
        }
        PRINT_DEBUG(1, "\n");
        }
        */
        /* Allgather on recvbuf with  one elemented  shifted to right */

        for (i = 1; i < comm_size; i++) {
            recv_chunk =
                ((rank - (i - 1) + comm_size) % comm_size) * chunk_size +
                recvbuf;
            MPIR_PVAR_INC(allreduce, pt2pt_ring, recv, chunk_count, datatype);
            mpi_errno = MPID_Irecv(recv_chunk, chunk_count, datatype, left, 11,
                                   comm_ptr, context_id, &recv_req_ptr);
            MPIR_ERR_CHECK(mpi_errno);

            send_chunk =
                ((rank - (i - 2) + comm_size) % comm_size) * chunk_size +
                recvbuf;
            MPIR_PVAR_INC(allreduce, pt2pt_ring, send, chunk_count, datatype);
            mpi_errno = MPID_Isend(send_chunk, chunk_count, datatype, right, 11,
                                   comm_ptr, context_id, &send_req_ptr);
            MPIR_ERR_CHECK(mpi_errno);

            mpi_errno = MPIC_Wait(recv_req_ptr, errflag);
            MPIR_ERR_CHECK(mpi_errno);
            mpi_errno = MPIC_Wait(send_req_ptr, errflag);
            MPIR_ERR_CHECK(mpi_errno);

            MPIR_Request_free(send_req_ptr);
            MPIR_Request_free(recv_req_ptr);
        }
    }

fn_exit:
    MPIR_TIMER_END(coll, allreduce, pt2pt_ring);
    return (mpi_errno);

fn_fail:
    goto fn_exit;
}

/* This is flat ring-allreduce  allreduce with MPI_INPLACE for sendbuf */
/* TODO: this is similar to the func: MPIR_Allreduce_pt2pt_rs_MVP
 *  * And it is not implemented efficiently causing an additional local shifting
 *  */
int MPIR_Allreduce_pt2pt_ring_inplace_MVP(const void *sendbuf, void *recvbuf,
                                          int count, MPI_Datatype datatype,
                                          MPI_Op op, MPIR_Comm *comm_ptr,
                                          MPIR_Errflag_t *errflag)
{
    int comm_size, rank;
    int mpi_errno = MPI_SUCCESS;
    MPI_Aint true_lb, true_extent, extent;
    MPI_User_function *uop;
    int is_commutative;
#ifdef HAVE_CXX_BINDING
    int is_cxx_uop = 0;
#endif
    MPIR_CHKLMEM_DECL(3);

    if (count == 0) {
        return MPI_SUCCESS;
    }

    /* homogeneous */

    comm_size = comm_ptr->local_size;
    rank = comm_ptr->rank;

    /* Get the operator and check whether it is commutative or not */
    MPIR_get_op_ptr(&uop, &is_commutative,
#ifdef HAVE_CXX_BINDING
                    &is_cxx_uop,
#endif
                    op);

    int context_id = (comm_ptr->comm_kind == MPIR_COMM_KIND__INTRACOMM) ?
                         MPIR_CONTEXT_INTRA_COLL :
                         MPIR_CONTEXT_INTER_COLL;

    MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);
    MPIR_Datatype_get_extent_macro(datatype, extent);

    MPI_Aint type_size = MPL_MAX(extent, true_extent);

    if (count % comm_size != 0 || sendbuf == MPI_IN_PLACE ||
        count < comm_size || !is_commutative) {
        mpi_errno = MPIR_Allreduce_pt2pt_rs_MVP(
            sendbuf, recvbuf, count, datatype, op, comm_ptr, errflag);
        return mpi_errno;
    }
    MPIR_TIMER_START(coll, allreduce, pt2pt_ring_inplace);
    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_allreduce_pt2pt_ring_inplace, 1);
    // memset(recvbuf, 0, type_size*count);

    MPIR_Assert((count * type_size) % comm_size == 0);
    MPI_Aint chunk_size = (count * type_size) / comm_size;
    int chunk_count = count / comm_size;

    /* TODO: try to do this malloc only once for entire job run */
    void *temp_buf = MPL_malloc(chunk_size, MPL_MEM_COLL);

    {
        /* do a reduce-scatter followed by allgather */
        MPIR_Assert(sendbuf == MPI_IN_PLACE);

        int i = 0;
        int left = (rank - 1 + comm_size) % comm_size;
        int right = (rank + 1) % comm_size;
        void *recv_chunk = NULL, *send_chunk = NULL, *comp_chunk = NULL;
        MPIR_Request *recv_req_ptr = NULL, *send_req_ptr = NULL;

        // fprintf(stderr, "starting the  main loop\n");
        for (i = 1; i < comm_size; i++) {
            MPIR_Request *recv_req_ptr = NULL, *send_req_ptr = NULL;

            /* iteration 1: read chunk murank - 1 from myrank - 1
             * and do compute locally, at the
             * same time, send chunk myrank + 1 to myrank + 1 */

            send_chunk = recvbuf + ((rank - (i - 1) + comm_size) % comm_size) *
                                       chunk_size;

            // recv_chunk  = recvbuf
            //+ ((rank - i + comm_size) % comm_size)*chunk_size;
            recv_chunk = temp_buf;

            // comp_chunk  = sendbuf
            //+ ((rank - i + comm_size) % comm_size)*chunk_size;
            comp_chunk =
                recvbuf + ((rank - i + comm_size) % comm_size) * chunk_size;

            /*
            PRINT_DEBUG(1, "i = %d, sending chunk=%d to rank=%d
            and receiving chunk=%d from rank=%d
            and doing compute on chunk=%d \n",
                   i, ((rank - (i-1) + comm_size) % comm_size), right, ((rank -
                            i + comm_size) % comm_size), left, ((rank - i +
                                comm_size) % comm_size));
            */
            MPIR_PVAR_INC(allreduce, pt2pt_ring_inplace, recv, chunk_count,
                          datatype);
            mpi_errno = MPID_Irecv(recv_chunk, chunk_count, datatype, left, 11,
                                   comm_ptr, context_id, &recv_req_ptr);
            MPIR_ERR_CHECK(mpi_errno);

            MPIR_PVAR_INC(allreduce, pt2pt_ring_inplace, send, chunk_count,
                          datatype);
            mpi_errno = MPID_Isend(send_chunk, chunk_count, datatype, right, 11,
                                   comm_ptr, context_id, &send_req_ptr);
            MPIR_ERR_CHECK(mpi_errno);

            mpi_errno = MPIC_Wait(recv_req_ptr, errflag);
            MPIR_ERR_CHECK(mpi_errno);

            /* now do the computation with some MPI_Test (TODO) in between */
            MPIR_MVP_Reduce_local(recv_chunk, comp_chunk, (MPI_Aint)chunk_count,
                                  datatype, uop
#ifdef HAVE_CXX_BINDING
                                  ,
                                  is_cxx_uop
#endif
            );

            mpi_errno = MPIC_Wait(send_req_ptr, errflag);
            MPIR_ERR_CHECK(mpi_errno);

            MPIR_Request_free(send_req_ptr);
            MPIR_Request_free(recv_req_ptr);

            //  fprintf(stderr, "finished it = %d \n");
        }

        /*
        if (rank == 0) {
        PRINT_DEBUG(1, "Final Results count = %d\n", count);
        for (i = 0; i < count; i++) {
            fprintf(stderr, "%d ", ((float*)recvbuf)[i]);
        }
        PRINT_DEBUG(1, "\n");
        }
        */
        /* Allgather on recvbuf with  one elemented  shifted to right */
        // MPIR_Allgather_MVP(MPI_IN_PLACE, chunk_count, datatype, recvbuf,
        // chunk_count,
        //       datatype, comm_ptr, errflag);

        for (i = 1; i < comm_size; i++) {
            recv_chunk =
                ((rank - (i - 1) + comm_size) % comm_size) * chunk_size +
                recvbuf;
            MPIR_PVAR_INC(allreduce, pt2pt_ring_inplace, recv, chunk_count,
                          datatype);
            mpi_errno = MPID_Irecv(recv_chunk, chunk_count, datatype, left, 11,
                                   comm_ptr, context_id, &recv_req_ptr);
            MPIR_ERR_CHECK(mpi_errno);

            send_chunk =
                ((rank - (i - 2) + comm_size) % comm_size) * chunk_size +
                recvbuf;
            MPIR_PVAR_INC(allreduce, pt2pt_ring_inplace, send, chunk_count,
                          datatype);
            mpi_errno = MPID_Isend(send_chunk, chunk_count, datatype, right, 11,
                                   comm_ptr, context_id, &send_req_ptr);
            MPIR_ERR_CHECK(mpi_errno);

            mpi_errno = MPIC_Wait(recv_req_ptr, errflag);
            MPIR_ERR_CHECK(mpi_errno);
            mpi_errno = MPIC_Wait(send_req_ptr, errflag);
            MPIR_ERR_CHECK(mpi_errno);

            MPIR_Request_free(send_req_ptr);
            MPIR_Request_free(recv_req_ptr);
        }
    }

fn_exit:
    MPIR_CHKLMEM_FREEALL();
    MPL_free(temp_buf);
    MPIR_TIMER_END(coll, allreduce, pt2pt_ring);
    return (mpi_errno);

fn_fail:
    goto fn_exit;
}

int MPIR_Allreduce_pt2pt_ring_wrapper_MVP(const void *sendbuf, void *recvbuf,
                                          int count, MPI_Datatype datatype,
                                          MPI_Op op, MPIR_Comm *comm_ptr,
                                          MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, allreduce, pt2pt_ring_wrapper);
    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_allreduce_pt2pt_ring_wrapper, 1);

    /* TODO make this algo cyclic process mapping aware */
    int mpi_errno = MPI_SUCCESS;
    int comm_size = 0;
    int chunk = 0;
    int new_count = 0;
    int remaining_count = 0;
    MPI_Aint sendtype_size = 0;

    comm_size = comm_ptr->local_size;
    mpi_errno = MPI_SUCCESS;
    chunk = count / comm_size;
    new_count = chunk * comm_size;
    remaining_count = count - new_count;

    MPI_Aint true_lb, true_extent, extent;
    MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);
    MPIR_Datatype_get_extent_macro(datatype, extent);
    sendtype_size = MPL_MAX(extent, true_extent);

    if ((comm_ptr->local_size == 1) ||
        (comm_ptr->dev.ch.allgather_comm_ok != 0 &&
         comm_ptr->dev.ch.is_blocked == 0)) {
        /* for local size 1
         * or cyclic hostfiles use red-scat-allgather algorithm */
        mpi_errno = MPIR_Allreduce_pt2pt_rs_MVP(
            sendbuf, recvbuf, count, datatype, op, comm_ptr, errflag);
        MPIR_TIMER_END(coll, allreduce, pt2pt_ring_wrapper);
        return mpi_errno;
    }

    if (sendbuf != MPI_IN_PLACE)
        mpi_errno = MPIR_Allreduce_pt2pt_ring_MVP(
            sendbuf, recvbuf, new_count, datatype, op, comm_ptr, errflag);
    else
        mpi_errno = MPIR_Allreduce_pt2pt_ring_inplace_MVP(
            sendbuf, recvbuf, new_count, datatype, op, comm_ptr, errflag);

    if (mpi_errno != MPI_SUCCESS || (remaining_count == 0)) {
        MPIR_TIMER_END(coll, allreduce, pt2pt_ring_wrapper);
        return mpi_errno;
    }

    /* now Allreduce the remaining count */
    if (sendbuf != MPI_IN_PLACE) {
        mpi_errno = MPIR_Allreduce_pt2pt_rs_MVP(
            sendbuf + new_count * sendtype_size,
            recvbuf + new_count * sendtype_size, remaining_count, datatype, op,
            comm_ptr, errflag);
        MPIR_TIMER_END(coll, allreduce, pt2pt_ring_wrapper);
        return mpi_errno;
    } else {
        mpi_errno = MPIR_Allreduce_pt2pt_rs_MVP(
            MPI_IN_PLACE, recvbuf + new_count * sendtype_size, remaining_count,
            datatype, op, comm_ptr, errflag);
        MPIR_TIMER_END(coll, allreduce, pt2pt_ring_wrapper);
        return mpi_errno;
    }
}

/* This is flat reduce-scatter-allgather allreduce using MPR_reduce_scatter
 * and  MPR_Allgather instead of uisng Send/Recv like other functions */
/* This function uses reduce_scatter and allgather colls */
int MPIR_Allreduce_pt2pt_reduce_scatter_allgather_MVP(
    const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
    MPI_Op op, MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, allreduce, reduce_scatter_allgather_colls);
    MPIR_T_PVAR_COUNTER_INC(
        MVP, mvp_coll_allreduce_reduce_scatter_allgather_colls, 1);
    int comm_size, rank;
    int mpi_errno = MPI_SUCCESS;
    int i, *cnts, *disps;
    MPI_Aint true_lb, true_extent, extent;
    void *tmp_buf;
    MPIR_CHKLMEM_DECL(3);

    if (count == 0) {
        MPIR_TIMER_END(coll, allreduce, reduce_scatter_allgather_colls);
        return MPI_SUCCESS;
    }

    /* homogeneous */

    comm_size = comm_ptr->local_size;
    rank = comm_ptr->rank;

    if (count < comm_size) {
        mpi_errno = MPIR_Allreduce_pt2pt_rs_MVP(
            sendbuf, recvbuf, count, datatype, op, comm_ptr, errflag);
        return mpi_errno;
    }

    MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);
    MPIR_Datatype_get_extent_macro(datatype, extent);

    {
        /* do a reduce-scatter followed by allgather */

        /* for the reduce-scatter, calculate the count that
           each process receives and the displacement within
           the buffer */

        MPIR_CHKLMEM_MALLOC(cnts, int *, comm_size * sizeof(int), mpi_errno,
                            "counts", MPL_MEM_COLL);
        for (i = 0; i < (comm_size - 1); i++) {
            cnts[i] = count / comm_size;
        }
        cnts[comm_size - 1] = count - (count / comm_size) * (comm_size - 1);

        MPIR_CHKLMEM_MALLOC(tmp_buf, void *,
                            cnts[rank] * (MPL_MAX(extent, true_extent)),
                            mpi_errno, "temporary buffer", MPL_MEM_COLL);

        /* adjust for potential negative lower bound in datatype */
        tmp_buf = (void *)((char *)tmp_buf - true_lb);
        MPIR_CHKLMEM_MALLOC(disps, int *, comm_size * sizeof(int), mpi_errno,
                            "displacements", MPL_MEM_COLL);
        disps[0] = 0;
        for (i = 1; i < comm_size; i++) {
            disps[i] = disps[i - 1] + cnts[i - 1];
        }

        void *tmp_recvbuf =
            recvbuf + disps[rank] * (MPL_MAX(extent, true_extent));
        if (sendbuf != MPI_IN_PLACE) {
            mpi_errno = MPIR_Reduce_scatter_MVP(
                sendbuf, tmp_recvbuf, cnts, datatype, op, comm_ptr, errflag);
            MPIR_ERR_CHKANDJUMP((mpi_errno), mpi_errno, MPI_ERR_OTHER,
                                "**fail");
        } else {
            mpi_errno = MPIR_Reduce_scatter_MVP(
                MPI_IN_PLACE, recvbuf, cnts, datatype, op, comm_ptr, errflag);
            MPIR_ERR_CHKANDJUMP((mpi_errno), mpi_errno, MPI_ERR_OTHER,
                                "**fail");

            if (recvbuf != tmp_recvbuf) {
                /* need to shift the resutls to the location pointed by
                 * tmp_recbuf so that the following Allgather IN_PLACE
                 * works properly */
                if (disps[rank] >= cnts[rank]) {
                    /* make sure that there is no overlap between src and dst */
                    mpi_errno =
                        MPIR_Localcopy(recvbuf, cnts[rank], datatype,
                                       tmp_recvbuf, cnts[rank], datatype);
                    MPIR_ERR_CHKANDJUMP((mpi_errno), mpi_errno, MPI_ERR_OTHER,
                                        "**fail");
                } else {
                    /* there is overlap between src and dst of shift */
                    void *shift_recvbuf =
                        tmp_recvbuf +
                        disps[rank] * (MPL_MAX(extent, true_extent));
                    MPI_Aint overlapped_count = cnts[rank] - disps[rank];
                    MPIR_Assert(overlapped_count > 0);

                    /* first shift the overlapped data */
                    mpi_errno = MPIR_Localcopy(tmp_recvbuf, overlapped_count,
                                               datatype, shift_recvbuf,
                                               overlapped_count, datatype);
                    MPIR_ERR_CHKANDJUMP((mpi_errno), mpi_errno, MPI_ERR_OTHER,
                                        "**fail");
                    /* now copy the non overlapped data */
                    mpi_errno =
                        MPIR_Localcopy(recvbuf, disps[rank], datatype,
                                       tmp_recvbuf, disps[rank], datatype);
                    MPIR_ERR_CHKANDJUMP((mpi_errno), mpi_errno, MPI_ERR_OTHER,
                                        "**fail");
                }
            }
        }

        if (count % comm_size == 0) {
            MPIR_Allgather_MVP(MPI_IN_PLACE, cnts[rank], datatype, recvbuf,
                               cnts[rank], datatype, comm_ptr, errflag);
        } else {
            MPIR_Allgatherv_MVP(MPI_IN_PLACE, cnts[rank], datatype, recvbuf,
                                cnts, disps, datatype, comm_ptr, errflag);
        }
    }

fn_exit:
    MPIR_CHKLMEM_FREEALL();
    MPIR_TIMER_END(coll, allreduce, reduce_scatter_allgather_colls);
    return (mpi_errno);

fn_fail:
    goto fn_exit;
}
