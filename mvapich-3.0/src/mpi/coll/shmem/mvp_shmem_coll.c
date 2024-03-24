/*
 * This source file was derived from code in the MPICH-GM implementation
 * of MPI, which was developed by Myricom, Inc.
 * Myricom MPICH-GM ch_gm backend
 * Copyright (c) 2001 by Myricom, Inc.
 * All rights reserved.
 */

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

/*
 * TODO: this file should be moved to the ch3 directory. It implements ch3
 * specific functionality. We should define an MPID_ level interface that
 * exposes these functions to the MPI_ level if needed. For now most of these
 * files appear to only be used inside ch3 though.
 */
#define _GNU_SOURCE 1

#include "mpichconf.h"
#include "mpiimpl.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include "upmi.h"

#include <sched.h>

#ifdef MAC_OSX
#include <netinet/in.h>
#endif

#include "mvp_common_tuning.h"
#include "mvp_shmem_bar.h"
#include "mvp_coll_shmem.h"
#include "coll_shmem_internal.h"
#include "gather_tuning.h"
#include "bcast_tuning.h"
#include "alltoall_tuning.h"
#include "alltoallv_tuning.h"
#include "scatter_tuning.h"
#include "allreduce_tuning.h"
#include "reduce_tuning.h"
#include "allgather_tuning.h"
#include "reduce_scatter_tuning.h"
#include "reduce_scatter_block_tuning.h"
#include "allgatherv_tuning.h"
#include "igather_tuning.h"
#include "ibcast_tuning.h"
#include "ialltoall_tuning.h"
#include "ialltoallv_tuning.h"
#include "iscatter_tuning.h"
#include "iallreduce_tuning.h"
#include "ireduce_tuning.h"
#include "iallgather_tuning.h"
#include "ireduce_scatter_tuning.h"
#include "iallgatherv_tuning.h"
#include "ibarrier_tuning.h"
#if defined(CKPT)
#include <cr.h>
#endif
#if defined(_ENABLE_CUDA_)
#include "datatype.h"
#endif
#include "mvp_debug_utils.h"
#if defined(_MCST_SUPPORT_)
#include "ibv_mcast.h"
#endif

/*
=== BEGIN_MPI_T_MVP_CVAR_INFO_BLOCK ===

cvars:
    - name        : MVP_SHMEM_DIR
      category    : CH3
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This parameter can be used to specify the path to the shared memory
        files for intra-node communication.


    - name        : MVP_KNOMIAL_INTER_LEADER_THRESHOLD
      category    : COLLECTIVE
      type        : int
      default     : (64 * 1024)
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_KNOMIAL_INTER_NODE_FACTOR
      category    : COLLECTIVE
      type        : int
      default     : 4
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This defines the degree of the knomial operation during the
        inter-node knomial broadcast phase.

    - name        : MVP_KNOMIAL_INTRA_NODE_FACTOR
      category    : COLLECTIVE
      type        : int
      default     : 4
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This defines the degree of the knomial operation during the
        intra-node knomial broadcast phase.

    - name        : MVP_KNOMIAL_INTRA_NODE_THRESHOLD
      category    : COLLECTIVE
      type        : int
      default     : (128 * 1024)
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_SHMEM_COLL_MAX_MSG_SIZE
      category    : COLLECTIVE
      type        : int
      default     : (128 * 1024)
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This parameter can be used to select the max buffer size of
        message for shared memory collectives.

    - name        : MVP_SHMEM_COLL_NUM_COMM
      category    : COLLECTIVE
      type        : int
      default     : 32
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This parameter can be used to select the number of
        communicators using shared memory collectives.

    - name        : MVP_SHMEM_COLL_NUM_PROCS
      category    : COLLECTIVE
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_SHMEM_COLL_SPIN_COUNT
      category    : COLLECTIVE
      type        : int
      default     : 5
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_OSU_COLLECTIVES
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_OSU_NB_COLLECTIVES
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_SHMEM_COLL
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Use shared memory for collective communication. Set this to 0
        for disabling shared memory collectives.

    - name        : MVP_ENABLE_SOCKET_AWARE_COLLECTIVES
      category    : COLLECTIVE
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This parameter enables/disables support for socket-aware
        collective communication. The parameter MVP_USE_SHMEM_COLL
        must be set to 1 for this to work.

    - name        : MVP_USE_BLOCKING
      category    : COLLECTIVE
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Setting this parameter enables mvapich to use blocking mode progress.
        MPI applications do not take up any CPU when they are waiting for
        incoming messages.

    - name        : MVP_USE_SHARED_MEM
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Use shared memory for intra-node communication.

    - name        : MVP_ENABLE_TOPO_AWARE_COLLECTIVES
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_TOPO_AWARE_ALLREDUCE
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This parameter determines whether a topology-aware algorithm
        should be used or not for allreduce collective operations. It
        takes effect only if MVP_ENABLE_TOPO_AWARE_COLLECTIVES is set
        to 1.

    - name        : MVP_USE_TOPO_AWARE_REDUCE
      category    : COLLECTIVE
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_TOPO_AWARE_BCAST
      category    : COLLECTIVE
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_OPTIMIZED_RELEASE_ALLREDUCE
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_SHMEM_TREE_ENABLE
      category    : COLLECTIVE
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC
        TODO: not used

    - name        : MVP_USE_SHMEM_TREE_MIN_MESSAGE_SIZE
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_SHMEM_TREE_MAX_MESSAGE_SIZE
      category    : COLLECTIVE
      type        : int
      default     : 16384
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_SHMEM_NUM_TREES
      category    : COLLECTIVE
      type        : int
      default     : 8
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_COLL_TMP_BUF_SIZE
      category    : COLLECTIVE
      type        : int
      default     : 2048
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_TOPO_AWARE_BARRIER
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This parameter determines whether a topology-aware algorithm
        should be used or not for barrier collective operations. It
        takes effect only if MVP_ENABLE_TOPO_AWARE_COLLECTIVES is set
        to 1.

    - name        : MVP_TOPO_AWARE_ALLREDUCE_MAX_MSG
      category    : COLLECTIVE
      type        : int
      default     : 2048
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_TOPO_AWARE_ALLREDUCE_MIN_MSG
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_TOPO_AWARE_REDUCE_MAX_MSG
      category    : COLLECTIVE
      type        : int
      default     : 2048
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_TOPO_AWARE_REDUCE_MIN_MSG
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_TOPO_AWARE_BCAST_MAX_MSG
      category    : COLLECTIVE
      type        : int
      default     : 2048
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_TOPO_AWARE_BCAST_MIN_MSG
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC


    - name        : MVP_TOPO_AWARE_REDUCE_PPN_THRESHOLD
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_TOPO_AWARE_BCAST_PPN_THRESHOLD
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_TOPO_AWARE_BCAST_NODE_THRESHOLD
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_TOPO_AWARE_REDUCE_NODE_THRESHOLD
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_GATHERV_SSEND_MIN_PROCS
      category    : COLLECTIVE
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_MCAST_ALLREDUCE
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_MCAST_ALLREDUCE_SMALL_MSG_SIZE
      category    : COLLECTIVE
      type        : int
      default     : 1024
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_MCAST_ALLREDUCE_LARGE_MSG_SIZE
      category    : COLLECTIVE
      type        : int
      default     : (128 * 1024)
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_MCAST_SCATTER
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_MCAST_SCATTER_MSG_SIZE
      category    : COLLECTIVE
      type        : int
      default     : 32
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_MCAST_SCATTER_SMALL_SYS_SIZE
      category    : COLLECTIVE
      type        : int
      default     : 32
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_MCAST_SCATTER_LARGE_SYS_SIZE
      category    : COLLECTIVE
      type        : int
      default     : 2048
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_BITONIC_COMM_SPLIT
      category    : COLLECTIVE
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_BITONIC_COMM_SPLIT_THRESHOLD
      category    : COLLECTIVE
      type        : int
      default     : 12000
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_ENABLE_SKIP_TUNING_TABLE_SEARCH
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_COLL_SKIP_TABLE_THRESHOLD
      category    : COLLECTIVE
      type        : int
      default     : 1024
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Message sizes larger than threshold will be pick an algorithm from
        tuning table.

    - name        : MVP_USE_INDEXED_ALLGATHER_TUNING
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_INDEXED_ALLTOALL_TUNING
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_INDEXED_ALLTOALLV_TUNING
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_IBCAST_ENABLE
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_IGATHER_ENABLE
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_ISCATTER_ENABLE
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_IALLGATHER_ENABLE
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_IREDUCE_ENABLE
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_IALLREDUCE_ENABLE
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_IREDUCE_SCATTER_ENABLE
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_IALLTOALL_ENABLE
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_IALLTOALLV_ENABLE
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_IALLGATHERV_ENABLE
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_IBARRIER_ENABLE
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_SHMEM_COLL_MAX_NUM_EMPTY_PROGRESS_POLLS
      category    : COLLECTIVE
      type        : int
      default     : 10000
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_KNOMIAL_REDUCE
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_REDUCE_INTER_KNOMIAL_FACTOR
      category    : COLLECTIVE
      type        : int
      default     : 4
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_REDUCE_ZCOPY_INTER_KNOMIAL_FACTOR
      category    : COLLECTIVE
      type        : int
      default     : 4
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_REDUCE_INTRA_KNOMIAL_FACTOR
      category    : COLLECTIVE
      type        : int
      default     : 4
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_INTRA_MULTI_LEVEL_GATHER_TUNING
      category    : COLLECTIVE
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_OLD_BCAST
      category    : COLLECTIVE
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_ALLRED_USE_RING
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_OLD_ALLGATHER
      category    : COLLECTIVE
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_OLD_SCATTER
      category    : COLLECTIVE
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_OLD_ALLREDUCE
      category    : COLLECTIVE
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_OLD_REDUCE
      category    : COLLECTIVE
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_OLD_ALLTOALL
      category    : COLLECTIVE
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_ALLTOALL_INPLACE_OLD
      category    : COLLECTIVE
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_ALLTOALL_RD_MAX_MSG_SIZE
      category    : COLLECTIVE
      type        : int
      default     : 512
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC - in bytes.

    - name        : MVP_INTRA_IBCAST_TUNING
      category    : COLLECTIVE
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC


    - name        : MVP_INTRA_IGATHER_TUNING
      category    : COLLECTIVE
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_INTER_IBCAST_TUNING
      category    : COLLECTIVE
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC


    - name        : MVP_INTER_IGATHER_TUNING
      category    : COLLECTIVE
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC



    - name        : MVP_INTRA_ISCATTER_TUNING
      category    : COLLECTIVE
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC


    - name        : MVP_INTER_ISCATTER_TUNING
      category    : COLLECTIVE
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC


    - name        : MVP_INTRA_IALLREDUCE_TUNING
      category    : COLLECTIVE
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_INTER_IALLREDUCE_TUNING
      category    : COLLECTIVE
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_INTRA_IREDUCE_TUNING
      category    : COLLECTIVE
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC


    - name        : MVP_INTER_IREDUCE_TUNING
      category    : COLLECTIVE
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC


    - name        : MVP_INTRA_IREDUCE_SCATTER_TUNING
      category    : COLLECTIVE
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC


    - name        : MVP_INTER_IREDUCE_SCATTER_TUNING
      category    : COLLECTIVE
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_INTER_IALLGATHER_TUNING
      category    : COLLECTIVE
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC


    - name        : MVP_INTRA_IALLGATHER_TUNING
      category    : COLLECTIVE
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC


    - name        : MVP_INTRA_IALLTOALL_TUNING
      category    : COLLECTIVE
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC


    - name        : MVP_INTER_IALLTOALL_TUNING
      category    : COLLECTIVE
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC


    - name        : MVP_INTRA_IALLTOALLV_TUNING
      category    : COLLECTIVE
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC


    - name        : MVP_INTER_IALLTOALLV_TUNING
      category    : COLLECTIVE
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC


    - name        : MVP_INTER_IALLGATHERV_TUNING
      category    : COLLECTIVE
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC


    - name        : MVP_INTRA_IALLGATHERV_TUNING
      category    : COLLECTIVE
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC


    - name        : MVP_INTRA_IBARRIER_TUNING
      category    : COLLECTIVE
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC


    - name        : MVP_INTER_IBARRIER_TUNING
      category    : COLLECTIVE
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC


    - name        : MVP_INTER_RED_SCAT_TUNING
      category    : COLLECTIVE
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC


    - name        : MVP_INTER_RED_SCAT_BLOCK_TUNING
      category    : COLLECTIVE
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC


    - name        : MVP_INTER_ALLGATHERV_TUNING
      category    : COLLECTIVE
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_SCATTER_DEST_ALLTOALLV
      category    : COLLECTIVE
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_PIPELINED_ZCPY_BCAST_KNOMIAL_FACTOR
      category    : COLLECTIVE
      type        : int
      default     : 2
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_ZCOPY_BCAST
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_ZCOPY_REDUCE
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_GATHERV_SSEND_THRESHOLD
      category    : COLLECTIVE
      type        : int
      default     : 32768
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_ALLREDUCE_RING_ALGO_PPN_THRESHOLD
      category    : COLLECTIVE
      type        : int
      default     : 8
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_ALLREDUCE_RING_ALGO_THRESHOLD
      category    : COLLECTIVE
      type        : int
      default     : (2 * 1024 * 1024)
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_ALLREDUCE_RED_SCAT_ALLGATHER_ALGO_PPN_THRESHOLD
      category    : COLLECTIVE
      type        : int
      default     : 8
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_ALLREDUCE_RED_SCAT_ALLGATHER_ALGO_THRESHOLD
      category    : COLLECTIVE
      type        : int
      default     : (2 * 1024 * 1024)
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_ALLGATHER_RING_ALGO_THRESHOLD
      category    : COLLECTIVE
      type        : int
      default     : 131072
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_ALLTOALLV_INTERMEDIATE_WAIT_THRESHOLD
      category    : COLLECTIVE
      type        : int
      default     : (256 * 1024)
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_ALLGATHER_CYCLIC_ALGO_THRESHOLD
      category    : COLLECTIVE
      type        : int
      default     : 1024
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_ALLREDUCE_CYCLIC_ALGO_THRESHOLD
      category    : COLLECTIVE
      type        : int
      default     : (1024 * 1024)
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_REDSCAT_CYCLIC_ALGO_THRESHOLD
      category    : COLLECTIVE
      type        : int
      default     : 1024
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_RED_SCAT_RING_ALGO_THRESHOLD
      category    : COLLECTIVE
      type        : int
      default     : 131072
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_BCAST_SCATTER_RING_OVERLAP
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_BCAST_SCATTER_RING_OVERLAP_MSG_UPPERBOUND
      category    : COLLECTIVE
      type        : int
      default     : 1048576
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_BCAST_SCATTER_RING_OVERLAP_CORES_LOWERBOUND
      category    : COLLECTIVE
      type        : int
      default     : 32
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_PIPELINE_BCAST
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_BCAST_SEGMENT_SIZE
      alias       : BCAST_SEGMENT_SIZE
      category    : COLLECTIVE
      type        : int
      default     : 8192
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_LIMIC_GATHER
      category    : COLLECTIVE
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        If this flag is set to 1, we will use intra-node Zero-Copy
        MPI_Gather designs, when the library has been configured to
        use LiMIC2.

    - name        : MVP_USE_2LEVEL_ALLGATHER
      category    : COLLECTIVE
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_SHMEM_COLL_WINDOW_SIZE
      category    : COLLECTIVE
      type        : int
      default     : 128
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_SHMEM_REDUCE_TREE_DEGREE
      category    : COLLECTIVE
      type        : int
      default     : 4
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_SHMEM_COLL_SLOT_LEN
      category    : COLLECTIVE
      type        : int
      default     : 8192
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_SLOT_SHMEM_COLL
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_SLOT_SHMEM_BCAST
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_MCAST_PIPELINE_SHM
      category    : COLLECTIVE
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_TWO_LEVEL_COMM_THRESHOLD
      category    : COLLECTIVE
      type        : int
      default     : 16
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_TWO_LEVEL_COMM_EARLY_INIT_THRESHOLD
      category    : COLLECTIVE
      type        : int
      default     : 2048
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Max process count for MPI_COMM_WORLD to create 2lvl comm in
        MPI_INIT

    - name        : MVP_ENABLE_PVAR_TIMER
      category    : COLLECTIVE
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_ENABLE_ALLREDUCE_ALL_COMPUTE
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_GATHER_STATUS_ALIGNMENT
      category    : COLLECTIVE
      type        : int
      default     : 16
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_BCAST_STATUS_ALIGNMENT
      category    : COLLECTIVE
      type        : int
      default     : 16
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_ENABLE_ALLREDUCE_SKIP_LARGE_MESSAGE_TUNING_TABLE_SEARCH
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_ENABLE_ALLREDUCE_SKIP_SMALL_MESSAGE_TUNING_TABLE_SEARCH
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_SOCKET_AWARE_ALLREDUCE_PPN_THRESHOLD
      category    : COLLECTIVE
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

=== END_MPI_T_MVP_CVAR_INFO_BLOCK ===
*/

#if defined(CHANNEL_MRAIL)
/*
 * TODO: just passing in NULL since this is CH3 specific and CH3 does
 * not use the state value of MPID_Progress_test
 */
#define MPID_MVP_PROGRESS_IF_NEEDED                                            \
    if (unlikely(mvp_posted_recvq_length || mvp_unexp_msg_recv ||              \
                 mvp_num_posted_send || rdma_global_ext_sendq_size)) {         \
        MPID_Progress_test(NULL);                                              \
    } else if (mvp_shmem_coll_num_empty_progress_polls >                       \
               MVP_SHMEM_COLL_MAX_NUM_EMPTY_PROGRESS_POLLS) {                  \
        MVP_INC_NUM_UNEXP_RECV();                                              \
        MPID_Progress_test(NULL);                                              \
        MVP_DEC_NUM_UNEXP_RECV();                                              \
        mvp_shmem_coll_num_empty_progress_polls = 0;                           \
    } else {                                                                   \
        mvp_shmem_coll_num_empty_progress_polls++;                             \
    }
#else
#define MPID_MVP_PROGRESS_IF_NEEDED MPID_Progress_test(NULL);
#endif /* #if defined(CHANNEL_MRAIL) */

int mvp_tuning_table[COLL_COUNT][COLL_SIZE] = {
    {2048, 1024, 512}, {-1, -1, -1}, {-1, -1, -1}};

/* array used to tune scatter*/
int mvp_size_mvp_scatter_mvp_tuning_table = 4;
struct scatter_tuning mvp_scatter_mvp_tuning_table[] = {
    {64, 4096, 8192}, {128, 8192, 16384}, {256, 4096, 8192}, {512, 4096, 8192}};

int mvp_enable_allreduce_all_compute = 1;

int mvp_pipelined_knomial_factor = 2;

int mvp_topo_aware_allreduce_ppn_threshold = 1;

/* Runtime threshold for gather */
int igather_segment_size = 8192;
const char *mvp_user_igather_intra = NULL;
const char *mvp_user_igather_inter = NULL;

/* runtime flag for alltoall tuning  */
const char *mvp_user_ialltoall_intra = NULL;
const char *mvp_user_ialltoall_inter = NULL;
int ialltoall_segment_size = 8192;

/* runtime flag for alltoallv tuning  */
const char *mvp_user_ialltoallv_intra = NULL;
const char *mvp_user_ialltoallv_inter = NULL;
int ialltoallv_segment_size = 8192;

/* Runtime threshold for bcast */
const char *mvp_user_ibcast_intra = NULL;
const char *mvp_user_ibcast_inter = NULL;

/* Runtime threshold for scatter */
const char *mvp_user_iscatter_intra = NULL;
const char *mvp_user_iscatter_inter = NULL;
int iscatter_segment_size = 8192;

/* Runtime threshold for allreduce */
const char *mvp_user_iallreduce_intra = NULL;
const char *mvp_user_iallreduce_inter = NULL;
int iallreduce_segment_size = 8192;

/* Runtime threshold for reduce */
const char *mvp_user_ireduce_intra = NULL;
const char *mvp_user_ireduce_inter = NULL;
int ireduce_segment_size = 8192;

/* Runtime threshold for reduce */
const char *mvp_user_ireduce_scatter_intra = NULL;
const char *mvp_user_ireduce_scatter_inter = NULL;
int ireduce_scatter_segment_size = 8192;

/* Runtime threshold for allgather */
const char *mvp_user_allgather_intra = NULL;
const char *mvp_user_allgather_inter = NULL;
const char *mvp_user_iallgather_intra = NULL;
const char *mvp_user_iallgather_inter = NULL;
int iallgather_segment_size = 8192;

/* Runtime threshold for allgatherv */
const char *mvp_user_iallgatherv_intra = NULL;
const char *mvp_user_iallgatherv_inter = NULL;
int iallgatherv_segment_size = 8192;

/* Runtime threshold for barrier */
const char *mvp_user_ibarrier_intra = NULL;
const char *mvp_user_ibarrier_inter = NULL;
int ibarrier_segment_size = 8192;
int mvp_gather_status_alignment = 16;
int mvp_bcast_status_alignment = 16;

/* Runtime threshold for red_scat */
const char *mvp_user_red_scat_inter = NULL;

/* Runtime threshold for red_scat_block */
const char *mvp_user_red_scat_block_inter = NULL;

int mvp_bcast_large_msg = MPIR_BCAST_LARGE_MSG;

int ibcast_segment_size = 8192;

