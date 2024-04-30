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

static int MPIDI_MVP_VC_Init(MPIDI_MVP_ep_t *vc)
{
    int mpi_errno = MPI_SUCCESS;

    if (vc == NULL) {
        MPIR_ERR_SETANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**fail", "**fail %s",
                             "MVAPICH SMP endpoint is NULL");
    }
    vc->smp.local_nodes = -1;
    vc->smp.sendq_head = NULL;
    vc->smp.sendq_tail = NULL;
    vc->smp.recv_active = NULL;
    vc->smp.send_active = NULL;
    vc->mrail.sreq_head = NULL;
    vc->mrail.sreq_tail = NULL;
    vc->ch.req = NULL;
    //    vc->mrail.rails = NULL;
    //    vc->mrail.srp.credits = NULL;
    //    vc->mrail.cmanager.msg_channels = NULL;
    vc->ch.sendq_head = NULL;
    vc->ch.sendq_tail = NULL;
    /* vc->ch.state = MPIDI_CH3I_VC_STATE_IDLE; */
    vc->ch.state = MPIDI_MVP_VC_STATE_UNCONNECTED;
    //    vc->ch.read_state = MPIDI_MVP_READ_STATE_IDLE;
    vc->ch.recv_active = NULL;
    vc->ch.send_active = NULL;
    vc->ch.cm_sendq_head = NULL;
    vc->ch.cm_sendq_tail = NULL;
    vc->ch.cm_1sc_sendq_head = NULL;
    vc->ch.cm_1sc_sendq_tail = NULL;
    vc->ch.pending_r3_data = 0;
    vc->ch.received_r3_data = 0;
#ifdef _ENABLE_XRC_
    vc->ch.xrc_flags = 0;
    vc->ch.xrc_conn_queue = NULL;
    vc->ch.orig_vc = NULL;
    memset(vc->ch.xrc_srqn, 0, sizeof(uint32_t) * MAX_NUM_HCAS);
    memset(vc->ch.xrc_rqpn, 0, sizeof(uint32_t) * MAX_NUM_SUBRAILS);
    memset(vc->ch.xrc_my_rqpn, 0, sizeof(uint32_t) * MAX_NUM_SUBRAILS);
#endif

    vc->smp.hostid = -1;
    vc->force_rndv = 0;

    // vc->rndvSend_fn = MPID_MRAIL_RndvSend;
    // vc->rndvRecv_fn = MPID_MRAIL_RndvRecv;

#if defined(CKPT)
    vc->ch.rput_stop = 0;
#endif /* defined(CKPT) */

#ifdef USE_RDMA_UNEX
    vc->ch.unex_finished_next = NULL;
    vc->ch.unex_list = NULL;
#endif

fn_fail:
    return mpi_errno;
}

int MPIDI_MVP_smp_init_vcs()
{
    int mpi_errno = MPI_SUCCESS;
    int i = 0;
    int pg_size;
    MPIDI_MVP_ep_t *vc = NULL;
    MPIR_Comm *comm_ptr;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPIDI_MVP_SMP_INIT_VCS);
    MPIR_FUNC_VERBOSE_ENTER(MPIDI_MVP_SMP_INIT_VCS);

    MPIR_Comm_get_ptr(MPI_COMM_WORLD, comm_ptr);
    pg_size = MPIR_Process.size;
    MPIDI_MVP_vc_table =
        MPL_malloc((sizeof(MPIDI_MVP_ep_t *) * pg_size), MPL_MEM_OTHER);
    for (i = 0; i < pg_size; ++i) {
        vc = MPL_malloc(sizeof(MPIDI_MVP_ep_t), MPL_MEM_OTHER);
        MPIDI_MVP_VC(MPIDIU_comm_rank_to_av(comm_ptr, i)) = vc;
        mpi_errno = MPIDI_MVP_VC_Init(vc);
        MPIR_ERR_CHECK(mpi_errno);
        MPIDI_MVP_vc_table[i] = vc;
        vc->pg_rank = i;
    }

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPIDI_MVP_SMP_INIT_VCS);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
