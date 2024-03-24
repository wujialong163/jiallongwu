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
/* TODO - Split this file as done with the C files */
#ifndef _MVP_COLL_SHMEM_
#define _MVP_COLL_SHMEM_

#include <pthread.h>
#include <math.h>
#include "mvp_debug_utils.h"
#include "mpidimpl.h"
#include "mvp_load_imbalance.h"
#include "mvp_pvars.h"

#ifdef CHANNEL_MRAIL
#include "mvp_arch_hca_detect.h"
#endif

#ifdef CHANNEL_MRAIL
#include "ib_param.h"
#endif
#ifdef CHANNEL_MRAIL_GEN2
#include "ibv_param.h"
#endif

#if defined(_SMP_LIMIC_)
#define LIMIC_COLL_NUM_COMM 128
#endif /* #if defined(_SMP_LIMIC_) */

#define PID_CHAR_LEN 22

#define SHMEM_COLL_HOSTNAME_LEN (255)

#define SHMEM_SMALLEST_SIZE (64)

#define SHMEM_MAX_INT ((unsigned int)(-1))

#define MVP_DEFAULT_SHMEM_BCAST_LEADERS      4096

#define MVP_DEFAULT_ZCPY_KNOMIAL_FACTOR       2

#define MVP_INTER_NODE_KNOMIAL_FACTOR_MAX 8
#define MVP_INTER_NODE_KNOMIAL_FACTOR_MIN 2
#define MVP_INTRA_NODE_KNOMIAL_FACTOR_MAX 8
#define MVP_INTRA_NODE_KNOMIAL_FACTOR_MIN 2

#if defined(_IA32_)

#define SHMEM_CACHE_LINE_SIZE 64
#define SHMEM_ALIGN(a)        ((a + SHMEM_CACHE_LINE_SIZE + 7) & 0xFFFFFFF8)
#define SHMEM_AVAIL(a)        ((a & 0xFFFFFFF8) - SHMEM_CACHE_LINE_SIZE)

#elif defined(_IA64_)

#define SHMEM_CACHE_LINE_SIZE 128
#define SHMEM_ALIGN(a)        ((a + SHMEM_CACHE_LINE_SIZE + 7) & 0xFFFFFFFFFFFFFFF8)
#define SHMEM_AVAIL(a)        ((a & 0xFFFFFFFFFFFFFFF8) - SHMEM_CACHE_LINE_SIZE)

#elif defined(__x86_64__)

#define SHMEM_CACHE_LINE_SIZE 64
#define SHMEM_ALIGN(a)        ((a + SHMEM_CACHE_LINE_SIZE + 7) & 0xFFFFFFFFFFFFFFF8)
#define SHMEM_AVAIL(a)        ((a & 0xFFFFFFFFFFFFFFF8) - SHMEM_CACHE_LINE_SIZE)

#elif defined(_EM64T_)

#define SHMEM_CACHE_LINE_SIZE 64
#define SHMEM_ALIGN(a)        (a + SHMEM_CACHE_LINE_SIZE)
#define SHMEM_AVAIL(a)        ((a & 0xFFFFFFFFFFFFFFF8) - SHMEM_CACHE_LINE_SIZE)

#elif defined(MAC_OSX)

#define SHMEM_CACHE_LINE_SIZE 16
#define SHMEM_ALIGN(a)        (((a + SHMEM_CACHE_LINE_SIZE + 7) & 0xFFFFFFF8))
#define SHMEM_AVAIL(a)        ((a & 0xFFFFFFF8) - SHMEM_CACHE_LINE_SIZE)

#else

#define SHMEM_CACHE_LINE_SIZE 64
#define SHMEM_ALIGN(a)        (a + SHMEM_CACHE_LINE_SIZE)
#define SHMEM_AVAIL(a)        ((a & 0xFFFFFFFFFFFFFFF8) - SHMEM_CACHE_LINE_SIZE)

#endif

#define SHMEM_COLL_BLOCK_SIZE                                                  \
    (MPIR_Process.local_size * MVP_SHMEM_COLL_MAX_MSG_SIZE)
