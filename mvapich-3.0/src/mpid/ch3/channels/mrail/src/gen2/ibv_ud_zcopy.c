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

#include "mpichconf.h"
#include <mpir_mem.h>
#include "rdma_impl.h"
#include "ibv_impl.h"
#include "vbuf.h"
#include "mvp_ud.h"
#include "dreg.h"

extern unsigned long PVAR_COUNTER_mvp_vbuf_allocated;
extern unsigned long PVAR_COUNTER_mvp_vbuf_freed;
extern unsigned long PVAR_LEVEL_mvp_vbuf_available;
extern unsigned long PVAR_COUNTER_mvp_ud_vbuf_allocated;
extern unsigned long PVAR_COUNTER_mvp_ud_vbuf_freed;
extern unsigned long PVAR_LEVEL_mvp_ud_vbuf_available;

#ifdef _ENABLE_UD_
#define MVP_GET_RNDV_QP(_rqp, _proc)                                           \
    do {                                                                       \
        _rqp = (_proc)->zcopy_info.rndv_qp_pool_free_head;                     \
        (_proc)->zcopy_info.rndv_qp_pool_free_head =                           \
            ((_proc)->zcopy_info.rndv_qp_pool_free_head)->next;                \
        _rqp->next = NULL;                                                     \
    } while (0)

#define MVP_RELEASE_RNDV_QP(_rqp, _proc)                                       \
    do {                                                                       \
        if ((_proc)->zcopy_info.rndv_qp_pool_free_head != NULL) {              \
            _rqp->next = (_proc)->zcopy_info.rndv_qp_pool_free_head;           \
        } else {                                                               \
            _rqp->next = NULL;                                                 \
        }                                                                      \
        (_proc)->zcopy_info.rndv_qp_pool_free_head = _rqp;                     \
    } while (0)

static inline int MPIDI_CH3_Rendezvous_zcopy_resend_cts(MPIDI_VC_t *vc,
                                                        MPIR_Request *rreq)
{
    int hca_index = 0;
    int mpi_errno = MPI_SUCCESS;
    MPIDI_CH3_Pkt_t upkt;
    MPIDI_CH3_Pkt_rndv_clr_to_send_t *cts_pkt = &upkt.rndv_clr_to_send;
    MPIR_Request *cts_req;

    MPIDI_Pkt_init(cts_pkt, MPIDI_CH3_PKT_RNDV_CLR_TO_SEND);
    cts_pkt->sender_req_id = rreq->dev.sender_req_id;
    cts_pkt->receiver_req_id = rreq->handle;

    if (rreq->dev.OnDataAvail == NULL) {
        cts_pkt->recv_sz = rreq->dev.iov[0].iov_len;
        if (rreq->dev.iov_count > 1) {
            int k = 1;
            for (; k < rreq->dev.iov_count; ++k) {
                cts_pkt->recv_sz += rreq->dev.iov[k].iov_len;
            }
        }
    } else {
        if (rreq->dev.msgsize > 0) {
            cts_pkt->recv_sz = rreq->dev.msgsize;
        } else {
            cts_pkt->recv_sz = rreq->mrail.rndv_buf_sz;
        }
    }

    MPIDI_CH3I_MRAIL_SET_PKT_RNDV(cts_pkt, rreq);
    MPIR_Assert(cts_pkt->rndv.protocol == MRAILI_PROTOCOL_UD_ZCOPY);

    for (hca_index = 0; hca_index < rreq->mrail.num_hcas; ++hca_index) {
        cts_pkt->rndv.rndv_qpn[hca_index] =
            rreq->mrail.rndv_qp_entry->ud_qp[hca_index]->qp_num;
        PRINT_DEBUG(DEBUG_ZCY_verbose > 0, "my qpn: %d, hca_index: %d\n",
                    cts_pkt->rndv.rndv_qpn[hca_index], hca_index);
    }
    rreq->mrail.remote_complete = 0;

    mpi_errno = MPIDI_CH3_iStartMsg(vc, cts_pkt, sizeof(*cts_pkt), &cts_req);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER, "**ch3|ctspkt");
    }
    if (cts_req != NULL) {
        MPIR_Request_free(cts_req);
    }
