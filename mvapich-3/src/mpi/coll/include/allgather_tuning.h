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

#ifndef _ALLGATHER_TUNING_
#define _ALLGATHER_TUNING_

#include "mvp_coll_shmem.h"

#define NMATCH (3 + 1)
/* Indicates number of processes per node */
extern int *mvp_allgather_table_ppn_conf;
/* Indicates total number of configurations */
extern int mvp_allgather_num_ppn_conf;

typedef int (*MVP_Allgather_fn_t)(const void *sendbuf, int sendcount,
                                  MPI_Datatype sendtype, void *recvbuf,
                                  int recvcount, MPI_Datatype recvtype,
                                  MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);

typedef struct {
    int min;
    int max;
    MVP_Allgather_fn_t allgather_fn;
} mvp_allgather_tuning_element;

typedef struct {
    int numproc;
    int two_level[MVP_MAX_NB_THRESHOLDS];
    int size_inter_table;
    mvp_allgather_tuning_element inter_leader[MVP_MAX_NB_THRESHOLDS];
} mvp_allgather_tuning_table;

extern int *mvp_size_allgather_tuning_table;
extern mvp_allgather_tuning_table **mvp_allgather_thresholds_table;

typedef struct {
    int msg_sz;
    MVP_Allgather_fn_t allgather_fn;
} mvp_allgather_indexed_tuning_element;

typedef struct {
    int numproc;
    int size_inter_table;
    mvp_allgather_indexed_tuning_element inter_leader[MVP_MAX_NB_THRESHOLDS];
} mvp_allgather_indexed_tuning_table;

/* Indicates number of processes per node */
extern int *mvp_allgather_indexed_table_ppn_conf;
/* Indicates total number of configurations */
extern int mvp_allgather_indexed_num_ppn_conf;
extern int *mvp_size_allgather_indexed_tuning_table;
extern mvp_allgather_indexed_tuning_table *
    *mvp_allgather_indexed_thresholds_table;

extern int MPIR_Allgather_RD_Allgather_Comm_MVP(
    const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf,
    int recvcount, MPI_Datatype recvtype, MPIR_Comm *comm_ptr,
    MPIR_Errflag_t *errflag);

extern int MPIR_Allgather_RD_MVP(const void *sendbuf, int sendcount,
                                 MPI_Datatype sendtype, void *recvbuf,
                                 int recvcount, MPI_Datatype recvtype,
                                 MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);

extern int MPIR_Allgather_Bruck_MVP(const void *sendbuf, int sendcount,
                                    MPI_Datatype sendtype, void *recvbuf,
                                    int recvcount, MPI_Datatype recvtype,
                                    MPIR_Comm *comm_ptr,
                                    MPIR_Errflag_t *errflag);

extern int MPIR_Allgather_Ring_MVP(const void *sendbuf, int sendcount,
                                   MPI_Datatype sendtype, void *recvbuf,
                                   int recvcount, MPI_Datatype recvtype,
                                   MPIR_Comm *comm_ptr,
                                   MPIR_Errflag_t *errflag);

extern int MPIR_2lvl_Allgather_MVP(const void *sendbuf, int sendcnt,
                                   MPI_Datatype sendtype, void *recvbuf,
                                   int recvcnt, MPI_Datatype recvtype,
                                   MPIR_Comm *comm_ptr,
                                   MPIR_Errflag_t *errflag);

extern MVP_Allgather_fn_t MVP_Allgather_function;

int MPIR_Allgather_Direct_MVP(const void *sendbuf, int sendcnt,
                              MPI_Datatype sendtype, void *recvbuf, int recvcnt,
                              MPI_Datatype recvtype, MPIR_Comm *comm_ptr,
                              MPIR_Errflag_t *errflag);

int MPIR_Allgather_DirectSpread_MVP(const void *sendbuf, int sendcnt,
                                    MPI_Datatype sendtype, void *recvbuf,
                                    int recvcnt, MPI_Datatype recvtype,
                                    MPIR_Comm *comm_ptr,
                                    MPIR_Errflag_t *errflag);

int MPIR_Allgather_gather_bcast_MVP(const void *sendbuf, int sendcount,
                                    MPI_Datatype sendtype, void *recvbuf,
                                    int recvcount, MPI_Datatype recvtype,
                                    MPIR_Comm *comm_ptr,
                                    MPIR_Errflag_t *errflag);

int MPIR_2lvl_Allgather_nonblocked_MVP(const void *sendbuf, int sendcnt,
                                       MPI_Datatype sendtype, void *recvbuf,
                                       int recvcnt, MPI_Datatype recvtype,
                                       MPIR_Comm *comm_ptr,
                                       MPIR_Errflag_t *errflag);

int MPIR_2lvl_Allgather_Ring_nonblocked_MVP(const void *sendbuf, int sendcount,
                                            MPI_Datatype sendtype,
                                            void *recvbuf, int recvcount,
                                            MPI_Datatype recvtype,
                                            MPIR_Comm *comm_ptr,
                                            MPIR_Errflag_t *errflag);

int MPIR_2lvl_Allgather_Direct_MVP(const void *sendbuf, int sendcnt,
                                   MPI_Datatype sendtype, void *recvbuf,
                                   int recvcnt, MPI_Datatype recvtype,
                                   MPIR_Comm *comm_ptr,
                                   MPIR_Errflag_t *errflag);

int MPIR_2lvl_Allgather_Ring_MVP(const void *sendbuf, int sendcnt,
                                 MPI_Datatype sendtype, void *recvbuf,
                                 int recvcnt, MPI_Datatype recvtype,
                                 MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);

/* Architecture detection tuning */
int MVP_set_allgather_tuning_table(int heterogeneity,
                                   struct coll_info *colls_arch_hca);

/* Function to clean free memory allocated by allgather tuning table*/
void MVP_cleanup_allgather_tuning_table();
#endif
