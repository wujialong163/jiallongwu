/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 *
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda.
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level MVAPICH directory.
 */

#ifndef _MVP_SMP_RNDV_H_
#define _MVP_SMP_RNDV_H_

#ifndef MVP_SMP_IMPL_INCLUDED
#error "Requres mvp_smp_impl.h, include that header first"
#endif

#include "mvp_pkt.h"
#include "mvp_req_fields.h"
#include "mvp_req.h"

extern int mvp_num_rails;

int MPIDI_MVP_smp_handle_recv_req(MPIR_Request *rreq, int *complete);
int MPIDI_MVP_smp_handle_send_req(MPIR_Request *rreq, int *complete);

MPL_STATIC_INLINE_PREFIX int MPIDI_MVP_smp_free_rndv_buffer(MPIR_Request *req)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_SMP_FREE_RNDV_BUFFER);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_FREE_RNDV_BUFFER);

    if (1 == MPIDI_MVP_REQUEST(req, mrail.rndv_buf_alloc) &&
        NULL != MPIDI_MVP_REQUEST(req, mrail.rndv_buf)) {
        MPL_free(MPIDI_MVP_REQUEST(req, mrail.rndv_buf));
        MPIDI_MVP_REQUEST(req, mrail.rndv_buf_alloc) = 0;
        MPIDI_MVP_REQUEST(req, mrail.rndv_buf_off) = 0;
        MPIDI_MVP_REQUEST(req, mrail.rndv_buf_sz) = 0;
        MPIDI_MVP_REQUEST(req, mrail.rndv_buf) = NULL;
    } else {
        MPIDI_MVP_REQUEST(req, mrail.rndv_buf_off) = 0;
        MPIDI_MVP_REQUEST(req, mrail.rndv_buf_sz) = 0;
    }
fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_FREE_RNDV_BUFFER);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#define MPIDI_MVP_SMP_REQ_RNDV_FINISH(_req)         \
{                                                   \
   if (_req != NULL) {                              \
        MPIDI_MVP_smp_free_rndv_buffer(_req);       \
    }                                               \
    MPIDI_MVP_REQUEST(_req, mrail.protocol) =       \
        MRAILI_PROTOCOL_RENDEZVOUS_UNSPECIFIED;     \
}

#define MPIDI_MVP_SET_REQUEST_INFO(rreq_, pkt_, msg_type_)                  \
{                                                                           \
    (rreq_)->status.MPI_SOURCE = (pkt_)->match.parts.rank;                  \
    (rreq_)->status.MPI_TAG = (pkt_)->match.parts.tag;                      \
    MPIR_STATUS_SET_COUNT((rreq_)->status, (pkt_)->data_sz);                \
    MPIDI_MVP_REQUEST(rreq_, dev.sender_req_id) = (pkt_)->sender_req_id;    \
    MPIDI_MVP_REQUEST(rreq_, dev.recv_data_sz) = (pkt_)->data_sz;           \
    MPIDI_Request_set_seqnum(rreq_, (pkt_)->seqnum);                        \
    MPIDI_Request_set_msg_type(rreq_, (msg_type_));                         \
}

MPL_STATIC_INLINE_PREFIX int MPIDI_MVP_smp_rndv_transfer(MPIR_Request *req, 
                                                         void * pkt)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_MVP_MRAILI_Rndv_info_t *rndv;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_SMP_RNDV_TRANSFER);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_RNDV_TRANSFER);

    MPIR_Err_create_code(mpi_errno, MPIR_ERR_FATAL, __func__,
                         __LINE__, MPI_ERR_OTHER, "**fail", 
                         "**fail %s", "Function Incomplete");
    /* TODO: lazy fix, remove this dependency - also may not work if we don't 
     * actually set this */
    MPIDI_MVP_ep_h vc = MPIDI_MVP_REQUEST(req, ch.vc);

    switch (MPIDI_MVP_REQUEST(req, mrail.protocol)) {
        case MVP_RNDV_PROTOCOL_R3:
            mpi_errno =
                MPIR_Err_create_code(mpi_errno, MPIR_ERR_FATAL, __func__,
                                     __LINE__, MPI_ERR_OTHER, "**fail", 
                                     "**fail %s", "Unsupported R3");
            break;
        case MVP_RNDV_PROTOCOL_RPUT:
            rndv = &((MPIDI_MVP_Pkt_rndv_clr_to_send_t *)pkt)->rndv;
            break;
        case MVP_RNDV_PROTOCOL_RGET:
            rndv = &((MPIDI_MVP_Pkt_rndv_req_to_send_t *)pkt)->rndv;
            break;
        default:
            mpi_errno =
                MPIR_Err_create_code(mpi_errno, MPIR_ERR_FATAL, __func__,
                                     __LINE__, MPI_ERR_OTHER, "**fail", 
                                     "**fail %s", "Unsupported protocol");
            MPIR_ERR_POP(mpi_errno);
    }
#if 0
    RENDEZVOUS_IN_PROGRESS(vc, req);

    MPIDI_MVP_REQUEST(req, mrail.nearly_complete) = 0;

    PUSH_FLOWLIST(vc);
#endif

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_RNDV_TRANSFER);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}


