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

#include "mpichconf.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <infiniband/verbs.h>
#include "rdma_impl.h"
#include "vbuf.h"
#include "ibv_param.h"
#include "sysreport.h"
#include "smp_smpi.h"
#include "mvp_utils.h"
#include "upmi.h"
#include "ibv_mcast.h"
#include <inttypes.h>
#ifdef HAVE_ROMIO
#include "romioconf.h"
#endif
#include "mvp_ch3_shmem.h"

/*
=== BEGIN_MPI_T_MVP_CVAR_INFO_BLOCK ===

categories:
    - name        : CH3
      description : INHERITED

    - name        : COMMON
      description : INHERITED

cvars:
    - name        : MVP_SMP_EAGERSIZE
      category    : CH3
      type        : int
      default     : 8192
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This parameter defines the size of shared buffer between every two
        processes on the same node for transferring messages smaller than or
        equal to MVP_SMP_EAGERSIZE. Note that this variable can be set with
        suffixes such as 'K'/'k', 'M'/'m' or 'G'/'g' to denote Kilobyte,
        Megabyte or Gigabyte respectively.

    - name        : MVP_IBA_HCA
      category    : CH3
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This specifies the HCA's to be used for performing network operations.

    - name        : MVP_SMP_USE_CMA
      category    : CH3
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This parameter enables/disables CMA based intra-node communication at
        run time. It does not take effect if MVAPICH is configured with
        -without-cma. When -with-limic2 is included in the configure flags,
        LiMIC2 is used in preference over CMA. Please set MVP_SMP_USE_LIMIC2
        to 0 in order to choose CMA if MVAPICH is configured with -with-limic2.

    - name        : MVP_SMP_USE_LIMIC2
      category    : CH3
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This parameter enables/disables LiMIC2 at run time. It does not take
        effect if MVAPICH is not configured with -with-limic2.

        Not used currently.

    - name        : MVP_SMP_POLLING_THRESHOLD
      category    : CH3
      type        : int
      default     : 200
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_SMP_CMA_MAX_SIZE
      category    : CH3
      type        : int
      default     : 4194304
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_SMP_LIMIC2_MAX_SIZE
      category    : CH3
      type        : int
      default     : 4194304
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_SMP_QUEUE_LENGTH
      category    : CH3
      type        : int
      default     : 262144
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This parameter defines the size of the shared buffer between
        every two processes on the same node for transferring intra-node
        messages smaller than or equal to MVP_SMP_EAGERSIZE. This variable
        can be set with suffixes suck as 'K'/'k', 'M'/'m', or 'G'/'g' to
        denote Kilobyte, Megabyte, or Gigabyte respectively.

    - name        : MVP_LIMIC_PUT_THRESHOLD
      category    : CH3
      type        : int
      default     : (8 * 1024)
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_LIMIC_GET_THRESHOLD
      category    : CH3
      type        : int
      default     : (8 * 1024)
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_R3_THRESHOLD
      category    : CH3
      type        : int
      default     : 4096
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        The value of this variable controls what message sizes go over
        the R3 rendezvous protocol. Messages above this message size use
        MVP_RNDV_PROTOCOL.

    - name        : MVP_INTRA_NODE_R3_THRESHOLD
      category    : CH3
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_INTER_NODE_R3_THRESHOLD
      category    : CH3
      type        : int
      default     : 4096
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_R3_NOCACHE_THRESHOLD
      category    : CH3
      type        : int
      default     : 32768
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        The value of this variable controls what message sizes go over the R3
        rendezvous protocol when the registration cache is disabled
        (MVP_USE_LAZY_MEM_UNREGISTER=0). Messages above this message size use
        MVP_RNDV_PROTOCOL.

    - name        : MVP_MAX_R3_PENDING_DATA
      category    : CH3
      type        : int
      default     : (512 * 1024)
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_LIMIC2_COLL
      category    : CH3
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_SMP_USE_MAX_SWITCH
      category    : CH3
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_CUDA_SMP_PIPELINE
      category    : CH3
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_SMP_DELAY_SHMEM_POOL_INIT
      category    : CH3
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_SMP_PRIORITY_POLLING
      category    : CH3
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_SMP_NUM_SEND_BUFFER
      category    : CH3
      type        : int
      default     : 128
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This parameter defines the number of internal send buffers for sending
        intra-node messages larger than MVP_SMP_EAGERSIZE.

    - name        : MVP_SMP_BATCH_SIZE
      category    : CH3
      type        : int
      default     : 8
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_SMP_SEND_BUF_SIZE
      category    : CH3
      type        : int
      default     : 8192
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This parameter defines the packet size when sending intra-node messages
        larger than MVP_SMP_EAGERSIZE.

    - name        : MVP_USE_OPT_EAGER_RECV
      category    : CH3
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_NUM_NODES_IN_JOB
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_MCAST
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Set this to 1, to enable hardware multicast support in collective
        communication.

    - name        : MVP_USE_RDMA_CM_MCAST
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This parameter enables support for RDMA_CM based multicast group setup.
        Requires the parameter MVP_USE_MCAST to be set to 1.

    - name        : MVP_USE_QOS
      category    : CH3
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_3DTORUS_SUPPORT
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_PATH_SL_QUERY
      category    : CH3
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_NUM_SLS
      category    : CH3
      type        : int
      default     : 8
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_NUM_SA_QUERY_RETRIES
      category    : CH3
      type        : int
      default     : 20
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Number of times the MPI library will attempt to query the subnet to
        obtain the path record information before giving up.

    - name        : MVP_MAX_RDMA_CONNECT_ATTEMPTS
      category    : CH3
      type        : int
      default     : 20
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_RDMA_CM_CONNECT_RETRY_INTERVAL
      category    : CH3
      type        : int
      default     : 100
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_HSAM
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This parameter is used for utilizing hot-spot avoidance with InfiniBand
        clusters. To leverage this functionality, the subnet should be
        configured with lmc greater than zero.

    - name        : MVP_USE_APM
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This parameter is used for recovery from network faults using Automatic
        Path Migration. This functionality is beneficial in the presence of
        multiple paths in the network, which can be enabled by using lmc
        mechanism.

    - name        : MVP_USE_APM_TEST
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This parameter is used for testing the Automatic Path Migration
        functionality. It periodically moves the alternate path as the primary
        path of communication and re-loads another alternate path.

    - name        : MVP_USE_IWARP_MODE
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This parameter enables the library to run in iWARP mode.

    - name        : MVP_USE_RDMA_CM
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This parameter enables the use of RDMA CM for establishing the
        connections.

    - name        : MVP_RDMA_CM_MULTI_SUBNET_SUPPORT
      category    : CH3
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This parameter allows MPI jobs to be run across multiple subnets
        interconnected by InfiniBand routers. Note that this requires RDMA_CM
        support to be enabled at configure time and runtime. Note that, RDMA_CM
        support is enabled by default at configure time.

    - name        : MVP_ON_DEMAND_THRESHOLD
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This defines threshold for enabling on-demand connection management
        scheme. When the size of the job is larger than the threshold value,
        on-demand connection management will be used.

    - name        : MVP_USE_XRC
      category    : CH3
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Use the XRC InfiniBand transport available since Mellanox ConnectX
        adapters. This features requires OFED version later than 1.3. It also
        automatically enables SRQ and ON-DEMAND connection management. Note
        that the MVAPICH library needs to have been configured with
        -enable-xrc=yes to use this feature.

    - name        : MVP_USE_SRQ
      category    : CH3
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Setting this parameter enables MVAPICH to use shared receive queue.

    - name        : MVP_IWARP_MULTIPLE_CQ_THRESHOLD
      category    : CH3
      type        : int
      default     : 32
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This defines the process size beyond which we use multiple completion
        queues for iWARP interface.

    - name        : MVP_USE_LAZY_MEM_UNREGISTER
      category    : CH3
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Setting this parameter enables MVAPICH to use memory registration cache.

    - name        : MVP_USE_RMA_FAST_PATH
      category    : CH3
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_FORCE_IB_ATOMIC
      category    : CH3
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_USE_RDMA_ONE_SIDED
      category    : CH3
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Setting this parameter allows MVAPICH to use optimized one sided
        implementation based RDMA operations.

    - name        : MVP_RNDV_EXT_SENDQ_SIZE
      category    : CH3
      type        : int
      default     : 5
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_RDMA_NUM_EXTRA_POLLS
      category    : CH3
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_COALESCE_THRESHOLD
      category    : CH3
      type        : int
      default     : 6
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
         This parameter determines the threshold for message coalescing.

    - name        : MVP_USE_COALESCE
      category    : CH3
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Setting this parameter enables message coalescing to increase small
        message throughput.

    - name        : MVP_SPIN_COUNT
      category    : CH3
      type        : int
      default     : 5000
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_PROCESS_TO_RAIL_MAPPING
      category    : CH3
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        When MVP_RAIL_SHARING_POLICY is set to the value "FIXED_MAPPING" this
        variable decides the manner in which the HCAs will be mapped to the
        rails. The <CUSTOM LIST> is colon(:) separated list with the HCA ranks
        specified. e.g. 0:1:1:0. This list must map equally to the number of
        local processes on the nodes failing which, the default policy will be
        used. Similarly the number of processes on each node must be the same.

    - name        : MVP_SM_SCHEDULING
      category    : CH3
      type        : string
      default     : USE_FIRST
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_SMALL_MSG_RAIL_SHARING_POLICY
      category    : CH3
      type        : string
      default     : ROUND_ROBIN
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This specifies the policy that will be used to assign HCAs to each of
        the processes with small message sizes.

    - name        : MVP_MED_MSG_RAIL_SHARING_POLICY
      category    : CH3
      type        : string
      default     : ROUND_ROBIN
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This specifies the policy that will be used to assign HCAs to each of
        the processes with medium message sizes.

    - name        : MVP_RAIL_SHARING_POLICY
      category    : CH3
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This specifies the policy that will be used to assign HCAs to each of
        the processes. In the previous versions of MVAPICH it was known as
        MVP_SM_SCHEDULING.
        Value Domain: USE_FIRST, ROUND_ROBIN, FIXED_MAPPING

    - name        : MVP_SUPPRESS_JOB_STARTUP_PERFORMANCE_WARNING
      category    : CH3
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : Set to 1 to suppress performance warnings.

    - name        : MVP_USE_RDMA_FAST_PATH
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Setting this parameter enables MVAPICH to use adaptive RDMA fast path
        features for OFA-IB-CH3 interface.

    - name        : MVP_RDMA_FAST_PATH_BUF_SIZE
      category    : CH3
      type        : int
      default     : 4096
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        The size of the buffer used in RDMA fast path communication.
        This value will be ineffective if MVP_USE_RDMA_FAST_PATH is not set

    - name        : MVP_POLLING_SET_LIMIT
      category    : CH3
      type        : int
      default     : 64
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_POLLING_SET_THRESHOLD
      category    : CH3
      type        : int
      default     : 256
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_RDMA_EAGER_LIMIT
      category    : CH3
      type        : int
      default     : 32
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_NUM_RDMA_BUFFER
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        The number of RDMA buffers used for the RDMA fast path. This fast path
        is used to reduce latency and overhead of small data and control
        messages. This value will be ineffective if MVP_USE_RDMA_FAST_PATH is
        not set.

    - name        : MVP_RDMA_FAST_PATH_PREALLOCATE_BUFFERS
      category    : CH3
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_SYSREPORT
      category    : CH3
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_DEFAULT_MTU
      category    : CH3
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        The internal MTU size. For OFA-IB-CH3, this parameter should be a
        string instead of an integer. Valid values are: IBV_MTU_256,
        IBV_MTU_512, IBV_MTU_1024, IBV_MTU_2048, IBV_MTU_4096.

    - name        : MVP_NUM_CQES_PER_POLL
      category    : CH3
      type        : int
      default     : 96
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Set this to 1, to enable hardware multicast support in collective
        communication

    - name        : MVP_NUM_PORTS
      category    : CH3
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This parameter indicates number of ports per InfiniBand adapter to be
        used for communication per adapter on an end node.

    - name        : MVP_NUM_QP_PER_PORT
      category    : CH3
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This parameter indicates number of queue pairs per port to be used for
        communication on an end node. This is useful in the presence of multiple
        send/recv engines available per port for data transfer.

    - name        : MVP_PIN_POOL_SIZE
      category    : CH3
      type        : int
      default     : (2 * 1024 * 1024)
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_DEFAULT_MAX_CQ_SIZE
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Setting this value forces a different CQ size. By default the value is
        40000 or the max value supported by the hardware. A value of -1
        indicates deferal to the hardware/default value.

    - name        : MVP_IBA_EAGER_THRESHOLD
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This specifies the switch point between eager and rendezvous protocol in
        MVAPICH. For better performance, the value of
        MPIR_CVAR_MVP_IBA_EAGER_THRESHOLD should be set the same as
        MPIR_CVAR_MVP_VBUF_TOTAL_SIZE. The default value is architecture
        dependent.

    - name        : MVP_STRIPING_THRESHOLD
      category    : CH3
      type        : int
      default     : 8192
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This parameter specifies the message size above which we begin the
        stripe the message across multiple rails (if present).

    - name        : MVP_RAIL_SHARING_MED_MSG_THRESHOLD
      category    : CH3
      type        : int
      default     : 2048
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This specifies the threshold for the message size beyond which striping
        will take place. In the previous versions of MVAPICH it was known as
        MVP_STRIPING_THRESHOLD.

    - name        : MVP_RAIL_SHARING_LARGE_MSG_THRESHOLD
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This specifies the threshold for the message size beyond which
        striping will take place. In the previous versions of MVAPICH
        it was known as MVP_STRIPING_THRESHOLD


    - name        : MVP_DEFAULT_PUT_GET_LIST_SIZE
      category    : CH3
      type        : int
      default     : 200
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_EAGERSIZE_1SC
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_PUT_FALLBACK_THRESHOLD
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This defines the threshold beyond which the MPI_Put
        implementation is based on direct one sided RDMA operations.

    - name        : MVP_GET_FALLBACK_THRESHOLD
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_DEFAULT_QP_OUS_RD_ATOM
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        The maximum number of outstanding RDMA reads and atomic operations
        per QP. Default value is 1 if supported. A value of -1 indicates
        defering to the hardware support.

    - name        : MVP_DEFAULT_MAX_RDMA_DST_OPS
      category    : CH3
      type        : int
      default     : 4
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_DEFAULT_PSN
      category    : CH3
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_DEFAULT_PKEY
      category    : CH3
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Select the partition to be used for the job.

    - name        : MVP_DEFAULT_QKEY
      category    : CH3
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_DEFAULT_MIN_RNR_TIMER
      category    : CH3
      type        : int
      default     : 12
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_DEFAULT_SERVICE_LEVEL
      category    : CH3
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_DEFAULT_TIME_OUT
      category    : CH3
      type        : int
      default     : 20
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_DEFAULT_STATIC_RATE
      category    : CH3
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_DEFAULT_SRC_PATH_BITS
      category    : CH3
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_DEFAULT_RETRY_COUNT
      category    : CH3
      type        : int
      default     : 7
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_DEFAULT_RNR_RETRY
      category    : CH3
      type        : int
      default     : 7
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_DEFAULT_MAX_SG_LIST
      category    : CH3
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_DEFAULT_MAX_SEND_WQE
      category    : CH3
      type        : int
      default     : 64
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This specifies the maximum number of send WQEs on each
        QP. Please note that for OFA-IB-CH3 and OFA-iWARP-CH3, the
        default value of this parameter will be 16 if the number of
        processes is larger than 256 for better memory scalability.


    - name        : MVP_CM_WAIT_TIME
      category    : CH3
      type        : int
      default     : 5
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_UD_HYBRID
      category    : CH3
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Set this to Zero, to disable UD transport in hybrid configuration mode.

    - name        : MVP_UD_MTU
      category    : CH3
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_HYBRID_MAX_RC_CONN
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Maximum number of RC or XRC connections created per
        process. This limits the amount of connection memory and
        prevents HCA QP cache thrashing. Default value is dependent
        on the process count, setting this value to -1 will use
        the default value for the process count.

    - name        : MVP_UD_NUM_MSG_LIMIT
      category    : CH3
      type        : int
      default     : 100
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_UD_SENDWINDOW_SIZE
      category    : CH3
      type        : int
      default     : 400
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_UD_RECVWINDOW_SIZE
      category    : CH3
      type        : int
      default     : 2501
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_UD_RETRY_TIMEOUT
      category    : CH3
      type        : int
      default     : 500000
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Time (usec) after which an unacknowledged message will be
        retried.

    - name        : MVP_UD_MAX_RETRY_TIMEOUT
      category    : CH3
      type        : int
      default     : 20000000
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_UD_PROGRESS_SPIN
      category    : CH3
      type        : int
      default     : 1200
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_UD_RETRY_COUNT
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Number of retries of a message before the job is aborted. This
        is needed in case of HCA fails. The default value is based on
        the process count. Setting this to -1 will use the default for
        the process count.

    - name        : MVP_UD_PROGRESS_TIMEOUT
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Time (usec) until ACK status is checked (and ACKs are sent if
        needed). To avoid unnecessary retries, set this value less
        than MVP_UD_RETRY_TIMEOUT. It is recommended to set this to
        1/10 of MVP_UD_RETRY_TIMEOUT. The default value is based on
        the process count. Setting this to -1 will use the default for
        the process count.

    - name        : MVP_UD_MAX_SEND_WQE
      category    : CH3
      type        : int
      default     : 256
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_UD_MAX_RECV_WQE
      category    : CH3
      type        : int
      default     : 4096
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_UD_SRQ
      category    : CH3
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_UD_VBUF_POOL_SIZE
      category    : CH3
      type        : int
      default     : 8192
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_UD_MAX_ACK_PENDING
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Maximum number of outstanding UD buffers waiting for ACK. Setting this
        value to -1 will use the default value of MVP_UD_SENDWINDOW_SIZE / 4.

    - name        : MVP_SHMEM_BACKED_UD_CM
      category    : CH3
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC - Not used currently.

    - name        : MVP_USE_UD_ZCOPY
      category    : CH3
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Whether or not to use the zero-copy transfer mechanism to
        transfer large messages on UD transport.

    - name        : MVP_UD_ZCOPY_THRESHOLD
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Defines the threshold for transitioning to UD ZCOPY algorithm
        from R3 protocol for rndv messages. Defaults to
        MVP_IBA_EAGER_THRESHOLD. Setting this value to -1 will
        use the eager threshold (MVP_IBA_EAGER_THRESHOLD) as the default
        value causing all rndv messages to use UD_ZCOPY.

    - name        : MVP_UD_NUM_ZCOPY_RNDV_QPS
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Number of UD QPs to create for ZCOPY transfers. Setting this to -1
        will use the default value of 64 * <number_of_hcas>.

    - name        : MVP_UD_ZCOPY_RQ_SIZE
      category    : CH3
      type        : int
      default     : 4096
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_UD_ZCOPY_NUM_RETRY
      category    : CH3
      type        : int
      default     : 50000
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_UD_ZCOPY_ENABLE_POLLING
      category    : CH3
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_UD_ZCOPY_PUSH_SEGMENT
      category    : CH3
      type        : int
      default     : 32
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_UD_DROP_PACKET_RATE
      category    : CH3
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_MCAST_ENABLE_REL
      category    : CH3
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_MCAST_USE_MCAST_NACK
      category    : CH3
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_MCAST_NUM_NODES_THRESHOLD
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This defines the threshold for enabling multicast support in
        collective communication. When MVP_USE_MCAST is set to 1 and
        the number of nodes in the job is greater than or equal to the
        threshold value, it uses multicast support in collective
        communication.

    - name        : MVP_MCAST_MAX_RECV_WQE
      category    : CH3
      type        : int
      default     : 2096
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_MCAST_WINDOW_SIZE
      category    : CH3
      type        : int
      default     : 256
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_MCAST_DROP_PACKET_RATE
      category    : CH3
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_MCAST_RETRY_TIMEOUT
      category    : CH3
      type        : int
      default     : 500000
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_MCAST_MAX_RETRY_TIMEOUT
      category    : CH3
      type        : int
      default     : 20000000
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_MCAST_NSPIN_THRESHOLD
      category    : CH3
      type        : int
      default     : 1200
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_MCAST_COMM_INIT_TIMEOUT
      category    : CH3
      type        : int
      default     : 10000
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_MCAST_COMM_INIT_RETRIES
      category    : CH3
      type        : int
      default     : 128
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_MCAST_SKIP_LOOPBACK
      category    : CH3
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_MCAST_BCAST_MIN_MSG
      category    : CH3
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_MCAST_BCAST_MAX_MSG
      category    : CH3
      type        : int
      default     : 512 * 1042
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_DEFAULT_MAX_RECV_WQE
      category    : CH3
      type        : int
      default     : 128
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This specifies the maximum number of receive WQEs on each QP
        (maximum number of receives that can be posted on a single QP).

    - name        : MVP_NDREG_ENTRIES_MAX
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_NDREG_ENTRIES
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This defines the total number of buffers that can be stored in
        the registration cache. It has no effect if
        MVP_USE_LAZY_MEM_UNREGISTER is not set. A larger value will
        lead to less frequent lazy de-registration.

    - name        : MVP_DREG_CACHE_LIMIT
      category    : CH3
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_HWLOC_CPU_BINDING
      category    : CH3
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_THREAD_YIELD_SPIN_THRESHOLD
      category    : CH3
      type        : int
      default     : 5
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_THREAD_YIELD
      category    : CH3
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_NUM_SPINS_BEFORE_LOCK
      category    : CH3
      type        : int
      default     : 2000
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_ASYNC_THREAD_STACK_SIZE
      category    : CH3
      type        : int
      default     : (1 << 20)
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_HUGEPAGES
      category    : CH3
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Set this to 0, to not use any HugePages.

    - name        : MVP_MEMORY_OPTIMIZATION
      category    : CH3
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_SRQ_MAX_SIZE
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This is the maximum number of work requests allowed on the
        Shared Receive Queue. Upon receiving a SRQ limit event, the
        current value of MVP_SRQ_SIZE will be doubled or moved to the
        maximum of MVP_SRQ_MAX_SIZE, whichever is smaller.

        When using RDMA_CM, we cannot support very large SRQ. So, unless user
        set it, reduce the max_srq_size to 4K.

    - name        : MVP_SRQ_SIZE
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This is the initial number of work requests posted to the
        Shared Receive Queue.

    - name        : MVP_SRQ_LIMIT
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This is the low water-mark limit for the Shared Receive
        Queue. If the number of available work entries on the SRQ
        drops below this limit, the flow control will be activated.

    - name        : MVP_UD_SRQ_MAX_SIZE
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_UD_SRQ_SIZE
      category    : CH3
      type        : int
      default     : 4096
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_UD_SRQ_LIMIT
      category    : CH3
      type        : int
      default     : 128
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_MAX_NUM_UD_VBUFS
      category    : CH3
      type        : int
      default     : (INT_MAX)
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_MAX_INLINE_SIZE
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This defines the maximum inline size for data transfer. Please
        note that the default value of this parameter will be 0 when
        the number of processes is larger than 256 to improve memory
        usage scalability.

    - name        : MVP_VBUF_TOTAL_SIZE
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        The size of each vbuf, the basic communication buffer of
        MVAPICH. For better performance, the value of
        MVP_IBA_EAGER_THRESHOLD should be set the same as
        MVP_VBUF_TOTAL_SIZE.


    - name        : MVP_VBUF_MAX
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Max (total) number of vbufs to allocate, after which process
        terminates with a fatal error. -1 means no limit.

    - name        : MVP_INITIAL_PREPOST_DEPTH
      category    : CH3
      type        : int
      default     : 10
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This defines the initial number of pre-posted receive buffers
        for each connection. If communication happen for a particular
        connection, the number of buffers will be increased to
        RDMA_PREPOST_DEPTH.

    - name        : MVP_PREPOST_DEPTH
      category    : CH3
      type        : int
      default     : 64
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This defines the number of buffers pre-posted for each
        connection to handle send/receive operations.


    - name        : MVP_VBUF_POOL_SIZE
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        The number of vbufs in the initial pool. This pool is shared
        among all the connections.


    - name        : MVP_MAX_NUM_VBUFS
      category    : CH3
      type        : int
      default     : 8192
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_VBUF_SECONDARY_POOL_SIZE
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        The number of vbufs allocated each time when the global pool
        is running out in the initial pool. This is also shared among
        all the connections.

    - name        : MVP_CUDA_BLOCK_SIZE
      category    : CH3
      type        : int
      default     : (256 * 1024)
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        The chunk size used in large message transfer from device
        memory to host memory. The other suggested values for this
        parameter are 131072 and 524288.


    - name        : MVP_CUDA_NUM_RNDV_BLOCKS
      category    : CH3
      type        : int
      default     : 8
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_MPIRUN_RSH_LAUNCH
      category    : CH3
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_ROCE
      alt-env     : MVP_USE_RoCE
      category    : CH3
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This parameter enables the use of RDMA over Ethernet for MPI
        communication. The underlying HCA and network must support
        this feature.


    - name        : MVP_USE_ROCE_MODE
      alt-env     : MVP_USE_RoCE_MODE
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_RING_STARTUP
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Setting this parameter enables MVAPICH to use ring based
        start up.

    - name        : MVP_ON_DEMAND_UD_INFO_EXCHANGE
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Setting MVP_ON_DEMAND_UD_INFO_EXCHANGE to 1 will enable
        on-demand Address Handle creation for hybrid mode.

    - name        : MVP_USE_PMI_IBARRIER
      category    : CH3
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_PMI_IALLGATHER
      category    : CH3
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_NUM_HCAS
      category    : CH3
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This parameter indicates number of InfiniBand adapters to be
        used for communication on an end node.
        Value is potentially computed.

    - name        : MVP_HOMOGENEOUS_CLUSTER
      category    : CH3
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Set to 1 if using a homogeneous cluster.

=== END_MPI_T_MVP_CVAR_INFO_BLOCK ===
*/
/* Extra buffer space for header(s); used to adjust the eager-threshold */
#define EAGER_THRESHOLD_ADJUST  0
#define INLINE_THRESHOLD_ADJUST (40)
extern const char MPII_Version_string[];
extern unsigned int mvp_enable_affinity;
extern int g_mvp_num_cpus;
extern int rdma_skip_network_card(mvp_iba_network_classes network_type,
                                  struct ibv_device *ib_dev);
