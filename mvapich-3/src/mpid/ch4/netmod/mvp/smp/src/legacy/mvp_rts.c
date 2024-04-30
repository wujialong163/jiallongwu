/* Copyright (c) 2001-2024, The Ohio State University. All rights
 * reserved.
 *
 * This file is part of the MVAPICH software package developed by the
 * team members of The Ohio State University's Network-Based Computing
 * Laboratory (NBCL), headed by Professor Dhabaleswar K. (DK) Panda.
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level MVAPICH directory.
 *
 */

#if !defined(MPIDI_IOV_DENSITY_MIN)
#define MPIDI_IOV_DENSITY_MIN (16 * 1024)
#endif

#include "mvp_smp_impl.h"
#include "mvp_vc.h"
#include "mvp_pkt.h"
#include "mvp_req_utils.h"
#include "mvp_rts.h"
#include "mvp_shmem.h"
#include "../mvp_smp_progress.h"
#include "mvp_smp_progress_utils.h"
#include "mvp_smp_params.h"

#include "mvp_tagm.h"
#include "mvp_req.h"

int rdma_intra_node_r3_threshold;
int rdma_inter_node_r3_threshold;
int rdma_r3_threshold;

#define DEBUG_PRINT(args...)
//do {                                                          \
//    int rank = MPIR_Process.rank;                             \
//    fprintf(stderr, "[%d][%s:%d] ", rank, __FILE__, __LINE__);\
//    fprintf(stderr, args);                                    \
//} while (0)

MPIDI_MVP_SRBuf_element_t *MPIDI_MVP_SRBuf_pool = NULL;

/* TODO: FIXME: Dummy int */
int rdma_num_hcas = 10;

MPIDI_MVP_ep_t *flowlist;

int MPIDI_MVP_Handle_send_req(MPIDI_MVP_ep_t *vc, MPIR_Request *sreq,
                              int *complete);
void MRAILI_RDMA_Put_finish(MPIDI_MVP_ep_t *vc, MPIR_Request *sreq, int rail);
void MRAILI_RDMA_Get_finish(MPIDI_MVP_ep_t *vc, MPIR_Request *sreq, int rail);

MPIR_Request *create_request(void *hdr, intptr_t hdr_sz, size_t nb)
{
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_CREATE_REQUEST);

    MPIR_Request *sreq = MPIR_Request_create(MPIR_REQUEST_KIND__SEND);
    /* --BEGIN ERROR HANDLING-- */
    if (sreq == NULL) {
        MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_CREATE_REQUEST);
        return NULL;
    }
    /* --END ERROR HANDLING-- */
    MPIR_Object_set_ref(sreq, 2);
    MVP_INC_NUM_POSTED_SEND();

    MPIDI_MVP_REQUEST_FROM_MPICH(sreq) =
        MPIDI_MVP_smp_request_create_from_pool();
    MPIR_Memcpy(&MPIDI_MVP_REQUEST(sreq, dev).pending_pkt, hdr,
                sizeof(MPIDI_MVP_Pkt_t));
    MPIDI_MVP_REQUEST(sreq, dev).iov[0].iov_base =
        (void *)((char *)&MPIDI_MVP_REQUEST(sreq, dev).pending_pkt + nb);
    MPIDI_MVP_REQUEST(sreq, ch).reqtype = REQUEST_NORMAL;
    MPIDI_MVP_REQUEST(sreq, dev).iov[0].iov_len = hdr_sz - nb;
    MPIDI_MVP_REQUEST(sreq, dev).iov_count = 1;
    MPIDI_MVP_REQUEST(sreq, dev).OnDataAvail = 0;

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_CREATE_REQUEST);
    return sreq;
}

static inline MPIR_Request *create_requestv(struct iovec *iov, int iov_count,
                                            int iov_offset, size_t nb)
{
    MPIR_Request *sreq;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_CREATE_REQUESTV);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_CREATE_REQUESTV);

    sreq = MPIR_Request_create(MPIR_REQUEST_KIND__SEND);
    /* --BEGIN ERROR HANDLING-- */
    if (sreq == NULL)
        return NULL;
    /* --END ERROR HANDLING-- */
    MPIR_Object_set_ref(sreq, 2);
    MVP_INC_NUM_POSTED_SEND();

    MPIDI_MVP_REQUEST_FROM_MPICH(sreq) =
        MPIDI_MVP_smp_request_create_from_pool();
    MPIR_Memcpy(MPIDI_MVP_REQUEST(sreq, dev.iov), iov,
                iov_count * sizeof(struct iovec));

    if (iov_offset == 0) {
        /*
            MPIR_Assert(iov[0].iov_len == sizeof(MPIDI_CH3_Pkt_t));
        */
        MPIR_Memcpy(&MPIDI_MVP_REQUEST(sreq, dev.pending_pkt), iov[0].iov_base,
                    sizeof(MPIDI_MVP_Pkt_t));
        MPIDI_MVP_REQUEST(sreq, dev.iov[0].iov_base) =
            (void *)&MPIDI_MVP_REQUEST(sreq, dev.pending_pkt);
    }
    MPIDI_MVP_REQUEST(sreq, ch.reqtype) = REQUEST_NORMAL;
    MPIDI_MVP_REQUEST(sreq, dev.iov[iov_offset].iov_base) =
        (void *)((char *)MPIDI_MVP_REQUEST(sreq, dev.iov[iov_offset].iov_base) +
                 nb);
    MPIDI_MVP_REQUEST(sreq, dev.iov[iov_offset].iov_len) -= nb;
    MPIDI_MVP_REQUEST(sreq, dev.iov_offset) = iov_offset;
    MPIDI_MVP_REQUEST(sreq, dev.iov_count) = iov_count;
    MPIDI_MVP_REQUEST(sreq, dev.OnDataAvail) = 0;

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_CREATE_REQUESTV);
    return sreq;
}

static inline int mvp_is_dt_contig(MPIR_Request *req)
{
    int dt_contig = 1;
    if (MPIDI_MVP_REQUEST(req, dev).datatype &&
        HANDLE_GET_KIND(MPIDI_MVP_REQUEST(req, dev).datatype) !=
            HANDLE_KIND_BUILTIN) {
        MPIR_Datatype *dt_ptr = NULL;
        MPIR_Datatype_get_ptr(MPIDI_MVP_REQUEST(req, dev).datatype, dt_ptr);
        dt_contig = dt_ptr->is_contig;
    }
    return dt_contig;
}

void mvp_select_rndv_protocol(MPIDI_MVP_ep_t *vc, MPIR_Request *rreq,
                              MPIDI_MVP_Pkt_rndv_req_to_send_t *rts_pkt)
{
    int protocol = rts_pkt->rndv.protocol;
    int dt_contig = mvp_is_dt_contig(rreq);

    /*
     * TODO: add this bback once we support R3 again. Or once we determine why
     * we are seeing an iov count > 1 when it should not be because we are
     * packing the iovs into one
     */
#if 0
    if (protocol == MVP_RNDV_PROTOCOL_R3 ||
        MPIDI_MVP_REQUEST(rreq, dev.iov_count) > 1) {
        protocol = MVP_RNDV_PROTOCOL_R3;
    }
#endif
    MPIDI_MVP_REQUEST(rreq, mrail.protocol) = rts_pkt->rndv.protocol = protocol;
}

int MPIDI_MVP_PktHandler_RndvReqToSend(MPIDI_MVP_ep_t *vc, MPIDI_MVP_Pkt_t *pkt,
                                       void *data ATTRIBUTE((unused)),
                                       intptr_t *buflen, MPIR_Request **rreqp)
{
    MPIR_Request *rreq;
    int found;
    MPIDI_MVP_Pkt_rndv_req_to_send_t *rts_pkt = &pkt->rndv_req_to_send;
    int mpi_errno = MPI_SUCCESS;

    PRINT_DEBUG(DEBUG_RNDV_verbose,
                "received RTS, sreq: %08x, rank: %d, tag: %d, context: %d, "
                "protocol: %d, data_sz: %ld\n",
                rts_pkt->sender_req_id, rts_pkt->match.parts.rank,
                rts_pkt->match.parts.tag, rts_pkt->match.parts.context_id,
                rts_pkt->rndv.protocol, rts_pkt->data_sz);

    if (IS_VC_SMP(vc)) {
        return MPIDI_MVP_PktHandler_SMP_RTS(vc, pkt, buflen, rreqp);
    }

#if 0

    //MPL_DBG_MSG_FMT(MPIDI_MVP_DBG_OTHER,VERBOSE,(MPL_DBG_FDEST,
 "received rndv RTS pkt, sreq=0x%08x, rank=%d, tag=%d, context=%d, data_sz=%" PRIdPTR,
          rts_pkt->sender_req_id, rts_pkt->match.parts.rank, 
                    rts_pkt->match.parts.tag, 
              rts_pkt->match.parts.context_id, rts_pkt->data_sz));
    //MPL_DBG_MSGPKT(vc,rts_pkt->match.parts.tag,rts_pkt->match.parts.context_id,
            rts_pkt->match.parts.rank,rts_pkt->data_sz,
            "ReceivedRndv");

    MPID_THREAD_CS_ENTER(POBJ, MPIR_THREAD_POBJ_MSGQ_MUTEX);
    rreq = MPIDI_MVP_Recvq_FDP_or_AEU(&rts_pkt->match, &found);
#if defined(CHANNEL_MRAIL)
    if (!found && mvp_smp_init && vc->smp.local_nodes >= 0) {
        MVP_INC_NUM_POSTED_RECV();
    }
#endif
    MPIR_ERR_CHKANDJUMP1(!rreq, mpi_errno,MPI_ERR_OTHER, "**nomemreq", "**nomemuereq %d", MPIDI_MVP_Recvq_count_unexp());

    /* If the completion counter is 0, that means that the communicator to
     * which this message is being sent has been revoked and we shouldn't
     * bother finishing this. */
    if (!found && MPIR_cc_get(rreq->cc) == 0) {
        *rreqp = NULL;
        goto fn_fail;
    }
    
    set_request_info(rreq, rts_pkt, MPIDI_REQUEST_RNDV_MSG);

    MPID_THREAD_CS_EXIT(POBJ, MPIR_THREAD_POBJ_MSGQ_MUTEX);

    *buflen = 0;
    MPIDI_MVP_Append_pkt_size();
#if defined(CHANNEL_MRAIL)
    MPIDI_MVP_RNDV_SET_REQ_INFO(rreq, rts_pkt);
