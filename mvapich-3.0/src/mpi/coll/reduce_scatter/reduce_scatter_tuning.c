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
#include "reduce_scatter_tuning.h"
#include "mvp_common_tuning.h"

/* TODO: This should be moved to the pre-header as it is in ch4 */
#ifdef CHANNEL_MRAIL
#include "mvp_arch_hca_detect.h"
#endif

int mvp_size_red_scat_tuning_table = 0;
mvp_red_scat_tuning_table *mvp_red_scat_thresholds_table = NULL;

int MVP_set_red_scat_tuning_table(int heterogeneity,
                                  struct coll_info *colls_arch_hca)
{
#ifndef _MVP_CH4_OVERRIDE_
    if (MVP_IS_ARCH_HCA_TYPE(MVP_get_arch_hca_type(),
                             MVP_ARCH_INTEL_XEON_X5650_12,
                             MVP_HCA_MLX_CX_QDR) &&
        !heterogeneity) {
        mvp_size_red_scat_tuning_table = 6;
        mvp_red_scat_thresholds_table = MPL_malloc(
            mvp_size_red_scat_tuning_table * sizeof(mvp_red_scat_tuning_table),
            MPL_MEM_COLL);
        mvp_red_scat_tuning_table mvp_tmp_red_scat_thresholds_table[] = {
            {
                12,
                4,
                {
                    {0, 256, &MPIR_Reduce_Scatter_Basic_MVP},
                    {256, 8192, &MPIR_Reduce_scatter_Rec_Halving_MVP},
                    {8192, 65536, &MPIR_Reduce_scatter_Pair_Wise_MVP},
                    {65536, -1, &MPIR_Reduce_scatter_ring_2lvl},
                },
            },
            {
                24,
                3,
                {
                    {0, 64, &MPIR_Reduce_Scatter_Basic_MVP},
                    {64, 65536, &MPIR_Reduce_scatter_Rec_Halving_MVP},
                    {65536, -1, &MPIR_Reduce_scatter_ring_2lvl},
                },
            },
            {
                48,
                3,
                {
                    {0, 64, &MPIR_Reduce_Scatter_Basic_MVP},
                    {64, 131072, &MPIR_Reduce_scatter_Rec_Halving_MVP},
                    {131072, -1, &MPIR_Reduce_scatter_ring_2lvl},
                },
            },
            {
                96,
                3,
                {
                    {0, 128, &MPIR_Reduce_Scatter_Basic_MVP},
                    {128, 262144, &MPIR_Reduce_scatter_Rec_Halving_MVP},
                    {262144, -1, &MPIR_Reduce_scatter_ring_2lvl},
                },
            },
            {
                192,
                3,
                {
                    {0, 128, &MPIR_Reduce_Scatter_Basic_MVP},
                    {128, 65536, &MPIR_Reduce_scatter_Rec_Halving_MVP},
                    {65536, -1, &MPIR_Reduce_scatter_ring_2lvl},
                },
            },
            {
                384,
                3,
                {
                    {0, 128, &MPIR_Reduce_Scatter_Basic_MVP},
                    {128, 65536, &MPIR_Reduce_scatter_Rec_Halving_MVP},
                    {65536, -1, &MPIR_Reduce_scatter_ring_2lvl},
                },
            },
        };
        MPIR_Memcpy(
            mvp_red_scat_thresholds_table, mvp_tmp_red_scat_thresholds_table,
            mvp_size_red_scat_tuning_table * sizeof(mvp_red_scat_tuning_table));
    } else if (MVP_IS_ARCH_HCA_TYPE(MVP_get_arch_hca_type(),
                                    MVP_ARCH_INTEL_XEON_E5_2680_16,
                                    MVP_HCA_MLX_CX_FDR) &&
               !heterogeneity) {
        mvp_size_red_scat_tuning_table = 6;
        mvp_red_scat_thresholds_table = MPL_malloc(
            mvp_size_red_scat_tuning_table * sizeof(mvp_red_scat_tuning_table),
            MPL_MEM_COLL);
        mvp_red_scat_tuning_table mvp_tmp_red_scat_thresholds_table[] = {
            {
                16,
                3,
                {
                    {0, 64, &MPIR_Reduce_Scatter_Basic_MVP},
                    {64, 65536, &MPIR_Reduce_scatter_Rec_Halving_MVP},
                    {65536, -1, &MPIR_Reduce_scatter_ring_2lvl},
                },
            },
            {
                32,
                3,
                {
                    {0, 64, &MPIR_Reduce_Scatter_Basic_MVP},
                    {64, 131072, &MPIR_Reduce_scatter_Rec_Halving_MVP},
                    {131072, -1, &MPIR_Reduce_scatter_ring_2lvl},
                },
            },
            {
                64,
                3,
                {
                    {0, 1024, &MPIR_Reduce_Scatter_Basic_MVP},
                    {1024, 262144, &MPIR_Reduce_scatter_Rec_Halving_MVP},
                    {262144, -1, &MPIR_Reduce_scatter_ring_2lvl},
                },
            },
            {
                128,
                3,
                {
                    {0, 128, &MPIR_Reduce_Scatter_Basic_MVP},
                    {128, 65536, &MPIR_Reduce_scatter_Rec_Halving_MVP},
                    {65536, -1, &MPIR_Reduce_scatter_ring_2lvl},
                },
            },
            {
                256,
                3,
                {
                    {0, 128, &MPIR_Reduce_Scatter_Basic_MVP},
                    {128, 65536, &MPIR_Reduce_scatter_Rec_Halving_MVP},
                    {65536, -1, &MPIR_Reduce_scatter_ring_2lvl},
                },
            },
            {
                512,
                3,
                {
                    {0, 256, &MPIR_Reduce_Scatter_Basic_MVP},
                    {256, 65536, &MPIR_Reduce_scatter_Rec_Halving_MVP},
                    {65536, -1, &MPIR_Reduce_scatter_ring_2lvl},
                },
            },

        };
        MPIR_Memcpy(
            mvp_red_scat_thresholds_table, mvp_tmp_red_scat_thresholds_table,
            mvp_size_red_scat_tuning_table * sizeof(mvp_red_scat_tuning_table));
    } else if (MVP_IS_ARCH_HCA_TYPE(MVP_get_arch_hca_type(),
                                    MVP_ARCH_AMD_OPTERON_6136_32,
                                    MVP_HCA_MLX_CX_QDR) &&
               !heterogeneity) {
        mvp_size_red_scat_tuning_table = 6;
        mvp_red_scat_thresholds_table = MPL_malloc(
            mvp_size_red_scat_tuning_table * sizeof(mvp_red_scat_tuning_table),
            MPL_MEM_COLL);
        mvp_red_scat_tuning_table mvp_tmp_red_scat_thresholds_table[] = {
            {
                32,
                3,
                {
                    {0, 8, &MPIR_Reduce_Scatter_Basic_MVP},
                    {8, 32768, &MPIR_Reduce_scatter_Rec_Halving_MVP},
                    {32768, -1, &MPIR_Reduce_scatter_ring_2lvl},
                },
            },
            {
                64,
                2,
                {
                    {0, 65536, &MPIR_Reduce_Scatter_Basic_MVP},
                    {65536, -1, &MPIR_Reduce_scatter_ring_2lvl},
                },
            },
            {
                128,
                2,
                {
                    {0, 65536, &MPIR_Reduce_Scatter_Basic_MVP},
                    {65536, -1, &MPIR_Reduce_scatter_ring_2lvl},
                },
            },
            {
                256,
                2,
                {
                    {0, 65536, &MPIR_Reduce_Scatter_Basic_MVP},
                    {65536, -1, &MPIR_Reduce_scatter_ring_2lvl},
                },
            },
            {
                512,
                2,
                {
                    {0, 65536, &MPIR_Reduce_Scatter_Basic_MVP},
                    {65536, -1, &MPIR_Reduce_scatter_ring_2lvl},
                },
            },
            {
                1024,
                2,
                {
                    {0, 65536, &MPIR_Reduce_Scatter_Basic_MVP},
                    {65536, -1, &MPIR_Reduce_scatter_ring_2lvl},
                },
            },
        };
        MPIR_Memcpy(
            mvp_red_scat_thresholds_table, mvp_tmp_red_scat_thresholds_table,
            mvp_size_red_scat_tuning_table * sizeof(mvp_red_scat_tuning_table));
    } else
#endif /* !_MVP_CH4_OVERRIDE_ */
    {
        mvp_size_red_scat_tuning_table = 7;
        mvp_red_scat_thresholds_table = MPL_malloc(
            mvp_size_red_scat_tuning_table * sizeof(mvp_red_scat_tuning_table),
            MPL_MEM_COLL);
        mvp_red_scat_tuning_table mvp_tmp_red_scat_thresholds_table[] = {
            {
                8,
                4,
                {
                    {0, 256, &MPIR_Reduce_Scatter_Basic_MVP},
                    {256, 16384, &MPIR_Reduce_scatter_Rec_Halving_MVP},
                    {16384, 65536, &MPIR_Reduce_scatter_Pair_Wise_MVP},
                    {65536, -1, &MPIR_Reduce_scatter_ring_2lvl},
                },
            },
            {
                16,
                3,
                {
                    {0, 64, &MPIR_Reduce_Scatter_Basic_MVP},
                    {64, 65536, &MPIR_Reduce_scatter_Rec_Halving_MVP},
                    {65536, -1, &MPIR_Reduce_scatter_ring_2lvl},
                },
            },
            {
                32,
                3,
                {
                    {0, 64, &MPIR_Reduce_Scatter_Basic_MVP},
                    {64, 131072, &MPIR_Reduce_scatter_Rec_Halving_MVP},
                    {131072, -1, &MPIR_Reduce_scatter_ring_2lvl},
                },
            },
            {
                64,
                3,
                {
                    {0, 1024, &MPIR_Reduce_Scatter_Basic_MVP},
                    {1024, 262144, &MPIR_Reduce_scatter_Rec_Halving_MVP},
                    {262144, -1, &MPIR_Reduce_scatter_ring_2lvl},
                },
            },
            {
                128,
                2,
                {
                    {0, 128, &MPIR_Reduce_Scatter_Basic_MVP},
                    {128, 65536, &MPIR_Reduce_scatter_Rec_Halving_MVP},
                    {65536, -1, &MPIR_Reduce_scatter_ring_2lvl},
                },
            },
            {
                256,
                2,
                {
                    {0, 128, &MPIR_Reduce_Scatter_Basic_MVP},
                    {128, 65536, &MPIR_Reduce_scatter_Rec_Halving_MVP},
                    {65536, -1, &MPIR_Reduce_scatter_ring_2lvl},
                },
            },
            {
                512,
                2,
                {
                    {0, 256, &MPIR_Reduce_Scatter_Basic_MVP},
                    {256, 65536, &MPIR_Reduce_scatter_Rec_Halving_MVP},
                    {65536, -1, &MPIR_Reduce_scatter_ring_2lvl},
                },
            },
        };
        MPIR_Memcpy(
            mvp_red_scat_thresholds_table, mvp_tmp_red_scat_thresholds_table,
            mvp_size_red_scat_tuning_table * sizeof(mvp_red_scat_tuning_table));
    }
    return 0;
}

