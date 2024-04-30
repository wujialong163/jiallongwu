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

#include "mvp_smp.h"
#include "mvp_smp_impl.h"
#include "mvp_smp_rndv.h"
#include "mvp_smp_params.h"
#include "mvp_smp_progress_utils.h"
/* for incrementing recv counters */
#include "mvp_tagm.h"

/* TODO: move/refactor these functions in here */
int MPIDI_MVP_Receive_data_found(MPIR_Request *rreq, void *buf,
                                 intptr_t *buflen, int *complete);
int MPIDI_MVP_Receive_data_unexpected(MPIR_Request *rreq, void *buf,
                                      intptr_t *buflen, int *complete);

MPL_STATIC_INLINE_PREFIX int MPIDI_MVP_smp_read_eager_header(
    MPIDI_MVP_Pkt_t *pkt, void *data, intptr_t *buflen, MPIR_Request **rreq_ptr)
{                                                                         
    int mpi_errno = MPI_SUCCESS;
    int found = 0;
    int complete = 0;
    intptr_t data_len;
    char *data_buf = NULL;
    MPIR_Request *rreq = NULL;
    MPIDI_MVP_Pkt_eager_send_t *eager_pkt = &pkt->eager_send;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_SMP_READ_EAGER_HEADER);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_READ_EAGER_HEADER);

    MPID_THREAD_CS_ENTER(POBJ, MPIR_THREAD_POBJ_MSGQ_MUTEX);

    MPIDI_MVP_smp_recvq_recv(eager_pkt->match, &found, rreq_ptr);
    rreq = *rreq_ptr;
    /* TODO: this should be handled by the recvq_recv function */
    if (!found) {
        MVP_INC_NUM_UNEXP_RECV();
    }
    MPIR_ERR_CHKANDJUMP1(!rreq, mpi_errno, MPI_ERR_OTHER, "**nomemreq",
                         "**nomemuereq %d", MPIDI_MVP_Recvq_count_unexp());
    /* completion counter set to 0 indicates the communicator was revoked */
    if (unlikely(!found && !MPIR_cc_get(rreq->cc))) {
        MPIDI_MVP_Request_free(rreq);
        *rreq_ptr = NULL;
        goto fn_exit;
    }
    MPIDI_MVP_SET_REQUEST_INFO(rreq, eager_pkt, MPIDI_REQUEST_EAGER_MSG);

    data_len = *buflen - MPIDI_MVP_PKT_SIZE(pkt) >=
        MPIDI_MVP_REQUEST(rreq, dev).recv_data_sz ?
        MPIDI_MVP_REQUEST(rreq, dev).recv_data_sz :
        *buflen - MPIDI_MVP_PKT_SIZE(pkt);
    data_buf = (char *)pkt + MPIDI_MVP_PKT_SIZE(pkt);

    if (!MPIDI_MVP_REQUEST(rreq, dev.recv_data_sz)) {
        /* return number of bytes processed in this function */
        *buflen = MPIDI_MVP_PKT_SIZE(pkt);
        /* TODO: this may break things */
        MPID_Request_complete(rreq);
        *rreq_ptr = NULL;
    } else {
        if (found) {
            mpi_errno = MPIDI_MVP_Receive_data_found(rreq, data_buf, &data_len,
                                                     &complete);
        } else {
            mpi_errno = MPIDI_MVP_Receive_data_unexpected(rreq, data_buf,
                                                          &data_len, &complete);
        }
        if (mpi_errno != MPI_SUCCESS) {
            MPIR_ERR_SETANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**ch4|postrecv",
                "**ch4|postrecv %s", "MPIDI_CH3_PKT_EAGER_SEND");
        }
        *buflen = data_len + MPIDI_MVP_PKT_SIZE(pkt);
        if (complete) {
            MPID_Request_complete(rreq);
            *rreq_ptr = NULL;
        } else {
            *rreq_ptr = rreq;
        }
    }
fn_exit:
    MPID_THREAD_CS_EXIT(POBJ, MPIR_THREAD_POBJ_MSGQ_MUTEX);
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_READ_EAGER_HEADER);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIDI_MVP_smp_process_header(MPIDI_MVP_ep_t *vc, int sender_local_rank,
                                 MPIDI_MVP_Pkt_t *pkt, int *index,
                                 void *dma_header, mvp_smp_dma_flag_t *dma_flag,
                                 SMP_pkt_type_t *recv_pkt_type,
                                 MPIR_Request **rreq_ptr)
{
    int mpi_errno = MPI_SUCCESS; 
    intptr_t buflen = MPIDI_MVP_PKT_SIZE(pkt); 
    MPIR_Request *rreq = NULL;
    MPIDI_MVP_Pkt_rndv_r3_data_t *pkt_header = NULL;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_SMP_PROCESS_HEADER);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_PROCESS_HEADER);

    /* this is safe, we are going to use it for indexing in arrays */
    MPIR_Assert(sender_local_rank >= 0);

    switch (pkt->type) {
        case MPIDI_MVP_PKT_EAGER_SEND_CONTIG:
        case MPIDI_MVP_PKT_EAGER_SEND:
            mpi_errno = MPIDI_MVP_smp_read_eager_header(pkt,
                (char *)pkt + MPIDI_MVP_PKT_SIZE(pkt), &buflen, &rreq);
            MPIR_ERR_CHECK(mpi_errno);

            if (!rreq) {
                s_current_ptr[sender_local_rank] = NULL;
                s_current_bytes[sender_local_rank] = 0;
                /* TODO: refactor/rename this */
                smpi_complete_recv(sender_local_rank, MPIR_Process.local_rank,
                                  s_total_bytes[sender_local_rank]);
                s_total_bytes[sender_local_rank] = 0;
            }
            *recv_pkt_type = SMP_EAGER_MSG;
            break;
        case MPIDI_MVP_PKT_RNDV_R3_DATA:
            MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**fail",
                                      "**fail %s", "R3 not yet supported");
            pkt_header = (MPIDI_MVP_Pkt_rndv_r3_data_t *)pkt;
            if ((*index = pkt_header->src.smp_index) == -1) {
                MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**fail",
                                          "**fail %s", "Invalid index");
            }
            *recv_pkt_type = SMP_RNDV_MSG;
            MPIR_Request_get_ptr(pkt_header->receiver_req_id, rreq);
            break;
        case MPIDI_MVP_PKT_RPUT_FINISH:
        case MPIDI_MVP_PKT_RGET_FINISH:
            mpi_errno = MPIDI_MVP_smp_rndv_dma_finish(
                (MPIDI_MVP_Pkt_dma_finish_t *)pkt, &rreq);
            break;
        case MPIDI_MVP_PKT_RNDV_CLR_TO_SEND:
            /* TODO: decouple from VC before uncomenting */
            mpi_errno = MPIDI_MVP_PktHandler_SMP_CTS(vc, pkt, &buflen, &rreq);
            /* New version */
            /* mpi_errno = MPIDI_MVP_smp_rndv_cts(pkt, &buflen, &rreq); */
            break;
        case MPIDI_MVP_PKT_RNDV_REQ_TO_SEND:
            /* TODO: decouple from VC before uncomenting */
            mpi_errno = MPIDI_MVP_PktHandler_SMP_RTS(vc, pkt, &buflen, &rreq);
            break;
    }
fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_PROCESS_HEADER);
    *rreq_ptr = rreq;
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
