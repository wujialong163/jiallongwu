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
#include "mvp_pkt.h"
#include "mvp_tagm.h"
#include "mvp_smp_params.h"
/* TODO: fixup includes */
#include "mvp_smp_progress_utils.h"
#ifdef _SMP_CMA_
/* TODO: probably make this a header with a static inline */
/* #include "mvp_cma_progress_write.h" */
int MPIDI_MVP_cma_writev_rndv_header(int tgt_local_rank,
                                     const struct iovec *iov, const int n,
                                     int *num_bytes_ptr,
                                     MPIDI_MVP_Pkt_rndv_r3_data_t *pkt_header,
                                     volatile void **ptr_in,
                                     volatile void **ptr_flag,
                                     int *write_len);
#endif

#define MVP_SMP_MY_BUF_POOL_PTR(index)                                         \
    ((mvp_smp_send_buf_t *)((unsigned long)s_my_buffer_head +                  \
                            (sizeof(mvp_smp_send_buf_t) +                      \
                             MVP_SMP_SEND_BUF_SIZE) *                          \
                                index))

static inline mvp_smp_send_buf_t *smp_get_buf()
{
    mvp_smp_send_buf_t *ptr;
    if (s_sh_buf_pool.free_head == -1) {
        ptr = NULL;
    } else {
        ptr = MVP_SMP_MY_BUF_POOL_PTR(s_sh_buf_pool.free_head);
        s_sh_buf_pool.free_head = ptr->next;
        ptr->next = -1;
        MPIR_Assert(!ptr->busy);
    }
    return ptr;
}

static inline int smp_link_buf(int dest, int index)
{
    int mpi_errno = MPI_SUCCESS;
    if (s_sh_buf_pool.send_queue[dest] == -1) {
        s_sh_buf_pool.send_queue[dest] = index;
    } else {
        MVP_SMP_MY_BUF_POOL_PTR(s_sh_buf_pool.tail[dest])->next = index;
    }
    s_sh_buf_pool.tail[dest] = index;
    return mpi_errno;
}

static inline void smp_release_buf(int head, int tail)
{
    mvp_smp_send_buf_t *ptr;

    MPIR_Assert(head != -1);
    MPIR_Assert(tail != -1);

    ptr = MVP_SMP_MY_BUF_POOL_PTR(tail);

    ptr->next = s_sh_buf_pool.free_head;
    s_sh_buf_pool.free_head = head;
}

static inline void smp_send_buf_reclaim()
{
    int i, index, last_index;
    mvp_smp_send_buf_t *ptr;

    for (i = 0; i < mvp_smp_info.num_local_nodes; ++i) {
        /* TODO: do we deliberately ignore the leader? */
        if (i != mvp_smp_info.my_local_id) {
            index = s_sh_buf_pool.send_queue[i];
            last_index = -1;
            //ptr = NULL;
            ptr = MVP_SMP_MY_BUF_POOL_PTR(index);
            while (index != -1 && !ptr->busy) {
                last_index = index;
                index = ptr->next;
                ptr = MVP_SMP_MY_BUF_POOL_PTR(index);
            }
            if (last_index != -1) {
                smp_release_buf(s_sh_buf_pool.send_queue[i], last_index);
            }
            s_sh_buf_pool.send_queue[i] = index;
            if (s_sh_buf_pool.send_queue[i] == -1) {
                s_sh_buf_pool.tail[i] = -1;
            }
        }
    }
}

static inline int smp_complete_send(unsigned int destination,
                                    unsigned int length, int data_sz,
                                    volatile void *ptr,
                                    volatile void *ptr_head,
                                    volatile void *ptr_flag)
{
    s_header_ptr_s[destination] += length + sizeof(int) * 2;
    /* set next flag to free */
    *((int *)ptr_head) = data_sz;
    *((int *)ptr) = MVP_SMP_CBUF_FREE;
    WRITEBAR();
    /* set current flag to busy */
    *((int *)ptr_flag) = MVP_SMP_CBUF_BUSY;
    WRITEBAR();
    avail[destination] -= length + sizeof(int) * 2;

    return MPI_SUCCESS;
}

