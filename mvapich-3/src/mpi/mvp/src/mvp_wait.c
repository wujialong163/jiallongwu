/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda. For detailed
 * copyright and licensing information, please refer to the copyright file
 * COPYRIGHT in the top level MVAPICH directory.
 */

#include "mpiimpl.h"
#if defined(_SHARP_SUPPORT_)
#include "sharp_type.h"
#endif

int MPIR_MVP_Wait(MPIR_Request *request_ptr, MPI_Status *status)
{
    int mpi_errno = MPI_SUCCESS;

#if defined(_SHARP_SUPPORT_)
    if (MPID_MVP_SHARP_REQUEST(request_ptr) != NULL) {
        mpi_errno = MPID_SHARP_COLL_REQ_WAIT(request_ptr);
        if (mpi_errno != MPID_SHARP_COLL_SUCCESS) {
            PRINT_ERROR("SHARP non-blocking collective failed\n");
            MPID_SHARP_COLL_REQ_FREE(request_ptr);
            /* TODO: Fix lazy error messages here */
            mpi_errno = MPI_ERR_INTERN;
            MPIR_ERR_POP(mpi_errno);
        }
        MPIR_Request_complete(request_ptr);
    }
#endif
fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
