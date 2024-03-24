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

#ifndef _BCAST_TUNING_
#define _BCAST_TUNING_

#include "mvp_coll_shmem.h"

#define NMATCH (3 + 1)

#define INTRA_NODE_ROOT 0

typedef int (*MVP_Bcast_fn_t)(void *buf, int count, MPI_Datatype datatype,
                              int root, MPIR_Comm *comm_ptr,
                              MPIR_Errflag_t *errflag);

typedef struct {
    int min;
    int max;
    MVP_Bcast_fn_t bcast_fn;
    int zcpy_pipelined_knomial_factor;
} mvp_bcast_tuning_element;

typedef struct {
    int numproc;
    int bcast_segment_size;
    int intra_node_knomial_factor;
    int inter_node_knomial_factor;
    int is_two_level_bcast[MVP_MAX_NB_THRESHOLDS];
    int size_inter_table;
    mvp_bcast_tuning_element inter_leader[MVP_MAX_NB_THRESHOLDS];
    int size_intra_table;
    mvp_bcast_tuning_element intra_node[MVP_MAX_NB_THRESHOLDS];
} mvp_bcast_tuning_table;

extern int mvp_pipelined_knomial_factor;

extern int mvp_size_bcast_tuning_table;
extern mvp_bcast_tuning_table *mvp_bcast_thresholds_table;

extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_bcast_binomial;
extern MPIR_T_pvar_timer_t
    PVAR_TIMER_mvp_coll_timer_bcast_scatter_doubling_allgather;
extern MPIR_T_pvar_timer_t
    PVAR_TIMER_mvp_coll_timer_bcast_scatter_ring_allgather;
extern MPIR_T_pvar_timer_t
    PVAR_TIMER_mvp_coll_timer_bcast_scatter_ring_allgather_shm;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_bcast_shmem;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_bcast_knomial_internode;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_bcast_knomial_intranode;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_bcast_mcast_internode;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_bcast_pipelined;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_bcast_shmem_zcpy;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_bcast_shmem_sharp;
extern MPIR_T_pvar_timer_t
    PVAR_TIMER_mvp_coll_timer_bcast_shmem_topo_aware_hierarchical;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_bcast_pipelined_zcpy;

extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_binomial;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_scatter_doubling_allgather;
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_scatter_ring_allgather;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_scatter_ring_allgather_shm;
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_shmem;
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_knomial_internode;
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_knomial_intranode;
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_mcast_internode;
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_pipelined;
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_pipelined_zcpy;
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_shmem_zcpy;
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_sharp;
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_topo_aware_hierarchical;
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_subcomm;

extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_binomial_bytes_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_scatter_for_bcast_bytes_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_scatter_doubling_allgather_bytes_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_scatter_ring_allgather_bytes_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_scatter_ring_allgather_shm_bytes_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_knomial_internode_bytes_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_knomial_intranode_bytes_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_mcast_internode_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_pipelined_zcpy_bytes_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_tune_inter_node_helper_bytes_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_inter_node_helper_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_binomial_bytes_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_scatter_for_bcast_bytes_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_scatter_doubling_allgather_bytes_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_scatter_ring_allgather_bytes_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_scatter_ring_allgather_shm_bytes_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_knomial_internode_bytes_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_knomial_intranode_bytes_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_mcast_internode_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_pipelined_zcpy_bytes_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_tune_inter_node_helper_bytes_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_inter_node_helper_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_binomial_count_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_scatter_for_bcast_count_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_scatter_doubling_allgather_count_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_scatter_ring_allgather_count_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_scatter_ring_allgather_shm_count_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_knomial_internode_count_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_knomial_intranode_count_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_mcast_internode_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_pipelined_zcpy_count_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_tune_inter_node_helper_count_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_inter_node_helper_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_binomial_count_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_scatter_for_bcast_count_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_scatter_doubling_allgather_count_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_scatter_ring_allgather_count_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_scatter_ring_allgather_shm_count_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_knomial_internode_count_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_knomial_intranode_count_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_mcast_internode_count_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_pipelined_zcpy_count_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_tune_inter_node_helper_count_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_bcast_inter_node_helper_count_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_count_recv;

/*Entries related to indexed tuning table*/
typedef struct {
    int msg_sz;
    MVP_Bcast_fn_t bcast_fn;
    int zcpy_pipelined_knomial_factor;
} mvp_bcast_indexed_tuning_element;

typedef struct {
    int numproc;
    int bcast_segment_size;
    int intra_node_knomial_factor;
    int inter_node_knomial_factor;
    int is_two_level_bcast[MVP_MAX_NB_THRESHOLDS];
    int size_inter_table;
    mvp_bcast_indexed_tuning_element inter_leader[MVP_MAX_NB_THRESHOLDS];
    int size_intra_table;
    mvp_bcast_indexed_tuning_element intra_node[MVP_MAX_NB_THRESHOLDS];
} mvp_bcast_indexed_tuning_table;

