/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda. For detailed
 * copyright and licensing information, please refer to the copyright file
 * COPYRIGHT in the top level MVAPICH directory.
 */

#include "mvp_smp_impl.h"
#include "mvp_vc.h"
#include "mvp_req_fields.h"
#include "mvp_req.h"
#include "mvp_smp_progress.h"
#include "mvp_smp_progress_utils.h"

volatile unsigned int MPIDI_MVP_progress_completion_count = 0;
#if (MPICH_THREAD_LEVEL == MPI_THREAD_MULTIPLE)
volatile int MPIDI_MVP_progress_blocked = 0;
volatile int MPIDI_MVP_progress_wakeup_signalled = 0;
MPID_Thread_cond_t MPIDI_MVP_progress_completion_cond;
#endif

extern MPIDI_MVP_ep_t *flowlist;

extern long int mvp_num_posted_send;
extern long int mvp_unexp_msg_recv;
extern long int mvp_posted_recvq_length;
extern long int mvp_num_posted_anysrc;
extern int mvp_smp_init;

unsigned long mvp_polling_spin_count_threshold = 5;
int mvp_use_thread_yield = 1;
int mvp_spins_before_lock = 2000;
int mvp_is_fair_polling = 0;

/* forward declarations that may need their own header */
int MPIDI_MVP_smp_handle_send_req(MPIR_Request *sreq, int *complete);

int MPIDI_MVP_Handle_send_req(MPIDI_MVP_ep_t *vc, MPIR_Request *sreq,
                              int *complete);
int MPIDI_MVP_Handle_recv_req(MPIDI_MVP_ep_t *vc, MPIR_Request *rreq,
                              int *complete);
int MPIDI_MVP_smp_process_header(MPIDI_MVP_ep_t *vc, int sender_local_rank,
                                 MPIDI_MVP_Pkt_t *pkt, int *index,
                                 void *dma_header, mvp_smp_dma_flag_t *dma_flag,
                                 SMP_pkt_type_t *recv_pkt_type,
                                 MPIR_Request **rreq_ptr);

int MPIDI_MVP_smp_recvq_probe_anysrc(int *found);

void MPIDI_MVP_MRAILI_Process_rndv();

