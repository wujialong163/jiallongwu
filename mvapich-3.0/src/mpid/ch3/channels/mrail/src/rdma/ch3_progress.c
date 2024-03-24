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

#include "mpidi_ch3_impl.h"
#include "rdma_impl.h"
#include "mpidrma.h"
#ifdef _ENABLE_XRC_
#include "cm.h"
#endif
#if defined(_MCST_SUPPORT_)
#include "ibv_mcast.h"
#endif
#include "ibv_send_inline.h"

/*
=== BEGIN_MPI_T_MVP_CVAR_INFO_BLOCK ===

cvars:
    - name        : MVP_POLLING_LEVEL
      category    : CH3
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Defines the frequency at which the IB device is polled

=== END_MPI_T_MVP_CVAR_INFO_BLOCK ===
*/

#ifdef DEBUG
#include "upmi.h"
#define DEBUG_PRINT(args...)                                                   \
    do {                                                                       \
        int rank;                                                              \
        UPMI_GET_RANK(&rank);                                                  \
        fprintf(stderr, "[%d][%s:%d] ", rank, __FILE__, __LINE__);             \
        fprintf(stderr, args);                                                 \
        fflush(stderr);                                                        \
    } while (0)
#else
#define DEBUG_PRINT(args...)
#endif

#define MVP_UD_SEND_ACKS()                                                     \
    {                                                                          \
        int i;                                                                 \
        MPIDI_VC_t *vc;                                                        \
        int size = MPIDI_PG_Get_size(MPIDI_Process.my_pg);                     \
        for (i = 0; i < size; i++) {                                           \
            MPIDI_PG_Get_vc(MPIDI_Process.my_pg, i, &vc);                      \
            if (vc->mrail.ack_need_tosend) {                                   \
                mvp_send_explicit_ack(vc);                                     \
            }                                                                  \
        }                                                                      \
    }

#define MAX_PROGRESS_HOOKS 4
long int mvp_num_posted_send = 0;
long int mvp_unexp_msg_recv = 0;

typedef int (*progress_func_ptr_t)(int *made_progress);

typedef struct progress_hook_slot {
    progress_func_ptr_t func_ptr;
    int active;
} progress_hook_slot_t;

static int handle_read_individual(MPIDI_VC_t *vc, vbuf *buffer,
                                  int *header_type);

static int cm_handle_pending_send();

extern int MPIDI_CH3_PktHandler_Init_MVP();

extern volatile int *rdma_cm_iwarp_msg_count;
extern volatile int *rdma_cm_connect_count;

#ifdef CKPT
static int cm_handle_reactivation_complete();
static int cm_send_pending_msg(MPIDI_VC_t *vc);
static int cm_send_pending_1sc_msg(MPIDI_VC_t *vc);
#endif

volatile unsigned int MPIDI_CH3I_progress_completion_count = 0;
#if (MPICH_THREAD_LEVEL == MPI_THREAD_MULTIPLE)
volatile int MPIDI_CH3I_progress_blocked = FALSE;
volatile int MPIDI_CH3I_progress_wakeup_signalled = FALSE;
MPID_Thread_cond_t MPIDI_CH3I_progress_completion_cond;
#endif

extern unsigned long PVAR_COUNTER_mpit_progress_poll;
extern unsigned long PVAR_COUNTER_rdma_ud_retransmissions;

inline static int MPIDI_CH3I_Seq(int type)
{
    switch (type) {
        case MPIDI_CH3_PKT_EAGER_SEND:
        case MPIDI_CH3_PKT_READY_SEND:
        case MPIDI_CH3_PKT_EAGER_SYNC_SEND:
        case MPIDI_CH3_PKT_RNDV_REQ_TO_SEND:
        case MPIDI_CH3_PKT_RNDV_READY_REQ_TO_SEND:
        case MPIDI_CH3_PKT_RNDV_CLR_TO_SEND:
        case MPIDI_CH3_PKT_PACKETIZED_SEND_START:
        case MPIDI_CH3_PKT_PACKETIZED_SEND_DATA:
        case MPIDI_CH3_PKT_RNDV_R3_DATA:
        case MPIDI_CH3_PKT_RNDV_R3_ACK:
#ifndef MVP_DISABLE_HEADER_CACHING
        case MPIDI_CH3_PKT_FAST_EAGER_SEND:
        case MPIDI_CH3_PKT_FAST_EAGER_SEND_WITH_REQ:
#endif
        case MPIDI_CH3_PKT_CANCEL_SEND_REQ:
        case MPIDI_CH3_PKT_CANCEL_SEND_RESP:
        case MPIDI_CH3_PKT_PUT:
        case MPIDI_CH3_PKT_PUT_IMMED:
        case MPIDI_CH3_PKT_GET:
        case MPIDI_CH3_PKT_GET_RESP:
        case MPIDI_CH3_PKT_ACCUMULATE:
        case MPIDI_CH3_PKT_ACCUMULATE_IMMED:
        case MPIDI_CH3_PKT_LOCK:
        case MPIDI_CH3_PKT_LOCK_ACK:
        case MPIDI_CH3_PKT_LOCK_OP_ACK:
        case MPIDI_CH3_PKT_ACK:
        case MPIDI_CH3_PKT_DECR_AT_COUNTER:
        case MPIDI_CH3_PKT_PUT_RNDV:
        case MPIDI_CH3_PKT_ACCUMULATE_RNDV:
        case MPIDI_CH3_PKT_GET_ACCUM_RNDV:
        case MPIDI_CH3_PKT_GET_RNDV:
        case MPIDI_CH3_PKT_RMA_RNDV_CLR_TO_SEND:
            return 1;
    }

    return 0;
}

#if (MPICH_THREAD_LEVEL == MPI_THREAD_MULTIPLE)
void MPIDI_CH3I_Progress_wakeup(void) {}
#endif

int MPIDI_CH3_Connection_terminate(MPIDI_VC_t *vc)
{
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_CH3_MRAIL_CONNECTION_TERMINATE);
    /* There is no post_close for shm connections so
     * handle them as closed immediately. */
    int mpi_errno = MPIDI_CH3U_Handle_connection(vc, MPIDI_VC_EVENT_TERMINATED);

    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }

fn_fail:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_CH3_MRAIL_CONNECTION_TERMINATE);
    return mpi_errno;
}

#ifndef MPIDI_CH3_Progress_start
void MPIDI_CH3_Progress_start(MPID_Progress_state *state)
{
    /* MT - This function is empty for the single-threaded implementation */
}
#endif

int MPIDI_CH3I_Progress(int is_blocking, MPID_Progress_state *state)
{
    MPIDI_VC_t *vc_ptr = NULL;
    int mpi_errno = MPI_SUCCESS;
    int spin_count = 0, err = 0, i = 0;
    unsigned completions = MPIDI_CH3I_progress_completion_count;
    vbuf *buffer = NULL;
    int rdmafp_found = 0;
    int smp_completions, smp_found;
    int made_progress = FALSE;
#ifdef _ENABLE_UD_
    static int nspin = 0;
#endif

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_CH3_MRAIL_PROGRESS);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_CH3_MRAIL_PROGRESS);

    MPL_DBG_MSG_S(MPIDI_CH3_DBG_CHANNEL, VERBOSE, "entering %s", __func__);
    MPL_DBG_MSG_S(MPIDI_CH3_DBG_CHANNEL, VERBOSE, "blocking=%s",
                  is_blocking ? "true" : "false");

#ifdef CKPT
    MPIDI_CH3I_CR_lock();
