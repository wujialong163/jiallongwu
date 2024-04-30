/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 *
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda.
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level MVAPICH directory.
 */

#include "mvp_smp_impl.h"
#include "mvp_shmem.h"
#include "mvp_vc.h"


#if defined(_SMP_CMA_)
extern size_t MVP_CMA_MSG_LIMIT;
#endif

void smpi_complete_recv(int from_grank, int my_id, int length);
int MPIDI_MVP_SMP_do_cma_read(const struct iovec *iov, const int iovlen,
                              void *cma_header, size_t *num_bytes_ptr);

int MPIDI_MVP_SMP_readv_rndv(MPIDI_MVP_ep_h recv_vc_ptr, const struct iovec * iov,
        const int iovlen, int index, void *limic_header,
        void *cma_header, size_t *num_bytes_ptr, mvp_smp_dma_flag_t dma_flag)
{
    int mpi_errno = MPI_SUCCESS;
   size_t iov_off = 0, buf_off = 0;
   size_t received_bytes = 0;
   size_t msglen, iov_len;
   /* all variable must be declared before the state declarations */

   int destination = recv_vc_ptr->smp.local_nodes;
   int current_index = index;
   void *current_buf;
   mvp_smp_send_buf_t *recv_buf;

   MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_READ_RNDV);

   if ( !mvp_shmem_pool_init )
   {
       if ((mpi_errno = MPIDI_MVP_smp_attach_shm_pool_inline()) !=
               MPI_SUCCESS)
       {
           MPIR_ERR_POP(mpi_errno);
       }
   }

   *num_bytes_ptr = 0;

    if (0 == s_current_bytes[recv_vc_ptr->smp.local_nodes]) {
    READBAR();
    s_current_ptr[recv_vc_ptr->smp.local_nodes] = NULL;

    smpi_complete_recv(recv_vc_ptr->smp.local_nodes, mvp_smp_info.my_local_id,
                       s_total_bytes[recv_vc_ptr->smp.local_nodes]);

    s_total_bytes[recv_vc_ptr->smp.local_nodes] = 0;
    }
