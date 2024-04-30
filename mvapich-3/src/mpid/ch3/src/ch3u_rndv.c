/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */
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

#include "mpidimpl.h"
#ifdef CHANNEL_MRAIL
#include "mpid_mrail_rndv.h"
#include "mpidi_ch3_impl.h"
#endif


extern unsigned long long PVAR_COUNTER_expected_recvs_rendezvous;

/*
 * This file contains the implementation of the rendezvous protocol
 * for MPI point-to-point messaging.
 */

static inline int mvp_is_dt_contig(MPIR_Request *req) {
    int dt_contig = 1;
    if (req->dev.datatype &&
            HANDLE_GET_KIND(req->dev.datatype) != HANDLE_KIND_BUILTIN) {
        MPIR_Datatype * dt_ptr = NULL;
        MPIR_Datatype_get_ptr(req->dev.datatype, dt_ptr);
        dt_contig = dt_ptr->is_contig;
    }
    return dt_contig;
}

/* MPIDI_CH3_RndvSend - Send a request to perform a rendezvous send */
int MPIDI_CH3_RndvSend( MPIR_Request **sreq_p, const void * buf, MPI_Aint count,
            MPI_Datatype datatype, int dt_contig, intptr_t data_sz,
            MPI_Aint dt_true_lb,
            int rank, 
            int tag, MPIR_Comm * comm, int context_offset )
{
    MPIDI_CH3_Pkt_t upkt;
    MPIDI_CH3_Pkt_rndv_req_to_send_t * const rts_pkt = &upkt.rndv_req_to_send;
    MPIDI_VC_t * vc;
#ifndef MPIDI_CH3_CHANNEL_RNDV
    MPIR_Request * rts_sreq;
#endif
    MPIR_Request *sreq =*sreq_p;
    int          mpi_errno = MPI_SUCCESS;
#if defined(CHANNEL_MRAIL) && defined(MPID_USE_SEQUENCE_NUMBERS)
    MPID_Seqnum_t seqnum;
#endif /* defined(CHANNEL_MRAIL) && defined(MPID_USE_SEQUENCE_NUMBERS) */
    
    MPL_DBG_MSG_D(MPIDI_CH3_DBG_OTHER,VERBOSE,
           "sending rndv RTS, data_sz=%" PRIdPTR, data_sz);

    sreq->dev.OnDataAvail = 0;
    
    sreq->dev.partner_request = NULL;
    
    MPIDI_Pkt_init(rts_pkt, MPIDI_CH3_PKT_RNDV_REQ_TO_SEND);
    rts_pkt->match.parts.rank         = comm->rank;
    rts_pkt->match.parts.tag          = tag;
    rts_pkt->match.parts.context_id = comm->context_id + context_offset;
    rts_pkt->sender_req_id    = sreq->handle;
    rts_pkt->data_sz          = data_sz;

    MPIDI_Comm_get_vc_set_active(comm, rank, &vc);
    MPIDI_VC_FAI_send_seqnum(vc, seqnum);
    MPIDI_Pkt_set_seqnum(rts_pkt, seqnum);
    MPIDI_Request_set_seqnum(sreq, seqnum);

    MPL_DBG_MSGPKT(vc,tag,rts_pkt->match.parts.context_id,rank,data_sz,"Rndv");

#ifdef MPIDI_CH3_CHANNEL_RNDV

    MPL_DBG_MSG(MPIDI_CH3_DBG_CHANNEL,VERBOSE,"Rendezvous send using iStartRndvMsg");
    
    if (dt_contig) 
    {
        MPL_DBG_MSG_D(MPIDI_CH3_DBG_CHANNEL,VERBOSE,
                      "contiguous rndv data, data_sz=%" PRIdPTR, data_sz);
            
        sreq->dev.OnDataAvail = 0;
        
        sreq->dev.iov[0].iov_base = (void *) ((char*)sreq->dev.user_buf + dt_true_lb);
        sreq->dev.iov[0].iov_len = data_sz;
        sreq->dev.iov_count = 1;
    } else {
        sreq->dev.msg_offset = 0;
        sreq->dev.msgsize = data_sz;

        /* One the initial load of a send iov req, set the OnFinal action (null
           for point-to-point) */
        sreq->dev.OnFinal = 0;
        mpi_errno = MPIDI_CH3U_Request_load_send_iov(sreq, &sreq->dev.iov[0],
                                 &sreq->dev.iov_count);
        /* --BEGIN ERROR HANDLING-- */
        if (mpi_errno != MPI_SUCCESS)
        {
            mpi_errno = MPIR_Err_create_code(mpi_errno, MPIR_ERR_FATAL,
                             __func__, __LINE__, MPI_ERR_OTHER,
                             "**ch3|loadsendiov", 0);
            goto fn_fail;
        }
        /* --END ERROR HANDLING-- */
    }
    mpi_errno = MPIDI_CH3_iStartRndvMsg (vc, sreq, &upkt);
    /* --BEGIN ERROR HANDLING-- */
    if (mpi_errno != MPI_SUCCESS)
    {
        MPIR_Object_set_ref(sreq, 0);
        MPIR_Request_free(sreq);
        *sreq_p = NULL;
        mpi_errno = MPIR_Err_create_code(mpi_errno, MPIR_ERR_FATAL,
                         __func__, __LINE__, MPI_ERR_OTHER,
                         "**ch3|rtspkt", 0);
        goto fn_fail;
    }
    /* --END ERROR HANDLING-- */
    
#else

    MPID_THREAD_CS_ENTER(POBJ, vc->pobj_mutex);
    mpi_errno = MPIDI_CH3_iStartMsg(vc, rts_pkt, sizeof(*rts_pkt), &rts_sreq);
    MPID_THREAD_CS_EXIT(POBJ, vc->pobj_mutex);
    /* --BEGIN ERROR HANDLING-- */
    if (mpi_errno != MPI_SUCCESS)
    {
        MPIR_Request_free(sreq);
    *sreq_p = NULL;
        MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER, "**ch3|rtspkt");
    }
    /* --END ERROR HANDLING-- */
    if (rts_sreq != NULL)
    {
    if (rts_sreq->status.MPI_ERROR != MPI_SUCCESS)
    {
            MPIR_Request_free(sreq);
        *sreq_p = NULL;
            mpi_errno = rts_sreq->status.MPI_ERROR;
            MPIR_Request_free(rts_sreq);
            MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER, "**ch3|rtspkt");
    }
    MPIR_Request_free(rts_sreq);
    }
