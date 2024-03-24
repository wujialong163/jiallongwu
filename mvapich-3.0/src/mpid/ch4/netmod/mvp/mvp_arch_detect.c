/* Copyright (c) 2001-2023, The Ohio State University. All rights
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

#include <stdio.h>
#include <string.h>
#if defined(HAVE_LIBIBVERBS)
#include <infiniband/verbs.h>
#endif

#include <mpichconf.h>

#include <hwloc.h>
#include <dirent.h>

#include "mpidimpl.h"
#include "hwloc_bind.h"
#include "mvp_arch_hca_detect.h"
#include "mvp_debug_utils.h"
#include "upmi.h"
#include "mpi.h"

#if ENABLE_PVAR_MVP && CHANNEL_MRAIL
#include "rdma_impl.h"
#endif

#if defined(_MCST_SUPPORT_)
#include "ibv_param.h"
#endif

/* A stub value to be used when forcing CPU architecture */
#define MVP_STUB_NUM_CPUS 28

#if defined(_SMP_LIMIC_)
#include "rdma_impl.h"

#define SOCKETS           32
#define CORES             32
#define HEX_FORMAT        16
#define CORES_REP_AS_BITS 32

/*global variables*/
static int node[SOCKETS][CORES] = {{0}};
static int no_sockets = 0;
static int socket_bound = -1;
static int numcores_persocket[SOCKETS] = {0};
#endif /*#if defined(_SMP_LIMIC_)*/

int g_mvp_num_cpus = -1;
static int g_mvp_cpu_model = -1;
static int mvp_cpu_arch_make = MVP_CPU_ARCH_MAKE__NONE;

mvp_arch_t table_arch_tmp;
mvp_hca_t table_hca_tmp;

/* global rdma structure for the local process */
mvp_arch_hca_info_t my_arch_info;

#define CONFIG_FILE     "/proc/cpuinfo"
#define MAX_LINE_LENGTH 512
#define MAX_NAME_LENGTH 512

#define CLOVERTOWN_MODEL            15
#define HARPERTOWN_MODEL            23
#define NEHALEM_MODEL               26
#define INTEL_E5630_MODEL           44
#define INTEL_X5650_MODEL           44
#define INTEL_E5_2670_MODEL         45
#define INTEL_XEON_E5_2670_V2_MODEL 62
#define INTEL_XEON_E5_2698_V3_MODEL 63
#define INTEL_XEON_E5_2660_V3_MODEL 63
#define INTEL_XEON_E5_2680_V3_MODEL 63
#define INTEL_XEON_E5_2695_V3_MODEL 63
#define INTEL_XEON_E5_2670_V3_MODEL 64
#define INTEL_XEON_E5_2680_V4_MODEL 79
#define INTEL_XEON_E5_2620_V4_MODEL 80
/* Skylake */
#define INTEL_PLATINUM_8160_MODEL 85
#define INTEL_PLATINUM_8170_MODEL 85
/* Cascade Lake */
#define INTEL_PLATINUM_8260_MODEL 85

#define MVP_STR_VENDOR_ID        "vendor_id"
#define MVP_STR_AUTH_AMD         "AuthenticAMD"
#define MVP_STR_MODEL            "model"
#define MVP_STR_WS               " "
#define MVP_STR_PHYSICAL         "physical"
#define MVP_STR_MODEL_NAME       "model name"
#define MVP_STR_POWER8_ID        "POWER8"
#define MVP_STR_POWER9_ID        "POWER9"
#define MVP_STR_CAVIUM_ID        "0x43"
#define MVP_STR_FUJITSU_ID       "0x46"
#define MVP_STR_GRACE_ID         "0x41"
#define MVP_ARM_CAVIUM_V8_MODEL  8
#define MVP_ARM_GRACE_MODEL      8
#define MVP_ARM_FUJITSU_V0_MODEL 0

#define INTEL_E5_2670_MODEL_NAME    "Intel(R) Xeon(R) CPU E5-2670 0 @ 2.60GHz"
#define INTEL_E5_2680_MODEL_NAME    "Intel(R) Xeon(R) CPU E5-2680 0 @ 2.70GHz"
#define INTEL_E5_2670_V2_MODEL_NAME "Intel(R) Xeon(R) CPU E5-2670 v2 @ 2.50GHz"
#define INTEL_E5_2630_V2_MODEL_NAME "Intel(R) Xeon(R) CPU E5-2630 v2 @ 2.60GHz"
#define INTEL_E5_2680_V2_MODEL_NAME "Intel(R) Xeon(R) CPU E5-2680 v2 @ 2.80GHz"
#define INTEL_E5_2690_V2_MODEL_NAME "Intel(R) Xeon(R) CPU E5-2690 v2 @ 3.00GHz"
#define INTEL_E5_2690_V3_MODEL_NAME "Intel(R) Xeon(R) CPU E5-2690 v3 @ 2.60GHz"
#define INTEL_E5_2698_V3_MODEL_NAME "Intel(R) Xeon(R) CPU E5-2698 v3 @ 2.30GHz"
#define INTEL_E5_2660_V3_MODEL_NAME "Intel(R) Xeon(R) CPU E5-2660 v3 @ 2.60GHz"
#define INTEL_E5_2680_V3_MODEL_NAME "Intel(R) Xeon(R) CPU E5-2680 v3 @ 2.50GHz"
#define INTEL_E5_2680_V4_MODEL_NAME "Intel(R) Xeon(R) CPU E5-2680 v4 @ 2.40GHz"
#define INTEL_E5_2687W_V3_MODEL_NAME                                           \
    "Intel(R) Xeon(R) CPU E5-2687W v3 @ 3.10GHz"
#define INTEL_E5_2670_V3_MODEL_NAME "Intel(R) Xeon(R) CPU E5-2670 v3 @ 2.30GHz"
#define INTEL_E5_2695_V3_MODEL_NAME "Intel(R) Xeon(R) CPU E5-2695 v3 @ 2.30GHz"
#define INTEL_E5_2695_V4_MODEL_NAME "Intel(R) Xeon(R) CPU E5-2695 v4 @ 2.10GHz"
#define INTEL_E5_2620_V4_MODEL_NAME "Intel(R) Xeon(R) CPU E5-2620 v4 @ 2.10GHz"
#define INTEL_E5_2697A_V4_MODEL_NAME                                           \
    "Intel(R) Xeon(R) CPU E5-2697A v4 @ 2.60GHz"

/* For both Skylake and Cascade Lake, generic models are the same */
#define INTEL_PLATINUM_GENERIC_MODEL_NAME "Intel(R) Xeon(R) Platinum"
#define INTEL_PLATINUM_8160_MODEL_NAME                                         \
    "Intel(R) Xeon(R) Platinum 8160 CPU @ 2.10GHz"
#define INTEL_PLATINUM_8170_MODEL_NAME                                         \
    "Intel(R) Xeon(R) Platinum 8170 CPU @ 2.10GHz"
#define INTEL_PLATINUM_8260_MODEL_NAME                                         \
    "Intel(R) Xeon(R) Platinum 8260Y CPU @ 2.40GHz"
#define INTEL_PLATINUM_8268_MODEL_NAME                                         \
    "Intel(R) Xeon(R) Platinum 8268 CPU @ 2.90GHz"
#define INTEL_PLATINUM_8280_MODEL_NAME                                         \
    "Intel(R) Xeon(R) Platinum 8280 CPU @ 2.70GHz"
#define INTEL_PLATINUM_8380_MODEL_NAME                                         \
    "Intel(R) Xeon(R) Platinum 8380 CPU @ 2.30GHz"
#define INTEL_PLATINUM_8480_MODEL_NAME "Intel(R) Xeon(R) Platinum 8480+"