fn_fail:
    return mpi_errno;
}

static inline void MRAILI_Rndv_send_zcopy_finish(
    MPIDI_VC_t *vc, MPIR_Request *sreq, mvp_ud_zcopy_info_t *zcopy_info)
{
    vbuf *v = NULL;
    int hca_index = 0;
    MPIDI_CH3_Pkt_zcopy_finish_t *zcopy_finish;

    PRINT_DEBUG(DEBUG_ZCY_verbose > 1,
                "sending zcopy finish message to:%d on %d HCAs\n", vc->pg_rank,
                sreq->mrail.num_hcas);

    for (hca_index = 0; hca_index < sreq->mrail.num_hcas; ++hca_index) {
        GET_UD_VBUF_BY_OFFSET_WITHOUT_LOCK(v,
                                           MVP_SMALL_SEND_UD_VBUF_POOL_OFFSET);
        zcopy_finish = v->pheader;

        MPIDI_Pkt_init(zcopy_finish, MPIDI_CH3_PKT_ZCOPY_FINISH);
        zcopy_finish->receiver_req_id = sreq->mrail.partner_id;
        zcopy_finish->hca_index = hca_index;

        v->flags |= UD_VBUF_RETRY_ALWAYS;

        vbuf_init_send(v, sizeof(MPIDI_CH3_Pkt_zcopy_finish_t), hca_index);
        /* Zcopy QP's do not support inlined send */
        v->desc.u.sr.send_flags = IBV_SEND_SIGNALED;
        /* Need to send on same UD qp on which zcopy data transferred */
        PRINT_DEBUG(DEBUG_ZCY_verbose > 1,
                    "sending zcopy finish message to:%d on HCAs %d\n",
                    vc->pg_rank, hca_index);
        /* Increment the count to switch to RC protocol */
        vc->mrail.rely.total_messages++;
        post_ud_send(vc, v, hca_index, zcopy_info->rndv_ud_qps[hca_index]);
    }
}

static inline void MRAILI_Rndv_send_zcopy_ack(MPIDI_VC_t *vc,
                                              MPIR_Request *rreq)
{
    int hca_index;
    vbuf *v;
    MPIDI_CH3_Pkt_zcopy_ack_t *zcopy_ack;

    PRINT_DEBUG(DEBUG_ZCY_verbose > 1,
                "sending zcopy ack message to:%d on %d HCAs\n", vc->pg_rank,
                rreq->mrail.num_hcas);

    for (hca_index = 0; hca_index < rreq->mrail.num_hcas; ++hca_index) {
        GET_UD_VBUF_BY_OFFSET_WITHOUT_LOCK(v,
                                           MVP_SMALL_SEND_UD_VBUF_POOL_OFFSET);
        zcopy_ack = v->pheader;

        MPIDI_Pkt_init(zcopy_ack, MPIDI_CH3_PKT_ZCOPY_ACK);
        zcopy_ack->sender_req_id = rreq->dev.sender_req_id;

        vbuf_init_send(v, sizeof(MPIDI_CH3_Pkt_zcopy_ack_t), hca_index);
        /* Increment the count to switch to RC protocol */
        vc->mrail.rely.total_messages++;
        post_ud_send(vc, v, hca_index, NULL);
    }
}

