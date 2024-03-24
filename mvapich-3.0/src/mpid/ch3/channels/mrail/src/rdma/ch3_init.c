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

#include "mpidi_ch3_impl.h"
#include "mpid_mrail_rndv.h"
#include "rdma_impl.h"
#include "mem_hooks.h"
#include "mvp_ch3_shmem.h"
#include "hwloc_bind.h"
#include "cm.h"
#include "mpir_nodemap.h"
#if defined(_MCST_SUPPORT_)
#include "ibv_mcast.h"
#endif
#if defined(_SHARP_SUPPORT_)
#include "ibv_sharp.h"
#endif
#ifdef HAVE_ROMIO
#include "romioconf.h"
#endif

#include <mvp_config.h>
#include <hwloc_bind.h>
#include <error_handling.h>

#ifdef CKPT

#ifdef ENABLE_SCR
#include "scr.h"
#endif

pthread_mutex_t MVAPICH_sync_ckpt_lock;
pthread_cond_t MVAPICH_sync_ckpt_cond;
int MVAPICH_Sync_Checkpoint();
#endif /* CKPT */

#if defined(_ENABLE_CUDA_)
MPIDI_CH3U_CUDA_SRBuf_element_t *MPIDI_CH3U_CUDA_SRBuf_pool = NULL;
MPIDI_CH3U_COLL_SRBuf_element_t *MPIDI_CH3U_COLL_SRBuf_pool = NULL;
#endif

#define MPIDI_CH3I_HOST_DESCRIPTION_KEY "description"

/*
=== BEGIN_MPI_T_MVP_CVAR_INFO_BLOCK ===

cvars:
    - name        : MVP_IOV_DENSITY_MIN
      category    : CH3
      type        : int
      default     : (1024*16)
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_ENABLE_EAGER_THRESHOLD_REDUCTION
      category    : CH3
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_USE_EAGER_FAST_SEND
      category    : CH3
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_SUPPORT_FORK_SAFETY
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_RDMA_MAX_TRANSFER_SIZE
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_RNDV_IMMEDIATE
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_CHECK_CACHE_ALIGNMENT
      category    : CH3
      type        : int
      default     : -1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_HYBRID_ENABLE_THRESHOLD
      category    : CH3
      type        : int
      default     : 512
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This defines the threshold for enabling Hybrid communication using UD
        and RC/XRC. When the size of the job is greater than or equal to the
        threshold value, Hybrid mode will be enabled. Otherwise, it uses default
        RC/XRC connections for communication.

    - name        : MVP_USE_ONLY_UD
      category    : CH3
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Set this to 1, to enable only UD transport in hybrid configuration mode.
        It will not use any RC/XRC connections in this mode.

    - name        : MVP_SHOW_ENV_INFO
      category    : CH3
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Show the values assigned to the run time environment parameters.

    - name        : MVP_DEBUG_CORESIZE
      category    : CH3
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_DEBUG_SHOW_BACKTRACE
      category    : CH3
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_USE_THREAD_WARNING
      category    : CH3
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

=== END_MPI_T_MVP_CVAR_INFO_BLOCK ===
*/

char *MPIDI_CH3_Pkt_type_to_string[MPIDI_CH3_PKT_END_ALL + 1] = {
    [MPIDI_CH3_PKT_EAGER_SEND] = "MPIDI_CH3_PKT_EAGER_SEND",
#if defined(CHANNEL_MRAIL)
    [MPIDI_CH3_PKT_EAGER_SEND_CONTIG] = "MPIDI_CH3_PKT_EAGER_SEND_CONTIG",
#ifndef MVP_DISABLE_HEADER_CACHING
    [MPIDI_CH3_PKT_FAST_EAGER_SEND] = "MPIDI_CH3_PKT_FAST_EAGER_SEND",
    [MPIDI_CH3_PKT_FAST_EAGER_SEND_WITH_REQ] =
        "MPIDI_CH3_PKT_FAST_EAGER_SEND_WITH_REQ",
#endif /* !MVP_DISABLE_HEADER_CACHING */
    [MPIDI_CH3_PKT_RPUT_FINISH] = "MPIDI_CH3_PKT_RPUT_FINISH",
    [MPIDI_CH3_PKT_RGET_FINISH] = "MPIDI_CH3_PKT_RGET_FINISH",
    [MPIDI_CH3_PKT_ZCOPY_FINISH] = "MPIDI_CH3_PKT_ZCOPY_FINISH",
    [MPIDI_CH3_PKT_ZCOPY_ACK] = "MPIDI_CH3_PKT_ZCOPY_ACK",
    [MPIDI_CH3_PKT_MCST] = "MPIDI_CH3_PKT_MCST",
    [MPIDI_CH3_PKT_MCST_NACK] = "MPIDI_CH3_PKT_MCST_NACK",
    [MPIDI_CH3_PKT_MCST_INIT] = "MPIDI_CH3_PKT_MCST_INIT",
    [MPIDI_CH3_PKT_MCST_INIT_ACK] = "MPIDI_CH3_PKT_MCST_INIT_ACK",
    [MPIDI_CH3_PKT_NOOP] = "MPIDI_CH3_PKT_NOOP",
    [MPIDI_CH3_PKT_RMA_RNDV_CLR_TO_SEND] = "MPIDI_CH3_PKT_RMA_RNDV_CLR_TO_SEND",
    [MPIDI_CH3_PKT_CUDA_CTS_CONTI] = "MPIDI_CH3_PKT_CUDA_CTS_CONTI",
    [MPIDI_CH3_PKT_PUT_RNDV] = "MPIDI_CH3_PKT_PUT_RNDV",
    [MPIDI_CH3_PKT_ACCUMULATE_RNDV] = "MPIDI_CH3_PKT_ACCUMULATE_RNDV",
    [MPIDI_CH3_PKT_GET_ACCUM_RNDV] = "MPIDI_CH3_PKT_GET_ACCUM_RNDV",
    [MPIDI_CH3_PKT_GET_RNDV] = "MPIDI_CH3_PKT_GET_RNDV",
    [MPIDI_CH3_PKT_RNDV_READY_REQ_TO_SEND] =
        "MPIDI_CH3_PKT_RNDV_READY_REQ_TO_SEND",
    [MPIDI_CH3_PKT_PACKETIZED_SEND_START] =
        "MPIDI_CH3_PKT_PACKETIZED_SEND_START",
    [MPIDI_CH3_PKT_PACKETIZED_SEND_DATA] = "MPIDI_CH3_PKT_PACKETIZED_SEND_DATA",
    [MPIDI_CH3_PKT_RNDV_R3_DATA] = "MPIDI_CH3_PKT_RNDV_R3_DATA",
    [MPIDI_CH3_PKT_RNDV_R3_ACK] = "MPIDI_CH3_PKT_RNDV_R3_ACK",
    [MPIDI_CH3_PKT_ADDRESS] = "MPIDI_CH3_PKT_ADDRESS",
    [MPIDI_CH3_PKT_ADDRESS_REPLY] = "MPIDI_CH3_PKT_ADDRESS_REPLY",
    [MPIDI_CH3_PKT_CM_ESTABLISH] = "MPIDI_CH3_PKT_CM_ESTABLISH",
#if defined(CKPT)
    [MPIDI_CH3_PKT_CM_SUSPEND] = "MPIDI_CH3_PKT_CM_SUSPEND",
    [MPIDI_CH3_PKT_CM_REACTIVATION_DONE] = "MPIDI_CH3_PKT_CM_REACTIVATION_DONE",
    [MPIDI_CH3_PKT_CR_REMOTE_UPDATE] = "MPIDI_CH3_PKT_CR_REMOTE_UPDATE",
#endif /* defined(CKPT) */
#if defined(_SMP_LIMIC_) || defined(_SMP_CMA_)
    [MPIDI_CH3_PKT_SMP_DMA_COMP] = "MPIDI_CH3_PKT_SMP_DMA_COMP",
#endif
#endif /* defined(CHANNEL_MRAIL) */
#if defined(USE_EAGER_SHORT)
    [MPIDI_CH3_PKT_EAGERSHORT_SEND] = "MPIDI_CH3_PKT_EAGERSHORT_SEND",
#endif /* defined(USE_EAGER_SHORT) */
    [MPIDI_CH3_PKT_EAGER_SYNC_SEND] = "MPIDI_CH3_PKT_EAGER_SYNC_SEND",
    [MPIDI_CH3_PKT_EAGER_SYNC_ACK] = "MPIDI_CH3_PKT_EAGER_SYNC_ACK",
    [MPIDI_CH3_PKT_READY_SEND] = "MPIDI_CH3_PKT_READY_SEND",
    [MPIDI_CH3_PKT_RNDV_REQ_TO_SEND] = "MPIDI_CH3_PKT_RNDV_REQ_TO_SEND",
    [MPIDI_CH3_PKT_RNDV_CLR_TO_SEND] = "MPIDI_CH3_PKT_RNDV_CLR_TO_SEND",
    [MPIDI_CH3_PKT_RNDV_SEND] = "MPIDI_CH3_PKT_RNDV_SEND",
    [MPIDI_CH3_PKT_CANCEL_SEND_REQ] = "MPIDI_CH3_PKT_CANCEL_SEND_REQ",
    [MPIDI_CH3_PKT_CANCEL_SEND_RESP] = "MPIDI_CH3_PKT_CANCEL_SEND_RESP",
    /* RMA Packets begin here */
    [MPIDI_CH3_PKT_PUT] = "MPIDI_CH3_PKT_PUT",
    [MPIDI_CH3_PKT_PUT_IMMED] = "MPIDI_CH3_PKT_PUT_IMMED",
    [MPIDI_CH3_PKT_GET] = "MPIDI_CH3_PKT_GET",
    [MPIDI_CH3_PKT_ACCUMULATE] = "MPIDI_CH3_PKT_ACCUMULATE",
    [MPIDI_CH3_PKT_ACCUMULATE_IMMED] = "MPIDI_CH3_PKT_ACCUMULATE_IMMED",
    [MPIDI_CH3_PKT_GET_ACCUM] = "MPIDI_CH3_PKT_GET_ACCUM",
    [MPIDI_CH3_PKT_GET_ACCUM_IMMED] = "MPIDI_CH3_PKT_GET_ACCUM_IMMED",
    [MPIDI_CH3_PKT_FOP] = "MPIDI_CH3_PKT_FOP",
    [MPIDI_CH3_PKT_FOP_IMMED] = "MPIDI_CH3_PKT_FOP_IMMED",
    [MPIDI_CH3_PKT_CAS_IMMED] = "MPIDI_CH3_PKT_CAS_IMMED",
    [MPIDI_CH3_PKT_GET_RESP] = "MPIDI_CH3_PKT_GET_RESP",
    [MPIDI_CH3_PKT_GET_RESP_IMMED] = "MPIDI_CH3_PKT_GET_RESP_IMMED",
    [MPIDI_CH3_PKT_GET_ACCUM_RESP] = "MPIDI_CH3_PKT_GET_ACCUM_RESP",
    [MPIDI_CH3_PKT_GET_ACCUM_RESP_IMMED] = "MPIDI_CH3_PKT_GET_ACCUM_RESP_IMMED",
    [MPIDI_CH3_PKT_FOP_RESP] = "MPIDI_CH3_PKT_FOP_RESP",
    [MPIDI_CH3_PKT_FOP_RESP_IMMED] = "MPIDI_CH3_PKT_FOP_RESP_IMMED",
    [MPIDI_CH3_PKT_CAS_RESP_IMMED] = "MPIDI_CH3_PKT_CAS_RESP_IMMED",
    [MPIDI_CH3_PKT_LOCK] = "MPIDI_CH3_PKT_LOCK",
    [MPIDI_CH3_PKT_LOCK_ACK] = "MPIDI_CH3_PKT_LOCK_ACK",
    [MPIDI_CH3_PKT_LOCK_OP_ACK] = "MPIDI_CH3_PKT_LOCK_OP_ACK",
    [MPIDI_CH3_PKT_UNLOCK] = "MPIDI_CH3_PKT_UNLOCK",
    [MPIDI_CH3_PKT_FLUSH] = "MPIDI_CH3_PKT_FLUSH",
    [MPIDI_CH3_PKT_ACK] = "MPIDI_CH3_PKT_ACK",
    [MPIDI_CH3_PKT_DECR_AT_COUNTER] = "MPIDI_CH3_PKT_DECR_AT_COUNTER",
    /* RMA Packets end here */
    [MPIDI_CH3_PKT_FLOW_CNTL_UPDATE] = "MPIDI_CH3_PKT_FLOW_CNTL_UPDATE",
    [MPIDI_CH3_PKT_CLOSE] = "MPIDI_CH3_PKT_CLOSE",
    [MPIDI_CH3_PKT_REVOKE] = "MPIDI_CH3_PKT_REVOKE",
    [MPIDI_CH3_PKT_END_CH3] = "MPIDI_CH3_PKT_END_CH3",
/* The channel can define additional types by defining the value
   MPIDI_CH3_PKT_ENUM */
#if defined(MPIDI_CH3_PKT_ENUM)
    MPIDI_CH3_PKT_ENUM_TYPE_TO_STRING
#endif
        [MPIDI_CH3_PKT_END_ALL] = "MPIDI_CH3_PKT_END_ALL"};

