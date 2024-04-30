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

#ifndef _IBV_PARAM_H
#define _IBV_PARAM_H

#include <infiniband/verbs.h>
#include "mvp_debug_utils.h"
#include "mvp_arch_hca_detect.h"

typedef enum _mvp_roce_mode_t {
    MVP_ROCE_MODE_V1 = 1,
    MVP_ROCE_MODE_V2 = 2
} mvp_roce_mode_t;

extern int mvp_use_opt_eager_recv;
extern int mvp_is_in_finalize;
/* Support multiple QPs/port, multiple ports, multiple HCAs and combinations */
extern int rdma_num_hcas;
extern int mvp_closest_hca_offset;
extern int rdma_num_req_hcas;
extern int rdma_num_ports;
extern int rdma_num_qp_per_port;
extern int rdma_num_rails;
extern int mvp_cm_wait_time;

extern unsigned long rdma_default_max_cq_size;
extern int rdma_num_cqes_per_poll;
extern int rdma_default_port;
extern int rdma_default_gid_index;
extern int rdma_default_max_send_wqe;
extern int rdma_default_max_recv_wqe;
extern uint32_t rdma_default_max_sg_list;
extern uint16_t rdma_default_pkey_ix;
extern uint16_t rdma_default_pkey;
extern uint32_t rdma_default_qkey;
extern uint8_t rdma_default_qp_ous_rd_atom;
extern uint8_t rdma_supported_max_qp_ous_rd_atom;
extern uint8_t rdma_default_max_rdma_dst_ops;
extern uint8_t rdma_supported_max_rdma_dst_ops;
extern enum ibv_mtu rdma_default_mtu;
extern uint32_t rdma_default_psn;
extern uint8_t rdma_default_min_rnr_timer;
extern uint8_t rdma_default_service_level;
extern uint8_t rdma_default_static_rate;
extern uint8_t rdma_default_src_path_bits;
extern uint8_t rdma_default_time_out;
extern uint8_t rdma_default_retry_count;
extern uint8_t rdma_default_rnr_retry;
extern int rdma_default_put_get_list_size;
extern float rdma_credit_update_threshold;
extern int num_rdma_buffer;
extern int rdma_iba_eager_threshold;
extern int rdma_ndreg_entries;
extern int rdma_ndreg_entries_max;
extern int rdma_vbuf_max;
extern int rdma_vbuf_pool_size;
extern int rdma_vbuf_secondary_pool_size;
extern int rdma_initial_prepost_depth;
extern int rdma_prepost_depth;
extern int rdma_prepost_threshold;
extern int rdma_prepost_noop_extra;
extern int rdma_initial_credits;
extern int rdma_prepost_rendezvous_extra;
extern int rdma_dynamic_credit_threshold;
extern int rdma_credit_notify_threshold;
extern int rdma_credit_preserve;
extern int rdma_rq_size;
extern unsigned long rdma_dreg_cache_limit;
extern int rdma_rndv_immediate;
extern int rdma_r3_threshold;
extern int rdma_intra_node_r3_threshold;
extern int rdma_inter_node_r3_threshold;
extern int rdma_r3_threshold_nocache;
extern int rdma_max_r3_pending_data;
extern int rdma_vbuf_total_size;
extern int rdma_max_num_vbufs;
extern int rdma_max_inline_size;
extern int rdma_local_id;
extern int rdma_num_local_procs;

extern uint32_t mvp_srq_alloc_size;
extern uint32_t mvp_srq_fill_size;
extern uint32_t mvp_srq_limit;
extern uint32_t mvp_max_r3_oust_send;
#if defined(_ENABLE_UD_)
extern uint32_t mvp_ud_srq_alloc_size;
extern uint32_t mvp_ud_srq_fill_size;
extern uint32_t mvp_ud_srq_limit;
#endif /*defined(_ENABLE_UD_)*/
extern int rdma_polling_set_threshold;
extern int rdma_polling_set_limit;
extern int rdma_fp_buffer_size;
extern int rdma_fp_sendconn_accepted;
extern int rdma_pending_conn_request;
extern int rdma_eager_limit;
extern int rdma_rndv_ext_sendq_size;
extern int rdma_global_ext_sendq_size;
extern int rdma_num_extra_polls;