#endif /* defined(CHANNEL_MRAIL) */
    
    if (found)
    {
        MPIR_Request * cts_req;
        MPIDI_MVP_Pkt_t upkt;
        MPIDI_MVP_Pkt_rndv_clr_to_send_t * cts_pkt = &upkt.rndv_clr_to_send;
#if defined(CHANNEL_MRAIL) && defined(MPID_USE_SEQUENCE_NUMBERS)
        MPID_Seqnum_t seqnum;
#endif /* defined(CHANNEL_MRAIL) && defined(MPID_USE_SEQUENCE_NUMBERS) */
    
        //MPL_DBG_MSG(MPIDI_MVP_DBG_OTHER,VERBOSE,"posted request found");
        MPIR_T_PVAR_COUNTER_INC(MVP, expected_recvs_rendezvous, 1);
#if defined(CHANNEL_MRAIL)
        if(MPIDI_MVP_RNDV_PROTOCOL_IS_READ(rts_pkt)) {

            mpi_errno = MPIDI_MVP_Post_data_receive_found(rreq);
            /* --BEGIN ERROR HANDLING-- */
            if (mpi_errno != MPI_SUCCESS && rreq != NULL) {
                MPIR_ERR_SETANDJUMP(mpi_errno,
                        MPI_ERR_OTHER,"**ch3|loadsendiov");
            }

            mpi_errno = MPIDI_MVP_Prepare_rndv_get(vc, rreq);
            if (mpi_errno != MPI_SUCCESS) {
                MPIR_ERR_SETANDJUMP(mpi_errno,MPI_ERR_OTHER,"**ch3|rndv");
            }

            if (MVP_RNDV_PROTOCOL_RGET == rreq->mrail.protocol) {
                mpi_errno = MPIDI_MVP_Rndv_transfer(vc,
                        NULL, rreq, NULL, rts_pkt);
                if (mpi_errno != MPI_SUCCESS && rreq != NULL) {
                    MPIR_ERR_SETANDJUMP(mpi_errno,
                            MPI_ERR_OTHER,"**ch3|senddata");
                }

                *rreqp = NULL;
                goto fn_exit;
            }
            /*else send back CTS with R3 protocol and fallback*/
        }
#endif /* defined(CHANNEL_MRAIL) */
    
    /* FIXME: What if the receive user buffer is not big enough to
       hold the data about to be cleared for sending? */
    
    //MPL_DBG_MSG(MPIDI_MVP_DBG_OTHER,VERBOSE,"sending rndv CTS packet");
    MPIDI_Pkt_init(cts_pkt, MPIDI_MVP_PKT_RNDV_CLR_TO_SEND);
    cts_pkt->sender_req_id = rts_pkt->sender_req_id;
    cts_pkt->receiver_req_id = rreq->handle;

        MPIDI_VC_FAI_send_seqnum(vc, seqnum);
        MPIDI_Pkt_set_seqnum(cts_pkt, seqnum);
        mpi_errno = MPIDI_MVP_Post_data_receive_found(rreq);
        /* --BEGIN ERROR HANDLING-- */
        if (mpi_errno != MPI_SUCCESS && rreq != NULL)
        {
            MPIR_ERR_SETANDJUMP(mpi_errno,MPI_ERR_OTHER,"**ch3|loadsendiov");
        }

        if (rreq->dev.OnDataAvail == NULL)
        {
            cts_pkt->recv_sz = rreq->dev.iov[0].iov_len;
                if (rreq->dev.iov_count > 1)
                {
                    int k = 1;
                    for (; k < rreq->dev.iov_count; ++k)
                    {
                        cts_pkt->recv_sz += rreq->dev.iov[k].iov_len;
                    }
                }
        }
        else
        {
            cts_pkt->recv_sz = rreq->dev.msgsize;
        }


        {
            mpi_errno = MPIDI_MVP_Prepare_rndv_cts(vc, cts_pkt, rreq);
        }
        if (mpi_errno != MPI_SUCCESS)
        {
            MPIR_ERR_SETANDJUMP(mpi_errno,MPI_ERR_OTHER,"**ch3|rndv");
        }

        MPID_THREAD_CS_ENTER(POBJ, vc->pobj_mutex);
    mpi_errno = MPIDI_MVP_iStartMsg(vc, cts_pkt, sizeof(*cts_pkt), &cts_req);
        MPID_THREAD_CS_EXIT(POBJ, vc->pobj_mutex);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_SETANDJUMP(mpi_errno,MPI_ERR_OTHER,
                "**ch3|ctspkt");
    }
    if (cts_req != NULL) {
        MPIDI_MVP_Request_free(cts_req);
    }
    }
    else
    {
        //MPL_DBG_MSG(MPIDI_MVP_DBG_OTHER,VERBOSE,"unexpected request allocated");
#if defined(CHANNEL_MRAIL)
        /* If the request is a read and is unexpected,
         * we have to buffer the remote information till
         * this request is matched and then processed */
        if(MPIDI_MVP_RNDV_PROTOCOL_IS_READ(rts_pkt)) {
#if (_ENABLE_CUDA_)
            /*If this is cuda is enabled, the pkt size is large, allocate 
              a buffer and then copy the rts packet*/
            rreq->dev.pending_pkt = MPIR_Malloc(sizeof(MPIDI_MVP_Pkt_rndv_req_to_send_t));
            MPIR_Memcpy(rreq->dev.pending_pkt, rts_pkt, sizeof(MPIDI_MVP_Pkt_rndv_req_to_send_t));
#else
            MPIR_Memcpy(&rreq->ch.pkt, pkt, sizeof(MPIDI_MVP_Pkt_t));
#endif
        }
#endif /* defined(CHANNEL_MRAIL) */

        /*
         * A MPID_Probe() may be waiting for the request we just 
         * inserted, so we need to tell the progress engine to exit.
         *
         * FIXME: This will cause MPID_Progress_wait() to return to the
         * MPI layer each time an unexpected RTS packet is
         * received.  MPID_Probe() should atomically increment a
         * counter and MPIDI_MVP_Progress_signal_completion()
         * should only be called if that counter is greater than zero.
         */
        MPIDI_MVP_Progress_signal_completion();
    }
    
    *rreqp = NULL;
#endif

fn_exit:
fn_fail:
    return mpi_errno;
}

/*
 * MPIDI_MVP_Request_load_recv_iov()
 *
 * Fill the request's IOV with the next (or remaining) portion of data
 * described by the segment (also contained in the request
 * structure).  If the density of IOV is not sufficient, allocate a
 * send/receive buffer and point the IOV at the buffer.
 */
int MPIDI_MVP_Request_load_recv_iov(MPIR_Request *const rreq)
{
    MPI_Aint last;
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_REQUEST_LOAD_RECV_IOV);

    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_REQUEST_LOAD_RECV_IOV);

    /* FIXME: MPICH 3.2 introduces orig_segment_first for STREAMED RMA ops,
     *        which causes issues for some regular datatype codepaths.
     *        This is a workaround until MPICH fixes the issue
     *        In MPICH 3.4.2 this is renamed to orig_msg_offset*/
    if (!(MPIDI_MVP_REQUEST(rreq, dev).pkt_flags &
          MPIDI_MVP_PKT_FLAG_RMA_STREAM)) {
        MPIDI_MVP_REQUEST(rreq, dev).orig_msg_offset = 0;
    } else {
        if (MPIDI_MVP_REQUEST(rreq, dev).orig_msg_offset ==
            MPIDI_LOAD_RECV_IOV_ORIG_MSG_OFFSET_UNSET) {
            MPIDI_MVP_REQUEST(rreq, dev).orig_msg_offset =
                MPIDI_MVP_REQUEST(rreq, dev).msg_offset;
        }
    }

    if (MPIDI_MVP_REQUEST(rreq, dev).msg_offset <
        MPIDI_MVP_REQUEST(rreq, dev).msgsize) {
        /* still reading data that needs to go into the user buffer */

        if (MPIDI_Request_get_type(rreq) != MPIDI_REQUEST_TYPE_ACCUM_RECV &&
            MPIDI_Request_get_type(rreq) != MPIDI_REQUEST_TYPE_GET_ACCUM_RECV &&
            MPIDI_Request_get_srbuf_flag(rreq)) {
            intptr_t data_sz;
            intptr_t tmpbuf_sz;

            /* Once a SRBuf is in use, we continue to use it since a small
               amount of data may already be present at the beginning
               of the buffer.  This data is left over from the previous unpack,
               most like a result of alignment issues.  NOTE: we
               could force the use of the SRBuf only
               when (MPIDI_MVP_REQUEST(rreq, dev).tmpbuf_off > 0)... */

            data_sz = MPIDI_MVP_REQUEST(rreq, dev).msgsize -
                      MPIDI_MVP_REQUEST(rreq, dev).msg_offset -
                      MPIDI_MVP_REQUEST(rreq, dev).tmpbuf_off;
            MPIR_Assert(data_sz > 0);
            tmpbuf_sz = MPIDI_MVP_REQUEST(rreq, dev).tmpbuf_sz -
                        MPIDI_MVP_REQUEST(rreq, dev).tmpbuf_off;
            if (data_sz > tmpbuf_sz) {
                data_sz = tmpbuf_sz;
            }
            MPIDI_MVP_REQUEST(rreq, dev).iov[0].iov_base =
                (void *)((char *)MPIDI_MVP_REQUEST(rreq, dev).tmpbuf +
                         MPIDI_MVP_REQUEST(rreq, dev).tmpbuf_off);
            MPIDI_MVP_REQUEST(rreq, dev).iov[0].iov_len = data_sz;
            MPIDI_MVP_REQUEST(rreq, dev).iov_offset = 0;
            MPIDI_MVP_REQUEST(rreq, dev).iov_count = 1;
            MPIR_Assert(MPIDI_MVP_REQUEST(rreq, dev).msg_offset -
                            MPIDI_MVP_REQUEST(rreq, dev).orig_msg_offset +
                            data_sz + MPIDI_MVP_REQUEST(rreq, dev).tmpbuf_off <=
                        MPIDI_MVP_REQUEST(rreq, dev).recv_data_sz);
            if (MPIDI_MVP_REQUEST(rreq, dev).msg_offset -
                    MPIDI_MVP_REQUEST(rreq, dev).orig_msg_offset + data_sz +
                    MPIDI_MVP_REQUEST(rreq, dev).tmpbuf_off ==
                MPIDI_MVP_REQUEST(rreq, dev).recv_data_sz) {
                /*MPL_DBG_MSG(MPIDI_MVP_DBG_CHANNEL,VERBOSE,
                  "updating rreq to read the remaining data into the SRBuf");*/
                // MPIDI_MVP_REQUEST(rreq, dev).OnDataAvail =
                // MPIDI_MVP_ReqHandler_UnpackSRBufComplete;
                MPIDI_MVP_REQUEST(rreq, dev).orig_msg_offset =
                    MPIDI_LOAD_RECV_IOV_ORIG_MSG_OFFSET_UNSET;
            } else {
                /*
                MPL_DBG_MSG(MPIDI_MVP_DBG_CHANNEL,VERBOSE,
                       "updating rreq to read more data into the SRBuf");
                MPIDI_MVP_REQUEST(rreq, dev).OnDataAvail =
                MPIDI_MVP_ReqHandler_UnpackSRBufReloadIOV;*/
            }
            goto fn_exit;
        }

        last = MPIDI_MVP_REQUEST(rreq, dev).msgsize;
        MPIDI_MVP_REQUEST(rreq, dev).iov_count = MPL_IOV_LIMIT;
        MPIDI_MVP_REQUEST(rreq, dev).iov_offset = 0;
        /*MPL_DBG_MSG_FMT(MPIDI_MVP_DBG_CHANNEL,VERBOSE,(MPL_DBG_FDEST,
       "pre-upv: first=%" PRIdPTR ", last=%" PRIdPTR ", iov_n=%d",
                  MPIDI_MVP_REQUEST(rreq, dev).msg_offset, last,
       MPIDI_MVP_REQUEST(rreq, dev).iov_count));*/
        MPIR_Assert(MPIDI_MVP_REQUEST(rreq, dev).msg_offset < last);
        MPIR_Assert(last > 0);

        MPI_Aint actual_iov_bytes, actual_iov_len;
        MPIR_Typerep_to_iov(MPIDI_MVP_REQUEST(rreq, dev).user_buf,
                            MPIDI_MVP_REQUEST(rreq, dev).user_count,
                            MPIDI_MVP_REQUEST(rreq, dev).datatype,
                            MPIDI_MVP_REQUEST(rreq, dev).msg_offset,
                            &MPIDI_MVP_REQUEST(rreq, dev).iov[0], MPL_IOV_LIMIT,
                            MPIDI_MVP_REQUEST(rreq, dev).msgsize -
                                MPIDI_MVP_REQUEST(rreq, dev).msg_offset,
                            &actual_iov_len, &actual_iov_bytes);
        MPIDI_MVP_REQUEST(rreq, dev).iov_count = (int)actual_iov_len;
        last = MPIDI_MVP_REQUEST(rreq, dev).msg_offset + actual_iov_bytes;

        /*MPL_DBG_MSG_FMT(MPIDI_MVP_DBG_CHANNEL,VERBOSE,(MPL_DBG_FDEST,
       "post-upv: first=%" PRIdPTR ", last=%" PRIdPTR ", iov_n=%d,
       iov_offset=%lld", MPIDI_MVP_REQUEST(rreq, dev).msg_offset, last,
       MPIDI_MVP_REQUEST(rreq, dev).iov_count, (long
       long)MPIDI_MVP_REQUEST(rreq, dev).iov_offset));*/
        MPIR_Assert(MPIDI_MVP_REQUEST(rreq, dev).iov_count >= 0 &&
                    MPIDI_MVP_REQUEST(rreq, dev).iov_count <= MPL_IOV_LIMIT);

        /* --BEGIN ERROR HANDLING-- */
        if (MPIDI_MVP_REQUEST(rreq, dev).iov_count == 0) {
            /* If the data can't be unpacked, the we have a mismatch between
               the datatype and the amount of data received.  Adjust
               the segment info so that the remaining data is received and
               thrown away. */
            rreq->status.MPI_ERROR = MPIR_Err_create_code(
                MPI_SUCCESS, MPIR_ERR_RECOVERABLE, __func__, __LINE__,
                MPI_ERR_TYPE, "**dtypemismatch", 0);
            MPIR_STATUS_SET_COUNT(rreq->status,
                                  MPIDI_MVP_REQUEST(rreq, dev).msg_offset);
            MPIDI_MVP_REQUEST(rreq, dev).msgsize =
                MPIDI_MVP_REQUEST(rreq, dev).msg_offset;
            mpi_errno = MPIDI_MVP_Request_load_recv_iov(rreq);
            goto fn_exit;
        } else {
            MPIR_Assert(MPIDI_MVP_REQUEST(rreq, dev).iov_offset <
                        MPIDI_MVP_REQUEST(rreq, dev).iov_count);
        }
        /* --END ERROR HANDLING-- */

        if (last == MPIDI_MVP_REQUEST(rreq, dev).recv_data_sz +
                        MPIDI_MVP_REQUEST(rreq, dev).orig_msg_offset) {
            /*MPL_DBG_MSG(MPIDI_MVP_DBG_CHANNEL,VERBOSE,
             "updating rreq to read the remaining data directly into the user
             buffer");*/
            /* Eventually, use OnFinal for this instead */
            MPIDI_MVP_REQUEST(rreq, dev).OnDataAvail =
                MPIDI_MVP_REQUEST(rreq, dev).OnFinal;
            MPIDI_MVP_REQUEST(rreq, dev).orig_msg_offset =
                MPIDI_LOAD_RECV_IOV_ORIG_MSG_OFFSET_UNSET;
        } else if (MPIDI_Request_get_type(rreq) ==
                       MPIDI_REQUEST_TYPE_ACCUM_RECV ||
                   MPIDI_Request_get_type(rreq) ==
                       MPIDI_REQUEST_TYPE_GET_ACCUM_RECV ||
                   (last == MPIDI_MVP_REQUEST(rreq, dev).msgsize ||
                    (last - MPIDI_MVP_REQUEST(rreq, dev).msg_offset) /
                            MPIDI_MVP_REQUEST(rreq, dev).iov_count >=
                        MPIDI_IOV_DENSITY_MIN)) {
            /*
            MPL_DBG_MSG(MPIDI_MVP_DBG_CHANNEL,VERBOSE,
             "updating rreq to read more data directly into the user buffer");
            MPIDI_MVP_REQUEST(rreq, dev).msg_offset = last;
            MPIDI_MVP_REQUEST(rreq, dev).OnDataAvail =
            MPIDI_MVP_ReqHandler_ReloadIOV;*/
        } else {
            /* Too little data would have been received using an IOV.
               We will start receiving data into a SRBuf and unpacking it
               later. */
            MPIR_Assert(MPIDI_Request_get_srbuf_flag(rreq) == FALSE);

            MPIDI_MVP_SRBuf_alloc(rreq,
                                  MPIDI_MVP_REQUEST(rreq, dev).msgsize -
                                      MPIDI_MVP_REQUEST(rreq, dev).msg_offset);
            MPIDI_MVP_REQUEST(rreq, dev).tmpbuf_off = 0;
            /* --BEGIN ERROR HANDLING-- */
            if (MPIDI_MVP_REQUEST(rreq, dev).tmpbuf_sz == 0) {
                /* FIXME - we should drain the data off the pipe here, but we
                   don't have a buffer to drain it into.  should this be
                   a fatal error? */
                // MPL_DBG_MSG(MPIDI_MVP_DBG_CHANNEL,VERBOSE,"SRBuf allocation
                // failure");
                mpi_errno = MPIR_Err_create_code(
                    MPI_SUCCESS, MPIR_ERR_FATAL, __func__, __LINE__,
                    MPI_ERR_OTHER, "**nomem", "**nomem %d",
                    MPIDI_MVP_REQUEST(rreq, dev).msgsize -
                        MPIDI_MVP_REQUEST(rreq, dev).msg_offset);
                rreq->status.MPI_ERROR = mpi_errno;
                goto fn_exit;
            }
            /* --END ERROR HANDLING-- */

            /* fill in the IOV using a recursive call */
            mpi_errno = MPIDI_MVP_Request_load_recv_iov(rreq);
        }
    } else {
        /* receive and toss any extra data that does not fit in the user's
           buffer */
        intptr_t data_sz;

        data_sz = MPIDI_MVP_REQUEST(rreq, dev).recv_data_sz -
                  MPIDI_MVP_REQUEST(rreq, dev).msg_offset;
        if (!MPIDI_Request_get_srbuf_flag((rreq))) {
            MPIDI_MVP_SRBuf_alloc(rreq, data_sz);
            /* --BEGIN ERROR HANDLING-- */
            if (MPIDI_MVP_REQUEST(rreq, dev).tmpbuf_sz == 0) {
                // MPL_DBG_MSG(MPIDI_MVP_DBG_CHANNEL,TYPICAL,"SRBuf allocation
                // failure");
                mpi_errno =
                    MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_FATAL, __func__,
                                         __LINE__, MPI_ERR_OTHER, "**nomem", 0);
                rreq->status.MPI_ERROR = mpi_errno;
                goto fn_exit;
            }
            /* --END ERROR HANDLING-- */
        }

        if (data_sz <= MPIDI_MVP_REQUEST(rreq, dev).tmpbuf_sz) {
            /*MPL_DBG_MSG(MPIDI_MVP_DBG_CHANNEL,VERBOSE,
            "updating rreq to read overflow data into the SRBuf and
            complete");*/
            MPIDI_MVP_REQUEST(rreq, dev).iov[0].iov_len = data_sz;
            MPIR_Assert(
                (MPIDI_Request_get_type(rreq) == MPIDI_REQUEST_TYPE_RECV) ||
                (MPIDI_Request_get_type(rreq) == MPIDI_REQUEST_TYPE_IRECV));
            /* Eventually, use OnFinal for this instead */
            MPIDI_MVP_REQUEST(rreq, dev).OnDataAvail =
                MPIDI_MVP_REQUEST(rreq, dev).OnFinal;
            MPIDI_MVP_REQUEST(rreq, dev).orig_msg_offset =
                MPIDI_LOAD_RECV_IOV_ORIG_MSG_OFFSET_UNSET;
        } else {
            /*MPL_DBG_MSG(MPIDI_MVP_DBG_CHANNEL,VERBOSE,
              "updating rreq to read overflow data into the SRBuf and reload
              IOV");*/
            MPIDI_MVP_REQUEST(rreq, dev).iov[0].iov_len =
                MPIDI_MVP_REQUEST(rreq, dev).tmpbuf_sz;
            MPIDI_MVP_REQUEST(rreq, dev).msg_offset +=
                MPIDI_MVP_REQUEST(rreq, dev).tmpbuf_sz;
            // MPIDI_MVP_REQUEST(rreq, dev).OnDataAvail =
            // MPIDI_MVP_ReqHandler_ReloadIOV;
        }

        MPIDI_MVP_REQUEST(rreq, dev).iov[0].iov_base =
            (void *)MPIDI_MVP_REQUEST(rreq, dev).tmpbuf;
        MPIDI_MVP_REQUEST(rreq, dev).iov_count = 1;
    }

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_REQUEST_LOAD_RECV_IOV);
    return mpi_errno;
}