#if defined(CKPT)
void *smc_store;
int smc_store_set;
#endif

#ifdef _ENABLE_CUDA_
static void *mvp_device_host_send_buf = NULL;
static void *mvp_device_host_recv_buf = NULL;
static void *mvp_device_dev_sr_buf = NULL;
static int mvp_device_host_sendbuf_size = 0;
static int mvp_device_host_recvbuf_size = 0;
static int mvp_device_dev_srbuf_size = 0;
static int *mvp_device_host_send_displs = NULL;
static int *mvp_device_host_recv_displs = NULL;
static int mvp_device_host_send_peers = 0;
static int mvp_device_host_recv_peers = 0;
static int *mvp_device_original_send_displs = NULL;
static int *mvp_device_original_recv_displs = NULL;
static void *mvp_device_original_send_buf = NULL;
static void *mvp_device_original_recv_buf = NULL;
void *mvp_device_allgather_store_buf = NULL;
int mvp_device_allgather_store_buf_size = 256 * 1024;
static void *mvp_device_coll_pack_buf = 0;
static int mvp_device_coll_pack_buf_size = 0;
static void *mvp_device_coll_unpack_buf = 0;
static int mvp_device_coll_unpack_buf_size = 0;
static void *mvp_device_orig_recvbuf = NULL;
static int mvp_device_orig_recvcount = 0;
static MPI_Datatype mvp_device_orig_recvtype;
#endif

/*
 * MPIT variables used by collectives
 */
extern unsigned long long PVAR_COUNTER_mvp_num_2level_comm_requests;
extern unsigned long long PVAR_COUNTER_mvp_num_2level_comm_success;
extern unsigned long long PVAR_COUNTER_mvp_num_shmem_coll_calls;

static inline void mvp_set_reduce_collective_algorithm();

static inline void mvp_set_gather_collective_algorithm();
static inline void mvp_set_bcast_collective_algorithm();

static inline void mvp_set_scatter_collective_algorithm();

int MVP_collectives_arch_init(int heterogeneity,
                              struct coll_info *colls_arch_hca)
{
    int mpi_errno = MPI_SUCCESS;

#if defined(_MVP_CH4_OVERRIDE_)
    /* tune the shmem size based on different architecture */
    if (MVP_IS_ARCH_HCA_TYPE(MVP_get_arch_hca_type(),
                             MVP_ARCH_INTEL_XEON_PHI_7250,
                             MVP_HCA_INTEL_HFI_100) &&
        !heterogeneity) {
        /* TACC KNL */
        if (MPIR_Process.local_size <= 64) {
            MVP_SHMEM_COLL_MAX_MSG_SIZE = 2 * 1024 * 1024;
            MVP_SHMEM_COLL_NUM_COMM = 7;
        }
    }
#endif

#if defined(_MCST_SUPPORT_)
    /* Disable mcast for any system other than Frontera by default */
    if (rdma_enable_mcast && (MVP_USE_MCAST == -1) &&
#if defined(RDMA_CM)
        (MVP_USE_RDMA_CM_MCAST == -1) &&
#endif /*defined(RDMA_CM)*/
        !(MVP_IS_ARCH_HCA_TYPE(MVP_get_arch_hca_type(),
                               MVP_ARCH_INTEL_PLATINUM_8280_2S_56,
                               MVP_HCA_MLX_CX_EDR))) {
        rdma_enable_mcast = 0;
#if defined(RDMA_CM)
        rdma_use_rdma_cm_mcast = 0;
#endif /*defined(RDMA_CM)*/
        MPL_free(mcast_ctx);
        PRINT_DEBUG(
            DEBUG_MCST_verbose,
            "Disable mcast by default on this"
            " architecture. Set MVP_USE_MCAST=1 to avoid this behavior\n");
    }
#endif

    MVP_Read_env_vars();
    if (MVP_USE_OSU_COLLECTIVES) {
        MVP_set_gather_tuning_table(heterogeneity, colls_arch_hca);
        MVP_set_bcast_tuning_table(heterogeneity, colls_arch_hca);
        MVP_set_alltoall_tuning_table(heterogeneity, colls_arch_hca);
        MVP_set_alltoallv_tuning_table(heterogeneity, colls_arch_hca);
        MVP_set_scatter_tuning_table(heterogeneity, colls_arch_hca);
        MVP_set_allreduce_tuning_table(heterogeneity, colls_arch_hca);
        MVP_set_allgather_tuning_table(heterogeneity, colls_arch_hca);
        MVP_set_red_scat_tuning_table(heterogeneity, colls_arch_hca);
        /* reduce_tuning needs to be init'ed after red_scat_tuning to prevent
         * reduce_scatter implementations from using reduce when the user sets
         * MVP_REDUCE_INTER_NODE_TUNING_ALGO to ALLREDUCE as this could lead to
         * potential infinite recursion.
         * reduce->allreduce->reduce_scatter->reduce->allreduce */
        MVP_set_reduce_tuning_table(heterogeneity, colls_arch_hca);
        MVP_set_allgatherv_tuning_table(heterogeneity, colls_arch_hca);
    }
#if defined(CHANNEL_MRAIL)
    if (MVP_USE_OSU_NB_COLLECTIVES) {
        MVP_set_igather_tuning_table(heterogeneity);
        MVP_set_ibcast_tuning_table(heterogeneity);
        MVP_set_ialltoall_tuning_table(heterogeneity);
        MVP_set_ialltoallv_tuning_table(heterogeneity);
        MVP_set_iscatter_tuning_table(heterogeneity);
        MVP_set_iallreduce_tuning_table(heterogeneity);
        MVP_set_ireduce_tuning_table(heterogeneity);
        MVP_set_ireduce_scatter_tuning_table(heterogeneity);
        MVP_set_iallgather_tuning_table(heterogeneity);
        MVP_set_iallgatherv_tuning_table(heterogeneity);
        MVP_set_ibarrier_tuning_table(heterogeneity);
    }
#endif

    /* Functions to set collective algorithm based on MPI_T CVAR */
    if (MVP_USE_OSU_COLLECTIVES) {
        mvp_set_gather_collective_algorithm();
        mvp_set_bcast_collective_algorithm();
        mvp_set_reduce_collective_algorithm();
        mvp_set_scatter_collective_algorithm();
    }

fn_exit:
    return mpi_errno;

fn_fail:
    goto fn_exit;
}

/* Change the values set inside the array by the one define by the user */
static inline int tuning_runtime_init()
{
    int i;

    /* If MVP_SCATTER_SMALL_MSG is define */
    if (MVP_CVAR_IS_SET_BY_USER(MVP_SCATTER_SMALL_MSG)) {
        for (i = 0; i < mvp_size_mvp_scatter_mvp_tuning_table; i++) {
            mvp_scatter_mvp_tuning_table[i].small = MVP_SCATTER_SMALL_MSG;
        }
    }

    /* If MVP_SCATTER_MEDIUM_MSG is define */
    if (MVP_CVAR_IS_SET_BY_USER(MVP_SCATTER_MEDIUM_MSG)) {
        for (i = 0; i < mvp_size_mvp_scatter_mvp_tuning_table; i++) {
            if (mvp_scatter_mvp_tuning_table[i].small <
                MVP_SCATTER_MEDIUM_MSG) {
                mvp_scatter_mvp_tuning_table[i].medium = MVP_SCATTER_MEDIUM_MSG;
            }
        }
    }

    /* If MVP_GATHER_SWITCH_POINT is define and if
     * inter gather algorithm is UNSET
     */
    if (MVP_CVAR_IS_SET_BY_USER(MVP_GATHER_SWITCH_PT) &&
        MVP_GATHER_INTER_NODE_TUNING_ALGO_UNSET ==
            MVP_GATHER_INTER_NODE_TUNING_ALGO) {
        for (i = 0; i < mvp_size_gather_tuning_table; i++) {
            mvp_gather_thresholds_table[0].inter_leader[1].min =
                MVP_GATHER_SWITCH_PT;
            mvp_gather_thresholds_table[0].inter_leader[0].max =
                MVP_GATHER_SWITCH_PT;
        }
    }
}

void MVP_collectives_arch_finalize()
{
    if (MVP_USE_OSU_COLLECTIVES) {
        MVP_cleanup_gather_tuning_table();
        MVP_cleanup_bcast_tuning_table();
        MVP_cleanup_alltoall_tuning_table();
        MVP_cleanup_alltoallv_tuning_table();
        MVP_cleanup_scatter_tuning_table();
        MVP_cleanup_allreduce_tuning_table();
        MVP_cleanup_reduce_tuning_table();
        MVP_cleanup_allgather_tuning_table();
        MVP_cleanup_red_scat_tuning_table();

        MVP_cleanup_allgatherv_tuning_table();
    }
}

static inline void mvp_set_bcast_collective_algorithm()
{
    switch (MVP_BCAST_COLLECTIVE_ALGORITHM) {
        case MVP_BCAST_COLLECTIVE_ALGORITHM_KNOMIAL:
            MVP_BCAST_INTER_NODE_TUNING_ALGO =
                MVP_BCAST_INTER_NODE_TUNING_ALGO_KNOMIAL;
            MVP_BCAST_INTRA_NODE_TUNING_ALGO =
                MVP_BCAST_INTRA_NODE_TUNING_ALGO_KNOMIAL;
            break;
        case MVP_BCAST_COLLECTIVE_ALGORITHM_UNSET:
        default:;
    }
}

static inline void mvp_set_scatter_collective_algorithm()
{
    switch (MVP_SCATTER_COLLECTIVE_ALGORITHM) {
        case MVP_SCATTER_COLLECTIVE_ALGORITHM_BINOMIAL:
            MVP_SCATTER_INTER_NODE_TUNING_ALGO =
                MVP_SCATTER_INTER_NODE_TUNING_ALGO_BINOMIAL;
            MVP_SCATTER_INTRA_NODE_TUNING_ALGO =
                MVP_SCATTER_INTRA_NODE_TUNING_ALGO_BINOMIAL;
            break;
        case MVP_SCATTER_COLLECTIVE_ALGORITHM_DIRECT:
            MVP_SCATTER_INTER_NODE_TUNING_ALGO =
                MVP_SCATTER_INTER_NODE_TUNING_ALGO_DIRECT;
            MVP_SCATTER_INTRA_NODE_TUNING_ALGO =
                MVP_SCATTER_INTRA_NODE_TUNING_ALGO_DIRECT;
        case MVP_SCATTER_COLLECTIVE_ALGORITHM_UNSET:
        default:;
    }
}

static inline void mvp_set_gather_collective_algorithm()
{
    switch (MVP_GATHER_COLLECTIVE_ALGORITHM) {
        case MVP_GATHER_COLLECTIVE_ALGORITHM_BINOMIAL:
            MVP_GATHER_INTER_NODE_TUNING_ALGO =
                MVP_GATHER_INTER_NODE_TUNING_ALGO_BINOMIAL;
            MVP_GATHER_INTRA_NODE_TUNING_ALGO =
                MVP_GATHER_INTRA_NODE_TUNING_ALGO_BINOMIAL;
            break;
        case MVP_GATHER_COLLECTIVE_ALGORITHM_DIRECT:
            MVP_GATHER_INTER_NODE_TUNING_ALGO =
                MVP_GATHER_INTER_NODE_TUNING_ALGO_DIRECT;
            MVP_GATHER_INTRA_NODE_TUNING_ALGO =
                MVP_GATHER_INTRA_NODE_TUNING_ALGO_DIRECT;
            break;
        case MVP_GATHER_COLLECTIVE_ALGORITHM_UNSET:
        default:;
    }
}

static inline void mvp_set_reduce_collective_algorithm()
{
    switch (MVP_REDUCE_COLLECTIVE_ALGORITHM) {
        case MVP_REDUCE_COLLECTIVE_ALGORITHM_BINOMIAL:
            MVP_REDUCE_INTER_NODE_TUNING_ALGO =
                MVP_REDUCE_INTER_NODE_TUNING_ALGO_BINOMIAL;
            MVP_REDUCE_INTRA_NODE_TUNING_ALGO =
                MVP_REDUCE_INTRA_NODE_TUNING_ALGO_BINOMIAL;
            break;
        case MVP_REDUCE_COLLECTIVE_ALGORITHM_KNOMIAL:
            MVP_REDUCE_INTER_NODE_TUNING_ALGO =
                MVP_REDUCE_INTER_NODE_TUNING_ALGO_KNOMIAL;
            MVP_REDUCE_INTRA_NODE_TUNING_ALGO =
                MVP_REDUCE_INTRA_NODE_TUNING_ALGO_KNOMIAL;
            break;
        case MVP_REDUCE_COLLECTIVE_ALGORITHM_UNSET:
        default:;
    }
}


#ifdef CKPT
#if defined(CHANNEL_MRAIL_GEN2) || defined(CHANNEL_NEMESIS_IB)
shmem_info_t *ckpt_free_head = NULL;
#endif /* defined(CHANNEL_MRAIL_GEN2) || defined(CHANNEL_NEMESIS_IB) */
#endif /* CKPT */

volatile int *child_complete_bcast;  /* use for initial synchro */
volatile int *child_complete_gather; /* use for initial synchro */
volatile int *root_complete_gather;
volatile int *barrier_gather;
volatile int *barrier_bcast;
volatile int *shmem_coll_block_status;

shmem_coll_region *shmem_coll = NULL;

/* Use a runtime modifiable parameter to determine the minimum IOV density */

typedef unsigned long addrint_t;

/* Shared memory collectives mgmt*/
struct shmem_coll_mgmt {
    void *mmap_ptr;
    int fd;
};
struct shmem_coll_mgmt mvp_shmem_coll_obj = {NULL, -1};

int mvp_init_call_once = 0;
int mvp_mmap_coll_once = 0;
int mvp_unlink_call_once = 0;
int finalize_coll_comm = 0;
int mvp_shmem_coll_num_empty_progress_polls = 0;

int mvp_shmem_coll_num_procs = 64;

size_t mvp_shmem_coll_size = 0;
char *mvp_shmem_coll_file = NULL;

static char mvp_hostname[SHMEM_COLL_HOSTNAME_LEN];
static int mvp_my_rank;

void MVP_Read_env_vars(void)
{
    if (MVP_USE_BLOCKING) {
        MVP_USE_SHARED_MEM = 0;
    }

    if (MVP_KNOMIAL_INTRA_NODE_THRESHOLD > MVP_SHMEM_COLL_MAX_MSG_SIZE) {
        MVP_KNOMIAL_INTRA_NODE_THRESHOLD = MVP_SHMEM_COLL_MAX_MSG_SIZE;
    }

    if (MVP_INTRA_IBCAST_TUNING != NULL) {
        mvp_user_ibcast_intra = MVP_INTRA_IBCAST_TUNING;
    }
    if (MVP_INTRA_IGATHER_TUNING != NULL) {
        mvp_user_igather_intra = MVP_INTRA_IGATHER_TUNING;
    }
    if (MVP_INTER_IBCAST_TUNING != NULL) {
        mvp_user_ibcast_inter = MVP_INTER_IBCAST_TUNING;
    }
    if (MVP_INTER_IGATHER_TUNING != NULL) {
        mvp_user_igather_inter = MVP_INTER_IGATHER_TUNING;
    }
    if (MVP_INTRA_ISCATTER_TUNING != NULL) {
        mvp_user_iscatter_intra = MVP_INTRA_ISCATTER_TUNING;
    }
    if (MVP_INTER_ISCATTER_TUNING != NULL) {
        mvp_user_iscatter_inter = MVP_INTER_ISCATTER_TUNING;
    }

    if (MVP_INTRA_IALLREDUCE_TUNING != NULL) {
        mvp_user_iallreduce_intra = MVP_INTRA_IALLREDUCE_TUNING;
    }
    if (MVP_INTER_IALLREDUCE_TUNING != NULL) {
        mvp_user_iallreduce_inter = MVP_INTER_IALLREDUCE_TUNING;
    }

    if (MVP_INTRA_IREDUCE_TUNING != NULL) {
        mvp_user_ireduce_intra = MVP_INTRA_IREDUCE_TUNING;
    }

    if (MVP_INTER_IREDUCE_TUNING != NULL) {
        mvp_user_ireduce_inter = MVP_INTER_IREDUCE_TUNING;
    }

    if (MVP_INTRA_IREDUCE_SCATTER_TUNING != NULL) {
        mvp_user_ireduce_scatter_intra = MVP_INTRA_IREDUCE_SCATTER_TUNING;
    }

    if (MVP_INTER_IREDUCE_SCATTER_TUNING != NULL) {
        mvp_user_ireduce_scatter_inter = MVP_INTER_IREDUCE_SCATTER_TUNING;
    }

    if (MVP_INTER_IALLGATHER_TUNING != NULL) {
        mvp_user_iallgather_inter = MVP_INTER_IALLGATHER_TUNING;
    }
    if (MVP_INTRA_IALLGATHER_TUNING != NULL) {
        mvp_user_iallgather_intra = MVP_INTRA_IALLGATHER_TUNING;
    }

    if (MVP_INTRA_IALLTOALL_TUNING != NULL) {
        mvp_user_ialltoall_intra = MVP_INTRA_IALLTOALL_TUNING;
    }
    if (MVP_INTER_IALLTOALL_TUNING != NULL) {
        mvp_user_ialltoall_inter = MVP_INTER_IALLTOALL_TUNING;
    }

    if (MVP_INTRA_IALLTOALLV_TUNING != NULL) {
        mvp_user_ialltoallv_intra = MVP_INTRA_IALLTOALLV_TUNING;
    }
    if (MVP_INTER_IALLTOALLV_TUNING != NULL) {
        mvp_user_ialltoallv_inter = MVP_INTER_IALLTOALLV_TUNING;
    }

    if (MVP_INTER_IALLGATHERV_TUNING != NULL) {
        mvp_user_iallgatherv_inter = MVP_INTER_IALLGATHERV_TUNING;
    }
    if (MVP_INTRA_IALLGATHERV_TUNING != NULL) {
        mvp_user_iallgatherv_intra = MVP_INTRA_IALLGATHERV_TUNING;
    }

    if (MVP_INTRA_IBARRIER_TUNING != NULL) {
        mvp_user_ibarrier_intra = MVP_INTRA_IBARRIER_TUNING;
    }
    if (MVP_INTER_IBARRIER_TUNING != NULL) {
        mvp_user_ibarrier_inter = MVP_INTER_IBARRIER_TUNING;
    }

    if (MVP_INTER_RED_SCAT_TUNING != NULL) {
        mvp_user_red_scat_inter = MVP_INTER_RED_SCAT_TUNING;
    }

    if (MVP_INTER_RED_SCAT_BLOCK_TUNING != NULL) {
        mvp_user_red_scat_block_inter = MVP_INTER_RED_SCAT_BLOCK_TUNING;
    }

    if (MVP_KNOMIAL_INTRA_NODE_FACTOR < MVP_INTRA_NODE_KNOMIAL_FACTOR_MIN) {
        MVP_KNOMIAL_INTRA_NODE_FACTOR = MVP_INTRA_NODE_KNOMIAL_FACTOR_MIN;
    }
    if (MVP_KNOMIAL_INTRA_NODE_FACTOR > MVP_INTRA_NODE_KNOMIAL_FACTOR_MAX) {
        MVP_KNOMIAL_INTRA_NODE_FACTOR = MVP_INTRA_NODE_KNOMIAL_FACTOR_MAX;
    }

    if (MVP_KNOMIAL_INTER_NODE_FACTOR < MVP_INTER_NODE_KNOMIAL_FACTOR_MIN) {
        MVP_KNOMIAL_INTER_NODE_FACTOR = MVP_INTER_NODE_KNOMIAL_FACTOR_MIN;
    }
    if (MVP_KNOMIAL_INTER_NODE_FACTOR > MVP_INTER_NODE_KNOMIAL_FACTOR_MAX) {
        MVP_KNOMIAL_INTER_NODE_FACTOR = MVP_INTER_NODE_KNOMIAL_FACTOR_MAX;
    }

#ifdef _ENABLE_UD_
    /* Disable ZCOPY_BCAST and ZCOPY_REDUCE if UD_HYBRID is disabled */
    if (!MVP_USE_UD_HYBRID) {
        MVP_USE_ZCOPY_BCAST = 0;
        MVP_USE_ZCOPY_REDUCE = 0;
    }
#endif

    if (MVP_ALLREDUCE_RING_ALGO_THRESHOLD < 0) {
        MVP_ALLREDUCE_RING_ALGO_THRESHOLD = 0;
    }

    if (MVP_ALLREDUCE_RED_SCAT_ALLGATHER_ALGO_THRESHOLD < 0) {
        MVP_ALLREDUCE_RED_SCAT_ALLGATHER_ALGO_THRESHOLD = 0;
    }

    if (MVP_ALLGATHER_RING_ALGO_THRESHOLD < 0) {
        MVP_ALLGATHER_RING_ALGO_THRESHOLD = 0;
    }

    if (MVP_ALLTOALLV_INTERMEDIATE_WAIT_THRESHOLD < 0) {
        MVP_ALLTOALLV_INTERMEDIATE_WAIT_THRESHOLD = 1024 * 1024;
    }

    if (MVP_ALLGATHER_CYCLIC_ALGO_THRESHOLD < 0) {
        MVP_ALLGATHER_CYCLIC_ALGO_THRESHOLD = 0;
    }

    if (MVP_ALLREDUCE_CYCLIC_ALGO_THRESHOLD < 0) {
        MVP_ALLREDUCE_CYCLIC_ALGO_THRESHOLD = 0;
    }

    if (MVP_REDSCAT_CYCLIC_ALGO_THRESHOLD < 0) {
        MVP_REDSCAT_CYCLIC_ALGO_THRESHOLD = 0;
    }

    if (MVP_RED_SCAT_RING_ALGO_THRESHOLD < 0) {
        MVP_RED_SCAT_RING_ALGO_THRESHOLD = 0;
    }

    if (!MVP_USE_SLOT_SHMEM_COLL || !MVP_USE_SLOT_SHMEM_BCAST) {
        /* Disable zero-copy bcast if slot-shmem, or slot-shmem-bcast params
         * are off  */
        MVP_USE_ZCOPY_BCAST = 0;
    }

    if (MVP_ENABLE_SOCKET_AWARE_COLLECTIVES) {
        MVP_ENABLE_TOPO_AWARE_COLLECTIVES = 0;
    } else if (MVP_ENABLE_TOPO_AWARE_COLLECTIVES) {
        MVP_ENABLE_SOCKET_AWARE_COLLECTIVES = 0;
    }

    /* Override MPICH2 default env values for Gatherv */
    MPIR_CVAR_GATHERV_INTER_SSEND_MIN_PROCS = 1024;
    if (MVP_GATHERV_SSEND_MIN_PROCS != -1) {
        if (MVP_GATHERV_SSEND_MIN_PROCS > 0) {
            MPIR_CVAR_GATHERV_INTER_SSEND_MIN_PROCS =
                MVP_GATHERV_SSEND_MIN_PROCS;
        }
    }
    init_thread_reg();
}