static inline void mvp_flush_zcopy_rndv_qp(mvp_rndv_qp_t *rqp, int hca_index,
                                           int num_to_flush)
{
    int ne, count;
    struct ibv_qp_attr qp_attr;
    struct ibv_wc wc;

    memset(&qp_attr, 0, sizeof(qp_attr));
    qp_attr.qp_state = IBV_QPS_ERR;

    /* Transition to error state to flush remaining buffers */
    if (ibv_ops.modify_qp(rqp->ud_qp[hca_index], &qp_attr, IBV_QP_STATE)) {
        ibv_error_abort(IBV_RETURN_ERR, "Error in changing QP state to err\n");
    }

    /* Flush remaining buffers */
    count = 0;
    do {
        ne = ibv_poll_cq(rqp->ud_cq[hca_index], 1, &wc);
        count += ne;
    } while (count < num_to_flush);

    memset(&qp_attr, 0, sizeof(qp_attr));
    qp_attr.qp_state = IBV_QPS_RESET;

    if (ibv_ops.modify_qp(rqp->ud_qp[hca_index], &qp_attr, IBV_QP_STATE)) {
        ibv_error_abort(IBV_RETURN_ERR, "Error in changing QP state to err\n");
    }

    /* transiotion to RTS state */
    if (mvp_ud_qp_transition(rqp->ud_qp[hca_index], hca_index)) {
        ibv_error_abort(IBV_RETURN_ERR,
                        "Error in changing RNDV UD QP transition\n");
    }
}

static inline void mvp_ud_post_zcopy_recv(MPIR_Request *req,
                                          mvp_ud_zcopy_info_t *zcopy_info)
{
    int i = 0, hca_num = 0;
    int bytes_to_post = 0, curr_len = 0;
    int posted_buffers = 0, posts_required = 0;
    int posted_buffers_per_hca[MAX_NUM_HCAS] = {0};
    int posts_required_per_hca[MAX_NUM_HCAS] = {0};
    struct ibv_recv_wr *bad_wr = NULL;
    struct ibv_recv_wr rr[rdma_ud_zcopy_push_segment];
    struct ibv_sge sge_entry[rdma_ud_zcopy_push_segment * 2];
    mvp_rndv_qp_t *rqp = NULL;

    posts_required =
        ((req->mrail.rndv_buf_sz + MRAIL_MAX_UD_SIZE - 1) / MRAIL_MAX_UD_SIZE);
    if (posts_required <= 0) {
        PRINT_DEBUG(DEBUG_ZCY_verbose > 0, "Posted zero or less buffers :%d\n",
                    posts_required);
    }
    PRINT_DEBUG(DEBUG_ZCY_verbose > 0, "Posts required :%d, HCAS: %d\n",
                posts_required, req->mrail.num_hcas);

    /* For segments that are not fully divisible, send out the remainder through
     * the first rail */
    for (hca_num = 0; hca_num < req->mrail.num_hcas; ++hca_num) {
        posts_required_per_hca[hca_num] =
            (posts_required / req->mrail.num_hcas);
        PRINT_DEBUG(DEBUG_ZCY_verbose > 0, "Posts for HCA %d = %d\n", hca_num,
                    posts_required_per_hca[hca_num]);
    }
    posts_required_per_hca[0] +=
        posts_required - (posts_required_per_hca[0] * req->mrail.num_hcas);
    PRINT_DEBUG(DEBUG_ZCY_verbose > 0, "Posts for HCA 0 = %d\n",
                posts_required_per_hca[0]);

    rqp = req->mrail.rndv_qp_entry;
    posted_buffers = 0;
    while (posted_buffers < posts_required) {
        for (hca_num = 0; hca_num < req->mrail.num_hcas; ++hca_num) {
            if (posted_buffers_per_hca[hca_num] >=
                posts_required_per_hca[hca_num]) {
                continue;
            }
            PRINT_DEBUG(
                DEBUG_ZCY_verbose > 0,
                "Preparing recv WQE HCA %d. posted_buffers = %d, req = %d\n",
                hca_num, posted_buffers_per_hca[hca_num],
                posts_required_per_hca[hca_num]);
            for (i = 0; i < rdma_ud_zcopy_push_segment - 1; i++) {
                bytes_to_post =
                    MIN(MRAIL_MAX_UD_SIZE, (req->mrail.rndv_buf_sz - curr_len));
                if (i > 0) {
                    rr[i - 1].next = &(rr[i]);
                }

                rr[i].next = NULL;
                rr[i].wr_id = posted_buffers_per_hca[hca_num];
                rr[i].num_sge = 2;
                rr[i].sg_list = &(sge_entry[i * 2]);

                sge_entry[i * 2].addr = (uintptr_t)zcopy_info->grh_buf;
                sge_entry[i * 2].length = 40;
                sge_entry[i * 2].lkey = ((dreg_entry *)(zcopy_info->grh_mr))
                                            ->memhandle[hca_num]
                                            ->lkey;

                sge_entry[i * 2 + 1].addr =
                    (uintptr_t)(req->mrail.rndv_buf + curr_len);
                sge_entry[i * 2 + 1].length = bytes_to_post;
                sge_entry[i * 2 + 1].lkey = ((dreg_entry *)(req->mrail.d_entry))
                                                ->memhandle[hca_num]
                                                ->lkey;

                curr_len += bytes_to_post;
                posted_buffers++;
                posted_buffers_per_hca[hca_num]++;
                if (posted_buffers_per_hca[hca_num] >=
                    posts_required_per_hca[hca_num]) {
                    i++;
                    break;
                }
            }
            PRINT_DEBUG(DEBUG_ZCY_verbose > 0,
                        "Posted %d/%d bytes %d (%d overall) for HCA %d\n",
                        bytes_to_post, curr_len, i, posted_buffers, hca_num);

            if (ibv_post_recv(rqp->ud_qp[hca_num], rr, &bad_wr)) {
                ibv_error_abort(IBV_RETURN_ERR,
                                "Failed to post zcopy rndv buf\n");
            }
        }
    }

    PRINT_DEBUG(DEBUG_ZCY_verbose > 0, "Posted zcopy recv buffers:%d\n",
                posted_buffers);

    MPIR_Assert(curr_len == req->mrail.rndv_buf_sz);
}

