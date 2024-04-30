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

#ifndef _IBV_SHARP_H_
#define _IBV_SHARP_H_

#include "api/sharp_coll.h"
#include "mvp_sharp_abstraction.h"

#define MAX_SHARP_PORT                 5
#define MVP_SHARP_DIRECT_ALGO_MAX_PROC 0
#define MVP_DEFAULT_SHARP_MAX_MSG_SIZE 1024

typedef struct mvp_coll_sharp_module {
    struct sharp_coll_comm *sharp_coll_comm;
    int is_leader;
    int ppn;
    MPI_Comm comm;
} mvp_coll_sharp_module_t;

typedef struct mvp_sharp_conf {
    int rank;
    int size;
    int ib_port;
    char *hostlist;
    char *jobid;
    char *ib_dev_list;
    char *ib_devname;
} mvp_sharp_conf_t;

typedef struct mvp_sharp_reduce_datatype_size {
    enum sharp_datatype sharp_data_type;
    int size;
} mvp_sharp_reduce_datatype_size_t;

typedef struct mvp_sharp_coll_component {
    struct sharp_coll_context *sharp_coll_context;
    struct sharp_coll_caps sharp_caps;
} mvp_sharp_coll_component_t;

typedef struct mvp_sharp_datatype {
    char *name;
    enum sharp_datatype sharp_data_type;
    MPI_Datatype mpi_data_type;
    MPI_Datatype mpi_data_type_pair;
    int size;
} mvp_sharp_datatype_t;

typedef struct mvp_sharp_op {
    char *name;
    enum sharp_reduce_op sharp_op_type;
    MPI_Datatype mpi_op_type;
} mvp_sharp_op_t;

typedef struct mvp_sharp_info {
    mvp_coll_sharp_module_t *sharp_comm_module;
    mvp_sharp_conf_t *sharp_conf;
} mvp_sharp_info_t;

void mvp_get_sharp_datatype(MPI_Datatype mpi_datatype,
                            mvp_sharp_reduce_datatype_size_t **dt_size_out);
enum sharp_reduce_op mvp_get_sharp_reduce_op(MPI_Op mpi_op);
int mvp_sharp_coll_init(mvp_sharp_conf_t *sharp_conf, int rank, int local_rank);
int mvp_setup_sharp_env(mvp_sharp_conf_t *sharp_conf, MPI_Comm comm);
int mvp_sharp_coll_comm_init(mvp_coll_sharp_module_t *sharp_module);
char *mvp_sharp_create_hostlist(MPI_Comm comm);
int mvp_free_sharp_handlers(mvp_sharp_info_t *sharp_info);
int mvp_create_sharp_comm(MPI_Comm comm, int size, int my_rank);

/* max collective message that uses SHArP */
int mvp_sharp_tuned_msg_size;

int MPIR_MVP_sharp_comm_setup_allreduce();
int MPIR_MVP_sharp_comm_setup_barrier();
int MPIR_MVP_sharp_comm_setup_bcast();
int MPIR_MVP_sharp_comm_setup_scatter();
int MPIR_MVP_sharp_comm_setup_scatterv();
int MPIR_MVP_sharp_comm_setup_reduce();

int MPIR_MVP_sharp_comm_setup_iallreduce();
int MPIR_MVP_sharp_comm_setup_ibarrier();
int MPIR_MVP_sharp_comm_setup_ibcast();
int MPIR_MVP_sharp_comm_setup_ireduce();

int MPIR_Sharp_Scatterv_MVP(const void *sendbuf, const int *sendcounts,
                            const int *displs, MPI_Datatype sendtype,
                            void *recvbuf, int recvcount, MPI_Datatype recvtype,
                            int root, MPIR_Comm *comm_ptr,
                            MPIR_Errflag_t *errflag);
int MPIR_Sharp_Iallreduce_MVP(const void *sendbuf, void *recvbuf, int count,
                              MPI_Datatype datatype, MPI_Op op,
                              MPIR_Comm *comm_ptr, int *errflag,
                              MPIR_Request **req);
int MPIR_Sharp_Ireduce_MVP(const void *sendbuf, void *recvbuf, int count,
                           MPI_Datatype datatype, MPI_Op op, int root,
                           MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag,
                           MPIR_Request **req);
int MPIR_Sharp_Ibcast_MVP(void *buffer, int count, MPI_Datatype datatype,
                          int root, MPIR_Comm *comm_ptr,
                          MPIR_Errflag_t *errflag, MPIR_Request **req);
int MPIR_Sharp_Ibarrier_MVP(MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag,
                            MPIR_Request **req);
#endif /* _IBV_SHARP_H_ */