extern int rdma_pin_pool_size;
extern int rdma_put_fallback_threshold;
extern int rdma_get_fallback_threshold;
extern int rdma_iba_eager_threshold;
extern long rdma_eagersize_1sc;
extern int rdma_qos_num_sls;
extern int rdma_use_qos;
extern int rdma_3dtorus_support;
extern int rdma_path_sl_query;
extern int rdma_num_sa_query_retries;
extern int rdma_multirail_usage_policy;
extern int rdma_small_msg_rail_sharing_policy;
extern int rdma_med_msg_rail_sharing_policy;
extern int rdma_med_msg_rail_sharing_threshold;
extern int rdma_large_msg_rail_sharing_threshold;

extern int mvp_use_pmi_ibarrier;
extern int mvp_use_pmi_iallgather;
extern int mvp_on_demand_ud_info_exchange;
extern int mvp_homogeneous_cluster;
extern int mvp_show_env_info;
extern int mvp_use_pmi_ibarrier;
extern int mvp_show_runlog_level;
extern int mvp_system_has_roce;
extern int mvp_system_has_rockport;
extern int mvp_system_has_ib;
extern int mvp_process_placement_aware_hca_mapping;
extern int mvp_allow_heterogeneous_hca_selection;
/* HSAM Definitions */

extern int striping_threshold;
extern int rdma_rail_sharing_policy;
extern double alpha;
extern int stripe_factor;
extern int apm_tester;

extern int rdma_coalesce_threshold;
extern int rdma_use_coalesce;

extern int rdma_use_blocking;
extern unsigned long rdma_blocking_spin_count_threshold;
extern unsigned long rdma_polling_spin_count_threshold;
extern int mvp_use_thread_yield;
extern int mvp_spins_before_lock;
extern int rdma_use_xrc;
extern int xrc_rdmafp_init;
extern int rdma_use_smp;
extern int mvp_use_smp;
extern int use_iboeth;
extern mvp_roce_mode_t mvp_use_roce_mode;
extern int mvp_use_post_srq_send;
extern int rdma_iwarp_multiple_cq_threshold;
extern int rdma_iwarp_use_multiple_cq;
extern int using_mpirun_rsh;

extern int use_hwloc_cpu_binding;
extern int max_rdma_connect_attempts;
#ifdef _MULTI_SUBNET_SUPPORT_
extern int mvp_rdma_cm_multi_subnet_support;
#endif /* _MULTI_SUBNET_SUPPORT_ */
extern int rdma_cm_connect_retry_interval;
extern int rdma_num_rails_per_hca;
extern int rdma_process_binding_rail_offset;
extern int g_atomics_support;
extern int g_atomics_support_be;

extern int rdma_enable_hugepage;

#ifdef _ENABLE_CUDA_
extern int mvp_enable_device;
extern int mvp_device_stage_block_size;
extern int mvp_device_num_rndv_blocks;
extern int mvp_device_event_count;
extern int rdma_enable_cuda;
extern int mvp_device_dynamic_init;
extern int rdma_cuda_vec_thread_blksz;
extern int rdma_cuda_vec_thread_ysz;
extern int rdma_cuda_subarr_thread_blksz;
extern int rdma_cuda_subarr_thread_xdim;
extern int rdma_cuda_subarr_thread_ydim;
extern int rdma_cuda_subarr_thread_zdim;
extern int mvp_device_nonblocking_streams;
extern int rdma_eager_devicehost_reg;
extern int rdma_cuda_vector_dt_opt;
extern int rdma_cuda_kernel_dt_opt;
extern int mvp_device_initialized;
#if defined(HAVE_CUDA_IPC)
extern int mvp_device_use_ipc;
extern int rdma_enable_ipc_share_gpu;
extern int mvp_device_use_smp_eager_ipc;
extern int mvp_device_enable_ipc_cache;
extern int mvp_device_ipc_threshold;
extern int mvp_device_ipc_cache_max_entries;
#endif /*#if defined(HAVE_CUDA_IPC) */
extern int mvp_device_coll_use_stage;
extern int mvp_device_coll_register_stage_buf_threshold;
extern int mvp_device_gather_stage_limit;
extern int mvp_device_scatter_stage_limit;
extern int mvp_device_gatherv_stage_limit;
extern int mvp_device_scatterv_stage_limit;
extern int mvp_device_allgather_stage_limit;
extern int mvp_device_allgatherv_stage_limit;
extern int mvp_device_alltoall_stage_limit;
extern int mvp_device_alltoallv_stage_limit;
extern int mvp_device_bcast_stage_limit;
extern int mvp_device_alltoall_dynamic;
extern int mvp_device_allgather_rd_limit;
extern int mvp_device_use_allgather_fgp;
extern int mvp_device_init_context;
extern int mvp_device_check_attribute;
#endif /*#ifdef _ENABLE_CUDA_ */

