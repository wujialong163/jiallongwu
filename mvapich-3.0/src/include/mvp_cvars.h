/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

/* Automatically generated
 *   by:   ./maint/extractmvpcvars
 *   on:   Fri Feb 16 17:33:28 2024 UTC
 *
 * DO NOT EDIT!!!
 */

#if !defined(MVP_CVARS_H_INCLUDED)
#define MVP_CVARS_H_INCLUDED

#include "mpitimpl.h" /* for MPIR_T_cvar_range_value_t */

/* Initializes cvar values from the environment */
int MPIR_T_MVP_cvar_init(void);
int MPIR_T_MVP_cvar_finalize(void);

/* Extern declarations for each cvar
 * (definitions in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/util/mvp_cvars.c) */

/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/allgather/allgather_osu.c */
extern int MVP_ALLGATHER_BRUCK_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/allgather/allgather_osu.c */
extern int MVP_ALLGATHER_RD_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/allgather/allgather_osu.c */
extern int MVP_ALLGATHER_REVERSE_RANKING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/allgather/allgather_osu.c */
extern int MVP_ALLGATHER_COLLECTIVE_ALGORITHM;
enum ALLGATHER_COLLECTIVE_ALGORITHM_choice {
    MVP_ALLGATHER_COLLECTIVE_ALGORITHM_UNSET,
    MVP_ALLGATHER_COLLECTIVE_ALGORITHM_RD_ALLGATHER_COMM,
    MVP_ALLGATHER_COLLECTIVE_ALGORITHM_RD,
    MVP_ALLGATHER_COLLECTIVE_ALGORITHM_BRUCK,
    MVP_ALLGATHER_COLLECTIVE_ALGORITHM_RING,
    MVP_ALLGATHER_COLLECTIVE_ALGORITHM_DIRECT,
    MVP_ALLGATHER_COLLECTIVE_ALGORITHM_DIRECTSPREAD,
    MVP_ALLGATHER_COLLECTIVE_ALGORITHM_GATHER_BCAST,
    MVP_ALLGATHER_COLLECTIVE_ALGORITHM_2LVL_NONBLOCKED,
    MVP_ALLGATHER_COLLECTIVE_ALGORITHM_2LVL_RING_NONBLOCKED,
    MVP_ALLGATHER_COLLECTIVE_ALGORITHM_2LVL_DIRECT,
    MVP_ALLGATHER_COLLECTIVE_ALGORITHM_2LVL_RING
};
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/allgather/allgather_osu.c */
extern int MVP_ALLGATHER_TUNING_IS_TWO_LEVEL;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/allgatherv/allgatherv_osu.c */
extern int MVP_ALLGATHERV_COLLECTIVE_ALGORITHM;
enum ALLGATHERV_COLLECTIVE_ALGORITHM_choice {
    MVP_ALLGATHERV_COLLECTIVE_ALGORITHM_UNSET,
    MVP_ALLGATHERV_COLLECTIVE_ALGORITHM_RD,
    MVP_ALLGATHERV_COLLECTIVE_ALGORITHM_BRUCK,
    MVP_ALLGATHERV_COLLECTIVE_ALGORITHM_RING,
    MVP_ALLGATHERV_COLLECTIVE_ALGORITHM_RING_CYCLIC
};
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/allreduce/allreduce_osu.c */
extern int MVP_ALLREDUCE_2LEVEL_MSG;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/allreduce/allreduce_osu.c */
extern int MVP_ALLREDUCE_SHORT_MSG;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/allreduce/allreduce_osu.c */
extern int MVP_SHMEM_ALLREDUCE_MSG;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/allreduce/allreduce_osu.c */
extern int MVP_USE_SHMEM_ALLREDUCE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/allreduce/allreduce_osu.c */
extern int MVP_USE_SOCKET_AWARE_ALLREDUCE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/allreduce/allreduce_osu.c */
extern int MVP_USE_SOCKET_AWARE_SHARP_ALLREDUCE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/allreduce/allreduce_osu.c */
extern int MVP_SOCKET_AWARE_ALLREDUCE_MAX_MSG;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/allreduce/allreduce_osu.c */
extern int MVP_SOCKET_AWARE_ALLREDUCE_MIN_MSG;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/allreduce/allreduce_osu.c */
extern int MVP_ALLREDUCE_COLLECTIVE_ALGORITHM;
enum ALLREDUCE_COLLECTIVE_ALGORITHM_choice {
    MVP_ALLREDUCE_COLLECTIVE_ALGORITHM_UNSET
};
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/allreduce/allreduce_osu.c */
extern int MVP_ALLREDUCE_INTER_NODE_TUNING_ALGO;
enum ALLREDUCE_INTER_NODE_TUNING_ALGO_choice {
    MVP_ALLREDUCE_INTER_NODE_TUNING_ALGO_UNSET,
    MVP_ALLREDUCE_INTER_NODE_TUNING_ALGO_P2P_RD,
    MVP_ALLREDUCE_INTER_NODE_TUNING_ALGO_P2P_RSA,
    MVP_ALLREDUCE_INTER_NODE_TUNING_ALGO_MCAST_2LVL,
    MVP_ALLREDUCE_INTER_NODE_TUNING_ALGO_MCAST_RSA,
    MVP_ALLREDUCE_INTER_NODE_TUNING_ALGO_RSA,
    MVP_ALLREDUCE_INTER_NODE_TUNING_ALGO_RING,
    MVP_ALLREDUCE_INTER_NODE_TUNING_ALGO_SOCK_AWARE
};
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/allreduce/allreduce_osu.c */
extern int MVP_ALLREDUCE_INTRA_NODE_TUNING_ALGO;
enum ALLREDUCE_INTRA_NODE_TUNING_ALGO_choice {
    MVP_ALLREDUCE_INTRA_NODE_TUNING_ALGO_UNSET,
    MVP_ALLREDUCE_INTRA_NODE_TUNING_ALGO_P2P,
    MVP_ALLREDUCE_INTRA_NODE_TUNING_ALGO_SHMEM,
    MVP_ALLREDUCE_INTRA_NODE_TUNING_ALGO_P2P_RD,
    MVP_ALLREDUCE_INTRA_NODE_TUNING_ALGO_P2P_RSA
};
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/allreduce/allreduce_osu.c */
extern int MVP_ALLREDUCE_TUNING_IS_TWO_LEVEL;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/alltoall/alltoall_osu.c */
extern int MVP_ALLTOALL_MEDIUM_MSG;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/alltoall/alltoall_osu.c */
extern int MVP_ALLTOALL_SMALL_MSG;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/alltoall/alltoall_osu.c */
extern int MVP_ALLTOALL_THROTTLE_FACTOR;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/alltoall/alltoall_osu.c */
extern int MVP_ALLTOALL_INTRA_THROTTLE_FACTOR;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/alltoall/alltoall_osu.c */
extern int MVP_ALLTOALL_LARGE_MSG_THROTTLE_FACTOR;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/alltoall/alltoall_osu.c */
extern int MVP_USE_XOR_ALLTOALL;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/alltoall/alltoall_osu.c */
extern int MVP_ALLTOALL_COLLECTIVE_ALGORITHM;
enum ALLTOALL_COLLECTIVE_ALGORITHM_choice {
    MVP_ALLTOALL_COLLECTIVE_ALGORITHM_UNSET,
    MVP_ALLTOALL_COLLECTIVE_ALGORITHM_BRUCK,
    MVP_ALLTOALL_COLLECTIVE_ALGORITHM_RD,
    MVP_ALLTOALL_COLLECTIVE_ALGORITHM_SCATTER_DEST,
    MVP_ALLTOALL_COLLECTIVE_ALGORITHM_PAIRWISE,
    MVP_ALLTOALL_COLLECTIVE_ALGORITHM_INPLACE
};
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/alltoallv/alltoallv_osu.c */
extern int MVP_ALLTOALLV_COLLECTIVE_ALGORITHM;
enum ALLTOALLV_COLLECTIVE_ALGORITHM_choice {
    MVP_ALLTOALLV_COLLECTIVE_ALGORITHM_UNSET,
    MVP_ALLTOALLV_COLLECTIVE_ALGORITHM_SCATTER,
    MVP_ALLTOALLV_COLLECTIVE_ALGORITHM_INTRA
};
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/barrier/barrier_osu.c */
extern int MVP_USE_SHMEM_BARRIER;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/barrier/barrier_osu.c */
extern int MVP_USE_SOCKET_AWARE_BARRIER;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/bcast/bcast_osu.c */
extern int MVP_BCAST_TWO_LEVEL_SYSTEM_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/bcast/bcast_osu.c */
extern int MVP_KNOMIAL_2LEVEL_BCAST_MESSAGE_SIZE_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/bcast/bcast_osu.c */
extern int MVP_KNOMIAL_2LEVEL_BCAST_SYSTEM_SIZE_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/bcast/bcast_osu.c */
extern int MVP_BCAST_SHORT_MSG;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/bcast/bcast_osu.c */
extern int MVP_USE_KNOMIAL_2LEVEL_BCAST;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/bcast/bcast_osu.c */
extern int MVP_USE_KNOMIAL_INTER_LEADER_BCAST;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/bcast/bcast_osu.c */
extern int MVP_USE_SHMEM_BCAST;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/bcast/bcast_osu.c */
extern int MVP_BCAST_COLLECTIVE_ALGORITHM;
enum BCAST_COLLECTIVE_ALGORITHM_choice {
    MVP_BCAST_COLLECTIVE_ALGORITHM_UNSET,
    MVP_BCAST_COLLECTIVE_ALGORITHM_KNOMIAL
};
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/bcast/bcast_osu.c */
extern int MVP_BCAST_INTER_NODE_TUNING_ALGO;
enum BCAST_INTER_NODE_TUNING_ALGO_choice {
    MVP_BCAST_INTER_NODE_TUNING_ALGO_UNSET,
    MVP_BCAST_INTER_NODE_TUNING_ALGO_BINOMIAL,
    MVP_BCAST_INTER_NODE_TUNING_ALGO_SCATTER_DOUBLING_ALLGATHER,
    MVP_BCAST_INTER_NODE_TUNING_ALGO_SCATTER_RING_ALLGATHER,
    MVP_BCAST_INTER_NODE_TUNING_ALGO_SCATTER_RING_ALLGATHER_SHM,
    MVP_BCAST_INTER_NODE_TUNING_ALGO_KNOMIAL,
    MVP_BCAST_INTER_NODE_TUNING_ALGO_PIPELINED
};
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/bcast/bcast_osu.c */
extern int MVP_BCAST_INTRA_NODE_TUNING_ALGO;
enum BCAST_INTRA_NODE_TUNING_ALGO_choice {
    MVP_BCAST_INTRA_NODE_TUNING_ALGO_UNSET,
    MVP_BCAST_INTRA_NODE_TUNING_ALGO_SHMEM,
    MVP_BCAST_INTRA_NODE_TUNING_ALGO_KNOMIAL
};
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/bcast/bcast_osu.c */
extern int MVP_BCAST_TUNING_IS_TWO_LEVEL;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/gather/gather_osu.c */
extern int MVP_GATHER_SWITCH_PT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/gather/gather_osu.c */
extern int MVP_USE_DIRECT_GATHER;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/gather/gather_osu.c */
extern int MVP_USE_DIRECT_GATHER_SYSTEM_SIZE_MEDIUM;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/gather/gather_osu.c */
extern int MVP_USE_DIRECT_GATHER_SYSTEM_SIZE_SMALL;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/gather/gather_osu.c */
extern int MVP_USE_TWO_LEVEL_GATHER;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/gather/gather_osu.c */
extern int MVP_GATHER_COLLECTIVE_ALGORITHM;
enum GATHER_COLLECTIVE_ALGORITHM_choice {
    MVP_GATHER_COLLECTIVE_ALGORITHM_UNSET,
    MVP_GATHER_COLLECTIVE_ALGORITHM_BINOMIAL,
    MVP_GATHER_COLLECTIVE_ALGORITHM_DIRECT
};
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/gather/gather_osu.c */
extern int MVP_GATHER_INTER_NODE_TUNING_ALGO;
enum GATHER_INTER_NODE_TUNING_ALGO_choice {
    MVP_GATHER_INTER_NODE_TUNING_ALGO_UNSET,
    MVP_GATHER_INTER_NODE_TUNING_ALGO_BINOMIAL,
    MVP_GATHER_INTER_NODE_TUNING_ALGO_DIRECT,
    MVP_GATHER_INTER_NODE_TUNING_ALGO_2LVL_DIRECT
};
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/gather/gather_osu.c */
extern int MVP_GATHER_INTRA_NODE_TUNING_ALGO;
enum GATHER_INTRA_NODE_TUNING_ALGO_choice {
    MVP_GATHER_INTRA_NODE_TUNING_ALGO_UNSET,
    MVP_GATHER_INTRA_NODE_TUNING_ALGO_BINOMIAL,
    MVP_GATHER_INTRA_NODE_TUNING_ALGO_DIRECT
};
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/offload/sharp/mvp_sharp.c */
extern int MVP_ENABLE_SHARP;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/offload/sharp/mvp_sharp.c */
extern int MVP_SHARP_MAX_MSG_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/offload/sharp/mvp_sharp.c */
extern int MVP_SHARP_PORT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/offload/sharp/mvp_sharp.c */
extern const char * MVP_SHARP_HCA_NAME;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/offload/sharp/mvp_sharp.c */
extern int MVP_SHARP_MIN_NODE_COUNT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/offload/sharp/mvp_sharp.c */
extern int MVP_ENABLE_SHARP_ALLREDUCE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/offload/sharp/mvp_sharp.c */
extern int MVP_ENABLE_SHARP_BARRIER;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/offload/sharp/mvp_sharp.c */
extern int MVP_ENABLE_SHARP_BCAST;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/offload/sharp/mvp_sharp.c */
extern int MVP_ENABLE_SHARP_REDUCE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/offload/sharp/mvp_sharp.c */
extern int MVP_ENABLE_SHARP_SCATTER;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/offload/sharp/mvp_sharp.c */
extern int MVP_ENABLE_SHARP_SCATTERV;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/offload/sharp/mvp_sharp.c */
extern int MVP_ENABLE_SHARP_IALLREDUCE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/offload/sharp/mvp_sharp.c */
extern int MVP_ENABLE_SHARP_IREDUCE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/offload/sharp/mvp_sharp.c */
extern int MVP_ENABLE_SHARP_IBCAST;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/offload/sharp/mvp_sharp.c */
extern int MVP_ENABLE_SHARP_IBARRIER;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/reduce/reduce_osu.c */
extern int MVP_INTRA_SHMEM_REDUCE_MSG;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/reduce/reduce_osu.c */
extern int MVP_REDUCE_2LEVEL_MSG;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/reduce/reduce_osu.c */
extern int MVP_REDUCE_SHORT_MSG;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/reduce/reduce_osu.c */
extern int MVP_SHMEM_REDUCE_MSG;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/reduce/reduce_osu.c */
extern int MVP_USE_SHMEM_REDUCE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/reduce/reduce_osu.c */
extern int MVP_REDUCE_COLLECTIVE_ALGORITHM;
enum REDUCE_COLLECTIVE_ALGORITHM_choice {
    MVP_REDUCE_COLLECTIVE_ALGORITHM_UNSET,
    MVP_REDUCE_COLLECTIVE_ALGORITHM_BINOMIAL,
    MVP_REDUCE_COLLECTIVE_ALGORITHM_KNOMIAL
};
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/reduce/reduce_osu.c */
extern int MVP_REDUCE_INTER_NODE_TUNING_ALGO;
enum REDUCE_INTER_NODE_TUNING_ALGO_choice {
    MVP_REDUCE_INTER_NODE_TUNING_ALGO_UNSET,
    MVP_REDUCE_INTER_NODE_TUNING_ALGO_BINOMIAL,
    MVP_REDUCE_INTER_NODE_TUNING_ALGO_KNOMIAL,
    MVP_REDUCE_INTER_NODE_TUNING_ALGO_RED_SCAT_GATHER,
    MVP_REDUCE_INTER_NODE_TUNING_ALGO_ALLREDUCE
};
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/reduce/reduce_osu.c */
extern int MVP_REDUCE_INTRA_NODE_TUNING_ALGO;
enum REDUCE_INTRA_NODE_TUNING_ALGO_choice {
    MVP_REDUCE_INTRA_NODE_TUNING_ALGO_UNSET,
    MVP_REDUCE_INTRA_NODE_TUNING_ALGO_KNOMIAL,
    MVP_REDUCE_INTRA_NODE_TUNING_ALGO_SHMEM,
    MVP_REDUCE_INTRA_NODE_TUNING_ALGO_BINOMIAL
};
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/reduce/reduce_osu.c */
extern int MVP_REDUCE_TUNING_IS_TWO_LEVEL;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/scatter/scatter_osu.c */
extern int MVP_RED_SCAT_LARGE_MSG;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/scatter/scatter_osu.c */
extern int MVP_RED_SCAT_SHORT_MSG;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/scatter/scatter_osu.c */
extern int MVP_SCATTER_MEDIUM_MSG;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/scatter/scatter_osu.c */
extern int MVP_SCATTER_SMALL_MSG;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/scatter/scatter_osu.c */
extern int MVP_USE_DIRECT_SCATTER;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/scatter/scatter_osu.c */
extern int MVP_USE_SCATTER_RD_INTER_LEADER_BCAST;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/scatter/scatter_osu.c */
extern int MVP_USE_SCATTER_RING_INTER_LEADER_BCAST;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/scatter/scatter_osu.c */
extern int MVP_USE_TWO_LEVEL_SCATTER;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/scatter/scatter_osu.c */
extern int MVP_SCATTER_COLLECTIVE_ALGORITHM;
enum SCATTER_COLLECTIVE_ALGORITHM_choice {
    MVP_SCATTER_COLLECTIVE_ALGORITHM_UNSET,
    MVP_SCATTER_COLLECTIVE_ALGORITHM_BINOMIAL,
    MVP_SCATTER_COLLECTIVE_ALGORITHM_DIRECT
};
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/scatter/scatter_osu.c */
extern int MVP_SCATTER_INTER_NODE_TUNING_ALGO;
enum SCATTER_INTER_NODE_TUNING_ALGO_choice {
    MVP_SCATTER_INTER_NODE_TUNING_ALGO_UNSET,
    MVP_SCATTER_INTER_NODE_TUNING_ALGO_BINOMIAL,
    MVP_SCATTER_INTER_NODE_TUNING_ALGO_DIRECT,
    MVP_SCATTER_INTER_NODE_TUNING_ALGO_2LVL_BINOMIAL,
    MVP_SCATTER_INTER_NODE_TUNING_ALGO_2LVL_DIRECT,
    MVP_SCATTER_INTER_NODE_TUNING_ALGO_MCAST
};
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/scatter/scatter_osu.c */
extern int MVP_SCATTER_INTRA_NODE_TUNING_ALGO;
enum SCATTER_INTRA_NODE_TUNING_ALGO_choice {
    MVP_SCATTER_INTRA_NODE_TUNING_ALGO_UNSET,
    MVP_SCATTER_INTRA_NODE_TUNING_ALGO_DIRECT,
    MVP_SCATTER_INTRA_NODE_TUNING_ALGO_BINOMIAL
};
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern const char * MVP_SHMEM_DIR;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_KNOMIAL_INTER_LEADER_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_KNOMIAL_INTER_NODE_FACTOR;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_KNOMIAL_INTRA_NODE_FACTOR;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_KNOMIAL_INTRA_NODE_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_SHMEM_COLL_MAX_MSG_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_SHMEM_COLL_NUM_COMM;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_SHMEM_COLL_NUM_PROCS;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_SHMEM_COLL_SPIN_COUNT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_USE_OSU_COLLECTIVES;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_USE_OSU_NB_COLLECTIVES;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_USE_SHMEM_COLL;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_ENABLE_SOCKET_AWARE_COLLECTIVES;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_USE_BLOCKING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_USE_SHARED_MEM;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_ENABLE_TOPO_AWARE_COLLECTIVES;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_USE_TOPO_AWARE_ALLREDUCE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_USE_TOPO_AWARE_REDUCE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_USE_TOPO_AWARE_BCAST;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_USE_OPTIMIZED_RELEASE_ALLREDUCE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_USE_SHMEM_TREE_ENABLE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_USE_SHMEM_TREE_MIN_MESSAGE_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_USE_SHMEM_TREE_MAX_MESSAGE_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_USE_SHMEM_NUM_TREES;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_COLL_TMP_BUF_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_USE_TOPO_AWARE_BARRIER;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_TOPO_AWARE_ALLREDUCE_MAX_MSG;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_TOPO_AWARE_ALLREDUCE_MIN_MSG;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_TOPO_AWARE_REDUCE_MAX_MSG;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_TOPO_AWARE_REDUCE_MIN_MSG;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_TOPO_AWARE_BCAST_MAX_MSG;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_TOPO_AWARE_BCAST_MIN_MSG;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_TOPO_AWARE_REDUCE_PPN_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_TOPO_AWARE_BCAST_PPN_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_TOPO_AWARE_BCAST_NODE_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_TOPO_AWARE_REDUCE_NODE_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_GATHERV_SSEND_MIN_PROCS;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_USE_MCAST_ALLREDUCE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_MCAST_ALLREDUCE_SMALL_MSG_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_MCAST_ALLREDUCE_LARGE_MSG_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_USE_MCAST_SCATTER;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_MCAST_SCATTER_MSG_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_MCAST_SCATTER_SMALL_SYS_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_MCAST_SCATTER_LARGE_SYS_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_USE_BITONIC_COMM_SPLIT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_BITONIC_COMM_SPLIT_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_ENABLE_SKIP_TUNING_TABLE_SEARCH;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_COLL_SKIP_TABLE_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_USE_INDEXED_ALLGATHER_TUNING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_USE_INDEXED_ALLTOALL_TUNING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_USE_INDEXED_ALLTOALLV_TUNING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_IBCAST_ENABLE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_IGATHER_ENABLE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_ISCATTER_ENABLE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_IALLGATHER_ENABLE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_IREDUCE_ENABLE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_IALLREDUCE_ENABLE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_IREDUCE_SCATTER_ENABLE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_IALLTOALL_ENABLE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_IALLTOALLV_ENABLE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_IALLGATHERV_ENABLE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_IBARRIER_ENABLE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_SHMEM_COLL_MAX_NUM_EMPTY_PROGRESS_POLLS;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_USE_KNOMIAL_REDUCE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_REDUCE_INTER_KNOMIAL_FACTOR;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_REDUCE_ZCOPY_INTER_KNOMIAL_FACTOR;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_REDUCE_INTRA_KNOMIAL_FACTOR;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern const char * MVP_INTRA_MULTI_LEVEL_GATHER_TUNING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_USE_OLD_BCAST;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_ALLRED_USE_RING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_USE_OLD_ALLGATHER;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_USE_OLD_SCATTER;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_USE_OLD_ALLREDUCE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_USE_OLD_REDUCE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_USE_OLD_ALLTOALL;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_ALLTOALL_INPLACE_OLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_ALLTOALL_RD_MAX_MSG_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern const char * MVP_INTRA_IBCAST_TUNING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern const char * MVP_INTRA_IGATHER_TUNING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern const char * MVP_INTER_IBCAST_TUNING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern const char * MVP_INTER_IGATHER_TUNING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern const char * MVP_INTRA_ISCATTER_TUNING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern const char * MVP_INTER_ISCATTER_TUNING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern const char * MVP_INTRA_IALLREDUCE_TUNING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern const char * MVP_INTER_IALLREDUCE_TUNING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern const char * MVP_INTRA_IREDUCE_TUNING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern const char * MVP_INTER_IREDUCE_TUNING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern const char * MVP_INTRA_IREDUCE_SCATTER_TUNING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern const char * MVP_INTER_IREDUCE_SCATTER_TUNING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern const char * MVP_INTER_IALLGATHER_TUNING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern const char * MVP_INTRA_IALLGATHER_TUNING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern const char * MVP_INTRA_IALLTOALL_TUNING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern const char * MVP_INTER_IALLTOALL_TUNING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern const char * MVP_INTRA_IALLTOALLV_TUNING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern const char * MVP_INTER_IALLTOALLV_TUNING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern const char * MVP_INTER_IALLGATHERV_TUNING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern const char * MVP_INTRA_IALLGATHERV_TUNING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern const char * MVP_INTRA_IBARRIER_TUNING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern const char * MVP_INTER_IBARRIER_TUNING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern const char * MVP_INTER_RED_SCAT_TUNING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern const char * MVP_INTER_RED_SCAT_BLOCK_TUNING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern const char * MVP_INTER_ALLGATHERV_TUNING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_USE_SCATTER_DEST_ALLTOALLV;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_PIPELINED_ZCPY_BCAST_KNOMIAL_FACTOR;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_USE_ZCOPY_BCAST;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_USE_ZCOPY_REDUCE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_GATHERV_SSEND_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_ALLREDUCE_RING_ALGO_PPN_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_ALLREDUCE_RING_ALGO_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_ALLREDUCE_RED_SCAT_ALLGATHER_ALGO_PPN_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_ALLREDUCE_RED_SCAT_ALLGATHER_ALGO_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_ALLGATHER_RING_ALGO_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_ALLTOALLV_INTERMEDIATE_WAIT_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_ALLGATHER_CYCLIC_ALGO_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_ALLREDUCE_CYCLIC_ALGO_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_REDSCAT_CYCLIC_ALGO_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_RED_SCAT_RING_ALGO_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_BCAST_SCATTER_RING_OVERLAP;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_BCAST_SCATTER_RING_OVERLAP_MSG_UPPERBOUND;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_BCAST_SCATTER_RING_OVERLAP_CORES_LOWERBOUND;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_USE_PIPELINE_BCAST;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_BCAST_SEGMENT_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_USE_LIMIC_GATHER;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_USE_2LEVEL_ALLGATHER;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_SHMEM_COLL_WINDOW_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_SHMEM_REDUCE_TREE_DEGREE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_SHMEM_COLL_SLOT_LEN;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_USE_SLOT_SHMEM_COLL;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_USE_SLOT_SHMEM_BCAST;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_USE_MCAST_PIPELINE_SHM;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_TWO_LEVEL_COMM_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_TWO_LEVEL_COMM_EARLY_INIT_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_ENABLE_PVAR_TIMER;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_ENABLE_ALLREDUCE_ALL_COMPUTE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_GATHER_STATUS_ALIGNMENT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_BCAST_STATUS_ALIGNMENT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_ENABLE_ALLREDUCE_SKIP_LARGE_MESSAGE_TUNING_TABLE_SEARCH;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_ENABLE_ALLREDUCE_SKIP_SMALL_MESSAGE_TUNING_TABLE_SEARCH;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/coll/shmem/mvp_shmem_coll.c */
extern int MVP_SOCKET_AWARE_ALLREDUCE_PPN_THRESHOLD;

