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

#ifndef _SCATTER_TUNING_
#define _SCATTER_TUNING_

#include "mvp_coll_shmem.h"

#define NMATCH (3 + 1)

/* Scatter tuning flag
 * Binomial: MVP_INTER_SCATTER_TUNING=1
 * Direct: MVP_INTER_SCATTER_TUNING=2
 * Mcast: MVP_INTER_SCATTER_TUNING=5
 *        MVP_USE_MCAST_SCATTER=1 MVP_USE_MCAST_PIPELINE_SHM=1 MVP_USE_MCAST=1
 * 2-level inter-binomial-intra-binomial:
 *        MVP_INTER_SCATTER_TUNING=3 MVP_INTRA_SCATTER_TUNING=1
 * 2-level inter-binomial-intra-direct:
 *        MVP_INTER_SCATTER_TUNING=3 MVP_INTRA_SCATTER_TUNING=2
 * 2-level inter-direct-intra-binomial:
 *        MVP_INTER_SCATTER_TUNING=4 MVP_INTRA_SCATTER_TUNING=1
 * 2-level inter-direct-intra-direct:
 *        MVP_INTER_SCATTER_TUNING=4 MVP_INTRA_SCATTER_TUNING=2
 */

extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_scatter_mcast;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_scatter_binomial;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_scatter_direct;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_scatter_direct_blk;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_scatter_two_level_binomial;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_scatter_two_level_direct;

extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_mcast;
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_binomial;
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_direct;
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_direct_blk;
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_two_level_binomial;
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_two_level_direct;
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_sharp;

extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_mcast_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_mcast_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_binomial_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_binomial_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_direct_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_direct_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_direct_blk_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_direct_blk_bytes_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_scatter_two_level_binomial_bytes_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_scatter_two_level_binomial_bytes_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_scatter_two_level_direct_bytes_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_scatter_two_level_direct_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_inter_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_inter_bytes_recv;

extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_mcast_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_mcast_count_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_binomial_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_binomial_count_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_direct_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_direct_count_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_direct_blk_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_direct_blk_count_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_scatter_two_level_binomial_count_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_scatter_two_level_binomial_count_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_scatter_two_level_direct_count_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_scatter_two_level_direct_count_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_inter_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_inter_count_recv;

extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_count_recv;

/* Indicates number of processes per node */
extern int *mvp_scatter_table_ppn_conf;
/* Indicates total number of configurations */
extern int mvp_scatter_num_ppn_conf;

typedef int (*MVP_Scatter_fn_t)(const void *sendbuf, int sendcnt,
                                MPI_Datatype sendtype, void *recvbuf,
                                int recvcnt, MPI_Datatype recvtype, int root,
                                MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);

typedef struct {
    int min;
    int max;
    MVP_Scatter_fn_t scatter_fn;
} mvp_scatter_tuning_element;

typedef struct {
    int numproc;
    int size_inter_table;
    mvp_scatter_tuning_element inter_leader[MVP_MAX_NB_THRESHOLDS];
    int size_intra_table;
    mvp_scatter_tuning_element intra_node[MVP_MAX_NB_THRESHOLDS];
} mvp_scatter_tuning_table;

extern int *mvp_size_scatter_tuning_table;
extern mvp_scatter_tuning_table **mvp_scatter_thresholds_table;

/*Entries related to indexed tuning table*/
typedef struct {
    int msg_sz;
    MVP_Scatter_fn_t scatter_fn;
} mvp_scatter_indexed_tuning_element;

typedef struct {
    int numproc;
    int size_inter_table;
    mvp_scatter_indexed_tuning_element inter_leader[MVP_MAX_NB_THRESHOLDS];
    int size_intra_table;
    mvp_scatter_indexed_tuning_element intra_node[MVP_MAX_NB_THRESHOLDS];
} mvp_scatter_indexed_tuning_table;

/* Indicates number of processes per node */
extern int *mvp_scatter_indexed_table_ppn_conf;
/* Indicates total number of configurations */
extern int mvp_scatter_indexed_num_ppn_conf;
extern int *mvp_size_scatter_indexed_tuning_table;
extern mvp_scatter_indexed_tuning_table **mvp_scatter_indexed_thresholds_table;

extern int MPIR_Scatter_mcst_MVP(const void *sendbuf, int sendcnt,
                                 MPI_Datatype sendtype, void *recvbuf,
                                 int recvcnt, MPI_Datatype recvtype, int root,
                                 MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);

extern int MPIR_Scatter_mcst_wrap_MVP(const void *sendbuf, int sendcnt,
                                      MPI_Datatype sendtype, void *recvbuf,
                                      int recvcnt, MPI_Datatype recvtype,
                                      int root, MPIR_Comm *comm_ptr,
                                      MPIR_Errflag_t *errflag);

extern int MPIR_Scatter_MVP_Binomial(const void *sendbuf, int sendcnt,
                                     MPI_Datatype sendtype, void *recvbuf,
                                     int recvcnt, MPI_Datatype recvtype,
                                     int root, MPIR_Comm *comm_ptr,
                                     MPIR_Errflag_t *errflag);

extern int MPIR_Scatter_MVP_Direct(const void *sendbuf, int sendcnt,
                                   MPI_Datatype sendtype, void *recvbuf,
                                   int recvcnt, MPI_Datatype recvtype, int root,
                                   MPIR_Comm *comm_ptr,
                                   MPIR_Errflag_t *errflag);

extern int MPIR_Scatter_MVP_Direct_Blk(const void *sendbuf, int sendcnt,
                                       MPI_Datatype sendtype, void *recvbuf,
                                       int recvcnt, MPI_Datatype recvtype,
                                       int root, MPIR_Comm *comm_ptr,
                                       MPIR_Errflag_t *errflag);

extern int MPIR_Scatter_MVP_two_level_Binomial(const void *sendbuf, int sendcnt,
                                               MPI_Datatype sendtype,
                                               void *recvbuf, int recvcnt,
                                               MPI_Datatype recvtype, int root,
                                               MPIR_Comm *comm_ptr,
                                               MPIR_Errflag_t *errflag);

extern int MPIR_Scatter_MVP_two_level_Direct(const void *sendbuf, int sendcnt,
                                             MPI_Datatype sendtype,
                                             void *recvbuf, int recvcnt,
                                             MPI_Datatype recvtype, int root,
                                             MPIR_Comm *comm_ptr,
                                             MPIR_Errflag_t *errflag);

extern MVP_Scatter_fn_t MVP_Scatter_intra_function;

#if defined(_SHARP_SUPPORT_)
int MPIR_Sharp_Scatter_MVP(const void *sendbuf, int sendcount,
                           MPI_Datatype sendtype, void *recvbuf, int recvcount,
                           MPI_Datatype recvtype, int root, MPIR_Comm *comm_ptr,
                           MPIR_Errflag_t *errflag);
#endif

/* Architecture detection tuning */
int MVP_set_scatter_tuning_table(int heterogeneity,
                                 struct coll_info *colls_arch_hca);

/* Function to clean free memory allocated by scatter tuning table*/
void MVP_cleanup_scatter_tuning_table();

#endif