int MPIDI_MVP_MRAIL_Prepare_rndv(MPIDI_MVP_ep_t *vc, MPIR_Request *req)
{
    int mpi_errno = MPI_SUCCESS;
    PRINT_DEBUG(DEBUG_RNDV_verbose > 1,
                "vc->rank: %d, req: %p, protocol: %d, recv size %ld, segsize "
                "%ld, iov count %d\n",
                vc->pg_rank, req, MPIDI_MVP_REQUEST(req, mrail).protocol,
                MPIDI_MVP_REQUEST(req, dev).recv_data_sz,
                MPIDI_MVP_REQUEST(req, dev).msgsize,
                MPIDI_MVP_REQUEST(req, dev).iov_count);

    if (IS_VC_SMP(vc)) {
        if (MPIDI_MVP_REQUEST(req, dev).iov_count > 1) {
            MPIDI_MVP_REQUEST(req, mrail).protocol = MVP_RNDV_PROTOCOL_R3;
            MPIDI_MVP_MRAIL_FREE_RNDV_BUFFER(req);
        } else if (MPIDI_MVP_REQUEST(req, mrail).protocol ==
                   MRAILI_PROTOCOL_RENDEZVOUS_UNSPECIFIED) {
            MPIDI_MVP_REQUEST(req, mrail).protocol = MVP_SMP_RNDV_PROTOCOL;
        }
        if (MPIDI_MVP_REQUEST(req, mrail).protocol == MVP_RNDV_PROTOCOL_R3) {
            goto fn_exit;
        }
    }

    /* Step 1: ready for user space (user buffer or pack) */
    if (1 == MPIDI_MVP_REQUEST(req, dev).iov_count &&
        (MPIDI_MVP_REQUEST(req, dev).OnDataAvail == NULL ||
         MPIDI_MVP_REQUEST(req, dev).OnDataAvail ==
             MPIDI_MVP_REQUEST(req, dev).OnFinal)) {
        MPIDI_MVP_REQUEST(req, mrail).rndv_buf =
            MPIDI_MVP_REQUEST(req, dev).iov[0].iov_base;
        MPIDI_MVP_REQUEST(req, mrail).rndv_buf_sz =
            MPIDI_MVP_REQUEST(req, dev).iov[0].iov_len;
        MPIDI_MVP_REQUEST(req, mrail).rndv_buf_alloc = 0;
        /* This buffer allocation is not needed for R3 protocol,
         * as R3 does eager send directly from IOVs*/
    } else if (MPIDI_MVP_REQUEST(req, mrail).protocol != MVP_RNDV_PROTOCOL_R3) {
        MPIDI_MVP_REQUEST(req, mrail).rndv_buf_sz =
            MPIDI_MVP_REQUEST(req, dev).msgsize;
        MPIDI_MVP_REQUEST(req, mrail).rndv_buf = MPL_malloc(
            MPIDI_MVP_REQUEST(req, mrail).rndv_buf_sz, MPL_MEM_BUFFER);

        if (MPIDI_MVP_REQUEST(req, mrail).rndv_buf == NULL) {
            /* fall back to r3 if cannot allocate tmp buf */
            PRINT_DEBUG(DEBUG_RNDV_verbose,
                        "cannot allocate space, falling back to R3\n");
            MPIDI_MVP_REQUEST(req, mrail).protocol = MVP_RNDV_PROTOCOL_R3;
            MPIDI_MVP_REQUEST(req, mrail).rndv_buf_sz = 0;
        } else {
            MPIDI_MVP_REQUEST(req, mrail).rndv_buf_alloc = 1;
        }
    }
    MPIDI_MVP_REQUEST(req, mrail).rndv_buf_off = 0;

    /* Step 1.5: If use R3 for smaller messages */
    if (MPIDI_MVP_REQUEST(req, mrail).rndv_buf_sz <=
        MPIDI_MVP_R3_THRESHOLD(vc)) {
        PRINT_DEBUG(DEBUG_RNDV_verbose > 1,
                    "Using R3, rndv_buf_sz: %ld, rdma_r3_threshold: %d\n",
                    MPIDI_MVP_REQUEST(req, mrail).rndv_buf_sz,
                    rdma_r3_threshold);
        MPIDI_MVP_REQUEST(req, mrail).protocol = MVP_RNDV_PROTOCOL_R3;
        MPIDI_MVP_MRAIL_FREE_RNDV_BUFFER(req);
    }

#ifdef _ENABLE_UD_
    if (MVP_USE_UD_HYBRID &&
        (MPIDI_MVP_REQUEST(req, mrail).rndv_buf_sz < rdma_ud_zcopy_threshold ||
         MPIDI_MVP_REQUEST(req, mrail).rndv_buf_sz >
             (MRAIL_MAX_UD_SIZE * rdma_ud_zcopy_rq_size))) {
        /*len <= (rdma_default_ud_mtu * 4096) */
        MPIDI_MVP_REQUEST(req, mrail).protocol = MVP_RNDV_PROTOCOL_R3;
        MPIDI_MVP_MRAIL_FREE_RNDV_BUFFER(req);
    }
#endif /* _ENABLE_UD_ */

    /* Step 2: try register and decide the protocol */
    /* The additional check for intra-node peer has been added to handle
       case when RGET is used for CUDA IPC communication when shared memory
       is enabled. In this case registration cache is not initialized and
       hence dreg_register leads to a hang. Better separation of thsee cases
       might be possible */
    if (

        (MVP_RNDV_PROTOCOL_RPUT == MPIDI_MVP_REQUEST(req, mrail).protocol ||
         MVP_RNDV_PROTOCOL_RGET == MPIDI_MVP_REQUEST(req, mrail).protocol ||
         MRAILI_PROTOCOL_UD_ZCOPY == MPIDI_MVP_REQUEST(req, mrail).protocol)) {
        if (IS_VC_SMP(vc)) {
            PRINT_DEBUG(DEBUG_RNDV_verbose > 1,
                        "SMP vc, not registering. rank: %d, buf size: %ld, "
                        "addr: %p, protocol: %d\n",
                        vc->pg_rank, MPIDI_MVP_REQUEST(req, mrail).rndv_buf_sz,
                        MPIDI_MVP_REQUEST(req, mrail).rndv_buf,
                        MPIDI_MVP_REQUEST(req, mrail).protocol);
            MPIDI_MVP_REQUEST(req, mrail).d_entry = NULL;
        }

        MPIDI_MVP_REQUEST(req, mrail).local_complete = 0;
        MPIDI_MVP_REQUEST(req, mrail).remote_complete = 0;
    } else {
        MPIDI_MVP_REQUEST(req, mrail).local_complete = 0;
        MPIDI_MVP_REQUEST(req, mrail).remote_complete = 0;
    }

fn_fail:
fn_exit:
    return mpi_errno;
}

