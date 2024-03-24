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

#ifndef _MVP_SHMEM_PROGRESS_UTILS_H_
#define _MVP_SHMEM_PROGRESS_UTILS_H_

#ifndef MVP_SMP_IMPL_INCLUDED
#error "Requres mvp_smp_impl.h, include that header first"
#endif

#include "mvp_shmem.h"
#include "mvp_req_fields.h"
#include "mvp_req.h"
#include "../src/legacy/mvp_eager_handlers.h"
#include "mvp_rts.h"

void smpi_complete_recv(int from_grank, int my_id, int length);
int MPIDI_MVP_Rendezvous_rput_finish(MPIDI_MVP_ep_t *vc,
                                     MPIDI_MVP_Pkt_rput_finish_t *rf_pkt);
int MPIDI_MVP_Rendezvous_rget_send_finish(
    MPIDI_MVP_ep_t *vc, MPIDI_MVP_Pkt_rget_finish_t *rget_pkt);

static inline int MPIDI_MVP_smp_check_pool()
{
    int mpi_errno = MPI_SUCCESS;
    /* TODO: this is used by CMA and shmem */
    if (!mvp_shmem_pool_init) {
        mpi_errno = MPIDI_MVP_smp_attach_shm_pool_inline();
    }
    return mpi_errno;
}

#define MPIDI_MVP_SMP_SendQ_enqueue(vc, req)                                   \
    {                                                                          \
        /* MT - not thread safe! */                                            \
        MPIDI_MVP_REQUEST(req, dev).next = NULL;                               \
        MPIR_Request_add_ref(req);                                             \
        if (vc->smp.sendq_tail != NULL) {                                      \
            MPIDI_MVP_REQUEST(vc->smp.sendq_tail, dev).next = req;             \
        } else {                                                               \
            vc->smp.sendq_head = req;                                          \
        }                                                                      \
        vc->smp.sendq_tail = req;                                              \
        if (vc->smp.send_active == NULL) {                                     \
            vc->smp.send_active = vc->smp.sendq_head;                          \
        }                                                                      \
        /* Increment number of queued ops */                                   \
        mvp_num_queued_smp_ops++;                                              \
        /* Disable direct send */                                              \
        vc->use_eager_fast_fn = 0;                                             \
    }

#define MPIDI_MVP_SMP_SendQ_enqueue_head(_vc, _req)                            \
    {                                                                          \
        /* MT - not thread safe! */                                            \
        MPIR_Request_add_ref(_req);                                            \
        MPIDI_MVP_REQUEST((_req), dev).next = (_vc)->smp.sendq_head;           \
        if ((_vc)->smp.sendq_tail == NULL) {                                   \
            (_vc)->smp.sendq_tail = (_req);                                    \
        }                                                                      \
        (_vc)->smp.sendq_head = (_req);                                        \
        /* Increment number of queued ops */                                   \
        mvp_num_queued_smp_ops++;                                              \
        /* Disable direct send */                                              \
        (_vc)->use_eager_fast_fn = 0;                                          \
        (_vc)->smp.send_active = (_req);                                       \
    }

#define MPIDI_MVP_SMP_SendQ_dequeue(_vc)                                       \
    {                                                                          \
        MPIR_Request *_req = (_vc)->smp.sendq_head;                            \
        /* MT - not thread safe! */                                            \
        (_vc)->smp.sendq_head =                                                \
            MPIDI_MVP_REQUEST((_vc)->smp.sendq_head, dev.next);                \
        if ((_vc)->smp.sendq_head == NULL) {                                   \
            (_vc)->smp.sendq_tail = NULL;                                      \
            /* Enable direct send */                                           \
            if (MVP_USE_EAGER_FAST_SEND) {                                     \
                (_vc)->use_eager_fast_fn = 1;                                  \
            }                                                                  \
        }                                                                      \
        /* Decrement number of queued ops */                                   \
        mvp_num_queued_smp_ops--;                                              \
        MPIR_Request_free_unsafe(_req);                                        \
    }

#define MPIDI_MVP_SMP_SendQ_head(_vc) ((_vc)->smp.sendq_head)

#define MPIDI_MVP_SMP_SendQ_empty(_vc) ((_vc)->smp.sendq_head == NULL)

