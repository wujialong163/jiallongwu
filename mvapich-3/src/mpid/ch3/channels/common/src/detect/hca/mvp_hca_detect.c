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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mpichconf.h"

#if defined(HAVE_LIBIBUMAD)
#include <infiniband/umad.h>
#endif

#include "mvp_arch_hca_detect.h"
#include "upmi.h"
#include "mvp_debug_utils.h"

#include "upmi.h"
#include "mpi.h"
#if CHANNEL_MRAIL
#include "rdma_impl.h"
#endif /* CHANNEL_MRAIL */

/*
=== BEGIN_MPI_T_CVAR_INFO_BLOCK ===

cvars:
    - name        : FORCE_HCA_TYPE
      category    : CH3
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This parameter forces the HCA type.

=== END_MPI_T_CVAR_INFO_BLOCK ===

=== BEGIN_MPI_T_MVP_CVAR_INFO_BLOCK ===

cvars:
    - name        : MVP_FORCE_HCA_TYPE
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_HCA_AWARE_PROCESS_MAPPING
      category    : CH3
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC - Not used currently.

=== END_MPI_T_MVP_CVAR_INFO_BLOCK ===

*/

int mvp_suppress_hca_warnings = 0;
extern int g_mvp_num_cpus;
static mvp_multirail_info_type g_mvp_multirail_info = mvp_num_rail_unknown;

#define MVP_STR_MLX   "mlx"
#define MVP_STR_MLX4  "mlx4"
#define MVP_STR_MLX5  "mlx5"
#define MVP_STR_MTHCA "mthca"
#define MVP_STR_IPATH "ipath"
#define MVP_STR_QIB   "qib"
#define MVP_STR_HFI1  "hfi1"
#define MVP_STR_EHCA  "ehca"
#define MVP_STR_CXGB3 "cxgb3"
#define MVP_STR_CXGB4 "cxgb4"
#define MVP_STR_NES0  "nes0"
#define MVP_STR_QEDR  "qedr"
#define MVP_STR_BRDCM "bnxt"

typedef struct _mvp_hca_types_log_t {
    mvp_hca_type hca_type;
    char *hca_name;
} mvp_hca_types_log_t;

typedef struct _mvp_network_types_log_t {
    mvp_iba_network_classes network_type;
    char *network_name;
} mvp_network_types_log_t;

#define MVP_HCA_LAST_ENTRY MVP_HCA_LIST_END
static mvp_hca_types_log_t mvp_hca_types_log[] = {
    /*Unknown */
    {MVP_HCA_UNKWN, "MVP_HCA_UNKWN"},

    /* Mellanox Cards */
    {MVP_HCA_MLX_PCI_EX_SDR, "MVP_HCA_MLX_PCI_EX_SDR"},
    {MVP_HCA_MLX_PCI_EX_DDR, "MVP_HCA_MLX_PCI_EX_DDR"},
    {MVP_HCA_MLX_CX_SDR, "MVP_HCA_MLX_CX_SDR"},
    {MVP_HCA_MLX_CX_DDR, "MVP_HCA_MLX_CX_DDR"},
    {MVP_HCA_MLX_CX_QDR, "MVP_HCA_MLX_CX_QDR"},
    {MVP_HCA_MLX_CX_FDR, "MVP_HCA_MLX_CX_FDR"},
    {MVP_HCA_MLX_CX_EDR, "MVP_HCA_MLX_CX_EDR"},
    {MVP_HCA_MLX_CX_HDR, "MVP_HCA_MLX_CX_HDR"},
    {MVP_HCA_MLX_CX_CONNIB, "MVP_HCA_MLX_CX_CONNIB"},
    {MVP_HCA_MLX_PCI_X, "MVP_HCA_MLX_PCI_X"},

    /* Qlogic Cards */
    {MVP_HCA_QLGIC_PATH_HT, "MVP_HCA_QLGIC_PATH_HT"},
    {MVP_HCA_QLGIC_QIB, "MVP_HCA_QLGIC_QIB"},

    /* IBM Cards */
    {MVP_HCA_IBM_EHCA, "MVP_HCA_IBM_EHCA"},

    /* Intel Cards */
    {MVP_HCA_INTEL_HFI_100, "MVP_HCA_INTEL_HFI_100"},

    /* Chelsio Cards */
    {MVP_HCA_CHELSIO_T3, "MVP_HCA_CHELSIO_T3"},
    {MVP_HCA_CHELSIO_T4, "MVP_HCA_CHELSIO_T4"},

    /* Intel iWarp Cards */
    {MVP_HCA_INTEL_NE020, "MVP_HCA_INTEL_NE020"},

    /* Marvel RoCE Cards */
    {MVP_HCA_MARVEL_QEDR, "MVP_HCA_MARVEL_QEDR"},

    /* Broadcom RoCE Cards */
    {MVP_HCA_BROADCOM_BNXTRE, "MVP_HCA_BROADCOM_BNXTRE"},

    /* Last Entry */
    {MVP_HCA_LAST_ENTRY, "MVP_HCA_LAST_ENTRY"},
};