#endif

    do {
    start_polling:
        MPIR_T_PVAR_COUNTER_INC(MVP, mpit_progress_poll, 1);
        smp_completions = MPIDI_CH3I_progress_completion_count;
        smp_found = 0;
        /*needed if early send complete does not occur */
        if (SMP_INIT) {
            if (mvp_posted_recvq_length || mvp_unexp_msg_recv) {
                mpi_errno = MPIDI_CH3I_SMP_read_progress(MPIDI_Process.my_pg);
                if (mpi_errno != MPI_SUCCESS) {
                    MPIR_ERR_POP(mpi_errno);
                }
            }
            if (mvp_num_posted_send) {
                if ((mpi_errno = MPIDI_CH3I_SMP_write_progress(
                         MPIDI_Process.my_pg)) != MPI_SUCCESS) {
                    MPIR_ERR_POP(mpi_errno);
                }
            } else {
                MPIR_Assert(mvp_num_queued_smp_ops == 0);
            }
            if (smp_completions != MPIDI_CH3I_progress_completion_count) {
                break;
            }
        }

#ifdef CKPT
        if (MPIDI_CH3I_Process.reactivation_complete) {
            /*Some channel has been reactivated*/
            // MPIDI_CH3I_Process.reactivation_complete = 0;
            cm_handle_reactivation_complete();
        }
#endif

        if (!SMP_ONLY) {
            cq_poll_completion = 0;
            rdmafp_found = 0;
            mpi_errno = MPIDI_CH3I_read_progress(&vc_ptr, &buffer,
                                                 &rdmafp_found, is_blocking);
            if (mpi_errno != MPI_SUCCESS) {
                MPIR_ERR_POP(mpi_errno);
            }

            if (vc_ptr != NULL) {
                /* We have picked up a packet, re-set the spin_count */
                spin_count = 0;
                /*CM code*/
            handle_recv_pkt:
                if ((vc_ptr->ch.state == MPIDI_CH3I_VC_STATE_IDLE)
#ifdef _ENABLE_XRC_
                    || (USE_XRC && (VC_XST_ISSET(vc_ptr, XF_SEND_IDLE) ||
                                    VC_XST_ISSET(vc_ptr, XF_RECV_IDLE)))
#endif
#ifdef CKPT
                    ||
                    ((vc_ptr->ch.state >= MPIDI_CH3I_VC_STATE_SUSPENDING) &&
                     (vc_ptr->ch.state < MPIDI_CH3I_VC_STATE_REACTIVATING_SRV))
#endif
                ) {
#ifdef _ENABLE_UD_
                    if (MVP_USE_UD_HYBRID) {
                        MRAILI_Process_recv(buffer);
                    } else
#endif
                    {
                        mpi_errno = handle_read(vc_ptr, buffer);
                        MPIR_ERR_CHECK(mpi_errno);
                    }
#ifdef _ENABLE_UD_
                } else if (MVP_USE_UD_HYBRID &&
                           (vc_ptr->ch.state <
                            MPIDI_CH3I_VC_STATE_CONNECTING_SRV) &&
                           (vc_ptr->mrail.state & MRAILI_RC_CONNECTING)) {
                    if (unlikely(
                            !(vc_ptr->mrail.state & MRAILI_RC_CONNECTED) &&
                            (((vbuf *)buffer)->transport == IB_TRANSPORT_RC))) {
                        MPIR_Assert(vc_ptr->ch.state ==
                                    MPIDI_CH3I_VC_STATE_CONNECTING_CLI);
                        while (vc_ptr->ch.state ==
                               MPIDI_CH3I_VC_STATE_CONNECTING_CLI) {
                            MPL_sched_yield();
                        }
                        MPIR_Assert(vc_ptr->ch.state ==
                                    MPIDI_CH3I_VC_STATE_IDLE);
                        cm_handle_pending_send();
                    }
                    MRAILI_Process_recv(buffer);
#endif
                } else
#ifdef _ENABLE_XRC_
                    if (vc_ptr->ch.state ==
                            MPIDI_CH3I_VC_STATE_CONNECTING_SRV ||
                        VC_XST_ISSET(vc_ptr, XF_NEW_RECV)) {
                    MPIDI_CH3I_CM_Establish(vc_ptr);
                    if (!USE_XRC) {
                        cm_handle_pending_send();
                    }
                    goto handle_recv_pkt;
                }
#else /* _ENABLE_XRC_ */
                    if (vc_ptr->ch.state ==
                        MPIDI_CH3I_VC_STATE_CONNECTING_SRV) {
                        /*newly established connection on server side*/
                        MPIDI_CH3I_CM_Establish(vc_ptr);
                        cm_handle_pending_send();
                        goto handle_recv_pkt;
                    }
#endif /* _ENABLE_XRC_ */

#ifdef CKPT
                else if (vc_ptr->ch.state ==
                         MPIDI_CH3I_VC_STATE_REACTIVATING_SRV) {
                    MPIDI_CH3I_CM_Establish(vc_ptr);
                    MPIDI_CH3I_CM_Send_logged_msg(vc_ptr);
                    if (vc_ptr->mrail.sreq_head) /*has rndv*/
                        PUSH_FLOWLIST(vc_ptr);
                    /* Handle pending two-sided sends */
                    if (!MPIDI_CH3I_CM_SendQ_empty(vc_ptr)) {
                        cm_send_pending_msg(vc_ptr);
                        /* Handle pending one-sided sends */
                    }
                    if (!MPIDI_CH3I_CM_One_Sided_SendQ_empty(vc_ptr)) {
                        cm_send_pending_1sc_msg(vc_ptr);
                    }
                    goto handle_recv_pkt;
                }
#endif /* CKPT */
#if defined(RDMA_CM)
                else if ((vc_ptr->ch.state ==
                          MPIDI_CH3I_VC_STATE_IWARP_CLI_WAITING) ||
                         (vc_ptr->ch.state ==
                          MPIDI_CH3I_VC_STATE_IWARP_SRV_WAITING) ||
                         (vc_ptr->ch.state ==
                          MPIDI_CH3I_VC_STATE_CONNECTING_SRV)) {
                    mpi_errno = handle_read(vc_ptr, buffer);
                    if (mpi_errno != MPI_SUCCESS) {
                        MPIR_ERR_POP(mpi_errno);
                    }
                }
#endif /* defined(RDMA_CM) */
                else {
                    /* The RDMA_CM or the on_demand_cm thread can potentially
                     * change the state in parallel */
                    if ((MPIDI_CH3I_Process.cm_type == MPIDI_CH3I_CM_ON_DEMAND)
#if defined(RDMA_CM)
                        || (MPIDI_CH3I_Process.cm_type == MPIDI_CH3I_CM_RDMA_CM)
#endif /* defined(RDMA_CM) */
                    ) {
                        goto handle_recv_pkt;
                    } else {
                        /* Control should not reach here */
                        PRINT_ERROR(
                            "vc_ptr->state = %s, vc_ptr->ch.state = %d\n",
                            MPIDI_VC_GetStateString(vc_ptr->state),
                            vc_ptr->ch.state);
                        MPIR_Assert(0);
                    }
                }
            }
#ifdef _ENABLE_UD_
            else if (MVP_USE_UD_HYBRID) {
                nspin++;
                if (nspin % rdma_ud_progress_spin == 0) {
                    if (UD_ACK_PROGRESS_TIMEOUT) {
                        rdma_ud_last_check = mvp_get_time_us();
                        mvp_check_resend();
                        MPIR_T_PVAR_COUNTER_INC(MVP, rdma_ud_retransmissions,
                                                1);
                        MVP_UD_SEND_ACKS();
                    }
                }
            }
#endif
#if defined(_MCST_SUPPORT_)
            if (rdma_enable_mcast && mcast_ctx->init_list) {
                mvp_mcast_process_comm_init_req(mcast_ctx->init_list);
            }
#endif
            /*CM code*/
            if (MPIDI_CH3I_Process.new_conn_complete) {
                /*New connection has been established*/
                MPIDI_CH3I_Process.new_conn_complete = 0;
                PRINT_DEBUG(DEBUG_XRC_verbose > 0, "New connection\n");
                cm_handle_pending_send();
            }
        }

        if (flowlist) {
            MPIDI_CH3I_MRAILI_Process_rndv();
        }

#if defined(_ENABLE_CUDA_)
        MVP_DEVICE_PROGRESS();
#endif
        mpi_errno = MPIR_Progress_hook_exec_all(&made_progress);
        MPIR_ERR_CHECK(mpi_errno);

#ifdef CKPT
        if (MPIDI_CH3I_CR_Get_state() == MPICR_STATE_REQUESTED) {
            /*Release the lock if it is about to checkpoint*/
            break;
        }
#endif
#if (MPICH_THREAD_LEVEL == MPI_THREAD_MULTIPLE)
        if (completions == MPIDI_CH3I_progress_completion_count) {
            /* We are yet to get all the completions, increment the spin_count
             */
            MPIR_THREAD_CHECK_BEGIN
            spin_count++;
            if (spin_count > rdma_polling_spin_count_threshold) {
                /* We have reached the polling_spin_count_threshold. So, we
                 * are now going to release the lock. */
                spin_count = 0;
                MPID_Thread_mutex_unlock(&MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX,
                                         &err);
                if (mvp_use_thread_yield == 1) {
                    /* User has requested this thread to yield the CPU */
                    MPL_sched_yield();
                }
                /* After releasing the lock, lets just wait for a
                 * short time before trying to acquire the lock
                 * again. */
                int spins = 0;
                do {
                    spins++;
                } while (spins < mvp_spins_before_lock);
                MPID_Thread_mutex_lock(&MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX, &err);
            }
            MPIR_THREAD_CHECK_END
        }
#endif

        if (rdma_polling_level == MVP_POLLING_LEVEL_2) {
            /* Level 2 : exit on finding a message on RDMA_FP or SMP channel.
            ** Continue on ibv_poll_cq success.
            */
            if (rdmafp_found || smp_found) {
                if (MPIDI_CH3I_progress_completion_count != completions) {
                    break;
                }
            }

            if (cq_poll_completion) {
                goto start_polling;
            }
        } else if (rdma_polling_level == MVP_POLLING_LEVEL_3) {
            /* Level 3 : exit on finding a message on RDMA_FP channel.
            ** continue polling on SMP and ibv_poll_cq channels until
            ** no more messages.
            */
            if (rdmafp_found) {
                if (MPIDI_CH3I_progress_completion_count != completions) {
                    break;
                }
            }
            if (smp_found || cq_poll_completion) {
                goto start_polling;
            }
        } else if (rdma_polling_level == MVP_POLLING_LEVEL_4) {
            /* Level 4 : exit only after processing all the messages on
            ** all the channels
            */
            if (smp_found || rdmafp_found || cq_poll_completion) {
                goto start_polling;
            }
        }
    }
    /* Level 1 : exit on finding a message on any channel */
    while (completions == MPIDI_CH3I_progress_completion_count && is_blocking);

