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

#ifndef _IALLREDUCE_TUNING_
#define _IALLREDUCE_TUNING_

#include "mvp_coll_shmem.h"
#if defined(CHANNEL_MRAIL)
#include "ibv_param.h"
#endif /* #if defined(CHANNEL_MRAIL) */

#define NMATCH (3 + 1)

/* Note: Several members of the structures used are meant to be used
   sometime in the future */

typedef struct {
    int min;
    int max;
    int (*MVP_pt_Iallreduce_function)(const void *sendbuf, void *recvbuf,
                                      int count, MPI_Datatype datatype,
                                      MPI_Op op, MPIR_Comm *comm_ptr,
                                      MPIR_Sched_t s);
    int zcpy_knomial_factor;
} mvp_iallreduce_tuning_element;

typedef struct {
    int numproc;
    int iallreduce_segment_size;
    int intra_node_knomial_factor;
    int inter_node_knomial_factor;
    int is_two_level_iallreduce[MVP_MAX_NB_THRESHOLDS];
    int size_inter_table;
    mvp_iallreduce_tuning_element inter_leader[MVP_MAX_NB_THRESHOLDS];
    int size_intra_table;
    mvp_iallreduce_tuning_element intra_node[MVP_MAX_NB_THRESHOLDS];
} mvp_iallreduce_tuning_table;

// extern int mvp_use_pipelined_allreduce;
extern int iallreduce_segment_size;
extern int mvp_size_iallreduce_tuning_table;
extern mvp_iallreduce_tuning_table *mvp_iallreduce_thresholds_table;

/* Architecture detection tuning */
int MVP_set_iallreduce_tuning_table(int heterogeneity);

/* Function to clean free memory allocated by iallreduce tuning table*/
void MVP_cleanup_iallreduce_tuning_table();

// Consider removing
/* Function used inside ch3_shmem_coll.c to tune iallreduce thresholds */
int MVP_internode_Iallreduce_is_define(char *mvp_user_iallreduce_inter,
                                       char *mvp_user_iallreduce_intra);
int MVP_intranode_Iallreduce_is_define(char *mvp_user_iallreduce_intra);

extern int MPIR_Iallreduce_intra_sched_naive(const void *sendbuf, void *recvbuf,
                                             int count, MPI_Datatype datatype,
                                             MPI_Op op, MPIR_Comm *comm_ptr,
                                             MPIR_Sched_t s);
extern int MPIR_Iallreduce_intra_sched_reduce_scatter_allgather(
    const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
    MPI_Op op, MPIR_Comm *comm_ptr, MPIR_Sched_t s);
extern int MPIR_Iallreduce_intra_sched_recursive_doubling(
    const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
    MPI_Op op, MPIR_Comm *comm_ptr, MPIR_Sched_t s);
#if defined(_SHARP_SUPPORT_)
extern int MPIR_Sharp_Iallreduce_MVP(const void *sendbuf, void *recvbuf,
                                     int count, MPI_Datatype datatype,
                                     MPI_Op op, MPIR_Comm *comm_ptr,
                                     int *errflag, MPIR_Request **req);
#endif /*defined (_SHARP_SUPPORT_)*/
#endif
