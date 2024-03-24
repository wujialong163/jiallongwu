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

#include "mpiimpl.h"

#if defined(_SHARP_SUPPORT_)
#include "mvp_sharp_abstraction.h"
#include "mvp_sharp.h"
#include "mvp_coll_shmem.h"
#include "upmi.h"

/*
=== BEGIN_MPI_T_MVP_CVAR_INFO_BLOCK ===

categories:
    - name : COLLECTIVE-OFFLOAD
      description: >-
        CVARs controlling MVAPICH collective offloading mechanisms including
        Mellanox SHArP

cvars:
    - name        : MVP_ENABLE_SHARP
      category    : COLLECTIVE-OFFLOAD
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Set this to 1, to enable hardware SHArP support in collective
        communication.

    - name        : MVP_SHARP_MAX_MSG_SIZE
      category    : COLLECTIVE
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_SHARP_PORT
      category    : COLLECTIVE
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        By default, this is set by the MVAPICH library. However, you
        can explicitly set the HCA port which is realized by the SHArP
        library.

    - name        : MVP_SHARP_HCA_NAME
      category    : COLLECTIVE
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        By default, this is set by the MVAPICH library. However, you
        can explicitly set the HCA name which is realized by the SHArP
        library.

    - name        : MVP_SHARP_MIN_NODE_COUNT
      category    : COLLECTIVE
      type        : int
      default     : 4
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_ENABLE_SHARP_ALLREDUCE
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Enable collective offloading via Mellanox SHArP for allreduce

    - name        : MVP_ENABLE_SHARP_BARRIER
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Enable collective offloading via Mellanox SHArP for barrier

    - name        : MVP_ENABLE_SHARP_BCAST
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Enable collective offloading via Mellanox SHArP for bcast

    - name        : MVP_ENABLE_SHARP_REDUCE
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Enable collective offloading via Mellanox SHArP for reduce

    - name        : MVP_ENABLE_SHARP_SCATTER
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Enable collective offloading via Mellanox SHArP for scatter

    - name        : MVP_ENABLE_SHARP_SCATTERV
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Enable collective offloading via Mellanox SHArP for scatterv

    - name        : MVP_ENABLE_SHARP_IALLREDUCE
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Enable collective offloading via Mellanox SHArP for iallreduce

    - name        : MVP_ENABLE_SHARP_IREDUCE
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Enable collective offloading via Mellanox SHArP for ireduce

    - name        : MVP_ENABLE_SHARP_IBCAST
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Enable collective offloading via Mellanox SHArP for ibcast

    - name        : MVP_ENABLE_SHARP_IBARRIER
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Enable collective offloading via Mellanox SHArP for ibarrier

=== END_MPI_T_MVP_CVAR_INFO_BLOCK ===
*/

MPI_Comm mvp_sharp_comm;

/* contains sharp_coll_context */
mvp_sharp_coll_component_t coll_sharp_component;
sharp_ops_t sharp_ops;
void *sharp_dl_handle = NULL;
int g_sharp_port = 0;
char *g_sharp_dev_name = "";

/* sharp supported datatype info*/
mvp_sharp_datatype_t sharp_data_types[] = {
    {"UINT_32_BIT", SHARP_DTYPE_UNSIGNED, MPI_UNSIGNED, MPI_2INT, 4},
    {"INT_32_BIT", SHARP_DTYPE_INT, MPI_INT, MPI_2INT, 4},
    {"UINT_64_BIT", SHARP_DTYPE_UNSIGNED_LONG, MPI_UNSIGNED_LONG, MPI_LONG_INT,
     8},
    {"INT_64_BIT", SHARP_DTYPE_LONG, MPI_LONG, MPI_LONG_INT, 8},
    {"FLOAT_32_BIT", SHARP_DTYPE_FLOAT, MPI_FLOAT, MPI_FLOAT_INT, 4},
    {"FLOAT_64_BIT", SHARP_DTYPE_DOUBLE, MPI_DOUBLE, MPI_DOUBLE_INT, 8},
    {"NULL", SHARP_DTYPE_NULL, 0, 0, 0}};