#define MVP_DEFAULT_UD_MTU 2048
extern uint16_t rdma_default_ud_mtu;
#if defined(_ENABLE_UD_)
extern uint8_t rdma_use_ud_srq;
extern uint8_t rdma_use_ud_zcopy;
extern uint8_t rdma_ud_zcopy_enable_polling;
extern uint8_t rdma_ud_zcopy_push_segment;
extern uint32_t rdma_hybrid_enable_threshold;
extern uint32_t rdma_default_max_ud_send_wqe;
extern uint32_t rdma_default_max_ud_recv_wqe;
extern uint32_t rdma_default_ud_sendwin_size;
extern uint32_t rdma_default_ud_recvwin_size;
extern long rdma_ud_progress_timeout;
extern long rdma_ud_retry_timeout;
extern long rdma_ud_max_retry_timeout;
extern long rdma_ud_last_check;
extern uint16_t rdma_ud_max_retry_count;
extern uint16_t rdma_ud_progress_spin;
extern uint16_t rdma_ud_max_ack_pending;
extern uint16_t rdma_ud_num_rndv_qps;
extern uint32_t rdma_ud_num_msg_limit;
extern uint32_t rdma_ud_vbuf_pool_size;
extern uint32_t rdma_ud_zcopy_threshold;
extern uint32_t rdma_ud_zcopy_rq_size;
extern uint32_t rdma_ud_zcopy_num_retry;
extern uint16_t rdma_hybrid_max_rc_conn;
extern uint16_t rdma_hybrid_pending_rc_conn;
#ifdef _MVP_UD_DROP_PACKET_RATE_
extern uint32_t ud_drop_packet_rate;
#endif
extern int rdma_max_num_ud_vbufs;
#endif
#if defined(_MCST_SUPPORT_)
extern uint32_t mcast_bcast_min_msg;
extern uint32_t mcast_bcast_max_msg;
extern uint8_t rdma_enable_mcast;
#if defined(RDMA_CM)
extern uint8_t rdma_use_rdma_cm_mcast;
#endif /*defined(RDMA_CM)*/
extern uint8_t mcast_enable_rel;
extern uint8_t mcast_use_mcast_nack;
extern uint16_t mcast_window_size;
extern uint16_t mcast_drop_packet_rate;
extern uint32_t mcast_num_nodes_threshold;
extern uint32_t mcast_max_ud_recv_wqe;
extern long mcast_retry_timeout;
extern long mcast_max_retry_timeout;
extern long mcast_comm_init_timeout;
extern int mcast_comm_init_retries;
extern int mcast_nspin_threshold;
extern int mcast_skip_loopback;
#endif
extern int mvp_enable_progress_affinity;
extern int rdma_default_async_thread_stack_size;

