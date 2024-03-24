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
#include "mpidrma.h"

int MPIDI_CH3U_Handle_send_req(MPIDI_VC_t * vc, MPIR_Request * sreq, int *complete)
{
    int mpi_errno = MPI_SUCCESS;
    int (*reqFn) (MPIDI_VC_t *, MPIR_Request *, int *);
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_CH3U_HANDLE_SEND_REQ);

    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_CH3U_HANDLE_SEND_REQ);

#if defined(CHANNEL_MRAIL)
    PRINT_DEBUG(DEBUG_SHM_verbose>1,
            "vc: %p, rank: %d, sreq: %p, type: %d, onDataAvail: %p\n",
            vc, vc->pg_rank, sreq, MPIDI_Request_get_type(sreq), sreq->dev.OnDataAvail);
#endif /*defined(CHANNEL_MRAIL)*/

    /* Use the associated function rather than switching on the old ca field */
    /* Routines can call the attached function directly */
    reqFn = sreq->dev.OnDataAvail;
    if (!reqFn) {
        MPIR_Assert(MPIDI_Request_get_type(sreq) != MPIDI_REQUEST_TYPE_GET_RESP);
        mpi_errno = MPID_Request_complete(sreq);
        *complete = 1;
    }
    else {
        mpi_errno = reqFn(vc, sreq, complete);
    }
    MPIR_ERR_CHECK(mpi_errno);

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_CH3U_HANDLE_SEND_REQ);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

/* ----------------------------------------------------------------------- */
/* Here are the functions that implement the actions that are taken when
 * data is available for a send request (or other completion operations)
 * These include "send" requests that are part of the RMA implementation.
 */
/* ----------------------------------------------------------------------- */

int MPIDI_CH3_ReqHandler_GetSendComplete(MPIDI_VC_t * vc ATTRIBUTE((unused)),
                                         MPIR_Request * sreq, int *complete)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Win *win_ptr;
    
    int pkt_flags = sreq->dev.pkt_flags;

    MPIR_Win_get_ptr(sreq->dev.target_win_handle, win_ptr);

    /* MVP designs for get rndv with R3 protocol. 
     * Should this come after the check for double completions? 
     */
#if defined(CHANNEL_MRAIL)
    if (MRAILI_PROTOCOL_R3 == sreq->mrail.protocol &&
        (sreq->dev.recv_data_sz + sizeof(MPIDI_CH3_Pkt_get_resp_t)) >
            vc->eager_max_msg_sz) {
        struct iovec iov[1] = {0};
        int iovcnt;
        MPIDI_CH3_Pkt_t upkt;
        MPIR_Request *resp_req;

        resp_req = MPIR_Request_create(MPIR_REQUEST_KIND__SEND);
        MPIR_ERR_CHKANDJUMP(resp_req == NULL, mpi_errno, MPI_ERR_OTHER, "**nomemreq");
        MPIR_Object_set_ref(resp_req, 1);
        MPIDI_CH3_Pkt_get_resp_t *get_resp_pkt = &upkt.get_resp;
        MPIDI_Pkt_init(get_resp_pkt, MPIDI_CH3_PKT_GET_RESP);
        resp_req->dev.OnFinal = NULL;
        resp_req->dev.OnDataAvail = NULL;
        MVP_INC_NUM_POSTED_SEND();


        get_resp_pkt->request_handle = sreq->dev.resp_request_handle;
        get_resp_pkt->pkt_flags = MPIDI_CH3_PKT_FLAG_NONE;
        if (pkt_flags & MPIDI_CH3_PKT_FLAG_RMA_LOCK_SHARED ||
                pkt_flags & MPIDI_CH3_PKT_FLAG_RMA_LOCK_EXCLUSIVE)
            get_resp_pkt->pkt_flags |= MPIDI_CH3_PKT_FLAG_RMA_LOCK_GRANTED;
        if ((pkt_flags & MPIDI_CH3_PKT_FLAG_RMA_FLUSH) ||
                (pkt_flags & MPIDI_CH3_PKT_FLAG_RMA_UNLOCK))
            get_resp_pkt->pkt_flags |= MPIDI_CH3_PKT_FLAG_RMA_ACK;
        get_resp_pkt->target_rank = win_ptr->comm_ptr->rank;

        get_resp_pkt->protocol = MRAILI_PROTOCOL_EAGER;

        iov[0].iov_base = (void *) get_resp_pkt;
        iov[0].iov_len = sizeof(*get_resp_pkt);
        iovcnt = 1;

        mpi_errno = MPIDI_CH3_iSendv(vc, resp_req, iov, iovcnt);
    }