/*sharp supported reduce op info */
mvp_sharp_op_t sharp_ops_type[] = {{"MAX", SHARP_OP_MAX, MPI_MAX},
                                   {"MIN", SHARP_OP_MIN, MPI_MIN},
                                   {"SUM", SHARP_OP_SUM, MPI_SUM},
                                   {"LAND", SHARP_OP_LAND, MPI_LAND},
                                   {"BAND", SHARP_OP_BAND, MPI_BAND},
                                   {"LOR", SHARP_OP_LOR, MPI_LOR},
                                   {"BOR", SHARP_OP_BOR, MPI_BOR},
                                   {"LXOR", SHARP_OP_LXOR, MPI_LXOR},
                                   {"BXOR", SHARP_OP_BXOR, MPI_BXOR},
                                   {"MAXLOC", SHARP_OP_MAXLOC, MPI_MAXLOC},
                                   {"MINLOC", SHARP_OP_MINLOC, MPI_MINLOC},
                                   {"NOOP", SHARP_OP_NULL, 0}};

/* declared in Mellanox sharp.h, defined here */
const struct sharp_coll_config sharp_coll_default_config = {
    .ib_dev_list = "mlx5_0",
    .user_progress_num_polls = 128,
    .coll_timeout = 100};

int mvp_sharp_tuned_msg_size = MVP_DEFAULT_SHARP_MAX_MSG_SIZE;

void mvp_get_sharp_datatype(MPI_Datatype mpi_datatype,
                            mvp_sharp_reduce_datatype_size_t **dt_size_out)
{
    int i = 0;
    mvp_sharp_reduce_datatype_size_t *dt_size =
        MPL_malloc(sizeof(mvp_sharp_reduce_datatype_size_t), MPL_MEM_COLL);

    dt_size->sharp_data_type = SHARP_DTYPE_NULL;
    for (i = 0; sharp_data_types[i].sharp_data_type != SHARP_DTYPE_NULL; i++) {
        if (mpi_datatype == sharp_data_types[i].mpi_data_type) {
            dt_size->sharp_data_type = sharp_data_types[i].sharp_data_type;
            dt_size->size = sharp_data_types[i].size;
            break;
        }
    }

    *dt_size_out = dt_size;
}

enum sharp_reduce_op mvp_get_sharp_reduce_op(MPI_Op mpi_op)
{
    int i = 0;

    for (i = 0; sharp_ops_type[i].sharp_op_type != SHARP_OP_NULL; i++) {
        if (mpi_op == sharp_ops_type[i].mpi_op_type) {
            return sharp_ops_type[i].sharp_op_type;
        }
    }
    /* undefined sharp reduce op type */
    return SHARP_OP_NULL;
}

int mvp_oob_bcast(void *comm_context, void *buf, int size, int root)
{
    MPI_Comm comm;
    MPIR_Comm *comm_ptr = NULL;
    MPIR_Errflag_t errflag = FALSE;
    int mpi_errno = MPI_SUCCESS;

    /* currently only comm_world has SHARP support */
    comm = MPI_COMM_WORLD;
    MPIR_Comm_get_ptr(comm, comm_ptr);

    if (comm_ptr->dev.ch.is_sharp_ok == 1 && comm_context != NULL) {
        comm = ((mvp_coll_sharp_module_t *)comm_context)->comm;
    } else {
        comm = MPI_COMM_WORLD;
    }

    MPIR_Comm_get_ptr(comm, comm_ptr);
    mpi_errno = MPIR_Bcast_impl(buf, size, MPI_BYTE, root, comm_ptr, &errflag);
    if (mpi_errno) {
        PRINT_ERROR("sharp mvp_oob_bcast failed\n");
        MPIR_ERR_POP(mpi_errno);
    }

fn_exit:
    return (mpi_errno);

fn_fail:
    goto fn_exit;
}

int mvp_oob_barrier(void *comm_context)
{
    MPI_Comm comm;
    MPIR_Comm *comm_ptr = NULL;
    MPIR_Errflag_t errflag = FALSE;
    int mpi_errno = MPI_SUCCESS;

    /* currently only comm_world has SHARP support */
    comm = MPI_COMM_WORLD;
    MPIR_Comm_get_ptr(comm, comm_ptr);

    if (comm_ptr->dev.ch.is_sharp_ok == 1 && comm_context != NULL) {
        comm = ((mvp_coll_sharp_module_t *)comm_context)->comm;
    } else {
        comm = MPI_COMM_WORLD;
    }

    MPIR_Comm_get_ptr(comm, comm_ptr);
    mpi_errno = MPIR_Barrier_impl(comm_ptr, &errflag);
    if (mpi_errno) {
        PRINT_ERROR("sharp mvp_oob_barrier failed\n");
        MPIR_ERR_POP(mpi_errno);
    }

fn_exit:
    return (mpi_errno);

fn_fail:
    goto fn_exit;
}

