/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda. For detailed
 * copyright and licensing information, please refer to the copyright file
 * COPYRIGHT in the top level MVAPICH directory.
 */

#ifndef _MVP_REQ_H_
#define _MVP_REQ_H_

#ifndef MVP_SMP_IMPL_INCLUDED
#error "Requres mvp_smp_impl.h, include that header first"
#endif

#if defined(HAVE_SYS_TYPES_H)
#include <sys/types.h>
#endif

//#include "../mvp_types.h"
//#include "mvp_tagm.h"
/* TODO: move this header in here it is a dependency */
#ifndef _MVP_REQ_FIELDS_H_
#error "Header requires mvp_req_fields be included first."
#endif

extern MPIDI_MVP_smp_request_t MPIDI_MVP_smp_request_direct[];
extern MPIR_Object_alloc_t MPIDI_MVP_smp_request_mem;

/* Request type macros */
#define MPIDI_REQUEST_TYPE_MASK (0xF << MPIDI_REQUEST_TYPE_SHIFT)
#define MPIDI_REQUEST_TYPE_SHIFT 5
#define MPIDI_REQUEST_TYPE_RECV 0
#define MPIDI_REQUEST_TYPE_SEND 1
#define MPIDI_REQUEST_TYPE_RSEND 2
#define MPIDI_REQUEST_TYPE_SSEND 3
/* We need a BSEND type for persistent bsends (see mpid_startall.c) */
#define MPIDI_REQUEST_TYPE_BSEND 4
#define MPIDI_REQUEST_TYPE_ISEND 5
#define MPIDI_REQUEST_TYPE_IRECV 6

#define MPIDI_REQUEST_TYPE_PUT_RECV 7                     /* target is receiving PUT data */
#define MPIDI_REQUEST_TYPE_GET_RESP 8                     /* target is sending GET response data */
#define MPIDI_REQUEST_TYPE_ACCUM_RECV 9                   /* target is receiving ACC data */
#define MPIDI_REQUEST_TYPE_PUT_RECV_DERIVED_DT 10         /* target is receiving derived DT info for PUT data */
#define MPIDI_REQUEST_TYPE_GET_RECV_DERIVED_DT 11         /* target is receiving derived DT info for GET data */
#define MPIDI_REQUEST_TYPE_ACCUM_RECV_DERIVED_DT 12       /* target is receiving derived DT info for ACC data */
#define MPIDI_REQUEST_TYPE_GET_ACCUM_RECV 13              /* target is receiving GACC data */
#define MPIDI_REQUEST_TYPE_GET_ACCUM_RECV_DERIVED_DT 14   /* target is receiving derived DT info for GACC data */
#define MPIDI_REQUEST_TYPE_GET_ACCUM_RESP 15              /* target is sending GACC response data */
#define MPIDI_REQUEST_TYPE_FOP_RECV 16                    /* target is receiving FOP data */
#define MPIDI_REQUEST_TYPE_FOP_RESP 17                    /* target is sending FOP response data */

#define MPIDI_LOAD_RECV_IOV_ORIG_SEGMENT_FIRST_UNSET (-1)

