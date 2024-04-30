/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda. For detailed
 * copyright and licensing information, please refer to the copyright file
 * COPYRIGHT in the top level MVAPICH directory.
 */

#ifndef _MVP_VC_H_
#define _MVP_VC_H_

#ifndef MVP_SMP_IMPL_INCLUDED
#error "Requres mvp_smp_impl.h, include that header first"
#endif

#include "mpir_objects.h"
#include <stdint.h>

typedef unsigned long MPID_Seqnum_t;

/*--------------------------------
  BEGIN VIRTUAL CONNECTION SECTION
  --------------------------------*/
/*E
  MPIDI_VC_State - States for a virtual connection.
 
  Notes:
  A closed connection is placed into 'STATE_INACTIVE'. (is this true?)
 E*/
typedef enum MPIDI_MVP_ep_state {
    MPIDI_MVP_EP_STATE_INACTIVE = 1,    /* Comm either hasn't started or has completed. */
    MPIDI_MVP_EP_STATE_LOCAL_ACTIVE,    /* Local process has initiated connection, but remote process has not ack'ed */
    MPIDI_MVP_EP_STATE_ACTIVE,          /* Comm has started and hasn't completed */
    MPIDI_MVP_EP_STATE_LOCAL_CLOSE,     /* Local side has initiated close protocol */
    MPIDI_MVP_EP_STATE_REMOTE_CLOSE,    /* Remote side has initiated close protocol */
    MPIDI_MVP_EP_STATE_CLOSE_ACKED,     /* Both have initiated close, we have acknowledged remote side */
    MPIDI_MVP_EP_STATE_CLOSED,          /* Both have initiated close, both have acked */
    MPIDI_MVP_EP_STATE_INACTIVE_CLOSED, /* INACTIVE MVP_EPs are moved to this state in Finalize */
    MPIDI_MVP_EP_STATE_MORIBUND         /* Abnormally terminated, there may be unsent/unreceived msgs */
} MPIDI_MVP_ep_state_t;

typedef enum MPIDI_MVP_vc_state
{
    MPIDI_MVP_VC_STATE_INVALID,
    MPIDI_MVP_VC_STATE_UNCONNECTED,
    MPIDI_MVP_VC_STATE_CONNECTING_CLI,
    MPIDI_MVP_VC_STATE_CONNECTING_SRV,
#ifdef CKPT
    MPIDI_MVP_VC_STATE_SUSPENDING,
    MPIDI_MVP_VC_STATE_SUSPENDED,
    MPIDI_MVP_VC_STATE_REACTIVATING_CLI_1,
    MPIDI_MVP_VC_STATE_REACTIVATING_CLI_2,
    MPIDI_MVP_VC_STATE_REACTIVATING_SRV,
#endif
#ifdef RDMA_CM
    MPIDI_MVP_VC_STATE_IWARP_SRV_WAITING,
    MPIDI_MVP_VC_STATE_IWARP_CLI_WAITING,
#endif 
    MPIDI_MVP_VC_STATE_IDLE,
    MPIDI_MVP_VC_STATE_FAILED
} MPIDI_MVP_vc_state_t;

/* This structure requires the iovec structure macros to be defined */
typedef struct MPIDI_MVP_Buffer_t {
    int use_iov;
    unsigned int num_bytes;
    void *buffer;
    unsigned int bufflen;
    struct iovec *iov;
    int iovlen;
    int index;
    int total;
} MPIDI_MVP_Buffer_t;

