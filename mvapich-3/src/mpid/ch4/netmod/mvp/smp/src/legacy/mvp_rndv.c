/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda. For detailed
 * copyright and licensing information, please refer to the copyright file
 * COPYRIGHT in the top level MVAPICH directory.
 */

#include "mvp_smp_impl.h"
#include "mvp_vc.h"
#include "mvp_req_fields.h"
#include "mvp_req.h"

#include "mvp_rts.h"

int MPIDI_MVP_MRAIL_Finish_request(MPIR_Request *rreq);
int MPIDI_MVP_Handle_recv_req(MPIDI_MVP_ep_t *vc, MPIR_Request *rreq,
                              int *complete);
int MPIDI_MVP_Handle_send_req(MPIDI_MVP_ep_t *vc, MPIR_Request *sreq,
                              int *compete);

int rts_send = 0;
int cts_recv = 0;

int mvp_num_rails = 1;

#define MPIDI_MVP_RECV_REQ_IS_READ(rreq)                                       \
    (MVP_RNDV_PROTOCOL_RGET == MPIDI_MVP_REQUEST(rreq, mrail).protocol)

#define MPIDI_MVP_SendQ_empty(vc) (vc->ch.sendq_head == NULL)

#define MPIDI_MVP_SendQ_enqueue(vc, req)                                       \
    {                                                                          \
        MPIDI_MVP_REQUEST(req, dev).next = NULL;                               \
        if (vc->ch.sendq_tail != NULL) {                                       \
            MPIDI_MVP_REQUEST(vc->ch.sendq_tail, dev).next = req;              \
        } else {                                                               \
            vc->ch.sendq_head = req;                                           \
        }                                                                      \
        vc->ch.sendq_tail = req;                                               \
        /* Disable direct send */                                              \
        vc->use_eager_fast_fn = 0;                                             \
    }

#define ibv_error_abort(code, message)                                         \
    {                                                                          \
        if (errno) {                                                           \
            PRINT_ERROR_ERRNO("%s:%d: " message, errno, __FILE__, __LINE__);   \
        } else {                                                               \
            PRINT_ERROR("%s:%d: " message "\n", __FILE__, __LINE__);           \
        }                                                                      \
        fflush(stderr);                                                        \
        exit(code);                                                            \
    }
#define IBV_STATUS_ERR -4 /*  gen2 function status error */

#define MPIDI_MVP_MRAILI_RREQ_RNDV_FINISH(rreq)                                \
    {                                                                          \
        if (rreq != NULL) {                                                    \
            /*     if (rreq->mrail.d_entry != NULL) {                    */    \
            /*         dreg_unregister(rreq->mrail.d_entry); */                \
            /*         rreq->mrail.d_entry = NULL; */                          \
            /*     } */                                                        \
            MPIDI_MVP_MRAIL_FREE_RNDV_BUFFER(rreq);                            \
        }                                                                      \
        /*rreq->mrail.d_entry = NULL;                                    */    \
        MPIDI_MVP_REQUEST(rreq, mrail).protocol =                              \
            MRAILI_PROTOCOL_RENDEZVOUS_UNSPECIFIED;                            \
    }

int MPIDI_MVP_Rendezvous_rput_finish(MPIDI_MVP_ep_t *vc,
                                     MPIDI_MVP_Pkt_rput_finish_t *rf_pkt)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Request *rreq;
    int complete;
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_RNDV_RPUT_FINISH);

    MPIR_Request_get_ptr(rf_pkt->receiver_req_id, rreq);
    PRINT_DEBUG(
        DEBUG_RNDV_verbose,
        "Received RPUT finish, rreq: %p, protocol: %d, local: %d, remote: %d\n",
        rreq, MPIDI_MVP_REQUEST(rreq, mrail).protocol,
        MPIDI_MVP_REQUEST(rreq, mrail).local_complete,
        MPIDI_MVP_REQUEST(rreq, mrail).remote_complete);

    {
        if (IS_VC_SMP(vc)) {
            MPIDI_MVP_REQUEST(rreq, mrail).remote_complete = UINT32_MAX;
        } /*else {
            MPIDI_MVP_REQUEST(rreq, mrail).remote_complete++;
            if (MPIDI_MVP_REQUEST(rreq, mrail).remote_complete == mvp_num_rails)
        { MPIDI_MVP_REQUEST(rreq, mrail).remote_complete = UINT32_MAX;
            }
        }*/
        if (!MPIDI_MVP_MRAIL_Finish_request(rreq)) {
            return MPI_SUCCESS;
        }
    }

    if (MPIDI_MVP_REQUEST(rreq, mrail).rndv_buf_alloc == 1) {
        //        MPIDI_MVP_Rendezvous_unpack_data(vc, rreq);
        PRINT_ERROR("rndv_buf_alloc == 1 not supported currently \n");
    } else {
        MPIDI_MVP_REQUEST(rreq, mrail).rndv_buf = NULL;
    }

