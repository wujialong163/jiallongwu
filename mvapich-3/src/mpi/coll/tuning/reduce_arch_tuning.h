/*
 * Copyright (c) 2001-2023, The Ohio State University. All rights
 * reserved.
 *
 * This file is part of the MVAPICH software package developed by the
 * team members of The Ohio State University's Network-Based Computing
 * Laboratory (NBCL), headed by Professor Dhabaleswar K. (DK) Panda.
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level MVAPICH directory.
 */

#include "reduce/gen2_RI2_1ppn.h"
#include "reduce/gen2_RI2_2ppn.h"
#include "reduce/gen2_RI2_4ppn.h"
#include "reduce/gen2_RI2_8ppn.h"
#include "reduce/gen2_RI2_16ppn.h"
#include "reduce/gen2_RI2_28ppn.h"
#include "reduce/cxi_TIOGA_1ppn.h"
#include "reduce/cxi_TIOGA_2ppn.h"
#include "reduce/cxi_TIOGA_4ppn.h"
#include "reduce/cxi_TIOGA_8ppn.h"
#include "reduce/cxi_TIOGA_16ppn.h"
#include "reduce/cxi_TIOGA_32ppn.h"
#include "reduce/cxi_TIOGA_64ppn.h"
#include "reduce/gen2_RI2_1ppn.h"
#include "reduce/gen2_RI2_2ppn.h"
#include "reduce/gen2_RI2_4ppn.h"
#include "reduce/gen2_RI2_8ppn.h"
#include "reduce/gen2_RI2_16ppn.h"
#include "reduce/gen2_RI2_28ppn.h"
#include "reduce/gen2_RI_1ppn.h"
#include "reduce/gen2_RI_2ppn.h"
#include "reduce/gen2_RI_4ppn.h"
#include "reduce/gen2_RI_8ppn.h"
#include "reduce/gen2_RI_1ppn.h"
#include "reduce/gen2_RI_2ppn.h"
#include "reduce/gen2_RI_4ppn.h"
#include "reduce/gen2_RI_8ppn.h"
#include "reduce/psm_RI_1ppn.h"
#include "reduce/psm_RI_2ppn.h"
#include "reduce/psm_RI_8ppn.h"
#include "reduce/psm_INTEL_XEON_X5650_12_MVP_HCA_QLGIC_QIB_1ppn.h"
#include "reduce/psm_INTEL_XEON_X5650_12_MVP_HCA_QLGIC_QIB_12ppn.h"
#include "reduce/gen2_AMD_OPTERON_6136_32_MLX_CX_QDR_1ppn.h"
#include "reduce/gen2_AMD_OPTERON_6136_32_MLX_CX_QDR_2ppn.h"
#include "reduce/gen2_AMD_OPTERON_6136_32_MLX_CX_QDR_4ppn.h"
#include "reduce/gen2_AMD_OPTERON_6136_32_MLX_CX_QDR_8ppn.h"
#include "reduce/gen2_AMD_OPTERON_6136_32_MLX_CX_QDR_16ppn.h"
#include "reduce/gen2_AMD_OPTERON_6136_32_MLX_CX_QDR_32ppn.h"
#include "reduce/gen2_INTEL_XEON_X5650_12_MLX_CX_QDR_1ppn.h"
#include "reduce/gen2_INTEL_XEON_X5650_12_MLX_CX_QDR_2ppn.h"
#include "reduce/gen2_INTEL_XEON_X5650_12_MLX_CX_QDR_12ppn.h"
#include "reduce/gen2_INTEL_XEON_E5_2690_V2_2S_20_MLX_CX_CONNIB_1ppn.h"
#include "reduce/gen2_INTEL_XEON_E5_2690_V2_2S_20_MLX_CX_CONNIB_2ppn.h"
#include "reduce/gen2_INTEL_XEON_E5_2690_V2_2S_20_MLX_CX_CONNIB_4ppn.h"
#include "reduce/gen2_INTEL_XEON_E5_2690_V2_2S_20_MLX_CX_CONNIB_8ppn.h"
#include "reduce/gen2_INTEL_XEON_E5_2690_V2_2S_20_MLX_CX_CONNIB_16ppn.h"
#include "reduce/gen2_INTEL_XEON_E5_2690_V2_2S_20_MLX_CX_CONNIB_20ppn.h"
#include "reduce/gen2_INTEL_XEON_E5_2630_V2_2S_12_MLX_CX_CONNIB_1ppn.h"
#include "reduce/gen2_INTEL_XEON_E5_2630_V2_2S_12_MLX_CX_CONNIB_12ppn.h"
#include "reduce/gen2_INTEL_XEON_E5_2670_16_MLX_CX_QDR_1ppn.h"
#include "reduce/gen2_INTEL_XEON_E5_2670_16_MLX_CX_QDR_2ppn.h"
#include "reduce/gen2_INTEL_XEON_E5_2670_16_MLX_CX_QDR_16ppn.h"
#include "reduce/gen2_INTEL_XEON_E5_2670_16_MLX_CX_FDR_1ppn.h"
#include "reduce/gen2_INTEL_XEON_E5_2670_16_MLX_CX_FDR_2ppn.h"
#include "reduce/gen2_INTEL_XEON_E5_2670_16_MLX_CX_FDR_4ppn.h"
#include "reduce/gen2_INTEL_XEON_E5_2670_16_MLX_CX_FDR_8ppn.h"
#include "reduce/gen2_INTEL_XEON_E5_2670_16_MLX_CX_FDR_16ppn.h"
#include "reduce/gen2_INTEL_XEON_E5_2670_16_MLX_CX_FDR_1ppn.h"
#include "reduce/gen2_INTEL_XEON_E5_2670_16_MLX_CX_FDR_2ppn.h"
#include "reduce/gen2_INTEL_XEON_E5_2670_16_MLX_CX_FDR_16ppn.h"
#include "reduce/gen2_INTEL_XEON_E5_2670_16_MLX_CX_QDR_1ppn.h"
#include "reduce/gen2_INTEL_XEON_E5_2670_16_MLX_CX_QDR_2ppn.h"
#include "reduce/gen2_INTEL_XEON_E5_2670_16_MLX_CX_QDR_4ppn.h"
#include "reduce/gen2_INTEL_XEON_E5_2670_16_MLX_CX_QDR_8ppn.h"
#include "reduce/gen2_INTEL_XEON_E5_2670_16_MLX_CX_QDR_16ppn.h"
#include "reduce/gen2_INTEL_XEON_E5_2680_16_MLX_CX_FDR_1ppn.h"
#include "reduce/gen2_INTEL_XEON_E5_2680_16_MLX_CX_FDR_2ppn.h"
#include "reduce/gen2_INTEL_XEON_E5_2680_16_MLX_CX_FDR_4ppn.h"
#include "reduce/gen2_INTEL_XEON_E5_2680_16_MLX_CX_FDR_8ppn.h"
#include "reduce/gen2_INTEL_XEON_E5_2680_16_MLX_CX_FDR_16ppn.h"
#include "reduce/gen2_INTEL_XEON_E5_2680_16_MLX_CX_FDR_1ppn.h"
#include "reduce/gen2_INTEL_XEON_E5_2680_16_MLX_CX_FDR_2ppn.h"
#include "reduce/gen2_INTEL_XEON_E5_2680_16_MLX_CX_FDR_4ppn.h"
#include "reduce/gen2_INTEL_XEON_E5_2680_16_MLX_CX_FDR_16ppn.h"
#include "reduce/gen2_INTEL_XEON_E5_2680_24_MLX_CX_FDR_1ppn.h"
#include "reduce/gen2_INTEL_XEON_E5_2680_24_MLX_CX_FDR_24ppn.h"
#include "reduce/gen2__INTEL_GOLD_6148_2S_40__MLX_CX_EDR__1ppn.h"
#include "reduce/gen2__INTEL_GOLD_6148_2S_40__MLX_CX_EDR__2ppn.h"
#include "reduce/gen2__INTEL_GOLD_6148_2S_40__MLX_CX_EDR__4ppn.h"
#include "reduce/gen2__INTEL_GOLD_6148_2S_40__MLX_CX_EDR__8ppn.h"
#include "reduce/gen2__INTEL_GOLD_6148_2S_40__MLX_CX_EDR__16ppn.h"
#include "reduce/gen2__INTEL_GOLD_6148_2S_40__MLX_CX_EDR__32ppn.h"
#include "reduce/gen2__INTEL_GOLD_6148_2S_40__MLX_CX_EDR__40ppn.h"
#include "reduce/psm_INTEL_XEON_E5_2695_V3_2S_28_INTEL_HFI_100_1ppn.h"
#include "reduce/psm_INTEL_XEON_E5_2695_V3_2S_28_INTEL_HFI_100_2ppn.h"
#include "reduce/psm_INTEL_XEON_E5_2695_V3_2S_28_INTEL_HFI_100_4ppn.h"
#include "reduce/psm_INTEL_XEON_E5_2695_V3_2S_28_INTEL_HFI_100_8ppn.h"
#include "reduce/psm_INTEL_XEON_E5_2695_V3_2S_28_INTEL_HFI_100_16ppn.h"
#include "reduce/psm_INTEL_XEON_E5_2695_V3_2S_28_INTEL_HFI_100_28ppn.h"
#include "reduce/psm_INTEL_XEON_E5_2695_V4_2S_36_INTEL_HFI_100_1ppn.h"
#include "reduce/psm_INTEL_XEON_E5_2695_V4_2S_36_INTEL_HFI_100_2ppn.h"
#include "reduce/psm_INTEL_XEON_E5_2695_V4_2S_36_INTEL_HFI_100_4ppn.h"
#include "reduce/psm_INTEL_XEON_E5_2695_V4_2S_36_INTEL_HFI_100_8ppn.h"
#include "reduce/psm_INTEL_XEON_E5_2695_V4_2S_36_INTEL_HFI_100_16ppn.h"
#include "reduce/psm_INTEL_XEON_E5_2695_V4_2S_36_INTEL_HFI_100_36ppn.h"
#include "reduce/psm_INTEL_XEON_PHI_7250_68_INTEL_HFI_100_1ppn.h"
#include "reduce/psm_INTEL_XEON_PHI_7250_68_INTEL_HFI_100_2ppn.h"
#include "reduce/psm_INTEL_XEON_PHI_7250_68_INTEL_HFI_100_4ppn.h"
#include "reduce/psm_INTEL_XEON_PHI_7250_68_INTEL_HFI_100_8ppn.h"
#include "reduce/psm_INTEL_XEON_PHI_7250_68_INTEL_HFI_100_16ppn.h"
#include "reduce/psm_INTEL_XEON_PHI_7250_68_INTEL_HFI_100_32ppn.h"
#include "reduce/psm_INTEL_XEON_PHI_7250_68_INTEL_HFI_100_64ppn.h"
#include "reduce/psm_INTEL_PLATINUM_8170_2S_52_INTEL_HFI_100_1ppn.h"
#include "reduce/psm_INTEL_PLATINUM_8170_2S_52_INTEL_HFI_100_2ppn.h"
#include "reduce/psm_INTEL_PLATINUM_8170_2S_52_INTEL_HFI_100_4ppn.h"
#include "reduce/psm_INTEL_PLATINUM_8170_2S_52_INTEL_HFI_100_8ppn.h"
#include "reduce/psm_INTEL_PLATINUM_8170_2S_52_INTEL_HFI_100_16ppn.h"
#include "reduce/psm_INTEL_PLATINUM_8170_2S_52_INTEL_HFI_100_26ppn.h"
#include "reduce/psm_INTEL_PLATINUM_8170_2S_52_INTEL_HFI_100_32ppn.h"
#include "reduce/psm_INTEL_PLATINUM_8170_2S_52_INTEL_HFI_100_52ppn.h"
#include "reduce/gen2_ARM_CAVIUM_V8_2S_28_MLX_CX_FDR_1ppn.h"
#include "reduce/gen2_ARM_CAVIUM_V8_2S_28_MLX_CX_FDR_2ppn.h"
#include "reduce/gen2_ARM_CAVIUM_V8_2S_28_MLX_CX_FDR_4ppn.h"
#include "reduce/gen2_ARM_CAVIUM_V8_2S_28_MLX_CX_FDR_8ppn.h"
#include "reduce/gen2_ARM_CAVIUM_V8_2S_28_MLX_CX_FDR_16ppn.h"
#include "reduce/gen2_ARM_CAVIUM_V8_2S_28_MLX_CX_FDR_24ppn.h"
#include "reduce/gen2_IBM_POWER8_MLX_CX_EDR_1ppn.h"
#include "reduce/gen2_IBM_POWER8_MLX_CX_EDR_2ppn.h"
#include "reduce/gen2_IBM_POWER8_MLX_CX_EDR_4ppn.h"
#include "reduce/gen2_IBM_POWER8_MLX_CX_EDR_8ppn.h"
#include "reduce/gen2_IBM_POWER9_MLX_CX_EDR_1ppn.h"
#include "reduce/gen2_IBM_POWER9_MLX_CX_EDR_2ppn.h"
#include "reduce/gen2_IBM_POWER9_MLX_CX_EDR_4ppn.h"
#include "reduce/gen2_IBM_POWER9_MLX_CX_EDR_8ppn.h"
#include "reduce/gen2_IBM_POWER9_MLX_CX_EDR_16ppn.h"
#include "reduce/gen2_IBM_POWER9_MLX_CX_EDR_22ppn.h"
#include "reduce/gen2_IBM_POWER9_MLX_CX_EDR_32ppn.h"
#include "reduce/gen2_IBM_POWER9_MLX_CX_EDR_44ppn.h"
#include "reduce/gen2_IBM_POWER9_MLX_CX_EDR_1ppn.h"
#include "reduce/gen2_IBM_POWER9_MLX_CX_EDR_2ppn.h"
#include "reduce/gen2_IBM_POWER9_MLX_CX_EDR_4ppn.h"
#include "reduce/gen2_IBM_POWER9_MLX_CX_EDR_6ppn.h"
#include "reduce/gen2_IBM_POWER9_MLX_CX_EDR_8ppn.h"
#include "reduce/gen2_IBM_POWER9_MLX_CX_EDR_16ppn.h"
#include "reduce/gen2_IBM_POWER9_MLX_CX_EDR_22ppn.h"
#include "reduce/gen2_IBM_POWER9_MLX_CX_EDR_32ppn.h"
#include "reduce/gen2_IBM_POWER9_MLX_CX_EDR_44ppn.h"
#include "reduce/gen2_AMD_EPYC_1ppn.h"
#include "reduce/gen2_AMD_EPYC_2ppn.h"
#include "reduce/gen2_AMD_EPYC_4ppn.h"
#include "reduce/gen2_AMD_EPYC_8ppn.h"
#include "reduce/gen2_AMD_EPYC_16ppn.h"
#include "reduce/gen2_AMD_EPYC_32ppn.h"
#include "reduce/gen2_AMD_EPYC_64ppn.h"
#include "reduce/gen2_AMD_EPYC_1ppn.h"
#include "reduce/gen2_AMD_EPYC_2ppn.h"
#include "reduce/gen2_AMD_EPYC_4ppn.h"
#include "reduce/gen2_AMD_EPYC_8ppn.h"
#include "reduce/gen2_AMD_EPYC_16ppn.h"
#include "reduce/gen2_AMD_EPYC_32ppn.h"
#include "reduce/gen2_AMD_EPYC_64ppn.h"
#include "reduce/gen2_AMD_EPYC_VENUS_64ppn.h"
#include "reduce/gen2_AMD_EPYC_ROME_1ppn.h"
#include "reduce/gen2_AMD_EPYC_ROME_2ppn.h"
#include "reduce/gen2_AMD_EPYC_ROME_4ppn.h"
#include "reduce/gen2_AMD_EPYC_ROME_8ppn.h"
#include "reduce/gen2_AMD_EPYC_ROME_16ppn.h"
#include "reduce/gen2_AMD_EPYC_ROME_32ppn.h"
#include "reduce/gen2_AMD_EPYC_ROME_60ppn.h"
#include "reduce/gen2_AMD_EPYC_ROME_64ppn.h"
#include "reduce/gen2_AMD_EPYC_ROME_120ppn.h"
#include "reduce/gen2_AMD_EPYC_ROME_128ppn.h"
#include "reduce/gen2_NOWHASWELL_1ppn.h"
#include "reduce/gen2_NOWHASWELL_2ppn.h"
#include "reduce/gen2_NOWHASWELL_4ppn.h"
#include "reduce/gen2_NOWHASWELL_8ppn.h"
#include "reduce/gen2_NOWHASWELL_16ppn.h"
#include "reduce/gen2_NOWHASWELL_20ppn.h"
#include "reduce/gen2_FRONTERA_1ppn.h"
#include "reduce/gen2_FRONTERA_2ppn.h"
#include "reduce/gen2_FRONTERA_4ppn.h"
#include "reduce/gen2_FRONTERA_8ppn.h"
#include "reduce/gen2_FRONTERA_16ppn.h"
#include "reduce/gen2_FRONTERA_28ppn.h"
#include "reduce/gen2_FRONTERA_32ppn.h"
#include "reduce/gen2_FRONTERA_56ppn.h"
#include "reduce/gen2_MVP_ARCH_INTEL_XEON_E5_2620_V4_2S_16_1ppn.h"
#include "reduce/gen2_MVP_ARCH_INTEL_XEON_E5_2620_V4_2S_16_2ppn.h"
#include "reduce/gen2_MVP_ARCH_INTEL_XEON_E5_2620_V4_2S_16_4ppn.h"
#include "reduce/gen2_MVP_ARCH_INTEL_XEON_E5_2620_V4_2S_16_8ppn.h"
#include "reduce/gen2_MVP_ARCH_INTEL_XEON_E5_2620_V4_2S_16_16ppn.h"
#include "reduce/gen2_MAYER_1ppn.h"
#include "reduce/gen2_MAYER_2ppn.h"
#include "reduce/gen2_MAYER_4ppn.h"
#include "reduce/gen2_MAYER_8ppn.h"
#include "reduce/gen2_MAYER_16ppn.h"
#include "reduce/gen2_MAYER_28ppn.h"
#include "reduce/gen2_MAYER_32ppn.h"
#include "reduce/gen2_MAYER_56ppn.h"
#include "reduce/gen2_ARM_CAVIUM_V8_2S_32_MLX_CX_EDR_1ppn.h"
#include "reduce/gen2_ARM_CAVIUM_V8_2S_32_MLX_CX_EDR_2ppn.h"
#include "reduce/gen2_ARM_CAVIUM_V8_2S_32_MLX_CX_EDR_4ppn.h"
#include "reduce/gen2_ARM_CAVIUM_V8_2S_32_MLX_CX_EDR_8ppn.h"
#include "reduce/gen2_ARM_CAVIUM_V8_2S_32_MLX_CX_EDR_16ppn.h"
#include "reduce/gen2_ARM_CAVIUM_V8_2S_32_MLX_CX_EDR_32ppn.h"
#include "reduce/gen2_ARM_CAVIUM_V8_2S_32_MLX_CX_EDR_64ppn.h"
#include "reduce/gen2_AMD_EPYC_7401_24_1ppn.h"
#include "reduce/gen2_AMD_EPYC_7401_24_2ppn.h"
#include "reduce/gen2_AMD_EPYC_7401_24_4ppn.h"
#include "reduce/gen2_AMD_EPYC_7401_24_8ppn.h"
#include "reduce/gen2_AMD_EPYC_7401_24_16ppn.h"
#include "reduce/gen2_AMD_EPYC_7401_24_32ppn.h"
#include "reduce/gen2_AMD_EPYC_7401_24_48ppn.h"
#include "reduce/gen2_AMD_EPYC_7763_128_1ppn.h"
#include "reduce/gen2_AMD_EPYC_7763_128_2ppn.h"
#include "reduce/gen2_AMD_EPYC_7763_128_4ppn.h"
#include "reduce/gen2_AMD_EPYC_7763_128_8ppn.h"
#include "reduce/gen2_AMD_EPYC_7763_128_16ppn.h"
#include "reduce/gen2_AMD_EPYC_7763_128_32ppn.h"
#include "reduce/gen2_AMD_EPYC_7763_128_64ppn.h"
#include "reduce/gen2_AMD_EPYC_7763_128_128ppn.h"
#include "reduce/gen2__AMD_EPYC_7763_128__MLX_CX_HDR__1ppn.h"
#include "reduce/gen2__AMD_EPYC_7763_128__MLX_CX_HDR__2ppn.h"
#include "reduce/gen2__AMD_EPYC_7763_128__MLX_CX_HDR__4ppn.h"
#include "reduce/gen2__AMD_EPYC_7763_128__MLX_CX_HDR__8ppn.h"
#include "reduce/gen2__AMD_EPYC_7763_128__MLX_CX_HDR__16ppn.h"
#include "reduce/gen2__AMD_EPYC_7763_128__MLX_CX_HDR__32ppn.h"
#include "reduce/gen2__AMD_EPYC_7763_128__MLX_CX_HDR__64ppn.h"
#include "reduce/gen2__AMD_EPYC_7763_128__MLX_CX_HDR__128ppn.h"
#include "reduce/cxi_TIOGA_1ppn.h"
#include "reduce/cxi_TIOGA_2ppn.h"
#include "reduce/cxi_TIOGA_4ppn.h"
#include "reduce/cxi_TIOGA_8ppn.h"
#include "reduce/cxi_TIOGA_16ppn.h"
#include "reduce/cxi_TIOGA_32ppn.h"
#include "reduce/cxi_TIOGA_64ppn.h"
#include "reduce/psm_Stampede2_1ppn.h"
#include "reduce/psm_Stampede2_1ppn.h"
#include "reduce/psm_Stampede2_2ppn.h"
#include "reduce/psm_Stampede2_4ppn.h"
#include "reduce/psm_Stampede2_8ppn.h"
#include "reduce/psm_Stampede2_16ppn.h"
#include "reduce/psm_Stampede2_32ppn.h"
#include "reduce/psm_Stampede2_48ppn.h"
#include "reduce/gen2_INTEL_PLATINUM_8480_2S_112_MLX_CX_NDR_1ppn.h"
#include "reduce/gen2_INTEL_PLATINUM_8480_2S_112_MLX_CX_NDR_2ppn.h"
#include "reduce/gen2_INTEL_PLATINUM_8480_2S_112_MLX_CX_NDR_4ppn.h"
#include "reduce/gen2_INTEL_PLATINUM_8480_2S_112_MLX_CX_NDR_8ppn.h"
#include "reduce/gen2_INTEL_PLATINUM_8480_2S_112_MLX_CX_NDR_16ppn.h"
#include "reduce/gen2_INTEL_PLATINUM_8480_2S_112_MLX_CX_NDR_32ppn.h"
#include "reduce/gen2_INTEL_PLATINUM_8480_2S_112_MLX_CX_NDR_56ppn.h"
#include "reduce/gen2_INTEL_PLATINUM_8480_2S_112_MLX_CX_NDR_64ppn.h"
#include "reduce/gen2_INTEL_PLATINUM_8480_2S_112_MLX_CX_NDR_112ppn.h"
#include "reduce/gen2_ARM_GRACE_2S_144_MLX_CX_NDR_1ppn.h"
#include "reduce/gen2_ARM_GRACE_2S_144_MLX_CX_NDR_2ppn.h"
#include "reduce/gen2_ARM_GRACE_2S_144_MLX_CX_NDR_4ppn.h"
#include "reduce/gen2_ARM_GRACE_2S_144_MLX_CX_NDR_8ppn.h"
#include "reduce/gen2_ARM_GRACE_2S_144_MLX_CX_NDR_16ppn.h"
#include "reduce/gen2_ARM_GRACE_2S_144_MLX_CX_NDR_32ppn.h"
#include "reduce/gen2_ARM_GRACE_2S_144_MLX_CX_NDR_36ppn.h"
#include "reduce/gen2_ARM_GRACE_2S_144_MLX_CX_NDR_64ppn.h"
#include "reduce/gen2_ARM_GRACE_2S_144_MLX_CX_NDR_72ppn.h"
#include "reduce/gen2_ARM_GRACE_2S_144_MLX_CX_NDR_128ppn.h"
#include "reduce/gen2_ARM_GRACE_2S_144_MLX_CX_NDR_144ppn.h"