static inline void rdma_get_vbuf_user_parameters(int num_proc, int me);

/*
 * ==============================================================
 * Initialize global parameter variables to default values
 * ==============================================================
 */

int mvp_use_opt_eager_recv = 0;
int mvp_num_extra_polls = 0;
int mvp_is_in_finalize = 0;
int mvp_cm_wait_time = DEF_MVP_CM_WAIT_TIME;
int rdma_num_cqes_per_poll = RDMA_MAX_CQE_ENTRIES_PER_POLL;
int rdma_num_hcas = 1;
int mvp_closest_hca_offset = 0;
int rdma_num_req_hcas = 0;
int rdma_num_ports = DEFAULT_NUM_PORTS;
int rdma_num_qp_per_port = DEFAULT_NUM_QP_PER_PORT;
int rdma_num_rails;
int rdma_memory_optimization = 1;
int rdma_pin_pool_size = RDMA_PIN_POOL_SIZE;
unsigned long rdma_default_max_cq_size = RDMA_DEFAULT_MAX_CQ_SIZE;
int rdma_default_port = RDMA_DEFAULT_PORT;
int rdma_default_gid_index = RDMA_DEFAULT_GID_INDEX;
int rdma_default_max_send_wqe = RDMA_DEFAULT_MAX_SEND_WQE;
int rdma_default_max_recv_wqe = RDMA_DEFAULT_MAX_RECV_WQE;
uint32_t rdma_default_max_sg_list = RDMA_DEFAULT_MAX_SG_LIST;
uint16_t rdma_default_pkey_ix = RDMA_DEFAULT_PKEY_IX;
uint16_t rdma_default_pkey = RDMA_DEFAULT_PKEY;
uint32_t rdma_default_qkey = RDMA_DEFAULT_QKEY;
uint8_t rdma_default_qp_ous_rd_atom = RDMA_DEFAULT_QP_OUS_RD_ATOM;
uint8_t rdma_supported_max_qp_ous_rd_atom = RDMA_DEFAULT_QP_OUS_RD_ATOM;
uint8_t rdma_default_max_rdma_dst_ops = RDMA_DEFAULT_MAX_RDMA_DST_OPS;
uint8_t rdma_supported_max_rdma_dst_ops = RDMA_DEFAULT_MAX_RDMA_DST_OPS;
enum ibv_mtu rdma_default_mtu = IBV_MTU_1024;
uint32_t rdma_default_psn = RDMA_DEFAULT_PSN;
uint8_t rdma_default_min_rnr_timer = RDMA_DEFAULT_MIN_RNR_TIMER;
uint8_t rdma_default_service_level = RDMA_DEFAULT_SERVICE_LEVEL;
uint8_t rdma_default_static_rate = RDMA_DEFAULT_STATIC_RATE;
uint8_t rdma_default_src_path_bits = RDMA_DEFAULT_SRC_PATH_BITS;
uint8_t rdma_default_time_out = RDMA_DEFAULT_TIME_OUT;
uint8_t rdma_default_retry_count = RDMA_DEFAULT_RETRY_COUNT;
uint8_t rdma_default_rnr_retry = RDMA_DEFAULT_RNR_RETRY;
int rdma_default_put_get_list_size = RDMA_DEFAULT_PUT_GET_LIST_SIZE;
long rdma_eagersize_1sc;
int rdma_put_fallback_threshold;
int rdma_get_fallback_threshold;
int rdma_polling_set_limit = -1;
int rdma_polling_set_threshold = 256;
int rdma_fp_buffer_size = RDMA_FP_DEFAULT_BUF_SIZE;
int rdma_fp_sendconn_accepted = 0;
int rdma_pending_conn_request = 0;
int rdma_eager_limit = 32;
int rdma_iba_eager_threshold;
char rdma_iba_hcas[MAX_NUM_HCAS][32];
int rdma_max_inline_size;
int rdma_ndreg_entries = 0;
int rdma_ndreg_entries_max = RDMA_NDREG_ENTRIES_MAX;
int rdma_rndv_immediate = 1;
int mvp_use_balanced_rndv = 1;
int rdma_r3_threshold = RDMA_DEFAULT_R3_THRESHOLD;
int rdma_intra_node_r3_threshold = RDMA_DEFAULT_INTRA_NODE_R3_THRESHOLD;
int rdma_inter_node_r3_threshold = RDMA_DEFAULT_INTER_NODE_R3_THRESHOLD;
int rdma_r3_threshold_nocache = 8192 * 4;
int rdma_max_r3_pending_data = 512 * 1024;
int num_rdma_buffer = 0;
int rdma_use_xrc = 0;
int xrc_rdmafp_init = 0;
int rdma_use_smp = 1;
int mvp_use_smp = 1;
int rdma_use_qos = 0;
#ifdef ENABLE_3DTORUS_SUPPORT
int rdma_3dtorus_support = 1;
#else
int rdma_3dtorus_support = 0;
#endif /* ENABLE_3DTORUS_SUPPORT */
int rdma_path_sl_query = 0;
int rdma_num_sa_query_retries = RDMA_DEFAULT_NUM_SA_QUERY_RETRIES;
int rdma_num_nodes_in_job = 0;
int rdma_qos_num_sls = RDMA_QOS_DEFAULT_NUM_SLS;
int max_rdma_connect_attempts = DEFAULT_RDMA_CONNECT_ATTEMPTS;
#ifdef _MULTI_SUBNET_SUPPORT_
int mvp_rdma_cm_multi_subnet_support = 0;
#endif /* _MULTI_SUBNET_SUPPORT_ */
int rdma_cm_connect_retry_interval = RDMA_DEFAULT_CONNECT_INTERVAL;
int rdma_default_async_thread_stack_size = RDMA_DEFAULT_ASYNC_THREAD_STACK_SIZE;
int rdma_num_rails_per_hca = 1;
int rdma_process_binding_rail_offset = 0;
int rdma_multirail_usage_policy = MVP_MRAIL_BINDING;
int rdma_small_msg_rail_sharing_policy = ROUND_ROBIN;
int rdma_med_msg_rail_sharing_policy = ROUND_ROBIN;
int rdma_med_msg_rail_sharing_threshold =
    RDMA_DEFAULT_MED_MSG_RAIL_SHARING_THRESHOLD;
int rdma_large_msg_rail_sharing_threshold =
    RDMA_DEFAULT_LARGE_MSG_RAIL_SHARING_THRESHOLD;

/* This is to allow users to specify rail mapping at run time */
int mrail_user_defined_p2r_mapping = -1;
int mrail_p2r_length;
int mrail_use_default_mapping = 0;
char *mrail_p2r_string = NULL;
/* Threshold of job size beyond which we want to use 2-cq approach */
int rdma_iwarp_multiple_cq_threshold = RDMA_IWARP_DEFAULT_MULTIPLE_CQ_THRESHOLD;
int rdma_iwarp_use_multiple_cq = 0;
/* Force to use rendezvous if extended sendq size exceeds this value */
int rdma_rndv_ext_sendq_size = 5;
/* Global count of extended sendq size across all rails*/
int rdma_global_ext_sendq_size = 0;
/* Number of times to poll while global ext sendq has outstanding requests */
int rdma_num_extra_polls = 1;
int rdma_local_id = -1;
int rdma_num_local_procs = -1;
/* Whether coalescing of messages should be attempted */
int rdma_use_coalesce = DEFAULT_USE_COALESCE;
unsigned long rdma_polling_spin_count_threshold = 5;
int mvp_use_thread_yield = 1;
int mvp_spins_before_lock = 2000;
int mvp_on_demand_ud_info_exchange = 1;
int mvp_homogeneous_cluster = 0;
int mvp_show_env_info = 0;
int mvp_use_pmi_ibarrier = 0;
int mvp_use_pmi_iallgather = 0;
int mvp_system_has_roce = 0;
int mvp_system_has_rockport = 0;
int mvp_system_has_ib = 0;
int mvp_process_placement_aware_hca_mapping = 0;
int mvp_allow_heterogeneous_hca_selection = 0;

/* If this number of eager sends are already outstanding
 * the message can be coalesced with other messages (and
 * will not be sent until a previous message completes)
 */
int rdma_coalesce_threshold = DEFAULT_COALESCE_THRESHOLD;

/* max (total) number of vbufs to allocate, after which process
 * terminates with a fatal error.
 * -1 means no limit.
 */
int rdma_vbuf_max = -1;
int rdma_max_num_vbufs = DEFAULT_MAX_NUM_VBUFS;

/* number of vbufs to allocate in a secondary region if we should
 * run out of the initial allocation.  This is re-computed (below)
 * once other parameters are known.
 */
int rdma_vbuf_secondary_pool_size = RDMA_VBUF_SECONDARY_POOL_SIZE;

/* number of vbufs to allocate initially.
 * This will be re-defined after reading the parameters below
 * to scale to the number of VIs and other factors.
 */
int rdma_vbuf_pool_size = RDMA_VBUF_POOL_SIZE;
int rdma_prepost_depth = RDMA_PREPOST_DEPTH;
int rdma_initial_prepost_depth = RDMA_INITIAL_PREPOST_DEPTH;

/* allow some extra buffers for non-credited packets (eg. NOOP) */
int rdma_prepost_noop_extra = 6;
int rdma_credit_preserve;
int rdma_initial_credits = 0;

uint16_t rdma_default_ud_mtu = 0;
#if defined(_ENABLE_UD_)
uint8_t rdma_use_ud_zcopy = 1;
uint8_t rdma_use_ud_srq = 0;
uint8_t rdma_ud_zcopy_enable_polling = 0;
uint8_t rdma_ud_zcopy_push_segment = 32;
uint32_t rdma_default_max_ud_send_wqe = RDMA_DEFAULT_MAX_UD_SEND_WQE;
uint32_t rdma_default_max_ud_recv_wqe = RDMA_DEFAULT_MAX_UD_RECV_WQE;
uint32_t rdma_ud_num_msg_limit = RDMA_UD_NUM_MSG_LIMIT;
uint32_t rdma_ud_vbuf_pool_size = RDMA_UD_VBUF_POOL_SIZE;
/* Maximum number of outstanding buffers (waiting for ACK)*/
uint32_t rdma_default_ud_sendwin_size = RDMA_DEFAULT_UD_SENDWIN_SIZE;
/* Maximum number of out-of-order messages that will be buffered */
uint32_t rdma_default_ud_recvwin_size = 2501;
/* Time (usec) until ACK status is checked (and ACKs are sent) */
long rdma_ud_progress_timeout = 48000;
/* Time (usec) until a message is resent */
long rdma_ud_retry_timeout = 500000;
long rdma_ud_max_retry_timeout = 20000000;
long rdma_ud_last_check;
uint32_t rdma_ud_zcopy_threshold;
uint32_t rdma_ud_zcopy_rq_size = 4096;
uint32_t rdma_hybrid_enable_threshold = 512;
uint32_t rdma_ud_zcopy_num_retry = 50000;
uint16_t rdma_ud_progress_spin = 1200;
uint16_t rdma_ud_max_retry_count = 1000;
uint16_t rdma_ud_max_ack_pending;
uint16_t rdma_ud_num_rndv_qps = RDMA_UD_DEFAULT_NUM_RNDV_QPS;
uint16_t rdma_hybrid_max_rc_conn = 64;
uint16_t rdma_hybrid_pending_rc_conn = 0;
#ifdef _MVP_UD_DROP_PACKET_RATE_
uint32_t ud_drop_packet_rate = 0;
#endif
int rdma_max_num_ud_vbufs = DEFAULT_MAX_NUM_UD_VBUFS;
#endif
#if defined(_MCST_SUPPORT_)
uint8_t rdma_enable_mcast = USE_MCAST_DEFAULT_FLAG;
#if defined(RDMA_CM)
uint8_t rdma_use_rdma_cm_mcast = USE_MCAST_DEFAULT_FLAG;
#endif /*defined(RDMA_CM)*/
uint8_t mcast_enable_rel = 1;
uint8_t mcast_use_mcast_nack = 1;
uint16_t mcast_window_size = 256;
uint16_t mcast_drop_packet_rate = 0;
uint32_t mcast_num_nodes_threshold = MCAST_NUM_THRESHOLD;
uint32_t mcast_max_ud_recv_wqe = 2096;
long mcast_retry_timeout = 500000;
long mcast_max_retry_timeout = 20000000;
long mcast_comm_init_timeout = 10000;
int mcast_comm_init_retries = 128;
int mcast_nspin_threshold = 1200;
int mcast_skip_loopback = 1;
uint32_t mcast_bcast_min_msg = 1;
uint32_t mcast_bcast_max_msg = 512 * 1024;
#endif

/* Max number of entries on the RecvQ of QPs per connection.
 * computed to be:
 * prepost_depth + rdma_prepost_rendezvous_extra + rdma_prepost_noop_extra
 * Must be within NIC MaxQpEntries limit.
 */
int rdma_rq_size;
int using_mpirun_rsh = 0;

uint32_t mvp_srq_alloc_size = MVP_DEFAULT_SRQ_ALLOC_SIZE;
uint32_t mvp_srq_fill_size = MVP_DEFAULT_SRQ_FILL_SIZE;
uint32_t mvp_srq_limit = MVP_DEFAULT_SRQ_LIMIT;
uint32_t mvp_max_r3_oust_send = 32;
#if defined(_ENABLE_UD_)
uint32_t mvp_ud_srq_alloc_size = MVP_DEFAULT_UD_SRQ_ALLOC_SIZE;
uint32_t mvp_ud_srq_fill_size = MVP_DEFAULT_UD_SRQ_FILL_SIZE;
uint32_t mvp_ud_srq_limit = MVP_DEFAULT_UD_SRQ_LIMIT;
#endif /*defined(_ENABLE_UD_)*/

/* The number of "extra" vbufs that will be posted as receives
 * on a connection in anticipation of an R3 rendezvous message.
 * The TOTAL number of VBUFs posted on a receive queue at any
 * time is rdma_prepost_depth + rdma_prepost_rendezvous_extra
 * regardless of the number of outstanding R3 sends active on
 * a connection.
 */
int rdma_prepost_rendezvous_extra = 10;
int rdma_dynamic_credit_threshold = 10;
int rdma_credit_notify_threshold = 10;
int rdma_prepost_threshold = 5;

unsigned long rdma_dreg_cache_limit = 0;

/* Blocking mode progress */
int rdma_use_blocking = 0;
unsigned long rdma_blocking_spin_count_threshold = DEFAULT_SPIN_COUNT;

/* The total size of each vbuf. Used to be the eager threshold, but
 * it can be smaller, so that each eager message will span over few
 * vbufs
 */
int rdma_vbuf_total_size;

/* Small message scheduling policy
 * Was earlier set to USE_FIRST, optimized for minimal QP cache misses
 * Now setting it to FIXED_MAPPING as we get better performance.
 * 10/06/2010
 */
int rdma_rail_sharing_policy = FIXED_MAPPING;

/* This value should increase with the increase in number
 * of rails */
int striping_threshold = STRIPING_THRESHOLD;

/* Used IBoEth mode */
int use_iboeth = 0;
int mvp_use_post_srq_send = 1;
int rdma_enable_hugepage = 1;

/* Set default version for RoCE*/
mvp_roce_mode_t mvp_use_roce_mode = MVP_ROCE_MODE_V2;

/* Linear update factor for HSAM */
double alpha = 0.9;
int stripe_factor = 1;
int apm_tester = 0;

#ifdef _ENABLE_CUDA_
int mvp_enable_device = 0;
int mvp_device_stage_block_size = DEFAULT_CUDA_BLOCK_SIZE;
int mvp_device_num_rndv_blocks = 8;
int mvp_device_event_count = 64;
int rdma_enable_cuda = 0;
int mvp_device_dynamic_init = 1;
int mvp_device_nonblocking_streams = 1;
int mvp_device_initialized = 0;
int rdma_cuda_vec_thread_blksz = 0;
int rdma_cuda_vec_thread_ysz = 0;
int rdma_cuda_subarr_thread_blksz = 0;
int rdma_cuda_subarr_thread_xdim = 0;
int rdma_cuda_subarr_thread_ydim = 0;
int rdma_cuda_subarr_thread_zdim = 0;
int rdma_eager_devicehost_reg = 0;
int rdma_cuda_vector_dt_opt = 1;
int rdma_cuda_kernel_dt_opt = 1;
#if defined(HAVE_CUDA_IPC)
int mvp_device_use_ipc = 1;
int rdma_enable_ipc_share_gpu = 1;
int mvp_device_use_smp_eager_ipc = 0;
int mvp_device_enable_ipc_cache = 1;
int mvp_device_ipc_threshold = 512 * 1024;
int mvp_device_ipc_cache_max_entries = 64;
#endif /*#if defined(HAVE_CUDA_IPC) */
int mvp_device_coll_use_stage = 1;
int mvp_device_coll_register_stage_buf_threshold = 524288;
int mvp_device_gather_stage_limit = 32768;
int mvp_device_scatter_stage_limit = 2048;
int mvp_device_gatherv_stage_limit = 0;
int mvp_device_scatterv_stage_limit = 16384;
int mvp_device_allgather_stage_limit = 1048576;
int mvp_device_allgatherv_stage_limit = 524288;
int mvp_device_alltoall_stage_limit = 262144;
int mvp_device_alltoallv_stage_limit = 262144;
int mvp_device_bcast_stage_limit = 2097152;
int mvp_device_alltoall_dynamic = 1;
int mvp_device_allgather_rd_limit = 1024;
int mvp_device_use_allgather_fgp = 0;
int mvp_device_init_context = 1;
int mvp_device_check_attribute = 0;
#endif /*#ifdef _ENABLE_CUDA_ */
int mvp_enable_progress_affinity = 0;

typedef enum _mvp_user_defined_mapping_policies {

    MVP_UDEF_POLICY_BUNCH = 1,
    MVP_UDEF_POLICY_SCATTER,
    MVP_UDEF_POLICY_NONE,

} user_defined_mapping_policies;

/* Optimal CPU Binding parameters */
int use_hwloc_cpu_binding = 1;

mvp_polling_level rdma_polling_level = MVP_POLLING_LEVEL_1;

int g_atomics_support = 0;
int g_atomics_support_be = 0;

static int check_hsam_parameters(void);

static inline int log_2(int np)
{
    int lgN, t;
    for (lgN = 0, t = 1; t < np; lgN++, t += t)
        ;
    return lgN;
}

mvp_arch_hca_type MVP_get_arch_hca_type()
{
    struct mvp_MPIDI_CH3I_RDMA_Process_t *proc = &mvp_MPIDI_CH3I_RDMA_Process;
    if (!proc->arch_hca_type) {
        proc->arch_type = mvp_get_arch_type();
        proc->hca_type = MVP_HCA_ANY;
        proc->arch_hca_type = proc->arch_type;
        proc->arch_hca_type <<= 16;
        proc->arch_hca_type |= proc->hca_type;
        proc->arch_hca_type <<= 16;
        proc->arch_hca_type |= g_mvp_num_cpus;
    }

    return proc->arch_hca_type;
}

/*
 * Function: rdma_cm_get_hca_type
 *
 * Description:
 *      Finds out the type of the HCA on the system.
 *
 * Input:
 *      use_iwarp_mode  - Command line input specifying whether we need to use
 *                        iWARP mode.
 * Output:
 *      hca_type        - The type of HCA we are going to use.
 *
 * Return:
 *      Success:    MPI_SUCCESS.
 *      Failure:    ERROR (-1).
 */
