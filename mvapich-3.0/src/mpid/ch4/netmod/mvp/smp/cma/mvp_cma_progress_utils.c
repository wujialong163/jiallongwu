/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda. For detailed
 * copyright and licensing information, please refer to the copyright file
 * COPYRIGHT in the top level MVAPICH directory.
 */

#include "mvp_smp_impl.h"
#include "mvp_pkt.h"
#include "mvp_tagm.h"
#include "mvp_smp_progress_utils.h"

int MPIDI_MVP_cma_writev_rndv_header(int tgt_local_rank,
                                     const struct iovec *iov, const int n,
                                     int *num_bytes_ptr,
                                     MPIDI_MVP_Pkt_rndv_r3_data_t *pkt_header,
                                     volatile void **ptr_in,
                                     volatile void **ptr_flag,
                                     int *write_len)
{
    size_t cma_total_bytes = 0;
    int pkt_len, i;
    MPIR_Request *creq = NULL;
    pid_t pid;
    int mpi_errno = MPI_SUCCESS;
    volatile void *ptr = *ptr_in;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_CMA_WRITEV_RNDV_HEADER);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_CMA_WRITEV_RNDV_HEADER);

    creq = (pkt_header->csend_req_id);

    MPIR_Assert(MPIDI_MVP_REQUEST(creq, dev.iov_count) == 1);

    /* The last sizeof(intptr_t) is the total num of data bytes */
    /* TODO: we just asserted that iov_count is 1 ... so why multiply? */
    pkt_len = iov[0].iov_len +
              sizeof(struct iovec) * MPIDI_MVP_REQUEST(creq, dev.iov_count) +
              sizeof(pid_t) + sizeof(intptr_t);

    /* check if avail is less than data size */
    if (!smpi_check_avail(tgt_local_rank, pkt_len, ptr_flag, ONE_FREE)) {
        goto fn_exit;
    }

    /* header format:
     * flag | pkt_len | normal header | iov | pid | total_num_size
     */
    MPIR_Memcpy((void *)ptr, iov[0].iov_base, iov[0].iov_len);
    ptr = (volatile void *)((unsigned long)ptr + iov[0].iov_len);

    for (i = 0; i < MPIDI_MVP_REQUEST(creq, dev).iov_count; ++i) {
        cma_total_bytes += MPIDI_MVP_REQUEST(creq, dev).iov[i].iov_len;

        /* copy the limic_user information to the shared memory
           and move the shared memory pointer properly
         */
        MPIR_Memcpy((void *)ptr, &(MPIDI_MVP_REQUEST(creq, dev).iov[i]),
                    sizeof(struct iovec));
        ptr = (volatile void *)((unsigned long)ptr + sizeof(struct iovec));
    }
    pid = mvp_smp_shmem_region->pid[mvp_smp_info.my_local_id];
    *((volatile pid_t *)ptr) = pid;
    ptr = (volatile void *)((unsigned long)ptr + sizeof(pid_t));
    *((volatile intptr_t *)ptr) = cma_total_bytes;
    ptr = (volatile void *)((unsigned long)ptr + sizeof(intptr_t));

    *num_bytes_ptr = iov[0].iov_len;
    *write_len = pkt_len;
    *ptr_in = ptr;
    /* For a CMA based trasnfer, we expect a FIN message */
    MVP_INC_NUM_POSTED_RECV();

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_CMA_WRITEV_RNDV_HEADER);
    return mpi_errno;
}
