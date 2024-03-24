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

void MPIDI_MVP_Buffer_copy(const void *const sbuf, MPI_Aint scount,
                           MPI_Datatype sdt, int *smpi_errno, void *const rbuf,
                           MPI_Aint rcount, MPI_Datatype rdt, intptr_t *rsz,
                           int *rmpi_errno)
{
    int sdt_contig;
    int rdt_contig;
    MPI_Aint sdt_true_lb, rdt_true_lb;
    intptr_t sdata_sz;
    intptr_t rdata_sz;
    MPIR_Datatype *sdt_ptr;
    MPIR_Datatype *rdt_ptr;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_SMP_MPI_SEND);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_MPI_SEND);

    *smpi_errno = MPI_SUCCESS;
    *rmpi_errno = MPI_SUCCESS;

    MPIDI_Datatype_get_info(scount, sdt, sdt_contig, sdata_sz, sdt_ptr,
                            sdt_true_lb);
    MPIDI_Datatype_get_info(rcount, rdt, rdt_contig, rdata_sz, rdt_ptr,
                            rdt_true_lb);

    /* --BEGIN ERROR HANDLING-- */
    if (sdata_sz > rdata_sz) {
        sdata_sz = rdata_sz;
        *rmpi_errno =
            MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, __func__,
                                 __LINE__, MPI_ERR_TRUNCATE, "**truncate",
                                 "**truncate %d %d", sdata_sz, rdata_sz);
    }
    /* --END ERROR HANDLING-- */

    if (sdata_sz == 0) {
        *rsz = 0;
        goto fn_exit;
    }

    if (sdt_contig && rdt_contig) {
        MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MEMCPY);
        MPIR_Memcpy((char *)rbuf + rdt_true_lb,
                    (const char *)sbuf + sdt_true_lb, sdata_sz);
        MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MEMCPY);
        *rsz = sdata_sz;
    } else if (sdt_contig) {
        MPI_Aint actual_unpack_bytes;
        MPIR_Typerep_unpack((char *)sbuf + sdt_true_lb, sdata_sz, rbuf, rcount,
                            rdt, 0, &actual_unpack_bytes);
        /* --BEGIN ERROR HANDLING-- */
        if (actual_unpack_bytes != sdata_sz) {
            *rmpi_errno = MPIR_Err_create_code(
                MPI_SUCCESS, MPIR_ERR_RECOVERABLE, __func__, __LINE__,
                MPI_ERR_TYPE, "**dtypemismatch", 0);
        }
        /* --END ERROR HANDLING-- */
        *rsz = actual_unpack_bytes;
    } else if (rdt_contig) {
        MPI_Aint actual_pack_bytes;
        MPIR_Typerep_pack(sbuf, scount, sdt, 0, (char *)rbuf + rdt_true_lb,
                          sdata_sz, &actual_pack_bytes);
        /* --BEGIN ERROR HANDLING-- */
        if (actual_pack_bytes != sdata_sz) {
            *rmpi_errno = MPIR_Err_create_code(
                MPI_SUCCESS, MPIR_ERR_RECOVERABLE, __func__, __LINE__,
                MPI_ERR_TYPE, "**dtypemismatch", 0);
        }
        /* --END ERROR HANDLING-- */
        *rsz = actual_pack_bytes;
    } else {
        char *buf;
        intptr_t sfirst;
        intptr_t rfirst;

        buf = MPL_malloc(MPIDI_COPY_BUFFER_SZ, MPL_MEM_BUFFER);
        /* --BEGIN ERROR HANDLING-- */
        if (buf == NULL) {
            *smpi_errno =
                MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_FATAL, __func__,
                                     __LINE__, MPI_ERR_OTHER, "**nomem", 0);
            *rmpi_errno = *smpi_errno;
            *rsz = 0;
            goto fn_exit;
        }
        /* --END ERROR HANDLING-- */

        sfirst = 0;
        rfirst = 0;

        for (;;) {
            MPI_Aint max_pack_bytes;
            MPI_Aint actual_pack_bytes;
            MPI_Aint actual_unpack_bytes;

            /* rdata_sz is allowed to be larger than sdata_sz, so if
             * we copied everything from the source buffer to the
             * receive buffer, we are done */

            if (sdata_sz - sfirst > MPIDI_COPY_BUFFER_SZ) {
                max_pack_bytes = MPIDI_COPY_BUFFER_SZ;
            } else {
                max_pack_bytes = sdata_sz - sfirst;
            }

            /* nothing left to copy, break out */
            if (max_pack_bytes == 0) {
                break;
            }

            MPIR_Typerep_pack(sbuf, scount, sdt, sfirst, buf, max_pack_bytes,
                              &actual_pack_bytes);
            MPIR_Typerep_unpack(buf, actual_pack_bytes, rbuf, rcount, rdt,
                                rfirst, &actual_unpack_bytes);
            MPIR_Assert(actual_pack_bytes == actual_unpack_bytes);

            sfirst += actual_pack_bytes;
            rfirst += actual_unpack_bytes;

            /* --BEGIN ERROR HANDLING-- */
            if (rfirst == sdata_sz && sfirst != sdata_sz) {
                /* datatype mismatch -- remaining bytes could not be unpacked */
                *rmpi_errno = MPIR_Err_create_code(
                    MPI_SUCCESS, MPIR_ERR_RECOVERABLE, __func__, __LINE__,
                    MPI_ERR_TYPE, "**dtypemismatch", 0);
                break;
            }
            /* --END ERROR HANDLING-- */
        }

        *rsz = rfirst;
        MPL_free(buf);
    }

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_BUFFER_COPY);
    return;
