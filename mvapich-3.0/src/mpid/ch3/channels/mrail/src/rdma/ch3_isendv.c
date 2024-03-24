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
#include "rdma_impl.h"
#include "ibv_send_inline.h"

static inline int update_request(MPIR_Request *sreq, struct iovec *iov,
                                 int count, int offset, int nb)
{
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_UPDATE_REQUEST);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_UPDATE_REQUEST);
    int mpi_errno = MPI_SUCCESS;

    MPIR_Memcpy(sreq->dev.iov, iov, count * sizeof(struct iovec));

    if (offset == 0) {
#if defined(MPICH_DBG_OUTPUT)
        if (iov[0].iov_len != sizeof(MPIDI_CH3_Pkt_t)) {
            MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER, "**arg");
        }
#endif /* defined(MPICH_DBG_OUTPUT) */
#ifdef _ENABLE_CUDA_
        sreq->dev.pending_pkt = MPL_malloc(iov[0].iov_len, MPL_MEM_OTHER);
        MPIR_Memcpy(sreq->dev.pending_pkt, iov[0].iov_base, iov[0].iov_len);
        sreq->dev.iov[0].iov_base = (void *)sreq->dev.pending_pkt;
#else
        MPIR_Memcpy(&sreq->dev.pending_pkt, iov[0].iov_base,
                    sizeof(MPIDI_CH3_Pkt_t));
        sreq->dev.iov[0].iov_base = (void *)&sreq->dev.pending_pkt;
#endif
    }

    sreq->dev.iov[offset].iov_base =
        (char *)sreq->dev.iov[offset].iov_base + nb;
    sreq->dev.iov[offset].iov_len -= nb;
    sreq->dev.iov_count = count;
    sreq->dev.iov_offset = offset;

    MVP_INC_NUM_POSTED_SEND();

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_UPDATE_REQUEST);
    return mpi_errno;

#ifndef CHANNEL_MRAIL
fn_fail:
#endif
    goto fn_exit;
}

#undef DEBUG_PRINT
#if defined(DEBUG)
#define DEBUG_PRINT(args...)                                                   \
    do {                                                                       \
        int rank;                                                              \
        UPMI_GET_RANK(&rank);                                                  \
        fprintf(stderr, "[%d][%s:%d] ", rank, __FILE__, __LINE__);             \
        fprintf(stderr, args);                                                 \
    } while (0)
#else /* defined(DEBUG) */
#define DEBUG_PRINT(args...)
#endif /* defined(DEBUG) */

static inline int MPIDI_CH3_SMP_iSendv(MPIDI_VC_t *vc, MPIR_Request *sreq,
                                       struct iovec *iov, int n_iov)
{
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_CH3_SMP_ISENDV);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_CH3_SMP_ISENDV);
    int mpi_errno = MPI_SUCCESS;

    PRINT_DEBUG(
        DEBUG_SHM_verbose > 1,
        "dst: %d, sreq: %p, niov: %d, iov[0].buf: %p, iov[0].len: %lu\n",
        vc->pg_rank, sreq, n_iov, iov[0].iov_base, iov[0].iov_len);

    /* Connection already formed.  If send queue is empty attempt to send data,
     * queuing any unsent data. */
    if (MPIDI_CH3I_SMP_SendQ_empty(vc)) { /* MT */
        int nb;

        DEBUG_PRINT("Send queue empty, attempting to write\n");

        /* MT - need some signalling to lock down our right to use the channel,
         thus
         * insuring that the progress engine does
         also try to write */
        MPIDI_CH3I_SMP_writev(vc, iov, n_iov, &nb);
        int offset = 0;

        DEBUG_PRINT("wrote %d bytes\n", nb);

        while (offset < n_iov) {
            if ((int)iov[offset].iov_len <= nb) {
                nb -= iov[offset].iov_len;
                ++offset;
            } else {
                DEBUG_PRINT("partial write, enqueuing at head\n");

                if ((mpi_errno = update_request(sreq, iov, n_iov, offset,
                                                nb)) != MPI_SUCCESS) {
                    MPIR_ERR_POP(mpi_errno);
                }

                MVP_INC_NUM_POSTED_SEND();
                MPIR_Assert(vc->smp.send_active == NULL);
                MPIDI_CH3I_SMP_SendQ_enqueue_head(vc, sreq);
                vc->smp.send_active = sreq;
                break;
            }
        }

        if (offset == n_iov) {
            DEBUG_PRINT(
                "write complete, calling MPIDI_CH3U_Handle_send_req()\n");
            int complete;
            MPIDI_CH3U_Handle_send_req(vc, sreq, &complete);

            if (!complete) {
                /* NOTE: dev.iov_count is used to detect completion instead of
                 * cc because the transfer may be complete, but request may
                 * still be active (see MPI_Ssend())
                 */
                MVP_INC_NUM_POSTED_SEND();
                MPIR_Assert(vc->smp.send_active == NULL);
                MPIDI_CH3I_SMP_SendQ_enqueue_head(vc, sreq);
                vc->smp.send_active = sreq;
            } else {
                vc->smp.send_active = MPIDI_CH3I_SMP_SendQ_head(vc);
            }
        }
    } else {
        MPL_DBG_MSG(MPIDI_CH3_DBG_CHANNEL, VERBOSE,
                    "send queue not empty, enqueuing");

        if ((mpi_errno = update_request(sreq, iov, n_iov, 0, 0)) !=
            MPI_SUCCESS) {
            MPIR_ERR_POP(mpi_errno);
        }

        MPIDI_CH3I_SMP_SendQ_enqueue(vc, sreq);
    }

