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

#define COLLECTIVE bcast

#include <regex.h>
#include "mvp_common_tuning.h"
#include "bcast_tuning.h"
#include "bcast_arch_tuning.h"
#include "mvp_tuning_tables.h"

/* TODO: This should be moved to the pre-header as it is in ch4 */
#ifdef CHANNEL_MRAIL
#include "mvp_arch_hca_detect.h"
#endif

/* array used to tune bcast */

int mvp_size_bcast_tuning_table = 0;
mvp_bcast_tuning_table *mvp_bcast_thresholds_table = NULL;

int *mvp_bcast_indexed_table_ppn_conf = NULL;
int mvp_bcast_indexed_num_ppn_conf = 1;
int *mvp_size_bcast_indexed_tuning_table = NULL;
mvp_bcast_indexed_tuning_table **mvp_bcast_indexed_thresholds_table = NULL;

static inline MVP_Bcast_fn_t MVP_get_inter_node_bcast_fn()
{
    switch (MVP_BCAST_INTER_NODE_TUNING_ALGO) {
        case MVP_BCAST_INTER_NODE_TUNING_ALGO_SCATTER_DOUBLING_ALLGATHER:
            return &MPIR_Bcast_intra_scatter_recursive_doubling_allgather;
        case MVP_BCAST_INTER_NODE_TUNING_ALGO_SCATTER_RING_ALLGATHER:
            return &MPIR_Bcast_scatter_ring_allgather_MVP;
        case MVP_BCAST_INTER_NODE_TUNING_ALGO_SCATTER_RING_ALLGATHER_SHM:
            return &MPIR_Bcast_scatter_ring_allgather_shm_MVP;
        case MVP_BCAST_INTER_NODE_TUNING_ALGO_KNOMIAL:
            return &MPIR_Knomial_Bcast_inter_node_wrapper_MVP;
        case MVP_BCAST_INTER_NODE_TUNING_ALGO_PIPELINED:
            return &MPIR_Pipelined_Bcast_MVP;
        default:
            PRINT_ERROR("WARNING: Invalid value for "
                        "MVP_BCAST_INTER_NODE_TUNING_ALGO.\n");
        case MVP_BCAST_INTER_NODE_TUNING_ALGO_UNSET:
        case MVP_BCAST_INTER_NODE_TUNING_ALGO_BINOMIAL:
            return &MPIR_Bcast_binomial_MVP;
    }
}
static inline MVP_Bcast_fn_t MVP_get_intra_node_bcast_fn()
{
    switch (MVP_BCAST_INTRA_NODE_TUNING_ALGO) {
        case MVP_BCAST_INTRA_NODE_TUNING_ALGO_KNOMIAL:
            return &MPIR_Knomial_Bcast_intra_node_MVP;
        default:
            PRINT_ERROR("WARNING: Invalid value for "
                        "MVP_BCAST_INTRA_NODE_TUNING_ALGO.\n");
        case MVP_BCAST_INTRA_NODE_TUNING_ALGO_UNSET:
        case MVP_BCAST_INTRA_NODE_TUNING_ALGO_SHMEM:
            return &MPIR_Shmem_Bcast_MVP;
    }
}

static inline void MVP_set_user_defined_gather_tuning_table()
{
    mvp_bcast_indexed_num_ppn_conf = 1;

    mvp_bcast_indexed_thresholds_table =
        MPL_malloc(mvp_bcast_indexed_num_ppn_conf *
                       sizeof(mvp_bcast_indexed_tuning_table *),
                   MPL_MEM_COLL);
    mvp_size_bcast_indexed_tuning_table =
        MPL_malloc(mvp_bcast_indexed_num_ppn_conf * sizeof(int), MPL_MEM_COLL);
    mvp_bcast_indexed_table_ppn_conf =
        MPL_malloc(mvp_bcast_indexed_num_ppn_conf * sizeof(int), MPL_MEM_COLL);

    /* -1 indicates user defined algorithm */
    mvp_bcast_indexed_table_ppn_conf[0] = -1;
    mvp_size_bcast_indexed_tuning_table[0] = 1;
    mvp_bcast_indexed_thresholds_table[0] =
        MPL_malloc(mvp_size_bcast_indexed_tuning_table[0] *
                       sizeof(mvp_bcast_indexed_tuning_table),
                   MPL_MEM_COLL);

    mvp_bcast_indexed_tuning_table tmp_table = {
        .numproc = 1,
        .bcast_segment_size = MVP_BCAST_SEGMENT_SIZE,
        .inter_node_knomial_factor = MVP_KNOMIAL_INTER_NODE_FACTOR,
        .intra_node_knomial_factor = MVP_KNOMIAL_INTRA_NODE_FACTOR,
        .is_two_level_bcast[0] = MVP_BCAST_TUNING_IS_TWO_LEVEL,
        .size_inter_table = 1,
        .inter_leader[0] = {.msg_sz = 1,
                            .zcpy_pipelined_knomial_factor = -1,
                            .bcast_fn = MVP_get_inter_node_bcast_fn()},
        .size_intra_table = 1,
        .intra_node[0] = {.msg_sz = 1,
                          .zcpy_pipelined_knomial_factor = -1,
                          .bcast_fn = MVP_get_intra_node_bcast_fn()},
    };

    MPIR_Memcpy(mvp_bcast_indexed_thresholds_table[0], &tmp_table,
                sizeof(mvp_bcast_indexed_tuning_table));
}

int MVP_set_bcast_tuning_table(int heterogeneity,
                               struct coll_info *colls_arch_hca)
{
    int mpi_errno = MPI_SUCCESS;
    int agg_table_sum = 0;
    int i;
    mvp_bcast_indexed_tuning_table **table_ptrs = NULL;

    if (MVP_BCAST_INTER_NODE_TUNING_ALGO_UNSET !=
            MVP_BCAST_INTER_NODE_TUNING_ALGO ||
        MVP_BCAST_INTRA_NODE_TUNING_ALGO_UNSET !=
            MVP_BCAST_INTRA_NODE_TUNING_ALGO) {
        MVP_set_user_defined_gather_tuning_table();
        goto fn_exit;
    }

#if defined(_MVP_CH4_OVERRIDE_)
    /* setting GEN2 tuning tables - NET must be defined to switch between GEN2,
     * PSM, UCX, OFI, etc */
    MVP_ARCH_TUNING_ENTER
#define NET GEN2
    MVP_ARCH_TUNING_LIST_MLX
#undef NET
#define NET CXI
    MVP_ARCH_TUNING_LIST_CXI
#undef NET
#define NET PSM
    MVP_ARCH_TUNING_LIST_PSM
#undef NET
    MVP_ARCH_TUNING_DEFAULT
#endif

fn_exit:
fn_fail:
    if (table_ptrs) {
        MPL_free(table_ptrs);
    }
    return mpi_errno;
}

void MVP_cleanup_bcast_tuning_table()
{
        MPL_free(mvp_bcast_indexed_thresholds_table[0]);
        MPL_free(mvp_bcast_indexed_table_ppn_conf);
        MPL_free(mvp_size_bcast_indexed_tuning_table);
        if (mvp_bcast_indexed_thresholds_table != NULL) {
            MPL_free(mvp_bcast_indexed_thresholds_table);
        }
}
