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

extern unsigned long long PVAR_COUNTER_unexpected_recvs_rendezvous;
/* TODO: Replace or reimplement these bucket macros */
/* MPIR_T_PVAR_ULONG2_COUNTER_BUCKET_DECL_EXTERN(MVP,mvp_pt2pt_mpid_recv);  */

int MPID_Recv(void * buf, MPI_Aint count, MPI_Datatype datatype, int rank, int tag,
          MPIR_Comm * comm, int context_offset,
          MPI_Status * status, MPIR_Request ** request)
{
    /* TODO: Replace or reimplement these bucket macros */
    /* MPIR_T_PVAR_COUNTER_BUCKET_INC(MVP,mvp_pt2pt_mpid_recv,count,datatype); */

    /* FIXME: in the common case, we want to simply complete the message
       and make as few updates as possible.
       Note in addition that this routine is used only by MPI_Recv (a
       blocking routine; the intent of the interface (which returns 
       a request) was to simplify the handling of the case where the
       message was not found in the unexpected queue. */

    int mpi_errno = MPI_SUCCESS;
    MPIR_Request * rreq;
    int found;
#if defined(_ENABLE_CUDA_) && defined(HAVE_CUDA_IPC)
    int is_devbuf = 0;
#endif
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPID_RECV);

    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPID_RECV);

    MPL_DBG_MSG_FMT(MPIDI_CH3_DBG_OTHER,VERBOSE,(MPL_DBG_FDEST,
                      "rank=%d, tag=%d, context=%d", rank, tag,
              comm->recvcontext_id + context_offset));

    /* Check to make sure the communicator hasn't already been revoked */
    if (comm->revoked &&
            MPIR_AGREE_TAG != MPIR_TAG_MASK_ERROR_BITS(tag & ~MPIR_TAG_COLL_BIT) &&
            MPIR_SHRINK_TAG != MPIR_TAG_MASK_ERROR_BITS(tag & ~MPIR_TAG_COLL_BIT)) {
        MPIR_ERR_SETANDJUMP(mpi_errno,MPIX_ERR_REVOKED,"**revoked");
    }

    MPID_THREAD_CS_ENTER(POBJ, MPIR_THREAD_POBJ_MSGQ_MUTEX);
    rreq = MPIDI_CH3U_Recvq_FDU_or_AEP(rank, tag, 
                       comm->recvcontext_id + context_offset,
                                       comm, buf, count, datatype, &found);
    if (rreq == NULL) {
    MPID_THREAD_CS_EXIT(POBJ, MPIR_THREAD_POBJ_MSGQ_MUTEX);
    MPIR_ERR_SETANDJUMP(mpi_errno,MPI_ERR_OTHER,"**nomemreq");
    }
    MPIDI_Request_set_type(rreq, MPIDI_REQUEST_TYPE_RECV);

#ifdef _ENABLE_CUDA_
    if (mvp_enable_device) {
        if (is_device_buffer(buf)) {
            /* buf is in the GPU device memory */
            rreq->mrail.device_transfer_mode = DEVICE_TO_DEVICE;
        } else {
            /* buf is in the host memory*/
            rreq->mrail.device_transfer_mode = NONE;
        }
    }
