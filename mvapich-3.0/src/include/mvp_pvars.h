/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

/* Automatically generated
 *   by:   ./maint/extractmvppvars
 *   on:   Fri Feb 16 17:33:28 2024 UTC
 *
 * DO NOT EDIT!!!
 */

#if !defined(MVP_PVARS_H_INCLUDED)
#define MVP_PVARS_H_INCLUDED

#include "mpiimpl.h"

/* Initializes pvar values from the environment */
int MPIR_T_MVP_pvar_init(void);
int MPIR_T_MVP_pvar_finalize(void);

/* Extern declarations for each pvar
 * (definitions in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi/mvp/mvp_pvars.c) */

/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_num_2level_comm_requests;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_num_2level_comm_success;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_num_shmem_coll_calls;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long PVAR_COUNTER_mpit_progress_poll;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long PVAR_COUNTER_mvp_smp_read_progress_poll;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long PVAR_COUNTER_mvp_smp_write_progress_poll;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long PVAR_COUNTER_mvp_smp_read_progress_poll_success;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long PVAR_COUNTER_mvp_smp_write_progress_poll_success;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long PVAR_COUNTER_rdma_ud_retransmissions;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_unexpected_recvs_rendezvous;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_expected_recvs_rendezvous;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_bcast_binomial;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_bcast_scatter_doubling_allgather;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_bcast_scatter_ring_allgather;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_bcast_scatter_ring_allgather_shm;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_bcast_shmem;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_bcast_knomial_internode;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_bcast_knomial_intranode;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_bcast_mcast_internode;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_bcast_pipelined;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_bcast_shmem_zcpy;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_bcast_sharp;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_bcast_topo_aware_hierarchical;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_bcast_pipelined_zcpy;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_binomial;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_scatter_doubling_allgather;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_scatter_ring_allgather;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_scatter_ring_allgather_shm;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_shmem;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_knomial_internode;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_knomial_intranode;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_mcast_internode;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_pipelined;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_shmem_zcpy;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_sharp;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_topo_aware_hierarchical;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_pipelined_zcpy;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_binomial_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_scatter_for_bcast_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_scatter_doubling_allgather_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_scatter_ring_allgather_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_scatter_ring_allgather_shm_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_knomial_internode_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_knomial_intranode_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_mcast_internode_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_pipelined_zcpy_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_tune_inter_node_helper_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_inter_node_helper_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_binomial_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_scatter_for_bcast_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_scatter_doubling_allgather_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_scatter_ring_allgather_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_scatter_ring_allgather_shm_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_knomial_internode_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_knomial_intranode_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_mcast_internode_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_pipelined_zcpy_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_tune_inter_node_helper_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_inter_node_helper_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_binomial_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_scatter_for_bcast_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_scatter_doubling_allgather_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_scatter_ring_allgather_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_scatter_ring_allgather_shm_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_knomial_internode_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_knomial_intranode_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_mcast_internode_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_pipelined_zcpy_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_tune_inter_node_helper_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_inter_node_helper_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_binomial_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_scatter_for_bcast_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_scatter_doubling_allgather_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_scatter_ring_allgather_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_scatter_ring_allgather_shm_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_knomial_internode_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_knomial_intranode_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_mcast_internode_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_pipelined_zcpy_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_tune_inter_node_helper_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_inter_node_helper_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_bcast_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_alltoall_inplace;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_alltoall_bruck;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_alltoall_rd;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_alltoall_sd;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_alltoall_pw;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_inplace;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_bruck;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_rd;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_sd;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_pw;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_inplace_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_bruck_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_sd_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_pw_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_inplace_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_bruck_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_sd_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_pw_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_inplace_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_bruck_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_sd_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_pw_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_inplace_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_bruck_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_sd_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_pw_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_intra_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_intra_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_intra_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_intra_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_alltoallv_intra;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_alltoallv_intra_scatter;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoallv_intra;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoallv_intra_scatter;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoallv_intra_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoallv_intra_scatter_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoallv_intra_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoallv_intra_scatter_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoallv_intra_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoallv_intra_scatter_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoallv_intra_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoallv_intra_scatter_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoallv_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoallv_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoallv_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoallv_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_alltoall_cuda;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_cuda_intra_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_cuda_intra_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_cuda_intra_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_cuda_intra_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_cuda_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_cuda_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_cuda_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_alltoall_cuda_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allreduce_subcomm;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allreduce_sharp;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allreduce_shm_rd;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allreduce_shm_rs;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allreduce_shm_intra;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allreduce_intra_p2p;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allreduce_2lvl;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allreduce_shmem;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allreduce_mcast;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allreduce_reduce_scatter_allgather_colls;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allreduce_topo_aware_hierarchical;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allreduce_pt2pt_ring_wrapper;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allreduce_pt2pt_ring;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allreduce_pt2pt_ring_inplace;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_subcomm;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_sharp;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_shm_rd;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_shm_rs;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_shm_intra;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_intra_p2p;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_2lvl;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_shmem;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_mcast;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_reduce_scatter_allgather_colls;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_topo_aware_hierarchical;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_pt2pt_ring_wrapper;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_pt2pt_ring;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_pt2pt_ring_inplace;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_pt2pt_rd_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_pt2pt_rs_bytes_send;