#ifdef _ENABLE_UD_
    if (!SMP_ONLY && MVP_USE_UD_HYBRID && UD_ACK_PROGRESS_TIMEOUT) {
        mvp_check_resend();
        MPIR_T_PVAR_COUNTER_INC(MVP, rdma_ud_retransmissions, 1);
        MVP_UD_SEND_ACKS();
        rdma_ud_last_check = mvp_get_time_us();
    }
#endif
fn_fail:
#ifdef CKPT
    MPIDI_CH3I_CR_unlock();
#endif
    MPL_DBG_MSG_S(MPIDI_CH3_DBG_CHANNEL, VERBOSE, "entering %s", __func__);
    MPL_DBG_MSG_D(MPIDI_CH3_DBG_CHANNEL, VERBOSE, "count %d",
                  MPIDI_CH3I_progress_completion_count - completions);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_CH3_MRAIL_PROGRESS);
    DEBUG_PRINT("Exiting ch3 progress\n");
    return mpi_errno;
}

int MPIDI_CH3I_Progress_test()
{
    int mpi_errno = MPI_SUCCESS;
    int made_progress;

    MPIR_FUNC_VERBOSE_ENTER(MPID_CH3I_PROGRESS_TEST);

#if defined(CKPT)
    MPIDI_CH3I_CR_lock();
#endif /* defined(CKPT) */

#if (MPICH_THREAD_LEVEL == MPI_THREAD_MULTIPLE)
    if (MPIDI_CH3I_progress_blocked == TRUE) {
        DEBUG_PRINT("progress blocked\n");
        goto fn_exit;
    }
#endif /* (MPICH_THREAD_LEVEL == MPI_THREAD_MULTIPLE) */

    /*needed if early send complete does not occur */
    if (SMP_INIT) {
        int smp_completions = MPIDI_CH3I_progress_completion_count;

        if (mvp_posted_recvq_length || mvp_unexp_msg_recv) {
            mpi_errno = MPIDI_CH3I_SMP_read_progress(MPIDI_Process.my_pg);
            if (mpi_errno != MPI_SUCCESS) {
                MPIR_ERR_POP(mpi_errno);
            }
        }
        if (mvp_num_posted_send) {
            if ((mpi_errno = MPIDI_CH3I_SMP_write_progress(
                     MPIDI_Process.my_pg)) != MPI_SUCCESS) {
                MPIR_ERR_POP(mpi_errno);
            }
        } else {
            MPIR_Assert(mvp_num_queued_smp_ops == 0);
        }
        /* check if we made any progress */
        if (smp_completions != MPIDI_CH3I_progress_completion_count) {
            goto fn_exit;
        }
    }

#if defined(CKPT)
    if (MPIDI_CH3I_Process.reactivation_complete) {
        /*Some channel has been reactivated*/
        cm_handle_reactivation_complete();
    }
#endif /* defined(CKPT) */

    if (!SMP_ONLY) {
        MPIDI_VC_t *vc_ptr = NULL;
        vbuf *buffer = NULL;

        /* SS: Progress test should not be blocking */
        if ((mpi_errno = MPIDI_CH3I_read_progress(&vc_ptr, &buffer, NULL, 0)) !=
            MPI_SUCCESS) {
            MPIR_ERR_POP(mpi_errno);
        }

        if (vc_ptr != NULL) {
        test_handle_recv_pkt:
            if ((vc_ptr->ch.state == MPIDI_CH3I_VC_STATE_IDLE)
#ifdef _ENABLE_XRC_
                || (USE_XRC && (VC_XST_ISSET(vc_ptr, XF_SEND_IDLE) ||
                                VC_XST_ISSET(vc_ptr, XF_RECV_IDLE)))
#endif
#ifdef CKPT
                || ((vc_ptr->ch.state >= MPIDI_CH3I_VC_STATE_SUSPENDING) &&
                    (vc_ptr->ch.state < MPIDI_CH3I_VC_STATE_REACTIVATING_SRV))
#endif
            ) {
#ifdef _ENABLE_UD_
                if (MVP_USE_UD_HYBRID) {
                    MRAILI_Process_recv(buffer);
                } else
#endif
                {
                    mpi_errno = handle_read(vc_ptr, buffer);
                    if (mpi_errno != MPI_SUCCESS) {
                        MPIR_ERR_POP(mpi_errno);
                    }
                }
#ifdef _ENABLE_UD_
            } else if (MVP_USE_UD_HYBRID &&
                       (vc_ptr->ch.state <
                        MPIDI_CH3I_VC_STATE_CONNECTING_SRV) &&
                       (vc_ptr->mrail.state & MRAILI_RC_CONNECTING)) {
                if (unlikely(
                        !(vc_ptr->mrail.state & MRAILI_RC_CONNECTED) &&
                        (((vbuf *)buffer)->transport == IB_TRANSPORT_RC))) {
                    MPIR_Assert(vc_ptr->ch.state ==
                                MPIDI_CH3I_VC_STATE_CONNECTING_CLI);
                    while (vc_ptr->ch.state ==
                           MPIDI_CH3I_VC_STATE_CONNECTING_CLI) {
                        MPL_sched_yield();
                    }
                    MPIR_Assert(vc_ptr->ch.state == MPIDI_CH3I_VC_STATE_IDLE);
                    cm_handle_pending_send();
                }
                MRAILI_Process_recv(buffer);
#endif
            } else
            /*CM code*/
#ifdef _ENABLE_XRC_
                if (vc_ptr->ch.state == MPIDI_CH3I_VC_STATE_CONNECTING_SRV ||
                    VC_XST_ISSET(vc_ptr, XF_NEW_RECV)) {
                /*newly established connection on server side*/
                MPIDI_CH3I_CM_Establish(vc_ptr);
                if (!USE_XRC)
                    cm_handle_pending_send();
                goto test_handle_recv_pkt;
            }
#else /* _ENABLE_XRC_ */
                if (vc_ptr->ch.state == MPIDI_CH3I_VC_STATE_CONNECTING_SRV) {
                    /*newly established connection on server side*/
                    MPIDI_CH3I_CM_Establish(vc_ptr);
                    cm_handle_pending_send();
                    goto test_handle_recv_pkt;
                }
#endif /* _ENABLE_XRC_ */

#if defined(CKPT)
            else if (vc_ptr->ch.state == MPIDI_CH3I_VC_STATE_REACTIVATING_SRV) {
                MPIDI_CH3I_CM_Establish(vc_ptr);
                MPIDI_CH3I_CM_Send_logged_msg(vc_ptr);

                if (vc_ptr->mrail.sreq_head) {
                    /* has rndv */
                    PUSH_FLOWLIST(vc_ptr);
                }

                /* Handle pending two-sided sends */
                if (!MPIDI_CH3I_CM_SendQ_empty(vc_ptr)) {
                    cm_send_pending_msg(vc_ptr);
                }
                /* Handle pending one-sided sends */
                if (!MPIDI_CH3I_CM_One_Sided_SendQ_empty(vc_ptr)) {
                    cm_send_pending_1sc_msg(vc_ptr);
                }
                goto test_handle_recv_pkt;
            }
#endif /* defined(CKPT) */
#if defined(RDMA_CM)
            else if ((vc_ptr->ch.state ==
                      MPIDI_CH3I_VC_STATE_IWARP_CLI_WAITING) ||
                     (vc_ptr->ch.state ==
                      MPIDI_CH3I_VC_STATE_IWARP_SRV_WAITING) ||
                     (vc_ptr->ch.state == MPIDI_CH3I_VC_STATE_CONNECTING_SRV)) {
                mpi_errno = handle_read(vc_ptr, buffer);
                MPIR_ERR_CHECK(mpi_errno);
            }
#endif /* defined(RDMA_CM) */
            else {
                /* Control should not reach here */
                PRINT_ERROR("vc_ptr->state = %s, vc_ptr->ch.state = %d\n",
                            MPIDI_VC_GetStateString(vc_ptr->state),
                            vc_ptr->ch.state);
                MPIR_Assert(0);
            }
        }
#if defined(_MCST_SUPPORT_)
        if (rdma_enable_mcast && mcast_ctx->init_list) {
            mvp_mcast_process_comm_init_req(mcast_ctx->init_list);
        }
#endif
        /*CM code*/
        if (MPIDI_CH3I_Process.new_conn_complete) {
            /*New connection has been established*/
            PRINT_DEBUG(DEBUG_XRC_verbose > 0, "New Connection\n");
            MPIDI_CH3I_Process.new_conn_complete = 0;
            cm_handle_pending_send();
        }
    }

    /* issue RDMA write ops if we got a clr_to_send */
    if (flowlist) {
        MPIDI_CH3I_MRAILI_Process_rndv();
    }

#if defined(_ENABLE_CUDA_)
    MVP_DEVICE_PROGRESS();
#endif

    /* make progress on NBC schedules */
    made_progress = FALSE;
    mpi_errno = MPIDU_Sched_progress(&made_progress);
    MPIR_ERR_CHECK(mpi_errno);
    if (made_progress) {
        MPIDI_CH3_Progress_signal_completion();
    }

#ifdef _ENABLE_UD_
    if (!SMP_ONLY && MVP_USE_UD_HYBRID && UD_ACK_PROGRESS_TIMEOUT) {
        mvp_check_resend();
        MPIR_T_PVAR_COUNTER_INC(MVP, rdma_ud_retransmissions, 1);
        MVP_UD_SEND_ACKS();
        rdma_ud_last_check = mvp_get_time_us();
    }
#endif
fn_fail:
fn_exit:
#if defined(CKPT)
    MPIDI_CH3I_CR_unlock();
#endif /* defined(CKPT) */

    MPIR_FUNC_VERBOSE_EXIT(MPID_CH3I_PROGRESS_TEST);
    DEBUG_PRINT("Exiting ch3 progress test\n");
    return mpi_errno;
}

