/* -*- Mode: C; c-basic-offset:4 ; -*- */
/* Copyright (c) 2001-2024, The Ohio State University. All rights
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
 *
 *  (C) 2001 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include <unistd.h>
#include "mvp_coll_shmem.h"
#include <unistd.h>

extern unsigned long long PVAR_COUNTER_mvp_coll_scan_algo;

int MPIR_Scan_MVP(const void *sendbuf, void *recvbuf, int count,
                  MPI_Datatype datatype, MPI_Op op, MPIR_Comm *comm_ptr,
                  MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_scan_algo, 1);

    mpi_errno =
        MPIR_Scan(sendbuf, recvbuf, count, datatype, op, comm_ptr, errflag);
    MPIR_ERR_CHECK(mpi_errno);

    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
