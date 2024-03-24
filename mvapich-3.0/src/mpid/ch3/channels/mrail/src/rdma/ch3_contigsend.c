/* -*- Mode: C; c-basic-offset:4 ; -*- */
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

#include "mpidi_ch3_impl.h"
#include "rdma_impl.h"

#undef FUNCNAME
#define FUNCNAME create_eagercontig_request_inline
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline MPIR_Request *create_eagercontig_request_inline(
    MPIDI_VC_t *vc, MPIDI_CH3_Pkt_type_t reqtype, const void *buf,
    intptr_t data_sz, int rank, int tag, MPIR_Comm *comm, int context_offset)
{
    MPIR_Request *sreq;
    MPIDI_CH3_Pkt_t upkt;
    MPIDI_CH3_Pkt_eager_send_t *const eager_pkt = &upkt.eager_send;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_CREATE_EAGERCONTIG_REQUEST);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_CREATE_EAGERCONTIG_REQUEST);
#if defined(MPID_USE_SEQUENCE_NUMBERS)
    MPID_Seqnum_t seqnum;
#endif /* defined(MPID_USE_SEQUENCE_NUMBERS) */

    MPIDI_Pkt_init(eager_pkt, reqtype);
    eager_pkt->match.parts.rank = comm->rank;
    eager_pkt->match.parts.tag = tag;
    eager_pkt->match.parts.context_id = comm->context_id + context_offset;
    eager_pkt->sender_req_id = MPI_REQUEST_NULL;
    eager_pkt->data_sz = data_sz;

    MPIDI_VC_FAI_send_seqnum(vc, seqnum);
    MPIDI_Pkt_set_seqnum(eager_pkt, seqnum);
    MPL_DBG_MSGPKT(vc, tag, eager_pkt->match.parts.context_id, rank, data_sz,
                   "EagerContig");
    sreq = MPIR_Request_create(MPIR_REQUEST_KIND__SEND);
    /* --BEGIN ERROR HANDLING-- */
    if (sreq == NULL)
        return NULL;
    /* --END ERROR HANDLING-- */
    MPIR_Object_set_ref(sreq, 2);
    MVP_INC_NUM_POSTED_SEND();

    sreq->dev.iov[0].iov_base = (void *)eager_pkt;
    sreq->dev.iov[0].iov_len = sizeof(*eager_pkt);
    MPL_DBG_MSG_FMT(MPIDI_CH3_DBG_OTHER, VERBOSE,
                    (MPL_DBG_FDEST,
                     "sending smp contiguous eager message, data_sz=" PRIdPTR,
                     data_sz));
    sreq->dev.iov[1].iov_base = (void *)buf;
    sreq->dev.iov[1].iov_len = data_sz;
#ifdef _ENABLE_CUDA_
    sreq->dev.pending_pkt = MPL_malloc(sreq->dev.iov[0].iov_len);
    MPIR_Memcpy(sreq->dev.pending_pkt, sreq->dev.iov[0].iov_base,
                sreq->dev.iov[0].iov_len);
    sreq->dev.iov[0].iov_base = (void *)sreq->dev.pending_pkt;
#else
    MPIR_Memcpy(&sreq->dev.pending_pkt, sreq->dev.iov[0].iov_base,
                sizeof(MPIDI_CH3_Pkt_t));
    sreq->dev.iov[0].iov_base = (void *)&sreq->dev.pending_pkt;
#endif
    sreq->ch.reqtype = REQUEST_NORMAL;
    sreq->dev.iov_offset = 0;
    sreq->dev.iov_count = 2;
    sreq->dev.OnDataAvail = 0;

    MPIDI_Request_set_seqnum(sreq, seqnum);
    MPIDI_Request_set_type(sreq, MPIDI_REQUEST_TYPE_SEND);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_CREATE_EAGERCONTIG_REQUEST);
    return sreq;
}

