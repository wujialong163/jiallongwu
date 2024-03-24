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
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include "mvp_smp_progress_utils.h"
#include "mvp_req.h"
#include "mvp_req.h"
#include "mvp_smp_params.h"

extern unsigned long PVAR_COUNTER_mvp_smp_eager_sent;
extern unsigned long PVAR_COUNTER_mvp_smp_rndv_sent;
extern unsigned long PVAR_COUNTER_mvp_smp_eager_received;
extern unsigned long PVAR_COUNTER_mvp_smp_rndv_received;
extern unsigned long PVAR_LEVEL_mvp_smp_eager_total_buffer;
extern unsigned long PVAR_LEVEL_mvp_smp_rndv_total_buffer;
extern unsigned long PVAR_LEVEL_mvp_smp_eager_avail_buffer;
extern unsigned long PVAR_LEVEL_mvp_smp_rndv_avail_buffer;
extern unsigned long PVAR_LEVEL_mvp_smp_eager_buffer_max_use;
extern unsigned long PVAR_LEVEL_mvp_smp_rndv_buffer_max_use;

extern unsigned long PVAR_COUNTER_mvp_smp_read_progress_poll;
extern unsigned long PVAR_COUNTER_mvp_smp_write_progress_poll;
extern unsigned long PVAR_COUNTER_mvp_smp_read_progress_poll_success;
extern unsigned long PVAR_COUNTER_mvp_smp_write_progress_poll_success;

#if defined(_SMP_CMA_)  || defined (_SMP_LIMIC_)
void MPIDI_MVP_SMP_send_comp(void *c_header,
        MPIDI_MVP_ep_t* vc, intptr_t nb,
        mvp_smp_dma_flag_t dma_flag, smp_fallback_flag_t fallback);
#endif

int MPIDI_MVP_Handle_recv_req(MPIDI_MVP_ep_t *vc, MPIR_Request *rreq,
                              int *complete);

int MPIDI_MVP_Handle_send_req(MPIDI_MVP_ep_t *vc, MPIR_Request *sreq,
                              int *complete);

MPIR_Request *create_request(void *hdr, intptr_t hdr_sz, size_t nb);

/* TODO: deprecate after move in favor of smpi_complete_send */
void smpi_complete_send(unsigned int destination, unsigned int length,
                        int data_sz, volatile void *ptr,
                        volatile void *ptr_head, volatile void *ptr_flag);

/* TODO: may need a header for some of these rndv/recv functions */
int MPIDI_MVP_SMP_readv_rndv(MPIDI_MVP_ep_h recv_vc_ptr,
        const struct iovec *iov, const int iovlen, int index,
        void *limic_header, void *cma_header, size_t *num_bytes_ptr,
        mvp_smp_dma_flag_t dma_flag);


/* may require some CMA code - for R3 fallback */
/* RGET is default - always CMA
 *
 * If datatype is noncontig (possibly for very large messages as well) we fall
 * back to R3 which requires switches for CMA (not supported for noncontig)
 */