#if defined(CKPT)
    MPIDI_MVP_CR_req_dequeue(rreq);
#endif /* defined(CKPT) */

    if (MPIDI_MVP_REQUEST(rreq, mrail).remote_addr == NULL) {
        MPIDI_MVP_MRAILI_RREQ_RNDV_FINISH(rreq);
    }

    mpi_errno = MPIDI_MVP_Handle_recv_req(vc, rreq, &complete);
    if (mpi_errno != MPI_SUCCESS) {
        mpi_errno =
            MPIR_Err_create_code(mpi_errno, MPIR_ERR_RECOVERABLE, __func__,
                                 __LINE__, MPI_ERR_OTHER, "**fail", 0);
    }

    PRINT_DEBUG(DEBUG_RNDV_verbose, "rreq: %p, complete: %d\n", rreq, complete);
    if (complete) {
        vc->ch.recv_active = NULL;
    } else {
        mpi_errno = MPIR_Err_create_code(mpi_errno, MPIR_ERR_FATAL, __func__,
                                         __LINE__, MPI_ERR_OTHER, "**fail", 0);
        goto fn_exit;
    }

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_RNDV_RPUT_FINISH);
    return mpi_errno;
}

int MPIDI_MVP_Rendezvous_rget_send_finish(MPIDI_MVP_ep_t *vc,
                                          MPIDI_MVP_Pkt_rget_finish_t *rget_pkt)
{
    int mpi_errno = MPI_SUCCESS;
    int complete;
    MPIR_Request *sreq;
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_RNDV_RGET_SEND_FINISH);

    MPIR_Request_get_ptr(rget_pkt->sender_req_id, sreq);
    PRINT_DEBUG(
        DEBUG_RNDV_verbose,
        "Received RGET finish, sreq: %p, protocol: %d, local: %d, remote: %d\n",
        sreq, MPIDI_MVP_REQUEST(sreq, mrail).protocol,
        MPIDI_MVP_REQUEST(sreq, mrail).local_complete,
        MPIDI_MVP_REQUEST(sreq, mrail).remote_complete);

    MPIDI_MVP_REQUEST(sreq, mrail).remote_complete = UINT32_MAX;
    if (!MPIDI_MVP_MRAIL_Finish_request(sreq)) {
        return MPI_SUCCESS;
    }

    MPIDI_MVP_MRAILI_RREQ_RNDV_FINISH(sreq);

#if 0
#ifdef _ENABLE_HSAM_
    if(mvp_MPIDI_MVP_RDMA_Process.has_hsam && 
            ((req->mrail.rndv_buf_sz > rdma_large_msg_rail_sharing_threshold))) {

        /* Adjust the weights of different paths according to the
         * timings obtained for the stripes */

        adjust_weights(v->vc, req->mrail.stripe_start_time,
                req->mrail.stripe_finish_time, req->mrail.initial_weight);
    }
#endif /*_ENABLE_HSAM_*/
#endif

    MPIDI_MVP_Handle_send_req(vc, sreq, &complete);

    if (complete != TRUE) {
        mpi_errno = MPIR_Err_create_code(mpi_errno, MPIR_ERR_FATAL, __func__,
                                         __LINE__, MPI_ERR_OTHER, "**fail", 0);
        goto fn_exit;
    }

#if defined(CKPT)
    MPIDI_MVP_CR_req_dequeue(sreq);
