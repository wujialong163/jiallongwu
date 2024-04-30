/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda. For detailed
 * copyright and licensing information, please refer to the copyright file
 * COPYRIGHT in the top level MVAPICH directory.
 */

#include "mvp_smp_impl.h"
#include "mvp_pkt.h"
#include "mvp_req_fields.h"
#include "mvp_req.h"

/* Send a contiguous eager message.  We'll want to optimize (and possibly
   inline) this.

   Make sure that buf is at the beginning of the data to send;
   adjust by adding dt_true_lb if necessary

   TODO: this can probably be made into a `mvp_eager.h` header file
         and filled with static inline functions
*/
int mvp_smp_fast_write_contig();
int MPIDI_MVP_ContigSend();

int MPIDI_MVP_EagerContigSend(MPIR_Request **sreq_p,
                              MPIDI_MVP_Pkt_type_t reqtype, const void *buf,
                              intptr_t data_sz, int rank, int tag,
                              MPIR_Comm *comm, int context_offset)
{
    int mpi_errno = MPI_SUCCESS;

    /* TODO: add propper error handling here */
    if (MPIDI_MVP_ContigSend(sreq_p, MPIDI_MVP_PKT_EAGER_SEND_CONTIG, buf,
                             data_sz, rank, tag, comm, context_offset) == 0) {
        goto fn_exit;
        /* do not error out here, ContigSend does not return error codes yet*/
        MPIR_ERR_SETANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**fail", "**fail %s",
                             "ContigSend failed, failure unknown");
    }
#if 0 /* What is all this? */
    MPIDI_av_entry_t *av;
    MPIDI_MVP_ep_t *vc;
    MPIDI_MVP_Pkt_t upkt;
    MPIDI_MVP_Pkt_eager_send_t *const eager_pkt = &upkt.eager_send;
    MPIDI_MVP_request_t *sreq = NULL;
    struct iovec iov[2];
    MPID_Seqnum_t seqnum;

    MPIDI_Pkt_init(eager_pkt, reqtype);
    eager_pkt->match.parts.rank       = comm->rank;
    eager_pkt->match.parts.tag        = tag;
    eager_pkt->match.parts.context_id = comm->context_id + context_offset;
    eager_pkt->sender_req_id          = MPI_REQUEST_NULL;
    eager_pkt->data_sz                = data_sz;

    iov[0].iov_base = (void *)eager_pkt;
    iov[0].iov_len  = sizeof(*eager_pkt);

    /*MPL_DBG_MSG_FMT(MPIDI_MVP_DBG_OTHER,
                    VERBOSE,
                    (MPL_DBG_FDEST,
                     "sending contiguous eager message, data_sz=%" PRIdPTR,
                     data_sz));*/

    iov[1].iov_base = (void *)buf;
    iov[1].iov_len  = data_sz;

    /* TODO: remove this comment */
    /* MPIDI_Comm_get_vc_set_active(comm, rank, &vc); */
    av = MPIDIU_comm_rank_to_av(comm, rank);
    vc = MPIDI_MVP_VC(av);
    /* MPIDI_CHANGE_VC_STATE(vc, ACTIVE) */

    MPIDI_VC_FAI_send_seqnum(vc, seqnum);
    MPIDI_Pkt_set_seqnum(eager_pkt, seqnum);

    /*MPL_DBG_MSGPKT(
      vc, tag, eager_pkt->match.parts.context_id, rank, data_sz, "EagerContig");*/
    MPID_THREAD_CS_ENTER(POBJ, vc->pobj_mutex);
    mpi_errno = MPIDI_MVP_smp_iStartMsgv(vc, iov, 2, sreq_p);
    MPID_THREAD_CS_EXIT(POBJ, vc->pobj_mutex);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER, "**ch4|eagermsg");
    }

    sreq = MPIDI_MVP_REQUEST_FROM_MPICH(*sreq_p);
    if (sreq != NULL) {
        MPIDI_Request_set_seqnum((*sreq_p), seqnum);
        MPIDI_Request_set_type((*sreq_p), MPIDI_REQUEST_TYPE_SEND);
    }
#endif
fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIDI_MVP_EagerNonContigSend(MPIR_Request **sreq_p,
                                 MPIDI_MVP_Pkt_type_t reqtype, const void *buf,
                                 MPI_Aint count, MPI_Datatype datatype,
                                 intptr_t data_sz, int rank, int tag,
                                 MPIR_Comm *comm, int context_offset)
{
    int mpi_errno = MPI_SUCCESS;
    void *tmp_buf = NULL;
    MPI_Aint position = 0;

    tmp_buf = MPL_malloc(data_sz, MPL_MEM_BUFFER);
    if (!tmp_buf) {
        mpi_errno =
            MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, __func__,
                                 __LINE__, MPI_ERR_OTHER, "**nomem", 0);
        MPIR_ERR_POP(mpi_errno);
    }

    mpi_errno =
        MPIR_Typerep_pack(buf, count, datatype, 0, tmp_buf, data_sz, &position);
    MPIR_ERR_CHECK(mpi_errno);

    mpi_errno =
        MPIDI_MVP_ContigSend(sreq_p, MPIDI_MVP_PKT_EAGER_SEND_CONTIG, tmp_buf,
                             data_sz, rank, tag, comm, context_offset);
    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    if (tmp_buf) {
        MPL_free(tmp_buf);
    }
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