#if defined(_SMP_CMA_)
    if (dma_flag == MVP_SMP_DMA_CMA) {
        mpi_errno = MPIDI_MVP_SMP_do_cma_read(
                iov, iovlen, cma_header, num_bytes_ptr);
        if (mpi_errno) {
            MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER,
                        "**fail", "**fail %s",
                        "CMA: MPIDI_MVP_SMP_readv_rndv fail");
        }
    } else {
#endif /* _SMP_CMA_ */

#if defined(_SMP_LIMIC_)
    if (dma_flag == MVP_SMP_DMA_LIMIC) {
        mpi_errno = MPIDI_MVP_SMP_do_limic_read(
                recv_vc_ptr, iov, iovlen, limic_header, num_bytes_ptr);
    } else {
#endif /* _SMP_LIMIC_ */

    volatile void *ptr;
    volatile int *ptr_flag;
    ptr = (void *)((unsigned long)mvp_smp_shmem_region->pool +
                   s_header_ptr_r[recv_vc_ptr->smp.local_nodes]);
    ptr_flag = (volatile int *) ptr;
    READBAR();
    while (*ptr_flag == MVP_SMP_CBUF_FREE ||
           *ptr_flag == MVP_SMP_CBUF_PENDING) {
        READBAR();
    }

    READBAR();
    if (*ptr_flag == MVP_SMP_CBUF_END) {
        s_header_ptr_r[recv_vc_ptr->smp.local_nodes] =
            MPIDI_MVP_SHMEM_FIRST_RECV(recv_vc_ptr->smp.local_nodes,
                                       mvp_smp_info.my_local_id);
        ptr = (volatile void *)((unsigned long)mvp_smp_shmem_region->pool +
                                s_header_ptr_r[recv_vc_ptr->smp.local_nodes]);
        ptr_flag = (volatile int *)ptr;
        READBAR();
        while (*ptr_flag == MVP_SMP_CBUF_FREE ||
               *ptr_flag == MVP_SMP_CBUF_PENDING) {
            READBAR();
        }
    }

    READBAR();
    s_current_ptr[recv_vc_ptr->smp.local_nodes] = (void *)ptr;
    s_total_bytes[recv_vc_ptr->smp.local_nodes] = *(int*)((unsigned long)ptr +
            sizeof(int));
    s_current_bytes[recv_vc_ptr->smp.local_nodes] =
    s_total_bytes[recv_vc_ptr->smp.local_nodes];
    smpi_complete_recv(recv_vc_ptr->smp.local_nodes, mvp_smp_info.my_local_id,
                       0);
    PRINT_DEBUG(DEBUG_SHM_verbose>1, "current byte %ld, total bytes %ld, iovlen %d, iov[0].len %ld\n",
     s_current_bytes[recv_vc_ptr->smp.local_nodes],
     s_total_bytes[recv_vc_ptr->smp.local_nodes], iovlen,
     iov[0].iov_len);

    //MPIR_T_PVAR_LEVEL_DEC(MVP, mvp_smp_rndv_avail_buffer, s_current_bytes[recv_vc_ptr->smp.local_nodes]);
    rndv_buffer_max_usage = (rndv_buffer_max_usage > s_current_bytes[recv_vc_ptr->smp.local_nodes]) ?
        rndv_buffer_max_usage : s_current_bytes[recv_vc_ptr->smp.local_nodes];
    //MPIR_T_PVAR_LEVEL_SET(MVP, mvp_smp_rndv_buffer_max_use, rndv_buffer_max_usage);

    WRITEBAR();

    if (current_index != -1) {
    /** last smp packet has not been drained up yet **/
    PRINT_DEBUG(DEBUG_SHM_verbose>1, "iov_off %ld, current bytes %ld, iov len %ld\n",
        iov_off, s_current_bytes[recv_vc_ptr->smp.local_nodes],
        iov[iov_off].iov_len);

    recv_buf = MPIDI_MVP_SHMEM_BUF_POOL_PTR(destination, current_index);

    if(recv_buf->busy != 1) {
        MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**fail",
            "**fail %s", "recv_buf->busy == 1");
    }

    msglen = recv_buf->len;
    current_buf = (void *) &recv_buf->buf;
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

//        MPIR_T_PVAR_COUNTER_INC(MVP, mvp_smp_rndv_received, iov_len);
//        MPIR_T_PVAR_LEVEL_INC(MVP, mvp_smp_rndv_avail_buffer, iov_len);

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
        if (s_current_bytes[recv_vc_ptr->smp.local_nodes] <= 0) {
            MPIR_Assert(s_current_bytes[recv_vc_ptr->smp.local_nodes] == 0);
            recv_buf->busy = 0;
            break;
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

//        MPIR_T_PVAR_COUNTER_INC(MVP, mvp_smp_rndv_received, iov_len);
//        MPIR_T_PVAR_LEVEL_INC(MVP, mvp_smp_rndv_avail_buffer, iov_len);

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

        if (s_current_bytes[recv_vc_ptr->smp.local_nodes] == 0) {
            recv_buf->busy = 0;
            break;
        }

        else if(s_current_bytes[recv_vc_ptr->smp.local_nodes] < 0) {
            MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER,
                "**fail", "**fail %s",
                "s_current_bytes[recv_vc_ptr->smp.local_nodes] < 0");
        }

        iov_len = iov[iov_off].iov_len;

        if(recv_buf->has_next == 0){
            recv_buf->busy = 0;
            break;
        }

        current_index = recv_buf->next;
        recv_buf->busy = 0;
        recv_buf = MPIDI_MVP_SHMEM_BUF_POOL_PTR(destination, current_index);

        if(recv_buf->busy != 1) {
            MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER,
                "**fail", "**fail %s", "recv_buf->busy != 1");
        }

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

//        MPIR_T_PVAR_COUNTER_INC(MVP, mvp_smp_rndv_received, msglen);
//        MPIR_T_PVAR_LEVEL_INC(MVP, mvp_smp_rndv_avail_buffer, msglen);

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

        if(recv_buf->busy != 1) {
            MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER,
                "**fail", "**fail %s", "recv_buf->busy != 1");
        }

        msglen = recv_buf->len;
        current_buf = (void *) &recv_buf->buf;
        }
    }
    *num_bytes_ptr += received_bytes;
    PRINT_DEBUG(DEBUG_SHM_verbose>1, "current bytes %ld, num_bytes %ld, iov_off %ld, iovlen %d\n",
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
#if defined(_SMP_LIMIC_)
    }
#endif
#if defined(_SMP_CMA_)
    }
#endif
fn_exit:
    PRINT_DEBUG(DEBUG_SHM_verbose>1, "return with nb %ld\n", *num_bytes_ptr);
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_READ_RNDV);
    return mpi_errno;

