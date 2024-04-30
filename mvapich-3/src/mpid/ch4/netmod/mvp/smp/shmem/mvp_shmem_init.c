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

#include "mvp_smp_impl.h"
#include "mvp_vc.h"
#include "mvp_smp_params.h"
#include "mvp_shmem.h"
#include "mvp_coll_shmem.h"
#include "mvp_vc_utils.h"

#include <assert.h>
#if defined(MAC_OSX)
#include <netinet/in.h>
#endif /* defined(MAC_OSX) */

int mvp_shmem_pool_init = 0;
int polling_set_p_head = 0;
int polling_set_p_tail = 0;
int polling_set_c_head = 0;
int polling_set_c_tail = 0;
mvp_smp_poll_t *polling_set_p = NULL;
mvp_smp_poll_t *polling_set_c = NULL;
int *polling_counters;
extern int mvp_is_fair_polling;

int mvp_set_smp_tuning_parameters();

#define SMP_EXIT_ERR -1
#define smp_error_abort(code, message) do {                     \
    if (errno) {                                                \
        PRINT_ERROR_ERRNO( "%s:%d: " message, errno, __FILE__, __LINE__);     \
    } else {                                                    \
        PRINT_ERROR( "%s:%d: " message "\n", __FILE__, __LINE__);     \
    }                                                           \
    fflush (stderr);                                            \
    exit(code);                                                 \
} while (0)

/* Shared Tail Pointer: updated by receiver after every receive;
 * read by sender when local header meets local tail. */
mvp_smp_info_t mvp_smp_info;
mvp_smp_shmem_region_t *mvp_smp_shmem_region;
struct mvp_shared_buffer_pool s_sh_buf_pool;
mvp_smp_send_buf_t **s_buffer_head = NULL;
mvp_smp_send_buf_t *s_my_buffer_head = NULL;
int mvp_smp_init = 0;
int mvp_smp_only = 0;
long int mvp_num_queued_smp_ops = 0;
void** s_current_ptr = NULL;
intptr_t* s_current_bytes = NULL;
intptr_t* s_total_bytes = NULL;
static char *shmem_file = NULL;
static char *pool_file = NULL;

/* local header/tail for send and receive pointing to cyclic buffer */
size_t* s_header_ptr_s = NULL;
size_t* s_tail_ptr_s = NULL;
size_t* avail = NULL;
size_t* s_header_ptr_r = NULL;

size_t g_size_shmem = 0;
size_t g_size_pool = 0;

/* SMP user parameters */
unsigned long eager_buffer_max_usage = 0;
unsigned long rndv_buffer_max_usage = 0;

int MPIR_MVP_SHMEM_Sync(volatile int *volatile bar_array, int my_local_id,
                        int num_local_procs);
int MPIR_MVP_SHMEM_Helper_fn(int local_id, char **filename, char *prefix,
                             int *fd, size_t file_size);

static int smpi_exchange_info()
{
    int mpi_errno = MPI_SUCCESS;
    int pg_rank, pg_size;
    int i = 0;
    int j;
    MPIDI_MVP_ep_t *vc = NULL;
    MPIR_Comm *comm_ptr;
    MPIR_Comm_get_ptr(MPI_COMM_WORLD, comm_ptr);
    int my_node, remote_node;
    int num_local_procs = 0;
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_SMPI_EXCHANGE_INFO);

    pg_rank = MPIR_Process.rank;
    pg_size = MPIR_Process.size;

    mvp_smp_info.num_local_nodes = MPIR_Process.local_size;
    if (mvp_smp_info.num_local_nodes >= MVP_SHMEM_PRIORTY_THRESHOLD) {
        MVP_SMP_PRIORITY_FACTOR = MVP_SHMEM_PRIORTY_FACTOR;
    }

    MPID_Get_node_id(comm_ptr, pg_rank, &my_node);
    for (i = 0; i < pg_size; ++i) {
        MPID_Get_node_id(comm_ptr, i, &remote_node);
        MPIDI_av_entry_t *av = MPIDIU_comm_rank_to_av(comm_ptr, i);
        vc = MPIDI_MVP_VC(av);
        if (my_node == remote_node) {
            vc->smp.local_rank = num_local_procs;
            num_local_procs++;
        } else {
            vc->smp.local_rank = -1;
        }
        vc->smp.local_nodes = vc->smp.local_rank;
        PRINT_DEBUG(DEBUG_SHM_verbose, "vc %p, i %d vc->smp.local_nodes %d \n ", vc, i, vc->smp.local_nodes); 
    }

    MPIR_Assert(num_local_procs == MPIR_Process.local_size);

    /* Get my VC */
    vc = MPIDI_MVP_VC(MPIDIU_comm_rank_to_av(comm_ptr, pg_rank));
    mvp_smp_info.my_local_id = vc->smp.local_nodes;

    PRINT_DEBUG(DEBUG_SHM_verbose > 1, "num local nodes %d, my local id %d\n",
                mvp_smp_info.num_local_nodes, mvp_smp_info.my_local_id);

    mvp_smp_info.l2g_rank = (unsigned int *)MPL_malloc(
        mvp_smp_info.num_local_nodes * sizeof(int), MPL_MEM_OTHER);
    if (mvp_smp_info.l2g_rank == NULL) {
        MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**nomem",
                                  "**nomem %s", "mvp_smp_info.12g_rank");