/* Send a short contiguous eager message.  We'll want to optimize (and possibly
   inline) this

   Make sure that buf is at the beginning of the data to send;
   adjust by adding dt_true_lb if necessary

   We may need a nonblocking (cancellable) version of this, which will
   have a smaller payload.
*/
int MPIDI_MVP_EagerContigShortSend(MPIR_Request **sreq_p,
                                   MPIDI_MVP_Pkt_type_t reqtype,
                                   const void *buf, intptr_t data_sz, int rank,
                                   int tag, MPIR_Comm *comm, int context_offset)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_av_entry_t *av;
    MPIDI_MVP_ep_t *vc;
    MPIDI_MVP_Pkt_t upkt;
    MPIDI_MVP_Pkt_eagershort_send_t *const eagershort_pkt =
        &upkt.eagershort_send;
    struct iovec iov[MPL_IOV_LIMIT];
    MPIR_Request *sreq = *sreq_p;
    MPID_Seqnum_t seqnum = 0;

    PRINT_DEBUG(DEBUG_SHM_verbose,
                "sending contiguous short eager message to rank: %d, data_sz: "
                "%ld, sreq: %p\n",
                rank, data_sz, sreq);

    /* TODO: remove this comment */
    /* MPIDI_Comm_get_vc_set_active(comm, rank, &vc); */
    av = MPIDIU_comm_rank_to_av(comm, rank);
    vc = MPIDI_MVP_VC(av);

    mpi_errno =
        mvp_smp_fast_write_contig(vc, buf, data_sz, rank, tag, comm,
                                  context_offset, sreq_p, vc->smp.local_nodes);
    if (sreq) {
        MPIR_Object_set_ref(sreq, 1);
        MPIR_cc_set(&sreq->cc, 0);
    }
    /* Message was queued. Will resend later */
    mpi_errno = MPI_SUCCESS;
    sreq = *sreq_p;
    if (sreq != NULL) {
        /*printf( "Surprise, did not complete send of eagershort (starting
          connection?)\n" ); fflush(stdout); */
        /* MT FIXME setting fields in the request after it has been given to the
         * progress engine is racy.  The start call above is protected by
         * vc CS, but the progress engine is protected by MPIDCOMM.  So
         * we can't just extend the CS type below this point... what's the fix?
         */
        PRINT_DEBUG(DEBUG_SHM_verbose > 1,
                    "eager send to %d delayed, request enqueued: %p\n", rank,
                    sreq);
        MPIDI_Request_set_seqnum(sreq, seqnum);
        MPIDI_Request_set_type(sreq, MPIDI_REQUEST_TYPE_SEND);
    }
    return mpi_errno;
}

int MPIDI_MVP_smp_eager_send(MPIDI_MVP_ep_t *vc, const void *buf,
                             MPI_Aint count, MPI_Datatype datatype,
                             intptr_t data_sz, int dt_contig,
                             MPI_Aint dt_true_lb, int rank, int tag,
                             MPIR_Comm *comm, int context_offset,
                             MPIR_Request **sreq_p)
{
    int mpi_errno = MPI_SUCCESS;

    MPIR_Assert(!vc->force_rndv);
    MPIR_Assert(data_sz + sizeof(MPIDI_MVP_Pkt_eager_send_t) <
                MVP_SMP_EAGERSIZE);
    /* FIXME: flow control: limit number of outstanding eager messages
       containing data and need to be buffered by the receiver */

    if ((data_sz + sizeof(MPIDI_MVP_Pkt_eager_send_t) <=
         vc->eager_fast_max_msg_sz) &&
        vc->use_eager_fast_fn && dt_contig) {
        mpi_errno = MPIDI_MVP_EagerContigShortSend(
            sreq_p, MPIDI_MVP_PKT_EAGERSHORT_SEND, buf + dt_true_lb, data_sz,
            rank, tag, comm, context_offset);
        MPIR_ERR_CHECK(mpi_errno);
    } else if (dt_contig) {
        mpi_errno = MPIDI_MVP_EagerContigSend(sreq_p, MPIDI_MVP_PKT_EAGER_SEND,
                                              buf + dt_true_lb, data_sz, rank,
                                              tag, comm, context_offset);
        MPIR_ERR_CHECK(mpi_errno);
    } else {
        mpi_errno = MPIDI_MVP_EagerNonContigSend(
            sreq_p, MPIDI_MVP_PKT_EAGER_SEND, buf, count, datatype,
            data_sz, rank, tag, comm, context_offset);
        MPIR_ERR_CHECK(mpi_errno);
    }
fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