extern int MVP_PVAR_INFO_NAME;

/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/common/src/affinity/hwloc_bind.c */
extern const char * MVP_CPU_BINDING_LEVEL;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/common/src/affinity/hwloc_bind.c */
extern const char * MVP_CPU_MAPPING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/common/src/affinity/hwloc_bind.c */
extern int MVP_ENABLE_AFFINITY;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/common/src/affinity/hwloc_bind.c */
extern int MVP_ENABLE_LEASTLOAD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/common/src/cm/cm.c */
extern int MVP_CM_MAX_SPIN_COUNT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/common/src/cm/cm.c */
extern int MVP_CM_RECV_BUFFERS;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/common/src/cm/cm.c */
extern int MVP_CM_SEND_DEPTH;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/common/src/cm/cm.c */
extern int MVP_CM_TIMEOUT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/common/src/cm/cm.c */
extern int MVP_CM_UD_PSN;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/common/src/cm/cm.c */
extern int MVP_CM_THREAD_STACKSIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/common/src/detect/hca/mvp_hca_detect.c */
extern int MVP_FORCE_HCA_TYPE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/common/src/detect/hca/mvp_hca_detect.c */
extern int MVP_HCA_AWARE_PROCESS_MAPPING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/common/src/rdma_cm/rdma_cm.c */
extern int MVP_RDMA_CM_ARP_TIMEOUT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/common/src/rdma_cm/rdma_cm.c */
extern int MVP_RDMA_CM_PORT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/common/src/rdma_cm/rdma_cm.c */
extern int MVP_RDMA_CM_MAX_PORT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/common/src/rdma_cm/rdma_cm.c */
extern int MVP_RDMA_CM_MIN_PORT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_EAGER_CUDAHOST_REG;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_VECTOR_OPT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_KERNEL_OPT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_KERNEL_VECTOR_TIDBLK_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_KERNEL_VECTOR_YSIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_KERNEL_SUBARR_TIDBLK_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_KERNEL_SUBARR_XDIM;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_KERNEL_SUBARR_YDIM;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_KERNEL_SUBARR_ZDIM;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_NUM_EVENTS;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_INIT_CONTEXT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_USE_NAIVE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_ALLTOALL_DYNAMIC;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_REGISTER_NAIVE_BUF;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_GATHER_NAIVE_LIMIT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_SCATTER_NAIVE_LIMIT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_ALLTOALL_NAIVE_LIMIT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_ALLTOALLV_NAIVE_LIMIT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_ALLGATHER_NAIVE_LIMIT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_ALLGATHERV_NAIVE_LIMIT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_BCAST_NAIVE_LIMIT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_GATHERV_NAIVE_LIMIT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_SCATTERV_NAIVE_LIMIT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_ALLGATHER_RD_LIMIT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_ALLGATHER_FGP;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_IPC;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_IPC_SHARE_GPU;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_SMP_IPC;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_ENABLE_IPC_CACHE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_IPC_MAX_CACHE_ENTRIES;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_IPC_NUM_STAGE_BUFFERS;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_IPC_STAGE_BUF_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_IPC_BUFFERED;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_IPC_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_IPC_BUFFERED_LIMIT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_IPC_SYNC_LIMIT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_DYNAMIC_INIT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_cuda_util.c */
extern int MVP_CUDA_NONBLOCKING_STREAMS;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_SMP_EAGERSIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern const char * MVP_IBA_HCA;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_SMP_USE_CMA;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_SMP_USE_LIMIC2;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_SMP_POLLING_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_SMP_CMA_MAX_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_SMP_LIMIC2_MAX_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_SMP_QUEUE_LENGTH;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_LIMIC_PUT_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_LIMIC_GET_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_R3_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_INTRA_NODE_R3_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_INTER_NODE_R3_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_R3_NOCACHE_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_MAX_R3_PENDING_DATA;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_USE_LIMIC2_COLL;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_SMP_USE_MAX_SWITCH;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_CUDA_SMP_PIPELINE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_SMP_DELAY_SHMEM_POOL_INIT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_SMP_PRIORITY_POLLING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_SMP_NUM_SEND_BUFFER;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_SMP_BATCH_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_SMP_SEND_BUF_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_USE_OPT_EAGER_RECV;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_NUM_NODES_IN_JOB;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_USE_MCAST;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_USE_RDMA_CM_MCAST;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_USE_QOS;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_3DTORUS_SUPPORT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_PATH_SL_QUERY;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_NUM_SLS;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_NUM_SA_QUERY_RETRIES;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_MAX_RDMA_CONNECT_ATTEMPTS;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_RDMA_CM_CONNECT_RETRY_INTERVAL;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_USE_HSAM;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_USE_APM;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_USE_APM_TEST;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_USE_IWARP_MODE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_USE_RDMA_CM;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_RDMA_CM_MULTI_SUBNET_SUPPORT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_ON_DEMAND_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_USE_XRC;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_USE_SRQ;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_IWARP_MULTIPLE_CQ_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_USE_LAZY_MEM_UNREGISTER;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_USE_RMA_FAST_PATH;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_FORCE_IB_ATOMIC;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_USE_RDMA_ONE_SIDED;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_RNDV_EXT_SENDQ_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_RDMA_NUM_EXTRA_POLLS;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_COALESCE_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_USE_COALESCE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_SPIN_COUNT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern const char * MVP_PROCESS_TO_RAIL_MAPPING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern const char * MVP_SM_SCHEDULING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern const char * MVP_SMALL_MSG_RAIL_SHARING_POLICY;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern const char * MVP_MED_MSG_RAIL_SHARING_POLICY;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern const char * MVP_RAIL_SHARING_POLICY;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_SUPPRESS_JOB_STARTUP_PERFORMANCE_WARNING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_USE_RDMA_FAST_PATH;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_RDMA_FAST_PATH_BUF_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_POLLING_SET_LIMIT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_POLLING_SET_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_RDMA_EAGER_LIMIT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_NUM_RDMA_BUFFER;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_RDMA_FAST_PATH_PREALLOCATE_BUFFERS;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_SYSREPORT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern const char * MVP_DEFAULT_MTU;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_NUM_CQES_PER_POLL;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_NUM_PORTS;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_NUM_QP_PER_PORT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_PIN_POOL_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_DEFAULT_MAX_CQ_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_IBA_EAGER_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_STRIPING_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_RAIL_SHARING_MED_MSG_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_RAIL_SHARING_LARGE_MSG_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_DEFAULT_PUT_GET_LIST_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_EAGERSIZE_1SC;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_PUT_FALLBACK_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_GET_FALLBACK_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_DEFAULT_QP_OUS_RD_ATOM;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_DEFAULT_MAX_RDMA_DST_OPS;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_DEFAULT_PSN;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern const char * MVP_DEFAULT_PKEY;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern const char * MVP_DEFAULT_QKEY;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_DEFAULT_MIN_RNR_TIMER;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_DEFAULT_SERVICE_LEVEL;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_DEFAULT_TIME_OUT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_DEFAULT_STATIC_RATE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_DEFAULT_SRC_PATH_BITS;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_DEFAULT_RETRY_COUNT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_DEFAULT_RNR_RETRY;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_DEFAULT_MAX_SG_LIST;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_DEFAULT_MAX_SEND_WQE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_CM_WAIT_TIME;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_USE_UD_HYBRID;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_UD_MTU;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_HYBRID_MAX_RC_CONN;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_UD_NUM_MSG_LIMIT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_UD_SENDWINDOW_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_UD_RECVWINDOW_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_UD_RETRY_TIMEOUT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_UD_MAX_RETRY_TIMEOUT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_UD_PROGRESS_SPIN;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_UD_RETRY_COUNT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_UD_PROGRESS_TIMEOUT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_UD_MAX_SEND_WQE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_UD_MAX_RECV_WQE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_USE_UD_SRQ;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_UD_VBUF_POOL_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_UD_MAX_ACK_PENDING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_SHMEM_BACKED_UD_CM;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_USE_UD_ZCOPY;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_UD_ZCOPY_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_UD_NUM_ZCOPY_RNDV_QPS;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_UD_ZCOPY_RQ_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_UD_ZCOPY_NUM_RETRY;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_UD_ZCOPY_ENABLE_POLLING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_UD_ZCOPY_PUSH_SEGMENT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_UD_DROP_PACKET_RATE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_MCAST_ENABLE_REL;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_MCAST_USE_MCAST_NACK;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_MCAST_NUM_NODES_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_MCAST_MAX_RECV_WQE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_MCAST_WINDOW_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_MCAST_DROP_PACKET_RATE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_MCAST_RETRY_TIMEOUT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_MCAST_MAX_RETRY_TIMEOUT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_MCAST_NSPIN_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_MCAST_COMM_INIT_TIMEOUT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_MCAST_COMM_INIT_RETRIES;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_MCAST_SKIP_LOOPBACK;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_MCAST_BCAST_MIN_MSG;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_MCAST_BCAST_MAX_MSG;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_DEFAULT_MAX_RECV_WQE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_NDREG_ENTRIES_MAX;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_NDREG_ENTRIES;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_DREG_CACHE_LIMIT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_USE_HWLOC_CPU_BINDING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_THREAD_YIELD_SPIN_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_USE_THREAD_YIELD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_NUM_SPINS_BEFORE_LOCK;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_ASYNC_THREAD_STACK_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_USE_HUGEPAGES;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_MEMORY_OPTIMIZATION;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_SRQ_MAX_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_SRQ_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_SRQ_LIMIT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_UD_SRQ_MAX_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_UD_SRQ_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_UD_SRQ_LIMIT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_MAX_NUM_UD_VBUFS;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_MAX_INLINE_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_VBUF_TOTAL_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_VBUF_MAX;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_INITIAL_PREPOST_DEPTH;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_PREPOST_DEPTH;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_VBUF_POOL_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_MAX_NUM_VBUFS;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_VBUF_SECONDARY_POOL_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_CUDA_BLOCK_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_CUDA_NUM_RNDV_BLOCKS;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_MPIRUN_RSH_LAUNCH;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_USE_ROCE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_USE_ROCE_MODE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_USE_RING_STARTUP;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_ON_DEMAND_UD_INFO_EXCHANGE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_USE_PMI_IBARRIER;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_USE_PMI_IALLGATHER;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_NUM_HCAS;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ibv_param.c */
extern int MVP_HOMOGENEOUS_CLUSTER;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ofed_abstraction.h */
extern const char * MVP_LIBIBVERBS_PATH;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ofed_abstraction.h */
extern const char * MVP_LIBIBMAD_PATH;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ofed_abstraction.h */
extern const char * MVP_LIBIBUMAD_PATH;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ofed_abstraction.h */
extern const char * MVP_LIBRDMACM_PATH;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ofed_abstraction.h */
extern const char * MVP_LIBSHARP_PATH;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/rdma_iba_priv.c */
extern int MVP_ALLOW_HETEROGENEOUS_HCA_SELECTION;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/rdma_iba_priv.c */
extern int MVP_PROCESS_PLACEMENT_AWARE_HCA_MAPPING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ring_startup.c */
extern int MVP_DEFAULT_PORT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/gen2/ring_startup.c */
extern int MVP_DEFAULT_GID_INDEX;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/rdma/ch3_init.c */
extern int MVP_IOV_DENSITY_MIN;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/rdma/ch3_init.c */
extern int MVP_ENABLE_EAGER_THRESHOLD_REDUCTION;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/rdma/ch3_init.c */
extern int MVP_USE_EAGER_FAST_SEND;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/rdma/ch3_init.c */
extern int MVP_SUPPORT_FORK_SAFETY;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/rdma/ch3_init.c */
extern int MVP_RDMA_MAX_TRANSFER_SIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/rdma/ch3_init.c */
extern int MVP_RNDV_IMMEDIATE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/rdma/ch3_init.c */
extern int MVP_CHECK_CACHE_ALIGNMENT;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/rdma/ch3_init.c */
extern int MVP_HYBRID_ENABLE_THRESHOLD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/rdma/ch3_init.c */
extern int MVP_USE_ONLY_UD;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/rdma/ch3_init.c */
extern int MVP_SHOW_ENV_INFO;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/rdma/ch3_init.c */
extern const char * MVP_DEBUG_CORESIZE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/rdma/ch3_init.c */
extern int MVP_DEBUG_SHOW_BACKTRACE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/rdma/ch3_init.c */
extern int MVP_USE_THREAD_WARNING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/rdma/ch3_progress.c */
extern int MVP_POLLING_LEVEL;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/rdma/ch3_smp_progress.c */
extern int MVP_WALK_SHARED_PAGES;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/rdma/ch3_smp_progress.c */
extern int MVP_SMP_PRIORITY_FACTOR;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/channels/mrail/src/rdma/ch3_smp_progress.c */
extern int MVP_USE_PT2PT_SHMEM;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch3/src/mpid_abort.c */
extern int MVP_ABORT_SLEEP_SECONDS;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch4/netmod/mvp/mvp_init.c */
extern int MVP_SHOW_CPU_BINDING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch4/netmod/mvp/mvp_init.c */
extern int MVP_SHOW_HCA_BINDING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch4/netmod/mvp/mvp_init.c */
extern int MVP_SHOW_HW_BINDING;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch4/netmod/mvp/mvp_init.c */
extern int MVP_REPORT_LOAD_IMBALANCE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch4/netmod/mvp/mvp_init.c */
extern int MVP_BCAST_HWLOC_TOPOLOGY;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch4/netmod/mvp/mvp_init.c */
extern int MVP_SUPPORT_DPM;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch4/netmod/mvp/mvp_init.c */
extern int MVP_PIVOT_CORE_ID;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch4/netmod/mvp/mvp_init.c */
extern int MVP_FORCE_ARCH_TYPE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch4/netmod/mvp/mvp_init.c */
extern int MVP_THREADS_PER_PROCESS;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch4/netmod/mvp/mvp_init.c */
extern int MVP_SHOW_RUNLOG_INFO;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch4/netmod/mvp/mvp_init.c */
extern int MVP_USE_ALIGNED_ALLOC;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch4/netmod/mvp/mvp_init.c */
extern int MVP_HYBRID_BINDING_POLICY;
enum HYBRID_BINDING_POLICY_choice {
    MVP_HYBRID_BINDING_POLICY_LINEAR,
    MVP_HYBRID_BINDING_POLICY_COMPACT,
    MVP_HYBRID_BINDING_POLICY_SPREAD,
    MVP_HYBRID_BINDING_POLICY_BUNCH,
    MVP_HYBRID_BINDING_POLICY_SCATTER,
    MVP_HYBRID_BINDING_POLICY_NUMA
};
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch4/netmod/mvp/mvp_init.c */
extern int MVP_CPU_BINDING_POLICY;
enum CPU_BINDING_POLICY_choice {
    MVP_CPU_BINDING_POLICY_BUNCH,
    MVP_CPU_BINDING_POLICY_SCATTER,
    MVP_CPU_BINDING_POLICY_HYBRID
};
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch4/netmod/mvp/mvp_init.c */
extern int MVP_ENABLE_PVAR_MEM;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch4/netmod/mvp/mvp_init.c */
extern const char * MVP_USE_BUCKET_FILE;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch4/netmod/mvp/mvp_init.c */
extern int MVP_RNDV_PROTOCOL;
enum RNDV_PROTOCOL_choice {
    MVP_RNDV_PROTOCOL_R3 = 2,
    MVP_RNDV_PROTOCOL_RPUT = 3,
    MVP_RNDV_PROTOCOL_RGET = 4
};
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpid/ch4/netmod/mvp/mvp_init.c */
extern int MVP_SMP_RNDV_PROTOCOL;
enum SMP_RNDV_PROTOCOL_choice {
    MVP_SMP_RNDV_PROTOCOL_R3 = 2,
    MVP_SMP_RNDV_PROTOCOL_RPUT = 3,
    MVP_SMP_RNDV_PROTOCOL_RGET = 4
};
enum {
    MVP_ALLGATHER_BRUCK_THRESHOLD_ID,
    MVP_ALLGATHER_RD_THRESHOLD_ID,
    MVP_ALLGATHER_REVERSE_RANKING_ID,
    MVP_ALLGATHER_COLLECTIVE_ALGORITHM_ID,
    MVP_ALLGATHER_TUNING_IS_TWO_LEVEL_ID,
    MVP_ALLGATHERV_COLLECTIVE_ALGORITHM_ID,
    MVP_ALLREDUCE_2LEVEL_MSG_ID,
    MVP_ALLREDUCE_SHORT_MSG_ID,
    MVP_SHMEM_ALLREDUCE_MSG_ID,
    MVP_USE_SHMEM_ALLREDUCE_ID,
    MVP_USE_SOCKET_AWARE_ALLREDUCE_ID,
    MVP_USE_SOCKET_AWARE_SHARP_ALLREDUCE_ID,
    MVP_SOCKET_AWARE_ALLREDUCE_MAX_MSG_ID,
    MVP_SOCKET_AWARE_ALLREDUCE_MIN_MSG_ID,
    MVP_ALLREDUCE_COLLECTIVE_ALGORITHM_ID,
    MVP_ALLREDUCE_INTER_NODE_TUNING_ALGO_ID,
    MVP_ALLREDUCE_INTRA_NODE_TUNING_ALGO_ID,
    MVP_ALLREDUCE_TUNING_IS_TWO_LEVEL_ID,
    MVP_ALLTOALL_MEDIUM_MSG_ID,
    MVP_ALLTOALL_SMALL_MSG_ID,
    MVP_ALLTOALL_THROTTLE_FACTOR_ID,
    MVP_ALLTOALL_INTRA_THROTTLE_FACTOR_ID,
    MVP_ALLTOALL_LARGE_MSG_THROTTLE_FACTOR_ID,
    MVP_USE_XOR_ALLTOALL_ID,
    MVP_ALLTOALL_COLLECTIVE_ALGORITHM_ID,
    MVP_ALLTOALLV_COLLECTIVE_ALGORITHM_ID,
    MVP_USE_SHMEM_BARRIER_ID,
    MVP_USE_SOCKET_AWARE_BARRIER_ID,
    MVP_BCAST_TWO_LEVEL_SYSTEM_SIZE_ID,
    MVP_KNOMIAL_2LEVEL_BCAST_MESSAGE_SIZE_THRESHOLD_ID,
    MVP_KNOMIAL_2LEVEL_BCAST_SYSTEM_SIZE_THRESHOLD_ID,
    MVP_BCAST_SHORT_MSG_ID,
    MVP_USE_KNOMIAL_2LEVEL_BCAST_ID,
    MVP_USE_KNOMIAL_INTER_LEADER_BCAST_ID,
    MVP_USE_SHMEM_BCAST_ID,
    MVP_BCAST_COLLECTIVE_ALGORITHM_ID,
    MVP_BCAST_INTER_NODE_TUNING_ALGO_ID,
    MVP_BCAST_INTRA_NODE_TUNING_ALGO_ID,
    MVP_BCAST_TUNING_IS_TWO_LEVEL_ID,
    MVP_GATHER_SWITCH_PT_ID,
    MVP_USE_DIRECT_GATHER_ID,
    MVP_USE_DIRECT_GATHER_SYSTEM_SIZE_MEDIUM_ID,
    MVP_USE_DIRECT_GATHER_SYSTEM_SIZE_SMALL_ID,
    MVP_USE_TWO_LEVEL_GATHER_ID,
    MVP_GATHER_COLLECTIVE_ALGORITHM_ID,
    MVP_GATHER_INTER_NODE_TUNING_ALGO_ID,
    MVP_GATHER_INTRA_NODE_TUNING_ALGO_ID,
    MVP_ENABLE_SHARP_ID,
    MVP_SHARP_MAX_MSG_SIZE_ID,
    MVP_SHARP_PORT_ID,
    MVP_SHARP_HCA_NAME_ID,
    MVP_SHARP_MIN_NODE_COUNT_ID,
    MVP_ENABLE_SHARP_ALLREDUCE_ID,
    MVP_ENABLE_SHARP_BARRIER_ID,
    MVP_ENABLE_SHARP_BCAST_ID,
    MVP_ENABLE_SHARP_REDUCE_ID,
    MVP_ENABLE_SHARP_SCATTER_ID,
    MVP_ENABLE_SHARP_SCATTERV_ID,
    MVP_ENABLE_SHARP_IALLREDUCE_ID,
    MVP_ENABLE_SHARP_IREDUCE_ID,
    MVP_ENABLE_SHARP_IBCAST_ID,
    MVP_ENABLE_SHARP_IBARRIER_ID,
    MVP_INTRA_SHMEM_REDUCE_MSG_ID,
    MVP_REDUCE_2LEVEL_MSG_ID,
    MVP_REDUCE_SHORT_MSG_ID,
    MVP_SHMEM_REDUCE_MSG_ID,
    MVP_USE_SHMEM_REDUCE_ID,
    MVP_REDUCE_COLLECTIVE_ALGORITHM_ID,
    MVP_REDUCE_INTER_NODE_TUNING_ALGO_ID,
    MVP_REDUCE_INTRA_NODE_TUNING_ALGO_ID,
    MVP_REDUCE_TUNING_IS_TWO_LEVEL_ID,
    MVP_RED_SCAT_LARGE_MSG_ID,
    MVP_RED_SCAT_SHORT_MSG_ID,
    MVP_SCATTER_MEDIUM_MSG_ID,
    MVP_SCATTER_SMALL_MSG_ID,
    MVP_USE_DIRECT_SCATTER_ID,
    MVP_USE_SCATTER_RD_INTER_LEADER_BCAST_ID,
    MVP_USE_SCATTER_RING_INTER_LEADER_BCAST_ID,
    MVP_USE_TWO_LEVEL_SCATTER_ID,
    MVP_SCATTER_COLLECTIVE_ALGORITHM_ID,
    MVP_SCATTER_INTER_NODE_TUNING_ALGO_ID,
    MVP_SCATTER_INTRA_NODE_TUNING_ALGO_ID,
    MVP_SHMEM_DIR_ID,
    MVP_KNOMIAL_INTER_LEADER_THRESHOLD_ID,
    MVP_KNOMIAL_INTER_NODE_FACTOR_ID,
    MVP_KNOMIAL_INTRA_NODE_FACTOR_ID,
    MVP_KNOMIAL_INTRA_NODE_THRESHOLD_ID,
    MVP_SHMEM_COLL_MAX_MSG_SIZE_ID,
    MVP_SHMEM_COLL_NUM_COMM_ID,
    MVP_SHMEM_COLL_NUM_PROCS_ID,
    MVP_SHMEM_COLL_SPIN_COUNT_ID,
    MVP_USE_OSU_COLLECTIVES_ID,
    MVP_USE_OSU_NB_COLLECTIVES_ID,
    MVP_USE_SHMEM_COLL_ID,
    MVP_ENABLE_SOCKET_AWARE_COLLECTIVES_ID,
    MVP_USE_BLOCKING_ID,
    MVP_USE_SHARED_MEM_ID,
    MVP_ENABLE_TOPO_AWARE_COLLECTIVES_ID,
    MVP_USE_TOPO_AWARE_ALLREDUCE_ID,
    MVP_USE_TOPO_AWARE_REDUCE_ID,
    MVP_USE_TOPO_AWARE_BCAST_ID,
    MVP_USE_OPTIMIZED_RELEASE_ALLREDUCE_ID,
    MVP_USE_SHMEM_TREE_ENABLE_ID,
    MVP_USE_SHMEM_TREE_MIN_MESSAGE_SIZE_ID,
    MVP_USE_SHMEM_TREE_MAX_MESSAGE_SIZE_ID,
    MVP_USE_SHMEM_NUM_TREES_ID,
    MVP_COLL_TMP_BUF_SIZE_ID,
    MVP_USE_TOPO_AWARE_BARRIER_ID,
    MVP_TOPO_AWARE_ALLREDUCE_MAX_MSG_ID,
    MVP_TOPO_AWARE_ALLREDUCE_MIN_MSG_ID,
    MVP_TOPO_AWARE_REDUCE_MAX_MSG_ID,
    MVP_TOPO_AWARE_REDUCE_MIN_MSG_ID,
    MVP_TOPO_AWARE_BCAST_MAX_MSG_ID,
    MVP_TOPO_AWARE_BCAST_MIN_MSG_ID,
    MVP_TOPO_AWARE_REDUCE_PPN_THRESHOLD_ID,
    MVP_TOPO_AWARE_BCAST_PPN_THRESHOLD_ID,
    MVP_TOPO_AWARE_BCAST_NODE_THRESHOLD_ID,
    MVP_TOPO_AWARE_REDUCE_NODE_THRESHOLD_ID,
    MVP_GATHERV_SSEND_MIN_PROCS_ID,
    MVP_USE_MCAST_ALLREDUCE_ID,
    MVP_MCAST_ALLREDUCE_SMALL_MSG_SIZE_ID,
    MVP_MCAST_ALLREDUCE_LARGE_MSG_SIZE_ID,
    MVP_USE_MCAST_SCATTER_ID,
    MVP_MCAST_SCATTER_MSG_SIZE_ID,
    MVP_MCAST_SCATTER_SMALL_SYS_SIZE_ID,
    MVP_MCAST_SCATTER_LARGE_SYS_SIZE_ID,
    MVP_USE_BITONIC_COMM_SPLIT_ID,
    MVP_BITONIC_COMM_SPLIT_THRESHOLD_ID,
    MVP_ENABLE_SKIP_TUNING_TABLE_SEARCH_ID,
    MVP_COLL_SKIP_TABLE_THRESHOLD_ID,
    MVP_USE_INDEXED_ALLGATHER_TUNING_ID,
    MVP_USE_INDEXED_ALLTOALL_TUNING_ID,
    MVP_USE_INDEXED_ALLTOALLV_TUNING_ID,
    MVP_IBCAST_ENABLE_ID,
    MVP_IGATHER_ENABLE_ID,
    MVP_ISCATTER_ENABLE_ID,
    MVP_IALLGATHER_ENABLE_ID,
    MVP_IREDUCE_ENABLE_ID,
    MVP_IALLREDUCE_ENABLE_ID,
    MVP_IREDUCE_SCATTER_ENABLE_ID,
    MVP_IALLTOALL_ENABLE_ID,
    MVP_IALLTOALLV_ENABLE_ID,
    MVP_IALLGATHERV_ENABLE_ID,
    MVP_IBARRIER_ENABLE_ID,
    MVP_SHMEM_COLL_MAX_NUM_EMPTY_PROGRESS_POLLS_ID,
    MVP_USE_KNOMIAL_REDUCE_ID,
    MVP_REDUCE_INTER_KNOMIAL_FACTOR_ID,
    MVP_REDUCE_ZCOPY_INTER_KNOMIAL_FACTOR_ID,
    MVP_REDUCE_INTRA_KNOMIAL_FACTOR_ID,
    MVP_INTRA_MULTI_LEVEL_GATHER_TUNING_ID,
    MVP_USE_OLD_BCAST_ID,
    MVP_ALLRED_USE_RING_ID,
    MVP_USE_OLD_ALLGATHER_ID,
    MVP_USE_OLD_SCATTER_ID,
    MVP_USE_OLD_ALLREDUCE_ID,
    MVP_USE_OLD_REDUCE_ID,
    MVP_USE_OLD_ALLTOALL_ID,
    MVP_ALLTOALL_INPLACE_OLD_ID,
    MVP_ALLTOALL_RD_MAX_MSG_SIZE_ID,
    MVP_INTRA_IBCAST_TUNING_ID,
    MVP_INTRA_IGATHER_TUNING_ID,
    MVP_INTER_IBCAST_TUNING_ID,
    MVP_INTER_IGATHER_TUNING_ID,
    MVP_INTRA_ISCATTER_TUNING_ID,
    MVP_INTER_ISCATTER_TUNING_ID,
    MVP_INTRA_IALLREDUCE_TUNING_ID,
    MVP_INTER_IALLREDUCE_TUNING_ID,
    MVP_INTRA_IREDUCE_TUNING_ID,
    MVP_INTER_IREDUCE_TUNING_ID,
    MVP_INTRA_IREDUCE_SCATTER_TUNING_ID,
    MVP_INTER_IREDUCE_SCATTER_TUNING_ID,
    MVP_INTER_IALLGATHER_TUNING_ID,
    MVP_INTRA_IALLGATHER_TUNING_ID,
    MVP_INTRA_IALLTOALL_TUNING_ID,
    MVP_INTER_IALLTOALL_TUNING_ID,
    MVP_INTRA_IALLTOALLV_TUNING_ID,
    MVP_INTER_IALLTOALLV_TUNING_ID,
    MVP_INTER_IALLGATHERV_TUNING_ID,
    MVP_INTRA_IALLGATHERV_TUNING_ID,
    MVP_INTRA_IBARRIER_TUNING_ID,
    MVP_INTER_IBARRIER_TUNING_ID,
    MVP_INTER_RED_SCAT_TUNING_ID,
    MVP_INTER_RED_SCAT_BLOCK_TUNING_ID,
    MVP_INTER_ALLGATHERV_TUNING_ID,
    MVP_USE_SCATTER_DEST_ALLTOALLV_ID,
    MVP_PIPELINED_ZCPY_BCAST_KNOMIAL_FACTOR_ID,
    MVP_USE_ZCOPY_BCAST_ID,
    MVP_USE_ZCOPY_REDUCE_ID,
    MVP_GATHERV_SSEND_THRESHOLD_ID,
    MVP_ALLREDUCE_RING_ALGO_PPN_THRESHOLD_ID,
    MVP_ALLREDUCE_RING_ALGO_THRESHOLD_ID,
    MVP_ALLREDUCE_RED_SCAT_ALLGATHER_ALGO_PPN_THRESHOLD_ID,
    MVP_ALLREDUCE_RED_SCAT_ALLGATHER_ALGO_THRESHOLD_ID,
    MVP_ALLGATHER_RING_ALGO_THRESHOLD_ID,
    MVP_ALLTOALLV_INTERMEDIATE_WAIT_THRESHOLD_ID,
    MVP_ALLGATHER_CYCLIC_ALGO_THRESHOLD_ID,
    MVP_ALLREDUCE_CYCLIC_ALGO_THRESHOLD_ID,
    MVP_REDSCAT_CYCLIC_ALGO_THRESHOLD_ID,
    MVP_RED_SCAT_RING_ALGO_THRESHOLD_ID,
    MVP_BCAST_SCATTER_RING_OVERLAP_ID,
    MVP_BCAST_SCATTER_RING_OVERLAP_MSG_UPPERBOUND_ID,
    MVP_BCAST_SCATTER_RING_OVERLAP_CORES_LOWERBOUND_ID,
    MVP_USE_PIPELINE_BCAST_ID,
    MVP_BCAST_SEGMENT_SIZE_ID,
    MVP_USE_LIMIC_GATHER_ID,
    MVP_USE_2LEVEL_ALLGATHER_ID,
    MVP_SHMEM_COLL_WINDOW_SIZE_ID,
    MVP_SHMEM_REDUCE_TREE_DEGREE_ID,
    MVP_SHMEM_COLL_SLOT_LEN_ID,
    MVP_USE_SLOT_SHMEM_COLL_ID,
    MVP_USE_SLOT_SHMEM_BCAST_ID,
    MVP_USE_MCAST_PIPELINE_SHM_ID,
    MVP_TWO_LEVEL_COMM_THRESHOLD_ID,
    MVP_TWO_LEVEL_COMM_EARLY_INIT_THRESHOLD_ID,
    MVP_ENABLE_PVAR_TIMER_ID,
    MVP_ENABLE_ALLREDUCE_ALL_COMPUTE_ID,
    MVP_GATHER_STATUS_ALIGNMENT_ID,
    MVP_BCAST_STATUS_ALIGNMENT_ID,
    MVP_ENABLE_ALLREDUCE_SKIP_LARGE_MESSAGE_TUNING_TABLE_SEARCH_ID,
    MVP_ENABLE_ALLREDUCE_SKIP_SMALL_MESSAGE_TUNING_TABLE_SEARCH_ID,
    MVP_SOCKET_AWARE_ALLREDUCE_PPN_THRESHOLD_ID,

