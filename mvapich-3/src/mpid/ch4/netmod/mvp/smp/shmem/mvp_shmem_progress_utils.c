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

#include "mvp_smp_impl.h"
#include "mvp_vc.h"
#include "mvp_smp_progress_utils.h"
#include "mvp_smp_params.h"
#include "mvp_tagm.h"

void smpi_complete_send(unsigned int destination, unsigned int length,
                        int data_sz, volatile void *ptr,
                        volatile void *ptr_head, volatile void *ptr_flag);
int MPIDI_MVP_SMP_do_cma_read(const struct iovec *iov, const int iovlen,
                              void *cma_header, size_t *num_bytes_ptr);
MPIR_Request *create_request(void *hdr, intptr_t hdr_sz, size_t nb);

#if defined(_SMP_LIMIC_) || defined(_SMP_CMA_)
void MPIDI_MVP_SMP_send_comp(void *header,
                                    MPIDI_MVP_ep_t* vc, intptr_t nb, mvp_smp_dma_flag_t dma_flag,
                                    smp_fallback_flag_t fallback)
{
    MPIDI_MVP_Pkt_comp_t pkt;
    int pkt_sz = sizeof(MPIDI_MVP_Pkt_comp_t);
    volatile void *ptr_head, *ptr, *ptr_flag;
    MPIR_Request *creq = NULL;

    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_SEND_COMP);

#if defined(_SMP_LIMIC_)
    if (dma_flag == MVP_SMP_DMA_LIMIC){
        pkt.type = MPIDI_CH3_PKT_SMP_DMA_COMP;
        struct limic_header *l_header = (struct limic_header *)header;
        pkt.send_req_id = (MPI_Request *)l_header->send_req_id;
        pkt.fallback = fallback;
    }
#endif

#if defined(_SMP_CMA_)
    if (dma_flag == MVP_SMP_DMA_CMA){
        pkt.type = MPIDI_MVP_PKT_SMP_DMA_COMP;
        struct cma_header *c_header = (struct cma_header *)header;
        pkt.send_req_id = (MPI_Request *)c_header->csend_req_id;
        pkt.fallback = fallback;
    }
#endif

    pkt.nb = nb;

    /*make sure the complete message not sent between other unfinished message */
    if (MPIDI_MVP_SMP_SendQ_head(vc)) {
        creq = create_request(&pkt, pkt_sz, 0);
        int inuse = 0;
        MPIR_Object_release_ref_always(creq, &inuse);
        MPIDI_MVP_SMP_SendQ_enqueue(vc, creq);
        return;
    }

    ptr_flag = (volatile void *)((mvp_smp_shmem_region->pool) +
                                 s_header_ptr_s[vc->smp.local_nodes]);

    /* check if avail is less than data size */
    if(!smpi_check_avail(vc->smp.local_nodes, pkt_sz, (volatile void **)&ptr_flag, ONE_FREE))
    {
        /* queue the message */
        creq = create_request(&pkt, pkt_sz, 0);
        int inuse = 0;
        MPIR_Object_release_ref_always(creq, &inuse);
        MPIDI_MVP_SMP_SendQ_enqueue(vc, creq);
        return;
    }

    ptr_head = (volatile void *) ((unsigned long) ptr_flag + sizeof(int));
    ptr = (volatile void *) ((unsigned long) ptr_flag + sizeof(int)*2);
    MPIR_Memcpy((void *)ptr, (const void *)&pkt, pkt_sz);

    ptr = (volatile void *) ((unsigned long) ptr + pkt_sz);
    smpi_complete_send(vc->smp.local_nodes, pkt_sz, pkt_sz, ptr, ptr_head, ptr_flag);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_SEND_COMP);
    return;
}

#endif /* _SMP_LIMIC_ */