int mvp_oob_gather(void *comm_context, int root, void *sbuf, void *rbuf,
                   int len)
{
    MPI_Comm comm;
    MPIR_Comm *comm_ptr = NULL;
    MPIR_Errflag_t errflag = FALSE;
    int mpi_errno = MPI_SUCCESS;

    /* currently only comm_world has SHARP support */
    comm = MPI_COMM_WORLD;
    MPIR_Comm_get_ptr(comm, comm_ptr);

    if (comm_ptr->dev.ch.is_sharp_ok == 1 && comm_context != NULL) {
        comm = ((mvp_coll_sharp_module_t *)comm_context)->comm;
    } else {
        comm = MPI_COMM_WORLD;
    }
    MPIR_Comm_get_ptr(comm, comm_ptr);

    mpi_errno = MPIR_Gather_impl(sbuf, len, MPI_BYTE, rbuf, len, MPI_BYTE, root,
                                 comm_ptr, &errflag);
    if (mpi_errno) {
        PRINT_ERROR("sharp mvp_oob_gather failed\n");
        MPIR_ERR_POP(mpi_errno);
    }

fn_exit:
    return (mpi_errno);

fn_fail:
    goto fn_exit;
}

/* initialize sharp */
int mvp_sharp_coll_init(mvp_sharp_conf_t *sharp_conf, int rank, int local_rank)
{
    int mpi_errno = SHARP_COLL_SUCCESS;
    struct sharp_coll_init_spec *init_spec =
        MPL_malloc(sizeof(struct sharp_coll_init_spec), MPL_MEM_COLL);

    init_spec->progress_func = NULL;
    init_spec->job_id = atol(sharp_conf->jobid);
    init_spec->world_local_rank = local_rank;
    init_spec->enable_thread_support = 0;
    init_spec->oob_colls.barrier = mvp_oob_barrier;
    init_spec->oob_colls.bcast = mvp_oob_bcast;
    init_spec->oob_colls.gather = mvp_oob_gather;
    init_spec->config = sharp_coll_default_config;
    init_spec->group_channel_idx = rank;
    init_spec->config.ib_dev_list = sharp_conf->ib_dev_list;
    MPI_Comm_rank(MPI_COMM_WORLD, &(init_spec->world_rank));
    MPI_Comm_size(MPI_COMM_WORLD, &(init_spec->world_size));

    mpi_errno = sharp_ops.coll_init(init_spec,
                                    &(coll_sharp_component.sharp_coll_context));

    if (mpi_errno != SHARP_COLL_SUCCESS) {
        if (rank == 0) {
            PRINT_DEBUG(DEBUG_Sharp_verbose,
                        "SHARP initialization failed. Continuing without SHARP"
                        " support. Errno: %d (%s) \n ",
                        mpi_errno, sharp_ops.coll_strerror(mpi_errno));
        }
        mpi_errno = MPI_ERR_INTERN;
        goto fn_exit;
    }

    mpi_errno =
        sharp_ops.coll_caps_query(coll_sharp_component.sharp_coll_context,
                                  &coll_sharp_component.sharp_caps);

    if (mpi_errno != SHARP_COLL_SUCCESS) {
        if (rank == 0) {
            PRINT_DEBUG(DEBUG_Sharp_verbose,
                        "SHARP cap query failed. Continuing without SHARP"
                        " support. Errno: %d (%s) \n ",
                        mpi_errno, sharp_ops.coll_strerror(mpi_errno));
        }
        mpi_errno = MPI_ERR_INTERN;
        goto fn_exit;
    }

fn_exit:
    MPL_free(init_spec);
    return (mpi_errno);

fn_fail:
    goto fn_exit;
}

