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

#include "rdma_impl.h"
#include "vbuf.h"

#ifdef _ENABLE_UD_
#include "cm.h"
#include "mvp_ud.h"
#include "mvp_ud_inline.h"
#include "ibv_send_inline.h"
#include <mvp_debug_utils.h>

extern unsigned long PVAR_COUNTER_mvp_vbuf_allocated;
extern unsigned long PVAR_COUNTER_mvp_vbuf_freed;
extern unsigned long PVAR_LEVEL_mvp_vbuf_available;
extern unsigned long PVAR_COUNTER_mvp_ud_vbuf_allocated;
extern unsigned long PVAR_COUNTER_mvp_ud_vbuf_freed;
extern unsigned long PVAR_LEVEL_mvp_ud_vbuf_available;

static inline void mvp_ud_flush_ext_window(MPIDI_VC_t *vc)
{
    vbuf *next;
    message_queue_t *q = &vc->mrail.rely.ext_window;
    while (q->head != NULL &&
           vc->mrail.rely.send_window.count < rdma_default_ud_sendwin_size) {
        next = (q->head)->extwin_msg.next;
        post_send(vc, q->head, (q->head)->rail);
        PRINT_DEBUG(DEBUG_UD_verbose > 1,
                    "Send ext window message(%p) nextseqno :"
                    "%d\n",
                    next, vc->mrail.seqnum_next_tosend);
        q->head = next;
        --q->count;
        rdma_global_ext_sendq_size--;
        vc->mrail.rely.ext_win_send_count++;
    }
    if (q->head == NULL) {
        MPIR_Assert(q->count == 0);
        q->tail = NULL;
    }
}

static inline void mvp_unack_queue_clear(MPIDI_VC_t *vc)
{
    vbuf *sendwin_head = vc->mrail.rely.send_window.head;

    while (sendwin_head != NULL) {
        PRINT_DEBUG(DEBUG_UD_verbose > 2,
                    "Clearing seqnum %d from unack queue for rank %d\n",
                    sendwin_head->seqnum, vc->pg_rank);
        mvp_ud_send_window_remove(&vc->mrail.rely.send_window, sendwin_head);
        mvp_ud_unack_queue_remove(&(mvp_MPIDI_CH3I_RDMA_Process.unack_queue),
                                  sendwin_head);
        MRAILI_Process_send(sendwin_head);
        sendwin_head = vc->mrail.rely.send_window.head;
    }
}

static inline void mvp_ud_process_ack(MPIDI_VC_t *vc, uint16_t acknum)
{
    vbuf *sendwin_head = vc->mrail.rely.send_window.head;

    PRINT_DEBUG(DEBUG_UD_verbose > 2,
                "ack: %d received from rank: %d, next_to_ack: %d\n", acknum,
                vc->pg_rank, vc->mrail.seqnum_next_toack);

    while (sendwin_head != NULL && INCL_BETWEEN(acknum, sendwin_head->seqnum,
                                                vc->mrail.seqnum_next_tosend)) {
        PRINT_DEBUG(DEBUG_UD_verbose > 2,
                    "Removing seqnum %d from unack queue for rank %d\n",
                    sendwin_head->seqnum, vc->pg_rank);
        mvp_ud_send_window_remove(&vc->mrail.rely.send_window, sendwin_head);
        mvp_ud_unack_queue_remove(&(mvp_MPIDI_CH3I_RDMA_Process.unack_queue),
                                  sendwin_head);
        MRAILI_Process_send(sendwin_head);
        sendwin_head = vc->mrail.rely.send_window.head;
    }

    /*see if we can flush from ext window queue */
    if (vc->mrail.rely.ext_window.head != NULL &&
        vc->mrail.rely.send_window.count < rdma_default_ud_sendwin_size) {
        mvp_ud_flush_ext_window(vc);
    }
}

