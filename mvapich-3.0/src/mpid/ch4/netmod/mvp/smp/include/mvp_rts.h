
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

#ifndef _MVP_RTS_H_
#define _MVP_RTS_H_

#ifndef MVP_SMP_IMPL_INCLUDED
#error "Requres mvp_smp_impl.h, include that header first"
#endif

//#include "mpiimpl.h"
//#include "mvp_req_utils.h"
extern int mvp_smp_init;
//#include "../mvp_types.h"
//#include "mvp_vc_fields.h"
//#include "mvp_pkt.h"


#define IS_VC_SMP(_vc)  \
   (mvp_smp_init && (_vc)->smp.local_nodes >= 0)


int MPIDI_MVP_PktHandler_RndvReqToSend( MPIDI_MVP_ep_t *vc, MPIDI_MVP_Pkt_t *pkt, void *data ATTRIBUTE((unused)),
                    intptr_t *buflen, MPIR_Request **rreqp );


void mvp_select_rndv_protocol(
        MPIDI_MVP_ep_t *vc,
        MPIR_Request *rreq,
        MPIDI_MVP_Pkt_rndv_req_to_send_t * rts_pkt);

int MPIDI_MVP_Request_load_recv_iov(MPIR_Request * const rreq);

int MPIDI_MVP_ReqHandler_UnpackSRBufComplete(MPIDI_MVP_ep_t * vc, MPIR_Request * rreq, int *complete);


/* Max depth of recursive calls of MPID_Request_complete */
#define REQUEST_CB_DEPTH 2
#define MPIDI_LOAD_RECV_IOV_ORIG_MSG_OFFSET_UNSET (-1)

#if !defined(MPIDI_MVP_SRBuf_size)
#    define MPIDI_MVP_SRBuf_size (256 * 1024)
#endif

typedef struct __MPIDI_MVP_SRBuf_element {
    /* Keep the buffer at the top to help keep the memory alignment */
    char   buf[MPIDI_MVP_SRBuf_size];
    struct __MPIDI_MVP_SRBuf_element * next;
} MPIDI_MVP_SRBuf_element_t;


#define MPIDI_REQUEST_SRBUF_MASK (0x1 << MPIDI_REQUEST_SRBUF_SHIFT)
#define MPIDI_REQUEST_SRBUF_SHIFT 2

#define MPIDI_Request_get_srbuf_flag(req_)                                     \
    (((MPIDI_MVP_REQUEST(req_, dev)).state & MPIDI_REQUEST_SRBUF_MASK) >>      \
     MPIDI_REQUEST_SRBUF_SHIFT)

#define MPIDI_Request_set_srbuf_flag(req_, flag_)                              \
    {                                                                          \
        (MPIDI_MVP_REQUEST(req_, dev)).state &= ~MPIDI_REQUEST_SRBUF_MASK;     \
        (MPIDI_MVP_REQUEST(req_, dev)).state |=                                \
            ((flag_) << MPIDI_REQUEST_SRBUF_SHIFT) & MPIDI_REQUEST_SRBUF_MASK; \
    }

#if !defined (MPIDI_MVP_SRBuf_get)
#define MPIDI_MVP_SRBuf_get(req_)                                              \
    {                                                                          \
        MPIDI_MVP_SRBuf_element_t *tmp;                                        \
        if (!MPIDI_MVP_SRBuf_pool) {                                           \
            MPIDI_MVP_SRBuf_pool =                                             \
                MPL_malloc(sizeof(MPIDI_MVP_SRBuf_element_t), MPL_MEM_BUFFER); \
            MPIDI_MVP_SRBuf_pool->next = NULL;                                 \
        }                                                                      \
        tmp = MPIDI_MVP_SRBuf_pool;                                            \
        MPIDI_MVP_SRBuf_pool = MPIDI_MVP_SRBuf_pool->next;                     \
        tmp->next = NULL;                                                      \
        MPIDI_MVP_REQUEST(req_, dev).tmpbuf = tmp->buf;                        \
    }
#endif