#undef FUNCNAME
#define FUNCNAME create_eagercontig_request
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
MPIR_Request *create_eagercontig_request(MPIDI_VC_t *vc,
                                         MPIDI_CH3_Pkt_type_t reqtype,
                                         const void *buf, intptr_t data_sz,
                                         int rank, int tag, MPIR_Comm *comm,
                                         int context_offset)
{
    return create_eagercontig_request_inline(vc, reqtype, buf, data_sz, rank,
                                             tag, comm, context_offset);
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH3_SMP_ContigSend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static int MPIDI_CH3_SMP_ContigSend(MPIDI_VC_t *vc, MPIR_Request **sreq_p,
                                    MPIDI_CH3_Pkt_type_t reqtype,
                                    const void *buf, intptr_t data_sz, int rank,
                                    int tag, MPIR_Comm *comm,
                                    int context_offset)
{
    MPIR_Request *sreq = NULL;
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_CH3_SMP_CONTIGSEND);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_CH3_SMP_CONTIGSEND);

    /* If send queue is empty attempt to send
       data, queuing any unsent data. */
    if (MPIDI_CH3I_SMP_SendQ_empty(vc)) {
        int nb;
        /* MT - need some signalling to lock down our right to use the
           channel, thus insuring that the progress engine does also try to
           write */
        MPIDI_CH3I_SMP_write_contig(vc, reqtype, buf, data_sz, rank, tag, comm,
                                    context_offset, &nb);
        PRINT_DEBUG(
            DEBUG_SHM_verbose > 1,
            "dst: %d, reqtype: %d, data_sz: %ld, writev returned %d bytes\n",
            vc->pg_rank, reqtype, data_sz, nb);

        /* send all or NULL */
        if (!nb) {
            /* no available shared memory buffer, enqueue request, fallback to
             * MPIDI_CH3_PKT_EAGER_SEND */
            sreq = create_eagercontig_request_inline(
                vc, MPIDI_CH3_PKT_EAGER_SEND, buf, data_sz, rank, tag, comm,
                context_offset);
            if (sreq == NULL) {
                MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER,
                                    "**ch3|contigsend");
            }
            MPIDI_CH3I_SMP_SendQ_enqueue_head(vc, sreq);
            vc->smp.send_active = sreq;
            PRINT_DEBUG(DEBUG_SHM_verbose > 1,
                        "Enqueueing sreq: %p to vc: %d\n", sreq, vc->pg_rank);
        }
    } else {
        /* sendQ not empty, enqueue request, fallback MPIDI_CH3_PKT_EAGER_SEND
         */
        sreq = create_eagercontig_request_inline(vc, MPIDI_CH3_PKT_EAGER_SEND,
                                                 buf, data_sz, rank, tag, comm,
                                                 context_offset);
        if (sreq == NULL) {
            MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER, "**ch3|contigsend");
        }
        MPIDI_CH3I_SMP_SendQ_enqueue(vc, sreq);
        PRINT_DEBUG(DEBUG_SHM_verbose > 1, "Enqueueing sreq: %p to vc: %d\n",
                    sreq, vc->pg_rank);
    }

    *sreq_p = sreq;

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_CH3_SMP_CONTIGSEND);
fn_fail:
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH3_ContigSend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPIDI_CH3_ContigSend(MPIR_Request **sreq_p, MPIDI_CH3_Pkt_type_t reqtype,
                         const void *buf, intptr_t data_sz, int rank, int tag,
                         MPIR_Comm *comm, int context_offset)
{
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_CH3_CONTIGSEND);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_CH3_CONTIGSEND);

    MPIDI_VC_t *vc;
    MPIDI_Comm_get_vc_set_active(comm, rank, &vc);

#if defined(CKPT)
    MPIDI_CH3I_CR_lock();
#endif

    if (SMP_INIT && vc->smp.local_nodes >= 0 &&
        vc->smp.local_nodes != g_smpi.my_local_id) {
        MPID_THREAD_CS_ENTER(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);
        if (MPIDI_CH3_SMP_ContigSend(vc, sreq_p, reqtype, buf, data_sz, rank,
                                     tag, comm, context_offset)) {
            MPID_THREAD_CS_EXIT(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);
#ifdef CKPT
            MPIDI_CH3I_CR_unlock();
#endif
            return 1;
        }
        MPID_THREAD_CS_EXIT(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);
#ifdef CKPT
        MPIDI_CH3I_CR_unlock();
#endif
        return 0;
    }

#ifdef CKPT
    MPIDI_CH3I_CR_unlock();
#endif
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_CH3_CONTIGSEND);
    return 1;
}
