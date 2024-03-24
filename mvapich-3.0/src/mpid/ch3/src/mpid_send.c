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

/* TODO: Replace or reimplement these bucket macros */
/* MPIR_T_PVAR_ULONG2_COUNTER_BUCKET_DECL_EXTERN(MVP,mvp_pt2pt_mpid_send); */

/*
 * MPID_Send()
 */
int MPID_Send(const void * buf, MPI_Aint count, MPI_Datatype datatype, int rank,
          int tag, MPIR_Comm * comm, int context_offset,
          MPIR_Request ** request)
{
    /* TODO: Replace or reimplement these bucket macros */
    /* MPIR_T_PVAR_COUNTER_BUCKET_INC(MVP,mvp_pt2pt_mpid_send,count,datatype); */

    intptr_t data_sz;
    int dt_contig;
    MPI_Aint dt_true_lb;
    MPIR_Datatype* dt_ptr;
    MPIR_Request * sreq = NULL;
    MPIDI_VC_t * vc;
#if defined(MPID_USE_SEQUENCE_NUMBERS)
    MPID_Seqnum_t seqnum;
#endif    
    intptr_t eager_threshold = -1;
    int mpi_errno = MPI_SUCCESS;    
#ifdef _ENABLE_CUDA_
    int device_transfer_mode = NONE;
#endif 

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPID_SEND);

    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPID_SEND);

    MPL_DBG_MSG_FMT(MPIDI_CH3_DBG_OTHER,VERBOSE,(MPL_DBG_FDEST,
                "rank=%d, tag=%d, context=%d", 
        rank, tag, comm->context_id + context_offset));

    /* Check to make sure the communicator hasn't already been revoked */
    if (comm->revoked &&
            MPIR_AGREE_TAG != MPIR_TAG_MASK_ERROR_BITS(tag & ~MPIR_TAG_COLL_BIT) &&
            MPIR_SHRINK_TAG != MPIR_TAG_MASK_ERROR_BITS(tag & ~MPIR_TAG_COLL_BIT)) {
        MPIR_ERR_SETANDJUMP(mpi_errno,MPIX_ERR_REVOKED,"**revoked");
    }

    if (rank == comm->rank && comm->comm_kind != MPIR_COMM_KIND__INTERCOMM)
    {
        mpi_errno = MPIDI_Isend_self(buf, count, datatype, rank, tag, comm, 
                         context_offset, MPIDI_REQUEST_TYPE_SEND, 
                         &sreq);

        /* In the single threaded case, sending to yourself will cause 
           deadlock.  Note that in the runtime-thread case, this check
           will not be made (long-term FIXME) */
#       ifndef MPICH_IS_THREADED
        {
            if (sreq != NULL && MPIR_cc_get(sreq->cc) != 0) {
                MPIR_ERR_SETANDJUMP(mpi_errno,MPI_ERR_OTHER,
                            "**dev|selfsenddeadlock");
            }
        }
#   endif
        if (mpi_errno != MPI_SUCCESS) { MPIR_ERR_POP(mpi_errno); }
        goto fn_exit;
    }

    MPIDI_Comm_get_vc_set_active(comm, rank, &vc);
    MPIR_ERR_CHKANDJUMP1(vc->state == MPIDI_VC_STATE_MORIBUND, mpi_errno, MPIX_ERR_PROC_FAILED, "**comm_fail", "**comm_fail %d", rank);

#ifdef ENABLE_COMM_OVERRIDES
    if (vc->comm_ops && vc->comm_ops->send)
    {
        mpi_errno = vc->comm_ops->send( vc, buf, count, datatype, rank, tag, comm, context_offset, &sreq);
        goto fn_exit;
    }
#endif

    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, 
                dt_true_lb);

#ifdef _ENABLE_CUDA_
    if (mvp_enable_device) {
        if (is_device_buffer((void *)buf)) {
            /* buf is in the GPU device memory */
            device_transfer_mode = DEVICE_TO_DEVICE;
        } else {
            /* buf is in the main memory */
            device_transfer_mode = NONE;
        }
    }
#endif