int MPIDI_MVP_Rendezvous_push(MPIDI_MVP_ep_t *vc, MPIR_Request *sreq)
{
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_RNDV_PUSH);

    if (mvp_smp_init && vc->smp.local_nodes >= 0 &&
        vc->smp.local_nodes != mvp_smp_info.my_local_id) {
        MPIDI_MVP_SMP_Rendezvous_push(vc, sreq);
        return MPI_SUCCESS;
    }
#if 0
    switch (sreq->mrail.protocol)
    {
    case MVP_RNDV_PROTOCOL_RPUT:
            MPIDI_MVP_MRAILI_Rendezvous_rput_push(vc, sreq);
        break;
    case MVP_RNDV_PROTOCOL_RGET:
            MPIDI_MVP_MRAILI_Rendezvous_rget_push(vc, sreq);
        break;
#ifdef _ENABLE_UD_
    case MVP_RNDV_PROTOCOL_UD_ZCOPY:
            MPIDI_MVP_MRAILI_Rendezvous_zcopy_push(vc, sreq,
                        &(mvp_MPIDI_MVP_RDMA_Process.zcopy_info));
        break;
#endif
#if defined(_ENABLE_CUDA_) && defined(HAVE_CUDA_IPC)
    case MVP_RNDV_PROTOCOL_CUDAIPC:
            MPIDI_MVP_DEVICE_IPC_Rendezvous_push(vc, sreq);
        break;
#endif
    default:
            MPIDI_MVP_Rendezvous_r3_push(vc, sreq);
        break;
    }
#endif

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_RNDV_PUSH);
    return MPI_SUCCESS;
}

/* returns the number of elements in the unexpected queue */

/* TODO: FIXME: DUMMY VAR FOR RDMA_RNDV_IMMEDIATE */
int rdma_rndv_immediate = 1;

/* MVP optimized SMP RGET design */
int MPIDI_MVP_PktHandler_SMP_RTS(MPIDI_MVP_ep_t *vc, MPIDI_MVP_Pkt_t *pkt,
                                 intptr_t *buflen, MPIR_Request **rreqp)
{
    int found, k;
    int mpi_errno = MPI_SUCCESS;
    MPIR_Request *rreq;
    MPID_Seqnum_t seqnum;
    MPIDI_MVP_Pkt_rndv_req_to_send_t *rts_pkt = &pkt->rndv_req_to_send;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_PKTHANDLER_SMP_RTS);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_PKTHANDLER_SMP_RTS);

    MPID_THREAD_CS_ENTER(POBJ, MPIR_THREAD_POBJ_MSGQ_MUTEX);
    if (*rreqp == NULL) {
        MPIDI_MVP_smp_recvq_recv(rts_pkt->match, &found, &rreq);
        MPIR_ERR_CHKANDJUMP1(!rreq, mpi_errno, MPI_ERR_OTHER, "**nomemreq",
                             "**nomemuereq %d", MPIDI_MVP_Recvq_count_unexp());

        if (found) {
            PRINT_DEBUG(DEBUG_RNDV_verbose > 1,
                        "posted request found, "
                        "rreq: %p, rank: %d, tag: %d, context: %d, protocol: "
                        "%d, recv_sz: %ld\n",
                        rreq, MPIDI_MVP_REQUEST(rreq, dev).match.parts.rank,
                        MPIDI_MVP_REQUEST(rreq, dev).match.parts.tag,
                        MPIDI_MVP_REQUEST(rreq, dev).match.parts.context_id,
                        MPIDI_MVP_REQUEST(rreq, mrail).protocol,
                        MPIDI_MVP_REQUEST(rreq, dev).recv_data_sz);
        } else if (MPIR_cc_get(rreq->cc) == 0) {
            *rreqp = NULL;
            goto fn_fail;
        }

        set_request_info(rreq, rts_pkt, MPIDI_REQUEST_RNDV_MSG);
        MPID_THREAD_CS_EXIT(POBJ, MPIR_THREAD_POBJ_MSGQ_MUTEX);

        *buflen = sizeof(MPIDI_MVP_Pkt_t);
        mvp_select_rndv_protocol(vc, rreq, rts_pkt);
    } else {
        found = 1;
        rreq = *rreqp;
        *buflen = sizeof(MPIDI_MVP_Pkt_t);
        set_request_info(rreq, rts_pkt, MPIDI_REQUEST_RNDV_MSG);
        mvp_select_rndv_protocol(vc, rreq, rts_pkt);
        PRINT_DEBUG(DEBUG_RNDV_verbose > 1,
                    "Saved RTS, sreq: %08x, rank: %d, tag: %d, context: %d, "
                    "protocol: %d, data_sz: %ld\n",
                    rts_pkt->sender_req_id, rts_pkt->match.parts.rank,
                    rts_pkt->match.parts.tag, rts_pkt->match.parts.context_id,
                    rts_pkt->rndv.protocol, rts_pkt->data_sz);
    }

    if (found) {
        if (MVP_RNDV_PROTOCOL_RGET == rts_pkt->rndv.protocol) {
            MPIDI_MVP_MRAIL_SET_REQ_REMOTE_RNDV(
                MPIDI_MVP_REQUEST_FROM_MPICH(rreq), rts_pkt);

            mpi_errno = MPIDI_MVP_Post_data_receive_found(rreq);
            if (mpi_errno != MPI_SUCCESS && rreq != NULL) {
                MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER,
                                    "**ch3|loadsendiov");
            }
            mpi_errno = MPIDI_MVP_Prepare_rndv_get(vc, rreq);
            if (mpi_errno != MPI_SUCCESS) {
                MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER, "**ch3|rndv");
            }
            if (!rdma_rndv_immediate) {
                RENDEZVOUS_IN_PROGRESS(MPIDI_MVP_VC(vc), rreq);
                MPIDI_MVP_REQUEST(rreq, mrail).nearly_complete = 0;
                PUSH_FLOWLIST(vc);
            } else {
                mpi_errno = MPIDI_MVP_Rendezvous_push(vc, rreq);
                if (mpi_errno != MPI_SUCCESS && rreq != NULL) {
                    MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER,
                                        "**ch3|senddata");
                }
            }
        }

        if (MVP_RNDV_PROTOCOL_RPUT == rts_pkt->rndv.protocol ||
            MVP_RNDV_PROTOCOL_R3 == rts_pkt->rndv.protocol) {
            MPIR_Request *cts_req;
            MPIDI_MVP_Pkt_t upkt;
            MPIDI_MVP_Pkt_rndv_clr_to_send_t *cts_pkt = &upkt.rndv_clr_to_send;

            MPIDI_Pkt_init(cts_pkt, MPIDI_MVP_PKT_RNDV_CLR_TO_SEND);
            cts_pkt->sender_req_id = rts_pkt->sender_req_id;
            cts_pkt->receiver_req_id = rreq->handle;
            cts_pkt->recv_sz = 0;

            MPIDI_VC_FAI_send_seqnum(vc, seqnum);
            MPIDI_Pkt_set_seqnum(cts_pkt, seqnum);

            mpi_errno = MPIDI_MVP_Post_data_receive_found(rreq);
            if (mpi_errno != MPI_SUCCESS && rreq != NULL) {
                MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER,
                                    "**ch3|loadsendiov");
            }

            if (MPIDI_MVP_REQUEST(rreq, dev).OnDataAvail == NULL) {
                for (k = 0; k < MPIDI_MVP_REQUEST(rreq, dev).iov_count; ++k) {
                    cts_pkt->recv_sz +=
                        MPIDI_MVP_REQUEST(rreq, dev).iov[k].iov_len;
                }
            } else {
                cts_pkt->recv_sz = MPIDI_MVP_REQUEST(rreq, dev).msgsize;
            }

            mpi_errno = MPIDI_MVP_Prepare_rndv_cts(vc, cts_pkt, rreq);
            if (mpi_errno != MPI_SUCCESS) {
                MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER, "**ch3|rndv");
            }

            PRINT_DEBUG(DEBUG_RNDV_verbose > 1,
                        "sending CTS to rank: %d, buf_addr: %p, recv_sz: %ld, "
                        "protocol: %d\n",
                        vc->pg_rank, cts_pkt->rndv.buf_addr, cts_pkt->recv_sz,
                        cts_pkt->rndv.protocol);

            MPID_THREAD_CS_ENTER(POBJ, vc->pobj_mutex);
            mpi_errno =
                MPIDI_MVP_iStartMsg(vc, cts_pkt, sizeof(*cts_pkt), &cts_req);
            MPID_THREAD_CS_EXIT(POBJ, vc->pobj_mutex);
            if (mpi_errno != MPI_SUCCESS) {
                MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER, "**ch3|ctspkt");
            }
            if (cts_req != NULL) {
                MPIDI_MVP_Request_free(cts_req);
            }
        }
    } else {
        MVP_INC_NUM_POSTED_RECV();
        PRINT_DEBUG(DEBUG_RNDV_verbose > 1,
                    "unexpected request allocated, rreq: %p, sreq: %08x\n",
                    rreq, rts_pkt->sender_req_id);
        MPIR_Memcpy(&MPIDI_MVP_REQUEST(rreq, ch).pkt, pkt,
                    sizeof(MPIDI_MVP_Pkt_t));
        MPIDI_MVP_Progress_signal_completion();
    }

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_PKTHANDLER_SMP_RTS);
    *rreqp = NULL;
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

/*
 * This function is used to post a receive operation on a request for the
 * next data to arrive.  In turn, this request is attached to a virtual
 * connection.
 */
int MPIDI_MVP_Post_data_receive_found(MPIR_Request *rreq)
{
    int mpi_errno = MPI_SUCCESS;
    int dt_contig;
    MPI_Aint dt_true_lb;
    intptr_t userbuf_sz;
    MPIR_Datatype *dt_ptr = NULL;
    intptr_t data_sz;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_POST_DATA_RECEIVE_FOUND);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_POST_DATA_RECEIVE_FOUND);

    // MPL_DBG_MSG(MPIDI_MVP_DBG_OTHER,VERBOSE,"posted request found");

    MPIDI_Datatype_get_info(MPIDI_MVP_REQUEST(rreq, dev).user_count,
                            MPIDI_MVP_REQUEST(rreq, dev).datatype, dt_contig,
                            userbuf_sz, dt_ptr, dt_true_lb);

    if (MPIDI_MVP_REQUEST(rreq, dev).recv_data_sz <= userbuf_sz) {
        data_sz = MPIDI_MVP_REQUEST(rreq, dev).recv_data_sz;
    } else {
        /*MPL_DBG_MSG_FMT(MPIDI_MVP_DBG_OTHER,VERBOSE,(MPL_DBG_FDEST,
                   "receive buffer too small; message truncated, msg_sz=%"
           PRIdPTR ", userbuf_sz=%" PRIdPTR, MPIDI_MVP_REQUEST(rreq,
           dev).recv_data_sz, userbuf_sz));*/
        rreq->status.MPI_ERROR = MPIR_Err_create_code(
            MPI_SUCCESS, MPIR_ERR_RECOVERABLE, __func__, __LINE__,
            MPI_ERR_TRUNCATE, "**truncate", "**truncate %d %d %d %d",
            rreq->status.MPI_SOURCE, rreq->status.MPI_TAG,
            MPIDI_MVP_REQUEST(rreq, dev).recv_data_sz, userbuf_sz);
        MPIR_STATUS_SET_COUNT(rreq->status, userbuf_sz);
        data_sz = userbuf_sz;
    }

    MPIR_Assert(data_sz == MPIDI_MVP_REQUEST(rreq, dev).recv_data_sz);

    /* user buffer is contiguous and large enough to store the
       entire message.  However, we haven't yet *read* the data
       (this code describes how to read the data into the destination) */
    MPIDI_MVP_REQUEST(rreq, dev).iov[0].iov_base =
        (void *)((char *)(MPIDI_MVP_REQUEST(rreq, dev.user_buf)) + dt_true_lb);
    MPIDI_MVP_REQUEST(rreq, dev.iov[0].iov_len) = data_sz;
    MPIDI_MVP_REQUEST(rreq, dev.iov_count) = 1;
    /* FIXME: We want to set the OnDataAvail to the appropriate
       function, which depends on whether this is an RMA
       request or a pt-to-pt request. */
    MPIDI_MVP_REQUEST(rreq, dev.OnDataAvail) = 0;

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_POST_DATA_RECEIVE_FOUND);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIDI_MVP_Prepare_rndv_get(MPIDI_MVP_ep_t *vc, MPIR_Request *rreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_PREPARE_RNDV_GET);

    /*#ifdef CKPT
        MPIDI_MVP_CR_lock();
    #endif
    */
    MPIR_Assert(MVP_RNDV_PROTOCOL_RGET ==
                MPIDI_MVP_REQUEST(rreq, mrail).protocol);

    mpi_errno = MPIDI_MVP_MRAIL_Prepare_rndv(vc, rreq);
    MPIR_ERR_CHECK(mpi_errno);

    /*#ifdef CKPT
        MPIDI_MVP_CR_unlock();
    #endif*/

