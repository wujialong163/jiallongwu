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
#include "allgatherv_tuning.h"

int mvp_size_allgatherv_tuning_table = 0;
mvp_allgatherv_tuning_table *mvp_allgatherv_thresholds_table = NULL;

static inline MVP_Allgatherv_fn_t MVP_get_user_defined_allgatherv_fn()
{
    switch (MVP_ALLGATHERV_COLLECTIVE_ALGORITHM) {
        default:
            PRINT_ERROR("WARNING: Invalid value for "
                        "MVP_ALLGATHERV_COLLECTIVE_ALGORITHM.\n");
        case MVP_ALLGATHERV_COLLECTIVE_ALGORITHM_UNSET:
        case MVP_ALLGATHERV_COLLECTIVE_ALGORITHM_BRUCK:
            return &MPIR_Allgatherv_Bruck_MVP;
        case MVP_ALLGATHERV_COLLECTIVE_ALGORITHM_RD:
            return &MPIR_Allgatherv_Rec_Doubling_MVP;
        case MVP_ALLGATHERV_COLLECTIVE_ALGORITHM_RING:
            return &MPIR_Allgatherv_Ring_MVP;
        case MVP_ALLGATHERV_COLLECTIVE_ALGORITHM_RING_CYCLIC:
            return &MPIR_Allgatherv_Ring_Cyclic_MVP;
    }
}
static inline void MVP_set_user_defined_allgatherv_tuning_table()
{
    mvp_size_allgatherv_tuning_table = 1;
    mvp_allgatherv_thresholds_table = MPL_malloc(
        mvp_size_allgatherv_tuning_table * sizeof(mvp_allgatherv_tuning_table),
        MPL_MEM_COLL);

    mvp_allgatherv_tuning_table tmp_table = {
        .numproc = 1,
        .size_inter_table = 1,
        .inter_leader[0] = {
            .min = 0,
            .max = -1,
            .allgatherv_fn = MVP_get_user_defined_allgatherv_fn(),
        }};

    MPIR_Memcpy(mvp_allgatherv_thresholds_table, &tmp_table,
                sizeof(mvp_allgatherv_tuning_table));
}

