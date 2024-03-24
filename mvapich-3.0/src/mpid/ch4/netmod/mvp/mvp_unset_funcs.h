/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef MVP_UNSET_H_INCLUDED
#define MVP_UNSET_H_INCLUDED

/* Remove inline definitions of base level netmod */
#ifdef NETMOD_INLINE

#undef MPIDI_NM_mpi_comm_commit_pre_hook
#undef MPIDI_NM_mpi_comm_commit_post_hook
#undef MPIDI_NM_mpi_comm_free_hook

#undef MPIDI_NM_mpi_comm_connect
#undef MPIDI_NM_mpi_comm_disconnect
#undef MPIDI_NM_mpi_open_port
#undef MPIDI_NM_mpi_close_port
#undef MPIDI_NM_mpi_comm_accept

#undef MPIDI_NM_mpi_op_free_hook
#undef MPIDI_NM_mpi_op_commit_hook

#undef MPIDI_NM_mpi_win_set_info
#undef MPIDI_NM_mpi_win_get_info
#undef MPIDI_NM_mpi_win_free
#undef MPIDI_NM_mpi_win_create
#undef MPIDI_NM_mpi_win_attach
#undef MPIDI_NM_mpi_win_allocate_shared
#undef MPIDI_NM_mpi_win_detach
#undef MPIDI_NM_mpi_win_allocate
#undef MPIDI_NM_mpi_win_create_dynamic
#undef MPIDI_NM_mpi_win_create_hook
#undef MPIDI_NM_mpi_win_allocate_hook
#undef MPIDI_NM_mpi_win_allocate_shared_hook
#undef MPIDI_NM_mpi_win_create_dynamic_hook
#undef MPIDI_NM_mpi_win_attach_hook
#undef MPIDI_NM_mpi_win_detach_hook
#undef MPIDI_NM_mpi_win_free_hook

#undef MPIDI_NM_mpi_init_hook
#undef MPIDI_NM_mpi_finalize_hook
#undef MPIDI_NM_post_init
#undef MPIDI_NM_get_vci_attr
#undef MPIDI_NM_mpi_alloc_mem
#undef MPIDI_NM_mpi_free_mem
#undef MPIDI_NM_get_local_upids
#undef MPIDI_NM_upids_to_lupids
#undef MPIDI_NM_create_intercomm_from_lpids

#undef MPIDI_NM_mpi_type_free_hook
#undef MPIDI_NM_mpi_type_commit_hook

#undef MPIDI_NM_progress

#endif /* NETMOD_INLINE  */

#endif /* MVP_NOINLINE_H_INCLUDED */