#define INTEL_GOLD_GENERIC_MODEL_NAME "Intel(R) Xeon(R) Gold"
#define INTEL_GOLD_6132_MODEL_NAME    "Intel(R) Xeon(R) Gold 6132 CPU @ 2.60GHz"
#define INTEL_GOLD_6148_MODEL_NAME    "Intel(R) Xeon(R) Gold 6148 CPU @ 2.40GHz"
#define INTEL_GOLD_6248_MODEL_NAME    "Intel(R) Xeon(R) Gold 6248 CPU @ 2.50GHz"
#define INTEL_GOLD_6154_MODEL_NAME    "Intel(R) Xeon(R) Gold 6154 CPU @ 3.00GHz"

#define INTEL_XEON_PHI_GENERIC_MODEL_NAME "Intel(R) Xeon Phi(TM) CPU"
#define INTEL_XEON_PHI_7210_MODEL_NAME                                         \
    "Intel(R) Xeon Phi(TM) CPU 7210 @ 1.30GHz"
#define INTEL_XEON_PHI_7230_MODEL_NAME                                         \
    "Intel(R) Xeon Phi(TM) CPU 7230 @ 1.30GHz"
#define INTEL_XEON_PHI_7250_MODEL_NAME                                         \
    "Intel(R) Xeon Phi(TM) CPU 7250 @ 1.40GHz"
#define INTEL_XEON_PHI_7290_MODEL_NAME                                         \
    "Intel(R) Xeon Phi(TM) CPU 7290 @ 1.50GHz"

#define AMD_EPYC_GENERAL_MODEL_NAME "AMD EPYC"
#define AMD_EPYC_7601_MODEL_NAME    "AMD EPYC 7601 32-Core Processor"
#define AMD_EPYC_7643_MODEL_NAME    "AMD EPYC 7643 48-Core Processor"
#define AMD_EPYC_7401_MODEL_NAME    "AMD EPYC 7401 24-Core Processor"
#define AMD_EPYC_7V12_MODEL_NAME    "AMD EPYC 7V12 64-Core Processor"
#define AMD_EPYC_7A53_MODEL_NAME    "AMD EPYC 7A53 64-Core Processor"
#define AMD_EPYC_7763_MODEL_NAME                                               \
    "AMD EPYC 7763 64-Core Processor" /* Lonestar-6 */
#define AMD_EPYC_7713_MODEL_NAME                                               \
    "AMD EPYC 7713 64-Core Processor" /* Lonestar-6 */
#define AMD_EPYC_7662_MODEL_NAME "AMD EPYC 7662 64-Core Processor" /* Spock */
#define AMD_EPYC_9124_MODEL_NAME "AMD EPYC 9124 16-Core Processor" /* Genoa */

typedef struct _mvp_arch_types_log_t {
    uint64_t arch_type;
    char *arch_name;
} mvp_arch_types_log_t;

#define MVP_ARCH_LAST_ENTRY -1
static mvp_arch_types_log_t mvp_arch_types_log[] = {
    /* Intel Architectures */
    {MVP_ARCH_INTEL_GENERIC, "MVP_ARCH_INTEL_GENERIC"},
    {MVP_ARCH_INTEL_CLOVERTOWN_8, "MVP_ARCH_INTEL_CLOVERTOWN_8"},
    {MVP_ARCH_INTEL_NEHALEM_8, "MVP_ARCH_INTEL_NEHALEM_8"},
    {MVP_ARCH_INTEL_NEHALEM_16, "MVP_ARCH_INTEL_NEHALEM_16"},
    {MVP_ARCH_INTEL_HARPERTOWN_8, "MVP_ARCH_INTEL_HARPERTOWN_8"},
    {MVP_ARCH_INTEL_XEON_DUAL_4, "MVP_ARCH_INTEL_XEON_DUAL_4"},
    {MVP_ARCH_INTEL_XEON_E5630_8, "MVP_ARCH_INTEL_XEON_E5630_8"},
    {MVP_ARCH_INTEL_XEON_X5650_12, "MVP_ARCH_INTEL_XEON_X5650_12"},
    {MVP_ARCH_INTEL_XEON_E5_2670_16, "MVP_ARCH_INTEL_XEON_E5_2670_16"},
    {MVP_ARCH_INTEL_XEON_E5_2680_16, "MVP_ARCH_INTEL_XEON_E5_2680_16"},
    {MVP_ARCH_INTEL_XEON_E5_2670_V2_2S_20,
     "MVP_ARCH_INTEL_XEON_E5_2670_V2_2S_20"},
    {MVP_ARCH_INTEL_XEON_E5_2630_V2_2S_12,
     "MVP_ARCH_INTEL_XEON_E5_2630_V2_2S_12"},
    {MVP_ARCH_INTEL_XEON_E5_2680_V2_2S_20,
     "MVP_ARCH_INTEL_XEON_E5_2680_V2_2S_20"},
    {MVP_ARCH_INTEL_XEON_E5_2690_V2_2S_20,
     "MVP_ARCH_INTEL_XEON_E5_2690_V2_2S_20"},
    {MVP_ARCH_INTEL_XEON_E5_2690_V3_2S_24,
     "MVP_ARCH_INTEL_XEON_E5_2690_V3_2S_24"},
    {MVP_ARCH_INTEL_XEON_E5_2698_V3_2S_32,
     "MVP_ARCH_INTEL_XEON_E5_2698_V3_2S_32"},
    {MVP_ARCH_INTEL_XEON_E5_2660_V3_2S_20,
     "MVP_ARCH_INTEL_XEON_E5_2660_V3_2S_20"},
    {MVP_ARCH_INTEL_XEON_E5_2680_V3_2S_24,
     "MVP_ARCH_INTEL_XEON_E5_2680_V3_2S_24"},
    {MVP_ARCH_INTEL_XEON_E5_2687W_V3_2S_20,
     "MVP_ARCH_INTEL_XEON_E5_2687W_V3_2S_20"},
    {MVP_ARCH_INTEL_XEON_E5_2670_V3_2S_24,
     "MVP_ARCH_INTEL_XEON_E5_2670_V3_2S_24"},
    {MVP_ARCH_INTEL_XEON_E5_2695_V3_2S_28,
     "MVP_ARCH_INTEL_XEON_E5_2695_V3_2S_28"},
    {MVP_ARCH_INTEL_XEON_E5_2695_V4_2S_36,
     "MVP_ARCH_INTEL_XEON_E5_2695_V4_2S_36"},
    {MVP_ARCH_INTEL_XEON_E5_2680_V4_2S_28,
     "MVP_ARCH_INTEL_XEON_E5_2680_V4_2S_28"},
    {MVP_ARCH_INTEL_XEON_E5_2620_V4_2S_16,
     "MVP_ARCH_INTEL_XEON_E5_2620_V4_2S_16"},
    {MVP_ARCH_INTEL_XEON_E5_2697A_V4_2S_32,
     "MVP_ARCH_INTEL_XEON_E5_2697A_V4_2S_32"},

    /* Skylake and Cascade Lake Architectures */
    {MVP_ARCH_INTEL_PLATINUM_GENERIC, "MVP_ARCH_INTEL_PLATINUM_GENERIC"},
    {MVP_ARCH_INTEL_PLATINUM_8160_2S_48, "MVP_ARCH_INTEL_PLATINUM_8160_2S_48"},
    {MVP_ARCH_INTEL_PLATINUM_8260_2S_48, "MVP_ARCH_INTEL_PLATINUM_8260_2S_48"},
    {MVP_ARCH_INTEL_PLATINUM_8268_2S_48, "MVP_ARCH_INTEL_PLATINUM_8268_2S_48"},
    {MVP_ARCH_INTEL_PLATINUM_8280_2S_56, "MVP_ARCH_INTEL_PLATINUM_8280_2S_56"},
    {MVP_ARCH_INTEL_PLATINUM_8170_2S_52, "MVP_ARCH_INTEL_PLATINUM_8170_2S_52"},
    {MVP_ARCH_INTEL_PLATINUM_8380_2S_80, "MVP_ARCH_INTEL_PLATINUM_8380_2S_80"},
    {MVP_ARCH_INTEL_GOLD_GENERIC, "MVP_ARCH_INTEL_GOLD_GENERIC"},
    {MVP_ARCH_INTEL_GOLD_6132_2S_28, "MVP_ARCH_INTEL_GOLD_6132_2S_28"},
    {MVP_ARCH_INTEL_GOLD_6148_2S_40, "MVP_ARCH_INTEL_GOLD_6148_2S_40"},
    {MVP_ARCH_INTEL_GOLD_6154_2S_36, "MVP_ARCH_INTEL_GOLD_6154_2S_36"},

    /* Intel Sapphire Rapids Architectures */
    {MVP_ARCH_INTEL_PLATINUM_8480_2S_112,
     "MVP_ARCH_INTEL_PLATINUM_8480_2S_112"},

    /* KNL Architectures */
    {MVP_ARCH_INTEL_KNL_GENERIC, "MVP_ARCH_INTEL_KNL_GENERIC"},
    {MVP_ARCH_INTEL_XEON_PHI_7210, "MVP_ARCH_INTEL_XEON_PHI_7210"},
    {MVP_ARCH_INTEL_XEON_PHI_7230, "MVP_ARCH_INTEL_XEON_PHI_7230"},
    {MVP_ARCH_INTEL_XEON_PHI_7250, "MVP_ARCH_INTEL_XEON_PHI_7250"},
    {MVP_ARCH_INTEL_XEON_PHI_7290, "MVP_ARCH_INTEL_XEON_PHI_7290"},

    /* AMD Architectures */
    {MVP_ARCH_AMD_GENERIC, "MVP_ARCH_AMD_GENERIC"},
    {MVP_ARCH_AMD_BARCELONA_16, "MVP_ARCH_AMD_BARCELONA_16"},
    {MVP_ARCH_AMD_MAGNY_COURS_24, "MVP_ARCH_AMD_MAGNY_COURS_24"},
    {MVP_ARCH_AMD_OPTERON_DUAL_4, "MVP_ARCH_AMD_OPTERON_DUAL_4"},
    {MVP_ARCH_AMD_OPTERON_6136_32, "MVP_ARCH_AMD_OPTERON_6136_32"},
    {MVP_ARCH_AMD_OPTERON_6276_64, "MVP_ARCH_AMD_OPTERON_6276_64"},
    {MVP_ARCH_AMD_BULLDOZER_4274HE_16, "MVP_ARCH_AMD_BULLDOZER_4274HE_16"},
    {MVP_ARCH_AMD_EPYC_7401_48, "MVP_ARCH_AMD_EPYC_7401_48"},
    {MVP_ARCH_AMD_EPYC_7551_64, "MVP_ARCH_AMD_EPYC_7551_64"},
    {MVP_ARCH_AMD_EPYC_7V12_64, "MVP_ARCH_AMD_EPYC_7V12_64"},
    {MVP_ARCH_AMD_EPYC_7763_64, "MVP_ARCH_AMD_EPYC_7763_64"},
    {MVP_ARCH_AMD_EPYC_7763_128, "MVP_ARCH_AMD_EPYC_7763_128"},
    {MVP_ARCH_AMD_EPYC_7601_64, "MVP_ARCH_AMD_EPYC_7601_64"},
    {MVP_ARCH_AMD_EPYC_7643_96, "MVP_ARCH_AMD_EPYC_7643_96"},
    {MVP_ARCH_AMD_EPYC_7742_128, "MVP_ARCH_AMD_EPYC_7742_128"},
    {MVP_ARCH_AMD_EPYC_7662_64, "MVP_ARCH_AMD_EPYC_7662_64"},
    {MVP_ARCH_AMD_EPYC_7A53_64, "MVP_ARCH_AMD_EPYC_7A53_64"},
    {MVP_ARCH_AMD_EPYC_9124_16, "MVP_ARCH_AMD_EPYC_9124_16"},
    {MVP_ARCH_AMD_EPYC_7A53_56, "MVP_ARCH_AMD_EPYC_7A53_56"},

    /* IBM Architectures */
    {MVP_ARCH_IBM_PPC, "MVP_ARCH_IBM_PPC"},
    {MVP_ARCH_IBM_POWER8, "MVP_ARCH_IBM_POWER8"},
    {MVP_ARCH_IBM_POWER9, "MVP_ARCH_IBM_POWER9"},

    /* ARM Architectures */
    {MVP_ARCH_ARM_GENERIC, "MVP_ARCH_ARM_GENERIC"},
    {MVP_ARCH_ARM_CAVIUM_V8_2S_28, "MVP_ARCH_ARM_CAVIUM_V8_2S_28"},
    {MVP_ARCH_ARM_CAVIUM_V8_2S_32, "MVP_ARCH_ARM_CAVIUM_V8_2S_32"},
    {MVP_ARCH_ARM_FUJITSU_V0_4S_48, "MVP_ARCH_ARM_FUJITSU_V0_4S_48"},
    {MVP_ARCH_ARM_GRACE_2S_144, "MVP_ARCH_ARM_GRACE_2S_144"},

    /* Unknown */
    {MVP_ARCH_UNKWN, "MVP_ARCH_UNKWN"},
    {MVP_ARCH_LAST_ENTRY, "MVP_ARCH_LAST_ENTRY"},
};

