
/*
 * This is here as a reminder of how we used to set multiple tuning table algo
 * and min/max values using one CVAR but also independently in the form of
 * algo1:min1-max1, algo2:min2-+, algo3:min3-max3, ...
 */

#if 0

/* Return the number of separator inside a string */
static int count_sep(const char *string)
{
    return *string == '\0' ? 0 : (count_sep(string + 1) + (*string == ','));
}

int MVP_internode_Allgather_is_define(const char *mvp_user_allgather_inter)
{
    int i = 0;
    int nb_element = count_sep(mvp_user_allgather_inter) + 1;

    if (MVP_USE_INDEXED_ALLGATHER_TUNING) {
        mvp_allgather_indexed_tuning_table
            mvp_tmp_allgather_indexed_thresholds_table[1];
        mvp_allgather_indexed_num_ppn_conf = 1;
        if (mvp_size_allgather_indexed_tuning_table == NULL) {
            mvp_size_allgather_indexed_tuning_table = MPL_malloc(
                mvp_allgather_indexed_num_ppn_conf * sizeof(int), MPL_MEM_COLL);
        }
        mvp_size_allgather_indexed_tuning_table[0] = 1;

        if (mvp_allgather_indexed_table_ppn_conf == NULL) {
            mvp_allgather_indexed_table_ppn_conf = MPL_malloc(
                mvp_allgather_indexed_num_ppn_conf * sizeof(int), MPL_MEM_COLL);
        }
        /* -1 indicates user defined algorithm */
        mvp_allgather_indexed_table_ppn_conf[0] = -1;

        /* If one allgather tuning table is already defined */
        if (mvp_allgather_indexed_thresholds_table != NULL) {
            if (mvp_allgather_indexed_thresholds_table[0] != NULL) {
                MPL_free(mvp_allgather_indexed_thresholds_table[0]);
            }
            MPL_free(mvp_allgather_indexed_thresholds_table);
        }

        /* We realloc the space for the new allgather_indexed tuning table */
        mvp_allgather_indexed_thresholds_table =
            MPL_malloc(mvp_allgather_indexed_num_ppn_conf *
                           sizeof(mvp_allgather_indexed_tuning_table *),
                       MPL_MEM_COLL);
        mvp_allgather_indexed_thresholds_table[0] =
            MPL_malloc(mvp_size_allgather_indexed_tuning_table[0] *
                           sizeof(mvp_allgather_indexed_tuning_table),
                       MPL_MEM_COLL);

        if (nb_element == 1) {
            mvp_tmp_allgather_indexed_thresholds_table[0].numproc = 1;
            mvp_tmp_allgather_indexed_thresholds_table[0].size_inter_table = 1;
            mvp_tmp_allgather_indexed_thresholds_table[0]
                .inter_leader[0]
                .msg_sz = 1;

            switch (atoi(mvp_user_allgather_inter)) {
                case ALLGATHER_RD_ALLGATHER_COMM:
                    mvp_tmp_allgather_indexed_thresholds_table[0]
                        .inter_leader[0]
                        .MVP_pt_Allgather_function =
                        &MPIR_Allgather_RD_Allgather_Comm_MVP;
                    break;
                case ALLGATHER_RD:
                    mvp_tmp_allgather_indexed_thresholds_table[0]
                        .inter_leader[0]
                        .MVP_pt_Allgather_function = &MPIR_Allgather_RD_MVP;
                    break;
                case ALLGATHER_BRUCK:
                    mvp_tmp_allgather_indexed_thresholds_table[0]
                        .inter_leader[0]
                        .MVP_pt_Allgather_function = &MPIR_Allgather_Bruck_MVP;
                    break;
                case ALLGATHER_RING:
                    mvp_tmp_allgather_indexed_thresholds_table[0]
                        .inter_leader[0]
                        .MVP_pt_Allgather_function = &MPIR_Allgather_Ring_MVP;
                    break;
                case ALLGATHER_DIRECT:
                    mvp_tmp_allgather_indexed_thresholds_table[0]
                        .inter_leader[0]
                        .MVP_pt_Allgather_function = &MPIR_Allgather_Direct_MVP;
                    break;
                case ALLGATHER_DIRECTSPREAD:
                    mvp_tmp_allgather_indexed_thresholds_table[0]
                        .inter_leader[0]
                        .MVP_pt_Allgather_function =
                        &MPIR_Allgather_DirectSpread_MVP;
                    break;
                case ALLGATHER_GATHER_BCAST:
                    mvp_tmp_allgather_indexed_thresholds_table[0]
                        .inter_leader[0]
                        .MVP_pt_Allgather_function =
                        &MPIR_Allgather_gather_bcast_MVP;
                    break;
                case ALLGATHER_2LVL_NONBLOCKED:
                    mvp_tmp_allgather_indexed_thresholds_table[0]
                        .inter_leader[0]
                        .MVP_pt_Allgather_function =
                        &MPIR_2lvl_Allgather_nonblocked_MVP;
                    break;
                case ALLGATHER_2LVL_RING_NONBLOCKED:
                    mvp_tmp_allgather_indexed_thresholds_table[0]
                        .inter_leader[0]
                        .MVP_pt_Allgather_function =
                        &MPIR_2lvl_Allgather_Ring_nonblocked_MVP;
                    break;
                case ALLGATHER_2LVL_DIRECT:
                    mvp_tmp_allgather_indexed_thresholds_table[0]
                        .inter_leader[0]
                        .MVP_pt_Allgather_function =
                        &MPIR_2lvl_Allgather_Direct_MVP;
                    break;
                case ALLGATHER_2LVL_RING:
                    mvp_tmp_allgather_indexed_thresholds_table[0]
                        .inter_leader[0]
                        .MVP_pt_Allgather_function =
                        &MPIR_2lvl_Allgather_Ring_MVP;
                    break;
                default:
                    mvp_tmp_allgather_indexed_thresholds_table[0]
                        .inter_leader[0]
                        .MVP_pt_Allgather_function = &MPIR_Allgather_RD_MVP;
            }
        }
        MPIR_Memcpy(mvp_allgather_indexed_thresholds_table[0],
                    mvp_tmp_allgather_indexed_thresholds_table,
                    sizeof(mvp_allgather_indexed_tuning_table));
    } else {
        mvp_allgather_tuning_table mvp_tmp_allgather_thresholds_table[1];
        mvp_allgather_num_ppn_conf = 1;
        if (mvp_size_allgather_tuning_table == NULL) {
            mvp_size_allgather_tuning_table = MPL_malloc(
                mvp_allgather_num_ppn_conf * sizeof(int), MPL_MEM_COLL);
        }
        mvp_size_allgather_tuning_table[0] = 1;

        if (mvp_allgather_table_ppn_conf == NULL) {
            mvp_allgather_table_ppn_conf = MPL_malloc(
                mvp_allgather_num_ppn_conf * sizeof(int), MPL_MEM_COLL);
        }
        /* -1 indicates user defined algorithm */
        mvp_allgather_table_ppn_conf[0] = -1;

        /* If one allgather tuning table is already defined */
        if (mvp_allgather_thresholds_table != NULL) {
            MPL_free(mvp_allgather_thresholds_table);
        }

        /* We realloc the space for the new allgather tuning table */
        mvp_allgather_thresholds_table = MPL_malloc(
            mvp_allgather_num_ppn_conf * sizeof(mvp_allgather_tuning_table *),
            MPL_MEM_COLL);
        mvp_allgather_thresholds_table[0] =
            MPL_malloc(mvp_size_allgather_tuning_table[0] *
                           sizeof(mvp_allgather_tuning_table),
                       MPL_MEM_COLL);

        if (nb_element == 1) {
            mvp_tmp_allgather_thresholds_table[0].numproc = 1;
            mvp_tmp_allgather_thresholds_table[0].size_inter_table = 1;
            mvp_tmp_allgather_thresholds_table[0].inter_leader[0].min = 0;
            mvp_tmp_allgather_thresholds_table[0].inter_leader[0].max = -1;
            mvp_tmp_allgather_thresholds_table[0].two_level[0] =
                MVP_INTER_ALLGATHER_TUNING_TWO_LEVEL;

            switch (atoi(mvp_user_allgather_inter)) {
                case ALLGATHER_RD_ALLGATHER_COMM:
                    mvp_tmp_allgather_thresholds_table[0]
                        .inter_leader[0]
                        .MVP_pt_Allgather_function =
                        &MPIR_Allgather_RD_Allgather_Comm_MVP;
                    break;
                case ALLGATHER_RD:
                    mvp_tmp_allgather_thresholds_table[0]
                        .inter_leader[0]
                        .MVP_pt_Allgather_function = &MPIR_Allgather_RD_MVP;
                    break;
                case ALLGATHER_BRUCK:
                    mvp_tmp_allgather_thresholds_table[0]
                        .inter_leader[0]
                        .MVP_pt_Allgather_function = &MPIR_Allgather_Bruck_MVP;
                    break;
                case ALLGATHER_RING:
                    mvp_tmp_allgather_thresholds_table[0]
                        .inter_leader[0]
                        .MVP_pt_Allgather_function = &MPIR_Allgather_Ring_MVP;
                    break;
                case ALLGATHER_DIRECT:
                    mvp_tmp_allgather_thresholds_table[0]
                        .inter_leader[0]
                        .MVP_pt_Allgather_function = &MPIR_Allgather_Direct_MVP;
                    break;
                case ALLGATHER_DIRECTSPREAD:
                    mvp_tmp_allgather_thresholds_table[0]
                        .inter_leader[0]
                        .MVP_pt_Allgather_function =
                        &MPIR_Allgather_DirectSpread_MVP;
                    break;
                case ALLGATHER_GATHER_BCAST:
                    mvp_tmp_allgather_thresholds_table[0]
                        .inter_leader[0]
                        .MVP_pt_Allgather_function =
                        &MPIR_Allgather_gather_bcast_MVP;
                    break;
                case ALLGATHER_2LVL_NONBLOCKED:
                    mvp_tmp_allgather_thresholds_table[0]
                        .inter_leader[0]
                        .MVP_pt_Allgather_function =
                        &MPIR_2lvl_Allgather_nonblocked_MVP;
                    break;
                case ALLGATHER_2LVL_RING_NONBLOCKED:
                    mvp_tmp_allgather_thresholds_table[0]
                        .inter_leader[0]
                        .MVP_pt_Allgather_function =
                        &MPIR_2lvl_Allgather_Ring_nonblocked_MVP;
                    break;
                case ALLGATHER_2LVL_DIRECT:
                    mvp_tmp_allgather_thresholds_table[0]
                        .inter_leader[0]
                        .MVP_pt_Allgather_function =
                        &MPIR_2lvl_Allgather_Direct_MVP;
                    break;
                case ALLGATHER_2LVL_RING:
                    mvp_tmp_allgather_thresholds_table[0]
                        .inter_leader[0]
                        .MVP_pt_Allgather_function =
                        &MPIR_2lvl_Allgather_Ring_MVP;
                    break;

                default:
                    mvp_tmp_allgather_thresholds_table[0]
                        .inter_leader[0]
                        .MVP_pt_Allgather_function = &MPIR_Allgather_RD_MVP;
            }

        } else {
            char *dup, *p, *save_p;
            regmatch_t match[NMATCH];
            regex_t preg;
            const char *regexp = "([0-9]+):([0-9]+)-([0-9]+|\\+)";

            if (!(dup = MPL_strdup(mvp_user_allgather_inter))) {
                fprintf(stderr, "failed to duplicate `%s'\n",
                        mvp_user_allgather_inter);
                return -1;
            }

            if (regcomp(&preg, regexp, REG_EXTENDED)) {
                fprintf(stderr, "failed to compile regexp `%s'\n",
                        mvp_user_allgather_inter);
                MPL_free(dup);
                return -1;
            }

            mvp_tmp_allgather_thresholds_table[0].numproc = 1;
            mvp_tmp_allgather_thresholds_table[0].size_inter_table = nb_element;

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
                    case ALLGATHER_RD_ALLGATHER_COMM:
                        mvp_tmp_allgather_thresholds_table[0]
                            .inter_leader[i]
                            .MVP_pt_Allgather_function =
                            &MPIR_Allgather_RD_Allgather_Comm_MVP;
                        break;
                    case ALLGATHER_RD:
                        mvp_tmp_allgather_thresholds_table[0]
                            .inter_leader[i]
                            .MVP_pt_Allgather_function = &MPIR_Allgather_RD_MVP;
                        break;
                    case ALLGATHER_BRUCK:
                        mvp_tmp_allgather_thresholds_table[0]
                            .inter_leader[i]
                            .MVP_pt_Allgather_function =
                            &MPIR_Allgather_Bruck_MVP;
                        break;
                    case ALLGATHER_RING:
                        mvp_tmp_allgather_thresholds_table[0]
                            .inter_leader[i]
                            .MVP_pt_Allgather_function =
                            &MPIR_Allgather_Ring_MVP;
                        break;
                    case ALLGATHER_DIRECT:
                        mvp_tmp_allgather_thresholds_table[0]
                            .inter_leader[0]
                            .MVP_pt_Allgather_function =
                            &MPIR_Allgather_Direct_MVP;
                        break;
                    case ALLGATHER_DIRECTSPREAD:
                        mvp_tmp_allgather_thresholds_table[0]
                            .inter_leader[0]
                            .MVP_pt_Allgather_function =
                            &MPIR_Allgather_DirectSpread_MVP;
                        break;
                    case ALLGATHER_GATHER_BCAST:
                        mvp_tmp_allgather_thresholds_table[0]
                            .inter_leader[0]
                            .MVP_pt_Allgather_function =
                            &MPIR_Allgather_gather_bcast_MVP;
                        break;
                    case ALLGATHER_2LVL_NONBLOCKED:
                        mvp_tmp_allgather_thresholds_table[0]
                            .inter_leader[0]
                            .MVP_pt_Allgather_function =
                            &MPIR_2lvl_Allgather_nonblocked_MVP;
                        break;
                    case ALLGATHER_2LVL_RING_NONBLOCKED:
                        mvp_tmp_allgather_thresholds_table[0]
                            .inter_leader[0]
                            .MVP_pt_Allgather_function =
                            &MPIR_2lvl_Allgather_Ring_nonblocked_MVP;
                        break;
                    case ALLGATHER_2LVL_DIRECT:
                        mvp_tmp_allgather_thresholds_table[0]
                            .inter_leader[0]
                            .MVP_pt_Allgather_function =
                            &MPIR_2lvl_Allgather_Direct_MVP;
                        break;
                    case ALLGATHER_2LVL_RING:
                        mvp_tmp_allgather_thresholds_table[0]
                            .inter_leader[0]
                            .MVP_pt_Allgather_function =
                            &MPIR_2lvl_Allgather_Ring_MVP;
                        break;
                    default:
                        mvp_tmp_allgather_thresholds_table[0]
                            .inter_leader[i]
                            .MVP_pt_Allgather_function = &MPIR_Allgather_RD_MVP;
                }

                mvp_tmp_allgather_thresholds_table[0].inter_leader[i].min =
                    atoi(p + match[2].rm_so);
                if (p[match[3].rm_so] == '+') {
                    mvp_tmp_allgather_thresholds_table[0].inter_leader[i].max =
                        -1;
                } else {
                    mvp_tmp_allgather_thresholds_table[0].inter_leader[i].max =
                        atoi(p + match[3].rm_so);
                }
                i++;
            }
            MPL_free(dup);
            regfree(&preg);
        }

        MPIR_Memcpy(mvp_allgather_thresholds_table[0],
                    mvp_tmp_allgather_thresholds_table,
                    sizeof(mvp_allgather_tuning_table));
    }
    return 0;
}
#endif