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

#ifndef MPIDPKT_H_INCLUDED
#define MPIDPKT_H_INCLUDED

#ifndef MVP_SMP_IMPL_INCLUDED
#error "Requres mvp_smp_impl.h, include that header first"
#endif

#include "mpir_op_util.h"
#include "mvp_vc.h"
#define MPIDI_EAGER_SHORT_SIZE 16
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#ifdef CRC_CHECK
#define VC_CRC_INFO  unsigned long crc;
#else
#define VC_CRC_INFO
#endif

#define VC_SRC_INFO \
    union {                     \
        uint32_t smp_index;     \
        uint32_t rank;          \
        uint64_t vc_addr;       \
    } src;

/* TODO: par down */
#define MPIDI_MVP_MRAILI_IBA_PKT_DECL \
    uint16_t seqnum;            \
    uint16_t acknum;            \
    uint8_t  remote_credit;     \
    uint8_t  rdma_credit;       \
    VC_SRC_INFO                 \
    VC_CRC_INFO                 \
    uint8_t  vbuf_credit;       \
    uint8_t  rail;

#define MPIDI_Pkt_init(pkt_, type_)              \
{                               \
    (pkt_)->type = (type_);                 \
}

#   define MPIDI_VC_FAI_send_seqnum(vc_, seqnum_out_)	\
    {							\
	(seqnum_out_) = (vc_)->seqnum_send++;		\
    }
#   define MPIDI_Pkt_set_seqnum(pkt_, seqnum_)	\
    {						\
    	(pkt_)->seqnum = (seqnum_);		\
    }
#   define MPIDI_VC_Init_seqnum_send(vc_)	\
    {						\
    	(vc_)->seqnum_send = 0;			\
    }
#define MPIDI_VC_revoke_seqnum_send(vc_, seqnum_)    \
{                                                    \
    MPIR_Assert((((seqnum_) + 1)%(UINT16_MAX+1)) == (vc_)->seqnum_send); \
    (vc_)->seqnum_send--;                            \
}


/*
 * These Macros or structs may need to re-consider the location to place them
 */
typedef enum {
    MRAILI_PROTOCOL_RENDEZVOUS_UNSPECIFIED = 0,
    MRAILI_PROTOCOL_EAGER,
    MRAILI_PROTOCOL_R3,
    MRAILI_PROTOCOL_RPUT,
    MRAILI_PROTOCOL_RGET,
    MRAILI_PROTOCOL_UD_ZCOPY,
    MRAILI_PROTOCOL_CUDAIPC
} MRAILI_Protocol_t;

#define MAX_NUM_HCAS (10)

#if CH4_RANK_BITS == 16
typedef int16_t MPIDI_Rank_t;
#elif CH4_RANK_BITS == 32
typedef int32_t MPIDI_Rank_t;
#endif /* CH4_RANK_BITS */

/* For the typical communication system for which the ch3 channel is
   appropriate, 16 bits is sufficient for the rank.  By also using 16
   bits for the context, we can reduce the size of the match
   information, which is beneficial for slower communication
   links. Further, this allows the total structure size to be 64 bits
   and the search operations can be optimized on 64-bit platforms. We
   use a union of the actual required structure with a uintptr_t, so
   in this optimized case, the "whole" field can be used for
   comparisons.

   Note that the MPICH code (in src/mpi) uses int for rank (and usually for
   contextids, though some work is needed there).

   Note:  We need to check for truncation of rank in MPID_Init - it should
   confirm that the size of comm_world is less than 2^15, and in an communicator
   create (that may make use of dynamically created processes) that the
   size of the communicator is within range.

   If any part of the definition of this type is changed, those changes
   must be reflected in the debugger interface in src/mpi/debugger/dll_mpich.c
   and dbgstub.c
*/
typedef struct MPIDI_Message_match_parts {
    int32_t tag;
    MPIDI_Rank_t rank;
    MPIR_Context_id_t context_id;
} MPIDI_Message_match_parts_t;

typedef union {
    MPIDI_Message_match_parts_t parts;
    uintptr_t whole;
} MPIDI_Message_match;

typedef struct MPIDI_MVP_MRAILI_Rndv_info {
    /* Buffer Address */
    void                *buf_addr;
    /* rkey for RDMA for all HCAs */
    uint32_t            rkey[MAX_NUM_HCAS];
    /* Protocol to be used, Choices: R3/RPUT/RGET */
    MRAILI_Protocol_t   protocol;
    /* This is required for telling the receiver
     * when to mark the recv as complete */
    uint8_t             weight_rail[MAX_NUM_HCAS];
    uint8_t             reqtype;
#ifdef _ENABLE_UD_
    uint32_t            rndv_qpn[MAX_NUM_HCAS];
#endif
} MPIDI_MVP_MRAILI_Rndv_info_t;

#define MPIDI_MVP_MRAILI_RNDV_INFO_DECL \
    MPIDI_MVP_MRAILI_Rndv_info_t rndv;

/* Enable the use of data within the message packet for small messages */
#if defined(NEMESIS_BUILD)
#define USE_EAGER_SHORT
#define MPIDI_EAGER_SHORT_INTS 4
/* FIXME: This appears to assume that sizeof(int) == 4 (or at least >= 4) */
#define MPIDI_EAGER_SHORT_SIZE 16
#endif

/* This is the number of ints that can be carried within an RMA packet */
#define MPIDI_RMA_IMMED_BYTES 8

/* Union for immediate data in RMA packet headers.*/
typedef union {
    char payload[MPIDI_RMA_IMMED_BYTES];
#ifdef NEEDS_STRICT_ALIGNMENT
    /* Because the data is accessed per predefined type in the packet handler
     * of accumulate-like operations, we need extra union members to ensure
     * aligned access.
     * NOTE: this fix might increase the packet size (long double complex
     * can be 32bytes), thus we only enable this fix for a few platforms which
     * are alignment-sensitive.*/
    MPL_mem_alignment_t alignment;
#endif
} MPIDI_MVP_RMA_Immed_u;

/* Union over all types (integer, logical, and multi-language types) that are
   allowed in a CAS operation.  This is used to allocate enough space in the
   packet header for immediate data.  */
typedef union {
#define MPIR_OP_TYPE_MACRO(mpi_type_, c_type_, type_name_) c_type_ cas_##type_name_;
    MPIR_OP_TYPE_GROUP(C_INTEGER)
    MPIR_OP_TYPE_GROUP(FORTRAN_INTEGER)
    MPIR_OP_TYPE_GROUP(LOGICAL)
    MPIR_OP_TYPE_GROUP(BYTE)
    MPIR_OP_TYPE_GROUP(C_INTEGER_EXTRA)
    MPIR_OP_TYPE_GROUP(FORTRAN_INTEGER_EXTRA)
    MPIR_OP_TYPE_GROUP(LOGICAL_EXTRA)
    MPIR_OP_TYPE_GROUP(BYTE_EXTRA)
#undef MPIR_OP_TYPE_MACRO
} MPIDI_MVP_CAS_Immed_u;

/* Union over all types (all predefined types) that are allowed in a
   Fetch-and-op operation.  This can be too large for the packet header, so we
   limit the immediate space in the header to FOP_IMMED_INTS. */

/* *INDENT-OFF* */
/* Indentation turned off because "indent" is getting confused with
 * the lack of a semi-colon in the fields below */
typedef union {
#define MPIR_OP_TYPE_MACRO(mpi_type_, c_type_, type_name_) c_type_ fop##type_name_;
    MPIR_OP_TYPE_GROUP_ALL_BASIC
    MPIR_OP_TYPE_GROUP_ALL_EXTRA
#undef MPIR_OP_TYPE_MACRO
} MPIDI_MVP_FOP_Immed_u;
/* *INDENT-ON* */

/*
 * Predefined packet types.  This simplifies some of the code.
 */
/* FIXME: Having predefined names makes it harder to add new message types,
   such as different RMA types. */
/* We start with an arbitrarily chosen number (42), to help with
 * debugging when a packet type is not initialized or wrongly
 * initialized. */