extern MPI_T_PVAR_detail_info_t PVAR_INFO_mvp_coll_allreduce_sendrecv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_pt2pt_ring_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_pt2pt_ring_wrapper_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_pt2pt_ring_inplace_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_pt2pt_rd_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_pt2pt_rs_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_pt2pt_ring_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_pt2pt_ring_wrapper_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_pt2pt_ring_inplace_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_pt2pt_rd_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_pt2pt_rs_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_pt2pt_ring_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_pt2pt_ring_wrapper_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_pt2pt_ring_inplace_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_pt2pt_rd_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_pt2pt_rs_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_pt2pt_ring_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_pt2pt_ring_wrapper_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_pt2pt_ring_inplace_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allgather_rd;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allgather_bruck;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allgather_ring;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allgather_direct;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allgather_directspread;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allgather_gather_bcast;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allgather_2lvl;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allgather_2lvl_nonblocked;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allgather_2lvl_ring_nonblocked;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allgather_2lvl_direct;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allgather_2lvl_ring;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_rd_allgather_comm;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_rd;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_bruck;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_ring;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_direct;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_directspread;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_gather_bcast;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_2lvl;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_2lvl_nonblocked;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_2lvl_ring_nonblocked;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_2lvl_direct;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_2lvl_ring;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_rd_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_bruck_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_ring_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_direct_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_directspread_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_2lvl_nonblocked_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_2lvl_ring_nonblocked_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_2lvl_direct_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_2lvl_ring_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_rd_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_bruck_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_ring_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_direct_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_directspread_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_2lvl_nonblocked_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_2lvl_ring_nonblocked_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_2lvl_direct_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_2lvl_ring_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_rd_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_bruck_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_ring_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_direct_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_directspread_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_2lvl_nonblocked_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_2lvl_ring_nonblocked_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_2lvl_direct_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_2lvl_ring_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_rd_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_bruck_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_ring_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_direct_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_directspread_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_2lvl_nonblocked_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_2lvl_ring_nonblocked_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_2lvl_direct_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_2lvl_ring_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allgather_cuda;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_cuda_intra_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_cuda_intra_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_cuda_intra_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_cuda_intra_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_cuda_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_cuda_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_cuda_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgather_cuda_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_gather_pt2pt;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_gather_direct;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_gather_direct_blk;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_gather_two_level_direct;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_gather_limic_scheme_pt_pt;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_gather_limic_scheme_pt_linear;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_gather_limic_scheme_linear_pt;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_gather_limic_scheme_linear_linear;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_gather_limic_scheme_single_leader;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_gather_intra_node_limic;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_pt2pt;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_direct;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_direct_blk;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_two_level_direct;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_limic_scheme_pt_pt;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_limic_scheme_pt_linear;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_limic_scheme_linear_pt;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_limic_scheme_linear_linear;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_limic_scheme_single_leader;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_intra_node_limic;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_direct_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_direct_blk_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_two_level_direct_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_direct_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_direct_blk_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_two_level_direct_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_direct_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_direct_blk_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_two_level_direct_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_direct_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_direct_blk_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_two_level_direct_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_gather_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_reduce_scatter_noncomm;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_reduce_scatter_basic;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_reduce_scatter_rec_halving;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_reduce_scatter_pairwise;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_reduce_scatter_ring;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_reduce_scatter_non_comm;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_reduce_scatter_ring_2lvl;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_ring;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_ring_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_ring_2lvl_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_ring_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_ring_2lvl_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_ring_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_ring_2lvl_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_ring_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_ring_2lvl_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_ring_2lvl;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_basic;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_rec_halving;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_pairwise;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_non_comm;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_noncomm;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_noncomm_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_basic_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_rec_halving_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_pairwise_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_non_comm_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_noncomm_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_basic_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_rec_halving_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_pairwise_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_non_comm_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_noncomm_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_basic_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_rec_halving_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_pairwise_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_non_comm_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_noncomm_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_basic_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_rec_halving_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_pairwise_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_non_comm_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_scatter_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_scatter_mcast;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_scatter_binomial;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_scatter_direct;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_scatter_direct_blk;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_scatter_two_level_binomial;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_scatter_two_level_direct;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_scatter_inter;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_scatter_sharp;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_mcast;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_binomial;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_direct;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_direct_blk;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_two_level_binomial;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_two_level_direct;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_inter;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_sharp;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_mcast_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_mcast_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_binomial_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_binomial_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_direct_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_direct_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_direct_blk_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_direct_blk_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_two_level_direct_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_two_level_direct_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_two_level_binomial_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_two_level_binomial_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_inter_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_inter_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_mcast_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_mcast_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_binomial_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_binomial_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_direct_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_direct_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_direct_blk_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_direct_blk_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_two_level_direct_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_two_level_direct_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_two_level_binomial_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_two_level_binomial_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_inter_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_inter_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scatter_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_reduce_binomial;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_reduce_allreduce;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_reduce_redscat_gather;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_reduce_shmem;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_reduce_knomial;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_reduce_zcpy;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_reduce_sharp;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_reduce_topo_aware_hierarchical;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_reduce_two_level_helper;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_subcomm;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_binomial;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_allreduce;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_redscat_gather;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_shmem;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_knomial;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_zcpy;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_sharp;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_topo_aware_hierarchical;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_two_level_helper;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_binomial_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_allreduce_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_redscat_gather_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_two_level_helper_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_knomial_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_zcpy_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_binomial_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_allreduce_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_redscat_gather_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_two_level_helper_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_knomial_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_zcpy_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_binomial_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_allreduce_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_redscat_gather_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_two_level_helper_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_knomial_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_zcpy_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_binomial_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_allreduce_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_redscat_gather_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_two_level_helper_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_knomial_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_zcpy_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_reduce_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_gatherv_algo;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_gatherv_algo;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_gatherv_default_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_gatherv_default_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_gatherv_default_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_gatherv_default_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_gatherv_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_gatherv_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_gatherv_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_gatherv_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allgatherv_rec_doubling;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allgatherv_bruck;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allgatherv_ring;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allgatherv_ring_cyclic;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgatherv_rec_doubling;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgatherv_bruck;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgatherv_ring;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgatherv_ring_cyclic;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_barrier_subcomm;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_barrier_pairwise;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_barrier_shmem;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_barrier_sharp;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_barrier_topo_aware_shmem;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_barrier_subcomm;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_barrier_pairwise;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_barrier_shmem;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_barrier_sharp;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_barrier_topo_aware_shmem;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_barrier_pairwise_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_barrier_pairwise_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_barrier_pairwise_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_barrier_pairwise_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_barrier_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_barrier_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_barrier_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_barrier_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgatherv_rec_doubling_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgatherv_bruck_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgatherv_ring_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgatherv_rec_doubling_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgatherv_bruck_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgatherv_ring_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgatherv_rec_doubling_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgatherv_bruck_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgatherv_ring_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgatherv_rec_doubling_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgatherv_bruck_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgatherv_ring_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgatherv_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgatherv_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgatherv_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_allgatherv_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_exscan_algo;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_scan_algo;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long PVAR_COUNTER_mvp_reg_cache_hits;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long PVAR_COUNTER_mvp_reg_cache_misses;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long PVAR_COUNTER_mvp_vbuf_allocated;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long PVAR_COUNTER_mvp_vbuf_freed;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long PVAR_COUNTER_mvp_ud_vbuf_allocated;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long PVAR_COUNTER_mvp_ud_vbuf_freed;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long PVAR_LEVEL_mvp_vbuf_available;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long PVAR_LEVEL_mvp_ud_vbuf_available;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long PVAR_LEVEL_mvp_smp_eager_avail_buffer;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long PVAR_LEVEL_mvp_smp_rndv_avail_buffer;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long PVAR_LEVEL_mvp_smp_eager_total_buffer;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long PVAR_LEVEL_mvp_smp_rndv_total_buffer;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long PVAR_LEVEL_mvp_smp_eager_buffer_max_use;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long PVAR_LEVEL_mvp_smp_rndv_buffer_max_use;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long PVAR_COUNTER_mvp_smp_eager_sent;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long PVAR_COUNTER_mvp_smp_rndv_sent;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long PVAR_COUNTER_mvp_smp_eager_received;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long PVAR_COUNTER_mvp_smp_rndv_received;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long PVAR_COUNTER_mvp_rdmafp_ctrl_packet_count;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long PVAR_COUNTER_mvp_rdmafp_out_of_order_packet_count;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long PVAR_COUNTER_mvp_rdmafp_exact_recv_count;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long PVAR_COUNTER_mvp_ibv_channel_ctrl_packet_count;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long PVAR_COUNTER_mvp_ibv_channel_out_of_order_packet_count;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long PVAR_COUNTER_mvp_ibv_channel_exact_recv_count;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iscatter_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iscatter_binomial_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iscatter_binomial_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iscatter_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iscatter_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iscatter_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iscatter_binomial_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iscatter_binomial_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_iallgather_rec_dbl;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_iallgather_bruck;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_iallgather_ring;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_iallgather_intra;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_iallgather_inter;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgather_rec_dbl;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgather_bruck;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgather_ring;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgather_intra;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgather_inter;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgather_intra_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgather_intra_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgather_intra_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgather_intra_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgather_inter_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgather_inter_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgather_inter_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgather_inter_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgather_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgather_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgather_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgather_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgather_ring_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgather_ring_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgather_ring_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgather_ring_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgather_bruck_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgather_bruck_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgather_bruck_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgather_bruck_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgather_rec_dbl_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgather_rec_dbl_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgather_rec_dbl_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgather_rec_dbl_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_iallgatherv_rec_dbl;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_iallgatherv_bruck;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_iallgatherv_ring;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_iallgatherv_intra;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_iallgatherv_inter;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgatherv_rec_dbl;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgatherv_bruck;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgatherv_ring;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgatherv_intra;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgatherv_inter;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgatherv_intra_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgatherv_intra_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgatherv_intra_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgatherv_intra_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgatherv_inter_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgatherv_inter_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgatherv_inter_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgatherv_inter_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgatherv_ring_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgatherv_ring_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgatherv_ring_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgatherv_ring_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgatherv_bruck_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgatherv_bruck_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgatherv_bruck_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgatherv_bruck_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgatherv_rec_dbl_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgatherv_rec_dbl_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgatherv_rec_dbl_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgatherv_rec_dbl_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgatherv_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgatherv_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgatherv_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallgatherv_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_iallreduce_naive;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_iallreduce_rec_dbl;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_iallreduce_intra;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_iallreduce_inter;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_iallreduce_redscat_allgather;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_iallreduce_SMP;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallreduce_naive;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallreduce_rec_dbl;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallreduce_intra;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallreduce_inter;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallreduce_redscat_allgather;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallreduce_SMP;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallreduce_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallreduce_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallreduce_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallreduce_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallreduce_naive_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallreduce_naive_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallreduce_naive_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallreduce_naive_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallreduce_rec_dbl_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallreduce_rec_dbl_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallreduce_rec_dbl_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallreduce_rec_dbl_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallreduce_intra_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallreduce_intra_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallreduce_intra_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallreduce_intra_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallreduce_inter_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallreduce_inter_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallreduce_inter_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallreduce_inter_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallreduce_redscat_allgather_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallreduce_redscat_allgather_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallreduce_redscat_allgather_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallreduce_redscat_allgather_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallreduce_SMP_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallreduce_SMP_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallreduce_SMP_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_iallreduce_SMP_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_ialltoall_intra;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_ialltoall_inter;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_ialltoall_pairwise;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_ialltoall_perm_sr;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_ialltoall_bruck;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_ialltoall_inplace;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoall_intra;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoall_inter;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoall_pairwise;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoall_perm_sr;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoall_bruck;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoall_inplace;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoall_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoall_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoall_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoall_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoall_inplace_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoall_inplace_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoall_inplace_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoall_inplace_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoall_bruck_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoall_bruck_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoall_bruck_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoall_bruck_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoall_perm_sr_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoall_perm_sr_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoall_perm_sr_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoall_perm_sr_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoall_pairwise_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoall_pairwise_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoall_pairwise_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoall_pairwise_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoall_intra_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoall_intra_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoall_intra_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoall_intra_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoall_inter_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoall_inter_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoall_inter_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoall_inter_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_ialltoallv_intra;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_ialltoallv_inter;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoallv_intra;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoallv_inter;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoallv_intra_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoallv_intra_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoallv_intra_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoallv_intra_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoallv_inter_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoallv_inter_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoallv_inter_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoallv_inter_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoallv_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoallv_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoallv_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ialltoallv_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_igather_intra;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_igather_inter;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_igather_binomial;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_igather_intra;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_igather_inter;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_igather_binomial;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_igather_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_igather_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_igather_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_igather_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_igather_intra_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_igather_intra_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_igather_intra_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_igather_intra_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_igather_inter_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_igather_inter_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_igather_inter_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_igather_inter_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_igather_binomial_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_igather_binomial_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_igather_binomial_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_igather_binomial_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_igatherv_algo;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_igatherv_algo;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_igatherv_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_igatherv_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_igatherv_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_igatherv_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_igatherv_def_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_igatherv_def_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_igatherv_def_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_igatherv_def_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_ireduce_binomial;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_ireduce_intra;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_ireduce_inter;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_ireduce_sharp;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_ireduce_redscat_gather;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_ireduce_SMP;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_binomial;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_intra;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_inter;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_redscat_gather;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_SMP;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_binomial_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_binomial_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_binomial_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_binomial_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_intra_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_intra_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_intra_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_intra_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_inter_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_inter_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_inter_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_inter_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_redscat_gather_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_redscat_gather_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_redscat_gather_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_redscat_gather_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_SMP_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_SMP_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_SMP_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_SMP_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_ibcast_binomial;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_ibcast_SMP;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_ibcast_scatter_rec_dbl_allgather;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_ibcast_scatter_ring_allgather;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_ibcast_intra;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_ibcast_inter;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_ibcast_sharp;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibcast_binomial;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibcast_SMP;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibcast_scatter_rec_dbl_allgather;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibcast_scatter_ring_allgather;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibcast_intra;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibcast_inter;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibcast_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibcast_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibcast_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibcast_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibcast_intra_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibcast_intra_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibcast_intra_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibcast_intra_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibcast_inter_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibcast_inter_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibcast_inter_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibcast_inter_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibcast_binomial_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibcast_binomial_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibcast_binomial_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibcast_binomial_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibcast_SMP_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibcast_SMP_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibcast_SMP_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibcast_SMP_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibcast_scatter_rec_dbl_allgather_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibcast_scatter_rec_dbl_allgather_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibcast_scatter_rec_dbl_allgather_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibcast_scatter_rec_dbl_allgather_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibcast_scatter_ring_allgather_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibcast_scatter_ring_allgather_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibcast_scatter_ring_allgather_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibcast_scatter_ring_allgather_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_ibarrier_intra;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_ibarrier_inter;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_ibarrier_sharp;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibarrier_intra;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibarrier_inter;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibarrier_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibarrier_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibarrier_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibarrier_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibarrier_intra_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibarrier_intra_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibarrier_intra_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibarrier_intra_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibarrier_inter_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibarrier_inter_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibarrier_inter_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ibarrier_inter_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_ireduce_scatter_rec_hlv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_ireduce_scatter_pairwise;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_ireduce_scatter_rec_dbl;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_ireduce_scatter_noncomm;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_ireduce_scatter_intra;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_ireduce_scatter_inter;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_rec_hlv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_pairwise;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_rec_dbl;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_noncomm;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_intra;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_inter;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_rec_hlv_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_rec_hlv_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_rec_hlv_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_rec_hlv_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_pairwise_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_pairwise_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_pairwise_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_pairwise_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_rec_dbl_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_rec_dbl_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_rec_dbl_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_rec_dbl_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_noncomm_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_noncomm_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_noncomm_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_noncomm_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_intra_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_intra_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_intra_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_intra_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_inter_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_inter_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_inter_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_inter_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_ireduce_scatter_block_rec_hlv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_ireduce_scatter_block_pairwise;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_ireduce_scatter_block_rec_dbl;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_ireduce_scatter_block_noncomm;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_ireduce_scatter_block_intra;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_ireduce_scatter_block_inter;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_block_rec_hlv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_block_pairwise;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_block_rec_dbl;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_block_noncomm;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_block_intra;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_block_inter;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_block_rec_hlv_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_block_rec_hlv_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_block_rec_hlv_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_block_rec_hlv_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_block_pairwise_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_block_pairwise_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_block_pairwise_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_block_pairwise_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_block_rec_dbl_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_block_rec_dbl_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_block_rec_dbl_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_block_rec_dbl_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_block_noncomm_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_block_noncomm_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_block_noncomm_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_block_noncomm_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_block_intra_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_block_intra_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_block_intra_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_block_intra_count_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_block_inter_bytes_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_block_inter_bytes_recv;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_block_inter_count_send;
/* declared in /tmp/XiCjhDXj0Y/mvapich-3.0/maint/../src/mpi_t/mvp_pvars_list.txt */
extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_scatter_block_inter_count_recv;

#define MVP_PVAR_COUNT 893

#endif /* MVP_PVARS_H_INCLUDED */