fn_fail:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_PREPARE_RNDV_GET);
    return mpi_errno;
}

#if defined(_SMP_CMA_)
// extern size_t MVP_CMA_MSG_LIMIT;
static pid_t MPIDI_MVP_SMP_get_pid(MPIDI_MVP_ep_t *vc)
{
    pid_t pid;
    MPIR_Assert(IS_VC_SMP(vc));
    pid = mvp_smp_shmem_region->pid[vc->smp.local_nodes];
    return pid;
}

int MPIDI_MVP_SMP_do_cma_put(MPIDI_MVP_ep_t *vc, const void *src, void *dst,
                             ssize_t len)
{
    pid_t pid;
    ssize_t nbytes, total = 0;
    struct iovec local, remote;
    int mpi_errno = MPI_SUCCESS;

    local.iov_base = (void *)src;
    local.iov_len = len;
    remote.iov_base = dst;
    remote.iov_len = len;

    MPIR_FUNC_VERBOSE_ENTER(MPIDI_MVP_SMP_DO_CMA_PUT);

    pid = MPIDI_MVP_SMP_get_pid(vc);
    PRINT_DEBUG(DEBUG_RNDV_verbose > 0,
                "CMA write to rank: %d, pid: %d, src: %p, dst: %p, len: %ld\n",
                vc->pg_rank, pid, src, dst, len);

    do {
        total += nbytes = process_vm_writev(pid, &local, 1, &remote, 1, 0);
        PRINT_DEBUG(
            DEBUG_RNDV_verbose > 2,
            "CMA write to rank: %d, nbytes: %ld, len: %ld, remaining: %ld\n",
            vc->pg_rank, nbytes, len, len - total);

        if (nbytes < 0) {
            MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**fail",
                                      "**fail %s", "process_vm_writev fail");
        }

        local.iov_base += nbytes;
        local.iov_len -= nbytes;
        remote.iov_base += nbytes;
        remote.iov_len -= nbytes;
    } while (total < len);

    MPIR_FUNC_VERBOSE_EXIT(MPIDI_MVP_SMP_DO_CMA_PUT);
fn_fail:
    return mpi_errno;
}

int MPIDI_MVP_SMP_do_cma_get(MPIDI_MVP_ep_t *vc, const void *src, void *dst,
                             ssize_t len)
{
    pid_t pid;
    ssize_t nbytes, total = 0;
    struct iovec local, remote;
    int mpi_errno = MPI_SUCCESS;

    local.iov_base = dst;
    local.iov_len = len;
    remote.iov_base = (void *)src;
    remote.iov_len = len;

    MPIR_FUNC_VERBOSE_ENTER(MPIDI_MVP_SMP_DO_CMA_GET);

    pid = MPIDI_MVP_SMP_get_pid(vc);
    PRINT_DEBUG(DEBUG_RNDV_verbose > 0,
                "CMA read from rank: %d, pid: %d, src: %p, dst: %p, len: %ld\n",
                vc->pg_rank, pid, src, dst, len);

    do {
        total += nbytes = process_vm_readv(pid, &local, 1, &remote, 1, 0);
        PRINT_DEBUG(
            DEBUG_RNDV_verbose > 2,
            "CMA read from rank: %d, nbytes: %ld, len: %ld, remaining: %ld\n",
            vc->pg_rank, nbytes, len, len - total);

        if (nbytes < 0) {
            MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**fail",
                                      "**fail %s", "process_vm_readv fail");
        }

        local.iov_base += nbytes;
        local.iov_len -= nbytes;
        remote.iov_base += nbytes;
        remote.iov_len -= nbytes;
    } while (total < len);

    MPIR_FUNC_VERBOSE_EXIT(MPIDI_MVP_SMP_DO_CMA_GET);
fn_fail:
    return mpi_errno;
}

/*
int MPIDI_MVP_SMP_do_cma_read(const struct iovec * iov,
        const int iovlen, void *cma_header,
        size_t *num_bytes_ptr)
{
    int mpi_errno = MPI_SUCCESS;
    size_t cerr;
    pid_t pid;
    struct cma_header *c_header = (struct cma_header *) cma_header;
    size_t cma_total_bytes = c_header->total_bytes;
    struct iovec *local_iovec;
    size_t msglen, iov_len;
    int iov_off = 0, buf_off = 0;
    size_t received_bytes = 0;

    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_DO_CMA_READ);

    local_iovec = (struct iovec *)iov;
    pid = c_header->pid;
    msglen = cma_total_bytes;
    iov_len = iov[0].iov_len;
    for (; cma_total_bytes > 0 && iov_off < iovlen; ) {
        if (unlikely(msglen > MVP_CMA_MSG_LIMIT)) {
            local_iovec[iov_off].iov_len = MVP_CMA_MSG_LIMIT;
            cerr = process_vm_readv(pid, &local_iovec[iov_off], 1,
c_header->remote, 1, 0); if( cerr == -1 ) MPIR_ERR_SETFATALANDJUMP1(mpi_errno,
MPI_ERR_OTHER,
                        "**fail", "**fail %s",
                        "CMA: (MPIDI_CH3I_SMP_do_cma_read) process_vm_readv
fail");

            MPIR_Assert(cerr == MVP_CMA_MSG_LIMIT);
            local_iovec[iov_off].iov_base += MVP_CMA_MSG_LIMIT;
            local_iovec[iov_off].iov_len = iov_len - MVP_CMA_MSG_LIMIT;
            received_bytes += MVP_CMA_MSG_LIMIT;
            cma_total_bytes -= MVP_CMA_MSG_LIMIT;
            msglen -= MVP_CMA_MSG_LIMIT;
            iov_len -= MVP_CMA_MSG_LIMIT;

            c_header->remote[0].iov_len -= MVP_CMA_MSG_LIMIT;
            c_header->remote[0].iov_base += MVP_CMA_MSG_LIMIT;


        } else if (msglen == iov_len) {
            local_iovec[iov_off].iov_base += buf_off;
            cerr = process_vm_readv(pid, &local_iovec[iov_off], 1,
c_header->remote, 1, 0); if( cerr == -1 ) MPIR_ERR_SETFATALANDJUMP1(mpi_errno,
MPI_ERR_OTHER,
                        "**fail", "**fail %s",
                        "CMA: (MPIDI_CH3I_SMP_do_cma_read) process_vm_readv
fail");

            MPIR_Assert(cerr == msglen);
            received_bytes += msglen;
            cma_total_bytes -= msglen;

            MPIR_Assert(cma_total_bytes == 0 && ++iov_off >= iovlen);

        } else if (msglen > iov_len) {
            local_iovec[iov_off].iov_base += buf_off;
            cerr = process_vm_readv(pid, &local_iovec[iov_off], 1,
c_header->remote, 1, 0); if( cerr == -1 ) MPIR_ERR_SETFATALANDJUMP1(mpi_errno,
MPI_ERR_OTHER,
                        "**fail", "**fail %s",
                        "CMA: (MPIDI_MVP_SMP_do_cma_read) process_vm_readv
fail");

            MPIR_Assert(cerr == iov_len);
            received_bytes += iov_len;
            cma_total_bytes -= iov_len;
            msglen -= iov_len;

            c_header->remote[0].iov_len -= iov_len;
            c_header->remote[0].iov_base += iov_len;

            if (++iov_off >= iovlen)
                break;
            buf_off = 0;
            iov_len = iov[iov_off].iov_len;

        }  else if (msglen > 0) {
            local_iovec[iov_off].iov_base += buf_off;
            cerr = process_vm_readv(pid, &local_iovec[iov_off], 1,
c_header->remote, 1, 0); if( cerr == -1 ) MPIR_ERR_SETFATALANDJUMP1(mpi_errno,
MPI_ERR_OTHER,
                        "**fail", "**fail %s",
                        "CMA: (MPIDI_MVP_SMP_do_cma_read) process_vm_readv
fail");

            MPIR_Assert(cerr == msglen);
            received_bytes += msglen;
            cma_total_bytes -= msglen;
        }
    }
    *num_bytes_ptr = received_bytes;
    c_header->total_bytes -= received_bytes;

fn_exit:
    PRINT_DEBUG(DEBUG_SHM_verbose>1, "return with nb %ld\n", *num_bytes_ptr);
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_DO_CMA_READ);
    return mpi_errno;

fn_fail:
    goto fn_exit;
}
*/

static int MPIDI_MVP_CMA_Rendezvous_push(MPIDI_MVP_ep_t *vc, MPIR_Request *sreq)
{
    int mpi_errno = MPI_SUCCESS;
    int complete = 0, rail = -1;
    void *src, *dst, *user_buf;
    ssize_t len;
    int dt_contig;
    MPI_Aint dt_true_lb;
    size_t data_sz;
    MPIR_Datatype *dt_ptr;
    /* TODO: get these out of comments so we don't have random unused vars */
    ssize_t offset ATTRIBUTE((unused));
    int type ATTRIBUTE((unused)) = MPIDI_Request_get_type(sreq);

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_CMA_RNDV_PUSH);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_CMA_RNDV_PUSH);

    PRINT_DEBUG(DEBUG_RNDV_verbose > 2,
                "req type: %d, protocol: %d, partner_id: %08x, iov count: %d, "
                "offset %lu, len: %lu\n",
                type, MPIDI_MVP_REQUEST(sreq, mrail.protocol),
                MPIDI_MVP_REQUEST(sreq, mrail.partner_id),
                MPIDI_MVP_REQUEST(sreq, dev.iov_count),
                MPIDI_MVP_REQUEST(sreq, dev.iov_offset),
                MPIDI_MVP_REQUEST(sreq, dev.iov[0].iov_len));

    /* Non-contig sends are handled using the R3 protocol */
    MPIR_Assert(MPIDI_MVP_REQUEST(sreq, dev.iov_count) == 1);
    MPIR_Assert(
        MPIDI_MVP_REQUEST(sreq, mrail.protocol) == MVP_RNDV_PROTOCOL_RPUT ||
        MPIDI_MVP_REQUEST(sreq, mrail.protocol) == MVP_RNDV_PROTOCOL_RGET);

    MPIDI_Datatype_get_info(MPIDI_MVP_REQUEST(sreq, dev.user_count),
                            MPIDI_MVP_REQUEST(sreq, dev.datatype), dt_contig,
                            data_sz, dt_ptr, dt_true_lb);

    if (MPIDI_MVP_REQUEST(sreq, mrail.protocol) == MVP_RNDV_PROTOCOL_RPUT) {
        src = MPIDI_MVP_REQUEST(sreq, dev.iov[0].iov_base);
        len = MPIDI_MVP_REQUEST(sreq, dev.iov[0].iov_len);
        dst = MPIDI_MVP_REQUEST(sreq, mrail.remote_addr);

        mpi_errno = MPIDI_MVP_SMP_do_cma_put(vc, src, dst, len);
        if (MPI_SUCCESS != mpi_errno) {
            vc->ch.state = MPIDI_MVP_VC_STATE_FAILED;
            sreq->status.MPI_ERROR = MPI_ERR_INTERN;
            MPID_Request_complete(sreq);
            MPIR_ERR_POP(mpi_errno);
        }

        MPIDI_MVP_Handle_send_req(vc, sreq, &complete);
        MRAILI_RDMA_Put_finish(vc, sreq, rail);
        MPIDI_MVP_REQUEST(sreq, mrail.nearly_complete) = 1;
    } else if (MPIDI_MVP_REQUEST(sreq, mrail.protocol) ==
               MVP_RNDV_PROTOCOL_RGET) {
        if (dt_contig) {
            dst = MPIDI_MVP_REQUEST(sreq, dev.iov[0].iov_base);
            len = MPIDI_MVP_REQUEST(sreq, dev.iov[0].iov_len);
            src = MPIDI_MVP_REQUEST(sreq, mrail.remote_addr);
        } else {
            MPIDI_MVP_REQUEST(sreq, dev.tmpbuf) =
                MPL_malloc(data_sz, MPL_MEM_BUFFER);
            if (!MPIDI_MVP_REQUEST(sreq, dev.tmpbuf)) {
                mpi_errno = MPIR_Err_create_code(
                    MPI_SUCCESS, MPIR_ERR_RECOVERABLE, __func__, __LINE__,
                    MPI_ERR_OTHER, "**nomem", "**nomem %s %d", "tmp_buf",
                    data_sz);
                MPIR_ERR_POP(mpi_errno);
            }

            user_buf = MPIDI_MVP_REQUEST(sreq, dev.user_buf);
            dst = MPIDI_MVP_REQUEST(sreq, dev.tmpbuf);
            len = MPIDI_MVP_REQUEST(sreq, dev.iov[0].iov_len);
            src = MPIDI_MVP_REQUEST(sreq, mrail.remote_addr);
        }

        mpi_errno = MPIDI_MVP_SMP_do_cma_get(vc, src, dst, len);
        if (MPI_SUCCESS != mpi_errno) {
            vc->ch.state = MPIDI_MVP_VC_STATE_FAILED;
            sreq->status.MPI_ERROR = MPI_ERR_INTERN;
            MPID_Request_complete(sreq);
            MPIR_ERR_POP(mpi_errno);
        }

        if (!dt_contig) {
            MPI_Aint actual_unpack_bytes;

            mpi_errno = MPIR_Typerep_unpack(
                dst, len, user_buf, MPIDI_MVP_REQUEST(sreq, dev.user_count),
                MPIDI_MVP_REQUEST(sreq, dev.datatype),
                MPIDI_MVP_REQUEST(sreq, dev.msg_offset), &actual_unpack_bytes);
            MPIR_ERR_CHECK(mpi_errno);

            MPIR_Assert(actual_unpack_bytes == len);
            MPL_free(MPIDI_MVP_REQUEST(sreq, dev.tmpbuf));
        }

        MPIDI_MVP_REQUEST(sreq, mrail.nearly_complete) = 1;
        MPIDI_MVP_REQUEST(sreq, mrail.num_rdma_read_completions) = 1;
        MRAILI_RDMA_Get_finish(vc, sreq, rail);
    } else {
        mpi_errno =
            MPIR_Err_create_code(mpi_errno, MPIR_ERR_FATAL, __func__, __LINE__,
                                 MPI_ERR_OTHER, "**notimpl", 0);
        MPIR_ERR_POP(mpi_errno);
    }

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_CMA_RNDV_PUSH);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
#endif