#if !defined(MPIDI_CH3_Progress_end)
void MPIDI_CH3_Progress_end(MPID_Progress_state *state)
{
    /* MT - This function is empty for the single-threaded implementation */
}
#endif /* !defined(MPIDI_CH3_Progress_end) */

int MPIDI_CH3I_Progress_init()
{
    int i;

    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_CH3_PROGRESS_INIT);
#if (MPICH_THREAD_LEVEL == MPI_THREAD_MULTIPLE)
    MPIR_THREAD_CHECK_BEGIN
    {
        int err;
        MPID_Thread_cond_create(&MPIDI_CH3I_progress_completion_cond, &err);
        MPIR_Assert(err == 0);
    }
    MPIR_THREAD_CHECK_END
#endif /* (MPICH_THREAD_LEVEL == MPI_THREAD_MULTIPLE) */

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_CH3_PROGRESS_INIT);
    return MPI_SUCCESS;
}

int MPIDI_CH3I_Progress_finalize()
{
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_CH3_PROGRESS_FINALIZE);
    MPIDI_CH3I_progress_completion_count = 0;
    mvp_read_progress_pending_vc = NULL;

    PRINT_DEBUG(DEBUG_SHM_verbose > 1,
                "mvp_num_posted_send: %ld, mvp_posted_recvq_length: %ld, "
                "mvp_unexp_msg_recv: %ld\n",
                mvp_num_posted_send, mvp_posted_recvq_length,
                mvp_unexp_msg_recv);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_CH3_PROGRESS_FINALIZE);
    return MPI_SUCCESS;
}