#define MPICH_LARGE_MSG_COLLECTIVE_SIZE (1 * 1024 * 1024 * 1024)

#define COLL_COUNT          7
#define COLL_SIZE           3
#define ALLGATHER_ID        0
#define ALLREDUCE_SHMEM_ID  1
#define ALLREDUCE_2LEVEL_ID 2
#define BCAST_KNOMIAL_ID    3
#define BCAST_SHMEM_ID      4
#define REDUCE_SHMEM_ID     5
#define REDUCE_2LEVEL_ID    6

#define SMALL  0
#define MEDIUM 1
#define LARGE  2

#define MVP_PARA_PACKET_SIZE 5

/* collective helper functions */
int mvp_increment_shmem_coll_counter(MPIR_Comm *comm_ptr);
int is_shmem_collectives_enabled();

/* tuning tables */
extern int mvp_tuning_table[COLL_COUNT][COLL_SIZE];

struct scatter_tuning {
    int numproc;
    int small;
    int medium;
};

struct gather_tuning {
    int numproc;
    int switchp;
};

struct allgatherv_tuning {
    int numproc;
    int switchp;
};

#define BCAST_LEN         20
#define SHMEM_BCAST_FLAGS 1024

/*
 * We're converting this into a environment variable
 * #define SHMEM_BCAST_LEADERS     1024
 */
#define SHMEM_BCAST_METADATA (sizeof(addrint_t) + 2 * sizeof(int))
/* METADATA: buffer address, offset, num_bytes */

extern int mvp_num_intra_node_comm_levels;
extern int mvp_shmem_coll_num_procs;

extern int mvp_topo_aware_allreduce_ppn_threshold;
extern int finalize_coll_comm;

/* Use for collective tuning based on arch detection*/
int MVP_collectives_arch_init(int heterogeneity,
                              struct coll_info *collective_tuning_info);
void MVP_collectives_arch_finalize();

/* Use for allgather_osu.c */
#define MVP_ALLGATHER_SMALL_SYSTEM_SIZE  128
#define MVP_ALLGATHER_MEDIUM_SYSTEM_SIZE 256
#define MVP_ALLGATHER_LARGE_SYSTEM_SIZE  512

/* Use for scatter_osu.c*/
extern int mvp_size_mvp_scatter_mvp_tuning_table;
extern struct scatter_tuning mvp_scatter_mvp_tuning_table[4];


/* Use inside bcast_osu.c */
typedef struct bcast_ring_allgather_shm_packet {
    int j;
    int jnext;
    int root;
    intptr_t nbytes;
    intptr_t scatter_size;
} bcast_ring_allgather_shm_packet;

#define MVP_MAX_NB_THRESHOLDS 32

enum mvp_alltoall_coll_funcs {
    MVP_BRUCK_ALLTOALL,
    MVP_RD_ALLTOALL,
    MVP_SCATTER_DESTINATION_ALLTOALL,
    MVP_PAIRWISE_ALLTOALL,
    MVP_INPLACE_ALLTOALL,
    MVP_MAX_NUM_ALLTOALL_FUNCS
};


/* utils */
int mvp_increment_shmem_coll_counter(MPIR_Comm *comm_ptr);
int mvp_increment_allgather_coll_counter(MPIR_Comm *comm_ptr);

extern int mvp_bcast_large_msg;

/*Fn pointers for collectives */
typedef int (*MVP_Reduce_fn_t)(const void *sendbuf, void *recvbuf, int count,
                               MPI_Datatype datatype, MPI_Op op, int root,
                               MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);
extern MVP_Reduce_fn_t reduce_fn;

#ifdef _ENABLE_CUDA_
int device_stage_alloc(void **, int, void **, int, int, int, int);
void device_stage_free(void **, void **, int, int, int);
void DEVICE_COLL_Finalize();
void device_coll_pack(void **, int *, MPI_Datatype *, void **, int *,
                      MPI_Datatype *, int, int, int);