static inline int MPIDI_MVP_shmem_writev_rndv_header(
    int tgt_local_rank, const struct iovec *iov, const int n,
    int *num_bytes_ptr, MPIDI_MVP_Pkt_rndv_r3_data_t *pkt_header,
    volatile void **ptr_in, volatile void **ptr_flag, int *write_len)
{
    int mpi_errno = MPI_SUCCESS;
    int pkt_len = 0;
    int i;
    int len = iov[0].iov_len;
    volatile void *ptr = *ptr_in;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_SHMEM_WRITEV_RNDV_HEADER);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SHMEM_WRITEV_RNDV_HEADER);

    if (!mvp_shmem_pool_init) {
        mpi_errno = MPIDI_MVP_smp_attach_shm_pool_inline();
        MPIR_ERR_CHECK(mpi_errno);
    }

    /* check if avail is less than data size */
    if (!smpi_check_avail(tgt_local_rank, len, ptr_flag, TWO_FREE)) {
        /* TODO: return what here? Is this a failure or just a NULL op */
        goto fn_exit;
    }

    send_buf_reclaim();

    if (s_sh_buf_pool.free_head == -1) {
        goto fn_exit;
    }

    pkt_header->src.smp_index = s_sh_buf_pool.free_head;

    for (i = 0; i < n; i++) {
        MPIR_Memcpy((void *)ptr, (void *)((unsigned long)iov[i].iov_base),
                    iov[i].iov_len);
        ptr = (volatile void *)((unsigned long)ptr + iov[i].iov_len);
        pkt_len += iov[i].iov_len;
    }

    MPIR_Assert(len == pkt_len);
    *num_bytes_ptr += pkt_len;
    *write_len = len;
    *ptr_in = ptr;

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SHMEM_WRITEV_RNDV_HEADER);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIDI_MVP_smp_write_rndv_header(int tgt_local_rank,
                                    const struct iovec *iov,
                                    const int n, int *num_bytes_ptr)
{
    int mpi_errno = MPI_SUCCESS;
    int write_len = 0;
    int i;
    volatile void *ptr_head = NULL, *ptr = NULL, *ptr_flag = NULL;
    MPIDI_MVP_Pkt_rndv_r3_data_t *pkt_header = NULL;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_SMP_WRITE_RNDV_HEADER);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_WRITE_RNDV_HEADER);

    *num_bytes_ptr = 0;

    /* iov[0] is the header pkt */
    pkt_header = (MPIDI_MVP_Pkt_rndv_r3_data_t *)(iov[0].iov_base);

    mpi_errno = MPIDI_MVP_smp_check_pool();
    MPIR_ERR_CHECK(mpi_errno);

    /* these are not shmem specific */
    ptr_flag = (volatile void *)(mvp_smp_shmem_region->pool) +
               s_header_ptr_s[tgt_local_rank];
    ptr_head = (volatile void *)((unsigned long)ptr_flag + sizeof(int));
    ptr = (volatile void *)((unsigned long)ptr_flag + 2 * sizeof(int));

#ifdef _SMP_CMA_
    /* fall back to shmem for non-contig data (csend_req_id is NULL) */
    if (MVP_SMP_USE_CMA && pkt_header->csend_req_id) {
        mpi_errno =
            MPIDI_MVP_cma_writev_rndv_header(tgt_local_rank, iov, n,
                                             num_bytes_ptr, pkt_header, &ptr,
                                             &ptr_flag, &write_len);
    } else
#endif
    mpi_errno =
        MPIDI_MVP_shmem_writev_rndv_header(tgt_local_rank, iov, n,
                                           num_bytes_ptr, pkt_header, &ptr,
                                           &ptr_flag, &write_len);
    smp_complete_send(tgt_local_rank, write_len, write_len, ptr, ptr_head,
                      ptr_flag);