    MVP_PVAR_INFO_NAME_ID,

    MVP_CPU_BINDING_LEVEL_ID,
    MVP_CPU_MAPPING_ID,
    MVP_ENABLE_AFFINITY_ID,
    MVP_ENABLE_LEASTLOAD_ID,
    MVP_CM_MAX_SPIN_COUNT_ID,
    MVP_CM_RECV_BUFFERS_ID,
    MVP_CM_SEND_DEPTH_ID,
    MVP_CM_TIMEOUT_ID,
    MVP_CM_UD_PSN_ID,
    MVP_CM_THREAD_STACKSIZE_ID,
    MVP_FORCE_HCA_TYPE_ID,
    MVP_HCA_AWARE_PROCESS_MAPPING_ID,
    MVP_RDMA_CM_ARP_TIMEOUT_ID,
    MVP_RDMA_CM_PORT_ID,
    MVP_RDMA_CM_MAX_PORT_ID,
    MVP_RDMA_CM_MIN_PORT_ID,
    MVP_EAGER_CUDAHOST_REG_ID,
    MVP_CUDA_VECTOR_OPT_ID,
    MVP_CUDA_KERNEL_OPT_ID,
    MVP_CUDA_KERNEL_VECTOR_TIDBLK_SIZE_ID,
    MVP_CUDA_KERNEL_VECTOR_YSIZE_ID,
    MVP_CUDA_KERNEL_SUBARR_TIDBLK_SIZE_ID,
    MVP_CUDA_KERNEL_SUBARR_XDIM_ID,
    MVP_CUDA_KERNEL_SUBARR_YDIM_ID,
    MVP_CUDA_KERNEL_SUBARR_ZDIM_ID,
    MVP_CUDA_NUM_EVENTS_ID,
    MVP_CUDA_INIT_CONTEXT_ID,
    MVP_CUDA_USE_NAIVE_ID,
    MVP_CUDA_ALLTOALL_DYNAMIC_ID,
    MVP_CUDA_REGISTER_NAIVE_BUF_ID,
    MVP_CUDA_GATHER_NAIVE_LIMIT_ID,
    MVP_CUDA_SCATTER_NAIVE_LIMIT_ID,
    MVP_CUDA_ALLTOALL_NAIVE_LIMIT_ID,
    MVP_CUDA_ALLTOALLV_NAIVE_LIMIT_ID,
    MVP_CUDA_ALLGATHER_NAIVE_LIMIT_ID,
    MVP_CUDA_ALLGATHERV_NAIVE_LIMIT_ID,
    MVP_CUDA_BCAST_NAIVE_LIMIT_ID,
    MVP_CUDA_GATHERV_NAIVE_LIMIT_ID,
    MVP_CUDA_SCATTERV_NAIVE_LIMIT_ID,
    MVP_CUDA_ALLGATHER_RD_LIMIT_ID,
    MVP_CUDA_ALLGATHER_FGP_ID,
    MVP_CUDA_IPC_ID,
    MVP_CUDA_IPC_SHARE_GPU_ID,
    MVP_CUDA_SMP_IPC_ID,
    MVP_CUDA_ENABLE_IPC_CACHE_ID,
    MVP_CUDA_IPC_MAX_CACHE_ENTRIES_ID,
    MVP_CUDA_IPC_NUM_STAGE_BUFFERS_ID,
    MVP_CUDA_IPC_STAGE_BUF_SIZE_ID,
    MVP_CUDA_IPC_BUFFERED_ID,
    MVP_CUDA_IPC_THRESHOLD_ID,
    MVP_CUDA_IPC_BUFFERED_LIMIT_ID,
    MVP_CUDA_IPC_SYNC_LIMIT_ID,
    MVP_CUDA_DYNAMIC_INIT_ID,
    MVP_CUDA_NONBLOCKING_STREAMS_ID,
    MVP_SMP_EAGERSIZE_ID,
    MVP_IBA_HCA_ID,
    MVP_SMP_USE_CMA_ID,
    MVP_SMP_USE_LIMIC2_ID,
    MVP_SMP_POLLING_THRESHOLD_ID,
    MVP_SMP_CMA_MAX_SIZE_ID,
    MVP_SMP_LIMIC2_MAX_SIZE_ID,
    MVP_SMP_QUEUE_LENGTH_ID,
    MVP_LIMIC_PUT_THRESHOLD_ID,
    MVP_LIMIC_GET_THRESHOLD_ID,
    MVP_R3_THRESHOLD_ID,
    MVP_INTRA_NODE_R3_THRESHOLD_ID,
    MVP_INTER_NODE_R3_THRESHOLD_ID,
    MVP_R3_NOCACHE_THRESHOLD_ID,
    MVP_MAX_R3_PENDING_DATA_ID,
    MVP_USE_LIMIC2_COLL_ID,
    MVP_SMP_USE_MAX_SWITCH_ID,
    MVP_CUDA_SMP_PIPELINE_ID,
    MVP_SMP_DELAY_SHMEM_POOL_INIT_ID,
    MVP_SMP_PRIORITY_POLLING_ID,
    MVP_SMP_NUM_SEND_BUFFER_ID,
    MVP_SMP_BATCH_SIZE_ID,
    MVP_SMP_SEND_BUF_SIZE_ID,
    MVP_USE_OPT_EAGER_RECV_ID,
    MVP_NUM_NODES_IN_JOB_ID,
    MVP_USE_MCAST_ID,
    MVP_USE_RDMA_CM_MCAST_ID,
    MVP_USE_QOS_ID,
    MVP_3DTORUS_SUPPORT_ID,
    MVP_PATH_SL_QUERY_ID,
    MVP_NUM_SLS_ID,
    MVP_NUM_SA_QUERY_RETRIES_ID,
    MVP_MAX_RDMA_CONNECT_ATTEMPTS_ID,
    MVP_RDMA_CM_CONNECT_RETRY_INTERVAL_ID,
    MVP_USE_HSAM_ID,
    MVP_USE_APM_ID,
    MVP_USE_APM_TEST_ID,
    MVP_USE_IWARP_MODE_ID,
    MVP_USE_RDMA_CM_ID,
    MVP_RDMA_CM_MULTI_SUBNET_SUPPORT_ID,
    MVP_ON_DEMAND_THRESHOLD_ID,
    MVP_USE_XRC_ID,
    MVP_USE_SRQ_ID,
    MVP_IWARP_MULTIPLE_CQ_THRESHOLD_ID,
    MVP_USE_LAZY_MEM_UNREGISTER_ID,
    MVP_USE_RMA_FAST_PATH_ID,
    MVP_FORCE_IB_ATOMIC_ID,
    MVP_USE_RDMA_ONE_SIDED_ID,
    MVP_RNDV_EXT_SENDQ_SIZE_ID,
    MVP_RDMA_NUM_EXTRA_POLLS_ID,
    MVP_COALESCE_THRESHOLD_ID,
    MVP_USE_COALESCE_ID,
    MVP_SPIN_COUNT_ID,
    MVP_PROCESS_TO_RAIL_MAPPING_ID,
    MVP_SM_SCHEDULING_ID,
    MVP_SMALL_MSG_RAIL_SHARING_POLICY_ID,
    MVP_MED_MSG_RAIL_SHARING_POLICY_ID,
    MVP_RAIL_SHARING_POLICY_ID,
    MVP_SUPPRESS_JOB_STARTUP_PERFORMANCE_WARNING_ID,
    MVP_USE_RDMA_FAST_PATH_ID,
    MVP_RDMA_FAST_PATH_BUF_SIZE_ID,
    MVP_POLLING_SET_LIMIT_ID,
    MVP_POLLING_SET_THRESHOLD_ID,
    MVP_RDMA_EAGER_LIMIT_ID,
    MVP_NUM_RDMA_BUFFER_ID,
    MVP_RDMA_FAST_PATH_PREALLOCATE_BUFFERS_ID,
    MVP_SYSREPORT_ID,
    MVP_DEFAULT_MTU_ID,
    MVP_NUM_CQES_PER_POLL_ID,
    MVP_NUM_PORTS_ID,
    MVP_NUM_QP_PER_PORT_ID,
    MVP_PIN_POOL_SIZE_ID,
    MVP_DEFAULT_MAX_CQ_SIZE_ID,
    MVP_IBA_EAGER_THRESHOLD_ID,
    MVP_STRIPING_THRESHOLD_ID,
    MVP_RAIL_SHARING_MED_MSG_THRESHOLD_ID,
    MVP_RAIL_SHARING_LARGE_MSG_THRESHOLD_ID,
    MVP_DEFAULT_PUT_GET_LIST_SIZE_ID,
    MVP_EAGERSIZE_1SC_ID,
    MVP_PUT_FALLBACK_THRESHOLD_ID,
    MVP_GET_FALLBACK_THRESHOLD_ID,
    MVP_DEFAULT_QP_OUS_RD_ATOM_ID,
    MVP_DEFAULT_MAX_RDMA_DST_OPS_ID,
    MVP_DEFAULT_PSN_ID,
    MVP_DEFAULT_PKEY_ID,
    MVP_DEFAULT_QKEY_ID,
    MVP_DEFAULT_MIN_RNR_TIMER_ID,
    MVP_DEFAULT_SERVICE_LEVEL_ID,
    MVP_DEFAULT_TIME_OUT_ID,
    MVP_DEFAULT_STATIC_RATE_ID,
    MVP_DEFAULT_SRC_PATH_BITS_ID,
    MVP_DEFAULT_RETRY_COUNT_ID,
    MVP_DEFAULT_RNR_RETRY_ID,
    MVP_DEFAULT_MAX_SG_LIST_ID,
    MVP_DEFAULT_MAX_SEND_WQE_ID,
    MVP_CM_WAIT_TIME_ID,
    MVP_USE_UD_HYBRID_ID,
    MVP_UD_MTU_ID,
    MVP_HYBRID_MAX_RC_CONN_ID,
    MVP_UD_NUM_MSG_LIMIT_ID,
    MVP_UD_SENDWINDOW_SIZE_ID,
    MVP_UD_RECVWINDOW_SIZE_ID,
    MVP_UD_RETRY_TIMEOUT_ID,
    MVP_UD_MAX_RETRY_TIMEOUT_ID,
    MVP_UD_PROGRESS_SPIN_ID,
    MVP_UD_RETRY_COUNT_ID,
    MVP_UD_PROGRESS_TIMEOUT_ID,
    MVP_UD_MAX_SEND_WQE_ID,
    MVP_UD_MAX_RECV_WQE_ID,
    MVP_USE_UD_SRQ_ID,
    MVP_UD_VBUF_POOL_SIZE_ID,
    MVP_UD_MAX_ACK_PENDING_ID,
    MVP_SHMEM_BACKED_UD_CM_ID,
    MVP_USE_UD_ZCOPY_ID,
    MVP_UD_ZCOPY_THRESHOLD_ID,
    MVP_UD_NUM_ZCOPY_RNDV_QPS_ID,
    MVP_UD_ZCOPY_RQ_SIZE_ID,
    MVP_UD_ZCOPY_NUM_RETRY_ID,
    MVP_UD_ZCOPY_ENABLE_POLLING_ID,
    MVP_UD_ZCOPY_PUSH_SEGMENT_ID,
    MVP_UD_DROP_PACKET_RATE_ID,
    MVP_MCAST_ENABLE_REL_ID,
    MVP_MCAST_USE_MCAST_NACK_ID,
    MVP_MCAST_NUM_NODES_THRESHOLD_ID,
    MVP_MCAST_MAX_RECV_WQE_ID,
    MVP_MCAST_WINDOW_SIZE_ID,
    MVP_MCAST_DROP_PACKET_RATE_ID,
    MVP_MCAST_RETRY_TIMEOUT_ID,
    MVP_MCAST_MAX_RETRY_TIMEOUT_ID,
    MVP_MCAST_NSPIN_THRESHOLD_ID,
    MVP_MCAST_COMM_INIT_TIMEOUT_ID,
    MVP_MCAST_COMM_INIT_RETRIES_ID,
    MVP_MCAST_SKIP_LOOPBACK_ID,
    MVP_MCAST_BCAST_MIN_MSG_ID,
    MVP_MCAST_BCAST_MAX_MSG_ID,
    MVP_DEFAULT_MAX_RECV_WQE_ID,
    MVP_NDREG_ENTRIES_MAX_ID,
    MVP_NDREG_ENTRIES_ID,
    MVP_DREG_CACHE_LIMIT_ID,
    MVP_USE_HWLOC_CPU_BINDING_ID,
    MVP_THREAD_YIELD_SPIN_THRESHOLD_ID,
    MVP_USE_THREAD_YIELD_ID,
    MVP_NUM_SPINS_BEFORE_LOCK_ID,
    MVP_ASYNC_THREAD_STACK_SIZE_ID,
    MVP_USE_HUGEPAGES_ID,
    MVP_MEMORY_OPTIMIZATION_ID,
    MVP_SRQ_MAX_SIZE_ID,
    MVP_SRQ_SIZE_ID,
    MVP_SRQ_LIMIT_ID,
    MVP_UD_SRQ_MAX_SIZE_ID,
    MVP_UD_SRQ_SIZE_ID,
    MVP_UD_SRQ_LIMIT_ID,
    MVP_MAX_NUM_UD_VBUFS_ID,
    MVP_MAX_INLINE_SIZE_ID,
    MVP_VBUF_TOTAL_SIZE_ID,
    MVP_VBUF_MAX_ID,
    MVP_INITIAL_PREPOST_DEPTH_ID,
    MVP_PREPOST_DEPTH_ID,
    MVP_VBUF_POOL_SIZE_ID,
    MVP_MAX_NUM_VBUFS_ID,
    MVP_VBUF_SECONDARY_POOL_SIZE_ID,
    MVP_CUDA_BLOCK_SIZE_ID,
    MVP_CUDA_NUM_RNDV_BLOCKS_ID,
    MVP_MPIRUN_RSH_LAUNCH_ID,
    MVP_USE_ROCE_ID,
    MVP_USE_ROCE_MODE_ID,
    MVP_USE_RING_STARTUP_ID,
    MVP_ON_DEMAND_UD_INFO_EXCHANGE_ID,
    MVP_USE_PMI_IBARRIER_ID,
    MVP_USE_PMI_IALLGATHER_ID,
    MVP_NUM_HCAS_ID,
    MVP_HOMOGENEOUS_CLUSTER_ID,
    MVP_LIBIBVERBS_PATH_ID,
    MVP_LIBIBMAD_PATH_ID,
    MVP_LIBIBUMAD_PATH_ID,
    MVP_LIBRDMACM_PATH_ID,
    MVP_LIBSHARP_PATH_ID,
    MVP_ALLOW_HETEROGENEOUS_HCA_SELECTION_ID,
    MVP_PROCESS_PLACEMENT_AWARE_HCA_MAPPING_ID,
    MVP_DEFAULT_PORT_ID,
    MVP_DEFAULT_GID_INDEX_ID,
    MVP_IOV_DENSITY_MIN_ID,
    MVP_ENABLE_EAGER_THRESHOLD_REDUCTION_ID,
    MVP_USE_EAGER_FAST_SEND_ID,
    MVP_SUPPORT_FORK_SAFETY_ID,
    MVP_RDMA_MAX_TRANSFER_SIZE_ID,
    MVP_RNDV_IMMEDIATE_ID,
    MVP_CHECK_CACHE_ALIGNMENT_ID,
    MVP_HYBRID_ENABLE_THRESHOLD_ID,
    MVP_USE_ONLY_UD_ID,
    MVP_SHOW_ENV_INFO_ID,
    MVP_DEBUG_CORESIZE_ID,
    MVP_DEBUG_SHOW_BACKTRACE_ID,
    MVP_USE_THREAD_WARNING_ID,
    MVP_POLLING_LEVEL_ID,
    MVP_WALK_SHARED_PAGES_ID,
    MVP_SMP_PRIORITY_FACTOR_ID,
    MVP_USE_PT2PT_SHMEM_ID,
    MVP_ABORT_SLEEP_SECONDS_ID,
    MVP_SHOW_CPU_BINDING_ID,
    MVP_SHOW_HCA_BINDING_ID,
    MVP_SHOW_HW_BINDING_ID,
    MVP_REPORT_LOAD_IMBALANCE_ID,
    MVP_BCAST_HWLOC_TOPOLOGY_ID,
    MVP_SUPPORT_DPM_ID,
    MVP_PIVOT_CORE_ID_ID,
    MVP_FORCE_ARCH_TYPE_ID,
    MVP_THREADS_PER_PROCESS_ID,
    MVP_SHOW_RUNLOG_INFO_ID,
    MVP_USE_ALIGNED_ALLOC_ID,
    MVP_HYBRID_BINDING_POLICY_ID,
    MVP_CPU_BINDING_POLICY_ID,
    MVP_ENABLE_PVAR_MEM_ID,
    MVP_USE_BUCKET_FILE_ID,
    MVP_RNDV_PROTOCOL_ID,
    MVP_SMP_RNDV_PROTOCOL_ID,
};

#define MVP_CVAR_COUNT 491
extern u_int8_t MVP_CVAR_USER_SET_FLAGS[(MVP_CVAR_COUNT + 7) >> 3];

/* Evaluates to 1 if the CVAR was explicitly defined by the user else 0 */
#define MVP_CVAR_IS_SET_BY_USER(_cvar)                                      \
    (1 & MVP_CVAR_USER_SET_FLAGS[_cvar##_ID >> 3] >> (_cvar##_ID & 7))

#define MVP_CVAR_SOFT_SET(_cvar, _val)                                      \
    if (!(MVP_CVAR_IS_SET_BY_USER(_cvar))) { _cvar = _val; }

/* TODO: this should be defined elsewhere */
#define MVP_assert MPIR_Assert

/* Arbitrary, simplifies interaction with external interfaces like MPI_T_ */
#define MVP_MAX_STRLEN (384)

/* Shortens enum value comparisons */
#define MVP_ENUM_IS(A, a) (MVP_ ## A == MVP_ ## A ## _ ## a)

#endif /* MVP_CVARS_H_INCLUDED */