void device_coll_unpack(int *, int);
#endif /*_ENABLE_CUDA_*/

#define MVP_SHM_ALIGN (128)

#define MVP_SHM_ALIGN_LEN(len, align_unit)                                     \
    {                                                                          \
        len = ((int)(((len) + align_unit - 1) / align_unit)) * align_unit;     \
    }
#define IS_SHMEM_WINDOW_FULL(start, end)                                       \
    ((((int)(start) - (end)) >= MVP_SHMEM_COLL_WINDOW_SIZE - 1) ? 1 : 0)

#define IS_SHMEM_WINDOW_HALF_FULL(start, end)                                  \
    ((((int)(start) - (end)) >= MVP_SHMEM_COLL_WINDOW_SIZE / 2) ? 1 : 0)

int IS_SHMEM_WINDOW_REDUCE_HALF_FULL(int start, int end);

#if defined(CHANNEL_MRAIL_GEN2) || defined(CHANNEL_NEMESIS_IB)
#define MAX_NUM_HCAS (10)
typedef struct shm_coll_pkt {
    int peer_rank;
    int recv_id;
    uint32_t key[MAX_NUM_HCAS];
    uint64_t addr[MAX_NUM_HCAS];
} shm_coll_pkt;
#endif /* defined(CHANNEL_MRAIL_GEN2) || defined(CHANNEL_NEMESIS_IB) */

typedef struct shm_slot_t {
    volatile uint32_t psn __attribute__((aligned(MVP_SHM_ALIGN)));
    volatile uint32_t *tail_psn __attribute__((aligned(MVP_SHM_ALIGN)));
    char buf[] __attribute__((aligned(MVP_SHM_ALIGN)));
} shm_slot_t;

typedef struct shm_slot_cntrl_t {
    volatile uint32_t psn __attribute__((aligned(MVP_SHM_ALIGN)));
    volatile uint32_t *tail_psn __attribute__((aligned(MVP_SHM_ALIGN)));
} shm_slot_cntrl_t;

typedef struct shm_queue_t {
    shm_slot_t **shm_slots;
} shm_queue_t;

typedef struct shm_info_t {
    char *buffer;
    char *file_name;
    int local_rank;
    int local_size;
    int file_fd;
    int size;
    int count;
    unsigned long long int write;
    unsigned long long int read;
    int tail;
    shm_queue_t *queue;
    MPI_Comm comm;
    int max_local_size;
#if defined(CHANNEL_MRAIL_GEN2) || defined(CHANNEL_NEMESIS_IB)
    int buffer_registered;
    /* zcpy bcast */
    int bcast_exchange_rdma_keys;
    int bcast_knomial_factor;
    int bcast_expected_send_count;
    shm_coll_pkt *bcast_remote_handle_info_parent;
    shm_coll_pkt *bcast_remote_handle_info_children;
    /* zcpy reduce */
    int reduce_exchange_rdma_keys;
    int reduce_knomial_factor;
    int reduce_expected_recv_count;
    int reduce_expected_send_count;
    shm_coll_pkt *reduce_remote_handle_info_parent;
    shm_coll_pkt *reduce_remote_handle_info_children;
    int *inter_node_reduce_status_array;
    /* request info */
    int mid_request_active;
    int end_request_active;
    MPI_Request mid_request;
    MPI_Request end_request;
    int half_full_complete;
    struct ibv_mr *mem_handle[MAX_NUM_HCAS]; /* mem hndl for entire region */
#ifdef CKPT
    struct shm_info_t *next;
    struct shm_info_t *prev;
#endif /* CKPT */
    /* Number of sends in zcopy broadcast/reduce for which no completion event
     * has been raised */
    int zcpy_coll_pending_send_ops;
    /* Flag to check if deferred freeing of shmem_info is needed. An example
     * where this is useful is when the communicator and associated shared
     * memory regions are freed before zcopy bcast/reduce related sends are
     * complete.*/
    int defer_free;
#endif /* defined(CHANNEL_MRAIL_GEN2) || defined(CHANNEL_NEMESIS_IB) */
} shmem_info_t;