#define ENQUEUE_p(x)                                                           \
    if (!polling_set_p_head) {                                                 \
        polling_set_p_head = x;                                                \
        polling_set_p_tail = x;                                                \
        polling_set_p[x].prev = 0;                                             \
        polling_set_p[x].next = 0;                                             \
    } else {                                                                   \
        polling_set_p[polling_set_p_tail].next = x;                            \
        polling_set_p[x].prev = polling_set_p_tail;                            \
        polling_set_p[x].next = 0;                                             \
        polling_set_p_tail = x;                                                \
    }                                                                          \
    if (polling_set_c[x].prev) {                                               \
        polling_set_c[polling_set_c[x].prev].next = polling_set_c[x].next;     \
    } else {                                                                   \
        polling_set_c_head = polling_set_c[x].next;                            \
    }                                                                          \
    if (polling_set_c[x].next) {                                               \
        polling_set_c[polling_set_c[x].next].prev = polling_set_c[x].prev;     \
    } else {                                                                   \
        polling_set_c_tail = polling_set_c[x].prev;                            \
    }

#define SMP_EXIT_ERR -1
#define smp_error_abort(code, message)                                         \
    do {                                                                       \
        if (errno) {                                                           \
            PRINT_ERROR_ERRNO("%s:%d: " message, errno, __FILE__, __LINE__);   \
        } else {                                                               \
            PRINT_ERROR("%s:%d: " message "\n", __FILE__, __LINE__);           \
        }                                                                      \
        fflush(stderr);                                                        \
        exit(code);                                                            \
    } while (0)

static inline mvp_smp_send_buf_t *get_buf_from_pool(void);
static inline void send_buf_reclaim(void);
static inline void put_buf_to_pool(int, int);
static inline void link_buf_to_send_queue(int, int);

/*
 * called by sender before every send in order to check if enough room left in
 * cyclic buffer.
 */
static inline int smpi_check_avail(int rank, int len, volatile void **pptr_flag,
                                   smp_ctrl_avail_flag_t num)
{
    /* check if avail is less than data size */
    if (avail[rank] < num * (len + sizeof(int) * 3)) {
        /* update local tail according to shared tail */
        if (s_header_ptr_s[rank] + len + sizeof(int) * 3 >=
            MPIDI_MVP_SHMEM_LAST_SEND(mvp_smp_info.my_local_id, rank)) {
            /* check if the beginning of the cyclic buffer is already free */
            if (*(int *)((unsigned long)mvp_smp_shmem_region->pool +
                         MPIDI_MVP_SHMEM_FIRST_SEND(mvp_smp_info.my_local_id,
                                                    rank))) {
                return 0;
            }
            s_tail_ptr_s[rank] =
                MPIDI_MVP_SHMEM_SHARED_TAIL(mvp_smp_info.my_local_id, rank);
            if (s_tail_ptr_s[rank] ==
                MPIDI_MVP_SHMEM_FIRST_SEND(mvp_smp_info.my_local_id, rank)) {
                avail[rank] =
                    MPIDI_MVP_SHMEM_LAST_SEND(mvp_smp_info.my_local_id, rank) -
                    s_header_ptr_s[rank];
                return 0;
            }
            s_header_ptr_s[rank] =
                MPIDI_MVP_SHMEM_FIRST_SEND(mvp_smp_info.my_local_id, rank);
            volatile void *ptr_flag;
            ptr_flag = *pptr_flag;
            *(volatile int *)ptr_flag = MVP_SMP_CBUF_END;
            WRITEBAR();
            ptr_flag = (volatile void *)((mvp_smp_shmem_region->pool) +
                                         s_header_ptr_s[rank]);
            *pptr_flag = ptr_flag;
        } else {
            s_tail_ptr_s[rank] =
                MPIDI_MVP_SHMEM_SHARED_TAIL(mvp_smp_info.my_local_id, rank);
        }

        /* update avail */
        READBAR();
        avail[rank] =
            (s_tail_ptr_s[rank] >= s_header_ptr_s[rank] ?
                 (s_tail_ptr_s[rank] - s_header_ptr_s[rank]) :
                 (MPIDI_MVP_SHMEM_LAST_SEND(mvp_smp_info.my_local_id, rank) -
                  s_header_ptr_s[rank]));
        if (avail[rank] < len + sizeof(int) * 3) {
            return 0;
        }
    }
    return 1;
}
/*----------------------------------------------------------*/
static inline mvp_smp_send_buf_t *get_buf_from_pool()
{
    mvp_smp_send_buf_t *ptr;

    if (s_sh_buf_pool.free_head == -1)
        return NULL;

    ptr = MPIDI_MVP_MY_SHMEM_BUF_POOL_PTR(s_sh_buf_pool.free_head);
    s_sh_buf_pool.free_head = ptr->next;
    ptr->next = -1;

    MPIR_Assert(ptr->busy == 0);

    return ptr;
}