/* TODO: these should all become static inline functions */
#define MPIDI_MVP_REQUEST_DEV_INIT(_rreq)                                      \
    MPIDI_MVP_REQUEST((_rreq), dev.datatype_ptr) = NULL;                       \
    MPIDI_MVP_REQUEST((_rreq), dev.state) = 0;                                 \
    MPIDI_MVP_REQUEST((_rreq), dev.cancel_pending) = FALSE;                    \
    MPIDI_MVP_REQUEST((_rreq), dev.target_win_handle) = MPI_WIN_NULL;          \
    MPIDI_MVP_REQUEST((_rreq), dev.source_win_handle) = MPI_WIN_NULL;          \
    MPIDI_MVP_REQUEST((_rreq), dev.target_lock_queue_entry) = NULL;            \
    MPIDI_MVP_REQUEST((_rreq), dev.iov_offset) = 0;                            \
    MPIDI_MVP_REQUEST((_rreq), dev.chunk_count) = 0;                           \
    MPIDI_MVP_REQUEST((_rreq), dev.resp_request_handle) = MPI_REQUEST_NULL;    \
    MPIDI_MVP_REQUEST((_rreq), dev.user_buf) = NULL;                           \
    MPIDI_MVP_REQUEST((_rreq), dev.OnDataAvail) = NULL;                        \
    MPIDI_MVP_REQUEST((_rreq), dev.OnFinal) = NULL;                            \
    MPIDI_MVP_REQUEST((_rreq), dev.datatype) = 0;                              \
    MPIDI_MVP_REQUEST((_rreq), dev.drop_data) = FALSE;                         \
    MPIDI_MVP_REQUEST((_rreq), dev.tmpbuf) = NULL;                             \
    MPIDI_MVP_REQUEST((_rreq), dev.ext_hdr_ptr) = NULL;                        \
    MPIDI_MVP_REQUEST((_rreq), dev.ext_hdr_sz) = 0;                            \
    MPIDI_MVP_REQUEST((_rreq), dev.rma_target_ptr) = NULL;                     \
    MPIDI_MVP_REQUEST((_rreq), dev.request_handle) = MPI_REQUEST_NULL;

#define MPIDI_MVP_REQUEST_INIT(_rreq)                                          \
    MPIDI_MVP_REQUEST((_rreq), dev.OnDataAvail) = NULL;                        \
    MPIDI_MVP_REQUEST((_rreq), dev.OnFinal) = NULL;                            \
    MPIDI_MVP_REQUEST((_rreq), mrail.rndv_buf_alloc) = 0;                      \
    MPIDI_MVP_REQUEST((_rreq), mrail.rndv_buf) = NULL;                         \
    MPIDI_MVP_REQUEST((_rreq), mrail.rndv_buf_sz) = 0;                         \
    MPIDI_MVP_REQUEST((_rreq), mrail.rndv_buf_off) = 0;                        \
    MPIDI_MVP_REQUEST((_rreq), mrail.protocol) = 0;                            \
    MPIDI_MVP_REQUEST((_rreq), mrail.d_entry) = NULL;                          \
    MPIDI_MVP_REQUEST((_rreq), mrail.remote_addr) = NULL;                      \
    MPIDI_MVP_REQUEST((_rreq), mrail.nearly_complete) = 0;                     \
    MPIDI_MVP_REQUEST((_rreq), mrail.local_complete) = 0;                      \
    MPIDI_MVP_REQUEST((_rreq), mrail.remote_complete) = 0;                     \
    MPIDI_MVP_REQUEST((_rreq), mrail.is_eager_vbuf_queued) = 0;                \
    MPIDI_MVP_REQUEST((_rreq), mrail.eager_vbuf_head) = NULL;                  \
    MPIDI_MVP_REQUEST((_rreq), mrail.eager_vbuf_tail) = NULL;                  \
    MPIDI_MVP_REQUEST((_rreq), mrail.eager_unexp_size) = 0;                    \
    MPIDI_MVP_REQUEST((_rreq), mrail.is_rma_last_stream_unit) = 1;

#define MPIDI_Request_create_sreq(sreq_, mpi_errno_, FAIL_)                    \
    {                                                                          \
        (sreq_) = MPIR_Request_create(MPIR_REQUEST_KIND__SEND);                \
        MPIDI_MVP_REQUEST_FROM_MPICH(sreq_) =                                  \
            MPIDI_MVP_smp_request_create_from_pool();                          \
        MPIDI_MVP_REQUEST_INIT(sreq_);                                         \
        MPIDI_MVP_REQUEST_DEV_INIT(sreq_);                                     \
        MPIR_Object_set_ref((sreq_), 2);                                       \
        (sreq_)->comm = comm;                                                  \
        MPIDI_MVP_REQUEST(sreq_, dev).partner_request = NULL;                  \
        MPIR_Comm_add_ref(comm);                                               \
        MPIDI_MVP_REQUEST(sreq_, dev).match.parts.rank = rank;                 \
        MPIDI_MVP_REQUEST(sreq_, dev).match.parts.tag = tag;                   \
        MPIDI_MVP_REQUEST(sreq_, dev).match.parts.context_id =                 \
            comm->context_id + context_offset;                                 \
        MPIDI_MVP_REQUEST(sreq_, dev).user_buf = (void *)buf;                  \
        MPIDI_MVP_REQUEST(sreq_, dev).user_count = count;                      \
        MPIDI_MVP_REQUEST(sreq_, dev).datatype = datatype;                     \
        MPIDI_MVP_REQUEST(sreq_, dev).iov_count = 0;                           \
        MPIDI_MVP_REQUEST(sreq_, dev).drop_data = FALSE;                       \
        MVP_INC_NUM_POSTED_SEND();                                             \
    }