int MPIDI_MVP_SMP_pull_header(MPIDI_MVP_ep_t *vc, MPIDI_MVP_Pkt_t **pkt_head)
{
    int ret_val = MPI_SUCCESS;

    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_PULL_HEADER);

    if(s_total_bytes[vc->smp.local_nodes] != 0) {
        s_current_ptr[vc->smp.local_nodes] = NULL;
        smpi_complete_recv(vc->smp.local_nodes, mvp_smp_info.my_local_id,
                           s_total_bytes[vc->smp.local_nodes]);
        s_total_bytes[vc->smp.local_nodes] = 0;
    }

    void *ptr;
    volatile int *ptr_flag;
    ptr = (void *)(mvp_smp_shmem_region->pool +
                   s_header_ptr_r[vc->smp.local_nodes]);
    ptr_flag = (volatile int *)ptr;

    READBAR();
    if (*ptr_flag == MVP_SMP_CBUF_FREE || *ptr_flag == MVP_SMP_CBUF_PENDING) {
        *pkt_head = NULL;
    } else {
        READBAR();
        if (*ptr_flag == MVP_SMP_CBUF_END) {
            *pkt_head = NULL;
            s_header_ptr_r[vc->smp.local_nodes] = MPIDI_MVP_SHMEM_FIRST_RECV(
                vc->smp.local_nodes, mvp_smp_info.my_local_id);
            ptr = (void *)(mvp_smp_shmem_region->pool +
                           s_header_ptr_r[vc->smp.local_nodes]);
            ptr_flag = (volatile int*)ptr;

            READBAR();
            if (*ptr_flag == MVP_SMP_CBUF_FREE ||
                *ptr_flag == MVP_SMP_CBUF_PENDING) {
                goto fn_exit;
            }
        }
        READBAR();
        s_total_bytes[vc->smp.local_nodes] = *(int*)((unsigned long)ptr + sizeof(int));
        *pkt_head = (void *)((unsigned long)ptr + sizeof(int)*2);
        WRITEBAR();
        s_current_bytes[vc->smp.local_nodes] =
            s_total_bytes[vc->smp.local_nodes] - MPIDI_MVP_PKT_SIZE(*pkt_head);
        s_current_ptr[vc->smp.local_nodes] = (void*)((unsigned long) *pkt_head
                + MPIDI_MVP_PKT_SIZE(*pkt_head));
        /* TODO: FIXME: Re-define MPIDI_MVP_PKT_SIZE */
    }

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_PULL_HEADER);
    return ret_val;

fn_fail:
    goto fn_exit;
}

int MPIDI_MVP_SMP_readv_rndv_cont(MPIDI_MVP_ep_t * recv_vc_ptr, const struct iovec * iov,
        const int iovlen, int index, void *limic_header,
        void *cma_header, size_t *num_bytes_ptr, mvp_smp_dma_flag_t dma_flag)
{
    int mpi_errno = MPI_SUCCESS;
    int iov_off = 0, buf_off = 0;
    int received_bytes = 0;
    int destination = recv_vc_ptr->smp.local_nodes;
    int current_index = index;
    int recv_offset = 0;
    size_t msglen, iov_len;
    void *current_buf;
    mvp_smp_send_buf_t *recv_buf;

    /* all variable must be declared before the state declarations */
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_READV_RNDV_CONT);

    *num_bytes_ptr = 0;