static int MPIDI_MVP_SMP_Rendezvous_push(MPIDI_MVP_ep_t *vc, MPIR_Request *sreq)
{
    int nb;
    int complete = 0;
    int seqnum;
    int mpi_errno;
    MPIDI_MVP_Pkt_rndv_r3_data_t pkt_head;
    MPIR_Request *send_req;

    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_RNDV_PUSH);

    if (MPIDI_MVP_REQUEST(sreq, mrail).protocol != MVP_RNDV_PROTOCOL_R3

    ) {
#if defined(_SMP_CMA_)
        if (MVP_SMP_USE_CMA) {
            return MPIDI_MVP_CMA_Rendezvous_push(vc, sreq);
        }
#endif
    }

    MPIDI_Pkt_init(&pkt_head, MPIDI_MVP_PKT_RNDV_R3_DATA);
    pkt_head.receiver_req_id = MPIDI_MVP_REQUEST(sreq, mrail).partner_id;
    MPIDI_VC_FAI_send_seqnum(vc, seqnum);
    MPIDI_Pkt_set_seqnum(&pkt_head, seqnum);
    MPIDI_Request_set_seqnum(sreq, seqnum);

#if defined(_SMP_CMA_) || defined(_SMP_LIMIC_)

    /* Use cma for contiguous data
     * Use shared memory for non-contiguous data
     */
    pkt_head.csend_req_id = NULL;
    pkt_head.send_req_id = NULL;

#if defined(_SMP_CMA_)
    if (MVP_SMP_USE_CMA &&
        (!MVP_SMP_USE_MAX_SWITCH ||
         (MVP_SMP_USE_MAX_SWITCH &&
          MPIDI_MVP_REQUEST(sreq, dev).iov[0].iov_len < MVP_SMP_CMA_MAX_SIZE))
        /*            && MPIDI_MVP_REQUEST(sreq, dev).OnDataAvail !=
           MPIDI_MVP_ReqHandler_SendReloadIOV*/
        && MPIDI_MVP_REQUEST(sreq, dev).iov_count == 1

    ) {
        pkt_head.csend_req_id = sreq;
        pkt_head.send_req_id = NULL;
    }
#endif
#endif /*_SMP_SMA_ || _SMP_LIMIC_ */

#if defined(_SMP_LIMIC_) || defined(_SMP_CMA_)
    PRINT_DEBUG(DEBUG_RNDV_verbose > 1,
                "Sending R3 Data to %d, sreq: %p, partner: %08x, niov: %d, "
                "cma: %p, limic: %p\n",
                vc->pg_rank, sreq, MPIDI_MVP_REQUEST(sreq, mrail).partner_id,
                MPIDI_MVP_REQUEST(sreq, dev).iov_count, pkt_head.csend_req_id,
                pkt_head.send_req_id);
#else
    PRINT_DEBUG(DEBUG_RNDV_verbose > 1,
                "Sending R3 Data to %d, sreq: %p, partner: %08x, niov: %d\n",
                vc->pg_rank, sreq, MPIDI_MVP_REQUEST(sreq, mrail).partner_id,
                MPIDI_MVP_REQUEST(sreq, dev).iov_count);
#endif

    mpi_errno = MPIDI_MVP_iStartMsg(
        vc, &pkt_head, sizeof(MPIDI_MVP_Pkt_rndv_r3_data_t), &send_req);

    if (mpi_errno != MPI_SUCCESS) {
        MPIR_Object_set_ref(sreq, 0);
        MPIDI_MVP_Request_free(sreq);
        sreq = NULL;
        mpi_errno =
            MPIR_Err_create_code(mpi_errno, MPIR_ERR_FATAL, __func__, __LINE__,
                                 MPI_ERR_OTHER, "**ch3|rtspkt", 0);
        return mpi_errno;
    }
    /* --END ERROR HANDLING-- */
    if (send_req != NULL) {
        DEBUG_PRINT("r3 packet not sent \n");
        MPIDI_MVP_Request_free(send_req);
    }

#if defined(_SMP_LIMIC_) || defined(_SMP_CMA_)
    if (pkt_head.send_req_id || pkt_head.csend_req_id) {
        MPIDI_MVP_REQUEST(sreq, mrail).nearly_complete = 1;
        return MPI_SUCCESS;
    }
#endif

    PRINT_DEBUG(DEBUG_RNDV_verbose > 1, "R3 Data sent to %d, sreq: %p\n",
                vc->pg_rank, sreq);
    if (MPIDI_MVP_SMP_SendQ_empty(vc)) {
        vc->smp.send_current_pkt_type = SMP_RNDV_MSG;
        for (;;) {
            PRINT_DEBUG(DEBUG_RNDV_verbose > 1,
                        "sreq: %p, iov count: %d, offset %lu, len[0]: %lu\n",
                        sreq, MPIDI_MVP_REQUEST(sreq, dev).iov_count,
                        MPIDI_MVP_REQUEST(sreq, dev).iov_offset,
                        MPIDI_MVP_REQUEST(sreq, dev).iov[0].iov_len);

            if (vc->smp.send_current_pkt_type == SMP_RNDV_MSG) {
                {
                    mpi_errno = MPIDI_MVP_smp_write_rndv_data(
                        vc->smp.local_nodes,
                        &(MPIDI_MVP_REQUEST(sreq, dev)
                              .iov[MPIDI_MVP_REQUEST(sreq, dev).iov_offset]),
                        MPIDI_MVP_REQUEST(sreq, dev).iov_count -
                            MPIDI_MVP_REQUEST(sreq, dev).iov_offset,
                        0, &nb);
                }
            } else {
                MPIR_Assert(vc->smp.send_current_pkt_type == SMP_RNDV_MSG_CONT);

                {
                    mpi_errno = MPIDI_MVP_smp_write_rndv_data(
                        vc->smp.local_nodes,
                        &MPIDI_MVP_REQUEST(sreq, dev)
                             .iov[MPIDI_MVP_REQUEST(sreq, dev).iov_offset],
                        MPIDI_MVP_REQUEST(sreq, dev).iov_count -
                            MPIDI_MVP_REQUEST(sreq, dev).iov_offset,
                        1, &nb);
                }
            }

            if (MPI_SUCCESS != mpi_errno) {
                vc->ch.state = MPIDI_MVP_VC_STATE_FAILED;
                sreq->status.MPI_ERROR = MPI_ERR_INTERN;
                MPID_Request_complete(sreq);
                return mpi_errno;
            }

            if (nb > 0) {
                PRINT_DEBUG(DEBUG_RNDV_verbose > 1,
                            "Wrote R3 data, dest: %d, sreq: %p, bytes: %d\n",
                            vc->pg_rank, sreq, nb);
                if (MPIDI_MVP_Request_adjust_iov(sreq, nb)) {
                    MPIDI_MVP_Handle_send_req(vc, sreq, &complete);
                    if (complete) {
                        MPIDI_MVP_REQUEST(sreq, mrail).nearly_complete = 1;
                        break;
                    } else {
                        vc->smp.send_current_pkt_type = SMP_RNDV_MSG_CONT;
                    }
                } else {
                    MPIDI_MVP_REQUEST(sreq, ch).reqtype = REQUEST_RNDV_R3_DATA;
                    MPIR_Assert(vc->smp.send_active == NULL);
                    MPIDI_MVP_SMP_SendQ_enqueue_head(vc, sreq);
                    MPIDI_MVP_REQUEST(sreq, mrail).nearly_complete = 1;
                    vc->smp.send_current_pkt_type = SMP_RNDV_MSG_CONT;
                    MVP_INC_NUM_POSTED_SEND();
                    PRINT_DEBUG(DEBUG_RNDV_verbose > 1,
                                "Enqueue next R3 data, dest: %d, sreq: %p\n",
                                vc->pg_rank, sreq);
                    break;
                }
            } else {
                MPIDI_MVP_REQUEST(sreq, ch).reqtype = REQUEST_RNDV_R3_DATA;
                MPIR_Assert(vc->smp.send_active == NULL);
                MPIDI_MVP_SMP_SendQ_enqueue_head(vc, sreq);
                MPIDI_MVP_REQUEST(sreq, mrail).nearly_complete = 1;
                PRINT_DEBUG(DEBUG_RNDV_verbose > 1,
                            "Enqueue R3 data, dest: %d, sreq: %p\n",
                            vc->pg_rank, sreq);
                break;
            }
        }
    } else {
        MPIDI_MVP_REQUEST(sreq, ch).reqtype = REQUEST_RNDV_R3_DATA;
        MVP_INC_NUM_POSTED_SEND();
        MPIDI_MVP_SMP_SendQ_enqueue(vc, sreq);
        MPIDI_MVP_REQUEST(sreq, mrail).nearly_complete = 1;
        PRINT_DEBUG(DEBUG_RNDV_verbose > 1,
                    "Enqueue R3 data, dest: %d, sreq: %p\n", vc->pg_rank, sreq);
    }

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_RNDV_PUSH);
    return MPI_SUCCESS;
}

int MPIDI_MVP_Prepare_rndv_cts(MPIDI_MVP_ep_t *vc,
                               MPIDI_MVP_Pkt_rndv_clr_to_send_t *cts_pkt,
                               MPIR_Request *rreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_PREPARE_RNDV_CTS);

#ifdef CKPT
    MPIDI_MVP_CR_lock();