static inline void mvp_ud_place_recvwin(vbuf *v)
{
    MPIDI_VC_t *vc;
    int recv_win_start;
    int recv_win_end;
    int ret;

    vc = v->vc;
    recv_win_start = vc->mrail.seqnum_next_torecv;
    recv_win_end =
        (recv_win_start + rdma_default_ud_recvwin_size) % MAX_SEQ_NUM;

    /* check if the packet is in the window or not */
    if (INCL_BETWEEN(v->seqnum, recv_win_start, recv_win_end) ||
        (v->transport != IB_TRANSPORT_UD)) {
        if (v->seqnum == vc->mrail.seqnum_next_torecv) {
            PRINT_DEBUG(DEBUG_UD_verbose > 2,
                        "Get one %s pkt from %d with in-order seqnum:%d, "
                        "next_to_ack:%d, next_exp = %u \n",
                        (v->transport == IB_TRANSPORT_UD) ? "UD" : "RC",
                        vc->pg_rank, v->seqnum, vc->mrail.seqnum_next_torecv,
                        (vc->mrail.seqnum_next_torecv + 1));
            vc->mrail.seqnum_next_toack = vc->mrail.seqnum_next_torecv;
            /* process in-order message */
            handle_read(vc, v);
            ++vc->mrail.seqnum_next_torecv;
            if (v->transport == IB_TRANSPORT_UD) {
                MARK_ACK_REQUIRED(vc);
            }
        } else {
            /* we are not in order */
            PRINT_DEBUG(
                DEBUG_UD_verbose > 1,
                "Get one out-of-order %s packet from %d recv:%d expected:%d\n",
                (v->transport == IB_TRANSPORT_UD) ? "UD" : "RC", vc->pg_rank,
                v->seqnum, vc->mrail.seqnum_next_torecv);
            ret = mvp_ud_recv_window_add(&vc->mrail.rely.recv_window, v,
                                         vc->mrail.seqnum_next_torecv);
            if (ret == MSG_IN_RECVWIN) {
                MPIR_Assert(v->transport == IB_TRANSPORT_UD);
                PRINT_DEBUG(DEBUG_UD_verbose > 1,
                            "Releasing out-of-order %s packet from %d recv:%d "
                            "expected:%d\n",
                            (v->transport == IB_TRANSPORT_UD) ? "UD" : "RC",
                            vc->pg_rank, v->seqnum,
                            vc->mrail.seqnum_next_torecv);
                MPIDI_CH3I_MRAIL_Release_vbuf(v);
            }

            if (v->transport == IB_TRANSPORT_UD) {
                MARK_ACK_REQUIRED(vc);
            }
        }

        /* process in-order messages in recv windiw head */
        while (vc->mrail.rely.recv_window.head != NULL &&
               (vc->mrail.rely.recv_window.head->seqnum ==
                vc->mrail.seqnum_next_torecv)) {
            PRINT_DEBUG(DEBUG_UD_verbose > 2,
                        "Process one %s pkt from %d with in-order seqnum:%d, "
                        "next_exp: %u \n",
                        (v->transport == IB_TRANSPORT_UD) ? "UD" : "RC",
                        vc->pg_rank, vc->mrail.seqnum_next_torecv,
                        (vc->mrail.seqnum_next_torecv + 1));
            vc->mrail.seqnum_next_toack = vc->mrail.seqnum_next_torecv;
            handle_read(vc, vc->mrail.rely.recv_window.head);
            mvp_ud_recv_window_remove(&vc->mrail.rely.recv_window);
            ++vc->mrail.seqnum_next_torecv;
        }
    } else {
        /* We should never drop a packet that is not sent over UD */
        MPIR_Assert(v->transport == IB_TRANSPORT_UD);
        PRINT_DEBUG(DEBUG_UD_verbose > 1,
                    "Message from %d is not in recv window seqnum:%d win "
                    "start:%d win end:%d\n",
                    vc->pg_rank, v->seqnum, recv_win_start, recv_win_end);
        MPIDI_CH3I_MRAIL_Release_vbuf(v);
        MARK_ACK_REQUIRED(vc);
    }
}

