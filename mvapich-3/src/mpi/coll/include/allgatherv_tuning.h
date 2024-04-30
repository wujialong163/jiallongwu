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

#ifndef _ALLGATHERV_TUNING_
#define _ALLGATHERV_TUNING_

#include "mvp_coll_shmem.h"

#define NMATCH (3 + 1)

extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allgatherv_rec_doubling;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allgatherv_bruck;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allgatherv_ring;
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allgatherv_ring_cyclic;

extern unsigned long long PVAR_COUNTER_mvp_coll_allgatherv_rec_doubling;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgatherv_bruck;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgatherv_ring;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgatherv_ring_cyclic;

extern unsigned long long
    PVAR_COUNTER_mvp_coll_allgatherv_rec_doubling_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgatherv_bruck_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgatherv_ring_bytes_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_allgatherv_rec_doubling_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgatherv_bruck_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgatherv_ring_bytes_recv;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_allgatherv_rec_doubling_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgatherv_bruck_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgatherv_ring_count_send;
extern unsigned long long
    PVAR_COUNTER_mvp_coll_allgatherv_rec_doubling_count_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgatherv_bruck_count_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgatherv_ring_count_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgatherv_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgatherv_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgatherv_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_allgatherv_count_recv;

typedef int (*MVP_Allgatherv_fn_t)(const void *sendbuf, int sendcount,
                                   MPI_Datatype sendtype, void *recvbuf,
                                   const int *recvcounts, const int *displs,
                                   MPI_Datatype recvtype, MPIR_Comm *comm_ptr,
                                   MPIR_Errflag_t *errflag);

typedef struct {
    int min;
    int max;
    MVP_Allgatherv_fn_t allgatherv_fn;
} mvp_allgatherv_tuning_element;

typedef struct {
    int numproc;
    int size_inter_table;
    mvp_allgatherv_tuning_element inter_leader[MVP_MAX_NB_THRESHOLDS];
} mvp_allgatherv_tuning_table;

extern int mvp_size_allgatherv_tuning_table;
extern mvp_allgatherv_tuning_table *mvp_allgatherv_thresholds_table;

extern int MPIR_Allgatherv_Rec_Doubling_MVP(
    const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf,
    const int *recvcounts, const int *displs, MPI_Datatype recvtype,
    MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);

extern int MPIR_Allgatherv_Bruck_MVP(const void *sendbuf, int sendcount,
                                     MPI_Datatype sendtype, void *recvbuf,
                                     const int *recvcounts, const int *displs,
                                     MPI_Datatype recvtype, MPIR_Comm *comm_ptr,
                                     MPIR_Errflag_t *errflag);

extern int MPIR_Allgatherv_Ring_MVP(const void *sendbuf, int sendcount,
                                    MPI_Datatype sendtype, void *recvbuf,
                                    const int *recvcounts, const int *displs,
                                    MPI_Datatype recvtype, MPIR_Comm *comm_ptr,
                                    MPIR_Errflag_t *errflag);

extern int MPIR_Allgatherv_Ring_Cyclic_MVP(
    const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf,
    const int *recvcounts, const int *displs, MPI_Datatype recvtype,
    MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);

/* Architecture detection tuning */
int MVP_set_allgatherv_tuning_table(int heterogeneity,
                                    struct coll_info *colls_arch_hca);

/* Function to clean free memory allocated by allgatherv tuning table*/
void MVP_cleanup_allgatherv_tuning_table();

#endif