#if defined(CHANNEL_MRAIL_GEN2) || defined(_OSU_COLLECTIVES_)
static inline int Cyclic_Rank_list_mapper(MPIR_Comm *comm_ptr, int idx)
{
    return comm_ptr->dev.ch.rank_list[idx];
};

static inline int Bunch_Rank_list_mapper(MPIR_Comm *comm_ptr, int idx)
{
    return idx;
};
#endif /* defined(CHANNEL_MRAIL_GEN2) || defined(_MVP_CH4_OVERRIDE_) */

/* SHEMEM_COLL functions */
/* TODO: most of these are not MPIDI_CH3I_ functions and should be moved to the
 * MPIR_MVP_ or MPID_ namespace as they are device independent or device level.
 *
 * For the ones that are channel dependent they need to be moved to the
 * ch3_shmem_coll.h head with the appropriate abstraction
 *
 * TODO: Update: MPIDI_PG_t is not defined for CH4. These will need to be
 *       abstracted to CH3 as well as an MPIR level to implement the
 *       functionality in a device independent way
 */

int MPIR_MVP_SHMEM_COLL_init(int local_id);
int MPIR_MVP_SHMEM_COLL_Mmap(int local_id);
int MPIR_MVP_SHMEM_COLL_finalize(int local_id, int num_local_nodes);
void MPIR_MVP_SHMEM_COLL_Unlink(void);
void MPIR_MVP_SHMEM_COLL_Barrier_gather(int, int, int);
void MPIR_MVP_SHMEM_COLL_Barrier_bcast(int, int, int);
void MPIR_MVP_SHMEM_COLL_GetShmemBuf(int size, int rank, int shmem_comm_rank,
                                     void **output_buf);
void MPIR_MVP_SHMEM_COLL_SetGatherComplete(int size, int rank,
                                           int shmem_comm_rank);
void MPIR_MVP_SHMEM_COLL_GetShmemBuf(int, int, int, void **);
void MPIR_MVP_SHMEM_COLL_SetGatherComplete(int, int, int);
void MPIR_MVP_SHMEM_COLL_GetShmemBuf_optrels(int, int, int, int, void **);
void MPIR_MVP_SHMEM_COLL_SetGatherComplete_optrels(int, int, int, int);
void MPIR_MVP_SHMEM_TREE_COLL_GetShmemBuf_optrels(int, int, int, int, void **,
                                                  int);
void MPIR_MVP_SHMEM_TREE_COLL_SetGatherComplete_optrels(int, int, int, int,
                                                        int);
void MPIR_MVP_SHMEM_Bcast_GetBuf(int, int, int, void **);
void MPIR_MVP_SHMEM_Bcast_Complete(int, int, int);
void MPIR_MVP_SHMEM_Coll_Block_Clear_Status(int block_id);
int MPIR_MVP_SHMEM_Coll_get_free_block();
/* TODO - the below function should ideally be in a header for SMP collective
 * operations.
 * This is relevant when multiple intra-node channels exist. Leaving this here
 * for now*/
int MPIDI_CH3I_SMP_COLL_init();
void MVP_Read_env_vars(void);

int create_allgather_comm(MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);
shmem_info_t *mvp_shm_coll_init(int id, int local_rank, int local_size,
                                MPIR_Comm *comm_ptr);
void mvp_shm_coll_cleanup(shmem_info_t *shmem);
void mvp_shm_barrier_gather(shmem_info_t *shmem);
void mvp_shm_barrier_bcast(shmem_info_t *shmem);
void mvp_shm_barrier(shmem_info_t *shmem);
int mvp_shm_bcast(shmem_info_t *shmem, char *buf, int len, int root);
void mvp_shm_reduce(shmem_info_t *shmem, char *buf, int len, int count,
                    int root, MPI_User_function *uop, MPI_Datatype datatype,
                    int is_cxx_uop);
void mvp_shm_tree_reduce(shmem_info_t *shmem, char *in_buf, int len, int count,
                         int root, MPI_User_function *uop,
                         MPI_Datatype datatype, int is_cxx_uop);

