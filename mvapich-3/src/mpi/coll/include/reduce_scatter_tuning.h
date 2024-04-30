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

#ifndef _RED_SCAT_TUNING_
#define _RED_SCAT_TUNING_

#include "mvp_coll_shmem.h"

#define NMATCH (3 + 1)

/* Red_scat tuning flags
 * RED_SCAT_BASIC:    MVP_INTER_RED_SCAT_TUNING=1
 * RED_SCAT_REC_HALF: MVP_INTER_RED_SCAT_TUNING=2
 * RED_SCAT_PAIRWISE: MVP_INTER_RED_SCAT_TUNING=3
 *
 * Regular expression example:
 *   MVP_INTER_RED_SCAT_TUNING=2:0-1024,1:1024-8192,3:8192-+
 *   meaning: use RED_SCAT_REC_HALF for 0 byte to 1024 bytes
 *            use RED_SCAT_BASIC for 1024 byte to 8192 bytes
 *            use RED_SCAT_PAIRWISE since 8192 bytes
 *
 */

extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_reduce_scatter_noncomm;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_reduce_scatter_basic;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_reduce_scatter_rec_halving;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_reduce_scatter_pairwise;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_reduce_scatter_non_comm;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_reduce_scatter_ring;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_reduce_scatter_ring_2lvl;

extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_ring;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_ring_2lvl;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_noncomm;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_basic;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_rec_halving;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_pairwise;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_non_comm;

extern unsigned long long
    PVAR_COUNTER_mvp_coll_reduce_scatter_noncomm_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_basic_bytes_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_reduce_scatter_rec_halving_bytes_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_reduce_scatter_pairwise_bytes_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_reduce_scatter_non_comm_bytes_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_reduce_scatter_noncomm_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_basic_bytes_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_reduce_scatter_rec_halving_bytes_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_reduce_scatter_pairwise_bytes_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_reduce_scatter_non_comm_bytes_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_reduce_scatter_noncomm_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_basic_count_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_reduce_scatter_rec_halving_count_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_reduce_scatter_pairwise_count_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_reduce_scatter_non_comm_count_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_reduce_scatter_noncomm_count_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_basic_count_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_reduce_scatter_rec_halving_count_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_reduce_scatter_pairwise_count_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_reduce_scatter_non_comm_count_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_count_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_ring_count_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_ring_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_ring_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_ring_bytes_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_reduce_scatter_ring_2lvl_count_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_reduce_scatter_ring_2lvl_count_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_reduce_scatter_ring_2lvl_bytes_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_reduce_scatter_ring_2lvl_bytes_send;

typedef struct {
    int min;
    int max;
    int (*MVP_pt_Red_scat_function)(const void *sendbuf, void *recvbuf,
                                    const int *recvcnts, MPI_Datatype datatype,
                                    MPI_Op op, MPIR_Comm *comm_ptr,
                                    MPIR_Errflag_t *errflag);
} mvp_red_scat_tuning_element;

typedef struct {
    int numproc;
    int size_inter_table;
    mvp_red_scat_tuning_element inter_leader[MVP_MAX_NB_THRESHOLDS];
} mvp_red_scat_tuning_table;

extern int mvp_size_red_scat_tuning_table;
extern mvp_red_scat_tuning_table *mvp_red_scat_thresholds_table;

extern int MPIR_Reduce_Scatter_Basic_MVP(const void *sendbuf, void *recvbuf,
                                         const int *recvcnts,
                                         MPI_Datatype datatype, MPI_Op op,
                                         MPIR_Comm *comm_ptr,
                                         MPIR_Errflag_t *errflag);

extern int MPIR_Reduce_scatter_Rec_Halving_MVP(const void *sendbuf,
                                               void *recvbuf,
                                               const int *recvcnts,
                                               MPI_Datatype datatype, MPI_Op op,
                                               MPIR_Comm *comm_ptr,
                                               MPIR_Errflag_t *errflag);

extern int MPIR_Reduce_scatter_Pair_Wise_MVP(const void *sendbuf, void *recvbuf,
                                             const int *recvcnts,
                                             MPI_Datatype datatype, MPI_Op op,
                                             MPIR_Comm *comm_ptr,
                                             MPIR_Errflag_t *errflag);

extern int MPIR_Reduce_scatter_ring(const void *sendbuf, void *recvbuf,
                                    const int *recvcnts, MPI_Datatype datatype,
                                    MPI_Op op, MPIR_Comm *comm_ptr,
                                    MPIR_Errflag_t *errflag);

extern int MPIR_Reduce_scatter_ring_2lvl(const void *sendbuf, void *recvbuf,
                                         const int *recvcnts,
                                         MPI_Datatype datatype, MPI_Op op,
                                         MPIR_Comm *comm_ptr,
                                         MPIR_Errflag_t *errflag);

extern int MPIR_Reduce_scatter_noncomm_MVP(const void *sendbuf, void *recvbuf,
                                           const int *recvcnts,
                                           MPI_Datatype datatype, MPI_Op op,
                                           MPIR_Comm *comm_ptr,
                                           MPIR_Errflag_t *errflag);

/* Architecture detection tuning */
int MVP_set_red_scat_tuning_table(int heterogeneity,
                                  struct coll_info *colls_arch_hca);

/* Function to clean free memory allocated by red_scat tuning table*/
void MVP_cleanup_red_scat_tuning_table();

/* Function used inside ch3_shmem_coll.c to tune red_scat thresholds */
int MVP_internode_Red_scat_is_define(char *mvp_user_red_scat_inter);
#endif
