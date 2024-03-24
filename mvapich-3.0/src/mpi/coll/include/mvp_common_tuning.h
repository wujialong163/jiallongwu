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
#ifndef _COMMON_TUNING_
#define _COMMON_TUNING_

#include "mpiimpl.h"

#define MVP_COLL_TUNING_SETUP_TABLE(_cname)                                    \
    int *mvp_##_cname##_table_ppn_conf = NULL;                                 \
    int mvp_##_cname##_num_ppn_conf = 1;                                       \
    int *mvp_size_##_cname##_tuning_table = NULL;                              \
    mvp_##_cname##_tuning_table **mvp_##_cname##_thresholds_table = NULL;      \
    int *mvp_##_cname##_indexed_table_ppn_conf = NULL;                         \
    int mvp_##_cname##_indexed_num_ppn_conf = 1;                               \
    int *mvp_size_##_cname##_indexed_tuning_table = NULL;                      \
    mvp_##_cname##_indexed_tuning_table *                                      \
        *mvp_##_cname##_indexed_thresholds_table = NULL;

#define MVP_COLL_TUNING_START_TABLE(_cname, _nconf)                            \
    {                                                                          \
        colls_arch_hca[_cname].arch_type = table_arch_tmp;                     \
        colls_arch_hca[_cname].hca_type = table_hca_tmp;                       \
        int idx = -1, nconf = _nconf;                                          \
        mvp_##_cname##_indexed_num_ppn_conf = nconf;                           \
        mvp_##_cname##_indexed_thresholds_table =                              \
            MPL_malloc(sizeof(mvp_##_cname##_indexed_tuning_table *) * nconf,  \
                       MPL_MEM_COLL);                                          \
        table_ptrs =                                                           \
            MPL_malloc(sizeof(mvp_##_cname##_indexed_tuning_table *) * nconf,  \
                       MPL_MEM_COLL);                                          \
        mvp_size_##_cname##_indexed_tuning_table =                             \
            MPL_malloc(sizeof(int) * nconf, MPL_MEM_COLL);                     \
        mvp_##_cname##_indexed_table_ppn_conf =                                \
            MPL_malloc(sizeof(int) * nconf, MPL_MEM_COLL);

#define MVP_COLL_TUNING_ADD_CONF(_cname, _ppn, _size, _name)                   \
    ++idx;                                                                     \
    mvp_##_cname##_indexed_tuning_table tmp_##_cname##_ppn[] = _name;          \
    mvp_##_cname##_indexed_table_ppn_conf[idx] = _ppn;                         \
    mvp_size_##_cname##_indexed_tuning_table[idx] = _size;                     \
    table_ptrs[idx] = tmp_##_cname##_ppn;

#if defined(_SMP_CMA_)
#define MVP_COLL_TUNING_ADD_CONF_CMA(_cname, _ppn, _size, _name)               \
    mvp_##_cname##_indexed_tuning_table tmp_cma_##_cname##_ppn[] = _name;      \
    if (MVP_SMP_USE_CMA) {                                                     \
        mvp_##_cname##_indexed_table_ppn_conf[idx] = _ppn;                     \
        mvp_size_##_cname##_indexed_tuning_table[idx] = _size;                 \
        table_ptrs[idx] = tmp_cma_##_cname##_ppn;                              \
    }
#else
#define MVP_COLL_TUNING_ADD_CONF_CMA(_cname, _ppn, _size, _name)
#endif

#define MVP_COLL_TUNING_FINISH_TABLE(_cname)                                   \
    agg_table_sum = 0;                                                         \
    for (i = 0; i < nconf; i++) {                                              \
        agg_table_sum += mvp_size_##_cname##_indexed_tuning_table[i];          \
    }                                                                          \
    mvp_##_cname##_indexed_thresholds_table[0] = MPL_malloc(                   \
        sizeof(mvp_##_cname##_indexed_tuning_table) * agg_table_sum,           \
        MPL_MEM_COLL);                                                         \
    MPIR_Memcpy(mvp_##_cname##_indexed_thresholds_table[0], table_ptrs[0],     \
                sizeof(mvp_##_cname##_indexed_tuning_table) *                  \
                    mvp_size_##_cname##_indexed_tuning_table[0]);              \
    for (i = 1; i < nconf; i++) {                                              \
        mvp_##_cname##_indexed_thresholds_table[i] =                           \
            mvp_##_cname##_indexed_thresholds_table[i - 1] +                   \
            mvp_size_##_cname##_indexed_tuning_table[i - 1];                   \
        MPIR_Memcpy(mvp_##_cname##_indexed_thresholds_table[i], table_ptrs[i], \
                    sizeof(mvp_##_cname##_indexed_tuning_table) *              \
                        mvp_size_##_cname##_indexed_tuning_table[i]);          \
    }                                                                          \
    MPL_free(table_ptrs);                                                      \
    return 0;                                                                  \
    }

#define FIND_PPN_INDEX(_cname, _locsize, _confindx, _parflag)                  \
    {                                                                          \
        int i = 0;                                                             \
        do {                                                                   \
            if (_locsize == mvp_##_cname##_indexed_table_ppn_conf[i]) {        \
                _confindx = i;                                                 \
                _parflag = 1;                                                  \
                break;                                                         \
            } else if (i < mvp_##_cname##_indexed_num_ppn_conf - 1) {          \
                if (_locsize > mvp_##_cname##_indexed_table_ppn_conf[i] &&     \
                    _locsize < mvp_##_cname##_indexed_table_ppn_conf[i + 1]) { \
                    _confindx = i + 1;                                         \
                    _parflag = 1;                                              \
                    break;                                                     \
                }                                                              \
            } else if (i == mvp_##_cname##_indexed_num_ppn_conf - 1) {         \
                if (_locsize > mvp_##_cname##_indexed_table_ppn_conf[i]) {     \
                    _confindx = i;                                             \
                    _parflag = 1;                                              \
                    break;                                                     \
                }                                                              \
            }                                                                  \
            i++;                                                               \
        } while (i < mvp_##_cname##_indexed_num_ppn_conf);                     \
    }

/* standard powers of 2 */
#define MVP_COLL_TUNING_ADD_ARCH_CONF_1(_archid, _coll)                        \
    MVP_COLL_TUNING_ADD_CONF(_coll, 1, _archid##__1PPN_CNT, _archid##__1PPN)
#define MVP_COLL_TUNING_ADD_ARCH_CONF_2(_archid, _coll)                        \
    MVP_COLL_TUNING_ADD_ARCH_CONF_1(_archid, _coll)                            \
    MVP_COLL_TUNING_ADD_CONF(_coll, 2, _archid##__2PPN_CNT, _archid##__2PPN)
#define MVP_COLL_TUNING_ADD_ARCH_CONF_4(_archid, _coll)                        \
    MVP_COLL_TUNING_ADD_ARCH_CONF_2(_archid, _coll)                            \
    MVP_COLL_TUNING_ADD_CONF(_coll, 4, _archid##__4PPN_CNT, _archid##__4PPN)
#define MVP_COLL_TUNING_ADD_ARCH_CONF_8(_archid, _coll)                        \
    MVP_COLL_TUNING_ADD_ARCH_CONF_4(_archid, _coll)                            \
    MVP_COLL_TUNING_ADD_CONF(_coll, 8, _archid##__8PPN_CNT, _archid##__8PPN)
#define MVP_COLL_TUNING_ADD_ARCH_CONF_16(_archid, _coll)                       \
    MVP_COLL_TUNING_ADD_ARCH_CONF_8(_archid, _coll)                            \
    MVP_COLL_TUNING_ADD_CONF(_coll, 16, _archid##__16PPN_CNT, _archid##__16PPN)
#define MVP_COLL_TUNING_ADD_ARCH_CONF_32(_archid, _coll)                       \
    MVP_COLL_TUNING_ADD_ARCH_CONF_16(_archid, _coll)                           \
    MVP_COLL_TUNING_ADD_CONF(_coll, 32, _archid##__32PPN_CNT, _archid##__32PPN)
#define MVP_COLL_TUNING_ADD_ARCH_CONF_64(_archid, _coll)                       \
    MVP_COLL_TUNING_ADD_ARCH_CONF_32(_archid, _coll)                           \
    MVP_COLL_TUNING_ADD_CONF(_coll, 64, _archid##__64PPN_CNT, _archid##__64PPN)
#define MVP_COLL_TUNING_ADD_ARCH_CONF_128(_archid, _coll)                      \
    MVP_COLL_TUNING_ADD_ARCH_CONF_64(_archid, _coll)                           \
    MVP_COLL_TUNING_ADD_CONF(_coll, 128, _archid##__128PPN_CNT,                \
                             _archid##__128PPN)
#define MVP_COLL_TUNING_ADD_ARCH_CONF_256(_archid, _coll)                      \
    MVP_COLL_TUNING_ADD_ARCH_CONF_128(_archid, _coll)                          \
    MVP_COLL_TUNING_ADD_CONF(_coll, 256, _archid##__256PPN_CNT,                \
                             _archid##__256PPN)

/* non-standard, arch specific version of these macros, usually include
 * half sub */
#define MVP_COLL_TUNING_ADD_ARCH_CONF_12(_archid, _coll)                       \
    MVP_COLL_TUNING_ADD_ARCH_CONF_1(_archid, _coll)                            \
    MVP_COLL_TUNING_ADD_CONF(_coll, 12, _archid##__12PPN_CNT, _archid##__12PPN)
#define MVP_COLL_TUNING_ADD_ARCH_CONF_20(_archid, _coll)                       \
    MVP_COLL_TUNING_ADD_ARCH_CONF_16(_archid, _coll)                           \
    MVP_COLL_TUNING_ADD_CONF(_coll, 20, _archid##__20PPN_CNT, _archid##__20PPN)
#define MVP_COLL_TUNING_ADD_ARCH_CONF_22(_archid, _coll)                       \
    MVP_COLL_TUNING_ADD_ARCH_CONF_16(_archid, _coll)                           \
    MVP_COLL_TUNING_ADD_CONF(_coll, 22, _archid##__22PPN_CNT, _archid##__22PPN)
#define MVP_COLL_TUNING_ADD_ARCH_CONF_24(_archid, _coll)                       \
    MVP_COLL_TUNING_ADD_ARCH_CONF_16(_archid, _coll)                           \
    MVP_COLL_TUNING_ADD_CONF(_coll, 24, _archid##__24PPN_CNT, _archid##__24PPN)
#define MVP_COLL_TUNING_ADD_ARCH_CONF_28(_archid, _coll)                       \
    MVP_COLL_TUNING_ADD_ARCH_CONF_16(_archid, _coll)                           \
    MVP_COLL_TUNING_ADD_CONF(_coll, 28, _archid##__28PPN_CNT, _archid##__28PPN)
#define MVP_COLL_TUNING_ADD_ARCH_CONF_36(_archid, _coll)                       \
    MVP_COLL_TUNING_ADD_ARCH_CONF_16(_archid, _coll)                           \
    MVP_COLL_TUNING_ADD_CONF(_coll, 36, _archid##__36PPN_CNT, _archid##__36PPN)
#define MVP_COLL_TUNING_ADD_ARCH_CONF_40(_archid, _coll)                       \
    MVP_COLL_TUNING_ADD_ARCH_CONF_32(_archid, _coll)                           \
    MVP_COLL_TUNING_ADD_CONF(_coll, 40, _archid##__40PPN_CNT, _archid##__40PPN)
#define MVP_COLL_TUNING_ADD_ARCH_CONF_44(_archid, _coll)                       \
    MVP_COLL_TUNING_ADD_ARCH_CONF_22(_archid, _coll)                           \
    MVP_COLL_TUNING_ADD_CONF(_coll, 32, _archid##__32PPN_CNT,                  \
                             _archid##__32PPN)                                 \
    MVP_COLL_TUNING_ADD_CONF(_coll, 44, _archid##__44PPN_CNT, _archid##__44PPN)
#define MVP_COLL_TUNING_ADD_ARCH_CONF_48(_archid, _coll)                       \
    MVP_COLL_TUNING_ADD_ARCH_CONF_32(_archid, _coll)                           \
    MVP_COLL_TUNING_ADD_CONF(_coll, 48, _archid##__48PPN_CNT, _archid##__48PPN)
#define MVP_COLL_TUNING_ADD_ARCH_CONF_52(_archid, _coll)                       \
    MVP_COLL_TUNING_ADD_ARCH_CONF_32(_archid, _coll)                           \
    MVP_COLL_TUNING_ADD_CONF(_coll, 52, _archid##__52PPN_CNT, _archid##__52PPN)
#define MVP_COLL_TUNING_ADD_ARCH_CONF_56(_archid, _coll)                       \
    MVP_COLL_TUNING_ADD_ARCH_CONF_28(_archid, _coll)                           \
    MVP_COLL_TUNING_ADD_CONF(_coll, 32, _archid##__32PPN_CNT,                  \
                             _archid##__32PPN)                                 \
    MVP_COLL_TUNING_ADD_CONF(_coll, 56, _archid##__56PPN_CNT, _archid##__56PPN)
#define MVP_COLL_TUNING_ADD_ARCH_CONF_112(_archid, _coll)                      \
    MVP_COLL_TUNING_ADD_ARCH_CONF_64(_archid, _coll)                           \
    MVP_COLL_TUNING_ADD_CONF(_coll, 112, _archid##__112PPN_CNT,                \
                             _archid##__112PPN)

#define MVP_COLL_TUNING_ADD_ARCH_CONF_72(_archid, _coll)                       \
    MVP_COLL_TUNING_ADD_ARCH_CONF_64(_archid, _coll)                           \
    MVP_COLL_TUNING_ADD_CONF(_coll, 36, _archid##__36PPN_CNT,                  \
                             _archid##__36PPN)                                 \
    MVP_COLL_TUNING_ADD_CONF(_coll, 72, _archid##__72PPN_CNT, _archid##__72PPN)
#define MVP_COLL_TUNING_ADD_ARCH_CONF_144(_archid, _coll)                      \
    MVP_COLL_TUNING_ADD_ARCH_CONF_72(_archid, _coll)                           \
    MVP_COLL_TUNING_ADD_CONF(_coll, 128, _archid##__128PPN_CNT,                \
                             _archid##__128PPN)                                \
    MVP_COLL_TUNING_ADD_CONF(_coll, 144, _archid##__144PPN_CNT,                \
                             _archid##__144PPN)

/* alternate version - heterogeneity need not be checked everytime, this can be
 * a case*/
#define MVP_SET_TABLE_BY_ARCH(_archid, _coll, _count, _maxppn)                 \
    MVP_COLL_TUNING_START_TABLE(_coll, _count)                                 \
    MVP_COLL_TUNING_ADD_ARCH_CONF_##_maxppn(_archid, _coll)                    \
        MVP_COLL_TUNING_FINISH_TABLE(_coll)

/* should cover autogeneration of most ARCH/HCA ids */
#define MVP_ARCH_HCA_ID_STRING(_net, _arch, _hca) _net##__##_arch##__##_hca
#define MVP_ARCH_HCA_ID_STRING_STRING(_net, _arch, _hca)                       \
#_net "__" #_arch "__" #_hca

#define MVP_ARCH_HCA_ID(_arch, _hca) (_arch << 16) | _hca

/* fallback cases */
/* has the added benefit of revealing bad tuning tables with duplicates, but
 * only does fallback support for the ARCH. There are many more architectures
 * than HCAs. And it makes very little sense to pick a random arch for a
 * specific HCA */
/* requires that NET and COLLECTIVE be defined before a set of these are
 * established */
#define MVP_ARCH_FALLBACK_CASE(_arch)                                          \
    case MVP_ARCH_HCA_ID(MVP_ARCH_##_arch, MVP_HCA_ANY):

#define MVP_HCA_FALLBACK_CASE(_hca)                                            \
    case MVP_ARCH_HCA_ID(MVP_ARCH_ANY, MVP_HCA_##_hca):

#define MVP_ARCH_HCA_CASE(_arch, _hca)                                         \
    case MVP_ARCH_HCA_ID(MVP_ARCH_##_arch, MVP_HCA_##_hca):

/* exclusively the primary match cases for a given arch/hca combo */
/* has the added benefit of revealing bad tuning tables with duplicates */
/* requires that NET and COLLECTIVE be defined before a set of these are
 * established */
#define MVP_ARCH_HCA_CASE_SETX(_net, _arch, _hca, _count, _maxppn)             \
    MVP_ARCH_HCA_CASE(_arch, _hca)                                             \
    MVP_SET_TABLE_BY_ARCH(MVP_ARCH_HCA_ID_STRING(_net, _arch, _hca),           \
                          COLLECTIVE, _count, _maxppn)                         \
    break;

/* the generalized case, specific and arch fallback */
#define MVP_ARCH_HCA_CASE_SET(_net, _arch, _hca, _count, _maxppn)              \
    MVP_ARCH_FALLBACK_CASE(_arch)                                              \
    MVP_ARCH_HCA_CASE_SETX(_net, _arch, _hca, _count, _maxppn)

/* architecture fallbacks */
enum mvp_arch_defaults {
    MVP_ARCH_DEFAULT__INTEL =
        MVP_ARCH_HCA_ID(MVP_ARCH_INTEL_PLATINUM_8280_2S_56, MVP_HCA_MLX_CX_EDR),
    MVP_ARCH_DEFAULT__AMD =
        MVP_ARCH_HCA_ID(MVP_ARCH_AMD_EPYC_7763_128, MVP_HCA_MLX_CX_HDR),
    MVP_ARCH_DEFAULT__IBM =
        MVP_ARCH_HCA_ID(MVP_ARCH_IBM_POWER9, MVP_HCA_MLX_CX_EDR),
    MVP_ARCH_DEFAULT__ARM =
        MVP_ARCH_HCA_ID(MVP_ARCH_ARM_CAVIUM_V8_2S_32, MVP_HCA_MLX_CX_EDR),
};

/* defined enum for right hand side values used in mvp collective algorithms
 selection. */
enum mvp_bcast_tuning {
    SHMEM_BCAST_INTRA,
    KNOMIAL_BCAST_INTRA,
    BCAST_BIONOMIAL_INTRA,
    BCAST_SCATTER_DOUBLING_ALLGATHER_FLAT,
    BCAST_SCATTER_DOUBLING_ALLGATHER,
    BCAST_SCATTER_RING_ALLGATEHR_FLAT,
    BCAST_SCATTER_RING_ALLGATHER,
    BCAST_SCATTER_RING_ALLGATHER_SHM,
    KNOMIAL_BCAST_INTER_NODE_WRAPPER,
    PIPELINED_BCAST,
    PIPELINED_BCAST_ZCPY
};

enum mvp_reduce_tuning {
    REDUCE_BINOMIAL = 1,
    REDUCE_INTER_KNOMIAL,
    REDUCE_INTRA_KNOMIAL,
    REDUCE_SHMEM,
    REDUCE_RDSC_GATHER,
    REDUCE_ZCPY,
    REDUCE_X1, /* place holder for master-x algorithm */
    REDUCE_ALLREDUCE
};

enum mvp_allreduce_tuning {
    ALLREDUCE_P2P_RD = 1,
    ALLREDUCE_P2P_RS,
    ALLREDUCE_MCAST_2LEVEL,
    ALLREDUCE_MCAST_RSA,
    ALLREDUCE_SHMEM_REDUCE,
    ALLREDUCE_P2P_REDUCE,
    ALLREDUCE_X1, /* place holder for master-x algorithm */
    ALLREDUCE_RED_SCAT_ALLGA_COLL,
    ALLREDUCE_RING,
    ALLREDUCE_X2, /* place holder for master-x algorithm */
    ALLREDUCE_SOCK_AWARE,
    ALLREDUCE_X3, /* place holder for master-x algorithm */
};

enum mvp_scatter_tuning {
    SCATTER_BINOMIAL = 1,
    SCATTER_DIRECT,
    SCATTER_TWO_LEVEL_BINOMIAL,
    SCATTER_TWO_LEVEL_DIRECT,
    SCATTER_MCAST
};

enum {
    RED_SCAT_BASIC = 1,
    RED_SCAT_REC_HALF,
    RED_SCAT_PAIRWISE,
    RED_SCAT_RING,
    RED_SCAT_RING_2LVL,
};

enum mvp_alltoallv_tuning {
    ALLTOALLV_INTRA_SCATTER_MVP,
    ALLTOALLV_INTRA_MVP,
    ALLTOALLV_MVP
};

enum mvp_ibcast_tuning {
    IBCAST_BINOMIAL = 1,
    IBCAST_SCATTER_REC_DBL_ALLGATHER,
    IBCAST_SCATTER_RING_ALLGATHER
};

enum mvp_igather_tuning {
    IGATHER_BINOMIAL = 1,
#ifdef _USE_CORE_DIRECT_
    IGATHER_DIRECT
#endif /* _USE_CORE_DIRECT_ */
};

enum mvp_iallreduce_tuning {
    IALLREDUCE_NAIVE = 1,
    IALLREDUCE_REDSCAT_ALLGATHER,
    IALLREDUCE_REC_DBL,
#if defined(_SHARP_SUPPORT_)
    SHARP_IALLREDUCE_MVP
#endif /*defined (_SHARP_SUPPORT_)*/
};

enum mvp_ired_scat_tuning {
    IREDUCE_SCATTER_PAIRWISE = 1,
    IREDUCE_SCATTER_REC_HLV,
    IREDUCE_SCATTER_REC_DBL,
    IREDUCE_SCATTER_NONCOMM
};

enum mvp_ialltoall_tuning {
    IALLTOALL_BRUCK = 1,
    IALLTOALL_PERM_SR,
    IALLTOALL_PAIRWISE
};

enum mvp_ireduce_tuning { IREDUCE_BINOMIAL = 1, IREDUCE_REDSCAT_GATHER };

enum mvp_iscatter_tuning {
    ISCATTER_BINOMIAL = 1,
#ifdef _USE_CORE_DIRECT_
    ISCATTER_DIRECT
#endif /* _USE_CORE_DIRECT_ */
};

enum mvp_ibarrier_tuning { IBARRIER_INTRA = 1 };
#endif