int mvp_sharp_get_hca_name_and_port()
{
    int mpi_errno = MPI_SUCCESS;
#if defined(HAVE_LIBIBVERBS)
    int num_devices = 0, i = 0;
    int sharp_port = 0;
    struct ibv_device **dev_list = NULL;
    struct ibv_context *ctx;
    struct ibv_device_attr dev_attr;
    mvp_hca_t hca_type = 0;

    dev_list = ibv_ops.get_device_list(&num_devices);
    for (i = 0; i < num_devices; i++) {
        hca_type = mvp_get_hca_type(dev_list[i]);
        if (!MVP_IS_IB_CARD(hca_type)) {
            MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER,
                                "**mvp_sharp_devname");
        }

        // ctx = mvp_MPIDI_CH3I_RDMA_Process.nic_context[i];
        // if (ctx == NULL) {
        ctx = ibv_ops.open_device(dev_list[i]);
        //}
        if ((ibv_ops.query_device(ctx, &dev_attr))) {
            PRINT_ERROR("ibv_query_device failed \n");
            mpi_errno = MPI_ERR_INTERN;
            MPIR_ERR_POP(mpi_errno);
        }

        /* TODO: Need to bring in a cleaner version of this
         sharp_port = rdma_find_active_port(ctx, ctx->device,
                                           NULL, NULL, NULL);
        */
        sharp_port = 0; /* for now, hard coded */
        if (sharp_port != -1) {
            /* found a open port to be used for SHArP */
            goto finished_query;
        }
    }

finished_query:
    g_sharp_port = sharp_port;
    g_sharp_dev_name = (char *)ibv_ops.get_device_name(dev_list[i]);
    if (!g_sharp_dev_name) {
        MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER, "**mvp_sharp_devname");
    }

    if (dev_list) {
        ibv_ops.free_device_list(dev_list);
    }
    return 0;
#else
    MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER, "**mvp_sharp_devname");
#endif

fn_fail:
    return mpi_errno;
}

static char *mvp_sharp_get_kvs_id()
{
    int i = 0;
    char *id_str = MPL_malloc(100, MPL_MEM_COLL);
    MPIR_Memset(id_str, 0, 100);
    char KVSname[100] = {0};
    UPMI_KVS_GET_MY_NAME(KVSname, 100); /* set kvsname as job id */
    int kvs_offset = 4;                 // offset to skip kvs_

    for (i = kvs_offset; i < 100; i++) {
        if (KVSname[i] == '_')
            break;
        /* format is like kvs_906_storage03.cluster_26667_0 */
        if (isdigit(KVSname[i])) {
            id_str[i - kvs_offset] = KVSname[i];
        }
    }
    return id_str;
}

int mvp_setup_sharp_env(mvp_sharp_conf_t *sharp_conf, MPI_Comm comm)
{
    char *dev_list = NULL;
    int mpi_errno = MPI_SUCCESS;
    mvp_sharp_comm = comm;

    /* detect architecture and hca type */
    if (MVP_SHARP_HCA_NAME != 0) {
        /* user set HCA name and port for SHArP */
        g_sharp_dev_name = MVP_SHARP_HCA_NAME;
        if (MVP_SHARP_PORT != -1) {
            g_sharp_port = MVP_SHARP_PORT;
        } else {
            PRINT_DEBUG(
                DEBUG_Sharp_verbose,
                "sharp port number is not set. here we choose port 1 \n");
            g_sharp_port = 1;
        }
    } else {
        /* user did not specify the HCA name and port for SHArP, so here we
         * get these information */
        // TODO: Auto dectection is not working for now
        mpi_errno = mvp_sharp_get_hca_name_and_port();
        if (mpi_errno) {
            MPIR_ERR_POP(mpi_errno);
        }
    }
    sharp_conf->ib_devname = g_sharp_dev_name;

    /* set device name list and port number which is used for SHArP in
     * HCA_NAME:PORT_NUM format */
    dev_list = MPL_malloc(sizeof(sharp_conf->ib_devname) + 3, MPL_MEM_COLL);
    sprintf(dev_list, "%s:%d", sharp_conf->ib_devname, g_sharp_port);
    sharp_conf->ib_dev_list = dev_list;

    /* set comam separated hostlist */
    sharp_conf->hostlist = mvp_sharp_create_hostlist(comm);

    /* set kvsname as job id */
    sharp_conf->jobid = mvp_sharp_get_kvs_id();
    PRINT_DEBUG(DEBUG_Sharp_verbose > 2,
                "sharp_conf->jobid = %s and dev_list =  %s\n",
                sharp_conf->jobid, dev_list);

    MPI_Comm_rank(comm, &(sharp_conf->rank));
    MPI_Comm_size(comm, &(sharp_conf->size));

fn_exit:
    return mpi_errno;

fn_fail:
    goto fn_exit;
}

