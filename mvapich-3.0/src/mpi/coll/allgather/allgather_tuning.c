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

#define COLLECTIVE allgather

#include <regex.h>
#include "allgather_tuning.h"
#include "allgather_arch_tuning.h"

#include "mvp_arch_hca.h"
#include "mvp_common_tuning.h"
#include "mvp_tuning_tables.h"

/* TODO: This should be moved to the pre-header as it is in ch4 */
#ifdef CHANNEL_MRAIL
#include "mvp_arch_hca_detect.h"
#endif

int *mvp_allgather_table_ppn_conf = NULL;
int mvp_allgather_num_ppn_conf = 1;
int *mvp_size_allgather_tuning_table = NULL;
mvp_allgather_tuning_table **mvp_allgather_thresholds_table = NULL;

int *mvp_allgather_indexed_table_ppn_conf = NULL;
int mvp_allgather_indexed_num_ppn_conf = 1;
int *mvp_size_allgather_indexed_tuning_table = NULL;
mvp_allgather_indexed_tuning_table **mvp_allgather_indexed_thresholds_table =
    NULL;

static inline MVP_Allgather_fn_t MVP_get_inter_node_allgather_fn()
{
    switch (MVP_ALLGATHER_COLLECTIVE_ALGORITHM) {
        case MVP_ALLGATHER_COLLECTIVE_ALGORITHM_RD_ALLGATHER_COMM:
            return &MPIR_Allgather_RD_Allgather_Comm_MVP;
        case MVP_ALLGATHER_COLLECTIVE_ALGORITHM_RD:
            return &MPIR_Allgather_RD_MVP;
        case MVP_ALLGATHER_COLLECTIVE_ALGORITHM_BRUCK:
            return &MPIR_Allgather_Bruck_MVP;
        case MVP_ALLGATHER_COLLECTIVE_ALGORITHM_RING:
            return &MPIR_Allgather_Ring_MVP;
        case MVP_ALLGATHER_COLLECTIVE_ALGORITHM_DIRECT:
            return &MPIR_Allgather_Direct_MVP;
        case MVP_ALLGATHER_COLLECTIVE_ALGORITHM_DIRECTSPREAD:
            return &MPIR_Allgather_DirectSpread_MVP;
        case MVP_ALLGATHER_COLLECTIVE_ALGORITHM_GATHER_BCAST:
            return &MPIR_Allgather_gather_bcast_MVP;
        case MVP_ALLGATHER_COLLECTIVE_ALGORITHM_2LVL_NONBLOCKED:
            return &MPIR_2lvl_Allgather_nonblocked_MVP;
        case MVP_ALLGATHER_COLLECTIVE_ALGORITHM_2LVL_RING_NONBLOCKED:
            return &MPIR_2lvl_Allgather_Ring_nonblocked_MVP;
        case MVP_ALLGATHER_COLLECTIVE_ALGORITHM_2LVL_DIRECT:
            return &MPIR_2lvl_Allgather_Direct_MVP;
        case MVP_ALLGATHER_COLLECTIVE_ALGORITHM_2LVL_RING:
            return &MPIR_2lvl_Allgather_Ring_MVP;
        case MVP_ALLGATHER_COLLECTIVE_ALGORITHM_UNSET:
        default:
            PRINT_ERROR("WARNING: Invalid value for "
                        "MVP_ALLGATHER_INTER_NODE_TUNING_ALGO.\n");
            return &MPIR_Allgather_allcomm_auto;
    }
}