/* This is the receive request version of MPIDI_Request_create_sreq */
#define MPIDI_Request_create_rreq(rreq_, mpi_errno_, FAIL_)                    \
    {                                                                          \
        if (!rreq_) {                                                          \
            (rreq_) = MPIR_Request_create(MPIR_REQUEST_KIND__RECV);            \
            MPIR_Object_set_ref((rreq_), 2);                                   \
        }                                                                      \
        MPIDI_MVP_REQUEST_FROM_MPICH(rreq_) =                                  \
            MPIDI_MVP_smp_request_create_from_pool();                          \
        MPIR_Object_set_ref(MPIDI_MVP_REQUEST_FROM_MPICH(rreq_), 1);           \
        MPIDI_MVP_REQUEST(rreq_, dev.partner_request) = NULL;                  \
        MPIDI_MVP_REQUEST(rreq_, dev.drop_data) = FALSE;                       \
        MPIDI_MVP_REQUEST_INIT(rreq_);                                         \
        MPIDI_MVP_REQUEST_DEV_INIT(rreq_);                                     \
    }

#define MPIDI_MVP_SMP_REQUEST_INIT(_rreq)                                      \
    _rreq->dev.datatype_ptr = NULL;                                            \
    _rreq->dev.state = 0;                                                      \
    _rreq->dev.cancel_pending = FALSE;                                         \
    _rreq->dev.target_win_handle = MPI_WIN_NULL;                               \
    _rreq->dev.source_win_handle = MPI_WIN_NULL;                               \
    _rreq->dev.target_lock_queue_entry = NULL;                                 \
    _rreq->dev.iov_offset = 0;                                                 \
    _rreq->dev.chunk_count = 0;                                                \
    _rreq->dev.resp_request_handle = MPI_REQUEST_NULL;                         \
    _rreq->dev.user_buf = NULL;                                                \
    _rreq->dev.OnDataAvail = NULL;                                             \
    _rreq->dev.OnFinal = NULL;                                                 \
    _rreq->dev.datatype = 0;                                                   \
    _rreq->dev.drop_data = FALSE;                                              \
    _rreq->dev.tmpbuf = NULL;                                                  \
    _rreq->dev.ext_hdr_ptr = NULL;                                             \
    _rreq->dev.ext_hdr_sz = 0;                                                 \
    _rreq->dev.rma_target_ptr = NULL;                                          \
    _rreq->dev.request_handle = MPI_REQUEST_NULL;                              \
    _rreq->dev.OnDataAvail = NULL;                                             \
    _rreq->dev.OnFinal = NULL;                                                 \
    _rreq->mrail.rndv_buf_alloc = 0;                                           \
    _rreq->mrail.rndv_buf = NULL;                                              \
    _rreq->mrail.rndv_buf_sz = 0;                                              \
    _rreq->mrail.rndv_buf_off = 0;                                             \
    _rreq->mrail.protocol = MRAILI_PROTOCOL_RENDEZVOUS_UNSPECIFIED;            \
    _rreq->mrail.d_entry = NULL;                                               \
    _rreq->mrail.remote_addr = NULL;                                           \
    _rreq->mrail.nearly_complete = 0;                                          \
    _rreq->mrail.local_complete = 0;                                           \
    _rreq->mrail.remote_complete = 0;                                          \
    _rreq->mrail.is_eager_vbuf_queued = 0;                                     \
    _rreq->mrail.eager_vbuf_head = NULL;                                       \
    _rreq->mrail.eager_vbuf_tail = NULL;                                       \
    _rreq->mrail.eager_unexp_size = 0;                                         \
    _rreq->mrail.is_rma_last_stream_unit = 1;