typedef struct _mvp_cpu_family_types_log_t {
    int family_type;
    char *cpu_family_name;
} mvp_cpu_family_types_log_t;

static mvp_cpu_family_types_log_t mvp_cpu_family_types_log[] = {
    {MVP_CPU_ARCH_MAKE__NONE, "MVP_CPU_ARCH_MAKE__NONE"},
    {MVP_CPU_ARCH_MAKE__INTEL, "MVP_CPU_ARCH_MAKE__INTEL"},
    {MVP_CPU_ARCH_MAKE__AMD, "MVP_CPU_ARCH_MAKE__AMD"},
    {MVP_CPU_ARCH_MAKE__IBM, "MVP_CPU_ARCH_MAKE__IBM"},
    {MVP_CPU_ARCH_MAKE__ARM, "MVP_CPU_ARCH_MAKE__ARM"},
};

char *mvp_get_cpu_family_name(int cpu_family_type)
{
    return mvp_cpu_family_types_log[cpu_family_type].cpu_family_name;
}

char *mvp_get_arch_name(mvp_arch_t arch_type)
{
    int i = 0;
    while (mvp_arch_types_log[i].arch_type != MVP_ARCH_LAST_ENTRY) {
        if (mvp_arch_types_log[i].arch_type == arch_type) {
            return (mvp_arch_types_log[i].arch_name);
        }
        i++;
    }
    return ("MVP_ARCH_UNKWN");
}

int mvp_check_proc_arch(mvp_arch_t type, int rank)
{
    if (type < 0 || type >= MVP_ARCH_LIST_END || type == MVP_ARCH_INTEL_START ||
        type == MVP_ARCH_INTEL_END || type == MVP_ARCH_AMD_START ||
        type == MVP_ARCH_AMD_END || type == MVP_ARCH_IBM_START ||
        type == MVP_ARCH_IBM_END || type == MVP_ARCH_ARM_START ||
        type == MVP_ARCH_ARM_END) {
        PRINT_INFO((rank == 0),
                   "Wrong value specified for MVP_FORCE_ARCH_TYPE\n");
        PRINT_INFO((rank == 0),
                   "Value must be greater than %d and less than %d \n",
                   MVP_ARCH_LIST_START, MVP_ARCH_LIST_END);
        PRINT_INFO((rank == 0),
                   "For Intel Architectures: Please enter value greater than "
                   "%d and less than %d\n",
                   MVP_ARCH_INTEL_START, MVP_ARCH_INTEL_END);
        PRINT_INFO((rank == 0),
                   "For AMD Architectures: Please enter value greater than %d "
                   "and less than %d\n",
                   MVP_ARCH_AMD_START, MVP_ARCH_AMD_END);
        PRINT_INFO((rank == 0),
                   "For IBM Architectures: Please enter value greater than %d "
                   "and less than %d\n",
                   MVP_ARCH_IBM_START, MVP_ARCH_IBM_END);
        PRINT_INFO((rank == 0),
                   "For ARM Architectures: Please enter value greater than %d "
                   "and less than %d\n",
                   MVP_ARCH_ARM_START, MVP_ARCH_ARM_END);
        return 1;
    }
    return 0;
}

