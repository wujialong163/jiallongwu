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

#ifndef _RED_SCAT_BLOCK_TUNING_
#define _RED_SCAT_BLOCK_TUNING_

#include "mvp_coll_shmem.h"

#define NMATCH (3 + 1)

/* Red_scat tuning flags
 * RED_SCAT_BLOCK_RING      :   MVP_INTER_RED_SCAT_BLOCK_TUNING=1
 * RED_SCAT_BLOCK_RING_2LVL :   MVP_INTER_RED_SCAT_BLOCK_TUNING=2
 *
 * Regular expression example:
 *   MVP_INTER_RED_SCAT_BLOCK_TUNING=2:16384-,1:1-8192+
 *   meaning: use RED_SCAT_BLOCK_RING for 2 byte to 16384 bytes
 *            use RED_SCAT_BLOCK_RING_2LVL for 1 byte to 8192 bytes
 */

typedef struct {
    int min;
    int max;
    int (*MVP_Red_scat_block_function)(const void *sendbuf, void *recvbuf,
                                       int recvcount, MPI_Datatype datatype,
                                       MPI_Op op, MPIR_Comm *comm_ptr,
                                       MPIR_Errflag_t *errflag);
} mvp_red_scat_block_tuning_element;

typedef struct {
    int numproc;
    int size_inter_table;
    mvp_red_scat_block_tuning_element inter_leader[MVP_MAX_NB_THRESHOLDS];
} mvp_red_scat_block_tuning_table;

extern int mvp_size_red_scat_block_tuning_table;
extern mvp_red_scat_block_tuning_table *mvp_red_scat_block_thresholds_table;

extern int MPIR_Reduce_scatter_block_ring_2lvl_MVP(
    const void *sendbuf, void *recvbuf, int recvcount, MPI_Datatype datatype,
    MPI_Op op, MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);

extern int MPIR_Reduce_scatter_block_ring_MVP(const void *sendbuf,
                                              void *recvbuf, int recvcount,
                                              MPI_Datatype datatype, MPI_Op op,
                                              MPIR_Comm *comm_ptr,
                                              MPIR_Errflag_t *errflag);

/* Architecture detection tuning */
int MVP_set_red_scat_block_tuning_table(int heterogeneity,
                                        struct coll_info *colls_arch_hca);

/* Function to clean free memory allocated by red_scat tuning table*/
void MVP_cleanup_red_scat_block_tuning_table();

/* Function used inside ch3_shmem_coll.c to tune red_scat thresholds */
int MVP_internode_Red_scat_block_is_define(char *mvp_user_red_scat_block_inter);
#endif /* ifndef _RED_SCAT_BLOCK_TUNING_ */
