/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda. For detailed
 * copyright and licensing information, please refer to the copyright file
 * COPYRIGHT in the top level MVAPICH directory.
 */

#ifndef _MVP_VC_UTILS_H_
#define _MVP_VC_UTILS_H_

#ifndef MVP_SMP_IMPL_INCLUDED
#error "Requres mvp_smp_impl.h, include that header first"
#endif

static inline int MPIDI_MVP_smp_init_vc(MPIDI_MVP_ep_t *vc)
{
    /* initialize RNDV parameters */
    vc->mrail.sreq_head = NULL;
    vc->mrail.sreq_tail = NULL;
    vc->mrail.nextflow = NULL;
    vc->mrail.inflow = 0;

    return MPI_SUCCESS;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_MVP_smp_setup_vcs(void)
{
    int mpi_errno = MPI_SUCCESS;
    int i = 0;
    int pg_size = MPIR_Process.size;
    MPIDI_MVP_ep_t *vc = NULL;
    MPIR_Comm *comm_ptr;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_SMP_SETUP_VCS);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_SETUP_VCS);

    if (!mvp_smp_init) {
        goto fn_exit;
    }

    MPIR_Comm_get_ptr(MPI_COMM_WORLD, comm_ptr);
    for (i = 0; i < pg_size; ++i) {
        vc = MPIDI_MVP_VC(MPIDIU_comm_rank_to_av(comm_ptr, i));
        /* Mark the SMP VC as Idle */
        if (vc->smp.local_nodes >= 0) {
            vc->ch.state = MPIDI_MVP_VC_STATE_IDLE;
            /* Enable fast send */
            if (MVP_USE_EAGER_FAST_SEND) {
                vc->use_eager_fast_fn = 1;
                vc->eager_fast_max_msg_sz = MVP_SMP_EAGERSIZE;
            }
            if (mvp_smp_only) {
                MPIDI_MVP_smp_init_vc(vc);
            }
        }
    }
fn_exit:
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_SETUP_VCS);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
#endif /* _MVP_VC_UTILS_H_ */