void MPIR_MVP_SHMEM_COLL_Cleanup()
{
    /*unmap */
    if (mvp_shmem_coll_obj.mmap_ptr != NULL) {
        munmap(mvp_shmem_coll_obj.mmap_ptr, mvp_shmem_coll_size);
    }
    /*unlink and close */
    if (mvp_shmem_coll_obj.fd != -1) {
        close(mvp_shmem_coll_obj.fd);
        unlink(mvp_shmem_coll_file);
    }
    /*free filename variable */
    if (mvp_shmem_coll_file != NULL) {
        MPL_free(mvp_shmem_coll_file);
    }
    mvp_shmem_coll_obj.mmap_ptr = NULL;
    mvp_shmem_coll_obj.fd = -1;
    mvp_shmem_coll_file = NULL;
}

void MPIR_MVP_SHMEM_COLL_Unlink()
{
    if (mvp_unlink_call_once == 1)
        return;

    mvp_unlink_call_once = 1;
    if (mvp_init_call_once == 1 && mvp_mmap_coll_once == 1)
        finalize_coll_comm = 1;

    if (mvp_shmem_coll_obj.fd != -1) {
        unlink(mvp_shmem_coll_file);
        mvp_shmem_coll_obj.mmap_ptr = NULL;
        mvp_shmem_coll_obj.fd = -1;
    }

    if (mvp_shmem_coll_file != NULL) {
        MPL_free(mvp_shmem_coll_file);
        mvp_shmem_coll_file = NULL;
    }
}

int MPIR_MVP_SHMEM_Helper_fn(int local_id, char **filename, char *prefix,
                             int *fd, size_t file_size)
{
    int mpi_errno = MPI_SUCCESS;
    size_t pathlen = 0;
    const char *shmem_dir = NULL, *shmdir = NULL;
    struct stat file_status;
#if defined(SOLARIS)
    char *setdir = "/tmp";
#else
    char *setdir = "/dev/shm";
#endif

    MPIR_FUNC_TERSE_STATE_DECL(MPID_STATE_MPIR_MVP_SHMEM_HELPER_FN);
    MPIR_FUNC_TERSE_ENTER(MPID_STATE_MPIR_MVP_SHMEM_HELPER_FN);

    PRINT_DEBUG(DEBUG_SHM_verbose > 0, "Set up shmem for %s\n", prefix);

    /* Obtain prefix to create shared file name */
    if (MVP_SHMEM_DIR != NULL) {
        shmem_dir = MVP_SHMEM_DIR;
    } else {
        shmem_dir = setdir;
    }
    /*Find length of prefix */
    pathlen = strlen(shmem_dir);

    /* Get hostname to create unique shared file name */
    if (gethostname(mvp_hostname, sizeof(char) * HOSTNAME_LEN) < 0) {
        MPIR_ERR_SETFATALANDJUMP2(mpi_errno, MPI_ERR_OTHER, "**fail", "%s: %s",
                                  "gethostname", strerror(errno));
    }

    /* TODO -- Remove UPMI calls here and replace with something that's
     * generic and will not be deprecated. Added here for now to avoid
     * weird issues in case there's a dependency.*/
    int kvs_name_sz;
    UPMI_KVS_GET_NAME_LENGTH_MAX(&kvs_name_sz);
    char *mvp_kvs_name = (char *)MPL_malloc(kvs_name_sz + 1, MPL_MEM_STRINGS);
    UPMI_KVS_GET_MY_NAME(mvp_kvs_name, kvs_name_sz);

    /* Allocate memory for unique file name */
    *filename = (char *)MPL_malloc(
        sizeof(char) * (pathlen + HOSTNAME_LEN + 26 + PID_CHAR_LEN),
        MPL_MEM_COLL);
    if (!*filename) {
        MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**nomem",
                                  "**nomem %s", "mvp_shmem_coll_file");
    }

    /* Create unique shared file name */
    sprintf(*filename, "%s/%s-%s-%s-%d.tmp", shmem_dir, prefix, mvp_kvs_name,
            mvp_hostname, getuid());

    /* Open the shared memory file */
    *fd = open(*filename, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    if (*fd < 0) {
        /* Fallback */
        sprintf(*filename, "/tmp/%s-%s-%s-%d.tmp", prefix, mvp_kvs_name,
                mvp_hostname, getuid());

        *fd = open(*filename, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
        if (*fd < 0) {
            MPIR_ERR_SETFATALANDJUMP2(mpi_errno, MPI_ERR_OTHER, "**fail",
                                      "%s: %s", "open", strerror(errno));
        }
    }
    PRINT_DEBUG(DEBUG_SHM_verbose > 0, "Shmem file opened for %s\n", prefix);

    if (local_id == 0) {
        PRINT_DEBUG(DEBUG_SHM_verbose > 0, "Set file size for %s\n", prefix);
        if (ftruncate(*fd, 0)) {
            /* to clean up tmp shared file */
            mpi_errno = MPIR_Err_create_code(
                MPI_SUCCESS, MPI_ERR_OTHER, __func__, __LINE__, MPI_ERR_OTHER,
                "**fail", "%s: %s", "ftruncate", strerror(errno));
            goto fn_fail;
        }

        /* set file size, without touching pages */
        if (ftruncate(*fd, file_size)) {
            /* to clean up tmp shared file */
            mpi_errno = MPIR_Err_create_code(
                MPI_SUCCESS, MPI_ERR_OTHER, __func__, __LINE__, MPI_ERR_OTHER,
                "**fail", "%s: %s", "ftruncate", strerror(errno));
            goto fn_fail;
        }

        PRINT_DEBUG(DEBUG_SHM_verbose > 0, "Call MPL_calloc for %s\n", prefix);
/* Ignoring optimal memory allocation for now */
#if !defined(__x86_64__)

#define FSIZE_LIMIT 2147483640 /* 2G - c */
        {
            char *buf =
                (char *)MPL_calloc(file_size + 1, sizeof(char), MPL_MEM_BUFFER);
            size_t remaining_bytes, transfer_size;
            size_t transferred = 0, offset = 0;

            /* if filesize exceeds 2G limit, then split it up in multiple chunks
             * and then do the write iteratively.
             */
            remaining_bytes = file_size;
            while (remaining_bytes > 0) {
                transfer_size = (remaining_bytes > FSIZE_LIMIT) ?
                                    FSIZE_LIMIT :
                                    remaining_bytes;
                write(*fd, buf + offset, transfer_size);
                remaining_bytes -= transfer_size;
                transferred += transfer_size;

                offset += transfer_size;
            }

            /* if total bytes transferred doesn't match filesize, throw error */
            if (transferred != file_size) {
                mpi_errno =
                    MPIR_Err_create_code(MPI_SUCCESS, MPI_ERR_OTHER, __func__,
                                         __LINE__, MPI_ERR_OTHER, "**fail",
                                         "%s: %s", "write", strerror(errno));
                MPL_free(buf);
                goto fn_fail;
            }
            MPL_free(buf);
        }
#endif /* !defined(__x86_64__) */

        PRINT_DEBUG(DEBUG_SHM_verbose > 0, "Call lseek for %s\n", prefix);
        if (lseek(*fd, 0, SEEK_SET) != 0) {
            /* to clean up tmp shared file */
            mpi_errno = MPIR_Err_create_code(
                MPI_SUCCESS, MPI_ERR_OTHER, __func__, __LINE__, MPI_ERR_OTHER,
                "**fail", "%s: %s", "lseek", strerror(errno));
            goto fn_fail;
        }
    }

    PRINT_DEBUG(DEBUG_SHM_verbose > 0, "Wait for local sync for %s\n", prefix);
    /* Synchronization between local processes */
    do {
        if (fstat(*fd, &file_status) != 0) {
            /* to clean up tmp shared file */
            mpi_errno = MPIR_Err_create_code(
                MPI_SUCCESS, MPI_ERR_OTHER, __func__, __LINE__, MPI_ERR_OTHER,
                "**fail", "%s: %s", "fstat", strerror(errno));
            goto fn_fail;
        }
        usleep(1);
    } while (file_status.st_size != file_size);

    PRINT_DEBUG(DEBUG_SHM_verbose > 0, "Finished set up shmem for %s\n",
                prefix);

fn_exit:
    MPL_free(mvp_kvs_name);
    MPIR_FUNC_TERSE_EXIT(MPID_STATE_MPIR_MVP_SHMEM_HELPER_FN);
    return mpi_errno;

fn_fail:
    goto fn_exit;
}

int MPIR_MVP_SHMEM_COLL_init(int local_id)
{
    MPIR_FUNC_TERSE_STATE_DECL(MPID_STATE_MPIR_MVP_SHMEM_COLL_INIT);
    MPIR_FUNC_TERSE_ENTER(MPID_STATE_MPIR_MVP_SHMEM_COLL_INIT);
    int mpi_errno = MPI_SUCCESS;

    if (mvp_init_call_once == 1)
        return mpi_errno;

    mvp_init_call_once = 1;

    mvp_shmem_coll_num_procs = MPIR_Process.local_size;
    if (MVP_SHMEM_COLL_NUM_PROCS != -1) {
        mvp_shmem_coll_num_procs = MVP_SHMEM_COLL_NUM_PROCS;
    }
    mvp_enable_allreduce_all_compute = MVP_ENABLE_ALLREDUCE_ALL_COMPUTE;
    mvp_gather_status_alignment = MVP_GATHER_STATUS_ALIGNMENT;
    mvp_bcast_status_alignment = MVP_BCAST_STATUS_ALIGNMENT;

    /* Find out the size of the region to create */
    mvp_shmem_coll_size = SHMEM_ALIGN(SHMEM_COLL_BUF_SIZE + getpagesize()) +
                          SHMEM_CACHE_LINE_SIZE;

    /* Call helper function to create shmem region */
    mpi_errno = MPIR_MVP_SHMEM_Helper_fn(
        local_id, &mvp_shmem_coll_file, "ib_shmem_coll", &mvp_shmem_coll_obj.fd,
        mvp_shmem_coll_size);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }

    tuning_runtime_init();

fn_exit:
    MPIR_FUNC_TERSE_EXIT(MPID_STATE_MPIR_MVP_SHMEM_COLL_INIT);
    return mpi_errno;

fn_fail:
    MPIR_MVP_SHMEM_COLL_Cleanup();
    goto fn_exit;
}

int MPIR_MVP_SHMEM_COLL_Mmap(int local_id)
{
    int i = 0;
    int j = 0;
    char *buf = NULL;
    int mpi_errno = MPI_SUCCESS;
    int num_cntrl_bufs = 5;
    MPIR_FUNC_TERSE_STATE_DECL(MPID_STATE_MPIR_MVP_SHMEM_COLLMMAP);
    MPIR_FUNC_TERSE_ENTER(MPID_STATE_MPIR_MVP_SHMEM_COLLMMAP);

    if (mvp_mmap_coll_once)
        return mpi_errno;

    mvp_mmap_coll_once = 1;

    mvp_shmem_coll_obj.mmap_ptr =
        mmap(0, mvp_shmem_coll_size, (PROT_READ | PROT_WRITE), (MAP_SHARED),
             mvp_shmem_coll_obj.fd, 0);
    if (mvp_shmem_coll_obj.mmap_ptr == (void *)-1) {
        /* to clean up tmp shared file */
        mpi_errno = MPIR_Err_create_code(MPI_SUCCESS, MPI_ERR_OTHER, __func__,
                                         __LINE__, MPI_ERR_OTHER, "**fail",
                                         "%s: %s", "mmap", strerror(errno));
        goto cleanup_files;
    }
#if defined(CKPT)
    if (smc_store_set) {
        MPIR_Memcpy(mvp_shmem_coll_obj.mmap_ptr, smc_store,
                    mvp_shmem_coll_size);
        MPL_free(smc_store);
        smc_store_set = 0;
    }
#endif

    shmem_coll = (shmem_coll_region *)mvp_shmem_coll_obj.mmap_ptr;

    /* layout the shared memory for variable length vars */
    buf = &shmem_coll->shmem_coll_buf +
          (MVP_SHMEM_COLL_NUM_COMM * 2 * SHMEM_COLL_BLOCK_SIZE);

    if (local_id == 0) {
        MPIR_Memset(buf, 0,
                    (num_cntrl_bufs - 1) * SHMEM_COLL_SYNC_ARRAY_SIZE +
                        SHMEM_BCAST_SYNC_ARRAY_SIZE);
    }

    shmem_coll_block_status = (volatile int *)buf;
    buf += SHMEM_COLL_STATUS_ARRAY_SIZE;
    child_complete_bcast = (volatile int *)buf;
    buf += SHMEM_BCAST_SYNC_ARRAY_SIZE;
    child_complete_gather = (volatile int *)buf;
    buf += SHMEM_COLL_SYNC_ARRAY_SIZE;
    root_complete_gather = (volatile int *)buf;
    buf += SHMEM_COLL_SYNC_ARRAY_SIZE;
    barrier_gather = (volatile int *)buf;
    buf += SHMEM_COLL_SYNC_ARRAY_SIZE;
    barrier_bcast = (volatile int *)buf;

    if (local_id == 0) {
        for (j = 0; j < MVP_SHMEM_COLL_NUM_COMM; ++j) {
            for (i = 0; i < mvp_shmem_coll_num_procs; ++i) {
                SHMEM_BCAST_SYNC_CLR(child_complete_bcast, j, i);
                WRITEBAR();
            }

            for (i = 0; i < mvp_shmem_coll_num_procs; ++i) {
                SHMEM_COLL_SYNC_SET(root_complete_gather, j, i);
                WRITEBAR();
            }
        }
        for (j = 0; j < MVP_SHMEM_COLL_NUM_COMM; j++) {
            SHMEM_COLL_BLOCK_STATUS_CLR(shmem_coll_block_status, j);
            WRITEBAR();
        }
        pthread_spin_init(&shmem_coll->shmem_coll_lock, PTHREAD_PROCESS_SHARED);
        shmem_coll->mvp_shmem_comm_count = 0;
#if defined(CKPT)
        /*
         * FIXME: The second argument to pthread_spin_init() indicates whether
         * the Lock can be accessed by a process other than the one that
         * initialized it. So, it should actually be PTHREAD_PROCESS_SHARED.
         * However, the "shmem_coll_lock" above sets this to 0. Hence, I am
         * doing the same.
         */
        pthread_spin_init(&shmem_coll->cr_smc_spinlock, PTHREAD_PROCESS_SHARED);
        shmem_coll->cr_smc_cnt = 0;
#endif
    }

fn_exit:
    MPIR_FUNC_TERSE_EXIT(MPID_STATE_MPIR_MVP_SHMEM_COLLMMAP);
    return mpi_errno;

cleanup_files:
    MPIR_MVP_SHMEM_COLL_Cleanup();
    goto fn_exit;
}

int MPIR_MVP_SHMEM_COLL_finalize(int local_id, int num_local_nodes)
{
    MPIR_FUNC_TERSE_STATE_DECL(MPID_STATE_MPIR_MVP_SHMEM_COLL_FINALIZE);
    MPIR_FUNC_TERSE_ENTER(MPID_STATE_MPIR_MVP_SHMEM_COLL_FINALIZE);

#if defined(CKPT)

    extern int g_cr_in_progress;

    if (g_cr_in_progress) {
        /* Wait for other local processes to check-in */
        pthread_spin_lock(&shmem_coll->cr_smc_spinlock);
        ++(shmem_coll->cr_smc_cnt);
        pthread_spin_unlock(&shmem_coll->cr_smc_spinlock);
        while (shmem_coll->cr_smc_cnt < num_local_nodes)
            ;

        if ((mvp_shmem_coll_obj.mmap_ptr != NULL) && (local_id == 0)) {
            smc_store = MPL_malloc(mvp_shmem_coll_size, MPL_MEM_COLL);
            PRINT_DEBUG(DEBUG_CR_verbose > 1,
                        "mvp_shmem_coll_obj.mmap_ptr = %p\n",
                        mvp_shmem_coll_obj.mmap_ptr);
            MPIR_Memcpy(smc_store, mvp_shmem_coll_obj.mmap_ptr,
                        mvp_shmem_coll_size);
            smc_store_set = 1;
        }
    }
#endif

    MPIR_MVP_SHMEM_COLL_Cleanup();

    MPIR_FUNC_TERSE_EXIT(MPID_STATE_MPIR_MVP_SHMEM_COLL_FINALIZE);
    return MPI_SUCCESS;
}

int MPIR_MVP_SHMEM_Sync(volatile int *volatile bar_array, int my_local_id,
                        int num_local_procs)
{
    int mpi_errno = MPI_SUCCESS;
    volatile int i = 0, wait = 0, pid = 0;

    /* Sanity check */
    while (bar_array == NULL)
        ;

    if (0 == my_local_id) {
        wait = 1;
        while (wait) {
            wait = 0;
            for (i = 1; i < num_local_procs; ++i) {
                if (bar_array[i] == 0) {
                    wait = 1;
                }
            }
        }

        pid = getpid();
        if (0 == pid) {
            mpi_errno = MPIR_Err_create_code(
                MPI_SUCCESS, MPI_ERR_OTHER, __func__, __LINE__, MPI_ERR_OTHER,
                "**fail", "%s: %s", "getpid", strerror(errno));
            return mpi_errno;
        }

        bar_array[my_local_id] = pid;
        WRITEBAR();
    } else {
        while (bar_array[0] != 0)
            ;
        while (bar_array[0] == 0) {
            bar_array[my_local_id] = getpid();
            WRITEBAR();
        }
        for (i = 0; i < num_local_procs; ++i) {
            if (bar_array[i] <= 0) {
                mpi_errno =
                    MPIR_Err_create_code(MPI_SUCCESS, MPI_ERR_OTHER, __func__,
                                         __LINE__, MPI_ERR_OTHER, "**fail",
                                         "%s: %s", "getpid", strerror(errno));
                return mpi_errno;
            }
        }
    }
    return mpi_errno;
}

int MPIR_MVP_SMP_COLL_init()
{
    int mpi_errno = MPI_SUCCESS;
    int local_num_processes = MPIR_Process.local_size;
    int local_id = MPIR_Process.local_rank;
    void *mmap_ptr = NULL;
    int *bar_array = NULL;
    int fd = -1;
    char *shmem_coll_init_file = NULL;
    size_t bar_array_size = sizeof(int) * local_num_processes;
    mpi_errno = MPIR_MVP_SHMEM_Helper_fn(local_id, &shmem_coll_init_file,
                                         "coll_bar_shmem", &fd, bar_array_size);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
    mmap_ptr =
        mmap(0, bar_array_size, (PROT_READ | PROT_WRITE), (MAP_SHARED), fd, 0);
    if (mmap_ptr == (void *)-1) {
        /* to clean up tmp shared file */
        mpi_errno = MPIR_Err_create_code(MPI_SUCCESS, MPI_ERR_OTHER, __func__,
                                         __LINE__, MPI_ERR_OTHER, "**nomem",
                                         "%s: %s", "mmap", strerror(errno));
        MPIR_ERR_POP(mpi_errno);
    }
    bar_array = (int *)mmap_ptr;

    /* Shared memory for collectives */
    if ((mpi_errno = MPIR_MVP_SHMEM_COLL_init(local_id)) != MPI_SUCCESS) {
        mpi_errno = MPIR_Err_create_code(MPI_SUCCESS, MPI_ERR_OTHER, __func__,
                                         __LINE__, MPI_ERR_OTHER, "**fail",
                                         "%s", "SHMEM_COLL_init failed");
        MPIR_ERR_POP(mpi_errno);
    }
    /* Memory Mapping shared files for collectives*/
    if ((mpi_errno = MPIR_MVP_SHMEM_COLL_Mmap(local_id)) != MPI_SUCCESS) {
        mpi_errno = MPIR_Err_create_code(MPI_SUCCESS, MPI_ERR_OTHER, __func__,
                                         __LINE__, MPI_ERR_OTHER, "**fail",
                                         "%s", "SHMEM_COLL_Mmap failed");
    }
    mpi_errno = MPIR_MVP_SHMEM_Sync(bar_array, local_id, local_num_processes);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
    MPIR_MVP_SHMEM_COLL_Unlink();
fn_exit:
    if (mmap_ptr != NULL) {
        munmap((void *)mmap_ptr, bar_array_size);
    }
    if (fd != -1) {
        close(fd);
        unlink(shmem_coll_init_file);
    }
    if (shmem_coll_init_file != NULL) {
        MPL_free(shmem_coll_init_file);
    }
    shmem_coll_init_file = NULL;
    return mpi_errno;
fn_fail:
    MPIR_MVP_SHMEM_COLL_Cleanup();
    goto fn_exit;
}