static mvp_network_types_log_t mvp_network_types_log[] = {
    {MVP_NETWORK_CLASS_UNKNOWN, "MVP_NETWORK_CLASS_UNKNOWN"},
    {MVP_NETWORK_CLASS_IB, "MVP_NETWORK_CLASS_IB"},
    {MVP_NETWORK_CLASS_IWARP, "MVP_NETWORK_CLASS_IWARP"},
    {MVP_NETWORK_CLASS_MARVEL, "MVP_NETWORK_CLASS_MARVEL"},
    {MVP_NETWORK_CLASS_BROADCOM, "MVP_NETWORK_CLASS_BROADCOM"},
};

char *mvp_get_network_name(mvp_iba_network_classes network_type)
{
    int i = 0;
    while (mvp_network_types_log[i].network_type != MVP_NETWORK_LAST_ENTRY) {
        if (mvp_network_types_log[i].network_type == network_type) {
            return (mvp_network_types_log[i].network_name);
        }
        i++;
    }
    return ("MVP_NETWORK_CLASS_UNKNOWN");
}

char *mvp_get_hca_name(mvp_hca_type hca_type)
{
    int i = 0;
    if (hca_type == MVP_HCA_ANY) {
        return ("MVP_HCA_ANY");
    }
    while (mvp_hca_types_log[i].hca_type != MVP_HCA_LAST_ENTRY) {
        if (mvp_hca_types_log[i].hca_type == hca_type) {
            return (mvp_hca_types_log[i].hca_name);
        }
        i++;
    }
    return ("MVP_HCA_UNKWN");
}

#if defined(HAVE_LIBIBUMAD)
static int get_rate(umad_ca_t *umad_ca)
{
    int i;
    char *value;

    if ((value = getenv("MVP_DEFAULT_PORT")) != NULL) {
        int default_port = atoi(value);

        if (default_port <= umad_ca->numports) {
            if (IBV_PORT_ACTIVE == umad_ca->ports[default_port]->state) {
                return umad_ca->ports[default_port]->rate;
            }
        }
    }

    for (i = 1; i <= umad_ca->numports; i++) {
        if (IBV_PORT_ACTIVE == umad_ca->ports[i]->state) {
            return umad_ca->ports[i]->rate;
        }
    }
    return 0;
}
#endif

const int get_link_width(uint8_t width)
{
    switch (width) {
        case 1:
            return 1;
        case 2:
            return 4;
        case 4:
            return 8;
        case 8:
            return 12;
        /* Links on Frontera are returning 16 as link width for now.
         * This is a temporary work around for that. */
        case 16:
            return 2;
        default:
            PRINT_ERROR("Invalid link width %u\n", width);
            return 0;
    }
}

const float get_link_speed(uint8_t speed)
{
    switch (speed) {
        case 1:
            return 2.5; /* SDR */
        case 2:
            return 5.0; /* DDR */

        case 4: /* fall through */
        case 8:
            return 10.0; /* QDR */

        case 16:
            return 14.0; /* FDR */
        case 32:
            return 25.0; /* EDR */
        case 64:
            return 50.0; /* HDR */
        default:
            PRINT_ERROR("Invalid link speed %u\n", speed);
            return 0; /* Invalid speed */
    }
}

