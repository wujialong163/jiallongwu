/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda. For detailed
 * copyright and licensing information, please refer to the copyright file
 * COPYRIGHT in the top level MVAPICH directory.
 */

#ifndef _MVP_REQ_FIELDS_H_
#define _MVP_REQ_FIELDS_H_

#ifndef MVP_SMP_IMPL_INCLUDED
#error "Requres mvp_smp_impl.h, include that header first"
#endif

#if defined(HAVE_SYS_TYPES_H)
#include <sys/types.h>
#endif

#include "mvp_vc.h"
#include "mvp_pkt.h"

#define MPIDI_REQUEST_MSG_MASK (0x3 << MPIDI_REQUEST_MSG_SHIFT)
#define MPIDI_REQUEST_MSG_SHIFT 0
#define MPIDI_REQUEST_NO_MSG 0
#define MPIDI_REQUEST_EAGER_MSG 1
#define MPIDI_REQUEST_RNDV_MSG 2
#define MPIDI_REQUEST_SELF_MSG 3

#define MPIDI_Request_get_msg_type(req_)                                       \
    ((MPIDI_MVP_REQUEST(req_, dev).state & MPIDI_REQUEST_MSG_MASK) >>          \
     MPIDI_REQUEST_MSG_SHIFT)

/* Placeholder for MPIDI_Rank_t (FIXME) (TODO) */
//typedef int32_t MPIDI_Rank_t;
/* need a better way of handling this */
typedef unsigned long MPID_Seqnum_t;


/* DUMMY BIT FOR MPIDI_REQUEST_SEQNUM (FIXME) (TODO) */
#   define MPIDI_REQUEST_SEQNUM	\
        MPID_Seqnum_t seqnum;

/*
 * doing this here in case we need to add to it.
 * For ch4 we set the status MPI_SOURCE to -1 so we can tell if this was
 * handled by the channel or not
 */
#ifndef MPIDI_POSTED_RECV_ENQUEUE_HOOK
#define MPIDI_POSTED_RECV_ENQUEUE_HOOK(_req) (_req)->status.MPI_SOURCE = -1
#endif
/* MPIDI_POSTED_RECV_DEQUEUE_HOOK(req): Notifies channel that req has
   been dequeued from the posted recv queue.  Returns non-zero if the
   channel has already matched the request; 0 otherwise.  This happens
   when the channel supports shared-memory and network communication
   with a network capable of matching, and the same request is matched
   by the network and, e.g., shared-memory.  When that happens the
   dequeue functions below should, either search for the next matching
   request, or report that no request was found. */
/* in ch4 we verify that our status field has a valid value for source
 * since we do network progress first */
#ifndef MPIDI_POSTED_RECV_DEQUEUE_HOOK
#define MPIDI_POSTED_RECV_DEQUEUE_HOOK(_req) (_req)->status.MPI_SOURCE > -1
#endif

struct __attribute__((__aligned__(64))) MPIDI_MVP_MRAILI_Request {
    void * rndv_buf;                                              
    void     *remote_addr;                                        
    intptr_t rndv_buf_sz;                                         
    intptr_t rndv_buf_off;                                        
    struct MPID_Request *next_inflow;                             
    struct vbuf *eager_vbuf_head;                                 
    struct vbuf *eager_vbuf_tail;                                 
    struct dreg_entry *d_entry;                                   
    intptr_t eager_unexp_size;                                    
    uint32_t rkey[MAX_NUM_HCAS];                                  
    MPI_Request partner_id;                                       
    MRAILI_Protocol_t protocol;                                   
    uint32_t  local_complete;                                     
    uint32_t  remote_complete;                                    
    uint32_t  num_rdma_read_completions;                          
    uint8_t  nearly_complete;                                     
    uint8_t is_rma_last_stream_unit;                              
    uint8_t rndv_buf_alloc;                                       
    uint8_t  is_eager_vbuf_queued;
} /*mrail*/;