int MPIR_MVP_SHMEM_Coll_get_free_block()
{
    int i = 0;
    while (i < MVP_SHMEM_COLL_NUM_COMM) {
        READBAR();
        if (SHMEM_COLL_BLOCK_STATUS_INUSE(shmem_coll_block_status, i)) {
            i++;
        } else {
            break;
        }
    }

    if (i < MVP_SHMEM_COLL_NUM_COMM) {
        SHMEM_COLL_BLOCK_STATUS_SET(shmem_coll_block_status, i);
        WRITEBAR();
        return i;
    } else {
        return -1;
    }
}

void MPIR_MVP_SHMEM_Coll_Block_Clear_Status(int block_id)
{
    SHMEM_COLL_BLOCK_STATUS_CLR(shmem_coll_block_status, block_id);
    WRITEBAR();
}

/* Shared memory gather: root is given as the input of function(int parent)*/
void MPIR_MVP_SHMEM_TREE_COLL_GetShmemBuf_optrels(int size, int rank,
                                                  int shmem_comm_rank,
                                                  int target_rank,
                                                  void **output_buf, int parent)
{
    int cnt = 0, err = 0;
    char *shmem_coll_buf = (char *)(&(shmem_coll->shmem_coll_buf));
    MPIR_FUNC_TERSE_STATE_DECL(
        MPID_STATE_MPIR_MVP_TREE_SHMEM_COLL_GETSHMEMBUF_OPTRELS);
    MPIR_FUNC_TERSE_ENTER(
        MPID_STATE_MPIR_MVP_TREE_SHMEM_COLL_GETSHMEMBUF_OPTRELS);

    READBAR();
    if (rank == parent) {
        READBAR();
        while (SHMEM_COLL_SYNC_ISCLR(child_complete_gather, shmem_comm_rank,
                                     target_rank)) {
#if defined(CKPT)
            Wait_for_CR_Completion();
#endif
            MPID_MVP_PROGRESS_IF_NEEDED
            /* Yield once in a while */
            MPIR_THREAD_CHECK_BEGIN++ cnt;
            if (cnt >= MVP_SHMEM_COLL_SPIN_COUNT) {
                cnt = 0;
#if defined(CKPT)
                MPIDI_CH3I_CR_unlock();
#endif
#if (MPICH_THREAD_LEVEL == MPI_THREAD_MULTIPLE)
                MPIR_THREAD_CHECK_BEGIN
                MPID_THREAD_CS_YIELD(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);
                MPIR_THREAD_CHECK_END
#endif
#if defined(CKPT)
                MPIDI_CH3I_CR_lock();
#endif
            }
            MPIR_THREAD_CHECK_END
            READBAR();
        }
        /* Set the completion flags back to zero */
        SHMEM_COLL_SYNC_CLR(child_complete_gather, shmem_comm_rank,
                            target_rank);
        WRITEBAR();

        *output_buf =
            (char *)shmem_coll_buf + shmem_comm_rank * SHMEM_COLL_BLOCK_SIZE;
    } else {
        READBAR();
        while (SHMEM_COLL_SYNC_ISCLR(root_complete_gather, shmem_comm_rank,
                                     rank)) {
#if defined(CKPT)
            Wait_for_CR_Completion();
#endif
            MPID_MVP_PROGRESS_IF_NEEDED
            /* Yield once in a while */
            MPIR_THREAD_CHECK_BEGIN++ cnt;
            if (cnt >= MVP_SHMEM_COLL_SPIN_COUNT) {
                cnt = 0;
#if defined(CKPT)
                MPIDI_CH3I_CR_unlock();
#endif
#if (MPICH_THREAD_LEVEL == MPI_THREAD_MULTIPLE)
                MPIR_THREAD_CHECK_BEGIN
                MPID_THREAD_CS_YIELD(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);
                MPIR_THREAD_CHECK_END
#endif
#if defined(CKPT)
                MPIDI_CH3I_CR_lock();
#endif
            }
            MPIR_THREAD_CHECK_END
            READBAR();
        }

        SHMEM_COLL_SYNC_CLR(root_complete_gather, shmem_comm_rank, rank);
        WRITEBAR();
        *output_buf =
            (char *)shmem_coll_buf + shmem_comm_rank * SHMEM_COLL_BLOCK_SIZE;
    }
    MPIR_FUNC_TERSE_EXIT(
        MPID_STATE_MPIR_MVP_TREE_SHMEM_COLL_GETSHMEMBUF_OPTRELS);
}

/* Shared memory gather: rank zero is the root always*/
void MPIR_MVP_SHMEM_COLL_GetShmemBuf_optrels(int size, int rank,
                                             int shmem_comm_rank,
                                             int target_rank, void **output_buf)
{
    int cnt = 0, err = 0;
    char *shmem_coll_buf = (char *)(&(shmem_coll->shmem_coll_buf));
    MPIR_FUNC_TERSE_STATE_DECL(
        MPID_STATE_MPIR_MVP_SHMEM_COLL_GETSHMEMBUF_OPTRELS);
    MPIR_FUNC_TERSE_ENTER(MPID_STATE_MPIR_MVP_SHMEM_COLL_GETSHMEMBUF_OPTRELS);

    READBAR();
    if (rank == 0) {
        READBAR();
        while (SHMEM_COLL_SYNC_ISCLR(child_complete_gather, shmem_comm_rank,
                                     target_rank)) {
#if defined(CKPT)
            Wait_for_CR_Completion();
#endif
            MPID_MVP_PROGRESS_IF_NEEDED
            /* Yield once in a while */
            MPIR_THREAD_CHECK_BEGIN++ cnt;
            if (cnt >= MVP_SHMEM_COLL_SPIN_COUNT) {
                cnt = 0;
#if defined(CKPT)
                MPIDI_CH3I_CR_unlock();
#endif
#if (MPICH_THREAD_LEVEL == MPI_THREAD_MULTIPLE)
                MPIR_THREAD_CHECK_BEGIN
                MPID_THREAD_CS_YIELD(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);
                MPIR_THREAD_CHECK_END
#endif
#if defined(CKPT)
                MPIDI_CH3I_CR_lock();
#endif
            }
            MPIR_THREAD_CHECK_END
            READBAR();
        }
        /* Set the completion flags back to zero */
        SHMEM_COLL_SYNC_CLR(child_complete_gather, shmem_comm_rank,
                            target_rank);
        WRITEBAR();

        *output_buf =
            (char *)shmem_coll_buf + shmem_comm_rank * SHMEM_COLL_BLOCK_SIZE;
    } else {
        READBAR();
        while (SHMEM_COLL_SYNC_ISCLR(root_complete_gather, shmem_comm_rank,
                                     rank)) {
#if defined(CKPT)
            Wait_for_CR_Completion();
#endif
            MPID_MVP_PROGRESS_IF_NEEDED
            /* Yield once in a while */
            MPIR_THREAD_CHECK_BEGIN++ cnt;
            if (cnt >= MVP_SHMEM_COLL_SPIN_COUNT) {
                cnt = 0;
#if defined(CKPT)
                MPIDI_CH3I_CR_unlock();
#endif
#if (MPICH_THREAD_LEVEL == MPI_THREAD_MULTIPLE)
                MPIR_THREAD_CHECK_BEGIN
                MPID_THREAD_CS_YIELD(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);
                MPIR_THREAD_CHECK_END
#endif
#if defined(CKPT)
                MPIDI_CH3I_CR_lock();
#endif
            }
            MPIR_THREAD_CHECK_END
            READBAR();
        }

        SHMEM_COLL_SYNC_CLR(root_complete_gather, shmem_comm_rank, rank);
        WRITEBAR();
        *output_buf =
            (char *)shmem_coll_buf + shmem_comm_rank * SHMEM_COLL_BLOCK_SIZE;
    }
    MPIR_FUNC_TERSE_EXIT(MPID_STATE_MPIR_MVP_SHMEM_COLL_GETSHMEMBUF_OPTRELS);
}

/* Shared memory gather: rank zero is the root always*/
void MPIR_MVP_SHMEM_COLL_GetShmemBuf(int size, int rank, int shmem_comm_rank,
                                     void **output_buf)
{
    int i = 1, cnt = 0, err = 0;
    char *shmem_coll_buf = (char *)(&(shmem_coll->shmem_coll_buf));
    MPIR_FUNC_TERSE_STATE_DECL(MPID_STATE_MPIR_MVP_SHMEM_COLL_GETSHMEMBUF);
    MPIR_FUNC_TERSE_ENTER(MPID_STATE_MPIR_MVP_SHMEM_COLL_GETSHMEMBUF);

    READBAR();
    if (rank == 0) {
        for (i = 1; i < size; ++i) {
            READBAR();
            while (SHMEM_COLL_SYNC_ISCLR(child_complete_gather, shmem_comm_rank,
                                         i)) {
#if defined(CKPT)
                Wait_for_CR_Completion();
#endif
                MPID_MVP_PROGRESS_IF_NEEDED
                /* Yield once in a while */
                MPIR_THREAD_CHECK_BEGIN++ cnt;
                if (cnt >= MVP_SHMEM_COLL_SPIN_COUNT) {
                    cnt = 0;
#if defined(CKPT)
                    MPIDI_CH3I_CR_unlock();
#endif
#if (MPICH_THREAD_LEVEL == MPI_THREAD_MULTIPLE)
                    MPIR_THREAD_CHECK_BEGIN
                    MPID_THREAD_CS_YIELD(GLOBAL,
                                         MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);
                    MPIR_THREAD_CHECK_END
#endif
#if defined(CKPT)
                    MPIDI_CH3I_CR_lock();
#endif
                }
                MPIR_THREAD_CHECK_END
                READBAR();
            }
        }
        /* Set the completion flags back to zero */
        for (i = 1; i < size; ++i) {
            SHMEM_COLL_SYNC_CLR(child_complete_gather, shmem_comm_rank, i);
            WRITEBAR();
        }

        *output_buf =
            (char *)shmem_coll_buf + shmem_comm_rank * SHMEM_COLL_BLOCK_SIZE;
    } else {
        READBAR();
        while (SHMEM_COLL_SYNC_ISCLR(root_complete_gather, shmem_comm_rank,
                                     rank)) {
#if defined(CKPT)
            Wait_for_CR_Completion();
#endif
            MPID_MVP_PROGRESS_IF_NEEDED
            /* Yield once in a while */
            MPIR_THREAD_CHECK_BEGIN++ cnt;
            if (cnt >= MVP_SHMEM_COLL_SPIN_COUNT) {
                cnt = 0;
#if defined(CKPT)
                MPIDI_CH3I_CR_unlock();
#endif
#if (MPICH_THREAD_LEVEL == MPI_THREAD_MULTIPLE)
                MPIR_THREAD_CHECK_BEGIN
                MPID_THREAD_CS_YIELD(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);
                MPIR_THREAD_CHECK_END
#endif
#if defined(CKPT)
                MPIDI_CH3I_CR_lock();
#endif
            }
            MPIR_THREAD_CHECK_END
            READBAR();
        }

        SHMEM_COLL_SYNC_CLR(root_complete_gather, shmem_comm_rank, rank);
        WRITEBAR();
        *output_buf =
            (char *)shmem_coll_buf + shmem_comm_rank * SHMEM_COLL_BLOCK_SIZE;
    }
    MPIR_FUNC_TERSE_EXIT(MPID_STATE_MPIR_MVP_SHMEM_COLL_GETSHMEMBUF);
}

/* Shared memory bcast: rank zero is the root always*/
void MPIR_MVP_SHMEM_Bcast_GetBuf(int size, int rank, int shmem_comm_rank,
                                 void **output_buf)
{
    int i = 1, cnt = 0, err = 0;
    char *shmem_coll_buf =
        (char *)(&(shmem_coll->shmem_coll_buf) +
                 MVP_SHMEM_COLL_NUM_COMM * SHMEM_COLL_BLOCK_SIZE);
    MPIR_FUNC_TERSE_STATE_DECL(MPID_STATE_MPIR_MVP_SHMEM_BCAST_GETBUF);
    MPIR_FUNC_TERSE_ENTER(MPID_STATE_MPIR_MVP_SHMEM_BCAST_GETBUF);

    READBAR();
    if (rank == 0) {
        for (i = 1; i < size; ++i) {
            READBAR();
            while (SHMEM_BCAST_SYNC_ISSET(child_complete_bcast, shmem_comm_rank,
                                          i)) {
#if defined(CKPT)
                Wait_for_CR_Completion();
#endif
                MPID_MVP_PROGRESS_IF_NEEDED
                /* Yield once in a while */
                MPIR_THREAD_CHECK_BEGIN++ cnt;
                if (cnt >= MVP_SHMEM_COLL_SPIN_COUNT) {
                    cnt = 0;
#if defined(CKPT)
                    MPIDI_CH3I_CR_unlock();
#endif
#if (MPICH_THREAD_LEVEL == MPI_THREAD_MULTIPLE)
                    MPIR_THREAD_CHECK_BEGIN
                    MPID_THREAD_CS_YIELD(GLOBAL,
                                         MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);
                    MPIR_THREAD_CHECK_END
#endif
#if defined(CKPT)
                    MPIDI_CH3I_CR_lock();
#endif
                }
                MPIR_THREAD_CHECK_END
                READBAR();
            }
        }
        *output_buf =
            (char *)shmem_coll_buf + shmem_comm_rank * SHMEM_COLL_BLOCK_SIZE;
    } else {
        READBAR();
        while (SHMEM_BCAST_SYNC_ISCLR(child_complete_bcast, shmem_comm_rank,
                                      rank)) {
#if defined(CKPT)
            Wait_for_CR_Completion();
#endif
            MPID_MVP_PROGRESS_IF_NEEDED
            /* Yield once in a while */
            MPIR_THREAD_CHECK_BEGIN++ cnt;
            if (cnt >= MVP_SHMEM_COLL_SPIN_COUNT) {
                cnt = 0;
#if defined(CKPT)
                MPIDI_CH3I_CR_unlock();
#endif
#if (MPICH_THREAD_LEVEL == MPI_THREAD_MULTIPLE)
                MPIR_THREAD_CHECK_BEGIN
                MPID_THREAD_CS_YIELD(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);
                MPIR_THREAD_CHECK_END
#endif
#if defined(CKPT)
                MPIDI_CH3I_CR_lock();
#endif
            }
            MPIR_THREAD_CHECK_END
            READBAR();
        }
        *output_buf =
            (char *)shmem_coll_buf + shmem_comm_rank * SHMEM_COLL_BLOCK_SIZE;
    }
    READBAR();
    MPIR_FUNC_TERSE_EXIT(MPID_STATE_MPIR_MVP_SHMEM_BCAST_GETBUF);
}

/* Shared memory bcast: rank zero is the root always*/
void MPIR_MVP_SHMEM_Bcast_Complete(int size, int rank, int shmem_comm_rank)
{
    int i = 1;
    MPIR_FUNC_TERSE_STATE_DECL(MPID_STATE_MPIR_MVP_SHMEM_COLL_SETBCASTCOMPLETE);
    MPIR_FUNC_TERSE_ENTER(MPID_STATE_MPIR_MVP_SHMEM_COLL_SETBCASTCOMPLETE);

    READBAR();
    if (rank == 0) {
        for (i = 1; i < size; ++i) {
            SHMEM_BCAST_SYNC_SET(child_complete_bcast, shmem_comm_rank, i);
            WRITEBAR();
        }
    } else {
        SHMEM_BCAST_SYNC_CLR(child_complete_bcast, shmem_comm_rank, rank);
        WRITEBAR();
    }
    READBAR();
    MPIR_FUNC_TERSE_EXIT(MPID_STATE_MPIR_MVP_SHMEM_COLL_SETBCASTCOMPLETE);
}

void MPIR_MVP_SHMEM_TREE_COLL_SetGatherComplete_optrels(int size, int rank,
                                                        int shmem_comm_rank,
                                                        int target_rank,
                                                        int parent)
{
    MPIR_FUNC_TERSE_STATE_DECL(
        MPID_STATE_MPIR_MVP_TREE_SHMEM_COLL_SETGATHERCOMPLETE_OPTRELS);
    MPIR_FUNC_TERSE_ENTER(
        MPID_STATE_MPIR_MVP_TREE_SHMEM_COLL_SETGATHERCOMPLETE_OPTRELS);

    READBAR();
    if (rank == parent) {
        SHMEM_COLL_SYNC_SET(root_complete_gather, shmem_comm_rank, target_rank);
    } else {
        SHMEM_COLL_SYNC_SET(child_complete_gather, shmem_comm_rank, rank);
        WRITEBAR();
    }
    MPIR_FUNC_TERSE_EXIT(
        MPID_STATE_MPIR_MVP_TREE_SHMEM_COLL_SETGATHERCOMPLETE_OPTRELS);
}

void MPIR_MVP_SHMEM_COLL_SetGatherComplete_optrels(int size, int rank,
                                                   int shmem_comm_rank,
                                                   int target_rank)
{
    MPIR_FUNC_TERSE_STATE_DECL(
        MPID_STATE_MPIR_MVP_SHMEM_COLL_SETGATHERCOMPLETE_OPTRELS);
    MPIR_FUNC_TERSE_ENTER(
        MPID_STATE_MPIR_MVP_SHMEM_COLL_SETGATHERCOMPLETE_OPTRELS);

    READBAR();
    if (rank == 0) {
        SHMEM_COLL_SYNC_SET(root_complete_gather, shmem_comm_rank, target_rank);
    } else {
        SHMEM_COLL_SYNC_SET(child_complete_gather, shmem_comm_rank, rank);
        WRITEBAR();
    }
    MPIR_FUNC_TERSE_EXIT(
        MPID_STATE_MPIR_MVP_SHMEM_COLL_SETGATHERCOMPLETE_OPTRELS);
}

void MPIR_MVP_SHMEM_COLL_SetGatherComplete(int size, int rank,
                                           int shmem_comm_rank)
{
    int i = 1;
    MPIR_FUNC_TERSE_STATE_DECL(
        MPID_STATE_MPIR_MVP_SHMEM_COLL_SETGATHERCOMPLETE);
    MPIR_FUNC_TERSE_ENTER(MPID_STATE_MPIR_MVP_SHMEM_COLL_SETGATHERCOMPLETE);

    READBAR();
    if (rank == 0) {
        for (; i < size; ++i) {
            SHMEM_COLL_SYNC_SET(root_complete_gather, shmem_comm_rank, i);
            WRITEBAR();
        }
    } else {
        SHMEM_COLL_SYNC_SET(child_complete_gather, shmem_comm_rank, rank);
        WRITEBAR();
    }
    MPIR_FUNC_TERSE_EXIT(MPID_STATE_MPIR_MVP_SHMEM_COLL_SETGATHERCOMPLETE);
}

void MPIR_MVP_SHMEM_COLL_Barrier_gather(int size, int rank, int shmem_comm_rank)
{
    int i = 1, cnt = 0, err = 0;
    MPIR_FUNC_TERSE_STATE_DECL(MPID_STATE_MPIR_MVP_SHMEM_COLL_BARRIER_GATHER);
    MPIR_FUNC_TERSE_ENTER(MPID_STATE_MPIR_MVP_SHMEM_COLL_BARRIER_GATHER);

    READBAR();
    if (rank == 0) {
        for (i = 1; i < size; ++i) {
            READBAR();
            while (SHMEM_COLL_SYNC_ISCLR(barrier_gather, shmem_comm_rank, i)) {
#if defined(CKPT)
                Wait_for_CR_Completion();
#endif
                MPID_MVP_PROGRESS_IF_NEEDED
                /* Yield once in a while */
                MPIR_THREAD_CHECK_BEGIN++ cnt;
                if (cnt >= MVP_SHMEM_COLL_SPIN_COUNT) {
                    cnt = 0;
#if defined(CKPT)
                    MPIDI_CH3I_CR_unlock();
#endif
#if (MPICH_THREAD_LEVEL == MPI_THREAD_MULTIPLE)
                    MPIR_THREAD_CHECK_BEGIN
                    MPID_THREAD_CS_YIELD(GLOBAL,
                                         MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);
                    MPIR_THREAD_CHECK_END
#endif
#if defined(CKPT)
                    MPIDI_CH3I_CR_lock();
#endif
                }
                MPIR_THREAD_CHECK_END
                READBAR();
            }
        }
        for (i = 1; i < size; ++i) {
            SHMEM_COLL_SYNC_CLR(barrier_gather, shmem_comm_rank, i);
            WRITEBAR();
        }
    } else {
        SHMEM_COLL_SYNC_SET(barrier_gather, shmem_comm_rank, rank);
        WRITEBAR();
    }
    MPIR_FUNC_TERSE_EXIT(MPID_STATE_MPIR_MVP_SHMEM_COLL_BARRIER_GATHER);
}