typedef enum
{
    MPIDI_MVP_PKT_EAGER_SEND = 0,
    MPIDI_MVP_PKT_EAGER_SEND_CONTIG,
#ifndef MVP_DISABLE_HEADER_CACHING 
    MPIDI_MVP_PKT_FAST_EAGER_SEND,
    MPIDI_MVP_PKT_FAST_EAGER_SEND_WITH_REQ,
#endif /* !MVP_DISABLE_HEADER_CACHING */
    MPIDI_MVP_PKT_RPUT_FINISH,
    MPIDI_MVP_PKT_RGET_FINISH,
    MPIDI_MVP_PKT_DMA_FINISH,   /* TODO: implement this */
    MPIDI_MVP_PKT_ZCOPY_FINISH,
    MPIDI_MVP_PKT_ZCOPY_ACK,
    MPIDI_MVP_PKT_MCST,
    MPIDI_MVP_PKT_MCST_NACK,
    MPIDI_MVP_PKT_MCST_INIT,
    MPIDI_MVP_PKT_MCST_INIT_ACK,
    MPIDI_MVP_PKT_NOOP,
    MPIDI_MVP_PKT_RMA_RNDV_CLR_TO_SEND,
    //MPIDI_MVP_PKT_CUDA_CTS_CONTI,
    MPIDI_MVP_PKT_PUT_RNDV,
    MPIDI_MVP_PKT_ACCUMULATE_RNDV, 
    MPIDI_MVP_PKT_GET_ACCUM_RNDV,
    MPIDI_MVP_PKT_GET_RNDV,        
    MPIDI_MVP_PKT_RNDV_READY_REQ_TO_SEND,
    MPIDI_MVP_PKT_PACKETIZED_SEND_START,
    MPIDI_MVP_PKT_PACKETIZED_SEND_DATA,
    MPIDI_MVP_PKT_RNDV_R3_DATA,
    MPIDI_MVP_PKT_RNDV_R3_ACK,
    MPIDI_MVP_PKT_ADDRESS,
    MPIDI_MVP_PKT_ADDRESS_REPLY,
    MPIDI_MVP_PKT_CM_ESTABLISH,
#if defined(CKPT)
    MPIDI_MVP_PKT_CM_SUSPEND,
    MPIDI_MVP_PKT_CM_REACTIVATION_DONE,
    MPIDI_MVP_PKT_CR_REMOTE_UPDATE,
#endif /* defined(CKPT) */
#if defined(_SMP_LIMIC_) || defined(_SMP_CMA_)
    MPIDI_MVP_PKT_SMP_DMA_COMP,
#endif
    MPIDI_MVP_PKT_EAGERSHORT_SEND,
    MPIDI_MVP_PKT_EAGER_SYNC_SEND,      /* FIXME: no sync eager */
    MPIDI_MVP_PKT_EAGER_SYNC_ACK,
    MPIDI_MVP_PKT_READY_SEND,
    MPIDI_MVP_PKT_RNDV_REQ_TO_SEND,
    MPIDI_MVP_PKT_RNDV_CLR_TO_SEND,
    MPIDI_MVP_PKT_RNDV_SEND,    /* FIXME: should be stream put */
    MPIDI_MVP_PKT_CANCEL_SEND_REQ,
    MPIDI_MVP_PKT_CANCEL_SEND_RESP,
    /* RMA Packets begin here */
    MPIDI_MVP_PKT_PUT,
    MPIDI_MVP_PKT_PUT_IMMED,
    MPIDI_MVP_PKT_GET,
    MPIDI_MVP_PKT_ACCUMULATE,
    MPIDI_MVP_PKT_ACCUMULATE_IMMED,
    MPIDI_MVP_PKT_GET_ACCUM,
    MPIDI_MVP_PKT_GET_ACCUM_IMMED,
    MPIDI_MVP_PKT_FOP,
    MPIDI_MVP_PKT_FOP_IMMED,
    MPIDI_MVP_PKT_CAS_IMMED,
    MPIDI_MVP_PKT_GET_RESP,
    MPIDI_MVP_PKT_GET_RESP_IMMED,
    MPIDI_MVP_PKT_GET_ACCUM_RESP,
    MPIDI_MVP_PKT_GET_ACCUM_RESP_IMMED,
    MPIDI_MVP_PKT_FOP_RESP,
    MPIDI_MVP_PKT_FOP_RESP_IMMED,
    MPIDI_MVP_PKT_CAS_RESP_IMMED,
    MPIDI_MVP_PKT_LOCK,
    MPIDI_MVP_PKT_LOCK_ACK,
    MPIDI_MVP_PKT_LOCK_OP_ACK,
    MPIDI_MVP_PKT_UNLOCK,
    MPIDI_MVP_PKT_FLUSH,
    MPIDI_MVP_PKT_ACK,  /* ACK packet for FLUSH, UNLOCK, DECR_AT_COUNTER */
    MPIDI_MVP_PKT_DECR_AT_COUNTER,
    /* RMA Packets end here */
    MPIDI_MVP_PKT_FLOW_CNTL_UPDATE,
    MPIDI_MVP_PKT_CLOSE,
    MPIDI_MVP_PKT_REVOKE,
#ifndef MPIDI_MVP_HAS_NO_DYNAMIC_PROCESS
    /* Dynamic Connection Management */
    MPIDI_MVP_PKT_CONN_ACK,
    MPIDI_MVP_PKT_ACCEPT_ACK,
#endif
    MPIDI_MVP_PKT_END_MVP,
    /* The channel can define additional types by defining the value
     * MPIDI_MVP_PKT_ENUM */
#if defined(MPIDI_MVP_PKT_ENUM)
    MPIDI_MVP_PKT_ENUM,
#endif
    MPIDI_MVP_PKT_END_ALL,
    MPIDI_MVP_PKT_INVALID = -1  /* forces a signed enum to quash warnings */
} MPIDI_MVP_Pkt_type_t;

/* These pkt_flags can be "OR'ed" together */
typedef enum {
    MPIDI_MVP_PKT_FLAG_NONE = 0,
    MPIDI_MVP_PKT_FLAG_RMA_LOCK_SHARED = 1,
    MPIDI_MVP_PKT_FLAG_RMA_LOCK_EXCLUSIVE = 2,
    MPIDI_MVP_PKT_FLAG_RMA_UNLOCK = 4,
    MPIDI_MVP_PKT_FLAG_RMA_FLUSH = 8,
    MPIDI_MVP_PKT_FLAG_RMA_REQ_ACK = 16,
    MPIDI_MVP_PKT_FLAG_RMA_DECR_AT_COUNTER = 32,
    MPIDI_MVP_PKT_FLAG_RMA_NOCHECK = 64,
    MPIDI_MVP_PKT_FLAG_RMA_ACK = 128,
    MPIDI_MVP_PKT_FLAG_RMA_LOCK_GRANTED = 256,
    MPIDI_MVP_PKT_FLAG_RMA_LOCK_QUEUED_DATA_QUEUED = 512,
    MPIDI_MVP_PKT_FLAG_RMA_LOCK_QUEUED_DATA_DISCARDED = 1024,
    MPIDI_MVP_PKT_FLAG_RMA_LOCK_DISCARDED = 2048,
    MPIDI_MVP_PKT_FLAG_RMA_UNLOCK_NO_ACK = 4096,
    MPIDI_MVP_PKT_FLAG_RMA_IMMED_RESP = 8192,
    MPIDI_MVP_PKT_FLAG_RMA_STREAM = 16384
} MPIDI_MVP_Pkt_flags_t;

typedef struct MPIDI_MVP_Pkt_send
{
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    MPIDI_Message_match match;
    MPI_Request sender_req_id;  /* needed for ssend and send cancel */
    intptr_t data_sz;
#if defined(MPID_USE_SEQUENCE_NUMBERS)
    MPID_Seqnum_t seqnum;
#endif
}
MPIDI_MVP_Pkt_send_t;

#if defined(_SMP_LIMIC_) || defined(_SMP_CMA_)
typedef struct MPIDI_MVP_Pkt_comp
{
    uint8_t type;
    uint8_t fallback;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    size_t nb;
    MPI_Request *send_req_id;
} MPIDI_MVP_Pkt_comp_t;
#endif

/* NOTE: Normal and synchronous eager sends, as well as all ready-mode sends, 
   use the same structure but have a different type value. */
