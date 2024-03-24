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

#include "mpidi_ch3_impl.h"
#include "rdma_impl.h"
#include "mem_hooks.h"
#include "mvp_debug_utils.h"
#include "upmi.h"
#include "mvp_ch3_shmem.h"
#include "hwloc_bind.h"
#include "cm.h"

int MPIDI_CH3_Flush()
{
    MPIR_FUNC_VERBOSE_STATE_DECL(MPIDI_CH3_FLUSH);
    MPIR_FUNC_VERBOSE_ENTER(MPIDI_CH3_FLUSH);
#ifdef MPIDI_CH3I_MRAILI_FLUSH
    MPIDI_CH3I_MRAILI_Flush();
#endif
    MPIR_FUNC_VERBOSE_EXIT(MPIDI_CH3_FLUSH);
    return MPI_SUCCESS;
}

int MPIDI_CH3_Finalize()
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPIDI_CH3_FINALIZE);
    MPIR_FUNC_VERBOSE_ENTER(MPIDI_CH3_FINALIZE);

    mvp_is_in_finalize = 1;

    MPL_DBG_MSG_S(MPIDI_CH3_DBG_CHANNEL, VERBOSE, "entering %s", __func__);

    MPIR_T_MVP_cvar_finalize();

#ifdef _ENABLE_CUDA_
    if (mvp_enable_device) {
        device_cleanup();
    }
#endif

#ifdef _ENABLE_XRC_
    xrc_rdmafp_init = 0;
    if (USE_XRC) {
        mpi_errno = UPMI_BARRIER();
        if (mpi_errno)
            MPIR_ERR_POP(mpi_errno);
    }
#endif

    /* Let the lower layer flush out. */
    MPIDI_CH3_Flush();

    if (!SMP_ONLY) {
#ifndef DISABLE_PTMALLOC
        mvapich_mfin();
#endif
        /* allocate rmda memory and set up the queues */
        if (MPIDI_CH3I_Process.cm_type == MPIDI_CH3I_CM_ON_DEMAND) {
            /*FillMe:call MPIDI_CH3I_CM_Finalize here*/
            mpi_errno = MPIDI_CH3I_CM_Finalize();
        }
#ifdef RDMA_CM
        else if (MPIDI_CH3I_Process.cm_type == MPIDI_CH3I_CM_RDMA_CM) {
            /*FillMe:call RDMA_CM's finalization here*/
            mpi_errno = MPIDI_CH3I_RDMA_CM_Finalize();
        }
#endif
        else {
            /*call old init to setup all connections*/
            mpi_errno = MPIDI_CH3I_RDMA_finalize();
        }

        if (mpi_errno) {
            MPIR_ERR_POP(mpi_errno);
        }
#if defined(RDMA_CM)
        if (ip_address_enabled_devices) {
            /* This is used by both rdma_cm code and the mcast code */
            MPL_free(ip_address_enabled_devices);
        }
#endif /*defined(RDMA_CM)*/
    }

#ifdef CKPT
    mpi_errno = MPIDI_CH3I_CR_Finalize();
    if (mpi_errno)
        MPIR_ERR_POP(mpi_errno);
#endif

#ifdef _ENABLE_CUDA_
    DEVICE_COLL_Finalize();
#endif

    /* Shutdown the progress engine */
    mpi_errno = MPIDI_CH3I_Progress_finalize();
    if (mpi_errno)
        MPIR_ERR_POP(mpi_errno);

    MVP_collectives_arch_finalize();

    if (SMP_INIT) {
        mpi_errno = MPIDI_CH3I_SMP_finalize();
        if (mpi_errno)
            MPIR_ERR_POP(mpi_errno);
    }

    if (mvp_enable_shmem_collectives) {
        mpi_errno = MPIDI_CH3I_SMP_COLL_finalize();
        if (mpi_errno) {
            MPIR_ERR_POP(mpi_errno);
        }
    }

    /* Deallocate PMI Key Value Pair */
    mvp_free_pmi_keyval();

    /* Clear functions used for OFED abstraction */
    mvp_dlopen_finalize();

    /* Deallocate hwloc topology and remove corresponding files */
    smpi_destroy_hwloc_topology();

#if defined(_ENABLE_CUDA_)
    /* Release any COLL SRbuf pool storage */
    if (MPIDI_CH3U_COLL_SRBuf_pool) {
        MPIDI_CH3U_COLL_SRBuf_element_t *p, *pNext;
        p = MPIDI_CH3U_COLL_SRBuf_pool;
        while (p) {
            pNext = p->next;
            MVP_MPIDI_Free_Device_Pinned_Host(p->buf);
            MPL_free(p);
            p = pNext;
        }
    }

    /*free cuda resources allocated in Alltoall*/
    MPIR_Alltoall_CUDA_cleanup();

    /* Release any CUDA SRbuf storage */
    if (MPIDI_CH3U_CUDA_SRBuf_pool) {
        MPIDI_CH3U_CUDA_SRBuf_element_t *p, *pNext;
        p = MPIDI_CH3U_CUDA_SRBuf_pool;
        while (p) {
            pNext = p->next;
            MVP_MPIDI_Free_Device(p->buf);
            MPL_free(p);
            p = pNext;
        }
    }
#endif

fn_exit:
    MPL_DBG_MSG_S(MPIDI_CH3_DBG_CHANNEL, VERBOSE, "exiting %s", __func__);
    MPIR_FUNC_VERBOSE_EXIT(MPIDI_CH3_FINALIZE);
    return mpi_errno;

fn_fail:
    /*
     * We need to add "**ch3_finalize" to the list of error messages
     */
    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**mpi_finalize");

    goto fn_exit;
}

/* vi:set sw=4 tw=80: */