void MVP_cleanup_red_scat_tuning_table()
{
    if (mvp_red_scat_thresholds_table != NULL) {
        MPL_free(mvp_red_scat_thresholds_table);
    }
}

/* Return the number of separator inside a string */
static int count_sep(char *string)
{
    return *string == '\0' ? 0 : (count_sep(string + 1) + (*string == ','));
}

int MVP_internode_Red_scat_is_define(char *mvp_user_red_scat_inter)
{
    int i = 0;
    int nb_element = count_sep(mvp_user_red_scat_inter) + 1;

    /* If one red_scat tuning table is already defined */
    if (mvp_red_scat_thresholds_table != NULL) {
        MPL_free(mvp_red_scat_thresholds_table);
    }

    mvp_red_scat_tuning_table mvp_tmp_red_scat_thresholds_table[1];
    mvp_size_red_scat_tuning_table = 1;

    /* We realloc the space for the new red_scat tuning table */
    mvp_red_scat_thresholds_table = MPL_malloc(
        mvp_size_red_scat_tuning_table * sizeof(mvp_red_scat_tuning_table),
        MPL_MEM_COLL);

    if (nb_element == 1) {
        mvp_tmp_red_scat_thresholds_table[0].numproc = 1;
        mvp_tmp_red_scat_thresholds_table[0].size_inter_table = 1;
        mvp_tmp_red_scat_thresholds_table[0].inter_leader[0].min = 0;
        mvp_tmp_red_scat_thresholds_table[0].inter_leader[0].max = -1;

        switch (atoi(mvp_user_red_scat_inter)) {
            case RED_SCAT_BASIC:
                mvp_tmp_red_scat_thresholds_table[0]
                    .inter_leader[0]
                    .MVP_pt_Red_scat_function = &MPIR_Reduce_Scatter_Basic_MVP;
                break;
            case RED_SCAT_REC_HALF:
                mvp_tmp_red_scat_thresholds_table[0]
                    .inter_leader[0]
                    .MVP_pt_Red_scat_function =
                    &MPIR_Reduce_scatter_Rec_Halving_MVP;
                break;
            case RED_SCAT_PAIRWISE:
                mvp_tmp_red_scat_thresholds_table[0]
                    .inter_leader[0]
                    .MVP_pt_Red_scat_function =
                    &MPIR_Reduce_scatter_Pair_Wise_MVP;
                break;
            case RED_SCAT_RING:
                mvp_tmp_red_scat_thresholds_table[0]
                    .inter_leader[0]
                    .MVP_pt_Red_scat_function = &MPIR_Reduce_scatter_ring;
                break;
            case RED_SCAT_RING_2LVL:
                mvp_tmp_red_scat_thresholds_table[0]
                    .inter_leader[0]
                    .MVP_pt_Red_scat_function = &MPIR_Reduce_scatter_ring_2lvl;
                break;
            default:
                mvp_tmp_red_scat_thresholds_table[0]
                    .inter_leader[0]
                    .MVP_pt_Red_scat_function = &MPIR_Reduce_Scatter_Basic_MVP;
        }

    } else {
        char *dup, *p, *save_p;
        regmatch_t match[NMATCH];
        regex_t preg;
        const char *regexp = "([0-9]+):([0-9]+)-([0-9]+|\\+)";

        if (!(dup = MPL_strdup(mvp_user_red_scat_inter))) {
            fprintf(stderr, "failed to duplicate `%s'\n",
                    mvp_user_red_scat_inter);
            return -1;
        }

        if (regcomp(&preg, regexp, REG_EXTENDED)) {
            fprintf(stderr, "failed to compile regexp `%s'\n",
                    mvp_user_red_scat_inter);
            MPL_free(dup);
            return -1;
        }

        mvp_tmp_red_scat_thresholds_table[0].numproc = 1;
        mvp_tmp_red_scat_thresholds_table[0].size_inter_table = nb_element;

        i = 0;
        for (p = strtok_r(dup, ",", &save_p); p;
             p = strtok_r(NULL, ",", &save_p)) {
            if (regexec(&preg, p, NMATCH, match, 0)) {
                fprintf(stderr, "failed to match on `%s'\n", p);
                regfree(&preg);
                MPL_free(dup);
                return -1;
            }
            /* given () start at 1 */
            switch (atoi(p + match[1].rm_so)) {
                case RED_SCAT_BASIC:
                    mvp_tmp_red_scat_thresholds_table[0]
                        .inter_leader[i]
                        .MVP_pt_Red_scat_function =
                        &MPIR_Reduce_Scatter_Basic_MVP;
                    break;
                case RED_SCAT_REC_HALF:
                    mvp_tmp_red_scat_thresholds_table[0]
                        .inter_leader[i]
                        .MVP_pt_Red_scat_function =
                        &MPIR_Reduce_scatter_Rec_Halving_MVP;
                    break;
                case RED_SCAT_PAIRWISE:
                    mvp_tmp_red_scat_thresholds_table[0]
                        .inter_leader[i]
                        .MVP_pt_Red_scat_function =
                        &MPIR_Reduce_scatter_Pair_Wise_MVP;
                    break;
                case RED_SCAT_RING:
                    mvp_tmp_red_scat_thresholds_table[0]
                        .inter_leader[i]
                        .MVP_pt_Red_scat_function = &MPIR_Reduce_scatter_ring;
                    break;
                case RED_SCAT_RING_2LVL:
                    mvp_tmp_red_scat_thresholds_table[0]
                        .inter_leader[i]
                        .MVP_pt_Red_scat_function =
                        &MPIR_Reduce_scatter_ring_2lvl;
                    break;
                default:
                    mvp_tmp_red_scat_thresholds_table[0]
                        .inter_leader[i]
                        .MVP_pt_Red_scat_function =
                        &MPIR_Reduce_Scatter_Basic_MVP;
            }

            mvp_tmp_red_scat_thresholds_table[0].inter_leader[i].min =
                atoi(p + match[2].rm_so);
            if (p[match[3].rm_so] == '+') {
                mvp_tmp_red_scat_thresholds_table[0].inter_leader[i].max = -1;
            } else {
                mvp_tmp_red_scat_thresholds_table[0].inter_leader[i].max =
                    atoi(p + match[3].rm_so);
            }
            i++;
        }
        MPL_free(dup);
        regfree(&preg);
    }
    MPIR_Memcpy(mvp_red_scat_thresholds_table,
                mvp_tmp_red_scat_thresholds_table,
                sizeof(mvp_red_scat_tuning_table));
    return 0;
}