int MVP_set_allgatherv_tuning_table(int heterogeneity,
                                    struct coll_info *colls_arch_hca)
{
    if (MVP_ALLGATHERV_COLLECTIVE_ALGORITHM_UNSET !=
        MVP_ALLGATHERV_COLLECTIVE_ALGORITHM) {
        MVP_set_user_defined_allgatherv_tuning_table();
        return 0;
    }

#ifndef _MVP_CH4_OVERRIDE_
#if defined(CHANNEL_NEMESIS_IB)
    if (MVP_IS_ARCH_HCA_TYPE(MVP_get_arch_hca_type(),
                             MVP_ARCH_INTEL_XEON_X5650_12,
                             MVP_HCA_MLX_CX_QDR) &&
        !heterogeneity) {
        mvp_size_allgatherv_tuning_table = 6;
        mvp_allgatherv_thresholds_table =
            MPL_malloc(mvp_size_allgatherv_tuning_table *
                           sizeof(mvp_allgatherv_tuning_table),
                       MPL_MEM_COLL);
        mvp_allgatherv_tuning_table mvp_tmp_allgatherv_thresholds_table[] = {
            {
                12,
                2,
                {
                    {0, 512, &MPIR_Allgatherv_Rec_Doubling_MVP},
                    {512, -1, &MPIR_Allgatherv_Ring_MVP},
                },
            },
            {
                24,
                2,
                {
                    {0, 512, &MPIR_Allgatherv_Rec_Doubling_MVP},
                    {512, -1, &MPIR_Allgatherv_Ring_MVP},
                },
            },
            {
                48,
                2,
                {
                    {0, 256, &MPIR_Allgatherv_Rec_Doubling_MVP},
                    {256, -1, &MPIR_Allgatherv_Ring_MVP},
                },
            },
            {
                96,
                2,
                {
                    {0, 256, &MPIR_Allgatherv_Rec_Doubling_MVP},
                    {256, -1, &MPIR_Allgatherv_Ring_MVP},
                },
            },
            {
                192,
                2,
                {
                    {0, 256, &MPIR_Allgatherv_Rec_Doubling_MVP},
                    {256, -1, &MPIR_Allgatherv_Ring_MVP},
                },
            },
            {
                384,
                2,
                {
                    {0, 256, &MPIR_Allgatherv_Rec_Doubling_MVP},
                    {256, -1, &MPIR_Allgatherv_Ring_MVP},
                },
            },
        };
        MPIR_Memcpy(mvp_allgatherv_thresholds_table,
                    mvp_tmp_allgatherv_thresholds_table,
                    mvp_size_allgatherv_tuning_table *
                        sizeof(mvp_allgatherv_tuning_table));
    } else if (MVP_IS_ARCH_HCA_TYPE(MVP_get_arch_hca_type(),
                                    MVP_ARCH_INTEL_XEON_E5_2680_16,
                                    MVP_HCA_MLX_CX_FDR) &&
               !heterogeneity) {
        mvp_size_allgatherv_tuning_table = 6;
        mvp_allgatherv_thresholds_table =
            MPL_malloc(mvp_size_allgatherv_tuning_table *
                           sizeof(mvp_allgatherv_tuning_table),
                       MPL_MEM_COLL);
        mvp_allgatherv_tuning_table mvp_tmp_allgatherv_thresholds_table[] = {
            {
                16,
                2,
                {
                    {0, 512, &MPIR_Allgatherv_Rec_Doubling_MVP},
                    {512, -1, &MPIR_Allgatherv_Ring_MVP},
                },
            },
            {
                32,
                2,
                {
                    {0, 512, &MPIR_Allgatherv_Rec_Doubling_MVP},
                    {512, -1, &MPIR_Allgatherv_Ring_MVP},
                },
            },
            {
                64,
                2,
                {
                    {0, 256, &MPIR_Allgatherv_Rec_Doubling_MVP},
                    {256, -1, &MPIR_Allgatherv_Ring_MVP},
                },
            },
            {
                128,
                2,
                {
                    {0, 256, &MPIR_Allgatherv_Rec_Doubling_MVP},
                    {256, -1, &MPIR_Allgatherv_Ring_MVP},
                },
            },
            {
                256,
                2,
                {
                    {0, 256, &MPIR_Allgatherv_Rec_Doubling_MVP},
                    {256, -1, &MPIR_Allgatherv_Ring_MVP},
                },
            },
            {
                512,
                2,
                {
                    {0, 256, &MPIR_Allgatherv_Rec_Doubling_MVP},
                    {256, -1, &MPIR_Allgatherv_Ring_MVP},
                },
            },

        };
        MPIR_Memcpy(mvp_allgatherv_thresholds_table,
                    mvp_tmp_allgatherv_thresholds_table,
                    mvp_size_allgatherv_tuning_table *
                        sizeof(mvp_allgatherv_tuning_table));
    } else if (MVP_IS_ARCH_HCA_TYPE(MVP_get_arch_hca_type(),
                                    MVP_ARCH_AMD_OPTERON_6136_32,
                                    MVP_HCA_MLX_CX_QDR) &&
               !heterogeneity) {
        mvp_size_allgatherv_tuning_table = 6;
        mvp_allgatherv_thresholds_table =
            MPL_malloc(mvp_size_allgatherv_tuning_table *
                           sizeof(mvp_allgatherv_tuning_table),
                       MPL_MEM_COLL);
        mvp_allgatherv_tuning_table mvp_tmp_allgatherv_thresholds_table[] = {
            /*Trestles*/
            {
                32,
                2,
                {
                    {0, 512, &MPIR_Allgatherv_Rec_Doubling_MVP},
                    {512, -1, &MPIR_Allgatherv_Ring_MVP},
                },
            },
            {
                64,
                2,
                {
                    {0, 256, &MPIR_Allgatherv_Rec_Doubling_MVP},
                    {256, -1, &MPIR_Allgatherv_Ring_MVP},
                },
            },
            {
                128,
                2,
                {
                    {0, 128, &MPIR_Allgatherv_Rec_Doubling_MVP},
                    {128, -1, &MPIR_Allgatherv_Ring_MVP},
                },
            },
            {
                256,
                2,
                {
                    {0, 128, &MPIR_Allgatherv_Rec_Doubling_MVP},
                    {128, -1, &MPIR_Allgatherv_Ring_MVP},
                },
            },
            {
                512,
                2,
                {
                    {0, 128, &MPIR_Allgatherv_Rec_Doubling_MVP},
                    {128, -1, &MPIR_Allgatherv_Ring_MVP},
                },
            },
            {
                1024,
                2,
                {
                    {0, 256, &MPIR_Allgatherv_Rec_Doubling_MVP},
                    {256, -1, &MPIR_Allgatherv_Ring_MVP},
                },
            },
        };
        MPIR_Memcpy(mvp_allgatherv_thresholds_table,
                    mvp_tmp_allgatherv_thresholds_table,
                    mvp_size_allgatherv_tuning_table *
                        sizeof(mvp_allgatherv_tuning_table));
    } else
#endif
#endif /* !_MVP_CH4_OVERRIDE_ */
    {
        mvp_size_allgatherv_tuning_table = 7;
        mvp_allgatherv_thresholds_table =
            MPL_malloc(mvp_size_allgatherv_tuning_table *
                           sizeof(mvp_allgatherv_tuning_table),
                       MPL_MEM_COLL);
        mvp_allgatherv_tuning_table mvp_tmp_allgatherv_thresholds_table[] = {
            {
                8,
                2,
                {
                    {0, 512, &MPIR_Allgatherv_Rec_Doubling_MVP},
                    {512, -1, &MPIR_Allgatherv_Ring_MVP},
                },
            },
            {
                16,
                2,
                {
                    {0, 512, &MPIR_Allgatherv_Rec_Doubling_MVP},
                    {512, -1, &MPIR_Allgatherv_Ring_MVP},
                },
            },
            {
                32,
                2,
                {
                    {0, 512, &MPIR_Allgatherv_Rec_Doubling_MVP},
                    {512, -1, &MPIR_Allgatherv_Ring_MVP},
                },
            },
            {
                64,
                2,
                {
                    {0, 256, &MPIR_Allgatherv_Rec_Doubling_MVP},
                    {256, -1, &MPIR_Allgatherv_Ring_MVP},
                },
            },
            {
                128,
                2,
                {
                    {0, 256, &MPIR_Allgatherv_Rec_Doubling_MVP},
                    {256, -1, &MPIR_Allgatherv_Ring_MVP},
                },
            },
            {
                256,
                2,
                {
                    {0, 256, &MPIR_Allgatherv_Rec_Doubling_MVP},
                    {256, -1, &MPIR_Allgatherv_Ring_MVP},
                },
            },
            {
                512,
                2,
                {
                    {0, 256, &MPIR_Allgatherv_Rec_Doubling_MVP},
                    {256, -1, &MPIR_Allgatherv_Ring_MVP},
                },
            },

        };
        MPIR_Memcpy(mvp_allgatherv_thresholds_table,
                    mvp_tmp_allgatherv_thresholds_table,
                    mvp_size_allgatherv_tuning_table *
                        sizeof(mvp_allgatherv_tuning_table));
    }
    return 0;
}

void MVP_cleanup_allgatherv_tuning_table()
{
    if (mvp_allgatherv_thresholds_table != NULL) {
        MPL_free(mvp_allgatherv_thresholds_table);
    }
}