MPIDI_CH3I_Process_t MPIDI_CH3I_Process;
int mvp_use_ib_channel = 1;
extern void ib_finalize_rdma_cm(int pg_rank, MPIDI_PG_t *pg);

ibv_ops_t ibv_ops;
void *ibv_dl_handle = NULL;

#if defined(_MCST_SUPPORT_)
mad_ops_t mad_ops;
void *mad_dl_handle = NULL;
#endif /*defined(_MCST_SUPPORT_)*/

#if defined(HAVE_LIBIBUMAD)
umad_ops_t umad_ops;
void *umad_dl_handle = NULL;
#endif /*defined(HAVE_LIBIBUMAD)*/

#if defined(RDMA_CM)
rdma_ops_t rdma_ops;
void *rdma_dl_handle = NULL;
#endif /*defined(RDMA_CM)*/

#if defined(_SHARP_SUPPORT_)
sharp_ops_t sharp_ops;
void *sharp_dl_handle = NULL;
#endif /* defined(_SHARP_SUPPORT_) */

/* TODO: These should go in a proper header */
extern int mvp_my_async_cpu_id;
extern int smpi_identify_core_for_async_thread(MPIDI_PG_t *pg);

static int split_type(MPIR_Comm *comm_ptr, int stype, int key,
                      MPIR_Info *info_ptr, MPIR_Comm **newcomm_ptr)
{
    int id;
    MPIDI_Rank_t nid;
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPID_Get_node_id(comm_ptr, comm_ptr->rank, &id);
    MPIR_ERR_CHECK(mpi_errno);

    nid = (stype == MPI_COMM_TYPE_SHARED) ? id : MPI_UNDEFINED;
    mpi_errno = MPIR_Comm_split_impl(comm_ptr, nid, key, newcomm_ptr);
    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    return mpi_errno;

    /* --BEGIN ERROR HANDLING-- */
fn_fail:
    goto fn_exit;
    /* --END ERROR HANDLING-- */
}

static MPIR_Commops comm_fns = {split_type};

#define MVP_CHECK_ALIGNMENT(_size_, _name_, _cache_)                           \
    do {                                                                       \
        int _align = (_size_) % (_cache_);                                     \
                                                                               \
        if (_align) {                                                          \
            fprintf(stderr,                                                    \
                    "Warning: %s of size %d is not aligned to"                 \
                    " cache line size %d\n",                                   \
                    (_name_), (_size_), (_cache_));                            \
        }                                                                      \
    } while (0)

int mvp_check_cache_alignment()
{
    int i = 0;
    int mpi_errno = MPI_SUCCESS;

    /* Look at packet headers */
    for (i = 0; i < MPIDI_CH3_PKT_END_ALL; ++i) {
        if ((i == MPIDI_CH3_PKT_END_CH3) ||
            (MPIDI_CH3_Pkt_size_index[i] <= SMPI_CACHE_LINE_SIZE)) {
            continue;
        }
        MVP_CHECK_ALIGNMENT(MPIDI_CH3_Pkt_size_index[i],
                            MPIDI_CH3_Pkt_type_to_string[i],
                            SMPI_CACHE_LINE_SIZE);
    }

    /* Look at different structures */
    MVP_CHECK_ALIGNMENT(sizeof(MPIR_Request), "MPIR_Request",
                        SMPI_CACHE_LINE_SIZE);

    return mpi_errno;
}

