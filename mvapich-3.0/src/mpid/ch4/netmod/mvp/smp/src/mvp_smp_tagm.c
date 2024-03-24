/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda. For detailed
 * copyright and licensing information, please refer to the copyright file
 * COPYRIGHT in the top level MVAPICH directory.
 */
/*
 * define this to avoid breaking the old implementations when we include the
 * implmentation headers that include our overrides
 */
#define _MVP_INTERNAL_DISABLE_OVERRIDES_

#include "mvp_smp_impl.h"
#include "mvp_req_fields.h"
#include "mvp_req.h"
#include "mvp_tagm.h"
#include "mvp_pkt.h"
#include "mvp_util.h"

extern int mvp_smp_init;
/* FIXME : Only here so that the code compiles for the time being. HAS to
 * be removed before rebase */
unsigned long PVAR_LEVEL_unexpected_recvq_buffer_size;

long int mvp_posted_recvq_length = 0;
long int mvp_num_posted_send = 0;
long int mvp_unexp_msg_recv = 0;
long int mvp_num_posted_anysrc = 0;
/* Assuming existence of the MPIR Request structure... */
MPIR_Request *recvq_posted_head = NULL;
MPIR_Request *recvq_posted_tail = NULL;
MPIR_Request *recvq_unexpected_head = NULL;
MPIR_Request *recvq_unexpected_tail = NULL;

#define MVP_ERR_BIT_MASK       0x1
#define MVP_PROC_FAIL_BIT_MASK 0x2

#define MVP_PKT_MASK_ERROR_BITS(_tag, _bits_masked)                            \
    if (MPIR_TAG_CHECK_ERROR_BIT(_tag)) {                                      \
        _bits_masked &= MVP_ERR_BIT_MASK;                                      \
        if (MPIR_TAG_CHECK_PROC_FAILURE_BIT(_tag)) {                           \
            _bits_masked &= MVP_PROC_FAIL_BIT_MASK;                            \
        }                                                                      \
        MPIR_TAG_CLEAR_ERROR_BITS(_tag);                                       \
    }

#define MVP_PKT_UNMASK_ERROR_BITS(_tag, _bits_masked)                          \
    if (_bits_masked & MVP_ERR_BIT_MASK) {                                     \
        MPIR_TAG_SET_ERROR_BIT(_tag);                                          \
        if (_bits_masked & MVP_PROC_FAIL_BIT_MASK) {                           \
            MPIR_TAG_SET_PROC_FAILURE_BIT(_tag);                               \
        }                                                                      \
    }

static inline int MPIDI_MVP_smp_recvq_dequeue_posted(MPIDI_Message_match match,
                                                     int *foundp,
                                                     MPIR_Request **rreq_ptr)
{
    int network_matched;
    MPIR_Request *prev_rreq = NULL;
    MPIR_Request *rreq = recvq_posted_head;
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_SMP_RECVQ_DEQUEUE_POSTED);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_RECVQ_DEQUEUE_POSTED);

    *foundp = 0;
    /* MPIR_T_PVAR_TIMER_START(RECVQ, time_failed_matching_postedq); */
    while (rreq != NULL && !(*foundp)) {
        /* MPIR_T_PVAR_COUNTER_INC(RECVQ, posted_recvq_match_attempts, 1); */
        if (MATCH_WITH_LEFT_RIGHT_MASK((MPIDI_MVP_REQUEST(rreq, dev)).match,
                                       match,
                                       (MPIDI_MVP_REQUEST(rreq, dev)).mask)) {
            if (prev_rreq != NULL) {
                (MPIDI_MVP_REQUEST(prev_rreq, dev)).next =
                    (MPIDI_MVP_REQUEST(rreq, dev)).next;
            } else {
                recvq_posted_head = (MPIDI_MVP_REQUEST(rreq, dev).next);
            }
            if (MPIDI_MVP_REQUEST(rreq, dev.next) == NULL) {
                recvq_posted_tail = prev_rreq;
            }
            /* MPIR_T_PVAR_LEVEL_DEC(RECVQ, posted_recvq_length, 1); */

            /* decrement our anysrc counter if this was anysrc this avoids
             * unneccesary probe calls */
            if (MPI_ANY_SOURCE ==
                MPIDI_MVP_REQUEST(rreq, dev.match.parts.rank)) {
                MVP_DEC_NUM_POSTED_ANYSRC();
            }
            *foundp = 1;
        } else {
            prev_rreq = rreq;
            rreq = MPIDI_MVP_REQUEST(rreq, dev.next);
        }
    }
fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_RECVQ_DEQUEUE_POSTED);
    *rreq_ptr = rreq;
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

static inline int MPIDI_MVP_smp_recvq_dequeue_unexpected(
    MPIDI_Message_match match, int *foundp, MPIR_Request **rreq_ptr)
{
    MPIDI_Message_match mask;
    MPIR_Request *prev_rreq = NULL;
    MPIR_Request *rreq = recvq_unexpected_head;
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_VERBOSE_STATE_DECL(
        MPID_STATE_MPIDI_MVP_SMP_RECVQ_DEQUEUE_UNEXPECTED);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_RECVQ_DEQUEUE_UNEXPECTED);

    /* should only enter here if we know there are unexpected recieves */
    MPIR_Assert(rreq != NULL);

    mask.parts.context_id = mask.parts.rank = mask.parts.tag = ~0;
    /* Mask the error bit that might be set on incoming messages. It is
     * assumed that the local receive operation won't have the error bit set
     * (or it is masked away at some other level). */
    MPIR_TAG_CLEAR_ERROR_BITS(mask.parts.tag);
    /* mask of the tag or source if applicable */
    if (match.parts.tag == MPI_ANY_TAG) {
        match.parts.tag = mask.parts.tag = 0;
    }
    if (match.parts.rank == MPI_ANY_SOURCE) {
        match.parts.rank = mask.parts.rank = 0;
    }
    while (rreq) {
        /* MPIR_T_PVAR_COUNTER_INC(RECVQ,
         * unexpected_recvq_match_attempts, 1); */
        if (MATCH_WITH_LEFT_MASK(MPIDI_MVP_REQUEST(rreq, dev.match), match,
                                 mask)) {
            if (prev_rreq != NULL) {
                MPIDI_MVP_REQUEST(prev_rreq, dev.next) =
                    MPIDI_MVP_REQUEST(rreq, dev.next);
            } else {
                recvq_unexpected_head = MPIDI_MVP_REQUEST(rreq, dev.next);
            }

            if (MPIDI_MVP_REQUEST(rreq, dev.next) == NULL) {
                recvq_unexpected_tail = prev_rreq;
            }

            /* MPIR_T_PVAR_LEVEL_DEC(RECVQ, unexpected_recvq_length, 1); */

            /* TODO: bring back pvars
            if (MPIDI_Request_get_msg_type(rreq) ==
                MPIDI_REQUEST_EAGER_MSG) {
                MPIR_T_PVAR_LEVEL_DEC(RECVQ, unexpected_recvq_buffer_size,
                    (MPIDI_MVP_REQUEST(rreq, dev)).tmpbuf_sz);
            }
            */
            *foundp = 1;
            *rreq_ptr = rreq;
            break;
        }
        prev_rreq = rreq;
        rreq = MPIDI_MVP_REQUEST(rreq, dev.next);
    }
fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_RECVQ_DEQUEUE_UNEXPECTED);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