fn_exit:
    PRINT_DEBUG(DEBUG_SHM_verbose > 1, "writev_rndv_header returns bytes %d\n",
                *num_bytes_ptr);
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_WRITE_RNDV_HEADER);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIDI_MVP_smp_write_rndv_data(int tgt_local_rank, const struct iovec *iov,
                                  const int n, int is_contig,
                                  int *num_bytes_ptr)
{
    int i = 0;
    int len = 0;
    int pkt_len = 0;
    int has_sent = 0;
    int pkt_avail = 0;
    int first_index = 0;
    int mpi_errno = MPI_SUCCESS;
    int destination = tgt_local_rank;
    mvp_smp_send_buf_t *send_buf = NULL;
    mvp_smp_send_buf_t *tmp_buf = NULL;
    volatile void *ptr_head = NULL, *ptr_flag = NULL, *ptr = NULL;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_SMP_WRITE_RNDV_DATA);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_WRITE_RNDV_DATA);

    *num_bytes_ptr = 0;
    ptr_flag = (volatile void *)((mvp_smp_shmem_region->pool) +
                                 s_header_ptr_s[tgt_local_rank]);

    /* check if avail is less than data size */
    if (!smpi_check_avail(tgt_local_rank, len, &ptr_flag, ONE_FREE)) {
        /* TODO: return what here? Is this a failure or just a NULL op */
        goto fn_exit;
    }

    /* try to make sure we have buffers */
    send_buf_reclaim();

    if (s_sh_buf_pool.free_head == -1) {
        MPIR_ERR_SETFATALANDJUMP(mpi_errno, MPI_ERR_OTHER,
                                 "**mvp_shared_buf_full");
    }
    first_index = s_sh_buf_pool.free_head;

    while ((s_sh_buf_pool.free_head != -1) && (i != n) &&
           (has_sent < MVP_SMP_BATCH_SIZE)) {
        int offset = 0;
        int pkt_len = 0;
        for (i = 0; i < n && has_sent < MVP_SMP_BATCH_SIZE; ++has_sent) {
            int pkt_avail = MVP_SMP_SEND_BUF_SIZE;
            send_buf = smp_get_buf();
            if (!send_buf) {
                /*
                 * break out and try to reclaim buffers - 
                 * only time we actually leave this loop and interate the while
                 * loop is there a better way?
                 */
                break;
            }
            if (pkt_avail >= (iov[i].iov_len - offset)) {
                pkt_avail = iov[i].iov_len - offset; 
                ++i;
            }
            MPIR_Memcpy(&send_buf->buf, iov[i].iov_base + offset,
                        pkt_avail);
            /* TODO: reimplement PVARs
            MPIR_T_PVAR_COUNTER_INC(MVP, mvp_smp_rndv_sent,
                                    iov[i].iov_len - offset);
            */
            send_buf->busy = 1;
            send_buf->len = pkt_avail;
            send_buf->has_next = 1;

            mpi_errno = smp_link_buf(destination, send_buf->myindex);
            MPIR_ERR_CHECK(mpi_errno);

            tmp_buf = send_buf;

            pkt_len += pkt_avail;
            offset = pkt_avail < MVP_SMP_SEND_BUF_SIZE ? offset + pkt_avail : 0;
        }
        *num_bytes_ptr += pkt_len;
        smp_send_buf_reclaim();
    }
    if (tmp_buf) {
        tmp_buf->has_next = 0;
    }

    ptr_head = (void *)((unsigned long)ptr_flag + sizeof(int));
    ptr = (void *)((unsigned long)ptr_flag + 2 * sizeof(int));

    if (is_contig) {
        *((int *)ptr) = first_index;
        ptr = (void *)((unsigned long)ptr + sizeof(int));
    }

    smp_complete_send(tgt_local_rank, len, *num_bytes_ptr, ptr, ptr_head,
                      ptr_flag);

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_WRITE_RNDV_DATA);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIDI_MVP_smp_write(int tgt_local_rank, const struct iovec *iov,
                        const int n, int *num_bytes_ptr)
{
    int i = 0;
    int len = 0;
    int pkt_len = 0;
    int mpi_errno = MPI_SUCCESS;
    volatile void *ptr_head = NULL, *ptr_flag = NULL, *ptr = NULL;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_SMP_WRITE);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_WRITE);

    for (i = 0; i < n; ++i) {
        len += iov[i].iov_len;
    }
    *num_bytes_ptr = 0;
    ptr_flag = (void *)((mvp_smp_shmem_region->pool) +
                        s_header_ptr_s[tgt_local_rank]);

    if(!smpi_check_avail(tgt_local_rank, len, &ptr_flag, ONE_FREE)) {
        /* TODO: return what here? Is this a failure or just a NULL op */
        goto fn_exit;
    }
    ptr_head = (void *)((unsigned long)ptr_flag + sizeof(int));
    ptr = (void *)((unsigned long)ptr_flag + sizeof(int) * 2);

    for (i = 0; i < n; i++) {
        MPIR_Memcpy((void *)ptr, iov[i].iov_base, iov[i].iov_len);

        /* MPIR_T_PVAR_COUNTER_INC(MVP, mvp_smp_eager_sent, iov[i].iov_len); */

        ptr = (void *)((unsigned long)ptr + iov[i].iov_len);
        pkt_len += iov[i].iov_len;
    }

    /* update(header) */
    smp_complete_send(tgt_local_rank, len, len, ptr, ptr_head, ptr_flag);
    /* For a CMA based trasnfer, we expect a FIN message */
    MVP_INC_NUM_POSTED_RECV();

    *num_bytes_ptr += pkt_len;

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_WRITE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
