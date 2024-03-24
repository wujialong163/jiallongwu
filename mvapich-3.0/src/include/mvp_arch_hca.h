/*
 * Copyright (c) 2001-2023, The Ohio State University. All rights reserved.
 *
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda.
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level MVAPICH directory.
 */

#ifndef _MVP_ARCH_LISTS_H_
#define _MVP_ARCH_LISTS_H_

/* HCA Types */
#define MVP_HCA_UNKWN 0
#define MVP_HCA_ANY   (UINT16_MAX)
#define MVP_HCA_MASK  (uint32_t)(UINT16_MAX)
#define MVP_HCA_BITS  (16)

/*
 * Proposed Layout - Hex Ecoding
 * Using low 16 bits only (allows for crossing with ARCH later)
 * Archetype - IB, ROCE, PSM, iWarp, Other - High nibble
 *             0x1 0x2   0x3  0x4     0xe
 * Sub Type - Second nibble - Archetype dependent
 * Model - Low byte
 *
 */

#define MVP_HCA_NET_CLASS_SHIFT 12
enum mvp_hca_net_class {
    MVP_HCA_NET_CLASS__IB = 0x1,
    MVP_HCA_NET_CLASS__ROCE,
    MVP_HCA_NET_CLASS__PSM,
    MVP_HCA_NET_CLASS__IWARP,
    MVP_HCA_NET_CLASS__OTHER = 0xe,
    MVP_HCA_NET_CLASS__LAST = 0xf
};

