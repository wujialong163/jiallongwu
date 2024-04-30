
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
#include "mvp_req_fields.h"
#include "mvp_req.h"
#include "mvp_req_utils.h"
#include "mvp_eager_handlers.h"
#include "mvp_recv_utils.h"

extern unsigned long long PVAR_LEVEL_unexpected_recvq_buffer_size
    ATTRIBUTE((unused));

int MPIDI_MVP_smp_handle_send_req(MPIR_Request *sreq, int *complete)
{
    int mpi_errno = MPI_SUCCESS;
    int (*reqFn)(MPIR_Request *, int *);

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_HANDLE_SEND_REQ);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_HANDLE_SEND_REQ);

    /* Use the associated function rather than switching on the old ca field */
    /* Routines can call the attached function directly */
    reqFn = MPIDI_MVP_REQUEST(sreq, dev.OnDataAvail);
    if (!reqFn) {
        MPIR_Assert(MPIDI_Request_get_type(sreq) !=
                    MPIDI_REQUEST_TYPE_GET_RESP);

        /* Free tmp_buf for packed ddt */
        if (MPIDI_MVP_REQUEST(sreq, dev.iov[0].iov_base) ==
            MPIDI_MVP_REQUEST(sreq, dev.tmpbuf)) {
            MPL_free(MPIDI_MVP_REQUEST(sreq, dev.iov[0].iov_base));
        }
        MPID_Request_complete(sreq);
        *complete = 1;
    } else {
        mpi_errno = reqFn(sreq, complete);
    }
    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_HANDLE_SEND_REQ);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIDI_MVP_smp_handle_recv_req(MPIR_Request *rreq, int *complete)
{
    static int in_routine ATTRIBUTE((unused)) = FALSE;
    int mpi_errno = MPI_SUCCESS;
    int (*reqFn)(MPIR_Request *, int *);

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_HANDLE_RECV_REQ);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_HANDLE_RECV_REQ);

    MPIR_Assert(!in_routine);
    in_routine = TRUE;

    reqFn = MPIDI_MVP_REQUEST(rreq, dev.OnDataAvail);
    if (!reqFn) {
        if (MPIDI_Request_get_type(rreq) != MPIDI_REQUEST_TYPE_RECV &&
            MPIDI_Request_get_type(rreq) != MPIDI_REQUEST_TYPE_IRECV &&
            MPIDI_Request_get_type(rreq) != MPIDI_REQUEST_TYPE_ACCUM_RECV &&
            MPIDI_Request_get_type(rreq) != MPIDI_REQUEST_TYPE_GET_ACCUM_RECV) {
            MPIR_ERR_SETFATALANDJUMP1(
                mpi_errno, MPI_ERR_OTHER, "**fail", "**fail %s",
                "MPIDI_Request_get_type(rreq) != MPIDI_REQUEST_TYPE_RECV");
        }
        MPID_Request_complete(rreq);
        *complete = TRUE;
    } else {
        mpi_errno = reqFn(rreq, complete);
    }
    MPIR_ERR_CHECK(mpi_errno);

    in_routine = FALSE;

fn_exit:
    /*if (TRUE == *complete && MVP_RNDV_PROTOCOL_R3 == rreq->mrail.protocol) {
      MPIDI_CH3I_MRAILI_RREQ_RNDV_FINISH(rreq);
      }
    else if (PARTIAL_COMPLETION == *complete) {
        *complete = 1;
    }*/
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_HANDLE_RECV_REQ);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

