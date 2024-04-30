/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  (C) 2001 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
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

#include "mpidi_ch3_impl.h"
#include "mpidi_ch3_rdma_pre.h"
#include "dreg.h"
#include "mpidrma.h"

#ifdef DEBUG
#define DEBUG_PRINT(args...)                                                   \
    do {                                                                       \
        int rank;                                                              \
        UPMI_GET_RANK(&rank);                                                  \
        fprintf(stderr, "[%d][%s:%d] ", rank, __FILE__, __LINE__);             \
        fprintf(stderr, args);                                                 \
        fflush(stderr);                                                        \
    } while (0)
#else
#define DEBUG_PRINT(args...)
#endif

int rts_send = 0;
int cts_recv = 0;

static inline void MPIDI_CH3_Prepare_rndv(MPIDI_VC_t *vc, MPIR_Request *sreq)
{
    MPIR_FUNC_TERSE_ENTER(MPID_STATE_MPIDI_CH3_PREPARE_RNDV);

#if defined(_ENABLE_CUDA_) && defined(HAVE_CUDA_IPC)
    if (mvp_enable_device && mvp_device_use_ipc) {
        if (mvp_device_use_ipc_stage_buffer &&
            sreq->dev.iov[0].iov_len < mvp_device_ipc_stage_buffer_limit) {
            if (MPIDI_CH3I_MRAIL_Prepare_rndv_device_ipc_buffered(vc, sreq)) {
                goto fn_exit;
            }
        } else {
            if (MPIDI_CH3I_MRAIL_Prepare_rndv_device_ipc(vc, sreq)) {
                goto fn_exit;
            }
        }
    }
    /* Use R3 for intra-node D-D transfer if CUDA IPC is not available */
    if (mvp_enable_device && IS_VC_SMP(vc) &&
        sreq->mrail.device_transfer_mode != NONE) {
        sreq->mrail.protocol = MRAILI_PROTOCOL_R3;
        goto fn_exit;
    }
#endif

    MPIDI_CH3I_MRAIL_Prepare_rndv(vc, sreq);

#if defined(_ENABLE_CUDA_) && defined(HAVE_CUDA_IPC)
fn_exit:
#endif
    MPIR_FUNC_TERSE_EXIT(MPID_STATE_MPIDI_CH3_PREPARE_RNDV);
}

int MPIDI_CH3_iStartRndvMsg(MPIDI_VC_t *vc, MPIR_Request *sreq,
                            MPIDI_CH3_Pkt_t *rts_pkt)
{
    MPIR_FUNC_TERSE_ENTER(MPID_STATE_MPIDI_CH3_ISTARTRNDVMSG);
    int mpi_errno = MPI_SUCCESS;
    DEBUG_PRINT("ch3_istartrndvmsg\n");
    MPL_DBG_MSG_S(MPIDI_CH3_DBG_CHANNEL, VERBOSE, "entering %s", __func__);
    /* If send queue is empty attempt to send
       data, queuing any unsent data. */
#ifdef CKPT
    MPIDI_CH3I_CR_lock();
#endif

    ++rts_send;
    if (MPIDI_CH3I_SendQ_empty(vc)) { /* MT */
        MPIR_Request *send_req;

        MPIDI_CH3_Pkt_rndv_req_to_send_t *rndv_pkt =
            &(rts_pkt->rndv_req_to_send);
        /* MT - need some signalling to lock down our right to use the
           channel, thus insuring that the progress engine does also try to
           write */
        MPIDI_CH3_Prepare_rndv(vc, sreq);
        MPIDI_CH3I_MRAIL_SET_PKT_RNDV(rndv_pkt, sreq);

        PRINT_DEBUG(DEBUG_RNDV_verbose > 1,
                    "Sending RTS to: %d, sreq: %p, protocol: %d, buf: %p, "
                    "rndv_buf_alloc: %d\n",
                    vc->pg_rank, sreq, sreq->mrail.protocol,
                    sreq->mrail.rndv_buf, sreq->mrail.rndv_buf_alloc);

        if ((mpi_errno = MPIDI_CH3_iStartMsg(
                 vc, rndv_pkt, sizeof(MPIDI_CH3_Pkt_rndv_req_to_send_t),
                 &send_req)) != MPI_SUCCESS) {
            MPIR_Object_set_ref(sreq, 0);
            MPIR_Request_free(sreq);
            sreq = NULL;
            MPIR_ERR_POP(mpi_errno);
        }

        if (send_req != NULL) {
            MPIR_Request_free(send_req);
        }
    } else {
        PRINT_DEBUG(DEBUG_RNDV_verbose > 1,
                    "Enqueuing RNDV msg to rank %d, sreq: %p\n", vc->pg_rank,
                    sreq);
        MPIDI_CH3I_SendQ_enqueue(vc, sreq);
    }

fn_exit:
#ifdef CKPT
    MPIDI_CH3I_CR_unlock();
#endif
    DEBUG_PRINT("[send rts]successful complete\n");
    MPL_DBG_MSG_S(MPIDI_CH3_DBG_CHANNEL, VERBOSE, "exiting %s", __func__);
    MPIR_FUNC_TERSE_EXIT(MPID_STATE_MPIDI_CH3_ISTARTRNDVMSG);
    return mpi_errno;

fn_fail:
    goto fn_exit;
}

