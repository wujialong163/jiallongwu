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

#ifndef _IGATHER_TUNING_
#define _IGATHER_TUNING_

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
    int (*MVP_pt_Igather_function)(const void *sendbuf, int sendcount,
                                   MPI_Datatype sendtype, void *recvbuf,
                                   int recvcount, MPI_Datatype recvtype,
                                   int root, MPIR_Comm *comm_ptr,
                                   MPIR_Sched_t s);
    int zcpy_knomial_factor;
} mvp_igather_tuning_element;

typedef struct {
    int numproc;
    int igather_segment_size;
    int intra_node_knomial_factor;
    int inter_node_knomial_factor;
    int is_two_level_igather[MVP_MAX_NB_THRESHOLDS];
    int size_inter_table;
    mvp_igather_tuning_element inter_leader[MVP_MAX_NB_THRESHOLDS];
    int size_intra_table;
    mvp_igather_tuning_element intra_node[MVP_MAX_NB_THRESHOLDS];
} mvp_igather_tuning_table;

extern int igather_segment_size;
extern int mvp_size_igather_tuning_table;
extern mvp_igather_tuning_table *mvp_igather_thresholds_table;

/* Architecture detection tuning */
int MVP_set_igather_tuning_table(int heterogeneity);

/* Function to clean free memory allocated by igather tuning table*/
void MVP_cleanup_igather_tuning_table();

// Consider removing
/* Function used inside ch3_shmem_coll.c to tune igather thresholds */
int MVP_internode_Igather_is_define(char *mvp_user_igather_inter,
                                    char *mvp_user_igather_intra);
int MVP_intranode_Igather_is_define(char *mvp_user_igather_intra);

extern int MPIR_Igather_intra_sched_binomial(const void *sendbuf, int sendcount,
                                             MPI_Datatype sendtype,
                                             void *recvbuf, int recvcount,
                                             MPI_Datatype recvtype, int root,
                                             MPIR_Comm *comm_ptr,
                                             MPIR_Sched_t s);
#endif