static inline int MPIDI_CH3_Populate_vc_local_ranks(MPIDI_PG_t *pg,
                                                    int our_pg_rank)
{
    int mpi_errno = MPI_SUCCESS;
    int i;
    int *local_map = MPIR_Process.node_local_map;

    if (pg->size == 1) {
        pg->ch.local_process_id = 0;
        pg->ch.num_local_processes = 1;
        pg->vct[0].smp.local_rank = 0;

        return mpi_errno;
    }

    for (i = 0; i < pg->size; i++) {
        pg->vct[i].smp.local_rank = -1;
    }

    /* assume local_nodemap is correctly filled by channel independent init */
    pg->ch.num_local_processes = MPIR_Process.local_size;
    for (i = 0; i < MPIR_Process.local_size; i++) {
        pg->vct[local_map[i]].smp.local_rank = i;
        if (our_pg_rank == local_map[i]) {
            pg->ch.local_process_id = i;
        }
    }

    return mpi_errno;
}

void init_debug1()
{
    /* Set coresize limit */
    char *value = NULL;
    int backtrace = 0;
    int setup_sighandler = 1;

    set_coresize_limit(MVP_DEBUG_CORESIZE);
    /* ignore error code, failure if not fatal */

    /* Set an error signal handler */
    backtrace = MVP_DEBUG_SHOW_BACKTRACE;

    value = getenv("MVP_DEBUG_SETUP_SIGHDLR");
    if (value != NULL) {
        setup_sighandler = !!atoi(value);
    }

    if (setup_sighandler) {
        setup_error_sighandler(backtrace);
    }

    /* ignore error code, failure if not fatal */

    /* Initialize DEBUG variables */
    initialize_debug_variables();
}

void init_debug2(int mpi_rank)
{
    /* Set prefix for debug output */
    const int MAX_LENGTH = 256;
    char hostname[MAX_LENGTH];
    gethostname(hostname, MAX_LENGTH);
    hostname[MAX_LENGTH - 1] = '\0';
    char output_prefix[MAX_LENGTH];
    snprintf(output_prefix, MAX_LENGTH, "%s:mpi_rank_%i", hostname, mpi_rank);
    set_output_prefix(output_prefix);
}

static int MPIDI_CH3I_Thread_check(MPIDI_PG_t *pg, int *provided)
{
    char *value;
    int mpi_errno = MPI_SUCCESS;
    int pg_rank = MPIDI_Process.my_pg_rank;

    /* affinity/thread checks */
    if (provided != NULL) {
        /* If user has enabled blocking mode progress,
         * then we cannot support MPI_THREAD_MULTIPLE */
        int thread_warning = 1;
        int aligned_alloc = 0;
        thread_warning = MVP_USE_THREAD_WARNING;
        if ((value = getenv("MVP_USE_ALIGNED_ALLOC")) != NULL) {
            aligned_alloc = !!atoi(value);
        }
        if (!aligned_alloc && (0 == pg_rank) &&
            (*provided > MPI_THREAD_SINGLE)) {
            PRINT_INFO(thread_warning,
                       "[Performance Suggestion]: Application has requested"
                       " for multi-thread capability. If allocating memory"
                       " from different pthreads/OpenMP threads, please"
                       " consider setting MVP_USE_ALIGNED_ALLOC=1 for improved"
                       " performance.\n"
                       "Use MVP_USE_THREAD_WARNING=0 to suppress this error "
                       "message.\n");
        }
        if (rdma_use_blocking) {
            if (0 == pg_rank && MPI_THREAD_MULTIPLE == *provided) {
                PRINT_INFO(thread_warning,
                           "WARNING: Requested MPI_THREAD_MULTIPLE, \n"
                           "  but MVP_USE_BLOCKING=1 only supports "
                           "MPI_THREAD_SERIALIZED.\n"
                           "  Use MVP_USE_THREAD_WARNING=0 to suppress this "
                           "error message.\n");
            }
            *provided = (MPICH_THREAD_LEVEL < *provided) ?
                            MPICH_THREAD_LEVEL :
                            MPI_THREAD_SERIALIZED;
        }
        /* Check if user has allocated spare cores for progress thread */
        if ((value = getenv("MVP_ENABLE_PROGRESS_AFFINITY")) != NULL) {
            mvp_enable_progress_affinity = !!atoi(value);
        }
        if (SMP_INIT && mvp_enable_progress_affinity) {
            mpi_errno = smpi_identify_core_for_async_thread(pg);
            MPIR_ERR_CHECK(mpi_errno);
        }
        /*
         * Check to see if the user has explicitly disabled affinity.  If not
         * then affinity will be enabled barring any errors.
         * Need to disable thread_multiple only if binding level is core.
         */
        if (mvp_enable_affinity && (mvp_my_async_cpu_id == -1) &&
            (mvp_binding_level == LEVEL_CORE)) {
            /*
             * Affinity will be enabled, MPI_THREAD_SINGLE will be the provided
             * MPICH_THREAD_LEVEL in this case.
             */
            *provided = MPI_THREAD_SINGLE;
        }
        if ((value = getenv("OMP_NUM_THREADS")) != NULL) {
            int _temp = atoi(value);
            if ((_temp > 1) && mvp_enable_affinity && (0 == pg_rank) &&
                thread_warning && (mvp_binding_level == LEVEL_CORE)) {
                fprintf(
                    stderr,
                    "Warning: Process to core binding is enabled and"
                    " OMP_NUM_THREADS is greater than one (%d).\nIf"
                    " your program has OpenMP sections, this can cause"
                    " over-subscription of cores and consequently poor"
                    " performance\nTo avoid this, please re-run your"
                    " application after setting MVP_ENABLE_AFFINITY=0\n"
                    "Use MVP_USE_THREAD_WARNING=0 to suppress this message\n",
                    _temp);
            }
        }
    }

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIDI_CH3_Init(int has_parent, MPIDI_PG_t *pg, int pg_rank)
{
    int mpi_errno = MPI_SUCCESS;
    int pg_size, threshold, dpm = 0, p;
    char *conn_info = NULL;
    int mvp_rdma_init_timers = 0;
    int user_selected_rdma_cm = 0;
    MPIDI_VC_t *vc;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_CH3_INIT);

    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_CH3_INIT);

    /* init ch3 specific struct info */
    mpi_errno = MPIDI_CH3_Populate_vc_local_ranks(pg, pg_rank);
    MPIR_ERR_CHECK(mpi_errno);

    mpi_errno = MPIDI_CH3I_Thread_check(pg, &MPIR_ThreadInfo.thread_provided);
    MPIR_ERR_CHECK(mpi_errno);

    /* FIXME: This is a good place to check for environment variables
     * and command line options that may control the device.
     */
    if (mpi_errno = read_configuration_files(&MPIDI_Process.mvp_config_crc)) {
        MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**fail",
                                  "**fail %s",
                                  "Error processing configuration file");
    }

    init_debug1();

    init_debug2(pg_rank);

    if (has_parent) {
        pg->is_spawned = has_parent;
        /* TODO: CVARs - this will no longer be relevant and we need to change
         * the backing value since CVARs are already read in */
        putenv("MVP_SUPPORT_DPM=1");
    }

    /* Setting CPU affinity before opening PSM contexts and opening IB HCAs. For
     * IB HCAs, the change is to ensure HCA aware process mapping */
    mpi_errno = MPIDI_CH3I_set_affinity(pg, pg_rank);
    MPIR_ERR_CHECK(mpi_errno);

    /* Override split_type */
    MPIR_Comm_fns = &comm_fns;

    /* Explicitly initializing RDMA_FP to 0 */
    mvp_MPIDI_CH3I_RDMA_Process.has_adaptive_fast_path = 0;

    if (MPIDI_CH3_Pkt_size_index[MPIDI_CH3_PKT_CLOSE] !=
        sizeof(MPIDI_CH3_Pkt_close_t)) {
        MPIR_ERR_SETFATALANDJUMP1(
            mpi_errno, MPI_ERR_OTHER, "**fail", "**fail %s",
            "Failed sanity check! Packet size table mismatch");
    }

    pg_size = MPIDI_PG_Get_size(pg);

    /* Allocate PMI Key Value Pair */
    mvp_allocate_pmi_keyval();

    mpi_errno =
        MPIDI_CH3U_Comm_register_create_hook(MPIDI_CH3I_comm_create, NULL);
    MPIR_ERR_CHECK(mpi_errno);

    /* Choose default startup method and set default on-demand threshold */
