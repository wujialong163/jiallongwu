/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 *
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda.
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level MVAPICH directory.
 */

#include "mvp_smp_impl.h"
#include "mvp_vc.h"
#include "mvp_pkt.h"
#include "mvp_req_fields.h"
#include "mvp_req.h"
#include "mvp_rndv_utils.h"
#include "mvp_rts.h"

extern MPIDI_MVP_ep_t *flowlist;

void MPIDI_MVP_MRAILI_Process_rndv()
{
    MPIR_Request *sreq;
    MPIDI_MVP_ep_t *pending_flowlist = NULL, *temp_vc = NULL;
    int need_vc_enqueue = 0;

    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_CH3I_PROCESS_RNDV);
    while (flowlist) {
        /* Push on the the first ongoing receive with
         * MPIDI_CH3_Rendezvous_push. If the receive
         * finishes, it will advance the shandle_head
         * pointer on the connection.
         *
         * xxx the side effect of MPIDI_CH3_Rendezvous_push is
         * bad practice. Find a way to do this so the logic
         * is obvious.
         */
#ifdef CKPT
        /*If vc is suspended, ignore this flow and move on*/
        if (flowlist->ch.state != MPIDI_CH3I_VC_STATE_IDLE) {
            POP_FLOWLIST(); /*VC will be push back when state becomes
                               MPIDI_CH3I_VC_STATE_IDLE*/
            continue;
        }
#endif

        //#ifdef _ENABLE_XRC_
        //        if (USE_XRC && VC_XSTS_ISUNSET (flowlist, XF_SMP_VC |
        //                    XF_DPM_INI | XF_SEND_IDLE | XF_UD_CONNECTED)) {
        //            PRINT_DEBUG(DEBUG_XRC_verbose>0, "No conn for RNDV!
        //            0x%08x\n", flowlist->ch.xrc_flags);
        //            MPIDI_CH3I_CM_Connect(flowlist);
        //            POP_FLOWLIST();
        //            continue;
        //        }
        //#endif
        sreq = flowlist->mrail.sreq_head;
        while (sreq != NULL) {
#ifdef CKPT
            if (flowlist->ch.rput_stop &&
                MVP_RNDV_PROTOCOL_RPUT ==
                    MPIDI_MVP_REQUEST(sreq, mrail).protocol) {
                break; /*VC will be push back when the rput_stop becomes 0*/
            }
#endif
            MPIDI_MVP_Rendezvous_push(flowlist, sreq);
            PRINT_DEBUG(DEBUG_RNDV_verbose > 1,
                        "after rndv push, sreq: %p, nearly_complete: %d, "
                        "local_complete: %d, remote_complete: %d\n",
                        sreq, MPIDI_MVP_REQUEST(sreq, mrail).nearly_complete,
                        MPIDI_MVP_REQUEST(sreq, mrail).local_complete,
                        MPIDI_MVP_REQUEST(sreq, mrail).remote_complete);
            if (1 != MPIDI_MVP_REQUEST(sreq, mrail).nearly_complete) {
                break;
            }
            PRINT_DEBUG(DEBUG_RNDV_verbose,
                        "sreq: %p, protocol: %d, "
                        "nearly_complete: %d, local_complete: %d, "
                        "remote_complete: %d\n",
                        sreq, MPIDI_MVP_REQUEST(sreq, mrail).protocol,
                        MPIDI_MVP_REQUEST(sreq, mrail).nearly_complete,
                        MPIDI_MVP_REQUEST(sreq, mrail).local_complete,
                        MPIDI_MVP_REQUEST(sreq, mrail).remote_complete);
            RENDEZVOUS_DONE(flowlist);
            sreq = flowlist->mrail.sreq_head;
        }

        temp_vc = flowlist;
        need_vc_enqueue = 0;
        if (sreq && 1 != MPIDI_MVP_REQUEST(sreq, mrail).nearly_complete) {
            need_vc_enqueue = 1;
        }

        /* now move on to the next connection */
        POP_FLOWLIST();

        if (need_vc_enqueue) {
            ADD_PENDING_FLOWLIST(temp_vc, pending_flowlist);
        }
    }

    while (pending_flowlist) {
        /* push pending vc to the flowlist */
        REMOVE_PENDING_FLOWLIST(temp_vc, pending_flowlist)
        PUSH_FLOWLIST(temp_vc);
    }

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_CH3I_PROCESS_RNDV);
}