#if !defined(MPIDI_MVP_SRBuf_alloc)
#define MPIDI_MVP_SRBuf_alloc(req_, size_)                                     \
    {                                                                          \
        MPIDI_MVP_SRBuf_get(req_);                                             \
        if (MPIDI_MVP_REQUEST(req_, dev).tmpbuf != NULL) {                     \
            MPIDI_MVP_REQUEST(req_, dev).tmpbuf_sz = MPIDI_MVP_SRBuf_size;     \
            MPIDI_Request_set_srbuf_flag((req_), TRUE);                        \
        } else {                                                               \
            MPIDI_MVP_REQUEST(req_, dev).tmpbuf_sz = 0;                        \
        }                                                                      \
    }
#endif

int MPIDI_MVP_MRAIL_Prepare_rndv(MPIDI_MVP_ep_t * vc, MPIR_Request * req);


#define MPIDI_MVP_MRAIL_FREE_RNDV_BUFFER(req)                  \
do {                                                            \
    if (1 == MPIDI_MVP_REQUEST(req, mrail.rndv_buf_alloc)       \
            && NULL != MPIDI_MVP_REQUEST(req, mrail).rndv_buf) {         \
        MPL_free(MPIDI_MVP_REQUEST(req, mrail.rndv_buf));       \
        MPIDI_MVP_REQUEST(req, mrail.rndv_buf_alloc) = 0;       \
        MPIDI_MVP_REQUEST(req, mrail.rndv_buf_off) = 0;         \
        MPIDI_MVP_REQUEST(req, mrail.rndv_buf_sz) = 0;          \
        MPIDI_MVP_REQUEST(req, mrail.rndv_buf) = NULL;          \
    } else {                                                    \
        MPIDI_MVP_REQUEST(req, mrail.rndv_buf_off) = 0;         \
        MPIDI_MVP_REQUEST(req, mrail.rndv_buf_sz) = 0;          \
    }                                                           \
}while(0)


#define MPIDI_MVP_R3_THRESHOLD(vc)                                              \
    (((vc)->smp.local_nodes >= 0 && mvp_smp_init) ?  rdma_intra_node_r3_threshold:  \
        rdma_inter_node_r3_threshold)


#define MPIDI_CH3I_MVP_SET_REQ_REMOTE_RNDV(_req,_pkt)         \
{                                                               \
    int _i;                                                     \
    (_req)->mrail.protocol = (_pkt)->rndv.protocol;             \
    if (  (MVP_RNDV_PROTOCOL_RPUT == (_pkt)->rndv.protocol) ||  \
          (MVP_RNDV_PROTOCOL_RGET == (_pkt)->rndv.protocol) ) {     \
        (_req)->mrail.remote_addr = (_pkt)->rndv.buf_addr;          \
        for (_i = 0; _i < rdma_num_hcas; _i ++)                     \
        (_req)->mrail.rkey[_i] = (_pkt)->rndv.rkey[_i];             \
    }                                                               \
}

int MPIDI_MVP_Rendezvous_push(MPIDI_MVP_ep_t * vc, MPIR_Request * sreq);

#define RENDEZVOUS_IN_PROGRESS(c, s)                                           \
    {                                                                          \
        MPIR_Request_add_ref(s);                                               \
        if (NULL == (c)->mrail.sreq_tail) {                                    \
            (c)->mrail.sreq_head = (void *)(s);                                \
        } else {                                                               \
            MPIDI_MVP_REQUEST(((MPIR_Request *)((c)->mrail.sreq_tail)), mrail) \
                .next_inflow = (void *)(s);                                    \
        }                                                                      \
        (c)->mrail.sreq_tail = (void *)(s);                                    \
        MPIDI_MVP_REQUEST(((MPIR_Request *)(s)), mrail).next_inflow = NULL;    \
    }

#define MPIDI_MVP_MRAIL_SET_REQ_REMOTE_RNDV(_req,_pkt)                  \
{                                                                       \
    int _i;                                                             \
    (_req)->mrail.protocol = (_pkt)->rndv.protocol;    \
    if ((MVP_RNDV_PROTOCOL_RPUT == (_pkt)->rndv.protocol) ||            \
        (MVP_RNDV_PROTOCOL_RGET == (_pkt)->rndv.protocol) ) {           \
        (_req)->mrail.remote_addr =                    \
            (_pkt)->rndv.buf_addr;                                      \
        for (_i = 0; _i < rdma_num_hcas; _i ++)                         \
        (_req)->mrail.rkey[_i] =                       \
            (_pkt)->rndv.rkey[_i];                                      \
    }                                                                   \
}