#endif

    /* NOTE: It is possible that this request is already completed before
     * entering this handler. This happens when this req handler is called
     * within the same req handler on the same request.
     * Consider this case: req is queued up in SHM queue with ref count of 2:
     * one is for completing the request and another is for dequeueing from
     * the queue. The first called req handler on this request completed
     * this request and decrement ref counter to 1. Request is still in the
     * queue. Within this handler, we call the req handler on the same request
     * for the second time (for example when making progress on SHM queue),
     * and the second called handler also tries to complete this request,
     * which leads to wrong execution.
     * Here we check if req is already completed to prevent processing the
     * same request twice. */
    if (MPIR_Request_is_complete(sreq)) {
        *complete = FALSE;
        goto fn_exit;
    }

    /* here we decrement the Active Target counter to guarantee the GET-like
     * operation are completed when counter reaches zero. */
    win_ptr->at_completion_counter--;
    MPIR_Assert(win_ptr->at_completion_counter >= 0);

    /* mark data transfer as complete and decrement CC */
    mpi_errno = MPID_Request_complete(sreq);
    MPIR_ERR_CHECK(mpi_errno);

    /* NOTE: finish_op_on_target() must be called after we complete this request,
     * because inside finish_op_on_target() we may call this request handler
     * on the same request again (in release_lock()). Marking this request as
     * completed will prevent us from processing the same request twice. */
    mpi_errno = finish_op_on_target(win_ptr, vc, TRUE /* has response data */ ,
                                    pkt_flags, MPI_WIN_NULL);
    MPIR_ERR_CHECK(mpi_errno);

    *complete = TRUE;

  fn_exit:
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

int MPIDI_CH3_ReqHandler_GaccumSendComplete(MPIDI_VC_t * vc, MPIR_Request * rreq, int *complete)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Win *win_ptr;
    int pkt_flags = rreq->dev.pkt_flags;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_CH3_REQHANDLER_GACCUMSENDCOMPLETE);

    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_CH3_REQHANDLER_GACCUMSENDCOMPLETE);

    /* NOTE: It is possible that this request is already completed before
     * entering this handler. This happens when this req handler is called
     * within the same req handler on the same request.
     * Consider this case: req is queued up in SHM queue with ref count of 2:
     * one is for completing the request and another is for dequeueing from
     * the queue. The first called req handler on this request completed
     * this request and decrement ref counter to 1. Request is still in the
     * queue. Within this handler, we call the req handler on the same request
     * for the second time (for example when making progress on SHM queue),
     * and the second called handler also tries to complete this request,
     * which leads to wrong execution.
     * Here we check if req is already completed to prevent processing the
     * same request twice. */
    if (MPIR_Request_is_complete(rreq)) {
        *complete = FALSE;
        goto fn_exit;
    }

    /* This function is triggered when sending back process of GACC/FOP/CAS
     * is finished. Only GACC used user_buf. FOP and CAS can fit all data
     * in response packet. */
    MPL_free(rreq->dev.user_buf);

    MPIR_Win_get_ptr(rreq->dev.target_win_handle, win_ptr);

