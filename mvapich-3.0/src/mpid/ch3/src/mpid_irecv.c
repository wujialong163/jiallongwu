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
/* MPIR_T_PVAR_ULONG2_COUNTER_BUCKET_DECL_EXTERN(MVP,mvp_pt2pt_mpid_irecv); */

int MPID_Irecv(void * buf, MPI_Aint count, MPI_Datatype datatype, int rank, int tag,
           MPIR_Comm * comm, int context_offset,
               MPIR_Request ** request)
{
    MPIR_Request * rreq = NULL;
    int found;
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPID_IRECV);

    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPID_IRECV);

    MPL_DBG_MSG_FMT(MPIDI_CH3_DBG_OTHER,VERBOSE,(MPL_DBG_FDEST,
            "rank=%d, tag=%d, context=%d", 
            rank, tag, comm->recvcontext_id + context_offset));

    /* Check to make sure the communicator hasn't already been revoked */
    if (comm->revoked &&
            MPIR_AGREE_TAG != MPIR_TAG_MASK_ERROR_BITS(tag & ~MPIR_TAG_COLL_BIT) &&
            MPIR_SHRINK_TAG != MPIR_TAG_MASK_ERROR_BITS(tag & ~MPIR_TAG_COLL_BIT)) {
        MPL_DBG_MSG(MPIDI_CH3_DBG_OTHER,VERBOSE,"Comm has been revoked. Returning from MPID_IRECV.");
        MPIR_ERR_SETANDJUMP(mpi_errno,MPIX_ERR_REVOKED,"**revoked");
    }

    MPID_THREAD_CS_ENTER(POBJ, MPIR_THREAD_POBJ_MSGQ_MUTEX);
    rreq = MPIDI_CH3U_Recvq_FDU_or_AEP(rank, tag, 
                       comm->recvcontext_id + context_offset,
                                       comm, buf, count, datatype, &found);
    if (rreq == NULL)
    {
    MPID_THREAD_CS_EXIT(POBJ, MPIR_THREAD_POBJ_MSGQ_MUTEX);
    MPIR_ERR_SETANDJUMP(mpi_errno,MPI_ERR_OTHER,"**nomemreq");
    }
    MPIDI_Request_set_type(rreq, MPIDI_REQUEST_TYPE_IRECV);

#ifdef _ENABLE_CUDA_
    int device_buf = 0;
    if (mvp_enable_device) {
        device_buf = is_device_buffer(buf);
    }