typedef MPIDI_MVP_Pkt_send_t MPIDI_MVP_Pkt_eager_send_t;
typedef MPIDI_MVP_Pkt_send_t MPIDI_MVP_Pkt_eager_sync_send_t;
typedef MPIDI_MVP_Pkt_send_t MPIDI_MVP_Pkt_ready_send_t;
typedef MPIDI_MVP_Pkt_send_t MPIDI_MVP_Pkt_eager_send_contig_t;

typedef struct MPIDI_MVP_Pkt_eagershort_send
{
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    MPIDI_Message_match match;
    intptr_t data_sz;
    char data[MPIDI_EAGER_SHORT_SIZE];  /* MVP support for RDMA FP */
} MPIDI_MVP_Pkt_eagershort_send_t;

typedef struct MPIDI_MVP_Pkt_eager_sync_ack
{
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    MPI_Request sender_req_id;
} MPIDI_MVP_Pkt_eager_sync_ack_t;

typedef struct MPIDI_MVP_Pkt_rndv_req_to_send
{
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    MPI_Request sender_req_id;  /* needed for ssend and send cancel */
    MPIDI_Message_match match;
    intptr_t data_sz;
    MPIDI_MVP_MRAILI_RNDV_INFO_DECL
} MPIDI_MVP_Pkt_rndv_req_to_send_t;

typedef struct MPIDI_MVP_Pkt_address {
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    uint32_t rdma_hndl[MAX_NUM_HCAS];
    unsigned long rdma_address;
} MPIDI_MVP_Pkt_address_t;

typedef struct MPIDI_MVP_Pkt_address_reply {
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    uint32_t reply_data;
} MPIDI_MVP_Pkt_address_reply_t;
/* data values for reply_data field*/
#define RDMA_FP_SUCCESS                 111
#define RDMA_FP_SENDBUFF_ALLOC_FAILED   121
#define RDMA_FP_MAX_SEND_CONN_REACHED   131

typedef struct MPIDI_MVP_Pkt_rndv_r3_Ack
{
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    uint32_t ack_data;
} MPIDI_MVP_Pkt_rndv_r3_ack_t;


typedef struct MPIDI_MVP_Pkt_rndv_clr_to_send
{
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    intptr_t recv_sz;
    MPIDI_MVP_MRAILI_RNDV_INFO_DECL
    MPI_Request sender_req_id;
    MPI_Request receiver_req_id;
} MPIDI_MVP_Pkt_rndv_clr_to_send_t;


typedef struct MPIDI_MVP_Pkt_rndv_send
{
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    MPI_Request receiver_req_id;
} MPIDI_MVP_Pkt_rndv_send_t;

typedef struct MPIDI_MVP_Pkt_packetized_send_start {
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    intptr_t origin_head_size;
} MPIDI_MVP_Pkt_packetized_send_start_t;

typedef struct MPIDI_MVP_Pkt_packetized_send_data {
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    MPI_Request receiver_req_id;
#if defined(_SMP_LIMIC_) || defined(_SMP_CMA_)
    struct MPIR_Request *send_req_id;
    struct MPIR_Request *csend_req_id;
#endif
} MPIDI_MVP_Pkt_packetized_send_data_t;

typedef MPIDI_MVP_Pkt_packetized_send_data_t MPIDI_MVP_Pkt_rndv_r3_data_t;

typedef struct MPIDI_MVP_Pkt_rput_finish_t
{
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    MPI_Request receiver_req_id; /* echoed*/
} MPIDI_MVP_Pkt_rput_finish_t;

typedef struct MPIDI_MVP_Pkt_zcopy_finish_t
{
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    uint8_t hca_index;
    MPI_Request receiver_req_id;
} MPIDI_MVP_Pkt_zcopy_finish_t;

typedef struct MPIDI_MVP_Pkt_zcopy_ack_t
{
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    MPI_Request sender_req_id; 
} MPIDI_MVP_Pkt_zcopy_ack_t;

typedef struct MPIDI_MVP_Pkt_mcast_t
{
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    int comm_id;
    int src_rank;
    int root;
    uint32_t psn;
} MPIDI_MVP_Pkt_mcast_t;
typedef MPIDI_MVP_Pkt_mcast_t MPIDI_MVP_Pkt_mcast_nack_t;
typedef MPIDI_MVP_Pkt_mcast_t MPIDI_MVP_Pkt_mcast_init_t;
typedef MPIDI_MVP_Pkt_mcast_t MPIDI_MVP_Pkt_mcast_init_ack_t;

typedef struct MPIDI_MVP_Pkt_rget_finish_t
{
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    MPI_Request sender_req_id;
} MPIDI_MVP_Pkt_rget_finish_t;

typedef struct MPIDI_MVP_pkt_dma_finish
{
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    /* TODO: MPI_Requests should not be here */
    MPI_Request req_id;
} MPIDI_MVP_Pkt_dma_finish_t;

typedef struct MPIDI_MVP_Pkt_cm_establish_t
{
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    int port_name_tag;
    uint64_t vc_addr; /* The VC that is newly created */
} MPIDI_MVP_Pkt_cm_establish_t;


typedef struct MPIDI_MVP_Pkt_cancel_send_req
{
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    MPI_Request sender_req_id;
    MPIDI_Message_match match;
}
MPIDI_MVP_Pkt_cancel_send_req_t;

typedef struct MPIDI_MVP_Pkt_cancel_send_resp
{
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    MPI_Request sender_req_id;
    int ack;
} MPIDI_MVP_Pkt_cancel_send_resp_t;

#ifndef MVP_DISABLE_HEADER_CACHING
#define MAX_SIZE_WITH_HEADER_CACHING 255

typedef struct MPIDI_MVP_MRAILI_Pkt_fast_eager_t {
    uint8_t     type;
    uint8_t     bytes_in_pkt;
    uint16_t    seqnum;
} MPIDI_MVP_MRAILI_Pkt_fast_eager;

typedef struct MPIDI_MVP_MRAILI_Pkt_fast_eager_with_req_t {
    uint8_t     type;
    uint8_t     bytes_in_pkt;
    uint16_t    seqnum;
    int         sender_req_id;
} MPIDI_MVP_MRAILI_Pkt_fast_eager_with_req;
#endif

typedef struct MPIDI_MVP_MRAILI_Pkt_comm_header_t {
    uint8_t type;
#if defined(MPIDI_MVP_MRAILI_IBA_PKT_DECL)
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
#endif
} MPIDI_MVP_MRAILI_Pkt_comm_header;

#define MPIDI_MVP_MRAILI_Pkt_noop MPIDI_MVP_MRAILI_Pkt_comm_header
#define MPIDI_MVP_MRAILI_Pkt_flow_cntl MPIDI_MVP_MRAILI_Pkt_comm_header

/* *INDENT-OFF* */
/* Indentation turned off because "indent" is getting confused with
 * the lack of a semi-colon in the field below */
#if defined(MPIDI_MVP_PKT_DEFS)
MPIDI_MVP_PKT_DEFS
#endif
/* *INDENT-ON* */

#define MPIDI_MVP_PKT_RMA_GET_TARGET_DATATYPE(pkt_, datatype_, err_)    \
    {                                                                   \
        /* This macro returns target_datatype in RMA operation          \
           packets. (PUT, GET, ACC, GACC, CAS, FOP) */                  \
        err_ = MPI_SUCCESS;                                             \
        switch((pkt_).type) {                                           \
        case (MPIDI_MVP_PKT_PUT):                                       \
        case (MPIDI_MVP_PKT_PUT_IMMED):                                 \
            datatype_ = (pkt_).put.datatype;                            \
            break;                                                      \
        case (MPIDI_MVP_PKT_GET):                                       \
            datatype_ = (pkt_).get.datatype;                            \
            break;                                                      \
        case (MPIDI_MVP_PKT_ACCUMULATE):                                \
        case (MPIDI_MVP_PKT_ACCUMULATE_IMMED):                          \
            datatype_ = (pkt_).accum.datatype;                          \
            break;                                                      \
        case (MPIDI_MVP_PKT_GET_ACCUM):                                 \
        case (MPIDI_MVP_PKT_GET_ACCUM_IMMED):                           \
            datatype_ = (pkt_).get_accum.datatype;                      \
            break;                                                      \
        case (MPIDI_MVP_PKT_CAS_IMMED):                                 \
            datatype_ = (pkt_).cas.datatype;                            \
            break;                                                      \
        case (MPIDI_MVP_PKT_FOP):                                       \
        case (MPIDI_MVP_PKT_FOP_IMMED):                                 \
            datatype_ = (pkt_).fop.datatype;                            \
            break;                                                      \
        default:                                                        \
            MPIR_ERR_SETANDJUMP1(err_, MPI_ERR_OTHER, "**invalidpkt", "**invalidpkt %d", (pkt_).type); \
        }                                                               \
    }