/*
 * MPIDI_CH3I_Request_adjust_iov()
 *
 * Adjust the iovec in the request by the supplied number of bytes.  If the
 * iovec has been consumed, return true; otherwise return false.
 */
int MPIDI_CH3I_Request_adjust_iov(MPIR_Request *req, intptr_t nb)
{
    int offset = req->dev.iov_offset;
    const int count = req->dev.iov_count;

    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_CH3I_REQUEST_ADJUST_IOV);
    if (req->mrail.is_eager_vbuf_queued == 1) {
        if (req->dev.recv_data_sz == req->mrail.eager_unexp_size) {
            return TRUE;
        } else {
            return FALSE;
        }
    }

    while (offset < count) {
        if (req->dev.iov[offset].iov_len <= (intptr_t)nb) {
            nb -= req->dev.iov[offset].iov_len;
            ++offset;
        } else {
            req->dev.iov[offset].iov_base =
                ((char *)req->dev.iov[offset].iov_base) + nb;
            req->dev.iov[offset].iov_len -= nb;
            req->dev.iov_offset = offset;
            DEBUG_PRINT("offset after adjust %d, count %d, remaining %d\n",
                        offset, req->dev.iov_count,
                        req->dev.iov[offset].iov_len);
            MPL_DBG_MSG(MPIDI_CH3_DBG_CHANNEL, VERBOSE,
                        "adjust_iov returning FALSE");
            MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_CH3I_REQUEST_ADJUST_IOV);
            return FALSE;
        }
    }

    req->dev.iov_offset = 0;

    MPL_DBG_MSG(MPIDI_CH3_DBG_CHANNEL, VERBOSE, "adjust_iov returning TRUE");
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_CH3I_REQUEST_ADJUST_IOV);

    return TRUE;
}

#if defined(CKPT)
static int cm_handle_reactivation_complete()
{
    int i = 0;
    MPIDI_VC_t *vc = NULL;
    MPIDI_PG_t *pg = MPIDI_Process.my_pg;
    MPIR_FUNC_VERBOSE_ENTER(
        MPID_STATE_MPIDI_CH3I_CM_HANDLE_REACTIVATION_COMPLETE);

    for (; i < MPIDI_PG_Get_size(pg); ++i) {
        if (i == MPIDI_Process.my_pg_rank) {
            continue;
        }

        MPIDI_PG_Get_vc(pg, i, &vc);

        if (vc->ch.state == MPIDI_CH3I_VC_STATE_REACTIVATING_CLI_2) {
            MPIDI_CH3I_CM_Send_logged_msg(vc);
            vc->ch.state = MPIDI_CH3I_VC_STATE_IDLE;

            if (vc->mrail.sreq_head) {
                /*has pending rndv*/
                PUSH_FLOWLIST(vc);
            }

            /* Handle pending two-sided sends */
            if (!MPIDI_CH3I_CM_SendQ_empty(vc)) {
                cm_send_pending_msg(vc);
            }
            /* Handle pending one-sided sends */
            if (!MPIDI_CH3I_CM_One_Sided_SendQ_empty(vc)) {
                cm_send_pending_1sc_msg(vc);
            }
        }
    }

    MPIR_FUNC_VERBOSE_EXIT(
        MPID_STATE_MPIDI_CH3I_CM_HANDLE_REACTIVATION_COMPLETE);
    return MPI_SUCCESS;
}
#endif /* defined(CKPT) */

int cm_send_pending_msg(MPIDI_VC_t *vc)
{
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_CM_SENDING_PENDING_MSG);
    int mpi_errno = MPI_SUCCESS;

#ifdef _ENABLE_XRC_
    PRINT_DEBUG(DEBUG_XRC_verbose > 0, "cm_send_pending_msg %d 0x%08x %d\n",
                vc->pg_rank, vc->ch.xrc_flags, vc->ch.state);
    MPIR_Assert(vc->ch.state == MPIDI_CH3I_VC_STATE_IDLE ||
                VC_XST_ISSET(vc, XF_SEND_IDLE));
#else
    MPIR_Assert(vc->ch.state == MPIDI_CH3I_VC_STATE_IDLE);
#endif

    while (!MPIDI_CH3I_CM_SendQ_empty(vc)) {
        struct MPIR_Request *sreq;
        struct iovec *iov;
        int n_iov;

        sreq = MPIDI_CH3I_CM_SendQ_head(vc);
        MPIDI_CH3I_CM_SendQ_dequeue(vc);
        iov = sreq->dev.iov;
        n_iov = sreq->dev.iov_count;
        void *databuf = NULL;

        {
            /*Code copied from ch3_isendv*/
            int nb;
            int pkt_len;
            int complete;

            /* MT - need some signalling to lock down our right to use the
               channel, thus insuring that the progress engine does also try to
               write */
            Calculate_IOV_len(iov, n_iov, pkt_len);

            if (pkt_len > MRAIL_MAX_EAGER_SIZE) {
                MPIR_Memcpy(sreq->dev.iov, iov, n_iov * sizeof(struct iovec));
                sreq->dev.iov_count = n_iov;

                switch ((mpi_errno = MPIDI_CH3_Packetized_send(vc, sreq))) {
                    case MPI_MRAIL_MSG_QUEUED:
                        mpi_errno = MPI_SUCCESS;
                        break;
                    case MPI_SUCCESS:
                        break;
                    default:
                        MPIR_ERR_POP(mpi_errno);
                }

                goto loop_exit;
            }

            if (sreq->dev.OnDataAvail == MPIDI_CH3_ReqHandler_SendReloadIOV) {
                /*reload iov */
                void *tmpbuf = NULL;
                int iter_iov = 0;

                if ((tmpbuf = MPL_malloc(sreq->dev.msgsize + pkt_len,
                                         MPL_MEM_OTHER)) == NULL) {
                    MPIR_CHKMEM_SETERR(mpi_errno, sreq->dev.msgsize + pkt_len,
                                       "temporary iov buffer");
                }

                databuf = tmpbuf;
                pkt_len = 0;

                /* First copy whatever has already been in iov set */
                for (; iter_iov < n_iov; ++iter_iov) {
                    MPIR_Memcpy(tmpbuf, iov[iter_iov].iov_base,
                                iov[iter_iov].iov_len);
                    tmpbuf =
                        (void *)((unsigned long)tmpbuf + iov[iter_iov].iov_len);
                    pkt_len += iov[iter_iov].iov_len;
                }

                DEBUG_PRINT("Pkt len after first stage %d\n", pkt_len);
                /* Second reload iov and copy */
                do {
                    sreq->dev.iov_count = MPL_IOV_LIMIT;

                    mpi_errno = MPIDI_CH3U_Request_load_send_iov(
                        sreq, sreq->dev.iov, &sreq->dev.iov_count);
                    MPIR_ERR_CHECK(mpi_errno);

                    for (iter_iov = 0; iter_iov < sreq->dev.iov_count;
                         ++iter_iov) {
                        MPIR_Memcpy(tmpbuf, sreq->dev.iov[iter_iov].iov_base,
                                    sreq->dev.iov[iter_iov].iov_len);
                        tmpbuf = (void *)((unsigned long)tmpbuf +
                                          sreq->dev.iov[iter_iov].iov_len);
                        pkt_len += sreq->dev.iov[iter_iov].iov_len;
                    }
                } while (sreq->dev.OnDataAvail ==
                         MPIDI_CH3_ReqHandler_SendReloadIOV);

                iov[0].iov_base = databuf;
                iov[0].iov_len = pkt_len;
                n_iov = 1;
            }

            if (pkt_len > MRAIL_MAX_EAGER_SIZE) {
                MPIR_Memcpy(sreq->dev.iov, iov, n_iov * sizeof(struct iovec));
                sreq->dev.iov_count = n_iov;

                switch ((mpi_errno = MPIDI_CH3_Packetized_send(vc, sreq))) {
                    case MPI_MRAIL_MSG_QUEUED:
                        mpi_errno = MPI_SUCCESS;
                        break;
                    case MPI_SUCCESS:
                        break;
                    default:
                        MPIR_ERR_POP(mpi_errno);
                }

                goto loop_exit;
            }

            {
                /* TODO: Codes to send pkt through send/recv path */
                vbuf *buf;
                mpi_errno = MPIDI_CH3I_MRAILI_Eager_send(vc, iov, n_iov,
                                                         pkt_len, &nb, &buf);
                DEBUG_PRINT("[istartmsgv] mpierr %d, nb %d\n", mpi_errno, nb);

                switch (mpi_errno) {
                    case MPI_SUCCESS:
                        DEBUG_PRINT("[send path] eager send return %d bytes\n",
                                    nb);
#if 0
                        if (nb == 0)
                        {
                            /* under layer cannot send out the msg because there is no credit or
                             * no send wqe available 
                             * DEBUG_PRINT("Send 0 bytes\n");
                             * create_request(sreq, iov, n_iov, 0, 0);
                             * MPIDI_CH3I_SendQ_enqueue(vc, sreq);
                             */
                        }
                        else
#endif /* 0 */

                        if (nb) {
                            MPIDI_CH3U_Handle_send_req(vc, sreq, &complete);

                            /* TODO: Check return code of
                             * MPIDI_CH3U_Handle_send_req and produce an error
                             * on failure.
                             */

                            vc->ch.send_active = MPIDI_CH3I_CM_SendQ_head(vc);
                        }
                        break;
                    case MPI_MRAIL_MSG_QUEUED:
                        buf->sreq = (void *)sreq;
                        mpi_errno = MPI_SUCCESS;
                        break;
                    default:
                        /* Connection just failed.  Mark the request complete
                         * and return an error. */
                        vc->ch.state = MPIDI_CH3I_VC_STATE_FAILED;
                        /* TODO: Create an appropriate error message based on
                         * the value of errno */
                        sreq->status.MPI_ERROR = MPI_ERR_INTERN;
                        /* MT - CH3U_Request_complete performs write barrier */
                        MPID_Request_complete(sreq);
                        MPIR_ERR_POP(mpi_errno);
                        break;
                }

                goto loop_exit;
            }
        }

    loop_exit:
        if (databuf) {
            MPL_free(databuf);
        }
    }

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_CM_SENDING_PENDING_MSG);
    return mpi_errno;