static inline int MPIDI_MVP_smp_recvq_enqueue_posted(MPIDI_Message_match match,
                                                     MPIR_Comm *comm,
                                                     int local_nodes,
                                                     MPIR_Request **rreq_ptr)
{
    MPIR_Request *rreq = *rreq_ptr;
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_SMP_RECVQ_ENQUEUE_POSTED);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_RECVQ_ENQUEUE_POSTED);

    MPIDI_Request_create_rreq(rreq, mpi_errno, NULL);

    MPIDI_MVP_REQUEST(rreq, dev.match) = match;

    /* Added a mask for faster search on 64-bit capable
     * platforms */
    MPIDI_MVP_REQUEST(rreq, dev.mask.parts.context_id) = ~0;
    MPIDI_MVP_REQUEST(rreq, dev.mask.parts.rank) = ~0;
    MPIDI_MVP_REQUEST(rreq, dev.mask.parts.tag) = ~0;
    if (match.parts.rank == MPI_ANY_SOURCE) {
        MPIDI_MVP_REQUEST(rreq, dev.mask.parts.rank) = 0;
    }
    if (match.parts.tag == MPI_ANY_TAG) {
        MPIDI_MVP_REQUEST(rreq, dev.mask.parts.tag) = 0;
    }

    /* check whether VC has failed, or this is an ANY_SOURCE in a
       failed communicator */
    if (match.parts.rank != MPI_ANY_SOURCE) {
        MPIDI_MVP_ep_t *vc = NULL;
        MPIDI_av_entry_t *av = NULL;
        av = MPIDIU_comm_rank_to_av(comm, match.parts.rank);
        vc = MPIDI_MVP_VC(av);
        if (vc->state == MPIDI_MVP_EP_STATE_MORIBUND) {
            MPIR_ERR_SET1(mpi_errno, MPIX_ERR_PROC_FAILED, "**comm_fail",
                          "**comm_fail %d", vc->pg_rank);
            rreq->status.MPI_ERROR = mpi_errno;
            MPID_Request_complete(rreq);
            /* TODO: failing here may not be correct */
            MPIR_ERR_CHECK(mpi_errno);
        }
        if (mvp_smp_init && local_nodes >= 0) {
            MVP_INC_NUM_POSTED_RECV();
        }
    } else {
        MVP_INC_NUM_POSTED_RECV();
        MVP_INC_NUM_POSTED_ANYSRC();
    }

    MPIDI_MVP_REQUEST(rreq, dev.next) = NULL;
    if (recvq_posted_tail) {
        MPIDI_MVP_REQUEST(recvq_posted_tail, dev.next) = rreq;
    } else {
        recvq_posted_head = rreq;
    }
    recvq_posted_tail = rreq;

    /* MPIR_T_PVAR_LEVEL_INC(RECVQ, posted_recvq_length, 1); */
    MPIDI_POSTED_RECV_ENQUEUE_HOOK(rreq);

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_RECVQ_ENQUEUE_POSTED);
    *rreq_ptr = rreq;
    return mpi_errno;
fn_fail:
    rreq = NULL;
    goto fn_exit;
}

static inline int MPIDI_MVP_smp_recvq_enqueue_unexpected(
    MPIDI_Message_match match, char pkt_bits_masked, MPIR_Request **rreq_ptr)
{
    MPIR_Request *rreq = NULL;
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_RECVQ_ENQUEUE_UNEXPECTED);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_RECVQ_ENQUEUE_UNEXPECTED);

    /* TODO: this macro seems unnecessarily convoluted - is FAIL even used? */
    MPIDI_Request_create_rreq(rreq, mpi_errno, goto fn_fail);
    MPIR_Assert(!mpi_errno);

    MPIDI_MVP_REQUEST(rreq, dev.recv_pending_count) = 1;

    MVP_PKT_UNMASK_ERROR_BITS(match.parts.tag, pkt_bits_masked)

    MPIR_Memcpy(&(MPIDI_MVP_REQUEST(rreq, dev.match)), &match,
                sizeof(MPIDI_Message_match));

    MPIDI_MVP_REQUEST(rreq, dev.next) = NULL;
    if (recvq_unexpected_tail != NULL) {
        MPIDI_MVP_REQUEST(recvq_unexpected_tail, dev.next) = rreq;
    } else {
        recvq_unexpected_head = rreq;
    }
    recvq_unexpected_tail = rreq;

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_RECVQ_ENQUEUE_UNEXPECTED);
    *rreq_ptr = rreq;
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

/*
 * MPIDI_MVP_smp_recvq_recv()
 *
 * Handle the case where the reciever side has recieved a message from the
 * network. We do not know yet whether that request was posted or is unexpected.
 * Locate a request in the posted queue and dequeue it, or allocate a new
 * request and enqueue it in the unexpected queue
 *
 * TODO: does this still apply?
 * Multithread - This routine must be called from within a MSGQUEUE
 * critical section.  If a request is allocated, it must not release
 * the MSGQUEUE until the request is completely valid, as another thread
 * may then find it and dequeue it.
 *
 */