#if defined(RDMA_CM)
int rdma_cm_get_hca_type(struct mvp_MPIDI_CH3I_RDMA_Process_t *proc)
{
    int i = 0;
    int ret = 0;
    int mpi_errno = MPI_SUCCESS;
    int numdevices = 0;
    int network_type = MVP_HCA_UNKWN, hca_type = MVP_HCA_UNKWN;
    struct ibv_device_attr dev_attr;
    char *dev_name = NULL, *value;
    struct ibv_context **ctx = rdma_ops.get_devices(&numdevices);

    MPIR_FUNC_TERSE_STATE_DECL(MPID_STATE_RDMA_CM_GET_HCA_TYPE);
    MPIR_FUNC_TERSE_ENTER(MPID_STATE_RDMA_CM_GET_HCA_TYPE);

    for (i = 0; i < numdevices; ++i) {
        hca_type = mvp_get_hca_type(ctx[i]->device);
        PRINT_DEBUG(DEBUG_INIT_verbose > 1, "HCA %s type = %s\n",
                    ctx[i]->device->name, mvp_get_hca_name(hca_type));
        if (network_type <= hca_type) {
            network_type = hca_type;
        }
    }

    for (i = 0; i < numdevices; ++i) {
        proc->hca_type = MVP_HCA_UNKWN;
        dev_name = (char *)ibv_ops.get_device_name(ctx[i]->device);

        if (!dev_name) {
            MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER,
                                "**ibv_get_device_name");
        }

        if (MVP_IBA_HCA != NULL) {
            if (strstr(MVP_IBA_HCA, dev_name) == NULL) {
                continue;
            }
        }

        if (rdma_skip_network_card(network_type, ctx[i]->device)) {
            /* Skip HCA's that don't match with network type */
            PRINT_DEBUG(DEBUG_INIT_verbose,
                        "1. Skipping HCA %s since type does not match."
                        "Selected: %s; Current: %s\n",
                        ctx[i]->device->name, mvp_get_hca_name(network_type),
                        mvp_get_hca_name(mvp_get_hca_type(ctx[i]->device)));
            continue;
        }

        if ((ret = ibv_ops.query_device(ctx[i], &dev_attr))) {
            MPIR_ERR_SETANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**ibv_query_device",
                                 "**ibv_query_device %s", dev_name);
        }

        if (ERROR ==
            rdma_find_active_port(ctx[i], ctx[i]->device, NULL, NULL, NULL)) {
            /* Trac #376 The device has no active ports, continue to next device
             */
            continue;
        }

        proc->hca_type = mvp_get_hca_type(ctx[i]->device);
        proc->arch_hca_type = mvp_get_arch_hca_type(ctx[i]->device);
        if (MVP_IS_CHELSIO_IWARP_CARD(proc->hca_type)) {
            /* Trac #376 recognize chelsio nic even if it's not the first */
            proc->use_rdma_cm = 1;
            proc->use_iwarp_mode = 1;
            MPIDI_CH3I_Process.cm_type = MPIDI_CH3I_CM_RDMA_CM;
            strncpy(rdma_iba_hcas[0], CHELSIO_RNIC, 32);
        } else if (MVP_HCA_INTEL_NE020 == proc->hca_type) {
            proc->use_rdma_cm = 1;
            proc->use_iwarp_mode = 1;
            MPIDI_CH3I_Process.cm_type = MPIDI_CH3I_CM_RDMA_CM;
            strncpy(rdma_iba_hcas[0], INTEL_NE020_RNIC, 32);
        }

        if (MVP_HCA_UNKWN != proc->hca_type) {
            /* We've found the HCA */
            break;
        }
    }

fn_fail:
    rdma_ops.free_devices(ctx);
    MPIR_FUNC_TERSE_EXIT(MPID_STATE_RDMA_CM_GET_HCA_TYPE);
    return mpi_errno;
}

#endif /* defined(RDMA_CM) */

/* The rdma_get_process_to_rail_mapping function is called from
 * ch3_smp_progress.c to set the mapping given by the user at run time
 */
int rdma_get_process_to_rail_mapping(int mrail_user_defined_p2r_type)
{
    char *p2r_only_numbers = (char *)MPL_malloc(
        (mrail_p2r_length + 1) * sizeof(char), MPL_MEM_OTHER);
    int i, j = 0, k = 0;
    int num_total_devices = 0;
    int num_devices = 0, num_devices_on_my_numa = 0;
    char *tp = mrail_p2r_string;
    char *cp = NULL;
    char tp_str[mrail_p2r_length + 1];
    struct ibv_device **dev_list = NULL;
    struct ibv_device **usable_dev_list =
        MPL_malloc(sizeof(struct ibv_device *) * MAX_NUM_HCAS, MPL_MEM_OTHER);
    struct ibv_device **usable_devs_on_my_numa =
        MPL_malloc(sizeof(struct ibv_device *) * MAX_NUM_HCAS, MPL_MEM_OTHER);
    int bunch_hca_count;

    dev_list = ibv_ops.get_device_list(&num_total_devices);

    rdma_find_network_type(dev_list, num_total_devices, usable_dev_list,
                           usable_devs_on_my_numa, &num_devices,
                           &num_devices_on_my_numa, NULL);

    if (rdma_num_req_hcas && rdma_num_req_hcas <= num_devices) {
        num_devices = rdma_num_req_hcas;
    }
    switch (mrail_user_defined_p2r_type) {
        case MVP_UDEF_POLICY_NONE:
            /* Start with default setting */
            mrail_use_default_mapping = 1;
            while (*tp != '\0') {
                i = 0;
                cp = tp;
                while (*cp != '\0' && *cp != ':' && i < mrail_p2r_length) {
                    ++cp;
                    ++i;
                }

                strncpy(tp_str, tp, i);
                tp_str[i] = '\0';

                if (j == rdma_local_id) {
                    for (k = 0; k < num_devices; ++k) {
                        if (!strcmp(usable_dev_list[k]->name, tp_str)) {
                            mrail_user_defined_p2r_mapping = k;
                            mrail_use_default_mapping = 0;
                            break;
                        }
                    }
                    if (k == num_devices) {
                        PRINT_ERROR(
                            "HCA %s either does not exist on this machine "
                            "or is deemed unusable due to heterogeneity or "
                            "because of being on different subnets. "
                            "Falling back to default scheme\n",
                            tp_str);
                        mrail_use_default_mapping = 1;
                        rdma_rail_sharing_policy = FIXED_MAPPING;
                        goto fn_exit;
                    }
                    PRINT_DEBUG(DEBUG_INIT_verbose,
                                "Mapping HCA %s. Index: %d\n", tp_str,
                                mrail_user_defined_p2r_mapping);
                    break;
                }

                if (*cp == '\0') {
                    break;
                }

                tp = cp;
                ++tp;
                ++j;
            }
            break;

        case MVP_UDEF_POLICY_SCATTER:
            mrail_user_defined_p2r_mapping = rdma_local_id % num_devices;
            break;
        case MVP_UDEF_POLICY_BUNCH:
            bunch_hca_count = rdma_num_local_procs / num_devices;
            if ((bunch_hca_count * num_devices) < rdma_num_local_procs) {
                bunch_hca_count++;
            }
            mrail_user_defined_p2r_mapping = rdma_local_id / bunch_hca_count;
            break;
        default:
            if (rdma_local_id == 0) {
                fprintf(stderr, "\nError determining type of user defined"
                                " binding. Falling back to default scheme...!");
            }
            mrail_use_default_mapping = 1;
            rdma_rail_sharing_policy = FIXED_MAPPING;
            break;
    }
fn_exit:
    /* Housekeeping operations */
    MPL_free(usable_dev_list);
    MPL_free(usable_devs_on_my_numa);
    if (dev_list) {
        ibv_ops.free_device_list(dev_list);
    }
    MPL_free(p2r_only_numbers);
    MPL_free(mrail_p2r_string);
    mrail_p2r_string = NULL;
    p2r_only_numbers = NULL;

    return 0;
}

int rdma_get_rail_sharing_policy(const char *value)
{
    int policy = FIXED_MAPPING;

    if (!strcmp(value, "USE_FIRST")) {
        policy = USE_FIRST;
    } else if (!strcmp(value, "ROUND_ROBIN")) {
        policy = ROUND_ROBIN;
    } else if (!strcmp(value, "FIXED_MAPPING")) {
        policy = FIXED_MAPPING;
    } else {
        MPL_usage_printf("Invalid small message scheduling\n");
    }
    return policy;
}

/* Set thresholds for Nnum_rail=unknown */
static void set_limic_thresholds(struct mvp_MPIDI_CH3I_RDMA_Process_t *proc)
{
    switch (proc->arch_type) {
        case MVP_ARCH_AMD_BARCELONA_16:
            MVP_LIMIC_PUT_THRESHOLD = 1 * 1024;
            MVP_LIMIC_GET_THRESHOLD = 256;
            break;
        case MVP_ARCH_INTEL_CLOVERTOWN_8:
            MVP_LIMIC_PUT_THRESHOLD = 1 * 1024;
            MVP_LIMIC_GET_THRESHOLD = 1 * 1024;
            break;
        case MVP_ARCH_INTEL_NEHALEM_8:
            MVP_LIMIC_PUT_THRESHOLD = 8 * 1024;
            MVP_LIMIC_GET_THRESHOLD = 4 * 1024;
            break;
        default:
            MVP_LIMIC_PUT_THRESHOLD = 8 * 1024;
            MVP_LIMIC_GET_THRESHOLD = 8 * 1024;
            break;
    }
}

int rdma_set_smp_parameters(struct mvp_MPIDI_CH3I_RDMA_Process_t *proc)
{
#if defined(_SMP_LIMIC_)
    if (!mvp_enable_affinity || !MVP_SMP_USE_LIMIC2) {
        MVP_USE_LIMIC2_COLL = 0;
    }

#if defined(_SMP_CMA_)
    if (MVP_SMP_USE_CMA && MVP_SMP_USE_LIMIC2) {
        if (MVP_CVAR_IS_SET_BY_USER(MVP_SMP_USE_CMA) &&
            MVP_CVAR_IS_SET_BY_USER(MVP_SMP_USE_LIMIC2)) {
            PRINT_INFO(
                (MPIDI_Process.my_pg_rank == 0),
                "User selected support for CMA and LiMIC2 simultatneously."
                "This is not supported. Disabling LiMIC2 and proceeding.\n");
            MVP_SMP_USE_LIMIC2 = 0;
        } else if (MVP_CVAR_IS_SET_BY_USER(MVP_SMP_USE_LIMIC2)) {
            /* If user specifcally selected LiMIC2, disable CMA */
            MVP_SMP_USE_CMA = 0;
        } else {
            /* Disable LiMIC2 by default if build supports both CMA and LiMIC2
             * and user did not specifically request any feature */
            MVP_SMP_USE_LIMIC2 = 0;
        }
    }
#endif /* _SMP_CMA_ */
#endif /* _SMP_LIMIC_ */

    if (!proc->arch_type) {
        proc->arch_type = mvp_get_arch_type();
    }

    switch (proc->arch_type) {
        case MVP_ARCH_INTEL_XEON_E5630_8:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 8192;
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_SMP_EAGERSIZE = 8192;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 65536;
            }
            MVP_SMP_QUEUE_LENGTH = 262144;
            MVP_SMP_NUM_SEND_BUFFER = 256;
            MVP_SMP_SEND_BUF_SIZE = 8192;
            break;
        case MVP_ARCH_INTEL_GENERIC:
        case MVP_ARCH_INTEL_CLOVERTOWN_8:
        case MVP_ARCH_INTEL_XEON_DUAL_4:
#if defined(_SMP_LIMIC_)
            if (MVP_SMP_USE_LIMIC2) {
                MVP_SMP_EAGERSIZE = 8192;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 65536;
            }
            MVP_SMP_QUEUE_LENGTH = 262144;
            MVP_SMP_NUM_SEND_BUFFER = 256;
            MVP_SMP_SEND_BUF_SIZE = 8192;
            break;

        case MVP_ARCH_INTEL_HARPERTOWN_8:
#if defined(_SMP_LIMIC_)
            if (MVP_SMP_USE_LIMIC2) {
                MVP_SMP_EAGERSIZE = 8192;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 32768;
            }
            MVP_SMP_QUEUE_LENGTH = 262144;
            MVP_SMP_NUM_SEND_BUFFER = 256;
            MVP_SMP_SEND_BUF_SIZE = 8192;
            break;

        case MVP_ARCH_INTEL_NEHALEM_8:
        case MVP_ARCH_INTEL_NEHALEM_16:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 32768;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 65536;
            }
            MVP_SMP_QUEUE_LENGTH = 262144;
            MVP_SMP_NUM_SEND_BUFFER = 256;
            MVP_SMP_SEND_BUF_SIZE = 8192;
            break;

        case MVP_ARCH_AMD_BARCELONA_16:
#if defined(_SMP_LIMIC_)
            if (MVP_SMP_USE_LIMIC2) {
                MVP_SMP_EAGERSIZE = 8192;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 32768;
            }
            MVP_SMP_QUEUE_LENGTH = 131072;
            MVP_SMP_NUM_SEND_BUFFER = 32;
            MVP_SMP_SEND_BUF_SIZE = 32768;
            break;

        case MVP_ARCH_AMD_EPYC_7551_64:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 16384;
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_SMP_EAGERSIZE = 16384;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 16384;
            }
            MVP_SMP_QUEUE_LENGTH = 65536;
            MVP_SMP_NUM_SEND_BUFFER = 32;
            MVP_SMP_SEND_BUF_SIZE = 8192;
            break;

        /* TODO: tune single socket millan */
        case MVP_ARCH_AMD_EPYC_7763_128:
            /* lonestar6 */
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 28672;
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_SMP_EAGERSIZE = 16384;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 28672;
            }
            MVP_SMP_QUEUE_LENGTH = 131072;
            MVP_SMP_NUM_SEND_BUFFER = 32;
            MVP_SMP_SEND_BUF_SIZE = 8192;
            break;

        case MVP_ARCH_AMD_EPYC_7601_64:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 16384;
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_SMP_EAGERSIZE = 16384;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 16384;
            }
            MVP_SMP_QUEUE_LENGTH = 65536;
            MVP_SMP_NUM_SEND_BUFFER = 32;
            MVP_SMP_SEND_BUF_SIZE = 32768;
            break;

        case MVP_ARCH_AMD_EPYC_7V12_64:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 16384;
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_SMP_EAGERSIZE = 16384;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 4096;
            }
            MVP_SMP_QUEUE_LENGTH = 128 * 1024;
            MVP_SMP_NUM_SEND_BUFFER = 32;
            MVP_SMP_SEND_BUF_SIZE = 32768;
            break;

        case MVP_ARCH_AMD_EPYC_7401_48:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 16384;
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_SMP_EAGERSIZE = 16384;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 4096;
            }
            MVP_SMP_QUEUE_LENGTH = 65536;
            MVP_SMP_NUM_SEND_BUFFER = 16;
            MVP_SMP_SEND_BUF_SIZE = 30 * 1024;
            break;

        case MVP_ARCH_AMD_EPYC_7662_64:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 1048576;
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_SMP_EAGERSIZE = 1048576;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 1048576;
            }
            MVP_SMP_QUEUE_LENGTH = 4194304;
            MVP_SMP_NUM_SEND_BUFFER = 32;
            MVP_SMP_SEND_BUF_SIZE = 16384;
            break;

        case MVP_ARCH_AMD_EPYC_7742_128:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 16384;
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_SMP_EAGERSIZE = 16384;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 16384;
            }
            MVP_SMP_QUEUE_LENGTH = 262144;
            MVP_SMP_NUM_SEND_BUFFER = 32;
            MVP_SMP_SEND_BUF_SIZE = 16384;
            break;

        case MVP_ARCH_AMD_OPTERON_DUAL_4:
        case MVP_ARCH_AMD_GENERIC:
            MVP_SMP_EAGERSIZE = 5120;
            MVP_SMP_QUEUE_LENGTH = 65536;
            MVP_SMP_NUM_SEND_BUFFER = 32;
            MVP_SMP_SEND_BUF_SIZE = 8192;
            break;

        case MVP_ARCH_AMD_MAGNY_COURS_24:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 16384;
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_SMP_EAGERSIZE = 8192;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 5120;
            }
            MVP_SMP_QUEUE_LENGTH = 65536;
            MVP_SMP_NUM_SEND_BUFFER = 64;
            MVP_SMP_SEND_BUF_SIZE = 32768;
            break;

        case MVP_ARCH_AMD_OPTERON_6136_32:
#if defined(_SMP_LIMIC_)
            if (MVP_SMP_USE_LIMIC2) {
                MVP_SMP_EAGERSIZE = 8192;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 16384;
            }
            MVP_SMP_QUEUE_LENGTH = 65536;
            MVP_SMP_NUM_SEND_BUFFER = 32;
            MVP_SMP_SEND_BUF_SIZE = 32768;
            break;

        case MVP_ARCH_AMD_OPTERON_6276_64:
#if defined(_SMP_LIMIC_)
            if (MVP_SMP_USE_LIMIC2) {
                MVP_SMP_EAGERSIZE = 8192;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 16384;
            }
            MVP_SMP_QUEUE_LENGTH = 65536;
            MVP_SMP_NUM_SEND_BUFFER = 128;
            MVP_SMP_SEND_BUF_SIZE = 8192;
            break;

        case MVP_ARCH_INTEL_XEON_X5650_12:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 32768;
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_SMP_EAGERSIZE = 8192;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 65536;
            }
            MVP_SMP_QUEUE_LENGTH = 262144;
            MVP_SMP_NUM_SEND_BUFFER = 32;
            MVP_SMP_SEND_BUF_SIZE = 16384;
            break;

        case MVP_ARCH_INTEL_XEON_E5_2670_16:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 32768;
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_SMP_EAGERSIZE = 8192;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 32768;
            }
            MVP_SMP_QUEUE_LENGTH = 131072;
            MVP_SMP_NUM_SEND_BUFFER = 16;
            MVP_SMP_SEND_BUF_SIZE = 16384;
            break;

        case MVP_ARCH_INTEL_XEON_E5_2620_V4_2S_16:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 8192;
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_SMP_EAGERSIZE = 8192;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 32768;
            }
            MVP_SMP_QUEUE_LENGTH = 524288;
            MVP_SMP_NUM_SEND_BUFFER = 16;
            MVP_SMP_SEND_BUF_SIZE = 32768;
            break;

        case MVP_ARCH_INTEL_XEON_E5_2680_16:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 8192;
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_SMP_EAGERSIZE = 8192;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 32768;
            }
            MVP_SMP_QUEUE_LENGTH = 524288;
            MVP_SMP_NUM_SEND_BUFFER = 16;
            MVP_SMP_SEND_BUF_SIZE = 14336;
            break;

        case MVP_ARCH_INTEL_XEON_E5_2670_V2_2S_20:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 8192;
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_SMP_EAGERSIZE = 8192;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 32768;
            }
            MVP_SMP_QUEUE_LENGTH = 131072;
            MVP_SMP_NUM_SEND_BUFFER = 64;
            MVP_SMP_SEND_BUF_SIZE = 65536;
            break;

        case MVP_ARCH_ARM_CAVIUM_V8_2S_28:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 8192;
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_SMP_EAGERSIZE = 8192;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 32768;
            }
            MVP_SMP_QUEUE_LENGTH = 262144;
            MVP_SMP_NUM_SEND_BUFFER = 16;
            MVP_SMP_SEND_BUF_SIZE = 32768;
            break;

        case MVP_ARCH_ARM_CAVIUM_V8_2S_32:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 8192;
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_SMP_EAGERSIZE = 8192;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 16384;
            }
            MVP_SMP_QUEUE_LENGTH = 393216;
            MVP_SMP_NUM_SEND_BUFFER = 16;
            MVP_SMP_SEND_BUF_SIZE = 32768;
            break;

        case MVP_ARCH_ARM_FUJITSU_V0_4S_48:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 16384;
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_SMP_EAGERSIZE = 8192;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 16384;
            }
            MVP_SMP_QUEUE_LENGTH = 393216;
            MVP_SMP_NUM_SEND_BUFFER = 16;
            MVP_SMP_SEND_BUF_SIZE = 32768;
            break;

        case MVP_ARCH_INTEL_XEON_E5_2670_V3_2S_24:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 32768;
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_SMP_EAGERSIZE = 32768;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 32768;
            }
            MVP_SMP_QUEUE_LENGTH = 131072;
            MVP_SMP_NUM_SEND_BUFFER = 8;
            MVP_SMP_SEND_BUF_SIZE = 32768;
            break;

        case MVP_ARCH_INTEL_XEON_E5_2698_V3_2S_32:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 16384;
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_SMP_EAGERSIZE = 16384;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 32768;
            }
            MVP_SMP_QUEUE_LENGTH = 131072;
            MVP_SMP_NUM_SEND_BUFFER = 8;
            MVP_SMP_SEND_BUF_SIZE = 8192;
            break;

        case MVP_ARCH_INTEL_XEON_E5_2680_V3_2S_24:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 32768;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 32768;
            }
            MVP_SMP_QUEUE_LENGTH = 131072;
            MVP_SMP_NUM_SEND_BUFFER = 32;
            MVP_SMP_SEND_BUF_SIZE = 8192;
            break;

        case MVP_ARCH_INTEL_XEON_PHI_7250:
#if defined(_SMP_CMA_)
            /* Use CMA from 2 ppn onwards */
            if (MPIDI_MVP_Num_local_processes(MPIDI_Process.my_pg) <= 2) {
                MVP_SMP_USE_CMA = 0;
            }
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 65536;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 65536;
            }
            MVP_SMP_QUEUE_LENGTH = 262144;
            MVP_SMP_NUM_SEND_BUFFER = 16;
            MVP_SMP_SEND_BUF_SIZE = 131072;
            break;

        case MVP_ARCH_INTEL_XEON_E5_2690_V3_2S_24:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 32768;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 65536;
            }
            MVP_SMP_QUEUE_LENGTH = 262144;
            MVP_SMP_NUM_SEND_BUFFER = 64;
            MVP_SMP_SEND_BUF_SIZE = 8192;
            break;

        case MVP_ARCH_INTEL_XEON_E5_2690_V2_2S_20:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 131072;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 65536;
            }
            MVP_SMP_QUEUE_LENGTH = 524288;
            MVP_SMP_NUM_SEND_BUFFER = 48;
            MVP_SMP_SEND_BUF_SIZE = 32768;
            break;

        case MVP_ARCH_INTEL_XEON_E5_2687W_V3_2S_20:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 32768;
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_SMP_EAGERSIZE = 32768;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 32768;
            }
            MVP_SMP_QUEUE_LENGTH = 131072;
            MVP_SMP_NUM_SEND_BUFFER = 8;
            MVP_SMP_SEND_BUF_SIZE = 8192;
