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

#ifndef _IALLTOALLV_TUNING_
#define _IALLTOALLV_TUNING_

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
    int (*MVP_pt_Ialltoallv_function)(
        const void *sendbuf, const int sendcounts[], const int sdispls[],
        MPI_Datatype sendtype, void *recvbuf, const int recvcounts[],
        const int rdispls[], MPI_Datatype recvtype, MPIR_Comm *comm_ptr,
        MPIR_Sched_t s);
    int zcopy_factor;
} mvp_ialltoallv_tuning_element;

typedef struct {
    int numproc;
    int ialltoallv_segment_size;
    int is_two_level_ialltoallv[MVP_MAX_NB_THRESHOLDS];
    int size_inter_table;
    mvp_ialltoallv_tuning_element inter_leader[MVP_MAX_NB_THRESHOLDS];
    int size_intra_table;
    mvp_ialltoallv_tuning_element intra_node[MVP_MAX_NB_THRESHOLDS];
} mvp_ialltoallv_tuning_table;

// extern int mvp_use_pipelined_alltoallv;
extern int ialltoallv_segment_size;
extern int mvp_size_ialltoallv_tuning_table;
extern mvp_ialltoallv_tuning_table *mvp_ialltoallv_thresholds_table;

/* Architecture detection tuning */
int MVP_set_ialltoallv_tuning_table(int heterogeneity);

/* Function to clean free memory allocated by ialltoallv tuning table*/
void MVP_cleanup_ialltoallv_tuning_table();

// Consider removing
/* Function used inside ch3_shmem_coll.c to tune ialltoallv thresholds */
int MVP_internode_Ialltoallv_is_define(char *mvp_user_ialltoallv_inter,
                                       char *mvp_user_ialltoallv_intra);
int MVP_intranode_Ialltoallv_is_define(char *mvp_user_ialltoallv_intra);

extern int MPIR_Ialltoallv_intra(const void *sendbuf, const int sendcounts[],
                                 const int sdispls[], MPI_Datatype sendtype,
                                 void *recvbuf, const int recvcounts[],
                                 const int rdispls[], MPI_Datatype recvtype,
                                 MPIR_Comm *comm_ptr, MPIR_Sched_t s);
#endif
