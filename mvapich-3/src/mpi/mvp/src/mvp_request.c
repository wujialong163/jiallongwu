/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda. For detailed
 * copyright and licensing information, please refer to the copyright file
 * COPYRIGHT in the top level MVAPICH directory.
 */

#include "mpiimpl.h"

int MPIR_MVP_Request_coll_init(MPIR_Comm *comm, MPIR_Request **request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Request *req;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIR_MVP_REQUEST_INIT);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIR_MVP_REQUEST_INIT);

    *request = NULL;

    /* create/populate request object */
    req = MPIR_Request_create(MPIR_REQUEST_KIND__COLL);
    if (!req) {
        MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER, "**nomem");
    }
    /* add a reference to the communicator */
    MPIR_Comm_add_ref(comm);
    req->comm = comm;
    /*
     * add a second reference to the communicator so that we don't release it
     * too early. I think technically we can consider this a reference for the
     * progress engine?
     */
    MPIR_Request_add_ref(req);

    *request = req;

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIR_MVP_REQUEST_INIT);
    return mpi_errno;
fn_fail:
    if (*request) {
        *request = NULL;
    }
    if (req) {
        MPIR_Request_free(req);
    }
    goto fn_exit;
}