int MPIDI_CH3_iStartRmaRndv(MPIDI_VC_t *vc, MPIR_Request *sreq, int control_cnt,
                            intptr_t stream_offset, intptr_t stream_size)
{
    MPIR_FUNC_TERSE_ENTER(MPID_STATE_MPIDI_CH3_ISTARTRMARNDV);
    int mpi_errno = MPI_SUCCESS;
    MPIDI_CH3_Pkt_put_rndv_t *put_rndv = (void *)sreq->dev.iov[0].iov_base;
    MPIDI_CH3_Pkt_accum_rndv_t *accum_rndv = (void *)sreq->dev.iov[0].iov_base;
    MPIDI_CH3_Pkt_get_accum_rndv_t *get_accum_rndv =
        (void *)sreq->dev.iov[0].iov_base;
    MPI_Aint dt_true_lb;
    MPIR_Request *rts_sreq;
    MPIR_Request *rreq;
    struct iovec *iov;

#ifdef CKPT
    MPIDI_CH3I_CR_lock();
#endif

    if ((iov = MPL_malloc(sizeof(struct iovec) * (control_cnt),
                          MPL_MEM_OBJECT)) == NULL) {
        MPIR_CHKMEM_SETERR(mpi_errno, sizeof(struct iovec) * control_cnt,
                           "MPID IOV");
    }

    DEBUG_PRINT("sreq before adjust iov0.len %d\n",
                sreq->dev.iov[control_cnt].iov_len);
    MPIR_Memcpy((void *)iov, (void *)sreq->dev.iov,
                sizeof(struct iovec) * control_cnt);

    /* we adjust iov because the rndv process assume the data starts from the
     * first vector of iov array */
    /* We can't use MPIR_Memcpy due to the overlapping check when using the
     * debug flags.*/
    memmove((void *)sreq->dev.iov, (void *)&sreq->dev.iov[control_cnt],
            sizeof(struct iovec) * (sreq->dev.iov_count - control_cnt));

    sreq->dev.iov_count -= control_cnt;

    /* MT - need some signalling to lock down our right to use the
       channel, thus insuring that the progress engine does also try to
       write */
    int origin_seq_size = sreq->dev.msgsize;
    if (stream_offset > 0)
        sreq->dev.msgsize = stream_size;

    MPIDI_CH3_Prepare_rndv(vc, sreq);
    sreq->dev.msgsize = origin_seq_size;

    MPIDI_CH3I_MRAIL_REVERT_RPUT(sreq);
    if (MPIDI_CH3_PKT_PUT_RNDV == put_rndv->type) {
        MPIDI_CH3I_MRAIL_SET_PKT_RNDV(put_rndv, sreq);
    } else if (MPIDI_CH3_PKT_GET_ACCUM_RNDV == get_accum_rndv->type) {
        MPIR_Request_get_ptr(get_accum_rndv->request_handle, rreq);
        MPIR_Datatype_get_true_lb(rreq->dev.datatype, &dt_true_lb);

        if (MPIR_DATATYPE_IS_PREDEFINED(rreq->dev.datatype)) {
            rreq->dev.OnDataAvail = 0;
            rreq->dev.iov[0].iov_base = (void *)((char *)rreq->dev.user_buf +
                                                 dt_true_lb + stream_offset);
            rreq->dev.iov[0].iov_len = stream_size;
            rreq->dev.iov_count = 1;
        } else {
            rreq->dev.iov_count = MPL_IOV_LIMIT;
            rreq->dev.msg_offset = stream_offset;
            rreq->dev.msgsize = stream_offset + stream_size;

            rreq->dev.OnFinal = 0;
            mpi_errno = MPIDI_CH3U_Request_load_send_iov(
                rreq, &rreq->dev.iov[0], &rreq->dev.iov_count);
        }
        MPIDI_CH3_Prepare_rndv(vc, rreq);
        MPIDI_CH3I_MRAIL_REVERT_RPUT(rreq);
#ifdef _ENABLE_UD_
        if (MVP_USE_UD_HYBRID &&
            rreq->mrail.protocol == MRAILI_PROTOCOL_UD_ZCOPY) {
            rreq->mrail.protocol = MRAILI_PROTOCOL_R3;
            MPIDI_CH3I_MRAIL_FREE_RNDV_BUFFER(rreq);
        }
#endif
        MPIDI_CH3I_MRAIL_SET_PKT_RNDV(get_accum_rndv, rreq);
    } else {
        MPIDI_CH3I_MRAIL_SET_PKT_RNDV(accum_rndv, sreq);
    }

    if (MPIDI_CH3_PKT_PUT_RNDV == put_rndv->type) {
        put_rndv->sender_req_id = sreq->handle;
    } else if (MPIDI_CH3_PKT_GET_ACCUM_RNDV == get_accum_rndv->type) {
        get_accum_rndv->sender_req_id = sreq->handle;
    } else {
        accum_rndv->sender_req_id = sreq->handle;
    }

    if ((mpi_errno = MPIDI_CH3_iStartMsgv(vc, iov, control_cnt, &rts_sreq)) !=
        MPI_SUCCESS) {
        MPIR_Object_set_ref(sreq, 0);
        MPIR_Request_free(sreq);
        sreq = NULL;
        MPIR_ERR_POP(mpi_errno);
    }

    if (rts_sreq != NULL) {
        MPIR_Request_free(rts_sreq);
    }
    MPL_free(iov);

    /* Wait until RNDV data has been sent to ensure the ordering between
     * multiple consecutive ACCUM/GET_ACCUM operations */
    if ((MPIDI_CH3_PKT_GET_ACCUM_RNDV == get_accum_rndv->type ||
         MPIDI_CH3_PKT_ACCUMULATE_RNDV == accum_rndv->type) &&
        sreq->mrail.is_rma_last_stream_unit == 1) {
        while (sreq->mrail.nearly_complete == 0)
            poke_progress_engine();
    }

fn_exit:
#if defined(CKPT)
    MPIDI_CH3I_CR_unlock();
#endif /* defined(CKPT) */
    DEBUG_PRINT("[send rts]successful complete\n");

    MPIR_FUNC_TERSE_EXIT(MPID_STATE_MPIDI_CH3_ISTARTRMARNDV);
    return mpi_errno;

fn_fail:
    goto fn_exit;
}