mvp_arch_t mvp_get_intel_arch_type(char *model_name, int num_sockets,
                                   int num_cpus)
{
    mvp_arch_t arch_type = MVP_ARCH_UNKWN;
    arch_type = MVP_ARCH_INTEL_GENERIC;

    if (1 == num_sockets) {
        if (64 == num_cpus || 68 == num_cpus || 72 == num_cpus) {
            /* Map all KNL CPUs to 7250 */
            if (NULL != strstr(model_name, INTEL_XEON_PHI_GENERIC_MODEL_NAME)) {
                arch_type = MVP_ARCH_INTEL_XEON_PHI_7250;
            }
        }
    } else if (2 == num_sockets) {
        if (4 == num_cpus) {
            arch_type = MVP_ARCH_INTEL_XEON_DUAL_4;
        } else if (8 == num_cpus) {
            if (CLOVERTOWN_MODEL == g_mvp_cpu_model) {
                arch_type = MVP_ARCH_INTEL_CLOVERTOWN_8;
            } else if (HARPERTOWN_MODEL == g_mvp_cpu_model) {
                arch_type = MVP_ARCH_INTEL_HARPERTOWN_8;
            } else if (NEHALEM_MODEL == g_mvp_cpu_model) {
                arch_type = MVP_ARCH_INTEL_NEHALEM_8;
            } else if (INTEL_E5630_MODEL == g_mvp_cpu_model) {
                arch_type = MVP_ARCH_INTEL_XEON_E5630_8;
            }
        } else if (12 == num_cpus) {
            if (INTEL_X5650_MODEL == g_mvp_cpu_model) {
                /* Westmere EP model, Lonestar */
                arch_type = MVP_ARCH_INTEL_XEON_X5650_12;
            } else if (INTEL_XEON_E5_2670_V2_MODEL == g_mvp_cpu_model) {
                if (NULL != strstr(model_name, INTEL_E5_2630_V2_MODEL_NAME)) {
                    arch_type = MVP_ARCH_INTEL_XEON_E5_2630_V2_2S_12;
                }
            }
        } else if (16 == num_cpus) {
            if (NEHALEM_MODEL == g_mvp_cpu_model) { /* nehalem with smt on */
                arch_type = MVP_ARCH_INTEL_NEHALEM_16;
            } else if (INTEL_E5_2670_MODEL == g_mvp_cpu_model) {
                if (strncmp(model_name, INTEL_E5_2670_MODEL_NAME,
                            strlen(INTEL_E5_2670_MODEL_NAME)) == 0) {
                    arch_type = MVP_ARCH_INTEL_XEON_E5_2670_16;
                } else if (strncmp(model_name, INTEL_E5_2680_MODEL_NAME,
                                   strlen(INTEL_E5_2680_MODEL_NAME)) == 0) {
                    arch_type = MVP_ARCH_INTEL_XEON_E5_2680_16;
                } else {
                    arch_type = MVP_ARCH_INTEL_GENERIC;
                }
            } else if (NULL !=
                       strstr(model_name, INTEL_E5_2620_V4_MODEL_NAME)) {
                arch_type = MVP_ARCH_INTEL_XEON_E5_2620_V4_2S_16;
            }
        } else if (20 == num_cpus) {
            if (INTEL_XEON_E5_2670_V2_MODEL == g_mvp_cpu_model) {
                if (NULL != strstr(model_name, INTEL_E5_2670_V2_MODEL_NAME)) {
                    arch_type = MVP_ARCH_INTEL_XEON_E5_2670_V2_2S_20;
                } else if (NULL !=
                           strstr(model_name, INTEL_E5_2680_V2_MODEL_NAME)) {
                    arch_type = MVP_ARCH_INTEL_XEON_E5_2680_V2_2S_20;
                } else if (NULL !=
                           strstr(model_name, INTEL_E5_2690_V2_MODEL_NAME)) {
                    arch_type = MVP_ARCH_INTEL_XEON_E5_2690_V2_2S_20;
                }
            } else if (NULL !=
                       strstr(model_name, INTEL_E5_2687W_V3_MODEL_NAME)) {
                arch_type = MVP_ARCH_INTEL_XEON_E5_2687W_V3_2S_20;
            } else if (INTEL_XEON_E5_2660_V3_MODEL == g_mvp_cpu_model) {
                if (NULL != strstr(model_name, INTEL_E5_2660_V3_MODEL_NAME)) {
                    arch_type = MVP_ARCH_INTEL_XEON_E5_2660_V3_2S_20;
                }
            }
        } else if (24 == num_cpus) {
            if (NULL != strstr(model_name, INTEL_E5_2680_V3_MODEL_NAME)) {
                arch_type = MVP_ARCH_INTEL_XEON_E5_2680_V3_2S_24;
            } else if (NULL !=
                       strstr(model_name, INTEL_E5_2690_V3_MODEL_NAME)) {
                arch_type = MVP_ARCH_INTEL_XEON_E5_2690_V3_2S_24;
            } else if (NULL !=
                       strstr(model_name, INTEL_E5_2670_V3_MODEL_NAME)) {
                arch_type = MVP_ARCH_INTEL_XEON_E5_2670_V3_2S_24;
            }
        } else if (28 == num_cpus) {
            if (NULL != strstr(model_name, INTEL_E5_2695_V3_MODEL_NAME)) {
                arch_type = MVP_ARCH_INTEL_XEON_E5_2695_V3_2S_28;
            } else if (NULL !=
                       strstr(model_name, INTEL_E5_2680_V4_MODEL_NAME)) {
                arch_type = MVP_ARCH_INTEL_XEON_E5_2680_V4_2S_28;
            } else if (NULL !=
                       strstr(model_name,
                              INTEL_GOLD_GENERIC_MODEL_NAME)) { /* SkL Gold */
                arch_type = MVP_ARCH_INTEL_PLATINUM_8170_2S_52; /* Use generic
                                                                   SKL tables */
            }
        } else if (32 == num_cpus) {
            if (INTEL_XEON_E5_2698_V3_MODEL == g_mvp_cpu_model) {
                if (NULL != strstr(model_name, INTEL_E5_2698_V3_MODEL_NAME)) {
                    arch_type = MVP_ARCH_INTEL_XEON_E5_2698_V3_2S_32;
                }
            }
            /* HPCAC Thor */
            if (NULL != strstr(model_name, INTEL_E5_2697A_V4_MODEL_NAME)) {
                arch_type = MVP_ARCH_INTEL_XEON_E5_2697A_V4_2S_32;
            }
        } else if (36 == num_cpus) {
            if (NULL !=
                strstr(model_name,
                       INTEL_GOLD_6154_MODEL_NAME)) { /* Oracle BM.HPC2 */
                arch_type = MVP_ARCH_INTEL_PLATINUM_8170_2S_52; /* Use generic
                                                                   SKL tables */
            }
            /* Support Pitzer cluster */
        } else if (40 == num_cpus) {
            /* EPFL */
            if (NULL != strstr(model_name, INTEL_GOLD_6248_MODEL_NAME)) {
                arch_type =
                    MVP_ARCH_INTEL_PLATINUM_8280_2S_56; /* Force frontera */
            } else if (NULL !=
                       strstr(model_name,
                              INTEL_GOLD_GENERIC_MODEL_NAME)) { /* SkL Gold */
                arch_type = MVP_ARCH_INTEL_PLATINUM_8170_2S_52; /* Use generic
                                                                   SKL tables */
            }
            /* Pitzer */
            if (NULL != strstr(model_name, INTEL_GOLD_6148_MODEL_NAME)) {
                arch_type = MVP_ARCH_INTEL_GOLD_6148_2S_40;
            }
            /* Detect skylake or cascade lake CPUs */
        } else if (48 == num_cpus || 52 == num_cpus || 56 == num_cpus ||
                   44 == num_cpus /* azure skx */) {
            if (NULL != strstr(model_name, INTEL_PLATINUM_GENERIC_MODEL_NAME)) {
                arch_type = MVP_ARCH_INTEL_PLATINUM_8170_2S_52;
            }
            /* Check if the model is Cascade lake, if yes then change from
             * generic */
            if (NULL != strstr(model_name, INTEL_PLATINUM_8260_MODEL_NAME)) {
                arch_type = MVP_ARCH_INTEL_PLATINUM_8260_2S_48;
            }
            /* Frontera */
            if (NULL != strstr(model_name, INTEL_PLATINUM_8280_MODEL_NAME)) {
                arch_type = MVP_ARCH_INTEL_PLATINUM_8280_2S_56;
            }
            /* Stampede2 */
            if (NULL != strstr(model_name, INTEL_PLATINUM_8160_MODEL_NAME)) {
                arch_type = MVP_ARCH_INTEL_PLATINUM_8160_2S_48;
            }
            /* Pitzer */
            if (NULL != strstr(model_name, INTEL_PLATINUM_8268_MODEL_NAME)) {
                arch_type = MVP_ARCH_INTEL_PLATINUM_8268_2S_48;
            }
        } else if (36 == num_cpus || 72 == num_cpus) {
            if (NULL != strstr(model_name, INTEL_E5_2695_V4_MODEL_NAME)) {
                arch_type = MVP_ARCH_INTEL_XEON_E5_2695_V4_2S_36;
            }
        } else if (80 == num_cpus) {
            if (NULL != strstr(model_name, INTEL_PLATINUM_8380_MODEL_NAME)) {
                arch_type = MVP_ARCH_INTEL_PLATINUM_8380_2S_80;
            }
        } else if (112 == num_cpus) {
            /* Sapphire Rapids */
            if (NULL != strstr(model_name, INTEL_PLATINUM_8480_MODEL_NAME)) {
                arch_type = MVP_ARCH_INTEL_PLATINUM_8480_2S_112;
            }
        }
    }
    /* if not able to determine exact CPU type based on the number of CPU cores,
     * see if we can at least get the model name */
    if (arch_type == MVP_ARCH_INTEL_GENERIC) {
        if (NULL != strstr(model_name, INTEL_XEON_PHI_GENERIC_MODEL_NAME)) {
            arch_type = MVP_ARCH_INTEL_XEON_PHI_7250;
        } else if ((NULL !=
                    strstr(model_name, INTEL_GOLD_GENERIC_MODEL_NAME)) ||
                   (NULL !=
                    strstr(model_name, INTEL_PLATINUM_GENERIC_MODEL_NAME))) {
            arch_type = MVP_ARCH_INTEL_PLATINUM_8280_2S_56;
        }
    }

    return arch_type;
}

