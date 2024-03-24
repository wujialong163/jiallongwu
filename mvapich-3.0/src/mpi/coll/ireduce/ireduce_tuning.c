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
#include "ireduce_tuning.h"
#include "mvp_common_tuning.h"

/* TODO: This should be moved to the pre-header as it is in ch4 */
#ifdef CHANNEL_MRAIL
#include "mvp_arch_hca_detect.h"
#endif

/* array used to tune ireduce */

int mvp_size_ireduce_tuning_table = 0;
mvp_ireduce_tuning_table *mvp_ireduce_thresholds_table = NULL;

int MVP_set_ireduce_tuning_table(int heterogeneity)
{
#if defined(CHANNEL_MRAIL) && !defined(_MVP_CH4_OVERRIDE_)
    if (MVP_IS_ARCH_HCA_TYPE(MVP_get_arch_hca_type(),
                             MVP_ARCH_AMD_OPTERON_6136_32,
                             MVP_HCA_MLX_CX_QDR) &&
        !heterogeneity) {
        /*Trestles Table*/
        mvp_size_ireduce_tuning_table = 5;
        mvp_ireduce_thresholds_table = MPL_malloc(
            mvp_size_ireduce_tuning_table * sizeof(mvp_ireduce_tuning_table),
            MPL_MEM_COLL);
        mvp_ireduce_tuning_table mvp_tmp_ireduce_thresholds_table[] = {
            {8,
             8192,
             4,
             4,
             {0},
             1,
             {{0, -1, &MPIR_Ireduce_intra_sched_binomial, -1}},
             1,
             {{0, -1, NULL, -1}}},
            {16,
             8192,
             4,
             4,
             {0},
             1,
             {{0, -1, &MPIR_Ireduce_intra_sched_binomial, -1}},
             1,
             {{0, -1, NULL, -1}}},
            {32,
             8192,
             4,
             4,
             {0},
             1,
             {{0, -1, &MPIR_Ireduce_intra_sched_binomial, -1}},
             1,
             {{0, -1, NULL, -1}}},
            {64,
             8192,
             4,
             4,
             {0},
             1,
             {{0, -1, &MPIR_Ireduce_intra_sched_binomial, -1}},
             1,
             {{0, -1, NULL, -1}}},
            {128,
             8192,
             4,
             4,
             {0},
             1,
             {{0, -1, &MPIR_Ireduce_intra_sched_binomial, -1}},
             1,
             {{0, -1, NULL, -1}}}};

        MPIR_Memcpy(
            mvp_ireduce_thresholds_table, mvp_tmp_ireduce_thresholds_table,
            mvp_size_ireduce_tuning_table * sizeof(mvp_ireduce_tuning_table));
    } else if (MVP_IS_ARCH_HCA_TYPE(MVP_get_arch_hca_type(),
                                    MVP_ARCH_INTEL_XEON_E5_2670_16,
                                    MVP_HCA_MLX_CX_FDR) &&
               !heterogeneity) {
        /*Gordon Table*/
        mvp_size_ireduce_tuning_table = 5;
        mvp_ireduce_thresholds_table = MPL_malloc(
            mvp_size_ireduce_tuning_table * sizeof(mvp_ireduce_tuning_table),
            MPL_MEM_COLL);
        mvp_ireduce_tuning_table mvp_tmp_ireduce_thresholds_table[] = {
            {8,
             8192,
             4,
             4,
             {0},
             1,
             {{0, -1, &MPIR_Ireduce_intra_sched_binomial, -1}},
             1,
             {{0, -1, NULL, -1}}},
            {16,
             8192,
             4,
             4,
             {0},
             1,
             {{0, -1, &MPIR_Ireduce_intra_sched_binomial, -1}},
             1,
             {{0, -1, NULL, -1}}},
            {32,
             8192,
             4,
             4,
             {0},
             1,
             {{0, -1, &MPIR_Ireduce_intra_sched_binomial, -1}},
             1,
             {{0, -1, NULL, -1}}},
            {64,
             8192,
             4,
             4,
             {0},
             1,
             {{0, -1, &MPIR_Ireduce_intra_sched_binomial, -1}},
             1,
             {{0, -1, NULL, -1}}},
            {128,
             8192,
             4,
             4,
             {0},
             1,
             {{0, -1, &MPIR_Ireduce_intra_sched_binomial, -1}},
             1,
             {{0, -1, NULL, -1}}}};

        MPIR_Memcpy(
            mvp_ireduce_thresholds_table, mvp_tmp_ireduce_thresholds_table,
            mvp_size_ireduce_tuning_table * sizeof(mvp_ireduce_tuning_table));
    } else if (MVP_IS_ARCH_HCA_TYPE(MVP_get_arch_hca_type(),
                                    MVP_ARCH_INTEL_XEON_E5_2680_16,
                                    MVP_HCA_MLX_CX_FDR) &&
               !heterogeneity) {
        /*Stampede,*/
        mvp_size_ireduce_tuning_table = 8;
        mvp_ireduce_thresholds_table = MPL_malloc(
            mvp_size_ireduce_tuning_table * sizeof(mvp_ireduce_tuning_table),
            MPL_MEM_COLL);
        mvp_ireduce_tuning_table mvp_tmp_ireduce_thresholds_table[] = {
            {8,
             8192,
             4,
             4,
             {0},
             1,
             {{0, -1, &MPIR_Ireduce_intra_sched_binomial, -1}},
             1,
             {{0, -1, NULL, -1}}},
            {16,
             8192,
             4,
             4,
             {0},
             1,
             {{0, -1, &MPIR_Ireduce_intra_sched_binomial, -1}},
             1,
             {{0, -1, NULL, -1}}},
            {32,
             8192,
             4,
             4,
             {0},
             1,
             {{0, -1, &MPIR_Ireduce_intra_sched_binomial, -1}},
             1,
             {{0, -1, NULL, -1}}},
            {64,
             8192,
             4,
             4,
             {0},
             1,
             {{0, -1, &MPIR_Ireduce_intra_sched_binomial, -1}},
             1,
             {{0, -1, NULL, -1}}},
            {128,
             8192,
             4,
             4,
             {0},
             1,
             {{0, -1, &MPIR_Ireduce_intra_sched_binomial, -1}},
             1,
             {{0, -1, NULL, -1}}},
            {256,
             8192,
             4,
             4,
             {0},
             1,
             {{0, -1, &MPIR_Ireduce_intra_sched_binomial, -1}},
             1,
             {{0, -1, NULL, -1}}},
            {512,
             8192,
             4,
             4,
             {0},
             1,
             {{0, -1, &MPIR_Ireduce_intra_sched_binomial, -1}},
             1,
             {{0, -1, NULL, -1}}},
            {1024,
             8192,
             4,
             4,
             {0},
             1,
             {{0, -1, &MPIR_Ireduce_intra_sched_binomial, -1}},
             1,
             {{0, -1, NULL, -1}}}};

        MPIR_Memcpy(
            mvp_ireduce_thresholds_table, mvp_tmp_ireduce_thresholds_table,
            mvp_size_ireduce_tuning_table * sizeof(mvp_ireduce_tuning_table));
    } else {
        /*RI*/
        mvp_size_ireduce_tuning_table = 7;
        mvp_ireduce_thresholds_table = MPL_malloc(
            mvp_size_ireduce_tuning_table * sizeof(mvp_ireduce_tuning_table),
            MPL_MEM_COLL);
        mvp_ireduce_tuning_table mvp_tmp_ireduce_thresholds_table[] = {
            {8,
             8192,
             4,
             4,
             {0},
             1,
             {{0, -1, &MPIR_Ireduce_intra_sched_binomial, -1}},
             1,
             {{0, -1, NULL, -1}}},
            {16,
             8192,
             4,
             4,
             {0},
             1,
             {{0, -1, &MPIR_Ireduce_intra_sched_binomial, -1}},
             1,
             {{0, -1, NULL, -1}}},
            {32,
             8192,
             4,
             4,
             {0},
             1,
             {{0, -1, &MPIR_Ireduce_intra_sched_binomial, -1}},
             1,
             {{0, -1, NULL, -1}}},
            {64,
             8192,
             4,
             4,
             {0},
             1,
             {{0, -1, &MPIR_Ireduce_intra_sched_binomial, -1}},
             1,
             {{0, -1, NULL, -1}}},
            {128,
             8192,
             4,
             4,
             {0},
             1,
             {{0, -1, &MPIR_Ireduce_intra_sched_binomial, -1}},
             1,
             {{0, -1, NULL, -1}}},
            {256,
             8192,
             4,
             4,
             {0},
             1,
             {{0, -1, &MPIR_Ireduce_intra_sched_binomial, -1}},
             1,
             {{0, -1, NULL, -1}}},
            {512,
             8192,
             4,
             4,
             {0},
             1,
             {{0, -1, &MPIR_Ireduce_intra_sched_binomial, -1}},
             1,
             {{0, -1, NULL, -1}}}};

        MPIR_Memcpy(
            mvp_ireduce_thresholds_table, mvp_tmp_ireduce_thresholds_table,
            mvp_size_ireduce_tuning_table * sizeof(mvp_ireduce_tuning_table));
    }
#else /* defined(CHANNEL_MRAIL) && !defined(_MVP_CH4_OVERRIDE_) */

    /*RI*/
    mvp_size_ireduce_tuning_table = 7;
    mvp_ireduce_thresholds_table = MPL_malloc(
        mvp_size_ireduce_tuning_table * sizeof(mvp_ireduce_tuning_table),
        MPL_MEM_COLL);
    mvp_ireduce_tuning_table mvp_tmp_ireduce_thresholds_table[] = {
        {8,
         8192,
         4,
         4,
         {0},
         1,
         {{0, -1, &MPIR_Ireduce_intra_sched_binomial, -1}},
         1,
         {{0, -1, NULL, -1}}},
        {16,
         8192,
         4,
         4,
         {0},
         1,
         {{0, -1, &MPIR_Ireduce_intra_sched_binomial, -1}},
         1,
         {{0, -1, NULL, -1}}},
        {32,
         8192,
         4,
         4,
         {0},
         1,
         {{0, -1, &MPIR_Ireduce_intra_sched_binomial, -1}},
         1,
         {{0, -1, NULL, -1}}},
        {64,
         8192,
         4,
         4,
         {0},
         1,
         {{0, -1, &MPIR_Ireduce_intra_sched_binomial, -1}},
         1,
         {{0, -1, NULL, -1}}},
        {128,
         8192,
         4,
         4,
         {0},
         1,
         {{0, -1, &MPIR_Ireduce_intra_sched_binomial, -1}},
         1,
         {{0, -1, NULL, -1}}},
        {256,
         8192,
         4,
         4,
         {0},
         1,
         {{0, -1, &MPIR_Ireduce_intra_sched_binomial, -1}},
         1,
         {{0, -1, NULL, -1}}},
        {512,
         8192,
         4,
         4,
         {0},
         1,
         {{0, -1, &MPIR_Ireduce_intra_sched_binomial, -1}},
         1,
         {{0, -1, NULL, -1}}}};

    MPIR_Memcpy(mvp_ireduce_thresholds_table, mvp_tmp_ireduce_thresholds_table,
                mvp_size_ireduce_tuning_table *
                    sizeof(mvp_ireduce_tuning_table));
#endif
    return MPI_SUCCESS;
}