int mvp_sharp_coll_comm_init(mvp_coll_sharp_module_t *sharp_module)
{
    int size = 0, rank = 0;
    int mpi_errno = SHARP_COLL_SUCCESS;
    MPI_Comm comm;
    struct sharp_coll_comm_init_spec *comm_spec =
        MPL_malloc(sizeof(struct sharp_coll_comm_init_spec), MPL_MEM_COLL);

    comm = sharp_module->comm;
    sharp_module->is_leader = 1;

    MPI_Comm_size(comm, &size);
    MPI_Comm_rank(comm, &rank);
    comm_spec->rank = rank;
    comm_spec->size = size;
    comm_spec->oob_ctx = sharp_module;

    mpi_errno =
        sharp_ops.coll_comm_init(coll_sharp_component.sharp_coll_context,
                                 comm_spec, &(sharp_module->sharp_coll_comm));
    if (mpi_errno != SHARP_COLL_SUCCESS) {
        PRINT_ERROR("sharp communicator init failed with error code %d = %s\n",
                    mpi_errno, sharp_ops.coll_strerror(mpi_errno));
        mpi_errno = MPI_ERR_INTERN;
        MPIR_ERR_POP(mpi_errno);
    }

    PRINT_DEBUG(
        DEBUG_Sharp_verbose,
        "sharp communicator was initialized successfully for rank %d \n", rank);

fn_exit:
    MPL_free(comm_spec);
    return (mpi_errno);

fn_fail:
    goto fn_exit;
}
/* comma separated hostlist */
char *mvp_sharp_create_hostlist(MPI_Comm comm)
{
    int name_length, size, i, rank;
    char name[MPI_MAX_PROCESSOR_NAME];
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);
    int name_len[size];
    int offsets[size];
    MPIR_Errflag_t errflag = FALSE;
    int mpi_errno = MPI_SUCCESS;
    MPIR_Comm *comm_ptr = NULL;

    MPI_Get_processor_name(name, &name_length);
    if (rank < size - 1) {
        name[name_length++] = ',';
    }

    MPIR_Comm_get_ptr(comm, comm_ptr);
    mpi_errno = MPIR_Allgather_impl(&name_length, 1, MPI_INT, &name_len[0], 1,
                                    MPI_INT, comm_ptr, &errflag);
    if (mpi_errno) {
        PRINT_ERROR("collect hostname len from all ranks failed\n");
        MPIR_ERR_POP(mpi_errno);
    }
    int bytes = 0;
    for (i = 0; i < size; ++i) {
        offsets[i] = bytes;
        bytes += name_len[i];
    }
    bytes++;
    char *receive_buffer = MPL_malloc(bytes, MPL_MEM_COLL);
    receive_buffer[bytes - 1] = 0;
    mpi_errno = MPIR_Allgatherv_impl(&name[0], name_length, MPI_CHAR,
                                     &receive_buffer[0], &name_len[0],
                                     &offsets[0], MPI_CHAR, comm_ptr, &errflag);

    if (mpi_errno != MPI_SUCCESS) {
        PRINT_ERROR("sharp hostname collect failed\n");
        MPIR_ERR_POP(mpi_errno);
    }

fn_exit:
    return (receive_buffer);

fn_fail:
    goto fn_exit;
}