enum REQ_TYPE {
    REQUEST_NORMAL,
    REQUEST_RNDV_R3_HEADER,
    REQUEST_RNDV_R3_DATA,
    REQUEST_LIGHT
};


struct __attribute__((__aligned__(64))) MPIDI_CH3I_Request {
    /*  pkt is used to temporarily store a packet header associated
       with this request */
    MPIDI_MVP_Pkt_t pkt;
    enum REQ_TYPE   reqtype;
    /* For CKPT, hard to put in ifdef because it's in macro define*/
    struct MPIR_Request *cr_queue_next;
    MPIDI_MVP_ep_h vc;
} /*ch*/;

typedef struct __attribute__((__aligned__(64))) MPIDI_Request {
    MPIDI_Message_match match;
    MPIDI_Message_match mask;

    /* user_buf, user_count, and datatype needed to process 
       rendezvous messages. */
    void        *user_buf;
    MPI_Aint   user_count;
    MPI_Datatype datatype;
    int drop_data;

    /* msg_offset, and msgsize are used when processing
       non-contiguous datatypes */
    intptr_t msg_offset;
    intptr_t msgsize;
    intptr_t orig_msg_offset;

    /* Pointer to datatype for reference counting purposes */
    struct MPIR_Datatype* datatype_ptr;

    /* iov and iov_count define the data to be transferred/received.  
       iov_offset points to the current head element in the IOV */
    struct iovec iov[MPL_IOV_LIMIT];
    int iov_count;
    size_t iov_offset;

    /* In case of chunked Send, how many chunks have been sent */
    int chunk_count;

    /* OnDataAvail is the action to take when data is now available.
       For example, when an operation described by an iov has 
       completed.  This replaces the MPIDI_CA_t (completion action)
       field used through MPICH 1.0.4. */
    int (*OnDataAvail)(struct MPIR_Request *, int *);
    /* OnFinal is used in the following case:
       OnDataAvail is set to a function, and that function has processed
       all of the data.  At that point, the OnDataAvail function can
       reset OnDataAvail to OnFinal.  This is normally used when processing
       non-contiguous data, where there is one more action to take (such
       as a get-response) when processing of the non-contiguous data 
       completes. This value need not be initialized unless OnDataAvail
       is set to a non-null value (and then only in certain cases) */
    int (*OnFinal)(struct MPIR_Request *, int *);

    /* tmpbuf and tmpbuf_sz describe temporary storage used for things like 
       unexpected eager messages and packing/unpacking
       buffers.  tmpuf_off is the current offset into the temporary buffer. */
    void          *tmpbuf;
    intptr_t tmpbuf_off;
    intptr_t tmpbuf_sz;

    intptr_t recv_data_sz;
    MPI_Request    sender_req_id;

    unsigned int   state;
    int            cancel_pending;
    MPIDI_MVP_Pkt_t pending_pkt;
    /* This field seems to be used for unexpected messages.  Unexpected messages
     * need to go through two steps: matching and receiving the data.  These
     * steps could happen in either order though, so this field is initialized
     * to 2.  It is decremented when the request is matched and also when all of
     * the data is available.  Once it reaches 0 it should be safe to copy from
     * the temporary buffer (if there is one) to the user buffer.  This field is
     * related to, but not quite the same thing as the completion counter (cc). */
    /* MT access should be controlled by the MSGQUEUE CS when the req is still
     * unexpected, exclusive access otherwise */
    int            recv_pending_count;

    /* The next several fields are used to hold state for ongoing RMA operations */
    MPI_Op op;
    /* For accumulate, since data is first read into a tmp_buf */
    void *real_user_buf;
    /* For derived datatypes at target. */
    void *flattened_type;
    /* req. handle needed to implement derived datatype gets.
     * It also used for remembering user request of request-based RMA operations. */
    MPI_Request request_handle;
    MPI_Win     target_win_handle;
    MPI_Win     source_win_handle;
    int pkt_flags; /* pkt_flags that were included in the original RMA packet header */
    struct MPIDI_RMA_Target_lock_entry *target_lock_queue_entry;
    MPI_Request resp_request_handle; /* Handle for get_accumulate response */

    void *ext_hdr_ptr; /* Pointer to extended packet header.
                        * It is allocated in RMA issuing/pkt_handler functions,
                        * and freed when release request. */
    intptr_t ext_hdr_sz;

    struct MPIDI_RMA_Target *rma_target_ptr;

    MPIDI_REQUEST_SEQNUM

    /* Occasionally, when a message cannot be sent, we need to cache the
       data that is required.  The fields above (such as userbuf and tmpbuf)
       are used for the message data.  However, we also need space for the
       message packet. This field provide a generic location for that.
       Question: do we want to make this a link instead of reserving 
       a fixed spot in the request? */

    /* (TODO) (FIXME) Create a type for this */
/*    MPIDI_MVP_Pkt_t pending_pkt; */


    /* Notes about request_completed_cb:
     *
     *   1. The callback function is triggered when this requests
     *      completion count reaches 0.
     *
     *   2. The callback function should be nonblocking.
     *
     *   3. The callback function should not poke the progress engine,
     *      or call any function that pokes the progress engine.
     *
     *   4. The callback function can complete other requests, thus
     *      calling those requests' callback functions.  However, the
     *      recursion depth of request completion function is limited.
     *      If we ever need deeper recurisve calls, we need to change
     *      to an iterative design instead of a recursive design for
     *      request completion.
     *
     *   5. In multithreaded programs, since the callback function is
     *      nonblocking and never calls the progress engine, it would
     *      never yield the lock to other threads.  So the recursion
     *      should be multithreading-safe.
     */
    int (*request_completed_cb)(struct MPIR_Request *);

    /* partner send request when a receive request is created by the
     * sender (only used for self send) */
    struct MPIR_Request * partner_request;

    struct MPIR_Request * next;
} MPIDI_Request;