#endif
    if (found)
    {
        MPIDI_VC_t * vc;

#ifdef _ENABLE_CUDA_
        if (mvp_enable_device) {
            if (device_buf) {
                /* buf is in the GPU device memory */
                rreq->mrail.device_transfer_mode = DEVICE_TO_DEVICE;
            } else {
                /* buf is in the host memory*/
                rreq->mrail.device_transfer_mode = NONE;
            }
        }
#endif
        
        /* Message was found in the unexepected queue */
        MPL_DBG_MSG(MPIDI_CH3_DBG_OTHER,VERBOSE,"request found in unexpected queue");

        /* Release the message queue - we've removed this request from 
           the queue already */
        MPID_THREAD_CS_EXIT(POBJ, MPIR_THREAD_POBJ_MSGQ_MUTEX);

        if (MPIDI_Request_get_msg_type(rreq) == MPIDI_REQUEST_EAGER_MSG)
        {
            int recv_pending;
            
            /* This is an eager message */
            MPL_DBG_MSG(MPIDI_CH3_DBG_OTHER,VERBOSE,"eager message in the request");
            
            /* If this is a eager synchronous message, then we need to send an 
               acknowledgement back to the sender. */
            if (MPIDI_Request_get_sync_send_flag(rreq))
            {
                MPIDI_Comm_get_vc_set_active(comm, rreq->dev.match.parts.rank, &vc);
                mpi_errno = MPIDI_CH3_EagerSyncAck( vc, rreq );
                MPIR_ERR_CHECK(mpi_errno);
            }

            /* the request was found in the unexpected queue, so it has a
               recv_pending_count of at least 1 */
            MPIDI_Request_decr_pending(rreq);
            MPIDI_Request_check_pending(rreq, &recv_pending);

            if (MPIR_Request_is_complete(rreq)) {
                /* is it ever possible to have (cc==0 && recv_pending>0) ? */
                MPIR_Assert(!recv_pending);

                /* All of the data has arrived, we need to copy the data and 
                   then free the buffer. */
                if (rreq->dev.recv_data_sz > 0)
                {
                    MPIDI_CH3U_Request_unpack_uebuf(rreq);
#if defined(_ENABLE_CUDA_) && defined(HAVE_CUDA_IPC)
                    if (mvp_enable_device && mvp_device_use_smp_eager_ipc
                            && is_device_buffer(rreq->dev.tmpbuf)) {
                       MPIL_Free_Device(rreq->dev.tmpbuf);
                    }
                    else
#endif
                    {
                        MPL_free(rreq->dev.tmpbuf);
                    }

                }

                mpi_errno = rreq->status.MPI_ERROR;
                goto fn_exit;
            }
            else
            {
                /* there should never be outstanding completion events for an unexpected
                 * recv without also having a "pending recv" */
                MPIR_Assert(recv_pending);
                /* The data is still being transfered across the net.  We'll 
                   leave it to the progress engine to handle once the
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
             MPIDI_Comm_get_vc_set_active(comm, rreq->dev.match.parts.rank, &vc);

#ifdef _ENABLE_CUDA_
            if (mvp_enable_device) {
                if (device_buf) {
                    /* buf is in the GPU device memory */
                    rreq->mrail.device_transfer_mode = DEVICE_TO_DEVICE;
                } else {
                    /* buf is in the host memory*/
                    rreq->mrail.device_transfer_mode = NONE;
                }
            }
#endif


#if defined(CHANNEL_MRAIL)
            mpi_errno = MPIDI_CH3_RecvRndv( vc, rreq );
#else
            mpi_errno = vc->rndvRecv_fn( vc, rreq );
#endif
            if (mpi_errno) MPIR_ERR_POP( mpi_errno );
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
    
    if (!HANDLE_IS_BUILTIN(datatype))
    {
        MPIR_Datatype_get_ptr(datatype, rreq->dev.datatype_ptr);
        MPIR_Datatype_ptr_add_ref(rreq->dev.datatype_ptr);
    }

#ifdef _ENABLE_CUDA_
    if(mvp_enable_device) {
        if (device_buf) {
            /* buf is in the GPU device memory */
            rreq->mrail.device_transfer_mode = DEVICE_TO_DEVICE;
        } else {
            /* buf is in the host memory*/
            rreq->mrail.device_transfer_mode = NONE;
        }
    }
#endif

        rreq->dev.recv_pending_count = 1;

        /* We must wait until here to exit the msgqueue critical section
           on this request (we needed to set the recv_pending_count
           and the datatype pointer) */
        MPID_THREAD_CS_EXIT(POBJ, MPIR_THREAD_POBJ_MSGQ_MUTEX);
    }

  fn_exit:
    *request = rreq;
    MPL_DBG_MSG_P(MPIDI_CH3_DBG_OTHER,VERBOSE,"request allocated, handle=0x%08x",
           rreq->handle);
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPID_IRECV);
    return mpi_errno;

 fn_fail:
    MPL_DBG_MSG_D(MPIDI_CH3_DBG_OTHER,VERBOSE,"IRECV errno: 0x%08x", mpi_errno);
    MPL_DBG_MSG_D(MPIDI_CH3_DBG_OTHER,VERBOSE,"(class: %d)", MPIR_ERR_GET_CLASS(mpi_errno));
    goto fn_exit;
}