void MPIDI_CH3I_MRAIL_Prepare_rndv_zcopy(MPIDI_VC_t *vc, MPIR_Request *req)
{
    mvp_rndv_qp_t *rqp = NULL;
    mvp_MPIDI_CH3I_RDMA_Process_t *proc = &mvp_MPIDI_CH3I_RDMA_Process;

    /* Make sure free QPs available */
    if (proc->zcopy_info.rndv_qp_pool_free_head == NULL) {
        PRINT_DEBUG(DEBUG_ZCY_verbose > 2,
                    "No free rndv QP, fall back to R3. remote:%d\n",
                    vc->pg_rank);
        req->mrail.protocol = MRAILI_PROTOCOL_R3;
        proc->zcopy_info.no_free_rndv_qp++;
        return;
    }

    MPIDI_CH3I_MRAIL_Prepare_rndv(vc, req);

    /* return if selected protocol is R3 */
    if (req->mrail.protocol == MRAILI_PROTOCOL_R3) {
        return;
    }

    PRINT_DEBUG(DEBUG_ZCY_verbose > 1,
                "Received RTS. Preparing zcopy rndv remote:%d\n", vc->pg_rank);

    MPIR_Assert(req->mrail.protocol == MRAILI_PROTOCOL_UD_ZCOPY);

    req->mrail.num_hcas = rdma_num_hcas;
    MVP_GET_RNDV_QP(rqp, proc);
    req->mrail.rndv_qp_entry = rqp;

    mvp_ud_post_zcopy_recv(req, &proc->zcopy_info);
}