fn_exit:
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_CH3_SMP_ISENDV);
    return mpi_errno;

fn_fail:
    goto fn_exit;
}

int MPIDI_CH3_iSendv(MPIDI_VC_t *vc, MPIR_Request *sreq, struct iovec *iov,
                     int n_iov)
{
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_CH3_ISENDV);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_CH3_ISENDV);
    int mpi_errno = MPI_SUCCESS;
    void *databuf = NULL;

    MPL_DBG_MSG_S(MPIDI_CH3_DBG_CHANNEL, VERBOSE, "entering %s", __func__);

#if defined(CKPT)
    MPIDI_CH3I_CR_lock();
#endif

    if (sreq->dev.ext_hdr_sz > 0) {
        int i;
        for (i = n_iov - 1; i >= 1; i--) {
            iov[i + 1].iov_base = iov[i].iov_base;
            iov[i + 1].iov_len = iov[i].iov_len;
        }
        iov[1].iov_base = (void *)sreq->dev.ext_hdr_ptr;
        iov[1].iov_len = sreq->dev.ext_hdr_sz;
        n_iov++;
    }

    if (SMP_INIT && vc->smp.local_nodes >= 0 &&
        vc->smp.local_nodes != g_smpi.my_local_id) {
        if ((mpi_errno = MPIDI_CH3_SMP_iSendv(vc, sreq, iov, n_iov)) !=
            MPI_SUCCESS) {
            MPIR_ERR_POP(mpi_errno);
        }

        MPL_DBG_MSG_S(MPIDI_CH3_DBG_CHANNEL, VERBOSE, "exiting %s", __func__);
        goto fn_exit;
    }

#ifdef _ENABLE_UD_
    if (MVP_USE_UD_HYBRID) {
        int len;
        Calculate_IOV_len(iov, n_iov, len);
    }