#define PKEY_MASK                      0x7fff /* the last bit is reserved */
#define RDMA_MAX_CQE_ENTRIES_PER_POLL  (96)
#define RDMA_PIN_POOL_SIZE             (2 * 1024 * 1024)
#define RDMA_DEFAULT_MAX_CQ_SIZE       (40000)
#define RDMA_DEFAULT_IWARP_CQ_SIZE     (8192)
#define RDMA_DEFAULT_PORT              (-1)
#define RDMA_DEFAULT_GID_INDEX         (0)
#define RDMA_DEFAULT_MAX_PORTS         (2)
#define RDMA_DEFAULT_MAX_SEND_WQE      (64)
#define RDMA_DEFAULT_MAX_RECV_WQE      (128)
#define RDMA_DEFAULT_MIN_UD_ZCOPY_WQE  (1)
#define RDMA_DEFAULT_MAX_UD_SEND_WQE   (256)
#define RDMA_DEFAULT_MAX_UD_RECV_WQE   (4096)
#define RDMA_UD_NUM_MSG_LIMIT          (100)
#define RDMA_DEFAULT_UD_SENDWIN_SIZE   (400)
#define RDMA_DEFAULT_MAX_SG_LIST       (1)
#define RDMA_DEFAULT_PKEY_IX           (0)
#define RDMA_DEFAULT_PKEY              (0x0)
#define RDMA_DEFAULT_QKEY              (0)
#define RDMA_DEFAULT_MAX_RDMA_DST_OPS  (4)
#define RDMA_DEFAULT_PSN               (0)
#define RDMA_DEFAULT_MIN_RNR_TIMER     (12)
#define RDMA_DEFAULT_SERVICE_LEVEL     (0)
#define RDMA_DEFAULT_STATIC_RATE       (0)
#define RDMA_DEFAULT_SRC_PATH_BITS     (0)
#define RDMA_DEFAULT_TIME_OUT          (20)
#define RDMA_DEFAULT_RETRY_COUNT       (7)
#define RDMA_DEFAULT_RNR_RETRY         (7)
#define RDMA_DEFAULT_PUT_GET_LIST_SIZE (200)
#define RDMA_IBA_NULL_HCA              "nohca"
#define RDMA_DEFAULT_POLLING_SET_LIMIT (64)
#define RDMA_FP_DEFAULT_BUF_SIZE       (4096)
#define RDMA_UD_DEFAULT_NUM_RNDV_QPS   (64)
/* DGX-2 boxes have 8 or 9 HCAs. Updating MAX_NUM_HCAS to 10 */
#define MAX_NUM_HCAS (10)
#ifndef MAX_NUM_PORTS
/* Even multi-port HCAs are being detected as multiple, single
 * port HCAS. Reduce MAX_NUM_PORTS to offset the increase in
 * MAX_NUM_HCAS is going to have on on-demand messages */
#define MAX_NUM_PORTS (1)
#endif
#ifndef MAX_NUM_QP_PER_PORT
#define MAX_NUM_QP_PER_PORT (4)
#endif
#define MVP_ROCKPORT_NUM_QP_PER_PORT                  (4)
#define RDMA_QOS_MAX_NUM_SLS                          (15)
#define RDMA_QOS_DEFAULT_NUM_SLS                      (8)
#define RDMA_DEFAULT_NUM_SA_QUERY_RETRIES             (20)
#define RDMA_DEFAULT_MED_MSG_RAIL_SHARING_THRESHOLD   (2048)
#define RDMA_DEFAULT_LARGE_MSG_RAIL_SHARING_THRESHOLD (16384)
#define DEF_MVP_CM_WAIT_TIME                          (5)
#define RDMA_DEFAULT_QP_OUS_RD_ATOM                   (1)
#define RDMA_DEFAULT_R3_THRESHOLD                     (4096)
#define RDMA_DEFAULT_INTER_NODE_R3_THRESHOLD          (4096)
#define RDMA_DEFAULT_INTRA_NODE_R3_THRESHOLD          (1)

/* This is a overprovision of resource, do not use in critical structures */
#define MAX_NUM_SUBRAILS (MAX_NUM_HCAS * MAX_NUM_PORTS * MAX_NUM_QP_PER_PORT)

#define RDMA_NDREG_ENTRIES                (8192)
#define RDMA_NDREG_ENTRIES_MAX            (16384)
#define RDMA_NDREG_ENTRIES_MIN            (4096)
#define RDMA_VBUF_POOL_SIZE               (512)
#define RDMA_OPT_VBUF_POOL_SIZE           (80)
#define RDMA_UD_VBUF_POOL_SIZE            (8192)
#define RDMA_UD_SRQ_VBUF_POOL_SIZE        (8192)
#define RDMA_MIN_VBUF_POOL_SIZE           (512)
#define RDMA_OPT_MIN_VBUF_POOL_SIZE       (32)
#define RDMA_VBUF_SECONDARY_POOL_SIZE     (256)
#define RDMA_OPT_VBUF_SECONDARY_POOL_SIZE (16)
#define RDMA_PREPOST_DEPTH                (64)
#define RDMA_INITIAL_PREPOST_DEPTH        (10)
#define RDMA_LOW_WQE_THRESHOLD            (10)
#define RDMA_MAX_RDMA_SIZE                (4194304)
#define DEFAULT_RDMA_CONNECT_ATTEMPTS     (20)
#define RDMA_DEFAULT_CONNECT_INTERVAL     (100)