void mvp_ud_zcopy_poll_cq(mvp_ud_zcopy_info_t *zcopy_info, mvp_ud_ctx_t *ud_ctx,
                          vbuf *resend_buf, int hca_index, int *found)
{
    int ne, i;
    struct ibv_wc wc_list[rdma_ud_zcopy_push_segment];

    ne = ibv_poll_cq(zcopy_info->rndv_ud_cqs[hca_index],
                     rdma_ud_zcopy_push_segment, wc_list);
    if (ne < 0) {
        ibv_error_abort(IBV_RETURN_ERR, "Error in polling RNDV CQ\n");
    } else if (ne > 0) {
        for (i = 0; i < ne; i++) {
            if (wc_list[i].status != IBV_WC_SUCCESS) {
                ibv_va_error_abort(IBV_STATUS_ERR,
                                   "Error in UD RNDV"
                                   " ibv_poll got completion with"
                                   " error code %d, wr_id: %lu\n",
                                   wc_list[i].status, wc_list[i].wr_id);
            }
            /* handle zcopy finish control message */
            if (wc_list[i].wr_id > (uint64_t)rdma_ud_zcopy_push_segment) {
                vbuf *v = (vbuf *)((uintptr_t)wc_list[i].wr_id);
                v->pending_send_polls--;
                if (v->pending_send_polls == 0) {
                    v->flags &= ~(UD_VBUF_SEND_INPROGRESS);
                }
                if (v->flags & UD_VBUF_FREE_PENIDING && v != resend_buf) {
                    v->flags &= ~(UD_VBUF_FREE_PENIDING);
                    v->flags &= ~(UD_VBUF_RETRY_ALWAYS);
                    MRAILI_Release_vbuf(v);
                }
                ud_ctx->send_wqes_avail++;
            } else {
                ud_ctx->send_wqes_avail += (int)wc_list[i].wr_id;
            }
        }
        *found = 1;
    } else {
        *found = 0;
    }
}