int post_ud_send(MPIDI_VC_t *vc, vbuf *v, int rail, mvp_ud_ctx_t *send_ud_ctx)
{
    mvp_ud_ctx_t *ud_ctx = send_ud_ctx;
    MPIDI_CH3I_MRAILI_Pkt_comm_header *p = v->pheader;

    MPIR_Assert(v->desc.sg_entry.length <= MRAIL_MAX_UD_SIZE);
    if (send_ud_ctx == NULL) {
        ud_ctx = mvp_MPIDI_CH3I_RDMA_Process.ud_rails[rail];
    }
    v->vc = (void *)vc;
    p->rail = rail;
    p->src.rank = MPIDI_Process.my_pg_rank;
    MPIR_Assert(v->transport == IB_TRANSPORT_UD);

    SEND_WINDOW_CHECK(&vc->mrail.rely, v);

    v->seqnum = p->seqnum = vc->mrail.seqnum_next_tosend;
    vc->mrail.seqnum_next_tosend++;

    p->acknum = vc->mrail.seqnum_next_toack;
    MARK_ACK_COMPLETED(vc);
    MVP_UD_RESET_CREDITS(vc, v);
    v->flags |= UD_VBUF_SEND_INPROGRESS;

    if (unlikely(vc->mrail.ud == NULL)) {
        if (likely(vc->pg->ch.mrail->cm_shmem.ud_cm[vc->pg_rank].cm_lid == 0)) {
            MPICM_lock();
            PRINT_DEBUG(DEBUG_CM_verbose > 0,
                        "Calling MPIDI_CH3I_PMI_Get_Init_Info for %d\n",
                        vc->pg_rank);
            MPIDI_CH3I_PMI_Get_Init_Info(vc->pg, vc->pg_rank, NULL);
            MPICM_unlock();
        }
        PRINT_DEBUG(DEBUG_CM_verbose > 0,
                    "Calling MPIDI_CH3I_UD_Generate_addr_handle_for_rank %d\n",
                    vc->pg_rank);
        MPIDI_CH3I_UD_Generate_addr_handle_for_rank(vc->pg, vc->pg_rank);
    }

    PRINT_DEBUG(DEBUG_UD_verbose > 1,
                "UD send to rank %d seqnum:%d acknum:%d len:%d rail:%d\n",
                vc->pg_rank, p->seqnum, p->acknum, v->desc.sg_entry.length,
                rail);

    IBV_UD_POST_SR(v, vc->mrail.ud[rail], ud_ctx);

    mvp_ud_track_send(&vc->mrail.rely, &mvp_MPIDI_CH3I_RDMA_Process.unack_queue,
                      v);

    return 0;
}

void mvp_send_control_msg(MPIDI_VC_t *vc, vbuf *v)
{
    mvp_ud_ctx_t *ud_ctx;
    MPIDI_CH3I_MRAILI_Pkt_comm_header *p = v->pheader;

    ud_ctx = mvp_MPIDI_CH3I_RDMA_Process.ud_rails[p->rail];
    v->vc = (void *)vc;
    p->src.rank = MPIDI_Process.my_pg_rank;
    MPIR_Assert(v->transport == IB_TRANSPORT_UD);

    v->seqnum = p->seqnum = -1;
    MARK_ACK_COMPLETED(vc);
    MVP_UD_RESET_CREDITS(vc, v);

    if (unlikely(vc->mrail.ud == NULL)) {
        if (likely(vc->pg->ch.mrail->cm_shmem.ud_cm[vc->pg_rank].cm_lid == 0)) {
            MPICM_lock();
            PRINT_DEBUG(DEBUG_CM_verbose > 0,
                        "Calling MPIDI_CH3I_PMI_Get_Init_Info for %d\n",
                        vc->pg_rank);
            MPIDI_CH3I_PMI_Get_Init_Info(vc->pg, vc->pg_rank, NULL);
            MPICM_unlock();
        }
        PRINT_DEBUG(DEBUG_CM_verbose > 0,
                    "Calling MPIDI_CH3I_UD_Generate_addr_handle_for_rank %d\n",
                    vc->pg_rank);
        MPIDI_CH3I_UD_Generate_addr_handle_for_rank(vc->pg, vc->pg_rank);
    }

    IBV_UD_POST_SR(v, vc->mrail.ud[p->rail], ud_ctx);

    vc->mrail.rely.cntl_acks++;
}