static inline int MPIDI_MVP_smp_read_progress()
{
    int mpi_errno = MPI_SUCCESS;
    int i = 0;
    int index = -1;
    int l_header = 0;
    int poll_flag = 0;
    int sender_rank = 0;
    int priority_index = 0;
    /* make this a DMA enum */
    int use_cma = 0;
    int complete = 0;
    /* TODO: what is this? Skip what? */
    size_t nb = 0;
    static int skip = 0;
    void *dma_header = NULL;
    mvp_smp_dma_flag_t dma_flag = MVP_SMP_DMA_NONE;
    MPIDI_MVP_ep_t *vc = NULL;
    MPIDI_av_entry_t *av = NULL;
    MPIDI_MVP_Pkt_t *pkt_head = NULL;
    /* clean this up to account for other types of DMA */
#ifdef _SMP_CMA_
    struct cma_header c_header;
    dma_header = (void *)&c_header;
#endif

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_SMP_READ_PROGRESS);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_READ_PROGRESS);

    /* track smp read progress polling for MPIT*/
    /* MPIR_T_PVAR_COUNTER_INC(MVP, mvp_smp_read_progress_poll, 1); */
    if (MVP_SMP_PRIORITY_POLLING) {
        /* TODO: what is set_p_head vs set_c_head? */
        if (polling_set_p_head) {
            priority_index = polling_set_p_head;
            /* TODO: what is this global? */
            mvp_is_fair_polling = 0;
        } else if (++skip >= MVP_SMP_PRIORITY_FACTOR) {
            priority_index = polling_set_c_head;
            mvp_is_fair_polling = 1;
            skip = 0;
        }
    }

    for (i = 1; i < MPIR_Process.local_size || priority_index; ++i) {
        int offset = i;
        int sender_rank; 
        MPIR_Comm *comm_ptr;
        if (MVP_SMP_PRIORITY_POLLING) {
            offset = priority_index;
            /* set the iterator exit condition */
            i += MPIR_Process.local_size;
        }
        sender_rank =
            (MPIR_Process.local_rank + offset) % MPIR_Process.local_size;
        if (sender_rank == MPIR_Process.local_rank) {
            continue;
        }
        MPIR_Comm_get_ptr(MPI_COMM_WORLD, comm_ptr);
        av = MPIDIU_comm_rank_to_av(comm_ptr,
            mvp_smp_info.l2g_rank[sender_rank]);
        vc = MPIDI_MVP_VC(av);

        if (!vc->smp.recv_active) {
            /* get a new message header to start a recv - needs refactor */
            MPIDI_MVP_SMP_pull_header(vc, &pkt_head);
            if (pkt_head) {
                poll_flag = 1;
                use_cma = 0;

                /* old version - use this version for things to work for now
                mpi_errno =
                    MPIDI_MVP_SMP_Process_header(vc, pkt_head, &index,
                                                 &l_header, &c_header, 0,
                                                 &use_cma);
                */
                /* TODO: remove VC once this is finalized */
                mpi_errno =
                    MPIDI_MVP_smp_process_header(vc, vc->smp.local_rank, pkt_head,
                                                 &index, dma_header, &dma_flag,
                                                 &vc->smp.recv_current_pkt_type,
                                                 &vc->smp.recv_active);
                MPIR_ERR_CHECK(mpi_errno);
                if (MVP_SMP_PRIORITY_POLLING && mvp_is_fair_polling) {
                    polling_counters[priority_index]++;
                    if (polling_counters[priority_index] ==
                        MVP_SMP_POLLING_THRESHOLD) {
                        ENQUEUE_p(priority_index);
                    }
                }
            }
        }
        /* need new check here, header processing may have changed this */
        if (vc->smp.recv_active) {
            MPIR_Request *req = vc->smp.recv_active;
            int iov_offset = MPIDI_MVP_REQUEST(req, dev).iov_offset;
            int iov_len =
                MPIDI_MVP_REQUEST(req, dev).iov_count - iov_offset;
            struct iovec *active_iov =
                &MPIDI_MVP_REQUEST(req, dev).iov[iov_offset];
            /* TODO: make this the one we check earlier */

            dma_flag = use_cma ? MVP_SMP_DMA_CMA : MVP_SMP_DMA_NONE;
            poll_flag = 1;

            switch (vc->smp.recv_current_pkt_type) {
                case SMP_RNDV_MSG:
                    mpi_errno = MPIDI_MVP_SMP_readv_rndv(vc, active_iov,
                        iov_len, index, &l_header, &c_header,
                        &nb, dma_flag);
                    break;
                case SMP_RNDV_MSG_CONT:
                    /* why isn't this the same check? */
                    dma_flag = vc->smp.use_cma ? MVP_SMP_DMA_CMA : MVP_SMP_DMA_NONE;
                    mpi_errno =
                        MPIDI_MVP_SMP_readv_rndv_cont(vc, active_iov, iov_len,
                                                      index, &l_header,
                                                      &c_header, &nb, dma_flag);
                    break;
                default:
                    mpi_errno = MPIDI_MVP_SMP_readv(vc, active_iov,
                                                    iov_len, &nb);
                    break;
            }
            MPIR_ERR_CHECK(mpi_errno);
            if (nb > 0) {
                /* iovec has been consumed */
                if (MPIDI_MVP_Request_adjust_iov(req, nb)) {
                    mpi_errno =
                        MPIDI_MVP_Handle_recv_req(vc, req, &complete);
                    MPIR_ERR_CHECK(mpi_errno);
                    while (vc->smp.recv_current_pkt_type == SMP_EAGER_MSG &&
                           !complete) {
                        /* continue to fill request */ 
                        mpi_errno = MPIDI_MVP_SMP_readv(vc, active_iov,
                                                        iov_len, &nb);
                        /* TODO: find a better way to do this */
                        if (!MPIDI_MVP_Request_adjust_iov(req, nb)) {
                            /* iovec not consumed */
                            goto fn_exit;
                        }
                        mpi_errno =
                            MPIDI_MVP_Handle_recv_req(vc, req, &complete);
                        MPIR_ERR_CHECK(mpi_errno);
                    }
                    if (complete) {
#if defined(_SMP_CMA_)
                    /* do CMA stuff */
                    /* send completion message with sender's send
                     * request and number of bytes received. header
                     * type is MPIDI_CH3_PKT_LIMIC_COMP
                     */
                    /* TODO: there's probably a better way to do this */
                        if (use_cma && 
                            vc->smp.recv_current_pkt_type == SMP_RNDV_MSG) {
                            MPIDI_MVP_SMP_send_comp(&c_header, vc,
                                                    nb, MVP_SMP_DMA_CMA,
                                                    NO_FALLBACK);
                        } else if (use_cma &&
                            vc->smp.recv_current_pkt_type ==
                            SMP_RNDV_MSG_CONT) {
                            vc->smp.current_cnb += nb;
                            MPIDI_MVP_SMP_send_comp(
                                &vc->smp.current_c_header, vc,
                                vc->smp.current_cnb, MVP_SMP_DMA_CMA,
                                NO_FALLBACK);
                        }
#endif
                        vc->smp.recv_active = NULL;
                    } else { /* incomplete */
                        if (vc->smp.recv_current_pkt_type ==
                            SMP_RNDV_MSG) {
                            vc->smp.recv_current_pkt_type =
                                SMP_RNDV_MSG_CONT;
                            vc->smp.use_cma = 0;
#if defined(_SMP_CMA_)
                            /*
                             * TODO: make a cma struct
                             *       move this to a CMA file
                             *       this is hideous
                             */
                            if (use_cma) {
                                vc->smp.current_c_header = c_header;
                                vc->smp.current_cnb = nb;
                                vc->smp.use_cma = 1;
                            }
                        } else {
                            if (vc->smp.use_cma) {
                                vc->smp.current_cnb += nb;
                            }
#endif
                        }
                    }
                } else { /* !MPIDI_MVP_Request_adjust_iov(req, nb) */
                    /* IOV was not consumed */
#if defined(_SMP_CMA_) /* TODO: remove this assert - not worth it */
                    MPIR_Assert(vc->smp.recv_current_pkt_type !=
                                SMP_RNDV_MSG || !use_cma);
#endif
                    if (vc->smp.recv_current_pkt_type == SMP_RNDV_MSG) {
                        vc->smp.recv_current_pkt_type = SMP_RNDV_MSG_CONT;
                        vc->smp.use_cma = 0;
                    }
                }
            }
        }
        if (MVP_SMP_PRIORITY_POLLING && mvp_is_fair_polling) {
            priority_index = polling_set_c[priority_index].next;
        } else {
            priority_index = polling_set_p[priority_index].next;
            if (!priority_index && !poll_flag &&
                ++skip >= MVP_SMP_PRIORITY_FACTOR) {
                priority_index = polling_set_c_head;
                mvp_is_fair_polling = 1;
                skip = 0;
            }
        }
    }

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_READ_PROGRESS);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

