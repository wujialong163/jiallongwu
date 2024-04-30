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

#define COLLECTIVE allreduce

#include <regex.h>
#include "mvp_common_tuning.h"
#include "allreduce_tuning.h"
#include "allreduce_arch_tuning.h"
#include "mvp_tuning_tables.h"

int mvp_size_allreduce_tuning_table = 0;
mvp_allreduce_tuning_table *mvp_allreduce_thresholds_table = NULL;

int *mvp_allreduce_indexed_table_ppn_conf = NULL;
int mvp_allreduce_indexed_num_ppn_conf = 1;
int *mvp_size_allreduce_indexed_tuning_table = NULL;
mvp_allreduce_indexed_tuning_table **mvp_allreduce_indexed_thresholds_table =
    NULL;

static inline MVP_Allreduce_fn_t MVP_get_inter_node_allreduce_fn()
{
    switch (MVP_ALLREDUCE_INTER_NODE_TUNING_ALGO) {
        default:
            PRINT_ERROR("WARNING: Invalid value for "
                        "MVP_ALLREDUCE_INTER_NODE_TUNING_ALGO.\n");
        case MVP_ALLREDUCE_INTER_NODE_TUNING_ALGO_UNSET:
        case MVP_ALLREDUCE_INTER_NODE_TUNING_ALGO_P2P_RD:
            return &MPIR_Allreduce_pt2pt_rd_MVP;
        case MVP_ALLREDUCE_INTER_NODE_TUNING_ALGO_P2P_RSA:
            return &MPIR_Allreduce_pt2pt_rs_MVP;
#if defined(_MCST_SUPPORT_)
        case MVP_ALLREDUCE_INTER_NODE_TUNING_ALGO_MCAST_2LVL:
            return &MPIR_Allreduce_mcst_reduce_two_level_helper_MVP;
        case MVP_ALLREDUCE_INTER_NODE_TUNING_ALGO_MCAST_RSA:
            return &MPIR_Allreduce_mcst_reduce_redscat_gather_MVP;
#endif /* #if defined(_MCST_SUPPORT_) */
        case MVP_ALLREDUCE_INTER_NODE_TUNING_ALGO_RSA:
            return &MPIR_Allreduce_pt2pt_reduce_scatter_allgather_MVP;
        case MVP_ALLREDUCE_INTER_NODE_TUNING_ALGO_RING:
            return &MPIR_Allreduce_pt2pt_ring_wrapper_MVP;
        case MVP_ALLREDUCE_INTER_NODE_TUNING_ALGO_SOCK_AWARE:
            return &MPIR_Allreduce_socket_aware_two_level_MVP;
    }
}
static inline MVP_Allreduce_fn_t MVP_get_intra_node_allreduce_fn()
{
    switch (MVP_ALLREDUCE_INTRA_NODE_TUNING_ALGO) {
        default:
            PRINT_ERROR("WARNING: Invalid value for "
                        "MVP_ALLREDUCE_INTRA_NODE_TUNING_ALGO.\n");
        case MVP_ALLREDUCE_INTRA_NODE_TUNING_ALGO_UNSET:
        case MVP_ALLREDUCE_INTRA_NODE_TUNING_ALGO_P2P:
            return &MPIR_Allreduce_reduce_p2p_MVP;
        case MVP_ALLREDUCE_INTRA_NODE_TUNING_ALGO_SHMEM:
            return &MPIR_Allreduce_reduce_shmem_MVP;
        case MVP_ALLREDUCE_INTRA_NODE_TUNING_ALGO_P2P_RD:
            return &MPIR_Allreduce_pt2pt_rd_MVP;
        case MVP_ALLREDUCE_INTRA_NODE_TUNING_ALGO_P2P_RSA:
            return &MPIR_Allreduce_pt2pt_rs_MVP;
    }
}
static inline void MVP_set_user_defined_allreduce_tuning_table()
{
    mvp_allreduce_indexed_num_ppn_conf = 1;
    mvp_size_allreduce_indexed_tuning_table = MPL_malloc(
        mvp_allreduce_indexed_num_ppn_conf * sizeof(int), MPL_MEM_COLL);
    mvp_allreduce_indexed_table_ppn_conf = MPL_malloc(
        mvp_allreduce_indexed_num_ppn_conf * sizeof(int), MPL_MEM_COLL);
    mvp_allreduce_indexed_thresholds_table =
        MPL_malloc(mvp_allreduce_indexed_num_ppn_conf *
                       sizeof(mvp_allreduce_indexed_tuning_table *),
                   MPL_MEM_COLL);

    mvp_allreduce_indexed_table_ppn_conf[0] = -1;
    mvp_size_allreduce_indexed_tuning_table[0] = 1;
    mvp_allreduce_indexed_thresholds_table[0] =
        MPL_malloc(mvp_size_allreduce_indexed_tuning_table[0] *
                       sizeof(mvp_allreduce_indexed_tuning_table),
                   MPL_MEM_COLL);

    mvp_allreduce_indexed_tuning_table tmp_table = {
        .numproc = 1,
        .size_inter_table = 1,
        .size_intra_table = 1,
        .is_two_level_allreduce[0] = MVP_ALLREDUCE_TUNING_IS_TWO_LEVEL,
        .inter_leader[0] =
            {
                .msg_sz = 1,
                .allreduce_fn = MVP_get_inter_node_allreduce_fn(),
            },
        .intra_node[0] =
            {
                .msg_sz = 1,
                .allreduce_fn = MVP_get_intra_node_allreduce_fn(),
            },
    };

    MPIR_Memcpy(mvp_allreduce_indexed_thresholds_table[0], &tmp_table,
                sizeof(mvp_allreduce_indexed_tuning_table));
}

int MVP_set_allreduce_tuning_table(int heterogeneity,
                                   struct coll_info *colls_arch_hca)
{
    int mpi_errno = MPI_SUCCESS;
    int agg_table_sum = 0;
    int i;
    mvp_allreduce_indexed_tuning_table **table_ptrs = NULL;

    if (MVP_ALLREDUCE_INTER_NODE_TUNING_ALGO_UNSET !=
            MVP_ALLREDUCE_INTER_NODE_TUNING_ALGO ||
        MVP_ALLREDUCE_INTRA_NODE_TUNING_ALGO_UNSET !=
            MVP_ALLREDUCE_INTRA_NODE_TUNING_ALGO) {
        MVP_set_user_defined_allreduce_tuning_table();
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

void MVP_cleanup_allreduce_tuning_table()
{
        MPL_free(mvp_allreduce_indexed_thresholds_table[0]);
        MPL_free(mvp_allreduce_indexed_table_ppn_conf);
        MPL_free(mvp_size_allreduce_indexed_tuning_table);
        if (mvp_allreduce_indexed_thresholds_table != NULL) {
            MPL_free(mvp_allreduce_indexed_thresholds_table);
        }
}