int MPIDI_MVP_smp_recvq_recv(MPIDI_Message_match match, int *foundp,
                             MPIR_Request **rreq_ptr)
{
    char pkt_bits_masked = 0;
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_SMP_RECVQ_RECV);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_RECVQ_RECV);

    /*
     * Unset the error bit if it is set on the incoming packet so we don't
     * have to mask it every time. It will get reset at the end of the loop or
     * before the request is added to the unexpected queue if was set here.
     */
    MVP_PKT_MASK_ERROR_BITS(match.parts.tag, pkt_bits_masked);

    mpi_errno = MPIDI_MVP_smp_recvq_dequeue_posted(match, foundp, rreq_ptr);
    if (!*foundp) {
        mpi_errno = MPIDI_MVP_smp_recvq_enqueue_unexpected(
            match, pkt_bits_masked, rreq_ptr);
    }

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_RECVQ_RECV);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

/*
 * MPIDI_MVP_smp_recvq_post()
 *
 * Hnadle the case where the reciever has posted a recieve. We need to identify
 * if there is already a matching request in the unexpected queue or else post
 * our request in the posted queue for when the request arrives (handled in the
 * recv function)
 * Atomically find a request in the unexpected queue and dequeue it, or
 * allocate a new request and enqueue it in the posted queue
 *
 * TODO: does this still apply?
 * Multithread - This routine must be called from within a MSGQUEUE
 * critical section.  If a request is allocated, it must not release
 * the MSGQUEUE until the request is completely valid, as another thread
 * may then find it and dequeue it.
 *
 * This routine is used in mpid_irecv and mpid_recv.
 *
 */