#if defined(__SUNPRO_C) || defined(__SUNPRO_CC)
#pragma error_messages(off, E_STATEMENT_NOT_REACHED)
#endif /* defined(__SUNPRO_C) || defined(__SUNPRO_CC) */
        MPL_error_printf(
                "malloc: in ib_rank_lid_table for SMP");
#if defined(__SUNPRO_C) || defined(__SUNPRO_CC)
#pragma error_messages(default, E_STATEMENT_NOT_REACHED)
#endif /* defined(__SUNPRO_C) || defined(__SUNPRO_CC) */
    }

    for (i = 0, j = 0; j < pg_size; ++j) {

        vc = MPIDI_MVP_VC(MPIDIU_comm_rank_to_av(comm_ptr, j));
        if (vc->smp.local_nodes != -1) {
            mvp_smp_info.l2g_rank[i] = j;
            i++;
        }
    }

fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_SMPI_EXCHANGE_INFO);
    return mpi_errno;

fn_fail:
    goto fn_exit;
}


void MPIDI_MVP_shmem_cleanup()
{
    /*clean up pool file*/
    if (mvp_smp_info.send_buf_pool_ptr != NULL) {
        munmap(mvp_smp_info.send_buf_pool_ptr, g_size_pool);
    }
    if (mvp_smp_info.fd_pool != -1) {
        close(mvp_smp_info.fd_pool);
        unlink(pool_file);
    }
    if (pool_file != NULL) {
        MPL_free(pool_file);
    }
    mvp_smp_info.send_buf_pool_ptr = NULL;
    mvp_smp_info.fd_pool = -1;
    pool_file = NULL;

    /*clean up shmem file*/
    if (mvp_smp_info.mmap_ptr != NULL) {
        munmap((void *)mvp_smp_info.mmap_ptr, g_size_shmem);
    }
    if (mvp_smp_info.fd != -1) {
        close(mvp_smp_info.fd);
        unlink(shmem_file);
    }
    if (shmem_file != NULL) {
        MPL_free(shmem_file);
    }
    mvp_smp_info.mmap_ptr = NULL;
    mvp_smp_info.fd = -1;
    shmem_file = NULL;
}

void MPIDI_MVP_shmem_unlink()
{
    /*clean up pool file*/
    if (mvp_smp_info.fd_pool != -1) {
        unlink(pool_file);
    }
    if (pool_file != NULL) {
        MPL_free(pool_file);
    }
    pool_file = NULL;

    /*clean up shmem file*/
    if (mvp_smp_info.fd != -1) {
        unlink(shmem_file);
    }
    if (shmem_file != NULL) {
        MPL_free(shmem_file);
    }
    shmem_file = NULL;
}

static inline int MPIDI_MVP_Get_num_nodes()
{
    int num_nodes = 0;
    MPID_Get_max_node_id(NULL, &num_nodes);
    return num_nodes + 1;
}

