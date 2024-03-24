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

#include <mpirun_util.h>

/*
 * Array of mpirun parameters that should be forwarded
 */
static char const *const parameters[] = {
    "MVP_CKPT_FILE",
    "MVP_CKPT_INTERVAL",
    "MVP_CKPT_MAX_SAVE_CKPTS",
    "MVP_CKPT_USE_AGGREGATION",
    "MVP_FASTSSH_THRESHOLD",
    "MVP_NPROCS_THRESHOLD",
    "MVP_MPIRUN_TIMEOUT",
    "MVP_MT_DEGREE",
    "MPIEXEC_TIMEOUT",
    "MVP_CKPT_AGGREGATION_BUFPOOL_SIZE",
    "MVP_CKPT_AGGREGATION_CHUNK_SIZE",
    "MVP_CKPT_MAX_CKPTS",
    "MVP_IGNORE_SYSTEM_CONFIG",
    "MVP_IGNORE_USER_CONFIG",
    "MVP_USER_CONFIG",
    "MVP_DEBUG_CORESIZE",
    "MVP_DEBUG_SHOW_BACKTRACE",
};

static size_t const num_parameters =
    sizeof(parameters) / sizeof(char const *const);

/*
 * str must be dynamically allocated
 */
extern char *append_mpirun_parameters(char *str)
{
    extern size_t const num_parameters;
    char const *value;
    size_t i;

    for (i = 0; i < num_parameters; i++) {
        if ((value = getenv(parameters[i]))) {
            char *key_value = mkstr(" %s=%s", parameters[i], value);

            str = append_str(str, key_value);
            free(key_value);
        }
    }

    return str;
}
/*
 * str must be dynamically allocated
 */
extern char *append_mpirun_srun_parameters(char *str)
{
    extern size_t const num_parameters;
    char const *value;
    size_t i;

    for (i = 0; i < num_parameters; i++) {
        if ((value = getenv(parameters[i]))) {
            char *key_value = mkstr(",%s=%s", parameters[i], value);

            str = append_str(str, key_value);
            free(key_value);
        }
    }

    return str;
}