int mvp_check_hca_type(mvp_hca_type type, int rank)
{
    if (type <= MVP_HCA_LIST_START || type >= MVP_HCA_LIST_END ||
        type == MVP_HCA_IB_TYPE_START || type == MVP_HCA_IB_TYPE_END ||
        type == MVP_HCA_MLX_START || type == MVP_HCA_MLX_END ||
        type == MVP_HCA_IWARP_TYPE_START || type == MVP_HCA_IWARP_TYPE_END ||
        type == MVP_HCA_CHLSIO_START || type == MVP_HCA_CHLSIO_END ||
        type == MVP_HCA_INTEL_IWARP_START || type == MVP_HCA_INTEL_IWARP_END ||
        type == MVP_HCA_QLGIC_START || type == MVP_HCA_QLGIC_END ||
        type == MVP_HCA_MARVEL_START || type == MVP_HCA_MARVEL_END ||
        type == MVP_HCA_INTEL_START || type == MVP_HCA_INTEL_END) {
        PRINT_INFO((rank == 0),
                   "Wrong value specified for MVP_FORCE_HCA_TYPE\n");
        PRINT_INFO((rank == 0),
                   "Value must be greater than %d and less than %d \n",
                   MVP_HCA_LIST_START, MVP_HCA_LIST_END);
        PRINT_INFO((rank == 0),
                   "For IB Cards: Please enter value greater than %d and less "
                   "than %d\n",
                   MVP_HCA_MLX_START, MVP_HCA_MLX_END);
        PRINT_INFO((rank == 0),
                   "For IBM Cards: Please enter value greater than %d and less "
                   "than %d\n",
                   MVP_HCA_IBM_START, MVP_HCA_IBM_END);
        PRINT_INFO((rank == 0),
                   "For Intel IWARP Cards: Please enter value greater than %d "
                   "and less than %d\n",
                   MVP_HCA_INTEL_IWARP_START, MVP_HCA_INTEL_IWARP_END);
        PRINT_INFO((rank == 0),
                   "For Chelsio IWARP Cards: Please enter value greater than "
                   "%d and less than %d\n",
                   MVP_HCA_CHLSIO_START, MVP_HCA_CHLSIO_END);
        PRINT_INFO((rank == 0),
                   "For QLogic Cards: Please enter value greater than %d and "
                   "less than %d\n",
                   MVP_HCA_QLGIC_START, MVP_HCA_QLGIC_END);
        PRINT_INFO((rank == 0),
                   "For Marvel Cards: Please enter value greater than %d and "
                   "less than %d\n",
                   MVP_HCA_MARVEL_START, MVP_HCA_MARVEL_END);
        PRINT_INFO((rank == 0),
                   "For Intel Cards: Please enter value greater than %d and "
                   "less than %d\n",
                   MVP_HCA_INTEL_START, MVP_HCA_INTEL_END);
        return 1;
    }
    return 0;
}

#if defined(HAVE_LIBIBVERBS)
mvp_hca_type mvp_new_get_hca_type(struct ibv_context *ctx,
                                  struct ibv_device *ib_dev, uint64_t *guid)
{
    int rate = 0;
    int my_rank = -1;
    char *value = NULL;
    char *dev_name = NULL;
    struct ibv_device_attr device_attr;
    int max_ports = 0;
    mvp_hca_type hca_type = MVP_HCA_UNKWN;

    UPMI_GET_RANK(&my_rank);

    if ((value = getenv("MVP_SUPPRESS_HCA_WARNINGS")) != NULL) {
        mvp_suppress_hca_warnings = !!atoi(value);
    }

    if (MVP_FORCE_HCA_TYPE != -1) {
        hca_type = MVP_FORCE_HCA_TYPE;
        PRINT_DEBUG(DEBUG_INIT_verbose, "Attempting to force HCA %s\n",
                    mvp_get_hca_name(hca_type));
        int retval = mvp_check_hca_type(hca_type, my_rank);
        if (retval) {
            PRINT_INFO((my_rank == 0),
                       "Falling back to Automatic HCA detection\n");
            hca_type = MVP_HCA_UNKWN;
        } else {
            return hca_type;
        }
    }

#if CHANNEL_MRAIL
    dev_name = (char *)ibv_ops.get_device_name(ib_dev);
#elif CHANNEL_PSM
    dev_name = (char *)ibv_get_device_name(ib_dev);
#endif