#if defined(CHANNEL_MRAIL)
    /* If R3 is used, we need to send MPIDI_CH3_PKT_GET_ACCUM_RESP pkt acting like FIN,
     * so the origin process can handle it properly */
    if (MRAILI_PROTOCOL_R3 == rreq->mrail.protocol) {
        struct iovec iov[1] = {0};
        int iovcnt;
        MPIDI_CH3_Pkt_t upkt;
        MPIR_Request *resp_req;

        resp_req = MPIR_Request_create(MPIR_REQUEST_KIND__SEND);
        MPIR_ERR_CHKANDJUMP(resp_req == NULL, mpi_errno, MPI_ERR_OTHER, "**nomemreq");
        MPIR_Object_set_ref(resp_req, 1);
        MPIDI_CH3_Pkt_get_accum_resp_t *get_accum_resp_pkt = (MPIDI_CH3_Pkt_get_accum_resp_t *) &upkt.get_resp;
        MPIDI_Pkt_init(get_accum_resp_pkt, MPIDI_CH3_PKT_GET_ACCUM_RESP);
        resp_req->dev.OnFinal = NULL;
        resp_req->dev.OnDataAvail = NULL;
        MVP_INC_NUM_POSTED_SEND();


        get_accum_resp_pkt->request_handle = rreq->dev.resp_request_handle;
        get_accum_resp_pkt->pkt_flags = MPIDI_CH3_PKT_FLAG_NONE;
        if (pkt_flags & MPIDI_CH3_PKT_FLAG_RMA_LOCK_SHARED ||
                pkt_flags & MPIDI_CH3_PKT_FLAG_RMA_LOCK_EXCLUSIVE)
            get_accum_resp_pkt->pkt_flags |= MPIDI_CH3_PKT_FLAG_RMA_LOCK_GRANTED;
        if ((pkt_flags & MPIDI_CH3_PKT_FLAG_RMA_FLUSH) ||
                (pkt_flags & MPIDI_CH3_PKT_FLAG_RMA_UNLOCK))
            get_accum_resp_pkt->pkt_flags |= MPIDI_CH3_PKT_FLAG_RMA_ACK;
        get_accum_resp_pkt->target_rank = win_ptr->comm_ptr->rank;

        get_accum_resp_pkt->protocol = MRAILI_PROTOCOL_EAGER;

        iov[0].iov_base = (void *) get_accum_resp_pkt;
        iov[0].iov_len = sizeof(*get_accum_resp_pkt);
        iovcnt = 1;

        mpi_errno = MPIDI_CH3_iSendv(vc, resp_req, iov, iovcnt);
    }
#endif

    /* here we decrement the Active Target counter to guarantee the GET-like
     * operation are completed when counter reaches zero. */
    win_ptr->at_completion_counter--;
    MPIR_Assert(win_ptr->at_completion_counter >= 0);

    mpi_errno = MPID_Request_complete(rreq);
    MPIR_ERR_CHECK(mpi_errno);

    /* NOTE: finish_op_on_target() must be called after we complete this request,
     * because inside finish_op_on_target() we may call this request handler
     * on the same request again (in release_lock()). Marking this request as
     * completed will prevent us from processing the same request twice. */
    mpi_errno = finish_op_on_target(win_ptr, vc, TRUE /* has response data */ ,
                                    pkt_flags, MPI_WIN_NULL);
    MPIR_ERR_CHECK(mpi_errno);

    *complete = TRUE;

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_CH3_REQHANDLER_GACCUMSENDCOMPLETE);
    return mpi_errno;

  fn_fail:
    goto fn_exit;
}


int MPIDI_CH3_ReqHandler_CASSendComplete(MPIDI_VC_t * vc, MPIR_Request * rreq, int *complete)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Win *win_ptr;
    int pkt_flags = rreq->dev.pkt_flags;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_CH3_REQHANDLER_CASSENDCOMPLETE);

    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_CH3_REQHANDLER_CASSENDCOMPLETE);

    /* NOTE: It is possible that this request is already completed before
     * entering this handler. This happens when this req handler is called
     * within the same req handler on the same request.
     * Consider this case: req is queued up in SHM queue with ref count of 2:
     * one is for completing the request and another is for dequeueing from
     * the queue. The first called req handler on this request completed
     * this request and decrement ref counter to 1. Request is still in the
     * queue. Within this handler, we call the req handler on the same request
     * for the second time (for example when making progress on SHM queue),
     * and the second called handler also tries to complete this request,
     * which leads to wrong execution.
     * Here we check if req is already completed to prevent processing the
     * same request twice. */
    if (MPIR_Request_is_complete(rreq)) {
        *complete = FALSE;
        goto fn_exit;
    }

    /* This function is triggered when sending back process of GACC/FOP/CAS
     * is finished. Only GACC used user_buf. FOP and CAS can fit all data
     * in response packet. */
    MPL_free(rreq->dev.user_buf);

    MPIR_Win_get_ptr(rreq->dev.target_win_handle, win_ptr);

    /* here we decrement the Active Target counter to guarantee the GET-like
     * operation are completed when counter reaches zero. */
    win_ptr->at_completion_counter--;
    MPIR_Assert(win_ptr->at_completion_counter >= 0);

    mpi_errno = MPID_Request_complete(rreq);
    MPIR_ERR_CHECK(mpi_errno);

    /* NOTE: finish_op_on_target() must be called after we complete this request,
     * because inside finish_op_on_target() we may call this request handler
     * on the same request again (in release_lock()). Marking this request as
     * completed will prevent us from processing the same request twice. */
    mpi_errno = finish_op_on_target(win_ptr, vc, TRUE /* has response data */ ,
                                    pkt_flags, MPI_WIN_NULL);
    MPIR_ERR_CHECK(mpi_errno);

    *complete = TRUE;

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_CH3_REQHANDLER_CASSENDCOMPLETE);
    return mpi_errno;

  fn_fail:
    goto fn_exit;
}