int MPIDI_MVP_smp_attach_shm_pool_inline()
{
    int mpi_errno = MPI_SUCCESS;
    int i, j;
    int pagesize = getpagesize();
    mvp_smp_send_buf_t *send_buf = NULL;
    volatile char tmpchar ATTRIBUTE((unused));

    mvp_smp_info.send_buf_pool_ptr =
        mmap(0, g_size_pool, (PROT_READ | PROT_WRITE), (MAP_SHARED),
             mvp_smp_info.fd_pool, 0);
    if (mvp_smp_info.send_buf_pool_ptr == (void *)-1) {
        /* to clean up tmp shared file */
        mpi_errno = MPIR_Err_create_code(MPI_SUCCESS, MPI_ERR_OTHER, __func__,
                                         __LINE__, MPI_ERR_OTHER, "**nomem",
                                         "%s: %s", "mmap", strerror(errno));
        MPIR_ERR_POP(mpi_errno);
    }

    s_buffer_head = (mvp_smp_send_buf_t **)MPL_malloc(
        sizeof(mvp_smp_send_buf_t *) * mvp_smp_info.num_local_nodes,
        MPL_MEM_ADDRESS);
    if(!s_buffer_head) {
       mpi_errno = MPIR_Err_create_code(MPI_SUCCESS, MPI_ERR_OTHER,
                __func__, __LINE__, MPI_ERR_OTHER, "**nomem", 0);
       MPIR_ERR_POP(mpi_errno);
    }

    for (i = 0; i < mvp_smp_info.num_local_nodes; ++i) {
        s_buffer_head[i] =
            (mvp_smp_send_buf_t *)((unsigned long)
                                       mvp_smp_info.send_buf_pool_ptr +
                                   SMPI_ALIGN((sizeof(mvp_smp_send_buf_t) +
                                               MVP_SMP_SEND_BUF_SIZE) *
                                                  MVP_SMP_NUM_SEND_BUFFER +
                                              pagesize) *
                                       i);

        if (((long)s_buffer_head[i] & (SMPI_CACHE_LINE_SIZE - 1)) != 0) {
            /* to clean up tmp shared file */
            mpi_errno = MPIR_Err_create_code(
                MPI_SUCCESS, MPI_ERR_OTHER, __func__, __LINE__, MPI_ERR_OTHER,
                "**fail", "%s", "error in shifting mapped pool");
            MPIR_ERR_POP(mpi_errno);
        }
    }
    s_my_buffer_head = s_buffer_head[mvp_smp_info.my_local_id];

    s_sh_buf_pool.free_head = 0;

    s_sh_buf_pool.send_queue = (int *)MPL_malloc(
        sizeof(int) * mvp_smp_info.num_local_nodes, MPL_MEM_ADDRESS);
    if(!s_sh_buf_pool.send_queue) {
       mpi_errno = MPIR_Err_create_code(MPI_SUCCESS, MPI_ERR_OTHER,
                __func__, __LINE__, MPI_ERR_OTHER, "**nomem", 0);
       MPIR_ERR_POP(mpi_errno);
    }

    s_sh_buf_pool.tail = (int *)MPL_malloc(
        sizeof(int) * mvp_smp_info.num_local_nodes, MPL_MEM_ADDRESS);
    if(!s_sh_buf_pool.tail) {
       mpi_errno = MPIR_Err_create_code(MPI_SUCCESS, MPI_ERR_OTHER,
                __func__, __LINE__, MPI_ERR_OTHER, "**nomem", 0);
       MPIR_ERR_POP(mpi_errno);
    }

    for (i = 0; i < mvp_smp_info.num_local_nodes; ++i) {
        s_sh_buf_pool.send_queue[i] = s_sh_buf_pool.tail[i] = -1;
    }

    for (i = 0; i < MVP_SMP_NUM_SEND_BUFFER; ++i) {
        send_buf = MPIDI_MVP_MY_SHMEM_BUF_POOL_PTR(i);
        send_buf->myindex = i;
        send_buf->next = i+1;
        send_buf->busy = 0;
        send_buf->len = 0;
        send_buf->has_next = 0;
        send_buf->msg_complete = 0;

        for (j = 0; j < MVP_SMP_SEND_BUF_SIZE; j += pagesize) {
            tmpchar = *((char *) &send_buf->buf + j);
        }
    }
    send_buf->next = -1;

    mvp_shmem_pool_init = 1;

fn_exit:
    return mpi_errno;
fn_fail:
    MPIDI_MVP_shmem_cleanup();
    goto fn_exit;
}

static inline int MPIDI_MVP_read_shmem_cvars()
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_SMP_SHMEM_INIT);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_SHMEM_INIT);

    /* Disable PT2PT shared memory if global shared memory is disabled */
    if (!MVP_USE_SHARED_MEM) {
        MVP_USE_PT2PT_SHMEM = 0;
    }

    /* Disable SMP if blocking is enabled */
    if (MVP_USE_BLOCKING) {
        MVP_USE_PT2PT_SHMEM = 0;
    }

    /* not a cvar, but relevant here */
    if (MPIR_Process.num_nodes == 1 && MVP_USE_PT2PT_SHMEM) {
        mvp_smp_info.only_one_device = 1;
        mvp_smp_only = 1;
    }
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_SHMEM_INIT);
    return mpi_errno;
}

