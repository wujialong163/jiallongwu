/*
 * Comm overrides for the CH4 device in MVP. Has to be included separetely from
 * the collective overrides since this is a part of mpidpre.h which is included
 * before mpidch4.h (which eventually includes the coll_override.h
 */
#ifndef MVP_COMM_OVERRIDE_H
#define MVP_COMM_OVERRIDE_H

#if ENABLE_PVAR_MVP
#include "mpitimpl.h"
#endif /* ENABLE_PVAR_MVP */

#ifdef _OSU_MVAPICH_
typedef struct {
    MPI_Comm leader_comm;
    MPI_Comm shmem_comm;
    MPI_Comm allgather_comm;
    MPI_Comm *topo_comm;
    MPI_Comm *topo_leader_comm;
    int *leader_map;
    int *leader_rank;
    int *node_sizes; /* number of processes on each node */
    int *node_disps; /* displacements into rank_list for each node */
    int *allgather_new_ranks;
    int *rank_list; /* list of ranks, ordered by node id, then shmem rank on
                       each node */
    void *coll_tmp_buf;
    int rank_list_index; /* index of this process in the rank_list array */
    int is_uniform;
    int is_blocked;
    int shmem_comm_rank;
    int shmem_coll_ok;
    int topo_coll_ok;
    int allgather_comm_ok;
    int leader_group_size;
    int is_global_block;
    int is_pof2;         /* Boolean to know if comm size is equal to pof2  */
    int gpof2;           /* Greater pof2 < size of comm */
    int intra_node_done; /* Used to check if intra node communication has been
                            done with mcast and bcast */
    int shmem_coll_count;
    int allgather_coll_count;
    int allreduce_coll_count;
    int barrier_coll_count;
    int reduce_coll_count;
    int bcast_coll_count;
    int scatter_coll_count;
    int scatterv_coll_count;
    int iallreduce_coll_count;
    int ireduce_coll_count;
    int ibcast_coll_count;
    int ibarrier_coll_count;
    void *shmem_info; /* intra node shmem info */
    MPI_Comm intra_sock_comm;
    MPI_Comm intra_sock_leader_comm;
    MPI_Comm global_sock_leader_comm;
    int *socket_size;
    int is_socket_uniform;
    int use_intra_sock_comm;
    int my_sock_id;
    int tried_to_create_leader_shmem;
#if defined(_MCST_SUPPORT_)
    int is_mcast_ok;
    void *bcast_info;
#endif
#if ENABLE_PVAR_MVP
    MPIR_T_pvar_timer_t *sub_comm_timers;
    unsigned long long *sub_comm_counters;
#endif
#if defined(_SHARP_SUPPORT_)
    int is_sharp_ok;
    void *sharp_coll_info;
#endif
} MPIDI_MVP_comm_t;
#else
typedef struct {
    int dummy;
} MPIDI_MVP_comm_t;
#endif /* _OSU_MVAPICH_ */

/* struct needed to implement our shmem colls */
#define MPIDI_MVP_LEGACY_COMM_DECL MPIDI_MVP_comm_t ch;

#endif /* MVP_COMM_OVERRIDE_H */