#if defined(_SMP_CMA_)
            if (MVP_ENABLE_EAGER_THRESHOLD_REDUCTION && MVP_SMP_USE_CMA &&
                10 < MPIDI_Process.my_pg->ch.num_local_processes) {
                /* if there are large number of processes per node, then
                 * reduce the eager threshold and queue length */
                MVP_SMP_EAGERSIZE = 5120;
                MVP_SMP_QUEUE_LENGTH = 32768;
            }
#endif
            break;

        case MVP_ARCH_IBM_POWER8:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 32768;
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_SMP_EAGERSIZE = 32768;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 65536;
            }
            MVP_SMP_QUEUE_LENGTH = 524288;
            MVP_SMP_NUM_SEND_BUFFER = 8;
            MVP_SMP_SEND_BUF_SIZE = 32768;
            break;

        case MVP_ARCH_IBM_POWER9:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 65536;
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_SMP_EAGERSIZE = 32768;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 32768;
            }
            MVP_SMP_QUEUE_LENGTH = 262144;
            MVP_SMP_NUM_SEND_BUFFER = 32;
            MVP_SMP_SEND_BUF_SIZE = 32768;
            break;

        case MVP_ARCH_INTEL_XEON_E5_2680_V2_2S_20:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 32768;
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_SMP_EAGERSIZE = 32768;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 65536;
            }
            MVP_SMP_QUEUE_LENGTH = 524288;
            MVP_SMP_NUM_SEND_BUFFER = 48;
            MVP_SMP_SEND_BUF_SIZE = 32768;
            break;

        case MVP_ARCH_INTEL_XEON_E5_2680_V4_2S_28:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 5120;
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_SMP_EAGERSIZE = 65536;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 65536;
            }
            MVP_SMP_QUEUE_LENGTH = 262144;
            MVP_SMP_NUM_SEND_BUFFER = 8;
            MVP_SMP_SEND_BUF_SIZE = 32768;
#if defined(_SMP_CMA_)
            if (MVP_ENABLE_EAGER_THRESHOLD_REDUCTION && MVP_SMP_USE_CMA &&
                14 < MPIDI_Process.my_pg->ch.num_local_processes) {
                /* if there are large number of processes per node, then
                 * reduce the eager threshold and queue length */
                MVP_SMP_EAGERSIZE = 5120;
                MVP_SMP_QUEUE_LENGTH = 32768;
            }
#endif
            break;

        case MVP_ARCH_INTEL_XEON_E5_2660_V3_2S_20:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 16384;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 32768;
            }
            MVP_SMP_QUEUE_LENGTH = 262144;
            MVP_SMP_NUM_SEND_BUFFER = 8;
            MVP_SMP_SEND_BUF_SIZE = 8192;
            break;

        case MVP_ARCH_INTEL_XEON_E5_2630_V2_2S_12:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 131072;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 65536;
            }
            MVP_SMP_QUEUE_LENGTH = 1048576;
            MVP_SMP_NUM_SEND_BUFFER = 8;
            MVP_SMP_SEND_BUF_SIZE = 65536;
            break;

        case MVP_ARCH_INTEL_XEON_E5_2695_V3_2S_28:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 65536;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 65536;
            }
            MVP_SMP_QUEUE_LENGTH = 262144;
            MVP_SMP_NUM_SEND_BUFFER = 16;
            MVP_SMP_SEND_BUF_SIZE = 8192;
            break;

        case MVP_ARCH_INTEL_XEON_E5_2695_V4_2S_36:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 5120;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 65536;
            }
            MVP_SMP_QUEUE_LENGTH = 262144;
            MVP_SMP_NUM_SEND_BUFFER = 32;
            MVP_SMP_SEND_BUF_SIZE = 8192;
            break;

        case MVP_ARCH_INTEL_XEON_E5_2697A_V4_2S_32:
            MVP_SMP_EAGERSIZE = 65536;
            MVP_SMP_QUEUE_LENGTH = 262144;
            MVP_SMP_NUM_SEND_BUFFER = 256;
            MVP_SMP_SEND_BUF_SIZE = 8192;
            break;

        case MVP_ARCH_INTEL_PLATINUM_8170_2S_52:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 5120;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 65536;
            }
            MVP_SMP_QUEUE_LENGTH = 262144;
            MVP_SMP_NUM_SEND_BUFFER = 32;
            MVP_SMP_SEND_BUF_SIZE = 8192;
            break;

        case MVP_ARCH_INTEL_PLATINUM_8268_2S_48:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 5120;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 65536;
            }
            MVP_SMP_QUEUE_LENGTH = 262144;
            MVP_SMP_NUM_SEND_BUFFER = 32;
            MVP_SMP_SEND_BUF_SIZE = 8192;
            break;

        case MVP_ARCH_INTEL_GOLD_6148_2S_40:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 5120;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 65536;
            }
            MVP_SMP_QUEUE_LENGTH = 262144;
            MVP_SMP_NUM_SEND_BUFFER = 32;
            MVP_SMP_SEND_BUF_SIZE = 8192;
            break;

        case MVP_ARCH_INTEL_PLATINUM_8280_2S_56:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 8192;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 8192;
            }
            MVP_SMP_QUEUE_LENGTH = 524288;
            MVP_SMP_NUM_SEND_BUFFER = 32;
            MVP_SMP_SEND_BUF_SIZE = 8192;
            break;

        case MVP_ARCH_AMD_BULLDOZER_4274HE_16:
#if defined(_SMP_CMA_)
            if (MVP_SMP_USE_CMA) {
                MVP_SMP_EAGERSIZE = 32768;
                MVP_SMP_QUEUE_LENGTH = 131072;
            } else
#endif
#if defined(_SMP_LIMIC_)
                if (MVP_SMP_USE_LIMIC2) {
                MVP_SMP_EAGERSIZE = 8192;
                MVP_SMP_QUEUE_LENGTH = 65536;
            } else
#endif
            {
                MVP_SMP_EAGERSIZE = 16384;
                MVP_SMP_QUEUE_LENGTH = 65536;
            }
            MVP_SMP_NUM_SEND_BUFFER = 16;
            MVP_SMP_SEND_BUF_SIZE = 8192;
            break;

        default:
            MVP_SMP_EAGERSIZE = 16384;
            MVP_SMP_QUEUE_LENGTH = 65536;
            MVP_SMP_NUM_SEND_BUFFER = 128;
            MVP_SMP_SEND_BUF_SIZE = 8192;
            break;
    }

#if defined(_ENABLE_CUDA_)
    if (rdma_enable_cuda && MVP_CUDA_SMP_PIPELINE) {
        MVP_SMP_SEND_BUF_SIZE = 128 * 1024;
    }

#if defined(HAVE_CUDA_IPC)
    smp_device_region_size = MVP_SMP_EAGERSIZE * 4;

    if (SMPI_CUDA_REGION_SIZE != NULL) {
        smp_device_region_size =
            user_val_to_bytes(SMPI_CUDA_REGION_SIZE, "SMPI_CUDA_REGION_SIZE");
    }

    if (smp_device_region_size <= MVP_SMP_EAGERSIZE * 4) {
        smp_device_region_size = MVP_SMP_EAGERSIZE * 4;
    }
#endif
#endif

    /* Set Limic Thresholds */
    set_limic_thresholds(proc);

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

int rdma_get_control_parameters(struct mvp_MPIDI_CH3I_RDMA_Process_t *proc)
{
    int size = -1;
    int pg_size = ERROR;
    int my_rank = -1;
    char *value = NULL;
    int mpi_errno = MPI_SUCCESS;
    int mrail_user_defined_p2r_type = 0;

    MPIR_FUNC_TERSE_STATE_DECL(MPID_STATE_RDMA_GET_CONTROL_PARAMETERS);
    MPIR_FUNC_TERSE_ENTER(MPID_STATE_RDMA_GET_CONTROL_PARAMETERS);

    proc->global_used_send_cq = 0;
    proc->global_used_recv_cq = 0;

    UPMI_GET_SIZE(&pg_size);
    UPMI_GET_RANK(&my_rank);

    mvp_use_opt_eager_recv = !!MVP_USE_OPT_EAGER_RECV;
    if (mvp_use_opt_eager_recv) {
        rdma_use_coalesce = 0;
    }

    if (MVP_NUM_NODES_IN_JOB != -1) {
        rdma_num_nodes_in_job = MVP_NUM_NODES_IN_JOB;
    } else {
        MPID_Get_max_node_id(NULL, &rdma_num_nodes_in_job);
        MVP_NUM_NODES_IN_JOB = rdma_num_nodes_in_job;
    }

#if defined(_MCST_SUPPORT_)
    if (MVP_USE_MCAST != -1) {
        rdma_enable_mcast = !!MVP_USE_MCAST;
#if defined(RDMA_CM)
        if (MVP_USE_MCAST == 0) {
            rdma_use_rdma_cm_mcast = 0;
        }
#endif /*defined(RDMA_CM)*/
    } else if (rdma_num_nodes_in_job < mcast_num_nodes_threshold) {
        /* Disable mcast by default when number of nodes less than 8 */
        MVP_USE_MCAST = 0;
#if defined(RDMA_CM)
        rdma_use_rdma_cm_mcast = 0;
#endif /*defined(RDMA_CM)*/
        PRINT_DEBUG(DEBUG_MCST_verbose,
                    "Disabling mcast by default as the number"
                    " of nodes are less than %d. Set MVP_USE_MCAST=1 or "
                    " MVP_MCAST_NUM_NODES_THRESHOLD=%d to avoid this"
                    " behavior\n",
                    mcast_num_nodes_threshold, mcast_num_nodes_threshold - 1);
    } else {
        MVP_USE_MCAST = 1;
    }
    rdma_enable_mcast = MVP_USE_MCAST;
#if defined(RDMA_CM)
    if (MVP_USE_RDMA_CM_MCAST != -1) {
        /* Set both values so that user only has to set MVP_USE_RDMA_CM_MCAST */
        MVP_USE_MCAST = rdma_enable_mcast = !!MVP_USE_RDMA_CM_MCAST;
    } else if (rdma_num_nodes_in_job < mcast_num_nodes_threshold) {
        /* Disable mcast by default when number of nodes less than 8 */
        MVP_USE_MCAST = rdma_enable_mcast = 0;
        MVP_USE_RDMA_CM_MCAST = 0;
        PRINT_DEBUG(
            DEBUG_MCST_verbose,
            "Disabling mcast by default as the number"
            " of nodes are less than %d. Set MVP_USE_RDMA_CM_MCAST=1 or "
            " MVP_MCAST_NUM_NODES_THRESHOLD=%d to avoid this"
            " behavior\n",
            mcast_num_nodes_threshold, mcast_num_nodes_threshold - 1);
    }
    rdma_use_rdma_cm_mcast = MVP_USE_RDMA_CM_MCAST;
#endif /*defined(RDMA_CM)*/
#endif /*defined(_MCST_SUPPORT_)*/

#ifdef ENABLE_QOS_SUPPORT

    rdma_use_qos = !!MVP_USE_QOS;

    if (MVP_3DTORUS_SUPPORT != -1) {
        rdma_3dtorus_support = !!MVP_3DTORUS_SUPPORT;
    } else {
        MVP_3DTORUS_SUPPORT = rdma_3dtorus_support;
    }

    rdma_path_sl_query = !!MVP_PATH_SL_QUERY;

    rdma_qos_num_sls = MVP_NUM_SLS;
    if (rdma_qos_num_sls <= 0 && rdma_qos_num_sls > RDMA_QOS_MAX_NUM_SLS) {
        rdma_qos_num_sls = RDMA_QOS_DEFAULT_NUM_SLS;
    }
    /* User asked us to use multiple SL's without enabling QoS globally. */
    if (rdma_use_qos == 0) {
        rdma_use_qos = 1;
    }
#endif /* ENABLE_QOS_SUPPORT */

    rdma_num_sa_query_retries = MVP_NUM_SA_QUERY_RETRIES;
    if (rdma_num_sa_query_retries < RDMA_DEFAULT_NUM_SA_QUERY_RETRIES) {
        rdma_num_sa_query_retries = RDMA_DEFAULT_NUM_SA_QUERY_RETRIES;
    }

    if (MVP_MAX_RDMA_CONNECT_ATTEMPTS <= 0) {
        MVP_MAX_RDMA_CONNECT_ATTEMPTS = DEFAULT_RDMA_CONNECT_ATTEMPTS;
    }
    max_rdma_connect_attempts = MVP_MAX_RDMA_CONNECT_ATTEMPTS;

    if (MVP_RDMA_CM_CONNECT_RETRY_INTERVAL <= 0) {
        MVP_RDMA_CM_CONNECT_RETRY_INTERVAL = RDMA_DEFAULT_CONNECT_INTERVAL;
    }
    rdma_cm_connect_retry_interval = MVP_RDMA_CM_CONNECT_RETRY_INTERVAL;

    /* Parameters to decide the p2r mapping
     * The check for this parameter should always be done before we check for
     * MVP_SM_SCHEDULING below as we find out if the user has specified a
     * mapping for the user defined scheme to take effect */
    if (MVP_PROCESS_TO_RAIL_MAPPING != NULL) {
        mrail_p2r_length = strlen(MVP_PROCESS_TO_RAIL_MAPPING);

        mrail_p2r_string =
            (char *)MPL_malloc(mrail_p2r_length * sizeof(char), MPL_MEM_OTHER);

        strcpy(mrail_p2r_string, MVP_PROCESS_TO_RAIL_MAPPING);
        mrail_p2r_string[mrail_p2r_length] = '\0';
        if (!strcmp(MVP_PROCESS_TO_RAIL_MAPPING, "BUNCH")) {
            mrail_user_defined_p2r_type = MVP_UDEF_POLICY_BUNCH;
        } else if (!strcmp(MVP_PROCESS_TO_RAIL_MAPPING, "SCATTER")) {
            mrail_user_defined_p2r_type = MVP_UDEF_POLICY_SCATTER;
        } else {
            mrail_user_defined_p2r_type = MVP_UDEF_POLICY_NONE;
        }
        rdma_get_process_to_rail_mapping(mrail_user_defined_p2r_type);
    } else {
        mrail_use_default_mapping = 1;
    }

#ifdef _ENABLE_HSAM_
    /* Start HSAM Parameters */
    if (MVP_USE_HSAM != -1) {
        proc->has_hsam = MVP_USE_HSAM;
        if (proc->has_hsam) {
            check_hsam_parameters();
        }
    } else
#endif /*_ENABLE_HSAM_*/
    {
        /* By default disable the HSAM, due to problem with
         * multi-pathing with current version of opensm and
         * up/down */
        proc->has_hsam = 0;
    }
    /* End : HSAM Parameters */
    if (MVP_USE_APM != -1) {
        proc->has_apm = MVP_USE_APM;
    } else {
        proc->has_apm = 0;
    }
    if (MVP_USE_APM_TEST != -1) {
        apm_tester = MVP_USE_APM_TEST;
    } else {
        apm_tester = 0;
    }

    /* Scheduling Parameters */
    if (MVP_SM_SCHEDULING != NULL) {
        rdma_multirail_usage_policy = MVP_MRAIL_SHARING;
        rdma_rail_sharing_policy =
            rdma_get_rail_sharing_policy(MVP_SM_SCHEDULING);
    }

    if (MVP_SMALL_MSG_RAIL_SHARING_POLICY != NULL) {
        rdma_multirail_usage_policy = MVP_MRAIL_SHARING;
        rdma_small_msg_rail_sharing_policy =
            rdma_get_rail_sharing_policy(MVP_SMALL_MSG_RAIL_SHARING_POLICY);
    }

    if (MVP_MED_MSG_RAIL_SHARING_POLICY != NULL) {
        rdma_multirail_usage_policy = MVP_MRAIL_SHARING;
        rdma_med_msg_rail_sharing_policy =
            rdma_get_rail_sharing_policy(MVP_MED_MSG_RAIL_SHARING_POLICY);
    }

    if (MVP_RAIL_SHARING_POLICY != NULL) {
        rdma_multirail_usage_policy = MVP_MRAIL_SHARING;
        rdma_rail_sharing_policy = rdma_med_msg_rail_sharing_policy =
            rdma_small_msg_rail_sharing_policy =
                rdma_get_rail_sharing_policy(MVP_RAIL_SHARING_POLICY);
    }
#if defined(RDMA_CM)
    if (MVP_USE_IWARP_MODE != -1) {
        proc->use_rdma_cm = !!MVP_USE_IWARP_MODE;
        proc->use_iwarp_mode = !!MVP_USE_IWARP_MODE;
    }

    if (!proc->use_rdma_cm) {
        if (MVP_USE_RDMA_CM != -1) {
#if !defined(ROMIO_IME)
            proc->use_rdma_cm = !!MVP_USE_RDMA_CM;
#else
            if (MVP_USE_RDMA_CM && (my_rank == 0)) {
                MPL_error_printf("Error: IME FS does not work with RDMA CM. "
                                 "Proceeding without RDMA support.\n");
            }
#endif
        } else {
            proc->use_rdma_cm = 0;
            proc->use_iwarp_mode = 0;
        }
    }

#ifdef _MULTI_SUBNET_SUPPORT_
    mvp_rdma_cm_multi_subnet_support = !!MVP_RDMA_CM_MULTI_SUBNET_SUPPORT;
    if (proc->use_rdma_cm == 0) {
        proc->use_rdma_cm = mvp_rdma_cm_multi_subnet_support;
    }
#endif /* _MULTI_SUBNET_SUPPORT_ */

    if (MVP_CVAR_IS_SET_BY_USER(MVP_SUPPORT_DPM) && MVP_SUPPORT_DPM) {
        proc->use_rdma_cm = 0;
        proc->use_iwarp_mode = 0;
        MVP_USE_EAGER_FAST_SEND = 0;
        mvp_on_demand_ud_info_exchange = 0; /* Trac #780 */
    }

    if (proc->use_rdma_cm) {
        int threshold = ERROR;

        if (proc->use_iwarp_mode) {
            /* Trac #423 */
            threshold = MPIDI_CH3I_CM_DEFAULT_IWARP_ON_DEMAND_THRESHOLD;
        } else {
            threshold = MPIDI_CH3I_RDMA_CM_DEFAULT_ON_DEMAND_THRESHOLD;
        }

        /* TODO: Cannot set the cvar here because check for defaults again
         * in CH3_Init. We need to adjust this logic so it does not cause
         * issues */
        if (MVP_ON_DEMAND_THRESHOLD != -1) {
            threshold = MVP_ON_DEMAND_THRESHOLD;
        }
        if (pg_size > threshold) {
            proc->use_rdma_cm_on_demand = 1;
            mvp_on_demand_ud_info_exchange = 1;
            proc->has_ring_startup = 0;
        }
#ifdef _ENABLE_XRC_
        /* XRC will not work with RDMA_CM */
        USE_XRC = 0;
        if ((MVP_USE_XRC != -1) && (my_rank == 0)) {
            if (MVP_USE_XRC) {
                MPL_error_printf("Error: XRC does not work with RDMA CM. "
                                 "Proceeding without XRC support.\n");
            }
        }
#endif
    }
#endif

    /* If there is only one process per node allow it use
     * all HCAs. The code to force rail selection to MVP_MRAIL_SHARING must
     * come before opening the HCAs. */
    if (rdma_num_nodes_in_job == pg_size) {
        rdma_multirail_usage_policy = MVP_MRAIL_SHARING;
    }

    if ((mpi_errno = rdma_open_hca(proc)) != MPI_SUCCESS) {
        MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**fail",
                                  "**fail %s", "rdma_open_hca");
    }

    /* Heterogeniety detection for HCAs */
    int i = 0;
    int heterogeneous = 0;
    uint64_t node_guid[MAX_NUM_HCAS];
    mvp_hca_type hca_type[MAX_NUM_HCAS];

    for (i = 0; i < rdma_num_hcas; ++i) {
        hca_type[i] = mvp_new_get_hca_type(proc->nic_context[i],
                                           proc->ib_dev[i], &node_guid[i]);
        if (i && (hca_type[i] != hca_type[i - 1])) {
            heterogeneous = 1;
        }
    }
    proc->node_guid = node_guid[0];

    if (heterogeneous) {
        proc->hca_type = MVP_HCA_UNKWN;
    } else {
        proc->hca_type = hca_type[0];
    }

    proc->arch_hca_type = mvp_new_get_arch_hca_type(proc->hca_type);
    proc->arch_type = MVP_GET_ARCH(proc->arch_hca_type);

    /* If there is only one process per node, or just one node allow it use all
     * HCAs */
    if ((rdma_num_nodes_in_job == pg_size) || (rdma_num_nodes_in_job == 1)) {
        /* For AMD EPYC systems that have PCIe Gen4 interfaces, we see that
         * using RDMA_READ primitive is not able to saturate the network
         * bandwidth on HDR200 adapters probably due to limited number of
         * outstanding RDMA_READ operation they allow in flight (currently 16).
         * This check will allow us to fall back to RDMA_WRITE protocol for
         * these cases */
        if ((MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7551_64,
                                  MVP_HCA_MLX_CX_HDR)) ||
            (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7763_128,
                                  MVP_HCA_MLX_CX_HDR)) ||
            (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7601_64,
                                  MVP_HCA_MLX_CX_HDR)) ||
            (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7V12_64,
                                  MVP_HCA_MLX_CX_HDR)) ||
            (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7401_48,
                                  MVP_HCA_MLX_CX_HDR)) ||
            (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7742_128,
                                  MVP_HCA_MLX_CX_HDR))) {
            MVP_RNDV_PROTOCOL = MVP_RNDV_PROTOCOL_RPUT;
        }
    }

    if (rdma_num_nodes_in_job == 0) {
        UPMI_GET_SIZE(&size);
    } else {
        size = rdma_num_nodes_in_job;
    }

    if (size <= 8) {
        proc->cluster_size = VERY_SMALL_CLUSTER;
    } else if (size <= 32) {
        proc->cluster_size = SMALL_CLUSTER;
    } else if (size < 128) {
        proc->cluster_size = MEDIUM_CLUSTER;
    } else {
        proc->cluster_size = LARGE_CLUSTER;
    }

#ifdef _ENABLE_UD_
    if (pg_size < 512) {
        rdma_ud_progress_timeout = 48000;
        rdma_ud_retry_timeout = 500000;
        rdma_ud_max_retry_count = 1024;
        rdma_hybrid_max_rc_conn = 32;
    } else if (pg_size < 1024) {
        rdma_ud_progress_timeout = 48000;
        rdma_ud_retry_timeout = 500000;
        rdma_ud_max_retry_count = 1024;
        rdma_hybrid_max_rc_conn = 64;
    } else if (pg_size < 4096) {
        rdma_ud_progress_timeout = 96000;
        rdma_ud_retry_timeout = 1000000;
        rdma_ud_max_retry_count = 512;
        rdma_hybrid_max_rc_conn = 64;
    } else if (pg_size < 8192) {
        rdma_ud_progress_timeout = 96000;
        rdma_ud_retry_timeout = 1000000;
        rdma_ud_max_retry_count = 512;
        rdma_hybrid_max_rc_conn = 128;
    } else {
        rdma_ud_progress_timeout = 190000;
        rdma_ud_retry_timeout = 2000000;
        rdma_ud_max_retry_count = 256;
        rdma_hybrid_max_rc_conn = 128;
    }