#if defined(CHANNEL_MRAIL_GEN2) || defined(CHANNEL_NEMESIS_IB)
int mvp_shm_coll_reg_buffer(void *buffer, int size, struct ibv_mr *mem_handle[],
                            int *buffer_registered);
int mvp_shm_coll_dereg_buffer(struct ibv_mr *mem_handle[]);
void mvp_shm_coll_prepare_post_send(void *zcpy_coll_shmem_info,
                                    uint64_t local_rdma_addr,
                                    uint64_t remote_rdma_addr,
                                    uint32_t local_rdma_key,
                                    uint32_t remote_rdma_key, int len, int rail,
                                    MPIDI_VC_t *vc);
int mvp_shm_zcpy_bcast(shmem_info_t *shmem, char *buf, int len, int root,
                       int src, int expected_recv_count, int *dst_array,
                       int expected_send_count, int knomial_degree,
                       MPIR_Comm *comm_ptr);
int mvp_shm_zcpy_reduce(shmem_info_t *shmem, void *in_buf, void **out_buf,
                        int count, int len, MPI_Datatype datatype, MPI_Op op,
                        int root, int expected_recv_count, int *src_array,
                        int expected_send_count, int dst, int knomial_degree,
                        MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);
int MPIR_MVP_SHMEM_Helper_fn(int local_id, char **filename, char *prefix,
                             int *fd, size_t file_size);
#endif /* defined(CHANNEL_MRAIL_GEN2) || defined(CHANNEL_NEMESIS_IB) */

/* Use inside allreduce_osu.c */
/* Use inside reduce_osu.c*/
extern int MPIR_Reduce_two_level_helper_MVP(const void *sendbuf, void *recvbuf,
                                            int count, MPI_Datatype datatype,
                                            MPI_Op op, int root,
                                            MPIR_Comm *comm_ptr,
                                            MPIR_Errflag_t *errflag);
extern int MPIR_Reduce_redscat_gather_MVP(const void *sendbuf, void *recvbuf,
                                          int count, MPI_Datatype datatype,
                                          MPI_Op op, int root,
                                          MPIR_Comm *comm_ptr,
                                          MPIR_Errflag_t *errflag);
extern int MPIR_Reduce_binomial_MVP(const void *sendbuf, void *recvbuf,
                                    int count, MPI_Datatype datatype, MPI_Op op,
                                    int root, MPIR_Comm *comm_ptr,
                                    MPIR_Errflag_t *errflag);
extern int MPIR_Reduce_allreduce_MVP(const void *sendbuf, void *recvbuf,
                                     int count, MPI_Datatype datatype,
                                     MPI_Op op, int root, MPIR_Comm *comm_ptr,
                                     MPIR_Errflag_t *errflag);

/* Lock/unlock shmem region */
void lock_shmem_region(void);
void unlock_shmem_region(void);

/* utils */
int mvp_increment_shmem_coll_counter(MPIR_Comm *comm_ptr);
int mvp_increment_allgather_coll_counter(MPIR_Comm *comm_ptr);
void increment_mvp_shmem_comm_count(void);
int get_mvp_shmem_comm_count(void);
int init_thread_reg(void);
int MPIR_MVP_SMP_COLL_init(void);

/* Comm functions*/
extern int split_comm;
int check_split_comm(pthread_t);
int disable_split_comm(pthread_t);
int create_2level_comm(MPI_Comm, int, int);
int free_2level_comm(MPIR_Comm *);
int enable_split_comm(pthread_t);
void MPIR_pof2_comm(MPIR_Comm *, int, int);
#if defined(_MCST_SUPPORT_)
int create_mcast_comm(MPI_Comm, int, int);
#endif /*defined(_MCST_SUPPORT_)*/
#if defined(_SHARP_SUPPORT_)
int create_sharp_comm(MPI_Comm, int, int);
#endif /*defined (_SHARP_SUPPORT_)*/

#endif /* _MVP_COLL_SHMEM_ */