void MPIR_MVP_SHMEM_COLL_Barrier_bcast(int size, int rank, int shmem_comm_rank)
{
    int i = 1, cnt = 0, err = 0;

    MPIR_FUNC_TERSE_STATE_DECL(MPID_STATE_MPIR_MVP_SHMEM_COLL_BARRIER_BCAST);
    MPIR_FUNC_TERSE_ENTER(MPID_STATE_MPIR_MVP_SHMEM_COLL_BARRIER_BCAST);

    READBAR();
    if (rank == 0) {
        for (i = 1; i < size; ++i) {
            SHMEM_COLL_SYNC_SET(barrier_bcast, shmem_comm_rank, i);
            WRITEBAR();
        }
    } else {
        READBAR();
        while (SHMEM_COLL_SYNC_ISCLR(barrier_bcast, shmem_comm_rank, rank)) {
#if defined(CKPT)
            Wait_for_CR_Completion();
#endif
            MPID_MVP_PROGRESS_IF_NEEDED
            /* Yield once in a while */
            MPIR_THREAD_CHECK_BEGIN++ cnt;
            if (cnt >= MVP_SHMEM_COLL_SPIN_COUNT) {
                cnt = 0;
#if defined(CKPT)
                MPIDI_CH3I_CR_unlock();
#endif
#if (MPICH_THREAD_LEVEL == MPI_THREAD_MULTIPLE)
                MPIR_THREAD_CHECK_BEGIN
                MPID_THREAD_CS_YIELD(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);
                MPIR_THREAD_CHECK_END
#endif
#if defined(CKPT)
                MPIDI_CH3I_CR_lock();
#endif
            }
            MPIR_THREAD_CHECK_END
            READBAR();
        }
        SHMEM_COLL_SYNC_CLR(barrier_bcast, shmem_comm_rank, rank);
        WRITEBAR();
    }

    MPIR_FUNC_TERSE_EXIT(MPID_STATE_MPIR_MVP_SHMEM_COLL_BARRIER_BCAST);
}

void lock_shmem_region() { pthread_spin_lock(&shmem_coll->shmem_coll_lock); }

void unlock_shmem_region()
{
    pthread_spin_unlock(&shmem_coll->shmem_coll_lock);
}

void increment_mvp_shmem_comm_count() { ++shmem_coll->mvp_shmem_comm_count; }

int get_mvp_shmem_comm_count() { return shmem_coll->mvp_shmem_comm_count; }

int is_shmem_collectives_enabled() { return MVP_USE_SHARED_MEM; }

int mvp_increment_shmem_coll_counter(MPIR_Comm *comm_ptr)
{
    int mpi_errno = MPI_SUCCESS, flag = 0;
    PMPI_Comm_test_inter(comm_ptr->handle, &flag);

    if (flag == 0 && MVP_USE_SHARED_MEM &&
        comm_ptr->dev.ch.shmem_coll_ok == 0 &&
        check_split_comm(pthread_self())) {
        comm_ptr->dev.ch.shmem_coll_count++;

        if (comm_ptr->dev.ch.shmem_coll_count >= MVP_TWO_LEVEL_COMM_THRESHOLD) {
            disable_split_comm(pthread_self());
            mpi_errno = create_2level_comm(
                comm_ptr->handle, comm_ptr->local_size, comm_ptr->rank);
            if (mpi_errno) {
                MPIR_ERR_POP(mpi_errno);
            }
            enable_split_comm(pthread_self());
            if (mpi_errno) {
                MPIR_ERR_POP(mpi_errno);
            }
        }
    }

fn_exit:
    return mpi_errno;

fn_fail:
    goto fn_exit;
}

int mvp_increment_allgather_coll_counter(MPIR_Comm *comm_ptr)
{
    MPIR_Errflag_t errflag = MPIR_ERR_NONE;
    int mpi_errno = MPI_SUCCESS, flag = 0;
    PMPI_Comm_test_inter(comm_ptr->handle, &flag);

    if (flag == 0 && MVP_ALLGATHER_REVERSE_RANKING && MVP_USE_SHARED_MEM &&
        check_split_comm(pthread_self())) {
        comm_ptr->dev.ch.allgather_coll_count++;

        if (comm_ptr->dev.ch.allgather_coll_count >=
            MVP_TWO_LEVEL_COMM_THRESHOLD) {
            disable_split_comm(pthread_self());
            if (comm_ptr->dev.ch.shmem_coll_ok == 0) {
                /* If comm_ptr does not have leader/shmem sub-communicators,
                 * create them now */
                mpi_errno = create_2level_comm(
                    comm_ptr->handle, comm_ptr->local_size, comm_ptr->rank);
                if (mpi_errno) {
                    MPIR_ERR_POP(mpi_errno);
                }
            }

            if (comm_ptr->dev.ch.shmem_coll_ok == 1) {
                /* Before we go ahead with allgather-comm creation, be sure that
                 * the sub-communicators are actually ready */
                mpi_errno = create_allgather_comm(comm_ptr, &errflag);
                if (mpi_errno) {
                    MPIR_ERR_POP(mpi_errno);
                }
            }
            enable_split_comm(pthread_self());
            if (mpi_errno) {
                MPIR_ERR_POP(mpi_errno);
            }
        }
    }

fn_exit:
    return mpi_errno;

fn_fail:
    goto fn_exit;
}

#ifdef _ENABLE_CUDA_
/******************************************************************/
// Checks if cuda stage buffer size is sufficient, if not allocates//
// more memory.                                                    //
/******************************************************************/
int device_stage_alloc(void **send_buf, int sendsize, void **recv_buf,
                       int recvsize, int send_on_device, int recv_on_device,
                       int disp)
{
    int page_size = getpagesize();
    int result, mpi_errno = MPI_SUCCESS;

    if (send_on_device && *send_buf != MPI_IN_PLACE &&
        mvp_device_host_sendbuf_size < sendsize) {
        if (mvp_device_host_send_buf) {
            if (mvp_device_host_sendbuf_size >=
                mvp_device_coll_register_stage_buf_threshold) {
                ibv_device_unregister(mvp_device_host_send_buf);
            }
            MPL_free(mvp_device_host_send_buf);
        }
        mvp_device_host_sendbuf_size = sendsize < mvp_device_stage_block_size ?
                                           mvp_device_stage_block_size :
                                           sendsize;
        mvp_device_host_send_buf = MPL_aligned_alloc(
            page_size, mvp_device_host_sendbuf_sizem, MPL_MEM_BUFFER);
        if (NULL == mvp_device_host_send_buf) {
            mpi_errno = MPIR_Err_create_code(
                MPI_SUCCESS, MPI_ERR_OTHER, __func__, __LINE__, MPI_ERR_OTHER,
                "**fail", "%s: %s", "posix_memalign", strerror(errno));
            return (mpi_errno);
        }
        if (mvp_device_host_sendbuf_size >=
            mvp_device_coll_register_stage_buf_threshold) {
            ibv_device_register(mvp_device_host_send_buf,
                                mvp_device_host_sendbuf_size);
        }
    }
    if (recv_on_device && mvp_device_host_recvbuf_size < recvsize) {
        if (mvp_device_host_recv_buf) {
            if (mvp_device_host_recvbuf_size >=
                mvp_device_coll_register_stage_buf_threshold) {
                ibv_device_unregister(mvp_device_host_recv_buf);
            }
            MPL_free(mvp_device_host_recv_buf);
        }
        mvp_device_host_recvbuf_size = recvsize < mvp_device_stage_block_size ?
                                           mvp_device_stage_block_size :
                                           recvsize;
        result = MPL_aligned_alloc(&mvp_device_host_recv_buf, page_size,
                                   mvp_device_host_recvbuf_size);
        if (NULL == mvp_device_host_recv_buf) {
            mpi_errno = MPIR_Err_create_code(
                MPI_SUCCESS, MPI_ERR_OTHER, __func__, __LINE__, MPI_ERR_OTHER,
                "**fail", "%s: %s", "posix_memalign", strerror(errno));
            return (mpi_errno);
        }
        if (mvp_device_host_recvbuf_size >=
            mvp_device_coll_register_stage_buf_threshold) {
            ibv_device_register(mvp_device_host_recv_buf,
                                mvp_device_host_recvbuf_size);
        }
    }

    if (send_on_device && *send_buf != MPI_IN_PLACE) {
        if (send_on_device) {
            MVP_MPIDI_Memcpy_Device(mvp_device_host_send_buf, *send_buf,
                                    sendsize, deviceMemcpyDeviceToHost);
        }
    } else {
        if (recv_on_device) {
            MVP_MPIDI_Memcpy_Device(((char *)(mvp_device_host_recv_buf) + disp),
                                    ((char *)(*recv_buf) + disp), sendsize,
                                    deviceMemcpyDeviceToHost);
        }
    }

    if (send_on_device && send_buf != MPI_IN_PLACE) {
        mvp_device_original_send_buf = *send_buf;
        *send_buf = mvp_device_host_send_buf;
    } else {
        mvp_device_original_send_buf = NULL;
    }
    if (recv_on_device) {
        mvp_device_original_recv_buf = *recv_buf;
        *recv_buf = mvp_device_host_recv_buf;
    } else {
        mvp_device_original_recv_buf = NULL;
    }
    return mpi_errno;
}

/******************************************************************/
// After performing the cuda collective operation, sendbuf and recv//
//-buf are made to point back to device buf.                      //
/******************************************************************/
void device_stage_free(void **send_buf, void **recv_buf, int recvsize,
                       int send_on_device, int recv_on_device)
{
    if (send_on_device && mvp_device_original_send_buf &&
        send_buf != MPI_IN_PLACE) {
        if (!recv_on_device && !mvp_device_original_recv_buf) {
            MVP_MPIDI_Memcpy_Device(mvp_device_original_send_buf, *send_buf,
                                    recvsize, deviceMemcpyHostToDevice);
        }
        *send_buf = mvp_device_original_send_buf;
        mvp_device_original_send_buf = NULL;
    }
    if (recv_on_device && mvp_device_original_recv_buf) {
        MVP_MPIDI_Memcpy_Device(mvp_device_original_recv_buf, *recv_buf,
                                recvsize, deviceMemcpyHostToDevice);
        *recv_buf = mvp_device_original_recv_buf;
        mvp_device_original_recv_buf = NULL;
    }
}

int device_stage_alloc_v(void **send_buf, int *send_counts,
                         MPI_Datatype send_type, int **send_displs,
                         int send_peers, void **recv_buf, int *recv_counts,
                         MPI_Datatype recv_type, int **recv_displs,
                         int recv_peers, int send_buf_on_device,
                         int recv_buf_on_device, int rank)
{
    int mpi_errno = MPI_SUCCESS;
    int i, page_size, result;
    int total_send_size = 0, total_recv_size = 0, total_buf_size = 0,
        offset = 0;
    int recv_type_contig = 0;
    MPI_Aint send_type_extent, recv_type_extent;
    MPIR_Datatype *dtp;

    page_size = getpagesize();
    MPIR_Datatype_get_extent_macro(send_type, send_type_extent);
    MPIR_Datatype_get_extent_macro(recv_type, recv_type_extent);

    if (HANDLE_GET_KIND(recv_type) == HANDLE_KIND_BUILTIN)
        recv_type_contig = 1;
    else {
        MPIR_Datatype_get_ptr(recv_type, dtp);
        recv_type_contig = dtp->is_contig;
    }

    if (send_buf_on_device && *send_buf != MPI_IN_PLACE) {
        for (i = 0; i < send_peers; i++) {
            total_send_size += send_counts[i] * send_type_extent;
        }
    }
    if (recv_buf_on_device && *recv_buf != MPI_IN_PLACE) {
        for (i = 0; i < recv_peers; i++) {
            total_recv_size += recv_counts[i] * recv_type_extent;
        }
    }
    total_buf_size =
        (total_send_size > total_recv_size) ? total_send_size : total_recv_size;

    /* Allocate the packing buffer on device if one does not exist
     * or is not large enough. Free the older one */
    if (mvp_device_dev_srbuf_size < total_buf_size) {
        if (mvp_device_dev_sr_buf) {
            MVP_MPIDI_Free_Device(mvp_device_dev_sr_buf);
        }
        mvp_device_dev_srbuf_size = total_buf_size;
        MVP_MPIDI_Malloc_Device(mvp_device_dev_sr_buf,
                                mvp_device_dev_srbuf_size);
    }

    /* Allocate the stage out (send) host buffers if they do not exist
     * or are not large enough. Free the older one */
    if (send_buf_on_device && *send_buf != MPI_IN_PLACE) {
        /*allocate buffer to stage displacements */
        if (mvp_device_host_send_peers < send_peers) {
            if (mvp_device_host_send_displs) {
                MPL_free(mvp_device_host_send_displs);
            }
            mvp_device_host_send_peers = send_peers;
            mvp_device_host_send_displs = MPL_malloc(
                sizeof(int) * mvp_device_host_send_peers, MPL_MEM_COLL);
            MPIR_Memset((void *)mvp_device_host_send_displs, 0,
                        sizeof(int) * mvp_device_host_send_peers);
        }
        /*allocate buffer to stage the data */
        if (mvp_device_host_sendbuf_size < total_send_size) {
            if (mvp_device_host_send_buf) {
                if (mvp_device_host_sendbuf_size >=
                    mvp_device_coll_register_stage_buf_threshold) {
                    ibv_device_unregister(mvp_device_host_send_buf);
                }
                MPL_free(mvp_device_host_send_buf);
            }
            mvp_device_host_sendbuf_size =
                total_send_size < mvp_device_stage_block_size ?
                    mvp_device_stage_block_size :
                    total_send_size;
            mvp_device_host_send_buf = MPL_aligned_alloc(
                page_size, mvp_device_host_sendbuf_size, MPL_MEM_BUFFER);
            if (NULL == mvp_device_host_send_buf) {
                mpi_errno = MPIR_Err_create_code(
                    MPI_SUCCESS, MPI_ERR_OTHER, __func__, __LINE__,
                    MPI_ERR_OTHER, "**fail", "%s: %s", "posix_memalign",
                    strerror(errno));
                return (mpi_errno);
            }
            if (mvp_device_host_sendbuf_size >=
                mvp_device_coll_register_stage_buf_threshold) {
                ibv_device_register(mvp_device_host_send_buf,
                                    mvp_device_host_sendbuf_size);
            }
        }
    }

    /* allocate the stage in (recv) host buffers if they do not exist
     * or are not large enough */
    if (recv_buf_on_device && *recv_buf != MPI_IN_PLACE) {
        /*allocate buffer to stage displacements */
        if (mvp_device_host_recv_peers < recv_peers) {
            if (mvp_device_host_recv_displs) {
                MPL_free(mvp_device_host_recv_displs);
            }
            mvp_device_host_recv_peers = recv_peers;
            mvp_device_host_recv_displs = MPL_malloc(
                sizeof(int) * mvp_device_host_recv_peers, MPL_MEM_COLL);
            MPIR_Memset(mvp_device_host_recv_displs, 0,
                        sizeof(int) * mvp_device_host_recv_peers);
        }
        /*allocate buffer to stage the data */
        if (mvp_device_host_recvbuf_size < total_recv_size) {
            if (mvp_device_host_recv_buf) {
                if (mvp_device_host_recvbuf_size >=
                    mvp_device_coll_register_stage_buf_threshold) {
                    ibv_device_unregister(mvp_device_host_recv_buf);
                }
                MPL_free(mvp_device_host_recv_buf);
            }
            mvp_device_host_recvbuf_size =
                total_recv_size < mvp_device_stage_block_size ?
                    mvp_device_stage_block_size :
                    total_recv_size;
            mvp_device_host_recv_buf = MPL_aligned_alloc(
                page_size, mvp_device_host_recvbuf_size, MPL_MEM_BUFFER);
            if (NULL == mvp_device_host_recv_buf) {
                mpi_errno = MPIR_Err_create_code(
                    MPI_SUCCESS, MPI_ERR_OTHER, __func__, __LINE__,
                    MPI_ERR_OTHER, "**fail", "%s: %s", "posix_memalign",
                    strerror(errno));
                return (mpi_errno);
            }
            if (mvp_device_host_recvbuf_size >=
                mvp_device_coll_register_stage_buf_threshold) {
                ibv_device_register(mvp_device_host_recv_buf,
                                    mvp_device_host_recvbuf_size);
            }
        }
    }

    /*Stage out the data to be sent, set the send buffer and displaceemnts */
    offset = 0;
    if (send_buf_on_device && *send_buf != MPI_IN_PLACE) {
        for (i = 0; i < send_peers; i++) {
            MVP_MPIDI_Memcpy_Device(
                (void *)((char *)mvp_device_dev_sr_buf +
                         offset * send_type_extent),
                (void *)((char *)(*send_buf) +
                         (*send_displs)[i] * send_type_extent),
                send_counts[i] * send_type_extent, deviceMemcpyDeviceToDevice);
            mvp_device_host_send_displs[i] = offset;
            offset += send_counts[i];
        }
        MVP_MPIDI_Memcpy_Device(mvp_device_host_send_buf, mvp_device_dev_sr_buf,
                                total_send_size, deviceMemcpyDeviceToHost);

        mvp_device_original_send_buf = (void *)*send_buf;
        mvp_device_original_send_displs = (int *)*send_displs;
        *send_buf = mvp_device_host_send_buf;
        *send_displs = mvp_device_host_send_displs;
    }

    /*Stage out buffer into which data is to be received and set the stage in
       (recv) displacements */
    offset = 0;
    if (recv_buf_on_device && *recv_buf != MPI_IN_PLACE) {
        for (i = 0; i < recv_peers; i++) {
            mvp_device_host_recv_displs[i] = offset;
            offset += recv_counts[i];
        }
        /*If data type is not contig, copy the device receive buffer out onto
           host receive buffer
           to maintain the original data in un-touched parts while copying back
         */
        if (!recv_type_contig) {
            for (i = 0; i < recv_peers; i++) {
                MVP_MPIDI_Memcpy_Device(
                    (void *)((char *)mvp_device_dev_sr_buf +
                             mvp_device_host_recv_displs[i] * recv_type_extent),
                    (void *)((char *)(*recv_buf) +
                             (*recv_displs)[i] * recv_type_extent),
                    recv_counts[i] * recv_type_extent,
                    deviceMemcpyDeviceToDevice);
            }
            MVP_MPIDI_Memcpy_Device(mvp_device_host_recv_buf,
                                    mvp_device_dev_sr_buf, total_recv_size,
                                    deviceMemcpyDeviceToHost);
        }
        mvp_device_original_recv_buf = *recv_buf;
        mvp_device_original_recv_displs = *recv_displs;
        *recv_buf = mvp_device_host_recv_buf;
        *recv_displs = mvp_device_host_recv_displs;
    }

    return mpi_errno;
}

void device_stage_free_v(void **send_buf, int *send_counts,
                         MPI_Datatype send_type, int **send_displs,
                         int send_peers, void **recv_buf, int *recv_counts,
                         MPI_Datatype recv_type, int **recv_displs,
                         int recv_peers, int send_buf_on_device,
                         int recv_buf_on_device, int rank)
{
    int i, total_recv_size = 0;
    MPI_Aint recv_type_extent = 0;

    if (recv_buf_on_device && *recv_buf != MPI_IN_PLACE) {
        MPIR_Datatype_get_extent_macro(recv_type, recv_type_extent);
        for (i = 0; i < recv_peers; i++) {
            total_recv_size += recv_counts[i] * recv_type_extent;
        }
    }

    if (send_buf_on_device && *send_buf != MPI_IN_PLACE) {
        MPIR_Assert(mvp_device_original_send_buf != NULL);
        *send_buf = mvp_device_original_send_buf;
        *send_displs = mvp_device_original_send_displs;
        mvp_device_original_send_buf = NULL;
        mvp_device_original_send_displs = NULL;
    }

    if (recv_buf_on_device && *recv_buf != MPI_IN_PLACE) {
        MVP_MPIDI_Memcpy_Device(mvp_device_dev_sr_buf, *recv_buf,
                                total_recv_size, deviceMemcpyHostToDevice);
        for (i = 0; i < recv_peers; i++) {
            if (send_buf_on_device && *send_buf == MPI_IN_PLACE && i == rank)
                continue;
            MVP_MPIDI_Memcpy_Device(
                (void *)((char *)mvp_device_original_recv_buf +
                         mvp_device_original_recv_displs[i] * recv_type_extent),
                (void *)((char *)mvp_device_dev_sr_buf +
                         (*recv_displs)[i] * recv_type_extent),
                recv_counts[i] * recv_type_extent, deviceMemcpyDeviceToDevice);
        }
        *recv_buf = mvp_device_original_recv_buf;
        *recv_displs = mvp_device_original_recv_displs;
        mvp_device_original_recv_buf = NULL;
        mvp_device_original_recv_displs = NULL;
    }
}