#endif

    proc->has_srq = !!MVP_USE_SRQ;

    rdma_iwarp_multiple_cq_threshold = MVP_IWARP_MULTIPLE_CQ_THRESHOLD;
    if (rdma_iwarp_multiple_cq_threshold < 0) {
        rdma_iwarp_multiple_cq_threshold =
            RDMA_IWARP_DEFAULT_MULTIPLE_CQ_THRESHOLD;
    }

    if (size > rdma_iwarp_multiple_cq_threshold) {
        rdma_iwarp_use_multiple_cq = 1;
    }
#ifdef _ENABLE_XRC_
    if (USE_XRC) {
        proc->has_srq = 1;
        MPIR_Assert(MPIDI_CH3I_Process.cm_type == MPIDI_CH3I_CM_ON_DEMAND);
        MPIDI_CH3I_Process.cm_type = MPIDI_CH3I_CM_ON_DEMAND;
        rdma_use_coalesce = 0;
        rdma_use_blocking = 0;
    }
#endif /* _ENABLE_XRC_ */

    if (proc->has_srq && proc->hca_type != MVP_HCA_QLGIC_PATH_HT &&
        proc->hca_type != MVP_HCA_QLGIC_QIB &&
        proc->hca_type != MVP_HCA_MLX_PCI_X &&
        proc->hca_type != MVP_HCA_IBM_EHCA &&
        proc->hca_type != MVP_HCA_MARVEL_QEDR
#if defined(RDMA_CM)
        && !proc->use_iwarp_mode
#endif /* defined(RDMA_CM) */
    ) {
        mvp_use_post_srq_send = 1;
    } else {
        proc->has_srq = 0;
        mvp_use_post_srq_send = 0;
    }

#if !defined(DISABLE_PTMALLOC)

    proc->has_lazy_mem_unregister = !!MVP_USE_LAZY_MEM_UNREGISTER;

#endif /* !defined(DISABLE_PTMALLOC) */

    proc->enable_rma_fast_path = !!MVP_USE_RMA_FAST_PATH;
    proc->force_ib_atomic = !!MVP_FORCE_IB_ATOMIC;

#if defined(CKPT)
    {
        proc->has_one_sided = 0;
    }
#else /* defined(CKPT) */
#ifdef _ENABLE_UD_
    if (MVP_USE_UD_HYBRID) {
        proc->has_one_sided = 0;
    } else
#endif
#ifdef _ENABLE_CUDA_
        if (mvp_enable_device) {
        proc->has_one_sided = 0;
    } else
#endif
    {

        proc->has_one_sided = !!MVP_USE_RDMA_ONE_SIDED;
    }
#endif /* defined(CKPT) */

    rdma_rndv_ext_sendq_size = MVP_RNDV_EXT_SENDQ_SIZE;
    if (rdma_rndv_ext_sendq_size <= 1) {
        MPL_usage_printf("Setting MVP_RNDV_EXT_SENDQ_SIZE smaller than 1 "
                         "will severely limit the MPI bandwidth.\n");
    }

    rdma_num_extra_polls = MVP_RDMA_NUM_EXTRA_POLLS;
    if (rdma_num_extra_polls <= 0) {
        rdma_num_extra_polls = 1;
    }

    rdma_coalesce_threshold = MVP_COALESCE_THRESHOLD;
    if (rdma_coalesce_threshold < 1) {
        MPL_usage_printf("MVP_COALESCE_THRESHOLD must be >= 1\n");
        rdma_coalesce_threshold = 1;
    }

    if (proc->hca_type == MVP_HCA_MLX_CX_SDR ||
        proc->hca_type == MVP_HCA_MLX_CX_DDR) {
        rdma_use_coalesce = 0;
    }
#ifdef _ENABLE_XRC_
    if (!USE_XRC) {
#endif
        rdma_use_coalesce = !!MVP_USE_COALESCE;

        if (rdma_use_coalesce && mvp_use_opt_eager_recv) {
            PRINT_INFO((my_rank == 0),
                       "Optimized eager recv is"
                       " not compatible with message coalescing. Disabling"
                       " optimized eager recv and continuing\n");
            mvp_use_opt_eager_recv = 0;
        }
#ifdef _ENABLE_XRC_
    }
#endif

    if (MVP_SPIN_COUNT < 1) {
        MVP_SPIN_COUNT = DEFAULT_SPIN_COUNT;
    }
    rdma_blocking_spin_count_threshold = MVP_SPIN_COUNT;

    if (MVP_RNDV_PROTOCOL_RPUT != MVP_RNDV_PROTOCOL &&
        MVP_RNDV_PROTOCOL_R3 != MVP_RNDV_PROTOCOL) {
        if (MVP_RNDV_PROTOCOL_RGET == MVP_RNDV_PROTOCOL
#ifdef _ENABLE_XRC_
            && !USE_XRC
#endif
        ) {
#if defined(CKPT)
            MPL_usage_printf("MVP_RNDV_PROTOCOL "
                             "must be either \"RPUT\" or \"R3\" when "
                             "checkpoint is enabled\n");
            MVP_RNDV_PROTOCOL = MVP_RNDV_PROTOCOL_RPUT;
#endif /* defined(CKPT) */
        } else {
#ifdef _ENABLE_XRC_
            if (!USE_XRC)
#endif
                MPL_usage_printf(
                    "MVP_RNDV_PROTOCOL "
                    "must be either \"RPUT\", \"RGET\", or \"R3\"\n");
            MVP_RNDV_PROTOCOL = MVP_RNDV_PROTOCOL_RPUT;
        }
    }

    switch (MVP_SMP_RNDV_PROTOCOL) {
        default:
            MPL_usage_printf("MVP_SMP_RNDV_PROTOCOL "
                             "must be one of: RPUT, RGET, R3\n");
            MVP_SMP_RNDV_PROTOCOL = MVP_RNDV_PROTOCOL;
        case MVP_SMP_RNDV_PROTOCOL_RPUT:
        case MVP_SMP_RNDV_PROTOCOL_RGET:
        case MVP_SMP_RNDV_PROTOCOL_R3:;
    }

    rdma_r3_threshold = MVP_R3_THRESHOLD;
    if (rdma_r3_threshold < 0) {
        rdma_r3_threshold = 0;
    }

    rdma_intra_node_r3_threshold = MVP_INTRA_NODE_R3_THRESHOLD;
    if (rdma_intra_node_r3_threshold <= 0) {
        rdma_intra_node_r3_threshold = RDMA_DEFAULT_R3_THRESHOLD;
    }

    rdma_inter_node_r3_threshold = MVP_INTER_NODE_R3_THRESHOLD;
    if (rdma_inter_node_r3_threshold <= 0) {
        rdma_inter_node_r3_threshold = RDMA_DEFAULT_R3_THRESHOLD;
    }

    rdma_r3_threshold_nocache = MVP_R3_NOCACHE_THRESHOLD;
    if (rdma_r3_threshold_nocache < 0) {
        rdma_r3_threshold_nocache = 0;
    }

    rdma_max_r3_pending_data = MVP_MAX_R3_PENDING_DATA;
    if (rdma_max_r3_pending_data < 0) {
        rdma_max_r3_pending_data = 0;
    }

#if defined(RDMA_CM)
    if (proc->use_rdma_cm_on_demand) {
        proc->use_iwarp_mode = 1;
    }
#endif /* defined(RDMA_CM) */

fn_exit:
    MPIR_FUNC_TERSE_EXIT(MPID_STATE_RDMA_GET_CONTROL_PARAMETERS);
    return mpi_errno;

fn_fail:
    goto fn_exit;
}

/* Set params based on cluster size */
static void rdma_set_params_based_on_cluster_size(
    int cluster_size, int lc_vbuf_total_size, int lc_num_rdma_buff,
    int mc_vbuf_total_size, int mc_num_rdma_buff, int sc_vbuf_total_size,
    int sc_num_rdma_buff, int vsc_vbuf_total_size, int vsc_num_rdma_buff,
    int def_vbuf_total_size, int def_num_rdma_buff)
{
    switch (cluster_size) {
        case LARGE_CLUSTER:
            rdma_vbuf_total_size = lc_vbuf_total_size + EAGER_THRESHOLD_ADJUST;
            rdma_fp_buffer_size = 4 * 1024;
            num_rdma_buffer = lc_num_rdma_buff;
            rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
            break;

        case MEDIUM_CLUSTER:
            rdma_vbuf_total_size = mc_vbuf_total_size + EAGER_THRESHOLD_ADJUST;
            rdma_fp_buffer_size = 4 * 1024;
            num_rdma_buffer = mc_num_rdma_buff;
            rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
            break;
        case SMALL_CLUSTER:
            rdma_vbuf_total_size = sc_vbuf_total_size + EAGER_THRESHOLD_ADJUST;
            rdma_fp_buffer_size = 4 * 1024;
            num_rdma_buffer = sc_num_rdma_buff;
            rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
            break;
        case VERY_SMALL_CLUSTER:
            rdma_vbuf_total_size = vsc_vbuf_total_size + EAGER_THRESHOLD_ADJUST;
            rdma_fp_buffer_size = 4 * 1024;
            num_rdma_buffer = vsc_num_rdma_buff;
            rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
            break;
        default:
            rdma_vbuf_total_size = def_vbuf_total_size + EAGER_THRESHOLD_ADJUST;
            rdma_fp_buffer_size = 4 * 1024;
            num_rdma_buffer = def_num_rdma_buff;
            rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
            break;
    }
}

/* Set thresholds for Nnum_rail=4 */
static void rdma_set_default_parameters_numrail_4(
    struct mvp_MPIDI_CH3I_RDMA_Process_t *proc)
{
    if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_INTEL_XEON_E5630_8,
                             MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2670_16,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 4 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2630_V2_2S_12,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 20 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2698_V3_2S_32,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 20 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_PHI_7250,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 20 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_PLATINUM_8280_2S_56,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V3_2S_24,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 20 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2690_V3_2S_24,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 20 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2690_V2_2S_20,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 64 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V2_2S_20,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V4_2S_28,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2687W_V3_2S_20,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2660_V3_2S_20,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_IBM_POWER8,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 32 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_IBM_POWER9,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 32 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V2_2S_20,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V4_2S_28,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2687W_V3_2S_20,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V3_2S_24,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2690_V3_2S_24,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2660_V3_2S_20,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_IBM_POWER8,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 32 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_IBM_POWER9,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 32 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V2_2S_20,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V4_2S_28,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2687W_V3_2S_20,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V3_2S_24,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2690_V3_2S_24,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2660_V3_2S_20,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2670_V2_2S_20,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_ARM_CAVIUM_V8_2S_28,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_ARM_CAVIUM_V8_2S_32,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_ARM_FUJITSU_V0_4S_48,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2670_V3_2S_24,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2698_V3_2S_32,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_PHI_7250,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 20 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2687W_V3_2S_20,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V3_2S_24,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2690_V3_2S_24,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2660_V3_2S_20,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_16,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_BULLDOZER_4274HE_16,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_CLOVERTOWN_8,
                                  MVP_HCA_MLX_CX_DDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_OPTERON_6136_32,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_BARCELONA_16,
                                  MVP_HCA_MLX_CX_SDR)) {
        rdma_vbuf_total_size = 12 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7551_64,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7551_64,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7662_64,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 12 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7662_64,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 32 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7763_128,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 12 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7763_128,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 32 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7601_64,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 13 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7V12_64,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 24 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7401_48,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 13 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7401_48,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 24 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7742_128,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    /* same tuning value for rome with EDR HCA */
    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7742_128,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_X5650_12,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 19 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_INTEL_NEHALEM_8,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2670_16,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2670_16,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_INTEL_GENERIC,
                                  MVP_HCA_BROADCOM_BNXTRE)) {
        rdma_vbuf_total_size = 10 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_BROADCOM_BNXTRE)) {
        rdma_vbuf_total_size = 10 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_MAGNY_COURS_24,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 4 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_BARCELONA_16,
                                  MVP_HCA_MLX_CX_DDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 9 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 128;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_HARPERTOWN_8,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 9 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_MLX_CX_DDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 9 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_IBM_EHCA) ||
             MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_MLX_PCI_X)) {
        rdma_set_params_based_on_cluster_size(
            proc->cluster_size, 2 * 1024, 4, /* Values for large cluster size */
            4 * 1024, 16,   /* Values for medium cluster size */
            12 * 1024, 32,  /* Values for small cluster size */
            12 * 1024, 32,  /* Values for very small cluster size */
            12 * 1024, 32); /* Values for unknown cluster size */
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 394 * 1024;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_CHELSIO_T3) ||
             MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_CHELSIO_T4)) {
        rdma_set_params_based_on_cluster_size(
            proc->cluster_size, 2 * 1024, 4, /* Values for large cluster size */
            4 * 1024, 8,    /* Values for medium cluster size */
            9 * 1024, 16,   /* Values for small cluster size */
            32 * 1024, 16,  /* Values for very small cluster size */
            32 * 1024, 16); /* Values for unknown cluster size */
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 64;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_INTEL_NE020)) {
        rdma_set_params_based_on_cluster_size(
            proc->cluster_size, 2 * 1024, 4, /* Values for large cluster size */
            4 * 1024, 8,    /* Values for medium cluster size */
            9 * 1024, 16,   /* Values for small cluster size */
            32 * 1024, 16,  /* Values for very small cluster size */
            32 * 1024, 16); /* Values for unknown cluster size */
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 394 * 1024;
    }

    else {
        rdma_set_params_based_on_cluster_size(
            proc->cluster_size, 16 * 1024,
            16,             /* Values for large cluster size */
            16 * 1024, 16,  /* Values for medium cluster size */
            16 * 1024, 16,  /* Values for small cluster size */
            16 * 1024, 16,  /* Values for very small cluster size */
            16 * 1024, 16); /* Values for unknown cluster size */
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 256 * 1024;
        rdma_fp_buffer_size = RDMA_FP_DEFAULT_BUF_SIZE;
    }
}

/* Set thresholds for Nnum_rail=3 */
static void rdma_set_default_parameters_numrail_3(
    struct mvp_MPIDI_CH3I_RDMA_Process_t *proc)
{
    if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_INTEL_XEON_E5630_8,
                             MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2670_16,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 4 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2630_V2_2S_12,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 20 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2698_V3_2S_32,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 20 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_PHI_7250,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 20 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_PLATINUM_8280_2S_56,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V3_2S_24,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 20 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2690_V3_2S_24,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 20 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2690_V2_2S_20,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 64 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V2_2S_20,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V4_2S_28,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2687W_V3_2S_20,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2660_V3_2S_20,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_IBM_POWER8,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 32 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_IBM_POWER9,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 32 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V2_2S_20,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V4_2S_28,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2687W_V3_2S_20,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V3_2S_24,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2690_V3_2S_24,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2660_V3_2S_20,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_IBM_POWER8,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 32 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_IBM_POWER9,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 32 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V2_2S_20,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V4_2S_28,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2620_V4_2S_16,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 9 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2687W_V3_2S_20,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V3_2S_24,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2690_V3_2S_24,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2660_V3_2S_20,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2670_V2_2S_20,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_ARM_CAVIUM_V8_2S_28,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_ARM_CAVIUM_V8_2S_32,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_ARM_FUJITSU_V0_4S_48,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2670_V3_2S_24,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2698_V3_2S_32,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_PHI_7250,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V3_2S_24,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2687W_V3_2S_20,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2690_V3_2S_24,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2660_V3_2S_20,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_16,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_BULLDOZER_4274HE_16,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_OPTERON_6136_32,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_CLOVERTOWN_8,
                                  MVP_HCA_MLX_CX_DDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_BARCELONA_16,
                                  MVP_HCA_MLX_CX_SDR)) {
        rdma_vbuf_total_size = 12 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7551_64,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7551_64,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7662_64,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 12 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7662_64,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 32 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7763_128,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 12 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7763_128,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 32 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7601_64,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 13 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7V12_64,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 24 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7401_48,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 13 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7401_48,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 24 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7742_128,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    /* same tuning value for rome with EDR HCA */
    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7742_128,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_X5650_12,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 19 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_INTEL_NEHALEM_8,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2670_16,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2670_16,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_INTEL_GENERIC,
                                  MVP_HCA_BROADCOM_BNXTRE)) {
        rdma_vbuf_total_size = 10 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_BROADCOM_BNXTRE)) {
        rdma_vbuf_total_size = 10 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_MAGNY_COURS_24,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 4 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_BARCELONA_16,
                                  MVP_HCA_MLX_CX_DDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 9 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 128;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_HARPERTOWN_8,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 9 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_MLX_CX_DDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 9 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_IBM_EHCA) ||
             MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_MLX_PCI_X)) {
        rdma_set_params_based_on_cluster_size(
            proc->cluster_size, 2 * 1024, 4, /* Values for large cluster size */
            4 * 1024, 16,   /* Values for medium cluster size */
            12 * 1024, 32,  /* Values for small cluster size */
            12 * 1024, 32,  /* Values for very small cluster size */
            12 * 1024, 32); /* Values for unknown cluster size */
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 394 * 1024;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_CHELSIO_T3) ||
             MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_CHELSIO_T4)) {
        rdma_set_params_based_on_cluster_size(
            proc->cluster_size, 2 * 1024, 4, /* Values for large cluster size */
            4 * 1024, 8,    /* Values for medium cluster size */
            9 * 1024, 16,   /* Values for small cluster size */
            32 * 1024, 16,  /* Values for very small cluster size */
            32 * 1024, 16); /* Values for unknown cluster size */
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 64;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_INTEL_NE020)) {
        rdma_set_params_based_on_cluster_size(
            proc->cluster_size, 2 * 1024, 4, /* Values for large cluster size */
            4 * 1024, 8,    /* Values for medium cluster size */
            9 * 1024, 16,   /* Values for small cluster size */
            32 * 1024, 16,  /* Values for very small cluster size */
            32 * 1024, 16); /* Values for unknown cluster size */
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 394 * 1024;
    }

    else {
        rdma_set_params_based_on_cluster_size(
            proc->cluster_size, 16 * 1024,
            16,             /* Values for large cluster size */
            16 * 1024, 16,  /* Values for medium cluster size */
            16 * 1024, 16,  /* Values for small cluster size */
            16 * 1024, 16,  /* Values for very small cluster size */
            16 * 1024, 16); /* Values for unknown cluster size */
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 256 * 1024;
        rdma_fp_buffer_size = RDMA_FP_DEFAULT_BUF_SIZE;
    }
}

/* Set thresholds for Nnum_rail=2 */
static void rdma_set_default_parameters_numrail_2(
    struct mvp_MPIDI_CH3I_RDMA_Process_t *proc)
{
    if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_INTEL_XEON_E5630_8,
                             MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2670_16,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 4 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2630_V2_2S_12,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 20 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2698_V3_2S_32,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 20 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_PHI_7250,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 20 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_PLATINUM_8280_2S_56,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V3_2S_24,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 20 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2690_V3_2S_24,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 20 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2690_V2_2S_20,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 64 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V2_2S_20,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V4_2S_28,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    } else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                    MVP_ARCH_INTEL_XEON_E5_2687W_V3_2S_20,
                                    MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2660_V3_2S_20,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_IBM_POWER8,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 32 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_IBM_POWER9,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 32 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V2_2S_20,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V4_2S_28,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2687W_V3_2S_20,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2620_V4_2S_16,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 9 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V3_2S_24,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2690_V3_2S_24,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2660_V3_2S_20,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_IBM_POWER8,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 32 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_IBM_POWER9,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 32 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V2_2S_20,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V4_2S_28,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2687W_V3_2S_20,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V3_2S_24,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2690_V3_2S_24,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2660_V3_2S_20,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2670_V2_2S_20,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_ARM_CAVIUM_V8_2S_28,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_ARM_CAVIUM_V8_2S_32,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_ARM_FUJITSU_V0_4S_48,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2670_V3_2S_24,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2698_V3_2S_32,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_PHI_7250,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V3_2S_24,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2687W_V3_2S_20,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2690_V3_2S_24,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2660_V3_2S_20,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_16,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_BULLDOZER_4274HE_16,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_OPTERON_6136_32,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_CLOVERTOWN_8,
                                  MVP_HCA_MLX_CX_DDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_BARCELONA_16,
                                  MVP_HCA_MLX_CX_SDR)) {
        rdma_vbuf_total_size = 12 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7551_64,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7551_64,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7662_64,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 12 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7662_64,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 32 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7763_128,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 12 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7763_128,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 32 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7601_64,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 13 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7V12_64,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 24 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7401_48,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 13 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7401_48,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 24 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7742_128,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    /* same tuning value for rome with EDR HCA */
    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7742_128,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_X5650_12,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 19 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_INTEL_NEHALEM_8,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2670_16,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2670_16,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_INTEL_GENERIC,
                                  MVP_HCA_BROADCOM_BNXTRE)) {
        rdma_vbuf_total_size = 10 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_BROADCOM_BNXTRE)) {
        rdma_vbuf_total_size = 10 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_MAGNY_COURS_24,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 4 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_BARCELONA_16,
                                  MVP_HCA_MLX_CX_DDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 9 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 128;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_HARPERTOWN_8,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 9 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_MLX_CX_DDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 9 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_IBM_EHCA) ||
             MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_MLX_PCI_X)) {
        rdma_set_params_based_on_cluster_size(
            proc->cluster_size, 2 * 1024, 4, /* Values for large cluster size */
            4 * 1024, 16,   /* Values for medium cluster size */
            12 * 1024, 32,  /* Values for small cluster size */
            12 * 1024, 32,  /* Values for very small cluster size */
            12 * 1024, 32); /* Values for unknown cluster size */
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 394 * 1024;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_CHELSIO_T3) ||
             MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_CHELSIO_T4)) {
        rdma_set_params_based_on_cluster_size(
            proc->cluster_size, 2 * 1024, 4, /* Values for large cluster size */
            4 * 1024, 8,    /* Values for medium cluster size */
            9 * 1024, 16,   /* Values for small cluster size */
            32 * 1024, 16,  /* Values for very small cluster size */
            32 * 1024, 16); /* Values for unknown cluster size */
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 64;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_INTEL_NE020)) {
        rdma_set_params_based_on_cluster_size(
            proc->cluster_size, 2 * 1024, 4, /* Values for large cluster size */
            4 * 1024, 8,    /* Values for medium cluster size */
            9 * 1024, 16,   /* Values for small cluster size */
            32 * 1024, 16,  /* Values for very small cluster size */
            32 * 1024, 16); /* Values for unknown cluster size */
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 394 * 1024;
    }

    else {
        rdma_set_params_based_on_cluster_size(
            proc->cluster_size, 16 * 1024,
            16,             /* Values for large cluster size */
            16 * 1024, 16,  /* Values for medium cluster size */
            16 * 1024, 16,  /* Values for small cluster size */
            16 * 1024, 16,  /* Values for very small cluster size */
            16 * 1024, 16); /* Values for unknown cluster size */
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 256 * 1024;
        rdma_fp_buffer_size = RDMA_FP_DEFAULT_BUF_SIZE;
    }
}