/* Identify architecture type */
mvp_arch_t mvp_get_arch_type()
{
    int my_rank = -1;
    char *value = NULL;

    my_rank = MPIR_Process.rank;

    if (MVP_ARCH_UNKWN != MVP_FORCE_ARCH_TYPE) {
        PRINT_DEBUG(DEBUG_INIT_verbose, "Attempting to force ARCH %s\n",
                    mvp_get_arch_name(MVP_FORCE_ARCH_TYPE));
        int retval = mvp_check_proc_arch(MVP_FORCE_ARCH_TYPE, my_rank);
        if (retval) {
            MVP_FORCE_ARCH_TYPE = MVP_ARCH_UNKWN;
            PRINT_INFO((my_rank == 0),
                       "Falling back to automatic architecture detection\n");
        } else {
            /* Set g_mvp_num_cpus to some value to ensure arch_hca_type
             * computation is not affected. Since th value of g_mvp_num_cpus is
             * not used anywhere, it should be fine. */
            g_mvp_num_cpus = MVP_STUB_NUM_CPUS;
            /* Set mvp_cpu_arch_make appropriately when forcing arch */
            if (MVP_FORCE_ARCH_TYPE >= MVP_ARCH_INTEL_START &&
                MVP_FORCE_ARCH_TYPE <= MVP_ARCH_INTEL_END) {
                mvp_cpu_arch_make = MVP_CPU_ARCH_MAKE__INTEL;
            } else if (MVP_FORCE_ARCH_TYPE >= MVP_ARCH_AMD_START &&
                       MVP_FORCE_ARCH_TYPE <= MVP_ARCH_AMD_END) {
                mvp_cpu_arch_make = MVP_CPU_ARCH_MAKE__AMD;
            } else if (MVP_FORCE_ARCH_TYPE >= MVP_ARCH_IBM_START &&
                       MVP_FORCE_ARCH_TYPE <= MVP_ARCH_IBM_END) {
                mvp_cpu_arch_make = MVP_CPU_ARCH_MAKE__IBM;
            } else if (MVP_FORCE_ARCH_TYPE >= MVP_ARCH_ARM_START &&
                       MVP_FORCE_ARCH_TYPE <= MVP_ARCH_ARM_END) {
                mvp_cpu_arch_make = MVP_CPU_ARCH_MAKE__ARM;
            } else {
                mvp_cpu_arch_make = MVP_CPU_ARCH_MAKE__NONE;
            }
        }
    }

    if (MVP_ARCH_UNKWN == MVP_FORCE_ARCH_TYPE) {
        FILE *fp;
        int num_sockets = 0, num_cpus = 0;
        int model_name_set = 0;
        unsigned topodepth = -1, depth = -1;
        char line[MAX_LINE_LENGTH], *tmp, *key;
        char model_name[MAX_NAME_LENGTH] = {0};

        mvp_arch_t arch_type = MVP_ARCH_UNKWN;
        if (smpi_load_hwloc_topology()) {
            return MPI_ERR_INTERN;
        }
        if (smpi_load_hwloc_topology_whole()) {
            return MPI_ERR_INTERN;
        }

        /* Determine topology depth */
        topodepth = hwloc_topology_get_depth(topology);
        if (HWLOC_TYPE_DEPTH_UNKNOWN == topodepth) {
            fprintf(stderr,
                    "Warning: %s: Failed to determine topology depth.\n",
                    __func__);
            return arch_type;
        }

        /* Count number of (logical) processors */
        depth = hwloc_get_type_depth(topology, HWLOC_OBJ_PU);

        if (HWLOC_TYPE_DEPTH_UNKNOWN == depth) {
            fprintf(stderr,
                    "Warning: %s: Failed to determine number of processors.\n",
                    __func__);
            return arch_type;
        }
        if (!(num_cpus =
                  hwloc_get_nbobjs_by_type(topology_whole, HWLOC_OBJ_CORE))) {
            fprintf(stderr,
                    "Warning: %s: Failed to determine number of processors.\n",
                    __func__);
            return arch_type;
        }
        g_mvp_num_cpus = num_cpus;

        /* Count number of sockets */
        depth = hwloc_get_type_depth(topology, HWLOC_OBJ_SOCKET);
        if (HWLOC_TYPE_DEPTH_UNKNOWN == depth) {
            fprintf(stderr,
                    "Warning: %s: Failed to determine number of sockets.\n",
                    __func__);
            return arch_type;
        } else {
            num_sockets = hwloc_get_nbobjs_by_depth(topology, depth);
        }

        /* Parse /proc/cpuinfo for additional useful things */
        if ((fp = fopen(CONFIG_FILE, "r"))) {
            while (!feof(fp)) {
                memset(line, 0, MAX_LINE_LENGTH);
                fgets(line, MAX_LINE_LENGTH - 1, fp);

                if (!(key = strtok(line, "\t:"))) {
                    continue;
                }

                /* Identify the CPU Family */
                if (!strcmp(key, MVP_STR_VENDOR_ID)) {
                    strtok(NULL, MVP_STR_WS);
                    tmp = strtok(NULL, MVP_STR_WS);

                    if (!strncmp(tmp, MVP_STR_AUTH_AMD,
                                 strlen(MVP_STR_AUTH_AMD))) {
                        mvp_cpu_arch_make = MVP_CPU_ARCH_MAKE__AMD;

                    } else {
                        mvp_cpu_arch_make = MVP_CPU_ARCH_MAKE__INTEL;
                    }
                    continue;
                }

                /* Identify the CPU Family for POWER */
                if (!strcmp(key, "cpu")) {
                    strtok(NULL, MVP_STR_WS);
                    tmp = strtok(NULL, MVP_STR_WS);
                    if (!strncmp(tmp, MVP_STR_POWER8_ID,
                                 strlen(MVP_STR_POWER8_ID))) {
                        mvp_cpu_arch_make = MVP_CPU_ARCH_MAKE__IBM;
                        arch_type = MVP_ARCH_IBM_POWER8;
                        continue;
                    } else if (!strncmp(tmp, MVP_STR_POWER9_ID,
                                        strlen(MVP_STR_POWER9_ID))) {
                        mvp_cpu_arch_make = MVP_CPU_ARCH_MAKE__IBM;
                        arch_type = MVP_ARCH_IBM_POWER9;
                        continue;
                    }
                }

                /* Identify the CPU Family for ARM */
                if (!strcmp(key, "CPU implementer")) {
                    /* Skip ':' */
                    strtok(NULL, MVP_STR_WS);
                    tmp = strtok(NULL, MVP_STR_WS);
                    if ((!strncmp(tmp, MVP_STR_CAVIUM_ID,
                                  strlen(MVP_STR_CAVIUM_ID))) ||
                        (!strncmp(tmp, MVP_STR_FUJITSU_ID,
                                  strlen(MVP_STR_FUJITSU_ID))) ||
                        (!strncmp(tmp, MVP_STR_GRACE_ID,
                                  strlen(MVP_STR_GRACE_ID)))) {
                        mvp_cpu_arch_make = MVP_CPU_ARCH_MAKE__ARM;
                        arch_type = MVP_ARCH_ARM_GENERIC;
                        if (num_cpus == 48) {
                            arch_type = MVP_ARCH_ARM_FUJITSU_V0_4S_48;
                            g_mvp_cpu_model = MVP_ARM_FUJITSU_V0_MODEL;
                        } else if (num_cpus == 56) {
                            arch_type = MVP_ARCH_ARM_CAVIUM_V8_2S_28;
                            g_mvp_cpu_model = MVP_ARM_CAVIUM_V8_MODEL;
                        } else if (num_cpus == 64) {
                            arch_type = MVP_ARCH_ARM_CAVIUM_V8_2S_32;
                            g_mvp_cpu_model = MVP_ARM_CAVIUM_V8_MODEL;
                        } else if (72 == num_cpus || 144 == num_cpus) {
                            arch_type = MVP_ARCH_ARM_GRACE_2S_144;
                            g_mvp_cpu_model = MVP_ARM_GRACE_MODEL;
                        }
                        continue;
                    }
                }

                if (-1 == g_mvp_cpu_model) {
                    if (!strcmp(key, MVP_STR_MODEL)) {
                        strtok(NULL, MVP_STR_WS);
                        tmp = strtok(NULL, MVP_STR_WS);
                        sscanf(tmp, "%d", &g_mvp_cpu_model);
                        continue;
                    }
                }

                if (!model_name_set) {
                    if (strncmp(key, MVP_STR_MODEL_NAME,
                                strlen(MVP_STR_MODEL_NAME)) == 0) {
                        strtok(NULL, MVP_STR_WS);
                        tmp = strtok(NULL, "\n");
                        sscanf(tmp, "%[^\n]\n", model_name);
                        model_name_set = 1;
                    }
                }
            }
            fclose(fp);

            if (MVP_CPU_ARCH_MAKE__INTEL == mvp_cpu_arch_make) {
                arch_type =
                    mvp_get_intel_arch_type(model_name, num_sockets, num_cpus);
            } else if (MVP_CPU_ARCH_MAKE__AMD == mvp_cpu_arch_make) {
                if (NULL != strstr(model_name, AMD_EPYC_GENERAL_MODEL_NAME)) {
                    arch_type = MVP_ARCH_AMD_EPYC_GENERIC;
                } else {
                    arch_type = MVP_ARCH_AMD_GENERIC;
                }
                if (1 == num_sockets) {
                    if (16 == num_cpus) {
                        if (NULL !=
                            strstr(model_name, AMD_EPYC_9124_MODEL_NAME)) {
                            arch_type = MVP_ARCH_AMD_EPYC_9124_16;
                        }
                    } else if (56 == num_cpus) { /* Frontier */
                        if (NULL !=
                            strstr(model_name, AMD_EPYC_7A53_MODEL_NAME)) {
                            arch_type = MVP_ARCH_AMD_EPYC_7A53_56;
                        }
                    } else if (64 == num_cpus) { /* Spock */
                        if (NULL !=
                            strstr(model_name, AMD_EPYC_7662_MODEL_NAME)) {
                            arch_type = MVP_ARCH_AMD_EPYC_7662_64;
                        } else if (NULL != strstr(model_name,
                                                  AMD_EPYC_7A53_MODEL_NAME)) {
                            arch_type = MVP_ARCH_AMD_EPYC_7A53_64;
                        }
                    }
                }
                if (2 == num_sockets) {
                    if (4 == num_cpus) {
                        arch_type = MVP_ARCH_AMD_OPTERON_DUAL_4;
                    } else if (16 == num_cpus) {
                        arch_type = MVP_ARCH_AMD_BULLDOZER_4274HE_16;
                    } else if (24 == num_cpus) {
                        arch_type = MVP_ARCH_AMD_MAGNY_COURS_24;
                    } else if (48 == num_cpus) {
                        arch_type = MVP_ARCH_AMD_EPYC_7401_48;
                    } else if (64 == num_cpus || 60 == num_cpus ||
                               120 == num_cpus /* azure vm */) {
                        if (NULL !=
                            strstr(model_name, AMD_EPYC_7601_MODEL_NAME)) {
                            arch_type = MVP_ARCH_AMD_EPYC_7601_64;
                        } else if (NULL != strstr(model_name,
                                                  AMD_EPYC_7V12_MODEL_NAME)) {
                            arch_type = MVP_ARCH_AMD_EPYC_7V12_64;
                        } else {
                            arch_type = MVP_ARCH_AMD_EPYC_7551_64;
                        }
                    } else if (96 == num_cpus) {
                        if (NULL !=
                            strstr(model_name, AMD_EPYC_7643_MODEL_NAME)) {
                            arch_type = MVP_ARCH_AMD_EPYC_7643_96;
                        }
                    } else if (128 == num_cpus) { /* expanse and ls6 */
                        if (NULL !=
                                strstr(model_name, AMD_EPYC_7763_MODEL_NAME) ||
                            NULL !=
                                strstr(model_name, AMD_EPYC_7713_MODEL_NAME)) {
                            /* lonestar6 */
                            arch_type = MVP_ARCH_AMD_EPYC_7763_128;
                        } else {
                            /* expanse */
                            arch_type = MVP_ARCH_AMD_EPYC_7742_128;
                        }
                    }
                    /* If we could not exactly find out what exact architecture
                     * was, fall back toMVP_ARCH_AMD_EPYC_7401_48 */
                    if (arch_type == MVP_ARCH_AMD_EPYC_GENERIC) {
                        arch_type = MVP_ARCH_AMD_EPYC_7401_48;
                    }
                } else if (4 == num_sockets) {
                    if (16 == num_cpus) {
                        arch_type = MVP_ARCH_AMD_BARCELONA_16;
                    } else if (32 == num_cpus) {
                        arch_type = MVP_ARCH_AMD_OPTERON_6136_32;
                    } else if (64 == num_cpus) {
                        arch_type = MVP_ARCH_AMD_OPTERON_6276_64;
                    }
                }
            }
        } else {
            fprintf(stderr, "Warning: %s: Failed to open \"%s\".\n", __func__,
                    CONFIG_FILE);
        }
        MVP_FORCE_ARCH_TYPE = arch_type;
        if (MVP_ARCH_UNKWN == MVP_FORCE_ARCH_TYPE) {
            PRINT_INFO(
                (my_rank == 0),
                "**********************WARNING***********************\n");
            PRINT_INFO(
                (my_rank == 0),
                "Failed to automatically detect the CPU architecture.\n");
            PRINT_INFO((my_rank == 0),
                       "This may lead to subpar communication performance.\n");
            PRINT_INFO(
                (my_rank == 0),
                "****************************************************\n");
        }
        return arch_type;
    } else {
        return MVP_FORCE_ARCH_TYPE;
    }
}

