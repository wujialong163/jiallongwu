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

#ifndef _COLL_SHMEM_INTERNAL
#define _COLL_SHMEM_INTERNAL
#include <pthread.h>

extern int mvp_shmem_coll_num_procs;
extern int mvp_gather_status_alignment;
extern int mvp_bcast_status_alignment;
extern int mvp_max_limic_comms;
extern volatile int *child_complete_bcast;  /* use for initial synchro */
extern volatile int *child_complete_gather; /* use for initial synchro */
extern volatile int *root_complete_gather;
extern volatile int *barrier_gather;
extern volatile int *barrier_bcast;
extern volatile int *shmem_coll_block_status;
#if defined(_SMP_LIMIC_)
extern volatile int *limic_progress;
#endif

#define SHMEM_COLL_NUM_SYNC_ARRAY    4
#define SHMEM_COLL_STATUS_ARRAY_SIZE (sizeof(int) * MVP_SHMEM_COLL_NUM_COMM)
#define SHMEM_COLL_SYNC_ARRAY_SIZE                                             \
    (mvp_gather_status_alignment * sizeof(int) * mvp_shmem_coll_num_procs *    \
     MVP_SHMEM_COLL_NUM_COMM)
#define SHMEM_BCAST_SYNC_ARRAY_SIZE                                            \
    (mvp_bcast_status_alignment * sizeof(int) * mvp_shmem_coll_num_procs *     \
     MVP_SHMEM_COLL_NUM_COMM)
#if defined(_SMP_LIMIC_)
/*since the number of processes would be same, be it shared mem or limic, we use
 * the same variable for no. of processes as in shemem case*/
#define LIMIC_COLL_SYNC_ARRAY_SIZE                                             \
    (mvp_gather_status_alignment * sizeof(int) * mvp_shmem_coll_num_procs *    \
     mvp_max_limic_comms)
#else
#define LIMIC_COLL_SYNC_ARRAY_SIZE 0
#endif

#define SHMEM_BCAST_SYNC_SET(p, r, c)                                          \
    (*(p + (mvp_bcast_status_alignment *                                       \
            ((mvp_shmem_coll_num_procs * r) + (c)))) = 1)
#define SHMEM_BCAST_SYNC_SET_ONE_SHOT(p, r)                                    \
    MPIR_Memset((p + ((mvp_shmem_coll_num_procs * r))), 1,                     \
                sizeof(int) * mvp_shmem_coll_num_procs);

#define SHMEM_BCAST_SYNC_CLR(p, r, c)                                          \
    (*(p + (mvp_bcast_status_alignment *                                       \
            ((mvp_shmem_coll_num_procs * r) + (c)))) = 0)
#define SHMEM_BCAST_SYNC_CLR_ONE_SHOT(p, r)                                    \
    MPIR_Memset((p + ((mvp_shmem_coll_num_procs * r))), 0,                     \
                sizeof(int) * mvp_shmem_coll_num_procs);
#define SHMEM_COLL_SYNC_SET(p, r, c)                                           \
    (*(p + (mvp_gather_status_alignment *                                      \
            ((mvp_shmem_coll_num_procs * r) + (c)))) = 1)
#define SHMEM_COLL_SYNC_CLR(p, r, c)                                           \
    (*(p + (mvp_gather_status_alignment *                                      \
            ((mvp_shmem_coll_num_procs * r) + (c)))) = 0)

#define SHMEM_COLL_BLOCK_STATUS_CLR(p, c)   (*(p + (c)) = 0)
#define SHMEM_COLL_BLOCK_STATUS_SET(p, c)   (*(p + (c)) = 1)
#define SHMEM_COLL_BLOCK_STATUS_INUSE(p, c) (*(p + (c)) == 1)

#define SHMEM_BCAST_SYNC_ISSET(p, r, c)                                        \
    (*(p + (mvp_bcast_status_alignment *                                       \
            ((mvp_shmem_coll_num_procs * r) + (c)))) == 1)
#define SHMEM_BCAST_SYNC_ISSET_ONE_SHOT(p, r, c)                               \
    (*((char *)(p + ((mvp_shmem_coll_num_procs * r) + (c)))) == '1')
#define SHMEM_BCAST_SYNC_ISCLR(p, r, c)                                        \
    (*(p + (mvp_bcast_status_alignment *                                       \
            ((mvp_shmem_coll_num_procs * r) + (c)))) == 0)
#define SHMEM_BCAST_SYNC_ISCLR_ONE_SHOT(p, r, c)                               \
    (*((char *)(p + ((mvp_shmem_coll_num_procs * r) + (c)))) == '0')

#define SHMEM_COLL_SYNC_ISSET(p, r, c)                                         \
    (*(p + (mvp_gather_status_alignment *                                      \
            ((mvp_shmem_coll_num_procs * r) + (c)))) == 1)
#define SHMEM_COLL_SYNC_ISCLR(p, r, c)                                         \
    (*(p + (mvp_gather_status_alignment *                                      \
            ((mvp_shmem_coll_num_procs * r) + (c)))) == 0)

/* the shared area itself */
typedef struct {
    volatile int mvp_shmem_comm_count;
    pthread_spinlock_t shmem_coll_lock;
#if defined(_SMP_LIMIC_)
    volatile int mvp_limic_comm_count;
    volatile int limic_coll_numCores_perSock;
    volatile int limic_coll_num_sockets;
    limic_user limic_hndl[LIMIC_COLL_NUM_COMM];
    volatile int leader_buf_offset[LIMIC_COLL_NUM_COMM];
#endif
#if defined(CKPT)
    volatile int cr_smc_cnt;
    volatile pthread_spinlock_t cr_smc_spinlock;
#endif
    /* Aligning the shmem_coll_buf on a address divisible by 16 */
    long double real16_padding;
    /* the collective buffer */
    char shmem_coll_buf;
} shmem_coll_region;

#define SHMEM_COLL_BUF_SIZE                                                    \
    (MVP_SHMEM_COLL_NUM_COMM * 2 * SHMEM_COLL_BLOCK_SIZE +                     \
     sizeof(shmem_coll_region) +                                               \
     (SHMEM_COLL_NUM_SYNC_ARRAY * SHMEM_COLL_SYNC_ARRAY_SIZE) +                \
     SHMEM_COLL_STATUS_ARRAY_SIZE + LIMIC_COLL_SYNC_ARRAY_SIZE +               \
     SHMEM_BCAST_SYNC_ARRAY_SIZE)

extern shmem_coll_region *shmem_coll;
#endif /* _COLL_SHMEM_INTERNAL */