static inline void mvp_ud_ext_sendq_send(MPIDI_VC_t *vc, mvp_ud_ctx_t *ud_ctx)
{
    vbuf *v;
    while (ud_ctx->send_wqes_avail > 0 && ud_ctx->ext_send_queue.head) {
        v = ud_ctx->ext_send_queue.head;
        ud_ctx->ext_send_queue.head = ud_ctx->ext_send_queue.head->desc.next;
        if (NULL == ud_ctx->ext_send_queue.head) {
            ud_ctx->ext_send_queue.tail = NULL;
        }
        ud_ctx->ext_send_queue.count--;
        rdma_global_ext_sendq_size--;
        v->in_ud_ext_sendq = 0;
        v->desc.next = NULL;

        /* can we reset ack to latest? */
        ud_ctx->send_wqes_avail--;
        /* Keep track of the number of times we sent this out */
        v->pending_send_polls++;
        if (ibv_post_send(ud_ctx->qp, &(v->desc.u.sr), &(v->desc.y.bad_sr))) {
            ibv_error_abort(-1, "extend sendq send  failed");
        }
        ud_ctx->ext_sendq_count++;
        PRINT_DEBUG(DEBUG_UD_verbose > 1,
                    "sending ext send queue to rank %d seqnum :%d qlen:%d\n",
                    vc->pg_rank, v->seqnum, ud_ctx->ext_send_queue.count);
    }
}
void mvp_ud_update_send_credits(vbuf *v)
{
    mvp_ud_ctx_t *ud_ctx;
    ud_ctx = mvp_MPIDI_CH3I_RDMA_Process.ud_rails[v->rail];
    ud_ctx->send_wqes_avail++;
    PRINT_DEBUG(DEBUG_UD_verbose > 2, "available wqes : %d seqno:%d \n",
                ud_ctx->send_wqes_avail, v->seqnum);
    if (NULL != ud_ctx->ext_send_queue.head && ud_ctx->send_wqes_avail > 0) {
        mvp_ud_ext_sendq_send(v->vc, ud_ctx);
    }
}

void mvp_send_explicit_ack(MPIDI_VC_t *vc)
{
    vbuf *v = NULL;
    GET_UD_VBUF_BY_OFFSET_WITHOUT_LOCK(v, MVP_SMALL_SEND_UD_VBUF_POOL_OFFSET);
    MPIDI_CH3I_MRAILI_Pkt_comm_header *ack_pkt = v->pheader;
    MPIDI_Pkt_init(ack_pkt, MPIDI_CH3_PKT_FLOW_CNTL_UPDATE);
    ack_pkt->acknum = vc->mrail.seqnum_next_toack;
    vbuf_init_send(v, sizeof(MPIDI_CH3I_MRAILI_Pkt_comm_header), 0);
    ack_pkt->rail = v->rail;
    mvp_send_control_msg(vc, v);
    PRINT_DEBUG(DEBUG_UD_verbose > 1, "Sent explicit ACK to :%d acknum:%d\n",
                vc->pg_rank, ack_pkt->acknum);
}