fn_fail:
    goto fn_exit;
}



#if defined(_SMP_CMA_)
int MPIDI_MVP_SMP_do_cma_read(const struct iovec * iov,
        const int iovlen, void *cma_header,
        size_t *num_bytes_ptr)
{
    int mpi_errno = MPI_SUCCESS;
    size_t cerr;
    pid_t pid;
    struct cma_header *c_header = (struct cma_header *) cma_header;
    size_t cma_total_bytes = c_header->total_bytes;
    struct iovec *local_iovec;
    size_t msglen, iov_len;
    int iov_off = 0, buf_off = 0;
    size_t received_bytes = 0;

    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_DO_CMA_READ);

    local_iovec = (struct iovec *)iov;
    pid = c_header->pid;
    msglen = cma_total_bytes;
    iov_len = iov[0].iov_len;
    for (; cma_total_bytes > 0 && iov_off < iovlen; ) {
        if (unlikely(msglen > MVP_CMA_MSG_LIMIT)) {
            local_iovec[iov_off].iov_len = MVP_CMA_MSG_LIMIT;
            cerr = process_vm_readv(pid, &local_iovec[iov_off], 1, c_header->remote, 1, 0);
            if( cerr == -1 )
                MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER,
                        "**fail", "**fail %s",
                        "CMA: (MPIDI_MVP_SMP_do_cma_read) process_vm_readv fail");

            MPIR_Assert(cerr == MVP_CMA_MSG_LIMIT);
            local_iovec[iov_off].iov_base += MVP_CMA_MSG_LIMIT;
            local_iovec[iov_off].iov_len = iov_len - MVP_CMA_MSG_LIMIT;
            received_bytes += MVP_CMA_MSG_LIMIT;
            cma_total_bytes -= MVP_CMA_MSG_LIMIT;
            msglen -= MVP_CMA_MSG_LIMIT;
            iov_len -= MVP_CMA_MSG_LIMIT;

            c_header->remote[0].iov_len -= MVP_CMA_MSG_LIMIT;
            c_header->remote[0].iov_base += MVP_CMA_MSG_LIMIT;


        } else if (msglen == iov_len) {
            local_iovec[iov_off].iov_base += buf_off;
            cerr = process_vm_readv(pid, &local_iovec[iov_off], 1, c_header->remote, 1, 0);
            if( cerr == -1 )
                MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER,
                        "**fail", "**fail %s",
                        "CMA: (MPIDI_MVP_SMP_do_cma_read) process_vm_readv fail");

            MPIR_Assert(cerr == msglen);
            received_bytes += msglen;
            cma_total_bytes -= msglen;

            MPIR_Assert(cma_total_bytes == 0 && ++iov_off >= iovlen);

        } else if (msglen > iov_len) {
            local_iovec[iov_off].iov_base += buf_off;
            cerr = process_vm_readv(pid, &local_iovec[iov_off], 1, c_header->remote, 1, 0);
            if( cerr == -1 )
                MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER,
                        "**fail", "**fail %s",
                        "CMA: (MPIDI_MVP_SMP_do_cma_read) process_vm_readv fail");

            MPIR_Assert(cerr == iov_len);
            received_bytes += iov_len;
            cma_total_bytes -= iov_len;
            msglen -= iov_len;

            c_header->remote[0].iov_len -= iov_len;
            c_header->remote[0].iov_base += iov_len;

            if (++iov_off >= iovlen)
                break;
            buf_off = 0;
            iov_len = iov[iov_off].iov_len;

        }  else if (msglen > 0) {
            local_iovec[iov_off].iov_base += buf_off;
            cerr = process_vm_readv(pid, &local_iovec[iov_off], 1, c_header->remote, 1, 0);
            if( cerr == -1 )
                MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER,
                        "**fail", "**fail %s",
                        "CMA: (MPIDI_MVP_SMP_do_cma_read) process_vm_readv fail");

            MPIR_Assert(cerr == msglen);
            received_bytes += msglen;
            cma_total_bytes -= msglen;
        }
    }
    *num_bytes_ptr = received_bytes;
    c_header->total_bytes -= received_bytes;

fn_exit:
    PRINT_DEBUG(DEBUG_SHM_verbose>1, "return with nb %ld\n", *num_bytes_ptr);
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_DO_CMA_READ);
    return mpi_errno;

fn_fail:
    goto fn_exit;
}
#endif