static inline void send_buf_reclaim()
{
    int i, index, last_index;
    mvp_smp_send_buf_t *ptr;

    for (i = 0; i < mvp_smp_info.num_local_nodes; ++i) {
        if (i != mvp_smp_info.my_local_id) {
            index = s_sh_buf_pool.send_queue[i];
            last_index = -1;
            ptr = NULL;
            while (index != -1) {
                ptr = MPIDI_MVP_MY_SHMEM_BUF_POOL_PTR(index);
                if (ptr->busy == 1)
                    break;
                last_index = index;
                index = ptr->next;
            }
            if (last_index != -1)
                put_buf_to_pool(s_sh_buf_pool.send_queue[i], last_index);
            s_sh_buf_pool.send_queue[i] = index;
            if (s_sh_buf_pool.send_queue[i] == -1)
                s_sh_buf_pool.tail[i] = -1;
        }
    }
}

static inline void put_buf_to_pool(int head, int tail)
{
    mvp_smp_send_buf_t *ptr;

    MPIR_Assert(head != -1);
    MPIR_Assert(tail != -1);

    ptr = MPIDI_MVP_MY_SHMEM_BUF_POOL_PTR(tail);

    ptr->next = s_sh_buf_pool.free_head;
    s_sh_buf_pool.free_head = head;
}

static inline void link_buf_to_send_queue(int dest, int index)
{
    if (s_sh_buf_pool.send_queue[dest] == -1) {
        s_sh_buf_pool.send_queue[dest] = index;
    } else {
        MPIDI_MVP_MY_SHMEM_BUF_POOL_PTR(s_sh_buf_pool.tail[dest])->next = index;
    }
    s_sh_buf_pool.tail[dest] = index;
}

static inline int MPIDI_MVP_SMP_Process_header(MPIDI_MVP_ep_t *vc,
                                               MPIDI_MVP_Pkt_t *pkt, int *index,
                                               void *limic_header,
                                               void *cma_header, int *use_limic,
                                               int *use_cma)
{
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_PROGRESS_HEADER);
    int mpi_errno = MPI_SUCCESS;
    PRINT_DEBUG(DEBUG_SHM_verbose, "pkt->type %d \n", pkt->type);

    if (pkt->type == MPIDI_MVP_PKT_EAGER_SEND_CONTIG ||
        pkt->type == MPIDI_MVP_PKT_EAGER_SEND) {
        intptr_t buflen = MPIDI_MVP_PKT_SIZE(pkt);
        mpi_errno = MPIDI_MVP_PktHandler_EagerSend(vc, pkt,
            ((char *)pkt + MPIDI_MVP_PKT_SIZE(pkt)), &buflen,
            &vc->smp.recv_active);
        MPIR_ERR_CHECK(mpi_errno);

        if (!vc->smp.recv_active) {
            s_current_ptr[vc->smp.local_nodes] = NULL;
            s_current_bytes[vc->smp.local_nodes] = 0;
            smpi_complete_recv(vc->smp.local_nodes, mvp_smp_info.my_local_id,
                               s_total_bytes[vc->smp.local_nodes]);
            s_total_bytes[vc->smp.local_nodes] = 0;
        }
        goto fn_exit;
    }

    if (MPIDI_MVP_PKT_RNDV_R3_DATA == pkt->type) {
        MPIDI_MVP_Pkt_rndv_r3_data_t *pkt_header =
            (MPIDI_MVP_Pkt_rndv_r3_data_t *)pkt;

        if ((*index = pkt_header->src.smp_index) == -1) {
            MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**fail",
                                      "**fail %s", "*index == -1");
        }

        vc->smp.recv_current_pkt_type = SMP_RNDV_MSG;

        MPIR_Request *rreq = NULL;
        MPIR_Request_get_ptr(
            ((MPIDI_MVP_Pkt_rndv_r3_data_t *)pkt)->receiver_req_id, rreq);
        PRINT_DEBUG(DEBUG_RNDV_verbose > 1,
                    "R3 data received from: %d, rreq: %p\n", vc->pg_rank, rreq);
        vc->smp.recv_active = rreq;
        goto fn_exit;
    } else if (pkt->type == MPIDI_MVP_PKT_RPUT_FINISH) {
        PRINT_DEBUG(DEBUG_RNDV_verbose > 1, "RPUT FINISH received from: %d\n",
                    vc->pg_rank);
        MPIDI_MVP_Rendezvous_rput_finish(vc,
                                         (MPIDI_MVP_Pkt_rput_finish_t *)pkt);
        goto fn_exit;
    } else if (pkt->type == MPIDI_MVP_PKT_RGET_FINISH) {
        PRINT_DEBUG(DEBUG_RNDV_verbose > 1, "RGET FINISH received from: %d\n",
                    vc->pg_rank);
        MPIDI_MVP_Rendezvous_rget_send_finish(
            vc, (MPIDI_MVP_Pkt_rget_finish_t *)pkt);
        goto fn_exit;
    }
