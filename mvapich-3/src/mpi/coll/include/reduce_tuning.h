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

#ifndef _REDUCE_TUNING_
#define _REDUCE_TUNING_

#include "mvp_coll_shmem.h"

#define NMATCH (3 + 1)

/* Reduce tuning flags
 * flat binomial: MVP_INTER_REDUCE_TUNING=1
 * flat knomial:  MVP_INTER_REDUCE_TUNING=2
 *                MVP_REDUCE_INTER_KNOMIAL_FACTOR=?
 * flat reduce-scatter-gather(rsa): MVP_INTER_REDUCE_TUNING=5
 * 2-level: MVP_INTER_REDUCE_TUNING=? MVP_INTRA_REDUCE_TUNING=?
 *          MVP_REDUCE_INTRA_KNOMIAL_FACTOR=4
 *          MVP_REDUCE_INTER_KNOMIAL_FACTOR=4
 *          MVP_INTER_REDUCE_TUNING_TWO_LEVEL=1
 *          where intra-reduce flag takes 1(binomial) 2(knomial) 4(shm) 5(rsa)
 */

extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_reduce_binomial;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_reduce_redscat_gather;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_reduce_shmem;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_reduce_knomial;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_reduce_zcpy;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_reduce_sharp;
extern MPIR_T_pvar_timer_t
    PVAR_TIMER_mvp_coll_timer_reduce_topo_aware_hierarchical;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_reduce_two_level_helper;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_reduce_allreduce;

extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_binomial;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_redscat_gather;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_shmem;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_knomial;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_zcpy;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_subcomm;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_sharp;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_topo_aware_hierarchical;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_two_level_helper;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_allreduce;

extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_binomial_bytes_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_reduce_redscat_gather_bytes_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_reduce_two_level_helper_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_knomial_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_zcpy_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_binomial_bytes_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_reduce_redscat_gather_bytes_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_reduce_two_level_helper_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_knomial_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_zcpy_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_binomial_count_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_reduce_redscat_gather_count_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_reduce_two_level_helper_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_knomial_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_zcpy_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_binomial_count_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_reduce_redscat_gather_count_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_reduce_two_level_helper_count_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_knomial_count_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_zcpy_count_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_count_recv;

typedef struct {
    int min;
    int max;
    MVP_Reduce_fn_t reduce_fn;
} mvp_reduce_tuning_element;

typedef struct {
    int numproc;
    int inter_k_degree;
    int intra_k_degree;
    int is_two_level_reduce[MVP_MAX_NB_THRESHOLDS];
    int size_inter_table;
    mvp_reduce_tuning_element inter_leader[MVP_MAX_NB_THRESHOLDS];
    int size_intra_table;
    mvp_reduce_tuning_element intra_node[MVP_MAX_NB_THRESHOLDS];
} mvp_reduce_tuning_table;

extern int mvp_size_reduce_tuning_table;
extern mvp_reduce_tuning_table *mvp_reduce_thresholds_table;

/*Entries related to indexed tuning table*/
typedef struct {
    int msg_sz;
    MVP_Reduce_fn_t reduce_fn;
} mvp_reduce_indexed_tuning_element;

typedef struct {
    int numproc;
    int inter_k_degree;
    int intra_k_degree;
    int is_two_level_reduce[MVP_MAX_NB_THRESHOLDS];
    int size_inter_table;
    mvp_reduce_indexed_tuning_element inter_leader[MVP_MAX_NB_THRESHOLDS];
    int size_intra_table;
    mvp_reduce_indexed_tuning_element intra_node[MVP_MAX_NB_THRESHOLDS];
} mvp_reduce_indexed_tuning_table;

/* Indicates number of processes per node */
extern int *mvp_reduce_indexed_table_ppn_conf;
/* Indicates total number of configurations */
extern int mvp_reduce_indexed_num_ppn_conf;
extern int *mvp_size_reduce_indexed_tuning_table;
extern mvp_reduce_indexed_tuning_table **mvp_reduce_indexed_thresholds_table;

extern int MPIR_Reduce_allreduce_MVP(const void *sendbuf, void *recvbuf,
                                     int count, MPI_Datatype datatype,
                                     MPI_Op op, int root, MPIR_Comm *comm_ptr,
                                     MPIR_Errflag_t *errflag);

extern int MPIR_Reduce_binomial_MVP(const void *sendbuf, void *recvbuf,
                                    int count, MPI_Datatype datatype, MPI_Op op,
                                    int root, MPIR_Comm *comm_ptr,
                                    MPIR_Errflag_t *errflag);

extern int MPIR_Reduce_intra_knomial_wrapper_MVP(
    const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
    MPI_Op op, int root, MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);

extern int MPIR_Reduce_inter_knomial_wrapper_MVP(
    const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
    MPI_Op op, int root, MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);

extern int MPIR_Reduce_shmem_MVP(const void *sendbuf, void *recvbuf, int count,
                                 MPI_Datatype datatype, MPI_Op op, int root,
                                 MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);

extern int MPIR_Reduce_shmem_MVP_optrels(const void *sendbuf, void *recvbuf,
                                         int count, MPI_Datatype datatype,
                                         MPI_Op op, int root,
                                         MPIR_Comm *comm_ptr,
                                         MPIR_Errflag_t *errflag);

extern int MPIR_Reduce_tree_shmem_MVP_optrels(const void *sendbuf,
                                              void *recvbuf, int count,
                                              MPI_Datatype datatype, MPI_Op op,
                                              int root, MPIR_Comm *comm_ptr,
                                              MPIR_Errflag_t *errflag);

extern int MPIR_Reduce_redscat_gather_MVP(const void *sendbuf, void *recvbuf,
                                          int count, MPI_Datatype datatype,
                                          MPI_Op op, int root,
                                          MPIR_Comm *comm_ptr,
                                          MPIR_Errflag_t *errflag);

extern int MPIR_Reduce_two_level_helper_MVP(const void *sendbuf, void *recvbuf,
                                            int count, MPI_Datatype datatype,
                                            MPI_Op op, int root,
                                            MPIR_Comm *comm_ptr,
                                            MPIR_Errflag_t *errflag);

extern int MPIR_Reduce_Zcpy_MVP(const void *sendbuf, void *recvbuf, int count,
                                MPI_Datatype datatype, MPI_Op op, int root,
                                MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);

extern int (*MPIR_Rank_list_mapper)(MPIR_Comm *, int);

extern int MPIR_Reduce_knomial_trace(int root, int mvp_reduce_knomial_factor,
                                     MPIR_Comm *comm_ptr, int *dst,
                                     int *expected_send_count,
                                     int *expected_recv_count, int **src_array);

extern MVP_Reduce_fn_t MVP_Reduce_function;
extern MVP_Reduce_fn_t MVP_Reduce_intra_function;

extern int MPIR_Reduce_topo_aware_hierarchical_MVP(
    const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
    MPI_Op op, int root, MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);

#if defined(_SHARP_SUPPORT_)
extern int MPIR_Sharp_Reduce_MVP(const void *sendbuf, void *recvbuf, int count,
                                 MPI_Datatype datatype, MPI_Op op, int root,
                                 MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);
#endif

/* Architecture detection tuning */
int MVP_set_reduce_tuning_table(int heterogeneity,
                                struct coll_info *colls_arch_hca);

/* Function to clean free memory allocated by reduce tuning table*/
void MVP_cleanup_reduce_tuning_table();

#endif /* _REDUCE_TUNING_ */