    if ((!dev_name) && !mvp_suppress_hca_warnings) {
        PRINT_INFO((my_rank == 0),
                   "**********************WARNING***********************\n");
        PRINT_INFO((my_rank == 0),
                   "Failed to automatically detect the HCA architecture.\n");
        PRINT_INFO((my_rank == 0),
                   "This may lead to subpar communication performance.\n");
        PRINT_INFO((my_rank == 0),
                   "****************************************************\n");
        return MVP_HCA_UNKWN;
    }

    memset(&device_attr, 0, sizeof(struct ibv_device_attr));
    if (
#if CHANNEL_MRAIL
        !ibv_ops.query_device(ctx, &device_attr)
#elif CHANNEL_PSM
        !ibv_query_device(ctx, &device_attr)
#endif
    ) {
        max_ports = device_attr.phys_port_cnt;
        *guid = device_attr.node_guid;
    }

    if (!strncmp(dev_name, MVP_STR_MLX, 3) ||
        !strncmp(dev_name, MVP_STR_MTHCA, 5)) {
        hca_type = MVP_HCA_MLX_PCI_X;

        int query_port = 1;
        struct ibv_port_attr port_attr;

        /* honor MVP_DEFAULT_PORT, if set */
        if ((value = getenv("MVP_DEFAULT_PORT")) != NULL) {
            int default_port = atoi(value);
            query_port = (default_port <= max_ports) ? default_port : 1;
        }

        if (
#if CHANNEL_MRAIL
            !ibv_ops.query_port(ctx, query_port, &port_attr)
#elif CHANNEL_PSM
            !ibv_query_port(ctx, query_port, &port_attr)
#endif
        ) {
            rate = (int)(get_link_width(port_attr.active_width) *
                         get_link_speed(port_attr.active_speed));
            PRINT_DEBUG(DEBUG_INIT_verbose, "rate : %d\n", rate);
        }
        /* mlx4, mlx5 */
        switch (rate) {
            case 200:
                hca_type = MVP_HCA_MLX_CX_HDR;
                break;

            case 100:
                hca_type = MVP_HCA_MLX_CX_EDR;
                break;

            case 56:
                hca_type = MVP_HCA_MLX_CX_FDR;
                break;

            case 40:
                hca_type = MVP_HCA_MLX_CX_QDR;
                break;

            case 20:
                hca_type = MVP_HCA_MLX_CX_DDR;
                break;

            case 10:
                hca_type = MVP_HCA_MLX_CX_SDR;
                break;

            default:
                hca_type = MVP_HCA_MLX_CX_FDR;
                break;
        }
        if (!strncmp(dev_name, MVP_STR_MLX5, 4) && rate == 56)
            hca_type = MVP_HCA_MLX_CX_CONNIB;
    } else if (!strncmp(dev_name, MVP_STR_IPATH, 5)) {
        hca_type = MVP_HCA_QLGIC_PATH_HT;

    } else if (!strncmp(dev_name, MVP_STR_QIB, 3)) {
        hca_type = MVP_HCA_QLGIC_QIB;

    } else if (!strncmp(dev_name, MVP_STR_HFI1, 4)) {
        hca_type = MVP_HCA_INTEL_HFI_100;

    } else if (!strncmp(dev_name, MVP_STR_EHCA, 4)) {
        hca_type = MVP_HCA_IBM_EHCA;

    } else if (!strncmp(dev_name, MVP_STR_CXGB3, 5)) {
        hca_type = MVP_HCA_CHELSIO_T3;

    } else if (!strncmp(dev_name, MVP_STR_CXGB4, 5)) {
        hca_type = MVP_HCA_CHELSIO_T4;

    } else if (!strncmp(dev_name, MVP_STR_NES0, 4)) {
        hca_type = MVP_HCA_INTEL_NE020;

    } else if (!strncmp(dev_name, MVP_STR_QEDR, 4)) {
        hca_type = MVP_HCA_MARVEL_QEDR;

    } else if (!strncmp(dev_name, MVP_STR_BRDCM, 4)) {
        hca_type = MVP_HCA_BROADCOM_BNXTRE;

    } else {
        hca_type = MVP_HCA_UNKWN;
    }