#if defined(RDMA_CM) && !defined(CKPT) && !(ROMIO_IME)
    /* If user has not forcefully disabled RDMA_CM, and if user has not
     * specified the use of MCAST use it by default */
    if (((MVP_USE_RDMA_CM == -1) || !!MVP_USE_RDMA_CM) &&
        ((MVP_USE_ROCE == -1) || !MVP_USE_ROCE)
#if defined(_MCST_SUPPORT_)
        && ((MVP_USE_MCAST == -1) || !MVP_USE_MCAST) && !rdma_enable_mcast
#endif /*defined(_MCST_SUPPORT_)*/
    ) {
        MPIDI_CH3I_Process.cm_type = MPIDI_CH3I_CM_RDMA_CM;
        threshold = MPIDI_CH3I_RDMA_CM_DEFAULT_ON_DEMAND_THRESHOLD;
    } else
#endif /*defined(RDMA_CM) && !defined(CKPT)*/
    {
        MPIDI_CH3I_Process.cm_type = MPIDI_CH3I_CM_BASIC_ALL2ALL;
        threshold = MPIDI_CH3I_CM_DEFAULT_ON_DEMAND_THRESHOLD;
    }

    /*check ON_DEMAND_THRESHOLD */
    if (MVP_ON_DEMAND_THRESHOLD != -1) {
        threshold = MVP_ON_DEMAND_THRESHOLD;
    }

    if (MVP_CVAR_IS_SET_BY_USER(MVP_SUPPORT_DPM)) {
        dpm = MVP_SUPPORT_DPM;
        MPIDI_CH3I_Process.has_dpm = dpm;
    }
    if (MPIDI_CH3I_Process.has_dpm) {
#if defined(RDMA_CM) && !defined(CKPT)
        /* DPM is not supported with RDMA_CM. Fall back to basic alltoall CM */
        MPIDI_CH3I_Process.cm_type = MPIDI_CH3I_CM_BASIC_ALL2ALL;
        /* Reset value of threshold if user has not set it already */
        if (MVP_ON_DEMAND_THRESHOLD == -1) {
            threshold = MPIDI_CH3I_CM_DEFAULT_ON_DEMAND_THRESHOLD;
        }
#endif /*defined(RDMA_CM) && !defined(CKPT)*/
#ifdef _ENABLE_UD_
        /* DPM and Hybrid cannot be enabled at the same time */
        MVP_USE_UD_HYBRID = 0;
#endif /*_ENABLE_UD_*/
    }

#ifdef _ENABLE_CUDA_
    if (MVP_USE_CUDA != -1) {
        rdma_enable_cuda = !!MVP_USE_CUDA;
        if (rdma_enable_cuda) {
            cuda_get_user_parameters();
        }
#ifdef ENABLE_LLNL_SITE_SPECIFIC_OPTIONS
    } else {
        rdma_enable_cuda = 1;
        if (!((MVP_SUPPRESS_CUDA_USAGE_WARNING != -1) &&
              !!MVP_SUPPRESS_CUDA_USAGE_WARNING)) {
            PRINT_INFO((pg_rank == 0),
                       " Automatically enabling CUDA support."
                       " If not using GPU buffers, disabling CUDA support by"
                       " setting MVP_USE_CUDA=0 may improve performance.\n"
                       "To suppress this message, please set"
                       " MVP_SUPPRESS_CUDA_USAGE_WARNING to 1\n");
        }
#endif /*ENABLE_LLNL_SITE_SPECIFIC_OPTIONS*/
    }
    if (!rdma_enable_cuda) {
        if (!((MVP_SUPPRESS_CUDA_USAGE_WARNING != -1) &&
              !!MVP_SUPPRESS_CUDA_USAGE_WARNING)) {
            PRINT_INFO(
                (pg_rank == 0),
                "MVAPICH has been built with support for CUDA."
                " But, MVP_USE_CUDA not set to 1. This can lead to errors in"
                " using GPU buffers. If you are running applications that use"
                " GPU buffers, please set MVP_USE_CUDA=1 and try again.\n");
            PRINT_INFO((pg_rank == 0),
                       "To suppress this warning, please set"
                       " MVP_SUPPRESS_CUDA_USAGE_WARNING to 1\n");
        }
    }
#endif

#ifdef _ENABLE_CUDA_
    /* set general device support, this can be extended for supporting other
     * third-party devices with similar runtime designs */
    mvp_enable_device = rdma_enable_cuda;
#endif

#ifdef _ENABLE_UD_
    int i = 0;
    for (i = 0; i < MAX_NUM_HCAS; ++i) {
        mvp_MPIDI_CH3I_RDMA_Process.ud_rails[i] = NULL;
    }
    if (MVP_HYBRID_ENABLE_THRESHOLD != -1) {
        rdma_hybrid_enable_threshold = MVP_HYBRID_ENABLE_THRESHOLD;
    }
    if (MVP_CVAR_IS_SET_BY_USER(MVP_USE_ONLY_UD) && MVP_USE_ONLY_UD) {
        MVP_USE_UD_HYBRID = MVP_USE_ONLY_UD;
        if (MVP_HYBRID_ENABLE_THRESHOLD != -1) {
            if (MVP_HYBRID_ENABLE_THRESHOLD > 0) {
                PRINT_INFO((pg_rank == 0),
                           "User requested only UD. Resetting "
                           "MVP_HYBRID_ENABLE_THRESHOLD to 0.\n");
            }
        }
        rdma_hybrid_enable_threshold = 0;
    }

    if (MVP_CVAR_IS_SET_BY_USER(MVP_SUPPORT_DPM) && MVP_SUPPORT_DPM) {
        MVP_USE_UD_HYBRID = 0;
    }

    if (pg_size < rdma_hybrid_enable_threshold) {
        MVP_USE_UD_HYBRID = 0;
    }
    if (MVP_USE_UD_HYBRID && MPIDI_CH3I_Process.has_dpm) {
        PRINT_INFO(
            (pg_rank == 0),
            "DPM is not supported with Hybrid builds. Disabling Hybrid\n");
        MVP_USE_ONLY_UD = 0;
        MVP_USE_UD_HYBRID = 0;
    }
    if (MVP_USE_UD_HYBRID) {
        /* The zero-copy bcast design is disabled when
         * hybrid is used */
        MVP_USE_ZCOPY_BCAST = 0;
        MVP_USE_ZCOPY_REDUCE = 0;
        rdma_use_coalesce = 0;
        /* TODO: Automatically use SRQ for UD once it is working right */
        rdma_use_ud_srq = 0;
        mvp_rdma_init_timers = 1;
#if defined(RDMA_CM) && !defined(CKPT)
        /* UD/Hybrid is not supported with RDMA_CM. Fall back to basic alltoall
         * CM */
        MPIDI_CH3I_Process.cm_type = MPIDI_CH3I_CM_BASIC_ALL2ALL;
        /* Reset value of threshold if user has not set it already */
        if (MVP_ON_DEMAND_THRESHOLD == -1) {
            threshold = MPIDI_CH3I_CM_DEFAULT_ON_DEMAND_THRESHOLD;
        }
#endif /*defined(RDMA_CM) && !defined(CKPT)*/
        if (MPIDI_CH3I_Process.has_dpm) {
            MPL_error_printf(
                "Error: DPM is not supported with UD-Hybrid option.\n"
                "Please retry after setting "
                "MVP_HYBRID_ENABLE_THRESHOLD=<nprocs+1>.\n");
            MPIR_ERR_SETFATALANDJUMP(mpi_errno, MPI_ERR_OTHER, "**fail");
        }
    }
#endif /* #ifdef _ENABLE_UD_ */

    if (MVP_USE_XRC != -1) {
#ifdef _ENABLE_XRC_
        USE_XRC = !!MVP_USE_XRC;
        if (MVP_USE_XRC) {
#ifdef _ENABLE_UD_
            if (MVP_USE_ONLY_UD) {
                PRINT_INFO((pg_rank == 0),
                           "XRC and only UD cannot be set at the same time.\n");
                PRINT_INFO((pg_rank == 0), "Proceeding after disabling XRC.\n");
                USE_XRC = 0;
            } else
#endif /*_ENABLE_UD_*/
            {
#if defined(RDMA_CM) && !defined(CKPT)
                /* XRC is not supported with RDMA_CM. Fall back to basic
                 * alltoall CM.
                 * This will get reset to on-demand CM later on in this
                 * function. */
                MPIDI_CH3I_Process.cm_type = MPIDI_CH3I_CM_BASIC_ALL2ALL;
#endif /*defined(RDMA_CM) && !defined(CKPT)*/
                /* Enable on-demand */
                threshold = 0;
            }
            /* RGET is not supporpted with XRC. Use RPUT by default */
            MVP_RNDV_PROTOCOL = MVP_RNDV_PROTOCOL_RPUT;
        }
#else
        if (MVP_USE_XRC) {
            PRINT_INFO((pg_rank == 0),
                       "XRC support is not configured. Please retry with"
                       "MVP_USE_XRC=0 (or) Reconfigure MVAPICH library without "
                       "--disable-xrc.\n");
            PRINT_INFO((pg_rank == 0), "Proceeding after disabling XRC.\n");
        }
#endif
    }