/* TODO: remove both old versions */
int MPIDI_MVP_Handle_send_req(MPIDI_MVP_ep_t *vc, MPIR_Request *sreq,
                              int *complete)
{
    int mpi_errno = MPI_SUCCESS;
    int (*reqFn)(MPIR_Request *, int *);
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_HANDLE_SEND_REQ);

    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_HANDLE_SEND_REQ);

    PRINT_DEBUG(DEBUG_SHM_verbose > 1,
                "vc: %p, rank: %d, sreq: %p, type: %d, onDataAvail: %p\n", vc,
                vc->pg_rank, sreq, MPIDI_Request_get_type(sreq),
                MPIDI_MVP_REQUEST(sreq, dev).OnDataAvail);

    /* Use the associated function rather than switching on the old ca field */
    /* Routines can call the attached function directly */
    reqFn = MPIDI_MVP_REQUEST(sreq, dev.OnDataAvail);
    if (!reqFn) {
        MPIR_Assert(MPIDI_Request_get_type(sreq) !=
                    MPIDI_REQUEST_TYPE_GET_RESP);
        *complete = 1;
    } else {
        mpi_errno = reqFn(sreq, complete);
    }
    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_HANDLE_SEND_REQ);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIDI_MVP_Handle_recv_req(MPIDI_MVP_ep_t *vc, MPIR_Request *rreq,
                              int *complete)
{
    static int in_routine ATTRIBUTE((unused)) = FALSE;
    int mpi_errno = MPI_SUCCESS;
    int (*reqFn)(MPIR_Request *, int *);

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_HANDLE_RECV_REQ);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_HANDLE_RECV_REQ);

    MPIR_Assert(!in_routine);
    in_routine = TRUE;

    reqFn = MPIDI_MVP_REQUEST(rreq, dev.OnDataAvail);
    if (!reqFn) {
        if (MPIDI_Request_get_type(rreq) != MPIDI_REQUEST_TYPE_RECV &&
            MPIDI_Request_get_type(rreq) != MPIDI_REQUEST_TYPE_IRECV &&
            MPIDI_Request_get_type(rreq) != MPIDI_REQUEST_TYPE_ACCUM_RECV &&
            MPIDI_Request_get_type(rreq) != MPIDI_REQUEST_TYPE_GET_ACCUM_RECV) {
            MPIR_ERR_SETFATALANDJUMP1(
                mpi_errno, MPI_ERR_OTHER, "**fail", "**fail %s",
                "MPIDI_Request_get_type(rreq) != MPIDI_REQUEST_TYPE_RECV");
        }
        MPID_Request_complete(rreq);
        *complete = TRUE;
    } else {
        mpi_errno = reqFn(rreq, complete);
        MPIR_ERR_CHECK(mpi_errno);
    }

    in_routine = FALSE;

fn_exit:
    /*if (TRUE == *complete && MVP_RNDV_PROTOCOL_R3 == rreq->mrail.protocol) {
      MPIDI_CH3I_MRAILI_RREQ_RNDV_FINISH(rreq);
      }
    else if (PARTIAL_COMPLETION == *complete) {
        *complete = 1;
    }*/

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_HANDLE_RECV_REQ);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

/*
 * MPIDI_MVP_Request_adjust_iov()
 *
 * Adjust the iovec in the request by the supplied number of bytes.
 * If the iovec has been consumed, return true; otherwise return
 * false.
 * TODO: change return calling convention
 */
int MPIDI_MVP_Request_adjust_iov(MPIR_Request *greq, intptr_t nb)
{
    MPIDI_MVP_smp_request_t *req = MPIDI_MVP_REQUEST_FROM_MPICH(greq);
    int offset = req->dev.iov_offset;
    const int count = req->dev.iov_count;

    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_REQUEST_ADJUST_IOV);
    if (req->mrail.is_eager_vbuf_queued == 1) {
        if (req->dev.recv_data_sz == req->mrail.eager_unexp_size) {
            return TRUE;
        } else {
            return FALSE;
        }
    }

    while (offset < count) {
        if (req->dev.iov[offset].iov_len <= (intptr_t)nb) {
            nb -= req->dev.iov[offset].iov_len;
            ++offset;
        } else {
            req->dev.iov[offset].iov_base =
                ((char *)req->dev.iov[offset].iov_base) + nb;
            req->dev.iov[offset].iov_len -= nb;
            req->dev.iov_offset = offset;
            MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_REQUEST_ADJUST_IOV);
            return FALSE;
        }
    }

    req->dev.iov_offset = 0;

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_REQUEST_ADJUST_IOV);

    return TRUE;
}

/*
 * MPIDI_MVP_Request_unpack_uebuf
 *
 * Copy/unpack data from an "unexpected eager buffer" into the user buffer.
 */
