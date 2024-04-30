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

#ifndef _IBCAST_TUNING_
#define _IBCAST_TUNING_

#include "mvp_coll_shmem.h"
#if defined(CHANNEL_MRAIL)
#include "ibv_param.h"
#endif /* #if defined(CHANNEL_MRAIL) */

#define NMATCH (3 + 1)

typedef struct {
    int min;
    int max;
    int (*MVP_pt_Ibcast_function)(void *buffer, int count,
                                  MPI_Datatype datatype, int root,
                                  MPIR_Comm *comm_ptr, MPIR_Sched_t s);
    int zcpy_knomial_factor;
} mvp_ibcast_tuning_element;

typedef struct {
    int numproc;
    int ibcast_segment_size;
    int intra_node_knomial_factor;
    int inter_node_knomial_factor;
    int is_two_level_ibcast[MVP_MAX_NB_THRESHOLDS];
    int size_inter_table;
    mvp_ibcast_tuning_element inter_leader[MVP_MAX_NB_THRESHOLDS];
    int size_intra_table;
    mvp_ibcast_tuning_element intra_node[MVP_MAX_NB_THRESHOLDS];
} mvp_ibcast_tuning_table;

extern int ibcast_segment_size;
extern int mvp_size_ibcast_tuning_table;
extern mvp_ibcast_tuning_table *mvp_ibcast_thresholds_table;

/* Architecture detection tuning */
int MVP_set_ibcast_tuning_table(int heterogeneity);

/* Function to clean free memory allocated by ibcast tuning table*/
void MVP_cleanup_ibcast_tuning_table();

// Consider removing
/* Function used inside ch3_shmem_coll.c to tune ibcast thresholds */
int MVP_internode_Ibcast_is_define(char *mvp_user_ibcast_inter,
                                   char *mvp_user_ibcast_intra);
int MVP_intranode_Ibcast_is_define(char *mvp_user_ibcast_intra);

extern int MPIR_Ibcast_intra_sched_binomial(void *buffer, int count,
                                            MPI_Datatype datatype, int root,
                                            MPIR_Comm *comm_ptr,
                                            MPIR_Sched_t s);

extern int MPIR_Ibcast_intra_sched_scatter_recursive_doubling_allgather(
    void *buffer, int count, MPI_Datatype datatype, int root,
    MPIR_Comm *comm_ptr, MPIR_Sched_t s);

extern int MPIR_Ibcast_intra_sched_scatter_ring_allgather(
    void *buffer, int count, MPI_Datatype datatype, int root,
    MPIR_Comm *comm_ptr, MPIR_Sched_t s);
#endif