fn_fail:
    goto fn_exit;
}

int MPIDI_MVP_mpi_recv_self(MPIR_Request *rreq, void *buf, MPI_Aint count,
                            MPI_Datatype datatype)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Request *sreq = MPIDI_MVP_REQUEST(rreq, dev.partner_request);

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_SMP_MPI_RECV_SELF);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_MPI_RECV_SELF);

    if (sreq != NULL) {
        intptr_t data_sz;

        MPIDI_MVP_Buffer_copy(MPIDI_MVP_REQUEST(sreq, dev.user_buf),
                              MPIDI_MVP_REQUEST(sreq, dev.user_count),
                              MPIDI_MVP_REQUEST(sreq, dev.datatype),
                              &sreq->status.MPI_ERROR, buf, count, datatype,
                              &data_sz, &rreq->status.MPI_ERROR);

        MPIR_STATUS_SET_COUNT(rreq->status, data_sz);

        MPIR_Request_complete(sreq);
        MPIR_Request_complete(rreq);
    } else {
        MPIR_Request_complete(rreq);
    }

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_MPI_SEND_SELF);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIDI_MVP_mpi_send_self(const void *buf, MPI_Aint count,
                            MPI_Datatype datatype, int rank, int tag,
                            MPIR_Comm *comm, int context_offset,
                            MPIDI_av_entry_t *addr, MPIR_Request **request)
{
    int found = 0;
    MPIR_Request *sreq = NULL;
    MPIR_Request *rreq = NULL;
    MPID_Seqnum_t seqnum;
    MPIDI_Message_match match;
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_SMP_MPI_SEND_SELF);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_MPI_SEND_SELF);

    MPIDI_Request_create_sreq(sreq, mpi_errno, goto fn_exit);
    MPIDI_Request_set_seqnum(sreq, seqnum);
    MPIDI_Request_set_type(sreq, MPIDI_REQUEST_TYPE_SEND);
    MPIDI_Request_set_msg_type(sreq, MPIDI_REQUEST_SELF_MSG);

    match.parts.rank = rank;
    match.parts.tag = tag;
    match.parts.context_id = comm->context_id + context_offset;

    mpi_errno = MPIDI_MVP_smp_recvq_recv(match, &found, &rreq);
    MPIR_ERR_CHECK(mpi_errno);

    /* --BEGIN ERROR HANDLING-- */
    if (rreq == NULL) {
        /* We release the send request twice, once to release the
         * progress engine reference and the second to release the
         * user reference since the user will never have a chance to
         * release their reference. */
        MPIR_Request_complete(sreq);
        MPIR_Request_complete(sreq);
        sreq = NULL;
        MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER, "**send_self");
    }
    /* --END ERROR HANDLING-- */

    /* If the completion counter is 0, that means that the communicator to
     * which this message is being sent has been revoked and we shouldn't
     * bother finishing this. */
    if (!found && MPIR_cc_get(rreq->cc) == 0) {
        /* We release the send request twice, once to release the
         * progress engine reference and the second to release the
         * user reference since the user will never have a chance to
         * release their reference. */
        MPIR_Request_complete(sreq);
        MPIR_Request_complete(sreq);
        sreq = NULL;
        goto fn_exit;
    }

    rreq->status.MPI_SOURCE = rank;
    rreq->status.MPI_TAG = tag;

    if (found) {
        intptr_t data_sz;
        MPIDI_MVP_Buffer_copy(buf, count, datatype, &sreq->status.MPI_ERROR,
                              MPIDI_MVP_REQUEST(rreq, dev.user_buf),
                              MPIDI_MVP_REQUEST(rreq, dev.user_count),
                              MPIDI_MVP_REQUEST(rreq, dev.datatype), &data_sz,
                              &rreq->status.MPI_ERROR);

        MPIR_STATUS_SET_COUNT(rreq->status, data_sz);

        MPIR_Request_complete(sreq);
        MPIR_Request_complete(rreq);
    } else {
        MPI_Aint dt_sz;

        if (!HANDLE_IS_BUILTIN(datatype)) {
            MPIR_Datatype_get_ptr(datatype,
                                  MPIDI_MVP_REQUEST(sreq, dev.datatype_ptr));
            MPIR_Datatype_ptr_add_ref(
                MPIDI_MVP_REQUEST(sreq, dev.datatype_ptr));
        }
        MPIDI_MVP_REQUEST(rreq, dev.partner_request) = sreq;
        MPIDI_MVP_REQUEST(rreq, dev.sender_req_id) = sreq->handle;
        MPIR_Datatype_get_size_macro(datatype, dt_sz);
        MPIR_STATUS_SET_COUNT(rreq->status, count * dt_sz);

        /* Kick the progress engine in case another thread that is
         * performing a blocking recv or probe is waiting in the
         * progress engine */
        MPIDI_MVP_Progress_signal_completion();
        MPIDI_Request_set_msg_type(rreq, MPIDI_REQUEST_SELF_MSG);
    }

fn_exit:
    *request = sreq;

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_MPI_SEND_SELF);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