int MPIDI_MVP_Request_unpack_uebuf(MPIR_Request *rreq)
{
    int dt_contig;
    MPI_Aint dt_true_lb;
    intptr_t userbuf_sz;
    MPIR_Datatype *dt_ptr;
    intptr_t unpack_sz;
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_REQUEST_UNPACK_UEBUF);
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MEMCPY);

    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_REQUEST_UNPACK_UEBUF);

    MPIDI_Datatype_get_info(MPIDI_MVP_REQUEST(rreq, dev).user_count,
                            MPIDI_MVP_REQUEST(rreq, dev).datatype, dt_contig,
                            userbuf_sz, dt_ptr, dt_true_lb);

    if (MPIDI_MVP_REQUEST(rreq, dev).recv_data_sz <= userbuf_sz) {
        unpack_sz = MPIDI_MVP_REQUEST(rreq, dev).recv_data_sz;
    } else {
        /* --BEGIN ERROR HANDLING-- */
        /*MPL_DBG_MSG_FMT(MPIDI_MVP_DBG_CHANNEL,VERBOSE,(MPL_DBG_FDEST,
          "receive buffer overflow; message truncated, msg_sz=%" PRIdPTR
              ", buf_sz=%" PRIdPTR,
                    MPIDI_MVP_REQUEST(rreq, dev).recv_data_sz, userbuf_sz));*/
        unpack_sz = userbuf_sz;
        MPIR_STATUS_SET_COUNT(rreq->status, userbuf_sz);
        rreq->status.MPI_ERROR = MPIR_Err_create_code(
            MPI_SUCCESS, MPIR_ERR_RECOVERABLE, __func__, __LINE__,
            MPI_ERR_TRUNCATE, "**truncate", "**truncate %d %d",
            MPIDI_MVP_REQUEST(rreq, dev).recv_data_sz, userbuf_sz);
        /* --END ERROR HANDLING-- */
    }

    if (unpack_sz > 0) {
        if (dt_contig) {
            /* TODO - check that amount of data is consistent with
               datatype.  If not we should return an error (unless
               configured with --enable-fast) */
            MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MEMCPY);
            //#if defined(CHANNEL_MRAIL)
            //            if (rreq->mrail.is_eager_vbuf_queued == 1) {
            //                MPIR_Assert(rreq->mrail.eager_unexp_size ==
            //                unpack_sz); vbuf *v = rreq->mrail.eager_vbuf_head;
            //                char *ubuf = (char *)MPIDI_MVP_REQUEST(rreq,
            //                dev).user_buf + dt_true_lb; while ((unpack_sz > 0)
            //                && (v != NULL)) {
            //                    MPIR_Memcpy(ubuf, v->unexp_data_buf,
            //                            v->data_size);
            //                    unpack_sz -= v->data_size;
            //                    ubuf = (char *)ubuf + v->data_size;
            //                    if ((v->next != NULL) ||
            //                    (v->in_eager_sgl_queue != 2)) {
            //                        v->in_eager_sgl_queue = 0;
            //                        MPIDI_MVPI_MRAIL_Release_vbuf(v);
            //                    }
            //                    v = v->next;
            //                }
            //                MPIR_Assert(unpack_sz == 0);
            //            } else
            //#endif
            {
                MPIR_Memcpy((char *)MPIDI_MVP_REQUEST(rreq, dev).user_buf +
                                dt_true_lb,
                            MPIDI_MVP_REQUEST(rreq, dev).tmpbuf, unpack_sz);
            }
            MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MEMCPY);
        } else {
            MPI_Aint actual_unpack_bytes;
            //#if defined(CHANNEL_MRAIL)
            //            if (rreq->mrail.is_eager_vbuf_queued == 1) {
            //                MPI_Aint first = 0;
            //                vbuf *v = rreq->mrail.eager_vbuf_head;
            //                MPIR_Assert(rreq->mrail.eager_unexp_size ==
            //                unpack_sz); while ((unpack_sz > 0) && (v != NULL))
            //                {
            //                    actual_unpack_bytes = v->data_size;
            //                    MPIR_Typerep_unpack(v->unexp_data_buf,
            //                    unpack_sz,
            //                            MPIDI_MVP_REQUEST(rreq, dev).user_buf,
            //                            MPIDI_MVP_REQUEST(rreq,
            //                            dev).user_count,
            //                            MPIDI_MVP_REQUEST(rreq, dev).datatype,
            //                            0, &actual_unpack_bytes);
            //                    if ((v->next != NULL) ||
            //                    (v->in_eager_sgl_queue != 2)) {
            //                        v->in_eager_sgl_queue = 0;
            //                        MPIDI_MVPI_MRAIL_Release_vbuf(v);
            //                    }
            //                    v = v->next;
            //                }
            //                MPIR_Assert(unpack_sz == 0);
            //            } else
            //#endif
            {
                actual_unpack_bytes = unpack_sz;
                MPIR_Typerep_unpack(MPIDI_MVP_REQUEST(rreq, dev).tmpbuf,
                                    unpack_sz,
                                    MPIDI_MVP_REQUEST(rreq, dev).user_buf,
                                    MPIDI_MVP_REQUEST(rreq, dev).user_count,
                                    MPIDI_MVP_REQUEST(rreq, dev).datatype, 0,
                                    &actual_unpack_bytes);

                if (actual_unpack_bytes != unpack_sz) {
                    /* --BEGIN ERROR HANDLING-- */
                    /* received data was not entirely consumed by unpack()
                       because too few bytes remained to fill the next basic
                       datatype */
                    MPIR_STATUS_SET_COUNT(rreq->status, actual_unpack_bytes);
                    rreq->status.MPI_ERROR = MPIR_Err_create_code(
                        MPI_SUCCESS, MPIR_ERR_RECOVERABLE, __func__, __LINE__,
                        MPI_ERR_TYPE, "**dtypemismatch", 0);
                    /* --END ERROR HANDLING-- */
                }
            }
        }
    }

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_REQUEST_UNPACK_UEBUF);
    return mpi_errno;
}