/* API for getting the number of cpus */
int mvp_get_num_cpus()
{
    /* Check if num_cores is already identified */
    if (-1 == g_mvp_num_cpus) {
        MVP_FORCE_ARCH_TYPE = mvp_get_arch_type();
    }
    return g_mvp_num_cpus;
}

/* API for getting the CPU model */
int mvp_get_cpu_model()
{
    /* Check if cpu model is already identified */
    if (-1 == g_mvp_cpu_model) {
        MVP_FORCE_ARCH_TYPE = mvp_get_arch_type();
    }
    return g_mvp_cpu_model;
}

/* Get CPU family */
int mvp_get_cpu_family()
{
    /* Check if cpu family is already identified */
    if (MVP_CPU_ARCH_MAKE__NONE == mvp_cpu_arch_make) {
        MVP_FORCE_ARCH_TYPE = mvp_get_arch_type();
    }
    return mvp_cpu_arch_make;
}

/* Check arch-hca type */
int mvp_is_arch_hca_type(mvp_arch_hca_info_t arch_hca_info,
                         mvp_arch_t arch_type, mvp_hca_t hca_type)
{
    int ret;
    uint16_t my_arch_type, my_hca_type;
    uint64_t mask = UINT16_MAX;
    uint32_t arch_hca_type = arch_hca_info.parts.arch_hca_id.full;
    arch_hca_type >>= 16;
    my_hca_type = arch_hca_type & mask;
    arch_hca_type >>= 16;
    my_arch_type = arch_hca_type & mask;

    table_arch_tmp = my_arch_type;
    table_hca_tmp = my_hca_type;

    if (((MVP_ARCH_ANY == arch_type) || (MVP_ARCH_ANY == my_arch_type)) &&
        ((MVP_HCA_ANY == hca_type) || (MVP_HCA_ANY == my_hca_type))) {
        ret = 1;
    } else if (MVP_ARCH_ANY == arch_type) { // cores
        ret = (my_hca_type == hca_type) ? 1 : 0;
    } else if ((MVP_HCA_ANY == hca_type) || (MVP_HCA_ANY == my_hca_type)) {
        ret = (my_arch_type == arch_type) ? 1 : 0;
    } else {
        ret = (my_arch_type == arch_type && my_hca_type == hca_type) ? 1 : 0;
    }
    return ret;
}
#if defined(_SMP_LIMIC_)
void hwlocSocketDetection(int print_details)
{
    int depth;
    unsigned i, j;
    char *str;
    char *pEnd;
    char *pch;
    int more32bit = 0, offset = 0;
    long int core_cnt[2];
    hwloc_cpuset_t cpuset;
    hwloc_obj_t sockets;

    /* Perform the topology detection. */
    if (smpi_load_hwloc_topology()) {
        return;
    }
    /*clear all the socket information and reset to -1*/
    for (i = 0; i < SOCKETS; i++)
        for (j = 0; j < CORES; j++)
            node[i][j] = -1;

    depth = hwloc_get_type_depth(topology, HWLOC_OBJ_SOCKET);
    no_sockets = hwloc_get_nbobjs_by_depth(topology, depth);

    PRINT_DEBUG(DEBUG_SHM_verbose > 0, "Total number of sockets=%d\n",
                no_sockets);

    for (i = 0; i < no_sockets; i++) {
        sockets = hwloc_get_obj_by_type(topology, HWLOC_OBJ_SOCKET, i);
        cpuset = sockets->cpuset;
        hwloc_bitmap_asprintf(&str, cpuset);

        /*tokenize the str*/
        pch = strtok(str, ",");
        while (pch != NULL) {
            pch = strtok(NULL, ",");
            if (pch != NULL) {
                more32bit = 1;
                break;
            }
        }

        core_cnt[0] = strtol(str, &pEnd, HEX_FORMAT);
        /*if more than bits, then explore the values*/
        if (more32bit) {
            /*tells multiple of 32 bits(eg if 0, then 64 bits)*/
            core_cnt[1] = strtol(pEnd, NULL, 0);
            offset = (core_cnt[1] + 1) * CORES_REP_AS_BITS;
        }

        for (j = 0; j < CORES_REP_AS_BITS; j++) {
            if (core_cnt[0] & 1) {
                node[i][j] = j + offset;
                (numcores_persocket[i])++;
            }
            core_cnt[0] = (core_cnt[0] >> 1);
        }

        if (DEBUG_SHM_verbose > 0) {
            printf("Socket %d, num of cores / socket=%d\n", i,
                   (numcores_persocket[i]));
            printf("core id\n");

            for (j = 0; j < CORES_REP_AS_BITS; j++) {
                printf("%d\t", node[i][j]);
            }
            printf("\n");
        }
    }
    MPL_free(str);
}

