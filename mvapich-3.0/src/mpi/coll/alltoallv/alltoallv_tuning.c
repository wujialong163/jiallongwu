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

#include <regex.h>
#include "mvp_common_tuning.h"
#include "alltoallv_tuning.h"
#include "alltoallv_arch_tuning.h"

/* array used to tune alltoallv */

int *mvp_alltoallv_table_ppn_conf = NULL;
int mvp_alltoallv_num_ppn_conf = 1;
int *mvp_size_alltoallv_tuning_table = NULL;
mvp_alltoallv_tuning_table **mvp_alltoallv_thresholds_table = NULL;

int *mvp_alltoallv_indexed_table_ppn_conf = NULL;
int mvp_alltoallv_indexed_num_ppn_conf = 1;
int *mvp_size_alltoallv_indexed_tuning_table = NULL;
mvp_alltoallv_indexed_tuning_table **mvp_alltoallv_indexed_thresholds_table =
    NULL;

static inline MVP_Alltoallv_fn_t MVP_get_user_defined_alltoallv_fn()
{
    switch (MVP_ALLTOALLV_COLLECTIVE_ALGORITHM) {
        default:
            PRINT_ERROR("WARNING: Invalid value for "
                        "MVP_ALLTOALLV_COLLECTIVE_ALGORITHM.\n");
        case MVP_ALLTOALLV_COLLECTIVE_ALGORITHM_UNSET:
        case MVP_ALLTOALLV_COLLECTIVE_ALGORITHM_SCATTER:
            return &MPIR_Alltoallv_intra_scatter_MVP;
        case MVP_ALLTOALLV_COLLECTIVE_ALGORITHM_INTRA:
            return &MPIR_Alltoallv_intra_MVP;
    }
}
static inline void MVP_set_user_defined_alltoallv_tuning_table()
{
    mvp_alltoallv_indexed_num_ppn_conf = 1;

    mvp_alltoallv_indexed_thresholds_table =
        MPL_malloc(mvp_alltoallv_indexed_num_ppn_conf *
                       sizeof(mvp_alltoallv_indexed_tuning_table *),
                   MPL_MEM_COLL);
    mvp_size_alltoallv_indexed_tuning_table = MPL_malloc(
        mvp_alltoallv_indexed_num_ppn_conf * sizeof(int), MPL_MEM_COLL);
    mvp_alltoallv_indexed_table_ppn_conf = MPL_malloc(
        mvp_alltoallv_indexed_num_ppn_conf * sizeof(int), MPL_MEM_COLL);

    mvp_alltoallv_indexed_table_ppn_conf[0] = -1;
    mvp_size_alltoallv_indexed_tuning_table[0] = 1;
    mvp_alltoallv_indexed_thresholds_table[0] =
        MPL_malloc(mvp_size_alltoallv_indexed_tuning_table[0] *
                       sizeof(mvp_alltoallv_indexed_tuning_table),
                   MPL_MEM_COLL);

    mvp_alltoallv_indexed_tuning_table tmp_table = {
        .numproc = 1,
        .size_table = 1,
        .in_place_algo_table[0] = 0,
        .algo_table[0] =
            {
                .msg_sz = 1,
                .alltoallv_fn = MVP_get_user_defined_alltoallv_fn(),
            },
    };

    MPIR_Memcpy(mvp_alltoallv_indexed_thresholds_table[0], &tmp_table,
                sizeof(mvp_alltoallv_indexed_tuning_table));
}

int MVP_set_alltoallv_tuning_table(int heterogeneity,
                                   struct coll_info *colls_arch_hca)
{
    int agg_table_sum = 0;
    int i;

    if (MVP_ALLTOALLV_COLLECTIVE_ALGORITHM_UNSET !=
        MVP_ALLTOALLV_COLLECTIVE_ALGORITHM) {
        MVP_set_user_defined_alltoallv_tuning_table();
        return 0;
    }

    /* Sample table */
        mvp_alltoallv_indexed_tuning_table **table_ptrs = NULL;
        if (MVP_IS_ARCH_HCA_TYPE(MVP_get_arch_hca_type(),
                                 MVP_ARCH_INTEL_XEON_E5_2680_V4_2S_28,
                                 MVP_HCA_MLX_CX_EDR) &&
            !heterogeneity) {
            MVP_COLL_TUNING_START_TABLE(alltoallv, 1)
            MVP_COLL_TUNING_ADD_CONF(
                alltoallv, 4, 2,
                test_table) // 2 node 4 and 8 ppn test table for RI2
            MVP_COLL_TUNING_FINISH_TABLE(alltoallv)
        } else {
            MVP_COLL_TUNING_START_TABLE(alltoallv, 1)
            MVP_COLL_TUNING_ADD_CONF(
                alltoallv, 4, 2,
                test_table) // 2 node 4 and 8 ppn test table for RI2
            MVP_COLL_TUNING_FINISH_TABLE(alltoallv)
        }

    return 0;
}

void MVP_cleanup_alltoallv_tuning_table()
{
        MPL_free(mvp_alltoallv_indexed_thresholds_table[0]);
        MPL_free(mvp_alltoallv_indexed_table_ppn_conf);
        MPL_free(mvp_size_alltoallv_indexed_tuning_table);
        if (mvp_alltoallv_indexed_thresholds_table != NULL) {
            MPL_free(mvp_alltoallv_indexed_thresholds_table);
        }
}