fn_fail:
    goto loop_exit;
}

int cm_send_pending_1sc_msg(MPIDI_VC_t *vc)
{
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_CM_SENDING_PENDING_1SC_MSG);

    vbuf *v = NULL;
    int mpi_errno = MPI_SUCCESS;
    MPIDI_VC_t *save_vc = vc;

#ifdef _ENABLE_XRC_
    PRINT_DEBUG(DEBUG_XRC_verbose > 0, "cm_send_pending_1sc_msg %d 0x%08x %d\n",
                vc->pg_rank, vc->ch.xrc_flags, vc->ch.state);
    MPIR_Assert(vc->ch.state == MPIDI_CH3I_VC_STATE_IDLE ||
                VC_XST_ISSET(vc, XF_SEND_IDLE));
#else
    MPIR_Assert(vc->ch.state == MPIDI_CH3I_VC_STATE_IDLE);
#endif

    while (!MPIDI_CH3I_CM_One_Sided_SendQ_empty(vc)) {
        /* Get the queued message */
        v = MPIDI_CH3I_CM_One_Sided_SendQ_head(vc);

        /* Fill the SRQ number. We wouldn't have done this while queueing the
         * message as the connection was not established then
         */
        XRC_FILL_SRQN_FIX_CONN(v, vc, v->rail);
        if (MRAILI_Flush_wqe(vc, v, v->rail) != -1) {
            --(vc->mrail.rails[v->rail].send_wqes_avail);
            /* send noop to generate a completion event at server, so that
             * server will establish the connection finally
             */
            MRAILI_Send_noop(vc, v->rail);
        }

        if (MRAILI_Flush_wqe(vc, v, v->rail) != -1) {
            --(vc->mrail.rails[v->rail].send_wqes_avail);
            IBV_POST_SR(v, vc, v->rail, "Failed to post rma put");
        }

        vc = save_vc;
        /* Dequeue the message */
        MPIDI_CH3I_CM_One_Sided_SendQ_dequeue(vc);
    }

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_CM_SENDING_PENDING_1SC_MSG);
    return mpi_errno;
}

static int cm_handle_pending_send()
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_PG_iterator iter;
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_CM_HANDLE_PENDING_SEND);

    /* MPIDI_PG_Iterate_reset(); */
    MPIDI_PG_Get_iterator(&iter);

    while (MPIDI_PG_Has_next(&iter)) {
        int i;
        MPIDI_PG_t *pg;
        MPIDI_PG_Get_next(&iter, &pg);
        for (i = 0; i < MPIDI_PG_Get_size(pg); ++i) {
            MPIDI_VC_t *vc = NULL;
            MPIDI_PG_Get_vc(pg, i, &vc);

            /* Handle pending two-sided sends */
            if ((vc->ch.state == MPIDI_CH3I_VC_STATE_IDLE
#ifdef _ENABLE_XRC_
                 || (USE_XRC && VC_XST_ISSET(vc, XF_SEND_IDLE))
#endif
                     ) &&
                !MPIDI_CH3I_CM_SendQ_empty(vc)) {
                if ((mpi_errno = cm_send_pending_msg(vc)) != MPI_SUCCESS) {
                    MPIR_ERR_POP(mpi_errno);
                }
            }
            /* Handle pending one-sided sends */
            if ((vc->ch.state == MPIDI_CH3I_VC_STATE_IDLE
#ifdef _ENABLE_XRC_
                 || (USE_XRC && VC_XST_ISSET(vc, XF_SEND_IDLE))
#endif
                     ) &&
                !MPIDI_CH3I_CM_One_Sided_SendQ_empty(vc)) {
                if ((mpi_errno = cm_send_pending_1sc_msg(vc)) != MPI_SUCCESS) {
                    MPIR_ERR_POP(mpi_errno);
                }
            }

#ifdef _ENABLE_XRC_
            if (USE_XRC && VC_XST_ISSET(vc, XF_SEND_IDLE) &&
                (vc->mrail.sreq_head)) { /*has rndv*/
                PUSH_FLOWLIST(vc);
            }

            MPICM_lock();
            if (USE_XRC && xrc_rdmafp_init &&
                VC_XSTS_ISSET(vc, (XF_START_RDMAFP | XF_SEND_IDLE)) &&
                VC_XST_ISUNSET(vc, XF_CONN_CLOSING)) {
                PRINT_DEBUG(DEBUG_XRC_verbose > 0, "FP to %d\n", vc->pg_rank);
                MPICM_unlock();
                vbuf_fast_rdma_alloc(vc, 1);
                vbuf_address_send(vc);
                MPICM_lock();
                VC_XST_CLR(vc, XF_START_RDMAFP);
            }
            MPICM_unlock();
#endif
        }
        /* MPIDI_PG_Get_next(&iter, &pg); */
    }

fn_fail:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_CM_HANDLE_PENDING_SEND);
    return mpi_errno;
}