#define MPIDI_MVP_PKT_RMA_GET_TARGET_COUNT(pkt_, count_, err_)          \
    {                                                                   \
        /* This macro returns target_count in RMA operation             \
           packets. (PUT, GET, ACC, GACC, CAS, FOP) */                  \
        err_ = MPI_SUCCESS;                                             \
        switch((pkt_).type) {                                           \
        case (MPIDI_MVP_PKT_PUT):                                       \
        case (MPIDI_MVP_PKT_PUT_IMMED):                                 \
            count_ = (pkt_).put.count;                                  \
            break;                                                      \
        case (MPIDI_MVP_PKT_GET):                                       \
            count_ = (pkt_).get.count;                                  \
            break;                                                      \
        case (MPIDI_MVP_PKT_ACCUMULATE):                                \
        case (MPIDI_MVP_PKT_ACCUMULATE_IMMED):                          \
            count_ = (pkt_).accum.count;                                \
            break;                                                      \
        case (MPIDI_MVP_PKT_GET_ACCUM):                                 \
        case (MPIDI_MVP_PKT_GET_ACCUM_IMMED):                           \
            count_ = (pkt_).get_accum.count;                            \
            break;                                                      \
        case (MPIDI_MVP_PKT_CAS_IMMED):                                 \
        case (MPIDI_MVP_PKT_FOP):                                       \
        case (MPIDI_MVP_PKT_FOP_IMMED):                                 \
            count_ = 1;                                                 \
            break;                                                      \
        default:                                                        \
            MPIR_ERR_SETANDJUMP1(err_, MPI_ERR_OTHER, "**invalidpkt", "**invalidpkt %d", (pkt_).type); \
        }                                                               \
    }

#define MPIDI_MVP_PKT_RMA_GET_IMMED_DATA_PTR(pkt_, immed_data_, err_)   \
    {                                                                   \
        /* This macro returns pointer to immed data in RMA operation    \
           packets (PUT, ACC, GACC, FOP, CAS) and RMA response          \
           packets (GET_RESP, GACC_RESP, FOP_RESP, CAS_RESP). */        \
        err_ = MPI_SUCCESS;                                             \
        switch((pkt_).type) {                                           \
        case (MPIDI_MVP_PKT_PUT_IMMED):                                 \
            immed_data_ = &((pkt_).put.info.data);                      \
            break;                                                      \
        case (MPIDI_MVP_PKT_ACCUMULATE_IMMED):                          \
            immed_data_ = &((pkt_).accum.info.data);                    \
            break;                                                      \
        case (MPIDI_MVP_PKT_GET_ACCUM_IMMED):                           \
            immed_data_ = &((pkt_).get_accum.info.data);                \
            break;                                                      \
        case (MPIDI_MVP_PKT_FOP_IMMED):                                 \
            immed_data_ = &((pkt_).fop.info.data);                      \
            break;                                                      \
        case (MPIDI_MVP_PKT_CAS_IMMED):                                 \
            /* Note that here we return pointer of origin data, not     \
               pointer of compare data. */                              \
            immed_data_ = &((pkt_).cas.origin_data);                    \
            break;                                                      \
        case (MPIDI_MVP_PKT_GET_RESP_IMMED):                            \
            immed_data_ = &((pkt_).get_resp.info.data);                 \
            break;                                                      \
        case (MPIDI_MVP_PKT_GET_ACCUM_RESP_IMMED):                      \
            immed_data_ = &((pkt_).get_accum_resp.info.data);           \
            break;                                                      \
        case (MPIDI_MVP_PKT_FOP_RESP_IMMED):                            \
            immed_data_ = &((pkt_).fop_resp.info.data);                 \
            break;                                                      \
        case (MPIDI_MVP_PKT_CAS_RESP_IMMED):                            \
            immed_data_ = &((pkt_).cas_resp.info.data);                 \
            break;                                                      \
        default:                                                        \
            MPIR_ERR_SETANDJUMP1(err_, MPI_ERR_OTHER, "**invalidpkt", "**invalidpkt %d", (pkt_).type); \
        }                                                               \
    }

#define MPIDI_MVP_PKT_RMA_GET_FLAGS(pkt_, flags_, err_)                 \
    {                                                                   \
        /* This macro returns flags in RMA operation packets (PUT, GET, \
           ACC, GACC, FOP, CAS), RMA operation response packets         \
           (GET_RESP, GET_ACCUM_RESP, FOP_RESP, CAS_RESP), RMA control  \
           packets (UNLOCK) and RMA control response packets (LOCK_ACK, \
           LOCK_OP_ACK) */                                              \
        err_ = MPI_SUCCESS;                                             \
        switch((pkt_).type) {                                           \
        case (MPIDI_MVP_PKT_PUT):                                       \
        case (MPIDI_MVP_PKT_PUT_IMMED):                                 \
        case (MPIDI_MVP_PKT_PUT_RNDV):                                  \
            flags_ = (pkt_).put.pkt_flags;                                  \
            break;                                                      \
        case (MPIDI_MVP_PKT_GET):                                       \
        case (MPIDI_MVP_PKT_GET_RNDV):                                  \
            flags_ = (pkt_).get.pkt_flags;                                  \
            break;                                                      \
        case (MPIDI_MVP_PKT_ACCUMULATE):                                \
        case (MPIDI_MVP_PKT_ACCUMULATE_IMMED):                          \
            flags_ = (pkt_).accum.pkt_flags;                                \
            break;                                                      \
        case (MPIDI_MVP_PKT_GET_ACCUM):                                 \
        case (MPIDI_MVP_PKT_GET_ACCUM_IMMED):                           \
            flags_ = (pkt_).get_accum.pkt_flags;                            \
            break;                                                      \
        case (MPIDI_MVP_PKT_CAS_IMMED):                                 \
            flags_ = (pkt_).cas.pkt_flags;                                  \
            break;                                                      \
        case (MPIDI_MVP_PKT_FOP):                                       \
        case (MPIDI_MVP_PKT_FOP_IMMED):                                 \
            flags_ = (pkt_).fop.pkt_flags;                                  \
            break;                                                      \
        case (MPIDI_MVP_PKT_ACCUMULATE_RNDV):                           \
        case (MPIDI_MVP_PKT_GET_ACCUM_RNDV):                            \
            flags_ = (pkt_).accum_rndv.pkt_flags;                           \
            break;                                                      \
        case (MPIDI_MVP_PKT_GET_RESP):                                  \
        case (MPIDI_MVP_PKT_GET_RESP_IMMED):                            \
            flags_ = (pkt_).get_resp.pkt_flags;                             \
            break;                                                      \
        case (MPIDI_MVP_PKT_GET_ACCUM_RESP):                            \
        case (MPIDI_MVP_PKT_GET_ACCUM_RESP_IMMED):                      \
            flags_ = (pkt_).get_accum_resp.pkt_flags;                       \
            break;                                                      \
        case (MPIDI_MVP_PKT_FOP_RESP):                                  \
        case (MPIDI_MVP_PKT_FOP_RESP_IMMED):                            \
            flags_ = (pkt_).fop_resp.pkt_flags;                             \
            break;                                                      \
        case (MPIDI_MVP_PKT_CAS_RESP_IMMED):                            \
            flags_ = (pkt_).cas_resp.pkt_flags;                             \
            break;                                                      \
        case (MPIDI_MVP_PKT_LOCK):                                      \
            flags_ = (pkt_).lock.pkt_flags;                                 \
            break;                                                      \
        case (MPIDI_MVP_PKT_UNLOCK):                                    \
            flags_ = (pkt_).unlock.pkt_flags;                               \
            break;                                                      \
        case (MPIDI_MVP_PKT_LOCK_ACK):                                  \
            flags_ = (pkt_).lock_ack.pkt_flags;                             \
            break;                                                      \
        case (MPIDI_MVP_PKT_LOCK_OP_ACK):                               \
            flags_ = (pkt_).lock_op_ack.pkt_flags;                          \
            break;                                                      \
        default:                                                        \
            MPIR_ERR_SETANDJUMP1(err_, MPI_ERR_OTHER, "**invalidpkt", "**invalidpkt %d", (pkt_).type); \
        }                                                               \
    }