#endif /* defined(CKPT) */

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_RNDV_RGET_SEND_FINISH);
    return mpi_errno;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_MVP_iStartRndvMsg(MPIDI_MVP_ep_t *vc,
                                                     MPIR_Request *sreq,
                                                     MPIDI_MVP_Pkt_t *rts_pkt)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_ISTARTRNDVMSG);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_ISTARTRNDVMSG);

    ++rts_send;
    if (MPIDI_MVP_SendQ_empty(vc)) { /* MT */
        MPIR_Request *send_req;

        MPIDI_MVP_Pkt_rndv_req_to_send_t *rndv_pkt =
            &(rts_pkt->rndv_req_to_send);
        /* MT - need some signalling to lock down our right to use the
           channel, thus insuring that the progress engine does also try to
           write */
        MPIDI_MVP_MRAIL_Prepare_rndv(vc, sreq);
        MPIDI_MVP_MRAIL_SET_PKT_RNDV(rndv_pkt, sreq);

        /* TODO: this seems to be running correctly but the reciever gets a
         * 0/overly high protocol */
        PRINT_DEBUG(DEBUG_RNDV_verbose > 1,
                    "Sending RTS to: %d, sreq: %p, protocol: %d, buf: %p, "
                    "rndv_buf_alloc: %d,"
                    " ondatavail %p, onfinal %p msg_offset %d\n",
                    vc->pg_rank, sreq, MPIDI_MVP_REQUEST(sreq, mrail).protocol,
                    MPIDI_MVP_REQUEST(sreq, mrail).rndv_buf,
                    MPIDI_MVP_REQUEST(sreq, mrail).rndv_buf_alloc,
                    MPIDI_MVP_REQUEST(sreq, dev).OnDataAvail,
                    MPIDI_MVP_REQUEST(sreq, dev).OnFinal,
                    MPIDI_MVP_REQUEST(sreq, dev).msg_offset);

        if ((mpi_errno = MPIDI_MVP_iStartMsg(
                 vc, rndv_pkt, sizeof(MPIDI_MVP_Pkt_rndv_req_to_send_t),
                 &send_req)) != MPI_SUCCESS) {
            MPIR_Object_set_ref(sreq, 0);
            MPIDI_MVP_Request_free(sreq);
            sreq = NULL;
            MPIR_ERR_POP(mpi_errno);
        }

        if (send_req != NULL) {
            MPIDI_MVP_Request_free(send_req);
        }
    } else {
        PRINT_DEBUG(DEBUG_RNDV_verbose > 1,
                    "Enqueuing RNDV msg to rank %d, sreq: %p\n", vc->pg_rank,
                    sreq);
        MPIDI_MVP_SendQ_enqueue(vc, sreq);
    }

fn_exit:
    //#ifdef CKPT
    //    MPIDI_CH3I_CR_unlock();
    //#endif
    // DEBUG_PRINT("[send rts]successful complete\n");
    // MPL_DBG_MSG_S(MPIDI_MVP_DBG_CHANNEL,VERBOSE,"exiting %s",__func__);
    MPIR_FUNC_TERSE_EXIT(MPID_STATE_MPIDI_MVP_ISTARTRNDVMSG);
    return mpi_errno;

fn_fail:
    goto fn_exit;
}

