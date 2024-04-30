/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda. For detailed
 * copyright and licensing information, please refer to the copyright file
 * COPYRIGHT in the top level MVAPICH directory.
 */

#ifndef _MVP_SMP_IMPL_H_
#define _MVP_SMP_IMPL_H_

/*
 * define this header check so that all our other headers can error out if thhis
 * is missing
 */
#define MVP_SMP_IMPL_INCLUDED

#include "mvp_smp.h"
#include "mvp_cache.h"
#include "mvp_shmem_bar.h"

typedef struct mvp_smp_shared_tails {
    volatile size_t ptr;
    char pad[SMPI_CACHE_LINE_SIZE - sizeof(unsigned int)];
} mvp_smp_shared_tails_t;

typedef struct mvp_smp_rq_limit {
    volatile size_t first;
    volatile size_t last;
} mvp_smp_rq_limit_t;

/* management information */
typedef struct mvp_smp_info {
    volatile void *mmap_ptr;
    void *send_buf_pool_ptr;
    unsigned int my_local_id;
    unsigned int num_local_nodes;
    short int only_one_device; /* to see if all processes are on one physical
                                  node */

    unsigned int *l2g_rank;
    size_t available_queue_length;
    int fd;
    int fd_pool;
    /*
    struct smpi_send_fifo_req *send_fifo_head;
    struct smpi_send_fifo_req *send_fifo_tail;
    unsigned int send_fifo_queued;
    unsigned int *local_nodes;
    int pending;
    */
} mvp_smp_info_t;

/* the shared area itself */
typedef struct mvp_smp_shmem_region {
    volatile int *pid; /* use for initial synchro */
    mvp_smp_shared_tails_t **shared_tails;
    mvp_smp_rq_limit_t *rqueues_limits_s;
    mvp_smp_rq_limit_t *rqueues_limits_r;
    /* the receives queues */
    char *pool;
    volatile char *test_buffer;
} mvp_smp_shmem_region_t;

/* structure for a buffer in the sending buffer pool */
typedef struct mvp_smp_send_buf {
    int myindex;
    volatile int next;
    volatile int busy;
    int len;
    volatile int has_next;
    int msg_complete;
    char buf[] __attribute__((aligned(SMPI_CACHE_LINE_SIZE)));
} mvp_smp_send_buf_t;

/* can we move this to an MPIR type? */
/* send queue, to be initialized */
typedef struct mvp_shared_buffer_pool {
    int free_head;
    int *send_queue;
    int *tail;
} mvp_shared_buffer_pool_t;

typedef struct polling_set_element {
    int rank;
    int prev;
    int next;
} mvp_smp_poll_t;

typedef enum {
    MVP_SMP_DMA_NONE,
    MVP_SMP_DMA_LIMIC,
    MVP_SMP_DMA_CMA,
} mvp_smp_dma_flag_t;

typedef enum {
    ONE_FREE = 1,
    TWO_FREE,
} smp_ctrl_avail_flag_t;

typedef enum {
    NO_FALLBACK = 0,
    FALLBACK,
} smp_fallback_flag_t;

/* TODO: rename */
extern mvp_smp_info_t mvp_smp_info;
extern mvp_smp_shmem_region_t *mvp_smp_shmem_region;
extern unsigned long eager_buffer_max_usage;
extern unsigned long rndv_buffer_max_usage;
extern struct mvp_shared_buffer_pool s_sh_buf_pool;

/* local header/tail ptr for send/receive in cyclic buffer */
extern size_t *s_header_ptr_s;
extern size_t *s_tail_ptr_s;
extern size_t *s_header_ptr_r;

extern int mvp_shmem_pool_init;
extern intptr_t *s_total_bytes;
extern intptr_t *s_current_bytes;
extern void **s_current_ptr;
extern long int mvp_num_queued_smp_ops;

 /* from shmem_init.h */
extern int mvp_shmem_pool_init;
extern size_t *s_header_ptr_s;
extern size_t *s_tail_ptr_s;
extern size_t *avail;
extern size_t *s_header_ptr_r;
extern intptr_t *s_total_bytes;
extern void **s_current_ptr;
extern intptr_t *s_current_bytes;
extern struct mvp_shared_buffer_pool s_sh_buf_pool;
extern mvp_smp_send_buf_t **s_buffer_head;
extern mvp_smp_send_buf_t *s_my_buffer_head;
extern int polling_set_p_head;
extern int mvp_is_fair_polling;
extern int polling_set_c_head;
extern int polling_set_c_tail;
extern int polling_set_p_head;
extern int polling_set_p_tail;
extern int polling_set_c_head;
extern int polling_set_c_tail;
extern mvp_smp_poll_t *polling_set_p;
extern mvp_smp_poll_t *polling_set_c;
extern mvp_smp_poll_t *polling_set_p;
extern mvp_smp_poll_t *polling_set_c;
extern int *polling_counters;

 /*********** Macro defines of local variables ************/
#define MVP_SHMEM_PRIORTY_THRESHOLD (24)
#define MVP_SHMEM_PRIORTY_FACTOR    (64)

#define PID_CHAR_LEN       22
#define SMPI_SMALLEST_SIZE (64)
#define SMPI_MAX_INT       ((unsigned int)(-1))

#define MVP_SMP_CBUF_FREE    0
#define MVP_SMP_CBUF_BUSY    1
#define MVP_SMP_CBUF_PENDING 2
#define MVP_SMP_CBUF_END     3

/* TODO: deprecate these CH3 macros */
#define MPIDI_CH3I_SPIN_COUNT_DEFAULT  100
#define MPIDI_CH3I_YIELD_COUNT_DEFAULT 5000

#define MPIDI_CH3I_READ_STATE_IDLE    0
#define MPIDI_CH3I_READ_STATE_READING 1

#define MPIDI_CH3I_CM_DEFAULT_ON_DEMAND_THRESHOLD       64
#define MPIDI_CH3I_RDMA_CM_DEFAULT_ON_DEMAND_THRESHOLD  512
#define MPIDI_CH3I_CM_DEFAULT_IWARP_ON_DEMAND_THRESHOLD 16
#define MPIDI_CH3I_RDMA_CM_DEFAULT_BASE_LISTEN_PORT     12000

/* TODO: CMA things - move these out of here to a DMA/CMA header */
/* SMP CMA parameters */

#if defined(_SMP_CMA_)

struct cma_header {
    struct iovec *remote;
    intptr_t total_bytes;
    pid_t pid;
    struct MPIR_Request *csend_req_id;
};
#endif /* _SMP_CMA_ */

#endif /* _MVP_SMP_IMPL_H_ */