void MPIDI_CH3I_MRAILI_Rendezvous_zcopy_push(MPIDI_VC_t *vc, MPIR_Request *sreq,
                                             mvp_ud_zcopy_info_t *zcopy_info)
{
    int seqnum[MAX_NUM_HCAS] = {0};
    int i = 0, hca_num = 0, found = 0;
    int bytes_to_post = 0;
    int posts_required = 0;
    int posted_buffers_per_hca[MAX_NUM_HCAS] = {0};
    int posts_required_per_hca[MAX_NUM_HCAS] = {0};
    struct ibv_send_wr *bad_wr = NULL;
    struct ibv_send_wr sr[rdma_ud_zcopy_push_segment];
    struct ibv_sge sg_entry[rdma_ud_zcopy_push_segment];
    mvp_ud_ctx_t *ud_ctx = NULL;
    PRINT_DEBUG(DEBUG_ZCY_verbose > 0,
                "ZCOPY rndv push remote hcas:%d "
                "remote:%d\n",
                sreq->mrail.num_hcas, vc->pg_rank);

    posts_required =
        ((sreq->mrail.rndv_buf_sz + MRAIL_MAX_UD_SIZE - 1) / MRAIL_MAX_UD_SIZE);
    if (posts_required <= 0) {
        PRINT_DEBUG(DEBUG_ZCY_verbose > 0, "Posted zero or less buffers :%d\n",
                    posts_required);
    }

    PRINT_DEBUG(DEBUG_ZCY_verbose > 0, "Posts required :%d, HCAS: %d\n",
                posts_required, sreq->mrail.num_hcas);

    /* For segments that are not fully divisible, send out the remainder through
     * the first rail */
    for (hca_num = 0; hca_num < sreq->mrail.num_hcas; ++hca_num) {
        posts_required_per_hca[hca_num] =
            (posts_required / sreq->mrail.num_hcas);
        PRINT_DEBUG(DEBUG_ZCY_verbose > 0, "Posts for HCA %d = %d\n", hca_num,
                    posts_required_per_hca[hca_num]);
    }
    posts_required_per_hca[0] +=
        posts_required - (posts_required_per_hca[0] * sreq->mrail.num_hcas);
    PRINT_DEBUG(DEBUG_ZCY_verbose > 0, "Posts for HCA 0 = %d\n",
                posts_required_per_hca[0]);

    while (sreq->mrail.rndv_buf_off < sreq->mrail.rndv_buf_sz) {
        for (hca_num = 0; hca_num < sreq->mrail.num_hcas; ++hca_num) {
            /* If we have finished posting required buffers, skip this HCA */
            if (posted_buffers_per_hca[hca_num] >=
                posts_required_per_hca[hca_num]) {
                continue;
            }
            ud_ctx = zcopy_info->rndv_ud_qps[hca_num];
            do {
                mvp_ud_zcopy_poll_cq(zcopy_info, ud_ctx, NULL, hca_num, &found);
            } while (ud_ctx->send_wqes_avail < rdma_ud_zcopy_push_segment ||
                     found);
            /* Post one chunk of data through this HCA */
            for (i = 0; i < rdma_ud_zcopy_push_segment - 1; i++) {
                sr[i].send_flags = 0;
                sr[i].next = &(sr[i + 1]);
                sr[i].wr_id = (uint64_t)0;

                sr[i].num_sge = 1;
                sr[i].wr.ud.remote_qkey = 0;

                sr[i].opcode = IBV_WR_SEND_WITH_IMM;
                sr[i].wr.ud.remote_qpn = sreq->mrail.remote_qpn[hca_num];
                sr[i].wr.ud.ah = vc->mrail.ud[hca_num].ah;

                bytes_to_post =
                    MIN(sreq->mrail.rndv_buf_sz - sreq->mrail.rndv_buf_off,
                        MRAIL_MAX_UD_SIZE);
                sr[i].sg_list = &(sg_entry[i]);
                sr[i].imm_data = seqnum[hca_num]++;
                sg_entry[i].addr =
                    (uint64_t)(uintptr_t)((char *)sreq->mrail.rndv_buf +
                                          sreq->mrail.rndv_buf_off);
                sg_entry[i].length = bytes_to_post;
                sg_entry[i].lkey = ((dreg_entry *)sreq->mrail.d_entry)
                                       ->memhandle[hca_num]
                                       ->lkey;

                ud_ctx->send_wqes_avail--;

                sreq->mrail.rndv_buf_off += bytes_to_post;

                posted_buffers_per_hca[hca_num]++;
                if (posted_buffers_per_hca[hca_num] >=
                    posts_required_per_hca[hca_num]) {
                    i++;
                    break;
                }
            }
            /* Only last work request sends a complition */
            sr[i - 1].send_flags = IBV_SEND_SIGNALED;
            sr[i - 1].next = NULL;
            sr[i - 1].wr_id = (uint64_t)i;

            PRINT_DEBUG(DEBUG_ZCY_verbose > 0,
                        "Posted %d bytes %d/%d for HCA %d\n", bytes_to_post,
                        posted_buffers_per_hca[hca_num],
                        posts_required_per_hca[hca_num], hca_num);

            if (ibv_post_send(ud_ctx->qp, sr, &bad_wr)) {
                ibv_va_error_abort(IBV_RETURN_ERR,
                                   "Error in posting UD RNDV QP %d %lu",
                                   ud_ctx->send_wqes_avail, bad_wr->wr_id);
            }
        }
    }

    for (hca_num = 0; hca_num < sreq->mrail.num_hcas; ++hca_num) {
        PRINT_DEBUG(DEBUG_ZCY_verbose > 1,
                    "Posted rndv send posts:%d on HCA %d to:%d\n",
                    seqnum[hca_num], hca_num, vc->pg_rank);
    }

    MPIR_Assert(sreq->mrail.rndv_buf_off == sreq->mrail.rndv_buf_sz);
    /* send finish msg*/
    MRAILI_Rndv_send_zcopy_finish(vc, sreq, zcopy_info);
    sreq->mrail.nearly_complete = 1;
}