/* MPID_MVP_RndvSend - Send a request to perform a rendezvous send */
int MPIDI_MVP_RndvSend(MPIR_Request **sreq_p, const void *buf, MPI_Aint count,
                       MPI_Datatype datatype, int dt_contig, intptr_t data_sz,
                       MPI_Aint dt_true_lb, int rank, int tag, MPIR_Comm *comm,
                       int context_offset)
{
    MPIDI_MVP_Pkt_t upkt;
    MPIDI_MVP_Pkt_rndv_req_to_send_t *const rts_pkt = &upkt.rndv_req_to_send;
    MPIDI_av_entry_t *av = NULL;
    MPIDI_MVP_ep_t *vc;
    MPIR_Request *sreq = *sreq_p;
    int mpi_errno = MPI_SUCCESS;
    MPID_Seqnum_t seqnum;
    /* For non-contig */
    void *tmp_buf = NULL;
    MPI_Aint position = 0;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_RNDVSEND);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_RNDVSEND);

    PRINT_DEBUG(DEBUG_RNDV_verbose,
                "Rndv Send to rank: %d, tag: %d, context: %d, buf: %p, size: "
                "%ld, contig: %d\n",
                rank, tag, comm->context_id + context_offset, buf, data_sz,
                dt_contig);
    // MPL_DBG_MSG_D(MPIDI_MVP_DBG_OTHER,VERBOSE,
    //        "sending rndv RTS, data_sz=" PRIdPTR, data_sz);

    MPIDI_MVP_REQUEST(sreq, dev).partner_request = NULL;

    MPIDI_Pkt_init(rts_pkt, MPIDI_MVP_PKT_RNDV_REQ_TO_SEND);
    rts_pkt->match.parts.rank = comm->rank;
    rts_pkt->match.parts.tag = tag;
    rts_pkt->match.parts.context_id = comm->context_id + context_offset;
    rts_pkt->sender_req_id = sreq->handle;
    rts_pkt->data_sz = data_sz;
    av = MPIDIU_comm_rank_to_av(comm, rank);
    vc = MPIDI_MVP_VC(av);
    MPIDI_VC_FAI_send_seqnum(vc, seqnum);
    MPIDI_Pkt_set_seqnum(rts_pkt, seqnum);
    MPIDI_Request_set_seqnum(sreq, seqnum);

    // MPL_DBG_MSGPKT(vc,tag,rts_pkt->match.parts.context_id,rank,data_sz,"Rndv");

    if (dt_contig) {
        // MPL_DBG_MSG_D(MPIDI_MVP_DBG_OTHER,VERBOSE,"  contiguous rndv data,
        // data_sz="
        //            PRIdPTR, data_sz);

        MPIDI_MVP_REQUEST(sreq, dev.OnDataAvail) = NULL;

        MPIDI_MVP_REQUEST(sreq, dev.iov[0].iov_base) =
            (void *)((char *)MPIDI_MVP_REQUEST(sreq, dev.user_buf) +
                     dt_true_lb);
        MPIDI_MVP_REQUEST(sreq, dev.iov[0].iov_len) = data_sz;
        MPIDI_MVP_REQUEST(sreq, dev.iov_count) = 1;
    } else {
        tmp_buf = MPL_malloc(data_sz, MPL_MEM_BUFFER);
        if (!tmp_buf) {
            mpi_errno = MPIR_Err_create_code(
                MPI_SUCCESS, MPIR_ERR_FATAL, __func__, __LINE__, MPI_ERR_OTHER,
                "**nomem", "**nomem %s %d", "tmp_buf", data_sz);
            MPIR_ERR_POP(mpi_errno);
        }

        MPIR_Typerep_pack(buf, count, datatype, 0, tmp_buf, data_sz, &position);

        /* Store it in req obj so we can free later after being sent
         * This is for non contig ddt only */
        MPIDI_MVP_REQUEST(sreq, dev.tmpbuf) = tmp_buf;

        MPIDI_MVP_REQUEST(sreq, dev.OnDataAvail) = 0;
        MPIDI_MVP_REQUEST(sreq, dev.iov[0].iov_base) = tmp_buf;
        MPIDI_MVP_REQUEST(sreq, dev.iov[0].iov_len) = data_sz;
        MPIDI_MVP_REQUEST(sreq, dev.iov_count) = 1;
    }
#if 0
    else
    {
        MPIDI_MVP_REQUEST(sreq, dev).iov_count = MPL_IOV_LIMIT;
        MPIDI_MVP_REQUEST(sreq, dev).msg_offset = 0;
        MPIDI_MVP_REQUEST(sreq, dev).msgsize = data_sz;
        /* One the initial load of a send iov req, set the OnFinal action (null
           for point-to-point) */
        MPIDI_MVP_REQUEST(sreq, dev).OnFinal = 0;
        mpi_errno = MPIDI_CH3U_Request_load_send_iov(sreq, &MPIDI_MVP_REQUEST(sreq, dev).iov[0],
                                 &MPIDI_MVP_REQUEST(sreq, dev).iov_count);
        /* Fallback to R3 if sender side is non-contiguous
         * and if the user has not forced RPUT. For Intra-node
         * we always switch to R3*/
        if(IS_VC_SMP(vc) || rdma_rndv_protocol != MVP_RNDV_PROTOCOL_RPUT) {
            MPIDI_MVP_REG_GET(sreq, mrail).protocol = MVP_RNDV_PROTOCOL_R3;
            MPIDI_CH3I_MVP_FREE_RNDV_BUFFER(sreq);
        }

        /* --BEGIN ERROR HANDLING-- */
        if (mpi_errno != MPI_SUCCESS)
        {
            mpi_errno = MPIR_Err_create_code(mpi_errno, MPIR_ERR_FATAL,
                             FCNAME, __LINE__, MPI_ERR_OTHER,
                             "**ch3|loadsendiov", 0);
            goto fn_exit;
        }
        /* --END ERROR HANDLING-- */
    }