int MPIDI_MVP_smp_shmem_init()
{
    int mpi_errno = MPI_SUCCESS;
    unsigned int i;
    size_t sh_size, pid_len, st_len;
    int pagesize = getpagesize();
    volatile mvp_smp_shmem_region_t *shmem;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_SMP_SHMEM_INIT);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_SHMEM_INIT);
#if defined(_SMP_CMA_)
    size_t cma_test_buffer_offset;
#endif /* defined(_SMP_CMA_) */
#if defined(__x86_64__)
    volatile char tmpchar ATTRIBUTE((unused));
#endif /* defined(__x86_64__) */

    mvp_set_smp_tuning_parameters();

    if (MVP_USE_BLOCKING) {
        /* blocking is enabled, so
         * automatically disable
         * shared memory */
        return MPI_SUCCESS;
    }

    if (!MVP_USE_PT2PT_SHMEM) {
        return MPI_SUCCESS;
    }

    /*
     * Do the initializations here. These will be needed on restart
     * after a checkpoint has been taken.
     */
    if ((mpi_errno = smpi_exchange_info()) != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }

    PRINT_DEBUG(DEBUG_SHM_verbose>1, "finished exchange info\n");

    PRINT_DEBUG(DEBUG_SHM_verbose > 1,
                "smp eager size %d\n, smp queue length %zu\n",
                MVP_SMP_EAGERSIZE, MVP_SMP_QUEUE_LENGTH);

    if (MVP_SMP_EAGERSIZE > MVP_SMP_QUEUE_LENGTH / 2) {
        MPIR_ERR_SETFATALANDJUMP1(
            mpi_errno, MPI_ERR_OTHER, "**fail", "**fail %s",
            "MVP_SMP_EAGERSIZE should not exceed half of "
            "MVP_SMP_QUEUE_LENGTH. Note that MVP_SMP_EAGERSIZE "
            "and MVP_SMP_QUEUE_LENGTH are set in KBytes.");
    }

    /* Initialize variables before setting up shmem regions */
    mvp_smp_info.fd = -1;
    mvp_smp_info.fd_pool = -1;
    mvp_smp_info.mmap_ptr = NULL;
    mvp_smp_info.send_buf_pool_ptr = NULL;
    mvp_smp_info.available_queue_length =
        (MVP_SMP_QUEUE_LENGTH - MVP_SMP_EAGERSIZE - sizeof(size_t));

    /* Compute the size of shmem files */
    pid_len = mvp_smp_info.num_local_nodes * sizeof(size_t);
    /* pid_len need to be padded to cache aligned, in order to make sure the
     * following flow control structures cache aligned.
     */
    pid_len = pid_len + SMPI_CACHE_LINE_SIZE - (pid_len % SMPI_CACHE_LINE_SIZE);
    st_len = sizeof(mvp_smp_shared_tails_t) * mvp_smp_info.num_local_nodes *
             (mvp_smp_info.num_local_nodes - 1);
    sh_size = sizeof(mvp_smp_shmem_region_t) + pid_len + SMPI_ALIGN(st_len) +
              SMPI_CACHE_LINE_SIZE * 3;

    g_size_shmem =
        (SMPI_CACHE_LINE_SIZE + sh_size + pagesize +
         (mvp_smp_info.num_local_nodes * (mvp_smp_info.num_local_nodes - 1) *
          (SMPI_ALIGN(MVP_SMP_QUEUE_LENGTH + pagesize))));

    MPIR_T_PVAR_LEVEL_INC(MVP, mvp_smp_eager_total_buffer,
                          ((mvp_smp_info.num_local_nodes - 1) *
                           (SMPI_ALIGN(MVP_SMP_QUEUE_LENGTH + pagesize))));

    MPIR_T_PVAR_LEVEL_INC(MVP, mvp_smp_eager_avail_buffer,
                          ((mvp_smp_info.num_local_nodes - 1) *
                           (SMPI_ALIGN(MVP_SMP_QUEUE_LENGTH + pagesize))));