#define MPIDI_MVP_PKT_RMA_GET_OP(pkt_, op_, err_)                       \
    {                                                                   \
        /* This macro returns op in RMA operation packets (ACC, GACC,   \
           FOP) */                                                      \
        err_ = MPI_SUCCESS;                                             \
        switch((pkt_).type) {                                           \
        case (MPIDI_MVP_PKT_ACCUMULATE):                                \
        case (MPIDI_MVP_PKT_ACCUMULATE_IMMED):                          \
            op_ = (pkt_).accum.op;                                      \
            break;                                                      \
        case (MPIDI_MVP_PKT_GET_ACCUM):                                 \
        case (MPIDI_MVP_PKT_GET_ACCUM_IMMED):                           \
            op_ = (pkt_).get_accum.op;                                  \
            break;                                                      \
        case (MPIDI_MVP_PKT_FOP):                                       \
        case (MPIDI_MVP_PKT_FOP_IMMED):                                 \
            op_ = (pkt_).fop.op;                                        \
            break;                                                      \
        default:                                                        \
            MPIR_ERR_SETANDJUMP1(err_, MPI_ERR_OTHER, "**invalidpkt", "**invalidpkt %d", (pkt_).type); \
        }                                                               \
    }

#define MPIDI_MVP_PKT_RMA_ERASE_FLAGS(pkt_, err_)                       \
    {                                                                   \
        /* This macro erases pkt_flags in RMA operation packets (PUT, GET,  \
           ACC, GACC, FOP, CAS), RMA operation response packets         \
           (GET_RESP, GET_ACCUM_RESP, FOP_RESP, CAS_RESP), RMA control  \
           packets (UNLOCK) and RMA control response packets (LOCK_ACK, \
           LOCK_OP_ACK) */                                              \
        err_ = MPI_SUCCESS;                                             \
        switch((pkt_).type) {                                           \
        case (MPIDI_MVP_PKT_PUT):                                       \
        case (MPIDI_MVP_PKT_PUT_IMMED):                                 \
            (pkt_).put.pkt_flags = MPIDI_MVP_PKT_FLAG_NONE;                 \
            break;                                                      \
        case (MPIDI_MVP_PKT_GET):                                       \
            (pkt_).get.pkt_flags = MPIDI_MVP_PKT_FLAG_NONE;                 \
            break;                                                      \
        case (MPIDI_MVP_PKT_ACCUMULATE):                                \
        case (MPIDI_MVP_PKT_ACCUMULATE_IMMED):                          \
            (pkt_).accum.pkt_flags = MPIDI_MVP_PKT_FLAG_NONE;               \
            break;                                                      \
        case (MPIDI_MVP_PKT_GET_ACCUM):                                 \
        case (MPIDI_MVP_PKT_GET_ACCUM_IMMED):                           \
            (pkt_).get_accum.pkt_flags = MPIDI_MVP_PKT_FLAG_NONE;           \
            break;                                                      \
        case (MPIDI_MVP_PKT_CAS_IMMED):                                 \
            (pkt_).cas.pkt_flags = MPIDI_MVP_PKT_FLAG_NONE;                 \
            break;                                                      \
        case (MPIDI_MVP_PKT_FOP):                                       \
        case (MPIDI_MVP_PKT_FOP_IMMED):                                 \
            (pkt_).fop.pkt_flags = MPIDI_MVP_PKT_FLAG_NONE;                 \
            break;                                                      \
        case (MPIDI_MVP_PKT_GET_RESP):                                  \
        case (MPIDI_MVP_PKT_GET_RESP_IMMED):                            \
            (pkt_).get_resp.pkt_flags = MPIDI_MVP_PKT_FLAG_NONE;            \
            break;                                                      \
        case (MPIDI_MVP_PKT_GET_ACCUM_RESP):                            \
        case (MPIDI_MVP_PKT_GET_ACCUM_RESP_IMMED):                      \
            (pkt_).get_accum_resp.pkt_flags = MPIDI_MVP_PKT_FLAG_NONE;      \
            break;                                                      \
        case (MPIDI_MVP_PKT_FOP_RESP):                                  \
        case (MPIDI_MVP_PKT_FOP_RESP_IMMED):                            \
            (pkt_).fop_resp.pkt_flags = MPIDI_MVP_PKT_FLAG_NONE;            \
            break;                                                      \
        case (MPIDI_MVP_PKT_CAS_RESP_IMMED):                            \
            (pkt_).cas_resp.pkt_flags = MPIDI_MVP_PKT_FLAG_NONE;            \
            break;                                                      \
        case (MPIDI_MVP_PKT_LOCK):                                      \
            (pkt_).lock.pkt_flags = MPIDI_MVP_PKT_FLAG_NONE;                \
            break;                                                      \
        case (MPIDI_MVP_PKT_UNLOCK):                                    \
            (pkt_).unlock.pkt_flags = MPIDI_MVP_PKT_FLAG_NONE;              \
            break;                                                      \
        case (MPIDI_MVP_PKT_LOCK_ACK):                                  \
            (pkt_).lock_ack.pkt_flags = MPIDI_MVP_PKT_FLAG_NONE;            \
            break;                                                      \
        case (MPIDI_MVP_PKT_LOCK_OP_ACK):                               \
            (pkt_).lock_op_ack.pkt_flags = MPIDI_MVP_PKT_FLAG_NONE;         \
            break;                                                      \
        default:                                                        \
            MPIR_ERR_SETANDJUMP1(err_, MPI_ERR_OTHER, "**invalidpkt", "**invalidpkt %d", (pkt_).type); \
        }                                                               \
    }

#define MPIDI_MVP_PKT_RMA_GET_SOURCE_WIN_HANDLE(pkt_, win_hdl_, err_)   \
    {                                                                   \
        /* This macro returns source_win_handle in RMA operation        \
           packets (PUT, GET, ACC, GACC, CAS, FOP), RMA operation       \
           response packets (GET_RESP, GACC_RESP, CAS_RESP, FOP_RESP),  \
           RMA control packets (LOCK, UNLOCK, FLUSH), and RMA control   \
           response packets (LOCK_ACK, LOCK_OP_ACK, ACK). */            \
        err_ = MPI_SUCCESS;                                             \
        switch((pkt_).type) {                                           \
        case (MPIDI_MVP_PKT_PUT):                                       \
        case (MPIDI_MVP_PKT_PUT_IMMED):                                 \
            win_hdl_ = (pkt_).put.source_win_handle;                    \
            break;                                                      \
        case (MPIDI_MVP_PKT_ACCUMULATE):                                \
        case (MPIDI_MVP_PKT_ACCUMULATE_IMMED):                          \
            win_hdl_ = (pkt_).accum.source_win_handle;                  \
            break;                                                      \
        case (MPIDI_MVP_PKT_LOCK):                                      \
            win_hdl_ = (pkt_).lock.source_win_handle;                   \
            break;                                                      \
        case (MPIDI_MVP_PKT_UNLOCK):                                    \
            win_hdl_ = (pkt_).unlock.source_win_handle;                 \
            break;                                                      \
        case (MPIDI_MVP_PKT_FLUSH):                                     \
            win_hdl_ = (pkt_).flush.source_win_handle;                  \
            break;                                                      \
        case (MPIDI_MVP_PKT_LOCK_ACK):                                  \
            win_hdl_ = (pkt_).lock_ack.source_win_handle;               \
            break;                                                      \
        case (MPIDI_MVP_PKT_LOCK_OP_ACK):                               \
            win_hdl_ = (pkt_).lock_op_ack.source_win_handle;            \
            break;                                                      \
        case (MPIDI_MVP_PKT_ACK):                                       \
            win_hdl_ = (pkt_).ack.source_win_handle;                    \
            break;                                                      \
        default:                                                        \
            MPIR_ERR_SETANDJUMP1(err_, MPI_ERR_OTHER, "**invalidpkt", "**invalidpkt %d", (pkt_).type); \
        }                                                               \
    }