int MPIDI_CH3_iStartGetRndv(MPIDI_VC_t *vc, MPIDI_CH3_Pkt_get_rndv_t *get_rndv,
                            MPIR_Request *sreq, struct iovec *control_iov,
                            int num_control)
{
    MPIR_FUNC_TERSE_ENTER(MPID_STATE_MPIDI_CH3_ISTARTGETRNDV);
    int mpi_errno = MPI_SUCCESS;
    MPIR_Request *send_req;

#ifdef CKPT
    MPIDI_CH3I_CR_lock();
#endif

    struct iovec *iov =
        MPL_malloc(sizeof(struct iovec) * (num_control + 1), MPL_MEM_OBJECT);

    if (iov == NULL) {
        MPIR_CHKMEM_SETERR(mpi_errno, sizeof(struct iovec) * (num_control + 1),
                           "MPID IOV");
    }

    int n_iov = num_control + 1;
    iov[0].iov_base = (void *)get_rndv;
    iov[0].iov_len = sizeof(MPIDI_CH3_Pkt_get_rndv_t);
    MPIR_Memcpy((void *)&iov[1], (void *)control_iov,
                sizeof(struct iovec) * num_control);

    MPIDI_CH3_Prepare_rndv(vc, sreq);
    if (IS_VC_SMP(vc)) {
        sreq->mrail.protocol = MRAILI_PROTOCOL_R3;
    } else {
#ifdef _ENABLE_UD_
        if (vc->mrail.state & MRAILI_RC_CONNECTED)
#endif /*ifdef _ENABLE_UD_*/
        {
            sreq->mrail.protocol = MRAILI_PROTOCOL_RPUT;
        }
    }

#ifdef _ENABLE_UD_
    if (MVP_USE_UD_HYBRID && sreq->mrail.protocol == MRAILI_PROTOCOL_UD_ZCOPY) {
        sreq->mrail.protocol = MRAILI_PROTOCOL_R3;
        MPIDI_CH3I_MRAIL_FREE_RNDV_BUFFER(sreq);
    }
#endif

    MPIDI_CH3I_MRAIL_SET_PKT_RNDV(get_rndv, sreq);

    mpi_errno = MPIDI_CH3_iStartMsgv(vc, iov, n_iov, &send_req);
    if (NULL != send_req) {
        MPIR_Request_free(send_req);
    }
    MPL_free(iov);

#ifdef CKPT
    MPIDI_CH3I_CR_unlock();
#endif

fn_exit:
    MPIR_FUNC_TERSE_EXIT(MPID_STATE_MPIDI_CH3_ISTARTGETRNDV);
    return mpi_errno;
#ifndef CHANNEL_MRAIL
fn_fail:
#endif
    goto fn_exit;
}