#define DEFAULT_SMALL_VBUF_SIZE  (256)
#define DEFAULT_MEDIUM_VBUF_SIZE (5120)

#define DEFAULT_MAX_NUM_VBUFS (8192)

#ifdef _ENABLE_CUDA_
#define DEFAULT_CUDA_VBUF_SIZES                                                \
    {                                                                          \
        DEFAULT_SMALL_VBUF_SIZE, DEFAULT_MEDIUM_VBUF_SIZE,                     \
            rdma_vbuf_total_size, mvp_device_stage_block_size,                 \
            mvp_device_stage_block_size                                        \
    }
#define DEFAULT_CUDA_VBUF_POOL_SIZE                                            \
    {                                                                          \
        rdma_vbuf_pool_size, rdma_vbuf_pool_size, rdma_vbuf_pool_size,         \
            rdma_vbuf_pool_size, rdma_vbuf_pool_size                           \
    }
#define DEFAULT_CUDA_VBUF_SECONDARY_POOL_SIZE                                  \
    {                                                                          \
        rdma_vbuf_secondary_pool_size, rdma_vbuf_secondary_pool_size,          \
            rdma_vbuf_secondary_pool_size, rdma_vbuf_secondary_pool_size,      \
            rdma_vbuf_secondary_pool_size                                      \
    }
#define DEFAULT_CUDA_BLOCK_SIZE (262144)
#endif

#define DEFAULT_VBUF_SIZES                                                     \
    {                                                                          \
        DEFAULT_SMALL_VBUF_SIZE, DEFAULT_MEDIUM_VBUF_SIZE,                     \
            rdma_vbuf_total_size, rdma_vbuf_total_size                         \
    }
#define DEFAULT_VBUF_POOL_SIZE                                                 \
    {                                                                          \
        rdma_vbuf_pool_size, rdma_vbuf_pool_size, rdma_vbuf_pool_size,         \
            rdma_vbuf_pool_size                                                \
    }
#define DEFAULT_VBUF_SECONDARY_POOL_SIZE                                       \
    {                                                                          \
        rdma_vbuf_secondary_pool_size, rdma_vbuf_secondary_pool_size,          \
            rdma_vbuf_secondary_pool_size, rdma_vbuf_secondary_pool_size       \
    }

#if defined(_ENABLE_UD_)
#define RDMA_MAX_UD_MTU (4096)
#define DEFAULT_UD_VBUF_SIZES                                                  \
    {                                                                          \
        DEFAULT_SMALL_VBUF_SIZE, RDMA_MAX_UD_MTU, RDMA_MAX_UD_MTU              \
    }
#define DEFAULT_UD_VBUF_POOL_SIZE                                              \
    {                                                                          \
        rdma_ud_vbuf_pool_size, rdma_ud_vbuf_pool_size, rdma_ud_vbuf_pool_size \
    }
#define DEFAULT_UD_VBUF_SECONDARY_POOL_SIZE                                    \
    {                                                                          \
        rdma_ud_vbuf_pool_size, rdma_ud_vbuf_pool_size, rdma_ud_vbuf_pool_size \
    }
#define DEFAULT_MAX_NUM_UD_VBUFS (INT_MAX)
#endif /*defined(_ENABLE_UD_)*/

#define RDMA_IWARP_DEFAULT_MULTIPLE_CQ_THRESHOLD (32)
#define RDMA_DEFAULT_ASYNC_THREAD_STACK_SIZE     (1 << 20)

/* Inline not supported for PPC */
#define HOSTNAME_LEN              (255)
#define RDMA_MAX_REGISTERED_PAGES (0)

/* SRQ related parameters */
#define MVP_DEFAULT_SRQ_ALLOC_SIZE 8192
#define MVP_DEFAULT_SRQ_FILL_SIZE  256
#define MVP_DEFAULT_SRQ_LIMIT      30

#define MVP_DEFAULT_UD_SRQ_ALLOC_SIZE 32767
#define MVP_DEFAULT_UD_SRQ_FILL_SIZE  4096
#define MVP_DEFAULT_UD_SRQ_LIMIT      128

