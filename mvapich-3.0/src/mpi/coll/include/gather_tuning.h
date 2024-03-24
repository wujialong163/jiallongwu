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

#ifndef _GATHER_TUNING_
#define _GATHER_TUNING_

#include "mvp_coll_shmem.h"
#if defined(CHANNEL_MRAIL)
#include "ibv_param.h"
#endif /* #if defined(CHANNEL_MRAIL) */
#define MVP_DEFAULT_SHMEM_BCAST_LEADERS      4096
#define MPIR_GATHER_BINOMIAL_MEDIUM_MSG      16384

#define NMATCH (3 + 1)

typedef int (*MVP_Gather_fn_t)(const void *sendbuf, int sendcnt,
                               MPI_Datatype sendtype, void *recvbuf,
                               int recvcnt, MPI_Datatype recvtype, int root,
                               MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);

typedef struct {
    int min;
    int max;
    MVP_Gather_fn_t gather_fn;
} mvp_gather_tuning_element;

typedef struct {
    int numproc;
    int size_inter_table;
    mvp_gather_tuning_element inter_leader[MVP_MAX_NB_THRESHOLDS];
    int size_intra_table;
    mvp_gather_tuning_element intra_node[MVP_MAX_NB_THRESHOLDS];
} mvp_gather_tuning_table;

extern MVP_Gather_fn_t MVP_Gather_inter_leader_function;
extern MVP_Gather_fn_t MVP_Gather_intra_node_function;

#define TEMP_BUF_HAS_DATA    (1)
#define TEMP_BUF_HAS_NO_DATA (0)

extern int mvp_size_gather_tuning_table;
extern mvp_gather_tuning_table *mvp_gather_thresholds_table;

extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_gather_pt2pt;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_gather_direct;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_gather_direct_blk;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_gather_two_level_direct;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_gather_limic_scheme_pt_pt;
extern MPIR_T_pvar_timer_t
    PVAR_TIMER_mvp_coll_timer_gather_limic_scheme_pt_linear;
extern MPIR_T_pvar_timer_t
    PVAR_TIMER_mvp_coll_timer_gather_limic_scheme_linear_pt;
extern MPIR_T_pvar_timer_t
    PVAR_TIMER_mvp_coll_timer_gather_limic_scheme_linear_linear;
extern MPIR_T_pvar_timer_t
    PVAR_TIMER_mvp_coll_timer_gather_limic_scheme_single_leader;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_gather_intra_node_limic;

extern unsigned long long PVAR_COUNTER_mvp_coll_gather_pt2pt;
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_direct;
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_direct_blk;
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_two_level_direct;
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_limic_scheme_pt_pt;
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_limic_scheme_pt_linear;
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_limic_scheme_linear_pt;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_gather_limic_scheme_linear_linear;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_gather_limic_scheme_single_leader;
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_intra_node_limic;

extern unsigned long long PVAR_COUNTER_mvp_coll_gather_direct_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_direct_blk_bytes_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_gather_two_level_direct_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_direct_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_direct_blk_bytes_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_gather_two_level_direct_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_direct_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_direct_blk_count_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_gather_two_level_direct_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_direct_count_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_direct_blk_count_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_gather_two_level_direct_count_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_count_recv;

/*Entries related to indexed tuning table*/

typedef struct {
    int msg_sz;
    MVP_Gather_fn_t gather_fn;
} mvp_gather_indexed_tuning_element;

typedef struct {
    int numproc;
    int size_inter_table;
    mvp_gather_indexed_tuning_element inter_leader[MVP_MAX_NB_THRESHOLDS];
    int size_intra_table;
    mvp_gather_indexed_tuning_element intra_node[MVP_MAX_NB_THRESHOLDS];
} mvp_gather_indexed_tuning_table;

/* Indicates number of processes per node */
extern int *mvp_gather_indexed_table_ppn_conf;
/* Indicates total number of configurations */
extern int mvp_gather_indexed_num_ppn_conf;
extern int *mvp_size_gather_indexed_tuning_table;
extern mvp_gather_indexed_tuning_table **mvp_gather_indexed_thresholds_table;

extern int MPIR_Gather_MVP_Direct_Blk(const void *sendbuf, int sendcnt,
                                      MPI_Datatype sendtype, void *recvbuf,
                                      int recvcnt, MPI_Datatype recvtype,
                                      int root, MPIR_Comm *comm_ptr,
                                      MPIR_Errflag_t *errflag);

extern int MPIR_Gather_MVP_Direct(const void *sendbuf, int sendcnt,
                                  MPI_Datatype sendtype, void *recvbuf,
                                  int recvcnt, MPI_Datatype recvtype, int root,
                                  MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);
extern int MPIR_Gather_MVP_two_level_Direct(const void *sendbuf, int sendcnt,
                                            MPI_Datatype sendtype,
                                            void *recvbuf, int recvcnt,
                                            MPI_Datatype recvtype, int root,
                                            MPIR_Comm *comm_ptr,
                                            MPIR_Errflag_t *errflag);

#if defined(_SMP_LIMIC_)
extern int MPIR_Intra_node_LIMIC_Gather_MVP(const void *sendbuf, int sendcnt,
                                            MPI_Datatype sendtype,
                                            void *recvbuf, int recvcnt,
                                            MPI_Datatype recvtype, int root,
                                            MPIR_Comm *comm_ptr,
                                            MPIR_Errflag_t *errflag);
#endif /*#if defined(_SMP_LIMIC_)*/

extern int MPIR_pt_pt_intra_gather(const void *sendbuf, int sendcnt,
                                   MPI_Datatype sendtype, void *recvbuf,
                                   int recvcnt, MPI_Datatype recvtype, int root,
                                   int rank, void *tmp_buf, int nbytes,
                                   int is_data_avail, MPIR_Comm *comm_ptr,
                                   MVP_Gather_fn_t intra_node_fn_ptr,
                                   MPIR_Errflag_t *errflag);

/* Architecture detection tuning */
int MVP_set_gather_tuning_table(int heterogeneity,
                                struct coll_info *colls_arch_hca);
void MVP_cleanup_gather_tuning_table();

#endif