// Check the core, where the process is bound to
int getProcessBinding(pid_t pid)
{
    int res, i = 0, j = 0;
    char *str = NULL;
    char *pEnd = NULL;
    char *pch = NULL;
    int more32bit = 0, offset = 0;
    unsigned int core_bind[2];
    hwloc_bitmap_t cpubind_set;

    /* Perform the topology detection. */
    if (smpi_load_hwloc_topology()) {
        return MPI_ERR_INTERN;
    }
    cpubind_set = hwloc_bitmap_alloc();
    res = hwloc_get_proc_cpubind(topology, pid, cpubind_set, 0);
    if (-1 == res)
        fprintf(stderr,
                "getProcessBinding(): Error in getting cpubinding of process");

    hwloc_bitmap_asprintf(&str, cpubind_set);

    /*tokenize the str*/
    pch = strtok(str, ",");
    while (pch != NULL) {
        pch = strtok(NULL, ",");
        if (pch != NULL) {
            more32bit = 1;
            break;
        }
    }

    core_bind[0] = strtol(str, &pEnd, HEX_FORMAT);

    /*if more than bits, then explore the values*/
    if (more32bit) {
        /*tells multiple of 32 bits(eg if 0, then 64 bits)*/
        PRINT_DEBUG(DEBUG_SHM_verbose > 0, "more bits set\n");
        core_bind[1] = strtol(pEnd, NULL, 0);
        PRINT_DEBUG(DEBUG_SHM_verbose > 0, "core_bind[1]=%x\n", core_bind[1]);
        offset = (core_bind[1] + 1) * CORES_REP_AS_BITS;
        PRINT_DEBUG(DEBUG_SHM_verbose > 0, "Offset=%d\n", offset);
    }

    for (j = 0; j < CORES_REP_AS_BITS; j++) {
        if (core_bind[0] & 1) {
            core_bind[0] = j + offset;
            break;
        }
        core_bind[0] = (core_bind[0] >> 1);
    }

    /*find the socket, where the core is present*/
    for (i = 0; i < no_sockets; i++) {
        j = core_bind[0] - offset;
        if (node[i][j] == j + offset) {
            MPL_free(str);
            hwloc_bitmap_free(cpubind_set);
            return i; /*index of socket where the process is bound*/
        }
    }
    fprintf(stderr, "Error: Process not bound on any core ??\n");
    MPL_free(str);
    hwloc_bitmap_free(cpubind_set);
    return -1;
}

int numOfCoresPerSocket(int socket) { return numcores_persocket[socket]; }

int numofSocketsPerNode(void) { return no_sockets; }

int get_socket_bound(void)
{
    if (socket_bound == -1) {
        socket_bound = getProcessBinding(getpid());
    }
    return socket_bound;
}
#else
void hwlocSocketDetection(int print_details) {}
int numOfCoresPerSocket(int socket) { return 0; }
int numofSocketsPerNode(void) { return 0; }
int get_socket_bound(void) { return -1; }
#endif /*#if defined(_SMP_LIMIC_)*/