#if defined(RDMA_CM)
    if (((MVP_USE_RDMA_CM != -1 && MVP_USE_RDMA_CM) ||
         (MVP_USE_IWARP_MODE != -1 && MVP_USE_IWARP_MODE)) &&
        !dpm) {
#if defined(ROMIO_IME)
        PRINT_INFO((pg_rank == 0), "Error: IME FS does not work with RDMA CM. "
                                   "Proceeding without RDMA support.\n");
#else
        MPIDI_CH3I_Process.cm_type = MPIDI_CH3I_CM_RDMA_CM;
#endif
#ifdef _ENABLE_XRC_
        USE_XRC = 0;
        if (MVP_USE_XRC != -1 && (pg_rank == 0)) {
            if (MVP_USE_XRC) {
                MPL_error_printf("Error: XRC does not work with RDMA CM. "
                                 "Proceeding without XRC support.\n");
            }
        }
#endif
    }
#endif /* defined(RDMA_CM) */

    if (MPIDI_CH3I_Process.cm_type != MPIDI_CH3I_CM_RDMA_CM) {
        if (pg_size > threshold || dpm
#ifdef _ENABLE_XRC_
            || USE_XRC
#endif /* _ENABLE_XRC_ */
#ifdef _ENABLE_UD_
            || MVP_USE_UD_HYBRID
#endif
        ) {
            MPIDI_CH3I_Process.cm_type = MPIDI_CH3I_CM_ON_DEMAND;
            MPIDI_CH3I_Process.num_conn = 0;
        } else {
            MPIDI_CH3I_Process.cm_type = MPIDI_CH3I_CM_BASIC_ALL2ALL;
        }
    }

    MPIDI_PG_GetConnKVSname(&pg->ch.kvs_name);

#if defined(CKPT)
#if defined(RDMA_CM)
    if (MPIDI_CH3I_Process.cm_type == MPIDI_CH3I_CM_RDMA_CM) {
        MPL_error_printf(
            "Error: Checkpointing does not work with RDMA CM.\n"
            "Please configure and compile MVAPICH with checkpointing disabled "
            "or without support for RDMA CM.\n");
        MPIR_ERR_SETFATALANDJUMP(mpi_errno, MPI_ERR_OTHER, "**fail");
    }
#endif /* defined(RDMA_CM) */

    // Always use CM_ON_DEMAND for Checkpoint/Restart and Migration
    MPIDI_CH3I_Process.cm_type = MPIDI_CH3I_CM_ON_DEMAND;

#endif /* defined(CKPT) */
#ifdef _ENABLE_UD_
    if (MVP_USE_UD_HYBRID) {
        MPIR_Assert(MPIDI_CH3I_Process.cm_type == MPIDI_CH3I_CM_ON_DEMAND);
    }
#endif

#if defined(RDMA_CM) && !defined(CKPT)
    if (MPIDI_CH3I_Process.cm_type == MPIDI_CH3I_CM_RDMA_CM) {
        if (MVP_USE_RDMA_CM != -1 && !!MVP_USE_RDMA_CM) {
            user_selected_rdma_cm = 1;
        }
        MVP_USE_RDMA_CM = 1;
        if (mvp_MPIDI_CH3I_RDMA_Process.use_iwarp_mode ||
            ((MVP_USE_IWARP_MODE != -1) && !!MVP_USE_IWARP_MODE)) {
            mvp_use_ib_channel = 0;
        } else {
            mvp_use_ib_channel = 1;
        }
    } else
#endif /* defined(RDMA_CM) && !defined(CKPT) */
    {
        mvp_use_ib_channel = 1;
    }

    /* save my vc_ptr for easy access */
    MPIDI_PG_Get_vc(pg, pg_rank, &MPIDI_CH3I_Process.vc);

    /* Initialize Progress Engine */
    if ((mpi_errno = MPIDI_CH3I_Progress_init())) {
        MPIR_ERR_POP(mpi_errno);
    }

    /* Get parameters from the job-launcher */
    rdma_get_pm_parameters(&mvp_MPIDI_CH3I_RDMA_Process);
    /* Check for SMP only */
    MPIDI_CH3I_set_smp_only();

    if (MPIDI_CH3I_Process.has_dpm) {
        MVP_USE_EAGER_FAST_SEND = 0;
    }

    if (MVP_POLLING_LEVEL < MVP_POLLING_LEVEL_MAX) {
        rdma_polling_level = MVP_POLLING_LEVEL;
    } else {
        MVP_POLLING_LEVEL = MVP_POLLING_LEVEL_MAX - 1;
        rdma_polling_level = MVP_POLLING_LEVEL_MAX - 1;
        PRINT_INFO(!pg_rank,
                   "WARNING: Requested polling level is higher than "
                   "max of %d. Falling back to max polling value.",
                   MVP_POLLING_LEVEL_MAX - 1);
    }

    /* Use abstractions to remove dependency on OFED */
    mpi_errno = mvp_dlopen_init();
    /* TODO: make this have better error handling in dl_open */
    if (mpi_errno) {
        mpi_errno = MPI_ERR_OTHER;
        PRINT_INFO(
            (pg_rank == 0),
            "Failed to locate underlying libraries using dlopen."
            " Please consider setting one of the suggested environment"
            " variables to the path of the missing library."
            " Or please reconfigure after setting --disable-ibv-dlopen\n");
        if (!SMP_ONLY) {
            /* If this is a multi-node execution, fail */
            MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**fail",
                                      "**fail %s", "mvp_dlopen_init");
        }
    }
    if (!SMP_ONLY) {
        /* ibv_fork_init() initializes libibverbs's data structures to handle
         * fork() function calls correctly and avoid data corruption, whether
         * fork() is called explicitly or implicitly (such as in system()).
         * If the user requested support for fork safety, call ibv_fork_init */
        if ((MVP_SUPPORT_FORK_SAFETY != -1) && !!MVP_SUPPORT_FORK_SAFETY) {
            mpi_errno = ibv_ops.fork_init();
            if (mpi_errno) {
                MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**fail",
                                          "**fail %s", "ibv_fork_init");
            }
        }

        rdma_local_id = MPIDI_MVP_Get_local_process_id(pg);
        rdma_num_local_procs = MPIDI_MVP_Num_local_processes(pg);

        /* Reading the values from user first and then allocating the memory */
        mpi_errno = rdma_get_control_parameters(&mvp_MPIDI_CH3I_RDMA_Process);
        if (mpi_errno) {
            MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**fail",
                                      "**fail %s",
                                      "rdma_get_control_parameters");
        }
        /* Set default values for parameters */
        rdma_set_default_parameters(&mvp_MPIDI_CH3I_RDMA_Process);
        /* Read user defined values for parameters */
        rdma_get_user_parameters(pg_size, pg_rank);

        /* Allocate structures to store CM information
         * This MUST come after reading env vars */
        mpi_errno = MPIDI_CH3I_MRAIL_CM_Alloc(pg);
        MPIR_ERR_CHECK(mpi_errno);

#if !defined(DISABLE_PTMALLOC)
        if (mvapich_minit()) {
            if (pg_rank == 0) {
                MPL_error_printf(
                    "WARNING: Error in initializing MVAPICH ptmalloc library."
                    "Continuing without InfiniBand registration cache "
                    "support.\n");
            }
            mvp_MPIDI_CH3I_RDMA_Process.has_lazy_mem_unregister = 0;
        }
#else  /* !defined(DISABLE_PTMALLOC) */
        mallopt(M_TRIM_THRESHOLD, -1);
        mallopt(M_MMAP_MAX, 0);
        mvp_MPIDI_CH3I_RDMA_Process.has_lazy_mem_unregister = 0;
