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

#define COLLECTIVE alltoall

#include <regex.h>
#include "mvp_common_tuning.h"
#include "alltoall_tuning.h"
#include "alltoall_arch_tuning.h"
#include "mvp_tuning_tables.h"

/* array used to tune alltoall */
int *mvp_alltoall_table_ppn_conf = NULL;
int mvp_alltoall_num_ppn_conf = 1;
int *mvp_size_alltoall_tuning_table = NULL;
mvp_alltoall_tuning_table **mvp_alltoall_thresholds_table = NULL;

int *mvp_alltoall_indexed_table_ppn_conf = NULL;
int mvp_alltoall_indexed_num_ppn_conf = 1;
int *mvp_size_alltoall_indexed_tuning_table = NULL;
mvp_alltoall_indexed_tuning_table **mvp_alltoall_indexed_thresholds_table =
    NULL;

static inline MVP_Alltoall_fn_t MVP_get_user_defined_alltoall_fn()
{
    switch (MVP_ALLTOALL_COLLECTIVE_ALGORITHM) {
        default:
            PRINT_ERROR("WARNING: Invalid value for "
                        "MVP_ALLTOALL_COLLECTIVE_ALGORITHM.\n");
        case MVP_ALLTOALL_COLLECTIVE_ALGORITHM_UNSET:
        case MVP_ALLTOALL_COLLECTIVE_ALGORITHM_BRUCK:
            return &MPIR_Alltoall_bruck_MVP;
        case MVP_ALLTOALL_COLLECTIVE_ALGORITHM_RD:
            return &MPIR_Alltoall_RD_MVP;
        case MVP_ALLTOALL_COLLECTIVE_ALGORITHM_SCATTER_DEST:
            return &MPIR_Alltoall_Scatter_dest_MVP;
        case MVP_ALLTOALL_COLLECTIVE_ALGORITHM_PAIRWISE:
            return &MPIR_Alltoall_pairwise_MVP;
        case MVP_ALLTOALL_COLLECTIVE_ALGORITHM_INPLACE:
            return &MPIR_Alltoall_inplace_MVP;
    }
}
static inline void MVP_set_user_defined_alltoall_tuning_table()
{
    mvp_alltoall_indexed_num_ppn_conf = 1;

    mvp_alltoall_indexed_thresholds_table =
        MPL_malloc(mvp_alltoall_indexed_num_ppn_conf *
                       sizeof(mvp_alltoall_indexed_tuning_table *),
                   MPL_MEM_COLL);
    mvp_size_alltoall_indexed_tuning_table = MPL_malloc(
        mvp_alltoall_indexed_num_ppn_conf * sizeof(int), MPL_MEM_COLL);
    mvp_alltoall_indexed_table_ppn_conf = MPL_malloc(
        mvp_alltoall_indexed_num_ppn_conf * sizeof(int), MPL_MEM_COLL);

    mvp_alltoall_indexed_table_ppn_conf[0] = -1;
    mvp_size_alltoall_indexed_tuning_table[0] = 1;
    mvp_alltoall_indexed_thresholds_table[0] =
        MPL_malloc(mvp_size_alltoall_indexed_tuning_table[0] *
                       sizeof(mvp_alltoall_indexed_tuning_table),
                   MPL_MEM_COLL);

    mvp_alltoall_indexed_tuning_table tmp_table = {
        .numproc = 1,
        .size_table = 1,
        .in_place_algo_table[0] = 0,
        .algo_table[0] =
            {
                .msg_sz = 1,
                .alltoall_fn = MVP_get_user_defined_alltoall_fn(),
            },
    };

    MPIR_Memcpy(mvp_alltoall_indexed_thresholds_table[0], &tmp_table,
                sizeof(mvp_alltoall_indexed_tuning_table));
}

int MVP_set_alltoall_tuning_table(int heterogeneity,
                                  struct coll_info *colls_arch_hca)
{
    int mpi_errno = MPI_SUCCESS;
    int agg_table_sum = 0;
    int i;
    mvp_alltoall_indexed_tuning_table **table_ptrs = NULL;

    if (MVP_ALLTOALL_COLLECTIVE_ALGORITHM_UNSET !=
        MVP_ALLTOALL_COLLECTIVE_ALGORITHM) {
        MVP_set_user_defined_alltoall_tuning_table();
        goto fn_exit;
    }

#if _MVP_CH4_OVERRIDE_
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

void MVP_cleanup_alltoall_tuning_table()
{
        MPL_free(mvp_alltoall_indexed_thresholds_table[0]);
        MPL_free(mvp_alltoall_indexed_table_ppn_conf);
        MPL_free(mvp_size_alltoall_indexed_tuning_table);
        if (mvp_alltoall_indexed_thresholds_table != NULL) {
            MPL_free(mvp_alltoall_indexed_thresholds_table);
        }
}