/* return a number with the value'th bit set */
int find_bit_pos(int value)
{
    int pos = 1, tmp = 1;
    if (value == 0)
        return 0;
    while (tmp < value) {
        pos++;
        tmp = tmp << 1;
    }
    return pos;
}

/* given a socket object, find the number of cores per socket.
 *  * This could be useful on systems where cores-per-socket
 *   * are not uniform */
int get_core_count_per_socket(hwloc_topology_t topology, hwloc_obj_t obj,
                              int depth)
{
    int i, count = 0;
    if (obj->type == HWLOC_OBJ_CORE)
        return 1;

    for (i = 0; i < obj->arity; i++) {
        count +=
            get_core_count_per_socket(topology, obj->children[i], depth + 1);
    }
    return count;
}

int get_socket_bound_info(int *socket_bound, int *num_sockets,
                          int *num_cores_socket, int *is_uniform)
{
    hwloc_cpuset_t cpuset;
    hwloc_obj_t socket;
    int i, num_cores;
    int err = -1;
    if (smpi_load_hwloc_topology_whole()) {
        return err;
    }
    *num_sockets = hwloc_get_nbobjs_by_type(topology_whole, HWLOC_OBJ_SOCKET);
    num_cores =
        hwloc_bitmap_weight(hwloc_topology_get_allowed_cpuset(topology_whole));
    pid_t pid = getpid();

    hwloc_bitmap_t cpubind_set = hwloc_bitmap_alloc();

    int result = hwloc_get_proc_cpubind(topology_whole, pid, cpubind_set, 0);
    if (result == -1) {
        PRINT_DEBUG(DEBUG_SHM_verbose > 0,
                    "Error in getting cpubinding of process\n");
        return -1;
    }

    *is_uniform = 1;
    int num_valid_sockets = 0;
    hwloc_cpuset_t allowed_cpuset = hwloc_bitmap_alloc();

    for (i = 0; i < *num_sockets; i++) {
        socket = hwloc_get_obj_by_type(topology_whole, HWLOC_OBJ_SOCKET, i);
        cpuset = socket->cpuset;
        hwloc_bitmap_zero(allowed_cpuset);
        hwloc_bitmap_and(allowed_cpuset, cpuset,
                         hwloc_topology_get_allowed_cpuset(topology_whole));
        int num_cores_in_socket = hwloc_bitmap_weight(allowed_cpuset);
        if (num_cores_in_socket != 0) {
            num_valid_sockets++;
        }
    }

    for (i = 0; i < *num_sockets; i++) {
        hwloc_bitmap_zero(allowed_cpuset);
        socket = hwloc_get_obj_by_type(topology_whole, HWLOC_OBJ_SOCKET, i);
        cpuset = socket->cpuset;
        hwloc_bitmap_and(allowed_cpuset, cpuset,
                         hwloc_topology_get_allowed_cpuset(topology_whole));
        int num_cores_in_socket = hwloc_bitmap_weight(allowed_cpuset);
        if (num_cores_in_socket != 0) {
            if (num_cores_in_socket != (num_cores / (num_valid_sockets))) {
                *is_uniform = 0;
            }

            hwloc_bitmap_t result_set = hwloc_bitmap_alloc();
            hwloc_bitmap_and(result_set, cpuset, cpubind_set);
            if (hwloc_bitmap_last(result_set) != -1) {
                *num_cores_socket = num_cores_in_socket;
                *socket_bound = i;
                PRINT_DEBUG(
                    DEBUG_SHM_verbose > 0,
                    "Socket : %d Num cores :%d"
                    " Num cores in socket: %d Num sockets : %d Uniform :%d"
                    "\n",
                    i, num_cores, *num_cores_socket, num_valid_sockets,
                    *is_uniform);
                err = 0;
            }
            hwloc_bitmap_free(result_set);
        }
    }
    *num_sockets = num_valid_sockets;
    // Socket aware collectives don't support non-uniform architectures yet
    if (!*is_uniform) {
        err = 1;
    }
    hwloc_bitmap_free(allowed_cpuset);
    hwloc_bitmap_free(cpubind_set);
    return err;
}

int get_numa_bound_info(int *numa_bound, int *num_numas, int *num_cores_numa,
                        int *is_uniform)
{
    hwloc_cpuset_t cpuset;
    hwloc_obj_t numa;
    int i, num_cores;
    int err = -1;
    if (smpi_load_hwloc_topology_whole()) {
        return err;
    }
    *num_numas = hwloc_get_nbobjs_by_type(topology_whole, HWLOC_OBJ_NUMANODE);
    num_cores =
        hwloc_bitmap_weight(hwloc_topology_get_allowed_cpuset(topology_whole));
    pid_t pid = getpid();

    hwloc_bitmap_t cpubind_set = hwloc_bitmap_alloc();

    int result = hwloc_get_proc_cpubind(topology_whole, pid, cpubind_set, 0);
    if (result == -1) {
        PRINT_DEBUG(DEBUG_SHM_verbose > 0,
                    "Error in getting cpubinding of process\n");
        return -1;
    }

    *is_uniform = 1;
    int num_valid_numas = 0;
    hwloc_cpuset_t allowed_cpuset = hwloc_bitmap_alloc();

    for (i = 0; i < *num_numas; i++) {
        numa = hwloc_get_obj_by_type(topology_whole, HWLOC_OBJ_NUMANODE, i);
        cpuset = numa->cpuset;
        hwloc_bitmap_zero(allowed_cpuset);
        hwloc_bitmap_and(allowed_cpuset, cpuset,
                         hwloc_topology_get_allowed_cpuset(topology_whole));
        int num_cores_in_numa = hwloc_bitmap_weight(allowed_cpuset);
        if (num_cores_in_numa != 0) {
            num_valid_numas++;
        }
    }

    for (i = 0; i < *num_numas; i++) {
        hwloc_bitmap_zero(allowed_cpuset);
        numa = hwloc_get_obj_by_type(topology_whole, HWLOC_OBJ_NUMANODE, i);
        cpuset = numa->cpuset;
        hwloc_bitmap_and(allowed_cpuset, cpuset,
                         hwloc_topology_get_allowed_cpuset(topology_whole));
        int num_cores_in_numa = hwloc_bitmap_weight(allowed_cpuset);
        if (num_cores_in_numa != 0) {
            if (num_cores_in_numa != (num_cores / (num_valid_numas))) {
                *is_uniform = 0;
            }

            hwloc_bitmap_t result_set = hwloc_bitmap_alloc();
            hwloc_bitmap_and(result_set, cpuset, cpubind_set);
            if (hwloc_bitmap_last(result_set) != -1) {
                *num_cores_numa = num_cores_in_numa;
                *numa_bound = i;
                PRINT_DEBUG(DEBUG_SHM_verbose > 0,
                            "Socket : %d Num cores :%d"
                            " Num cores in numa: %d Num numas : %d Uniform :%d"
                            "\n",
                            i, num_cores, *num_cores_numa, num_valid_numas,
                            *is_uniform);
                err = 0;
            }
            hwloc_bitmap_free(result_set);
        }
    }
    *num_numas = num_valid_numas;
    // Socket aware collectives don't support non-uniform architectures yet
    if (!*is_uniform) {
        err = 1;
    }
    hwloc_bitmap_free(allowed_cpuset);
    hwloc_bitmap_free(cpubind_set);
    return err;
}

mvp_arch_hca_info_t MVP_get_arch_hca_type()
{
    mvp_arch_hca_info_t *proc = &my_arch_info;
    if (!proc->arch_hca_info) {
        proc->parts.arch_hca_id.parts.arch_type = mvp_get_arch_type();
        proc->parts.arch_hca_id.parts.hca_type = MVP_HCA_ANY;
        proc->parts.cores = g_mvp_num_cpus;
        proc->parts.reserved = 0x0;
    }

    return *proc;
}