#endif

    if (found)
    {
        MPIDI_VC_t * vc;

        /* Message was found in the unexepected queue */
        MPL_DBG_MSG(MPIDI_CH3_DBG_OTHER,VERBOSE,"request found in unexpected queue");

        /* Release the message queue - we've removed this request from 
           the queue already */
        MPID_THREAD_CS_EXIT(POBJ, MPIR_THREAD_POBJ_MSGQ_MUTEX);
        if (MPIDI_Request_get_msg_type(rreq) == MPIDI_REQUEST_EAGER_MSG)
        {
            int recv_pending;
            
            /* This is an eager message. */
            MPL_DBG_MSG(MPIDI_CH3_DBG_OTHER,VERBOSE,"eager message in the request");

            if (MPIDI_Request_get_sync_send_flag(rreq))
            {
            MPIDI_Comm_get_vc_set_active(comm, rreq->dev.match.parts.rank, &vc);
            MPIR_ERR_CHKANDJUMP1(vc->state == MPIDI_VC_STATE_MORIBUND, mpi_errno, MPIX_ERR_PROC_FAILED, "**comm_fail", "**comm_fail %d", rreq->dev.match.parts.rank);
            mpi_errno = MPIDI_CH3_EagerSyncAck( vc, rreq );
            MPIR_ERR_CHECK(mpi_errno);
            }
            
                /* the request was found in the unexpected queue, so it has a
                   recv_pending_count of at least 1, corresponding to this matching */
                MPIDI_Request_decr_pending(rreq);
                MPIDI_Request_check_pending(rreq, &recv_pending);

                if (MPIR_Request_is_complete(rreq)) {
                    /* is it ever possible to have (cc==0 && recv_pending>0) ? */
                    MPIR_Assert(!recv_pending);

                    /* All of the data has arrived, we need to unpack the data and 
                       then free the buffer and the request. */
                    if (rreq->dev.recv_data_sz > 0)
                    {
                        MPIDI_CH3U_Request_unpack_uebuf(rreq);
#if defined(_ENABLE_CUDA_) && defined(HAVE_CUDA_IPC)
                        is_devbuf = is_device_buffer((void *)rreq->dev.tmpbuf);
                        if (is_devbuf)
                        {
                            MVP_MPIDI_Free_Device(rreq->dev.tmpbuf);
                        }
                        else
#endif
                        {
                            MPL_free(rreq->dev.tmpbuf);
                        }
                    }

                    mpi_errno = rreq->status.MPI_ERROR;
                    if (status != MPI_STATUS_IGNORE)
                    {
                        *status = rreq->status;
                    }

                    MPIR_Request_free(rreq);
                    rreq = NULL;

                    goto fn_exit;
                }
            else
            {
                    /* there should never be outstanding completion events for an unexpected
                     * recv without also having a "pending recv" */
                    MPIR_Assert(recv_pending);

            /* The data is still being transferred across the net.  
               We'll leave it to the progress engine to handle once the
               entire message has arrived. */
            if (!HANDLE_IS_BUILTIN(datatype))
            {
                MPIR_Datatype_get_ptr(datatype, rreq->dev.datatype_ptr);
                MPIR_Datatype_ptr_add_ref(rreq->dev.datatype_ptr);
            }
            }
        }
        else if (MPIDI_Request_get_msg_type(rreq) == MPIDI_REQUEST_RNDV_MSG)
        {
            MPIR_T_PVAR_COUNTER_INC(MVP, unexpected_recvs_rendezvous, 1);
#if defined(CHANNEL_MRAIL)
            MPIDI_Comm_get_vc(comm, rreq->dev.match.parts.rank, &vc);
            mpi_errno = MPIDI_CH3_RecvRndv( vc, rreq );
#else
            MPIDI_Comm_get_vc_set_active(comm, rreq->dev.match.parts.rank, &vc);
            MPIR_ERR_CHKANDJUMP1(vc->state == MPIDI_VC_STATE_MORIBUND, mpi_errno, MPIX_ERR_PROC_FAILED, "**comm_fail", "**comm_fail %d", rreq->dev.match.parts.rank);
            mpi_errno = vc->rndvRecv_fn( vc, rreq );
#endif
            MPIR_ERR_CHECK(mpi_errno);
            if (!HANDLE_IS_BUILTIN(datatype))
            {
                MPIR_Datatype_get_ptr(datatype, rreq->dev.datatype_ptr);
                MPIR_Datatype_ptr_add_ref(rreq->dev.datatype_ptr);
            }
        }
        else if (MPIDI_Request_get_msg_type(rreq) == MPIDI_REQUEST_SELF_MSG)
        {
            mpi_errno = MPIDI_CH3_RecvFromSelf( rreq, buf, count, datatype );
            MPIR_ERR_CHECK(mpi_errno);
            if (status != MPI_STATUS_IGNORE)
            {
                *status = rreq->status;
            }
        }
        else
        {
            /* --BEGIN ERROR HANDLING-- */
#ifdef HAVE_ERROR_CHECKING
            int msg_type = MPIDI_Request_get_msg_type(rreq);
#endif
            MPIR_Request_free(rreq);
            rreq = NULL;
            MPIR_ERR_SETANDJUMP1(mpi_errno,MPI_ERR_INTERN, "**ch3|badmsgtype",
                                     "**ch3|badmsgtype %d", msg_type);
            /* --END ERROR HANDLING-- */
        }
    }
    else
    {
        /* Message has yet to arrived.  The request has been placed on the 
           list of posted receive requests and populated with
               information supplied in the arguments. */
        MPL_DBG_MSG(MPIDI_CH3_DBG_OTHER,VERBOSE,"request allocated in posted queue");

        /* FIXME: We do not need to add a datatype reference if
           the request is blocking.  This is currently added because
           of the actions that are taken when a request is freed. 
           (specifically, the datatype and comm both have their refs
           decremented, and are freed if the refs are zero) */
        if (!HANDLE_IS_BUILTIN(datatype))
        {
            MPIR_Datatype_get_ptr(datatype, rreq->dev.datatype_ptr);
            MPIR_Datatype_ptr_add_ref(rreq->dev.datatype_ptr);
        }

        rreq->dev.recv_pending_count = 1;
        /* We must wait until here to exit the msgqueue critical section
           on this request (we needed to set the recv_pending_count
           and the datatype pointer) */
            MPID_THREAD_CS_EXIT(POBJ, MPIR_THREAD_POBJ_MSGQ_MUTEX);
    }

  fn_exit:
    *request = rreq;
    MPL_DBG_STMT(MPIDI_CH3_DBG_OTHER,VERBOSE,
    if (rreq)
    {
        MPL_DBG_MSG_P(MPIDI_CH3_DBG_OTHER,VERBOSE,
                   "request allocated, handle=0x%08x", rreq->handle);
    }
    else
    {
        MPL_DBG_MSG(MPIDI_CH3_DBG_OTHER,VERBOSE,
                 "operation complete, no requests allocated");
    });

 fn_fail:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPID_RECV);
    return mpi_errno;
}