/******************************************************************/
// Freeing the stage buffers during finalize                       //
/******************************************************************/
void DEVICE_COLL_Finalize()
{
    if (mvp_device_host_recv_buf) {
        if (mvp_device_host_recvbuf_size >=
            mvp_device_coll_register_stage_buf_threshold) {
            ibv_device_unregister(mvp_device_host_recv_buf);
        }
        MPL_free(mvp_device_host_recv_buf);
        mvp_device_host_recv_buf = NULL;
    }

    if (mvp_device_host_send_displs) {
        MPL_free(mvp_device_host_send_displs);
        mvp_device_host_send_displs = NULL;
    }

    if (mvp_device_host_send_buf) {
        if (mvp_device_host_sendbuf_size >=
            mvp_device_coll_register_stage_buf_threshold) {
            ibv_device_unregister(mvp_device_host_send_buf);
        }
        MPL_free(mvp_device_host_send_buf);
        mvp_device_host_send_buf = NULL;
    }

    if (mvp_device_host_recv_displs) {
        MPL_free(mvp_device_host_recv_displs);
        mvp_device_host_recv_displs = NULL;
    }

    if (mvp_device_allgather_store_buf) {
        ibv_device_unregister(mvp_device_allgather_store_buf);
        MPL_free(mvp_device_allgather_store_buf);
        mvp_device_allgather_store_buf = NULL;
    }

    if (mvp_device_coll_pack_buf_size) {
        MVP_MPIDI_Free_Device(mvp_device_coll_pack_buf);
        mvp_device_coll_pack_buf_size = 0;
    }

    if (mvp_device_coll_unpack_buf_size) {
        MVP_MPIDI_Free_Device(mvp_device_coll_unpack_buf);
        mvp_device_coll_unpack_buf_size = 0;
    }
}

/******************************************************************/
// Packing non-contig sendbuf                                      //
/******************************************************************/
void device_coll_pack(void **sendbuf, int *sendcount, MPI_Datatype *sendtype,
                      void **recvbuf, int *recvcount, MPI_Datatype *recvtype,
                      int disp, int procs_in_sendbuf, int comm_size)
{
    int sendtype_size = 0, recvtype_size = 0, sendsize = 0, recvsize = 0,
        send_copy_size = 0;
    int sendtype_iscontig = 0, recvtype_iscontig = 0;

    if (*sendtype != MPI_DATATYPE_NULL) {
        MPIR_Datatype_iscontig(*sendtype, &sendtype_iscontig);
    }
    if (*recvtype != MPI_DATATYPE_NULL) {
        MPIR_Datatype_iscontig(*recvtype, &recvtype_iscontig);
    }

    MPIR_Datatype_get_size_macro(*sendtype, sendtype_size);
    MPIR_Datatype_get_size_macro(*recvtype, recvtype_size);

    /*Calculating size of data in recv and send buffers */
    if (*sendbuf != MPI_IN_PLACE) {
        sendsize = *sendcount * sendtype_size;
        send_copy_size = *sendcount * sendtype_size * procs_in_sendbuf;
    } else {
        sendsize = *recvcount * recvtype_size;
        send_copy_size = *recvcount * recvtype_size * procs_in_sendbuf;
    }
    recvsize = *recvcount * recvtype_size * comm_size;

    /*Creating packing and unpacking buffers */
    if (!sendtype_iscontig && send_copy_size > mvp_device_coll_pack_buf_size) {
        MVP_MPIDI_Free_Device(mvp_device_coll_pack_buf);
        MVP_MPIDI_Malloc_Device(mvp_device_coll_pack_buf, send_copy_size);
        mvp_device_coll_pack_buf_size = send_copy_size;
    }
    if (!recvtype_iscontig && recvsize > mvp_device_coll_unpack_buf_size) {
        MVP_MPIDI_Free_Device(mvp_device_coll_unpack_buf);
        MVP_MPIDI_Malloc_Device(mvp_device_coll_unpack_buf, recvsize);
        mvp_device_coll_unpack_buf_size = recvsize;
    }

    /*Packing of data to sendbuf */
    if (*sendbuf != MPI_IN_PLACE && !sendtype_iscontig) {
        MPIR_Localcopy(*sendbuf, *sendcount * procs_in_sendbuf, *sendtype,
                       mvp_device_coll_pack_buf, send_copy_size, MPI_BYTE);
        *sendbuf = mvp_device_coll_pack_buf;
        *sendcount = sendsize;
        *sendtype = MPI_BYTE;
    } else if (*sendbuf == MPI_IN_PLACE && !recvtype_iscontig) {
        MPIR_Localcopy((void *)((char *)(*recvbuf) + disp),
                       (*recvcount) * procs_in_sendbuf, *recvtype,
                       mvp_device_coll_pack_buf, send_copy_size, MPI_BYTE);
        *sendbuf = mvp_device_coll_pack_buf;
        *sendcount = sendsize;
        *sendtype = MPI_BYTE;
    }

    /*Changing recvbuf to contig temp recvbuf */
    if (!recvtype_iscontig) {
        mvp_device_orig_recvbuf = *recvbuf;
        mvp_device_orig_recvcount = *recvcount;
        mvp_device_orig_recvtype = *recvtype;
        *recvbuf = mvp_device_coll_unpack_buf;
        *recvcount = *recvcount * recvtype_size;
        *recvtype = MPI_BYTE;
    }
}

/******************************************************************/
// Unpacking data to non-contig recvbuf                            //
/******************************************************************/
void device_coll_unpack(int *recvcount, int comm_size)
{
    int recvtype_iscontig = 0;

    if (mvp_device_orig_recvbuf &&
        mvp_device_orig_recvtype != MPI_DATATYPE_NULL) {
        MPIR_Datatype_iscontig(mvp_device_orig_recvtype, &recvtype_iscontig);
    }

    /*Unpacking of data to recvbuf */
    if (mvp_device_orig_recvbuf && !recvtype_iscontig) {
        MPIR_Localcopy(mvp_device_coll_unpack_buf, *recvcount * comm_size,
                       MPI_BYTE, mvp_device_orig_recvbuf,
                       mvp_device_orig_recvcount * comm_size,
                       mvp_device_orig_recvtype);
    }

    mvp_device_orig_recvbuf = NULL;
}
#endif /* #ifdef _ENABLE_CUDA_ */

static inline void mvp_shm_progress(int *nspin)
{
    int err = 0;
#if defined(CKPT)
    Wait_for_CR_Completion();
#endif
    MPID_MVP_PROGRESS_IF_NEEDED
    /* Yield once in a while */
    MPIR_THREAD_CHECK_BEGIN
    if (*nspin % 20 == 0) {
#if defined(CKPT)
        MPIDI_CH3I_CR_unlock();
#endif
        MPID_THREAD_CS_YIELD(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);
#if defined(CKPT)
        MPIDI_CH3I_CR_lock();
#endif
    }
    MPIR_THREAD_CHECK_END
}

void mvp_shm_barrier_gather(shmem_info_t *shmem)
{
    int i, nspin = 0;
    MPIR_Assert(shmem->write == shmem->read);
    if (shmem->write != shmem->read) {
        PRINT_ERROR("shmem->write != shmem->read\n");
    }
    int idx = shmem->read % MVP_SHMEM_COLL_WINDOW_SIZE;
    if (shmem->local_rank == 0) {
        for (i = 1; i < shmem->local_size; i++) {
            while (shmem->queue[i].shm_slots[idx]->psn != shmem->read) {
                nspin++;
                if (nspin % MVP_SHMEM_COLL_SPIN_COUNT == 0) {
                    mvp_shm_progress(&nspin);
                }
            }
            READBAR();
        }
    } else {
        shmem->queue[shmem->local_rank].shm_slots[idx]->psn = shmem->write;
        WRITEBAR();
    }
}

void mvp_shm_barrier_bcast(shmem_info_t *shmem)
{
    int nspin = 0;
    int idx = shmem->read % MVP_SHMEM_COLL_WINDOW_SIZE;
    if (shmem->local_rank == 0) {
        WRITEBAR();
        shmem->queue[0].shm_slots[idx]->psn = shmem->write;
    } else {
        while (shmem->queue[0].shm_slots[idx]->psn != shmem->read) {
            nspin++;
            if (nspin % MVP_SHMEM_COLL_SPIN_COUNT == 0) {
                mvp_shm_progress(&nspin);
            }
        }
        READBAR();
    }
    shmem->write++;
    shmem->read++;
    MPIR_Assert(shmem->write == shmem->read);
    if (shmem->write != shmem->read) {
        PRINT_ERROR("shmem->write != shmem->read\n");
    }
}

void mvp_shm_barrier(shmem_info_t *shmem)
{
    int i, nspin = 0;
    MPIR_Assert(shmem->write == shmem->read);
    int idx = shmem->read % MVP_SHMEM_COLL_WINDOW_SIZE;
    if (shmem->local_rank == 0) {
        for (i = 1; i < shmem->local_size; i++) {
            while (shmem->queue[i].shm_slots[idx]->psn != shmem->read) {
                nspin++;
                if (nspin % MVP_SHMEM_COLL_SPIN_COUNT == 0) {
                    mvp_shm_progress(&nspin);
                }
            }
            READBAR();
        }
        WRITEBAR();
        shmem->queue[0].shm_slots[idx]->psn = shmem->write;
    } else {
        shmem->queue[shmem->local_rank].shm_slots[idx]->psn = shmem->write;
        while (shmem->queue[0].shm_slots[idx]->psn != shmem->read) {
            nspin++;
            if (nspin % MVP_SHMEM_COLL_SPIN_COUNT == 0) {
                mvp_shm_progress(&nspin);
            }
        }
        READBAR();
    }
    shmem->write++;
    shmem->read++;
}

void mvp_shm_reduce(shmem_info_t *shmem, char *in_buf, int len, int count,
                    int root, MPI_User_function *uop, MPI_Datatype datatype,
                    int is_cxx_uop)
{
    char *buf = NULL;
    int i, nspin = 0;
    int windex = shmem->write % MVP_SHMEM_COLL_WINDOW_SIZE;
    int rindex = shmem->read % MVP_SHMEM_COLL_WINDOW_SIZE;

    /* Copy the data from the input buffer to the shm slot. This is to
     * ensure that we don't mess with the sendbuf supplied
     * by the application */
#if defined(_ENABLE_CUDA_)
    if (mvp_enable_device) {
        MPIR_Localcopy(in_buf, len, MPI_BYTE,
                       shmem->queue[shmem->local_rank].shm_slots[windex]->buf,
                       len, MPI_BYTE);
    } else
#endif
    {
        MPIR_Memcpy(shmem->queue[shmem->local_rank].shm_slots[windex]->buf,
                    in_buf, len);
    }

    buf = shmem->queue[shmem->local_rank].shm_slots[windex]->buf;

    if (shmem->local_rank == root) {
        for (i = 1; i < shmem->local_size; i++) {
            while (shmem->queue[i].shm_slots[rindex]->psn != shmem->read) {
                nspin++;
                if (nspin % MVP_SHMEM_COLL_SPIN_COUNT == 0) {
                    mvp_shm_progress(&nspin);
                }
            }
            READBAR();
#ifdef HAVE_CXX_BINDING
            if (is_cxx_uop) {
                (*MPIR_Process.cxx_call_op_fn)(
                    shmem->queue[i].shm_slots[rindex]->buf, buf, count,
                    datatype, uop);
            } else
#endif
                (*uop)(shmem->queue[i].shm_slots[rindex]->buf, buf, &count,
                       &datatype);
        }
    } else {
        WRITEBAR();
        shmem->queue[shmem->local_rank].shm_slots[windex]->psn = shmem->write;
    }
}

void mvp_shm_tree_reduce(shmem_info_t *shmem, char *in_buf, int len, int count,
                         int root, MPI_User_function *uop,
                         MPI_Datatype datatype, int is_cxx_uop)
{
    void *buf = NULL;
    int i, nspin = 0;
    int start = 0, end = 0;
    int windex = shmem->write % MVP_SHMEM_COLL_WINDOW_SIZE;
    int rindex = shmem->read % MVP_SHMEM_COLL_WINDOW_SIZE;

    if (shmem->local_rank == root ||
        shmem->local_rank % MVP_SHMEM_REDUCE_TREE_DEGREE == 0) {
        start = shmem->local_rank;
        end = shmem->local_rank + MVP_SHMEM_REDUCE_TREE_DEGREE;
        if (end > shmem->local_size)
            end = shmem->local_size;

            /* Copy the data from the input buffer to the shm slot. This is to
             * ensure that we don't mess with the sendbuf supplied by the
             * application */
#if defined(_ENABLE_CUDA_)
        if (mvp_enable_device) {
            MPIR_Localcopy(
                in_buf, len, MPI_BYTE,
                shmem->queue[shmem->local_rank].shm_slots[windex]->buf, len,
                MPI_BYTE);
        } else
#endif
        {
            MPIR_Memcpy(shmem->queue[shmem->local_rank].shm_slots[windex]->buf,
                        in_buf, len);
        }

        buf = shmem->queue[shmem->local_rank].shm_slots[windex]->buf;

        for (i = start + 1; i < end; i++) {
            while (shmem->queue[i].shm_slots[rindex]->psn != shmem->read) {
                nspin++;
                if (nspin % MVP_SHMEM_COLL_SPIN_COUNT == 0) {
                    mvp_shm_progress(&nspin);
                }
            }
            READBAR();
#ifdef HAVE_CXX_BINDING
            if (is_cxx_uop) {
                (*MPIR_Process.cxx_call_op_fn)(
                    shmem->queue[i].shm_slots[rindex]->buf, buf, count,
                    datatype, uop);
            } else
#endif
                (*uop)(shmem->queue[i].shm_slots[rindex]->buf, buf, &count,
                       &datatype);
        }

        if (shmem->local_rank == root) {
            for (i = MVP_SHMEM_REDUCE_TREE_DEGREE; i < shmem->local_size;
                 i = i + MVP_SHMEM_REDUCE_TREE_DEGREE) {
                while (shmem->queue[i].shm_slots[rindex]->psn != shmem->read) {
                    nspin++;
                    if (nspin % MVP_SHMEM_COLL_SPIN_COUNT == 0) {
                        mvp_shm_progress(&nspin);
                    }
                }
                READBAR();
#ifdef HAVE_CXX_BINDING
                if (is_cxx_uop) {
                    (*MPIR_Process.cxx_call_op_fn)(
                        shmem->queue[i].shm_slots[rindex]->buf, buf, count,
                        datatype, uop);
                } else
#endif

                    (*uop)(shmem->queue[i].shm_slots[rindex]->buf, buf, &count,
                           &datatype);
            }
        } else {
            WRITEBAR();
            shmem->queue[shmem->local_rank].shm_slots[windex]->psn =
                shmem->write;
        }

    } else {
#if defined(_ENABLE_CUDA_)
        if (mvp_enable_device) {
            MPIR_Localcopy(
                in_buf, len, MPI_BYTE,
                shmem->queue[shmem->local_rank].shm_slots[windex]->buf, len,
                MPI_BYTE);
        } else
#endif
        {
            MPIR_Memcpy(shmem->queue[shmem->local_rank].shm_slots[windex]->buf,
                        in_buf, len);
        }

        WRITEBAR();
        shmem->queue[shmem->local_rank].shm_slots[windex]->psn = shmem->write;
    }
}