#endif

    if (IS_VC_SMP(vc) && cts_pkt->type == MPIDI_MVP_PKT_RMA_RNDV_CLR_TO_SEND) {
        MPIDI_MVP_REQUEST(rreq, mrail).protocol = MVP_RNDV_PROTOCOL_R3;
        cts_pkt->rndv.protocol = MVP_RNDV_PROTOCOL_R3;
    }

    switch (MPIDI_MVP_REQUEST(rreq, mrail).protocol) {
        case MVP_RNDV_PROTOCOL_R3: {
            cts_pkt->rndv.protocol = MVP_RNDV_PROTOCOL_R3;
            /*MRAILI_Prepost_R3(); */
            break;
        }
        case MVP_RNDV_PROTOCOL_RPUT: {
            MPIDI_MVP_MRAIL_Prepare_rndv(vc, rreq);
            MPIDI_MVP_MRAIL_SET_PKT_RNDV(cts_pkt, rreq);
            MPIDI_MVP_MRAIL_REVERT_RPUT(MPIDI_MVP_REQUEST_FROM_MPICH(rreq));
            break;
        }
        case MVP_RNDV_PROTOCOL_RGET: {
            int rank = MPIR_Process.rank;
            fprintf(stderr, "[%d][%s:%d] ", rank, __FILE__, __LINE__);
            fprintf(stderr, "RGET preparing CTS?\n");
            mpi_errno = -1;
            break;
        }
#ifdef _ENABLE_UD_
        case MVP_RNDV_PROTOCOL_UD_ZCOPY: {
            int hca_num = 0;
            MPIDI_MVP_MRAIL_Prepare_rndv_zcopy(vc, rreq);
            MPIDI_MVP_MRAIL_SET_PKT_RNDV(cts_pkt, rreq);
            if (MPIDI_MVP_REQUEST(rreq, mrail).protocol ==
                MVP_RNDV_PROTOCOL_UD_ZCOPY) {
                for (hca_num = 0; hca_num < rdma_num_hcas; ++hca_num) {
                    cts_pkt->rndv.rndv_qpn[hca_num] =
                        MPIDI_MVP_REQUEST(rreq, mrail)
                            .rndv_qp_entry->ud_qp[hca_num]
                            ->qp_num;
                    PRINT_DEBUG(DEBUG_ZCY_verbose > 0,
                                "My qpn: %d, hca_index: %d\n",
                                cts_pkt->rndv.rndv_qpn[hca_num], hca_num);
                }
            }
            break;
        }
#endif
        default: {
            int rank = MPIR_Process.rank;
            fprintf(stderr, "[%d][%s:%d] ", rank, __FILE__, __LINE__);
            fprintf(stderr, "Unknown protocol %d type from rndv req to send\n",
                    MPIDI_MVP_REQUEST(rreq, mrail).protocol);
            mpi_errno = -1;
            break;
        }
    }

#ifdef CKPT
    MPIDI_MVP_CR_unlock();
#endif

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_PREPARE_RNDV_CTS);
    return mpi_errno;
}

int MPIDI_MVP_iStartMsg(MPIDI_MVP_ep_t *vc, void *pkt, intptr_t pkt_sz,
                        MPIR_Request **sreq_ptr)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Request *sreq = NULL;
    struct iovec iov[1];

    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_ISTARTMSG);

    // MPL_DBG_MSG_S(MPIDI_MVP_DBG_CHANNEL,VERBOSE,"entering %s", __func__);

    /* If send queue is empty attempt to send
       data, queuing any unsent data. */
    if (mvp_smp_init && vc->smp.local_nodes >= 0 &&
        vc->smp.local_nodes != mvp_smp_info.my_local_id) {
        mpi_errno = MPIDI_MVP_SMP_iStartMsg(vc, pkt, pkt_sz, sreq_ptr);
        // MPL_DBG_MSG_S(MPIDI_MVP_DBG_CHANNEL,VERBOSE,"exiting %s", __func__);
        MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_ISTARTMSG);
        return (mpi_errno);
    }

#if 0
/* TODO: FIXME: Work on SMP section first, then move from there */
#ifdef CKPT
    /*Detect whether the packet is CTS*/
    MPIDI_MVP_Pkt_t *upkt = (MPIDI_MVP_Pkt_t *)pkt;
    if (upkt->type == MPIDI_MVP_PKT_RNDV_CLR_TO_SEND) {
        MPIDI_MVP_Pkt_rndv_clr_to_send_t * cts_pkt = &(upkt->rndv_clr_to_send);
        if (cts_pkt->rndv.protocol == MVP_RNDV_PROTOCOL_RPUT) {
            /*If using rput protocol, keep track of the request*/
            MPIR_Request *rreq;
            MPIR_Request_get_ptr(cts_pkt->receiver_req_id, rreq);
            MPIDI_MVP_CR_req_enqueue(rreq, vc);
        }
    }
#endif

    /*CM code*/
    if ((vc->ch.state != MPIDI_MVP_VC_STATE_IDLE
#ifdef _ENABLE_XRC_
            || (USE_XRC && VC_XST_ISUNSET (vc, XF_SEND_IDLE))
#endif
            ) || !MPIDI_MVP_CM_SendQ_empty(vc)) {
        /*Request need to be queued*/
        //MPL_DBG_MSG(MPIDI_MVP_DBG_CHANNEL,VERBOSE,"not connected, enqueueing");
        sreq = create_request(pkt, pkt_sz, 0);
        MPIDI_MVP_CM_SendQ_enqueue(vc, sreq);
        if (vc->ch.state == MPIDI_MVP_VC_STATE_UNCONNECTED)  {
            MPIDI_MVP_CM_Connect(vc);
        }
        goto fn_exit;
    }

    if (MPIDI_MVP_SendQ_empty(vc)) {   /* MT */
        int nb;
        int pkt_len;
        vbuf *buf;

        /* MT - need some signalling to lock down our right to use the
           channel, thus insuring that the progress engine does also try to
           write */

        iov[0].iov_base = pkt;
        iov[0].iov_len = pkt_sz;
        pkt_len = pkt_sz;

        /* TODO: Codes to send pkt through send/recv path */
        mpi_errno =
            MPIDI_MVP_MRAILI_Eager_send(vc, iov, 1, pkt_len, &nb, &buf);
        DEBUG_PRINT("[istartmsgv] mpierr %d, nb %d\n", mpi_errno, nb);

        if (mpi_errno == MPI_SUCCESS) {
            DEBUG_PRINT("[send path] eager send return %d bytes\n", nb);
            goto fn_exit;
        } else if (MPI_MRAIL_MSG_QUEUED == mpi_errno) {
            /* fast rdma ok but cannot send: there is no send wqe available */
            /* sreq = create_request(pkt, pkt_sz, 0);
            buf->sreq = (void *) sreq;   */ 
            mpi_errno = MPI_SUCCESS;
            goto fn_exit;
        } else {
            sreq = MPIR_Request_create(MPIR_REQUEST_KIND__SEND);
            if (sreq == NULL) {
                mpi_errno =
                    MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_FATAL,
                                         __func__, __LINE__,
                                         MPI_ERR_OTHER, "**nomem", 0);
                goto fn_exit;
            }
            MVP_INC_NUM_POSTED_SEND();
            MPIR_cc_set(&sreq->cc, 0);
            /* TODO: Create an appropriate error message based on the value of errno
             * */
            sreq->status.MPI_ERROR = MPI_ERR_INTERN;
            PRINT_DEBUG(DEBUG_SHM_verbose>1,
                    "Enqueue send to rank: %d, sreq: %p, type: %d, ch.reqtype: %d\n",
                    vc->pg_rank, sreq, MPIDI_Request_get_type(sreq), sreq->ch.reqtype);
        }
    } else {
        sreq = create_request(pkt, pkt_sz, 0);
        MPIDI_MVP_SendQ_enqueue(vc, sreq);
        PRINT_DEBUG(DEBUG_SHM_verbose>1,
                "Eqnueue send to rank: %d, sreq: %p, type: %d, ch.reqtype: %d\n",
                vc->pg_rank, sreq, MPIDI_Request_get_type(sreq), sreq->ch.reqtype);
    }
#endif
fn_exit:
    *sreq_ptr = sreq;
#ifdef CKPT
    MPIDI_MVP_CR_unlock();
#endif

    DEBUG_PRINT("Exiting istartmsg\n");
    // MPL_DBG_MSG_S(MPIDI_MVP_DBG_CHANNEL,VERBOSE,"exiting %s",__func__);
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_ISTARTMSG);
    return mpi_errno;
}

int MPIDI_MVP_SMP_iStartMsg(MPIDI_MVP_ep_t *vc, void *pkt, intptr_t pkt_sz,
                            MPIR_Request **sreq_ptr)
{
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_ISTARTMSG);
    int mpi_errno = MPI_SUCCESS;
    MPIR_Request *sreq = NULL;
    struct iovec iov[1];
    MPIDI_MVP_Pkt_send_t *pkt_header;

    DEBUG_PRINT("entering ch3_istartmsg\n");

    pkt_header = (MPIDI_MVP_Pkt_send_t *)pkt;

    /* If send queue is empty attempt to send
       data, queuing any unsent data. */
    if (MPIDI_MVP_SMP_SendQ_empty(vc)) { /* MT */
        int nb;

        /* MT - need some signalling to lock down our right to use the
           channel, thus insuring that the progress engine does also try to
           write */

        iov[0].iov_base = pkt;
        iov[0].iov_len = pkt_sz;

        if (pkt_header->type == MPIDI_MVP_PKT_RNDV_R3_DATA) {
            MPIDI_MVP_smp_write_rndv_header(vc->smp.local_rank, iov, 1, &nb);
        } else {
            MPIDI_MVP_smp_write(vc->smp.local_rank, iov, 1, &nb);
        }
#ifdef CKPT
        MPIDI_MVP_MRAILI_Pkt_comm_header *p =
            (MPIDI_MVP_MRAILI_Pkt_comm_header *)pkt;
        if (p->type >= MPIDI_MVP_PKT_CM_SUSPEND &&
            p->type <= MPIDI_MVP_PKT_CR_REMOTE_UPDATE) {
            DEBUG_PRINT(
                "%s [%d => %d]: imm-write pkt %s(%d), ret nb=%d,pkt-size=%d\n",
                __func__, MPIDI_Process.my_pg_rank, vc->pg_rank,
                MPIDI_MVP_Pkt_type_to_string[p->type], p->type, nb, pkt_sz);
        }
#endif // CKPT
        if (nb != pkt_sz) {
            sreq = create_request(pkt, pkt_sz, nb);
            if (pkt_header->type == MPIDI_MVP_PKT_RNDV_R3_DATA) {
                MPIDI_MVP_REQUEST(sreq, ch).reqtype = REQUEST_RNDV_R3_HEADER;
            }

            MPIR_Assert(vc->smp.send_active == NULL);
            MPIDI_MVP_SMP_SendQ_enqueue_head(vc, sreq);

            PRINT_DEBUG(DEBUG_SHM_verbose > 1,
                        "send to %d delayed, request enqueued: %p, type: %d, "
                        "pkt_sz: %ld, ch.reqtype: %d\n",
                        vc->pg_rank, sreq, MPIDI_Request_get_type(sreq), pkt_sz,
                        MPIDI_MVP_REQUEST(sreq, ch).reqtype);
        }
#if defined(DEBUG)
        else {
            DEBUG_PRINT("data sent immediately.\n");
        }

        /* Free tmp_buf for packed ddt */
        if (MPIDI_MVP_REQUEST(sreq, dev).iov[0].iov_base ==
            MPIDI_MVP_REQUEST(sreq, dev).tmpbuf) {
            MPL_free(MPIDI_MVP_REQUEST(sreq, dev).iov[0].iov_base);
        }
#endif /* defined(DEBUG) */
    } else {
        sreq = create_request(pkt, pkt_sz, 0);
        if (pkt_header->type == MPIDI_MVP_PKT_RNDV_R3_DATA) {
            MPIDI_MVP_REQUEST(sreq, ch).reqtype = REQUEST_RNDV_R3_HEADER;
        }

        MPIDI_MVP_SMP_SendQ_enqueue(vc, sreq);
        PRINT_DEBUG(DEBUG_SHM_verbose > 1,
                    "send to %d delayed, request enqueued: %p, type: %d, "
                    "pkt_sz: %ld, ch.reqtype: %d\n",
                    vc->pg_rank, sreq, MPIDI_Request_get_type(sreq), pkt_sz,
                    MPIDI_MVP_REQUEST(sreq, ch).reqtype);
#ifdef CKPT
        MPIDI_MVP_MRAILI_Pkt_comm_header *p =
            (MPIDI_MVP_MRAILI_Pkt_comm_header *)pkt;
        if (p->type >= MPIDI_MVP_PKT_CM_SUSPEND &&
            p->type <= MPIDI_MVP_PKT_CR_REMOTE_UPDATE) {
            DEBUG_PRINT("%s [%d => %d]: Enqueue:  pkt %s(%d), pkt-size=%d\n",
                        __func__, MPIDI_Process.my_pg_rank, vc->pg_rank,
                        MPIDI_MVP_Pkt_type_to_string[p->type], p->type, pkt_sz);
        }
#endif // end of CKPT
    }

fn_exit:
    *sreq_ptr = sreq;
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_ISTARTMSG);
    return mpi_errno;

#ifndef CHANNEL_MRAIL
fn_fail:
#endif
    goto fn_exit;
}

