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

#include "mpid_nem_impl.h"
#undef utarray_oom
#define utarray_oom() do { goto fn_oom; } while (0)
#include "utarray.h"
/* MVP nemesis collective unification */
#include "mvp_ch3_shmem.h"

#define NULL_CONTEXT_ID -1

int MPIDI_CH3I_comm_create(MPIR_Comm *comm, void *param)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_CH3I_COMM_CREATE);

    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_CH3I_COMM_CREATE);

#ifdef _OSU_MVAPICH_
    if (MVP_USE_OSU_COLLECTIVES && comm->comm_kind == MPID_INTRACOMM) {
        comm->coll_fns->Barrier = MPIR_Barrier_MVP;
        comm->coll_fns->Bcast = MPIR_Bcast_MVP;
        comm->coll_fns->Gather = MPIR_Gather_MVP;
        comm->coll_fns->Gatherv = MPIR_Gatherv_MVP;
        comm->coll_fns->Scatter = MPIR_Scatter_MVP;
        comm->coll_fns->Scatterv = MPIR_Scatterv;
        comm->coll_fns->Allgather = MPIR_Allgather_MVP;
        comm->coll_fns->Allgatherv = MPIR_Allgatherv_MVP;
        comm->coll_fns->Alltoall = MPIR_Alltoall_MVP;
        comm->coll_fns->Alltoallv = MPIR_Alltoallv_MVP;
        comm->coll_fns->Alltoallw = MPIR_Alltoallw;
        comm->coll_fns->Reduce = MPIR_Reduce_MVP;
        comm->coll_fns->Allreduce = MPIR_Allreduce_MVP;
        comm->coll_fns->Reduce_scatter = MPIR_Reduce_scatter_MVP;
        comm->coll_fns->Scan = MPIR_Scan;
    }
    MPIR_pof2_comm(comm, comm->local_size, comm->rank);
#endif /* _OSU_MVAPICH_ */

 fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_CH3I_COMM_CREATE);
    return mpi_errno;
 fn_fail:
    goto fn_exit;
}

int MPIDI_CH3I_comm_destroy(MPIR_Comm *comm, void *param)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_CH3I_COMM_DESTROY);

    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_CH3I_COMM_DESTROY);
    
 fn_exit: ATTRIBUTE((unused))
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_CH3I_COMM_DESTROY);
    return mpi_errno;
}

static int nem_coll_finalize(void *param ATTRIBUTE((unused)))
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_NEM_COLL_FINALIZE);

    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_NEM_COLL_FINALIZE);

 fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_NEM_COLL_FINALIZE);
    return mpi_errno;
 fn_fail:
    goto fn_exit;
}


int MPID_nem_coll_init(void)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPID_NEM_COLL_INIT);

    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPID_NEM_COLL_INIT);

    MPIR_Add_finalize(nem_coll_finalize, NULL, MPIR_FINALIZE_CALLBACK_PRIO-1);
    
 fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPID_NEM_COLL_INIT);
    return mpi_errno;
 fn_oom: /* out-of-memory handler for utarray operations */
    MPIR_ERR_SET1(mpi_errno, MPI_ERR_OTHER, "**nomem", "**nomem %s", "utarray");
 fn_fail:
    goto fn_exit;
}