#if defined(_SMP_CMA_)
    cma_test_buffer_offset = g_size_shmem;
    g_size_shmem += SMPI_ALIGN(1);
#endif /* defined(_SMP_CMA_) */

    g_size_pool =
        SMPI_ALIGN((sizeof(mvp_smp_send_buf_t) + MVP_SMP_SEND_BUF_SIZE) *
                       MVP_SMP_NUM_SEND_BUFFER +
                   pagesize) *
            mvp_smp_info.num_local_nodes +
        SMPI_CACHE_LINE_SIZE;

    MPIR_T_PVAR_LEVEL_INC(
        MVP, mvp_smp_rndv_total_buffer,
        SMPI_ALIGN((sizeof(mvp_smp_send_buf_t) + MVP_SMP_SEND_BUF_SIZE) *
                       MVP_SMP_NUM_SEND_BUFFER +
                   pagesize));

    MPIR_T_PVAR_LEVEL_INC(
        MVP, mvp_smp_rndv_avail_buffer,
        SMPI_ALIGN((sizeof(mvp_smp_send_buf_t) + MVP_SMP_SEND_BUF_SIZE) *
                       MVP_SMP_NUM_SEND_BUFFER +
                   pagesize));

    PRINT_DEBUG(DEBUG_SHM_verbose>1, "size_shmem=%zu, size_pool = %zu\n",
                g_size_shmem, g_size_pool);

    /* Call helper function to create shmem region */
    mpi_errno =
        MPIR_MVP_SHMEM_Helper_fn(mvp_smp_info.my_local_id, &shmem_file,
                                 "ib_shmem", &mvp_smp_info.fd, g_size_shmem);
    MPIR_ERR_CHECK(mpi_errno);

    /* Call helper function to create shmem region */
    mpi_errno =
        MPIR_MVP_SHMEM_Helper_fn(mvp_smp_info.my_local_id, &pool_file,
                                 "ib_pool", &mvp_smp_info.fd_pool, g_size_pool);
    MPIR_ERR_CHECK(mpi_errno);

    mvp_smp_shmem_region = (mvp_smp_shmem_region_t *)MPL_malloc(
        sizeof(mvp_smp_shmem_region_t), MPL_MEM_SHM);
    if (!mvp_smp_shmem_region) {
        mpi_errno = MPIR_Err_create_code(MPI_SUCCESS, MPI_ERR_OTHER, __func__,
                                         __LINE__, MPI_ERR_OTHER, "**nomem", 0);
        MPIR_ERR_POP(mpi_errno);
    }

    PRINT_DEBUG(DEBUG_SHM_verbose>1, "before mmap\n");

    /* mmap of the shared memory file */
    mvp_smp_info.mmap_ptr = mmap(0, g_size_shmem, (PROT_READ | PROT_WRITE),
                                 (MAP_SHARED), mvp_smp_info.fd, 0);
    if (mvp_smp_info.mmap_ptr == (void *)-1) {
        /* to clean up tmp shared file */
        mpi_errno = MPIR_Err_create_code(MPI_SUCCESS, MPI_ERR_OTHER, __func__,
                                         __LINE__, MPI_ERR_OTHER, "**nomem",
                                         "%s: %s", "mmap", strerror(errno));
        MPIR_ERR_POP(mpi_errno);
    }

    shmem = (mvp_smp_shmem_region_t *)mvp_smp_info.mmap_ptr;
    if (((long) shmem & (SMPI_CACHE_LINE_SIZE - 1)) != 0) {
       /* to clean up tmp shared file */
       mpi_errno = MPIR_Err_create_code(MPI_SUCCESS, MPI_ERR_OTHER,
                __func__, __LINE__, MPI_ERR_OTHER, "**nomem", "%s",
                "error in shifting mapped shmem");
       MPIR_ERR_POP(mpi_errno);
    }

    if (!MVP_SMP_DELAY_SHMEM_POOL_INIT) {
        mpi_errno = MPIDI_MVP_smp_attach_shm_pool_inline();
        if (mpi_errno != MPI_SUCCESS) {
            /* to clean up tmp shared file */
            mpi_errno = MPIR_Err_create_code(MPI_SUCCESS, MPI_ERR_OTHER,
                    __func__, __LINE__, MPI_ERR_OTHER, "**nomem", "%s: %s",
                    "mmap", strerror(errno));
            MPIR_ERR_POP(mpi_errno);
        }
    }

    /* Initialize shared_mem pointers */
    mvp_smp_shmem_region->pid = (int *)shmem;

    mvp_smp_shmem_region->rqueues_limits_s = (mvp_smp_rq_limit_t *)MPL_malloc(
        sizeof(mvp_smp_rq_limit_t) * mvp_smp_info.num_local_nodes, MPL_MEM_SHM);
    mvp_smp_shmem_region->rqueues_limits_r = (mvp_smp_rq_limit_t *)MPL_malloc(
        sizeof(mvp_smp_rq_limit_t) * mvp_smp_info.num_local_nodes, MPL_MEM_SHM);
    mvp_smp_shmem_region->shared_tails = (mvp_smp_shared_tails_t **)MPL_malloc(
        sizeof(mvp_smp_shared_tails_t *) * mvp_smp_info.num_local_nodes,
        MPL_MEM_SHM);

    if (mvp_smp_shmem_region->rqueues_limits_s == NULL ||
        mvp_smp_shmem_region->rqueues_limits_r == NULL ||
        mvp_smp_shmem_region->shared_tails == NULL) {
        mpi_errno = MPIR_Err_create_code(MPI_SUCCESS, MPI_ERR_OTHER, __func__,
                                         __LINE__, MPI_ERR_OTHER, "**nomem", 0);
        MPIR_ERR_POP(mpi_errno);
    }

    if (mvp_smp_info.num_local_nodes > 1) {
        mvp_smp_shmem_region->shared_tails[0] =
            (mvp_smp_shared_tails_t *)((char *)shmem + pid_len +
                                       SMPI_CACHE_LINE_SIZE);

        for (i = 1; i < mvp_smp_info.num_local_nodes; ++i) {
            mvp_smp_shmem_region->shared_tails[i] =
                (mvp_smp_shared_tails_t *)(mvp_smp_shmem_region
                                               ->shared_tails[i - 1] +
                                           mvp_smp_info.num_local_nodes);
        }

        mvp_smp_shmem_region->pool =
            (char *)((char *)mvp_smp_shmem_region->shared_tails[1] +
                     SMPI_ALIGN(st_len) + SMPI_CACHE_LINE_SIZE);

    } else {
        mvp_smp_shmem_region->shared_tails[0] = NULL;
        mvp_smp_shmem_region->pool =
            (char *)((char *)shmem + pid_len + SMPI_CACHE_LINE_SIZE);
    }

    for (i = 0; i < mvp_smp_info.num_local_nodes; ++i) {
        if (i == mvp_smp_info.my_local_id)
            continue;
        mvp_smp_shmem_region->rqueues_limits_s[i].first =
            SMPI_ALIGN(pagesize + (pagesize + MVP_SMP_QUEUE_LENGTH) *
                                      (i * (mvp_smp_info.num_local_nodes - 1) +
                                       (mvp_smp_info.my_local_id > i ?
                                            (mvp_smp_info.my_local_id - 1) :
                                            mvp_smp_info.my_local_id)));
        mvp_smp_shmem_region->rqueues_limits_r[i].first = SMPI_ALIGN(
            pagesize +
            (pagesize + MVP_SMP_QUEUE_LENGTH) *
                (mvp_smp_info.my_local_id * (mvp_smp_info.num_local_nodes - 1) +
                 (i > mvp_smp_info.my_local_id ? (i - 1) : i)));
        mvp_smp_shmem_region->rqueues_limits_s[i].last =
            SMPI_ALIGN(pagesize +
                       (pagesize + MVP_SMP_QUEUE_LENGTH) *
                           (i * (mvp_smp_info.num_local_nodes - 1) +
                            (mvp_smp_info.my_local_id > i ?
                                 (mvp_smp_info.my_local_id - 1) :
                                 mvp_smp_info.my_local_id)) +
                       mvp_smp_info.available_queue_length);
        mvp_smp_shmem_region->rqueues_limits_r[i].last = SMPI_ALIGN(
            pagesize +
            (pagesize + MVP_SMP_QUEUE_LENGTH) *
                (mvp_smp_info.my_local_id * (mvp_smp_info.num_local_nodes - 1) +
                 (i > mvp_smp_info.my_local_id ? (i - 1) : i)) +
            mvp_smp_info.available_queue_length);
        mvp_smp_shmem_region->shared_tails[mvp_smp_info.my_local_id][i].ptr =
            mvp_smp_shmem_region->rqueues_limits_r[i].first;
        *(int *)((unsigned long)(mvp_smp_shmem_region->pool) +
                 mvp_smp_shmem_region->rqueues_limits_r[i].first) = 0;
    }

    /* Another synchronization barrier */
    mpi_errno =
        MPIR_MVP_SHMEM_Sync(mvp_smp_shmem_region->pid, mvp_smp_info.my_local_id,
                            mvp_smp_info.num_local_nodes);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }

    /* Unlinking shared memory files*/
    MPIDI_MVP_shmem_unlink();