/* MVP optimized SMP RGET design */
int MPIDI_MVP_PktHandler_SMP_RTS( MPIDI_MVP_ep_t *vc, MPIDI_MVP_Pkt_t *pkt,
                    intptr_t *buflen, MPIR_Request **rreqp );

int MPIDI_MVP_Post_data_receive_found(MPIR_Request *rreq);


int MPIDI_MVP_Prepare_rndv_get(MPIDI_MVP_ep_t * vc,
                               MPIR_Request * rreq);


static int MPIDI_MVP_SMP_Rendezvous_push(MPIDI_MVP_ep_t * vc,
                                                MPIR_Request * sreq);

int MPIDI_MVP_Post_data_receive_found(MPIR_Request * rreq);


#define PUSH_FLOWLIST(c) {                                      \
    if (0 == c->mrail.inflow) {                                 \
        c->mrail.inflow = 1;                                    \
        c->mrail.nextflow = flowlist;                           \
        flowlist = c;                                           \
    }                                                           \
}

#define MPIDI_MVP_MRAIL_SET_PKT_RNDV(_pkt, _req)                               \
    {                                                                          \
        int _i;                                                                \
        /*MPIDI_MVP_request_t *_req2 = MPIDI_MVP_REQUEST_FROM_MPICH(_req); */  \
        (_pkt)->rndv.protocol = MPIDI_MVP_REQUEST(_req, mrail).protocol;       \
        if ((MVP_RNDV_PROTOCOL_RPUT == (_pkt)->rndv.protocol) ||               \
            (MVP_RNDV_PROTOCOL_RGET == (_pkt)->rndv.protocol)) {               \
            /*  if (!IS_DEVICE_RNDV_REQ(_req) && ((_req2)->mrail.d_entry)) {   \
                  for (_i = 0; _i < rdma_num_hcas; _i ++) {                    \
                      (_pkt)->rndv.rkey[_i] =                                  \
                      ((_req2)->mrail.d_entry)->memhandle[_i]->rkey;           \
                  }                                                            \
              }*/                                                              \
            (_pkt)->rndv.buf_addr = MPIDI_MVP_REQUEST(_req, mrail).rndv_buf;   \
        }                                                                      \
        (_pkt)->rndv.reqtype = MPIDI_Request_get_type(_req);                   \
    }

#define MPIDI_MVP_MRAIL_REVERT_RPUT(_sreq)                     \
{                                                               \
    if (MVP_RNDV_PROTOCOL_RGET == _sreq->mrail.protocol)        \
        _sreq->mrail.protocol = MVP_RNDV_PROTOCOL_RPUT;           \
}

int MPIDI_MVP_iStartMsg(MPIDI_MVP_ep_t * vc, void *pkt, intptr_t pkt_sz,
                        MPIR_Request ** sreq_ptr);

int MPIDI_MVP_SMP_iStartMsg(MPIDI_MVP_ep_t * vc, void *pkt,
                                          intptr_t pkt_sz,
                                          MPIR_Request ** sreq_ptr);




/*@
  MPIDI_CH3_Progress_signal_completion - Inform the progress engine that a 
  pending request has completed.

  IMPLEMENTORS:
  In a single-threaded environment, this routine can be implemented by
  incrementing a request completion counter.  In a
  multi-threaded environment, the request completion counter must be atomically
  incremented, and any threaded blocking in the
  progress engine must be woken up when a request is completed.

  Notes on the implementation:

  This code is designed to support one particular model of thread-safety.
  It is common to many of the channels and was moved into this file because
  the MPIDI_CH3_Progress_signal_completion reference is used by the 
  function the implements MPID_Request_complete.
@*/

/*
 * MPIDI_CH3_Progress_signal_completion() is used to notify the progress
 * engine that a completion has occurred.  The multi-threaded version will need
 * to wake up any (and all) threads blocking in MPIDI_CH3_Progress().
 */

/* TODO: FIXME: DUMMY VAR for progress_completion count*/
//extern MPIDI_MVP_progress_completion_count = 0;
extern volatile unsigned int MPIDI_MVP_progress_completion_count;

