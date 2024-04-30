/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 *
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda.
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level MVAPICH directory.
 */

/* This is an XMacro header definition */

#ifndef _MVP_TUNING_TABLES_H_
#define _MVP_TUNING_TABLES_H_

/*
 * MVP_ARCH_TUNING_LIST
 *
 * This XMacro will add the entire supported list of architecture/HCA
 * combinations and their associated tuning tables as a case statement for a
 * particular collective.
 *
 * Requires COLLECTIVE and NET are defined to the collective (all lowercase)
 * that you are setting tuning tables for, and the netmod/channel prefix (GEN2,
 * PSM, UCX, OFI, etc).
 *
 * TODO: setup fallbacks (ARCH/HCA)
 */
#define MVP_ARCH_TUNING_LIST_MLX                                               \
    MVP_ARCH_HCA_CASE_SET(NET, INTEL_XEON_E5_2690_V2_2S_20, MLX_CX_CONNIB, 6,  \
                          20) /* PSG */                                        \
    MVP_ARCH_HCA_CASE_SETX(NET, INTEL_XEON_X5650_12, MLX_CX_QDR, 2,            \
                           12) /* Lonestar */                                  \
    MVP_ARCH_HCA_CASE_SET(NET, INTEL_XEON_E5630_8, MLX_CX_QDR, 4, 8) /* RI */  \
    MVP_ARCH_HCA_CASE_SET(NET, INTEL_XEON_E5_2620_V4_2S_16, MLX_CX_FDR, 5,     \
                          16) /* Frontera RTX */                               \
    MVP_ARCH_HCA_CASE_SET(NET, INTEL_XEON_E5_2670_16, MLX_CX_QDR, 5,           \
                          16) /* Gordon */                                     \
    MVP_ARCH_HCA_CASE_SETX(NET, INTEL_XEON_E5_2670_16, MLX_CX_FDR, 5,          \
                           16) /* Yellowstone */                               \
    MVP_ARCH_HCA_CASE_SET(NET, INTEL_XEON_E5_2680_16, MLX_CX_FDR, 5,           \
                          16) /* Stampede */                                   \
    MVP_ARCH_HCA_CASE_SET(NET, INTEL_PLATINUM_8480_2S_112, MLX_CX_NDR, 8,      \
                          112) /* SPR */                                       \
    MVP_ARCH_HCA_CASE_SET(NET, INTEL_XEON_E5_2680_V3_2S_24, MLX_CX_FDR, 1,     \
                          1) /* Comet */                                       \
    MVP_ARCH_HCA_CASE_SET(NET, INTEL_XEON_E5_2687W_V3_2S_20, MLX_CX_HDR, 6,    \
                          20) /* nowlab haswell */                             \
    MVP_ARCH_HCA_CASE_SET(NET, INTEL_GOLD_6148_2S_40, MLX_CX_EDR, 7,           \
                          40) /* Pitzer */                                     \
    MVP_ARCH_HCA_CASE_SET(NET, INTEL_PLATINUM_8280_2S_56, MLX_CX_EDR, 8,       \
                          56) /* Frontera */                                   \
    MVP_ARCH_HCA_CASE_SET(NET, AMD_OPTERON_6136_32, MLX_CX_QDR, 6,             \
                          32)                       /* trestles */             \
    MVP_ARCH_HCA_CASE(AMD_EPYC_7401_48, MLX_CX_EDR) /* LLNL Corona */          \
    MVP_ARCH_HCA_CASE_SET(NET, AMD_EPYC_7401_24, MLX_CX_EDR, 6, 48)            \
    MVP_ARCH_HCA_CASE(AMD_EPYC_7551_64, MLX_CX_EDR) /* AMD EPYC */             \
    MVP_ARCH_HCA_CASE_SET(NET, AMD_EPYC_7551_64, MLX_CX_HDR, 7, 64)            \
    MVP_ARCH_HCA_CASE(AMD_EPYC_7601_64, ANY) /* oslo */                        \
    MVP_ARCH_HCA_CASE_SET(NET, AMD_EPYC_7742_128, MLX_CX_HDR, 8,               \
                          128)               /* AMD rome */                    \
    MVP_ARCH_FALLBACK_CASE(AMD_EPYC_7662_64) /* ORNL Spock */                  \
    MVP_ARCH_HCA_CASE(AMD_EPYC_7662_64, MLX_CX_EDR)                            \
    MVP_ARCH_HCA_CASE(AMD_EPYC_7662_64, MLX_CX_HDR)                            \
    MVP_ARCH_HCA_CASE(AMD_EPYC_7763_128, MLX_CX_EDR) /* Lonestar-6 */          \
    MVP_ARCH_HCA_CASE_SET(NET, AMD_EPYC_7763_128, MLX_CX_HDR, 8, 128)          \
    MVP_ARCH_HCA_CASE_SET(NET, ARM_GRACE_2S_144, MLX_CX_NDR, 11, 144)          \
    /* Grace */                                                                \
    MVP_ARCH_HCA_CASE_SET(NET, ARM_CAVIUM_V8_2S_28, MLX_CX_EDR, 8,             \
                          56) /* Mayer */                                      \
    MVP_ARCH_HCA_CASE_SETX(NET, ARM_CAVIUM_V8_2S_28, MLX_CX_FDR, 5,            \
                           24)                          /* Hartree */          \
    MVP_ARCH_HCA_CASE(ARM_FUJITSU_V0_4S_48, MLX_CX_EDR) /* Catalyst/Ookami */  \
    MVP_ARCH_HCA_CASE_SET(NET, ARM_CAVIUM_V8_2S_32, MLX_CX_EDR, 7, 64)         \
    MVP_ARCH_HCA_CASE_SET(NET, IBM_POWER8, MLX_CX_EDR, 4, 8)  /* Ray */        \
    MVP_ARCH_HCA_CASE_SET(NET, IBM_POWER9, MLX_CX_EDR, 8, 44) /* Sierra */