int mvp_free_sharp_handlers(mvp_sharp_info_t *sharp_info)
{
    int mpi_errno = SHARP_COLL_SUCCESS;

    if (sharp_info != NULL && sharp_info->sharp_comm_module != NULL &&
        sharp_info->sharp_comm_module->sharp_coll_comm != NULL) {
        /* destroy the sharp comm */
        mpi_errno = sharp_ops.coll_comm_destroy(
            sharp_info->sharp_comm_module->sharp_coll_comm);
        if (mpi_errno != SHARP_COLL_SUCCESS) {
            PRINT_ERROR("sharp comm destroy failed with error code %d = %s \n",
                        mpi_errno, sharp_ops.coll_strerror(mpi_errno));
            mpi_errno = MPI_ERR_INTERN;
            MPIR_ERR_POP(mpi_errno);
        }

        /* finalize SHArP */
        mpi_errno =
            sharp_ops.coll_finalize(coll_sharp_component.sharp_coll_context);
        if (mpi_errno != SHARP_COLL_SUCCESS) {
            PRINT_ERROR("sharp coll finalize failed with error code %d = %s\n",
                        mpi_errno, sharp_ops.coll_strerror(mpi_errno));
            mpi_errno = MPI_ERR_INTERN;
            MPIR_ERR_POP(mpi_errno);
        }
    }

    /* clean up */
    if (sharp_info != NULL) {
        if (sharp_info->sharp_conf != NULL) {
            if (sharp_info->sharp_conf->hostlist != NULL) {
                MPL_free(sharp_info->sharp_conf->hostlist);
                sharp_info->sharp_conf->hostlist = NULL;
            }
            if (sharp_info->sharp_conf->jobid != NULL) {
                MPL_free(sharp_info->sharp_conf->jobid);
                sharp_info->sharp_conf->jobid = NULL;
            }
            if (sharp_info->sharp_conf->ib_dev_list != NULL) {
                MPL_free(sharp_info->sharp_conf->ib_dev_list);
                sharp_info->sharp_conf->ib_dev_list = NULL;
            }
            MPL_free(sharp_info->sharp_conf);
            sharp_info->sharp_conf = NULL;
        }
        if (sharp_info->sharp_comm_module != NULL) {
            MPL_free(sharp_info->sharp_comm_module);
            sharp_info->sharp_comm_module = NULL;
        }

        MPL_free(sharp_info);
    }
    mpi_errno = MPI_SUCCESS;

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int mvp_create_sharp_comm(MPI_Comm comm, int size, int my_rank)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Comm *comm_ptr = NULL;
    int leader_group_size = 0, my_local_id = -1;
    MPIR_Errflag_t errflag = MPIR_ERR_NONE;

    if (size <= 1) {
        return mpi_errno;
    }

    MPIR_Comm_get_ptr(comm, comm_ptr);
    mpi_errno = PMPI_Comm_rank(comm_ptr->dev.ch.shmem_comm, &my_local_id);
    if (mpi_errno) {
        MPIR_ERR_POP(mpi_errno);
    }
    leader_group_size = comm_ptr->dev.ch.leader_group_size;
    comm_ptr->dev.ch.sharp_coll_info = NULL;

    if (comm == MPI_COMM_WORLD && MVP_ENABLE_SHARP &&
        (comm_ptr->dev.ch.is_sharp_ok == 0) &&
        leader_group_size >= MVP_SHARP_MIN_NODE_COUNT) {
        setenv("SHARP_COLL_ENABLE_GROUP_TRIM", "0", 0);
        setenv("SHARP_COLL_SHARP_ENABLE_MCAST_TARGET", "0", 0);

        MPIR_Comm_get_ptr(comm, comm_ptr);
        if (comm_ptr->local_size <= MVP_SHARP_DIRECT_ALGO_MAX_PROC &&
            MVP_ENABLE_SHARP == 1) {
            MVP_ENABLE_SHARP = 2; /* use direct algo for sharp */
        }

        mvp_sharp_info_t *sharp_coll_info = NULL;

        comm_ptr->dev.ch.sharp_coll_info = (mvp_sharp_info_t *)MPL_malloc(
            sizeof(mvp_sharp_info_t), MPL_MEM_COLL);
        sharp_coll_info = comm_ptr->dev.ch.sharp_coll_info;
        sharp_coll_info->sharp_comm_module = NULL;
        /* TODO : Change memory allocation for sharp_conf. Using a calloc for
         * now to zero out memory */
        sharp_coll_info->sharp_conf =
            MPL_calloc(1, sizeof(mvp_sharp_conf_t), MPL_MEM_COLL);

        sharp_ops.coll_log_early_init();
        mpi_errno =
            mvp_setup_sharp_env(sharp_coll_info->sharp_conf, MPI_COMM_WORLD);
        if (mpi_errno) {
            MPIR_ERR_POP(mpi_errno);
        }

        /* Initialize sharp */
        if (MVP_ENABLE_SHARP == 2) {
            /* Flat algorithm in which every process uses SHArP */
            mpi_errno = mvp_sharp_coll_init(sharp_coll_info->sharp_conf,
                                            my_local_id, my_local_id);
        } else if (MVP_ENABLE_SHARP == 1) {
            /* Two-level hierarchical algorithm in which, one process at each
             * node uses SHArP for inter-node communication */
            mpi_errno = mvp_sharp_coll_init(sharp_coll_info->sharp_conf, 0,
                                            my_local_id);
        } else {
            PRINT_ERROR("Invalid value for MVP_ENABLE_SHARP\n");
            mpi_errno = MPI_ERR_OTHER;
        }
        int can_support_sharp = 0;
        can_support_sharp = (mpi_errno == SHARP_COLL_SUCCESS) ? 1 : 0;

        int global_sharp_init_ok = 0;
        mpi_errno =
            MPIR_Allreduce_impl(&can_support_sharp, &global_sharp_init_ok, 1,
                                MPI_INT, MPI_LAND, comm_ptr, &errflag);
        if (mpi_errno) {
            MPIR_ERR_POP(mpi_errno);
        }

        if (global_sharp_init_ok == 0) {
            mvp_free_sharp_handlers(comm_ptr->dev.ch.sharp_coll_info);
            /* avoid using sharp and fall back to other designs */
            comm_ptr->dev.ch.sharp_coll_info = NULL;
            comm_ptr->dev.ch.is_sharp_ok = -1;
            /* we set it to -1 so that we do not get back to here anymore */
            mpi_errno = MPI_SUCCESS;
            PRINT_DEBUG(DEBUG_Sharp_verbose, "Falling back from Sharp  \n");
            goto sharp_fall_back;
        }

        sharp_coll_info->sharp_comm_module =
            MPL_malloc(sizeof(mvp_coll_sharp_module_t), MPL_MEM_COLL);
        MPIR_Memset(sharp_coll_info->sharp_comm_module, 0,
                    sizeof(mvp_coll_sharp_module_t));
        /* create sharp module which contains sharp communicator */
        if (MVP_ENABLE_SHARP == 2) {
            sharp_coll_info->sharp_comm_module->comm = MPI_COMM_WORLD;
            mpi_errno =
                mvp_sharp_coll_comm_init(sharp_coll_info->sharp_comm_module);
            if (mpi_errno) {
                MPIR_ERR_POP(mpi_errno);
            }
        } else if (my_local_id == 0) {
            sharp_coll_info->sharp_comm_module->comm =
                comm_ptr->dev.ch.leader_comm;
            mpi_errno =
                mvp_sharp_coll_comm_init(sharp_coll_info->sharp_comm_module);
            if (mpi_errno) {
                MPIR_ERR_POP(mpi_errno);
            }
        }
        comm_ptr->dev.ch.is_sharp_ok = 1;

        PRINT_DEBUG(DEBUG_Sharp_verbose,
                    "Sharp was initialized successfully \n");

        /* If the user does not set the MVP_SHARP_MAX_MSG_SIZE then try to tune
         * mvp_sharp_tuned_msg_size variable based on node count */
        if (MVP_ENABLE_SHARP == 1 &&
            (getenv("MVP_SHARP_MAX_MSG_SIZE")) == NULL) {
            if (leader_group_size == 2) {
                mvp_sharp_tuned_msg_size = 256;
            } else if (leader_group_size <= 4) {
                mvp_sharp_tuned_msg_size = 512;
            } else {
                /* in all other cases set max msg size to
                 * MVP_DEFAULT_SHARP_MAX_MSG_SIZE */
                mvp_sharp_tuned_msg_size = MVP_DEFAULT_SHARP_MAX_MSG_SIZE;
            }
        }
    }
sharp_fall_back:

fn_exit:
    return (mpi_errno);
fn_fail:
    mvp_free_sharp_handlers(comm_ptr->dev.ch.sharp_coll_info);
    comm_ptr->dev.ch.sharp_coll_info = NULL;
    goto fn_exit;
}

#endif /* if defined (_SHARP_SUPPORT_) */