int MPIDI_CH3_ReqHandler_FOPSendComplete(MPIDI_VC_t * vc, MPIR_Request * rreq, int *complete)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Win *win_ptr;
    int pkt_flags = rreq->dev.pkt_flags;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_CH3_REQHANDLER_FOPSENDCOMPLETE);

    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_CH3_REQHANDLER_FOPSENDCOMPLETE);

    /* NOTE: It is possible that this request is already completed before
     * entering this handler. This happens when this req handler is called
     * within the same req handler on the same request.
     * Consider this case: req is queued up in SHM queue with ref count of 2:
     * one is for completing the request and another is for dequeueing from
     * the queue. The first called req handler on this request completed
     * this request and decrement ref counter to 1. Request is still in the
     * queue. Within this handler, we call the req handler on the same request
     * for the second time (for example when making progress on SHM queue),
     * and the second called handler also tries to complete this request,
     * which leads to wrong execution.
     * Here we check if req is already completed to prevent processing the
     * same request twice. */
    if (MPIR_Request_is_complete(rreq)) {
        *complete = FALSE;
        goto fn_exit;
    }

    /* This function is triggered when sending back process of GACC/FOP/CAS
     * is finished. Only GACC used user_buf. FOP and CAS can fit all data
     * in response packet. */
    MPL_free(rreq->dev.user_buf);

    MPIR_Win_get_ptr(rreq->dev.target_win_handle, win_ptr);

    /* here we decrement the Active Target counter to guarantee the GET-like
     * operation are completed when counter reaches zero. */
    win_ptr->at_completion_counter--;
    MPIR_Assert(win_ptr->at_completion_counter >= 0);

    mpi_errno = MPID_Request_complete(rreq);
    MPIR_ERR_CHECK(mpi_errno);

    /* NOTE: finish_op_on_target() must be called after we complete this request,
     * because inside finish_op_on_target() we may call this request handler
     * on the same request again (in release_lock()). Marking this request as
     * completed will prevent us from processing the same request twice. */
    mpi_errno = finish_op_on_target(win_ptr, vc, TRUE /* has response data */ ,
                                    pkt_flags, MPI_WIN_NULL);
    MPIR_ERR_CHECK(mpi_errno);

    *complete = TRUE;

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_CH3_REQHANDLER_FOPSENDCOMPLETE);
    return mpi_errno;

  fn_fail:
    goto fn_exit;
}


int MPIDI_CH3_ReqHandler_SendReloadIOV(MPIDI_VC_t * vc ATTRIBUTE((unused)), MPIR_Request * sreq,
                                       int *complete)
{
    int mpi_errno;

    /* setting the iov_offset to 0 here is critical, since it is intentionally
     * not set in the _load_send_iov function */
    sreq->dev.iov_offset = 0;
    sreq->dev.iov_count = MPL_IOV_LIMIT;
    mpi_errno = MPIDI_CH3U_Request_load_send_iov(sreq, sreq->dev.iov, &sreq->dev.iov_count);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_SETFATALANDJUMP(mpi_errno, MPI_ERR_OTHER, "**ch3|loadsendiov");
    }

    *complete = FALSE;

  fn_fail:
    return mpi_errno;
}
