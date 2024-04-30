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

#ifndef MVP_ARCH_HCA_DETECT_H
#define MVP_ARCH_HCA_DETECT_H

#include <stdint.h>
#include "mvp_arch_hca.h"

#if defined(HAVE_LIBIBVERBS)
#include <infiniband/verbs.h>
#endif

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
    mvp_hca_t hca_type;
    mvp_arch_t arch_type;
};

extern mvp_arch_t table_arch_tmp;
extern mvp_hca_t table_hca_tmp;
extern int mvp_suppress_hca_warnings;

/* ************************ FUNCTION DECLARATIONS ************************** */

/* Check arch-hca type */
int mvp_is_arch_hca_type(mvp_arch_hca_info_t arch_hca_type,
                         mvp_arch_t arch_type, mvp_hca_t hca_type);

/* Get architecture-hca type */
#if defined(HAVE_LIBIBVERBS)
mvp_arch_hca_info_t mvp_get_arch_hca_type(struct ibv_device *dev);
mvp_arch_hca_info_t mvp_new_get_arch_hca_type(mvp_hca_t hca_type);
#else
mvp_arch_hca_info_t mvp_get_arch_hca_type(void *dev);
#endif

/* Get architecture type */
mvp_arch_t mvp_get_arch_type(void);

/* Get card type */
#if defined(HAVE_LIBIBVERBS)
mvp_hca_t mvp_get_hca_type(struct ibv_device *dev);
mvp_hca_t mvp_new_get_hca_type(struct ibv_context *ctx, struct ibv_device *dev,
                               uint64_t *guid);
#else
mvp_hca_t mvp_get_hca_type(void *dev);
#endif

#endif /*  #ifndef MVP_ARCH_HCA_DETECT_H */