#endif
    mpi_errno = MPIDI_MVP_iStartRndvMsg(vc, sreq, &upkt);
    /* --BEGIN ERROR HANDLING-- */
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_Object_set_ref(sreq, 0);
        MPIDI_MVP_Request_free(sreq);
        *sreq_p = NULL;
        mpi_errno =
            MPIR_Err_create_code(mpi_errno, MPIR_ERR_FATAL, __func__, __LINE__,
                                 MPI_ERR_OTHER, "**mvp_rtspkt", 0);
        MPIR_ERR_POP(mpi_errno);
    }
    /* --END ERROR HANDLING-- */

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_RNDVSEND);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIDI_MVP_iStartRndvTransfer(MPIDI_MVP_ep_t *vc, MPIR_Request *rreq)
{
    MPIDI_MVP_Pkt_t upkt;
    MPIDI_MVP_Pkt_rndv_clr_to_send_t *cts_pkt = &upkt.rndv_clr_to_send;
    MPIR_Request *cts_req;
    MPID_Seqnum_t seqnum;
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_ISTARTRNDVTRANSFER);

    //#ifdef CKPT
    //    MPIDI_CH3I_CR_lock();
    //#endif

    MPIDI_Pkt_init(cts_pkt, MPIDI_MVP_PKT_RNDV_CLR_TO_SEND);
    if (MPIDI_MVP_REQUEST(rreq, dev).iov_count == 1 &&
        MPIDI_MVP_REQUEST(rreq, dev).OnDataAvail == NULL)
        cts_pkt->recv_sz = MPIDI_MVP_REQUEST(rreq, dev).iov[0].iov_len;
    else
        cts_pkt->recv_sz = MPIDI_MVP_REQUEST(rreq, dev).msgsize;

    cts_pkt->sender_req_id = MPIDI_MVP_REQUEST(rreq, dev).sender_req_id;
    cts_pkt->receiver_req_id = rreq->handle;
    MPIDI_VC_FAI_send_seqnum(vc, seqnum);
    MPIDI_Pkt_set_seqnum(cts_pkt, seqnum);

    mpi_errno = MPIDI_MVP_Prepare_rndv_cts(vc, cts_pkt, rreq);

    if (mpi_errno != MPI_SUCCESS) {
        mpi_errno =
            MPIR_Err_create_code(mpi_errno, MPIR_ERR_FATAL, __func__, __LINE__,
                                 MPI_ERR_OTHER, "**ch3|ctspkt", 0);
        goto fn_exit;
    }

    mpi_errno = MPIDI_MVP_iStartMsg(vc, cts_pkt, sizeof(*cts_pkt), &cts_req);
    /* --BEGIN ERROR HANDLING-- */
    if (mpi_errno != MPI_SUCCESS) {
        mpi_errno =
            MPIR_Err_create_code(mpi_errno, MPIR_ERR_FATAL, __func__, __LINE__,
                                 MPI_ERR_OTHER, "**ch3|ctspkt", 0);
        goto fn_exit;
    }
    /* --END ERROR HANDLING-- */
    if (cts_req != NULL) {
        MPIDI_MVP_Request_free(cts_req);
    }

fn_exit:
    //#ifdef CKPT
    //    MPIDI_CH3I_CR_unlock();
    //#endif
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_ISTARTRNDVTRANSFER);
    return mpi_errno;
}

/*
 * This routine processes a rendezvous message once the message is matched.
 * It is used in mpid_recv and mpid_irecv.
 */
int MPIDI_MVP_RecvRndv(MPIDI_MVP_ep_t *vc, MPIR_Request *rreq)
{
    int mpi_errno = MPI_SUCCESS;
    // MPIDI_MVP_request_t rreq = MPID_MVP_REQ(_rreq);
    MPIDI_MVP_Pkt_rndv_req_to_send_t *rts_pkt;
    /* A rendezvous request-to-send (RTS) message has arrived.  We need
       to send a CTS message to the remote process. */

    /* The channel will be performing the rendezvous */

    if (IS_VC_SMP(vc)) {
        intptr_t buflen;
        MPIR_Assert(rreq != NULL);
        return MPIDI_MVP_PktHandler_SMP_RTS(
            vc, &(MPIDI_MVP_REQUEST(rreq, ch).pkt), &buflen, &rreq);
    }
    mpi_errno = 1;

    if (MPIDI_MVP_REQUEST(rreq, dev).recv_data_sz == 0) {
        MPID_Request_complete(rreq);
    } else {
        mpi_errno = MPIDI_MVP_Post_data_receive_found(rreq);
        if (mpi_errno != MPI_SUCCESS) {
            MPIR_ERR_SETANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**ch3|postrecv",
                                 "**ch3|postrecv %s",
                                 "MPIDI_MVP_PKT_RNDV_REQ_TO_SEND");
        }
    }

    if (MPIDI_MVP_RECV_REQ_IS_READ(rreq)) {
        MPIDI_MVP_Pkt_t *tmp_pkt =
            (MPIDI_MVP_Pkt_t *)&(MPIDI_MVP_REQUEST(rreq, ch).pkt);
        rts_pkt = &(tmp_pkt->rndv_req_to_send);

        mpi_errno = MPIDI_MVP_Prepare_rndv_get(vc, rreq);
        if (mpi_errno != MPI_SUCCESS) {
            MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER, "**ch3|rndv");
        }

        if (MVP_RNDV_PROTOCOL_RGET == MPIDI_MVP_REQUEST(rreq, mrail).protocol) {
            mpi_errno = MPIDI_MVP_Rndv_transfer(vc, NULL, rreq, NULL, rts_pkt);
            if (mpi_errno != MPI_SUCCESS && rreq != NULL) {
                MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER, "**ch3|senddata");
            }
            goto fn_exit;
        }
    }

    mpi_errno = MPIDI_MVP_iStartRndvTransfer(vc, rreq);

    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER, "**ch3|ctspkt");
    }