#if defined(_SMP_CMA_)
    if (dma_flag == MVP_SMP_DMA_CMA) {
        mpi_errno = MPIDI_MVP_SMP_do_cma_read(
                iov, iovlen, cma_header, num_bytes_ptr);
        if (mpi_errno) {
            MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER,
                        "**fail", "**fail %s",
                        "CMA: MPIDI_MVP_SMP_readv_rndv_cont fail");
        }
    } else {
#endif /* _SMP_CMA_ */

    if (s_current_bytes[recv_vc_ptr->smp.local_nodes] == 0) {
    if(s_total_bytes[recv_vc_ptr->smp.local_nodes] != 0) {
        MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**fail",
            "**fail %s", "s_total_bytes[recv_vc_ptr->smp.local_nodes] "
            "!= 0");
    }

    void *ptr;
    ptr = (void *)(mvp_smp_shmem_region->pool +
                   s_header_ptr_r[recv_vc_ptr->smp.local_nodes]);
    volatile int *ptr_flag;
    ptr_flag = (volatile int *) ptr;

    READBAR();
    if (*ptr_flag == MVP_SMP_CBUF_FREE || *ptr_flag == MVP_SMP_CBUF_PENDING)
        goto fn_exit;
    READBAR();
    if (*ptr_flag == MVP_SMP_CBUF_END) {
        s_header_ptr_r[recv_vc_ptr->smp.local_nodes] =
            MPIDI_MVP_SHMEM_FIRST_RECV(recv_vc_ptr->smp.local_nodes,
                                       mvp_smp_info.my_local_id);
        ptr = (void *)(mvp_smp_shmem_region->pool +
                       s_header_ptr_r[recv_vc_ptr->smp.local_nodes]);
        ptr_flag = (volatile int *) ptr;
        READBAR();
        if (*ptr_flag == MVP_SMP_CBUF_FREE || *ptr_flag == MVP_SMP_CBUF_PENDING)
            goto fn_exit;
    }

    READBAR();
    s_current_ptr[recv_vc_ptr->smp.local_nodes] = ptr;
    s_total_bytes[recv_vc_ptr->smp.local_nodes] = *(int*)((unsigned long)ptr +
            sizeof(int));
    s_current_bytes[recv_vc_ptr->smp.local_nodes] =
        s_total_bytes[recv_vc_ptr->smp.local_nodes];

    /* MPIR_T_PVAR_LEVEL_DEC(MVP, mvp_smp_rndv_avail_buffer, s_current_bytes[recv_vc_ptr->smp.local_nodes]); */

    rndv_buffer_max_usage = (rndv_buffer_max_usage > s_current_bytes[recv_vc_ptr->smp.local_nodes]) ?
        rndv_buffer_max_usage : s_current_bytes[recv_vc_ptr->smp.local_nodes];
    /* MPIR_T_PVAR_LEVEL_SET(MVP, mvp_smp_rndv_buffer_max_use, rndv_buffer_max_usage); */

    PRINT_DEBUG(DEBUG_SHM_verbose>1, "current byte %ld, total bytes %ld, iovlen %d, iov[0].len %ld\n",
         s_current_bytes[recv_vc_ptr->smp.local_nodes],
         s_total_bytes[recv_vc_ptr->smp.local_nodes], iovlen,
         iov[0].iov_len);
    WRITEBAR();

    s_current_ptr[recv_vc_ptr->smp.local_nodes] =
        (void *)((unsigned long) s_current_ptr[recv_vc_ptr->smp.local_nodes] +
            sizeof(int)*2);
    current_index = *((int *) s_current_ptr[recv_vc_ptr->smp.local_nodes]);
    smpi_complete_recv(recv_vc_ptr->smp.local_nodes, mvp_smp_info.my_local_id,
                       sizeof(int));
    } else {
    s_total_bytes[recv_vc_ptr->smp.local_nodes] =
        s_current_bytes[recv_vc_ptr->smp.local_nodes];
    current_index = recv_vc_ptr->smp.read_index;
    recv_offset = recv_vc_ptr->smp.read_off;
    }

    if (current_index != -1) {
    /** last smp packet has not been drained up yet **/
    PRINT_DEBUG(DEBUG_SHM_verbose>1, "iov_off %d, current bytes %ld, iov len %ld\n",
        iov_off, s_current_bytes[recv_vc_ptr->smp.local_nodes],
        iov[iov_off].iov_len);

    recv_buf = MPIDI_MVP_SHMEM_BUF_POOL_PTR(destination, current_index);

    if(recv_buf->busy != 1) {
        MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**fail",
            "**fail %s", "recv_buf->busy != 1");
    }

    msglen = recv_buf->len - recv_offset;
    current_buf = (void *)((unsigned long) &recv_buf->buf + recv_offset);
    iov_len = iov[0].iov_len;


    for (;
        iov_off < iovlen
        && s_current_bytes[recv_vc_ptr->smp.local_nodes] > 0
        && current_index != -1;) {

        if (msglen > iov_len) {
        READBAR();
        {
            MPIR_Memcpy((void *) ((unsigned long)iov[iov_off].iov_base
                        + buf_off),
                        (void *) current_buf,
                        iov_len);
        }

        /* MPIR_T_PVAR_COUNTER_INC(MVP, mvp_smp_rndv_received, iov_len); */
        /* MPIR_T_PVAR_LEVEL_INC(MVP, mvp_smp_rndv_avail_buffer, iov_len); */

        READBAR();
        current_buf = (void *) ((unsigned long) current_buf +
            iov_len);
        msglen -= iov_len;
        s_current_bytes[recv_vc_ptr->smp.local_nodes] -=
            iov_len;
        received_bytes += iov_len;
        buf_off = 0;
        ++iov_off;

        if (iov_off >= iovlen) {
            recv_vc_ptr->smp.read_index = current_index;
            recv_vc_ptr->smp.read_off = (unsigned long) current_buf -
            (unsigned long) &recv_buf->buf;
            break;
        }

        if (s_current_bytes[recv_vc_ptr->smp.local_nodes] == 0) {
            recv_buf->busy = 0;
            break;
        }

        else if (s_current_bytes[recv_vc_ptr->smp.local_nodes] < 0) {
            MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER,
                "**fail", "**fail %s",
                "s_current_bytes[recv_vc_ptr->smp.local_nodes] < 0");
        }

        iov_len = iov[iov_off].iov_len;
        } else if (msglen == iov_len) {
        READBAR();
        {
            MPIR_Memcpy((void *) ((unsigned long)iov[iov_off].iov_base
                        + buf_off),
                        (void *) current_buf,
                        iov_len);
        }

        /* MPIR_T_PVAR_COUNTER_INC(MVP, mvp_smp_rndv_received, iov_len); */
        /* MPIR_T_PVAR_LEVEL_INC(MVP, mvp_smp_rndv_avail_buffer, iov_len); */

        READBAR();
        s_current_bytes[recv_vc_ptr->smp.local_nodes] -=
            iov_len;
        received_bytes += iov_len;
        buf_off = 0;
        ++iov_off;

        if (iov_off >= iovlen) {
            recv_vc_ptr->smp.read_index = recv_buf->next;
            recv_vc_ptr->smp.read_off = 0;
            recv_buf->busy = 0;
            break;
        }
        if (s_current_bytes[recv_vc_ptr->smp.local_nodes] <= 0) {
            MPIR_Assert(s_current_bytes[recv_vc_ptr->smp.local_nodes] == 0);
            recv_buf->busy = 0;
            break;
        }

        iov_len = iov[iov_off].iov_len;

        if(recv_buf->has_next == 0){
            recv_buf->busy = 0;
            break;
        }

        current_index = recv_buf->next;
        recv_buf->busy = 0;
        recv_buf = MPIDI_MVP_SHMEM_BUF_POOL_PTR(destination, current_index);
        MPIR_Assert(recv_buf->busy == 1);
        msglen = recv_buf->len;
        current_buf = (void *) &recv_buf->buf;

        } else if (msglen > 0) {
        READBAR();

        {
            MPIR_Memcpy((void *) ((unsigned long)iov[iov_off].iov_base
                        + buf_off),
                        (void *) current_buf,
                        msglen);
        }

        /* MPIR_T_PVAR_COUNTER_INC(MVP, mvp_smp_rndv_received, msglen); */
        /* MPIR_T_PVAR_LEVEL_INC(MVP, mvp_smp_rndv_avail_buffer, msglen); */

        READBAR();
        iov_len -= msglen;
        received_bytes += msglen;
        buf_off += msglen;
        s_current_bytes[recv_vc_ptr->smp.local_nodes] -= msglen;

        if (s_current_bytes[recv_vc_ptr->smp.local_nodes] == 0) {
            recv_buf->busy = 0;
            break;
        }
        if(recv_buf->has_next == 0){
            recv_buf->busy = 0;
            break;
        }

        current_index = recv_buf->next;
        recv_buf->busy = 0;
        recv_buf = MPIDI_MVP_SHMEM_BUF_POOL_PTR(destination, current_index);
        MPIR_Assert(recv_buf->busy == 1);
        msglen = recv_buf->len;
        current_buf = (void *) &recv_buf->buf;
        }
    }
    *num_bytes_ptr += received_bytes;
    PRINT_DEBUG(DEBUG_SHM_verbose>1, "current bytes %ld, num_bytes %ld, iov_off %d, iovlen %d\n",
         s_current_bytes[recv_vc_ptr->smp.local_nodes], *num_bytes_ptr,
         iov_off, iovlen);

    if (0 == s_current_bytes[recv_vc_ptr->smp.local_nodes]) {
        READBAR();
        s_current_ptr[recv_vc_ptr->smp.local_nodes] = NULL;
        s_total_bytes[recv_vc_ptr->smp.local_nodes] = 0;
    }
    received_bytes = 0;
    if (iov_off == iovlen) {
        /* assert: s_current_ptr[recv_vc_ptr->smp.local_nodes] == 0 */
        goto fn_exit;
    }
    }

