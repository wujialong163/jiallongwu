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

#ifndef _ALLTOALL_TUNING_
#define _ALLTOALL_TUNING_

#include "mvp_coll_shmem.h"

#define NMATCH (3 + 1)

/* Indicates number of processes per node */
extern int *mvp_alltoall_table_ppn_conf;
/* Indicates total number of configurations */
extern int mvp_alltoall_num_ppn_conf;

extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_alltoall_inplace;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_alltoall_bruck;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_alltoall_rd;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_alltoall_sd;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_alltoall_pw;

extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_inplace;
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_bruck;
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_rd;
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_sd;
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_pw;

extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_inplace_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_bruck_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_sd_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_pw_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_intra_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_inplace_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_bruck_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_sd_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_pw_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_intra_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_inplace_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_bruck_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_sd_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_pw_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_intra_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_inplace_count_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_bruck_count_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_sd_count_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_pw_count_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_intra_count_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_count_recv;

typedef int (*MVP_Alltoall_fn_t)(const void *sendbuf, int sendcount,
                                 MPI_Datatype sendtype, void *recvbuf,
                                 int recvcount, MPI_Datatype recvtype,
                                 MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);

typedef struct {
    int min;
    int max;
    MVP_Alltoall_fn_t alltoall_fn;
} mvp_alltoall_tuning_element;

typedef struct {
    int numproc;
    int size_table;
    mvp_alltoall_tuning_element algo_table[MVP_MAX_NB_THRESHOLDS];
    mvp_alltoall_tuning_element in_place_algo_table[MVP_MAX_NB_THRESHOLDS];
} mvp_alltoall_tuning_table;

extern int *mvp_size_alltoall_tuning_table;
extern mvp_alltoall_tuning_table **mvp_alltoall_thresholds_table;

typedef struct {
    int msg_sz;
    MVP_Alltoall_fn_t alltoall_fn;
} mvp_alltoall_indexed_tuning_element;

typedef struct {
    int numproc;
    int in_place_algo_table[MVP_MAX_NB_THRESHOLDS];
    int size_table;
    mvp_alltoall_indexed_tuning_element algo_table[MVP_MAX_NB_THRESHOLDS];
} mvp_alltoall_indexed_tuning_table;

/* Indicates number of processes per node */
extern int *mvp_alltoall_indexed_table_ppn_conf;
/* Indicates total number of configurations */
extern int mvp_alltoall_indexed_num_ppn_conf;
extern int *mvp_size_alltoall_indexed_tuning_table;
extern mvp_alltoall_indexed_tuning_table *
    *mvp_alltoall_indexed_thresholds_table;

extern int MPIR_Alltoall_bruck_MVP(const void *sendbuf, int sendcount,
                                   MPI_Datatype sendtype, void *recvbuf,
                                   int recvcount, MPI_Datatype recvtype,
                                   MPIR_Comm *comm_ptr,
                                   MPIR_Errflag_t *errflag);

extern int MPIR_Alltoall_ALG_MVP(const void *sendbuf, int sendcount,
                                 MPI_Datatype sendtype, void *recvbuf,
                                 int recvcount, MPI_Datatype recvtype,
                                 MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);

extern int MPIR_Alltoall_RD_MVP(const void *sendbuf, int sendcount,
                                MPI_Datatype sendtype, void *recvbuf,
                                int recvcount, MPI_Datatype recvtype,
                                MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);

extern int MPIR_Alltoall_Scatter_dest_MVP(const void *sendbuf, int sendcount,
                                          MPI_Datatype sendtype, void *recvbuf,
                                          int recvcount, MPI_Datatype recvtype,
                                          MPIR_Comm *comm_ptr,
                                          MPIR_Errflag_t *errflag);

extern int MPIR_Alltoall_pairwise_MVP(const void *sendbuf, int sendcount,
                                      MPI_Datatype sendtype, void *recvbuf,
                                      int recvcount, MPI_Datatype recvtype,
                                      MPIR_Comm *comm_ptr,
                                      MPIR_Errflag_t *errflag);

extern int MPIR_Alltoall_inplace_MVP(const void *sendbuf, int sendcount,
                                     MPI_Datatype sendtype, void *recvbuf,
                                     int recvcount, MPI_Datatype recvtype,
                                     MPIR_Comm *comm_ptr,
                                     MPIR_Errflag_t *errflag);

/* Architecture detection tuning */
int MVP_set_alltoall_tuning_table(int heterogeneity,
                                  struct coll_info *colls_arch_hca);

/* Function to clean free memory allocated by bcast tuning table*/
void MVP_cleanup_alltoall_tuning_table();

#endif