#if 0
    MPIR_Request *cts_req;
    MPIDI_MVP_Pkt_t upkt;
    MPIDI_MVP_Pkt_rndv_clr_to_send_t *cts_pkt = &upkt.rndv_clr_to_send;

    // MPL_DBG_MSG(MPIDI_MVP_DBG_OTHER,
    //             VERBOSE,
    //             "rndv RTS in the request, sending rndv CTS");

    MPIDI_Pkt_init(cts_pkt, MPIDI_MVP_PKT_RNDV_CLR_TO_SEND);
    cts_pkt->sender_req_id = MPIDI_MVP_REQUEST(rreq, dev).sender_req_id;
    cts_pkt->receiver_req_id = rreq->handle;
    MPID_THREAD_CS_ENTER(POBJ, vc->pobj_mutex);
    mpi_errno = MPIDI_MVP_iStartMsg(vc, cts_pkt, sizeof(*cts_pkt), &cts_req);
    MPID_THREAD_CS_EXIT(POBJ, vc->pobj_mutex);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER, "**ch3|ctspkt");
    }
    if (cts_req != NULL) {
        /* FIXME: Ideally we could specify that a req not be returned. */
        /*            This would avoid our having to decrement the */
        /*            reference count on a req we don't want/need. |)}># */
        MPIDI_MVP_Request_free(cts_req);
    }
#endif

fn_exit:
fn_fail:
    return mpi_errno;
}

void MRAILI_RDMA_Put_finish(MPIDI_MVP_ep_t *vc, MPIR_Request *sreq, int rail)
{
    MPIDI_MVP_Pkt_rput_finish_t rput_pkt;
    struct iovec iov;
    int n_iov = 1;
    int nb;
    MPID_Seqnum_t seqnum;
    int mpi_errno = MPI_SUCCESS;

    MPIDI_Pkt_init(&rput_pkt, MPIDI_MVP_PKT_RPUT_FINISH);
    /*
#ifdef _ENABLE_CUDA_
    if (mvp_enable_device) {
        rput_pkt.is_device = 0;
    }
#endif
*/
    rput_pkt.receiver_req_id = MPIDI_MVP_REQUEST(sreq, mrail).partner_id;
    MPIDI_VC_FAI_send_seqnum(vc, seqnum);
    MPIDI_Pkt_set_seqnum(&rput_pkt, seqnum);

    PRINT_DEBUG(DEBUG_RNDV_verbose > 1,
                "Sending RPUT FINISH to %d, sreq: %p, rreq: %08x, rail: %d\n",
                vc->pg_rank, sreq, MPIDI_MVP_REQUEST(sreq, mrail).partner_id,
                rail);

    if (IS_VC_SMP(vc)) {
        MPIR_Request *new_req = NULL;
        MPIDI_MVP_REQUEST(sreq, mrail).local_complete = UINT32_MAX;

        mpi_errno = MPIDI_MVP_SMP_iStartMsg(
            vc, &rput_pkt, sizeof(MPIDI_MVP_Pkt_rput_finish_t), &new_req);
        if (mpi_errno != MPI_SUCCESS) {
            ibv_error_abort(
                IBV_STATUS_ERR,
                "Failed sending rput finish through SMP channel \n");
        }

        if (new_req != NULL) {
            MPIDI_MVP_Request_free(new_req);
        }
    } else { /*
        iov.iov_base = &rput_pkt;
        iov.iov_len = sizeof(MPIDI_MVP_Pkt_rput_finish_t);

        int rc = MPIDI_CH3I_MRAILI_rput_complete(vc, &iov, n_iov, &nb, &buf,
        rail);

        if (rc != 0 && rc != MPI_MRAIL_MSG_QUEUED)
        {
            ibv_error_abort(IBV_STATUS_ERR,
                    "Cannot send rput through send/recv path");
        }

        buf->sreq = (void *) sreq;

        // mark MPI send complete when VIA send completes

        DEBUG_PRINT("VBUF ASSOCIATED: %p, %08x\n", buf, buf->desc.u.sr.wr_id);
   */
    }
}