#endif

    /* FIXME: fill temporary IOV or pack temporary buffer after send to hide 
       some latency.  This requires synchronization
       because the CTS packet could arrive and be processed before the above 
       iStartmsg completes (depending on the progress
       engine, threads, etc.). */

 fn_exit:
    return mpi_errno;
 fn_fail:
    goto fn_exit;
}

/* 
 * Here are the routines that are called by the progress engine to handle
 * the various rendezvous message requests (cancel of sends is in 
 * mpid_cancel_send.c).
 */    

#define set_request_info(rreq_, pkt_, msg_type_)        \
{                               \
    (rreq_)->status.MPI_SOURCE = (pkt_)->match.parts.rank;  \
    (rreq_)->status.MPI_TAG = (pkt_)->match.parts.tag;      \
    MPIR_STATUS_SET_COUNT((rreq_)->status, (pkt_)->data_sz);        \
    (rreq_)->dev.sender_req_id = (pkt_)->sender_req_id;     \
    (rreq_)->dev.recv_data_sz = (pkt_)->data_sz;        \
    MPIDI_Request_set_seqnum((rreq_), (pkt_)->seqnum);      \
    MPIDI_Request_set_msg_type((rreq_), (msg_type_));       \
}

#if defined(CHANNEL_MRAIL)
void mvp_select_rndv_protocol(
        MPIDI_VC_t *vc,
        MPIR_Request *rreq,
        MPIDI_CH3_Pkt_rndv_req_to_send_t * rts_pkt)
{
    int protocol = rts_pkt->rndv.protocol;
    int dt_contig = mvp_is_dt_contig(rreq);

    if (protocol == MRAILI_PROTOCOL_R3 || rreq->dev.iov_count > 1 ||
        !dt_contig) {
        /* Fallback to R3 for non-contiguous transfers */
        protocol = MRAILI_PROTOCOL_R3;
    }
    rreq->mrail.protocol = rts_pkt->rndv.protocol = protocol;
}

