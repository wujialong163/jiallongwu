/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda. For detailed
 * copyright and licensing information, please refer to the copyright file
 * COPYRIGHT in the top level MVAPICH directory.
 */

#include "mpiimpl.h"

void MPIR_MVP_print_cvars(int level)
{
    /* iterate over all cvars */
    cvar_table_entry_t *cvar_p = NULL;

    fprintf(stderr, "%55s|%20s|%10s\n", "CVAR Name", "Value", "Default");
    /* iterate past MPICH cvars at level 1 */
    while (level == 1 &&
           (cvar_p = (cvar_table_entry_t *)utarray_next(cvar_table, cvar_p)) &&
           cvar_p->name[1] != 'V') {
        /* cheating with our prefix here by just checking that the second char
         * is V */
        ;
    }
    while ((cvar_p = (cvar_table_entry_t *)utarray_next(cvar_table, cvar_p))) {
        if (cvar_p->datatype == MPI_INT) {
            fprintf(stderr, "%-55s|%20d|%20d\n", cvar_p->name,
                    *(int *)(cvar_p->addr), cvar_p->defaultval.d);
        } else if (cvar_p->datatype == MPI_DOUBLE) {
            fprintf(stderr, "%-55s|%20f|%20f\n", cvar_p->name,
                    *(float *)(cvar_p->addr), cvar_p->defaultval.f);
        } else if (cvar_p->datatype == MPI_CHAR) {
            const char *default_str = "((NULL))";
            if (cvar_p->defaultval.str) {
                default_str = cvar_p->defaultval.str;
            }
            if (NULL == cvar_p->addr || NULL == (*(char **)cvar_p->addr)) {
                char *tmp = "((NULL))";
                fprintf(stderr, "%-55s|%20s|%20s\n", cvar_p->name, tmp,
                        default_str);
            } else {
                fprintf(stderr, "%-55s|%20s|%20s\n", cvar_p->name,
                        (const char **)(cvar_p->addr), default_str);
            }
        }
    }
    fprintf(stderr, "--------------------------------------------------\n");
    fflush(stderr);
}