/* Algorithm:
 * if (message size < striping threshold)
 *     mark as complete, independent of the rendezvous protocol
 *
 * if (rendezvous protocol == RDMA Read)
 *     only one finish is expected, mark as complete
 *
 * if (rendezvous protocol == RDMA Write)
 *     mvp_num_rails finish messages are expected
 *     check this condition and mark complete accordingly
 */

int MPIDI_MVP_MRAIL_Finish_request(MPIR_Request *rreq)
{
    PRINT_DEBUG(
        DEBUG_RNDV_verbose > 1,
        "rreq: %p, protocol: %d, local_complete: %d, remote_complete: %d\n",
        rreq, MPIDI_MVP_REQUEST(rreq, mrail).protocol,
        MPIDI_MVP_REQUEST(rreq, mrail).local_complete,
        MPIDI_MVP_REQUEST(rreq, mrail).remote_complete);

    MPIR_Assert(MPIDI_MVP_REQUEST(rreq, mrail).local_complete == UINT32_MAX ||
                MPIDI_MVP_REQUEST(rreq, mrail).local_complete <= mvp_num_rails);
    MPIR_Assert(MPIDI_MVP_REQUEST(rreq, mrail).remote_complete == UINT32_MAX ||
                MPIDI_MVP_REQUEST(rreq, mrail).remote_complete <=
                    mvp_num_rails);

    switch (MPIDI_MVP_REQUEST(rreq, mrail).protocol) {
        case MVP_RNDV_PROTOCOL_RGET:
        case MVP_RNDV_PROTOCOL_RPUT:
            return (
                MPIDI_MVP_REQUEST(rreq, mrail).local_complete == UINT32_MAX ||
                MPIDI_MVP_REQUEST(rreq, mrail).remote_complete == UINT32_MAX);
        default:
            break;
    }

    return 1;
}

int MPIDI_MVP_Rendezvous_rget_recv_finish(MPIDI_MVP_ep_t *vc,
                                          MPIR_Request *rreq)
{
    int mpi_errno = MPI_SUCCESS;
    int complete;
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_RGET_RECV_FINISH);

    if (!MPIDI_MVP_MRAIL_Finish_request(rreq)) {
        return MPI_SUCCESS;
    }

    if (MPIDI_MVP_REQUEST(rreq, mrail).rndv_buf_alloc == 1) {
        /* If we are using datatype, then need to unpack data from tmpbuf */
        int iter = 0;
        int copied = 0;
        uintptr_t buf = (uintptr_t)MPIDI_MVP_REQUEST(rreq, mrail).rndv_buf;

        for (; iter < MPIDI_MVP_REQUEST(rreq, dev).iov_count; ++iter) {
            MPIR_Memcpy(MPIDI_MVP_REQUEST(rreq, dev).iov[iter].iov_base,
                        (void *)buf,
                        MPIDI_MVP_REQUEST(rreq, dev).iov[iter].iov_len);
            buf += MPIDI_MVP_REQUEST(rreq, dev).iov[iter].iov_len;
            copied += MPIDI_MVP_REQUEST(rreq, dev).iov[iter].iov_len;
        }

        MPIDI_MVP_Request_adjust_iov(rreq, copied);

        // need to revisit
        while (MPIDI_MVP_REQUEST(rreq, dev).OnDataAvail != NULL)
        //        while (MPIDI_MVP_REQUEST(rreq, dev).OnDataAvail ==
        //        MPIDI_MVP_ReqHandler_UnpackSRBufReloadIOV
        //            || MPIDI_MVP_REQUEST(rreq, dev).OnDataAvail ==
        //            MPIDI_MVP_ReqHandler_ReloadIOV)
        {
            /* XXX: dev.ca should only be CA_COMPLETE? */
            /* end of XXX */
            mpi_errno = MPIDI_MVP_Handle_recv_req(vc, rreq, &complete);

            if (mpi_errno != MPI_SUCCESS || complete == TRUE) {
                mpi_errno =
                    MPIR_Err_create_code(mpi_errno, MPIR_ERR_FATAL, __func__,
                                         __LINE__, MPI_ERR_OTHER, "**fail", 0);
                goto fn_exit;
            }

            copied = 0;

            for (iter = 0; iter < MPIDI_MVP_REQUEST(rreq, dev).iov_count;
                 ++iter) {
                MPIR_Memcpy(MPIDI_MVP_REQUEST(rreq, dev).iov[iter].iov_base,
                            (void *)buf,
                            MPIDI_MVP_REQUEST(rreq, dev).iov[iter].iov_len);
                buf += MPIDI_MVP_REQUEST(rreq, dev).iov[iter].iov_len;
                copied += MPIDI_MVP_REQUEST(rreq, dev).iov[iter].iov_len;
            }

            MPIDI_MVP_Request_adjust_iov(rreq, copied);
        }
    } else {
        MPIDI_MVP_REQUEST(rreq, mrail).rndv_buf = NULL;
    }

    /*
#if defined(CKPT)
    MPIDI_CH3I_CR_req_dequeue(rreq);
#endif // defined(CKPT)
*/

    MPIDI_MVP_MRAILI_RREQ_RNDV_FINISH(rreq);

    mpi_errno = MPIDI_MVP_Handle_recv_req(vc, rreq, &complete);

    if (mpi_errno != MPI_SUCCESS) {
        mpi_errno =
            MPIR_Err_create_code(mpi_errno, MPIR_ERR_RECOVERABLE, __func__,
                                 __LINE__, MPI_ERR_OTHER, "**fail", 0);
    }

    if (complete) {
        vc->ch.recv_active = NULL;
    } else {
        mpi_errno = MPIR_Err_create_code(mpi_errno, MPIR_ERR_FATAL, __func__,
                                         __LINE__, MPI_ERR_OTHER, "**fail", 0);
        goto fn_exit;
    }

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_RGET_RECV_FINISH);
    return mpi_errno;
}