static inline int MPIDI_MVP_smp_write_progress()
{
    int mpi_errno = MPI_SUCCESS;
    int i;
    int nb = 0;
    int complete;
    MPIDI_MVP_ep_t *vc = NULL;
    MPIDI_av_entry_t *av = NULL;
    MPIR_Request *preq = NULL;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_SMP_WRITE_PROGRESS);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_WRITE_PROGRESS);

    for (i = 0; i < MPIR_Process.local_size; ++i) {
        int tgt_rank = mvp_smp_info.l2g_rank[i];
        MPIR_Comm *comm_ptr;
        MPIR_Comm_get_ptr(MPI_COMM_WORLD, comm_ptr);
        av = MPIDIU_comm_rank_to_av(comm_ptr, tgt_rank); 
        vc = MPIDI_MVP_VC(av);

        /* make sure we have an active send request */
        /*
        if (NULL == vc->smp.send_active && !MPIDI_MVP_SMP_SendQ_empty(vc)) {
            vc->smp.send_active = MPIDI_MVP_SMP_SendQ_head(vc);
        }
        */

        while (vc->smp.send_active) {
            preq = vc->smp.send_active;
            MPIDI_MVP_smp_request_h req = MPIDI_MVP_REQUEST_FROM_MPICH(preq);

            if (req->dev.iov_offset >= req->dev.iov_count) {
                MPIR_ERR_SETFATALANDJUMP2(
                    mpi_errno, MPI_ERR_OTHER, "**mvp_iov_overflow",
                    "**mvp_iov_overflow %d %d", req->dev.iov_offset,
                    req->dev.iov_count);
            }

            switch (req->ch.reqtype) {
                case REQUEST_RNDV_R3_HEADER:
                    vc->smp.send_current_pkt_type = SMP_RNDV_MSG;
                    mpi_errno = MPIDI_MVP_smp_write_rndv_header(
                        vc->smp.local_rank, req->dev.iov + req->dev.iov_offset,
                        req->dev.iov_count - req->dev.iov_offset, &nb);
                    break;
                case REQUEST_RNDV_R3_DATA:
                    if (vc->smp.send_current_pkt_type == SMP_RNDV_MSG) {
                        mpi_errno = MPIDI_MVP_smp_write_rndv_data(
                            vc->smp.local_rank,
                            req->dev.iov + req->dev.iov_offset,
                            req->dev.iov_count - req->dev.iov_offset, 0, &nb);
                    } else {
                        mpi_errno = MPIDI_MVP_smp_write_rndv_data(
                            vc->smp.local_rank,
                            req->dev.iov + req->dev.iov_offset,
                            req->dev.iov_count - req->dev.iov_offset, 1, &nb);
                    }
                    break;
                default:
                    MPIDI_MVP_smp_write(
                        vc->smp.local_rank, req->dev.iov + req->dev.iov_offset,
                        req->dev.iov_count - req->dev.iov_offset, &nb);
                    break;
            }
            MPIR_ERR_CHECK(mpi_errno);

            if (nb > 0) {
                PRINT_DEBUG(DEBUG_RNDV_verbose > 1,
                            "Wrote R3 data, dest: %d, req: %p, bytes: %d\n",
                            vc->pg_rank, req, nb);
                if (MPIDI_MVP_Request_adjust_iov(preq, nb)) {
                    /* Write operation complete - iov consumed */
                    mpi_errno = MPIDI_MVP_smp_handle_send_req(preq, &complete);
                    MPIR_ERR_CHECK(mpi_errno);

                    if (complete) {
                        /* iov consumed and the send request was completed */
                        req->ch.reqtype = REQUEST_NORMAL;
                        if (!MPIDI_MVP_SMP_SendQ_empty(vc)) {
                            MPIDI_MVP_SMP_SendQ_dequeue(vc);
                        }
                        PRINT_DEBUG(
                            DEBUG_RNDV_verbose > 1,
                            "Dequeue request from sendq: %p, now head %p\n",
                            req, vc->smp.sendq_head);
                    } else if (vc->smp.send_current_pkt_type == SMP_RNDV_MSG) {
                        /*
                         * iov was consumed, but the message is still in
                         * progress
                         */
                        vc->smp.send_current_pkt_type = SMP_RNDV_MSG_CONT;
                    }
                    /* TODO: is this necessary for the incomplete path? */
                    vc->smp.send_active = MPIDI_MVP_SMP_SendQ_head(vc);
                } else {
                    /*
                     * iov was not consumed, write still in progress for this
                     * message
                     * Implies we ran out of space for the message in shmem
                     * buffer
                     */
                    if (vc->smp.send_current_pkt_type == SMP_RNDV_MSG) {
                        vc->smp.send_current_pkt_type = SMP_RNDV_MSG_CONT;
                    }
                    MPL_DBG_MSG_D(MPIR_DBG_OTHER, VERBOSE,
                                  "iovec updated by %d bytes but not complete",
                                  nb);

                    if (req->dev.iov_offset >= req->dev.iov_count) {
                        MPIR_ERR_SETFATALANDJUMP2(
                            mpi_errno, MPI_ERR_OTHER, "**mvp_iov_overflow",
                            "**mvp_iov_overflow %d %d", req->dev.iov_offset,
                            req->dev.iov_count);
                    }
                    break;
                }
            } else {
                /* was unable to write anything */
                MPL_DBG_MSG_D(MPIR_DBG_OTHER, VERBOSE,
                              "shm_post_writev returned %d bytes", nb);
                break;
            }
            /* MPIR_T_PVAR_COUNTER_INC(MVP, mvp_smp_write_progress_poll_success,
             * 1); */
        }
        /* reset the active send pointer to the head of the queue */
        if ((vc->smp.send_active != NULL) && (!MPIDI_MVP_SMP_SendQ_empty(vc))) {
            vc->smp.send_active = MPIDI_MVP_SMP_SendQ_head(vc);
        }
    }
fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_WRITE_PROGRESS);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

/* TODO: remove is_blocking as we only call this when it is */
int MPIDI_MVP_smp_progress_wait(int is_blocking, MPID_Progress_state *state)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_MVP_ep_t *vc_ptr = NULL;
    int spin_count = 0, err = 0, i = 0;
    unsigned completions = MPIDI_MVP_progress_completion_count;
    /* vbuf *buffer = NULL; */
    int smp_completions, smp_found;
    int made_progress = 0;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_SMP_PROGRESS_WAIT);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_PROGRESS_WAIT);

    /* poll for anysrc recvs on the network */
    if (mvp_num_posted_anysrc) {
        MPIDI_MVP_smp_recvq_probe_anysrc(&made_progress);
        if (made_progress) {
            goto fn_exit;
        }
    }

    do {
        /* TODO: refactor this loop body into a function */
    start_polling:
        /* MPIR_T_PVAR_COUNTER_INC(MVP, mpit_progress_poll, 1); */
        smp_completions = MPIDI_MVP_progress_completion_count;
        smp_found = 0;
        /* needed if early send complete does not occur */
        if (mvp_smp_init) {
            /* progress pending recvs */
            if (mvp_posted_recvq_length || mvp_unexp_msg_recv) {
                mpi_errno = MPIDI_MVP_smp_read_progress();
                /* mpi_errno = MPIDI_MVP_smp_recv_progress(); */
                MPIR_ERR_CHECK(mpi_errno);
            }
            /* progress pending sends */
            if (mvp_num_posted_send) {
                mpi_errno = MPIDI_MVP_smp_write_progress();
                MPIR_ERR_CHECK(mpi_errno);
            } else {
                MPIR_Assert(mvp_num_queued_smp_ops == 0);
            }
            if (smp_completions != MPIDI_MVP_progress_completion_count) {
                break;
            }
        }
        if (flowlist) {
            /* CMA is progressed here - RNDV only */
            /* unstarted sends */
            MPIDI_MVP_MRAILI_Process_rndv();
        }

        /* TODO: remove this if unneeded
        mpi_errno = MPIR_Progress_hook_exec_all(&made_progress);
        MPIR_ERR_CHECK(mpi_errno);
        */

#if (MPICH_THREAD_LEVEL == MPI_THREAD_MULTIPLE)
        if (completions == MPIDI_MVP_progress_completion_count) {
            /* We have yet to get all the completions, increment spin_count */
            MPIR_THREAD_CHECK_BEGIN
            spin_count++;
            if (spin_count > mvp_polling_spin_count_threshold) {
                /*
                 * We have reached the polling_spin_count_threshold. So, we
                 * are now going to release the lock.
                 */
                spin_count = 0;
                MPID_Thread_mutex_unlock(&MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX,
                                         &err);
                if (mvp_use_thread_yield) {
                    /* User has requested this thread to yield the CPU */
                    MPL_sched_yield();
                }
                /*
                 * After releasing the lock, lets just wait for a
                 * short time before trying to acquire the lock again.
                 */
                int spins = 0;
                while (spins < mvp_spins_before_lock) {
                    spins++;
                }
                MPID_Thread_mutex_lock(&MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX, &err);
            }
            MPIR_THREAD_CHECK_END
        }
#endif
        /*
         * TODO: this loop doesn't make a whole lot of sense. Using a goto to go
         * back to the top of the loop is a terrible practice.
         */
#if 0
        if (rdma_polling_level >= MVP_POLLING_LEVEL_3) {
            /* Level 3 or 4 : exit only after processing all the smp messages */
            if (smp_found) {
                goto start_polling;
            }
        }
#endif
        /* Level 1 : exit on finding a message on any channel */
    } while (completions == MPIDI_MVP_progress_completion_count && is_blocking);

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_PROGRESS_WAIT);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIDI_MVP_smp_progress_test()
{
    int mpi_errno = MPI_SUCCESS;
    int made_progress;
    int smp_completions, smp_found;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_SMP_PROGRESS_TEST);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_PROGRESS_TEST);