void MPIDI_CH3_Rendezvous_zcopy_finish(
    MPIDI_VC_t *vc, MPIDI_CH3_Pkt_zcopy_finish_t *zcopy_finish)
{
    int i, ne, posted_buffers, count = 0, complete, hca_num = 0,
                               num_retries = 0;
    int count_per_hca[MAX_NUM_HCAS] = {0},
        posted_buffers_per_hca[MAX_NUM_HCAS] = {0};
    int out_of_order = 0, next_to_recv[MAX_NUM_HCAS] = {0}, empty = 0;

    static int num_retry_print = 0;

    struct ibv_wc *wc;
    MPIR_Request *rreq;
    mvp_rndv_qp_t *rqp;

    MPIR_Request_get_ptr(zcopy_finish->receiver_req_id, rreq);

    rreq->mrail.remote_complete++;

    if (rreq->mrail.remote_complete < rdma_num_hcas) {
        return;
    }
    PRINT_DEBUG(DEBUG_ZCY_verbose, "Rcvd %d zcopy_finish packets from %d\n",
                rreq->mrail.remote_complete, vc->pg_rank);

    posted_buffers =
        ((rreq->mrail.rndv_buf_sz + MRAIL_MAX_UD_SIZE - 1) / MRAIL_MAX_UD_SIZE);
    /* For segments that are not fully divisible, send out the remainder through
     * the first rail */
    for (hca_num = 0; hca_num < rreq->mrail.num_hcas; ++hca_num) {
        posted_buffers_per_hca[hca_num] =
            (posted_buffers / rreq->mrail.num_hcas);
        PRINT_DEBUG(DEBUG_ZCY_verbose > 0, "Expected recvs for HCA %d = %d\n",
                    hca_num, posted_buffers_per_hca[hca_num]);
    }
    posted_buffers_per_hca[0] +=
        posted_buffers - (posted_buffers_per_hca[0] * rreq->mrail.num_hcas);
    PRINT_DEBUG(DEBUG_ZCY_verbose > 0, "Expected recvs for HCA 0 = %d\n",
                posted_buffers_per_hca[0]);

    wc = (struct ibv_wc *)MPL_malloc(
        sizeof(struct ibv_wc) * posted_buffers_per_hca[0], MPL_MEM_BUFFER);

    int need_resend = 0;
zcopy_recv_polling:
    for (hca_num = 0; hca_num < rreq->mrail.num_hcas; ++hca_num) {
        rqp = rreq->mrail.rndv_qp_entry;
        out_of_order = 0;
        do {
            PRINT_DEBUG(DEBUG_ZCY_verbose > 0, "Polling CQ for %d. %d/%d\n",
                        hca_num, count_per_hca[hca_num],
                        posted_buffers_per_hca[hca_num]);
            ne = ibv_poll_cq(rqp->ud_cq[hca_num],
                             posted_buffers_per_hca[hca_num], wc);
            if (ne < 0) {
                ibv_error_abort(IBV_RETURN_ERR,
                                "Error in polling recv RNDV UD QP\n");
            } else if (ne > 0) {
                PRINT_DEBUG(DEBUG_ZCY_verbose > 0,
                            "Rvcd %d completions on CQ for HCA %d\n", ne,
                            hca_num);
                for (i = 0; i < ne; i++) {
                    if (wc[i].status != IBV_WC_SUCCESS) {
                        ibv_va_error_abort(
                            IBV_RETURN_ERR,
                            "Got error completion "
                            "in RNDV UD recv error. code: %d, wr_id: %lu",
                            wc[i].status, wc[i].wr_id);
                    } else {
                        if (IBV_WC_RECV == wc[i].opcode) {
                            if (wc[i].imm_data != next_to_recv[hca_num]) {
                                PRINT_DEBUG(
                                    DEBUG_ZCY_verbose > 0,
                                    "Out of Order Rndv "
                                    "recv'ed:%d expected:%ls remote:%d\n",
                                    wc[i].imm_data, next_to_recv, vc->pg_rank);
                                out_of_order = 1;
                            }
                            next_to_recv[hca_num]++;
                        }
                    }
                    count_per_hca[hca_num]++;
                    /* Aggregate count */
                    count++;
                }
            } else {
                empty = 1;
            }
        } while (!empty &&
                 posted_buffers_per_hca[hca_num] != count_per_hca[hca_num]);

        PRINT_DEBUG(DEBUG_ZCY_verbose > 1,
                    "Done polling RNDV UD. got %d of %d. remote:%d\n", count,
                    posted_buffers, vc->pg_rank);

        if (count_per_hca[hca_num] != posted_buffers_per_hca[hca_num] ||
            out_of_order) {
            PRINT_DEBUG(
                DEBUG_ZCY_verbose,
                "Zcopy Rndv recv failed. "
                "posted: %d recv'ed: %d out_of_order:%d remote:%d hca: %d\n",
                posted_buffers_per_hca[hca_num], count_per_hca[hca_num],
                out_of_order, vc->pg_rank, hca_num);

            if (!out_of_order) {
                if (num_retries < rdma_ud_zcopy_num_retry) {
                    if (!rdma_ud_zcopy_enable_polling) {
                        num_retries++;
                    }
                    goto zcopy_recv_polling;
                }

                if (num_retry_print == 0 && !rdma_ud_zcopy_enable_polling) {
                    num_retry_print = 1;
                    PRINT_ERROR(
                        "[Performance Impact Warning]: Out-of-order Packets "
                        "detected. "
                        "Can you please try to set MVP_UD_ZCOPY_NUM_RETRY to "
                        "larger value"
                        "(current value: %d) to get better performance,\n",
                        rdma_ud_zcopy_num_retry);
                }
            }
        }
        if (posted_buffers_per_hca[hca_num] != count_per_hca[hca_num]) {
            MPIR_Assert(posted_buffers_per_hca[hca_num] >
                        count_per_hca[hca_num]);
            need_resend = 1;
            break;
        }
    }

    if (need_resend) {
        for (hca_num = 0; hca_num < rreq->mrail.num_hcas; ++hca_num) {
            rqp = rreq->mrail.rndv_qp_entry;
            mvp_flush_zcopy_rndv_qp(rqp, hca_num,
                                    posted_buffers_per_hca[hca_num] -
                                        count_per_hca[hca_num]);
        }
        mvp_ud_post_zcopy_recv(rreq, &mvp_MPIDI_CH3I_RDMA_Process.zcopy_info);
        MPIDI_CH3_Rendezvous_zcopy_resend_cts(vc, rreq);
    } else {
        /* Send zcopy ack */
        MRAILI_Rndv_send_zcopy_ack(vc, rreq);

        if (rreq->mrail.rndv_buf_alloc == 1) {
            MPIDI_CH3_Rendezvous_unpack_data(vc, rreq);
        } else {
            rreq->mrail.rndv_buf = NULL;
        }
        /* Release RNDV QP */
        rqp = rreq->mrail.rndv_qp_entry;
        MVP_RELEASE_RNDV_QP(rqp, &mvp_MPIDI_CH3I_RDMA_Process);
        /* Mark request as finished */
        MPIDI_CH3I_MRAILI_RREQ_RNDV_FINISH(rreq);
        MPIDI_CH3U_Handle_recv_req(vc, rreq, &complete);
        if (complete == TRUE) {
            vc->ch.recv_active = NULL;
        } else {
            ibv_error_abort(IBV_RETURN_ERR, "Error in UD RNDV completion\n");
        }
    }

    MPL_free(wc);
}

void MPIDI_CH3_Rendezvous_zcopy_ack(MPIDI_VC_t *vc,
                                    MPIDI_CH3_Pkt_zcopy_ack_t *zcopy_ack)
{
    int complete;
    MPIR_Request *sreq;

    MPIR_Request_get_ptr(zcopy_ack->sender_req_id, sreq);

    sreq->mrail.remote_complete++;

    if (sreq->mrail.remote_complete < rdma_num_hcas) {
        PRINT_DEBUG(DEBUG_ZCY_verbose > 1, "zcopy ack %d/%d received from:%d\n",
                    sreq->mrail.remote_complete, rdma_num_hcas, vc->pg_rank);
        return;
    }

    PRINT_DEBUG(DEBUG_ZCY_verbose > 1,
                "Zcopy ack received from:%d. Finishing req\n", vc->pg_rank);
    MPIDI_CH3I_MRAILI_RREQ_RNDV_FINISH(sreq);
    MPIDI_CH3U_Handle_send_req(vc, sreq, &complete);
    MPIR_Assert(complete == TRUE);
}

#endif /* _ENABLE_UD_ */
