/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda. For detailed
 * copyright and licensing information, please refer to the copyright file
 * COPYRIGHT in the top level MVAPICH directory.
 */

#include "mvp_smp_impl.h"
#include "mvp_pkt.h"

/** We maintain an index table to get the header size ******/
int MPIDI_MVP_Pkt_size_index[] = {
    sizeof(MPIDI_MVP_Pkt_eager_send_t), /* 0 */
    sizeof(MPIDI_MVP_Pkt_eager_send_contig_t),
#ifndef MVP_DISABLE_HEADER_CACHING
    sizeof(MPIDI_MVP_MRAILI_Pkt_fast_eager),
    sizeof(MPIDI_MVP_MRAILI_Pkt_fast_eager_with_req),
#endif /* !MVP_DISABLE_HEADER_CACHING */
    sizeof(MPIDI_MVP_Pkt_rput_finish_t), sizeof(MPIDI_MVP_Pkt_rget_finish_t),
    sizeof(MPIDI_MVP_Pkt_zcopy_finish_t), sizeof(MPIDI_MVP_Pkt_zcopy_ack_t),
    sizeof(MPIDI_MVP_Pkt_mcast_t), sizeof(MPIDI_MVP_Pkt_mcast_nack_t),
    sizeof(MPIDI_MVP_Pkt_mcast_init_t), sizeof(MPIDI_MVP_Pkt_mcast_init_ack_t),
    sizeof(MPIDI_MVP_MRAILI_Pkt_noop), sizeof(MPIDI_MVP_Pkt_rndv_clr_to_send_t),
    sizeof(MPIDI_MVP_Pkt_rndv_clr_to_send_t), sizeof(MPIDI_MVP_Pkt_put_rndv_t),
    sizeof(MPIDI_MVP_Pkt_accum_rndv_t), sizeof(MPIDI_MVP_Pkt_get_accum_rndv_t),
    sizeof(MPIDI_MVP_Pkt_get_rndv_t), sizeof(MPIDI_MVP_Pkt_rndv_req_to_send_t),
    sizeof(MPIDI_MVP_Pkt_packetized_send_start_t),
    sizeof(MPIDI_MVP_Pkt_packetized_send_data_t),
    sizeof(MPIDI_MVP_Pkt_rndv_r3_data_t), sizeof(MPIDI_MVP_Pkt_rndv_r3_ack_t),
    sizeof(MPIDI_MVP_Pkt_address_t), sizeof(MPIDI_MVP_Pkt_address_reply_t),
    sizeof(MPIDI_MVP_Pkt_cm_establish_t),
#if defined(CKPT)
    /* These control packet has no packet header,
     * use noop packet as the packet header size*/
    sizeof(MPIDI_MVP_MRAILI_Pkt_noop), sizeof(MPIDI_MVP_MRAILI_Pkt_noop),
    sizeof(MPIDI_MVP_MRAILI_Pkt_noop),
#endif /* defined(CKPT) */
#if defined(_SMP_LIMIC_) || defined(_SMP_CMA_)
    sizeof(MPIDI_MVP_Pkt_comp_t),
#endif
    sizeof(MPIDI_MVP_Pkt_eagershort_send_t),
    sizeof(MPIDI_MVP_Pkt_eager_sync_send_t),
    sizeof(MPIDI_MVP_Pkt_eager_sync_ack_t), sizeof(MPIDI_MVP_Pkt_ready_send_t),
    sizeof(MPIDI_MVP_Pkt_rndv_req_to_send_t),
    sizeof(MPIDI_MVP_Pkt_rndv_clr_to_send_t), sizeof(MPIDI_MVP_Pkt_rndv_send_t),
    sizeof(MPIDI_MVP_Pkt_cancel_send_req_t),
    sizeof(MPIDI_MVP_Pkt_cancel_send_resp_t), sizeof(MPIDI_MVP_Pkt_t),
    sizeof(MPIDI_MVP_Pkt_put_t), sizeof(MPIDI_MVP_Pkt_get_t),
    sizeof(MPIDI_MVP_Pkt_t), sizeof(MPIDI_MVP_Pkt_accum_t),
    sizeof(MPIDI_MVP_Pkt_t), sizeof(MPIDI_MVP_Pkt_get_accum_t),
    sizeof(MPIDI_MVP_Pkt_fop_t), sizeof(MPIDI_MVP_Pkt_fop_t),
    sizeof(MPIDI_MVP_Pkt_cas_t), sizeof(MPIDI_MVP_Pkt_get_resp_t),
    sizeof(MPIDI_MVP_Pkt_get_resp_t), sizeof(MPIDI_MVP_Pkt_get_accum_resp_t),
    sizeof(MPIDI_MVP_Pkt_get_accum_resp_t), sizeof(MPIDI_MVP_Pkt_fop_resp_t),
    sizeof(MPIDI_MVP_Pkt_fop_resp_t), sizeof(MPIDI_MVP_Pkt_cas_resp_t),
    sizeof(MPIDI_MVP_Pkt_lock_t), sizeof(MPIDI_MVP_Pkt_lock_ack_t),
    sizeof(MPIDI_MVP_Pkt_lock_op_ack_t), sizeof(MPIDI_MVP_Pkt_unlock_t),
    sizeof(MPIDI_MVP_Pkt_flush_t), sizeof(MPIDI_MVP_Pkt_ack_t),
    sizeof(MPIDI_MVP_Pkt_decr_at_counter_t),
    sizeof(MPIDI_MVP_MRAILI_Pkt_flow_cntl), sizeof(MPIDI_MVP_Pkt_close_t), -1};