/* #define MIN(a,b) ((a)<(b)?(a):(b)) */

#define NUM_BOOTSTRAP_BARRIERS 2

/* Statistically sending a stripe below this may not lead
 * to benefit */
#define STRIPING_THRESHOLD 8 * 1024

/* Parameters specific to DGX-2 A100 nodes at ALCF */
#define MVP_DGX2_A100_NUM_HCAS            12
#define MVP_DGX2_A100_SAN_HCA_MAX_DM_SIZE 131072

extern char rdma_iba_hcas[MAX_NUM_HCAS][32];

typedef enum _mvp_vbuf_pool_offsets {
    MVP_SMALL_DATA_VBUF_POOL_OFFSET = 0,
    MVP_MEDIUM_DATA_VBUF_POOL_OFFSET,
    MVP_LARGE_DATA_VBUF_POOL_OFFSET,
    MVP_RECV_VBUF_POOL_OFFSET,
#ifdef _ENABLE_CUDA_
    MVP_CUDA_VBUF_POOL_OFFSET,
#endif /*_ENABLE_CUDA_*/
    MVP_MAX_NUM_VBUF_POOLS
} mvp_vbuf_pool_offsets;

#if defined(_ENABLE_UD_)
typedef enum _mvp_ud_vbuf_pool_offsets {
    MVP_SMALL_SEND_UD_VBUF_POOL_OFFSET = 0,
    MVP_SEND_UD_VBUF_POOL_OFFSET,
    MVP_RECV_UD_VBUF_POOL_OFFSET,
    MVP_MAX_NUM_UD_VBUF_POOLS
} mvp_ud_vbuf_pool_offsets;
#endif /*defined(_ENABLE_UD_)*/

/* Below ROUND_ROBIN refers to the rails where the rails are alternately
 * given to any process asking for it. Where as FIXED_MAPPING refers
 * to a scheduling policy where processes are bound to rails in a round
 * robin manner. So once a process is bound to a rail it will use only
 * that rail to send out messages */

typedef enum _mvp_multirail_policies {
    MVP_MRAIL_BINDING = 0,
    MVP_MRAIL_SHARING,
} mvp_multirail_policies;

typedef enum _mvp_rail_sharing_policies {
    ROUND_ROBIN = 0,
    USE_FIRST,
    EVEN_STRIPING,
    ADAPTIVE_STRIPING,
    FIXED_MAPPING,
    PARTIAL_ADAPTIVE,
    BEST_ADAPTIVE
} mvp_rail_sharing_policies;

/* This is to allow users to specify rail mapping at run time */
extern int mrail_use_default_mapping;
extern int mrail_user_defined_p2r_mapping;
extern char *mrail_p2r_string;
extern int mrail_p2r_length;

#define DYNAMIC_TOTAL_WEIGHT (3 * 1024)

#define CHELSIO_RNIC     "cxgb"
#define INTEL_NE020_RNIC "nes0"

/* MVP_POLLING_LEVEL
Level 1 : Exit on finding a message on any channel
Level 2 : Exit on finding a message on RDMA_FP or SMP channel.
          Continue on ibv_poll_cq success.
Level 3 : Exit on finding a message on RDMA_FP channel.
          Continue polling on SMP and ibv_poll_cq channels
          until no more messages.
Level 4 : Exit only after processing all the messages on
          all the channels
*/
typedef enum mvp_polling_level {
    MVP_POLLING_LEVEL_1 = 1,
    MVP_POLLING_LEVEL_2,
    MVP_POLLING_LEVEL_3,
    MVP_POLLING_LEVEL_4,
    MVP_POLLING_LEVEL_MAX,
} mvp_polling_level;

extern mvp_polling_level rdma_polling_level;

