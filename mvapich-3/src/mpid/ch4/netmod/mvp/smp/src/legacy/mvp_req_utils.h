/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda. For detailed
 * copyright and licensing information, please refer to the copyright file
 * COPYRIGHT in the top level MVAPICH directory.
 */

#define set_request_info(rreq_, pkt_, msg_type_)                               \
    {                                                                          \
        (rreq_)->status.MPI_SOURCE = (pkt_)->match.parts.rank;                 \
        (rreq_)->status.MPI_TAG = (pkt_)->match.parts.tag;                     \
        MPIR_STATUS_SET_COUNT((rreq_)->status, (pkt_)->data_sz);               \
        MPIDI_MVP_REQUEST(rreq_, dev).sender_req_id = (pkt_)->sender_req_id;   \
        MPIDI_MVP_REQUEST(rreq_, dev).recv_data_sz = (pkt_)->data_sz;          \
        MPIDI_Request_set_seqnum(rreq_, (pkt_)->seqnum);                       \
        MPIDI_Request_set_msg_type(rreq_, (msg_type_));                        \
    }

#define MPIDI_REQUEST_SYNC_SEND_MASK  (0x1 << MPIDI_REQUEST_SYNC_SEND_SHIFT)
#define MPIDI_REQUEST_SYNC_SEND_SHIFT 3