#if defined(CHANNEL_MRAIL_GEN2) || defined(CHANNEL_NEMESIS_IB)
static inline int mvp_post_zcpy_mid_request(MPIR_Comm *leader_commptr,
                                            shmem_info_t *shmem)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Request *req_ptr = NULL;

    /* Post Ibarrier with mid-request */
    MPIR_Request_get_ptr(shmem->mid_request, req_ptr);
    mpi_errno = MPIR_Ibarrier(leader_commptr, &req_ptr);
    MPIR_ERR_CHECK(mpi_errno);

    /* create completed request handle */
    if (!req_ptr) {
        req_ptr = MPIR_Request_create_complete(MPIR_REQUEST_KIND__COLL);
    }
    /* assign handle to the shmem object */
    shmem->mid_request = req_ptr->handle;
    shmem->mid_request_active = 1;

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int mvp_flush_zcpy_mid_request(shmem_info_t *shmem)
{
    int mpi_errno = MPI_SUCCESS;
    MPI_Status status;

    /* Wait for previous ibarrier with mid-request to complete */
    mpi_errno = MPIR_Wait(&(shmem->mid_request), &status);
    MPIR_ERR_CHECK(mpi_errno);

    shmem->mid_request = MPI_REQUEST_NULL;
    shmem->mid_request_active = 0;

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int mvp_post_zcpy_end_request(MPIR_Comm *leader_commptr, shmem_info_t *shmem)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Request *req_ptr = NULL;

    /* Post Ibarrier with mid-request */
    MPIR_Request_get_ptr(shmem->end_request, req_ptr);
    mpi_errno = MPIR_Ibarrier(leader_commptr, &req_ptr);
    MPIR_ERR_CHECK(mpi_errno);

    /* create completed request handle */
    if (!req_ptr) {
        req_ptr = MPIR_Request_create_complete(MPIR_REQUEST_KIND__COLL);
    }
    /* assign handle to the shmem object */
    shmem->end_request = req_ptr->handle;
    shmem->end_request_active = 1;

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int mvp_flush_zcpy_end_request(shmem_info_t *shmem)
{
    int mpi_errno = MPI_SUCCESS;
    MPI_Status status;

    /* Wait for previous ibarrier with mid-request to complete */
    mpi_errno = MPIR_Wait(&(shmem->end_request), &status);
    MPIR_ERR_CHECK(mpi_errno);

    shmem->end_request = MPI_REQUEST_NULL;
    shmem->end_request_active = 0;

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
#endif /*defined(CHANNEL_MRAIL_GEN2) || defined(CHANNEL_NEMESIS_IB)*/

int mvp_shm_bcast(shmem_info_t *shmem, char *buf, int len, int root)
{
    int mpi_errno = MPI_SUCCESS;
    int nspin = 0;
    int windex = -1, rindex = -1;
    MPIR_Comm *shmem_commptr = NULL, *comm_ptr = NULL;

    MPIR_Comm_get_ptr(shmem->comm, comm_ptr);
    MPIR_Comm_get_ptr(comm_ptr->dev.ch.shmem_comm, shmem_commptr);
#if defined(CHANNEL_MRAIL_GEN2) || defined(CHANNEL_NEMESIS_IB)
    int intra_node_root = 0;
    MPIR_Comm *leader_commptr = NULL;
    MPIR_Comm_get_ptr(comm_ptr->dev.ch.leader_comm, leader_commptr);
#endif
    if (shmem_commptr) {
        shmem = shmem_commptr->dev.ch.shmem_info;
    }
    windex = shmem->write % MVP_SHMEM_COLL_WINDOW_SIZE;
    rindex = shmem->read % MVP_SHMEM_COLL_WINDOW_SIZE;

    if (shmem->local_size > 0) {
        if (shmem->local_rank == root) {
#if defined(_ENABLE_CUDA_)
            if (mvp_enable_device) {
                MPIR_Localcopy(buf, len, MPI_BYTE,
                               shmem->queue[root].shm_slots[windex]->buf, len,
                               MPI_BYTE);
            } else
#endif
            {
                MPIR_Memcpy(shmem->queue[root].shm_slots[windex]->buf, buf,
                            len);
            }
            WRITEBAR();
            shmem->queue[root].shm_slots[windex]->psn = shmem->write;
        } else {
            while (shmem->queue[root].shm_slots[rindex]->psn != shmem->read) {
                nspin++;
                if (nspin % MVP_SHMEM_COLL_SPIN_COUNT == 0) {
                    mvp_shm_progress(&nspin);
                }
            }
            READBAR();
#if defined(_ENABLE_CUDA_)
            if (mvp_enable_device) {
                MPIR_Localcopy(shmem->queue[root].shm_slots[rindex]->buf, len,
                               MPI_BYTE, buf, len, MPI_BYTE);
            } else
#endif
            {
                MPIR_Memcpy(buf, shmem->queue[root].shm_slots[rindex]->buf,
                            len);
            }
        }
    }
    shmem->write++;
    shmem->read++;
#if defined(CHANNEL_MRAIL_GEN2) || defined(CHANNEL_NEMESIS_IB)
    if (shmem->half_full_complete == 0 &&
        IS_SHMEM_WINDOW_HALF_FULL(shmem->write, shmem->tail)) {
        PRINT_DEBUG(DEBUG_SHM_verbose > 1, "shmem window half full: %llu \n",
                    shmem->write);
        mvp_shm_barrier(shmem);
        if (shmem->local_rank == intra_node_root && leader_commptr &&
            leader_commptr->local_size > 1) {
            if (shmem->end_request_active == 1) {
                mpi_errno = mvp_flush_zcpy_end_request(shmem);
                if (mpi_errno)
                    MPIR_ERR_POP(mpi_errno);
            }

            if (shmem->mid_request_active == 0) {
                mpi_errno = mvp_post_zcpy_mid_request(leader_commptr, shmem);
                if (mpi_errno)
                    MPIR_ERR_POP(mpi_errno);
            }
        }
        shmem->half_full_complete = 1;
    }

    if (IS_SHMEM_WINDOW_FULL(shmem->write, shmem->tail)) {
        PRINT_DEBUG(DEBUG_SHM_verbose > 1, "shmem window full: %llu \n",
                    shmem->write);
        mvp_shm_barrier(shmem);
        if (shmem->local_rank == intra_node_root && leader_commptr &&
            leader_commptr->local_size > 1) {
            if (shmem->mid_request_active == 1) {
                mpi_errno = mvp_flush_zcpy_mid_request(shmem);
                if (mpi_errno)
                    MPIR_ERR_POP(mpi_errno);
            }

            if (shmem->end_request_active == 0) {
                mpi_errno = mvp_post_zcpy_end_request(leader_commptr, shmem);
                if (mpi_errno)
                    MPIR_ERR_POP(mpi_errno);
            }
        }
        shmem->tail = shmem->read;
        shmem->half_full_complete = 0;
    }
#else  /* defined(CHANNEL_MRAIL_GEN2) || defined(CHANNEL_NEMESIS_IB) */
    if (IS_SHMEM_WINDOW_FULL(shmem->write, shmem->tail)) {
        PRINT_DEBUG(DEBUG_SHM_verbose > 1, "shmem window full: %llu \n",
                    shmem->write);
        mvp_shm_barrier(shmem);
        shmem->tail = shmem->read;
    }
#endif /* defined(CHANNEL_MRAIL_GEN2) || defined(CHANNEL_NEMESIS_IB) */

fn_exit:
    return mpi_errno;
#if defined(CHANNEL_MRAIL_GEN2) || defined(CHANNEL_NEMESIS_IB)
fn_fail:
#endif
    goto fn_exit;
}

int mvp_reduce_knomial_trace(int root, int mvp_reduce_knomial_factor,
                             MPIR_Comm *comm_ptr, int *expected_send_count,
                             int *expected_recv_count)
{
    int mask = 0x1, k, comm_size, rank, relative_rank, lroot = 0;
    int recv_iter = 0, send_iter = 0, dst;

    rank = comm_ptr->rank;
    comm_size = comm_ptr->local_size;

    lroot = root;
    relative_rank = (rank - lroot + comm_size) % comm_size;

    /* First compute to whom we need to send data */
    while (mask < comm_size) {
        if (relative_rank % (mvp_reduce_knomial_factor * mask)) {
            dst = relative_rank / (mvp_reduce_knomial_factor * mask) *
                      (mvp_reduce_knomial_factor * mask) +
                  root;
            if (dst >= comm_size) {
                dst -= comm_size;
            }
            send_iter++;
            break;
        }
        mask *= mvp_reduce_knomial_factor;
    }
    mask /= mvp_reduce_knomial_factor;

    /* Now compute how many children we have in the knomial-tree */
    while (mask > 0) {
        for (k = 1; k < mvp_reduce_knomial_factor; k++) {
            if (relative_rank + mask * k < comm_size) {
                recv_iter++;
            }
        }
        mask /= mvp_reduce_knomial_factor;
    }

    *expected_recv_count = recv_iter;
    *expected_send_count = send_iter;
    return 0;
}

shmem_info_t *mvp_shm_coll_init(int id, int local_rank, int local_size,
                                MPIR_Comm *comm_ptr)
{
    int slot_len, i, k;
    int mpi_errno = MPI_SUCCESS;
    int root = 0, expected_max_send_count = 0, expected_max_recv_count = 0;
    size_t size;
    const char *shmem_dir, *ptr;
    char s_hostname[SHMEM_COLL_HOSTNAME_LEN];
    struct stat file_status;
    shmem_info_t *shmem = NULL;
    int max_local_size = 0;
    MPIR_Errflag_t errflag = MPIR_ERR_NONE;
    MPIR_Comm *shmem_ptr = NULL;

    MPIR_Comm_get_ptr(comm_ptr->dev.ch.shmem_comm, shmem_ptr);
    if (shmem_ptr == NULL) {
        shmem_ptr = comm_ptr;
    }

    shmem = MPL_malloc(sizeof(shmem_info_t), MPL_MEM_COLL);
    MPIR_Assert(shmem != NULL);

    shmem->count = MVP_SHMEM_COLL_WINDOW_SIZE;
    shmem->write = 1;
    shmem->read = 1;
    shmem->tail = 0;
    shmem->file_fd = -1;
    shmem->file_name = NULL;
    shmem->size = 0;
    shmem->buffer = NULL;
    shmem->local_rank = local_rank;
    shmem->local_size = local_size;
    shmem->comm = comm_ptr->handle;
    shmem->max_local_size = 0;
#if defined(CHANNEL_MRAIL_GEN2)
    shmem->zcpy_coll_pending_send_ops = 0;
    shmem->defer_free = 0;
#endif
    slot_len = MVP_SHMEM_COLL_SLOT_LEN + sizeof(shm_slot_t) +
               sizeof(volatile uint32_t);

    MVP_SHM_ALIGN_LEN(slot_len, MVP_SHM_ALIGN)

    mpi_errno = mvp_reduce_knomial_trace(
        root, MVP_REDUCE_ZCOPY_INTER_KNOMIAL_FACTOR, comm_ptr,
        &expected_max_send_count, &expected_max_recv_count);
    if (local_size < expected_max_recv_count) {
        local_size = expected_max_recv_count;
    }

    /* Initialize max_local_size to local_size so that non-leader processes
     * get right value */
    max_local_size = local_size;
    mpi_errno = MPIR_Reduce_impl(&local_size, &max_local_size, 1, MPI_INT,
                                 MPI_MAX, 0, shmem_ptr, &errflag);
    if (mpi_errno) {
        MPIR_ERR_POP(mpi_errno);
    }

    size = (shmem->count) * slot_len * max_local_size;

    MVP_SHM_ALIGN_LEN(size, MVP_SHM_ALIGN);

    if (MVP_SHMEM_DIR != NULL) {
        shmem_dir = MVP_SHMEM_DIR;
    } else {
        shmem_dir = "/dev/shm";
    }

    if (gethostname(s_hostname, sizeof(char) * SHMEM_COLL_HOSTNAME_LEN) < 0) {
        PRINT_ERROR("gethostname filed\n");
    }
    shmem->file_name = (char *)MPL_malloc(
        sizeof(char) *
            (strlen(shmem_dir) + SHMEM_COLL_HOSTNAME_LEN + 26 + PID_CHAR_LEN),
        MPL_MEM_COLL);
    if (!shmem->file_name) {
        MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**nomem",
                                  "**nomem %s", "mvp_shmem_file");
    }

    /* TODO -- Remove UPMI calls here and replace with something that's
     * generic and will not be deprecated. Added here for now to avoid
     * weird issues in case there's a dependency.*/
    int kvs_name_sz;
    UPMI_KVS_GET_NAME_LENGTH_MAX(&kvs_name_sz);
    char *mvp_kvs_name = (char *)MPL_malloc(kvs_name_sz + 1, MPL_MEM_STRINGS);
    UPMI_KVS_GET_MY_NAME(mvp_kvs_name, kvs_name_sz);

    sprintf(shmem->file_name, "%s/slot_shmem-coll-%s-%s-%d-%d.tmp", shmem_dir,
            mvp_kvs_name, s_hostname, id, getuid());
    shmem->file_fd =
        open(shmem->file_name, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    if (shmem->file_fd < 0) {
        PRINT_ERROR("shmem open failed for file:%s\n", shmem->file_name);
        goto cleanup_slot_shmem;
    }

    if (local_rank == 0) {
        if (ftruncate(shmem->file_fd, 0)) {
            PRINT_ERROR("ftruncate failed file:%s\n", shmem->file_name);
            goto cleanup_slot_shmem;
        }
        if (ftruncate(shmem->file_fd, size)) {
            PRINT_ERROR("ftruncate failed file:%s\n", shmem->file_name);
            goto cleanup_slot_shmem;
        }
    }

    do {
        if (fstat(shmem->file_fd, &file_status) != 0) {
            PRINT_ERROR("fstat failed. file:%s\n", shmem->file_name);
            goto cleanup_slot_shmem;
        }
        usleep(1);
    } while (file_status.st_size < size);

    shmem->buffer = mmap(0, size, (PROT_READ | PROT_WRITE), (MAP_SHARED),
                         shmem->file_fd, 0);
    if (shmem->buffer == (void *)-1) {
        PRINT_ERROR("mmap failed. file:%s\n", shmem->file_name);
        goto cleanup_slot_shmem;
    }
    shmem->size = size;

    ptr = shmem->buffer;
    shmem->queue = (shm_queue_t *)MPL_malloc(
        sizeof(shm_queue_t *) * max_local_size, MPL_MEM_COLL);
    shmem->max_local_size = max_local_size;
    for (k = 0; k < max_local_size; k++) {
        shmem->queue[k].shm_slots = (shm_slot_t **)MPL_malloc(
            MVP_SHMEM_COLL_WINDOW_SIZE * sizeof(shm_slot_t *), MPL_MEM_COLL);
        for (i = 0; i < MVP_SHMEM_COLL_WINDOW_SIZE; i++) {
            shmem->queue[k].shm_slots[i] = (shm_slot_t *)ptr;
            ptr += slot_len;
        }
    }

#if defined(CHANNEL_MRAIL_GEN2) || defined(CHANNEL_NEMESIS_IB)
    shmem->bcast_remote_handle_info_parent = NULL;
    shmem->bcast_remote_handle_info_children = NULL;
    shmem->bcast_knomial_factor = -1;
    shmem->bcast_exchange_rdma_keys = 1;
    shmem->buffer_registered = 0;
    shmem->bcast_expected_send_count = 0;

    shmem->reduce_remote_handle_info_parent = NULL;
    shmem->reduce_remote_handle_info_children = NULL;
    shmem->inter_node_reduce_status_array = NULL;

    shmem->reduce_knomial_factor = -1;
    shmem->reduce_exchange_rdma_keys = 1;
    shmem->reduce_expected_send_count = 0;
    shmem->reduce_expected_recv_count = 0;
    shmem->mid_request_active = 0;
    shmem->end_request_active = 0;
    shmem->end_request = MPI_REQUEST_NULL;
    shmem->mid_request = MPI_REQUEST_NULL;
    shmem->half_full_complete = 0;

    for (k = 0; k < rdma_num_hcas; k++) {
        shmem->mem_handle[k] = NULL;
    }
#ifdef CKPT
    shmem->next = NULL;
#endif /* CKPT */
#endif /* defined(CHANNEL_MRAIL_GEN2) || defined(CHANNEL_NEMESIS_IB) */

    mvp_shm_barrier(shmem);

    PRINT_DEBUG(DEBUG_SHM_verbose > 0 && local_rank == 0,
                "Created shm file:%s size:%d \n", shmem->file_name,
                shmem->size);
    /* unlink the shmem file */
    if (shmem->file_name != NULL) {
        unlink(shmem->file_name);
    }

fn_exit:
    MPL_free(mvp_kvs_name);
    return shmem;
cleanup_slot_shmem:
    mvp_shm_coll_cleanup(shmem);
fn_fail:
    MPL_free(shmem);
    shmem = NULL;
    goto fn_exit;
}

#if defined(CHANNEL_MRAIL_GEN2)
void mvp_flush_all_zcpy_barrier_requests(shmem_info_t *shmem)
{
    int mpi_errno = MPI_SUCCESS;
    if (shmem->end_request_active) {
        mpi_errno = mvp_flush_zcpy_end_request(shmem);
        if (mpi_errno)
            PRINT_ERROR("MPIR_Wait_impl failure \n");
    }
    if (shmem->mid_request_active) {
        mpi_errno = mvp_flush_zcpy_mid_request(shmem);
        if (mpi_errno)
            PRINT_ERROR("MPIR_Wait_impl failure \n");
    }
}
#endif

void mvp_shm_coll_cleanup(shmem_info_t *shmem)
{
    int k;
    PRINT_DEBUG(DEBUG_SHM_verbose > 0, " Cleanup shmem file:%s fd:%d size:%d\n",
                shmem->file_name, shmem->file_fd, shmem->size);
    for (k = 0; k < shmem->max_local_size; k++) {
        MPL_free(shmem->queue[k].shm_slots);
    }

    MPL_free(shmem->queue);
#if defined(CHANNEL_MRAIL_GEN2) || defined(CHANNEL_NEMESIS_IB)
    /* Flush requests here only if not a deferred free */
    if (shmem->defer_free != 1) {
        mvp_flush_all_zcpy_barrier_requests(shmem);
    }

    if (shmem->local_rank == 0) {
        if (shmem->buffer_registered == 1) {
#ifdef CKPT
            PRINT_DEBUG(DEBUG_CR_verbose > 1,
                        "Cleaning up shmem=%p, prev=%p, next=%p\n", shmem,
                        shmem->prev, shmem->next);
            if (shmem->prev) {
                shmem->prev->next = shmem->next;
            }
            if (shmem->next) {
                shmem->next->prev = shmem->prev;
            }
            if (shmem == ckpt_free_head) {
                ckpt_free_head = shmem->next;
            }
#endif /* CKPT */
            mvp_shm_coll_dereg_buffer(shmem->mem_handle);
        }
        shmem->buffer_registered = 0;
    }
    if (shmem->bcast_remote_handle_info_parent != NULL) {
        MPL_free(shmem->bcast_remote_handle_info_parent);
    }
    if (shmem->bcast_remote_handle_info_children != NULL) {
        MPL_free(shmem->bcast_remote_handle_info_children);
    }
    if (shmem->reduce_remote_handle_info_parent != NULL) {
        MPL_free(shmem->reduce_remote_handle_info_parent);
    }
    if (shmem->reduce_remote_handle_info_children != NULL) {
        MPL_free(shmem->reduce_remote_handle_info_children);
    }
    if (shmem->inter_node_reduce_status_array != NULL) {
        MPL_free(shmem->inter_node_reduce_status_array);
    }
#endif /* defined(CHANNEL_MRAIL_GEN2) || defined(CHANNEL_NEMESIS_IB) */
    if (shmem->buffer != NULL) {
        munmap(shmem->buffer, shmem->size);
    }
    if (shmem->file_fd != -1) {
        close(shmem->file_fd);
    }
    if (shmem->file_name != NULL) {
        MPL_free(shmem->file_name);
        shmem->file_name = NULL;
    }
}

#ifdef CHANNEL_MRAIL_GEN2
int mvp_shm_zcpy_bcast(shmem_info_t *shmem, char *buf, int len, int root,
                       int src, int expected_recv_count, int *dst_array,
                       int expected_send_count, int knomial_degree,
                       MPIR_Comm *comm_ptr)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Errflag_t errflag = MPIR_ERR_NONE;
    int nspin = 0, i = 0, intra_node_root = 0;
    int windex = shmem->write % MVP_SHMEM_COLL_WINDOW_SIZE;
    int rindex = shmem->read % MVP_SHMEM_COLL_WINDOW_SIZE;
    MPIDI_VC_t *vc = NULL;
    MPIR_Comm *shmem_commptr = NULL, *leader_commptr = NULL;
    shm_coll_pkt pkt;
    shm_coll_pkt *remote_handle_info_parent =
        shmem->bcast_remote_handle_info_parent;
    shm_coll_pkt *remote_handle_info_children =
        shmem->bcast_remote_handle_info_children;

    MPIR_Comm_get_ptr(comm_ptr->dev.ch.shmem_comm, shmem_commptr);
    MPIR_Comm_get_ptr(comm_ptr->dev.ch.leader_comm, leader_commptr);

    if (shmem->buffer_registered == 0) {
        if (shmem_commptr->rank == 0 && leader_commptr->local_size > 1) {
#ifdef CKPT
            shmem->prev = NULL;
            shmem->next = NULL;
            if (ckpt_free_head) {
                ckpt_free_head->prev = shmem;
                shmem->next = ckpt_free_head;
            }
            ckpt_free_head = shmem;
            PRINT_DEBUG(DEBUG_CR_verbose > 1,
                        "Adding shmem region=%p, prev=%p, next=%p\n", shmem,
                        shmem->prev, shmem->next);
#endif /* CKPT */
            mpi_errno = mvp_shm_coll_reg_buffer(shmem->buffer, shmem->size,
                                                shmem->mem_handle,
                                                &(shmem->buffer_registered));
            if (mpi_errno) {
                MPIR_ERR_POP(mpi_errno);
            }
        }
    }

    if (shmem->local_rank == intra_node_root) {
        MPIR_Comm *leader_commptr = NULL;
        MPIR_Comm_get_ptr(comm_ptr->dev.ch.leader_comm, leader_commptr);

        if (leader_commptr->local_size > 1) {
            /*Exchange keys, (if needed) */
            if (shmem->bcast_exchange_rdma_keys == 1) {
                if (remote_handle_info_parent != NULL) {
                    MPL_free(remote_handle_info_parent);
                    remote_handle_info_parent = NULL;
                }
                if (remote_handle_info_children != NULL) {
                    MPL_free(remote_handle_info_children);
                    remote_handle_info_children = NULL;
                }
                if (expected_recv_count > 0) {
                    remote_handle_info_parent =
                        MPL_malloc(sizeof(shm_coll_pkt) * 1, MPL_MEM_COLL);
                    i = 0;
                    do {
                        pkt.key[i] = shmem->mem_handle[i]->rkey;
                        pkt.addr[i] = (uintptr_t)(shmem->buffer);
                        i++;
                    } while (i < rdma_num_hcas);

                    mpi_errno =
                        MPIC_Send(&pkt, sizeof(shm_coll_pkt), MPI_BYTE, src,
                                  MPIR_BCAST_TAG, leader_commptr, &errflag);
                    if (mpi_errno) {
                        MPIR_ERR_POP(mpi_errno);
                    }
                    remote_handle_info_parent->peer_rank = src;
                }

                if (expected_send_count > 0) {
                    remote_handle_info_children =
                        MPL_malloc(sizeof(shm_coll_pkt) * expected_send_count,
                                   MPL_MEM_COLL);
                }

                for (i = 0; i < expected_send_count; i++) {
                    /* Sending to dst. Receive its key info */
                    int j = 0;
                    mpi_errno =
                        MPIC_Recv(&pkt, sizeof(shm_coll_pkt), MPI_BYTE,
                                  dst_array[i], MPIR_BCAST_TAG, leader_commptr,
                                  MPI_STATUS_IGNORE, &errflag);
                    if (mpi_errno) {
                        MPIR_ERR_POP(mpi_errno);
                    }

                    do {
                        remote_handle_info_children[i].key[j] = pkt.key[j];
                        remote_handle_info_children[i].addr[j] = pkt.addr[j];
                        j++;
                    } while (j < rdma_num_hcas);
                    remote_handle_info_children[i].peer_rank = dst_array[i];
                }

                /* Trac #657: Wait till VC is ready to send */
                for (i = 0; i < expected_send_count; i++) {
                    MPIDI_Comm_get_vc(leader_commptr,
                                      remote_handle_info_children[i].peer_rank,
                                      &vc);
                    while (VC_NOT_READY(vc)) {
                        /* Yield CPU and wait for CM thread to complete
                         * connection */
                        MPL_sched_yield();
                    }
                }

                shmem->bcast_exchange_rdma_keys = 0;
                shmem->bcast_knomial_factor = knomial_degree;
                shmem->bcast_remote_handle_info_parent =
                    remote_handle_info_parent;
                shmem->bcast_remote_handle_info_children =
                    remote_handle_info_children;
                shmem->bcast_expected_send_count = expected_send_count;
            }

            /********************************************
             * the RDMA keys for the shmem buffer has been exchanged
             * We are ready to move the data around
             **************************************/
            if (leader_commptr->rank != root) {
                /* node-level leader, but not the root of the bcast */
                shmem->queue[intra_node_root].shm_slots[windex]->tail_psn =
                    (volatile uint32_t *)((char *)(shmem->queue[intra_node_root]
                                                       .shm_slots[windex]
                                                       ->buf) +
                                          len);
                /* Wait until the peer is yet to update the psn flag and
                 * until the psn and the tail flags have the same values*/
                while (shmem->write !=
                       (volatile uint32_t) * (shmem->queue[intra_node_root]
                                                  .shm_slots[windex]
                                                  ->tail_psn)) {
                    mvp_shm_progress(&nspin);
                }
                WRITEBAR();
                shmem->queue[intra_node_root].shm_slots[windex]->psn =
                    shmem->write;
            } else {
                /* node-level leader, and the root of the bcast */
#if defined(_ENABLE_CUDA_)
                if (mvp_enable_device) {
                    MPIR_Localcopy(
                        buf, len, MPI_BYTE,
                        shmem->queue[intra_node_root].shm_slots[windex]->buf,
                        len, MPI_BYTE);
                } else
#endif
                {

                    MPIR_Memcpy(
                        shmem->queue[intra_node_root].shm_slots[windex]->buf,
                        buf, len);
                }
                WRITEBAR();
                shmem->queue[intra_node_root].shm_slots[windex]->psn =
                    shmem->write;
                WRITEBAR();
                shmem->queue[intra_node_root].shm_slots[windex]->tail_psn =
                    (volatile uint32_t *)((char *)(shmem->queue[intra_node_root]
                                                       .shm_slots[windex]
                                                       ->buf) +
                                          len);
                *((volatile uint32_t *)shmem->queue[intra_node_root]
                      .shm_slots[windex]
                      ->tail_psn) = shmem->write;
            }

            /* Increment number of pending send ops by expected send count */
            shmem->zcpy_coll_pending_send_ops +=
                shmem->bcast_expected_send_count;
            /* Post the rdma-writes to all the children in the tree */
            for (i = 0; i < shmem->bcast_expected_send_count; i++) {
                uint32_t local_rdma_key, remote_rdma_key;
                uint64_t local_rdma_addr, remote_rdma_addr, offset;
                int rail = 0, hca_num;

                MPIDI_Comm_get_vc(leader_commptr,
                                  remote_handle_info_children[i].peer_rank,
                                  &vc);
                offset = ((uintptr_t)(shmem->queue[intra_node_root]
                                          .shm_slots[windex]
                                          ->buf) -
                          (uintptr_t)(shmem->buffer));
#ifdef CHANNEL_NEMESIS_IB
                rail = MPIDI_nem_ib_send_select_rail(vc);
#else
                rail = MRAILI_Send_select_rail(vc, len);
#endif
                hca_num = rail / (mvp_num_rails / rdma_num_hcas);

                local_rdma_addr = (uint64_t)(shmem->queue[intra_node_root]
                                                 .shm_slots[windex]
                                                 ->buf);
                local_rdma_key = (shmem->mem_handle[hca_num])->lkey;
                remote_rdma_addr =
                    (uint64_t)remote_handle_info_children[i].addr[hca_num] +
                    offset;
                remote_rdma_key = remote_handle_info_children[i].key[hca_num];

                mvp_shm_coll_prepare_post_send(
                    (void *)shmem, local_rdma_addr, remote_rdma_addr,
                    local_rdma_key, remote_rdma_key,
                    len + sizeof(volatile uint32_t), rail, vc);
            }

            /* If a node-leader is not root, we finally copy the data back into
             * the user-level buffer */
            if (leader_commptr->rank != root) {
#if defined(_ENABLE_CUDA_)
                if (mvp_enable_device) {
                    mpi_errno = MPIR_Localcopy(
                        shmem->queue[intra_node_root].shm_slots[windex]->buf,
                        len, MPI_BYTE, buf, len, MPI_BYTE);
                    if (mpi_errno) {
                        MPIR_ERR_POP(mpi_errno);
                    }
                } else
#endif
                {
                    MPIR_Memcpy(
                        buf,
                        shmem->queue[intra_node_root].shm_slots[windex]->buf,
                        len);
                }
            }
        } else {
#if defined(_ENABLE_CUDA_)
            if (mvp_enable_device) {
                mpi_errno = MPIR_Localcopy(
                    buf, len, MPI_BYTE,
                    shmem->queue[intra_node_root].shm_slots[windex]->buf, len,
                    MPI_BYTE);
                if (mpi_errno) {
                    MPIR_ERR_POP(mpi_errno);
                }
            } else
#endif
            {
                MPIR_Memcpy(
                    shmem->queue[intra_node_root].shm_slots[windex]->buf, buf,
                    len);
            }
            WRITEBAR();
            shmem->queue[intra_node_root].shm_slots[windex]->psn = shmem->write;
        }
    } else {
        while (shmem->queue[intra_node_root].shm_slots[rindex]->psn !=
               shmem->read) {
            nspin++;
            if (nspin % MVP_SHMEM_COLL_SPIN_COUNT == 0) {
                mvp_shm_progress(&nspin);
            }
        }
        READBAR();
#if defined(_ENABLE_CUDA_)
        if (mvp_enable_device) {
            mpi_errno = MPIR_Localcopy(
                shmem->queue[intra_node_root].shm_slots[rindex]->buf, len,
                MPI_BYTE, buf, len, MPI_BYTE);
            if (mpi_errno) {
                MPIR_ERR_POP(mpi_errno);
            }
        } else
#endif
        {
            MPIR_Memcpy(
                buf, shmem->queue[intra_node_root].shm_slots[rindex]->buf, len);
        }
    }
    shmem->write++;
    shmem->read++;

    if (shmem->half_full_complete == 0 &&
        IS_SHMEM_WINDOW_HALF_FULL(shmem->write, shmem->tail)) {
        PRINT_DEBUG(DEBUG_SHM_verbose > 1, "shmem window half full: %llu \n",
                    shmem->write);
        mvp_shm_barrier(shmem);
        if (shmem->local_rank == intra_node_root &&
            leader_commptr->local_size > 1) {
            if (shmem->end_request_active == 1) {
                mpi_errno = mvp_flush_zcpy_end_request(shmem);
                if (mpi_errno)
                    MPIR_ERR_POP(mpi_errno);
            }

            if (shmem->mid_request_active == 0) {
                mpi_errno = mvp_post_zcpy_mid_request(leader_commptr, shmem);
                if (mpi_errno)
                    MPIR_ERR_POP(mpi_errno);
            }
        }
        shmem->half_full_complete = 1;
    }

    if (IS_SHMEM_WINDOW_FULL(shmem->write, shmem->tail)) {
        PRINT_DEBUG(DEBUG_SHM_verbose > 1, "shmem window full: %llu \n",
                    shmem->write);
        mvp_shm_barrier(shmem);
        if (shmem->local_rank == intra_node_root &&
            leader_commptr->local_size > 1) {
            if (shmem->mid_request_active == 1) {
                mpi_errno = mvp_flush_zcpy_mid_request(shmem);
                if (mpi_errno)
                    MPIR_ERR_POP(mpi_errno);
            }

            if (shmem->end_request_active == 0) {
                mpi_errno = mvp_post_zcpy_end_request(leader_commptr, shmem);
                if (mpi_errno)
                    MPIR_ERR_POP(mpi_errno);
            }
        }
        shmem->tail = shmem->read;
        shmem->half_full_complete = 0;
    }

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int mvp_shm_zcpy_reduce(shmem_info_t *shmem, void *in_buf, void **out_buf,
                        int count, int len, MPI_Datatype datatype, MPI_Op op,
                        int root, int expected_recv_count, int *src_array,
                        int expected_send_count, int dst, int knomial_degree,
                        MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    void *buf = NULL;
    int mpi_errno = MPI_SUCCESS;
    int nspin = 0, i = 0, j = 0, intra_node_root = 0;
    int windex = 0, rindex = 0;
    MPIDI_VC_t *vc = NULL;
    MPIR_Comm *shmem_commptr = NULL, *leader_commptr = NULL;
    shm_coll_pkt pkt;
    shm_coll_pkt *remote_handle_info_parent =
        shmem->reduce_remote_handle_info_parent;
    shm_coll_pkt *remote_handle_info_children =
        shmem->reduce_remote_handle_info_children;
    int *inter_node_reduce_status_array = shmem->inter_node_reduce_status_array;
    MPI_User_function *uop;
    MPIR_Op *op_ptr;
    int slot_len;
    slot_len = MVP_SHMEM_COLL_SLOT_LEN + sizeof(shm_slot_t) +
               sizeof(volatile uint32_t);
    MVP_SHM_ALIGN_LEN(slot_len, MVP_SHM_ALIGN);
    int is_cxx_uop = 0;
    int shmem_comm_rank, local_size, local_rank;

    MPIR_Comm_get_ptr(comm_ptr->dev.ch.shmem_comm, shmem_commptr);
    MPIR_Comm_get_ptr(comm_ptr->dev.ch.leader_comm, leader_commptr);

    shmem_comm_rank = shmem_commptr->dev.ch.shmem_comm_rank;
    local_rank = shmem_commptr->rank;
    local_size = shmem_commptr->local_size;

    windex = shmem->write % MVP_SHMEM_COLL_WINDOW_SIZE;
    rindex = shmem->read % MVP_SHMEM_COLL_WINDOW_SIZE;

    if (shmem->half_full_complete == 0 &&
        (IS_SHMEM_WINDOW_HALF_FULL(shmem->write, shmem->tail))) {
        PRINT_DEBUG(DEBUG_SHM_verbose > 1, "shmem window half full: %llu \n",
                    shmem->write);

        if (shmem->local_rank == intra_node_root &&
            leader_commptr->local_size > 1) {
            if (shmem->mid_request_active == 0) {
                mpi_errno = mvp_post_zcpy_mid_request(leader_commptr, shmem);
                if (mpi_errno)
                    MPIR_ERR_POP(mpi_errno);
            }

            if (shmem->end_request_active == 1) {
                mpi_errno = mvp_flush_zcpy_end_request(shmem);
                if (mpi_errno)
                    MPIR_ERR_POP(mpi_errno);
            }
        }
        shmem->write++;
        shmem->read++;
        shmem->half_full_complete = 1;
        windex = shmem->write % MVP_SHMEM_COLL_WINDOW_SIZE;
        rindex = shmem->read % MVP_SHMEM_COLL_WINDOW_SIZE;
    }

    if (IS_SHMEM_WINDOW_FULL(shmem->write, shmem->tail) ||
        ((MVP_SHMEM_COLL_WINDOW_SIZE - windex < 2) ||
         (MVP_SHMEM_COLL_WINDOW_SIZE - rindex < 2))) {
        PRINT_DEBUG(DEBUG_SHM_verbose > 1, "shmem window full: %llu \n",
                    shmem->write);
        if (shmem->local_size > 1) {
            MPIR_MVP_SHMEM_COLL_Barrier_gather(local_size, local_rank,
                                               shmem_comm_rank);
        }

        if (shmem->local_rank == intra_node_root &&
            leader_commptr->local_size > 1) {
            if (shmem->end_request_active == 0) {
                mpi_errno = mvp_post_zcpy_end_request(leader_commptr, shmem);
                if (mpi_errno)
                    MPIR_ERR_POP(mpi_errno);
            }

            if (shmem->mid_request_active == 1) {
                mpi_errno = mvp_flush_zcpy_mid_request(shmem);
                if (mpi_errno)
                    MPIR_ERR_POP(mpi_errno);
            }
        }

        if (shmem->local_size > 1) {
            MPIR_MVP_SHMEM_COLL_Barrier_bcast(local_size, local_rank,
                                              shmem_comm_rank);
        }
        shmem->half_full_complete = 0;
        shmem->write++;
        shmem->read++;
        shmem->tail = shmem->read;
        windex = shmem->write % MVP_SHMEM_COLL_WINDOW_SIZE;
        rindex = shmem->read % MVP_SHMEM_COLL_WINDOW_SIZE;
    }

    /* Get the operator and check whether it is commutative or not */
    if (HANDLE_GET_KIND(op) == HANDLE_KIND_BUILTIN) {
        /* get the function by indexing into the op table */
        uop = MPIR_OP_HDL_TO_FN(op);
    } else {
        MPIR_Op_get_ptr(op, op_ptr);
#if defined(HAVE_CXX_BINDING)
        if (op_ptr->language == MPIR_LANG__CXX) {
            uop = (MPI_User_function *)op_ptr->function.c_function;
            is_cxx_uop = 1;
        } else {
#endif /* defined(HAVE_CXX_BINDING) */
            if (op_ptr->language == MPIR_LANG__C) {
                uop = (MPI_User_function *)op_ptr->function.c_function;
            } else {
                uop = (MPI_User_function *)op_ptr->function.f77_function;
            }
#if defined(HAVE_CXX_BINDING)
        }
#endif /* defined(HAVE_CXX_BINDING) */
    }

    if (shmem->local_rank == intra_node_root) {
        MPIR_Comm *leader_commptr = NULL;
        int inter_node_reduce_completions = 0;
        MPIR_Comm_get_ptr(comm_ptr->dev.ch.leader_comm, leader_commptr);

        if (leader_commptr->local_size > 1) {
            if (shmem->buffer_registered == 0) {
#ifdef CKPT
                shmem->prev = NULL;
                shmem->next = NULL;
                if (ckpt_free_head) {
                    ckpt_free_head->prev = shmem;
                    shmem->next = ckpt_free_head;
                }
                ckpt_free_head = shmem;
                PRINT_DEBUG(DEBUG_CR_verbose > 1,
                            "Adding shmem region=%p, prev=%p, next=%p\n", shmem,
                            shmem->prev, shmem->next);
#endif /* CKPT */

                mpi_errno = mvp_shm_coll_reg_buffer(
                    shmem->buffer, shmem->size, shmem->mem_handle,
                    &(shmem->buffer_registered));
                if (mpi_errno) {
                    MPIR_ERR_POP(mpi_errno);
                }
            }

            /*Exchange keys, (if needed) */
            if (shmem->reduce_exchange_rdma_keys == 1) {
                if (remote_handle_info_parent != NULL) {
                    MPL_free(remote_handle_info_parent);
                    remote_handle_info_parent = NULL;
                }
                if (remote_handle_info_children != NULL) {
                    MPL_free(remote_handle_info_children);
                    remote_handle_info_children = NULL;
                }
                if (shmem->inter_node_reduce_status_array != NULL) {
                    MPL_free(shmem->inter_node_reduce_status_array);
                    shmem->inter_node_reduce_status_array = NULL;
                }

                if (expected_send_count > 0) {
                    MPI_Status status;
                    remote_handle_info_parent =
                        MPL_malloc(sizeof(shm_coll_pkt) * expected_send_count,
                                   MPL_MEM_COLL);
                    if (remote_handle_info_parent == NULL) {
                        MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER,
                                                  "**nomem", "**nomem %s",
                                                  "mvp_shmem_file");
                    }
                    j = 0;

                    /* I am sending data to "dst". I will be needing dst's
                     * RDMA info */
                    mpi_errno = MPIC_Recv(&pkt, sizeof(shm_coll_pkt), MPI_BYTE,
                                          dst, MPIR_REDUCE_TAG, leader_commptr,
                                          &status, errflag);
                    if (mpi_errno) {
                        MPIR_ERR_POP(mpi_errno);
                    }

                    do {
                        remote_handle_info_parent->key[j] = pkt.key[j];
                        remote_handle_info_parent->addr[j] = pkt.addr[j];
                        j++;
                    } while (j < rdma_num_hcas);
                    remote_handle_info_parent->peer_rank = dst;
                    remote_handle_info_parent->recv_id = pkt.recv_id;

                    /* Trac #657: Wait till VC is ready to send */
                    MPIDI_Comm_get_vc(leader_commptr,
                                      remote_handle_info_parent->peer_rank,
                                      &vc);
                    while (VC_NOT_READY(vc)) {
                        /* Yield CPU and wait for CM thread to complete
                         * connection */
                        MPL_sched_yield();
                    }
                }

                if (expected_recv_count > 0) {
                    int j = 0;
                    MPIR_Request **request_array = NULL;
                    MPI_Status *status_array = NULL;
                    shm_coll_pkt *pkt_array = NULL;
                    remote_handle_info_children =
                        MPL_malloc(sizeof(shm_coll_pkt) * expected_recv_count,
                                   MPL_MEM_COLL);

                    pkt_array =
                        MPL_malloc(sizeof(shm_coll_pkt) * expected_recv_count,
                                   MPL_MEM_COLL);
                    request_array = (MPIR_Request **)MPL_malloc(
                        sizeof(MPIR_Request *) * expected_recv_count,
                        MPL_MEM_COLL);
                    status_array = MPL_malloc(
                        sizeof(MPI_Status) * expected_recv_count, MPL_MEM_COLL);

                    if (pkt_array == NULL || request_array == NULL ||
                        status_array == NULL ||
                        remote_handle_info_children == NULL) {
                        MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER,
                                                  "**nomem", "**nomem %s",
                                                  "mvp_shmem_file");
                    }

                    for (i = 0; i < expected_recv_count; i++) {
                        j = 0;
                        do {
                            pkt_array[i].key[j] = shmem->mem_handle[j]->rkey;
                            pkt_array[i].addr[j] = (uintptr_t)(shmem->buffer);
                            j++;
                        } while (j < rdma_num_hcas);
                    }

                    for (i = 0; i < expected_recv_count; i++) {
                        int src = -1;
                        /* Receiving data from src. Send my key info to src*/
                        src = src_array[i];
                        pkt_array[i].recv_id = i;
                        mpi_errno = MPIC_Isend(
                            &pkt_array[i], sizeof(shm_coll_pkt), MPI_BYTE, src,
                            MPIR_REDUCE_TAG, leader_commptr, &request_array[i],
                            errflag);
                        if (mpi_errno) {
                            MPIR_ERR_POP(mpi_errno);
                        }
                        remote_handle_info_children[i].peer_rank = src_array[i];
                    }
                    mpi_errno = MPIC_Waitall(expected_recv_count, request_array,
                                             status_array, errflag);
                    if (mpi_errno) {
                        MPIR_ERR_POP(mpi_errno);
                    }
                    MPL_free(request_array);
                    MPL_free(status_array);
                    MPL_free(pkt_array);
                }
                shmem->reduce_exchange_rdma_keys = 0;
                shmem->reduce_knomial_factor = knomial_degree;
                shmem->reduce_remote_handle_info_parent =
                    remote_handle_info_parent;
                shmem->reduce_remote_handle_info_children =
                    remote_handle_info_children;
                shmem->reduce_expected_recv_count = expected_recv_count;
                shmem->reduce_expected_send_count = expected_send_count;
                if (shmem->reduce_expected_recv_count > 0) {
                    inter_node_reduce_status_array = MPL_malloc(
                        sizeof(int) * shmem->reduce_expected_recv_count,
                        MPL_MEM_COLL);
                    if (inter_node_reduce_status_array == NULL) {
                        MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER,
                                                  "**nomem", "**nomem %s",
                                                  "mvp_shmem_file");
                    }
                    MPIR_Memset(inter_node_reduce_status_array, '0',
                                sizeof(int) *
                                    shmem->reduce_expected_recv_count);
                    shmem->inter_node_reduce_status_array =
                        inter_node_reduce_status_array;
                }
                if (src_array != NULL) {
                    MPL_free(src_array);
                }
            }
        }

        /********************************************
         * the RDMA keys for the shmem buffer has been exchanged
         * We are ready to move the data around
         **************************************/

        /* Lets start with intra-node, shmem, slot-based reduce*/

        buf = shmem->queue[shmem->local_rank].shm_slots[windex]->buf;

        /* Now, do the intra-node shm-slot reduction */
        mvp_shm_tree_reduce(shmem, in_buf, len, count, intra_node_root, uop,
                            datatype, is_cxx_uop);

        WRITEBAR();
        shmem->queue[shmem->local_rank].shm_slots[windex]->psn = shmem->write;
        WRITEBAR();
        shmem->queue[shmem->local_rank].shm_slots[windex]->tail_psn =
            (volatile uint32_t *)((char *)(shmem->queue[shmem->local_rank]
                                               .shm_slots[windex]
                                               ->buf) +
                                  len);
        *((volatile uint32_t *)shmem->queue[shmem->local_rank]
              .shm_slots[windex]
              ->tail_psn) = shmem->write;

        /* I am my node's leader, if I am an intermediate process in the  tree,
         * I need to wait until my inter-node peers have written their data
         */

        while (inter_node_reduce_completions <
               shmem->reduce_expected_recv_count) {
            for (i = 0; i < shmem->reduce_expected_recv_count; i++) {
                /* Wait until the peer is yet to update the psn flag and
                 * until the psn and the tail flags have the same values*/
                if (inter_node_reduce_status_array[i] != 1) {
                    shmem->queue[i].shm_slots[windex + 1]->tail_psn =
                        (volatile uint32_t
                             *)((char *)(shmem->queue[i]
                                             .shm_slots[windex + 1]
                                             ->buf) +
                                len);
                    if (shmem->write ==
                        (volatile uint32_t) *
                            (shmem->queue[i].shm_slots[windex + 1]->tail_psn)) {
#ifdef HAVE_CXX_BINDING
                        if (is_cxx_uop) {
                            (*MPIR_Process.cxx_call_op_fn)(
                                shmem->queue[i].shm_slots[rindex + 1]->buf, buf,
                                count, datatype, uop);
                        } else
#endif
                        {
                            (*uop)(shmem->queue[i].shm_slots[(rindex + 1)]->buf,
                                   buf, &count, &datatype);
                        }
                        inter_node_reduce_completions++;
                        inter_node_reduce_status_array[i] = 1;
                    }
                }
            }
            if (nspin % MVP_SHMEM_COLL_SPIN_COUNT == 0) {
                mvp_shm_progress(&nspin);
            }
            nspin++;
        }

        /* Increment number of pending send ops by expected send count */
        shmem->zcpy_coll_pending_send_ops += shmem->reduce_expected_send_count;
        /* Post the rdma-write to the parent in the tree */
        if (shmem->reduce_expected_send_count > 0) {
            uint32_t local_rdma_key, remote_rdma_key;
            uint64_t local_rdma_addr, remote_rdma_addr, offset;
            int rail = 0, row_id = 0, hca_num;

            MPIDI_Comm_get_vc(leader_commptr,
                              remote_handle_info_parent->peer_rank, &vc);

            row_id = remote_handle_info_parent->recv_id;
            offset = (slot_len * MVP_SHMEM_COLL_WINDOW_SIZE) * (row_id) +
                     slot_len * (windex + 1) + sizeof(shm_slot_cntrl_t);
#ifdef CHANNEL_NEMESIS_IB
            rail = MPIDI_nem_ib_send_select_rail(vc);
#else
            rail = MRAILI_Send_select_rail(vc, len);
#endif
            hca_num = rail / (mvp_num_rails / rdma_num_hcas);

            local_rdma_addr = (uint64_t)(shmem->queue[intra_node_root]
                                             .shm_slots[windex]
                                             ->buf);
            local_rdma_key = (shmem->mem_handle[hca_num])->lkey;
            remote_rdma_addr =
                (uint64_t)remote_handle_info_parent->addr[hca_num] + offset;
            remote_rdma_key = remote_handle_info_parent->key[hca_num];

            mvp_shm_coll_prepare_post_send(
                (void *)shmem, local_rdma_addr, remote_rdma_addr,
                local_rdma_key, remote_rdma_key,
                len + sizeof(volatile uint32_t), rail, vc);
        }
        if (shmem->reduce_expected_recv_count > 0) {
            MPIR_Memset(inter_node_reduce_status_array, '0',
                        sizeof(int) * shmem->reduce_expected_recv_count);
        }
    } else {
        mvp_shm_tree_reduce(shmem, in_buf, len, count, intra_node_root, uop,
                            datatype, is_cxx_uop);
        WRITEBAR();
        shmem->queue[shmem->local_rank].shm_slots[windex]->psn = shmem->write;
    }

    *out_buf = buf;
    shmem->write = shmem->write + 2;
    shmem->read = shmem->read + 2;

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
#endif /* CHANNEL_MRAIL_GEN2 */
