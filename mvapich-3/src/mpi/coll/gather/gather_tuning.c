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

#define COLLECTIVE gather

#include <regex.h>
#include "mvp_common_tuning.h"
#include "gather_tuning.h"
#include "gather_arch_tuning.h"
#include "mvp_tuning_tables.h"

/* array used to tune gather */
int mvp_size_gather_tuning_table = 7;
mvp_gather_tuning_table *mvp_gather_thresholds_table = NULL;

int *mvp_gather_indexed_table_ppn_conf = NULL;
int mvp_gather_indexed_num_ppn_conf = 1;
int *mvp_size_gather_indexed_tuning_table = NULL;
mvp_gather_indexed_tuning_table **mvp_gather_indexed_thresholds_table = NULL;

static inline MVP_Gather_fn_t MVP_get_inter_node_gather_fn()
{
    switch (MVP_GATHER_INTER_NODE_TUNING_ALGO) {
        case MVP_GATHER_INTER_NODE_TUNING_ALGO_BINOMIAL:
            return &MPIR_Gather_intra_binomial;
        case MVP_GATHER_INTER_NODE_TUNING_ALGO_2LVL_DIRECT:
            return &MPIR_Gather_MVP_two_level_Direct;
        default:
            PRINT_ERROR("WARNING: Invalid value for "
                        "MVP_GATHER_INTER_NODE_TUNING_ALGO.\n");
        case MVP_GATHER_INTER_NODE_TUNING_ALGO_UNSET:
        case MVP_GATHER_INTER_NODE_TUNING_ALGO_DIRECT:
            return &MPIR_Gather_MVP_Direct;
    }
}
static inline MVP_Gather_fn_t MVP_get_intra_node_gather_fn()
{
    switch (MVP_GATHER_INTRA_NODE_TUNING_ALGO) {
        case MVP_GATHER_INTRA_NODE_TUNING_ALGO_BINOMIAL:
            return &MPIR_Gather_intra_binomial;
        default:
            PRINT_ERROR("WARNING: Invalid value for "
                        "MVP_GATHER_INTRA_NODE_TUNING_ALGO.\n");
        case MVP_GATHER_INTRA_NODE_TUNING_ALGO_UNSET:
        case MVP_GATHER_INTRA_NODE_TUNING_ALGO_DIRECT:
            return &MPIR_Gather_MVP_Direct;
    }
}

static inline void MVP_set_user_defined_gather_tuning_table()
{
    /*
     * mvp_gather_indexed_thresholds_table: is a jagged 2D array of tuning
     * tables in contigous memory mvp_gather_indexed_num_ppn_conf: number of ppn
     * configurations, in other words, the number of rows in the 2D array
     * mvp_size_gather_indexed_tuning_table: this is an index table to keep
     * track of the number of seperate tuning tables (size) in each row of the
     * jagged array mvp_gather_indexed_table_ppn_conf: this is an index table
     * that tells you for what ppn does the corresponding row in
     * mvp_gather_indexed_thresholds_table account for
     */

    mvp_gather_indexed_num_ppn_conf = 1;

    /* Allocate index table to specify how long each row in
     * mvp_gather_indexed_thresholds_table is */
    mvp_size_gather_indexed_tuning_table =
        MPL_malloc(mvp_gather_indexed_num_ppn_conf * sizeof(int), MPL_MEM_COLL);
    /* Allocate index table to keep track what index correponds to what ppn
     * configuration */
    mvp_gather_indexed_table_ppn_conf =
        MPL_malloc(mvp_gather_indexed_num_ppn_conf * sizeof(int), MPL_MEM_COLL);

    /* Only one tuning table will be defined */
    mvp_size_gather_indexed_tuning_table[0] = 1;
    /* -1 indicates user defined algorithm */
    mvp_gather_indexed_table_ppn_conf[0] = -1;

    /* Allocate space for tuning table ptrs */
    mvp_gather_indexed_thresholds_table =
        MPL_malloc(mvp_gather_indexed_num_ppn_conf *
                       sizeof(mvp_gather_indexed_tuning_table *),
                   MPL_MEM_COLL);
    /* Allocate space for tuning tables */
    mvp_gather_indexed_thresholds_table[0] =
        MPL_malloc(mvp_size_gather_indexed_tuning_table[0] *
                       sizeof(mvp_gather_indexed_tuning_table),
                   MPL_MEM_COLL);

    mvp_gather_indexed_tuning_table tmp_table = {
        .numproc = 1,
        .size_inter_table = 1,
        .inter_leader[0] = {.msg_sz = 1,
                            .gather_fn = MVP_get_inter_node_gather_fn()},
        .size_intra_table = 1,
        .intra_node[0] = {.msg_sz = 1,
                          .gather_fn = MVP_get_intra_node_gather_fn()}};

    MPIR_Memcpy(mvp_gather_indexed_thresholds_table[0], &tmp_table,
                sizeof(mvp_gather_indexed_tuning_table));
}

int MVP_set_gather_tuning_table(int heterogeneity,
                                struct coll_info *colls_arch_hca)
{
    int mpi_errno = MPI_SUCCESS;
    int agg_table_sum = 0;
    int i;
    mvp_gather_indexed_tuning_table **table_ptrs = NULL;

    if (MVP_GATHER_INTER_NODE_TUNING_ALGO_UNSET !=
            MVP_GATHER_INTER_NODE_TUNING_ALGO ||
        MVP_GATHER_INTRA_NODE_TUNING_ALGO_UNSET !=
            MVP_GATHER_INTRA_NODE_TUNING_ALGO) {
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

void MVP_cleanup_gather_tuning_table()
{
        MPL_free(mvp_gather_indexed_thresholds_table[0]);
        MPL_free(mvp_gather_indexed_table_ppn_conf);
        MPL_free(mvp_size_gather_indexed_tuning_table);
        if (mvp_gather_indexed_thresholds_table != NULL) {
            MPL_free(mvp_gather_indexed_thresholds_table);
        }

}

