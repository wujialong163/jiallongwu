/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights
 * reserved.
 *
 * This file is part of the MVAPICH software package developed by the
 * team members of The Ohio State University's Network-Based Computing
 * Laboratory (NBCL), headed by Professor Dhabaleswar K. (DK) Panda.
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level MVAPICH directory.
 */

/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#include "mpid_nem_impl.h"
#ifdef _OSU_MVAPICH_
#include "mvp_ch3_shmem.h"
#endif /* _OSU_MVAPICH_ */

int MPIDI_CH3_Finalize(void)
{
    extern int finalize_coll_comm;
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_CH3_FINALIZE);

    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_CH3_FINALIZE);

    mpi_errno = MPIDI_CH3I_Progress_finalize();
    MPIR_ERR_CHECK(mpi_errno);
    
    mpi_errno = MPID_nem_finalize();
    if (mpi_errno) MPIR_ERR_POP (mpi_errno);

#ifdef _OSU_MVAPICH_
    if (mvp_enable_shmem_collectives || finalize_coll_comm == 1) {
        /* Freeing up shared memory collective resources*/
        mpi_errno = MPIR_MVP_SHMEM_COLL_finalize(MPID_nem_mem_region.local_rank,
                        MPID_nem_mem_region.num_local);
        if (mpi_errno) MPIR_ERR_POP (mpi_errno);

        MVP_collectives_arch_finalize();
    }
#endif

 fn_fail:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_CH3_FINALIZE);
    return mpi_errno;
}