#define MPIDI_MVP_PKT_RMA_GET_TARGET_WIN_HANDLE(pkt_, win_hdl_, err_)   \
    {                                                                   \
        /* This macro returns target_win_handle in RMA operation        \
           packets (PUT, GET, ACC, GACC, CAS, FOP) and RMA control      \
           packets (LOCK, UNLOCK, FLUSH, DECR_AT_CNT) */                \
        err_ = MPI_SUCCESS;                                             \
        switch((pkt_).type) {                                           \
        case (MPIDI_MVP_PKT_PUT):                                       \
        case (MPIDI_MVP_PKT_PUT_IMMED):                                 \
            win_hdl_ = (pkt_).put.target_win_handle;                    \
            break;                                                      \
        case (MPIDI_MVP_PKT_GET):                                       \
            win_hdl_ = (pkt_).get.target_win_handle;                    \
            break;                                                      \
        case (MPIDI_MVP_PKT_ACCUMULATE):                                \
        case (MPIDI_MVP_PKT_ACCUMULATE_IMMED):                          \
            win_hdl_ = (pkt_).accum.target_win_handle;                  \
            break;                                                      \
        case (MPIDI_MVP_PKT_GET_ACCUM):                                 \
        case (MPIDI_MVP_PKT_GET_ACCUM_IMMED):                           \
            win_hdl_ = (pkt_).get_accum.target_win_handle;              \
            break;                                                      \
        case (MPIDI_MVP_PKT_CAS_IMMED):                                 \
            win_hdl_ = (pkt_).cas.target_win_handle;                    \
            break;                                                      \
        case (MPIDI_MVP_PKT_FOP):                                       \
        case (MPIDI_MVP_PKT_FOP_IMMED):                                 \
            win_hdl_ = (pkt_).fop.target_win_handle;                    \
            break;                                                      \
        case (MPIDI_MVP_PKT_LOCK):                                      \
            win_hdl_ = (pkt_).lock.target_win_handle;                   \
            break;                                                      \
        case (MPIDI_MVP_PKT_UNLOCK):                                    \
            win_hdl_ = (pkt_).unlock.target_win_handle;                 \
            break;                                                      \
        case (MPIDI_MVP_PKT_FLUSH):                                     \
            win_hdl_ = (pkt_).flush.target_win_handle;                  \
            break;                                                      \
        case (MPIDI_MVP_PKT_DECR_AT_COUNTER):                           \
            win_hdl_ = (pkt_).decr_at_cnt.target_win_handle;            \
            break;                                                      \
        default:                                                        \
            MPIR_ERR_SETANDJUMP1(err_, MPI_ERR_OTHER, "**invalidpkt", "**invalidpkt %d", (pkt_).type); \
        }                                                               \
    }

#define MPIDI_MVP_PKT_RMA_GET_REQUEST_HANDLE(pkt_, request_hdl_, err_)  \
    {                                                                   \
        err_ = MPI_SUCCESS;                                             \
        switch((pkt_).type) {                                           \
        case (MPIDI_MVP_PKT_GET):                                       \
            request_hdl_ = (pkt_).get.request_handle;                   \
            break;                                                      \
        case (MPIDI_MVP_PKT_GET_ACCUM):                                 \
        case (MPIDI_MVP_PKT_GET_ACCUM_IMMED):                           \
            request_hdl_ = (pkt_).get_accum.request_handle;             \
            break;                                                      \
        case (MPIDI_MVP_PKT_CAS_IMMED):                                 \
            request_hdl_ = (pkt_).cas.request_handle;                   \
            break;                                                      \
        case (MPIDI_MVP_PKT_FOP):                                       \
        case (MPIDI_MVP_PKT_FOP_IMMED):                                 \
            request_hdl_ = (pkt_).fop.request_handle;                   \
            break;                                                      \
        case (MPIDI_MVP_PKT_GET_RESP):                                  \
        case (MPIDI_MVP_PKT_GET_RESP_IMMED):                            \
            request_hdl_ = (pkt_).get_resp.request_handle;              \
            break;                                                      \
        case (MPIDI_MVP_PKT_GET_ACCUM_RESP):                            \
        case (MPIDI_MVP_PKT_GET_ACCUM_RESP_IMMED):                      \
            request_hdl_ = (pkt_).get_accum_resp.request_handle;        \
            break;                                                      \
        case (MPIDI_MVP_PKT_FOP_RESP):                                  \
        case (MPIDI_MVP_PKT_FOP_RESP_IMMED):                            \
            request_hdl_ = (pkt_).fop_resp.request_handle;              \
            break;                                                      \
        case (MPIDI_MVP_PKT_CAS_RESP_IMMED):                            \
            request_hdl_ = (pkt_).cas_resp.request_handle;              \
            break;                                                      \
        case (MPIDI_MVP_PKT_LOCK):                                      \
            request_hdl_ = (pkt_).lock.request_handle;                  \
            break;                                                      \
        case (MPIDI_MVP_PKT_LOCK_ACK):                                  \
            request_hdl_ = (pkt_).lock_ack.request_handle;              \
            break;                                                      \
        case (MPIDI_MVP_PKT_LOCK_OP_ACK):                               \
            request_hdl_ = (pkt_).lock_op_ack.request_handle;           \
            break;                                                      \
        default:                                                        \
            MPIR_ERR_SETANDJUMP1(err_, MPI_ERR_OTHER, "**invalidpkt", "**invalidpkt %d", (pkt_).type); \
        }                                                               \
    }

/* This macro judges if the RMA operation is a read operation,
 * which means, it will triffer the issuing of response data from
 * the target to the origin */
#define MPIDI_MVP_RMA_PKT_IS_READ_OP(pkt_)                             \
    ((pkt_).type == MPIDI_MVP_PKT_GET_ACCUM_IMMED ||                    \
     (pkt_).type == MPIDI_MVP_PKT_GET_ACCUM ||                          \
     (pkt_).type == MPIDI_MVP_PKT_FOP_IMMED ||                          \
     (pkt_).type == MPIDI_MVP_PKT_FOP ||                                \
     (pkt_).type == MPIDI_MVP_PKT_CAS_IMMED ||                          \
     (pkt_).type == MPIDI_MVP_PKT_GET)

/* This macro judges if the RMA operation is a immed operation */
#define MPIDI_MVP_RMA_PKT_IS_IMMED_OP(pkt_)                            \
    ((pkt_).type == MPIDI_MVP_PKT_GET_ACCUM_IMMED ||                    \
     (pkt_).type == MPIDI_MVP_PKT_FOP_IMMED ||                          \
     (pkt_).type == MPIDI_MVP_PKT_CAS_IMMED ||                          \
     (pkt_).type == MPIDI_MVP_PKT_PUT_IMMED ||                          \
     (pkt_).type == MPIDI_MVP_PKT_ACCUMULATE_IMMED)


typedef struct MPIDI_MVP_Pkt_put
{
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    uint32_t rma_issued;
    int pkt_flags;
    void *addr;
    int count;
    MPI_Datatype datatype;
    MPI_Win target_win_handle;
    MPI_Win source_win_handle;
    union {
        int flattened_type_size;
        MPIDI_MVP_RMA_Immed_u data;
    } info;
#if defined (CHANNEL_PSM)
/*
    intptr_t rndv_len;
    int source_rank;
    int target_rank;
    int rndv_tag;
    int rndv_mode;
    int mapped_trank;
    int mapped_srank;
    */
#endif       
} MPIDI_MVP_Pkt_put_t;

/* 
 * MVP rndv version 
 * It is important that these definitions remain beneath their base
 * MPICH versions or Git will get confused when merging 
 */
/*
Top elements of MPIDI_MVP_Pkt_put_rndv should match with MPIDI_MVP_Pkt_put_t 
TODO: remove this restriction
*/
typedef struct MPIDI_MVP_Pkt_put_rndv
{
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    uint32_t rma_issued;
    int pkt_flags;
    void *addr;
    int count;
    MPI_Datatype datatype;
    MPI_Win target_win_handle;
    MPI_Win source_win_handle;
    union {
        int flattened_type_size;
        MPIDI_MVP_RMA_Immed_u data;
    } info;
    MPI_Request sender_req_id;
    intptr_t data_sz;
    MPIDI_MVP_MRAILI_Rndv_info_t rndv;
} MPIDI_MVP_Pkt_put_rndv_t;