int mvp_ud_resend(vbuf *v)
{
    int resend = 0;
    MPIDI_CH3I_MRAILI_Pkt_comm_header *p;
    MPIDI_VC_t *vc;
    mvp_ud_ctx_t *ud_ctx;

    if ((v->flags & UD_VBUF_SEND_INPROGRESS) &&
        !(v->flags & UD_VBUF_RETRY_ALWAYS)) {
        return resend;
    }

    p = v->pheader;
    vc = v->vc;

    if (p->type == MPIDI_CH3_PKT_ZCOPY_FINISH) {
        int found;
        int hca_index = ((MPIDI_CH3_Pkt_zcopy_finish_t *)p)->hca_index;
        ud_ctx = mvp_MPIDI_CH3I_RDMA_Process.zcopy_info.rndv_ud_qps[hca_index];
        do {
            mvp_ud_zcopy_poll_cq(&mvp_MPIDI_CH3I_RDMA_Process.zcopy_info,
                                 ud_ctx, v, hca_index, &found);
        } while (ud_ctx->send_wqes_avail <= 0 || found);
    } else {
        ud_ctx = mvp_MPIDI_CH3I_RDMA_Process.ud_rails[v->rail];
    }
    if (ud_ctx->send_wqes_avail > 0) {
        v->retry_count++;
        if (v->retry_count > rdma_ud_max_retry_count) {
            PRINT_ERROR(
                "UD reliability error. Exceeded max retries(%d) "
                "in resending the message(%p). current retry timeout(us): %lu. "
                "This Error may happen on clusters based on the InfiniBand "
                "topology and traffic patterns. Please try with increased "
                "timeout using MVP_UD_RETRY_TIMEOUT\n",
                v->retry_count, v, rdma_ud_retry_timeout);
            exit(EXIT_FAILURE);
        }

        /* Set the ACK number */
        p->acknum = vc->mrail.seqnum_next_toack;
        /* Mark ACK as completed for VC */
        MARK_ACK_COMPLETED(vc);
        /* Mark VBUF as send in progress */
        v->flags |= UD_VBUF_SEND_INPROGRESS;
        /* Remove free_pending flag from VBUF */
        v->flags &= ~(UD_VBUF_FREE_PENIDING);
        /* Decrement number of available WQEs */
        ud_ctx->send_wqes_avail--;
        /* Keep track of the number of times we sent this out */
        v->pending_send_polls++;
        if (ibv_post_send(ud_ctx->qp, &(v->desc.u.sr), &(v->desc.y.bad_sr))) {
            ibv_error_abort(-1, "reliability resend failed");
        }
        PRINT_DEBUG(DEBUG_UD_verbose > 1, "Resend seqnum %u to rank %d\n",
                    v->seqnum, vc->pg_rank);
        /* Handle condition where the last close message keeps getting lost */
        if (unlikely(vc->state == MPIDI_VC_STATE_CLOSE_ACKED ||
                     vc->state == MPIDI_VC_STATE_CLOSED)) {
            mvp_unack_queue_clear(vc);
        }
        vc->mrail.rely.resend_count++;
        /* Indicate to caller that we were able to resend sucessfully */
        resend = 1;
    } else {
        if (!(v->in_ud_ext_sendq)) {
            mvp_ud_ext_sendq_queue(&ud_ctx->ext_send_queue, v);
            /* Indicate to caller that we were able to resend sucessfully */
            resend = 1;
        } else {
            /* Indicate to caller that we were NOT able to resend sucessfully */
            resend = 0;
        }
    }

    return resend;
}

void MRAILI_Process_recv(vbuf *v)
{
    MPIDI_CH3I_MRAILI_Pkt_comm_header *p = v->pheader;

    mvp_ud_process_ack(v->vc, p->acknum);
#ifdef _MVP_UD_DROP_PACKET_RATE_
    static int drop_packet_seq = 0;
    if (ud_drop_packet_rate && v->transport == IB_TRANSPORT_UD &&
        drop_packet_seq == ud_drop_packet_rate) {
        drop_packet_seq = 0;
        PRINT_DEBUG(DEBUG_UD_verbose > 1, "UD Packet dropped.seqnum:%d\n",
                    v->seqnum);
        MPIDI_CH3I_MRAIL_Release_vbuf(v);
        goto fn_exit;
    } else if (v->transport == IB_TRANSPORT_UD) {
        drop_packet_seq++;
    }
#endif

    if (IS_CNTL_MSG(p)) {
        PRINT_DEBUG(DEBUG_UD_verbose > 1, "recv cntl message ack:%d \n",
                    p->acknum);
        handle_read(v->vc, v);
        goto fn_exit;
    }

    MVP_UD_ACK_CREDIT_CHECK(((MPIDI_VC_t *)v->vc), v);
    mvp_ud_place_recvwin(v);

fn_exit:
    return;
}

void mvp_check_resend()
{
    mvp_ud_unackq_traverse(&mvp_MPIDI_CH3I_RDMA_Process.unack_queue);
}

#endif
