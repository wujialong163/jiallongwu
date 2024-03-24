/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda. For detailed
 * copyright and licensing information, please refer to the copyright file
 * COPYRIGHT in the top level MVAPICH directory.
 */

/*
 * define to avoid breaking old implementations when we include the
 * implementation headers that include our overrides.
 * This should come before all includes.
 */
#define _MVP_INTERNAL_DISABLE_OVERRIDES_

#include "mvp_smp_impl.h"
#include "mvp_req_fields.h"
#include "mvp_req.h"
#include "mvp_tagm.h"

/* matching request pools with MPICH */
/* not sure if we really need this one */
#ifndef MPIDI_MVP_SMP_REQUEST_PREALLOC
#define MPIDI_MVP_SMP_REQUEST_PREALLOC MPIR_REQUEST_PREALLOC
#endif

MPIDI_MVP_smp_request_t
    MPIDI_MVP_smp_request_direct[MPIDI_MVP_SMP_REQUEST_PREALLOC];
MPIR_Object_alloc_t MPIDI_MVP_smp_request_mem = {
    0,
    0,
    0,
    0,
    MPIR_INTERNAL,
    sizeof(MPIDI_MVP_smp_request_t),
    MPIDI_MVP_smp_request_direct,
    MPIDI_MVP_SMP_REQUEST_PREALLOC,
    NULL};

void MPIDI_MVP_smp_request_free(MPIR_Request *req)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_MVP_smp_request_t *smp_rreq = MPIDI_MVP_REQUEST_FROM_MPICH(req);

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_SMP_REQUEST_FREE);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_SMP_REQUEST_FREE);

    if (!req->ref_count && req->kind != MPIR_REQUEST_KIND__PREQUEST_SEND &&
        req->kind != MPIR_REQUEST_KIND__PREQUEST_RECV && smp_rreq) {
        if (smp_rreq->dev.datatype_ptr) {
            MPIR_Datatype_ptr_release(smp_rreq->dev.datatype_ptr);
        }
        MPIR_Handle_obj_free(&MPIDI_MVP_smp_request_mem, smp_rreq);
    }
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_SMP_REQUEST_FREE);
    return;
}