typedef struct MPIDI_MVP_Pkt_get
{
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    uint32_t rma_issued;
    int pkt_flags;
    void *addr;
    int count;
    MPI_Datatype datatype;
    struct {
        int flattened_type_size;
    } info;
    MPI_Request request_handle;
    MPI_Win target_win_handle;
#if defined (CHANNEL_PSM)
    int rndv_mode;
    int target_rank;
    int source_rank;
    intptr_t rndv_len;
    int rndv_tag;
    int mapped_srank;
    int mapped_trank;
    MPI_Win source_win_handle;
#endif
} MPIDI_MVP_Pkt_get_t;

/* 
 * MVP rndv version
 * It is important that these definitions remain beneath their base
 * MPICH versions or Git will get confused when merging 
 */
/*
Top elements of MPIDI_MVP_Pkt_get_rndv should match with MPIDI_MVP_Pkt_get_t 
TODO: remove this restriction
*/
typedef struct MPIDI_MVP_Pkt_get_rndv
{
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    uint32_t rma_issued;
    int pkt_flags;
    void *addr;
    int count;
    MPI_Datatype datatype;
    struct {
        int flattened_type_size;
    } info;
    MPI_Request request_handle;
    MPI_Win target_win_handle;
    intptr_t data_sz;
    MPIDI_MVP_MRAILI_Rndv_info_t rndv;
} MPIDI_MVP_Pkt_get_rndv_t;

typedef struct MPIDI_MVP_Pkt_get_resp
{
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    int protocol;
    MPI_Request request_handle;
    /* following are used to decrement ack_counter at origin */
    int target_rank;
    int pkt_flags;
    /* Followings are to piggyback IMMED data */
    struct {
        /* note that we use struct here in order
         * to consistently access data
         * by "pkt->info.data". */
        MPIDI_MVP_RMA_Immed_u data;
    } info;
#if defined (CHANNEL_PSM)
    int source_rank;
    int rndv_tag;
    int rndv_mode;
    intptr_t rndv_len;
    MPI_Win target_win_handle;
    MPI_Win source_win_handle;
    int mapped_trank;
    int mapped_srank;
#endif /* CHANNEL_PSM */
} MPIDI_MVP_Pkt_get_resp_t;


typedef struct MPIDI_MVP_Pkt_accum
{
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    uint32_t rma_issued;
    int pkt_flags;
    void *addr;
    int count;
    MPI_Datatype datatype;
    MPI_Op op;
    MPI_Win target_win_handle;
    MPI_Win source_win_handle;
    union {
        int flattened_type_size;
        MPIDI_MVP_RMA_Immed_u data;
    } info;
#if defined (CHANNEL_PSM)
    intptr_t rndv_len;
    int source_rank;
    int target_rank;
    int rndv_tag;
    int resp_rndv_tag;
    int rndv_mode;
    int stream_mode;
    int mapped_srank;
    int mapped_trank;
#endif    
} MPIDI_MVP_Pkt_accum_t;

/* 
 * MVP rndv version
 * It is important that these definitions remain beneath their base
 * MPICH versions or Git will get confused when merging 
 */
/*
Top elements of MPIDI_MVP_Pkt_accum_rndv should match with MPIDI_MVP_Pkt_accum_t 
TODO: remove this restriction
*/
typedef struct MPIDI_MVP_Pkt_accum_rndv
{
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    uint32_t rma_issued;
    int pkt_flags;
    MPI_Request request_handle; /* For get_accumulate response */
    void *addr;
    int count;
    MPI_Datatype datatype;
    MPI_Op op;
    MPI_Win target_win_handle; 
    MPI_Win source_win_handle; 
    union {
        int flattened_type_size;
        MPIDI_MVP_RMA_Immed_u data;
    } info;
    MPI_Request sender_req_id;
    intptr_t data_sz;

    MPIDI_MVP_MRAILI_RNDV_INFO_DECL
} MPIDI_MVP_Pkt_accum_rndv_t;

typedef struct MPIDI_MVP_Pkt_get_accum {
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    uint32_t rma_issued;
    int pkt_flags;
    MPI_Request request_handle; /* For get_accumulate response */
    void *addr;
    int count;
    MPI_Datatype datatype;
    MPI_Op op;
    MPI_Win target_win_handle;
    union {
        int flattened_type_size;
        MPIDI_MVP_RMA_Immed_u data;
    } info;
#if defined (CHANNEL_PSM)
    intptr_t rndv_len;
    int source_rank;
    int target_rank;
    int rndv_tag;
    int resp_rndv_tag;
    int rndv_mode;
    int stream_mode;
    int mapped_srank;
    int mapped_trank;
    MPI_Win source_win_handle;
    void * target_ptr;
#endif    
} MPIDI_MVP_Pkt_get_accum_t;

/* 
 * MVP rndv version
 * It is important that these definitions remain beneath their base
 * MPICH versions or Git will get confused when merging 
 */
typedef struct MPIDI_MVP_Pkt_get_accum_rndv
{
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    uint32_t rma_issued;
    int pkt_flags;
    MPI_Request request_handle; 
    void *addr;
    int count;
    MPI_Datatype datatype;
    MPI_Op op;
    MPI_Win target_win_handle; 
    union {
        int flattened_type_size;
        MPIDI_MVP_RMA_Immed_u data;
    } info;
    MPI_Request sender_req_id;
    intptr_t data_sz;

    MPIDI_MVP_MRAILI_RNDV_INFO_DECL
} MPIDI_MVP_Pkt_get_accum_rndv_t;


typedef struct MPIDI_MVP_Pkt_get_accum_resp
{
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    int protocol;
    MPI_Request request_handle;
    /* following are used to decrement ack_counter at origin */
    int target_rank;
    int pkt_flags;
    /* Followings are to piggyback IMMED data */
    struct {
        /* note that we use struct here in order
         * to consistently access data
         * by "pkt->info.data". */
        MPIDI_MVP_RMA_Immed_u data;
    } info;
#if defined (CHANNEL_PSM)
    int source_rank;
    int rndv_tag;
    int rndv_mode;
    intptr_t rndv_len;
    MPI_Win target_win_handle;
    MPI_Win source_win_handle;
    int mapped_trank;
    int mapped_srank;
#endif /* CHANNEL_PSM */
} MPIDI_MVP_Pkt_get_accum_resp_t;

typedef struct MPIDI_MVP_Pkt_cas
{
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    int pkt_flags;
    MPI_Datatype datatype;
    void *addr;
    MPI_Request request_handle;
    MPI_Win target_win_handle;
    MPIDI_MVP_CAS_Immed_u origin_data;
    MPIDI_MVP_CAS_Immed_u compare_data;
#if defined (CHANNEL_PSM)
    int source_rank;
    int mapped_srank;
    int mapped_trank;
#endif
} MPIDI_MVP_Pkt_cas_t;

typedef struct MPIDI_MVP_Pkt_cas_resp
{
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    MPI_Request request_handle;
    struct {
        /* note that we use struct here in order
         * to consistently access data
         * by "pkt->info.data". */
        MPIDI_MVP_CAS_Immed_u data;
    } info;
    /* following are used to decrement ack_counter at origin */
    int target_rank;
    int pkt_flags;
#if defined (CHANNEL_PSM)
    int mapped_srank;
    int mapped_trank;
#endif
} MPIDI_MVP_Pkt_cas_resp_t;

typedef struct MPIDI_MVP_Pkt_fop
{
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    int pkt_flags;
    MPI_Datatype datatype;
    void *addr;
    MPI_Op op;
    MPI_Request request_handle;
    MPI_Win target_win_handle;
    struct {
        /* note that we use struct here in order
         * to consistently access data
         * by "pkt->info.data". */
        MPIDI_MVP_RMA_Immed_u data;
    } info;
#if defined (CHANNEL_PSM)
    int source_rank;
    int mapped_srank;
    int mapped_trank;
#endif
}
MPIDI_MVP_Pkt_fop_t;

