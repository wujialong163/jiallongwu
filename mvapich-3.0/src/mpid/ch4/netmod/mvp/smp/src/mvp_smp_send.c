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
#include "mvp_req.h"
#include "mvp_tagm.h"
#include "mvp_rts.h"

#if !defined(MPIDI_COPY_BUFFER_SZ)
#define MPIDI_COPY_BUFFER_SZ 16384
#endif

int MPIDI_MVP_RndvSend(MPIR_Request **sreq_p, const void *buf, MPI_Aint count,
                       MPI_Datatype datatype, int dt_contig, intptr_t data_sz,
                       MPI_Aint dt_true_lb, int rank, int tag, MPIR_Comm *comm,
                       int context_offset);

int MPIDI_MVP_smp_eager_send(MPIDI_MVP_ep_t *vc, const void *buf,
                             MPI_Aint count, MPI_Datatype datatype,
                             intptr_t data_sz, int dt_contig,
                             MPI_Aint dt_true_lb, int rank, int tag,
                             MPIR_Comm *comm, int context_offset,
                             MPIR_Request **sreq_p);

int MPIDI_MVP_mpi_send_self(const void *buf, MPI_Aint count,
                            MPI_Datatype datatype, int rank, int tag,
                            MPIR_Comm *comm, int context_offset,
                            MPIDI_av_entry_t *addr, MPIR_Request **request);

int MPIDI_MVP_smp_mpi_send(const void *buf, MPI_Aint count,
                           MPI_Datatype datatype, int rank, int tag,
                           MPIR_Comm *comm, int context_offset,
                           MPIDI_av_entry_t *addr, MPIR_Request **request)
{
    intptr_t data_sz;
    int dt_contig;
    MPI_Aint dt_true_lb;
    MPIR_Datatype *dt_ptr;
    MPIR_Request *sreq = NULL;
    MPIDI_av_entry_t *av;
    MPIDI_MVP_ep_t *vc;
#if defined(MPID_USE_SEQUENCE_NUMBERS)
    MPID_Seqnum_t seqnum;
#endif
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_SMP_MPI_SEND);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_MPI_SEND);

    /* TODO: Replace or reimplement these bucket macros */
    /* MPIR_T_PVAR_COUNTER_BUCKET_INC(MVP,mvp_pt2pt_mpid_send,count,datatype);
     */
    /* Check to make sure the communicator hasn't already been revoked */
    if (comm->revoked &&
        MPIR_AGREE_TAG != MPIR_TAG_MASK_ERROR_BITS(tag & ~MPIR_TAG_COLL_BIT) &&
        MPIR_SHRINK_TAG != MPIR_TAG_MASK_ERROR_BITS(tag & ~MPIR_TAG_COLL_BIT)) {
        MPIR_ERR_SETANDJUMP(mpi_errno, MPIX_ERR_REVOKED, "**revoked");
    }

    if ((comm->rank == rank) &&
        (comm->comm_kind != MPIR_COMM_KIND__INTERCOMM)) {
        mpi_errno =
            MPIDI_MVP_mpi_send_self(buf, count, datatype, rank, tag, comm,
                                    context_offset, addr, request);
        MPIR_ERR_CHECK(mpi_errno);
        goto fn_exit;
    }

    av = MPIDIU_comm_rank_to_av(comm, rank);
    vc = MPIDI_MVP_VC(av);

    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr,
                            dt_true_lb);

    if (likely(!vc->force_rndv) &&
        (data_sz + sizeof(MPIDI_MVP_Pkt_eager_send_t) <= MVP_SMP_EAGERSIZE)) {
        /* eager send */
        mpi_errno = MPIDI_MVP_smp_eager_send(vc, buf, count, datatype, data_sz,
                                             dt_contig, dt_true_lb, rank, tag,
                                             comm, context_offset, &sreq);
        MPIR_ERR_CHECK(mpi_errno);
        *request = sreq;
    } else {
        /* rndv send */
        MPIDI_Request_create_sreq(sreq, mpi_errno, NULL);
        MPIDI_Request_set_type(sreq, MPIDI_REQUEST_TYPE_SEND);
        *request = sreq;
        /* TODO: refactor this function still */
        mpi_errno =
            MPIDI_MVP_RndvSend(&sreq, buf, count, datatype, dt_contig, data_sz,
                               dt_true_lb, rank, tag, comm, context_offset);
        /* Note that we don't increase the ref count on the datatype
           because this is a blocking call, and the calling routine
           must wait until sreq completes */
    }

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_MPI_SEND);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