/* MVP optimized SMP RGET design */
int MPIDI_CH3_PktHandler_SMP_RTS( MPIDI_VC_t *vc, MPIDI_CH3_Pkt_t *pkt,
                    intptr_t *buflen, MPIR_Request **rreqp )
{
    int found, k;
    int mpi_errno = MPI_SUCCESS;
    MPIR_Request * rreq;
    MPID_Seqnum_t seqnum;
    MPIDI_CH3_Pkt_rndv_req_to_send_t * rts_pkt = &pkt->rndv_req_to_send;

    MPID_THREAD_CS_ENTER(POBJ, MPIR_THREAD_POBJ_MSGQ_MUTEX);
    if (*rreqp == NULL) {
        rreq = MPIDI_CH3U_Recvq_FDP_or_AEU(&rts_pkt->match, &found);
        MPIR_ERR_CHKANDJUMP1(!rreq, mpi_errno,MPI_ERR_OTHER, "**nomemreq", "**nomemuereq %d", MPIDI_CH3U_Recvq_count_unexp());

        if (found) {
            PRINT_DEBUG(DEBUG_RNDV_verbose>1, "posted request found, "
                    "rreq: %p, rank: %d, tag: %d, context: %d, protocol: %d, recv_sz: %ld\n",
                    rreq, rreq->dev.match.parts.rank, rreq->dev.match.parts.tag, 
                    rreq->dev.match.parts.context_id, rreq->mrail.protocol, rreq->dev.recv_data_sz);
        } else if (MPIR_cc_get(rreq->cc) == 0) {
            *rreqp = NULL;
            goto fn_fail;
        }

        set_request_info(rreq, rts_pkt, MPIDI_REQUEST_RNDV_MSG);
        MPID_THREAD_CS_EXIT(POBJ, MPIR_THREAD_POBJ_MSGQ_MUTEX);

        *buflen = sizeof(MPIDI_CH3_Pkt_t);
        mvp_select_rndv_protocol(vc, rreq, rts_pkt);
    } else {
        found = 1;
        rreq = *rreqp;
        *buflen = sizeof(MPIDI_CH3_Pkt_t);
        set_request_info(rreq, rts_pkt, MPIDI_REQUEST_RNDV_MSG);
        mvp_select_rndv_protocol(vc, rreq, rts_pkt);
        PRINT_DEBUG(DEBUG_RNDV_verbose>1,
                "Saved RTS, sreq: %08x, rank: %d, tag: %d, context: %d, protocol: %d, data_sz: %ld\n",
                rts_pkt->sender_req_id, rts_pkt->match.parts.rank, rts_pkt->match.parts.tag, 
                rts_pkt->match.parts.context_id, rts_pkt->rndv.protocol, rts_pkt->data_sz);
    }

    if (found) {
#if defined (_ENABLE_CUDA_)
        /* Fallback to R3 for heterogeneous transfer, i.e., H-D and D-H
         * TODO: may consider support RGET/RPUT for such case */
        if (mvp_enable_device && (rts_pkt->rndv.device_transfer_mode != rreq->mrail.device_transfer_mode)) {
            rreq->mrail.protocol = rts_pkt->rndv.protocol = MRAILI_PROTOCOL_R3;
        }
#if defined (HAVE_CUDA_IPC)
        /*initialize IPC buffers if not initialized*/
        if (mvp_enable_device &&
            mvp_device_use_ipc &&
            mvp_device_dynamic_init &&
            (rreq->mrail.device_transfer_mode != NONE ||
             rts_pkt->rndv.device_transfer_mode != NONE) &&
             vc->smp.can_access_peer == MVP_DEVICE_IPC_UNINITIALIZED) {
                if (mvp_device_initialized) {
                    device_ipc_init_dynamic (vc);
                }
        }
#endif /* defined (HAVE_CUDA_IPC) */
#endif /* defined (_ENABLE_CUDA_) */

        if (MRAILI_PROTOCOL_RGET == rts_pkt->rndv.protocol) {
            MPIDI_CH3I_MRAIL_SET_REQ_REMOTE_RNDV(rreq, rts_pkt);

            mpi_errno = MPIDI_CH3U_Post_data_receive_found(rreq);
            if (mpi_errno != MPI_SUCCESS && rreq != NULL) {
                MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER,"**ch3|loadsendiov");
            }

            mpi_errno = MPIDI_CH3_Prepare_rndv_get(vc, rreq);
            if (mpi_errno != MPI_SUCCESS) {
                MPIR_ERR_SETANDJUMP(mpi_errno,MPI_ERR_OTHER,"**ch3|rndv");
            }

#if defined(_ENABLE_CUDA_) && defined(HAVE_CUDA_IPC)
            if  (mvp_enable_device && mvp_device_use_ipc
                    && rts_pkt->rndv.device_transfer_mode != NONE) {
                /*revert to RGET if using IPC and rdma is possible*/
                if (MRAILI_PROTOCOL_RPUT == rreq->mrail.protocol) {
                    rreq->mrail.protocol = MRAILI_PROTOCOL_RGET;
                }
                if (MRAILI_PROTOCOL_RGET == rreq->mrail.protocol) {
                    if (MPIDI_CH3I_MRAIL_Rndv_transfer_device_ipc (vc, rreq, rts_pkt)) {
                        *rreqp = NULL;
                        goto fn_exit;
                    }
                }
            }
#endif

            if (!rdma_rndv_immediate) {
                RENDEZVOUS_IN_PROGRESS(vc, rreq);
                rreq->mrail.nearly_complete = 0;
                PUSH_FLOWLIST(vc);
            } else {
                mpi_errno = MPIDI_CH3_Rendezvous_push(vc, rreq);
                if (mpi_errno != MPI_SUCCESS && rreq != NULL) {
                    MPIR_ERR_SETANDJUMP(mpi_errno,
                            MPI_ERR_OTHER,"**ch3|senddata");
                }
            }
        }

        if (MRAILI_PROTOCOL_RPUT == rts_pkt->rndv.protocol ||
#if defined(_ENABLE_CUDA_) && defined(HAVE_CUDA_IPC)
            MRAILI_PROTOCOL_CUDAIPC == rts_pkt->rndv.protocol ||
#endif
            MRAILI_PROTOCOL_R3 == rts_pkt->rndv.protocol) {

            MPIR_Request * cts_req;
            MPIDI_CH3_Pkt_t upkt;
            MPIDI_CH3_Pkt_rndv_clr_to_send_t * cts_pkt = &upkt.rndv_clr_to_send;

            MPIDI_Pkt_init(cts_pkt, MPIDI_CH3_PKT_RNDV_CLR_TO_SEND);
            cts_pkt->sender_req_id = rts_pkt->sender_req_id;
            cts_pkt->receiver_req_id = rreq->handle;
            cts_pkt->recv_sz = 0;

            MPIDI_VC_FAI_send_seqnum(vc, seqnum);
            MPIDI_Pkt_set_seqnum(cts_pkt, seqnum);

            mpi_errno = MPIDI_CH3U_Post_data_receive_found(rreq);
            if (mpi_errno != MPI_SUCCESS && rreq != NULL) {
                MPIR_ERR_SETANDJUMP(mpi_errno,MPI_ERR_OTHER,"**ch3|loadsendiov");
            }

            if (rreq->dev.OnDataAvail == NULL) {
                for (k=0; k < rreq->dev.iov_count; ++k) {
                    cts_pkt->recv_sz += rreq->dev.iov[k].iov_len;
                }
            } else {
                cts_pkt->recv_sz = rreq->dev.msgsize;
            }
#if defined(_ENABLE_CUDA_) && defined(HAVE_CUDA_IPC)
            if (mvp_enable_device &&
                    (mvp_device_use_ipc && mvp_device_use_ipc_stage_buffer &&
                     rreq->mrail.device_transfer_mode != NONE &&
                     vc->smp.can_access_peer == MVP_DEVICE_IPC_ENABLED) ||
                (rreq->mrail.protocol == MRAILI_PROTOCOL_CUDAIPC)) {
                mpi_errno = MPIDI_CH3_Prepare_rndv_cts_device(vc, cts_pkt, rreq);
            } else
#endif
            mpi_errno = MPIDI_CH3_Prepare_rndv_cts(vc, cts_pkt, rreq);
            if (mpi_errno != MPI_SUCCESS) {
                MPIR_ERR_SETANDJUMP(mpi_errno,MPI_ERR_OTHER,"**ch3|rndv");
            }

            PRINT_DEBUG(DEBUG_RNDV_verbose>1,
                    "sending CTS to rank: %d, buf_addr: %p, recv_sz: %ld, protocol: %d\n",
                    vc->pg_rank, cts_pkt->rndv.buf_addr, cts_pkt->recv_sz, cts_pkt->rndv.protocol);

            MPID_THREAD_CS_ENTER(POBJ, vc->pobj_mutex);
            mpi_errno = MPIDI_CH3_iStartMsg(vc, cts_pkt, sizeof(*cts_pkt), &cts_req);
            MPID_THREAD_CS_EXIT(POBJ, vc->pobj_mutex);
            if (mpi_errno != MPI_SUCCESS) {
                MPIR_ERR_SETANDJUMP(mpi_errno,MPI_ERR_OTHER,
                        "**ch3|ctspkt");
            }
            if (cts_req != NULL) {
                MPIR_Request_free(cts_req);
            }
        }
    } else {
        MVP_INC_NUM_POSTED_RECV();
        PRINT_DEBUG(DEBUG_RNDV_verbose>1,
                "unexpected request allocated, rreq: %p, sreq: %08x\n",
                rreq, rts_pkt->sender_req_id);
#if defined(_ENABLE_CUDA_)
        /*If cuda is enabled, the pkt size is large, allocate
          a buffer and then copy the rts packet*/
        rreq->dev.pending_pkt = MPL_malloc(sizeof(MPIDI_CH3_Pkt_rndv_req_to_send_t));
        MPIR_Memcpy(rreq->dev.pending_pkt, rts_pkt, sizeof(MPIDI_CH3_Pkt_rndv_req_to_send_t));
#endif
        MPIR_Memcpy(&rreq->ch.pkt, pkt, sizeof(MPIDI_CH3_Pkt_t));
        MPIDI_CH3_Progress_signal_completion();
    }

 fn_exit:
    *rreqp = NULL;
 fn_fail:
    return mpi_errno;
}
#endif