int MPIDI_MVP_smp_iStartMsgv(MPIDI_MVP_ep_t *vc, struct iovec *iov, int n_iov,
                             MPIR_Request **sreq_ptr)
{
    MPIR_Request *sreq = NULL;
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_SMP_ISTARTMSGV);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_ISTARTMSGV);

    DEBUG_PRINT("entering ch3_smp_istartmsgv\n");

    /* If send queue is empty attempt to send
       data, queuing any unsent data. */
    if (MPIDI_MVP_SMP_SendQ_empty(vc)) {
        int nb;
        /* MT - need some signalling to lock down our right to use the
           channel, thus insuring that the progress engine does also try to
           write */
        MPIDI_MVP_smp_write(vc->smp.local_rank, iov, n_iov, &nb);
        int offset = 0;
        DEBUG_PRINT("ch3_smp_istartmsgv: writev returned %d bytes\n", nb);

        while (offset < n_iov) {
            if (nb >= (int)iov[offset].iov_len) {
                nb -= iov[offset].iov_len;
                ++offset;
            } else {
                DEBUG_PRINT("ch3_istartmsgv: shm_writev did not complete the "
                            "send, allocating request\n");
                sreq = create_requestv(iov, n_iov, offset, nb);
                MPIDI_MVP_SMP_SendQ_enqueue_head(vc, sreq);
                break;
            }
        }
    } else {
        sreq = create_requestv(iov, n_iov, 0, 0);
        MPIDI_MVP_SMP_SendQ_enqueue(vc, sreq);
    }

    *sreq_ptr = sreq;
fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_ISTARTMSGV);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIDI_MVP_MRAIL_Prepare_rndv_transfer(MPIR_Request *sreq,
                                          /* contains local info */
                                          MPIDI_MVP_MRAILI_Rndv_info_t *rndv)
{
    int hca_index;

    if (rndv->protocol == MVP_RNDV_PROTOCOL_R3) {
        //    if (MPIDI_MVP_REQUEST(sreq, mrail).d_entry != NULL) {
        //        dreg_unregister(MPIDI_MVP_REQUEST(sreq, mrail).d_entry);
        //        MPIDI_MVP_REQUEST(sreq, mrail).d_entry = NULL;
        //    }
        if (1 == MPIDI_MVP_REQUEST(sreq, mrail).rndv_buf_alloc &&
            NULL != MPIDI_MVP_REQUEST(sreq, mrail).rndv_buf) {
            MPL_free(MPIDI_MVP_REQUEST(sreq, mrail).rndv_buf);
            MPIDI_MVP_REQUEST(sreq, mrail).rndv_buf_alloc = 0;
            MPIDI_MVP_REQUEST(sreq, mrail).rndv_buf = NULL;
        }
        MPIDI_MVP_REQUEST(sreq, mrail).remote_addr = NULL;
        /* Initialize this completion counter to 0
         * required for even striping */
        MPIDI_MVP_REQUEST(sreq, mrail).local_complete = 0;

        /* revise later
        for (hca_index = 0; hca_index < rdma_num_hcas; hca_index ++)
            sreq->mrail.rkey[hca_index] = 0;
            */
        MPIDI_MVP_REQUEST(sreq, mrail).protocol = MVP_RNDV_PROTOCOL_R3;
    } else {
#ifdef _ENABLE_UD_
        int hca_num = 0;
        if (rndv->protocol == MVP_RNDV_PROTOCOL_UD_ZCOPY) {
            for (hca_num = 0; hca_num < rdma_num_hcas; ++hca_num) {
                PRINT_DEBUG(DEBUG_ZCY_verbose > 0,
                            "Received CTS.remote qpn: %d, hca_index: %d\n",
                            rndv->rndv_qpn[hca_num], hca_num);
                sreq->mrail.remote_qpn[hca_num] = rndv->rndv_qpn[hca_num];
            }
        }
        /* TODO: Can we avoid dev.iov copy for zcopy */
#endif
        {
            MPIDI_MVP_REQUEST(sreq, mrail).remote_addr = rndv->buf_addr;
            for (hca_index = 0; hca_index < rdma_num_hcas; hca_index++) {
                MPIDI_MVP_REQUEST(sreq, mrail).rkey[hca_index] =
                    rndv->rkey[hca_index];
            }
        }

        DEBUG_PRINT("[add rndv list] addr %p, key %p\n",
                    MPIDI_MVP_REQUEST(sreq, mrail).remote_addr,
                    MPIDI_MVP_REQUEST(sreq, mrail).rkey[0]);
        if (1 == MPIDI_MVP_REQUEST(sreq, mrail).rndv_buf_alloc) {
            int mpi_errno = MPI_SUCCESS;
            int i;
            uintptr_t buf;

            buf = (uintptr_t)MPIDI_MVP_REQUEST(sreq, mrail).rndv_buf;
            for (i = 0; i < MPIDI_MVP_REQUEST(sreq, dev).iov_count; i++) {
                MPIR_Memcpy((void *)buf,
                            MPIDI_MVP_REQUEST(sreq, dev).iov[i].iov_base,
                            MPIDI_MVP_REQUEST(sreq, dev).iov[i].iov_len);
                buf += MPIDI_MVP_REQUEST(sreq, dev).iov[i].iov_len;
            }

            /* TODO: Following part is a workaround to deal with
             * datatype with large number of segments.
             * We check if the datatype has finished
             * loading and reload if not.
             * May be better interface with
             * upper layer should be considered */

            //            while (MPIDI_MVP_REQUEST(sreq, dev).OnDataAvail ==
            //                    MPIDI_MVP_ReqHandler_SendReloadIOV) {
            //                MPIDI_MVP_REQUEST(sreq, dev).iov_count =
            //                MPL_IOV_LIMIT;
            //                /*
            //                mpi_errno =
            //                    MPIDI_MVP_Request_load_send_iov(sreq,
            //                            MPIDI_MVP_REQUEST(sreq, dev).iov,
            //                            &sreq->dev.iov_count);*/
            //                /* --BEGIN ERROR HANDLING-- */
            //                if (mpi_errno != MPI_SUCCESS) {
            //                    ibv_error_abort(IBV_STATUS_ERR, "Reload iov
            //                    error");
            //                }
            //                for (i = 0; i < MPIDI_MVP_REQUEST(sreq,
            //                dev).iov_count; i++) {
            //                   MPIR_Memcpy((void *) buf,
            //                   MPIDI_MVP_REQUEST(sreq, dev).iov[i].iov_base,
            //                            MPIDI_MVP_REQUEST(sreq,
            //                            dev).iov[i].iov_len);
            //                    buf += MPIDI_MVP_REQUEST(sreq,
            //                    dev).iov[i].iov_len;
            //                }
            //            }
        }
    }
    return MPI_SUCCESS;
}

/* TODO: simplify args here, this is silly */
int MPIDI_MVP_Rndv_transfer(MPIDI_MVP_ep_t *vc, MPIR_Request *sreq,
                            MPIR_Request *rreq,
                            MPIDI_MVP_Pkt_rndv_clr_to_send_t *cts_pkt,
                            MPIDI_MVP_Pkt_rndv_req_to_send_t *rts_pkt)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_MVP_MRAILI_Rndv_info_t *rndv; /* contains remote info */
    MPIR_Request *req;
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_RNDV_TRANSFER);

    DEBUG_PRINT("Get rndv reply, add to list\n");

    /* This function can adapt to either read
     * or write based on the value of sreq or
     * rreq. */
    if (sreq) {
        req = sreq;
    } else {
        req = rreq;
    }

    switch (MPIDI_MVP_REQUEST(req, mrail.protocol)) {
        case MVP_RNDV_PROTOCOL_RPUT:
            rndv = (cts_pkt == NULL) ? NULL : &cts_pkt->rndv;
            MPIDI_MVP_REQUEST(sreq, mrail).partner_id =
                cts_pkt->receiver_req_id;
            MPIDI_MVP_MRAIL_Prepare_rndv_transfer(sreq, rndv);
            break;
        case MVP_RNDV_PROTOCOL_R3:
            rndv = (cts_pkt == NULL) ? NULL : &cts_pkt->rndv;
            MPIDI_MVP_REQUEST(sreq, mrail).partner_id =
                cts_pkt->receiver_req_id;
            MPIR_Assert(rndv->protocol == MVP_RNDV_PROTOCOL_R3);
            break;
        case MVP_RNDV_PROTOCOL_RGET:
            rndv = (rts_pkt == NULL) ?
                       ((cts_pkt == NULL) ? NULL : &cts_pkt->rndv) :
                       &rts_pkt->rndv;
            MPIR_Assert(rndv != NULL);
            if (sreq != NULL && cts_pkt != NULL)
                MPIDI_MVP_REQUEST(sreq, mrail).partner_id =
                    cts_pkt->receiver_req_id;
            MPIDI_MVP_MRAIL_Prepare_rndv_transfer(req, rndv);
            break;
        default:
            mpi_errno = MPIR_Err_create_code(0, MPIR_ERR_FATAL, __func__,
                                             __LINE__, MPI_ERR_OTHER, "**fail",
                                             "**fail %s", "unknown protocol");
            MPIR_ERR_POP(mpi_errno);
    }

    RENDEZVOUS_IN_PROGRESS(vc, req);
    /*
     * this is where all rendezvous transfers are started,
     * so it is the only place we need to set this kludgy
     * field
     */

    MPIDI_MVP_REQUEST(req, mrail).nearly_complete = 0;

    PUSH_FLOWLIST(vc);

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_RNDV_TRANSFER);
    return MPI_SUCCESS;
fn_fail:
    goto fn_exit;
}

/*
 * Here are the routines that are called by the progress engine to handle
 * the various rendezvous message requests (cancel of sends is in
 * mpid_cancel_send.c).
 */

/* MVP optimized SMP RGET design */
int MPIDI_MVP_PktHandler_SMP_CTS(MPIDI_MVP_ep_t *vc, MPIDI_MVP_Pkt_t *pkt,
                                 intptr_t *buflen, MPIR_Request **rreqp)
{
    int i, mpi_errno = MPI_SUCCESS;
    MPIDI_MVP_Pkt_rndv_clr_to_send_t *cts_pkt = &pkt->rndv_clr_to_send;
    MPIR_Request *sreq;
    intptr_t recv_size;

    MPIR_Assert(pkt->type == MPIDI_MVP_PKT_RNDV_CLR_TO_SEND);
    MPIR_Request_get_ptr(cts_pkt->sender_req_id, sreq);
    MPIR_Assert(sreq != NULL);

    recv_size = cts_pkt->recv_sz;
    for (i = 0; i < MPIDI_MVP_REQUEST(sreq, dev).iov_count; i++) {
        if (recv_size < MPIDI_MVP_REQUEST(sreq, dev).iov[i].iov_len) {
            fprintf(stderr,
                    "Warning! Rndv Receiver is expecting %lu Bytes "
                    "But, is receiving %lu Bytes \n",
                    MPIDI_MVP_REQUEST(sreq, dev).iov[i].iov_len, recv_size);
            MPIDI_MVP_REQUEST(sreq, dev).iov[i].iov_len = recv_size;
            MPIDI_MVP_REQUEST(sreq, dev).iov_count = i + 1;
            break;
        } else {
            recv_size -= MPIDI_MVP_REQUEST(sreq, dev).iov[i].iov_len;
        }
    }
    MPIDI_MVP_REQUEST(sreq, mrail).rndv_buf_sz = cts_pkt->recv_sz;
    MPIDI_MVP_REQUEST(sreq, mrail).protocol = cts_pkt->rndv.protocol;

    mpi_errno = MPIDI_MVP_Rndv_transfer(vc, sreq, NULL, cts_pkt, NULL);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER, "**ch3|senddata");
    }
    *rreqp = NULL;
    *buflen = sizeof(MPIDI_MVP_Pkt_t);

fn_fail:
    return mpi_errno;
}

#ifdef MPICH_DBG_OUTPUT
int MPIDI_MVP_PktPrint_RndvClrToSend(FILE *fp, MPIDI_MVP_Pkt_t *pkt)
{
    // MPL_DBG_MSG(MPIDI_MVP_DBG_OTHER,TERSE," type ......... CLR_TO_SEND\n");
    // MPL_DBG_MSG_FMT(MPIDI_MVP_DBG_OTHER,TERSE,(MPL_DBG_FDEST," sender_reqid .
    // 0x%08X\n", pkt->rndv_clr_to_send.sender_req_id));
    // MPL_DBG_MSG_FMT(MPIDI_MVP_DBG_OTHER,TERSE,(MPL_DBG_FDEST," recvr_reqid ..
    // 0x%08X\n", pkt->rndv_clr_to_send.receiver_req_id));
    return MPI_SUCCESS;
}
#endif
