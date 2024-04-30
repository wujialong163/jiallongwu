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

#ifndef _BARRIER_TUNING_
#define _BARRIER_TUNING_

#include "mvp_coll_shmem.h"

#define NMATCH (3 + 1)

/* Barrier tuning flags: no flags
 */

extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_barrier_pairwise;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_barrier_shmem;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_barrier_subcomm;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_barrier_sharp;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_barrier_topo_aware_shmem;

extern unsigned long long PVAR_COUNTER_mvp_coll_barrier_pairwise;
extern unsigned long long PVAR_COUNTER_mvp_coll_barrier_shmem;
extern unsigned long long PVAR_COUNTER_mvp_coll_barrier_subcomm;
extern unsigned long long PVAR_COUNTER_mvp_coll_barrier_sharp;
extern unsigned long long PVAR_COUNTER_mvp_coll_barrier_topo_aware_shmem;

extern unsigned long long PVAR_COUNTER_mvp_coll_barrier_pairwise_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_barrier_pairwise_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_barrier_pairwise_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_barrier_pairwise_count_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_barrier_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_barrier_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_barrier_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_barrier_count_recv;

extern int MPIR_topo_aware_shmem_barrier_MVP(MPIR_Comm *comm_ptr,
                                             MPIR_Errflag_t *errflag);

extern int MPIR_socket_aware_shmem_barrier_old_MVP(MPIR_Comm *comm_ptr,
                                                   MPIR_Errflag_t *errflag);

extern int MPIR_socket_aware_shmem_barrier_MVP(MPIR_Comm *comm_ptr,
                                               MPIR_Errflag_t *errflag);

extern int MPIR_shmem_barrier_MVP(MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);

extern int MPIR_Sharp_Barrier_MVP(MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);

extern int MPIR_Pairwise_Barrier_MVP(MPIR_Comm *comm_ptr,
                                     MPIR_Errflag_t *errflag);

#if defined(_SHARP_SUPPORT_)
extern int MPIR_Sharp_Barrier_MVP(MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);
#endif

#endif