MPL_STATIC_INLINE_PREFIX int smp_dma_request_fin(MPIR_Request *rreq)
{
    MPIR_Assert(MPIDI_MVP_REQUEST(rreq, mrail).local_complete == UINT32_MAX ||
                MPIDI_MVP_REQUEST(rreq, mrail).local_complete <= mvp_num_rails);
    MPIR_Assert(MPIDI_MVP_REQUEST(rreq, mrail).remote_complete == UINT32_MAX ||
                MPIDI_MVP_REQUEST(rreq, mrail).remote_complete <=
                    mvp_num_rails);

    switch (MPIDI_MVP_REQUEST(rreq, mrail).protocol) {
        case MVP_RNDV_PROTOCOL_RGET:
        case MVP_RNDV_PROTOCOL_RPUT:
            return (MPIDI_MVP_REQUEST(rreq, mrail.local_complete) == UINT32_MAX ||
                MPIDI_MVP_REQUEST(rreq, mrail.remote_complete) == UINT32_MAX);
        default:
            break;
    }
    return 1;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_MVP_smp_rndv_dma_finish(
    MPIDI_MVP_Pkt_dma_finish_t *fin_pkt, MPIR_Request **req_ptr)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Request *req;
    int complete = 0;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_SMP_RNDV_DMA_FINISH);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_RNDV_DMA_FINISH);

    MPIR_Request_get_ptr(fin_pkt->req_id, req);
    MPIDI_MVP_REQUEST(req, mrail.remote_complete) = UINT32_MAX;

    /* this will always fail based on setting remote_complete above */
    if (!smp_dma_request_fin(req)) {
        goto fn_exit;
    }
    
    if (fin_pkt->type == MPIDI_MVP_PKT_RPUT_FINISH) {
        if (MPIDI_MVP_REQUEST(req, mrail.rndv_buf_alloc) == 1) {
            PRINT_ERROR("rndv_buf_alloc not supported\n");
        } else {
            MPIDI_MVP_REQUEST(req, mrail.rndv_buf) = NULL;
        }
    }

    MPIDI_MVP_SMP_REQ_RNDV_FINISH(req);

    if (MPIDI_MVP_PKT_RPUT_FINISH == fin_pkt->type) {
        mpi_errno = MPIDI_MVP_smp_handle_recv_req(req, &complete);
    } else if (MPIDI_MVP_PKT_RGET_FINISH == fin_pkt->type) {
        mpi_errno = MPIDI_MVP_smp_handle_send_req(req, &complete);
    }
    MPIR_ERR_CHECK(mpi_errno);

    if (!complete) {
        mpi_errno = MPIR_Err_create_code(mpi_errno, MPIR_ERR_FATAL, __func__,
                                         __LINE__, MPI_ERR_OTHER, "**fail",
                                         "**fail %s", 
                                         "Failed to complete rndv operation\n");
        MPIR_ERR_POP(mpi_errno);
    } else if (MPIDI_MVP_PKT_RPUT_FINISH == fin_pkt->type) {
        *req_ptr = NULL;
    }

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_RNDV_DMA_FINISH);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

/* MVP optimized SMP RGET design */
MPL_STATIC_INLINE_PREFIX int MPIDI_MVP_smp_rndv_cts(MPIDI_MVP_Pkt_t *pkt,
                                                    intptr_t *buflen,
                                                    MPIR_Request **rreq_ptr)
{
    int mpi_errno = MPI_SUCCESS;
    int i;
    intptr_t recv_size = 0;
    MPIR_Request *sreq = NULL;
    MPIDI_MVP_Pkt_rndv_clr_to_send_t *cts_pkt = &pkt->rndv_clr_to_send;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_SMP_RNDV_CTS);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_RNDV_CTS);
    
    MPIR_Assert(pkt->type == MPIDI_MVP_PKT_RNDV_CLR_TO_SEND);
    MPIR_Request_get_ptr(cts_pkt->sender_req_id, sreq);
    MPIR_Assert(sreq);

    recv_size = cts_pkt->recv_sz;
    for (i = 0; i < MPIDI_MVP_REQUEST(sreq, dev.iov_count); i++) {
        if (recv_size < MPIDI_MVP_REQUEST(sreq, dev.iov[i].iov_len)) {
            PRINT_ERROR("Warning! Rndv receiver is expecting %lu bytes "
                        "but is receiving %lu bytes\n",
                        MPIDI_MVP_REQUEST(sreq, dev.iov[i].iov_len), recv_size);
            MPIDI_MVP_REQUEST(sreq, dev.iov[i].iov_len) = recv_size;
            MPIDI_MVP_REQUEST(sreq, dev.iov_count) = i + 1;
            break;
        } else {
            recv_size -= MPIDI_MVP_REQUEST(sreq, dev.iov[i].iov_len);
        }
    }
    MPIDI_MVP_REQUEST(sreq, mrail.rndv_buf_sz) = cts_pkt->recv_sz;
    MPIDI_MVP_REQUEST(sreq, mrail.protocol) = cts_pkt->rndv.protocol;

    mpi_errno = MPIDI_MVP_smp_rndv_transfer(sreq, (void *)cts_pkt);
    MPIR_ERR_CHECK(mpi_errno);

    /* TODO: should we free here or should we leave it hanging? */
    MPIDI_MVP_Request_free(*rreq_ptr);
    *rreq_ptr = NULL;
    *buflen = sizeof(MPIDI_MVP_Pkt_t);

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_RNDV_CTS);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#endif /* ifndef _MVP_SMP_RNDV_H_ */