typedef struct MPIDI_MVP_vc {
    struct MPIR_Request * sendq_head;
    struct MPIR_Request * sendq_tail;
    struct MPIR_Request * send_active;
    struct MPIR_Request * recv_active;
    struct MPIR_Request * req;
    volatile MPIDI_MVP_vc_state_t state;
    MPIDI_MVP_Buffer_t read;
    int read_state;
    int port_name_tag;
    /* Connection management */
    struct MPIR_Request * cm_sendq_head;
    struct MPIR_Request * cm_sendq_tail;
    struct vbuf         * cm_1sc_sendq_head;
    struct vbuf         * cm_1sc_sendq_tail;
#ifdef CKPT
    volatile int rput_stop; /*Stop rput message and wait for rkey update*/
#endif
#ifdef _ENABLE_XRC_
    uint32_t                    xrc_flags;
    struct MPIDI_VC             *orig_vc;
    struct _xrc_pending_conn    *xrc_conn_queue;
    uint32_t                    xrc_srqn[MAX_NUM_HCAS];
    uint32_t                    xrc_rqpn[MAX_NUM_SUBRAILS];
    uint32_t                    xrc_my_rqpn[MAX_NUM_SUBRAILS];
#endif
    intptr_t              pending_r3_data;
    intptr_t              received_r3_data;
} MPIDI_MVP_vc_t;

/* SMP Channel is added by OSU-MPI2 */
typedef enum SMP_pkt_type {
    SMP_EAGER_MSG,
    SMP_RNDV_MSG,
    SMP_RNDV_MSG_CONT
} SMP_pkt_type_t;

typedef struct MPIDI_MVP_SMP_VC {
    struct MPIR_Request * sendq_head;
    struct MPIR_Request * sendq_tail;
    struct MPIR_Request * send_active;
    struct MPIR_Request * recv_active;
    int local_nodes;
    int local_rank;
    SMP_pkt_type_t send_current_pkt_type;
    SMP_pkt_type_t recv_current_pkt_type;
    int hostid;
    int read_index;
    int read_off;

#if defined(_SMP_LIMIC_)
    struct limic_header current_l_header;
#else
    int current_l_header;
#endif 

#if defined(_SMP_CMA_)
    struct cma_header current_c_header;
#else
    int current_c_header;
#endif

    int current_cnb;
    int current_nb;
    int use_limic;
    int use_cma;
} MPIDI_MVP_SMP_VC;

#ifndef MPIDI_MVP_VC_RDMA_DECL
#define MPIDI_MVP_VC_RDMA_DECL
#endif

/* Why is this defined here? 
#define MPIDI_MVP_VC_DECL \
MPIDI_MVP_vc_t ch; \
MPIDI_MVP_SMP_VC smp; \
MPIDI_MVP_VC_RDMA_DECL
*/

/* sample implementation structure */
typedef struct MPIDI_MVP_MRAIL_VC_t {
    int     	num_rails;
    /* qp handle for each of the sub-rails */
    //struct  	mrail_rail *rails;

    //uint16_t        lid[MAX_NUM_HCAS][MAX_NUM_PORTS];
    //union ibv_gid   gid[MAX_NUM_HCAS][MAX_NUM_PORTS];

    uint16_t    state;
    
    /* number of send wqes available */
    uint16_t    seqnum_next_tosend;
    uint16_t    seqnum_next_torecv;
    uint16_t    seqnum_last_recv;
    uint16_t    seqnum_next_toack;
    uint16_t    ack_need_tosend;

    /* how many eager sends do we have outstanding */
    //int outstanding_eager_vbufs;

    /* what buffer are we currently packing */
    //struct vbuf *coalesce_vbuf;

    //MPIDI_CH3I_MRAILI_RDMAPATH_VC 	rfp;
    //MPIDI_CH3I_MRAILI_SR_VC 		srp;
#ifdef _ENABLE_UD_ 
    //MPIDI_CH3I_MRAILI_UD_VC         *ud;
    //MPIDI_CH3I_MRAILI_UD_REL        rely;
#endif /* _ENABLE_UD_ */

    //MRAILI_Channel_manager  cmanager;
    /* Buffered receiving request for packetized transfer */
    void                    *packetized_recv;

    /* these fields are used to remember data transfer operations
     * that are currently in progress on this connection. The
     * send handle list is a queue of send handles representing
     * in-progress rendezvous transfers. It is processed in FIFO
     * order (because of MPI ordering rules) so there is both a head
     * and a tail.
     *
     * The receive handle is a pointer to a single
     * in-progress eager receive. We require that an eager sender
     * send *all* packets associated with an eager receive before
     * sending any others, so when we receive the first packet of
     * an eager series, we remember it by caching the rhandle
     * on the connection.
     *
     */
    void    *sreq_head; /* "queue" of send handles to process */
    void    *sreq_tail;
    /* these two fields are used *only* by MPID_DeviceCheck to
     * build up a list of connections that have received new
     * flow control credit so that pending operations should be
     * pushed. nextflow is a pointer to the next connection on the
     * list, and inflow is 1 (true) or 0 (false) to indicate whether
     * the connection is currently on the flowlist. This is needed
     * to prevent a circular list.
     */

    void    *nextflow;
    int     inflow;
    /* used to distinguish which VIA barrier synchronozations have
     * completed on this connection.  Currently, only used during
     * process teardown.
     *
     int barrier_id; */

    uint64_t remote_vc_addr; /* Used to find vc at remote side */
} MPIDI_MVP_MRAIL_VC;