/*
 * This function is used to receive data from the receive buffer to
 * the user buffer.  If all data for this message has not been
 * received, the request is set up to receive the next data to arrive.
 * In turn, this request is attached to a virtual connection.
 *
 * buflen is an I/O parameter.  The length of the received data is
 * passed in.  The function returns the number of bytes actually
 * processed by this function.
 *
 * complete is an OUTPUT variable.  It is set to TRUE iff all of the
 * data for the request has been received.  This function does not
 * actually complete the request.
 */
int MPIDI_MVP_Receive_data_found(MPIR_Request *rreq, void *buf,
                                 intptr_t *buflen, int *complete)
{
    int dt_contig;
    MPI_Aint dt_true_lb;
    intptr_t userbuf_sz;
    MPIR_Datatype *dt_ptr = NULL;
    intptr_t data_sz;
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_RECEIVE_DATA_FOUND);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_RECEIVE_DATA_FOUND);

    MPIDI_Datatype_get_info(MPIDI_MVP_REQUEST(rreq, dev.user_count),
                            MPIDI_MVP_REQUEST(rreq, dev.datatype), dt_contig,
                            userbuf_sz, dt_ptr, dt_true_lb);

    if (MPIDI_MVP_REQUEST(rreq, dev.recv_data_sz) <= userbuf_sz) {
        data_sz = MPIDI_MVP_REQUEST(rreq, dev.recv_data_sz);
    } else {
        /*MPL_DBG_MSG_FMT(MPIDI_MVP_DBG_OTHER,VERBOSE,(MPL_DBG_FDEST,
                "receive buffer too small; message truncated, msg_sz=%" PRIdPTR
           ", userbuf_sz=%" PRIdPTR,
                MPIDI_MVP_REQUEST(rreq, dev.recv_data_sz), userbuf_sz));*/
        rreq->status.MPI_ERROR = MPIR_Err_create_code(
            MPI_SUCCESS, MPIR_ERR_RECOVERABLE, __func__, __LINE__,
            MPI_ERR_TRUNCATE, "**truncate", "**truncate %d %d %d %d",
            rreq->status.MPI_SOURCE, rreq->status.MPI_TAG,
            MPIDI_MVP_REQUEST(rreq, dev.recv_data_sz), userbuf_sz);
        MPIR_STATUS_SET_COUNT(rreq->status, userbuf_sz);
        data_sz = userbuf_sz;
    }

    if (likely(dt_contig &&
               data_sz == MPIDI_MVP_REQUEST(rreq, dev.recv_data_sz))) {
        /* user buffer is contiguous and large enough to store the
           entire message.  However, we haven't yet *read* the data
           (this code describes how to read the data into the destination) */

        /* if all of the data has already been received, unpack it
           now, otherwise build an iov and let the channel unpack */
        if (likely(*buflen >= data_sz)) {
            // MPL_DBG_MSG(MPIDI_MVP_DBG_OTHER,VERBOSE,"Copying contiguous data
            // to user buffer");
            /* copy data out of the receive buffer */

            if (likely(MPIDI_MVP_REQUEST(rreq, dev.drop_data) == FALSE)) {
                MPIR_Memcpy((char *)(MPIDI_MVP_REQUEST(rreq, dev.user_buf)) +
                                dt_true_lb,
                            buf, data_sz);
            }
            *buflen = data_sz;
            *complete = TRUE;
        } else {
            // MPL_DBG_MSG(MPIDI_MVP_DBG_OTHER,VERBOSE,"IOV loaded for
            // contiguous read");

            MPIDI_MVP_REQUEST(rreq, dev.iov[0].iov_base) =
                (void *)((char *)(MPIDI_MVP_REQUEST(rreq, dev.user_buf)) +
                         dt_true_lb);
            MPIDI_MVP_REQUEST(rreq, dev.iov[0].iov_len) = data_sz;
            MPIDI_MVP_REQUEST(rreq, dev.iov_count) = 1;
            *buflen = 0;
            *complete = FALSE;
        }

        /* Trigger OnFinal when receiving the last segment */
        MPIDI_MVP_REQUEST(rreq, dev.OnDataAvail) =
            MPIDI_MVP_REQUEST(rreq, dev.OnFinal);
    } else {
        /* user buffer is not contiguous or is too small to hold
           the entire message */
        MPIDI_MVP_REQUEST(rreq, dev.msg_offset) = 0;
        MPIDI_MVP_REQUEST(rreq, dev.msgsize) = data_sz;

        /* if all of the data has already been received, and the
           message is not truncated, unpack it now, otherwise build an
           iov and let the channel unpack */
        if (data_sz == MPIDI_MVP_REQUEST(rreq, dev.recv_data_sz)) {
            // MPL_DBG_MSG(MPIDI_MVP_DBG_OTHER,VERBOSE,"Copying noncontiguous
            // data to user buffer");

            MPI_Aint actual_unpack_bytes;
            MPIR_Typerep_unpack(
                buf, data_sz, MPIDI_MVP_REQUEST(rreq, dev.user_buf),
                MPIDI_MVP_REQUEST(rreq, dev.user_count),
                MPIDI_MVP_REQUEST(rreq, dev.datatype),
                MPIDI_MVP_REQUEST(rreq, dev.msg_offset), &actual_unpack_bytes);

            /* --BEGIN ERROR HANDLING-- */
            if (actual_unpack_bytes != data_sz) {
                /* If the data can't be unpacked, the we have a
                   mismatch between the datatype and the amount of
                   data received.  Throw away received data. */
                MPIR_ERR_SET(rreq->status.MPI_ERROR, MPI_ERR_TYPE,
                             "**dtypemismatch");
                MPIR_STATUS_SET_COUNT(rreq->status,
                                      MPIDI_MVP_REQUEST(rreq, dev.msg_offset));
                *buflen = data_sz;
                *complete = TRUE;
                /* FIXME: Set OnDataAvail to 0?  If not, why not? */
                goto fn_exit;
            }
            /* --END ERROR HANDLING-- */
            *buflen = data_sz;
            /* Trigger OnFinal when receiving the last segment */
            MPIDI_MVP_REQUEST(rreq, dev.OnDataAvail) =
                MPIDI_MVP_REQUEST(rreq, dev.OnFinal);
            *complete = TRUE;
        } else {
            /* Should not enter */
            if (mpi_errno != MPI_SUCCESS) {
                MPIR_ERR_SETFATALANDJUMP(mpi_errno, MPI_ERR_OTHER,
                                         "**mvp_loadrecviov");
            }
        }
    }

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_RECEIVE_DATA_FOUND);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIDI_MVP_Receive_data_unexpected(MPIR_Request *rreq, void *buf,
                                      intptr_t *buflen, int *complete)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_RECEIVE_DATA_UNEXPECTED);

    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_RECEIVE_DATA_UNEXPECTED);

    /* FIXME: to improve performance, allocate temporary buffer from a
       specialized buffer pool. */
    /* FIXME: to avoid memory exhaustion, integrate buffer pool management
       with flow control */
    /* MPL_DBG_MSG(MPIDI_MVP_DBG_OTHER,VERBOSE,"unexpected request allocated");
     */

    if (MPIDI_MVP_REQUEST(rreq, mrail.is_eager_vbuf_queued) == 1) {
        MPIR_Assert(MPIDI_MVP_REQUEST(rreq, mrail.eager_vbuf_tail) == NULL);
        MPIR_Assert(MPIDI_MVP_REQUEST(rreq, mrail.eager_vbuf_head) == NULL);
        MPIDI_MVP_REQUEST(rreq, dev.recv_pending_count) = 2;
        *buflen = 0;
        *complete = FALSE;
        MPIDI_MVP_REQUEST(rreq, dev.OnDataAvail) =
            MPIDI_MVP_ReqHandler_UnpackUEBufComplete;
        return mpi_errno;
    }

    MPIDI_MVP_REQUEST(rreq, dev.tmpbuf) =
        MPL_malloc(MPIDI_MVP_REQUEST(rreq, dev.recv_data_sz), MPL_MEM_BUFFER);
    if (!MPIDI_MVP_REQUEST(rreq, dev.tmpbuf)) {
        MPIR_ERR_SETANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**nomem", "**nomem %d",
                             MPIDI_MVP_REQUEST(rreq, dev.recv_data_sz));
    }
    MPIDI_MVP_REQUEST(rreq, dev.tmpbuf_sz) =
        MPIDI_MVP_REQUEST(rreq, dev.recv_data_sz);

    /* if all of the data has already been received, copy it
       now, otherwise build an iov and let the channel copy it */
    if (MPIDI_MVP_REQUEST(rreq, dev.recv_data_sz) <= *buflen) {
        MPIR_Memcpy(MPIDI_MVP_REQUEST(rreq, dev.tmpbuf), buf,
                    MPIDI_MVP_REQUEST(rreq, dev.recv_data_sz));
        *buflen = MPIDI_MVP_REQUEST(rreq, dev.recv_data_sz);
        MPIDI_MVP_REQUEST(rreq, dev.recv_pending_count) = 1;
        *complete = TRUE;
    } else {
        MPIDI_MVP_REQUEST(rreq, dev.iov[0].iov_base) =
            (void *)((char *)MPIDI_MVP_REQUEST(rreq, dev.tmpbuf));
        MPIDI_MVP_REQUEST(rreq, dev.iov[0].iov_len) =
            MPIDI_MVP_REQUEST(rreq, dev.recv_data_sz);
        MPIDI_MVP_REQUEST(rreq, dev.iov_count) = 1;
        MPIDI_MVP_REQUEST(rreq, dev.recv_pending_count) = 2;
        *buflen = 0;
        *complete = FALSE;
    }

    if (MPIDI_Request_get_msg_type(rreq) == MPIDI_REQUEST_EAGER_MSG)
        MPIR_T_PVAR_LEVEL_INC(RECVQ, unexpected_recvq_buffer_size,
                              MPIDI_MVP_REQUEST(rreq, dev.tmpbuf_sz));

    MPIDI_MVP_REQUEST(rreq, dev.OnDataAvail) =
        MPIDI_MVP_ReqHandler_UnpackUEBufComplete;

fn_fail:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_RECEIVE_DATA_UNEXPECTED);
    return mpi_errno;
}
