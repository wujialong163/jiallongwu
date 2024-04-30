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

#ifndef _ALLREDUCE_TUNING_
#define _ALLREDUCE_TUNING_

#include "mvp_coll_shmem.h"

#define NMATCH (3 + 1)

/* Allreduce tuning flags
 * flat recursive doubling(rd): MVP_INTER_ALLREDUCE_TUNING=1
 * flat reduce scatter allgather(rsa): MVP_INTER_ALLREDUCE_TUNING=2
 * mcast: MVP_ALLREDUCE_TUNING_IS_TWO_LEVEL=1 MVP_USE_MCAST_ALLREDUCE=1
 *        MVP_USE_MCAST=1 MVP_INTER_ALLREDUCE_TUNING=3
 * 2-level: MVP_ALLREDUCE_TUNING_IS_TWO_LEVEL=1
 *          MVP_INTER_ALLREDUCE_TUNING=?  MVP_INTRA_ALLREDUCE_TUNING=?
 *          intra-reduce flag can take 1(rd), 2(rsa), 5(shm), 6(p2p), while
 *          inter-reduce flag can take 1(rd), 2(rsa)
 */

typedef int (*MVP_Allreduce_fn_t)(const void *sendbuf, void *recvbuf, int count,
                                  MPI_Datatype datatype, MPI_Op op,
                                  MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);

typedef struct {
    int min;
    int max;
    MVP_Allreduce_fn_t allreduce_fn;
} mvp_allreduce_tuning_element;

typedef struct {
    int numproc;
    int mcast_enabled;
    int is_two_level_allreduce[MVP_MAX_NB_THRESHOLDS];
    int size_inter_table;
    mvp_allreduce_tuning_element inter_leader[MVP_MAX_NB_THRESHOLDS];
    int size_intra_table;
    mvp_allreduce_tuning_element intra_node[MVP_MAX_NB_THRESHOLDS];
} mvp_allreduce_tuning_table;

extern int mvp_size_allreduce_tuning_table;
extern mvp_allreduce_tuning_table *mvp_allreduce_thresholds_table;

/*Entries related to indexed tuning table*/

typedef struct {
    int msg_sz;
    MVP_Allreduce_fn_t allreduce_fn;
} mvp_allreduce_indexed_tuning_element;

typedef struct {
    int numproc;
    int mcast_enabled;
    int is_two_level_allreduce[MVP_MAX_NB_THRESHOLDS];
    int size_inter_table;
    mvp_allreduce_indexed_tuning_element inter_leader[MVP_MAX_NB_THRESHOLDS];
    int size_intra_table;
    mvp_allreduce_indexed_tuning_element intra_node[MVP_MAX_NB_THRESHOLDS];
} mvp_allreduce_indexed_tuning_table;

/* Indicates number of processes per node */
extern int *mvp_allreduce_indexed_table_ppn_conf;
/* Indicates total number of configurations */
extern int mvp_allreduce_indexed_num_ppn_conf;
extern int *mvp_size_allreduce_indexed_tuning_table;
extern mvp_allreduce_indexed_tuning_table *
    *mvp_allreduce_indexed_thresholds_table;

/* flat p2p recursive-doubling allreduce */
extern int MPIR_Allreduce_pt2pt_rd_MVP(const void *sendbuf, void *recvbuf,
                                       int count, MPI_Datatype datatype,
                                       MPI_Op op, MPIR_Comm *comm_ptr,
                                       MPIR_Errflag_t *errflag);

/* flat p2p reduce-scatter-allgather allreduce */
extern int MPIR_Allreduce_pt2pt_rs_MVP(const void *sendbuf, void *recvbuf,
                                       int count, MPI_Datatype datatype,
                                       MPI_Op op, MPIR_Comm *comm_ptr,
                                       MPIR_Errflag_t *errflag);

extern int MPIR_Allreduce_pt2pt_reduce_scatter_allgather_MVP(
    const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
    MPI_Op op, MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);

extern int MPIR_Allreduce_mcst_MVP(const void *sendbuf, void *recvbuf,
                                   int count, MPI_Datatype datatype, MPI_Op op,
                                   MPIR_Comm *comm_ptr,
                                   MPIR_Errflag_t *errflag);

extern int MPIR_Allreduce_two_level_MVP(const void *sendbuf, void *recvbuf,
                                        int count, MPI_Datatype datatype,
                                        MPI_Op op, MPIR_Comm *comm_ptr,
                                        MPIR_Errflag_t *errflag);

#if defined(_SHARP_SUPPORT_)
extern int MPIR_Sharp_Allreduce_MVP(const void *sendbuf, void *recvbuf,
                                    int count, MPI_Datatype datatype, MPI_Op op,
                                    MPIR_Comm *comm_ptr,
                                    MPIR_Errflag_t *errflag);
#endif /*defined (_SHARP_SUPPORT_)*/

extern int MPIR_Allreduce_socket_aware_two_level_MVP(
    const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
    MPI_Op op, MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);

extern int MPIR_Allreduce_topo_aware_hierarchical_MVP(
    const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
    MPI_Op op, MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);

/* shmem reduce used as the first reduce in allreduce */
extern int MPIR_Allreduce_reduce_shmem_MVP(const void *sendbuf, void *recvbuf,
                                           int count, MPI_Datatype datatype,
                                           MPI_Op op, MPIR_Comm *comm_ptr,
                                           MPIR_Errflag_t *errflag);
/* p2p reduce used as the first reduce in allreduce */
extern int MPIR_Allreduce_reduce_p2p_MVP(const void *sendbuf, void *recvbuf,
                                         int count, MPI_Datatype datatype,
                                         MPI_Op op, MPIR_Comm *comm_ptr,
                                         MPIR_Errflag_t *errflag);

extern int MPIR_Allreduce_mcst_reduce_two_level_helper_MVP(
    const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
    MPI_Op op, MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);

extern int MPIR_Allreduce_mcst_reduce_redscat_gather_MVP(
    const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
    MPI_Op op, MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);

extern int MPIR_Allreduce_pt2pt_ring_wrapper_MVP(const void *sendbuf,
                                                 void *recvbuf, int count,
                                                 MPI_Datatype datatype,
                                                 MPI_Op op, MPIR_Comm *comm_ptr,
                                                 MPIR_Errflag_t *errflag);

extern int MPIR_Allreduce_pt2pt_ring_MVP(const void *sendbuf, void *recvbuf,
                                         int count, MPI_Datatype datatype,
                                         MPI_Op op, MPIR_Comm *comm_ptr,
                                         MPIR_Errflag_t *errflag);

extern int MPIR_Allreduce_pt2pt_ring_inplace_MVP(const void *sendbuf,
                                                 void *recvbuf, int count,
                                                 MPI_Datatype datatype,
                                                 MPI_Op op, MPIR_Comm *comm_ptr,
                                                 MPIR_Errflag_t *errflag);

extern int MPIR_Allreduce_shmem_MVP(const void *sendbuf, void *recvbuf,
                                    int count, MPI_Datatype datatype, MPI_Op op,
                                    MPIR_Comm *comm_ptr,
                                    MPIR_Errflag_t *errflag);

extern MVP_Allreduce_fn_t MVP_Allreduce_function;
extern MVP_Allreduce_fn_t MVP_Allreduce_intra_function;

/* Architecture detection tuning */
int MVP_set_allreduce_tuning_table(int heterogeneity,
                                   struct coll_info *colls_arch_hca);

/* Function to clean free memory allocated by allreduce tuning table*/
void MVP_cleanup_allreduce_tuning_table();

#endif