#define MVP_ARCH_TUNING_LIST_PSM                                               \
    MVP_ARCH_HCA_CASE_SET(NET, INTEL_XEON_PHI_7250, INTEL_HFI_100, 7,          \
                          64) /* Stampede-KNL */                               \
    MVP_ARCH_HCA_CASE_SET(NET, INTEL_XEON_E5_2695_V3_2S_28, INTEL_HFI_100, 6,  \
                          28) /* Bridges */                                    \
    MVP_ARCH_HCA_CASE_SET(NET, INTEL_XEON_E5_2695_V4_2S_36, INTEL_HFI_100, 6,  \
                          36) /* Jade/Opal */                                  \
    MVP_ARCH_HCA_CASE_SET(NET, INTEL_XEON_X5650_12, QLGIC_QIB, 2,              \
                          12) /* Sierra */                                     \
    MVP_ARCH_HCA_CASE_SET(NET, INTEL_PLATINUM_8160_2S_48, INTEL_HFI_100, 7,    \
                          48) /* Stampede-SKX */

#define MVP_ARCH_TUNING_LIST_CXI                                               \
    MVP_ARCH_HCA_CASE_SET(NET, AMD_EPYC_7A53_64, CRAY_SS11, 7,                 \
                          64) /* LLNL tioga */

#define MVP_ARCH_TUNING_ENTER                                                  \
    if (heterogeneity)                                                         \
        goto fn_exit;                                                          \
    mvp_arch_hca_info_t _arch_hca = MVP_get_arch_hca_type();                   \
    COLLECTIVE##_tuning_list_start : switch (_arch_hca.parts.arch_hca_id.full) \
    {
/*
 * hit the switch again with HCA_ANY and see if we get an arch hit at least,
 * otherwise, fallback to default CPU arch
 */
#define MVP_CPU_MAKE_SWITCH(_arch_hca_id)                                      \
    switch (MVP_CPU_MAKE(MVP_GET_ARCH(_arch_hca_id))) {                        \
        case MVP_CPU_ARCH_MAKE__INTEL:                                         \
            _arch_hca_id = MVP_ARCH_DEFAULT__INTEL;                            \
            break;                                                             \
        case MVP_CPU_ARCH_MAKE__AMD:                                           \
            _arch_hca_id = MVP_ARCH_DEFAULT__AMD;                              \
            break;                                                             \
        case MVP_CPU_ARCH_MAKE__IBM:                                           \
            _arch_hca_id = MVP_ARCH_DEFAULT__IBM;                              \
            break;                                                             \
        case MVP_CPU_ARCH_MAKE__ARM:                                           \
            _arch_hca_id = MVP_ARCH_DEFAULT__ARM;                              \
            break;                                                             \
        default:                                                               \
            break;                                                             \
    }

#define MVP_ARCH_TUNING_DEFAULT                                                \
    default:                                                                   \
        if (_arch_hca.parts.arch_hca_id.parts.hca_type != MVP_HCA_ANY) {       \
            _arch_hca.parts.arch_hca_id.parts.hca_type = MVP_HCA_ANY;          \
            goto COLLECTIVE##_tuning_list_start;                               \
        } else {                                                               \
            uint32_t _id = _arch_hca.parts.arch_hca_id.full;                   \
            MVP_CPU_MAKE_SWITCH(_id)                                           \
            _arch_hca.parts.arch_hca_id.full = _id;                            \
            goto COLLECTIVE##_tuning_list_start;                               \
        }                                                                      \
        }

#endif /* _MVP_TUNING_TABLES_H_ */
