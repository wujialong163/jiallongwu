
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
#include "mvp_pkt.h"
#include "mvp_req_fields.h"
#include "mvp_req.h"
#include "mvp_req_utils.h"
#include "mvp_recv_utils.h"
#include "mvp_tagm.h"
#include "mvp_recv_utils.h"
#include "mvp_eager_handlers.h"
/* #include "mvp_pkt.h" */

extern int mvp_smp_init;

/* removing the VC here since it is the only thing we use for OnDataAvail */
int MPIDI_MVP_ReqHandler_UnpackUEBufComplete(MPIR_Request *rreq, int *complete)
{
    int recv_pending;
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(
        MPID_STATE_MPIDI_CH3_REQHANDLER_UNPACKUEBUFCOMPLETE);

    MPIR_FUNC_VERBOSE_ENTER(
        MPID_STATE_MPIDI_CH3_REQHANDLER_UNPACKUEBUFCOMPLETE);

    MPIDI_Request_decr_pending(rreq);
    MPIDI_Request_check_pending(rreq, &recv_pending);
    if (!recv_pending) {
        if (MPIDI_MVP_REQUEST(rreq, dev).recv_data_sz > 0) {
            //#if defined(CHANNEL_MRAIL)
            //            if (MPIDI_MVP_REQUEST(rreq,
            //            mrail).is_eager_vbuf_queued == 1) {
            //                MPIDI_MVP_REQUEST(rreq,
            //                mrail).eager_vbuf_tail->in_eager_sgl_queue = 2;
            //            }
            //#endif
            MPIDI_MVP_Request_unpack_uebuf(rreq);

            //#if defined(CHANNEL_MRAIL)
            //            if (MPIDI_MVP_REQUEST(rreq,
            //            mrail).is_eager_vbuf_queued == 0) {
            //#endif
            MPL_free(MPIDI_MVP_REQUEST(rreq, dev).tmpbuf);
            //#if defined(CHANNEL_MRAIL)
            //            } else {
            //                MPIDI_MVP_REQUEST(rreq,
            //                mrail).is_eager_vbuf_queued = 0;
            //            }
            //#endif
        }
    } else {
        /* The receive has not been posted yet.  MPID_{Recv/Irecv}()
         * is responsible for unpacking the buffer. */
    }

    /* mark data transfer as complete and decrement CC */
    MPID_Request_complete(rreq);
    MPIR_ERR_CHECK(mpi_errno);

    *complete = TRUE;

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_CH3_REQHANDLER_UNPACKUEBUFCOMPLETE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIDI_MVP_PktHandler_EagerSend(MPIDI_MVP_ep_t *vc, MPIDI_MVP_Pkt_t *pkt,
                                   void *data, intptr_t *buflen,
                                   MPIR_Request **rreqp)
{
    MPIDI_MVP_Pkt_eager_send_t *eager_pkt = &pkt->eager_send;
    MPIR_Request *rreq;
    int found;
    int complete;
    char *data_buf;
    intptr_t data_len;
    int mpi_errno = MPI_SUCCESS;

    MPID_THREAD_CS_ENTER(POBJ, MPIR_THREAD_POBJ_MSGQ_MUTEX);

    PRINT_DEBUG(
        DEBUG_SEND_verbose,
        "received eager send pkt, sreq=0x%08x, rank=%d, tag=%d, context=%d",
        eager_pkt->sender_req_id, eager_pkt->match.parts.rank,
        eager_pkt->match.parts.tag, eager_pkt->match.parts.context_id);

    MPIDI_MVP_smp_recvq_recv(eager_pkt->match, &found, rreqp);
    rreq = *rreqp;
    if (!found && mvp_smp_init && vc->smp.local_nodes >= 0) {
        MVP_INC_NUM_POSTED_RECV();
    }
    MPIR_ERR_CHKANDJUMP1(!rreq, mpi_errno, MPI_ERR_OTHER, "**nomemreq",
                         "**nomemuereq %d", MPIDI_MVP_Recvq_count_unexp());

    PRINT_DEBUG(DEBUG_SEND_verbose, "rreq %p found %d \n", rreq, found);

    /* If the completion counter is 0, that means that the communicator to
     * which this message is being sent has been revoked and we shouldn't
     * bother finishing this. */
    if (unlikely(!found && MPIR_cc_get(rreq->cc) == 0)) {
        *rreqp = NULL;
        goto fn_fail;
    }

    set_request_info(rreq, eager_pkt, MPIDI_REQUEST_EAGER_MSG);

    data_len = ((*buflen - MPIDI_MVP_PKT_SIZE(pkt) >=
                 MPIDI_MVP_REQUEST(rreq, dev).recv_data_sz) ?
                    MPIDI_MVP_REQUEST(rreq, dev).recv_data_sz :
                    *buflen - MPIDI_MVP_PKT_SIZE(pkt));

    data_buf = (char *)pkt + MPIDI_MVP_PKT_SIZE(pkt);

    if (MPIDI_MVP_REQUEST(rreq, dev).recv_data_sz == 0) {
        /* return the number of bytes processed in this function */
        *buflen = MPIDI_MVP_PKT_SIZE(pkt);
        MPID_Request_complete(rreq);
        *rreqp = NULL;
    } else {
        if (found) {
            mpi_errno = MPIDI_MVP_Receive_data_found(rreq, data_buf, &data_len,
                                                     &complete);
        } else {
            /* TODO: determine if we want to keep this. is_eager_vbuf_queued
             * does seem extensively used throughout ch3 but not ch4 as of
             * writing this.
             * We need to figure out if the below is still useful. */
            /* if (!IS_VC_SMP(vc) && mvp_use_opt_eager_recv == 1) { */
            /*      rreq->mrail.is_eager_vbuf_queued = 1;  */
            /* } */
            mpi_errno = MPIDI_MVP_Receive_data_unexpected(rreq, data_buf,
                                                          &data_len, &complete);
        }
        if (mpi_errno != MPI_SUCCESS) {
            MPIR_ERR_SETANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**ch4|postrecv",
                                 "**ch4|postrecv %s",
                                 "MPIDI_CH3_PKT_EAGER_SEND");
        }

        /* return the number of bytes processed in this function */
        *buflen = data_len + MPIDI_MVP_PKT_SIZE(pkt);

        if (complete) {
            MPID_Request_complete(rreq);
            *rreqp = NULL;
        } else {
            *rreqp = rreq;
        }
    }

fn_fail:
    MPID_THREAD_CS_EXIT(POBJ, MPIR_THREAD_POBJ_MSGQ_MUTEX);
    return mpi_errno;
}