#if defined(_SMP_CMA_)
    }
#endif
fn_exit:
    PRINT_DEBUG(DEBUG_SHM_verbose>1, "return with nb %ld\n", *num_bytes_ptr);
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_SMP_READV_RNDV_CONT);
    return mpi_errno;

fn_fail:
    goto fn_exit;
}

int MPIDI_MVP_SMP_readv(MPIDI_MVP_ep_t * recv_vc_ptr, const struct iovec * iov, 
      const int iovlen, size_t  *num_bytes_ptr){
   
   
    int mpi_errno = MPI_SUCCESS;

    int iov_off = 0, buf_off=0;
    int received_bytes = 0;

    /* all variable must be declared before the state declarations */

    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_READV);

    *num_bytes_ptr = 0;

    if (s_current_ptr[recv_vc_ptr->smp.local_nodes] != NULL) {

        /* MPIR_T_PVAR_LEVEL_DEC(MVP, mvp_smp_eager_avail_buffer, s_current_bytes[recv_vc_ptr->smp.local_nodes]); */

        eager_buffer_max_usage = (eager_buffer_max_usage > s_current_bytes[recv_vc_ptr->smp.local_nodes]) ?
                                  eager_buffer_max_usage : s_current_bytes[recv_vc_ptr->smp.local_nodes];
        /* MPIR_T_PVAR_LEVEL_SET(MVP, mvp_smp_eager_buffer_max_use, eager_buffer_max_usage); */

        for (;
                iov_off < iovlen
                && s_current_bytes[recv_vc_ptr->smp.local_nodes] > 0;) {

            if (s_current_bytes[recv_vc_ptr->smp.local_nodes] >=
                    iov[iov_off].iov_len) {

                READBAR();

                {
                    MPIR_Memcpy((void *) iov[iov_off].iov_base,
                            s_current_ptr[recv_vc_ptr->smp.local_nodes],
                            iov[iov_off].iov_len);
                }

                /* MPIR_T_PVAR_COUNTER_INC(MVP, mvp_smp_eager_received, iov[iov_off].iov_len); */
                /* MPIR_T_PVAR_LEVEL_INC(MVP, mvp_smp_eager_avail_buffer, iov[iov_off].iov_len); */

                READBAR();
                s_current_ptr[recv_vc_ptr->smp.local_nodes] =
                    (void *) ((unsigned long)
                            s_current_ptr[recv_vc_ptr->smp.local_nodes] +
                            iov[iov_off].iov_len);
                s_current_bytes[recv_vc_ptr->smp.local_nodes] -=
                    iov[iov_off].iov_len;
                received_bytes += iov[iov_off].iov_len;
                ++iov_off;
            } else if (s_current_bytes[recv_vc_ptr->smp.local_nodes] > 0) {
                READBAR();

                {
                    MPIR_Memcpy((void *) iov[iov_off].iov_base,
                            (void *) s_current_ptr[recv_vc_ptr->smp.local_nodes],
                            s_current_bytes[recv_vc_ptr->smp.local_nodes]);
                }

                /* MPIR_T_PVAR_COUNTER_INC(MVP, mvp_smp_eager_received, s_current_bytes[recv_vc_ptr->smp.local_nodes]); */
                /* MPIR_T_PVAR_LEVEL_INC(MVP, mvp_smp_eager_avail_buffer, s_current_bytes[recv_vc_ptr->smp.local_nodes]); */

                READBAR();
                s_current_ptr[recv_vc_ptr->smp.local_nodes] =
                    (void *) ((unsigned long)
                            s_current_ptr[recv_vc_ptr->smp.local_nodes] +
                            s_current_bytes[recv_vc_ptr->smp.local_nodes]);
                received_bytes +=
                    s_current_bytes[recv_vc_ptr->smp.local_nodes];
                buf_off = s_current_bytes[recv_vc_ptr->smp.local_nodes];
                s_current_bytes[recv_vc_ptr->smp.local_nodes] = 0;
            }
        }
        *num_bytes_ptr += received_bytes;
        if (0 == s_current_bytes[recv_vc_ptr->smp.local_nodes]) {
            s_current_ptr[recv_vc_ptr->smp.local_nodes] = NULL;

            smpi_complete_recv(recv_vc_ptr->smp.local_nodes,
                               mvp_smp_info.my_local_id,
                               s_total_bytes[recv_vc_ptr->smp.local_nodes]);

            s_total_bytes[recv_vc_ptr->smp.local_nodes] = 0;
        }

        received_bytes = 0;
        if (iov_off == iovlen) {
            /* assert: s_current_ptr[recv_vc_ptr->smp.local_nodes] == 0 */
            goto fn_exit;
        }

    }
    WRITEBAR();

    void *ptr;
    volatile int *ptr_flag;
    ptr = (void *)(mvp_smp_shmem_region->pool +
                   s_header_ptr_r[recv_vc_ptr->smp.local_nodes]);
    ptr_flag = (volatile int*)ptr;

    READBAR();
    while (*ptr_flag != MVP_SMP_CBUF_FREE &&
           *ptr_flag != MVP_SMP_CBUF_PENDING) {
        READBAR();
        if (*ptr_flag == MVP_SMP_CBUF_END) {
            s_header_ptr_r[recv_vc_ptr->smp.local_nodes] =
                MPIDI_MVP_SHMEM_FIRST_RECV(recv_vc_ptr->smp.local_nodes,
                                           mvp_smp_info.my_local_id);

            ptr = (void *)(mvp_smp_shmem_region->pool +
                           s_header_ptr_r[recv_vc_ptr->smp.local_nodes]);
            ptr_flag = (volatile int*)ptr;

            READBAR();
            if (*ptr_flag == MVP_SMP_CBUF_FREE ||
                *ptr_flag == MVP_SMP_CBUF_PENDING)
                goto fn_exit;
        }

        READBAR();
        s_total_bytes[recv_vc_ptr->smp.local_nodes] = *(int*)((unsigned long)ptr +
                sizeof(int));
        ptr = (void *)((unsigned long)ptr + sizeof(int)*2);
        WRITEBAR();
        s_current_bytes[recv_vc_ptr->smp.local_nodes] =
            s_total_bytes[recv_vc_ptr->smp.local_nodes];

        /* MPIR_T_PVAR_LEVEL_DEC(MVP, mvp_smp_eager_avail_buffer, s_current_bytes[recv_vc_ptr->smp.local_nodes]); */
        eager_buffer_max_usage = (eager_buffer_max_usage > s_current_bytes[recv_vc_ptr->smp.local_nodes]) ?
                                  eager_buffer_max_usage : s_current_bytes[recv_vc_ptr->smp.local_nodes];
        /* MPIR_T_PVAR_LEVEL_SET(MVP, mvp_smp_eager_buffer_max_use, eager_buffer_max_usage); */

        READBAR();
        s_current_ptr[recv_vc_ptr->smp.local_nodes] = ptr;

        /****** starting to fill the iov buffers *********/
        for (;
                iov_off < iovlen
                && s_current_bytes[recv_vc_ptr->smp.local_nodes] > 0;) {

            if (s_current_bytes[recv_vc_ptr->smp.local_nodes] >=
                    iov[iov_off].iov_len - buf_off) {

                WRITEBAR();

                {
                    MPIR_Memcpy((void *) ((unsigned long) iov[iov_off].
                                iov_base + buf_off),
                            ptr, iov[iov_off].iov_len - buf_off);
                }

                /* MPIR_T_PVAR_COUNTER_INC(MVP, mvp_smp_eager_received, iov[iov_off].iov_len - buf_off); */
                /* MPIR_T_PVAR_LEVEL_INC(MVP, mvp_smp_eager_avail_buffer, iov[iov_off].iov_len - buf_off); */

                READBAR();
                s_current_bytes[recv_vc_ptr->smp.local_nodes] -=
                    (iov[iov_off].iov_len - buf_off);
                received_bytes += ( iov[iov_off].iov_len - buf_off);
                ptr = (void*)((unsigned long)ptr +
                        (iov[iov_off].iov_len - buf_off));
                ++iov_off;
                buf_off = 0;
            } else if (s_current_bytes[recv_vc_ptr->smp.local_nodes] > 0) {
                WRITEBAR();

                {
                    MPIR_Memcpy((void *) ((unsigned long) iov[iov_off].
                                iov_base + buf_off),
                            ptr, s_current_bytes[recv_vc_ptr->smp.local_nodes]);
                }

                /* MPIR_T_PVAR_COUNTER_INC(MVP, mvp_smp_eager_received, */
                /*         s_current_bytes[recv_vc_ptr->smp.local_nodes]); */
                /* MPIR_T_PVAR_LEVEL_INC(MVP, mvp_smp_eager_avail_buffer, */
                /*         s_current_bytes[recv_vc_ptr->smp.local_nodes]); */

                READBAR();
                ptr = (void*)((unsigned long)ptr + s_current_bytes[recv_vc_ptr->smp.local_nodes]);
                received_bytes +=
                    s_current_bytes[recv_vc_ptr->smp.local_nodes];
                buf_off += s_current_bytes[recv_vc_ptr->smp.local_nodes];
                s_current_bytes[recv_vc_ptr->smp.local_nodes] = 0;
            }
        }
        *num_bytes_ptr += received_bytes;
        s_current_ptr[recv_vc_ptr->smp.local_nodes] = ptr;

        /* update header */
        if (0 == s_current_bytes[recv_vc_ptr->smp.local_nodes]) {
            READBAR();
            s_current_ptr[recv_vc_ptr->smp.local_nodes] = NULL;
            smpi_complete_recv(recv_vc_ptr->smp.local_nodes,
                               mvp_smp_info.my_local_id,
                               s_total_bytes[recv_vc_ptr->smp.local_nodes]);
            s_total_bytes[recv_vc_ptr->smp.local_nodes] = 0;
        }
        received_bytes = 0;
        if (iov_off == iovlen) {
            goto fn_exit;
        }
        WRITEBAR();
    }
fn_exit:
    PRINT_DEBUG(DEBUG_SHM_verbose>4, "return with nb %ld\n", *num_bytes_ptr);
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_READV);
    return mpi_errno;
}