#if defined(__x86_64__)
    /*
     * Okay, here we touch every page in the shared memory region.
     * We do this to get the pages allocated so that they are local
     * to the receiver on a numa machine (instead of all being located
     * near the first process).
     */
    if (MVP_WALK_SHARED_PAGES) {
        int receiver, sender;

        for (receiver = 0; receiver < mvp_smp_info.num_local_nodes;
             ++receiver) {
            volatile char *ptr = mvp_smp_shmem_region->pool;
            volatile char tmp ATTRIBUTE((unused));

            sender = mvp_smp_info.my_local_id;
            if (sender != receiver) {
                int k;

                for (k = MPIDI_MVP_SHMEM_FIRST_SEND(sender, receiver);
                     k < MPIDI_MVP_SHMEM_LAST_SEND(sender, receiver);
                     k += pagesize) {
                    tmp = ptr[k];
                }
            }
        }
    }
#endif /* defined(__x86_64__) */

    s_current_ptr = (void **)MPL_malloc(
        sizeof(void *) * mvp_smp_info.num_local_nodes, MPL_MEM_OTHER);
    if (!s_current_ptr) {
      MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**nomem",
          "**nomem %s", "s_current_ptr");
    }

    s_current_bytes = (intptr_t *)MPL_malloc(
        sizeof(intptr_t) * mvp_smp_info.num_local_nodes, MPL_MEM_OTHER);
    if (!s_current_bytes) {
      MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**nomem",
          "**nomem %s", "s_current_bytes");
    }

    s_total_bytes = (intptr_t *)MPL_malloc(
        sizeof(intptr_t) * mvp_smp_info.num_local_nodes, MPL_MEM_OTHER);
    if (!s_total_bytes) {
       MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**nomem",
           "**nomem %s", "s_total_bytes");
    }

    s_header_ptr_s = (size_t *)MPL_malloc(
        sizeof(size_t) * mvp_smp_info.num_local_nodes, MPL_MEM_OTHER);

    if(!s_header_ptr_s) {
       MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**nomem",
			    "**nomem %s", "s_header_ptr");
    }

    s_header_ptr_r = (size_t *)MPL_malloc(
        sizeof(size_t) * mvp_smp_info.num_local_nodes, MPL_MEM_OTHER);

    if(!s_header_ptr_r) {
    MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**nomem",
        "**nomem %s", "s_header_ptr");
    }

    s_tail_ptr_s = (size_t *)MPL_malloc(
        sizeof(size_t) * mvp_smp_info.num_local_nodes, MPL_MEM_OTHER);

    if(!s_tail_ptr_s) {
    MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**nomem",
        "**nomem %s", "s_tail_ptr");
    }

    avail = (size_t *)MPL_malloc(sizeof(size_t) * mvp_smp_info.num_local_nodes,
                                 MPL_MEM_OTHER);

    if(!avail) {

    MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**nomem",
        "**nomem %s", "avail");
    }

    for (i = 0; i < mvp_smp_info.num_local_nodes; ++i) {
        s_current_ptr[i] = NULL;
        s_current_bytes[i] = 0;
        s_total_bytes[i] = 0;

        if (i == mvp_smp_info.my_local_id)
            continue;
        s_header_ptr_r[i] =
            MPIDI_MVP_SHMEM_FIRST_RECV(i, mvp_smp_info.my_local_id);
        s_header_ptr_s[i] =
            MPIDI_MVP_SHMEM_FIRST_SEND(mvp_smp_info.my_local_id, i);
        s_tail_ptr_s[i] =
            MPIDI_MVP_SHMEM_LAST_SEND(mvp_smp_info.my_local_id, i);
        avail[i] = s_tail_ptr_s[i] - s_header_ptr_s[i];
    }

    polling_set_p = (mvp_smp_poll_t *)MPL_malloc(
        sizeof(mvp_smp_poll_t) * mvp_smp_info.num_local_nodes, MPL_MEM_OTHER);
    polling_set_c = (mvp_smp_poll_t *)MPL_malloc(
        sizeof(mvp_smp_poll_t) * mvp_smp_info.num_local_nodes, MPL_MEM_OTHER);

    for (i = 0; i < mvp_smp_info.num_local_nodes; ++i) {
        polling_set_p[i].rank = i;
        polling_set_p[i].next = 0;
        polling_set_p[i].prev = 0;

        polling_set_c[i].rank = i;
        polling_set_c[i].prev = i-1;
        polling_set_c[i].next = i+1;
    }

    polling_set_c[mvp_smp_info.num_local_nodes - 1].next = 0;
    polling_set_c_tail = mvp_smp_info.num_local_nodes - 1;
    if (mvp_smp_info.num_local_nodes > 1)
        polling_set_c_head = 1;

    polling_counters = (int *)MPL_malloc(
        sizeof(int) * mvp_smp_info.num_local_nodes, MPL_MEM_OTHER);
    for (i = 0; i < mvp_smp_info.num_local_nodes; ++i) {
        polling_counters[i] = 0;
    }

    mvp_smp_init = 1;
    MPIDI_MVP_read_shmem_cvars();
    MPIDI_MVP_smp_setup_vcs();
fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_SHMEM_INIT);
    return mpi_errno;

fn_fail:
    PRINT_DEBUG(DEBUG_SHM_verbose, "shm init failed \n");
    MPIDI_MVP_shmem_cleanup();
    goto fn_exit;
}

int MPIDI_MVP_pt2pt_shmem_finalize()
{
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_PT2PT_SHMEM_FINALIZE);
    /* reset global variables */
    mvp_smp_init = 0;
    mvp_shmem_pool_init = 0;
    MVP_SMP_DELAY_SHMEM_POOL_INIT = 1;
    polling_set_p_head = 0;
    polling_set_p_tail = 0;
    polling_set_c_head = 0;
    polling_set_c_tail = 0;

    /* free polling set structures */
    if (polling_counters) {
        MPL_free(polling_counters);
    }
    if (polling_set_p) {
        MPL_free(polling_set_p);
    }
    if (polling_set_c) {
        MPL_free(polling_set_c);
    }

    /* unmap the shared memory file */
    munmap((void *)mvp_smp_info.mmap_ptr, g_size_shmem);
    close(mvp_smp_info.fd);

    if (mvp_smp_info.send_buf_pool_ptr != NULL) {
        munmap(mvp_smp_info.send_buf_pool_ptr, g_size_pool);
        close(mvp_smp_info.fd_pool);
    }

    if(s_buffer_head) {
        MPL_free(s_buffer_head);
    }

    if (mvp_smp_info.l2g_rank) {
        MPL_free(mvp_smp_info.l2g_rank);
    }

    if (mvp_smp_shmem_region) {
        if (mvp_smp_shmem_region->rqueues_limits_s != NULL) {
            MPL_free(mvp_smp_shmem_region->rqueues_limits_s);
        }
        if (mvp_smp_shmem_region->rqueues_limits_r != NULL) {
            MPL_free(mvp_smp_shmem_region->rqueues_limits_r);
        }
        if (mvp_smp_shmem_region->shared_tails != NULL) {
            MPL_free(mvp_smp_shmem_region->shared_tails);
        }
        if (mvp_smp_shmem_region != NULL) {
            MPL_free(mvp_smp_shmem_region);
        }
    }

    if (s_current_ptr) {
        MPL_free(s_current_ptr);
    }

    if (s_current_bytes) {
        MPL_free(s_current_bytes);
    }

    if (s_total_bytes) {
        MPL_free(s_total_bytes);
    }

    if (s_header_ptr_s) {
        MPL_free(s_header_ptr_s);
    }

    if (s_header_ptr_r) {
        MPL_free(s_header_ptr_r);
    }

    if (s_tail_ptr_s) {
        MPL_free(s_tail_ptr_s);
    }

    if (avail) {
        MPL_free(avail);
    }

    if (s_sh_buf_pool.send_queue) {
        MPL_free(s_sh_buf_pool.send_queue);
    }

    if (s_sh_buf_pool.tail) {
        MPL_free(s_sh_buf_pool.tail);
    }

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_PT2PT_SHMEM_FINALIZE);
    return MPI_SUCCESS;
}

/* vi:set sw=4 */