#define MPIDI_MVP_VC_DECL \
MPIDI_MVP_vc_t ch; \
MPIDI_MVP_SMP_VC smp; \
MPIDI_MVP_MRAIL_VC mrail;

/* TODO: FIXME: add mvp_vc.h */
typedef struct MPIDI_MVP_ep {

   /* XXX - need better comment */
    /* MPIR_Object fields.  MPIDI_VC_t objects are not allocated using the 
       MPIR_Object system, but we do use the associated
       reference counting routines.  The handle value is required 
       when debugging objects (the handle kind is used in reporting
       on changes to the object).
    */
    MPIR_OBJECT_HEADER; /* adds handle and ref_count fields */

    /* state of the VC */
    MPIDI_MVP_ep_state_t state;

    /* Process group to which this VC belongs */
//    struct MPIDI_PG * pg;

    /* Rank of the process in that process group associated with this VC */
    int pg_rank;

    /* Local process ID */
    int lpid;

    /* The node id of this process, used for topologically aware collectives. */
    int node_id;

    /* port name tag */ 
    int port_name_tag; /* added to handle dynamic process mgmt */

    /* Sequence number of the next packet to be sent */
    uint16_t seqnum_send;
    
    uint32_t rma_issued;


    /* rendezvous function pointers.  Called to send a rendezvous
       message or when one is matched */
    int (* rndvSend_fn)( struct MPIR_Request **sreq_p, const void * buf, MPI_Aint count,
                         MPI_Datatype datatype, int dt_contig, intptr_t data_sz,
                         MPI_Aint dt_true_lb, int rank, int tag,
                         struct MPIR_Comm * comm, int context_offset );
    int (* rndvRecv_fn)( struct MPIDI_MVP_ep * vc, struct MPIR_Request *rreq );

    /* MVP use static inline eager fast send functions */
    int use_eager_fast_fn;
    int use_eager_fast_rfp_fn;
    int use_smp_eager_fast_fn;

    /* eager message threshold */
    int eager_fast_max_msg_sz;
    /* eager message threshold for ready sends.  -1 means there's no limit */
    int ready_eager_max_msg_sz;
 

    int force_rndv;
    unsigned char tmp_dpmvc;
    unsigned char free_vc;
    unsigned char disconnect;
    int pending_close_ops;
 
#ifdef ENABLE_COMM_OVERRIDES
    MPIDI_Comm_ops_t *comm_ops;
#endif

#if defined(MPIDI_MVP_VC_DECL)
    MPIDI_MVP_VC_DECL
#endif
} MPIDI_MVP_ep_t;

/* list of all VCs on this process */
/* TODO: make this a little more elegant */
extern MPIDI_MVP_ep_t **MPIDI_MVP_vc_table;
//#define MPIDI_Comm_get_vc(comm_, rank_, vcp_) *(vcp_) = (comm_)->dev.vcrt->vcr_table[(rank_)]

#endif /* _MVP_VC_H_ */