/* This allows the channel to define an alternate to the 
   completion counter.  */
#ifndef MPIDI_MVP_INCR_PROGRESS_COMPLETION_COUNT
#define MPIDI_MVP_INCR_PROGRESS_COMPLETION_COUNT                                \
    do {                                                                         \
        MPID_THREAD_CS_ENTER(POBJ, MPIR_THREAD_POBJ_COMPLETION_MUTEX);                                       \
        ++MPIDI_MVP_progress_completion_count;                                  \
/*        MPL_DBG_MSG_D(MPIDI_CH3_DBG_PROGRESS,VERBOSE,                                     \
                     "just incremented MPIDI_CH3I_progress_completion_count=%d", \
                     MPIDI_CH3I_progress_completion_count);                     */ \
        MPID_THREAD_CS_EXIT(POBJ, MPIR_THREAD_POBJ_COMPLETION_MUTEX);                                        \
    } while (0)
#endif


/* The following is part of an implementation of a control of a 
   resource shared among threads - it needs to be managed more 
   explicitly as such as shared resource */
#ifndef MPICH_IS_THREADED
#   define MPIDI_MVP_Progress_signal_completion()	\
    {							\
       MPIDI_MVP_INCR_PROGRESS_COMPLETION_COUNT;		\
    }
#else
    /* TODO these decls should probably move into each channel as appropriate */
    extern volatile int MPIDI_MVP_progress_blocked;
    extern volatile int MPIDI_MVP_progress_wakeup_signalled;

/* This allows the channel to hook the MPIDI_CH3_Progress_signal_completion
 * macro when it is necessary to wake up some part of the progress engine from a
 * blocking operation.  Currently ch3:sock uses it, ch3:nemesis does not. */
/* MT alternative implementations of this macro are responsible for providing any
 * synchronization (acquiring MPIDCOMM, etc) */
#ifndef MPIDI_MVP_PROGRESS_WAKEUP
# define MPIDI_MVP_PROGRESS_WAKEUP do {/*do nothing*/} while(0)
#endif

    void MPIDI_CH3I_Progress_wakeup(void);
    /* MT TODO profiling is needed here.  We currently protect the completion
     * counter with the COMPLETION critical section, which could be a source of
     * contention.  It should be possible to perform these updates atomically via
     * OPA instead, but the additional complexity should be justified by
     * profiling evidence.  [goodell@ 2010-06-29] */
#   define MPIDI_MVP_Progress_signal_completion()			\
    do {                                                                \
        MPIDI_MVP_INCR_PROGRESS_COMPLETION_COUNT;                      \
        MPIDI_MVP_PROGRESS_WAKEUP;                                     \
    } while (0)
#endif /* #ifndef MPICH_IS_THREADED */


#define MPIDI_MVP_Append_pkt_size() \
*buflen += sizeof(MPIDI_MVP_Pkt_t);

int MPIDI_MVP_Prepare_rndv_cts(MPIDI_MVP_ep_t * vc,
                               MPIDI_MVP_Pkt_rndv_clr_to_send_t * cts_pkt,
                               MPIR_Request * rreq);

int MPIDI_MVP_MRAIL_Prepare_rndv_transfer(MPIR_Request * sreq, 
        /* contains local info */
        MPIDI_MVP_MRAILI_Rndv_info_t *rndv);

int MPIDI_MVP_Rndv_transfer(MPIDI_MVP_ep_t * vc,
        MPIR_Request * sreq, MPIR_Request * rreq,
        MPIDI_MVP_Pkt_rndv_clr_to_send_t * cts_pkt,
        MPIDI_MVP_Pkt_rndv_req_to_send_t * rts_pkt);

int MPIDI_MVP_PktHandler_SMP_CTS( MPIDI_MVP_ep_t *vc, MPIDI_MVP_Pkt_t *pkt,
                    intptr_t *buflen, MPIR_Request **rreqp );

#ifdef MPICH_DBG_OUTPUT
int MPIDI_MVP_PktPrint_RndvClrToSend( FILE *fp, MPIDI_MVP_Pkt_t *pkt );
#endif

#endif /* _ifndef _MVP_RTS_H_*/