#ifdef USE_EAGER_SHORT
#if defined (CHANNEL_MRAIL)
    if ((data_sz + sizeof(MPIDI_CH3_Pkt_eager_send_t) <= vc->eager_fast_max_msg_sz) &&
        vc->use_eager_fast_fn && dt_contig
#else
    if (dt_contig && data_sz <= MPIDI_EAGER_SHORT_SIZE
#endif
#ifdef _ENABLE_CUDA_
        && device_transfer_mode == NONE
#endif
#ifdef CKPT
        && vc->ch.state == MPIDI_CH3I_VC_STATE_IDLE
#endif /* CKPT */
        ) {
        mpi_errno = MPIDI_CH3_EagerContigShortSend( &sreq, 
                           MPIDI_CH3_PKT_EAGERSHORT_SEND,
                           (char *)buf + dt_true_lb,
                           data_sz, rank, tag, comm, 
                           context_offset );
        goto fn_exit;
    }
#endif

    if (data_sz == 0)
    {
    MPIDI_CH3_Pkt_t upkt;
    MPIDI_CH3_Pkt_eager_send_t * const eager_pkt = &upkt.eager_send;

    MPL_DBG_MSG(MPIDI_CH3_DBG_OTHER,VERBOSE,"sending zero length message");
    MPIDI_Pkt_init(eager_pkt, MPIDI_CH3_PKT_EAGER_SEND);
    eager_pkt->match.parts.rank = comm->rank;
    eager_pkt->match.parts.tag = tag;
    eager_pkt->match.parts.context_id = comm->context_id + context_offset;
    eager_pkt->sender_req_id = MPI_REQUEST_NULL;
    eager_pkt->data_sz = 0;
    
    MPIDI_VC_FAI_send_seqnum(vc, seqnum);
    MPIDI_Pkt_set_seqnum(eager_pkt, seqnum);
    
    MPID_THREAD_CS_ENTER(POBJ, vc->pobj_mutex);
    mpi_errno = MPIDI_CH3_iStartMsg(vc, eager_pkt, sizeof(*eager_pkt), &sreq);
    MPID_THREAD_CS_EXIT(POBJ, vc->pobj_mutex);
    /* --BEGIN ERROR HANDLING-- */
    if (mpi_errno != MPI_SUCCESS)
    {
        MPIR_ERR_SETANDJUMP(mpi_errno,MPI_ERR_OTHER,"**ch3|eagermsg");
    }
    /* --END ERROR HANDLING-- */
    if (sreq != NULL)
    {
        MPIDI_Request_set_seqnum(sreq, seqnum);
        MPIDI_Request_set_type(sreq, MPIDI_REQUEST_TYPE_SEND);
        /* sreq->comm = comm;
          MPIR_Comm_add_ref(comm); -- not necessary for blocking functions */
    }
    
    goto fn_exit;
    }

#ifdef _ENABLE_CUDA_
    if (mvp_enable_device) {
        /*forces rndv for some IPC based CUDA transfers*/
#ifdef HAVE_CUDA_IPC
        if (mvp_device_use_ipc &&
            vc->smp.local_rank != -1 &&
            device_transfer_mode != NONE) {

            /*initialize IPC buffered channel if not initialized*/
            if (mvp_device_dynamic_init &&
                mvp_device_initialized &&
                vc->smp.can_access_peer == MVP_DEVICE_IPC_UNINITIALIZED) {
                device_ipc_init_dynamic (vc);
            }

            if (vc->smp.can_access_peer == MVP_DEVICE_IPC_ENABLED &&
                mvp_device_use_ipc_stage_buffer &&
                dt_contig &&
                data_sz >= mvp_device_ipc_threshold)  {
                /*force RNDV for CUDA transfers when buffered CUDA IPC is enabled or 
                 ** if mvp_device_use_smp_eager_ipc is set off */
                if (!mvp_device_use_smp_eager_ipc) {
                    goto rndv_send;
                }
            }
        }
#endif

        /*forces rndv for non IPC based CUDA transfers*/
        if (SMP_INIT && 
            vc->smp.local_rank != -1 &&
            device_transfer_mode != NONE) {
#ifdef HAVE_CUDA_IPC
            if (mvp_device_use_ipc == 0 ||
                vc->smp.can_access_peer != MVP_DEVICE_IPC_ENABLED)
#endif
            {
                goto rndv_send;
            }
        }
    } 
#endif   
 
    MPIDI_CH3_GET_EAGER_THRESHOLD(&eager_threshold, comm, vc);

    /* FIXME: flow control: limit number of outstanding eager messages
       containing data and need to be buffered by the receiver */
#if defined(CHANNEL_MRAIL)
    if (data_sz + sizeof(MPIDI_CH3_Pkt_eager_send_t) <=  vc->eager_max_msg_sz 
            && ! vc->force_rndv)
#else /* defined(CHANNEL_MRAIL) */
    if (data_sz + sizeof(MPIDI_CH3_Pkt_eager_send_t) <= eager_threshold)
#endif /* defined(CHANNEL_MRAIL) */
    {

    if (dt_contig)
        {
        mpi_errno = MPIDI_CH3_EagerContigSend( &sreq, 
                           MPIDI_CH3_PKT_EAGER_SEND,
                           (char *)buf + dt_true_lb,
                           data_sz, rank, tag, comm, 
                           context_offset );
    }
    else
        {
        MPIDI_Request_create_sreq(sreq, mpi_errno, goto fn_exit);
#ifdef _ENABLE_CUDA_
            if(HANDLE_GET_KIND(datatype) != HANDLE_KIND_BUILTIN &&
                        sreq->dev.datatype_ptr == NULL) {
                sreq->dev.datatype_ptr = dt_ptr;
                MPIR_Datatype_ptr_add_ref(dt_ptr);
            }
            if (mvp_enable_device) {
                /* buf is in the GPU device memory */
                sreq->mrail.device_transfer_mode = device_transfer_mode;
            }
#endif
        MPIDI_Request_set_type(sreq, MPIDI_REQUEST_TYPE_SEND);
        mpi_errno = MPIDI_CH3_EagerNoncontigSend( &sreq, 
                                                      MPIDI_CH3_PKT_EAGER_SEND,
                                                      buf, count, datatype,
                                                      rank, tag,
                                                      comm, context_offset );
    }
    }
    else 
    {
#if defined(_ENABLE_CUDA_) && defined(HAVE_CUDA_IPC)
rndv_send:
#endif

    MPIDI_Request_create_sreq(sreq, mpi_errno, goto fn_exit);
#ifdef _ENABLE_CUDA_
    sreq->mrail.cts_received = 0;
    if(HANDLE_GET_KIND(datatype) != HANDLE_KIND_BUILTIN &&
                    sreq->dev.datatype_ptr == NULL) {
        sreq->dev.datatype_ptr = dt_ptr;
        MPIR_Datatype_ptr_add_ref(dt_ptr);
    }
    if (mvp_enable_device) {
        sreq->mrail.device_transfer_mode = device_transfer_mode;
    }
#endif
    MPIDI_Request_set_type(sreq, MPIDI_REQUEST_TYPE_SEND);
    mpi_errno = vc->rndvSend_fn( &sreq, buf, count, datatype, dt_contig,
                                     data_sz, dt_true_lb, rank, tag, comm, 
                                     context_offset );
    /* Note that we don't increase the ref count on the datatype
       because this is a blocking call, and the calling routine 
       must wait until sreq completes */
    }

 fn_fail:
 fn_exit:
    *request = sreq;

    MPL_DBG_STMT(MPIDI_CH3_DBG_OTHER,VERBOSE,
    {
    if (mpi_errno == MPI_SUCCESS) {
        if (sreq) {
        MPL_DBG_MSG_P(MPIDI_CH3_DBG_OTHER,VERBOSE,
             "request allocated, handle=0x%08x", sreq->handle);
        }
        else
        {
        MPL_DBG_MSG(MPIDI_CH3_DBG_OTHER,VERBOSE,
                 "operation complete, no requests allocated");
        }
    }
    }
          );
    
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPID_SEND);
    return mpi_errno;
}

int MPID_Send_coll(const void *buf, MPI_Aint count, MPI_Datatype datatype, int rank, int tag,
                   MPIR_Comm * comm, int context_offset, MPIR_Request ** request,
                   MPIR_Errflag_t * errflag)
{
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPID_SEND_COLL);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPID_SEND_COLL);

    switch (*errflag) {
    case MPIR_ERR_NONE:
        break;
    case MPIR_ERR_PROC_FAILED:
        MPIR_TAG_SET_PROC_FAILURE_BIT(tag);
        break;
    default:
        MPIR_TAG_SET_ERROR_BIT(tag);
    }

    mpi_errno = MPID_Send(buf, count, datatype, rank, tag, comm, context_offset, request);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPID_SEND_COLL);

    return mpi_errno;
}