#define MPI_MRAIL_MSG_QUEUED (-1)
void MRAILI_RDMA_Get_finish(MPIDI_MVP_ep_t *vc, MPIR_Request *rreq, int rail)
{
    MPIDI_MVP_Pkt_rget_finish_t rget_pkt;
    struct iovec iov;
    int n_iov = 1;
    int nb;
    int mpi_errno = MPI_SUCCESS;
    MPID_Seqnum_t seqnum;

    MPIDI_Pkt_init(&rget_pkt, MPIDI_MVP_PKT_RGET_FINISH);
    rget_pkt.sender_req_id = MPIDI_MVP_REQUEST(rreq, dev).sender_req_id;
    MPIDI_VC_FAI_send_seqnum(vc, seqnum);
    MPIDI_Pkt_set_seqnum(&rget_pkt, seqnum);

    MPIDI_MVP_REQUEST(rreq, mrail).local_complete = UINT32_MAX;
    PRINT_DEBUG(DEBUG_RNDV_verbose > 1,
                "Sending RGET FINISH to %d, rreq: %p, sreq: %08x, rail: %d\n",
                vc->pg_rank, rreq, MPIDI_MVP_REQUEST(rreq, dev).sender_req_id,
                rail);

    if (IS_VC_SMP(vc)) {
        MPIR_Request *new_req = NULL;

        mpi_errno = MPIDI_MVP_SMP_iStartMsg(
            vc, &rget_pkt, sizeof(MPIDI_MVP_Pkt_rget_finish_t), &new_req);
        if (mpi_errno != MPI_SUCCESS) {
            ibv_error_abort(
                IBV_STATUS_ERR,
                "Failed sending rget finish through SMP channel \n");
        }

        if (new_req != NULL) {
            MPIDI_MVP_Request_free(new_req);
        }

        MPIDI_MVP_Rendezvous_rget_recv_finish(vc, rreq);
    } else {
        /*
        vbuf *buf;

        iov.iov_base = &rget_pkt;
        iov.iov_len = sizeof(MPIDI_MVP_Pkt_rget_finish_t);

        mpi_errno =
            MPIDI_CH3I_MRAILI_rget_finish(vc, &iov, n_iov, &nb, &buf, rail);
        if (mpi_errno != MPI_SUCCESS &&
                mpi_errno != MPI_MRAIL_MSG_QUEUED) {
            ibv_error_abort(IBV_STATUS_ERR,
                    "Cannot send rget finish through send/recv path");
        }

        MPIDI_CH3_Rendezvous_rget_recv_finish(vc, rreq);
        DEBUG_PRINT("VBUF ASSOCIATED: %p, %08x\n", buf, buf->desc.u.sr.wr_id);
        */
    }
}