    if ((hca_type == MVP_HCA_UNKWN) && !mvp_suppress_hca_warnings) {
        PRINT_INFO((my_rank == 0),
                   "**********************WARNING***********************\n");
        PRINT_INFO((my_rank == 0),
                   "Failed to automatically detect the HCA architecture.\n");
        PRINT_INFO((my_rank == 0),
                   "This may lead to subpar communication performance.\n");
        PRINT_INFO((my_rank == 0),
                   "****************************************************\n");
    }

    return hca_type;
}

mvp_hca_type mvp_get_hca_type(struct ibv_device *dev)
{
    int rate = 0;
    char *value = NULL;
    char *dev_name;
    int my_rank = -1;
    mvp_hca_type hca_type = MVP_HCA_UNKWN;

    UPMI_GET_RANK(&my_rank);

    if ((value = getenv("MVP_SUPPRESS_HCA_WARNINGS")) != NULL) {
        mvp_suppress_hca_warnings = !!atoi(value);
    }
    if (MVP_FORCE_HCA_TYPE != -1) {
        hca_type = MVP_FORCE_HCA_TYPE;
        PRINT_DEBUG(DEBUG_INIT_verbose, "Attempting to force HCA %s\n",
                    mvp_get_hca_name(hca_type));
        int retval = mvp_check_hca_type(hca_type, my_rank);
        if (retval) {
            PRINT_INFO((my_rank == 0),
                       "Falling back to Automatic HCA detection\n");
            hca_type = MVP_HCA_UNKWN;
        } else {
            return hca_type;
        }
    }

#if CHANNEL_MRAIL
    dev_name = (char *)ibv_ops.get_device_name(dev);
#elif CHANNEL_PSM
    dev_name = (char *)ibv_get_device_name(dev);
#endif

    if ((!dev_name) && !mvp_suppress_hca_warnings) {
        PRINT_INFO((my_rank == 0),
                   "**********************WARNING***********************\n");
        PRINT_INFO((my_rank == 0),
                   "Failed to automatically detect the HCA architecture.\n");
        PRINT_INFO((my_rank == 0),
                   "This may lead to subpar communication performance.\n");
        PRINT_INFO((my_rank == 0),
                   "****************************************************\n");
        return MVP_HCA_UNKWN;
    }

#ifdef HAVE_LIBIBUMAD
    static char last_name[UMAD_CA_NAME_LEN + 1] = {'\0'};
    static mvp_hca_type last_type = MVP_HCA_UNKWN;
    if (!strncmp(dev_name, last_name, UMAD_CA_NAME_LEN)) {
        return last_type;
    } else {
        strncpy(last_name, dev_name, UMAD_CA_NAME_LEN);
    }
#endif /* #ifdef HAVE_LIBIBUMAD */

    if (!strncmp(dev_name, MVP_STR_MLX4, 4) ||
        !strncmp(dev_name, MVP_STR_MLX5, 4) ||
        !strncmp(dev_name, MVP_STR_MTHCA, 5)) {
        hca_type = MVP_HCA_UNKWN;
#if !defined(HAVE_LIBIBUMAD)
        int query_port = 1;
        struct ibv_context *ctx = NULL;
        struct ibv_port_attr port_attr;

#if CHANNEL_MRAIL
        ctx = ibv_ops.open_device(dev);
#elif CHANNEL_PSM
        ctx = ibv_open_device(dev);
#endif
        if (!ctx) {
            return MVP_HCA_UNKWN;
        }

        /* honor MVP_DEFAULT_PORT, if set */
        if ((value = getenv("MVP_DEFAULT_PORT")) != NULL) {
            int max_ports = 1;
            struct ibv_device_attr device_attr;
            int default_port = atoi(value);

            memset(&device_attr, 0, sizeof(struct ibv_device_attr));
            if (
#if CHANNEL_MRAIL
                !ibv_ops.query_device(ctx, &device_attr)
#elif CHANNEL_PSM
                !ibv_query_device(ctx, &device_attr)
#endif
            ) {
                max_ports = device_attr.phys_port_cnt;
            }
            query_port = (default_port <= max_ports) ? default_port : 1;
        }

        if (
#if CHANNEL_MRAIL
            !ibv_ops.query_port(ctx, query_port, &port_attr) &&
#elif CHANNEL_PSM
            !ibv_query_port(ctx, query_port, &port_attr) &&
#endif
            (port_attr.state == IBV_PORT_ACTIVE)) {
            rate = (int)(get_link_width(port_attr.active_width) *
                         get_link_speed(port_attr.active_speed));
            PRINT_DEBUG(DEBUG_INIT_verbose, "rate : %d\n", rate);
        }
#else
        umad_ca_t umad_ca;
        if (umad_ops.init() < 0) {
            last_type = hca_type;
            return hca_type;
        }

        memset(&umad_ca, 0, sizeof(umad_ca_t));

        if (umad_ops.get_ca(dev_name, &umad_ca) < 0) {
            last_type = hca_type;
            return hca_type;
        }

        rate = get_rate(&umad_ca);
        if (!rate) {
            umad_ops.release_ca(&umad_ca);
            umad_ops.done();
            last_type = hca_type;
            return hca_type;
        }

        umad_ops.release_ca(&umad_ca);
        umad_ops.done();

        if (!strncmp(dev_name, MVP_STR_MTHCA, 5)) {
            hca_type = MVP_HCA_MLX_PCI_X;

            if (!strncmp(umad_ca.ca_type, "MT25", 4)) {
                switch (rate) {
                    case 20:
                        hca_type = MVP_HCA_MLX_PCI_EX_DDR;
                        break;

                    case 10:
                        hca_type = MVP_HCA_MLX_PCI_EX_SDR;
                        break;

                    default:
                        hca_type = MVP_HCA_MLX_PCI_EX_SDR;
                        break;
                }

            } else if (!strncmp(umad_ca.ca_type, "MT23", 4)) {
                hca_type = MVP_HCA_MLX_PCI_X;

            } else {
                hca_type = MVP_HCA_MLX_PCI_EX_SDR;
            }
        } else
#endif
        { /* mlx4, mlx5 */
            switch (rate) {
                case 200:
                    hca_type = MVP_HCA_MLX_CX_HDR;
                    break;

                case 100:
                    hca_type = MVP_HCA_MLX_CX_EDR;
                    break;

                case 56:
                    hca_type = MVP_HCA_MLX_CX_FDR;
                    break;

                case 40:
                    hca_type = MVP_HCA_MLX_CX_QDR;
                    break;

                case 20:
                    hca_type = MVP_HCA_MLX_CX_DDR;
                    break;

                case 10:
                    hca_type = MVP_HCA_MLX_CX_SDR;
                    break;

                default:
                    hca_type = MVP_HCA_MLX_CX_SDR;
                    break;
            }
            if (!strncmp(dev_name, MVP_STR_MLX5, 4) && rate == 56)
                hca_type = MVP_HCA_MLX_CX_CONNIB;
        }

    } else if (!strncmp(dev_name, MVP_STR_IPATH, 5)) {
        hca_type = MVP_HCA_QLGIC_PATH_HT;

    } else if (!strncmp(dev_name, MVP_STR_QIB, 3)) {
        hca_type = MVP_HCA_QLGIC_QIB;

    } else if (!strncmp(dev_name, MVP_STR_HFI1, 4)) {
        hca_type = MVP_HCA_INTEL_HFI_100;

    } else if (!strncmp(dev_name, MVP_STR_EHCA, 4)) {
        hca_type = MVP_HCA_IBM_EHCA;

    } else if (!strncmp(dev_name, MVP_STR_CXGB3, 5)) {
        hca_type = MVP_HCA_CHELSIO_T3;

    } else if (!strncmp(dev_name, MVP_STR_CXGB4, 5)) {
        hca_type = MVP_HCA_CHELSIO_T4;

    } else if (!strncmp(dev_name, MVP_STR_NES0, 4)) {
        hca_type = MVP_HCA_INTEL_NE020;

    } else if (!strncmp(dev_name, MVP_STR_QEDR, 4)) {
        hca_type = MVP_HCA_MARVEL_QEDR;

    } else if (!strncmp(dev_name, MVP_STR_BRDCM, 4)) {
        hca_type = MVP_HCA_BROADCOM_BNXTRE;

    } else {
        hca_type = MVP_HCA_UNKWN;
    }
#ifdef HAVE_LIBIBUMAD
    last_type = hca_type;
#endif /* #ifdef HAVE_LIBIBUMAD */
    if ((hca_type == MVP_HCA_UNKWN) && !mvp_suppress_hca_warnings) {
        PRINT_INFO((my_rank == 0),
                   "**********************WARNING***********************\n");
        PRINT_INFO((my_rank == 0),
                   "Failed to automatically detect the HCA architecture.\n");
        PRINT_INFO((my_rank == 0),
                   "This may lead to subpar communication performance.\n");
        PRINT_INFO((my_rank == 0),
                   "****************************************************\n");
    }
    return hca_type;
}
#else
mvp_hca_type mvp_get_hca_type(void *dev)
{
    int my_rank = -1;
    char *value = NULL;
    mvp_hca_type hca_type = MVP_HCA_UNKWN;

    UPMI_GET_RANK(&my_rank);

    if ((value = getenv("MVP_SUPPRESS_HCA_WARNINGS")) != NULL) {
        mvp_suppress_hca_warnings = !!atoi(value);
    }
    if (MVP_FORCE_HCA_TYPE != -1) {
        hca_type = MVP_FORCE_HCA_TYPE;
        PRINT_DEBUG(DEBUG_INIT_verbose, "Attempting to force HCA %s\n",
                    mvp_get_hca_name(hca_type));
        int retval = mvp_check_hca_type(hca_type, my_rank);
        if (retval) {
            PRINT_INFO((my_rank == 0),
                       "Falling back to Automatic HCA detection\n");
            hca_type = MVP_HCA_UNKWN;
        } else {
            return hca_type;
        }
    }

#ifdef HAVE_LIBPSM2
    hca_type = MVP_HCA_INTEL_HFI_100;
#elif HAVE_LIBPSM_INFINIPATH
    hca_type = MVP_HCA_QLGIC_QIB;
#else
    hca_type = MVP_HCA_UNKWN;
#endif

    return hca_type;
}
#endif