int MPIDI_MVP_Request_adjust_iov(MPIR_Request *greq, intptr_t nb);

static inline MPIDI_MVP_smp_request_t *MPIDI_MVP_smp_request_create_from_pool()
{
    MPIDI_MVP_smp_request_t *req = NULL;
    req = (MPIDI_MVP_smp_request_t *)MPIR_Handle_obj_alloc(
        &MPIDI_MVP_smp_request_mem);
    MPIDI_MVP_SMP_REQUEST_INIT(req);
    return req;
}

#define MPIDI_MVP_Request_decrement_cc(req_, incomplete_)   \
    MPIR_cc_decr((req_)->cc_ptr, incomplete_)
#define MPIDI_MVP_Request_increment_cc(req_, was_incomplete_)   \
    MPIR_cc_incr((req_)->cc_ptr, was_incomplete_)

/* TODO: add any other details for freeing our smp_request handles */
#define MPIDI_MVP_Request_free(_req)                                           \
    MPIR_Handle_obj_free(&MPIDI_MVP_smp_request_mem,                           \
                         MPIDI_MVP_REQUEST_FROM_MPICH(_req));                  \
    MPIR_Request_free(_req);

#define MPIDI_Request_set_seqnum(req_, seqnum_)                                \
    {                                                                          \
        MPIDI_MVP_REQUEST(req_, dev).seqnum = (seqnum_);                       \
    }

#define MPIDI_Request_set_msg_type(req_, msgtype_)                             \
    {                                                                          \
        MPIDI_MVP_REQUEST(req_, dev).state &= ~MPIDI_REQUEST_MSG_MASK;         \
        MPIDI_MVP_REQUEST(req_, dev).state |=                                  \
            ((msgtype_) << MPIDI_REQUEST_MSG_SHIFT) & MPIDI_REQUEST_MSG_MASK;  \
    }

/* the following two macros were formerly a single confusing macro with side
   effects named MPIDI_Request_recv_pending() */
#define MPIDI_Request_check_pending(req_, recv_pending_)                       \
    do {                                                                       \
        *(recv_pending_) = MPIDI_MVP_REQUEST(req_, dev).recv_pending_count;    \
    } while (0)

#define MPIDI_Request_decr_pending(req_)                                       \
    do {                                                                       \
        --(MPIDI_MVP_REQUEST(req_, dev).recv_pending_count);                   \
    } while (0)

#define MPIDI_Request_get_type(req_)                                           \
    ((MPIDI_MVP_REQUEST(req_, dev).state & MPIDI_REQUEST_TYPE_MASK) >>         \
     MPIDI_REQUEST_TYPE_SHIFT)

#define MPIDI_Request_get_sync_send_flag(req_)                                 \
    ((MPIDI_MVP_REQUEST(req_, dev).state & MPIDI_REQUEST_SYNC_SEND_MASK) >>    \
     MPIDI_REQUEST_SYNC_SEND_SHIFT)

#define MPIDI_Request_set_type(req_, type_)                                    \
    {                                                                          \
        MPIDI_MVP_REQUEST(req_, dev).state &= ~MPIDI_REQUEST_TYPE_MASK;        \
        MPIDI_MVP_REQUEST(req_, dev).state |=                                  \
            ((type_) << MPIDI_REQUEST_TYPE_SHIFT) & MPIDI_REQUEST_TYPE_MASK;   \
    }

/* NOTE: Request updates may require atomic ops (critical sections) if
   a fine-grain thread-sync model is used. */
#define MPIDI_Request_cancel_pending(req_, flag_)                              \
    {                                                                          \
        *(flag_) = MPIDI_MVP_REQUEST(req_, dev).cancel_pending;                \
        MPIDI_MVP_REQUEST(req_, dev).cancel_pending = TRUE;                    \
    }
#endif /* ifndef _MVP_REQ_H */