#if (MPICH_THREAD_LEVEL == MPI_THREAD_MULTIPLE)
    if (MPIDI_MVP_progress_blocked == TRUE) {
        goto fn_exit;
    }
#endif /* (MPICH_THREAD_LEVEL == MPI_THREAD_MULTIPLE) */

    /* poll for anysrc recvs on the network */
    if (mvp_num_posted_anysrc) {
        MPIDI_MVP_smp_recvq_probe_anysrc(&made_progress);
        if (made_progress) {
            goto fn_exit;
        }
    }

    smp_completions = MPIDI_MVP_progress_completion_count;
    /*needed if early send complete does not occur */
    if (mvp_smp_init) {
        if (mvp_posted_recvq_length || mvp_unexp_msg_recv) {
            mpi_errno = MPIDI_MVP_smp_read_progress();
            if (mpi_errno != MPI_SUCCESS) {
                MPIR_ERR_POP(mpi_errno);
            }
        }
        if (mvp_num_posted_send) {
            if ((mpi_errno = MPIDI_MVP_smp_write_progress()) != MPI_SUCCESS) {
                MPIR_ERR_POP(mpi_errno);
            }
        } else {
            MPIR_Assert(mvp_num_queued_smp_ops == 0);
        }

        if (smp_completions != MPIDI_MVP_progress_completion_count) {
            goto fn_exit;
        }
    }
    if (flowlist) {
        MPIDI_MVP_MRAILI_Process_rndv();
    }

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_PROGRESS_TEST);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
