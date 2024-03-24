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

#if !defined(MPICH_MPIDI_CH3_POST_H_INCLUDED)
#define MPICH_MPIDI_CH3_POST_H_INCLUDED

#include "mpichconf.h"

#define MPIDI_CH3_EAGER_MAX_MSG_SIZE(vc)                                       \
    ((vc)->smp.local_nodes >= 0 && SMP_INIT ? MVP_SMP_EAGERSIZE :              \
                                              rdma_iba_eager_threshold)
#ifdef _ENABLE_UD_
#define MPIDI_CH3_EAGER_FAST_MAX_MSG_SIZE(vc)                                  \
    (((vc)->smp.local_nodes >= 0 && SMP_INIT) ?                                \
         MVP_SMP_EAGERSIZE :                                                   \
         ((MVP_USE_UD_HYBRID && (vc->mrail.state & MRAILI_UD_CONNECTED)) ?     \
              MRAIL_MAX_UD_SIZE :                                              \
              MIN(DEFAULT_MEDIUM_VBUF_SIZE, rdma_fp_buffer_size)))
#else
#define MPIDI_CH3_EAGER_FAST_MAX_MSG_SIZE(vc)                                  \
    (((vc)->smp.local_nodes >= 0 && SMP_INIT) ?                                \
         MVP_SMP_EAGERSIZE :                                                   \
         MIN(DEFAULT_MEDIUM_VBUF_SIZE, rdma_fp_buffer_size))
#endif

#define MPIDI_CH3_R3_THRESHOLD(vc)                                             \
    (((vc)->smp.local_nodes >= 0 && SMP_INIT) ? rdma_intra_node_r3_threshold : \
                                                rdma_inter_node_r3_threshold)
/*
 * Channel level request management macros
 */
#define MPIDI_CH3_Request_add_ref(req)                                         \
    {                                                                          \
        MPIR_Assert(HANDLE_GET_MPI_KIND(req->handle) == MPID_REQUEST);         \
        MPIR_Object_add_ref(req);                                              \
    }

#define MPIDI_CH3_Request_release_ref(req, req_ref_count)                      \
    {                                                                          \
        MPIR_Assert(HANDLE_GET_MPI_KIND(req->handle) == MPID_REQUEST);         \
        MPIR_Object_release_ref(req, req_ref_count);                           \
        MPIR_Assert(req->ref_count >= 0);                                      \
    }

/*
 * CH3 Progress routines (implemented as macros for performanace)
 */

#if defined(MPICH_SINGLE_THREADED)
#define MPIDI_CH3_Progress_start(state)
#define MPIDI_CH3_Progress_end(state)
#else
#define MPIDI_CH3_Progress_start(progress_state_)                              \
    {                                                                          \
        (progress_state_)->ch.completion_count =                               \
            MPIDI_CH3I_progress_completion_count;                              \
    }
#endif
#define MPIDI_CH3_Progress_poke() MPIDI_CH3_Progress_test()

#define MPIDI_CH3_Progress_test() (MPIDI_CH3I_Progress_test());

int MPIDI_CH3I_Progress(int blocking, MPID_Progress_state *state);
int MPIDI_CH3I_Progress_test(void);

#define MPIDI_CH3_Progress_wait(state) MPIDI_CH3I_Progress(TRUE, state)

#include "mpidi_ch3_rdma_post.h"

/*
 * Enable optional functionality
 */
#define MPIDI_CH3_Comm_Spawn MPIDI_CH3_Comm_Spawn

/* Macros for OSU-MPI2 */
#define MPIDI_CH3_RNDV_SET_REQ_INFO(rreq, rts_pkt)                             \
    {                                                                          \
        rreq->mrail.protocol = rts_pkt->rndv.protocol;                         \
    }

#define MPIDI_CH3_RNDV_PROTOCOL_IS_READ(rts_pkt)                               \
    (MRAILI_PROTOCOL_RGET == rts_pkt->rndv.protocol)

#define MPIDI_CH3_RECV_REQ_IS_READ(rreq)                                       \
    (MRAILI_PROTOCOL_RGET == rreq->mrail.protocol)

extern int SMP_INIT;
extern int SMP_ONLY;

/* End of OSU-MPI2 */

#endif /* !defined(MPICH_MPIDI_CH3_POST_H_INCLUDED) */