#endif

    /*CM code*/
    if ((vc->ch.state != MPIDI_CH3I_VC_STATE_IDLE
#ifdef _ENABLE_XRC_
         || (USE_XRC && VC_XST_ISUNSET(vc, XF_SEND_IDLE))
#endif
             ) ||
        !MPIDI_CH3I_CM_SendQ_empty(vc)) {
        /*Request need to be queued*/
        MPL_DBG_MSG(MPIDI_CH3_DBG_CHANNEL, VERBOSE, "not connected, enqueuing");

        if ((mpi_errno = update_request(sreq, iov, n_iov, 0, 0)) !=
            MPI_SUCCESS) {
            MPIR_ERR_POP(mpi_errno);
        }

        MPIDI_CH3I_CM_SendQ_enqueue(vc, sreq);

        if (vc->ch.state == MPIDI_CH3I_VC_STATE_UNCONNECTED) {
            MPIDI_CH3I_CM_Connect(vc);
        }

        goto fn_exit;
    }

    /* If send queue is empty attempt to send
       data, queuing any unsent data. */
    if (MPIDI_CH3I_SendQ_empty(vc)) { /* MT */
        int nb;
        int pkt_len;
        int complete;
        /* MT - need some signalling to lock down our right to use the
           channel, thus insuring that the progress engine does also try to
           write */
        Calculate_IOV_len(iov, n_iov, pkt_len);

        if (pkt_len > MRAIL_MAX_EAGER_SIZE
#ifdef _ENABLE_UD_
            ||
            (MVP_USE_UD_HYBRID && (!(vc->mrail.state & MRAILI_RC_CONNECTED) &&
                                   pkt_len > MRAIL_MAX_UD_SIZE))
#endif
        ) {
            MPIR_Memcpy(sreq->dev.iov, iov, n_iov * sizeof(struct iovec));
            sreq->dev.iov_count = n_iov;
            mpi_errno = MPIDI_CH3_Packetized_send(vc, sreq);
            if (MPI_MRAIL_MSG_QUEUED == mpi_errno) {
                mpi_errno = MPI_SUCCESS;
            }
            goto fn_exit;
        }

        if (sreq->dev.OnDataAvail == MPIDI_CH3_ReqHandler_SendReloadIOV) {
            /*reload iov */
            void *tmpbuf;
            int iter_iov;

            tmpbuf = MPL_malloc(sreq->dev.msgsize + pkt_len, MPL_MEM_OTHER);
            databuf = tmpbuf;
            pkt_len = 0;
            /* First copy whatever has already been in iov set */
            for (iter_iov = 0; iter_iov < n_iov; iter_iov++) {
                MPIR_Memcpy(tmpbuf, iov[iter_iov].iov_base,
                            iov[iter_iov].iov_len);
                tmpbuf =
                    (void *)((unsigned long)tmpbuf + iov[iter_iov].iov_len);
                pkt_len += iov[iter_iov].iov_len;
            }
            DEBUG_PRINT("Pkt len after first stage %d\n", pkt_len);
            /* Second reload iov and copy */
            do {
                sreq->dev.iov_count = MPL_IOV_LIMIT;
                mpi_errno = MPIDI_CH3U_Request_load_send_iov(
                    sreq, sreq->dev.iov, &sreq->dev.iov_count);
                /* --BEGIN ERROR HANDLING-- */
                if (mpi_errno != MPI_SUCCESS) {
                    MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER,
                                        "**ch3|loadsendiov");
                }
                for (iter_iov = 0; iter_iov < sreq->dev.iov_count; iter_iov++) {
                    MPIR_Memcpy(tmpbuf, sreq->dev.iov[iter_iov].iov_base,
                                sreq->dev.iov[iter_iov].iov_len);
                    tmpbuf = (void *)((unsigned long)tmpbuf +
                                      sreq->dev.iov[iter_iov].iov_len);
                    pkt_len += sreq->dev.iov[iter_iov].iov_len;
                }
            } while (sreq->dev.OnDataAvail ==
                     MPIDI_CH3_ReqHandler_SendReloadIOV);
            iov[0].iov_base = databuf;
            iov[0].iov_len = pkt_len;
            n_iov = 1;
        }

        if (pkt_len > MRAIL_MAX_EAGER_SIZE
#ifdef _ENABLE_UD_
            ||
            (MVP_USE_UD_HYBRID && (!(vc->mrail.state & MRAILI_RC_CONNECTED) &&
                                   pkt_len > MRAIL_MAX_UD_SIZE))
#endif
        ) {
            MPIR_Memcpy(sreq->dev.iov, iov, n_iov * sizeof(struct iovec));
            sreq->dev.iov_count = n_iov;
            mpi_errno = MPIDI_CH3_Packetized_send(vc, sreq);
            if (MPI_MRAIL_MSG_QUEUED == mpi_errno) {
                mpi_errno = MPI_SUCCESS;
            }
            goto fn_exit;
        }

        DEBUG_PRINT("[send], n_iov: %d, pkt_len %d\n", n_iov, pkt_len);

        {
            vbuf *buf;

            mpi_errno = MPIDI_CH3I_MRAILI_Eager_send(vc, iov, n_iov, pkt_len,
                                                     &nb, &buf);
            DEBUG_PRINT("[istartmsgv] mpierr %d, nb %d\n", mpi_errno, nb);
            if (mpi_errno == MPI_SUCCESS) {
                DEBUG_PRINT("[send path] eager send return %d bytes\n", nb);

                if (nb == 0) {
                    /* under layer cannot send out the msg because there is no
                     credit or
                     * no send wqe available
                     DEBUG_PRINT("Send 0 bytes\n");
                     create_request(sreq, iov, n_iov, 0, 0);
                     MPIDI_CH3I_SendQ_enqueue(vc, sreq);
                     */
                } else {
                    MPIDI_CH3U_Handle_send_req(vc, sreq, &complete);
                    if (!complete) {
                        /* NOTE: dev.iov_count is used to detect completion
                         instead of
                         * cc
                         * because the transfer may be complete, but
                         request may still be active (see MPI_Ssend()) */
                        MPIDI_CH3I_SendQ_enqueue_head(vc, sreq);
                        vc->ch.send_active = sreq;
                    } else {
                        vc->ch.send_active = MPIDI_CH3I_SendQ_head(vc);
                    }
                }
            } else if (MPI_MRAIL_MSG_QUEUED == mpi_errno) {
                buf->sreq = (void *)sreq;
                mpi_errno = MPI_SUCCESS;
            } else {
                /* Connection just failed.  Mark the request complete and return
                 * an error. */
                vc->ch.state = MPIDI_CH3I_VC_STATE_FAILED;
                /* TODO: Create an appropriate error message based on the value
                 * of errno */
                sreq->status.MPI_ERROR = MPI_ERR_INTERN;
                /* MT - CH3U_Request_complete performs write barrier */
                MPID_Request_complete(sreq);
            }
            goto fn_exit;
        }
    } else {
        MPL_DBG_MSG(MPIDI_CH3_DBG_CHANNEL, VERBOSE,
                    "send queue not empty, enqueuing");

        if ((mpi_errno = update_request(sreq, iov, n_iov, 0, 0)) !=
            MPI_SUCCESS) {
            MPIR_ERR_POP(mpi_errno);
        }

        MPIDI_CH3I_SendQ_enqueue(vc, sreq);
    }

fn_exit:
    if (databuf)
        MPL_free(databuf);
#ifdef CKPT
    MPIDI_CH3I_CR_unlock();
#endif
    MPL_DBG_MSG_S(MPIDI_CH3_DBG_CHANNEL, VERBOSE, "exiting %s", __func__);
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_CH3_ISENDV);
    return mpi_errno;

fn_fail:
    goto fn_exit;
}