/* Indicates number of processes per node */
extern int *mvp_bcast_indexed_table_ppn_conf;
/* Indicates total number of configurations */
extern int mvp_bcast_indexed_num_ppn_conf;
extern int *mvp_size_bcast_indexed_tuning_table;
extern mvp_bcast_indexed_tuning_table **mvp_bcast_indexed_thresholds_table;

extern int MPIR_Bcast_binomial_MVP(void *buffer, int count,
                                   MPI_Datatype datatype, int root,
                                   MPIR_Comm *comm_ptr,
                                   MPIR_Errflag_t *errflag);

extern int MPIR_Bcast_scatter_ring_allgather_MVP(void *buffer, int count,
                                                 MPI_Datatype datatype,
                                                 int root, MPIR_Comm *comm_ptr,
                                                 MPIR_Errflag_t *errflag);

extern int MPIR_Bcast_scatter_ring_allgather_shm_MVP(void *buffer, int count,
                                                     MPI_Datatype datatype,
                                                     int root,
                                                     MPIR_Comm *comm_ptr,
                                                     MPIR_Errflag_t *errflag);

extern int MPIR_Knomial_Bcast_inter_node_MVP(void *buffer, int count,
                                             MPI_Datatype datatype, int root,
                                             int knomial_factor,
                                             MPIR_Comm *comm_ptr,
                                             MPIR_Errflag_t *errflag);

extern int MPIR_Knomial_Bcast_inter_node_wrapper_MVP(void *buffer, int count,
                                                     MPI_Datatype datatype,
                                                     int root,
                                                     MPIR_Comm *comm_ptr,
                                                     MPIR_Errflag_t *errflag);

extern int MPIR_Pipelined_Bcast_MVP(void *buffer, int count,
                                    MPI_Datatype datatype, int root,
                                    MPIR_Comm *comm_ptr,
                                    MPIR_Errflag_t *errflag);

/* Use for intra-node in case of two lvl algo */
extern int MPIR_Shmem_Bcast_MVP(void *buffer, int count, MPI_Datatype datatype,
                                int root, MPIR_Comm *shmem_comm_ptr,
                                MPIR_Errflag_t *errflag);

extern int MPIR_Knomial_Bcast_intra_node_MVP(void *buffer, int count,
                                             MPI_Datatype datatype, int root,
                                             MPIR_Comm *comm_ptr,
                                             MPIR_Errflag_t *errflag);

extern int MPIR_Knomial_Bcast_inter_node_trace_MVP(
    int root, int mvp_bcast_knomial_factor, int *src, int *expected_send_count,
    int *expected_recv_count, int **dst_array, MPIR_Comm *comm_ptr);

extern int MPIR_Pipelined_Bcast_Zcpy_MVP(void *buffer, int count,
                                         MPI_Datatype datatype, int root,
                                         MPIR_Comm *comm_ptr,
                                         MPIR_Errflag_t *errflag);

extern int MPIR_Shmem_Bcast_Zcpy_MVP(void *buffer, int count,
                                     MPI_Datatype datatype, int root, int src,
                                     int expected_recv_count, int *dst_array,
                                     int expected_send_count,
                                     int knomial_factor, MPIR_Comm *comm_ptr,
                                     MPIR_Errflag_t *errflag);

#if defined(_MCST_SUPPORT_)
extern int MPIR_Mcast_inter_node_MVP(void *buffer, int count,
                                     MPI_Datatype datatype, int root,
                                     MPIR_Comm *comm_ptr,
                                     MPIR_Errflag_t *errflag);
#endif /* #if defined(_MCST_SUPPORT_) */

extern int MPIR_Bcast_topo_aware_hierarchical_MVP(void *buffer, int count,
                                                  MPI_Datatype datatype,
                                                  int root, MPIR_Comm *comm_ptr,
                                                  MPIR_Errflag_t *errflag);

extern MVP_Bcast_fn_t MVP_Bcast_function;
extern MVP_Bcast_fn_t MVP_Bcast_intra_node_function;

#if defined(_SHARP_SUPPORT_)
extern int MPIR_Sharp_Bcast_MVP(void *buffer, int count, MPI_Datatype datatype,
                                int root, MPIR_Comm *comm_ptr,
                                MPIR_Errflag_t *errflag);
#endif

/* Architecture detection tuning */
int MVP_set_bcast_tuning_table(int heterogeneity,
                               struct coll_info *colls_arch_hca);

/* Function to clean free memory allocated by bcast tuning table*/
void MVP_cleanup_bcast_tuning_table();

#endif