#if defined(HAVE_LIBIBVERBS)
mvp_arch_hca_type mvp_new_get_arch_hca_type(mvp_hca_type hca_type)
{
    mvp_arch_hca_type arch_hca = mvp_get_arch_type();
    arch_hca = arch_hca << 16 | hca_type;
    arch_hca = arch_hca << 16 | (mvp_arch_num_cores)g_mvp_num_cpus;
    return arch_hca;
}

mvp_arch_hca_type mvp_get_arch_hca_type(struct ibv_device *dev)
{
    mvp_arch_hca_type arch_hca = mvp_get_arch_type();
    arch_hca = arch_hca << 16 | mvp_get_hca_type(dev);
    arch_hca = arch_hca << 16 | (mvp_arch_num_cores)g_mvp_num_cpus;
    return arch_hca;
}
#else
mvp_arch_hca_type mvp_get_arch_hca_type(void *dev)
{
    mvp_arch_hca_type arch_hca = mvp_get_arch_type();
    arch_hca = arch_hca << 16 | mvp_get_hca_type(dev);
    arch_hca = arch_hca << 16 | (mvp_arch_num_cores)g_mvp_num_cpus;
    return arch_hca;
}
#endif

#if defined(HAVE_LIBIBVERBS) && defined(CHANNEL_MRAIL)
extern int rdma_num_hcas;
mvp_multirail_info_type mvp_get_multirail_info()
{
    if (mvp_num_rail_unknown == g_mvp_multirail_info) {
        switch (rdma_num_hcas) {
            case 1:
                g_mvp_multirail_info = mvp_num_rail_1;
                break;
            case 2:
                g_mvp_multirail_info = mvp_num_rail_2;
                break;
            case 3:
                g_mvp_multirail_info = mvp_num_rail_3;
                break;
            case 4:
                g_mvp_multirail_info = mvp_num_rail_4;
                break;
            default:
                g_mvp_multirail_info = mvp_num_rail_unknown;
                break;
        }
    }
    return g_mvp_multirail_info;
}
#else
mvp_multirail_info_type mvp_get_multirail_info()
{
    return mvp_num_rail_unknown;
}

#endif