int MPIDI_CH3_PktHandler_RndvReqToSend( MPIDI_VC_t *vc, MPIDI_CH3_Pkt_t *pkt, void *data ATTRIBUTE((unused)),
                    intptr_t *buflen, MPIR_Request **rreqp )
{
    MPIR_Request * rreq;
    int found;
    MPIDI_CH3_Pkt_rndv_req_to_send_t * rts_pkt = &pkt->rndv_req_to_send;
    int mpi_errno = MPI_SUCCESS;
    
#if defined(CHANNEL_MRAIL)
    PRINT_DEBUG(DEBUG_RNDV_verbose,
            "received RTS, sreq: %08x, rank: %d, tag: %d, context: %d, protocol: %d, data_sz: %ld\n",
            rts_pkt->sender_req_id, rts_pkt->match.parts.rank, rts_pkt->match.parts.tag, 
            rts_pkt->match.parts.context_id, rts_pkt->rndv.protocol, rts_pkt->data_sz);

    if (IS_VC_SMP(vc)) {
        return MPIDI_CH3_PktHandler_SMP_RTS(vc, pkt, buflen, rreqp);
    }
#endif

    MPL_DBG_MSG_FMT(MPIDI_CH3_DBG_OTHER,VERBOSE,(MPL_DBG_FDEST,
 "received rndv RTS pkt, sreq=0x%08x, rank=%d, tag=%d, context=%d, data_sz=%" PRIdPTR,
          rts_pkt->sender_req_id, rts_pkt->match.parts.rank, 
                    rts_pkt->match.parts.tag, 
              rts_pkt->match.parts.context_id, rts_pkt->data_sz));
    MPL_DBG_MSGPKT(vc,rts_pkt->match.parts.tag,rts_pkt->match.parts.context_id,
            rts_pkt->match.parts.rank,rts_pkt->data_sz,
            "ReceivedRndv");

    MPID_THREAD_CS_ENTER(POBJ, MPIR_THREAD_POBJ_MSGQ_MUTEX);
    rreq = MPIDI_CH3U_Recvq_FDP_or_AEU(&rts_pkt->match, &found);
#if defined(CHANNEL_MRAIL)
    if (!found && SMP_INIT && vc->smp.local_nodes >= 0) {
        MVP_INC_NUM_POSTED_RECV();
    }
#endif
    MPIR_ERR_CHKANDJUMP1(!rreq, mpi_errno,MPI_ERR_OTHER, "**nomemreq", "**nomemuereq %d", MPIDI_CH3U_Recvq_count_unexp());

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
    MPIDI_CH3U_Append_pkt_size();
#if defined(CHANNEL_MRAIL)
    MPIDI_CH3_RNDV_SET_REQ_INFO(rreq, rts_pkt);
#endif /* defined(CHANNEL_MRAIL) */
    
    if (found)
    {
        MPIR_Request * cts_req;
        MPIDI_CH3_Pkt_t upkt;
        MPIDI_CH3_Pkt_rndv_clr_to_send_t * cts_pkt = &upkt.rndv_clr_to_send;
#if defined(CHANNEL_MRAIL) && defined(MPID_USE_SEQUENCE_NUMBERS)
        MPID_Seqnum_t seqnum;
#endif /* defined(CHANNEL_MRAIL) && defined(MPID_USE_SEQUENCE_NUMBERS) */
    
        MPL_DBG_MSG(MPIDI_CH3_DBG_OTHER,VERBOSE,"posted request found");
        MPIR_T_PVAR_COUNTER_INC(MVP, expected_recvs_rendezvous, 1); 
#if defined(CHANNEL_MRAIL)
        if(MPIDI_CH3_RNDV_PROTOCOL_IS_READ(rts_pkt)) {

            mpi_errno = MPIDI_CH3U_Post_data_receive_found(rreq);
            /* --BEGIN ERROR HANDLING-- */
            if (mpi_errno != MPI_SUCCESS && rreq != NULL) {
                MPIR_ERR_SETANDJUMP(mpi_errno,
                        MPI_ERR_OTHER,"**ch3|loadsendiov");
            }

            mpi_errno = MPIDI_CH3_Prepare_rndv_get(vc, rreq);
            if (mpi_errno != MPI_SUCCESS) {
                MPIR_ERR_SETANDJUMP(mpi_errno,MPI_ERR_OTHER,"**ch3|rndv");
            }

            if (MRAILI_PROTOCOL_RGET == rreq->mrail.protocol) {
                mpi_errno = MPIDI_CH3_Rndv_transfer(vc,
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
    
    MPL_DBG_MSG(MPIDI_CH3_DBG_OTHER,VERBOSE,"sending rndv CTS packet");
    MPIDI_Pkt_init(cts_pkt, MPIDI_CH3_PKT_RNDV_CLR_TO_SEND);
    cts_pkt->sender_req_id = rts_pkt->sender_req_id;
    cts_pkt->receiver_req_id = rreq->handle;
#if defined(CHANNEL_MRAIL)
        MPIDI_VC_FAI_send_seqnum(vc, seqnum);
        MPIDI_Pkt_set_seqnum(cts_pkt, seqnum);
        mpi_errno = MPIDI_CH3U_Post_data_receive_found(rreq);
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

#if defined(_ENABLE_CUDA_)
        if (mvp_enable_device  &&
            (rreq->mrail.device_transfer_mode != NONE &&
                (vc->smp.local_nodes == -1)) 
           )
        {
            mpi_errno = MPIDI_CH3_Prepare_rndv_cts_device(vc, cts_pkt, rreq);
        } else
#endif
        {
            mpi_errno = MPIDI_CH3_Prepare_rndv_cts(vc, cts_pkt, rreq);
        }
        if (mpi_errno != MPI_SUCCESS)
        {
            MPIR_ERR_SETANDJUMP(mpi_errno,MPI_ERR_OTHER,"**ch3|rndv");
        }
#endif /* defined(CHANNEL_MRAIL) */
        MPID_THREAD_CS_ENTER(POBJ, vc->pobj_mutex);
    mpi_errno = MPIDI_CH3_iStartMsg(vc, cts_pkt, sizeof(*cts_pkt), &cts_req);
        MPID_THREAD_CS_EXIT(POBJ, vc->pobj_mutex);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_SETANDJUMP(mpi_errno,MPI_ERR_OTHER,
                "**ch3|ctspkt");
    }
    if (cts_req != NULL) {
        MPIR_Request_free(cts_req);
    }
    }
    else
    {
        MPL_DBG_MSG(MPIDI_CH3_DBG_OTHER,VERBOSE,"unexpected request allocated");
#if defined(CHANNEL_MRAIL)
        /* If the request is a read and is unexpected,
         * we have to buffer the remote information till
         * this request is matched and then processed */
        if(MPIDI_CH3_RNDV_PROTOCOL_IS_READ(rts_pkt)) {
#if(_ENABLE_CUDA_)
            /*If this is cuda is enabled, the pkt size is large, allocate 
              a buffer and then copy the rts packet*/
            rreq->dev.pending_pkt = MPIR_Malloc(sizeof(MPIDI_CH3_Pkt_rndv_req_to_send_t));
            MPIR_Memcpy(rreq->dev.pending_pkt, rts_pkt, sizeof(MPIDI_CH3_Pkt_rndv_req_to_send_t));
#else
            MPIR_Memcpy(&rreq->ch.pkt, pkt, sizeof(MPIDI_CH3_Pkt_t));
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
         * counter and MPIDI_CH3_Progress_signal_completion()
         * should only be called if that counter is greater than zero.
         */
        MPIDI_CH3_Progress_signal_completion();
    }
    
    *rreqp = NULL;

#if defined(CHANNEL_MRAIL)
 fn_exit:
#endif
 fn_fail:
    return mpi_errno;
}

/* MVP optimized SMP RGET design */
#if defined(CHANNEL_MRAIL)
int MPIDI_CH3_PktHandler_SMP_CTS( MPIDI_VC_t *vc, MPIDI_CH3_Pkt_t *pkt,
                    intptr_t *buflen, MPIR_Request **rreqp )
{
    int i, mpi_errno = MPI_SUCCESS;
    MPIDI_CH3_Pkt_rndv_clr_to_send_t * cts_pkt = &pkt->rndv_clr_to_send;
    MPIR_Request * sreq;
    intptr_t recv_size;

    MPIR_Assert(pkt->type == MPIDI_CH3_PKT_RNDV_CLR_TO_SEND);
    MPIR_Request_get_ptr(cts_pkt->sender_req_id, sreq);
    MPIR_Assert(sreq != NULL);

#if defined(_ENABLE_CUDA_) && defined(HAVE_CUDA_IPC)
    /* if receiver has set protocol to MRAILI_PROTOCOL_CUDAIPC
     * revert protocol to MRAILI_PROTOCOL_CUDAIPC */
    if (cts_pkt->rndv.protocol == MRAILI_PROTOCOL_CUDAIPC &&
        sreq->mrail.protocol != MRAILI_PROTOCOL_CUDAIPC) {
        MPIDI_CH3I_MRAIL_Revert_rndv_device_ipc_buffered (vc, sreq);
    }
#endif
    recv_size = cts_pkt->recv_sz;
    for (i = 0; i < sreq->dev.iov_count ; i ++) {
        if (recv_size < sreq->dev.iov[i].iov_len) {
            fprintf(stderr, "Warning! Rndv Receiver is expecting %lu Bytes "
                    "But, is receiving %lu Bytes \n", 
                    sreq->dev.iov[i].iov_len, recv_size);
            sreq->dev.iov[i].iov_len = recv_size;
            sreq->dev.iov_count = i + 1;
            break;
        } else {
            recv_size -= sreq->dev.iov[i].iov_len;
        }
    }
    sreq->mrail.rndv_buf_sz = cts_pkt->recv_sz;
    sreq->mrail.protocol = cts_pkt->rndv.protocol;

    mpi_errno = MPIDI_CH3_Rndv_transfer(vc, sreq, NULL, cts_pkt, NULL);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_SETANDJUMP(mpi_errno,MPI_ERR_OTHER,"**ch3|senddata");
    }
    *rreqp = NULL;
    *buflen = sizeof(MPIDI_CH3_Pkt_t);

fn_fail:
    return mpi_errno;
}
#endif

int MPIDI_CH3_PktHandler_RndvClrToSend( MPIDI_VC_t *vc, MPIDI_CH3_Pkt_t *pkt, void *data ATTRIBUTE((unused)),
                    intptr_t *buflen, MPIR_Request **rreqp )
{
    MPIDI_CH3_Pkt_rndv_clr_to_send_t * cts_pkt = &pkt->rndv_clr_to_send;
    MPIR_Request * sreq;
#if defined(CHANNEL_MRAIL)
    intptr_t recv_size;
    int i;
#else
    MPIR_Request * rts_sreq;
    MPIDI_CH3_Pkt_t upkt;
    MPIDI_CH3_Pkt_rndv_send_t * rs_pkt = &upkt.rndv_send;
    int dt_contig;
    MPI_Aint dt_true_lb;
    intptr_t data_sz;
    MPIR_Datatype* dt_ptr;
#endif /* defined(CHANNEL_MRAIL) */
    int mpi_errno = MPI_SUCCESS;
    
#if defined(CHANNEL_MRAIL)
    PRINT_DEBUG(DEBUG_RNDV_verbose,
            "received CTS, seqnum: %d, sender_req_id: %08x, receiver_req_id: %08x, protocol: %d\n",
            cts_pkt->seqnum, cts_pkt->sender_req_id, cts_pkt->receiver_req_id, cts_pkt->rndv.protocol);

    if (IS_VC_SMP(vc) && pkt->type == MPIDI_CH3_PKT_RNDV_CLR_TO_SEND) {
        return MPIDI_CH3_PktHandler_SMP_CTS(vc, pkt, buflen, rreqp);
    }
#endif

    MPL_DBG_MSG(MPIDI_CH3_DBG_OTHER,VERBOSE,"received rndv CTS pkt");
    
    MPIR_Request_get_ptr(cts_pkt->sender_req_id, sreq);
    MPL_DBG_MSG_FMT(MPIDI_CH3_DBG_OTHER,TERSE,(MPL_DBG_FDEST,"received cts, count=" MPI_AINT_FMT_DEC_SPEC "\n", sreq->dev.user_count));
    
#if defined(CHANNEL_MRAIL)
#if defined(_ENABLE_CUDA_) && defined(HAVE_CUDA_IPC)
    /* if receiver has set protocol to MRAILI_PROTOCOL_CUDAIPC
     * revert protocol to MRAILI_PROTOCOL_CUDAIPC */
    if (cts_pkt->rndv.protocol == MRAILI_PROTOCOL_CUDAIPC &&
        sreq->mrail.protocol != MRAILI_PROTOCOL_CUDAIPC) {
        MPIDI_CH3I_MRAIL_Revert_rndv_device_ipc_buffered (vc, sreq);
    }
#endif

    if (sreq->mrail.rndv_buf_off != 0 &&
        sreq->mrail.protocol == MRAILI_PROTOCOL_RPUT) {
        MPIR_Assert(sreq->mrail.rndv_buf_off == 0);
    }

    if (pkt->type == MPIDI_CH3_PKT_RNDV_CLR_TO_SEND) {
        recv_size = cts_pkt->recv_sz;
        for (i = 0; i < sreq->dev.iov_count ; i ++) {
            if (recv_size < sreq->dev.iov[i].iov_len) {
                PRINT_INFO(1, "Warning! Rndv Receiver is expecting %lu Bytes "
                        "But, is receiving %lu Bytes \n", 
                        sreq->dev.iov[i].iov_len, recv_size);
                sreq->dev.iov[i].iov_len = recv_size;
                sreq->dev.iov_count = i + 1;
                break;
            } else
                recv_size -= sreq->dev.iov[i].iov_len;
        }
        sreq->mrail.rndv_buf_sz = cts_pkt->recv_sz;
    } else if (IS_VC_SMP(vc)) {
        /* Used for one-sided communications */
        sreq->mrail.protocol = MRAILI_PROTOCOL_R3;
        cts_pkt->rndv.protocol = MRAILI_PROTOCOL_R3;
        MPIDI_CH3I_MRAIL_FREE_RNDV_BUFFER(sreq);
    }

    mpi_errno = MPIDI_CH3_Rndv_transfer(vc, sreq, NULL,
            cts_pkt, NULL);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_SETANDJUMP(mpi_errno,MPI_ERR_OTHER,"**ch3|senddata");
    }
    *rreqp = NULL;
#else /* defined(CHANNEL_MRAIL) */

    sreq->dev.OnDataAvail = 0;
    sreq->dev.OnFinal = 0;

    /* Release the RTS request if one exists.  
       MPID_Request_fetch_and_clear_rts_sreq() needs to be atomic to 
       prevent
       cancel send from cancelling the wrong (future) request.  
       If MPID_Request_fetch_and_clear_rts_sreq() returns a NULL
       rts_sreq, then MPID_Cancel_send() is responsible for releasing 
       the RTS request object. */
    MPIDI_Request_fetch_and_clear_rts_sreq(sreq, &rts_sreq);
    if (rts_sreq != NULL)
    {
    MPIR_Request_free(rts_sreq);
    }
#endif /* defined(CHANNEL_MRAIL) */

    *buflen = 0;
    MPIDI_CH3U_Append_pkt_size();
#if !defined(CHANNEL_MRAIL)

    MPIDI_Pkt_init(rs_pkt, MPIDI_CH3_PKT_RNDV_SEND);
    rs_pkt->receiver_req_id = cts_pkt->receiver_req_id;
    
    MPIDI_Datatype_get_info(sreq->dev.user_count, sreq->dev.datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);
    
    if (dt_contig) 
    {
        struct iovec iov[MPL_IOV_LIMIT] = {0};

        MPL_DBG_MSG_FMT(MPIDI_CH3_DBG_OTHER,VERBOSE,(MPL_DBG_FDEST,
                "sending contiguous rndv data, data_sz=%" PRIdPTR,
                            data_sz));
        
        iov[0].iov_base = (void *)rs_pkt;
        iov[0].iov_len = sizeof(*rs_pkt);
        
        iov[1].iov_base = (void *)((char *)sreq->dev.user_buf + dt_true_lb);
        iov[1].iov_len = data_sz;

            MPID_THREAD_CS_ENTER(POBJ, vc->pobj_mutex);
        mpi_errno = MPIDI_CH3_iSendv(vc, sreq, iov, 2);
            MPID_THREAD_CS_EXIT(POBJ, vc->pobj_mutex);
        MPIR_ERR_CHKANDJUMP(mpi_errno, mpi_errno, MPI_ERR_OTHER, "**ch3|senddata");
    }
    else
    {
        sreq->dev.msg_offset = 0;
        sreq->dev.msgsize = data_sz;

        MPID_THREAD_CS_ENTER(POBJ, vc->pobj_mutex);
        mpi_errno = vc->sendNoncontig_fn(vc, sreq, rs_pkt, sizeof(*rs_pkt), NULL, 0);
        MPID_THREAD_CS_EXIT(POBJ, vc->pobj_mutex);
        MPIR_ERR_CHKANDJUMP(mpi_errno, mpi_errno, MPI_ERR_OTHER, "**ch3|senddata");
    }    
    *rreqp = NULL;
#endif /* !defined(CHANNEL_MRAIL) */
 fn_fail:
    return mpi_errno;
}

int MPIDI_CH3_PktHandler_RndvSend( MPIDI_VC_t *vc, MPIDI_CH3_Pkt_t *pkt, void *data,
                   intptr_t *buflen, MPIR_Request **rreqp )
{
    MPIDI_CH3_Pkt_rndv_send_t * rs_pkt = &pkt->rndv_send;
    int mpi_errno = MPI_SUCCESS;
    int complete;
    intptr_t data_len;
    MPIR_Request *req;
    
    MPL_DBG_MSG(MPIDI_CH3_DBG_OTHER,VERBOSE,"received rndv send (data) pkt");

    MPIR_Request_get_ptr(rs_pkt->receiver_req_id, req);

    data_len = ((*buflen >= req->dev.recv_data_sz)
                ? req->dev.recv_data_sz : *buflen);
    
    if (req->dev.recv_data_sz == 0) {
        *buflen = 0;
        mpi_errno = MPID_Request_complete(req);
        MPIR_ERR_CHECK(mpi_errno);
    *rreqp = NULL;
    }
    else {
        mpi_errno = MPIDI_CH3U_Receive_data_found(req, data, &data_len,
                                                  &complete);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_SETANDJUMP1(mpi_errno,MPI_ERR_OTHER, "**ch3|postrecv",
                 "**ch3|postrecv %s", "MPIDI_CH3_PKT_RNDV_SEND");
    }

        *buflen = data_len;

        if (complete) 
        {
            mpi_errno = MPID_Request_complete(req);
            MPIR_ERR_CHECK(mpi_errno);
            *rreqp = NULL;
        }
        else
        {
            *rreqp = req;
        }
   }
    
 fn_fail:
    return mpi_errno;
}