#if 0
    if (pkt->type != MPIDI_MVP_PKT_EAGER_SEND_CONTIG) {
        intptr_t buflen = sizeof(MPIDI_MVP_Pkt_t);

        if ((mpi_errno = MPIDI_CH3U_Handle_recv_pkt(
                        vc,
                        pkt, ((char *)pkt + MPIDI_MVP_PKT_SIZE(pkt)),
                        &buflen,
                        &vc->smp.recv_active)) != MPI_SUCCESS)
        {
            MPIR_ERR_POP(mpi_errno);
        }

        vc->smp.recv_current_pkt_type = SMP_EAGER_MSG;
    }
#endif

    intptr_t buflen = sizeof(MPIDI_MVP_Pkt_t);
    if (pkt->type == MPIDI_MVP_PKT_RNDV_CLR_TO_SEND) {
        PRINT_DEBUG(DEBUG_RNDV_verbose > 1, "Calling SMP CTS Handler\n");
        return MPIDI_MVP_PktHandler_SMP_CTS(vc, pkt, &buflen,
                                            &vc->smp.recv_active);
    }

    if (pkt->type == MPIDI_MVP_PKT_RNDV_REQ_TO_SEND) {
        PRINT_DEBUG(DEBUG_RNDV_verbose > 1, "Calling SMP RTS Handler: %p\n");
        return MPIDI_MVP_PktHandler_SMP_RTS(vc, pkt, &buflen,
                                            &vc->smp.recv_active);
    }
fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_PROGRESS_HEADER);
    return mpi_errno;

fn_fail:
    goto fn_exit;
}

int MPIDI_MVP_SMP_pull_header(MPIDI_MVP_ep_t *vc, MPIDI_MVP_Pkt_t **pkt_head);
int MPIDI_MVP_SMP_readv_rndv_cont(MPIDI_MVP_ep_t *vc, const struct iovec *iov,
                                  const int iovlen, int index,
                                  void *limic_header, void *cma_header,
                                  size_t *num_bytes_ptr,
                                  mvp_smp_dma_flag_t dma_flag);
int MPIDI_MVP_SMP_readv_rndv(MPIDI_MVP_ep_t *vc, const struct iovec *iov,
                             const int iovlen, int index, void *limic_header,
                             void *cma_header, size_t *num_bytes_ptr,
                             mvp_smp_dma_flag_t dma_flag);
int MPIDI_MVP_SMP_readv(MPIDI_MVP_ep_t *recv_vc_ptr, const struct iovec *iov,
                        const int iovlen, size_t *num_bytes_ptr);
void MPIDI_MVP_SMP_send_comp(void *header, MPIDI_MVP_ep_t* vc, intptr_t nb,
                             mvp_smp_dma_flag_t dma_flag,
                             smp_fallback_flag_t fallback);
#endif /* _MVP_SHMEM_PROGRESS_UTILS_H_ */