void MVP_cleanup_ireduce_tuning_table()
{
    if (mvp_ireduce_thresholds_table != NULL) {
        MPL_free(mvp_ireduce_thresholds_table);
    }
}

/* Return the number of separator inside a string */
static int count_sep(char *string)
{
    return *string == '\0' ? 0 : (count_sep(string + 1) + (*string == ','));
}

int MVP_internode_Ireduce_is_define(char *mvp_user_ireduce_inter,
                                    char *mvp_user_ireduce_intra)
{
    int i;
    int nb_element = count_sep(mvp_user_ireduce_inter) + 1;

    /* If one ireduce tuning table is already defined */
    if (mvp_ireduce_thresholds_table != NULL) {
        MPL_free(mvp_ireduce_thresholds_table);
    }

    mvp_ireduce_tuning_table mvp_tmp_ireduce_thresholds_table[1];
    mvp_size_ireduce_tuning_table = 1;

    /* We realloc the space for the new ireduce tuning table */
    mvp_ireduce_thresholds_table = MPL_malloc(
        mvp_size_ireduce_tuning_table * sizeof(mvp_ireduce_tuning_table),
        MPL_MEM_COLL);

    if (nb_element == 1) {
        // consider removing some fields underneath
        mvp_tmp_ireduce_thresholds_table[0].numproc = 1;
        mvp_tmp_ireduce_thresholds_table[0].ireduce_segment_size =
            ireduce_segment_size;
        mvp_tmp_ireduce_thresholds_table[0].inter_node_knomial_factor =
            MVP_KNOMIAL_INTER_NODE_FACTOR;
        mvp_tmp_ireduce_thresholds_table[0].intra_node_knomial_factor =
            MVP_KNOMIAL_INTRA_NODE_FACTOR;
        mvp_tmp_ireduce_thresholds_table[0].is_two_level_ireduce[0] = 1;
        mvp_tmp_ireduce_thresholds_table[0].size_inter_table = 1;
        mvp_tmp_ireduce_thresholds_table[0].inter_leader[0].min = 0;
        mvp_tmp_ireduce_thresholds_table[0].inter_leader[0].max = -1;
        mvp_tmp_ireduce_thresholds_table[0].intra_node[0].min = 0;
        mvp_tmp_ireduce_thresholds_table[0].intra_node[0].max = -1;
        switch (atoi(mvp_user_ireduce_inter)) {
            case IREDUCE_BINOMIAL:
                mvp_tmp_ireduce_thresholds_table[0]
                    .inter_leader[0]
                    .MVP_pt_Ireduce_function =
                    &MPIR_Ireduce_intra_sched_binomial;
                mvp_tmp_ireduce_thresholds_table[0].is_two_level_ireduce[0] = 0;
                break;
            case IREDUCE_REDSCAT_GATHER:
                mvp_tmp_ireduce_thresholds_table[0]
                    .inter_leader[0]
                    .MVP_pt_Ireduce_function =
                    &MPIR_Ireduce_intra_sched_reduce_scatter_gather;
                mvp_tmp_ireduce_thresholds_table[0].is_two_level_ireduce[0] = 0;
                break;
            default:
                mvp_tmp_ireduce_thresholds_table[0]
                    .inter_leader[0]
                    .MVP_pt_Ireduce_function =
                    &MPIR_Ireduce_intra_sched_binomial;
                mvp_tmp_ireduce_thresholds_table[0].is_two_level_ireduce[0] = 0;
                break;
        }
        if (mvp_user_ireduce_intra == NULL) {
            mvp_tmp_ireduce_thresholds_table[0]
                .intra_node[0]
                .MVP_pt_Ireduce_function = NULL;
        } else {
            mvp_tmp_ireduce_thresholds_table[0]
                .intra_node[0]
                .MVP_pt_Ireduce_function = NULL;
        }
    } else {
        char *dup, *p, *save_p;
        regmatch_t match[NMATCH];
        regex_t preg;
        const char *regexp = "([0-9]+):([0-9]+)-([0-9]+|\\+)";

        if (!(dup = MPL_strdup(mvp_user_ireduce_inter))) {
            fprintf(stderr, "failed to duplicate `%s'\n",
                    mvp_user_ireduce_inter);
            return MPI_ERR_INTERN;
        }

        if (regcomp(&preg, regexp, REG_EXTENDED)) {
            fprintf(stderr, "failed to compile regexp `%s'\n",
                    mvp_user_ireduce_inter);
            MPL_free(dup);
            return MPI_ERR_INTERN;
        }

        mvp_tmp_ireduce_thresholds_table[0].numproc = 1;
        mvp_tmp_ireduce_thresholds_table[0].ireduce_segment_size =
            ireduce_segment_size;
        mvp_tmp_ireduce_thresholds_table[0].inter_node_knomial_factor =
            MVP_KNOMIAL_INTER_NODE_FACTOR;
        mvp_tmp_ireduce_thresholds_table[0].intra_node_knomial_factor =
            MVP_KNOMIAL_INTRA_NODE_FACTOR;
        mvp_tmp_ireduce_thresholds_table[0].size_inter_table = nb_element;
        i = 0;
        for (p = strtok_r(dup, ",", &save_p); p;
             p = strtok_r(NULL, ",", &save_p)) {
            if (regexec(&preg, p, NMATCH, match, 0)) {
                fprintf(stderr, "failed to match on `%s'\n", p);
                regfree(&preg);
                MPL_free(dup);
                return 2;
            }
            /* given () start at 1 */
            switch (atoi(p + match[1].rm_so)) {
                case IREDUCE_BINOMIAL:
                    mvp_tmp_ireduce_thresholds_table[0]
                        .inter_leader[0]
                        .MVP_pt_Ireduce_function =
                        &MPIR_Ireduce_intra_sched_binomial;
                    mvp_tmp_ireduce_thresholds_table[0]
                        .is_two_level_ireduce[0] = 0;
                    break;
                case IREDUCE_REDSCAT_GATHER:
                    mvp_tmp_ireduce_thresholds_table[0]
                        .inter_leader[0]
                        .MVP_pt_Ireduce_function =
                        &MPIR_Ireduce_intra_sched_reduce_scatter_gather;
                    mvp_tmp_ireduce_thresholds_table[0]
                        .is_two_level_ireduce[0] = 0;
                    break;
                default:
                    mvp_tmp_ireduce_thresholds_table[0]
                        .inter_leader[0]
                        .MVP_pt_Ireduce_function =
                        &MPIR_Ireduce_intra_sched_binomial;
                    mvp_tmp_ireduce_thresholds_table[0]
                        .is_two_level_ireduce[0] = 0;
                    break;
            }
            mvp_tmp_ireduce_thresholds_table[0].inter_leader[i].min =
                atoi(p + match[2].rm_so);
            if (p[match[3].rm_so] == '+') {
                mvp_tmp_ireduce_thresholds_table[0].inter_leader[i].max = -1;
            } else {
                mvp_tmp_ireduce_thresholds_table[0].inter_leader[i].max =
                    atoi(p + match[3].rm_so);
            }

            i++;
        }
        MPL_free(dup);
        regfree(&preg);
    }
    mvp_tmp_ireduce_thresholds_table[0].size_intra_table = 1;
    if (mvp_user_ireduce_intra == NULL) {
        mvp_tmp_ireduce_thresholds_table[0]
            .intra_node[0]
            .MVP_pt_Ireduce_function = NULL;
    } else {
        mvp_tmp_ireduce_thresholds_table[0]
            .intra_node[0]
            .MVP_pt_Ireduce_function = NULL;
    }
    MPIR_Memcpy(mvp_ireduce_thresholds_table, mvp_tmp_ireduce_thresholds_table,
                sizeof(mvp_ireduce_tuning_table));
    return MPI_SUCCESS;
}

int MVP_intranode_Ireduce_is_define(char *mvp_user_ireduce_intra)
{
    int i, j;
    for (i = 0; i < mvp_size_ireduce_tuning_table; i++) {
        for (j = 0; j < mvp_ireduce_thresholds_table[i].size_intra_table; j++) {
            mvp_ireduce_thresholds_table[i]
                .intra_node[j]
                .MVP_pt_Ireduce_function = NULL;
        }
    }
    return MPI_SUCCESS;
}