/* Set thresholds for Nnum_rail=1 */
static void rdma_set_default_parameters_numrail_1(
    struct mvp_MPIDI_CH3I_RDMA_Process_t *proc)
{
    if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_INTEL_XEON_E5630_8,
                             MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2670_16,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 4 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2630_V2_2S_12,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 20 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2698_V3_2S_32,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 20 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_PHI_7250,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 20 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_PLATINUM_8280_2S_56,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V3_2S_24,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 20 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2690_V3_2S_24,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 20 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2690_V2_2S_20,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 64 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V2_2S_20,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V4_2S_28,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    } else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                    MVP_ARCH_INTEL_XEON_E5_2687W_V3_2S_20,
                                    MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2660_V3_2S_20,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_IBM_POWER8,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 32 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_IBM_POWER9,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 32 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V2_2S_20,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V4_2S_28,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2687W_V3_2S_20,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V3_2S_24,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2690_V3_2S_24,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2660_V3_2S_20,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_IBM_POWER8,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 32 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_IBM_POWER9,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 32 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V2_2S_20,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V4_2S_28,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2620_V4_2S_16,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 9 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2687W_V3_2S_20,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V3_2S_24,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2690_V3_2S_24,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2660_V3_2S_20,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2670_V2_2S_20,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_ARM_CAVIUM_V8_2S_28,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_ARM_CAVIUM_V8_2S_32,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_ARM_FUJITSU_V0_4S_48,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2670_V3_2S_24,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2698_V3_2S_32,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_PHI_7250,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V3_2S_24,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2690_V3_2S_24,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2687W_V3_2S_20,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2660_V3_2S_20,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_16,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_BULLDOZER_4274HE_16,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_OPTERON_6136_32,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_CLOVERTOWN_8,
                                  MVP_HCA_MLX_CX_DDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_BARCELONA_16,
                                  MVP_HCA_MLX_CX_SDR)) {
        rdma_vbuf_total_size = 12 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7551_64,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7551_64,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7662_64,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 12 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7662_64,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 32 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7763_128,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 12 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7763_128,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 32 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7601_64,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 13 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7V12_64,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 24 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7401_48,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 13 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7401_48,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 13 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7742_128,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    /* same tuning value for rome with EDR HCA */
    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7742_128,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_X5650_12,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 19 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_INTEL_NEHALEM_8,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2670_16,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2670_16,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_INTEL_GENERIC,
                                  MVP_HCA_BROADCOM_BNXTRE)) {
        rdma_vbuf_total_size = 10 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_BROADCOM_BNXTRE)) {
        rdma_vbuf_total_size = 10 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_MAGNY_COURS_24,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 4 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_BARCELONA_16,
                                  MVP_HCA_MLX_CX_DDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 9 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 128;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_HARPERTOWN_8,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 9 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_MLX_CX_DDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 9 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_IBM_EHCA) ||
             MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_MLX_PCI_X)) {
        rdma_set_params_based_on_cluster_size(
            proc->cluster_size, 2 * 1024, 4, /* Values for large cluster size */
            4 * 1024, 16,   /* Values for medium cluster size */
            12 * 1024, 32,  /* Values for small cluster size */
            12 * 1024, 32,  /* Values for very small cluster size */
            12 * 1024, 32); /* Values for unknown cluster size */
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 394 * 1024;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_CHELSIO_T3) ||
             MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_CHELSIO_T4)) {
        rdma_set_params_based_on_cluster_size(
            proc->cluster_size, 2 * 1024, 4, /* Values for large cluster size */
            4 * 1024, 8,    /* Values for medium cluster size */
            9 * 1024, 16,   /* Values for small cluster size */
            32 * 1024, 16,  /* Values for very small cluster size */
            32 * 1024, 16); /* Values for unknown cluster size */
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 64;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_INTEL_NE020)) {
        rdma_set_params_based_on_cluster_size(
            proc->cluster_size, 2 * 1024, 4, /* Values for large cluster size */
            4 * 1024, 8,    /* Values for medium cluster size */
            9 * 1024, 16,   /* Values for small cluster size */
            32 * 1024, 16,  /* Values for very small cluster size */
            32 * 1024, 16); /* Values for unknown cluster size */
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 394 * 1024;
    }

    else {
        rdma_set_params_based_on_cluster_size(
            proc->cluster_size, 16 * 1024,
            16,             /* Values for large cluster size */
            16 * 1024, 16,  /* Values for medium cluster size */
            16 * 1024, 16,  /* Values for small cluster size */
            16 * 1024, 16,  /* Values for very small cluster size */
            16 * 1024, 16); /* Values for unknown cluster size */
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 256 * 1024;
        rdma_fp_buffer_size = RDMA_FP_DEFAULT_BUF_SIZE;
    }
}

/* Set thresholds for Nnum_rail=unknown */
static void rdma_set_default_parameters_numrail_unknwn(
    struct mvp_MPIDI_CH3I_RDMA_Process_t *proc)
{
    if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_INTEL_XEON_E5630_8,
                             MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2670_16,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 4 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2630_V2_2S_12,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 20 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2698_V3_2S_32,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 20 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_PHI_7250,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 20 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V3_2S_24,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 20 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2690_V3_2S_24,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 20 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2690_V2_2S_20,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 64 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V2_2S_20,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V4_2S_28,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    } else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                    MVP_ARCH_INTEL_XEON_E5_2687W_V3_2S_20,
                                    MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2660_V3_2S_20,
                                  MVP_HCA_MLX_CX_CONNIB)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_IBM_POWER8,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 32 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_IBM_POWER9,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 32 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V2_2S_20,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V4_2S_28,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2687W_V3_2S_20,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V3_2S_24,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2690_V3_2S_24,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2660_V3_2S_20,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_IBM_POWER8,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 32 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_IBM_POWER9,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 32 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V2_2S_20,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V4_2S_28,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2620_V4_2S_16,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 9 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2687W_V3_2S_20,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V3_2S_24,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2690_V3_2S_24,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2660_V3_2S_20,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2670_V2_2S_20,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_ARM_CAVIUM_V8_2S_28,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_ARM_CAVIUM_V8_2S_32,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_ARM_FUJITSU_V0_4S_48,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2670_V3_2S_24,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2698_V3_2S_32,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_PHI_7250,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_V3_2S_24,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2690_V3_2S_24,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2687W_V3_2S_20,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2660_V3_2S_20,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2680_16,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_BULLDOZER_4274HE_16,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_OPTERON_6136_32,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_CLOVERTOWN_8,
                                  MVP_HCA_MLX_CX_DDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7551_64,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7551_64,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7662_64,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 12 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7662_64,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 32 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7763_128,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 12 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7763_128,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 32 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7601_64,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 13 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7V12_64,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 24 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7401_48,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 13 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7401_48,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 24 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7742_128,
                                  MVP_HCA_MLX_CX_HDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    /* same tuning value for rome with EDR HCA */
    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_EPYC_7742_128,
                                  MVP_HCA_MLX_CX_EDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_AMD_BARCELONA_16,
                                  MVP_HCA_MLX_CX_SDR)) {
        rdma_vbuf_total_size = 12 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_X5650_12,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 19 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2670_16,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 17 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_XEON_E5_2670_16,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_vbuf_total_size = 16 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_INTEL_GENERIC,
                                  MVP_HCA_BROADCOM_BNXTRE)) {
        rdma_vbuf_total_size = 10 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_BROADCOM_BNXTRE)) {
        rdma_vbuf_total_size = 10 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 8 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type,
                                  MVP_ARCH_INTEL_HARPERTOWN_8,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_vbuf_total_size = 9 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 5 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_MLX_CX_FDR)) {
        rdma_set_params_based_on_cluster_size(
            proc->cluster_size, 2 * 1024, 4, /* Values for large cluster size */
            4 * 1024, 16,   /* Values for medium cluster size */
            12 * 1024, 32,  /* Values for small cluster size */
            12 * 1024, 32,  /* Values for very small cluster size */
            12 * 1024, 32); /* Values for unknown cluster size */
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
        rdma_fp_buffer_size = 4 * 1024;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_MLX_CX_QDR)) {
        rdma_set_params_based_on_cluster_size(
            proc->cluster_size, 2 * 1024, 4, /* Values for large cluster size */
            4 * 1024, 16,   /* Values for medium cluster size */
            12 * 1024, 32,  /* Values for small cluster size */
            12 * 1024, 32,  /* Values for very small cluster size */
            12 * 1024, 32); /* Values for unknown cluster size */
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
        rdma_fp_buffer_size = 4 * 1024;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_MLX_CX_DDR)) {
        rdma_set_params_based_on_cluster_size(
            proc->cluster_size, 2 * 1024, 4, /* Values for large cluster size */
            4 * 1024, 16,   /* Values for medium cluster size */
            12 * 1024, 32,  /* Values for small cluster size */
            12 * 1024, 32,  /* Values for very small cluster size */
            12 * 1024, 32); /* Values for unknown cluster size */
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 0;
        rdma_fp_buffer_size = 9 * 1024;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_CHELSIO_T3) ||
             MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_CHELSIO_T4)) {
        rdma_set_params_based_on_cluster_size(
            proc->cluster_size, 2 * 1024, 4, /* Values for large cluster size */
            4 * 1024, 8,    /* Values for medium cluster size */
            9 * 1024, 16,   /* Values for small cluster size */
            32 * 1024, 16,  /* Values for very small cluster size */
            32 * 1024, 16); /* Values for unknown cluster size */
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 64;
    }

    else if (MVP_IS_ARCH_HCA_TYPE(proc->arch_hca_type, MVP_ARCH_ANY,
                                  MVP_HCA_INTEL_NE020)) {
        rdma_set_params_based_on_cluster_size(
            proc->cluster_size, 2 * 1024, 4, /* Values for large cluster size */
            4 * 1024, 8,    /* Values for medium cluster size */
            9 * 1024, 16,   /* Values for small cluster size */
            32 * 1024, 16,  /* Values for very small cluster size */
            32 * 1024, 16); /* Values for unknown cluster size */
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 394 * 1024;
    }

    else {
        rdma_set_params_based_on_cluster_size(
            proc->cluster_size, 16 * 1024,
            16,             /* Values for large cluster size */
            16 * 1024, 16,  /* Values for medium cluster size */
            16 * 1024, 16,  /* Values for small cluster size */
            16 * 1024, 16,  /* Values for very small cluster size */
            16 * 1024, 16); /* Values for unknown cluster size */
        rdma_eagersize_1sc = 4 * 1024;
        rdma_put_fallback_threshold = 8 * 1024;
        rdma_get_fallback_threshold = 256 * 1024;
        rdma_fp_buffer_size = RDMA_FP_DEFAULT_BUF_SIZE;
    }
}

const char *mvp_ibv_mtu_enum_to_string(enum ibv_mtu mtu)
{
    switch (mtu) {
        case IBV_MTU_256:
            return "IBV_MTU_256";
        case IBV_MTU_512:
            return "IBV_MTU_512";
        case IBV_MTU_1024:
            return "IBV_MTU_1024";
        case IBV_MTU_2048:
            return "IBV_MTU_2048";
        case IBV_MTU_4096:
            return "IBV_MTU_4096";
        default:
            return "Unknown MTU";
    }
}

uint16_t mvp_ibv_mtu_enum_to_value(enum ibv_mtu mtu)
{
    switch (mtu) {
        case IBV_MTU_256:
            return 256;
        case IBV_MTU_512:
            return 512;
        case IBV_MTU_1024:
            return 1024;
        case IBV_MTU_2048:
            return 2048;
        case IBV_MTU_4096:
            return 4096;
        default:
            return -1;
    }
}

void rdma_set_default_parameters(struct mvp_MPIDI_CH3I_RDMA_Process_t *proc)
{
    mvp_multirail_info_type multirail_info = mvp_get_multirail_info();

    /* Setting the default values; these values are fine-tuned for specific
     * platforms in the following code */
    rdma_vbuf_total_size = 12 * 1024 + EAGER_THRESHOLD_ADJUST;
    rdma_fp_buffer_size = RDMA_FP_DEFAULT_BUF_SIZE;
    num_rdma_buffer = 16;
    rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;

    rdma_eagersize_1sc = 4 * 1024;
    rdma_put_fallback_threshold = 2 * 1024;
    rdma_get_fallback_threshold = 192 * 1024;

    switch (multirail_info) {
        case mvp_num_rail_4:
            /* Set thresholds for Nnum_rail=4 */
            rdma_set_default_parameters_numrail_4(proc);
            break;

        case mvp_num_rail_3:
            /* Set thresholds for Nnum_rail=3 */
            rdma_set_default_parameters_numrail_3(proc);
            break;
        case mvp_num_rail_2:
            /* Set thresholds for Nnum_rail=2 */
            rdma_set_default_parameters_numrail_2(proc);
            break;

        case mvp_num_rail_1:
            /* Set thresholds for Nnum_rail=1 */
            rdma_set_default_parameters_numrail_1(proc);
            break;

            /* mvp_num_rail_unknwon */
        default:
            rdma_set_default_parameters_numrail_unknwn(proc);
            break;
    }

    if ((MVP_HCA_QLGIC_PATH_HT == proc->hca_type) ||
        (MVP_HCA_QLGIC_QIB == proc->hca_type)) {
        rdma_default_qp_ous_rd_atom = 1;
    } else {
        rdma_default_qp_ous_rd_atom = 4;
    }

    if (MVP_HCA_IBM_EHCA == proc->hca_type) {
        rdma_max_inline_size = -1;
    } else if (MVP_IS_CHELSIO_IWARP_CARD(proc->hca_type)) {
        rdma_max_inline_size = 64;
    } else if (MVP_HCA_INTEL_NE020 == proc->hca_type) {
        rdma_max_inline_size = 64;
    } else if (MVP_HCA_BROADCOM_BNXTRE == proc->hca_type) {
        rdma_max_inline_size = 96;
    } else {
        rdma_max_inline_size = 128 + INLINE_THRESHOLD_ADJUST;
    }

    if (MVP_IS_CHELSIO_IWARP_CARD(proc->hca_type)) {
        /* Trac #423 */
        struct ibv_device_attr dev_attr;
        int mpi_errno = MPI_SUCCESS;

        /*querying device for cq depth */
        mpi_errno = ibv_ops.query_device(proc->nic_context[0], &dev_attr);

        if (!mpi_errno) {
            if (dev_attr.max_cqe < rdma_default_max_cq_size) {
                rdma_default_max_cq_size = dev_attr.max_cqe;
            }
        } else {
            rdma_default_max_cq_size = RDMA_DEFAULT_IWARP_CQ_SIZE;
        }
        rdma_prepost_noop_extra = 8;
    }

    if (MVP_HCA_INTEL_NE020 == proc->hca_type) {
        rdma_default_max_cq_size = 32766;
        rdma_prepost_noop_extra = 8;
    }

    if (proc->has_srq) {
        rdma_credit_preserve = 100;
    } else {
        rdma_credit_preserve = 3;
    }

    if (!SMP_ONLY && rdma_vbuf_total_size < 128 * 1024
#ifdef _ENABLE_UD_
        && !MVP_USE_ONLY_UD
#endif
        && !mvp_system_has_rockport) {
        rdma_vbuf_total_size = 128 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
    }

    return;
}

/* rdma_param_handle_heterogeneity resets control parameters given the
 * arch_hca_type from all ranks. Parameters may change: rdma_default_mtu
 *      rdma_iba_eager_threshold
 *      proc->has_srq
 *      rdma_credit_preserve
 *      rdma_default_qp_ous_rd_atom
 *      rdma_put_fallback_threshold
 *      rdma_get_fallback_threshold
 *      num_rdma_buffer
 *      rdma_vbuf_total_size
 */
void rdma_param_handle_heterogeneity(mvp_arch_hca_type arch_hca_type[],
                                     int pg_size)
{
    mvp_arch_hca_type type;
    mvp_MPIDI_CH3I_RDMA_Process.heterogeneity = 0;
    int i;
    char *value = NULL;

    type = arch_hca_type[0];
    for (i = 0; i < pg_size; ++i) {
        if (MVP_IS_ARCH_HCA_TYPE(arch_hca_type[i], MVP_ARCH_ANY,
                                 MVP_HCA_QLGIC_PATH_HT) ||
            MVP_IS_ARCH_HCA_TYPE(arch_hca_type[i], MVP_ARCH_ANY,
                                 MVP_HCA_QLGIC_QIB)) {
            mvp_MPIDI_CH3I_RDMA_Process.has_srq = 0;
            mvp_use_post_srq_send = 0;
            rdma_credit_preserve = 3;
            rdma_default_qp_ous_rd_atom = 1;
        }

        else if (MVP_IS_ARCH_HCA_TYPE(arch_hca_type[i], MVP_ARCH_ANY,
                                      MVP_HCA_MLX_PCI_X)) {
            mvp_MPIDI_CH3I_RDMA_Process.has_srq = 0;
            mvp_use_post_srq_send = 0;
            rdma_credit_preserve = 3;
        }

        else if (MVP_IS_ARCH_HCA_TYPE(arch_hca_type[i], MVP_ARCH_ANY,
                                      MVP_HCA_IBM_EHCA)) {
            mvp_MPIDI_CH3I_RDMA_Process.has_srq = 0;
            mvp_use_post_srq_send = 0;
            rdma_credit_preserve = 3;
        }

        if (arch_hca_type[i] != type) {
            mvp_MPIDI_CH3I_RDMA_Process.heterogeneity = 1;
        }

        PRINT_DEBUG(DEBUG_INIT_verbose, "rank %d, type %lu\n", i,
                    arch_hca_type[i]);
    }

    if (mvp_MPIDI_CH3I_RDMA_Process.heterogeneity) {
        PRINT_DEBUG(DEBUG_INIT_verbose, "heterogeneous hcas detected\n");
        rdma_default_mtu = IBV_MTU_1024;
        rdma_vbuf_total_size = 8 * 1024 + EAGER_THRESHOLD_ADJUST;
        rdma_fp_buffer_size = 8 * 1024;
        rdma_iba_eager_threshold = VBUF_BUFFER_SIZE;
        rdma_put_fallback_threshold = 4 * 1024;
        rdma_get_fallback_threshold = 192 * 1024;
        num_rdma_buffer = 16;
    } else {
        if (MVP_SUPPRESS_JOB_STARTUP_PERFORMANCE_WARNING == 0) {
            PRINT_INFO(
                (MPIDI_Process.my_pg_rank == 0),
                "All nodes involved in the job were detected to be "
                "homogeneous in terms of processors and interconnects. "
                "Setting MVP_HOMOGENEOUS_CLUSTER=1 can improve job "
                "startup performance on such systems. The following link "
                "has more details on enhancing job startup performance. "
                "http://mvapich.cse.ohio-state.edu/performance/job-startup/"
                ".\n");
            PRINT_INFO((MPIDI_Process.my_pg_rank == 0),
                       "To suppress this warning, please set"
                       " MVP_SUPPRESS_JOB_STARTUP_PERFORMANCE_WARNING to 1\n");
        }
    }
}

void rdma_set_rdma_fast_path_params(int num_proc)
{
    char *value = NULL;

#if defined(CKPT)
    mvp_MPIDI_CH3I_RDMA_Process.has_adaptive_fast_path = 0;
    rdma_polling_set_limit = 0;
#else /* defined(CKPT) */
    if (MVP_USE_RDMA_FAST_PATH != -1) {
        mvp_MPIDI_CH3I_RDMA_Process.has_adaptive_fast_path =
            !!MVP_USE_RDMA_FAST_PATH;

        if (!mvp_MPIDI_CH3I_RDMA_Process.has_adaptive_fast_path) {
            rdma_polling_set_limit = 0;
            MVP_RDMA_FAST_PATH_PREALLOCATE_BUFFERS = 0;
        }
    } else {
        if (!mvp_use_opt_eager_recv) {
            mvp_MPIDI_CH3I_RDMA_Process.has_adaptive_fast_path = 1;
        }
    }
    if ((mvp_MPIDI_CH3I_RDMA_Process.has_adaptive_fast_path == 1) &&
        mvp_use_opt_eager_recv) {
        PRINT_INFO((MPIDI_Process.my_pg_rank == 0),
                   "Optimized eager recv is"
                   " not compatible with RDMA Fast Path. Disabling"
                   " optimized eager recv and continuing\n");
        mvp_use_opt_eager_recv = 0;
    }
#endif /* defined(CKPT) */

#ifdef _ENABLE_XRC_
    if (!USE_XRC) {
#endif
        /* Automatically turn off RDMA fast path */
        if (rdma_use_blocking) {
            mvp_MPIDI_CH3I_RDMA_Process.has_adaptive_fast_path = 0;
        }
#ifdef _ENABLE_XRC_
    }
#endif

    if (mvp_MPIDI_CH3I_RDMA_Process.has_adaptive_fast_path) {
        rdma_fp_buffer_size = MVP_RDMA_FAST_PATH_BUF_SIZE;

        if (MVP_POLLING_SET_LIMIT == -1) {
            MVP_POLLING_SET_LIMIT = log_2(num_proc);
        }
        rdma_polling_set_limit = MVP_POLLING_SET_LIMIT;

        rdma_polling_set_threshold = MVP_POLLING_SET_THRESHOLD;

        rdma_eager_limit = MVP_RDMA_EAGER_LIMIT;
        if (rdma_eager_limit < 0) {
            rdma_eager_limit = 0;
        }
        if (MVP_NUM_RDMA_BUFFER != -1) {
            num_rdma_buffer = MVP_NUM_RDMA_BUFFER;
        } else {
            MVP_NUM_RDMA_BUFFER = num_rdma_buffer;
        }
    }

    return;
}