/*
 * This routine processes a rendezvous message once the message is matched.
 * It is used in mpid_recv and mpid_irecv.
 */
int MPIDI_CH3_RecvRndv( MPIDI_VC_t * vc, MPIR_Request *rreq )
{
    int mpi_errno = MPI_SUCCESS;
#if defined(CHANNEL_MRAIL)
    MPIDI_CH3_Pkt_rndv_req_to_send_t * rts_pkt;
#endif /* defined(CHANNEL_MRAIL) */
    /* A rendezvous request-to-send (RTS) message has arrived.  We need
       to send a CTS message to the remote process. */
#ifdef MPIDI_CH3_CHANNEL_RNDV
    /* The channel will be performing the rendezvous */
    
#if defined(CHANNEL_MRAIL)
    if (IS_VC_SMP(vc)) {
        intptr_t buflen;
        MPIR_Assert(rreq != NULL);
        return MPIDI_CH3_PktHandler_SMP_RTS(vc, &(rreq->ch.pkt), &buflen, &rreq);
    }
#endif

#if defined(CHANNEL_MRAIL)
    if (rreq->dev.recv_data_sz == 0) {
    MPID_Request_complete(rreq);
#else /* defined(CHANNEL_MRAIL) */
    if (req->dev.recv_data_sz == 0) {
    MPID_Request_complete(req);
    *rreqp = NULL;
#endif /* defined(CHANNEL_MRAIL) */
    }
    else {
#if defined(CHANNEL_MRAIL)
        mpi_errno = MPIDI_CH3U_Post_data_receive_found(rreq);
#else /* defined(CHANNEL_MRAIL) */
        mpi_errno = MPIDI_CH3U_Post_data_receive_found(req);
#endif /* defined(CHANNEL_MRAIL) */
        if (mpi_errno != MPI_SUCCESS) {
            MPIR_ERR_SETANDJUMP1(mpi_errno,MPI_ERR_OTHER,
                     "**ch3|postrecv",
                     "**ch3|postrecv %s",
                     "MPIDI_CH3_PKT_RNDV_REQ_TO_SEND");
        }
    }

    if(MPIDI_CH3_RECV_REQ_IS_READ(rreq)) {
#if defined(_ENABLE_CUDA_)
        /*when cuda is enabled, the rts packet is in a dynamically allocated buffer*/
        rts_pkt = (MPIDI_CH3_Pkt_rndv_req_to_send_t *) rreq->dev.pending_pkt;
#else
        MPIDI_CH3_Pkt_t *tmp_pkt = (MPIDI_CH3_Pkt_t *) &(rreq->ch.pkt);
        rts_pkt = &(tmp_pkt->rndv_req_to_send);;
#endif

        mpi_errno = MPIDI_CH3_Prepare_rndv_get(vc, rreq);
        if (mpi_errno != MPI_SUCCESS) {
            MPIR_ERR_SETANDJUMP(mpi_errno,MPI_ERR_OTHER,"**ch3|rndv");
        }

        if (MRAILI_PROTOCOL_RGET == rreq->mrail.protocol) {
            mpi_errno = MPIDI_CH3_Rndv_transfer(vc, NULL, rreq, NULL, rts_pkt);
            if (mpi_errno != MPI_SUCCESS && rreq != NULL) {
                MPIR_ERR_SETANDJUMP(mpi_errno,
                     MPI_ERR_OTHER,"**ch3|senddata");
            
            }
            goto fn_exit;
        }
        /*else send back CTS with R3 protocol and fallback*/
    } 

    mpi_errno = MPIDI_CH3_iStartRndvTransfer (vc, rreq);

    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_SETANDJUMP(mpi_errno,MPI_ERR_OTHER,
                "**ch3|ctspkt");
    }
    
#else
    MPIR_Request * cts_req;
    MPIDI_CH3_Pkt_t upkt;
    MPIDI_CH3_Pkt_rndv_clr_to_send_t * cts_pkt = &upkt.rndv_clr_to_send;
    
    MPL_DBG_MSG(MPIDI_CH3_DBG_OTHER,VERBOSE,
         "rndv RTS in the request, sending rndv CTS");
    
    MPIDI_Pkt_init(cts_pkt, MPIDI_CH3_PKT_RNDV_CLR_TO_SEND);
    cts_pkt->sender_req_id = rreq->dev.sender_req_id;
    cts_pkt->receiver_req_id = rreq->handle;
    MPID_THREAD_CS_ENTER(POBJ, vc->pobj_mutex);
    mpi_errno = MPIDI_CH3_iStartMsg(vc, cts_pkt, sizeof(*cts_pkt), &cts_req);
    MPID_THREAD_CS_EXIT(POBJ, vc->pobj_mutex);
    if (mpi_errno != MPI_SUCCESS) {
    MPIR_ERR_SETANDJUMP(mpi_errno,MPI_ERR_OTHER, "**ch3|ctspkt");
    }
    if (cts_req != NULL)
    {
        /* FIXME: Ideally we could specify that a req not be returned.  
           This would avoid our having to decrement the
           reference count on a req we don't want/need. */
        MPIR_Request_free(cts_req);
    }
#endif

#if defined(CHANNEL_MRAIL)
 fn_exit:
#endif
 fn_fail:    
    return mpi_errno;
}