int MPIDI_MVP_smp_recvq_post(int source, int tag, int context_id,
                             MPIR_Comm *comm, void *user_buf,
                             MPI_Aint user_count, MPI_Datatype datatype,
                             int *foundp, int local_nodes,
                             MPIR_Request **rreq_ptr)
{
    int found = 0;
    MPIR_Request *rreq = *rreq_ptr;
    MPIDI_Message_match match;
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_SMP_RECVQ_POST);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_RECVQ_POST);

    /* setup a match object with the information we recieved */
    match.parts.context_id = context_id;
    match.parts.tag = tag;
    match.parts.rank = source;

    /* Store how much time is spent traversing the queue */
    /* MPIR_T_PVAR_TIMER_START(RECVQ, time_matching_unexpectedq); */

    if (recvq_unexpected_head) {
        mpi_errno =
            MPIDI_MVP_smp_recvq_dequeue_unexpected(match, &found, &rreq);
        MPIR_ERR_CHECK(mpi_errno);
    }
    if (!found) {
        /* matching request was not found in the unexepected queue */
        mpi_errno =
            MPIDI_MVP_smp_recvq_enqueue_posted(match, comm, local_nodes, &rreq);
        MPIR_ERR_CHECK(mpi_errno);
    }

    MPIR_Assert(rreq != NULL);

    rreq->comm = comm;
    MPIR_Comm_add_ref(comm);
    MPIDI_MVP_REQUEST(rreq, dev.user_buf) = user_buf;
    MPIDI_MVP_REQUEST(rreq, dev.user_count) = user_count;
    MPIDI_MVP_REQUEST(rreq, dev.datatype) = datatype;
    MPIDI_MVP_REQUEST(rreq, dev.msg_offset) = 0;

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_RECVQ_POST);
    *foundp = found;
    *rreq_ptr = rreq;
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIDI_MVP_smp_recvq_delete_posted(MPIR_Request *rreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Request *prev_rreq = NULL;
    MPIR_Request *qreq = recvq_posted_head;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_RECVQ_CANCEL_POSTED);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_RECVQ_CANCEL_POSTED);

    while (qreq != NULL && qreq != rreq) {
        /* MPIR_T_PVAR_COUNTER_INC(RECVQ, posted_recvq_match_attempts, 1); */
        prev_rreq = qreq;
        qreq = MPIDI_MVP_REQUEST(qreq, dev.next);
    }
    if (!qreq) {
        goto fn_exit;
    }
    /* We shouldn't be here unless we know this request was posted */
    MPIR_Assert(qreq == rreq);
    if (prev_rreq != NULL) {
        MPIDI_MVP_REQUEST(prev_rreq, dev.next) =
            MPIDI_MVP_REQUEST(rreq, dev.next);
    } else {
        recvq_posted_head = MPIDI_MVP_REQUEST(rreq, dev.next);
    }
    if (MPIDI_MVP_REQUEST(rreq, dev.next) == NULL) {
        recvq_posted_tail = prev_rreq;
    }
    /* MPIR_T_PVAR_LEVEL_DEC(RECVQ, posted_recvq_length, 1); */

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_RECVQ_CANCEL_POSTED);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIDI_MVP_smp_recvq_probe_anysrc(int *foundp)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Request *prev_rreq = NULL;
    MPIR_Request *rreq = recvq_posted_head;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_SMP_RECVQ_PROBE_ANYSRC);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_RECVQ_PROBE_ANYSRC);

    /* should not be here if we didn't know we had pending anysrc recvs */
    MPIR_Assert(mvp_num_posted_anysrc);

    *foundp = 0;
    /* MPIR_T_PVAR_TIMER_START(RECVQ, time_failed_matching_postedq); */
    while (rreq != NULL && !(*foundp)) {
        /* this is an anysrc recv, probe the network */
        if (MPI_ANY_SOURCE == MPIDI_MVP_REQUEST(rreq, dev.match.parts.rank)) {
            MPI_Status pstatus;
            MPIR_Request *new_req = NULL;
            int context_offset =
                MPIDI_MVP_REQUEST(rreq, dev.match.parts.context_id) -
                rreq->comm->recvcontext_id;
            mpi_errno = MPIDI_NM_mpi_improbe(
                MPI_ANY_SOURCE, MPIDI_MVP_REQUEST(rreq, dev.match.parts.tag),
                rreq->comm, context_offset, NULL, foundp, &new_req, &pstatus);
            MPIR_ERR_CHECK(mpi_errno);
            /* there is a request waiting on the network, recv it */
            if (*foundp) {
                MPIDI_av_entry_t *av =
                    MPIDIU_comm_rank_to_av(rreq->comm, pstatus.MPI_SOURCE);
                /* remove the request from our shared memory queue */
                if (prev_rreq != NULL) {
                    (MPIDI_MVP_REQUEST(prev_rreq, dev)).next =
                        (MPIDI_MVP_REQUEST(rreq, dev)).next;
                } else {
                    recvq_posted_head = MPIDI_MVP_REQUEST(rreq, dev.next);
                }
                if (MPIDI_MVP_REQUEST(rreq, dev.next) == NULL) {
                    recvq_posted_tail = prev_rreq;
                }
                MVP_DEC_NUM_POSTED_ANYSRC();
                MPIR_Assert(mvp_num_posted_anysrc >= 0);

                /* recv the network request */
                MPIR_Assert(new_req->kind == MPIR_REQUEST_KIND__MPROBE);
                new_req->kind = MPIR_REQUEST_KIND__RECV;
                mpi_errno = MPIDI_NM_mpi_imrecv(
                    MPIDI_MVP_REQUEST(rreq, dev.user_buf),
                    MPIDI_MVP_REQUEST(rreq, dev.user_count),
                    MPIDI_MVP_REQUEST(rreq, dev.datatype), new_req);
                MPIR_ERR_CHECK(mpi_errno);
                while (!MPIR_Request_is_complete(new_req)) {
                    MPID_Wait(new_req, NULL);
                }
                MPIR_Assert(MPIR_Request_is_complete(new_req));
                /* copy over our completed status to the existing request */
                MPIR_Request_extract_status(new_req, &(rreq->status));
                MPIR_Request_free_unsafe(new_req);
                MPID_Request_complete(rreq);
                break;
            }
        }
        prev_rreq = rreq;
        rreq = MPIDI_MVP_REQUEST(rreq, dev.next);
    }
fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_RECVQ_PROBE_ANYSRC);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

/* returns the number of elements in the unexpected queue */
int MPIDI_MVP_Recvq_count_unexp(void)
{
    int count = 0;
    MPIR_Request *req = recvq_unexpected_head;

    while (req) {
        ++count;
        req = MPIDI_MVP_REQUEST(req, dev).next;
    }

    return count;
}