void rdma_get_user_parameters(int num_proc, int me)
{
    char *value;

    if (!MVP_SUPPORT_DPM) {
        /* Ensure BW tests have enough RFP buffers*/
        if (2 == num_proc) {
            num_rdma_buffer = 64;
        }
    }

    /* Check for a system report. See sysreport.h and sysreport.c */
    enable_sysreport = MVP_SYSREPORT;

    if (MVP_DEFAULT_MTU != NULL) {
        if (strncmp(MVP_DEFAULT_MTU, "IBV_MTU_256", 11) == 0) {
            rdma_default_mtu = IBV_MTU_256;
        } else if (strncmp(MVP_DEFAULT_MTU, "IBV_MTU_512", 11) == 0) {
            rdma_default_mtu = IBV_MTU_512;
        } else if (strncmp(MVP_DEFAULT_MTU, "IBV_MTU_1024", 12) == 0) {
            rdma_default_mtu = IBV_MTU_1024;
        } else if (strncmp(MVP_DEFAULT_MTU, "IBV_MTU_2048", 12) == 0) {
            rdma_default_mtu = IBV_MTU_2048;
        } else if (strncmp(MVP_DEFAULT_MTU, "IBV_MTU_4096", 12) == 0) {
            rdma_default_mtu = IBV_MTU_4096;
        } else {
            rdma_default_mtu = IBV_MTU_1024;
        }
    }

    /* Number of CQE's retrieved per poll */
    rdma_num_cqes_per_poll = MVP_NUM_CQES_PER_POLL;
    if (rdma_num_cqes_per_poll <= 0 ||
        rdma_num_cqes_per_poll >= RDMA_MAX_CQE_ENTRIES_PER_POLL) {
        rdma_num_cqes_per_poll = RDMA_MAX_CQE_ENTRIES_PER_POLL;
    }

    /* Get number of ports/HCA used by a process */
    rdma_num_ports = MVP_NUM_PORTS;
    if (rdma_num_ports > MAX_NUM_PORTS) {
        rdma_num_ports = MAX_NUM_PORTS;
        MPL_usage_printf("Warning, max ports per hca is %d, change %s in "
                         "ibv_param.h to override the option\n",
                         MAX_NUM_PORTS, "MAX_NUM_PORTS");
    }

    /* Get number of qps/port used by a process */
    rdma_num_qp_per_port = MVP_NUM_QP_PER_PORT;

    if (rdma_num_qp_per_port > MAX_NUM_QP_PER_PORT) {
        rdma_num_qp_per_port = MAX_NUM_QP_PER_PORT;
        MPL_usage_printf("Warning, max qps per port is %d, change %s in "
                         "ibv_param.h to override the option\n",
                         MAX_NUM_QP_PER_PORT, "MAX_NUM_QP_PER_PORT");
    }
#ifdef _ENABLE_UD_
    if (MVP_USE_ONLY_UD || MVP_USE_UD_HYBRID) {
        rdma_num_qp_per_port = 1;
        PRINT_INFO(
            (me == 0),
            "Cannot have more than one QP with UD_ONLY / Hybrid mode.\n");
        PRINT_INFO((me == 0), "Resetting MVP_NUM_QP_PER_PORT to 1.\n");
    }
#endif /* _ENABLE_UD_ */
    rdma_pin_pool_size = MVP_PIN_POOL_SIZE;

    /* TODO: Cannot set the cvar here as we check a default status later.
     *       Need to fix that logic */
    if (MVP_DEFAULT_MAX_CQ_SIZE != -1) {
        rdma_default_max_cq_size = MVP_DEFAULT_MAX_CQ_SIZE;
    }

    /* We have read the value of the rendezvous threshold, and the number of
     * rails used for communication, increase the striping threshold
     * accordingly */

    /* Messages in between will use the rendezvous protocol, however will
     * not be striped */

    striping_threshold = rdma_vbuf_total_size * rdma_num_ports *
                         rdma_num_qp_per_port * rdma_num_hcas;

    if (MVP_IBA_EAGER_THRESHOLD != -1) {
        rdma_iba_eager_threshold = MVP_IBA_EAGER_THRESHOLD;
    } else {
        MVP_IBA_EAGER_THRESHOLD = rdma_iba_eager_threshold;
    }

    if (MVP_STRIPING_THRESHOLD <= 0) {
        /* Invalid value - set to computed value */
        MVP_STRIPING_THRESHOLD = rdma_vbuf_total_size * rdma_num_ports *
                                 rdma_num_qp_per_port * rdma_num_hcas;
    }
    if (MVP_STRIPING_THRESHOLD < MVP_IBA_EAGER_THRESHOLD) {
        /* checking to make sure that the striping threshold is not less
         * than the RNDV threshold since it won't work as expected.
         */
        MVP_STRIPING_THRESHOLD = MVP_IBA_EAGER_THRESHOLD;
    }
    striping_threshold = MVP_STRIPING_THRESHOLD;

    rdma_med_msg_rail_sharing_threshold = MVP_RAIL_SHARING_MED_MSG_THRESHOLD;
    if (rdma_med_msg_rail_sharing_threshold <= 0) {
        rdma_med_msg_rail_sharing_threshold =
            RDMA_DEFAULT_MED_MSG_RAIL_SHARING_THRESHOLD;
    }

    if (MVP_RAIL_SHARING_LARGE_MSG_THRESHOLD > 0) {
        rdma_large_msg_rail_sharing_threshold =
            MVP_RAIL_SHARING_LARGE_MSG_THRESHOLD;
    } else {
        rdma_large_msg_rail_sharing_threshold = rdma_vbuf_total_size;
        MVP_RAIL_SHARING_LARGE_MSG_THRESHOLD =
            rdma_large_msg_rail_sharing_threshold;
    }

    rdma_default_put_get_list_size = MVP_DEFAULT_PUT_GET_LIST_SIZE;

    if (MVP_EAGERSIZE_1SC != -1) {
        rdma_eagersize_1sc = MVP_EAGERSIZE_1SC;
    } else {
        MVP_EAGERSIZE_1SC = rdma_eagersize_1sc;
    }
    if (MVP_PUT_FALLBACK_THRESHOLD != -1) {
        rdma_put_fallback_threshold = MVP_PUT_FALLBACK_THRESHOLD;
    } else {
        MVP_PUT_FALLBACK_THRESHOLD = rdma_put_fallback_threshold;
    }
    if (MVP_GET_FALLBACK_THRESHOLD != -1) {
        rdma_get_fallback_threshold = MVP_GET_FALLBACK_THRESHOLD;
    }

    rdma_default_port = MVP_DEFAULT_PORT;
    rdma_default_gid_index = MVP_DEFAULT_GID_INDEX;

    if (MVP_DEFAULT_QP_OUS_RD_ATOM != -1) {
        rdma_default_qp_ous_rd_atom = MVP_DEFAULT_QP_OUS_RD_ATOM;
    }
    rdma_default_max_rdma_dst_ops = MVP_DEFAULT_MAX_RDMA_DST_OPS;
    rdma_default_psn = MVP_DEFAULT_PSN;
    if (MVP_DEFAULT_PKEY != NULL) {
        rdma_default_pkey =
            (uint16_t)strtol(MVP_DEFAULT_PKEY, (char **)NULL, 0) & PKEY_MASK;
    }
    if (MVP_DEFAULT_QKEY != NULL) {
        rdma_default_qkey =
            (uint32_t)strtol(MVP_DEFAULT_QKEY, (char **)NULL, 0);
    }
    rdma_default_min_rnr_timer = MVP_DEFAULT_MIN_RNR_TIMER;
    rdma_default_service_level = MVP_DEFAULT_SERVICE_LEVEL;
    rdma_default_time_out = MVP_DEFAULT_TIME_OUT;
    rdma_default_static_rate = MVP_DEFAULT_STATIC_RATE;
    rdma_default_src_path_bits = MVP_DEFAULT_SRC_PATH_BITS;
    rdma_default_retry_count = MVP_DEFAULT_RETRY_COUNT;
    rdma_default_rnr_retry = MVP_DEFAULT_RNR_RETRY;
    rdma_default_max_sg_list = MVP_DEFAULT_MAX_SG_LIST;
    rdma_default_max_send_wqe = MVP_DEFAULT_MAX_SEND_WQE;

    mvp_cm_wait_time = MVP_CM_WAIT_TIME;
    if (mvp_cm_wait_time <= 0) {
        mvp_cm_wait_time = DEF_MVP_CM_WAIT_TIME;
    }

#if defined(_ENABLE_UD_) || defined(_MCST_SUPPORT_)

    if (MVP_UD_MTU) {
        rdma_default_ud_mtu = MVP_UD_MTU;
        if (rdma_default_ud_mtu < 256 || rdma_default_ud_mtu > 4096) {
            PRINT_INFO(!me,
                       "Invalid value used for UD MTU (Min: 256; Max: 4K). "
                       "Resetting to default value (2K)\n");
            rdma_default_ud_mtu = 2048;
        }
    }

#endif /* #if defined(_ENABLE_UD_) || defined(_MCST_SUPPORT_)*/
#ifdef _ENABLE_UD_
    if (MVP_HYBRID_MAX_RC_CONN != -1) {
        if (MVP_USE_ONLY_UD && MVP_HYBRID_MAX_RC_CONN) {
            PRINT_INFO((me == 0), "User requested only UD. Resetting "
                                  "MVP_HYBRID_MAX_RC_CONN to 0.\n");
            MVP_HYBRID_MAX_RC_CONN = 0;
        }
        rdma_hybrid_max_rc_conn = MVP_HYBRID_MAX_RC_CONN;
    } else {
        if (MVP_USE_ONLY_UD) {
            rdma_hybrid_max_rc_conn = 0;
        }
        MVP_HYBRID_MAX_RC_CONN = rdma_hybrid_max_rc_conn;
    }
    rdma_ud_num_msg_limit = MVP_UD_NUM_MSG_LIMIT;
    rdma_default_ud_sendwin_size = MVP_UD_SENDWINDOW_SIZE;
    rdma_default_ud_recvwin_size = MVP_UD_RECVWINDOW_SIZE;
    rdma_ud_retry_timeout = MVP_UD_RETRY_TIMEOUT;
    rdma_ud_max_retry_timeout = MVP_UD_MAX_RETRY_TIMEOUT;
    rdma_ud_progress_spin = MVP_UD_PROGRESS_SPIN;
    if (MVP_UD_RETRY_COUNT != -1) {
        rdma_ud_max_retry_count = MVP_UD_RETRY_COUNT;
    } else {
        MVP_UD_RETRY_COUNT = rdma_ud_max_retry_count;
    }
    if (MVP_UD_PROGRESS_TIMEOUT != -1) {
        rdma_ud_progress_timeout = MVP_UD_PROGRESS_TIMEOUT;
    } else {
        MVP_UD_PROGRESS_TIMEOUT = rdma_ud_progress_timeout;
    }
    rdma_default_max_ud_send_wqe = MVP_UD_MAX_SEND_WQE;
    rdma_default_max_ud_recv_wqe = MVP_UD_MAX_RECV_WQE;
    rdma_use_ud_srq = !!MVP_USE_UD_SRQ;
    rdma_ud_vbuf_pool_size = MVP_UD_VBUF_POOL_SIZE;

    if (MVP_UD_MAX_ACK_PENDING < 0) {
        MVP_UD_MAX_ACK_PENDING = (MVP_UD_SENDWINDOW_SIZE / 4);
    }
    rdma_ud_max_ack_pending = MVP_UD_MAX_ACK_PENDING;
    rdma_use_ud_zcopy = MVP_USE_UD_ZCOPY;
    if (MVP_UD_ZCOPY_THRESHOLD < 0) {
        MVP_UD_ZCOPY_THRESHOLD = MVP_IBA_EAGER_THRESHOLD;
    }
    rdma_ud_zcopy_threshold = MVP_UD_ZCOPY_THRESHOLD;

    if (MVP_UD_NUM_ZCOPY_RNDV_QPS == -1) {
        MVP_UD_NUM_ZCOPY_RNDV_QPS =
            RDMA_UD_DEFAULT_NUM_RNDV_QPS * rdma_num_hcas;
    }
    rdma_ud_num_rndv_qps = MVP_UD_NUM_ZCOPY_RNDV_QPS;
    rdma_ud_zcopy_rq_size = MVP_UD_ZCOPY_RQ_SIZE;
    rdma_ud_zcopy_num_retry = MVP_UD_ZCOPY_NUM_RETRY;
    rdma_ud_zcopy_enable_polling = MVP_UD_ZCOPY_ENABLE_POLLING;
    rdma_ud_zcopy_push_segment = MVP_UD_ZCOPY_PUSH_SEGMENT;
    /* Push segment needs to be at least 2, or there will be error setting
     * next of first entry to NULL */
    if (rdma_ud_zcopy_push_segment < RDMA_DEFAULT_MIN_UD_ZCOPY_WQE + 1) {
        PRINT_INFO((MPIDI_Process.my_pg_rank == 0),
                   "[Warning]:"
                   "MVP_UD_ZCOPY_PUSH_SEGMENT cannot be less than %d, setting "
                   "it back to %d\n",
                   RDMA_DEFAULT_MIN_UD_ZCOPY_WQE + 1,
                   RDMA_DEFAULT_MIN_UD_ZCOPY_WQE + 1);
    }

#ifdef _MVP_UD_DROP_PACKET_RATE_
    ud_drop_packet_rate = MVP_UD_DROP_PACKET_RATE;
#endif

#endif

#if defined(_MCST_SUPPORT_)
    mcast_enable_rel = MVP_MCAST_ENABLE_REL;
    mcast_use_mcast_nack = MVP_MCAST_USE_MCAST_NACK;
    if (MVP_MCAST_NUM_NODES_THRESHOLD != -1) {
        int env_num_threshold = MVP_MCAST_NUM_NODES_THRESHOLD;
        if (env_num_threshold < MCAST_MIN_THRESHOLD) {
            mcast_num_nodes_threshold = MCAST_MIN_THRESHOLD;
            PRINT_INFO((MPIDI_Process.my_pg_rank == 0),
                       "[Warning]:"
                       " MVP_MCAST_NUM_NODES_THRESHOLD cannot be less than %d, "
                       "setting it to %d\n",
                       MCAST_MIN_THRESHOLD, MCAST_MIN_THRESHOLD);
        } else {
            mcast_num_nodes_threshold = env_num_threshold;
        }
    }

    mcast_max_ud_recv_wqe = MVP_MCAST_MAX_RECV_WQE;
    mcast_window_size = MVP_MCAST_WINDOW_SIZE;
    mcast_drop_packet_rate = MVP_MCAST_DROP_PACKET_RATE;
    mcast_retry_timeout = MVP_MCAST_RETRY_TIMEOUT;
    mcast_max_retry_timeout = MVP_MCAST_MAX_RETRY_TIMEOUT;
    mcast_nspin_threshold = MVP_MCAST_NSPIN_THRESHOLD;
    mcast_comm_init_timeout = MVP_MCAST_COMM_INIT_TIMEOUT;
    mcast_comm_init_retries = MVP_MCAST_COMM_INIT_RETRIES;
    mcast_skip_loopback = MVP_MCAST_SKIP_LOOPBACK;
    mcast_bcast_min_msg = MVP_MCAST_BCAST_MIN_MSG;
    mcast_bcast_max_msg = MVP_MCAST_BCAST_MAX_MSG;

#endif

    rdma_default_max_recv_wqe = MVP_DEFAULT_MAX_RECV_WQE;
    /* TODO: this should have a default but it is unclear */
    if (MVP_NDREG_ENTRIES_MAX != -1) {
        rdma_ndreg_entries_max = MVP_NDREG_ENTRIES_MAX;
    } else {
        MVP_NDREG_ENTRIES_MAX = rdma_ndreg_entries_max;
    }
    if (MVP_NDREG_ENTRIES != -1) {
        if (MVP_NDREG_ENTRIES > MVP_NDREG_ENTRIES_MAX) {
            MVP_NDREG_ENTRIES = MVP_NDREG_ENTRIES_MAX;
        }
        rdma_ndreg_entries = MVP_NDREG_ENTRIES;
    } else {
        rdma_ndreg_entries =
            ((RDMA_NDREG_ENTRIES + 2 * num_proc < rdma_ndreg_entries_max) ?
                 RDMA_NDREG_ENTRIES + 2 * num_proc :
                 rdma_ndreg_entries_max);
        if (rdma_ndreg_entries < RDMA_NDREG_ENTRIES_MIN) {
            rdma_ndreg_entries = RDMA_NDREG_ENTRIES_MIN;
        }
        MVP_NDREG_ENTRIES = rdma_ndreg_entries;
    }
    rdma_dreg_cache_limit = MVP_DREG_CACHE_LIMIT;
#if defined(_ENABLE_UD_)
    if (MVP_USE_ONLY_UD) {
        rdma_vbuf_pool_size = 0;
        mvp_MPIDI_CH3I_RDMA_Process.has_srq = 0;
        mvp_use_post_srq_send = 0;
    }
#endif
    if (rdma_initial_prepost_depth <= rdma_prepost_noop_extra) {
        rdma_initial_credits = rdma_initial_prepost_depth;
    } else {
        rdma_initial_credits =
            rdma_initial_prepost_depth - rdma_prepost_noop_extra;
    }

    rdma_rq_size = rdma_prepost_depth + rdma_prepost_rendezvous_extra +
                   rdma_prepost_noop_extra;

    use_hwloc_cpu_binding = MVP_USE_HWLOC_CPU_BINDING;
    rdma_polling_spin_count_threshold = MVP_THREAD_YIELD_SPIN_THRESHOLD;
    mvp_use_thread_yield = MVP_USE_THREAD_YIELD;
    mvp_spins_before_lock = MVP_NUM_SPINS_BEFORE_LOCK;
    rdma_default_async_thread_stack_size = MVP_ASYNC_THREAD_STACK_SIZE;
    if (rdma_default_async_thread_stack_size < 1 << 10) {
        MPL_usage_printf(
            "Warning! Too small stack size for async thread (%d).  "
            "Reset to %d\n",
            rdma_vbuf_secondary_pool_size,
            RDMA_DEFAULT_ASYNC_THREAD_STACK_SIZE);
        rdma_default_async_thread_stack_size =
            RDMA_DEFAULT_ASYNC_THREAD_STACK_SIZE;
    }
    rdma_enable_hugepage = MVP_USE_HUGEPAGES;

    /* Read VBUF related user parameters */
    rdma_get_vbuf_user_parameters(num_proc, me);

    if (mvp_MPIDI_CH3I_RDMA_Process.has_srq) {
        rdma_credit_preserve = (mvp_srq_fill_size > 200) ?
                                   (mvp_srq_fill_size - 100) :
                                   (mvp_srq_fill_size / 4);
    }

    return;
}

