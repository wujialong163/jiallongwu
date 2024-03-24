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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <inttypes.h>
#include <limits.h>
#include "mvp_debug_utils.h"
#include "api/sharp_coll.h"
#include <mpiimpl.h>
#include "ofed_abstraction.h"

struct mvp_coll_sharp_module_t {
    struct sharp_coll_comm *sharp_coll_comm;
    int is_leader;
    int ppn;
    MPI_Comm comm;
};
typedef struct mvp_coll_sharp_module_t mvp_coll_sharp_module_t;

struct sharp_conf {
    int rank;
    int size;
    int ib_port;
    char *hostlist;
    char *jobid;
    char *ib_dev_list;
    char *ib_devname;
};
typedef struct sharp_conf mvp_sharp_conf_t;

mvp_sharp_reduce_datatype_size_t
{
    enum sharp_datatype sharp_data_type;
    int size;
};

struct mvp_sharp_coll_component_t {
    struct sharp_coll_context *sharp_coll_context;
    struct sharp_coll_caps sharp_caps;
};
typedef struct mvp_sharp_coll_component_t mvp_sharp_coll_component_t;

/* contains sharp_coll_context */
mvp_sharp_coll_component_t coll_sharp_component;

struct sharp_info {
    mvp_coll_sharp_module_t *sharp_comm_module;
    mvp_sharp_conf_t *sharp_conf;
};
typedef struct sharp_info mvp_sharp_info_t;

void mvp_get_sharp_datatype(MPI_Datatype mpi_datatype,
                            mvp_sharp_reduce_datatype_size_t **dt_size_out);
enum sharp_reduce_op mvp_get_sharp_reduce_op(MPI_Op mpi_op);
int mvp_sharp_coll_init(mvp_sharp_conf_t *sharp_conf, int rank, int local_rank);
int mvp_setup_sharp_env(mvp_sharp_conf_t *sharp_conf, MPI_Comm comm);
int mvp_sharp_coll_comm_init(mvp_coll_sharp_module_t *sharp_module);
char *sharp_create_hostlist(MPI_Comm comm);
int mvp_free_sharp_handlers(mvp_sharp_info_t *sharp_info);

#define MVP_SHARP_DIRECT_ALGO_MAX_PROC 0

#endif /* _IBV_SHARP_H_ */
