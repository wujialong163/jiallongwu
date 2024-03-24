/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda. For detailed
 * copyright and licensing information, please refer to the copyright file
 * COPYRIGHT in the top level MVAPICH directory.
 */
#define _MVP_INTERNAL_DISABLE_OVERRIDES_

#include "mvp_smp_impl.h"
#include "mvp_req_fields.h"
#include "mvp_req.h"
#include "mvp_tagm.h"
#include "legacy/mvp_recv_utils.h"

int MPIDI_MVP_RecvRndv(MPIDI_MVP_ep_t *vc, MPIR_Request *rreq);

int MPIDI_MVP_mpi_recv_self(MPIR_Request *rreq, void *buf, MPI_Aint count,
                            MPI_Datatype datatype);

MPL_STATIC_INLINE_PREFIX int MPIDI_MVP_smp_handle_matched_req(
    void *buf, MPI_Aint count, MPI_Datatype datatype, MPIR_Request *rreq,
    MPI_Status *status)
{
    MPIDI_MVP_ep_t *vc;
    MPIDI_av_entry_t *av;
    MPIR_Comm *comm = rreq->comm;
    MPIDI_MVP_smp_request_t *smp_rreq;
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_SMP_HANDLE_RECV_REQ);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_HANDLE_RECV_REQ);

    smp_rreq = MPIDI_MVP_REQUEST_FROM_MPICH(rreq);

    if (MPIDI_Request_get_msg_type(rreq) == MPIDI_REQUEST_EAGER_MSG) {
        int recv_pending;

        /* the request was found in the unexpected queue, so it has a
           recv_pending_count of at least 1, corresponding to this matching
         */
        MPIDI_Request_decr_pending(rreq);
        MPIDI_Request_check_pending(rreq, &recv_pending);

        if (MPIR_Request_is_complete(rreq)) {
            /* is it ever possible to have (cc==0 && recv_pending>0) ? */
            MPIR_Assert(!recv_pending);

            /* All of the data has arrived, we need to unpack the data and
               then free the buffer and the request. */
            if (MPIDI_MVP_REQUEST(rreq, dev.recv_data_sz) > 0) {
                MPIDI_MVP_Request_unpack_uebuf(rreq);
                MPL_free(smp_rreq->dev.tmpbuf);
            }
            mpi_errno = rreq->status.MPI_ERROR;
            if (status != MPI_STATUS_IGNORE) {
                *status = rreq->status;
            }

            goto fn_exit;
        } else {
            /* there should never be outstanding completion events for an
             * unexpected recv without also having a "pending recv" */
            MPIR_Assert(recv_pending);

            /* The data is still being transferred across the net.
               We'll leave it to the progress engine to handle once the
               entire message has arrived. */
            if (!HANDLE_IS_BUILTIN(datatype)) {
                MPIR_Datatype_get_ptr(
                    datatype, MPIDI_MVP_REQUEST(rreq, dev).datatype_ptr);
                MPIR_Datatype_ptr_add_ref(smp_rreq->dev.datatype_ptr);
            }
        }
    } else if (MPIDI_Request_get_msg_type(rreq) == MPIDI_REQUEST_RNDV_MSG) {
        //   MPIR_T_PVAR_COUNTER_INC(MVP, unexpected_recvs_rendezvous, 1);
        av = MPIDIU_comm_rank_to_av(
            comm, MPIDI_MVP_REQUEST(rreq, dev.match.parts.rank));
        vc = MPIDI_MVP_VC(av);
        //#if defined(CHANNEL_MRAIL)
        //       #<{(| TODO: remove this comment |)}>#
        //      #<{(| MPIDI_Comm_get_vc(comm, rreq->dev.match.parts.rank,
        //      &vc); |)}>#
        mpi_errno = MPIDI_MVP_RecvRndv(vc, rreq);
#if 0
//#else
       //       #<{(| TODO: remove this comment |)}>#
       //        #<{(| MPIDI_Comm_get_vc_set_active(comm, rreq->dev.match.parts.rank, &vc); |)}>#
       MPIDI_CHANGE_VC_STATE(vc, ACTIVE)
          MPIR_ERR_CHKANDJUMP1(vc->state == MPIDI_VC_STATE_MORIBUND,
                mpi_errno,
                MPIX_ERR_PROC_FAILED,
                "**comm_fail",
                "**comm_fail %d",
                rreq->dev.match.parts.rank);
       mpi_errno = vc->rndvRecv_fn(vc, _rreq);
#endif
        MPIR_ERR_CHECK(mpi_errno);
        if (!HANDLE_IS_BUILTIN(datatype)) {
            MPIR_Datatype_get_ptr(datatype,
                                  MPIDI_MVP_REQUEST(rreq, dev).datatype_ptr);
            MPIR_Datatype_ptr_add_ref(smp_rreq->dev.datatype_ptr);
        }
    } else if (MPIDI_Request_get_msg_type(rreq) == MPIDI_REQUEST_SELF_MSG) {
        mpi_errno = MPIDI_MVP_mpi_recv_self(rreq, buf, count, datatype);
        MPIR_ERR_CHECK(mpi_errno);
        if (status != MPI_STATUS_IGNORE) {
            *status = rreq->status;
        }
    } else {
        /* --BEGIN ERROR HANDLING-- */
        int msg_type = MPIDI_Request_get_msg_type(rreq);
        PRINT_DEBUG(DEBUG_RNDV_verbose > 2, "Msg_type: %d, req: %p (line %d)\n",
                    msg_type, rreq, __LINE__);
        MPIDI_MVP_Request_free(rreq);
        rreq = NULL;
        smp_rreq = NULL;
        MPIR_ERR_SETANDJUMP1(mpi_errno, MPI_ERR_INTERN, "**ch4|badmsgtype",
                             "**ch4|badmsgtype %d", msg_type);
        /* --END ERROR HANDLING-- */
    }

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_HANDLE_RECV_REQ);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIDI_MVP_smp_mpi_imrecv(void *buf, MPI_Aint count, MPI_Datatype datatype,
                             MPIR_Request *message)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_SMP_MPI_IMRECV);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_MPI_IMRECV);

    MPIDI_MVP_REQUEST(message, dev.user_buf) = buf;
    MPIDI_MVP_REQUEST(message, dev.user_count) = count;
    MPIDI_MVP_REQUEST(message, dev.datatype) = datatype;
    MPIDI_MVP_REQUEST(message, dev.msg_offset) = 0;

    MPIDI_MVP_smp_handle_matched_req(buf, count, datatype, message,
                                     MPI_STATUS_IGNORE);
    MPIR_Request_free(message);

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_MPI_IMRECV);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIDI_MVP_smp_mpi_recv(void *buf, MPI_Aint count, MPI_Datatype datatype,
                           int rank, int tag, MPIR_Comm *comm,
                           int context_offset, MPIDI_av_entry_t *addr,
                           MPI_Status *status, MPIR_Request **request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Request *rreq;
    int found, local_nodes = 0;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_SMP_MPI_RECV);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_MPI_RECV);

    if (comm->revoked &&
        MPIR_AGREE_TAG != MPIR_TAG_MASK_ERROR_BITS(tag & ~MPIR_TAG_COLL_BIT) &&
        MPIR_SHRINK_TAG != MPIR_TAG_MASK_ERROR_BITS(tag & ~MPIR_TAG_COLL_BIT)) {
        MPIR_ERR_SETANDJUMP(mpi_errno, MPIX_ERR_REVOKED, "**revoked");
    }

    MPID_THREAD_CS_ENTER(POBJ, MPIR_THREAD_POBJ_MSGQ_MUTEX);

    MPIDI_MVP_smp_recvq_post(rank, tag, comm->recvcontext_id + context_offset,
                             comm, buf, count, datatype, &found, local_nodes,
                             request);
    rreq = *request;

    if (rreq == NULL) {
        MPID_THREAD_CS_EXIT(POBJ, MPIR_THREAD_POBJ_MSGQ_MUTEX);
        MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER, "**nomemreq");
    }
    PRINT_DEBUG(DEBUG_SEND_verbose, "got mpir_req %p found %d \n", rreq, found);
    /* TODO: uncomment */
    /* MPIDI_Request_set_type(rreq, MPIDI_REQUEST_TYPE_RECV); */

    if (found) {
        /* Message was found in the unexepected queue */
        /* Release the message queue - we've removed this request from
           the queue already */
        MPID_THREAD_CS_EXIT(POBJ, MPIR_THREAD_POBJ_MSGQ_MUTEX);
        MPIDI_MVP_smp_handle_matched_req(buf, count, datatype, rreq, status);
    } else {
        /*
         * Message has yet to arrived. The request has been placed on the
         * list of posted receive requests and populated with
         * information supplied in the arguments.
         */
        MPIDI_MVP_smp_request_t *smp_rreq = MPIDI_MVP_REQUEST_FROM_MPICH(rreq);

        /* FIXME: We do not need to add a datatype reference if
           the request is blocking.  This is currently added because
           of the actions that are taken when a request is freed.
           (specifically, the datatype and comm both have their refs
           decremented, and are freed if the refs are zero) */
        // MPIDI_MVP_REQUEST(_rreq, dev).recv_pending_count = 1;
        if (!HANDLE_IS_BUILTIN(datatype)) {
            MPIR_Datatype_get_ptr(datatype, smp_rreq->dev.datatype_ptr);
            MPIR_Datatype_ptr_add_ref(smp_rreq->dev.datatype_ptr);
        }

        smp_rreq->dev.recv_pending_count = 1;
        /* We must wait until here to exit the msgqueue critical section
           on this request (we needed to set the recv_pending_count
           and the datatype pointer) */
        MPID_THREAD_CS_EXIT(POBJ, MPIR_THREAD_POBJ_MSGQ_MUTEX);
    }

fn_exit:
    *request = rreq;
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_MPI_RECV);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIDI_MVP_smp_mpi_cancel_recv(MPIR_Request *rreq)
{
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_SMP_MPI_CANCEL_RECV);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_MPI_CANCEL_RECV);

    if (!MPIR_Request_is_complete(rreq)) {
        mpi_errno = MPIDI_MVP_smp_recvq_delete_posted(rreq);
        MPIR_ERR_CHECK(mpi_errno);
        /* Complete the request to free it in the progress engine */
        MPIR_STATUS_SET_CANCEL_BIT(rreq->status, 1);
        MPIR_STATUS_SET_COUNT(rreq->status, 0);
        MPID_Request_complete(rreq);
    }
fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_MPI_CANCEL_RECV);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