static inline void rdma_get_vbuf_user_parameters(int num_proc, int me)
{
    char *value = NULL;
    int my_rank = -1;

    UPMI_GET_RANK(&my_rank);
    rdma_memory_optimization = !!MVP_MEMORY_OPTIMIZATION;
#ifdef _ENABLE_UD_
    if (!MVP_USE_ONLY_UD && rdma_memory_optimization)
#else
    if (rdma_memory_optimization)
#endif
    {
        mvp_srq_limit = RDMA_OPT_VBUF_POOL_SIZE / 8;
        mvp_srq_fill_size = RDMA_OPT_VBUF_POOL_SIZE;
        rdma_vbuf_pool_size = RDMA_OPT_VBUF_POOL_SIZE;
        rdma_credit_preserve = RDMA_OPT_VBUF_POOL_SIZE / 6;
        rdma_vbuf_secondary_pool_size = RDMA_OPT_VBUF_SECONDARY_POOL_SIZE;
    }

    if (MVP_SRQ_MAX_SIZE > 0) {
        mvp_srq_alloc_size = (uint32_t)MVP_SRQ_MAX_SIZE;
#if defined(RDMA_CM)
    } else if (MPIDI_CH3I_Process.cm_type == MPIDI_CH3I_CM_RDMA_CM) {
        /* When using RDMA_CM, we cannot support very large SRQ. So, unless user
         * set it, reduce the max_srq_size to 4K */
        mvp_srq_alloc_size = 4096;
#endif /*defined(RDMA_CM)*/
    }

    if (MVP_SRQ_SIZE != -1) {
        mvp_srq_fill_size = (uint32_t)MVP_SRQ_SIZE;
    } else {
        MVP_SRQ_SIZE = mvp_srq_fill_size;
    }

    if (MVP_SRQ_LIMIT != -1) {
        mvp_srq_limit = (uint32_t)MVP_SRQ_LIMIT;

        if (mvp_srq_limit > mvp_srq_fill_size) {
            MPL_usage_printf("SRQ limit shouldn't be greater than SRQ size\n");
        }
    } else {
        MVP_SRQ_LIMIT = mvp_srq_limit;
    }

#if defined(_ENABLE_UD_)
    if (MVP_UD_SRQ_MAX_SIZE != -1) {
        mvp_ud_srq_alloc_size = (uint32_t)MVP_UD_SRQ_MAX_SIZE;
#if defined(RDMA_CM)
    } else if (MPIDI_CH3I_Process.cm_type == MPIDI_CH3I_CM_RDMA_CM) {
        /* When using RDMA_CM, we cannot support very large SRQ. So, unless user
         * set it, reduce the max_srq_size to 4K */
        mvp_ud_srq_alloc_size = 4096;
#endif /*defined(RDMA_CM)*/
    }
    MVP_UD_SRQ_MAX_SIZE = mvp_ud_srq_alloc_size;

    if (MVP_UD_SRQ_LIMIT > MVP_UD_SRQ_SIZE) {
        MPL_usage_printf(
            "UD SRQ limit shouldn't be greater than UD SRQ size\n");
        MVP_UD_SRQ_LIMIT = MVP_UD_SRQ_SIZE;
    }
    mvp_ud_srq_fill_size = (uint32_t)MVP_UD_SRQ_SIZE;
    mvp_ud_srq_limit = (uint32_t)MVP_UD_SRQ_LIMIT;

    rdma_max_num_ud_vbufs = MVP_MAX_NUM_UD_VBUFS;
    if (rdma_max_num_ud_vbufs < 1) {
        PRINT_INFO(!my_rank,
                   "MVP_MAX_NUM_UD_VBFUS must be greater than 1\n"
                   "Setting MVP_MAX_NUM_UD_VBUFS to %d\n",
                   DEFAULT_MAX_NUM_UD_VBUFS);
        rdma_max_num_ud_vbufs = DEFAULT_MAX_NUM_UD_VBUFS;
    }
#endif /*defined(_ENABLE_UD_)*/

    if (MVP_MAX_INLINE_SIZE != -1) {
        rdma_max_inline_size = MVP_MAX_INLINE_SIZE;
    } else {
        MVP_MAX_INLINE_SIZE = rdma_max_inline_size;
    }
    if (MVP_VBUF_TOTAL_SIZE != -1) {
        if (rdma_memory_optimization) {
            if (RDMA_OPT_MIN_VBUF_POOL_SIZE < MVP_VBUF_TOTAL_SIZE) {
                rdma_vbuf_total_size =
                    MVP_VBUF_TOTAL_SIZE + EAGER_THRESHOLD_ADJUST;
            } else {
                /* We do not accept vbuf size < RDMA_MIN_VBUF_POOL_SIZE */
                MPL_usage_printf("Warning, it is inefficient to use a value for"
                                 "VBUF which is less than %d. Retaining the"
                                 " system default value of %d\n",
                                 RDMA_OPT_MIN_VBUF_POOL_SIZE,
                                 rdma_vbuf_total_size);
            }
        } else {
            if (RDMA_MIN_VBUF_POOL_SIZE < MVP_VBUF_TOTAL_SIZE) {
                rdma_vbuf_total_size =
                    MVP_VBUF_TOTAL_SIZE + EAGER_THRESHOLD_ADJUST;
            } else {
                /* We do not accept vbuf size < RDMA_MIN_VBUF_POOL_SIZE */
                MPL_usage_printf("Warning, it is inefficient to use a value for"
                                 "VBUF which is less than %d. Retaining the"
                                 " system default value of %d\n",
                                 RDMA_MIN_VBUF_POOL_SIZE, rdma_vbuf_total_size);
            }
        }
    }

    rdma_vbuf_max = MVP_VBUF_MAX;
    rdma_initial_prepost_depth = MVP_INITIAL_PREPOST_DEPTH;
    rdma_prepost_depth = MVP_PREPOST_DEPTH;

    if (MVP_VBUF_POOL_SIZE != -1) {
        rdma_vbuf_pool_size = MVP_VBUF_POOL_SIZE;
    } else {
        MVP_VBUF_POOL_SIZE = rdma_vbuf_pool_size;
    }

    rdma_max_num_vbufs = MVP_MAX_NUM_VBUFS;
    if (rdma_max_num_vbufs < 1) {
        PRINT_INFO(!my_rank,
                   "MVP_MAX_NUM_VBFUS must be greater than 1\n"
                   "Setting MVP_MAX_NUM_VBUFS to %d\n",
                   DEFAULT_MAX_NUM_VBUFS);
        rdma_max_num_vbufs = DEFAULT_MAX_NUM_VBUFS;
    }

#ifdef _ENABLE_UD_
    if (!MVP_USE_ONLY_UD && rdma_vbuf_pool_size <= 10)
#else
    if (rdma_vbuf_pool_size <= 10)
#endif
    {
        if (rdma_memory_optimization) {
            rdma_vbuf_pool_size = RDMA_OPT_VBUF_POOL_SIZE;
            MPL_usage_printf("Warning! Too small vbuf pool size (%d).  "
                             "Reset to %d\n",
                             rdma_vbuf_pool_size, RDMA_OPT_VBUF_POOL_SIZE);
        } else {
            rdma_vbuf_pool_size = RDMA_VBUF_POOL_SIZE;
            MPL_usage_printf("Warning! Too small vbuf pool size (%d).  "
                             "Reset to %d\n",
                             rdma_vbuf_pool_size, RDMA_VBUF_POOL_SIZE);
        }
    }
    if (MVP_VBUF_SECONDARY_POOL_SIZE != -1) {
        rdma_vbuf_secondary_pool_size = MVP_VBUF_SECONDARY_POOL_SIZE;
    } else {
        MVP_VBUF_SECONDARY_POOL_SIZE = rdma_vbuf_secondary_pool_size;
    }
    if (rdma_vbuf_secondary_pool_size <= 0) {
        if (rdma_memory_optimization) {
            rdma_vbuf_secondary_pool_size = RDMA_OPT_VBUF_SECONDARY_POOL_SIZE;
            MPL_usage_printf("Warning! Too small secondary vbuf pool size (%d)"
                             ". Reset to %d\n",
                             rdma_vbuf_secondary_pool_size,
                             RDMA_OPT_VBUF_SECONDARY_POOL_SIZE);
        } else {
            rdma_vbuf_secondary_pool_size = RDMA_VBUF_SECONDARY_POOL_SIZE;
            MPL_usage_printf("Warning! Too small secondary vbuf pool size (%d)"
                             ". Reset to %d\n",
                             rdma_vbuf_secondary_pool_size,
                             RDMA_VBUF_SECONDARY_POOL_SIZE);
        }
    }

    int i = 0;
    int result = 0;
    int alignment_dma = getpagesize();
#ifdef _ENABLE_CUDA_
    if (mvp_enable_device) {
        mvp_device_stage_block_size = MVP_CUDA_BLOCK_SIZE;
        mvp_device_num_rndv_blocks = MVP_CUDA_NUM_RNDV_BLOCKS;
    }
#endif /* defined(_ENABLE_CUDA_) */
#if defined(_ENABLE_UD_)
    if (!MVP_USE_ONLY_UD)
#endif
    {
#ifdef _ENABLE_CUDA_
        if (mvp_enable_device) {
            rdma_num_vbuf_pools = MVP_MAX_NUM_VBUF_POOLS;

            int default_vbuf_sizes[] = DEFAULT_CUDA_VBUF_SIZES;
            int default_vbuf_init_count[] = DEFAULT_CUDA_VBUF_POOL_SIZE;
            int default_vbuf_secondary_count[] =
                DEFAULT_CUDA_VBUF_SECONDARY_POOL_SIZE;

            rdma_vbuf_pools = MPL_aligned_alloc(
                alignment_dma, (sizeof(vbuf_pool_t) * rdma_num_vbuf_pools),
                MPL_MEM_OTHER);
            if (NULL == rdma_vbuf_pools) {
                ibv_error_abort(GEN_EXIT_ERR, "Unable to malloc vbuf_pool");
            }
            MPIR_Memset(rdma_vbuf_pools, 0,
                        sizeof(vbuf_pool_t) * rdma_num_vbuf_pools);
            MPIR_Memset(&mvp_srq_repost_pool, 0, sizeof(vbuf_pool_t));
            mvp_srq_repost_pool.buf_size = rdma_vbuf_total_size;
            mvp_srq_repost_pool.index = MVP_RECV_VBUF_POOL_OFFSET;

            for (i = 0; i < rdma_num_vbuf_pools; i++) {
                RDMA_VBUF_POOL_INIT(rdma_vbuf_pools[i]);
                rdma_vbuf_pools[i].buf_size = default_vbuf_sizes[i];
                rdma_vbuf_pools[i].incr_count = default_vbuf_secondary_count[i];
                rdma_vbuf_pools[i].initial_count = default_vbuf_init_count[i];
                /* Limit the number of VBUFs for large pool */
                if (i == MVP_LARGE_DATA_VBUF_POOL_OFFSET) {
                    rdma_vbuf_pools[i].max_num_buf = rdma_max_num_vbufs;
                }
                rdma_vbuf_pools[i].index = i;
            }
        } else
#endif /* defined(_ENABLE_CUDA_) */
        {
#ifdef _ENABLE_CUDA_
            /* If built with CUDA support, the last VBUF pool is for CUDA VBUF.
             * This last buffer is not needed if CUDA support is not enabled at
             * runtime */
            rdma_num_vbuf_pools = MVP_MAX_NUM_VBUF_POOLS - 1;
#else
            rdma_num_vbuf_pools = MVP_MAX_NUM_VBUF_POOLS;
#endif

            int default_vbuf_sizes[] = DEFAULT_VBUF_SIZES;
            int default_vbuf_init_count[] = DEFAULT_VBUF_POOL_SIZE;
            int default_vbuf_secondary_count[] =
                DEFAULT_VBUF_SECONDARY_POOL_SIZE;

            rdma_vbuf_pools = MPL_aligned_alloc(
                alignment_dma, (sizeof(vbuf_pool_t) * rdma_num_vbuf_pools),
                MPL_MEM_OTHER);
            if (NULL == rdma_vbuf_pools) {
                ibv_error_abort(GEN_EXIT_ERR, "Unable to malloc vbuf_pool");
            }
            MPIR_Memset(rdma_vbuf_pools, 0,
                        sizeof(vbuf_pool_t) * rdma_num_vbuf_pools);
            MPIR_Memset(&mvp_srq_repost_pool, 0, sizeof(vbuf_pool_t));
            mvp_srq_repost_pool.buf_size = rdma_vbuf_total_size;
            mvp_srq_repost_pool.index = MVP_RECV_VBUF_POOL_OFFSET;

            for (i = 0; i < rdma_num_vbuf_pools; i++) {
                RDMA_VBUF_POOL_INIT(rdma_vbuf_pools[i]);
                rdma_vbuf_pools[i].buf_size = default_vbuf_sizes[i];
                rdma_vbuf_pools[i].incr_count = default_vbuf_secondary_count[i];
                rdma_vbuf_pools[i].initial_count = default_vbuf_init_count[i];
                /* Limit the number of VBUFs for large pool */
                if (i == MVP_LARGE_DATA_VBUF_POOL_OFFSET) {
                    rdma_vbuf_pools[i].max_num_buf = rdma_max_num_vbufs;
                }
                rdma_vbuf_pools[i].index = i;
            }
        }
    }
#if defined(_ENABLE_UD_) || defined(_MCST_SUPPORT_)
    if (MVP_USE_UD_HYBRID
#if defined(_MCST_SUPPORT_)
        || rdma_enable_mcast
#endif
    ) {
        rdma_num_ud_vbuf_pools = MVP_MAX_NUM_UD_VBUF_POOLS;

        int default_ud_vbuf_sizes[] = DEFAULT_UD_VBUF_SIZES;
        int default_ud_vbuf_init_count[] = DEFAULT_UD_VBUF_POOL_SIZE;
        int default_ud_vbuf_secondary_count[] =
            DEFAULT_UD_VBUF_SECONDARY_POOL_SIZE;

        rdma_ud_vbuf_pools = MPL_aligned_alloc(
            alignment_dma, (sizeof(vbuf_pool_t) * rdma_num_ud_vbuf_pools),
            MPL_MEM_OTHER);
        if (NULL == rdma_ud_vbuf_pools) {
            ibv_error_abort(GEN_EXIT_ERR, "Unable to malloc vbuf_pool");
        }
        MPIR_Memset(rdma_ud_vbuf_pools, 0,
                    sizeof(vbuf_pool_t) * rdma_num_ud_vbuf_pools);
        MPIR_Memset(&mvp_ud_srq_repost_pool, 0, sizeof(vbuf_pool_t));
        mvp_ud_srq_repost_pool.buf_size = RDMA_MAX_UD_MTU;
        mvp_ud_srq_repost_pool.index = MVP_RECV_UD_VBUF_POOL_OFFSET;

        for (i = 0; i < rdma_num_ud_vbuf_pools; i++) {
            RDMA_VBUF_POOL_INIT(rdma_ud_vbuf_pools[i]);
            rdma_ud_vbuf_pools[i].buf_size = default_ud_vbuf_sizes[i];
            rdma_ud_vbuf_pools[i].incr_count =
                default_ud_vbuf_secondary_count[i];
            rdma_ud_vbuf_pools[i].initial_count = default_ud_vbuf_init_count[i];
            /* Limit the number of VBUFs for large pool */
            if (i == MVP_SEND_UD_VBUF_POOL_OFFSET) {
                rdma_ud_vbuf_pools[i].max_num_buf = rdma_max_num_ud_vbufs;
            }
            rdma_ud_vbuf_pools[i].index = i;
        }
    }
#endif /*defined(_ENABLE_UD_) || defined(_MCST_SUPPORT_)*/
    return;
}

/* This function is specifically written to make sure that HSAM
 * parameters are configured correctly */

#ifdef _ENABLE_HSAM_
static int check_hsam_parameters(void)
{
    char *value;
    int size;

    /* Get the number of processes */
    UPMI_GET_SIZE(&size);

    /* If the number of processes is less than 64, we can afford * to
     * have more RC QPs and hence a value of 4 is chosen, for * other
     * cases, a value of 2 is chosen */

    /* (rdma_num_qp_per_port/ stripe factor) represents the number
     * of QPs which will be chosen for data transfer at a given point */

    /* If the user has not specified any value, then perform
     * this tuning */
    if (MVP_NUM_QP_PER_PORT != -1) {
        rdma_num_qp_per_port = MVP_NUM_QP_PER_PORT;
        if (rdma_num_qp_per_port <= 2) {
            stripe_factor = 1;
        } else {
            stripe_factor = (rdma_num_qp_per_port / 2);
        }
    } else {
        /* Speculated value */

        /* The congestion is actually never seen for less
         * than 8 nodes */
        if ((size > 8) && (size < 64)) {
            rdma_num_qp_per_port = 4;
            stripe_factor = (rdma_num_qp_per_port / 2);
        } else {
            rdma_num_qp_per_port = 2;
            stripe_factor = 1;
        }
    }
    MVP_NUM_QP_PER_PORT = rdma_num_qp_per_port;
    ;
    return MPI_SUCCESS;
}
#endif /* _ENABLE_HSAM_ */

void rdma_get_pm_parameters(mvp_MPIDI_CH3I_RDMA_Process_t *proc)
{
    int ring_setup, i;
    char *value;
    int my_rank = -1;

    MPIR_T_MVP_cvar_init();

    UPMI_GET_RANK(&my_rank);

    /* still required as this has not been correctly integrated */
    if ((value = getenv("MPIRUN_RSH_LAUNCH")) != NULL) {
        MVP_MPIRUN_RSH_LAUNCH = 1;
    }
    using_mpirun_rsh = MVP_MPIRUN_RSH_LAUNCH;
#if defined(RDMA_CM)
    if (MVP_USE_RDMA_CM != -1) {
#if !defined(ROMIO_IME)
        proc->use_rdma_cm = !!MVP_USE_RDMA_CM;
#else
        if (MVP_USE_RDMA_CM && (my_rank == 0)) {
            MPL_error_printf("Error: IME FS does not work with RDMA CM. "
                             "Proceeding without RDMA support.\n");
        }
#endif
#ifdef _ENABLE_XRC_
        /* XRC will not work with RDMA_CM */
        if (proc->use_rdma_cm) {
            USE_XRC = 0;
            if ((MVP_USE_XRC != -1) && (my_rank == 0)) {
                if (MVP_USE_XRC) {
                    MPL_error_printf("Error: XRC does not work with RDMA CM. "
                                     "Proceeding without XRC support.\n");
                }
            }
        }
#endif
    }
#endif

    if (MVP_USE_ROCE_MODE != -1) {
        if (MVP_USE_ROCE_MODE != MVP_ROCE_MODE_V1 &&
            MVP_USE_ROCE_MODE != MVP_ROCE_MODE_V2) {
            PRINT_INFO(
                my_rank == 0,
                "Value for MVP_USE_ROCE_MODE must either be 1 or 2. Falling"
                " back to the latest RoCE version available on system.\n");
            /* Use RoCEv2 if avaiable. If not avaialble, we fall back to RoCEv1
             */
            MVP_USE_ROCE_MODE = MVP_ROCE_MODE_V2;
        }
        mvp_use_roce_mode = MVP_USE_ROCE_MODE;
        /* Automatically set use_iboeth = 1 if ROCE mode is set */
        MVP_USE_ROCE = use_iboeth = 1;
    } else if (MVP_USE_ROCE) {
        /* TODO: have a propper default setting */
        use_iboeth = !!MVP_USE_ROCE;
        MVP_USE_ROCE_MODE = mvp_use_roce_mode;
    }

    switch (MPIDI_CH3I_Process.cm_type) {
        case MPIDI_CH3I_CM_ON_DEMAND:
#if defined(RDMA_CM)
        case MPIDI_CH3I_CM_RDMA_CM:
#endif
            ring_setup = 1;
            if (using_mpirun_rsh) {
                ring_setup = 0;
#ifdef _ENABLE_XRC_
                if (USE_XRC) {
                    ring_setup = 1;
                }
#endif
            }
            if (MVP_USE_RING_STARTUP != -1) {
                proc->has_ring_startup = !!MVP_USE_RING_STARTUP;
            } else {
                proc->has_ring_startup = ring_setup;
            }
#ifdef CKPT
            if (!using_mpirun_rsh)
                proc->has_ring_startup = 0;

#endif /* CKPT */
            break;
        default:
            if (MVP_USE_RING_STARTUP != -1) {
                proc->has_ring_startup = !!MVP_USE_RING_STARTUP;
            } else {
                proc->has_ring_startup = 0;
            }
#ifdef CKPT
            if (!using_mpirun_rsh)
                proc->has_ring_startup = 0;
#endif /* CKPT */

            break;
    }

#ifdef _ENABLE_UD_
    if (MVP_USE_UD_HYBRID) {
        proc->has_ring_startup = 0;
    }
#endif
    if (MVP_ON_DEMAND_UD_INFO_EXCHANGE != -1) {
        mvp_on_demand_ud_info_exchange = !!MVP_ON_DEMAND_UD_INFO_EXCHANGE;
        if (mvp_on_demand_ud_info_exchange) {
            proc->has_ring_startup = 0;
        }
    }

#if defined(HAVE_PMI2_IALLGATHER) && defined(HAVE_PMI2_IALLGATHER_WAIT)
    /* prioritize Iallgather */
    mvp_use_pmi_iallgather = !!MVP_USE_PMI_IALLGATHER;
    if (mvp_use_pmi_iallgather) {
        MVP_USE_PMI_IBARRIER = 0;
        mvp_on_demand_ud_info_exchange = 1;
        proc->has_ring_startup = 0;
    }
#endif
#if (defined(HAVE_PMI2_KVS_IFENCE) && defined(HAVE_PMI2_KVS_WAIT)) ||          \
    (defined(HAVE_PMI_IBARRIER) && defined(HAVE_PMI_WAIT))
    /* on by default */
    if (!!MVP_USE_PMI_IBARRIER && !MVP_USE_PMI_IALLGATHER) {
        mvp_use_pmi_ibarrier = MVP_USE_PMI_IBARRIER = !!MVP_USE_PMI_IBARRIER;
        mvp_on_demand_ud_info_exchange = 1;
        proc->has_ring_startup = 0;
    }
#endif

    if (proc->has_ring_startup) {
        /* The on demand ud info is temporarily disabled
         * Revisit the issue later */
        mvp_on_demand_ud_info_exchange = 0;
    }

    if (MVP_NUM_HCAS) {
        rdma_num_req_hcas = MVP_NUM_HCAS;

        rdma_multirail_usage_policy = MVP_MRAIL_SHARING;

        if (rdma_num_req_hcas > MAX_NUM_HCAS) {
            rdma_num_req_hcas = MAX_NUM_HCAS;

            MPL_msg_printf("Warning, max hca is %d, change %s in ibv_param.h "
                           "to override the option\n",
                           MAX_NUM_HCAS, "MAX_NUM_HCAS");
        }
    }

    for (i = 0; i < MAX_NUM_HCAS; ++i) {
        strncpy(rdma_iba_hcas[i], RDMA_IBA_NULL_HCA, 32);
    }

    if (MVP_IBA_HCA != NULL) {
        rdma_multirail_usage_policy = MVP_MRAIL_SHARING;
        rdma_num_req_hcas = 0;
        {
            char *tok = NULL;

            tok = strtok(MVP_IBA_HCA, ":");
            while (tok != NULL) {
                strncpy(rdma_iba_hcas[rdma_num_req_hcas], tok, 32);
                tok = strtok(NULL, ":");
                DEBUG_PRINT("tok = %s, hca name = %s, hca num = %d\n", tok,
                            rdma_iba_hcas[rdma_num_req_hcas],
                            rdma_num_req_hcas);
                rdma_num_req_hcas++;
            }
        }
    }

    mvp_homogeneous_cluster = MVP_HOMOGENEOUS_CLUSTER;
}

void mvp_print_env_info(mvp_MPIDI_CH3I_RDMA_Process_t *proc,
                        struct coll_info *colls_arch_hca)
{
    int i = 0;
    mvp_arch_type arch_type = MVP_GET_ARCH(proc->arch_hca_type);
    mvp_hca_type hca_type = MVP_GET_HCA(proc->arch_hca_type);
    mvp_cpu_family_type family_type = mvp_get_cpu_family();
    fprintf(stderr, "\n MVAPICH-%s Parameters\n", MPII_Version_string);
    fprintf(stderr, "----------------------------------------------------------"
                    "-----------\n");
    fprintf(stderr, "\tPROCESSOR ARCH NAME            : %s\n",
            mvp_get_arch_name(arch_type));
    fprintf(stderr, "\tPROCESSOR FAMILY NAME          : %s\n",
            mvp_get_cpu_family_name(family_type));
    fprintf(stderr, "\tPROCESSOR MODEL NUMBER         : %d\n",
            mvp_get_cpu_model());
    fprintf(stderr, "\tHCA NAME                       : %s\n",
            mvp_get_hca_name(hca_type));
    fprintf(stderr, "\tHETEROGENEOUS HCA              : %s\n",
            (mvp_MPIDI_CH3I_RDMA_Process.heterogeneity) ? "YES" : "NO");
    if (!SMP_ONLY) {
        fprintf(stderr, "\tMVP_VBUF_TOTAL_SIZE            : %d\n",
                rdma_vbuf_total_size);
        fprintf(stderr, "\tMVP_IBA_EAGER_THRESHOLD        : %d\n",
                rdma_iba_eager_threshold);
        fprintf(stderr, "\tMVP_RDMA_FAST_PATH_BUF_SIZE    : %d\n",
                rdma_fp_buffer_size);
        fprintf(stderr, "\tMVP_PUT_FALLBACK_THRESHOLD     : %d\n",
                rdma_put_fallback_threshold);
        fprintf(stderr, "\tMVP_GET_FALLBACK_THRESHOLD     : %d\n",
                rdma_get_fallback_threshold);
    }
    fprintf(stderr, "\tMVP_EAGERSIZE_1SC              : %lu\n",
            rdma_eagersize_1sc);
    fprintf(stderr, "\tMVP_SMP_EAGERSIZE              : %d\n",
            MVP_SMP_EAGERSIZE);
    fprintf(stderr, "\tMVP_SMP_QUEUE_LENGTH           : %zu\n",
            MVP_SMP_QUEUE_LENGTH);
    fprintf(stderr, "\tMVP_SMP_NUM_SEND_BUFFER        : %d\n",
            MVP_SMP_NUM_SEND_BUFFER);
    fprintf(stderr, "\tMVP_SMP_BATCH_SIZE             : %d\n",
            MVP_SMP_BATCH_SIZE);
    fprintf(stderr, "\t%s                  : %s %s\n",
            "Tuning Table:", mvp_get_arch_name(colls_arch_hca[0].arch_type),
            mvp_get_hca_name(colls_arch_hca[0].hca_type));
    fprintf(stderr, "----------------------------------------------------------"
                    "-----------\n");
    if (mvp_show_env_info >= 2) {
        mvp_show_all_params();
        fprintf(stderr, "------------------------------------------------------"
                        "---------------\n");
    }

    if (mvp_show_env_info >= 3) {
        fprintf(stderr, "\nCollective Tuning Tables\n");
        fprintf(stderr, "\t%-20s %-40s %-40s\n", "Collective", "Architecture",
                "Interconnect");
        for (i = 0; i < colls_max; i++) {
            fprintf(stderr, "\t%-20s %-40s %-40s\n", collective_names[i],
                    mvp_get_arch_name(colls_arch_hca[i].arch_type),
                    mvp_get_hca_name(colls_arch_hca[i].hca_type));
        }
        fprintf(stderr, "\n----------------------------------------------------"
                        "-----------------\n");
    }
}
/* vi:set sw=4 */