#endif /* !defined(DISABLE_PTMALLOC) */

        if (MVP_RDMA_MAX_TRANSFER_SIZE != -1) {
            mvp_MPIDI_CH3I_RDMA_Process.maxtransfersize =
                MVP_RDMA_MAX_TRANSFER_SIZE;
        } else {
            mvp_MPIDI_CH3I_RDMA_Process.maxtransfersize = RDMA_MAX_RDMA_SIZE;
        }

        /* Read RDMA FAST Path related params */
        rdma_set_rdma_fast_path_params(pg_size);
        switch (MPIDI_CH3I_Process.cm_type) {
#if defined(RDMA_CM)
            case MPIDI_CH3I_CM_RDMA_CM:
                mpi_errno = MPIDI_CH3I_RDMA_CM_Init(pg, pg_rank, &conn_info);
                if (mpi_errno != MPI_SUCCESS) {
                    if (user_selected_rdma_cm) {
                        /* Print backtrace and exit */
                        MPIR_ERR_POP(mpi_errno);
                    } else if (!pg_rank) {
                        MPL_error_printf(
                            "Warning: RDMA CM Initialization failed. "
                            "Continuing without RDMA CM support. "
                            "Please set MVP_USE_RDMA_CM=0 to disable RDMA "
                            "CM.\n");
                    }
                    /* Fall back to On-Demand CM */
                    ib_finalize_rdma_cm(pg_rank, pg);
                    rdma_default_port = RDMA_DEFAULT_PORT;
                    mvp_MPIDI_CH3I_RDMA_Process.use_rdma_cm = 0;
                    mvp_MPIDI_CH3I_RDMA_Process.use_rdma_cm_on_demand = 0;
                } else {
                    break;
                }
#endif /* defined(RDMA_CM) */
            case MPIDI_CH3I_CM_ON_DEMAND:
                MPIDI_CH3I_Process.cm_type = MPIDI_CH3I_CM_ON_DEMAND;
                mpi_errno = MPIDI_CH3I_CM_Init(pg, pg_rank, &conn_info);
                MPIR_ERR_CHECK(mpi_errno);
                break;
            default:
                /*call old init to setup all connections */
                MPIDI_CH3I_Process.cm_type = MPIDI_CH3I_CM_BASIC_ALL2ALL;
                /* old init function */
                if ((mpi_errno = MPIDI_CH3I_RDMA_init(pg, pg_rank)) !=
                    MPI_SUCCESS) {
                    MPIR_ERR_POP(mpi_errno);
                }

                /* All vc should be connected */
                for (p = 0; p < pg_size; ++p) {
                    MPIDI_PG_Get_vc(pg, p, &vc);
                    vc->ch.state = MPIDI_CH3I_VC_STATE_IDLE;
                }
                break;
        }
#if defined(RDMA_CM)
    } else {
        /* If SMP_ONLY, we need to get the HCA type */
        rdma_cm_get_hca_type(&mvp_MPIDI_CH3I_RDMA_Process);
#endif /*defined(RDMA_CM)*/
    }

    switch (MVP_RNDV_PROTOCOL) {
        default:
            MVP_RNDV_PROTOCOL = MVP_RNDV_PROTOCOL_R3;
        case MVP_RNDV_PROTOCOL_RPUT:
        case MVP_RNDV_PROTOCOL_RGET:;
    }

    switch (MVP_SMP_RNDV_PROTOCOL) {
        default:
            MPL_usage_printf("MVP_SMP_RNDV_PROTOCOL "
                             "must be one of: RPUT, RGET, R3\n");
            MVP_SMP_RNDV_PROTOCOL = MVP_SMP_RNDV_PROTOCOL;
        case MVP_SMP_RNDV_PROTOCOL_RPUT:
        case MVP_SMP_RNDV_PROTOCOL_RGET:
        case MVP_SMP_RNDV_PROTOCOL_R3:;
    }

    if (MVP_RNDV_IMMEDIATE != -1) {
        rdma_rndv_immediate = !!MVP_RNDV_IMMEDIATE;
    }
#if defined(CKPT)
#if defined(DISABLE_PTMALLOC)
    MPL_error_printf("Error: Checkpointing does not work without registration "
                     "caching enabled.\nPlease configure and compile MVAPICH "
                     "without checkpointing "
                     " or enable registration caching.\n");
    MPIR_ERR_SETFATALANDJUMP(mpi_errno, MPI_ERR_OTHER, "**fail");
#endif /* defined(DISABLE_PTMALLOC) */

    if ((mpi_errno = MPIDI_CH3I_CR_Init(pg, pg_rank, pg_size))) {
        MPIR_ERR_POP(mpi_errno);
    }
#endif /* defined(CKPT) */

    if (conn_info) {
        /* set connection info for dynamic process management */
        if (dpm) {
            mpi_errno = MPIDI_PG_SetConnInfo(pg_rank, (const char *)conn_info);
            MPIR_ERR_CHECK(mpi_errno);
        }
        MPL_free(conn_info);
    }

    struct coll_info colls_arch_hca[colls_max];

    mpi_errno = MVP_collectives_arch_init(
        mvp_MPIDI_CH3I_RDMA_Process.heterogeneity, colls_arch_hca);
    MPIR_ERR_CHECK(mpi_errno);

    /* Initialize the smp channel */
    if ((mpi_errno = MPIDI_CH3I_SMP_init(pg))) {
        MPIR_ERR_POP(mpi_errno);
    }

    if (mvp_enable_shmem_collectives) {
        if ((mpi_errno = MPIR_MVP_SMP_COLL_init())) {
            MPIR_ERR_POP(mpi_errno);
        }
    }

    if (SMP_INIT) {
        for (p = 0; p < pg_size; ++p) {
            MPIDI_PG_Get_vc(pg, p, &vc);
            /* Mark the SMP VC as Idle */
            if (vc->smp.local_nodes >= 0) {
                vc->ch.state = MPIDI_CH3I_VC_STATE_IDLE;
                /* Enable fast send */
                if (MVP_USE_EAGER_FAST_SEND) {
                    vc->use_eager_fast_fn = 1;
                }
                if (SMP_ONLY) {
                    MPIDI_CH3I_SMP_Init_VC(vc);
                }
#ifdef _ENABLE_XRC_
                VC_XST_SET(vc, XF_SMP_VC);
#endif
            }
        }
    }

    /* Allocate and Init Dummy request */
    mpi_errno = mvp_create_dummy_request();

    /* Set the eager max msg size now that we know SMP and RDMA are initialized.
     * The max message size is also set during VC initialization, but the state
     * of SMP is unknown at that time.
     */
    for (p = 0; p < pg_size; ++p) {
        MPIDI_PG_Get_vc(pg, p, &vc);
        vc->eager_max_msg_sz = MPIDI_CH3_EAGER_MAX_MSG_SIZE(vc);
        if (MVP_USE_EAGER_FAST_SEND) {
            vc->eager_fast_max_msg_sz = MPIDI_CH3_EAGER_FAST_MAX_MSG_SIZE(vc);
#ifdef _ENABLE_UD_
            if (MVP_USE_UD_HYBRID) {
                vc->use_eager_fast_fn = 1;
            }
#endif /*ifdef _ENABLE_UD_*/
        } else {
            vc->eager_fast_max_msg_sz = 0;
        }
    }

    if (MVP_SHOW_ENV_INFO != -1) {
        mvp_show_env_info = MVP_SHOW_ENV_INFO;
    }
    if (pg_rank == 0 && mvp_show_env_info) {
        mvp_print_env_info(&mvp_MPIDI_CH3I_RDMA_Process, colls_arch_hca);
    }

#if defined(_MCST_SUPPORT_)
    if (!SMP_ONLY && rdma_enable_mcast) {
        mvp_rdma_init_timers = 1;
        /* TODO : Is there a better way to seed? */
        srand(time(NULL) * pg_rank);

        /* initialize comm table */
        for (p = 0; p < MVP_MCAST_MAX_COMMS; p++) {
            comm_table[p] = NULL;
        }
        /* init mcast context */
        if (mcast_ctx == NULL) {
            mcast_ctx = MPL_malloc(sizeof(mcast_context_t), MPL_MEM_OTHER);
            if (mcast_ctx == NULL) {
                MPIR_ERR_SETFATALANDSTMT1(
                    mpi_errno, MPI_ERR_NO_MEM, goto fn_fail, "**fail",
                    "**fail %s", "Failed to allocate resources for multicast");
            }
            mcast_ctx->selected_rail = 0;
            PRINT_DEBUG(DEBUG_MCST_verbose > 1,
                        "mcast using default rail:"
                        " %d\n",
                        mcast_ctx->selected_rail);
        }
        mcast_ctx->init_list = NULL;
#if defined(RDMA_CM)
        if (rdma_use_rdma_cm_mcast == 1) {
            int ret = 0;
            mcast_ctx->src_addr = (struct sockaddr *)&(mcast_ctx->src_in);
            PRINT_DEBUG(
                DEBUG_MCST_verbose > 1,
                "RDMA CM mcast source ip"
                " address:%s\n",
                ip_address_enabled_devices[mcast_ctx->ip_index].ip_address);

            ret = mvp_rdma_cm_mcst_get_addr_info(
                ip_address_enabled_devices[mcast_ctx->ip_index].ip_address,
                (struct sockaddr *)&mcast_ctx->src_in);
            if (ret) {
                if (MPIDI_Process.my_pg_rank == 0) {
                    PRINT_ERROR(
                        "[Warning]: get src addr failed: not using rdma cm"
                        " based mcast\n");
                }
                rdma_use_rdma_cm_mcast = 0;
            }
        }
#endif /* #if defined(RDMA_CM) */
        mcast_ctx->ud_ctx = mvp_mcast_prepare_ud_ctx();
        if (mcast_ctx->ud_ctx == NULL) {
            MPIR_ERR_SETFATALANDSTMT1(
                mpi_errno, MPI_ERR_OTHER, goto fn_fail, "**fail", "**fail %s",
                "Error in create multicast UD context for multicast");
        }
        PRINT_DEBUG(DEBUG_MCST_verbose, "Created multicast UD context \n");
    }
