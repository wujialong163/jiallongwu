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
#define COLLECTIVE scatter

#include <regex.h>
#include "mvp_common_tuning.h"
#include "scatter_tuning.h"
#include "scatter_arch_tuning.h"
#include "mvp_tuning_tables.h"

/* TODO: This should be moved to the pre-header as it is in ch4 */
#ifdef CHANNEL_MRAIL
#include "mvp_arch_hca_detect.h"
#endif

int *mvp_scatter_table_ppn_conf = NULL;
int mvp_scatter_num_ppn_conf = 1;
int *mvp_size_scatter_tuning_table = NULL;
mvp_scatter_tuning_table **mvp_scatter_thresholds_table = NULL;

int *mvp_scatter_indexed_table_ppn_conf = NULL;
int mvp_scatter_indexed_num_ppn_conf = 1;
int *mvp_size_scatter_indexed_tuning_table = NULL;
mvp_scatter_indexed_tuning_table **mvp_scatter_indexed_thresholds_table = NULL;

static inline MVP_Scatter_fn_t MVP_get_inter_node_scatter_fn()
{
    switch (MVP_SCATTER_INTER_NODE_TUNING_ALGO) {
        default:
            PRINT_ERROR("WARNING: Invalid value for "
                        "MVP_SCATTER_INTER_NODE_TUNING_ALGO.\n");
        case MVP_SCATTER_INTER_NODE_TUNING_ALGO_UNSET:
        case MVP_SCATTER_INTER_NODE_TUNING_ALGO_BINOMIAL:
            return &MPIR_Scatter_MVP_Binomial;
        case MVP_SCATTER_INTER_NODE_TUNING_ALGO_DIRECT:
            return &MPIR_Scatter_MVP_Direct;
        case MVP_SCATTER_INTER_NODE_TUNING_ALGO_2LVL_BINOMIAL:
            return &MPIR_Scatter_MVP_two_level_Binomial;
        case MVP_SCATTER_INTER_NODE_TUNING_ALGO_2LVL_DIRECT:
            return &MPIR_Scatter_MVP_two_level_Direct;
#if defined(_MCST_SUPPORT_)
        case MVP_SCATTER_INTER_NODE_TUNING_ALGO_MCAST:
            return &MPIR_Scatter_mcst_wrap_MVP;
#endif /* #if defined(_MCST_SUPPORT_) */
    }
}
static inline MVP_Scatter_fn_t MVP_get_intra_node_scatter_fn()
{
    switch (MVP_SCATTER_INTRA_NODE_TUNING_ALGO) {
        default:
            PRINT_ERROR("WARNING: Invalid value for "
                        "MVP_SCATTER_INTRA_NODE_TUNING_ALGO.\n");
        case MVP_SCATTER_INTRA_NODE_TUNING_ALGO_UNSET:
        case MVP_SCATTER_INTRA_NODE_TUNING_ALGO_DIRECT:
            return &MPIR_Scatter_MVP_Direct;
        case MVP_SCATTER_INTRA_NODE_TUNING_ALGO_BINOMIAL:
            return &MPIR_Scatter_MVP_Binomial;
    }
}
static inline void MVP_set_user_defined_scatter_tuning_table()
{
    mvp_scatter_indexed_num_ppn_conf = 1;
    mvp_scatter_indexed_thresholds_table =
        MPL_malloc(mvp_scatter_indexed_num_ppn_conf *
                       sizeof(mvp_scatter_indexed_tuning_table *),
                   MPL_MEM_COLL);
    mvp_size_scatter_indexed_tuning_table = MPL_malloc(
        mvp_scatter_indexed_num_ppn_conf * sizeof(int), MPL_MEM_COLL);
    mvp_scatter_indexed_table_ppn_conf = MPL_malloc(
        mvp_scatter_indexed_num_ppn_conf * sizeof(int), MPL_MEM_COLL);

    mvp_scatter_indexed_table_ppn_conf[0] = -1;
    mvp_size_scatter_indexed_tuning_table[0] = 1;
    mvp_scatter_indexed_thresholds_table[0] =
        MPL_malloc(mvp_size_scatter_indexed_tuning_table[0] *
                       sizeof(mvp_scatter_indexed_tuning_table),
                   MPL_MEM_COLL);

    mvp_scatter_indexed_tuning_table tmp_table = {
        .numproc = 1,
        .size_inter_table = 1,
        .size_intra_table = 1,
        .inter_leader[0] =
            {
                .msg_sz = 1,
                .scatter_fn = MVP_get_inter_node_scatter_fn(),
            },
        .intra_node[0] =
            {
                .msg_sz = 1,
                .scatter_fn = MVP_get_intra_node_scatter_fn(),
            },
    };

    MPIR_Memcpy(mvp_scatter_indexed_thresholds_table[0], &tmp_table,
                sizeof(mvp_scatter_indexed_tuning_table));
}

int MVP_set_scatter_tuning_table(int heterogeneity,
                                 struct coll_info *colls_arch_hca)
{
    int mpi_errno = MPI_SUCCESS;
    int agg_table_sum = 0;
    int i;
    mvp_scatter_indexed_tuning_table **table_ptrs = NULL;

    if (MVP_SCATTER_INTER_NODE_TUNING_ALGO_UNSET !=
            MVP_SCATTER_INTER_NODE_TUNING_ALGO ||
        MVP_SCATTER_INTRA_NODE_TUNING_ALGO_UNSET !=
            MVP_SCATTER_INTRA_NODE_TUNING_ALGO) {
        MVP_set_user_defined_scatter_tuning_table();
        goto fn_exit;
    }

#if defined(_MVP_CH4_OVERRIDE_)
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

void MVP_cleanup_scatter_tuning_table()
{
        MPL_free(mvp_scatter_indexed_thresholds_table[0]);
        MPL_free(mvp_scatter_indexed_table_ppn_conf);
        MPL_free(mvp_size_scatter_indexed_tuning_table);
        if (mvp_scatter_indexed_thresholds_table != NULL) {
            MPL_free(mvp_scatter_indexed_thresholds_table);
        }
}