/*
 * Define the routines that can print out the cancel packets if 
 * debugging is enabled.
 */
#ifdef MPICH_DBG_OUTPUT
int MPIDI_CH3_PktPrint_RndvReqToSend( FILE *fp, MPIDI_CH3_Pkt_t *pkt )
{
    MPL_DBG_MSG(MPIDI_CH3_DBG_OTHER,TERSE," type ......... REQ_TO_SEND\n");
    MPL_DBG_MSG_FMT(MPIDI_CH3_DBG_OTHER,TERSE,(MPL_DBG_FDEST," sender_reqid . 0x%08X\n", pkt->rndv_req_to_send.sender_req_id));
    MPL_DBG_MSG_D(MPIDI_CH3_DBG_OTHER,TERSE," context_id ... %d\n", pkt->rndv_req_to_send.match.parts.context_id);
    MPL_DBG_MSG_D(MPIDI_CH3_DBG_OTHER,TERSE," tag .......... %d\n", pkt->rndv_req_to_send.match.parts.tag);
    MPL_DBG_MSG_D(MPIDI_CH3_DBG_OTHER,TERSE," rank ......... %d\n", pkt->rndv_req_to_send.match.parts.rank);
    MPL_DBG_MSG_D(MPIDI_CH3_DBG_OTHER,TERSE," data_sz ...... %d\n", pkt->rndv_req_to_send.data_sz);
#ifdef MPID_USE_SEQUENCE_NUMBERS
    MPL_DBG_MSG_D(MPIDI_CH3_DBG_OTHER,TERSE," seqnum ....... %d\n", pkt->rndv_req_to_send.seqnum);
#endif
    return MPI_SUCCESS;
}
int MPIDI_CH3_PktPrint_RndvClrToSend( FILE *fp, MPIDI_CH3_Pkt_t *pkt )
{
    MPL_DBG_MSG(MPIDI_CH3_DBG_OTHER,TERSE," type ......... CLR_TO_SEND\n");
    MPL_DBG_MSG_FMT(MPIDI_CH3_DBG_OTHER,TERSE,(MPL_DBG_FDEST," sender_reqid . 0x%08X\n", pkt->rndv_clr_to_send.sender_req_id));
    MPL_DBG_MSG_FMT(MPIDI_CH3_DBG_OTHER,TERSE,(MPL_DBG_FDEST," recvr_reqid .. 0x%08X\n", pkt->rndv_clr_to_send.receiver_req_id));
    return MPI_SUCCESS;
}
int MPIDI_CH3_PktPrint_RndvSend( FILE *fp, MPIDI_CH3_Pkt_t *pkt )
{
    MPL_DBG_MSG(MPIDI_CH3_DBG_OTHER,TERSE," type ......... RNDV_SEND\n");
    MPL_DBG_MSG_FMT(MPIDI_CH3_DBG_OTHER,TERSE,(MPL_DBG_FDEST," recvr_reqid .. 0x%08X\n", pkt->rndv_send.receiver_req_id));
    return MPI_SUCCESS;
}
#endif 

