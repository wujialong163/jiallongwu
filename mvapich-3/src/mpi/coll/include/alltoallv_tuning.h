/* Copyright (c) 2001-2024, The Ohio State University. All rights
 ** reserved.
 **
 ** This file is part of the MVAPICH software package developed by the
 ** team members of The Ohio State University's Network-Based Computing
 ** Laboratory (NBCL), headed by Professor Dhabaleswar K. (DK) Panda.
 **
 ** For detailed copyright and licensing information, please refer to the
 ** copyright file COPYRIGHT in the top level MVAPICH directory.
 **
 **/

#ifndef _ALLTOALLV_TUNING_
#define _ALLTOALLV_TUNING_

#include "mvp_coll_shmem.h"

#define NMATCH (3 + 1)

/* Alltoallv tuning flags
  0: Alltoallv_intra_MVP
  1: Alltoallv_MVP
*/

/* Indicates number of processes per node */
extern int *mvp_alltoallv_table_ppn_conf;
/* Indicates total number of configurations */
extern int mvp_alltoallv_num_ppn_conf;

extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_alltoallv_intra;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_alltoallv_intra_scatter;

extern unsigned long long PVAR_COUNTER_mvp_coll_alltoallv_intra;
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoallv_intra_scatter;

extern unsigned long long PVAR_COUNTER_mvp_coll_alltoallv_pw;
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoallv_intra_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoallv_intra_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoallv_intra_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoallv_intra_count_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoallv_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoallv_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoallv_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoallv_count_recv;

typedef int (*MVP_Alltoallv_fn_t)(const void *sendbuf, const int *sendcnts,
                                  const int *sdispls, MPI_Datatype sendtype,
                                  void *recvbuf, const int *recvcnts,
                                  const int *rdispls, MPI_Datatype recvtype,
                                  MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);

typedef struct {
    int min;
    int max;
    MVP_Alltoallv_fn_t alltoallv_fn;
} mvp_alltoallv_tuning_element;

typedef struct {
    int numproc;
    int size_table;
    mvp_alltoallv_tuning_element algo_table[MVP_MAX_NB_THRESHOLDS];
    mvp_alltoallv_tuning_element in_place_algo_table[MVP_MAX_NB_THRESHOLDS];
} mvp_alltoallv_tuning_table;

extern int *mvp_size_alltoallv_tuning_table;
extern mvp_alltoallv_tuning_table **mvp_alltoallv_thresholds_table;
extern int mvp_use_old_alltoallv;

typedef struct {
    int msg_sz;
    MVP_Alltoallv_fn_t alltoallv_fn;
} mvp_alltoallv_indexed_tuning_element;

typedef struct {
    int numproc;
    int in_place_algo_table[MVP_MAX_NB_THRESHOLDS];
    int size_table;
    mvp_alltoallv_indexed_tuning_element algo_table[MVP_MAX_NB_THRESHOLDS];
} mvp_alltoallv_indexed_tuning_table;

/* Indicates number of processes per node */
extern int *mvp_alltoallv_indexed_table_ppn_conf;
/* Indicates total number of configurations */
extern int mvp_alltoallv_indexed_num_ppn_conf;
extern int *mvp_size_alltoallv_indexed_tuning_table;
extern mvp_alltoallv_indexed_tuning_table *
    *mvp_alltoallv_indexed_thresholds_table;

extern int MPIR_Alltoallv_index_tuned_intra_MVP(
    const void *sendbuf, const int *sendcnts, const int *sdispls,
    MPI_Datatype sendtype, void *recvbuf, const int *recvcnts,
    const int *rdispls, MPI_Datatype recvtype, MPIR_Comm *comm_ptr,
    MPIR_Errflag_t *errflag);

extern int MPIR_Alltoallv_intra_MVP(const void *sendbuf, const int *sendcnts,
                                    const int *sdispls, MPI_Datatype sendtype,
                                    void *recvbuf, const int *recvcnts,
                                    const int *rdispls, MPI_Datatype recvtype,
                                    MPIR_Comm *comm_ptr,
                                    MPIR_Errflag_t *errflag);

extern int MPIR_Alltoallv_intra_scatter_MVP(
    const void *sendbuf, const int *sendcnts, const int *sdispls,
    MPI_Datatype sendtype, void *recvbuf, const int *recvcnts,
    const int *rdispls, MPI_Datatype recvtype, MPIR_Comm *comm_ptr,
    MPIR_Errflag_t *errflag);

/* Architecture detection tuning */
int MVP_set_alltoallv_tuning_table(int heterogeneity,
                                   struct coll_info *colls_arch_hca);

/* Function to clean free memory allocated by bcast tuning table*/
void MVP_cleanup_alltoallv_tuning_table();

#endif