#define MVP_HCA_NET_CLASS_ID(_type)                                            \
    (MVP_HCA_NET_CLASS__##_type << MVP_HCA_NET_CLASS_SHIFT)

#define MVP_HCA_SUB_TYPE_SHIFT 8

enum mvp_hca_sub_type_ib {
    MVP_HCA_SUB_TYPE_IB__MLX = 0x1,
    MVP_HCA_SUB_TYPE_IB__LAST = 0xf,
};
enum mvp_hca_sub_type_roce {
    MVP_HCA_SUB_TYPE_ROCE__BROADCOM = 0x1,
    MVP_HCA_SUB_TYPE_ROCE__MARVEL,
    MVP_HCA_SUB_TYPE_ROCE__LAST = 0xf,
};
enum mvp_hca_sub_type_psm {
    MVP_HCA_SUB_TYPE_PSM__QLOGIC = 0x1,
    MVP_HCA_SUB_TYPE_PSM__INTEL,
    MVP_HCA_SUB_TYPE_PSM__CORNELIS,
    MVP_HCA_SUB_TYPE_PSM__LAST = 0xf,
};
enum mvp_hca_sub_type_iwarp {
    MVP_HCA_SUB_TYPE_IWARP__CHELSIO = 0x1,
    MVP_HCA_SUB_TYPE_IWARP__INTEL,
    MVP_HCA_SUB_TYPE_IWARP__IBM,
    MVP_HCA_SUB_TYPE_IWARP__LAST = 0xf,
};

#define MVP_HCA_SUB_TYPE_ID(_class, _type)                                     \
    (MVP_HCA_SUB_TYPE_##_class##__##_type << MVP_HCA_SUB_TYPE_SHIFT)

#define MVP_HCA_TYPE_ID(_class, _type)                                         \
    (MVP_HCA_NET_CLASS_ID(_class) | MVP_HCA_SUB_TYPE_ID(_class, _type))

/* To drop the model specific fields from an HCA and get a value equivalent to
 * that returned by MVP_HCA_TYPE_ID. Value shown in table as _FIRST */
#define MVP_NET_TYPE_MASK = ~(0xff)

#define MVP_HCA_MODEL_LAST = 0xff

enum mvp_hca_types {
    MVP_HCA_LIST_START = MVP_HCA_NET_CLASS_ID(IB) - 1,
    /************ BEGIN IB CARDS ***************/
    MVP_HCA_IB_TYPE_START = MVP_HCA_NET_CLASS_ID(IB),

    /* Mellanox IB HCAs */
    MVP_HCA_MLX_START = MVP_HCA_TYPE_ID(IB, MLX),
    MVP_HCA_MLX_PCI_X,
    MVP_HCA_MLX_PCI_EX_SDR,
    MVP_HCA_MLX_PCI_EX_DDR,
    MVP_HCA_MLX_CX_SDR,
    MVP_HCA_MLX_CX_DDR,
    MVP_HCA_MLX_CX_QDR,
    MVP_HCA_MLX_CX_FDR,
    MVP_HCA_MLX_CX_CONNIB,
    MVP_HCA_MLX_CX_EDR,
    MVP_HCA_MLX_CX_HDR,
    MVP_HCA_MLX_CX_NDR,
    MVP_HCA_MLX_END,

    MVP_HCA_IB_TYPE_END,
    /************ END IB CARDS ***************/

    /************ BEGIN ROCE CARDS ***************/
    MVP_HCA_ROCE_TYPE_START = MVP_HCA_NET_CLASS_ID(ROCE),

    /* Broadcom Cards */
    MVP_HCA_BROADCOM_START = MVP_HCA_TYPE_ID(ROCE, BROADCOM),
    MVP_HCA_BROADCOM_BNXTRE,
    MVP_HCA_BROADCOM_END,

    /* Marvel Cards */
    MVP_HCA_MARVEL_START = MVP_HCA_TYPE_ID(ROCE, MARVEL),
    MVP_HCA_MARVEL_QEDR,
    MVP_HCA_MARVEL_END,

    MVP_HCA_ROCE_TYPE_END,
    /************ END ROCE CARDS ***************/

    /************ BEGIN PSM CARDS ***************/
    MVP_HCA_PSM_START = MVP_HCA_NET_CLASS_ID(PSM),

    /* Qlogic Cards */
    MVP_HCA_QLGIC_START = MVP_HCA_TYPE_ID(PSM, QLOGIC),
    MVP_HCA_QLGIC_PATH_HT,
    MVP_HCA_QLGIC_QIB,
    MVP_HCA_QLGIC_END,

    /* Intel Cards */
    MVP_HCA_INTEL_START = MVP_HCA_TYPE_ID(PSM, INTEL),
    MVP_HCA_INTEL_HFI_100,
    MVP_HCA_INTEL_END,

    MVP_HCA_PSM_END,
    /************ END PSM CARDS ***************/

    /************ BEGIN IWARP CARDS ***************/
    MVP_HCA_IWARP_TYPE_START = MVP_HCA_NET_CLASS_ID(IWARP),
    /* Chelsio Cards */
    MVP_HCA_CHLSIO_START = MVP_HCA_TYPE_ID(IWARP, CHELSIO),
    MVP_HCA_CHELSIO_T3,
    MVP_HCA_CHELSIO_T4,
    MVP_HCA_CHLSIO_END,

    /* Intel iWarp Cards */
    MVP_HCA_INTEL_IWARP_START = MVP_HCA_TYPE_ID(IWARP, INTEL),
    MVP_HCA_INTEL_NE020,
    MVP_HCA_INTEL_IWARP_END,

    /* IBM Cards */
    MVP_HCA_IBM_START = MVP_HCA_TYPE_ID(IWARP, IBM),
    MVP_HCA_IBM_EHCA,
    MVP_HCA_IBM_END,

    MVP_HCA_IWARP_TYPE_END,
    /************ END IWARP CARDS ***************/

    /************ BEGIN OTHER CARDS *************/
    MVP_HCA_OTHER_START = MVP_HCA_NET_CLASS_ID(OTHER),
    /* CRAY Slingshot */
    MVP_HCA_CRAY_SS11,
    MVP_HCA_OTHER_END,
    /************ END OTHER CARDS ***************/
    MVP_HCA_LIST_MAX
};

/* right shift to dump everything but the network type */
#define MVP_NETWORK_CLASS(_hca) (_hca >> MVP_HCA_NET_CLASS_SHIFT)

/* network type is a combination of class and subtype - mask off low 16 */
#define MVP_NETWORK_TYPE(_hca) (_hca & MVP_HCA_MODEL_MASK)

/* evaluates to TRUE if _hca is of network class _class */
#define MVP_NET_CLASS_CHECK(_hca, _class)                                      \
    (MVP_NETWORK_CLASS(_hca) == MVP_HCA_NET_CLASS__##_class)

#define MVP_NET_TYPE_CHECK(_hca, type)                                         \
    (MVP_NETWORK_TYPE(_hca) == MVP_HCA_##_type_START)

/* Architecture Type
 * Proposed New Layout:
 * Using low 16 bits only (allows for crossing with HCA later)
 * Make - Intel, AMD, IBM, ARM - High Nibble - MAX
 *        0x1    0x2  0x3  0x4                 0xf
 */
#define MVP_ARCH_UNKWN 0
#define MVP_ARCH_ANY   (UINT16_MAX)
#define MVP_ARCH_MASK  ((uint32_t)UINT16_MAX << 16)
#define MVP_ARCH_BITS  (16)

#define MVP_CPU_ARCH_MAKE_SHIFT 12
enum mvp_cpu_arch_make {
    MVP_CPU_ARCH_MAKE__NONE = 0x0,
    MVP_CPU_ARCH_MAKE__INTEL,
    MVP_CPU_ARCH_MAKE__AMD,
    MVP_CPU_ARCH_MAKE__IBM,
    MVP_CPU_ARCH_MAKE__ARM,
    MVP_CPU_ARCH_MAKE__LAST,
};

#define MVP_CPU_ARCH_MAKE_ID(_make)                                            \
    (MVP_CPU_ARCH_MAKE__##_make << MVP_CPU_ARCH_MAKE_SHIFT)

enum mvp_arch_list {
    MVP_ARCH_LIST_START = MVP_CPU_ARCH_MAKE_ID(INTEL) - 1,
    /* BEGIN Intel Architectures */
    MVP_ARCH_INTEL_START = MVP_CPU_ARCH_MAKE_ID(INTEL),
    MVP_ARCH_INTEL_GENERIC,
    MVP_ARCH_INTEL_CLOVERTOWN_8,
    MVP_ARCH_INTEL_NEHALEM_8,
    MVP_ARCH_INTEL_NEHALEM_16,
    MVP_ARCH_INTEL_HARPERTOWN_8,
    MVP_ARCH_INTEL_XEON_DUAL_4,
    MVP_ARCH_INTEL_XEON_E5630_8,
    MVP_ARCH_INTEL_XEON_X5650_12,
    MVP_ARCH_INTEL_XEON_E5_2670_16,
    MVP_ARCH_INTEL_XEON_E5_2680_16,
    MVP_ARCH_INTEL_XEON_E5_2670_V2_2S_20,
    MVP_ARCH_INTEL_XEON_E5_2630_V2_2S_12,
    MVP_ARCH_INTEL_XEON_E5_2680_V2_2S_20,
    MVP_ARCH_INTEL_XEON_E5_2690_V2_2S_20,
    MVP_ARCH_INTEL_XEON_E5_2698_V3_2S_32,
    MVP_ARCH_INTEL_XEON_E5_2660_V3_2S_20,
    MVP_ARCH_INTEL_XEON_E5_2680_V3_2S_24,
    MVP_ARCH_INTEL_XEON_E5_2690_V3_2S_24,
    MVP_ARCH_INTEL_XEON_E5_2687W_V3_2S_20,
    MVP_ARCH_INTEL_XEON_E5_2670_V3_2S_24,
    MVP_ARCH_INTEL_XEON_E5_2695_V3_2S_28,
    MVP_ARCH_INTEL_XEON_E5_2680_V4_2S_28,
    MVP_ARCH_INTEL_XEON_E5_2695_V4_2S_36,
    MVP_ARCH_INTEL_XEON_E5_2620_V4_2S_16,
    MVP_ARCH_INTEL_XEON_E5_2697A_V4_2S_32,
    MVP_ARCH_INTEL_PLATINUM_8160_2S_48,
    MVP_ARCH_INTEL_PLATINUM_8260_2S_48,
    MVP_ARCH_INTEL_PLATINUM_8268_2S_48,
    MVP_ARCH_INTEL_PLATINUM_8280_2S_56,
    MVP_ARCH_INTEL_PLATINUM_8170_2S_52,
    MVP_ARCH_INTEL_PLATINUM_8380_2S_80,
    MVP_ARCH_INTEL_PLATINUM_8480_2S_112,
    MVP_ARCH_INTEL_PLATINUM_GENERIC,
    MVP_ARCH_INTEL_GOLD_6132_2S_28,
    MVP_ARCH_INTEL_GOLD_6148_2S_40,
    MVP_ARCH_INTEL_GOLD_6154_2S_36,
    MVP_ARCH_INTEL_GOLD_GENERIC,
    MVP_ARCH_INTEL_KNL_GENERIC,
    MVP_ARCH_INTEL_XEON_PHI_7210,
    MVP_ARCH_INTEL_XEON_PHI_7230,
    MVP_ARCH_INTEL_XEON_PHI_7250,
    MVP_ARCH_INTEL_XEON_PHI_7290,
    MVP_ARCH_INTEL_END,
    /* END Intel Architectures */

    /* BEGIN AMD Architectures */
    MVP_ARCH_AMD_START = MVP_CPU_ARCH_MAKE_ID(AMD),
    MVP_ARCH_AMD_GENERIC,
    MVP_ARCH_AMD_BARCELONA_16,
    MVP_ARCH_AMD_MAGNY_COURS_24,
    MVP_ARCH_AMD_OPTERON_DUAL_4,
    MVP_ARCH_AMD_OPTERON_6136_32,
    MVP_ARCH_AMD_OPTERON_6276_64,
    MVP_ARCH_AMD_BULLDOZER_4274HE_16,
    MVP_ARCH_AMD_EPYC_GENERIC,
    MVP_ARCH_AMD_EPYC_7551_64,
    MVP_ARCH_AMD_EPYC_7601_64,
    MVP_ARCH_AMD_EPYC_7643_96,
    MVP_ARCH_AMD_EPYC_7V12_64,
    MVP_ARCH_AMD_EPYC_7763_64,
    MVP_ARCH_AMD_EPYC_7763_128,
    MVP_ARCH_AMD_EPYC_7401_24,
    MVP_ARCH_AMD_EPYC_7401_48,
    MVP_ARCH_AMD_EPYC_7742_128,
    MVP_ARCH_AMD_EPYC_7662_64,
    MVP_ARCH_AMD_EPYC_7663_128,
    MVP_ARCH_AMD_EPYC_7A53_56,
    MVP_ARCH_AMD_EPYC_7A53_64,
    MVP_ARCH_AMD_EPYC_9124_16,
    MVP_ARCH_AMD_END,
    /* END AMD Architectures */

    /* BEGIN IBM Architectures */
    MVP_ARCH_IBM_START = MVP_CPU_ARCH_MAKE_ID(IBM),
    MVP_ARCH_IBM_GENERIC,
    MVP_ARCH_IBM_PPC,
    MVP_ARCH_IBM_POWER8,
    MVP_ARCH_IBM_POWER9,
    MVP_ARCH_IBM_END,
    /* END IBM Architectures */

    /* BEGIN ARM Architectures */
    MVP_ARCH_ARM_START = MVP_CPU_ARCH_MAKE_ID(ARM),
    MVP_ARCH_ARM_GENERIC,
    MVP_ARCH_ARM_CAVIUM_V8_2S_28,
    MVP_ARCH_ARM_CAVIUM_V8_2S_32,
    MVP_ARCH_ARM_FUJITSU_V0_4S_48,
    MVP_ARCH_ARM_GRACE_2S_144,
    MVP_ARCH_ARM_END,
    /* END ARM Architectures */

    MVP_ARCH_LIST_END,
};

/* right shift to dump everything but the make */
#define MVP_CPU_MAKE(_cpu) _cpu >> MVP_CPU_ARCH_MAKE_SHIFT

/* evaluates to TRUE if _cpu is of make _make */
#define MVP_CPU_MAKE_CHECK(_cpu, _make)                                        \
    (MVP_CPU_MAKE(_cpu) == MVP_CPU_ARCH_MAKE__##_make)

#define MVP_CPU_MAKE_CASE(_make)                                               \
    case MVP_CPU_ARCH_MAKE__##_make##:                                         \
        break;

typedef uint64_t mvp_arch_hca_t;
typedef uint16_t mvp_arch_t;
typedef uint16_t mvp_hca_t;

/*
 * not as convoluted as it looks, but it's a 64 bit struct representing 3 data
 * points.
 * The high 16 bits are the architecture
 * The second 16 bits are the hca
 * The third 16 bits is the core count
 * The final 16 bits are padding
 */
typedef union mvp_arch_hca_info {
    /* compound type for reading all of it */
    mvp_arch_hca_t arch_hca_info;
    struct {
        uint16_t reserved;
        uint16_t cores;
        /* the architecture/hca ID only , ignoring core count, in 32 bits */
        union {
            uint32_t full;
            /* individual member access */
            struct {
                mvp_hca_t hca_type;
                mvp_arch_t arch_type;
            } parts;
        } arch_hca_id;
    } parts;
} mvp_arch_hca_info_t;

#define MVP_GET_ARCH(_arch_hca) ((_arch_hca) >> MVP_ARCH_BITS)
#define MVP_GET_HCA(_arch_hca)  ((_arch_hca)&MVP_HCA_MASK)

#define MVP_IS_ARCH_HCA_TYPE(_arch_hca, _arch, _hca)                           \
    mvp_is_arch_hca_type(_arch_hca, _arch, _hca)

/* Get combined architecture + hca type */
mvp_arch_hca_info_t MVP_get_arch_hca_type(void);

int mvp_get_num_cpus(void);

int mvp_get_cpu_model(void);

int mvp_get_get_cpu_make(void);
#endif /* _MVP_ARCH_LISTS_H_ */
