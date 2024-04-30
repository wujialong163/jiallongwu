/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda. For detailed
 * copyright and licensing information, please refer to the copyright file
 * COPYRIGHT in the top level MVAPICH directory.
 */

#include "mvp_smp_impl.h"
#include "mvp_smp_params.h"
#include "mvp_pkt.h"

#if defined(_SMP_CMA_)
size_t MVP_CMA_MSG_LIMIT = 1 << 30;
#endif

int mvp_set_smp_tuning_parameters()
{
    mvp_arch_t arch_type = mvp_get_arch_type();

    switch (arch_type) {
        case MVP_ARCH_INTEL_XEON_E5630_8:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 8192);
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 8192);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 65536);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 262144);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 256);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 8192);
            break;
        case MVP_ARCH_INTEL_GENERIC:
        case MVP_ARCH_INTEL_CLOVERTOWN_8:
        case MVP_ARCH_INTEL_XEON_DUAL_4:
#if defined(_SMP_LIMIC_)
            if (MVP_SMP_USE_LIMIC2) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 8192);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 65536);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 262144);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 256);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 8192);
            break;

        case MVP_ARCH_INTEL_HARPERTOWN_8:
#if defined(_SMP_LIMIC_)
            if (MVP_SMP_USE_LIMIC2) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 8192);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 32768);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 262144);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 256);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 8192);
            break;

        case MVP_ARCH_INTEL_NEHALEM_8:
        case MVP_ARCH_INTEL_NEHALEM_16:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 32768);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 65536);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 262144);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 256);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 8192);
            break;

        case MVP_ARCH_AMD_BARCELONA_16:
#if defined(_SMP_LIMIC_)
            if (MVP_SMP_USE_LIMIC2) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 8192);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 32768);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 131072);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 32);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 32768);
            break;

        case MVP_ARCH_AMD_EPYC_7551_64:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 16384);
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 16384);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 16384);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 65536);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 32);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 8192);
            break;

        case MVP_ARCH_AMD_EPYC_7643_96:
            MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 8192);
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 262144);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 8);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 131072);
            break;

        /* TODO: tune single socket millan */
        case MVP_ARCH_AMD_EPYC_7763_128:
            /* lonestar6 */
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 8192);
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 16384);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 28672);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 131072);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 32);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 8192);
            break;

        case MVP_ARCH_AMD_EPYC_7601_64:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 16384);
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 16384);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 16384);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 65536);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 32);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 32768);
            break;

        case MVP_ARCH_AMD_EPYC_7V12_64:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 16384);
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 16384);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 4096);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 128 * 1024);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 32);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 32768);
            break;

        case MVP_ARCH_AMD_EPYC_7401_48:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 16384);
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 16384);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 4096);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 65536);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 16);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 30 * 1024);
            break;

        case MVP_ARCH_AMD_EPYC_7662_64:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 1048576);
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 1048576);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 1048576);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 4194304);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 32);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 16384);
            break;

        case MVP_ARCH_AMD_EPYC_7742_128:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 16384);
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 16384);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 16384);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 65536);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 32);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 16384);
            break;

        case MVP_ARCH_AMD_EPYC_9124_16:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 8192);
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 8192);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 8192);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 262144);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 16);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 34816);
            break;

        case MVP_ARCH_AMD_OPTERON_DUAL_4:
        case MVP_ARCH_AMD_GENERIC:
            MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 5120);
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 65536);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 32);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 8192);
            break;

        case MVP_ARCH_AMD_MAGNY_COURS_24:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 16384);
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 8192);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 5120);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 65536);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 64);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 32768);
            break;

        case MVP_ARCH_AMD_OPTERON_6136_32:
#if defined(_SMP_LIMIC_)
            if (MVP_SMP_USE_LIMIC2) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 8192);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 16384);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 65536);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 32);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 32768);
            break;

        case MVP_ARCH_AMD_OPTERON_6276_64:
#if defined(_SMP_LIMIC_)
            if (MVP_SMP_USE_LIMIC2) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 8192);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 16384);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 65536);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 128);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 8192);
            break;

        case MVP_ARCH_AMD_EPYC_7A53_64:
            MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 5120);
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 131072);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 128);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 32768);
            break;
        case MVP_ARCH_AMD_EPYC_7A53_56:
            MVP_SMP_EAGERSIZE = 8192;
            MVP_SMP_QUEUE_LENGTH = 131072;
            MVP_SMP_NUM_SEND_BUFFER = 16;
            MVP_SMP_SEND_BUF_SIZE = 16384;
            break;
        case MVP_ARCH_INTEL_XEON_X5650_12:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 32768);
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 8192);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 65536);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 262144);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 32);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 16384);
            break;

        case MVP_ARCH_INTEL_XEON_E5_2670_16:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 32768);
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 8192);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 32768);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 131072);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 16);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 16384);
            break;

        case MVP_ARCH_INTEL_XEON_E5_2620_V4_2S_16:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 8192);
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 8192);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 32768);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 524288);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 16);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 32768);
            break;

        case MVP_ARCH_INTEL_XEON_E5_2680_16:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 8192);
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 8192);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 32768);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 524288);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 16);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 14336);
            break;

        case MVP_ARCH_INTEL_XEON_E5_2670_V2_2S_20:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 8192);
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 8192);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 32768);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 131072);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 64);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 65536);
            break;

        case MVP_ARCH_ARM_CAVIUM_V8_2S_28:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 8192);
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 8192);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 32768);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 262144);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 16);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 32768);
            break;

        case MVP_ARCH_ARM_CAVIUM_V8_2S_32:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 8192);
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 8192);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 16384);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 393216);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 16);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 32768);
            break;

        case MVP_ARCH_ARM_FUJITSU_V0_4S_48:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 16384);
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 8192);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 16384);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 393216);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 16);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 32768);
            break;

        case MVP_ARCH_ARM_GRACE_2S_144:
            MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 32768);
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 131072);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 128);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 8192);
            break;

        case MVP_ARCH_INTEL_XEON_E5_2670_V3_2S_24:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 32768);
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 32768);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 32768);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 131072);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 8);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 32768);
            break;

        case MVP_ARCH_INTEL_XEON_E5_2698_V3_2S_32:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 16384);
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 16384);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 32768);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 131072);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 8);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 8192);
            break;

        case MVP_ARCH_INTEL_XEON_E5_2680_V3_2S_24:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 32768);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 32768);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 131072);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 32);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 8192);
            break;

        case MVP_ARCH_INTEL_XEON_PHI_7250:
#if defined(_SMP_CMA_)
            /* Use CMA from 2 ppn onwards */
            if (MPIR_Process.local_size <= 2) {
                MVP_SMP_USE_CMA = 0;
            }
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 65536);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 65536);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 262144);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 16);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 131072);
            break;

        case MVP_ARCH_INTEL_XEON_E5_2690_V3_2S_24:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 32768);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 65536);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 262144);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 64);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 8192);
            break;

        case MVP_ARCH_INTEL_XEON_E5_2690_V2_2S_20:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 131072);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 65536);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 524288);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 48);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 32768);
            break;

        case MVP_ARCH_INTEL_XEON_E5_2687W_V3_2S_20:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 32768);
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 32768);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 32768);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 131072);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 8);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 8192);
#if defined(_SMP_CMA_)
            if (MVP_ENABLE_EAGER_THRESHOLD_REDUCTION && MVP_SMP_USE_CMA &&
                10 < MPIR_Process.local_size) {
                /* if there are large number of processes per node, then
                 * reduce the eager threshold and queue length */
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 5120);
                MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 32768);
            }
#endif
            break;

        case MVP_ARCH_IBM_POWER8:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 32768);
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 32768);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 65536);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 524288);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 8);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 32768);
            break;

        case MVP_ARCH_IBM_POWER9:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 65536);
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 32768);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 32768);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 262144);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 32);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 32768);
            break;

        case MVP_ARCH_INTEL_XEON_E5_2680_V2_2S_20:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 32768);
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 32768);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 65536);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 524288);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 48);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 32768);
            break;

        case MVP_ARCH_INTEL_XEON_E5_2680_V4_2S_28:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 5120);
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 65536);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 65536);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 262144);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 8);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 32768);
#if defined(_SMP_CMA_)
            if (MVP_ENABLE_EAGER_THRESHOLD_REDUCTION && MVP_SMP_USE_CMA &&
                14 < MPIR_Process.local_size) {
                /* if there are large number of processes per node, then
                 * reduce the eager threshold and queue length */
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 5120);
                MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 32768);
            }
#endif
            break;

        case MVP_ARCH_INTEL_XEON_E5_2660_V3_2S_20:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 16384);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 32768);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 262144);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 8);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 8192);
            break;

        case MVP_ARCH_INTEL_XEON_E5_2630_V2_2S_12:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 131072);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 65536);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 1048576);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 8);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 65536);
            break;

        case MVP_ARCH_INTEL_XEON_E5_2695_V3_2S_28:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 65536);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 65536);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 262144);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 16);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 8192);
            break;

        case MVP_ARCH_INTEL_XEON_E5_2695_V4_2S_36:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 5120);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 65536);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 262144);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 32);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 8192);
            break;

        case MVP_ARCH_INTEL_XEON_E5_2697A_V4_2S_32:
            MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 65536);
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 262144);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 256);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 8192);
            break;

        case MVP_ARCH_INTEL_PLATINUM_8160_2S_48:
            MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 8192);
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 262144);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 8);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 4096);
            break;

        case MVP_ARCH_INTEL_PLATINUM_8170_2S_52:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 5120);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 65536);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 262144);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 32);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 8192);
            break;

        case MVP_ARCH_INTEL_PLATINUM_8268_2S_48:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 5120);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 65536);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 262144);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 32);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 8192);
            break;

        case MVP_ARCH_INTEL_GOLD_6148_2S_40:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 5120);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 65536);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 65536);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 32);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 16384);
            break;

        case MVP_ARCH_INTEL_PLATINUM_8280_2S_56:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 8192);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 8192);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 524288);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 32);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 8192);
            break;
        case MVP_ARCH_INTEL_PLATINUM_8380_2S_80:
            MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 4096);
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 131072);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 32);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 32768);
            break;
        case MVP_ARCH_AMD_BULLDOZER_4274HE_16:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 32768);
                MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 131072);
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 8192);
                MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 65536);
            } else
#endif
            {
                MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 16384);
                MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 65536);
            }
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 16);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 8192);
            break;

        default:
            MVP_CVAR_SOFT_SET(MVP_SMP_EAGERSIZE, 16384);
            MVP_CVAR_SOFT_SET(MVP_SMP_QUEUE_LENGTH, 65536);
            MVP_CVAR_SOFT_SET(MVP_SMP_NUM_SEND_BUFFER, 128);
            MVP_CVAR_SOFT_SET(MVP_SMP_SEND_BUF_SIZE, 8192);
            break;
    }

    int use_cma, use_limic;
    use_cma = use_limic = 0;
#if defined(_SMP_CMA_)
    use_cma = MVP_SMP_USE_CMA;
#endif
#if defined(_SMP_LIMIC_)
    use_limic = MVP_SMP_USE_LIMIC2;
#endif
    if (use_limic || !use_cma) {
        MVP_SMP_RNDV_PROTOCOL = MVP_SMP_RNDV_PROTOCOL_R3;
    }

    return 0;
}
