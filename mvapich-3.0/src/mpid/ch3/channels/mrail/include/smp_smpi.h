/*
 * This source file was derived from code in the MPICH-GM implementation
 * of MPI, which was developed by Myricom, Inc.
 * Myricom MPICH-GM ch_gm backend
 * Copyright (c) 2001 by Myricom, Inc.
 * All rights reserved.
 */

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

#ifndef _SMPI_SMP_
#define _SMPI_SMP_

#ifdef _SMP_LIMIC_
#include <limic.h>
#endif

#include <mvp_shmem_bar.h>

#if defined(_ENABLE_CUDA_) && defined(HAVE_CUDA_IPC)
extern void **smp_device_region_send;
extern void **smp_device_region_recv;
extern int smp_device_region_size;

extern deviceEvent_t *sr_event;
extern deviceEvent_t *sr_event_local;
extern deviceEvent_t *loop_event;
extern deviceEvent_t *loop_event_local;
#endif

typedef struct polling_set_element {
    int rank;
    int prev;
    int next;
} POLLING_ELEMENT_T;

/*********** Macro defines of local variables ************/
#define MVP_SHMEM_PRIORTY_THRESHOLD (24)
#define MVP_SHMEM_PRIORTY_FACTOR    (64)

#define PID_CHAR_LEN 22

#define SMPI_SMALLEST_SIZE (64)

#define SMPI_MAX_INT ((unsigned int)(-1))

#if defined(_IA32_)

#define SMPI_CACHE_LINE_SIZE 64
#define SMPI_ALIGN(a)        ((a + SMPI_CACHE_LINE_SIZE + 7) & 0xFFFFFFF8)
#define SMPI_AVAIL(a)        ((a & 0xFFFFFFF8) - SMPI_CACHE_LINE_SIZE)

#elif defined(_IA64_) || defined(__powerpc__) || defined(__ppc__) ||           \
    defined(__PPC__) || defined(__powerpc64__) || defined(__ppc64__) ||        \
    defined(__PPC64__)

#define SMPI_CACHE_LINE_SIZE 128
#define SMPI_ALIGN(a)        ((a + SMPI_CACHE_LINE_SIZE + 7) & 0xFFFFFFFFFFFFFFF8)
#define SMPI_AVAIL(a)        ((a & 0xFFFFFFFFFFFFFFF8) - SMPI_CACHE_LINE_SIZE)

#elif defined(__x86_64__) && defined(_AMD_QUAD_CORE_)

#define SMPI_CACHE_LINE_SIZE 128
#define SMPI_ALIGN(a)        ((a + SMPI_CACHE_LINE_SIZE + 7) & 0xFFFFFFFFFFFFFFF8)
#define SMPI_AVAIL(a)        ((a & 0xFFFFFFFFFFFFFFF8) - SMPI_CACHE_LINE_SIZE)

#elif defined(__x86_64__)

#define SMPI_CACHE_LINE_SIZE 64
#define SMPI_ALIGN(a)        ((a + SMPI_CACHE_LINE_SIZE + 7) & 0xFFFFFFFFFFFFFFF8)
#define SMPI_AVAIL(a)        ((a & 0xFFFFFFFFFFFFFFF8) - SMPI_CACHE_LINE_SIZE)

#elif defined(_EM64T_)

#define SMPI_CACHE_LINE_SIZE 64
#define SMPI_ALIGN(a)        (a + SMPI_CACHE_LINE_SIZE)

#define SMPI_AVAIL(a) ((a & 0xFFFFFFFFFFFFFFF8) - SMPI_CACHE_LINE_SIZE)

#elif defined(MAC_OSX)

#define SMPI_CACHE_LINE_SIZE 16
#define SMPI_ALIGN(a)        (((a + SMPI_CACHE_LINE_SIZE + 7) & 0xFFFFFFF8))
#define SMPI_AVAIL(a)        ((a & 0xFFFFFFF8) - SMPI_CACHE_LINE_SIZE)

#else

#define SMPI_CACHE_LINE_SIZE 64
#define SMPI_ALIGN(a)        (a + SMPI_CACHE_LINE_SIZE)

#define SMPI_AVAIL(a) ((a & 0xFFFFFFFFFFFFFFF8) - SMPI_CACHE_LINE_SIZE)

#endif

typedef struct {
    volatile unsigned int current;
} smpi_params_c;

typedef struct {
    volatile unsigned int next;
} smpi_params_n;

typedef struct {
    volatile unsigned int msgs_total_in;
    volatile unsigned int msgs_total_out;
    char pad[SMPI_CACHE_LINE_SIZE / 2 - 8];
} smpi_rqueues;

#if defined(_ENABLE_CUDA_) && defined(HAVE_CUDA_IPC)
typedef struct {
    volatile unsigned int device_head;
    volatile unsigned int device_tail;
    char pad[SMPI_CACHE_LINE_SIZE / 2 - 8];
} smpi_device_ipc_attr;
#endif

typedef struct {
    volatile size_t ptr;
    char pad[SMPI_CACHE_LINE_SIZE - sizeof(unsigned int)];
} smpi_shared_tails;

typedef struct {
    volatile size_t first;
    volatile size_t last;
} smpi_rq_limit;

/* the shared area itself */
struct shared_mem {
    volatile int *pid; /* use for initial synchro */

    smpi_shared_tails **shared_tails;

#if defined(_ENABLE_CUDA_) && defined(HAVE_CUDA_IPC)
    smpi_device_ipc_attr **cu_attrbs;
#endif

    smpi_rq_limit *rqueues_limits_s;
    smpi_rq_limit *rqueues_limits_r;

    /* the receives queues */
    char *pool;
#ifdef _SMP_CMA_
    volatile char *volatile *cma_test_buffer;
#endif
};

/* structure for a buffer in the sending buffer pool */
typedef struct send_buf_t {
    int myindex;
    volatile int next;
    volatile int busy;
    int len;
    volatile int has_next;
    int msg_complete;
    char buf[] __attribute__((aligned(SMPI_CACHE_LINE_SIZE)));
} SEND_BUF_T;

/* send queue, to be initialized */
struct shared_buffer_pool {
    int free_head;
    int *send_queue;
    int *tail;
};

#if defined(_SMP_CMA_)
struct cma_header {
    struct iovec remote[1];
    intptr_t total_bytes;
    pid_t pid;
    struct MPIR_Request *csend_req_id;
};
#endif

#if defined(_SMP_LIMIC_)
struct limic_header {
    limic_user lu;
    intptr_t total_bytes;
    struct MPIR_Request *send_req_id;
};
#endif

extern struct smpi_var g_smpi;
extern struct shared_mem *g_smpi_shmem;
extern unsigned long eager_buffer_max_usage;
extern unsigned long rndv_buffer_max_usage;

#endif