typedef struct MPIDI_MVP_Pkt_fop_resp
{
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    MPI_Request request_handle;
    struct {
        /* note that we use struct here in order
         * to consistently access data
         * by "pkt->info.data". */
        MPIDI_MVP_RMA_Immed_u data;
    } info;
    /* following are used to decrement ack_counter at origin */
    int target_rank;
    int pkt_flags;
#if defined (CHANNEL_PSM)
    int mapped_srank;
    int mapped_trank;
#endif
} MPIDI_MVP_Pkt_fop_resp_t;

typedef struct MPIDI_MVP_Pkt_lock
{
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    int pkt_flags;
    MPI_Win target_win_handle;
    /* Note that either source_win_handle
     * or request_handle will be used. Here
     * we need both of them because PUT/GET
     * may be converted to LOCK packet,
     * PUT has source_win_handle area and
     * GET has request_handle area. */
    MPI_Win source_win_handle;
    MPI_Request request_handle;
#if defined (CHANNEL_PSM)
    int source_rank;
    int target_rank;
    int mapped_srank;
    int mapped_trank;
#endif
}
MPIDI_MVP_Pkt_lock_t;

typedef struct MPIDI_MVP_Pkt_unlock {
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    MPI_Win target_win_handle;
    MPI_Win source_win_handle;
    int pkt_flags;
#if defined (CHANNEL_PSM)
    int source_rank;
    int mapped_srank;
    int mapped_trank;
#endif
} MPIDI_MVP_Pkt_unlock_t;

typedef struct MPIDI_MVP_Pkt_flush {
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    MPI_Win target_win_handle;
    MPI_Win source_win_handle;
#if defined (CHANNEL_PSM)
/*
    int source_rank;
    int target_rank;
    int mapped_srank;
    int mapped_trank;
    */
#endif
} MPIDI_MVP_Pkt_flush_t;

typedef struct MPIDI_MVP_Pkt_lock_ack {
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    int pkt_flags;
    /* note that either source_win_handle
     * or request_handle is used. */
    MPI_Win source_win_handle;
    MPI_Request request_handle;
    int target_rank;
#if defined (CHANNEL_PSM)
/*
    int source_rank;
    int mapped_srank;
    int mapped_trank;
    */
#endif
} MPIDI_MVP_Pkt_lock_ack_t;

typedef struct MPIDI_MVP_Pkt_lock_op_ack {
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    int pkt_flags;
    /* note that either source_win_handle
     * or request_handle is used. */
    MPI_Win source_win_handle;
    MPI_Request request_handle;
    int target_rank;
#if defined CHANNEL_PSM
    int source_rank;
    int mapped_trank;
    int mapped_srank;
    int trank;
#endif
} MPIDI_MVP_Pkt_lock_op_ack_t;

/* This ACK packet is the acknowledgement
 * for FLUSH, UNLOCK and DECR_AT_COUNTER
 * packet */
typedef struct MPIDI_MVP_Pkt_ack {
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    MPI_Win source_win_handle;
    int target_rank;
    int pkt_flags;
#if defined (CHANNEL_PSM)
    int source_rank;
    int mapped_srank;
    int mapped_trank;
#endif
} MPIDI_MVP_Pkt_ack_t;

typedef struct MPIDI_MVP_Pkt_decr_at_counter {
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    MPI_Win target_win_handle;
    MPI_Win source_win_handle;
    int pkt_flags;
#if defined (CHANNEL_PSM)
    int target_rank;
    int source_rank;
    int mapped_srank;
    int mapped_trank;
#endif
} MPIDI_MVP_Pkt_decr_at_counter_t;

typedef struct MPIDI_MVP_Pkt_close
{
    uint8_t type;
    MPIDI_MVP_MRAILI_IBA_PKT_DECL
    int ack;
} MPIDI_MVP_Pkt_close_t;

#ifndef MPIDI_MVP_HAS_NO_DYNAMIC_PROCESS
/* packet types used in dynamic process connection. */
typedef struct MPIDI_MVP_Pkt_conn_ack {
    MPIDI_MVP_Pkt_type_t type;
    int ack;
} MPIDI_MVP_Pkt_conn_ack_t;

typedef MPIDI_MVP_Pkt_conn_ack_t MPIDI_MVP_Pkt_accept_ack_t;
#endif /* end of MPIDI_MVP_HAS_NO_DYNAMIC_PROCESS */

typedef struct MPIDI_MVP_Pkt_revoke {
    MPIDI_MVP_Pkt_type_t type;
    MPIR_Context_id_t revoked_comm;
} MPIDI_MVP_Pkt_revoke_t;

typedef union MPIDI_MVP_Pkt
{
    uint8_t type;
    MPIDI_MVP_Pkt_address_t address;
    MPIDI_MVP_Pkt_address_reply_t addr_reply;
    MPIDI_MVP_Pkt_rput_finish_t rput_finish;
    MPIDI_MVP_Pkt_put_rndv_t put_rndv;
    MPIDI_MVP_Pkt_get_rndv_t get_rndv;
    MPIDI_MVP_Pkt_accum_rndv_t accum_rndv;
    MPIDI_MVP_Pkt_rndv_r3_ack_t rndv_r3_ack;
    MPIDI_MVP_Pkt_eager_send_t eager_send;
    MPIDI_MVP_Pkt_eagershort_send_t eagershort_send;
    MPIDI_MVP_Pkt_eager_sync_send_t eager_sync_send;
    MPIDI_MVP_Pkt_eager_sync_ack_t eager_sync_ack;
    MPIDI_MVP_Pkt_eager_send_t ready_send;
    MPIDI_MVP_Pkt_rndv_req_to_send_t rndv_req_to_send;
    MPIDI_MVP_Pkt_rndv_clr_to_send_t rndv_clr_to_send;
    MPIDI_MVP_Pkt_rndv_send_t rndv_send;
    MPIDI_MVP_Pkt_cancel_send_req_t cancel_send_req;
    MPIDI_MVP_Pkt_cancel_send_resp_t cancel_send_resp;
    MPIDI_MVP_Pkt_put_t put;
    MPIDI_MVP_Pkt_get_t get;
    MPIDI_MVP_Pkt_get_resp_t get_resp;
    MPIDI_MVP_Pkt_accum_t accum;
    MPIDI_MVP_Pkt_get_accum_t get_accum;
    MPIDI_MVP_Pkt_lock_t lock;
    MPIDI_MVP_Pkt_lock_ack_t lock_ack;
    MPIDI_MVP_Pkt_lock_op_ack_t lock_op_ack;
    MPIDI_MVP_Pkt_unlock_t unlock;
    MPIDI_MVP_Pkt_flush_t flush;
    MPIDI_MVP_Pkt_ack_t ack;
    MPIDI_MVP_Pkt_decr_at_counter_t decr_at_cnt;
    MPIDI_MVP_Pkt_close_t close;
#ifndef MPIDI_MVP_HAS_NO_DYNAMIC_PROCESS
    MPIDI_MVP_Pkt_conn_ack_t conn_ack;
    MPIDI_MVP_Pkt_accept_ack_t accept_ack;
#endif
    MPIDI_MVP_Pkt_cas_t cas;
    MPIDI_MVP_Pkt_cas_resp_t cas_resp;
    MPIDI_MVP_Pkt_fop_t fop;
    MPIDI_MVP_Pkt_fop_resp_t fop_resp;
    MPIDI_MVP_Pkt_get_accum_resp_t get_accum_resp;
    MPIDI_MVP_Pkt_revoke_t revoke;
#if defined(MPIDI_MVP_PKT_DECL)
     MPIDI_MVP_PKT_DECL
#endif
} MPIDI_MVP_Pkt_t;

/* Extended header packet types */

typedef struct MPIDI_MVP_Ext_pkt_stream {
    MPI_Aint stream_offset;
} MPIDI_MVP_Ext_pkt_stream_t;

#if defined(MPID_USE_SEQUENCE_NUMBERS)
typedef struct MPIDI_MVP_Pkt_send_container {
    MPIDI_MVP_Pkt_send_t pkt;
    struct MPIDI_MVP_Pkt_send_container_s *next;
} MPIDI_MVP_Pkt_send_container_t;
#endif

/* MVP reference array for packet sizes */
extern int MPIDI_MVP_Pkt_size_index[];

#define MPIDI_MVP_PKT_SIZE(_pkt) \
    (MPIDI_MVP_Pkt_size_index[((MPIDI_MVP_Pkt_t *)(_pkt))->type])

#endif /* MPIDPKT_H_INCLUDED */