/* enum list of MVP runtime environment variables */
typedef enum mvp_env_param_id {
    /* mpirun_rsh */
    MVP_COMM_WORLD_LOCAL_RANK,
    PMI_ID,
    MPIRUN_COMM_MULTIPLE,
    MPISPAWN_BINARY_PATH,
    MPISPAWN_CR_CKPT_CNT,
    MPISPAWN_CR_CONTEXT,
    MPISPAWN_CR_SESSIONID,
    MPISPAWN_GLOBAL_NPROCS,
    MPISPAWN_MPIRUN_CR_PORT,
    MPISPAWN_MPIRUN_HOST,
    MPISPAWN_MPIRUN_ID,
    MPISPAWN_NNODES,
    MPISPAWN_WORKING_DIR,
    MPIEXEC_TIMEOUT,
    MPISPAWN_USE_TOTALVIEW,
    MVP_FASTSSH_THRESHOLD,
    MVP_MPIRUN_TIMEOUT,
    MVP_MT_DEGREE,
    MVP_NPROCS_THRESHOLD,
    USE_LINEAR_SSH,
    PMI_SUBVERSION,
    PMI_VERSION,
    PMI_PORT,
    PARENT_ROOT_PORT_NAME,
    /* ckpt */
    MVP_CKPT_AGGRE_MIG_ROLE,
    MVP_CKPT_AGGREGATION_BUFPOOL_SIZE,
    MVP_CKPT_AGGREGATION_CHUNK_SIZE,
    MVP_CKPT_AGGRE_MIG_FILE,
    MVP_CKPT_FILE,
    MVP_CKPT_INTERVAL,
    MVP_CKPT_MAX_CKPTS,
    MVP_CKPT_MAX_SAVE_CKPTS,
    MVP_CKPT_MPD_BASE_PORT,
    MVP_CKPT_NO_SYNC,
    MVP_CKPT_SESSIONID,
    MVP_CKPT_USE_AGGREGATION,
    /* debug */
    TOTALVIEW,
    /* other */
    MVP_MAX_PARAM_ID
} mvp_env_param_id_t;

typedef enum mvp_env_param_type {
    MVP_PARAM_TYPE_INVALID = 0,
    MVP_PARAM_TYPE_INT8,
    MVP_PARAM_TYPE_INT16,
    MVP_PARAM_TYPE_INT,
    MVP_PARAM_TYPE_LONG,
    MVP_PARAM_TYPE_STRING,
} mvp_env_param_type_t;

/* parameter categories */
typedef enum mvp_env_param_group {
    MVP_PARAM_GROUP_launcher,
    MVP_PARAM_GROUP_QoS,
    MVP_PARAM_GROUP_collective,
    MVP_PARAM_GROUP_ckpt,
    MVP_PARAM_GROUP_startup,
    MVP_PARAM_GROUP_pt2pt,
    MVP_PARAM_GROUP_intranode,
    MVP_PARAM_GROUP_cuda,
    MVP_PARAM_GROUP_debugger,
    MVP_PARAM_GROUP_rma,
    MVP_PARAM_GROUP_rdma_cm,
    MVP_PARAM_GROUP_hybrid,
    MVP_PARAM_GROUP_threads,
    MVP_PARAM_GROUP_other,
    MVP_PARAM_NUM_GROUPS
} mvp_env_param_group_t;

/* runtime environment list structure */
typedef struct mvp_env_param_list {
    mvp_env_param_id_t id;       /* param id */
    mvp_env_param_type_t type;   /* param datatype */
    mvp_env_param_group_t group; /* param category */
    char *name;                  /* param name */
    void *value;                 /* param value store addr */
    int external_visible;        /* 1 or 0 */
    char *descrption;            /* param descrption */
} mvp_env_param_list_t;

extern mvp_env_param_list_t param_list[];
void mvp_show_all_params();
void mvp_show_runlog_info(int level);
void rdma_set_rdma_fast_path_params(int num_proc);
const char *mvp_ibv_mtu_enum_to_string(enum ibv_mtu mtu);
uint16_t mvp_ibv_mtu_enum_to_value(enum ibv_mtu mtu);
extern int rdma_get_rail_sharing_policy(const char *value);

mvp_arch_hca_type MVP_get_arch_hca_type();

extern int dreg_max_use_count;
#endif /* _RDMA_PARAM_H */

/* default values of CVARs */
#define USE_MCAST_DEFAULT_FLAG     1
#define DEFAULT_NUM_PORTS          1
#define DEFAULT_NUM_QP_PER_PORT    1
#define DEFAULT_COALESCE_THRESHOLD 6
#define DEFAULT_USE_COALESCE       1
#define DEFAULT_SPIN_COUNT         5000
#define MAX_NUM_CQES_PER_POLL      96
#define MIN_NUM_CQES_PER_POLL      1