static int cm_accept_new_vc(MPIDI_VC_t *vc,
                            MPIDI_CH3_Pkt_cm_establish_t *header)
{
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_CM_ACCEPT_NEW_VC);
    PRINT_DEBUG(DEBUG_XRC_verbose > 0, "cm_accept_new_vc");
    MPIR_Assert((uintptr_t)vc == header->vc_addr);
    MPIR_Assert(vc->pg == NULL);
#if 0
    MPIDI_CH3I_CM_Establish(vc);
#endif
    MPIDI_CH3I_Acceptq_enqueue(vc, header->port_name_tag);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_CM_ACCEPT_NEW_VC);
    return MPI_SUCCESS;
}

int handle_read(MPIDI_VC_t *vc, vbuf *buffer)
{
    int mpi_errno = MPI_SUCCESS;
    int header_type = 0;
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_HANDLE_READ);

#if defined(MPIDI_MRAILI_COALESCE_ENABLED)
    /* we don't know how many packets may be combined, so
     * we check after reading a packet to see if there are
     * more bytes yet to be consumed
     */

    buffer->content_consumed = 0;
    unsigned char *original_buffer = buffer->buffer;

    DEBUG_PRINT("[handle read] buffer %p\n", buffer);

    intptr_t total_consumed = 0;

    /* TODO: Refactor this algorithm so the first flag is not used. */
    int first = 1;

    do {
        vc->ch.recv_active = vc->ch.req;
        buffer->content_consumed = 0;

        if ((mpi_errno = handle_read_individual(vc, buffer, &header_type)) !=
            MPI_SUCCESS) {
            MPIR_ERR_POP(mpi_errno);
        }

        buffer->pheader =
            (void *)((uintptr_t)buffer->pheader + buffer->content_consumed);
        total_consumed += buffer->content_consumed;

        if (MPIDI_CH3I_Seq(header_type) && !first) {
            ++vc->seqnum_recv;
        }

        first = 0;
    } while (total_consumed != buffer->content_size);

    DEBUG_PRINT("Finished with buffer -- size: %d, consumed: %d\n",
                buffer->content_size, buffer->content_consumed);

    /* put the original buffer address back */
    buffer->buffer = original_buffer;
    buffer->pheader = original_buffer;

    DEBUG_PRINT("buffer set to: %p\n", buffer->buffer);
#else  /* defined(MPIDI_MRAILI_COALESCE_ENABLED) */
    if ((mpi_errno = handle_read_individual(vc, buffer, &header_type)) !=
        MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
#endif /* defined(MPIDI_MRAILI_COALESCE_ENABLED) */

fn_fail:
    /* by this point we can always free the vbuf */
    MPIDI_CH3I_MRAIL_Release_vbuf(buffer);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_HANDLE_READ);
    return mpi_errno;
}

static int handle_read_individual(MPIDI_VC_t *vc, vbuf *buffer,
                                  int *header_type)
{
    int mpi_errno = MPI_SUCCESS;
    int header_size = 0;
    MPIDI_CH3_Pkt_send_t *header = NULL;
    int packetized_recv = 0;
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_HANDLE_READ_INDIVIDUAL);

    /* Step one, ask lower level to provide header */
    /*  save header at req->dev.pending_pkt, and return the header size */
    /*  ??TODO: Possibly just return the address of the header */

    DEBUG_PRINT("[handle read] pheader: %p\n", buffer->pheader);

    mpi_errno = MPIDI_CH3I_MRAIL_Parse_header(vc, buffer, (void *)(&header),
                                              &header_size);
    if (mpi_errno) {
        MPIR_ERR_POP(mpi_errno);
    }
#if defined(MPIDI_MRAILI_COALESCE_ENABLED)
    buffer->content_consumed = header_size;