/* TODO : Refactor these internal structs. */
#define MPIDI_MVP_LEGACY_REQUEST_DECL       \
    MPIDI_Request dev;                      \
    struct MPIDI_CH3I_Request ch;           \
    struct MPIDI_MVP_MRAILI_Request mrail;

typedef struct MPIDI_MVP_smp_request {
    MPIR_OBJECT_HEADER;
    MPIDI_Request dev;
    struct MPIDI_CH3I_Request ch;
    struct MPIDI_MVP_MRAILI_Request mrail;
} MPIDI_MVP_smp_request_t;

/* MATCH_WITH_LEFT_MASK compares the match values after masking only
 * the left field. This is useful for the case where the right match
 * is a part of the unexpected queue and has no ANY_TAG or ANY_SOURCE
 * wild cards, but the left match might have them. */
#define MATCH_WITH_LEFT_MASK(match1, match2, mask)                      \
    ((sizeof(MPIDI_Message_match) == SIZEOF_VOID_P) ?                   \
     (((match1).whole & (mask).whole) == (match2).whole) :              \
     ((((match1).parts.rank & (mask).parts.rank) == (match2).parts.rank) && \
      (((match1).parts.tag & (mask).parts.tag) == (match2).parts.tag) && \
      ((match1).parts.context_id == (match2).parts.context_id)))

/* This is the most general case where both matches have to be
 * masked. Both matches are masked with the same value. There doesn't
 * seem to be a need for two different masks at this time. */
#define MATCH_WITH_LEFT_RIGHT_MASK(match1, match2, mask)                \
    ((sizeof(MPIDI_Message_match) == SIZEOF_VOID_P) ?                   \
     (((match1).whole & (mask).whole) == ((match2).whole & (mask).whole)) : \
     ((((match1).parts.rank & (mask).parts.rank) == ((match2).parts.rank & (mask).parts.rank)) && \
      (((match1).parts.tag & (mask).parts.tag) == ((match2).parts.tag & (mask).parts.tag)) && \
      ((match1).parts.context_id == (match2).parts.context_id)))


#endif /* ifnded _MVP_REQ_H */
