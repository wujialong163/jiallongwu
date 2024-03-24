/* Copyright (c) 2001-2024, The Ohio State University. All rights
 * reserved.
 * Copyright (c) 2016, Intel, Inc. All rights reserved.
 *
 * This file is part of the MVAPICH software package developed by the
 * team members of The Ohio State University's Network-Based Computing
 * Laboratory (NBCL), headed by Professor Dhabaleswar K. (DK) Panda.
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level MVAPICH directory.
 *
 */

#ifndef MVP_ARCH_HCA_DETECT_H
#define MVP_ARCH_HCA_DETECT_H

#include <stdint.h>

#if defined(HAVE_LIBIBVERBS)
#include <infiniband/verbs.h>
#endif

/* HCA Types */
#define MVP_HCA_UNKWN 0
#define MVP_HCA_ANY   (UINT16_MAX)

#define MVP_HCA_TYPE_IB

/*
 * Layout:
 *
 * 1-4095 - IB Cards
 *         1 - 1000 - Mellanox Cards
 *      1001 - 2000 - Qlogic Cards
 *      2001 - 3000 - IBM Cards
 *      3001 - 4000 - Intel HFI Cards
 *
 * 4096-8191 - iWarp Cards
 *      5001 - 6000 - Chelsio Cards
 *      6001 - 7000 - Intel iWarp Cards
 */

/* Mellanox Cards */
typedef enum {
    MVP_HCA_LIST_START = 1,
    /* Chelsio Cards */
    MVP_HCA_IWARP_TYPE_START,
    MVP_HCA_CHLSIO_START,
    MVP_HCA_CHELSIO_T3,
    MVP_HCA_CHELSIO_T4,
    MVP_HCA_CHLSIO_END,

    /* Intel iWarp Cards */
    MVP_HCA_INTEL_IWARP_START,
    MVP_HCA_INTEL_NE020,
    MVP_HCA_INTEL_IWARP_END,
    MVP_HCA_IWARP_TYPE_END,

    /* Mellanox IB HCAs */
    MVP_HCA_IB_TYPE_START,
    MVP_HCA_MLX_START,
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
    MVP_HCA_MLX_END,
    MVP_HCA_IB_TYPE_END,

    /* Qlogic Cards */
    MVP_HCA_QLGIC_START,
    MVP_HCA_QLGIC_PATH_HT,
    MVP_HCA_QLGIC_QIB,
    MVP_HCA_QLGIC_END,

    /* IBM Cards */
    MVP_HCA_IBM_START,
    MVP_HCA_IBM_EHCA,
    MVP_HCA_IBM_END,

    /* Intel Cards */
    MVP_HCA_INTEL_START,
    MVP_HCA_INTEL_HFI_100,
    MVP_HCA_INTEL_END,

    /* Marvel Cards */
    MVP_HCA_MARVEL_START,
    MVP_HCA_MARVEL_QEDR,
    MVP_HCA_MARVEL_END,

    /* Broadcom Cards */
    MVP_HCA_BROADCOM_START,
    MVP_HCA_BROADCOM_BNXTRE,
    MVP_HCA_BROADCOM_END,

    MVP_HCA_LIST_END,
} mvp_hca_types_list;

typedef enum {
    MVP_NETWORK_CLASS_UNKNOWN = 0,
    MVP_NETWORK_CLASS_IB,
    MVP_NETWORK_CLASS_IWARP,
    MVP_NETWORK_CLASS_MARVEL,
    MVP_NETWORK_CLASS_BROADCOM,
    MVP_NETWORK_LAST_ENTRY,
} mvp_iba_network_classes;

#define MVP_GET_NETWORK_TYPE(_x)                                               \
    ((MVP_IS_IB_CARD(_x)) ?                                                    \
         MVP_NETWORK_CLASS_IB :                                                \
         ((MVP_IS_IWARP_CARD(_x)) ?                                            \
              MVP_NETWORK_CLASS_IWARP :                                        \
              ((MVP_IS_MARVEL_CARD(_x)) ?                                      \
                   MVP_NETWORK_CLASS_MARVEL :                                  \
                   ((MVP_IS_BROADCOM_CARD(_x)) ? MVP_NETWORK_CLASS_BROADCOM :  \
                                                 MVP_NETWORK_CLASS_UNKNOWN))))
