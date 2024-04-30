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

#ifndef _MVP_SHMEM_H_
#define _MVP_SHMEM_H_

int MPIDI_MVP_smp_attach_shm_pool_inline();

/* Macros for flow control and rqueues management */
#define MPIDI_MVP_SHMEM_FIRST_SEND(sender, receiver)                           \
    mvp_smp_shmem_region->rqueues_limits_s[receiver].first

#define MPIDI_MVP_SHMEM_LAST_SEND(sender, receiver)                            \
    mvp_smp_shmem_region->rqueues_limits_s[receiver].last

#define MPIDI_MVP_SHMEM_FIRST_RECV(sender, receiver)                           \
    mvp_smp_shmem_region->rqueues_limits_r[sender].first

#define MPIDI_MVP_SHMEM_LAST_RECV(sender, receiver)                            \
    mvp_smp_shmem_region->rqueues_limits_r[sender].last

/* Shared Tail Pointer: updated by receiver after every receive;
 * read by sender when local header meets local tail. */

#define MPIDI_MVP_SHMEM_SHARED_TAIL(sender, receiver)                          \
    mvp_smp_shmem_region->shared_tails[receiver][sender].ptr

#define MPIDI_MVP_SHMEM_BUF_POOL_PTR(destination, index)                       \
    ((mvp_smp_send_buf_t *)((unsigned long)s_buffer_head[destination] +        \
                            (sizeof(mvp_smp_send_buf_t) +                      \
                             MVP_SMP_SEND_BUF_SIZE) *                          \
                                index))

#define MPIDI_MVP_MY_SHMEM_BUF_POOL_PTR(index)                                 \
    ((mvp_smp_send_buf_t *)((unsigned long)s_my_buffer_head +                  \
                            (sizeof(mvp_smp_send_buf_t) +                      \
                             MVP_SMP_SEND_BUF_SIZE) *                          \
                                index))

#endif /* _MVP_SHMEM_H_ */