static inline void MVP_set_user_defined_allgather_tuning_table()
{
    if (MVP_USE_INDEXED_ALLGATHER_TUNING) {
        mvp_allgather_indexed_num_ppn_conf = 1;
        mvp_allgather_indexed_thresholds_table =
            MPL_malloc(mvp_allgather_indexed_num_ppn_conf *
                           sizeof(mvp_allgather_indexed_tuning_table *),
                       MPL_MEM_COLL);
        mvp_allgather_indexed_table_ppn_conf = MPL_malloc(
            mvp_allgather_indexed_num_ppn_conf * sizeof(int), MPL_MEM_COLL);
        mvp_size_allgather_indexed_tuning_table = MPL_malloc(
            mvp_allgather_indexed_num_ppn_conf * sizeof(int), MPL_MEM_COLL);

        /* -1 indicates user defined algorithm */
        mvp_allgather_indexed_table_ppn_conf[0] = -1;
        mvp_size_allgather_indexed_tuning_table[0] = 1;
        mvp_allgather_indexed_thresholds_table[0] =
            MPL_malloc(mvp_size_allgather_indexed_tuning_table[0] *
                           sizeof(mvp_allgather_indexed_tuning_table),
                       MPL_MEM_COLL);

        mvp_allgather_indexed_tuning_table tmp_table = {
            .numproc = 1,
            .size_inter_table = 1,
            .inter_leader[0] = {.msg_sz = 1,
                                .allgather_fn =
                                    MVP_get_inter_node_allgather_fn()}};

        MPIR_Memcpy(mvp_allgather_indexed_thresholds_table[0], &tmp_table,
                    sizeof(mvp_allgather_indexed_tuning_table));
    } else {
        mvp_allgather_num_ppn_conf = 1;
        mvp_allgather_thresholds_table = MPL_malloc(
            mvp_allgather_num_ppn_conf * sizeof(mvp_allgather_tuning_table *),
            MPL_MEM_COLL);
        mvp_allgather_table_ppn_conf =
            MPL_malloc(mvp_allgather_num_ppn_conf * sizeof(int), MPL_MEM_COLL);
        mvp_size_allgather_tuning_table =
            MPL_malloc(mvp_allgather_num_ppn_conf * sizeof(int), MPL_MEM_COLL);

        /* -1 indicates user defined algorithm */
        mvp_allgather_table_ppn_conf[0] = -1;
        mvp_size_allgather_tuning_table[0] = 1;
        mvp_allgather_thresholds_table[0] =
            MPL_malloc(mvp_size_allgather_tuning_table[0] *
                           sizeof(mvp_allgather_tuning_table),
                       MPL_MEM_COLL);

        mvp_allgather_tuning_table tmp_table = {
            .numproc = 1,
            .size_inter_table = 1,
            .inter_leader[0] = {.min = 0,
                                .max = -1,
                                .allgather_fn =
                                    MVP_get_inter_node_allgather_fn()},
            .two_level[0] = MVP_ALLGATHER_TUNING_IS_TWO_LEVEL};

        MPIR_Memcpy(mvp_allgather_thresholds_table[0], &tmp_table,
                    sizeof(mvp_allgather_tuning_table));
    }
}

int MVP_set_allgather_tuning_table(int heterogeneity,
                                   struct coll_info *colls_arch_hca)
{
    int mpi_errno = MPI_SUCCESS;
    int agg_table_sum = 0;
    int i;
    mvp_allgather_indexed_tuning_table **table_ptrs = NULL;

    /* if MVP_GATHER_INTER_NODE_TUNING_ALGO is not set to AUTO with/without
     * MVP_GATHER_INTRA_NODE_TUNING_ALGO then forgo arch detection tuning
     * and use the user specified algorithms
     */
    if (MVP_ALLGATHER_COLLECTIVE_ALGORITHM_UNSET !=
        MVP_ALLGATHER_COLLECTIVE_ALGORITHM) {
        MVP_set_user_defined_allgather_tuning_table();
        goto fn_exit;
    }

#ifdef _MVP_CH4_OVERRIDE_
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

void MVP_cleanup_allgather_tuning_table()
{
        MPL_free(mvp_allgather_indexed_thresholds_table[0]);
        MPL_free(mvp_allgather_indexed_table_ppn_conf);
        MPL_free(mvp_size_allgather_indexed_tuning_table);
        if (mvp_allgather_indexed_thresholds_table != NULL) {
            MPL_free(mvp_allgather_indexed_thresholds_table);
        }
}
