/* Copyright (c) 2001-2021, The Ohio State University. All rights
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

#define COLLECTIVE reduce

#include <regex.h>
#include "mvp_common_tuning.h"
#include "reduce_tuning.h"
#include "reduce_arch_tuning.h"
#include "reduce_scatter_tuning.h"
#include "mvp_tuning_tables.h"

int mvp_size_reduce_tuning_table = 0;
mvp_reduce_tuning_table *mvp_reduce_thresholds_table = NULL;

int *mvp_reduce_indexed_table_ppn_conf = NULL;
int mvp_reduce_indexed_num_ppn_conf = 1;
int *mvp_size_reduce_indexed_tuning_table = NULL;
mvp_reduce_indexed_tuning_table **mvp_reduce_indexed_thresholds_table = NULL;

static inline MVP_Reduce_fn_t MVP_get_inter_node_reduce_fn()
{
    int i, j;
    switch (MVP_REDUCE_INTER_NODE_TUNING_ALGO) {
        default:
            PRINT_ERROR("WARNING: Invalid value for "
                        "MVP_REDUCE_INTER_NODE_TUNING_ALGO.\n");
        case MVP_REDUCE_INTER_NODE_TUNING_ALGO_UNSET:
        case MVP_REDUCE_INTER_NODE_TUNING_ALGO_KNOMIAL:
            return &MPIR_Reduce_inter_knomial_wrapper_MVP;
        case MVP_REDUCE_INTER_NODE_TUNING_ALGO_BINOMIAL:
            return &MPIR_Reduce_binomial_MVP;
        case MVP_REDUCE_INTER_NODE_TUNING_ALGO_RED_SCAT_GATHER:
            return &MPIR_Reduce_redscat_gather_MVP;
        case MVP_REDUCE_INTER_NODE_TUNING_ALGO_ALLREDUCE:
            /* Do not allow reduce_scatter implementations that use reduce.
            This leads to an infinite recursion of
            reduce->allreduce->reduce_scatter->reduce->allreduce */
            for (i = 0; i < mvp_size_red_scat_tuning_table; i++) {
                mvp_red_scat_tuning_table *tbl =
                    &mvp_red_scat_thresholds_table[i];
                int size = tbl->size_inter_table;
                for (j = 0; j < size; j++) {
                    if (tbl->inter_leader[j].MVP_pt_Red_scat_function ==
                        &MPIR_Reduce_Scatter_Basic_MVP) {
                        PRINT_DEBUG(
                            DEBUG_SHM_verbose,
                            "Invalid algorithm for reduce_scatter "
                            "when forcing MVP_REDUCE_INTER_NODE_TUNING_ALGO to "
                            "ALLREDUCE.\n"
                            "Setting to "
                            "MPIR_Reduce_scatter_Rec_Halving_MVP\n");
                        tbl->inter_leader[j].MVP_pt_Red_scat_function =
                            &MPIR_Reduce_scatter_Rec_Halving_MVP;
                    }
                }
            }
            return &MPIR_Reduce_allreduce_MVP;
    }
}
static inline MVP_Reduce_fn_t MVP_get_intra_node_reduce_fn()
{
    switch (MVP_REDUCE_INTRA_NODE_TUNING_ALGO) {
        default:
            PRINT_ERROR("WARNING: Invalid value for "
                        "MVP_REDUCE_INTRA_NODE_TUNING_ALGO.\n");
        case MVP_REDUCE_INTRA_NODE_TUNING_ALGO_UNSET:
        case MVP_REDUCE_INTRA_NODE_TUNING_ALGO_BINOMIAL:
            return &MPIR_Reduce_binomial_MVP;
        case MVP_REDUCE_INTRA_NODE_TUNING_ALGO_KNOMIAL:
            return &MPIR_Reduce_intra_knomial_wrapper_MVP;
        case MVP_REDUCE_INTRA_NODE_TUNING_ALGO_SHMEM:
            return &MPIR_Reduce_shmem_MVP;
    }
}
static inline void MVP_set_user_defined_reduce_tuning_table()
{
    mvp_reduce_indexed_num_ppn_conf = 1;
    mvp_reduce_indexed_thresholds_table =
        MPL_malloc(mvp_reduce_indexed_num_ppn_conf *
                       sizeof(mvp_reduce_indexed_tuning_table *),
                   MPL_MEM_COLL);
    mvp_reduce_indexed_table_ppn_conf =
        MPL_malloc(mvp_reduce_indexed_num_ppn_conf * sizeof(int), MPL_MEM_COLL);
    mvp_size_reduce_indexed_tuning_table =
        MPL_malloc(mvp_reduce_indexed_num_ppn_conf * sizeof(int), MPL_MEM_COLL);

    /* -1 indicates user defined algorithm */
    mvp_reduce_indexed_table_ppn_conf[0] = -1;
    mvp_size_reduce_indexed_tuning_table[0] = 1;
    mvp_reduce_indexed_thresholds_table[0] =
        MPL_malloc(mvp_size_reduce_indexed_tuning_table[0] *
                       sizeof(mvp_reduce_indexed_tuning_table),
                   MPL_MEM_COLL);

    mvp_reduce_indexed_tuning_table tmp_table = {
        .numproc = 1,
        .inter_k_degree = 4,
        .intra_k_degree = 4,
        .size_inter_table = 1,
        .size_intra_table = 1,
        .is_two_level_reduce = MVP_REDUCE_TUNING_IS_TWO_LEVEL,
        .inter_leader[0] =
            {
                .msg_sz = 1,
                .reduce_fn = MVP_get_inter_node_reduce_fn(),
            },
        .intra_node[0] =
            {
                .msg_sz = 1,
                .reduce_fn = MVP_get_intra_node_reduce_fn(),
            },
    };

    MPIR_Memcpy(mvp_reduce_indexed_thresholds_table[0], &tmp_table,
                sizeof(mvp_reduce_indexed_tuning_table));
}

int MVP_set_reduce_tuning_table(int heterogeneity,
                                struct coll_info *colls_arch_hca)
{
    int mpi_errno = MPI_SUCCESS;
    int agg_table_sum = 0;
    int i;
    mvp_reduce_indexed_tuning_table **table_ptrs = NULL;

    if (MVP_REDUCE_INTER_NODE_TUNING_ALGO_UNSET !=
            MVP_REDUCE_INTER_NODE_TUNING_ALGO ||
        MVP_REDUCE_INTRA_NODE_TUNING_ALGO_UNSET !=
            MVP_REDUCE_INTRA_NODE_TUNING_ALGO) {
        MVP_set_user_defined_reduce_tuning_table();
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

void MVP_cleanup_reduce_tuning_table()
{
    MPL_free(mvp_reduce_indexed_thresholds_table[0]);
    MPL_free(mvp_reduce_indexed_table_ppn_conf);
    MPL_free(mvp_size_reduce_indexed_tuning_table);
    if (mvp_reduce_indexed_thresholds_table != NULL) {
        MPL_free(mvp_reduce_indexed_thresholds_table);
    }
}