/* Check if given card is IB card or not */
#define MVP_IS_IB_CARD(_x)                                                     \
    ((_x) > MVP_HCA_IB_TYPE_START && (_x) < MVP_HCA_IB_TYPE_END)

/* Check if given card is iWarp card or not */
#define MVP_IS_IWARP_CARD(_x)                                                  \
    ((_x) > MVP_HCA_IWARP_TYPE_START && (_x) < MVP_HCA_IWARP_TYPE_END)

/* Check if given card is Chelsio iWarp card or not */
#define MVP_IS_CHELSIO_IWARP_CARD(_x)                                          \
    ((_x) > MVP_HCA_CHLSIO_START && (_x) < MVP_HCA_CHLSIO_END)

/* Check if given card is QLogic card or not */
#define MVP_IS_QLE_CARD(_x)                                                    \
    ((_x) > MVP_HCA_QLGIC_START && (_x) < MVP_HCA_QLGIC_END)

/* Check if given card is Intel card or not */
#define MVP_IS_INTEL_CARD(_x)                                                  \
    ((_x) > MVP_HCA_INTEL_START && (_x) < MVP_HCA_INTEL_END)

/* Check if given card is Marvel card or not */
#define MVP_IS_MARVEL_CARD(_x)                                                 \
    ((_x) > MVP_HCA_MARVEL_START && (_x) < MVP_HCA_MARVEL_END)

/* Check if given card is Broadcom card or not */
#define MVP_IS_BROADCOM_CARD(_x)                                               \
    ((_x) > MVP_HCA_BROADCOM_START && (_x) < MVP_HCA_BROADCOM_END)

/* Architecture Type
 * Layout:
 *    1 - 1000 - Intel architectures
 * 1001 - 2000 - AMD architectures
 * 2001 - 3000 - IBM architectures
 */
#define MVP_ARCH_UNKWN 0
#define MVP_ARCH_ANY   (UINT16_MAX)

/* Intel Architectures */
typedef enum {
    MVP_ARCH_LIST_START = 1,
    MVP_ARCH_INTEL_START,
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
    /* AMD Architectures */
    MVP_ARCH_AMD_START,
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
    MVP_ARCH_AMD_EPYC_7V12_64,
    MVP_ARCH_AMD_EPYC_7763_64,
    MVP_ARCH_AMD_EPYC_7763_128,
    MVP_ARCH_AMD_EPYC_7401_48,
    MVP_ARCH_AMD_EPYC_7742_128,
    MVP_ARCH_AMD_EPYC_7662_64,
    MVP_ARCH_AMD_END,
    /* IBM Architectures */
    MVP_ARCH_IBM_START,
    MVP_ARCH_IBM_PPC,
    MVP_ARCH_IBM_POWER8,
    MVP_ARCH_IBM_POWER9,
    MVP_ARCH_IBM_END,
    /* ARM Architectures */
    MVP_ARCH_ARM_START,
    MVP_ARCH_ARM_CAVIUM_V8_2S_28,
    MVP_ARCH_ARM_CAVIUM_V8_2S_32,
    MVP_ARCH_ARM_FUJITSU_V0_4S_48,
    MVP_ARCH_ARM_END,
    MVP_ARCH_LIST_END,
} mvp_proc_arch_list;

typedef uint64_t mvp_arch_hca_type;
typedef uint16_t mvp_arch_type;
typedef uint16_t mvp_hca_type;
typedef uint16_t mvp_arch_num_cores;
typedef uint16_t mvp_arch_reserved; /* reserved 16-bits for future use */

#define NUM_HCA_BITS  (16)
#define NUM_ARCH_BITS (16)

#define MVP_GET_ARCH(_arch_hca) ((_arch_hca) >> 32)
#define MVP_GET_HCA(_arch_hca)  (((_arch_hca) << 32) >> 48)