#endif

    if (mvp_rdma_init_timers) {
        mvp_init_timers();
    }

    mpi_errno =
        MPIDI_CH3U_Comm_register_destroy_hook(MPIDI_CH3I_comm_destroy, NULL);
    MPIR_ERR_CHECK(mpi_errno);

    if (MPIDI_CH3I_Process.cm_type == MPIDI_CH3I_CM_ON_DEMAND) {
        if (g_atomics_support ||
            ((rdma_use_blocking) && (pg_size > threshold))) {
            MPIDI_PG_Get_vc(pg, pg_rank, &vc);
            MPIDI_CH3I_CM_Connect_self(vc);
        }
    }
    if (pg_rank == 0 && MVP_CHECK_CACHE_ALIGNMENT != -1 &&
        !!MVP_CHECK_CACHE_ALIGNMENT) {
        mpi_errno = mvp_check_cache_alignment();
        if (mpi_errno)
            MPIR_ERR_POP(mpi_errno);
    }

#if defined(CKPT)
    MPIDI_Process.use_sync_ckpt = 1;
    /*Initialize conditional variable*/
    pthread_mutex_init(&MVAPICH_sync_ckpt_lock, NULL);
    pthread_cond_init(&MVAPICH_sync_ckpt_cond, NULL);
#endif /* defined(CKPT) */

#if defined(_ENABLE_CUDA_)
    if (mvp_enable_device) {
        if (mvp_device_dynamic_init) {
            device_preinit(pg);
        } else {
            device_init(pg);
        }
        if (pg_rank == 0 && mvp_show_env_info >= 2) {
            mvp_show_cuda_params();
            fprintf(stderr, "--------------------------------------------------"
                            "-------------------\n");
        }
    }
#endif /* defined(_ENABLE_CUDA_) */

#if defined(CKPT) && defined(ENABLE_SCR)
    /* Initialize the Scalable Checkpoint/Restart library */
    SCR_Init();
#endif

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_CH3_INIT);
    return mpi_errno;

fn_fail:
    goto fn_exit;
}

int MPIDI_CH3_VC_Init(MPIDI_VC_t *vc)
{
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_CH3_VC_INIT);

    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_CH3_VC_INIT);

    vc->smp.local_nodes = -1;
#if !defined(CHANNEL_PSM)
    vc->smp.sendq_head = NULL;
    vc->smp.sendq_tail = NULL;
    vc->smp.recv_active = NULL;
    vc->smp.send_active = NULL;
    vc->ch.req = NULL;
    vc->mrail.rails = NULL;
    vc->mrail.srp.credits = NULL;
    vc->mrail.cmanager.msg_channels = NULL;
#endif /* #if !defined (CHANNEL_PSM) */
    vc->ch.sendq_head = NULL;
    vc->ch.sendq_tail = NULL;
    vc->ch.req =
        (MPIR_Request *)MPL_malloc(sizeof(MPIR_Request), MPL_MEM_OBJECT);
    if (!vc->ch.req) {
        MPIR_CHKMEM_SETERR(mpi_errno, sizeof(MPIR_Request), "MPIR Request");
    }
    /* vc->ch.state = MPIDI_CH3I_VC_STATE_IDLE; */
    vc->ch.state = MPIDI_CH3I_VC_STATE_UNCONNECTED;
    vc->ch.read_state = MPIDI_CH3I_READ_STATE_IDLE;
    vc->ch.recv_active = NULL;
    vc->ch.send_active = NULL;
    vc->ch.cm_sendq_head = NULL;
    vc->ch.cm_sendq_tail = NULL;
    vc->ch.cm_1sc_sendq_head = NULL;
    vc->ch.cm_1sc_sendq_tail = NULL;
    vc->ch.pending_r3_data = 0;
    vc->ch.received_r3_data = 0;
#ifdef _ENABLE_XRC_
    vc->ch.xrc_flags = 0;
    vc->ch.xrc_conn_queue = NULL;
    vc->ch.orig_vc = NULL;
    memset(vc->ch.xrc_srqn, 0, sizeof(uint32_t) * MAX_NUM_HCAS);
    memset(vc->ch.xrc_rqpn, 0, sizeof(uint32_t) * MAX_NUM_SUBRAILS);
    memset(vc->ch.xrc_my_rqpn, 0, sizeof(uint32_t) * MAX_NUM_SUBRAILS);
#endif

    vc->smp.hostid = -1;
    vc->force_rndv = 0;

    vc->rndvSend_fn = MPID_MRAIL_RndvSend;
    vc->rndvRecv_fn = MPID_MRAIL_RndvRecv;

#if defined(CKPT)
    vc->ch.rput_stop = 0;
#endif /* defined(CKPT) */

#ifdef USE_RDMA_UNEX
    vc->ch.unex_finished_next = NULL;
    vc->ch.unex_list = NULL;
#endif
    /* It is needed for temp vc */
    vc->eager_max_msg_sz = rdma_iba_eager_threshold;

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_CH3_VC_INIT);
    return mpi_errno;
}

int MPIDI_CH3_PortFnsInit(MPIDI_PortFns *portFns)
{
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_CH3_RDMA_PORTFNSINIT);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_CH3_RDMA_PORTFNSINIT);

    if (!MPIDI_CH3I_Process.has_dpm) {
        portFns->OpenPort = 0;
        portFns->ClosePort = 0;
        portFns->CommAccept = 0;
        portFns->CommConnect = 0;
    } else
        MPL_UNREFERENCED_ARG(portFns);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_CH3_RDMA_PORTFNSINIT);
    return MPI_SUCCESS;
}

int MPIDI_CH3_Connect_to_root(const char *port_name, MPIDI_VC_t **new_vc)
{
    int mpi_errno = MPI_SUCCESS;
    int str_errno;
    char ifname[MAX_HOST_DESCRIPTION_LEN];
    MPIDI_VC_t *vc;
    MPIDI_CH3_Pkt_cm_establish_t pkt;
    MPIR_Request *sreq;
    int seqnum;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_CH3_CONNECT_TO_ROOT);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_CH3_CONNECT_TO_ROOT);

    *new_vc = NULL;
    if (!MPIDI_CH3I_Process.has_dpm)
        return MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_FATAL, __func__,
                                    __LINE__, MPI_ERR_OTHER, "**notimpl", 0);

    str_errno =
        MPL_str_get_string_arg(port_name, MPIDI_CH3I_HOST_DESCRIPTION_KEY,
                               ifname, MAX_HOST_DESCRIPTION_LEN);
    if (str_errno != MPL_SUCCESS) {
        /* --BEGIN ERROR HANDLING */
        if (str_errno == MPL_ERR_STR_FAIL) {
            MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER,
                                "**argstr_missinghost");
        } else {
            /* MPIU_STR_TRUNCATED or MPIU_STR_NONEM */
            MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER, "**argstr_hostd");
        }
        /* --END ERROR HANDLING-- */
    }

    vc = MPL_malloc(sizeof(MPIDI_VC_t), MPL_MEM_OBJECT);
    if (!vc) {
        MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER, "**nomem");
    }
    MPIDI_VC_Init(vc, NULL, 0);

    mpi_errno = MPIDI_CH3I_CM_Connect_raw_vc(vc, ifname);
    MPIR_ERR_CHECK(mpi_errno);

    while (vc->ch.state != MPIDI_CH3I_VC_STATE_IDLE) {
        mpi_errno = MPID_Progress_test(NULL);
        /* --BEGIN ERROR HANDLING-- */
        MPIR_ERR_CHECK(mpi_errno);
    }

    /* fprintf(stderr, "[###] vc state to idel, now send cm_establish msg\n") */
    /* Now a connection is created, send a cm_establish message */
    /* FIXME: vc->mrail.remote_vc_addr is used to find remote vc
     * A more elegant way is needed */
    MPIDI_Pkt_init(&pkt, MPIDI_CH3_PKT_CM_ESTABLISH);
    MPIDI_VC_FAI_send_seqnum(vc, seqnum);
    MPIDI_Pkt_set_seqnum(&pkt, seqnum);
    pkt.vc_addr = vc->mrail.remote_vc_addr;
    mpi_errno = MPIDI_GetTagFromPort(port_name, &pkt.port_name_tag);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER, "**argstr_port_name_tag");
    }

    mpi_errno = MPIDI_CH3_iStartMsg(vc, &pkt, sizeof(pkt), &sreq);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_SETANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**fail", "**fail %s",
                             "Failed to send cm establish message");
    }

    if (sreq != NULL) {
        if (sreq->status.MPI_ERROR != MPI_SUCCESS) {
            mpi_errno = MPIR_Err_create_code(sreq->status.MPI_ERROR,
                                             MPIR_ERR_FATAL, __func__, __LINE__,
                                             MPI_ERR_OTHER, "**fail", 0);
            MPIR_Request_free(sreq);
            goto fn_fail;
        }
        MPIR_Request_free(sreq);
    }

    *new_vc = vc;