#endif /* defined(MPIDI_MRAILI_COALESCE_ENABLED) */

    DEBUG_PRINT("[handle read] header type %d\n", header->type);

    *header_type = header->type;
    MPIR_Request *req = vc->ch.recv_active;

    switch (header->type) {
#if defined(CKPT)
        case MPIDI_CH3_PKT_CM_SUSPEND:
        case MPIDI_CH3_PKT_CM_REACTIVATION_DONE:
            MPIDI_CH3I_CM_Handle_recv(vc, header->type, buffer);
            goto fn_exit;
        case MPIDI_CH3_PKT_CR_REMOTE_UPDATE:
            MPIDI_CH3I_CR_Handle_recv(vc, header->type, buffer);
            goto fn_exit;
#endif /* defined(CKPT) */
        case MPIDI_CH3_PKT_NOOP:
#if defined(RDMA_CM)
            if (MPIDI_CH3I_Process.cm_type == MPIDI_CH3I_CM_RDMA_CM) {
                if (vc->ch.state == MPIDI_CH3I_VC_STATE_IWARP_CLI_WAITING) {
                    /* This case is needed only for multi-rail with rdma_cm */
                    vc->ch.state = MPIDI_CH3I_VC_STATE_IDLE;
                    if (vc->state < MPIDI_VC_STATE_ACTIVE) {
                        PRINT_DEBUG(
                            DEBUG_CM_verbose > 0,
                            "Current state (%s) State of %d to active\n",
                            MPIDI_VC_GetStateString(vc->state), vc->pg_rank);
                        vc->state = MPIDI_VC_STATE_ACTIVE;
                    } else {
                        PRINT_DEBUG(
                            DEBUG_CM_verbose > 0,
                            "Current state of %d is %s. Not moving to active\n",
                            vc->pg_rank, MPIDI_VC_GetStateString(vc->state));
                    }
                    MPIDI_CH3I_Process.new_conn_complete = 1;
                    PRINT_DEBUG(DEBUG_CM_verbose > 0,
                                "NOOP Received, RDMA CM is setting the proper "
                                "status on the client side for multirail.\n");
                    if (MVP_USE_EAGER_FAST_SEND &&
                        !(SMP_INIT && (vc->smp.local_nodes >= 0))) {
                        vc->use_eager_fast_fn = 1;
                    }
                }

                if ((vc->ch.state == MPIDI_CH3I_VC_STATE_IWARP_SRV_WAITING) ||
                    (vc->ch.state == MPIDI_CH3I_VC_STATE_CONNECTING_SRV)) {
                    ++rdma_cm_iwarp_msg_count[vc->pg_rank];

                    if (rdma_cm_iwarp_msg_count[vc->pg_rank] >=
                            rdma_num_rails &&
                        rdma_cm_connect_count[vc->pg_rank] >= rdma_num_rails) {
                        vc->ch.state = MPIDI_CH3I_VC_STATE_IDLE;
                        if (vc->state < MPIDI_VC_STATE_ACTIVE) {
                            PRINT_DEBUG(
                                DEBUG_CM_verbose > 0,
                                "Current state (%s) State of %d to active\n",
                                MPIDI_VC_GetStateString(vc->state),
                                vc->pg_rank);
                            vc->state = MPIDI_VC_STATE_ACTIVE;
                        } else {
                            PRINT_DEBUG(DEBUG_CM_verbose > 0,
                                        "Current state of %d is %s. Not moving "
                                        "to active\n",
                                        vc->pg_rank,
                                        MPIDI_VC_GetStateString(vc->state));
                        }
                        MPIDI_CH3I_Process.new_conn_complete = 1;
                        MRAILI_Send_noop(vc, 0);
                    }
                    PRINT_DEBUG(DEBUG_CM_verbose > 0,
                                "NOOP Received, RDMA CM is setting up the "
                                "proper status on the server side.\n");
                }
            }
#endif /* defined(RDMA_CM) */
        case MPIDI_CH3_PKT_ADDRESS:
        case MPIDI_CH3_PKT_ADDRESS_REPLY:
        case MPIDI_CH3_PKT_FLOW_CNTL_UPDATE:
            PRINT_DEBUG(DEBUG_RNDV_verbose > 2,
                        "flow control received from: %d\n", vc->pg_rank);
            goto fn_exit;
        case MPIDI_CH3_PKT_PACKETIZED_SEND_DATA:
            PRINT_DEBUG(DEBUG_RNDV_verbose > 1,
                        "Packetized data received from: %d\n", vc->pg_rank);
            MPIDI_CH3_Packetized_recv_data(vc, buffer);
            goto fn_exit;
        case MPIDI_CH3_PKT_RNDV_R3_DATA:
            PRINT_DEBUG(DEBUG_RNDV_verbose > 1, "R3 data received from: %d\n",
                        vc->pg_rank);
            MPIDI_CH3_Rendezvouz_r3_recv_data(vc, buffer);
            goto fn_exit;
        case MPIDI_CH3_PKT_RNDV_R3_ACK:
            MPIDI_CH3_Rendezvouz_r3_ack_recv(vc, (void *)header);
            goto fn_exit;
        case MPIDI_CH3_PKT_RPUT_FINISH:
            PRINT_DEBUG(DEBUG_RNDV_verbose > 1,
                        "RPUT finish received from: %d\n", vc->pg_rank);
            MPIDI_CH3_Rendezvous_rput_finish(vc, (void *)header);
            goto fn_exit;
        case MPIDI_CH3_PKT_RGET_FINISH:
            PRINT_DEBUG(DEBUG_RNDV_verbose > 1,
                        "RGET finish received from: %d\n", vc->pg_rank);
            MPIDI_CH3_Rendezvous_rget_send_finish(vc, (void *)header);
            goto fn_exit;
#ifdef _ENABLE_CUDA_
        case MPIDI_CH3_PKT_CUDA_CTS_CONTI:
            MPIDI_CH3_Rendezvous_device_cts_conti(vc, (void *)header);
            goto fn_exit;
#endif
#ifdef _ENABLE_UD_
        case MPIDI_CH3_PKT_ZCOPY_FINISH:
            PRINT_DEBUG(DEBUG_ZCY_verbose > 1,
                        "zcopy finish received from:%d\n", vc->pg_rank);
            MPIDI_CH3_Rendezvous_zcopy_finish(vc, (void *)header);
            goto fn_exit;
        case MPIDI_CH3_PKT_ZCOPY_ACK:
            PRINT_DEBUG(DEBUG_ZCY_verbose > 1, "zcopy ack received from:%d\n",
                        vc->pg_rank);
            MPIDI_CH3_Rendezvous_zcopy_ack(vc, (void *)header);
            goto fn_exit;
#endif
#if defined(_MCST_SUPPORT_)
        case MPIDI_CH3_PKT_MCST_NACK:
            mvp_mcast_handle_nack((void *)header);
            goto fn_exit;
        case MPIDI_CH3_PKT_MCST_INIT_ACK:
            mvp_mcast_handle_init_ack((void *)header);
            goto fn_exit;
#endif
        case MPIDI_CH3_PKT_CM_ESTABLISH:
            mpi_errno = cm_accept_new_vc(vc, (void *)header);
            if (mpi_errno)
                MPIR_ERR_POP(mpi_errno);
            goto fn_exit;
        case MPIDI_CH3_PKT_PACKETIZED_SEND_START:
            packetized_recv = 1;
            header_size += ((MPIDI_CH3_Pkt_packetized_send_start_t *)header)
                               ->origin_head_size;
#if defined(MPIDI_MRAILI_COALESCE_ENABLED)
            buffer->content_consumed = header_size;
#endif /* defined(MPIDI_MRAILI_COALESCE_ENABLED) */
            header = (void *)((uintptr_t)header +
                              sizeof(MPIDI_CH3_Pkt_packetized_send_start_t));
            break;
    }

    DEBUG_PRINT("[handle read] header eager %d, headersize %d", header->type,
                header_size);

    intptr_t buflen = sizeof(MPIDI_CH3_Pkt_t);

    /* Step two, load request according to the header content */
    if ((mpi_errno = MPIDI_CH3U_Handle_recv_pkt(
             vc, (void *)header, ((char *)header + MPIDI_CH3U_PKT_SIZE(header)),
             &buflen, &vc->ch.recv_active)) != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }

    DEBUG_PRINT("[recv: progress] about to fill request, recv_active %p\n",
                vc->ch.recv_active);

    if (vc->ch.recv_active != NULL) {
        /* Step three, ask lower level to fill the request */
        /*      request is vc->ch.recv_active */

        if (packetized_recv == 1) {
            if ((mpi_errno = MPIDI_CH3_Packetized_recv_req(
                     vc, vc->ch.recv_active)) != MPI_SUCCESS) {
                MPIR_ERR_POP(mpi_errno);
            }
        }

        int nb;
        if ((mpi_errno = MPIDI_CH3I_MRAIL_Fill_Request(vc->ch.recv_active,
                                                       buffer, header_size,
                                                       &nb)) != MPI_SUCCESS) {
            MPIR_ERR_POP(mpi_errno);
        }

        req = vc->ch.recv_active;
        DEBUG_PRINT("recv: handle read] nb %d, iov n %d, len %d, VBUFSIZE %d\n",
                    nb, req->dev.iov_count, req->dev.iov[0].iov_len,
                    VBUF_BUFFER_SIZE);

        if (MPIDI_CH3I_Request_adjust_iov(req, nb)) {
            /* Read operation complete */
            DEBUG_PRINT("[recv: handle read] adjust iov correct\n");
            int complete;

            if ((mpi_errno = MPIDI_CH3U_Handle_recv_req(vc, req, &complete)) !=
                MPI_SUCCESS) {
                MPIR_ERR_POP(mpi_errno);
            }

            DEBUG_PRINT("[recv: handle read] adjust req fine, complete %d\n",
                        complete);

            while (!complete) {
                header_size += nb;

                /* Fill request again */
                if ((mpi_errno = MPIDI_CH3I_MRAIL_Fill_Request(
                         req, buffer, header_size, &nb)) != MPI_SUCCESS) {
                    MPIR_ERR_POP(mpi_errno);
                }

                if (!MPIDI_CH3I_Request_adjust_iov(req, nb)) {
                    if (!packetized_recv) {
                        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                    }

                    goto fn_exit;
                }

                if ((mpi_errno = MPIDI_CH3U_Handle_recv_req(
                         vc, req, &complete)) != MPI_SUCCESS) {
                    MPIR_ERR_POP(mpi_errno);
                }
            }

            /* If the communication is packetized, we are expecing more packets
             * for the
             * request. We encounter an error if the request finishes at this
             * stage */
            if (packetized_recv) {
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
            }
        } else if (!packetized_recv) {
            MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
        }
#if defined(DEBUG)
        else {
            DEBUG_PRINT("unfinished req left to packetized send\n");
        }
#endif /* defined(DEBUG) */
        vc->ch.recv_active = NULL;
    }
#if defined(DEBUG)
    else {
        /* we are getting a 0 byte msg header */
    }
#endif /* if defined(DEBUG) */

fn_fail:
fn_exit:
    DEBUG_PRINT("exiting handle read\n");
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_HANDLE_READ_INDIVIDUAL);
    return mpi_errno;
}

/* vi:set sw=4 */