/* CPU Family */
typedef enum {
    MVP_CPU_FAMILY_NONE = 0,
    MVP_CPU_FAMILY_INTEL,
    MVP_CPU_FAMILY_AMD,
    MVP_CPU_FAMILY_POWER,
    MVP_CPU_FAMILY_ARM,
} mvp_cpu_family_type;

/* Multi-rail info */
typedef enum {
    mvp_num_rail_unknown = 0,
    mvp_num_rail_1,
    mvp_num_rail_2,
    mvp_num_rail_3,
    mvp_num_rail_4,
} mvp_multirail_info_type;

#define MVP_IS_ARCH_HCA_TYPE(_arch_hca, _arch, _hca)                           \
    mvp_is_arch_hca_type(_arch_hca, _arch, _hca)

enum collectives {
    allgather = 0,
    allreduce,
    alltoall,
    alltoallv,
    bcast,
    gather,
    reduce,
    scatter,
    colls_max
};

static const char collective_names[colls_max][12] = {
    "Allgather", "Allreduce", "Alltoall", "Alltoallv",
    "Broadcast", "Gather",    "Reduce",   "Scatter"};

struct coll_info {
    mvp_hca_type hca_type;
    mvp_arch_type arch_type;
};

extern mvp_arch_type table_arch_tmp;
extern mvp_hca_type table_hca_tmp;
extern int mvp_suppress_hca_warnings;

/* ************************ FUNCTION DECLARATIONS ************************** */

/* Check arch-hca type */
int mvp_is_arch_hca_type(mvp_arch_hca_type arch_hca_type,
                         mvp_arch_type arch_type, mvp_hca_type hca_type);

/* Get architecture-hca type */
#if defined(HAVE_LIBIBVERBS)
mvp_arch_hca_type mvp_get_arch_hca_type(struct ibv_device *dev);
mvp_arch_hca_type mvp_new_get_arch_hca_type(mvp_hca_type hca_type);
#else
mvp_arch_hca_type mvp_get_arch_hca_type(void *dev);
#endif

/* Check if the host has multiple rails or not */
mvp_multirail_info_type mvp_get_multirail_info(void);

/* Get architecture type */
mvp_arch_type mvp_get_arch_type(void);

/* Get card type */
#if defined(HAVE_LIBIBVERBS)
mvp_hca_type mvp_get_hca_type(struct ibv_device *dev);
mvp_hca_type mvp_new_get_hca_type(struct ibv_context *ctx,
                                  struct ibv_device *dev, uint64_t *guid);
#else
mvp_hca_type mvp_get_hca_type(void *dev);
#endif

/* Get combined architecture + hca type */
mvp_arch_hca_type MVP_get_arch_hca_type(void);

/* Get number of cpus */
int mvp_get_num_cpus(void);

/* Get the CPU model */
int mvp_get_cpu_model(void);

/* Get CPU family */
mvp_cpu_family_type mvp_get_cpu_family(void);

/* Log arch-hca type */
void mvp_log_arch_hca_type(mvp_arch_hca_type arch_hca);
char *mvp_get_network_name(mvp_iba_network_classes network_type);

char *mvp_get_hca_name(mvp_hca_type hca_type);
char *mvp_get_arch_name(mvp_arch_type arch_type);
char *mvp_get_cpu_family_name(mvp_cpu_family_type cpu_family_type);

#if defined(_SMP_LIMIC_)
/*Detecting number of cores in a socket, and number of sockets*/
void hwlocSocketDetection(int print_details);

/*Returns the socket where the process is bound*/
int getProcessBinding(pid_t pid);

/*Returns the number of cores in the socket*/
int numOfCoresPerSocket(int socket);

/*Returns the total number of sockets within the node*/
int numofSocketsPerNode(void);

/*Return socket bind to */
int get_socket_bound(void);
#endif /* defined(_SMP_LIMIC_) */

#endif /*  #ifndef MVP_ARCH_HCA_DETECT_H */