fn_fail:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_CH3_CONNECT_TO_ROOT);

    return mpi_errno;
}

int MPIDI_CH3_Get_business_card(int myRank, char *value, int length)
{
    char ifname[MAX_HOST_DESCRIPTION_LEN];
    int mpi_errno;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_CH3_GET_BUSINESS_CARD);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_CH3_GET_BUSINESS_CARD);

    mpi_errno = MPIDI_CH3I_CM_Get_port_info(ifname, MAX_HOST_DESCRIPTION_LEN);
    MPIR_ERR_CHECK(mpi_errno);

    mpi_errno = MPL_str_add_string_arg(&value, &length,
                                       MPIDI_CH3I_HOST_DESCRIPTION_KEY, ifname);
    if (mpi_errno != MPL_SUCCESS) {
        if (mpi_errno == MPL_ERR_STR_NOMEM) {
            MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER, "**buscard_len");
        } else {
            MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER, "**buscard");
        }
    }

fn_fail:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_CH3_GET_BUSINESS_CARD);
    return mpi_errno;
}

/* This routine is a hook for initializing information for a process
   group before the MPIDI_CH3_VC_Init routine is called */
int MPIDI_CH3_PG_Init(MPIDI_PG_t *pg)
{
    int mpi_errno = MPI_SUCCESS;

    if (MVP_CVAR_IS_SET_BY_USER(MVP_SUPPORT_DPM)) {
        if (MVP_SUPPORT_DPM) {
            MVP_SHMEM_BACKED_UD_CM = 0;
        }
    }

    pg->ch.mrail = MPL_malloc(sizeof(MPIDI_CH3I_MRAIL_CM_t), MPL_MEM_OBJECT);
    if (pg->ch.mrail == NULL) {
        MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_INTERN, "**nomem",
                                  "**nomem %s", "ud_cm mrail");
    }
    MPIR_Memset(pg->ch.mrail, 0, sizeof(MPIDI_CH3I_MRAIL_CM_t));

    if (!MVP_SHMEM_BACKED_UD_CM) {
        pg->ch.mrail->cm_shmem.ud_cm = MPL_malloc(
            pg->size * sizeof(MPIDI_CH3I_MRAIL_UD_CM_t), MPL_MEM_SHM);
        if (pg->ch.mrail->cm_shmem.ud_cm == NULL) {
            MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_INTERN, "**nomem",
                                      "**nomem %s", "ud_cm");
        }
        MPIR_Memset(pg->ch.mrail->cm_shmem.ud_cm, 0,
                    pg->size * sizeof(MPIDI_CH3I_MRAIL_UD_CM_t));
    }

fn_fail:
    return MPI_SUCCESS;
}

/* This routine is a hook for any operations that need to be performed before
   freeing a process group */
int MPIDI_CH3_PG_Destroy(struct MPIDI_PG *pg)
{
    return MPIDI_CH3I_MRAIL_CM_Dealloc(pg);
}

/* This routine is a hook for any operations that need to be performed before
   freeing a virtual connection */
int MPIDI_CH3_VC_Destroy(struct MPIDI_VC *vc)
{
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_CH3_VC_DESTROY);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_CH3_VC_DESTROY);

#if !defined(CHANNEL_PSM)
    if (vc->smp.sendq_head != NULL) {
        MPL_free(vc->smp.sendq_head);
    }
    if (vc->smp.sendq_tail != NULL) {
        MPL_free(vc->smp.sendq_tail);
    }
    if (vc->smp.recv_active != NULL) {
        MPL_free(vc->smp.recv_active);
    }
    if (vc->smp.send_active != NULL) {
        MPL_free(vc->smp.send_active);
    }
    if (vc->ch.req != NULL) {
        MPL_free(vc->ch.req);
    }
    if (vc->mrail.cmanager.msg_channels != NULL) {
        MPL_free(vc->mrail.cmanager.msg_channels);
    }
    if (vc->mrail.srp.credits != NULL) {
        MPL_free(vc->mrail.srp.credits);
    }
    if (vc->mrail.rails != NULL) {
        MPL_free(vc->mrail.rails);
    }
#endif /* #if !defined (CHANNEL_PSM) */

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_CH3_VC_DESTROY);
    return MPI_SUCCESS;
}

int MPIDI_CH3_InitCompleted(void)
{
    int show_binding = 0;
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_CH3_INITCOMPLETED);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_CH3_INITCOMPLETED);

    /* A reasonable place to put this rather than directly in the ch3 src */
    MPL_env2int("MVP_SHOW_CPU_BINDING", &show_binding);
    if (show_binding) {
        mvp_show_cpu_affinity(show_binding);
    }
    /* Unlink hwloc topology file */
    mpi_errno = smpi_unlink_hwloc_topology_file();

    MPL_env2int("MVP_SHOW_HCA_BINDING", &show_binding);
    if (show_binding && !SMP_ONLY) {
        mvp_show_hca_affinity(show_binding);
    }

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_CH3_INITCOMPLETED);
    return MPI_SUCCESS;
}

void rdma_process_hostid(MPIDI_PG_t *pg, int *host_ids, int my_rank,
                         int pg_size)
{
    int i;
    int my_host_id;
    ;
    MPIDI_VC_t *vc = NULL;

    pg->ch.local_process_id = 0;
    pg->ch.num_local_processes = 0;

    my_host_id = host_ids[my_rank];
    for (i = 0; i < pg_size; ++i) {
        MPIDI_PG_Get_vc(pg, i, &vc);
        if (host_ids[i] == my_host_id) {
            vc->smp.local_rank = pg->ch.num_local_processes++;
            if (i == my_rank) {
                pg->ch.local_process_id = vc->smp.local_rank;
            }
        } else {
            vc->smp.local_rank = -1;
        }
    }
}

/* TODO: this should move. Only here because it was in MPID_Init before */
/* Synchronous checkpoint interface*/
#if defined(CKPT)
int MVAPICH_Sync_Checkpoint()
{
    MPIR_Comm *comm_ptr;
    int errflag = FALSE;

    if (MPIDI_Process.use_sync_ckpt == 0) /*Not enabled*/
        return 0;

    MPIR_Comm_get_ptr(MPI_COMM_WORLD, comm_ptr);

    /*MPIU_THREAD_SINGLE_CS_ENTER("coll");*/
    MPID_THREAD_CS_ENTER(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);
    MPIR_Barrier_impl(comm_ptr, &errflag);
    MPID_THREAD_CS_EXIT(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);
    /*MPIU_THREAD_SINGLE_CS_EXIT("coll");*/

    if (MPIDI_Process.my_pg_rank == 0) { /*Notify console to take checkpoint*/
        MPIDI_CH3I_CR_Sync_ckpt_request();
    }

    /*Now wait for the lower layer to indicate that the checkpoint finished*/
    pthread_mutex_lock(&MVAPICH_sync_ckpt_lock);
    pthread_cond_wait(&MVAPICH_sync_ckpt_cond, &MVAPICH_sync_ckpt_lock);
    pthread_mutex_unlock(&MVAPICH_sync_ckpt_lock);
    return 0;
}
#endif /* defined(CKPT) */

/* vi: set sw=4 */
