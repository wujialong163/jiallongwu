/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda. For detailed
 * copyright and licensing information, please refer to the copyright file
 * COPYRIGHT in the top level MVAPICH directory.
 */

#include "mvp_smp_impl.h"
#include "mvp_req_fields.h"

extern MPIR_Request *recvq_unexpected_head;
extern MPIR_Request *recvq_unexpected_tail;

/* tag matching routine used only by iprobe function */
MPL_STATIC_INLINE_PREFIX int MPIDI_MVP_smp_probe_recvq(int source, int tag,
                                                       int context_id,
                                                       int *foundp,
                                                       MPIR_Request **message,
                                                       MPI_Status *s)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_Message_match match, mask;
    MPIR_Request *rreq, *prev_rreq = NULL;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_SMP_PROBE_RECVQ);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_PROBE_RECVQ);

    *foundp = 0;
    rreq = recvq_unexpected_head;

    if (!rreq) {
        /* can't check an empty queue */
        goto fn_exit;
    }

    match.parts.context_id = context_id;
    match.parts.tag = tag;
    match.parts.rank = source;

    mask.parts.context_id = mask.parts.rank = mask.parts.tag = ~0;
    MPIR_TAG_CLEAR_ERROR_BITS(mask.parts.tag);
    /* mask off the tag or source if applicable */
    if (match.parts.tag == MPI_ANY_TAG) {
        match.parts.tag = mask.parts.tag = 0;
    }
    if (match.parts.rank == MPI_ANY_SOURCE) {
        match.parts.rank = mask.parts.rank = 0;
    }
    while (rreq) {
        if (MATCH_WITH_LEFT_MASK(MPIDI_MVP_REQUEST(rreq, dev.match), match,
                                 mask)) {
            /* return info about this matched request */
            if (s != MPI_STATUS_IGNORE) {
                s->MPI_SOURCE = rreq->status.MPI_SOURCE;
                s->MPI_TAG = rreq->status.MPI_TAG;
                MPIR_STATUS_SET_COUNT(*s, MPIR_STATUS_GET_COUNT(rreq->status));
                MPIR_STATUS_SET_CANCEL_BIT(
                    *s, MPIR_STATUS_GET_CANCEL_BIT(rreq->status));
            }
            /* this is a matched probe and it needs to dequeue the request */
            if (message) {
                *message = rreq;
                MPIR_Request_add_ref(rreq);
                if (prev_rreq != NULL) {
                    MPIDI_MVP_REQUEST(prev_rreq, dev.next) =
                        MPIDI_MVP_REQUEST(rreq, dev.next);
                } else {
                    recvq_unexpected_head = MPIDI_MVP_REQUEST(rreq, dev.next);
                }
                if (MPIDI_MVP_REQUEST(rreq, dev.next) == NULL) {
                    recvq_unexpected_tail = prev_rreq;
                }
            }
            *foundp = 1;
            break;
        }
        prev_rreq = rreq;
        rreq = MPIDI_MVP_REQUEST(rreq, dev.next);
    }

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_PROBE_RECVQ);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIDI_MVP_smp_mpi_improbe(int source, int tag, MPIR_Comm *comm,
                              int context_offset, MPIDI_av_entry_t *addr,
                              int *flag, MPIR_Request **message,
                              MPI_Status *status)
{
    int mpi_errno = MPI_SUCCESS;
    int context_id = comm->recvcontext_id + context_offset;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_SMP_MPI_IPROBE);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_MPI_IPROBE);

    mpi_errno = MPIDI_MVP_smp_probe_recvq(source, tag, context_id, flag,
                                          message, status);

    if (*message && *flag) {
        (*message)->kind = MPIR_REQUEST_KIND__MPROBE;
        (*message)->comm = comm;
        MPIR_Comm_add_ref(comm);
    }
    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_MPI_IPROBE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIDI_MVP_smp_mpi_iprobe(int source, int tag, MPIR_Comm *comm,
                             int context_offset, MPIDI_av_entry_t *addr,
                             int *flag, MPI_Status *status)
{
    int mpi_errno = MPI_SUCCESS;
    int context_id = comm->recvcontext_id + context_offset;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_SMP_MPI_IPROBE);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_MPI_IPROBE);

    mpi_errno =
        MPIDI_MVP_smp_probe_recvq(source, tag, context_id, flag, NULL, status);
    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_MPI_IPROBE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
