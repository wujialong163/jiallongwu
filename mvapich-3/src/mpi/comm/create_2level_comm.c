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

#include "mpiimpl.h"
//#include "mpidimpl.h"
#include "mpicomm.h"
#include "mvp_coll_shmem.h"
#include <pthread.h>
#if defined(_MCST_SUPPORT_)
#include "ibv_mcast.h"
#endif

#if defined(_SHARP_SUPPORT_)
#include "mvp_sharp.h"
#endif

extern unsigned long long PVAR_COUNTER_mvp_num_2level_comm_requests;
extern unsigned long long PVAR_COUNTER_mvp_num_2level_comm_success;

#if defined(_SMP_LIMIC_)
int mvp_limic_comm_count = 0;
#endif

static pthread_mutex_t comm_lock = PTHREAD_MUTEX_INITIALIZER;

#define MAX_NUM_THREADS 1024
pthread_t thread_reg[MAX_NUM_THREADS];

extern int mvp_intra_node_cluster_at_level[];
extern int get_numa_bound_info(int *numa_bound, int *num_numas,
                               int *num_cores_numa, int *is_uniform);
extern void mvp_flush_all_zcpy_barrier_requests(shmem_info_t *shmem);

void clear_2level_comm(MPIR_Comm *comm_ptr)
{
    comm_ptr->dev.ch.allgather_comm_ok = 0;
    comm_ptr->dev.ch.shmem_coll_ok = 0;
    comm_ptr->dev.ch.topo_coll_ok = 0;
    comm_ptr->dev.ch.leader_map = NULL;
    comm_ptr->dev.ch.leader_rank = NULL;
    comm_ptr->dev.ch.node_disps = NULL;
    comm_ptr->dev.ch.rank_list = NULL;
    comm_ptr->dev.ch.coll_tmp_buf = NULL;
    comm_ptr->dev.ch.rank_list_index = -1;
    comm_ptr->dev.ch.shmem_comm = MPI_COMM_NULL;
    comm_ptr->dev.ch.leader_comm = MPI_COMM_NULL;
    comm_ptr->dev.ch.allgather_comm = MPI_COMM_NULL;
    comm_ptr->dev.ch.intra_node_done = 0;
    comm_ptr->dev.ch.topo_comm = NULL;
    comm_ptr->dev.ch.topo_leader_comm = NULL;
#if defined(_MCST_SUPPORT_)
    comm_ptr->dev.ch.is_mcast_ok = 0;
#endif

#if defined(_SHARP_SUPPORT_)
    comm_ptr->dev.ch.is_sharp_ok = 0;
#endif
}

int free_limic_comm(MPIR_Comm *shmem_comm_ptr)
{
    int intra_comm_rank = -1;
    int mpi_errno = MPI_SUCCESS;

    MPIR_Comm *intra_sock_comm_ptr = NULL;
    MPIR_Comm *intra_sock_leader_comm_ptr = NULL;

    MPIR_Comm_get_ptr(shmem_comm_ptr->dev.ch.intra_sock_comm,
                      intra_sock_comm_ptr);
    MPIR_Comm_get_ptr(shmem_comm_ptr->dev.ch.intra_sock_leader_comm,
                      intra_sock_leader_comm_ptr);

    if (intra_sock_comm_ptr != NULL) {
        PMPI_Comm_rank(shmem_comm_ptr->dev.ch.intra_sock_comm,
                       &intra_comm_rank);
        if (intra_comm_rank == 0) {
            if (shmem_comm_ptr->dev.ch.socket_size != NULL) {
                MPL_free(shmem_comm_ptr->dev.ch.socket_size);
            }
        }
    }
    if (intra_sock_comm_ptr->dev.ch.shmem_info) {
        mvp_shm_coll_cleanup(
            (shmem_info_t *)intra_sock_comm_ptr->dev.ch.shmem_info);
        MPL_free(intra_sock_comm_ptr->dev.ch.shmem_info);
    }
    if (intra_sock_comm_ptr != NULL) {
        mpi_errno = MPIR_Comm_release(intra_sock_comm_ptr);
        if (mpi_errno != MPI_SUCCESS) {
            MPIR_ERR_POP(mpi_errno);
        }
    }
    if (intra_sock_leader_comm_ptr != NULL) {
        // If shmem coll ok is set to 1 for a socket leader communicator of size
        // 1 (for correctness), set it back to 0 to prevent segmentation faults
        // while releasing the communicator
        if (intra_sock_leader_comm_ptr->local_size == 1 &&
            intra_sock_leader_comm_ptr->dev.ch.shmem_coll_ok == 1) {
            intra_sock_leader_comm_ptr->dev.ch.shmem_coll_ok = 0;
        }
        mpi_errno = MPIR_Comm_release(intra_sock_leader_comm_ptr);
        if (mpi_errno != MPI_SUCCESS) {
            MPIR_ERR_POP(mpi_errno);
        }
    }
    shmem_comm_ptr->dev.ch.socket_size = NULL;
    shmem_comm_ptr->dev.ch.is_socket_uniform = 0;
    shmem_comm_ptr->dev.ch.use_intra_sock_comm = 0;
    shmem_comm_ptr->dev.ch.intra_sock_comm = MPI_COMM_NULL;
    shmem_comm_ptr->dev.ch.intra_sock_leader_comm = MPI_COMM_NULL;
fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int free_intra_sock_comm(MPIR_Comm *comm_ptr)
{
    int mpi_errno;
    MPIR_Comm *shmem_comm_ptr = NULL;
    MPIR_Comm *global_sock_leader_comm_ptr = NULL;
    MPIR_Comm_get_ptr(comm_ptr->dev.ch.shmem_comm, shmem_comm_ptr);
    MPIR_Comm_get_ptr(comm_ptr->dev.ch.global_sock_leader_comm,
                      global_sock_leader_comm_ptr);

    if (global_sock_leader_comm_ptr != NULL) {
        mpi_errno = MPIR_Comm_release(global_sock_leader_comm_ptr);
        if (mpi_errno != MPI_SUCCESS) {
            MPIR_ERR_POP(mpi_errno);
        }
    }

    /*Reuse limic comm free since some variables are common between these comms
     */
    mpi_errno = free_limic_comm(shmem_comm_ptr);

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int free_domain_comm_shmem(MPIR_Comm *comm_ptr)
{
    int mpi_errno = MPI_SUCCESS;

    if (comm_ptr->dev.ch.shmem_info) {
        mvp_shm_coll_cleanup((shmem_info_t *)comm_ptr->dev.ch.shmem_info);
        MPL_free(comm_ptr->dev.ch.shmem_info);
    }
    if (comm_ptr->rank == 0 && comm_ptr->dev.ch.shmem_comm_rank >= 0) {
        lock_shmem_region();
        MPIR_MVP_SHMEM_Coll_Block_Clear_Status(
            comm_ptr->dev.ch.shmem_comm_rank);
        unlock_shmem_region();
    }
    comm_ptr->dev.ch.shmem_coll_ok = 0;
    if (comm_ptr->local_group != NULL) {
        MPIR_Group_release(comm_ptr->local_group);
    }
    mpi_errno = MPIR_Comm_release(comm_ptr);
    if (mpi_errno != MPI_SUCCESS) {
        goto fn_fail;
    }

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int free_domain_comms_at_level(MPIR_Comm *shmem_comm_ptr, int level)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Comm *comm_ptr = NULL;
    /* Free intra-domain comm */
    MPIR_Comm_get_ptr(shmem_comm_ptr->dev.ch.topo_comm[level], comm_ptr);
    if (comm_ptr) {
        mpi_errno = free_domain_comm_shmem(comm_ptr);
        if (mpi_errno) {
            MPIR_ERR_POP(mpi_errno);
        }
    }

    /* Free inter-domain comm */
    MPIR_Comm_get_ptr(shmem_comm_ptr->dev.ch.topo_leader_comm[level], comm_ptr);
    if (comm_ptr) {
        mpi_errno = free_domain_comm_shmem(comm_ptr);
        if (mpi_errno) {
            MPIR_ERR_POP(mpi_errno);
        }
    }

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int free_2level_comm(MPIR_Comm *comm_ptr)
{
    MPIR_Comm *shmem_comm_ptr = NULL;
    MPIR_Comm *leader_comm_ptr = NULL;
    MPIR_Comm *allgather_comm_ptr = NULL;

    int i = 0;
    int local_rank = 0;
    int mpi_errno = MPI_SUCCESS;

    if (comm_ptr->dev.ch.leader_map != NULL) {
        MPL_free(comm_ptr->dev.ch.leader_map);
    }
    if (comm_ptr->dev.ch.leader_rank != NULL) {
        MPL_free(comm_ptr->dev.ch.leader_rank);
    }
    if (comm_ptr->dev.ch.rank_list != NULL) {
        MPL_free(comm_ptr->dev.ch.rank_list);
    }
    if (comm_ptr->dev.ch.coll_tmp_buf != NULL) {
        MPL_free(comm_ptr->dev.ch.coll_tmp_buf);
        comm_ptr->dev.ch.coll_tmp_buf = NULL;
    }
    MPIR_Comm_get_ptr((comm_ptr->dev.ch.shmem_comm), shmem_comm_ptr);
    MPIR_Comm_get_ptr((comm_ptr->dev.ch.leader_comm), leader_comm_ptr);

    if (MVP_ENABLE_TOPO_AWARE_COLLECTIVES) {
        comm_ptr->dev.ch.topo_coll_ok = 0;
        if (shmem_comm_ptr->dev.ch.topo_comm ||
            shmem_comm_ptr->dev.ch.topo_leader_comm) {
            for (i = 0; i <= mvp_num_intra_node_comm_levels; ++i) {
                mpi_errno = free_domain_comms_at_level(shmem_comm_ptr, i);
                if (mpi_errno != MPI_SUCCESS) {
                    MPIR_ERR_POP(mpi_errno);
                }
            }
            MPL_free(shmem_comm_ptr->dev.ch.topo_comm);
            MPL_free(shmem_comm_ptr->dev.ch.topo_leader_comm);
        }
    }

    if (comm_ptr->dev.ch.allgather_comm_ok == 1) {
        MPIR_Comm_get_ptr((comm_ptr->dev.ch.allgather_comm),
                          allgather_comm_ptr);
        MPL_free(comm_ptr->dev.ch.allgather_new_ranks);
    }

    local_rank = shmem_comm_ptr->rank;

#if defined(_SHARP_SUPPORT_)
    if (MVP_ENABLE_SHARP) {
        mpi_errno = mvp_free_sharp_handlers(comm_ptr->dev.ch.sharp_coll_info);
        if (mpi_errno != MPI_SUCCESS) {
            MPIR_ERR_POP(mpi_errno);
        }
    }
#endif /* if defined (_SHARP_SUPPORT_) */

    if (local_rank == 0 && shmem_comm_ptr->dev.ch.shmem_comm_rank >= 0) {
        lock_shmem_region();
        MPIR_MVP_SHMEM_Coll_Block_Clear_Status(
            shmem_comm_ptr->dev.ch.shmem_comm_rank);
        unlock_shmem_region();
    }

#if defined(_SMP_LIMIC_)
    if (shmem_comm_ptr->dev.ch.use_intra_sock_comm == 1) {
        free_limic_comm(shmem_comm_ptr);
    }
#endif /* #if defined(_SMP_LIMIC_) */

    if (comm_ptr->dev.ch.use_intra_sock_comm == 1) {
        free_intra_sock_comm(comm_ptr);
    }

#if defined(_MCST_SUPPORT_)
    if (local_rank == 0 && comm_ptr->dev.ch.is_mcast_ok) {
        mvp_cleanup_multicast(
            &((bcast_info_t *)comm_ptr->dev.ch.bcast_info)->minfo, comm_ptr);
    }
    MPL_free(comm_ptr->dev.ch.bcast_info);
#endif

    if (comm_ptr->dev.ch.shmem_info) {
        shmem_info_t *shmem = comm_ptr->dev.ch.shmem_info;
#ifdef CHANNEL_MRAIL_GEN2
        /* If there are incomplete sends in zcopy bcast/reduce, defer freeing of
         * this shmem_info struct until all relevant completion events are
         * generated.  This deferred freeing is taken care of in
         * MRAILI_Process_send. */
        if (shmem->zcpy_coll_pending_send_ops > 0) {
            shmem->defer_free = 1;
            mvp_flush_all_zcpy_barrier_requests(shmem);
        } else {
            mvp_shm_coll_cleanup(shmem);
            MPL_free(shmem);
        }
#else
        mvp_shm_coll_cleanup(shmem);
        MPL_free(shmem);
#endif /* CHANNEL_MRAIL_GEN2 */
    }

    if (local_rank == 0) {
        if (comm_ptr->dev.ch.node_sizes != NULL) {
            MPL_free(comm_ptr->dev.ch.node_sizes);
        }
    }
    if (comm_ptr->dev.ch.node_disps != NULL) {
        MPL_free(comm_ptr->dev.ch.node_disps);
    }
    if (local_rank == 0 && leader_comm_ptr != NULL) {
        mpi_errno = MPIR_Comm_release(leader_comm_ptr);
        if (mpi_errno != MPI_SUCCESS) {
            MPIR_ERR_POP(mpi_errno);
        }
    }
    if (shmem_comm_ptr != NULL) {
        /* Decrease the reference number of shmem_group, which
         * was increased in create_2level_comm->PMPI_Group_incl */
        if (shmem_comm_ptr->local_group != NULL) {
            MPIR_Group_release(shmem_comm_ptr->local_group);
        }
        mpi_errno = MPIR_Comm_release(shmem_comm_ptr);
        if (mpi_errno != MPI_SUCCESS) {
            MPIR_ERR_POP(mpi_errno);
        }
    }
    if (allgather_comm_ptr != NULL) {
        mpi_errno = MPIR_Comm_release(allgather_comm_ptr);
        if (mpi_errno != MPI_SUCCESS) {
            MPIR_ERR_POP(mpi_errno);
        }
    }
    clear_2level_comm(comm_ptr);
fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

inline void MPIR_pof2_comm(MPIR_Comm *comm_ptr, int size, int my_rank)
{
    int v = 1, old_v = 1;

    //*  Check if comm is a pof2 or not */
    comm_ptr->dev.ch.is_pof2 = (size & (size - 1)) ? 0 : 1;

    /* retrieve the greatest power of two < size of comm */
    if (comm_ptr->dev.ch.is_pof2) {
        comm_ptr->dev.ch.gpof2 = size;
    } else {
        while (v < size) {
            old_v = v;
            v = v << 1;
        }
        comm_ptr->dev.ch.gpof2 = old_v;
    }
}

#if defined(_SMP_LIMIC_)
int create_intra_node_multi_level_comm(MPIR_Comm *comm_ptr)
{
    int socket_bound = -1;
    int numCoresSocket = 0;
    int numSocketsNode = 0;
    int *intra_socket_leader_map = NULL;
    int *intra_sock_leader_group = NULL;
    int intra_comm_rank = 0, intra_comm_size = 0;
    int intra_leader_comm_size = 0, intra_leader_comm_rank = 0;
    int ok_to_create_intra_sock_comm = 0, i = 0;
    int input_flag = 0, output_flag = 0;
    int my_local_size, my_local_id;
    int mpi_errno = MPI_SUCCESS;
    MPIR_Errflag_t errflag = MPIR_ERR_NONE;

    MPI_Group subgroup2;
    MPIR_Group *group_ptr = NULL;
    MPIR_Comm *shmem_ptr = NULL;
    MPIR_Comm *intra_sock_leader_commptr = NULL;
    MPIR_Comm *intra_sock_commptr = NULL;

    MPIR_Comm_get_ptr(comm_ptr->dev.ch.shmem_comm, shmem_ptr);
    mpi_errno = PMPI_Comm_rank(comm_ptr->dev.ch.shmem_comm, &my_local_id);
    if (mpi_errno) {
        MPIR_ERR_POP(mpi_errno);
    }

    mpi_errno = PMPI_Comm_size(comm_ptr->dev.ch.shmem_comm, &my_local_size);
    if (mpi_errno) {
        MPIR_ERR_POP(mpi_errno);
    }

    if (MVP_USE_LIMIC2_COLL) {
        MPI_Group comm_group1;
        lock_shmem_region();
        mvp_limic_comm_count = get_mvp_limic_comm_count();
        unlock_shmem_region();

        if (mvp_limic_comm_count <= mvp_max_limic_comms) {
            socket_bound = get_socket_bound();
            numSocketsNode = numofSocketsPerNode();
            numCoresSocket = numOfCoresPerSocket(socket_bound);
            int *intra_socket_map =
                MPL_malloc(sizeof(int) * my_local_size, MPL_MEM_COLL);
            if (NULL == intra_socket_map) {
                mpi_errno = MPIR_Err_create_code(
                    MPI_SUCCESS, MPI_ERR_OTHER, __func__, __LINE__,
                    MPI_ERR_OTHER, "**fail", "%s: %s",
                    "memory allocation failed", strerror(errno));
                MPIR_ERR_POP(mpi_errno);
            }

            memset(intra_socket_map, -1, sizeof(int) * my_local_size);

            mpi_errno =
                MPIR_Allgather_impl(&socket_bound, 1, MPI_INT, intra_socket_map,
                                    1, MPI_INT, shmem_ptr, &errflag);
            if (mpi_errno) {
                MPIR_ERR_POP(mpi_errno);
            }

            /*Check if all the proceses are not in same socket.
             * We create socket communicators only when 2 or
             * more sockets are present*/
            for (i = 1; i < my_local_size; i++) {
                if (intra_socket_map[0] != intra_socket_map[i]) {
                    ok_to_create_intra_sock_comm = 1;
                    break;
                }
            }
            MPL_free(intra_socket_map);
        }

        if (ok_to_create_intra_sock_comm != 0) {
            /*Create communicator for intra socket communication*/
            mpi_errno = PMPI_Comm_split(comm_ptr->dev.ch.shmem_comm,
                                        socket_bound, my_local_id,
                                        &(shmem_ptr->dev.ch.intra_sock_comm));
            if (mpi_errno) {
                MPIR_ERR_POP(mpi_errno);
            }
            int intra_socket_leader_id = -1;
            int intra_socket_leader_cnt = 0;
            PMPI_Comm_rank(shmem_ptr->dev.ch.intra_sock_comm, &intra_comm_rank);
            PMPI_Comm_size(shmem_ptr->dev.ch.intra_sock_comm, &intra_comm_size);

            if (intra_comm_rank == 0) {
                intra_socket_leader_id = 1;
            }

            /*Creating intra-socket leader group*/
            intra_socket_leader_map =
                MPL_malloc(sizeof(int) * my_local_size, MPL_MEM_COLL);
            if (NULL == intra_socket_leader_map) {
                mpi_errno = MPIR_Err_create_code(
                    MPI_SUCCESS, MPI_ERR_OTHER, __func__, __LINE__,
                    MPI_ERR_OTHER, "**fail", "%s: %s",
                    "memory allocation failed", strerror(errno));
                MPIR_ERR_POP(mpi_errno);
            }
            /*initialize the intra_socket_leader_map*/
            for (i = 0; i < my_local_size; i++) // TODO: Replace with memset
                intra_socket_leader_map[i] = -1;

            mpi_errno = MPIR_Allgather_impl(&intra_socket_leader_id, 1, MPI_INT,
                                            intra_socket_leader_map, 1, MPI_INT,
                                            shmem_ptr, &errflag);
            if (mpi_errno) {
                MPIR_ERR_POP(mpi_errno);
            }

            for (i = 0; i < my_local_size; i++) {
                if (intra_socket_leader_map[i] == 1)
                    intra_socket_leader_cnt++;
            }

            intra_sock_leader_group =
                MPL_malloc(sizeof(int) * intra_socket_leader_cnt, MPL_MEM_COLL);
            if (NULL == intra_sock_leader_group) {
                mpi_errno = MPIR_Err_create_code(
                    MPI_SUCCESS, MPI_ERR_OTHER, __func__, __LINE__,
                    MPI_ERR_OTHER, "**fail", "%s: %s",
                    "memory allocation failed", strerror(errno));
                MPIR_ERR_POP(mpi_errno);
            }

            /*Assuming homogeneous system, where every socket has same number
             * of cores */
            int j = 0;
            for (i = 0; i < my_local_size; i++) {
                if (intra_socket_leader_map[i] == 1)
                    /*i here actually is the my_local_id for which
                    intra_sock_rank == 0*/
                    intra_sock_leader_group[j++] = i;
            }

            /*Resuing comm_group and subgroup1 variables for creation of
             * intra socket leader comm*/
            mpi_errno =
                PMPI_Comm_group(comm_ptr->dev.ch.shmem_comm, &comm_group1);
            if (mpi_errno) {
                MPIR_ERR_POP(mpi_errno);
            }

            mpi_errno = PMPI_Group_incl(comm_group1, intra_socket_leader_cnt,
                                        intra_sock_leader_group, &subgroup2);
            if (mpi_errno) {
                MPIR_ERR_POP(mpi_errno);
            }

            /*Creating intra_sock_leader communicator*/
            mpi_errno =
                PMPI_Comm_create(comm_ptr->dev.ch.shmem_comm, subgroup2,
                                 &(shmem_ptr->dev.ch.intra_sock_leader_comm));
            if (mpi_errno) {
                MPIR_ERR_POP(mpi_errno);
            }

            if (intra_comm_rank == 0) {
                mpi_errno =
                    PMPI_Comm_rank(shmem_ptr->dev.ch.intra_sock_leader_comm,
                                   &intra_leader_comm_rank);
                if (mpi_errno) {
                    MPIR_ERR_POP(mpi_errno);
                }
                mpi_errno =
                    PMPI_Comm_size(shmem_ptr->dev.ch.intra_sock_leader_comm,
                                   &intra_leader_comm_size);
                if (mpi_errno) {
                    MPIR_ERR_POP(mpi_errno);
                }
            }

            /*Check if all the data in sockets are of uniform size*/
            if (intra_comm_rank == 0) {
                int array_index = 0;

                shmem_ptr->dev.ch.socket_size = MPL_malloc(
                    sizeof(int) * intra_leader_comm_size, MPL_MEM_COLL);
                mpi_errno = PMPI_Allgather(
                    &intra_comm_size, 1, MPI_INT, shmem_ptr->dev.ch.socket_size,
                    1, MPI_INT, shmem_ptr->dev.ch.intra_sock_leader_comm);
                if (mpi_errno) {
                    MPIR_ERR_POP(mpi_errno);
                }
                shmem_ptr->dev.ch.is_socket_uniform = 1;
                for (array_index = 0; array_index < intra_leader_comm_size;
                     array_index++) {
                    if (shmem_ptr->dev.ch.socket_size[0] !=
                        shmem_ptr->dev.ch.socket_size[array_index]) {
                        shmem_ptr->dev.ch.is_socket_uniform = 0;
                        break;
                    }
                }
            }

            MPIR_Group_get_ptr(subgroup2, group_ptr);
            if (group_ptr != NULL) {
                mpi_errno = PMPI_Group_free(&subgroup2);
                if (mpi_errno) {
                    MPIR_ERR_POP(mpi_errno);
                }
            }
            mpi_errno = PMPI_Group_free(&comm_group1);
            if (mpi_errno) {
                MPIR_ERR_POP(mpi_errno);
            }
        }

        if ((mvp_limic_comm_count <= mvp_max_limic_comms) &&
            (ok_to_create_intra_sock_comm != 0) && my_local_id == 0) {
            /*update num of sockets within a node*/
            lock_shmem_region();

            UpdateNumCoresPerSock(numCoresSocket);
            UpdateNumSocketsPerNode(numSocketsNode);
            /*only 1 intra sock leader comm*/
            increment_mvp_limic_comm_count();
            /*many per node intra sock comm*/
            for (i = 0; i < intra_leader_comm_size; i++)
                increment_mvp_limic_comm_count();

            mvp_limic_comm_count = get_mvp_limic_comm_count();
            unlock_shmem_region();
        }

        mpi_errno = MPIR_Bcast_impl(&mvp_limic_comm_count, 1, MPI_INT, 0,
                                    shmem_ptr, &errflag);
        if (mpi_errno) {
            MPIR_ERR_POP(mpi_errno);
        }

        if ((mvp_limic_comm_count <= mvp_max_limic_comms) &&
            (MVP_USE_LIMIC2_COLL) && (ok_to_create_intra_sock_comm != 0)) {
            MPIR_Comm_get_ptr(shmem_ptr->dev.ch.intra_sock_leader_comm,
                              intra_sock_leader_commptr);
            MPIR_Comm_get_ptr(shmem_ptr->dev.ch.intra_sock_comm,
                              intra_sock_commptr);
            if (intra_comm_rank == 0) {
                intra_sock_leader_commptr->dev.ch.shmem_comm_rank =
                    mvp_limic_comm_count - 1;
            }
            intra_sock_commptr->dev.ch.shmem_comm_rank =
                mvp_limic_comm_count - (socket_bound + 2);
            input_flag = 1;
        } else {
            input_flag = 0;
        }

        mpi_errno = MPIR_Allreduce_impl(&input_flag, &output_flag, 1, MPI_INT,
                                        MPI_LAND, comm_ptr, &errflag);
        if (mpi_errno) {
            MPIR_ERR_POP(mpi_errno);
        }

        if (output_flag == 1) {
            /*Enable using the intra-sock communicators*/
            shmem_ptr->dev.ch.use_intra_sock_comm = 1;
        } else {
            /*Disable using the intra-sock communicators*/
            shmem_ptr->dev.ch.use_intra_sock_comm = 0;

            if ((MVP_USE_LIMIC2_COLL) && (ok_to_create_intra_sock_comm != 0)) {
                MPIR_Group_get_ptr(subgroup2, group_ptr);
                if (group_ptr != NULL) {
                    mpi_errno = PMPI_Group_free(&subgroup2);
                    if (mpi_errno) {
                        MPIR_ERR_POP(mpi_errno);
                    }
                }
                MPIR_Group_get_ptr(comm_group1, group_ptr);
                if (group_ptr != NULL) {
                    mpi_errno = PMPI_Group_free(&comm_group1);
                    if (mpi_errno) {
                        MPIR_ERR_POP(mpi_errno);
                    }
                }
                free_limic_comm(shmem_ptr);
            }
        }
    }
fn_fail:
    if (intra_socket_leader_map != NULL)
        MPL_free(intra_socket_leader_map);
    if (intra_sock_leader_group != NULL)
        MPL_free(intra_sock_leader_group);
    return (mpi_errno);
}
#endif

static inline int create_intra_node_topo_comm_at_level(
    MPIR_Comm *parent_comm_ptr, int in_color, MPI_Comm *comm,
    MPI_Comm *ldr_comm)
{
    int i = 0;
    int leader = 0;
    int grp_index = 0;
    int rank_in_new_comm = 0;
    int input_flag = 0, output_flag = 0;
    int *color_array = NULL, *ldr_map = NULL;
    int *shmem_group = NULL, *ldr_group = NULL, *ldr_rank = NULL;
    int mpi_errno = MPI_SUCCESS;
    int mvp_shmem_coll_blk_stat = -1;
    MPIR_Group *subgroup1 = NULL, *comm_group = NULL;
    MPIR_Comm *comm_ptr = NULL, *ldr_comm_ptr = NULL;
    int comm_rank = -1, comm_size = -1;
    int parent_rank = -1, parent_size = -1;
    MPIR_Errflag_t errflag = MPIR_ERR_NONE;
    int tmp_cvar = 0;

    parent_rank = parent_comm_ptr->rank;
    parent_size = parent_comm_ptr->local_size;

    /* Allocate memory for gathering the shmem_group */
    shmem_group = MPL_malloc(sizeof(int) * parent_size, MPL_MEM_COLL);
    if (shmem_group == NULL) {
        MPIR_ERR_SETANDJUMP2(mpi_errno, MPI_ERR_NO_MEM, "**nomem",
                             "**nomem %s %d", "(int) shmem_group", parent_size);
    }
    /* Allocate memory for gathering the color */
    color_array = MPL_malloc(sizeof(int) * parent_size, MPL_MEM_COLL);
    if (color_array == NULL) {
        MPIR_ERR_SETANDJUMP2(mpi_errno, MPI_ERR_OTHER, "**nomem",
                             "**nomem %s %d", "(int) color_array", parent_size);
    }
    /* Allocate memory for gathering the ldr_group */
    ldr_group = MPL_malloc(sizeof(int) * parent_size, MPL_MEM_COLL);
    if (ldr_group == NULL) {
        MPIR_ERR_SETANDJUMP2(mpi_errno, MPI_ERR_OTHER, "**nomem",
                             "**nomem %s %d", "(int) ldr_group", parent_size);
    }
    /* Allocate memory for gathering the ldr_map */
    ldr_map = MPL_malloc(sizeof(int) * parent_size, MPL_MEM_COLL);
    if (ldr_map == NULL) {
        MPIR_ERR_SETANDJUMP2(mpi_errno, MPI_ERR_OTHER, "**nomem",
                             "**nomem %s %d", "(int) ldr_map", parent_size);
    }
    /* Allocate memory for gathering the ldr_rank */
    ldr_rank = MPL_malloc(sizeof(int) * parent_size, MPL_MEM_COLL);
    if (ldr_rank == NULL) {
        MPIR_ERR_SETANDJUMP2(mpi_errno, MPI_ERR_OTHER, "**nomem",
                             "**nomem %s %d", "(int) ldr_rank", parent_size);
        MPIR_ERR_POP(mpi_errno);
    }

    /* This is color of current process */
    mpi_errno =
        MPIR_Allgather_impl((void *)&in_color, 1, MPI_INT, (void *)color_array,
                            1, MPI_INT, parent_comm_ptr, &errflag);
    MPIR_ERR_CHECK(mpi_errno);

    /* This is color of current process */
    in_color = color_array[parent_rank];
    grp_index = 0;

    for (i = 0; i < parent_size; ++i) {
        if ((parent_rank == i) || (in_color == color_array[i])) {
            shmem_group[grp_index] = i;
            if (parent_rank == i) {
                rank_in_new_comm = grp_index;
                break;
            }
            ++grp_index;
        }
    }
    /* Create the intra-domain communicator */
    /* Need to disable device collectives, since this will trigger our MVAPICH
     * collectives which are not fully configured yet.
     */
    tmp_cvar = MPIR_CVAR_ALLGATHER_DEVICE_COLLECTIVE;
    MPIR_CVAR_ALLGATHER_DEVICE_COLLECTIVE = 0;
    mpi_errno = MPIR_Comm_split_impl(parent_comm_ptr, in_color,
                                     rank_in_new_comm, &comm_ptr);
    MPIR_ERR_CHECK(mpi_errno);
    MPIR_CVAR_ALLGATHER_DEVICE_COLLECTIVE = tmp_cvar;
    *comm = comm_ptr->handle;

    /* Get the intra-domain communicator rank */
    comm_rank = MPIR_Comm_rank(comm_ptr);
    /* Get the intra-domain communicator size */
    comm_size = MPIR_Comm_size(comm_ptr);

    /* Get shmem region for communicator */
    if (comm_rank == 0 && comm_size > 1) {
        lock_shmem_region();
        mvp_shmem_coll_blk_stat = MPIR_MVP_SHMEM_Coll_get_free_block();
        if (mvp_shmem_coll_blk_stat >= 0) {
            input_flag = 1;
        }
        unlock_shmem_region();
    } else {
        input_flag = 1;
    }
    /* Let all intra-parent-comm processes know the result */
    mpi_errno = MPIR_Allreduce_impl(&input_flag, &output_flag, 1, MPI_INT,
                                    MPI_LAND, parent_comm_ptr, &errflag);
    MPIR_ERR_CHECK(mpi_errno);

    if (!output_flag) {
        /* None of the shmem-coll-blocks are available. We cannot support
         * shared-memory collectives for this communicator */
        if ((comm_rank == 0) && (mvp_shmem_coll_blk_stat >= 0)) {
            /* Relese the slot if it is acquired */
            lock_shmem_region();
            MPIR_MVP_SHMEM_Coll_Block_Clear_Status(mvp_shmem_coll_blk_stat);
            unlock_shmem_region();
        }
        comm_ptr->dev.ch.shmem_coll_ok = 0;
        mpi_errno = MPI_ERR_INTERN;
        MPIR_ERR_POP(mpi_errno);
    } else if (comm_size > 1) {
        /* To prevent Bcast taking the knomial_2level_bcast route */
        mpi_errno = MPIR_Bcast_impl(&mvp_shmem_coll_blk_stat, 1, MPI_INT, 0,
                                    comm_ptr, &errflag);
        MPIR_ERR_CHECK(mpi_errno);

        comm_ptr->dev.ch.shmem_comm_rank = mvp_shmem_coll_blk_stat;
        PRINT_DEBUG(DEBUG_SHM_verbose,
                    "parent rank: %d, in_color: %d, "
                    "local rank: %d, mvp_shmem_coll_blk_stat = %d\n",
                    parent_comm_ptr->rank, in_color, comm_ptr->rank,
                    mvp_shmem_coll_blk_stat);
        if (MVP_USE_SLOT_SHMEM_COLL && comm_ptr->local_size > 1) {
            comm_ptr->dev.ch.shmem_info =
                mvp_shm_coll_init(mvp_shmem_coll_blk_stat, comm_ptr->rank,
                                  comm_ptr->local_size, comm_ptr);
            if (comm_ptr->dev.ch.shmem_info == NULL) {
                mpi_errno = MPIR_Err_create_code(
                    MPI_SUCCESS, MPI_ERR_OTHER, __func__, __LINE__,
                    MPI_ERR_OTHER, "**fail", "%s: %s",
                    "collective shmem allocation failed", strerror(errno));
                MPIR_ERR_POP(mpi_errno);
            }
        }
    }

    /* Creating leader group */
    leader = shmem_group[0];
    /* This is color of current process */
    mpi_errno =
        MPIR_Allgather_impl((void *)&leader, 1, MPI_INT, (void *)ldr_map, 1,
                            MPI_INT, parent_comm_ptr, &errflag);
    MPIR_ERR_CHECK(mpi_errno);

    /* Check if there is only going to be one leader */
    for (i = 0; i < parent_size; ++i) {
        if (ldr_map[i] != leader) {
            break;
        }
    }
    if (i == parent_size) {
        /* We only have one domain. There is no need to create a leader
         * communicator. Exit. */
        goto fn_exit;
    }
    /* Initialize array */
    for (i = 0; i < parent_size; ++i) {
        ldr_rank[i] = -1;
    }
    /* Find out rank within the leader_comm */
    grp_index = 0;
    for (i = 0; i < parent_size; ++i) {
        if (ldr_rank[(ldr_map[i])] == -1) {
            ldr_rank[(ldr_map[i])] = grp_index;
            ldr_group[grp_index++] = ldr_map[i];
        }
    }

    mpi_errno = MPIR_Comm_group_impl(parent_comm_ptr, &comm_group);
    MPIR_ERR_CHECK(mpi_errno);

    mpi_errno =
        MPIR_Group_incl_impl(comm_group, grp_index, ldr_group, &subgroup1);
    MPIR_ERR_CHECK(mpi_errno);

    mpi_errno =
        MPIR_Comm_create_intra(parent_comm_ptr, subgroup1, &ldr_comm_ptr);
    MPIR_ERR_CHECK(mpi_errno);

    if (ldr_comm_ptr != NULL) {
        /* Set leader_ptr's shmem_coll_ok so that we dont call
         * create_2level_comm on it again */
        *ldr_comm = ldr_comm_ptr->handle;
        ldr_comm_ptr->dev.ch.shmem_coll_ok = -1;
        PRINT_DEBUG(DEBUG_SHM_verbose,
                    "Created leader_comm. rank: %d, "
                    "size: %d\n",
                    ldr_comm_ptr->rank, ldr_comm_ptr->local_size);
    } else {
        if (comm_group) {
            mpi_errno = MPIR_Group_free_impl(subgroup1);
            MPIR_ERR_CHECK(mpi_errno);
        }
    }

    mpi_errno = MPIR_Group_free_impl(comm_group);
    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    if (ldr_group) {
        MPL_free(ldr_group);
        ldr_group = NULL;
    }
    if (ldr_rank) {
        MPL_free(ldr_rank);
        ldr_rank = NULL;
    }
    if (ldr_map) {
        MPL_free(ldr_map);
        ldr_map = NULL;
    }
    if (color_array) {
        MPL_free(color_array);
        color_array = NULL;
    }
    if (shmem_group) {
        MPL_free(shmem_group);
        shmem_group = NULL;
    }
    return (mpi_errno);

fn_fail:
    goto fn_exit;
}

static inline int create_intra_node_multi_level_topo_comm(MPIR_Comm *shmem_ptr)
{
    int i = 0;
    int color = 0;
    int mpi_errno = MPI_SUCCESS;
    int shmem_rank = -1, shmem_size = -1;
    MPIR_Comm *parent_comm_ptr = NULL;

    shmem_rank = MPIR_Comm_rank(shmem_ptr);
    shmem_size = MPIR_Comm_size(shmem_ptr);

    /* Allocate memory for communicators */
    shmem_ptr->dev.ch.topo_comm = MPL_malloc(
        sizeof(MPI_Comm) * mvp_num_intra_node_comm_levels + 1, MPL_MEM_COLL);
    shmem_ptr->dev.ch.topo_leader_comm = MPL_malloc(
        sizeof(MPI_Comm) * mvp_num_intra_node_comm_levels + 1, MPL_MEM_COLL);

    if ((shmem_ptr->dev.ch.topo_comm == NULL) ||
        (shmem_ptr->dev.ch.topo_leader_comm == NULL)) {
        PRINT_ERROR("Could not allocate memory for communicators\n");
        MPIR_ERR_POP(mpi_errno);
    }

    /* First parent comm is shmem_comm */
    parent_comm_ptr = shmem_ptr;
    /* Initialize communicator objects */
    for (i = 0; i <= mvp_num_intra_node_comm_levels; ++i) {
        shmem_ptr->dev.ch.topo_comm[i] = MPI_COMM_NULL;
        shmem_ptr->dev.ch.topo_leader_comm[i] = MPI_COMM_NULL;
    }
    /* Create communicator objects */
    for (i = 0; i <= mvp_num_intra_node_comm_levels; ++i) {
        /* Find color to split comm */
        color = mvp_intra_node_cluster_at_level[i];
        PRINT_DEBUG(
            DEBUG_SHM_verbose,
            "Calling create_intra_node_topo_comm_at_level for level %d\n", i);
        /* Create intra-node topo-aware communicators */
        mpi_errno = create_intra_node_topo_comm_at_level(
            parent_comm_ptr, color, &(shmem_ptr->dev.ch.topo_comm[i]),
            &(shmem_ptr->dev.ch.topo_leader_comm[i]));

        if (mpi_errno != MPI_SUCCESS) {
            goto fn_fail;
        }

        if (shmem_ptr->dev.ch.topo_leader_comm[i] == MPI_COMM_NULL) {
            /* Rank did not need to go beyond this level */
            break;
        }
        /* Get next parent pointer */
        MPIR_Comm_get_ptr(shmem_ptr->dev.ch.topo_leader_comm[i],
                          parent_comm_ptr);
    }

fn_exit:
    return (mpi_errno);

fn_fail:
    goto fn_exit;
}

int create_intra_sock_comm(MPI_Comm comm)
{
    int socket_bound = -1;
    int numCoresSocket = 0;
    int numSocketsNode = 0;
    int *intra_socket_leader_map = NULL;
    int *intra_sock_leader_group = NULL;
    int intra_comm_rank = 0, intra_comm_size = 0;
    int intra_leader_comm_size = 0, intra_leader_comm_rank = 0;
    int ok_to_create_intra_sock_comm = 1, i = 0;
    int output_flag = 0;
    int my_local_size, my_local_id;
    int mpi_errno = MPI_SUCCESS;
    MPIR_Errflag_t errflag = MPIR_ERR_NONE;
    int global_size = 0;
    int global_socket_leader_cnt = 0;
    int *global_sock_leader_group = NULL;
    int *global_socket_leader_map = NULL;
    MPIR_Comm *comm_ptr = NULL;
    MPIR_Comm_get_ptr(comm, comm_ptr);
    MPI_Group subgroup2;
    MPIR_Group *group_ptr = NULL;
    MPIR_Comm *shmem_ptr = NULL;
    MPIR_Comm *intra_sock_commptr = NULL;
    MPIR_Comm_get_ptr(comm_ptr->dev.ch.shmem_comm, shmem_ptr);
    mpi_errno = PMPI_Comm_rank(comm_ptr->dev.ch.shmem_comm, &my_local_id);
    if (mpi_errno) {
        MPIR_ERR_POP(mpi_errno);
    }

    comm_ptr->dev.ch.use_intra_sock_comm = 0;
    mpi_errno = PMPI_Comm_size(comm_ptr->dev.ch.shmem_comm, &my_local_size);
    if (mpi_errno) {
        MPIR_ERR_POP(mpi_errno);
    }

    if (MVP_ENABLE_SOCKET_AWARE_COLLECTIVES) {
        MPI_Group comm_group1;
        int is_uniform;
        int err = get_numa_bound_info(&socket_bound, &numSocketsNode,
                                      &numCoresSocket, &is_uniform);
        int output_err = 0;
        mpi_errno = MPIR_Allreduce_impl(&err, &output_err, 1, MPI_INT, MPI_LOR,
                                        comm_ptr, &errflag);
        if (mpi_errno) {
            MPIR_ERR_POP(mpi_errno);
        }

        if (output_err != 0) {
            if (numSocketsNode > 1) {
                PRINT_INFO(
                    comm_ptr->rank == 0,
                    "Failed to get correct process to socket binding info."
                    "Proceeding by disabling socket aware collectives "
                    "support.");
            }
            return MPI_SUCCESS;
        }

        comm_ptr->dev.ch.my_sock_id = socket_bound;
        int intra_socket_leader_id = -1;
        int intra_socket_leader_cnt = 0;
        if (ok_to_create_intra_sock_comm) {
            /*Create communicator for intra socket communication*/
            mpi_errno = PMPI_Comm_split(comm_ptr->dev.ch.shmem_comm,
                                        socket_bound, my_local_id,
                                        &(shmem_ptr->dev.ch.intra_sock_comm));
            if (mpi_errno) {
                MPIR_ERR_POP(mpi_errno);
            }

            PMPI_Comm_rank(shmem_ptr->dev.ch.intra_sock_comm, &intra_comm_rank);
            PMPI_Comm_size(shmem_ptr->dev.ch.intra_sock_comm, &intra_comm_size);
            /* allocate a shared region for each socket */
            int mvp_shmem_coll_blk_stat = -1;
            int input_flag = 0;
            if (intra_comm_rank == 0) {
                lock_shmem_region();
                mvp_shmem_coll_blk_stat = MPIR_MVP_SHMEM_Coll_get_free_block();
                if (mvp_shmem_coll_blk_stat >= 0) {
                    input_flag = 1;
                }
                unlock_shmem_region();
            } else {
                input_flag = 1;
            }

            mpi_errno =
                MPIR_Allreduce_impl(&input_flag, &output_flag, 1, MPI_INT,
                                    MPI_LAND, comm_ptr, &errflag);

            if (mpi_errno) {
                MPIR_ERR_POP(mpi_errno);
            }

            if (!output_flag) {
                /* None of the shmem-coll-blocks are available. We cannot
                   support shared-memory collectives for this communicator */
                PRINT_DEBUG(DEBUG_SHM_verbose > 1,
                            "Not enough shared memory regions."
                            " Cannot support socket aware collectives\n");
                if (intra_comm_rank == 0 && mvp_shmem_coll_blk_stat >= 0) {
                    /*release the slot if it is aquired */
                    lock_shmem_region();
                    MPIR_MVP_SHMEM_Coll_Block_Clear_Status(
                        mvp_shmem_coll_blk_stat);
                    unlock_shmem_region();
                }
                goto fn_fail;
            }

            MPIR_Comm_get_ptr(shmem_ptr->dev.ch.intra_sock_comm,
                              intra_sock_commptr);

            mpi_errno = MPIR_Bcast_impl(&mvp_shmem_coll_blk_stat, 1, MPI_INT, 0,
                                        intra_sock_commptr, &errflag);
            if (mpi_errno) {
                MPIR_ERR_POP(mpi_errno);
            }

            intra_sock_commptr->dev.ch.shmem_comm_rank =
                mvp_shmem_coll_blk_stat;
            if (MVP_USE_SLOT_SHMEM_COLL) {
                intra_sock_commptr->dev.ch.shmem_info = mvp_shm_coll_init(
                    mvp_shmem_coll_blk_stat, intra_sock_commptr->rank,
                    intra_sock_commptr->local_size, intra_sock_commptr);
                if (intra_sock_commptr->dev.ch.shmem_info == NULL) {
                    mpi_errno = MPIR_Err_create_code(
                        MPI_SUCCESS, MPI_ERR_OTHER, __func__, __LINE__,
                        MPI_ERR_OTHER, "**fail", "%s: %s",
                        "collective shmem allocation failed", strerror(errno));
                    MPIR_ERR_POP(mpi_errno);
                }
            }
            if (intra_comm_rank == 0) {
                intra_socket_leader_id = 1;
            }

            /*Creating intra-socket leader group*/
            intra_socket_leader_map =
                MPL_malloc(sizeof(int) * my_local_size, MPL_MEM_COLL);
            if (NULL == intra_socket_leader_map) {
                mpi_errno = MPIR_Err_create_code(
                    MPI_SUCCESS, MPI_ERR_OTHER, __func__, __LINE__,
                    MPI_ERR_OTHER, "**fail", "%s: %s",
                    "memory allocation failed", strerror(errno));
                MPIR_ERR_POP(mpi_errno);
            }

            /*initialize the intra_socket_leader_map*/
            memset(intra_socket_leader_map, -1, global_size);

            mpi_errno = MPIR_Allgather_impl(&intra_socket_leader_id, 1, MPI_INT,
                                            intra_socket_leader_map, 1, MPI_INT,
                                            shmem_ptr, &errflag);
            if (mpi_errno) {
                MPIR_ERR_POP(mpi_errno);
            }

            for (i = 0; i < my_local_size; i++) {
                if (intra_socket_leader_map[i] == 1)
                    intra_socket_leader_cnt++;
            }

            intra_sock_leader_group =
                MPL_malloc(sizeof(int) * intra_socket_leader_cnt, MPL_MEM_COLL);
            if (NULL == intra_sock_leader_group) {
                mpi_errno = MPIR_Err_create_code(
                    MPI_SUCCESS, MPI_ERR_OTHER, __func__, __LINE__,
                    MPI_ERR_OTHER, "**fail", "%s: %s",
                    "memory allocation failed", strerror(errno));
                MPIR_ERR_POP(mpi_errno);
            }

            /*Assuming homogeneous system, where every socket has same number
              of cores */
            int j = 0;
            for (i = 0; i < my_local_size; i++) {
                if (intra_socket_leader_map[i] == 1) {
                    /*i here actually is the my_local_id for which
                       intra_sock_rank == 0*/
                    intra_sock_leader_group[j++] = i;
                }
            }

            /*Resuing comm_group and subgroup1 variables for creation of
              intra socket leader comm*/
            mpi_errno =
                PMPI_Comm_group(comm_ptr->dev.ch.shmem_comm, &comm_group1);
            if (mpi_errno) {
                MPIR_ERR_POP(mpi_errno);
            }

            mpi_errno = PMPI_Group_incl(comm_group1, intra_socket_leader_cnt,
                                        intra_sock_leader_group, &subgroup2);
            if (mpi_errno) {
                MPIR_ERR_POP(mpi_errno);
            }

            /*Creating intra_sock_leader communicator*/
            mpi_errno =
                PMPI_Comm_create(comm_ptr->dev.ch.shmem_comm, subgroup2,
                                 &(shmem_ptr->dev.ch.intra_sock_leader_comm));
            if (mpi_errno) {
                MPIR_ERR_POP(mpi_errno);
            }

            if (intra_comm_rank == 0) {
                mpi_errno =
                    PMPI_Comm_rank(shmem_ptr->dev.ch.intra_sock_leader_comm,
                                   &intra_leader_comm_rank);
                if (mpi_errno) {
                    MPIR_ERR_POP(mpi_errno);
                }
                mpi_errno =
                    PMPI_Comm_size(shmem_ptr->dev.ch.intra_sock_leader_comm,
                                   &intra_leader_comm_size);
                if (mpi_errno) {
                    MPIR_ERR_POP(mpi_errno);
                }
            }

            /*Check if all the data in sockets are of uniform size*/
            if (intra_comm_rank == 0) {
                int array_index = 0;

                shmem_ptr->dev.ch.socket_size = MPL_malloc(
                    sizeof(int) * intra_leader_comm_size, MPL_MEM_COLL);
                mpi_errno = PMPI_Allgather(
                    &intra_comm_size, 1, MPI_INT, shmem_ptr->dev.ch.socket_size,
                    1, MPI_INT, shmem_ptr->dev.ch.intra_sock_leader_comm);
                if (mpi_errno) {
                    MPIR_ERR_POP(mpi_errno);
                }
                shmem_ptr->dev.ch.is_socket_uniform = 1;
                for (array_index = 0; array_index < intra_leader_comm_size;
                     array_index++) {
                    if (shmem_ptr->dev.ch.socket_size[0] !=
                        shmem_ptr->dev.ch.socket_size[array_index]) {
                        shmem_ptr->dev.ch.is_socket_uniform = 0;
                        break;
                    }
                }
            }

            MPIR_Group_get_ptr(subgroup2, group_ptr);
            if (group_ptr != NULL) {
                mpi_errno = PMPI_Group_free(&subgroup2);
                if (mpi_errno) {
                    MPIR_ERR_POP(mpi_errno);
                }
            }
            mpi_errno = PMPI_Group_free(&comm_group1);
            if (mpi_errno) {
                MPIR_ERR_POP(mpi_errno);
            }

            /* create comm between all the sock leaders across all nodes */
            mpi_errno = PMPI_Comm_size(comm, &global_size);
            if (mpi_errno) {
                MPIR_ERR_POP(mpi_errno);
            }

            global_socket_leader_map =
                MPL_malloc(sizeof(int) * global_size, MPL_MEM_COLL);
            if (NULL == global_socket_leader_map) {
                mpi_errno = MPIR_Err_create_code(
                    MPI_SUCCESS, MPI_ERR_OTHER, __func__, __LINE__,
                    MPI_ERR_OTHER, "**fail", "%s: %s",
                    "memory allocation failed", strerror(errno));
                MPIR_ERR_POP(mpi_errno);
            }
            /* initialize the global_socket_leader_map */
            memset(global_socket_leader_map, -1, global_size);
            mpi_errno = MPIR_Allgather_impl(&intra_socket_leader_id, 1, MPI_INT,
                                            global_socket_leader_map, 1,
                                            MPI_INT, comm_ptr, &errflag);
            if (mpi_errno) {
                MPIR_ERR_POP(mpi_errno);
            }

            for (i = 0; i < global_size; i++) {
                if (global_socket_leader_map[i] == 1)
                    global_socket_leader_cnt++;
            }

            global_sock_leader_group = MPL_malloc(
                sizeof(int) * global_socket_leader_cnt, MPL_MEM_COLL);
            if (NULL == global_sock_leader_group) {
                mpi_errno = MPIR_Err_create_code(
                    MPI_SUCCESS, MPI_ERR_OTHER, __func__, __LINE__,
                    MPI_ERR_OTHER, "**fail", "%s: %s",
                    "memory allocation failed", strerror(errno));
                MPIR_ERR_POP(mpi_errno);
            }

            /* Create the list of global sock leaders ranks */
            j = 0;
            for (i = 0; i < global_size; i++) {
                if (global_socket_leader_map[i] == 1)
                    global_sock_leader_group[j++] = i;
            }

            mpi_errno = PMPI_Comm_group(comm, &comm_group1);
            if (mpi_errno) {
                MPIR_ERR_POP(mpi_errno);
            }

            mpi_errno = PMPI_Group_incl(comm_group1, global_socket_leader_cnt,
                                        global_sock_leader_group, &subgroup2);
            if (mpi_errno) {
                MPIR_ERR_POP(mpi_errno);
            }

            /*Creating global_sock_leader communicator*/
            mpi_errno = PMPI_Comm_create(
                comm, subgroup2, &(comm_ptr->dev.ch.global_sock_leader_comm));
            if (mpi_errno) {
                MPIR_ERR_POP(mpi_errno);
            }

            MPIR_Group_get_ptr(subgroup2, group_ptr);
            if (group_ptr != NULL) {
                mpi_errno = PMPI_Group_free(&subgroup2);
                if (mpi_errno) {
                    MPIR_ERR_POP(mpi_errno);
                }
            }
            mpi_errno = PMPI_Group_free(&comm_group1);
            if (mpi_errno) {
                MPIR_ERR_POP(mpi_errno);
            }

            comm_ptr->dev.ch.use_intra_sock_comm = 1;

            MPIR_Comm *global_sock_leader_ptr;
            MPIR_Comm *intra_sock_leader_comm_ptr = NULL;

#if defined(_SHARP_SUPPORT_)
            MPIR_Comm *intra_sock_comm_ptr = NULL;
            MPIR_Comm_get_ptr(shmem_ptr->dev.ch.intra_sock_comm,
                              intra_sock_comm_ptr);
#endif

            MPIR_Comm_get_ptr(shmem_ptr->dev.ch.intra_sock_leader_comm,
                              intra_sock_leader_comm_ptr);
            MPIR_Comm_get_ptr(comm_ptr->dev.ch.global_sock_leader_comm,
                              global_sock_leader_ptr);
#if defined(_SHARP_SUPPORT_)
            if (global_sock_leader_ptr != NULL) {
                global_sock_leader_ptr->dev.ch.sharp_coll_info = NULL;
            }
            if (intra_sock_comm_ptr != NULL) {
                intra_sock_comm_ptr->dev.ch.sharp_coll_info = NULL;
            }
            if (intra_sock_leader_comm_ptr != NULL) {
                intra_sock_leader_comm_ptr->dev.ch.sharp_coll_info = NULL;
            }
#endif

            if (intra_comm_rank == 0) {
                if (intra_sock_leader_comm_ptr != NULL &&
                    intra_sock_leader_comm_ptr->dev.ch.shmem_coll_ok == 0) {
                    intra_sock_leader_comm_ptr->dev.ch
                        .tried_to_create_leader_shmem = 1;
                    mpi_errno = create_2level_comm(
                        shmem_ptr->dev.ch.intra_sock_leader_comm,
                        intra_sock_leader_comm_ptr->local_size,
                        intra_sock_leader_comm_ptr->rank);
                    if (mpi_errno == MPI_SUCCESS &&
                        intra_sock_leader_comm_ptr->local_size == 1) {
                        intra_sock_leader_comm_ptr->dev.ch.shmem_coll_ok = 1;
                    }
                }

                int local_leader_shmem_status =
                    intra_sock_leader_comm_ptr->dev.ch.shmem_coll_ok;
                int global_leader_shmem_status = 0;
                int allred_flag = MVP_USE_SOCKET_AWARE_ALLREDUCE;
                MVP_USE_SOCKET_AWARE_ALLREDUCE = 0;
                mpi_errno = MPIR_Allreduce_impl(
                    &local_leader_shmem_status, &global_leader_shmem_status, 1,
                    MPI_INT, MPI_MIN, global_sock_leader_ptr, &errflag);
                MVP_USE_SOCKET_AWARE_ALLREDUCE = allred_flag;
                if (mpi_errno) {
                    MPIR_ERR_POP(mpi_errno);
                }
                if (global_leader_shmem_status != 1) {
                    intra_sock_leader_comm_ptr->dev.ch.shmem_coll_ok = 0;
                }
            }
        }
    }

fn_exit:
    if (intra_socket_leader_map != NULL) {
        MPL_free(intra_socket_leader_map);
    }
    if (intra_sock_leader_group != NULL) {
        MPL_free(intra_sock_leader_group);
    }
    if (global_socket_leader_map != NULL) {
        MPL_free(global_socket_leader_map);
    }
    if (global_sock_leader_group != NULL) {
        MPL_free(global_sock_leader_group);
    }
    return mpi_errno;
fn_fail:
    free_intra_sock_comm(comm_ptr);
    goto fn_exit;
}

int create_allgather_comm(MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;
    int is_contig = 1, check_leader = 1, check_size = 1;
    int is_local_ok = 0, is_block = 0;
    int i = 0;
    int leader_rank = -1, leader_comm_size = -1;
    int size = comm_ptr->local_size;
    int my_rank = comm_ptr->rank;
    int my_node;
    int my_local_id = -1, my_local_size = -1;
    int grp_index = 0, leader = 0;
    MPIR_Comm *shmem_ptr = NULL;
    MPIR_Comm *leader_ptr = NULL;
    MPIR_Group *allgather_group = NULL, *comm_group = NULL;
    MPIR_Comm *allgather_comm = NULL;
    comm_ptr->dev.ch.allgather_comm = MPI_COMM_NULL;
    comm_ptr->dev.ch.allgather_new_ranks = NULL;

    if (comm_ptr->dev.ch.leader_comm != MPI_COMM_NULL) {
        MPIR_Comm_get_ptr(comm_ptr->dev.ch.leader_comm, leader_ptr);
        leader_rank = MPIR_Comm_rank(leader_ptr);
        leader_comm_size = MPIR_Comm_size(leader_ptr);
    }
    if (comm_ptr->dev.ch.shmem_comm != MPI_COMM_NULL) {
        MPIR_Comm_get_ptr(comm_ptr->dev.ch.shmem_comm, shmem_ptr);
        my_local_id = MPIR_Comm_rank(shmem_ptr);
        my_local_size = MPIR_Comm_size(shmem_ptr);
    }

    int *shmem_group = MPL_malloc(sizeof(int) * size, MPL_MEM_COLL);
    if (NULL == shmem_group) {
        mpi_errno = MPIR_Err_create_code(
            MPI_SUCCESS, MPI_ERR_OTHER, __func__, __LINE__, MPI_ERR_OTHER,
            "**fail", "%s: %s", "memory allocation failed", strerror(errno));
        MPIR_ERR_POP(mpi_errno);
    }

    MPID_Get_node_id(comm_ptr, my_rank, &my_node);
    for (i = 0; i < size; ++i) {
        int remote_node;
        MPID_Get_node_id(comm_ptr, i, &remote_node);
#if CHANNEL_NEMESIS_IB
        MPIDI_VC_t *vc = NULL;
        MPIDI_Comm_get_vc(comm_ptr, i, &vc);
        if (my_rank == i || vc->ch.is_local)
#else
        if (my_rank == i || my_node == remote_node)
#endif
        {
            shmem_group[grp_index++] = i;
        }
    }
    leader = shmem_group[0];

    mpi_errno = MPIR_Comm_group_impl(comm_ptr, &comm_group);
    MPIR_ERR_CHECK(mpi_errno);

    mpi_errno =
        MPIR_Bcast_impl(&leader_rank, 1, MPI_INT, 0, shmem_ptr, errflag);
    MPIR_ERR_CHECK(mpi_errno);

    for (i = 1; i < my_local_size; i++) {
        if (shmem_group[i] != shmem_group[i - 1] + 1) {
            is_contig = 0;
            break;
        }
    }

    if (leader != (my_local_size * leader_rank)) {
        check_leader = 0;
    }

    if (my_local_size != (size / comm_ptr->dev.ch.leader_group_size)) {
        check_size = 0;
    }

    is_local_ok = is_contig && check_leader && check_size;

    mpi_errno = MPIR_Allreduce_impl(&is_local_ok, &is_block, 1, MPI_INT,
                                    MPI_LAND, comm_ptr, errflag);
    MPIR_ERR_CHECK(mpi_errno);

    if (is_block) {
        int counter = 0, j;
        comm_ptr->dev.ch.allgather_new_ranks =
            MPL_malloc(sizeof(int) * size, MPL_MEM_COLL);
        if (NULL == comm_ptr->dev.ch.allgather_new_ranks) {
            mpi_errno = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE,
                                             __func__, __LINE__, MPI_ERR_OTHER,
                                             "**nomem", 0);
            return mpi_errno;
        }

        for (j = 0; j < my_local_size; j++) {
            for (i = 0; i < comm_ptr->dev.ch.leader_group_size; i++) {
                comm_ptr->dev.ch.allgather_new_ranks[counter] =
                    j + i * my_local_size;
                counter++;
            }
        }

        mpi_errno = MPIR_Group_incl_impl(comm_group, size,
                                         comm_ptr->dev.ch.allgather_new_ranks,
                                         &allgather_group);
        MPIR_ERR_CHECK(mpi_errno);
        mpi_errno =
            MPIR_Comm_create_intra(comm_ptr, allgather_group, &allgather_comm);
        MPIR_ERR_CHECK(mpi_errno);
        if (allgather_comm) {
            comm_ptr->dev.ch.allgather_comm = allgather_comm->handle;
            comm_ptr->dev.ch.allgather_comm_ok = 1;
        } else {
            mpi_errno = MPIR_Err_create_code(
                MPI_SUCCESS, MPI_ERR_OTHER, __func__, __LINE__, MPI_ERR_OTHER,
                "**fail", "%s: %s", "Failed to create allgather comm",
                strerror(errno));
            MPIR_ERR_POP(mpi_errno);
        }

        mpi_errno = MPIR_Group_free_impl(allgather_group);
        MPIR_ERR_CHECK(mpi_errno);
    } else {
        /* Set this to -1 so that we never get back in here again
         * for this cyclic comm */
        comm_ptr->dev.ch.allgather_comm_ok = -1;
    }

    /* Gives the mapping to any process's leader in comm */
    comm_ptr->dev.ch.rank_list = MPL_malloc(sizeof(int) * size, MPL_MEM_COLL);
    if (NULL == comm_ptr->dev.ch.rank_list) {
        mpi_errno = MPIR_Err_create_code(
            MPI_SUCCESS, MPI_ERR_OTHER, __func__, __LINE__, MPI_ERR_OTHER,
            "**fail", "%s: %s", "memory allocation failed", strerror(errno));
        MPIR_ERR_POP(mpi_errno);
    }

    /* gather full rank list on leader processes, the rank list is ordered
     * by node based on leader rank, and then by rank within the node according
     * to the shmem_group list */
    if (my_local_id == 0) {
        /* execute allgather or allgatherv across leaders */
        if (comm_ptr->dev.ch.is_uniform != 1) {
            /* allocate memory for displacements and counts */
            int *displs =
                MPL_malloc(sizeof(int) * leader_comm_size, MPL_MEM_COLL);
            int *counts =
                MPL_malloc(sizeof(int) * leader_comm_size, MPL_MEM_COLL);
            if (!displs || !counts) {
                mpi_errno = MPIR_Err_create_code(
                    MPI_SUCCESS, MPIR_ERR_RECOVERABLE, __func__, __LINE__,
                    MPI_ERR_OTHER, "**nomem", 0);
                return mpi_errno;
            }

            /* get pointer to array of node sizes */
            int *node_sizes = comm_ptr->dev.ch.node_sizes;

            /* compute values for displacements and counts arrays */
            displs[0] = 0;
            counts[0] = node_sizes[0];
            for (i = 1; i < leader_comm_size; i++) {
                displs[i] = displs[i - 1] + node_sizes[i - 1];
                counts[i] = node_sizes[i];
            }

            /* execute the allgatherv to collect full rank list */
            mpi_errno = MPIR_Allgatherv_impl(
                shmem_group, my_local_size, MPI_INT, comm_ptr->dev.ch.rank_list,
                counts, displs, MPI_INT, leader_ptr, errflag);

            /* free displacements and counts arrays */
            MPL_free(displs);
            MPL_free(counts);
        } else {
            /* execute the allgather to collect full rank list */
            mpi_errno = MPIR_Allgather_impl(
                shmem_group, my_local_size, MPI_INT, comm_ptr->dev.ch.rank_list,
                my_local_size, MPI_INT, leader_ptr, errflag);
        }
        MPIR_ERR_CHECK(mpi_errno);
    }

    /* broadcast rank list to other ranks on this node */
    mpi_errno = MPIR_Bcast_impl(comm_ptr->dev.ch.rank_list, size, MPI_INT, 0,
                                shmem_ptr, errflag);
    MPIR_ERR_CHECK(mpi_errno);

    /* lookup and record our index within the rank list */
    for (i = 0; i < size; i++) {
        if (my_rank == comm_ptr->dev.ch.rank_list[i]) {
            /* found ourself in the list, record the index */
            comm_ptr->dev.ch.rank_list_index = i;
            break;
        }
    }

    mpi_errno = MPIR_Group_free_impl(comm_group);
    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    MPL_free(shmem_group);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#if defined(_MCST_SUPPORT_)
int create_mcast_comm(MPI_Comm comm, int size, int my_rank)
{
    int mpi_errno = MPI_SUCCESS;
    int mcast_setup_success = 0;
    int leader_group_size = 0, my_local_id = -1;
    MPIR_Comm *comm_ptr = NULL;
    MPIR_Comm *shmem_ptr = NULL;
    MPIR_Comm *leader_ptr = NULL;
    MPIR_Errflag_t errflag = MPIR_ERR_NONE;

    if (size <= 1) {
        return mpi_errno;
    }

    MPIR_Comm_get_ptr(comm, comm_ptr);
    MPIR_Comm_get_ptr(comm_ptr->dev.ch.shmem_comm, shmem_ptr);
    MPIR_Comm_get_ptr(comm_ptr->dev.ch.leader_comm, leader_ptr);

    mpi_errno = PMPI_Comm_rank(comm_ptr->dev.ch.shmem_comm, &my_local_id);
    if (mpi_errno) {
        MPIR_ERR_POP(mpi_errno);
    }

    leader_group_size = comm_ptr->dev.ch.leader_group_size;

    comm_ptr->dev.ch.is_mcast_ok = 0;
    bcast_info_t **bcast_info = (bcast_info_t **)&comm_ptr->dev.ch.bcast_info;
    if (leader_group_size >= mcast_num_nodes_threshold && rdma_enable_mcast) {
        mvp_mcast_init_bcast_info(bcast_info);
        if (my_local_id == 0) {
            if (mvp_setup_multicast(&(*bcast_info)->minfo, comm_ptr) ==
                MCAST_SUCCESS) {
                mcast_setup_success = 1;
            }
        }

        int leader_rank;
        int status = 0;
        int mcast_status[2] = {0, 0}; /* status, comm_id */
        if (comm_ptr->dev.ch.leader_comm != MPI_COMM_NULL) {
            PMPI_Comm_rank(comm_ptr->dev.ch.leader_comm, &leader_rank);
            if (leader_rank == 0 && mcast_setup_success) {
                /* Wait for comm ready */
                status = mvp_mcast_progress_comm_ready(comm_ptr);
            }
        }

        if (my_local_id == 0) {
            mpi_errno =
                MPIR_Bcast_impl(&status, 1, MPI_INT, 0, leader_ptr, &errflag);
            if (mpi_errno) {
                MPIR_ERR_POP(mpi_errno);
            }
            mcast_status[0] = status;
            mcast_status[1] = ((bcast_info_t *)comm_ptr->dev.ch.bcast_info)
                                  ->minfo.grp_info.comm_id;
            if (!status) {
                mvp_cleanup_multicast(
                    &((bcast_info_t *)comm_ptr->dev.ch.bcast_info)->minfo,
                    comm_ptr);
            }
        }

        mpi_errno =
            MPIR_Bcast_impl(mcast_status, 2, MPI_INT, 0, shmem_ptr, &errflag);
        if (mpi_errno) {
            MPIR_ERR_POP(mpi_errno);
        }

        comm_ptr->dev.ch.is_mcast_ok = mcast_status[0];

        if (my_rank == 0) {
            PRINT_DEBUG(DEBUG_MCST_verbose > 1, "multicast setup status:%d\n",
                        comm_ptr->dev.ch.is_mcast_ok);
            if (comm_ptr->dev.ch.is_mcast_ok == 0) {
                PRINT_INFO(1, "Warning: Multicast group setup failed. Not "
                              "using any multicast features\n");
            }
        }

        if (comm_ptr->dev.ch.is_mcast_ok == 0 && comm == MPI_COMM_WORLD) {
            /* if mcast setup failed on comm world because of any reason, it is
            ** most likely is going to fail on other communicators. Hence,
            *disable
            ** the mcast feaure */
            rdma_enable_mcast = 0;
            mvp_ud_destroy_ctx(mcast_ctx->ud_ctx);
            MPL_free(mcast_ctx);
            PRINT_DEBUG(DEBUG_MCST_verbose,
                        "mcast setup failed on comm world, disabling mcast\n");
        }
    }
fn_exit:
    return (mpi_errno);
fn_fail:
    goto fn_exit;
}
#endif /*(_MCST_SUPPORT_)*/

int create_2level_comm(MPI_Comm comm, int size, int my_rank)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Comm *comm_ptr = NULL;
    MPIR_Comm *leader_ptr = NULL, *shmem_ptr = NULL;
    MPIR_Group *commgroup_ptr = NULL, *subgroup_ptr = NULL;
    int leader_comm_size, my_local_size, my_local_id;
    int input_flag = 0, output_flag = 0;
    MPIR_Errflag_t errflag = MPIR_ERR_NONE;
    int leader_group_size = 0;
    int mvp_shmem_coll_blk_stat = 0;
    int iter;
    int node_id;
    int blocked = 0;
    int up = 0;
    int prev = -1;
    int shmem_size;
    int local_topo_comms_ok = 0, global_topo_comms_ok = 0;

    MPIR_Comm_get_ptr(comm, comm_ptr);
    if (size <= 1) {
        return mpi_errno;
    }

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_num_2level_comm_requests, 1);

    /* Find out if ranks are block ordered locally */
    int my_node;
    MPID_Get_node_id(comm_ptr, my_rank, &my_node);
    for (iter = 0; iter < size; iter++) {
        MPID_Get_node_id(comm_ptr, iter, &node_id);
        if ((node_id == my_node) && (prev != my_node)) {
            up++;
            if (up > 1) {
                blocked = 0;
                break;
            }
        }
        prev = node_id;
    }

    int *shmem_group = MPL_malloc(sizeof(int) * size, MPL_MEM_COLL);
    if (NULL == shmem_group) {
        mpi_errno = MPIR_Err_create_code(
            MPI_SUCCESS, MPI_ERR_OTHER, __func__, __LINE__, MPI_ERR_OTHER,
            "**fail", "%s: %s", "memory allocation failed", strerror(errno));
        MPIR_ERR_POP(mpi_errno);
    }

    /*
     * TODO: sometimes we have what appears to be a 2level comm already setup.
     * We should look into the possibility of using these communicators instead
     * of configuring brand new ones if that would be possible.
     */
    /*
    if (comm_ptr->node_roots_comm && comm_ptr->node_comm) {
        comm_ptr->dev.ch.leader_comm = comm_ptr->node_roots_comm->handle;
        comm_ptr->dev.ch.shmem_comm = comm_ptr->node_comm->handle;
        leader_ptr = comm_ptr->node_roots_comm;
        shmem_ptr = comm_ptr->node_comm;
        PRINT_ERROR("We have Comms!\n");
    } else {
        PRINT_ERROR("No Comms?\n");
        MPIR_Assert(0);
    }
    */

    /* Creating local shmem group */
    int i = 0;
    int local_rank = 0;
    int grp_index = 0;
    comm_ptr->dev.ch.leader_comm = MPI_COMM_NULL;
    comm_ptr->dev.ch.shmem_comm = MPI_COMM_NULL;

    for (i = 0; i < size; ++i) {
        int remote_node;
        MPID_Get_node_id(comm_ptr, i, &remote_node);
#ifdef CHANNEL_NEMESIS_IB
        MPIDI_VC_t *vc = NULL;
        MPIDI_Comm_get_vc(comm_ptr, i, &vc);
        if (my_rank == i || vc->ch.is_local)
#else
        if (my_rank == i || my_node == remote_node)
#endif
        {
            shmem_group[grp_index] = i;
            if (my_rank == i) {
                local_rank = grp_index;
            }
            ++grp_index;
        }
    }
    shmem_size = grp_index;

    if (local_rank == 0) {
        lock_shmem_region();
        mvp_shmem_coll_blk_stat = MPIR_MVP_SHMEM_Coll_get_free_block();
        if (mvp_shmem_coll_blk_stat >= 0) {
            input_flag = 1;
        }
        unlock_shmem_region();
    } else {
        input_flag = 1;
    }

    mpi_errno = MPIR_Allreduce_impl(&input_flag, &output_flag, 1, MPI_INT,
                                    MPI_LAND, comm_ptr, &errflag);
    if (mpi_errno) {
        MPIR_ERR_POP(mpi_errno);
    }

    mpi_errno = MPIR_Allreduce_impl(&blocked, &(comm_ptr->dev.ch.is_blocked), 1,
                                    MPI_INT, MPI_LAND, comm_ptr, &errflag);
    if (mpi_errno) {
        MPIR_ERR_POP(mpi_errno);
    }

    if (!output_flag) {
        /* None of the shmem-coll-blocks are available. We cannot support
         * shared-memory collectives for this communicator */
        if (local_rank == 0 && mvp_shmem_coll_blk_stat >= 0) {
            /*relese the slot if it is aquired */
            lock_shmem_region();
            MPIR_MVP_SHMEM_Coll_Block_Clear_Status(mvp_shmem_coll_blk_stat);
            unlock_shmem_region();
        }
        comm_ptr->dev.ch.shmem_coll_ok = -1;
        goto fn_exit;
    }

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_num_2level_comm_success, 1);

    comm_ptr->dev.ch.coll_tmp_buf =
        MPL_malloc(MVP_COLL_TMP_BUF_SIZE, MPL_MEM_COLL);
    if (NULL == comm_ptr->dev.ch.coll_tmp_buf) {
        mpi_errno = MPIR_Err_create_code(
            MPI_SUCCESS, MPI_ERR_OTHER, __func__, __LINE__, MPI_ERR_OTHER,
            "**fail", "%s: %s", "memory allocation failed", strerror(errno));
        MPIR_ERR_POP(mpi_errno);
    }

    /* Creating leader group */
    int leader = 0;
    leader = shmem_group[0];

    /* Gives the mapping to any process's leader in comm */
    comm_ptr->dev.ch.leader_map = MPL_malloc(sizeof(int) * size, MPL_MEM_COLL);
    if (NULL == comm_ptr->dev.ch.leader_map) {
        mpi_errno = MPIR_Err_create_code(
            MPI_SUCCESS, MPI_ERR_OTHER, __func__, __LINE__, MPI_ERR_OTHER,
            "**fail", "%s: %s", "memory allocation failed", strerror(errno));
        MPIR_ERR_POP(mpi_errno);
    }

    mpi_errno =
        MPIR_Allgather_impl(&leader, 1, MPI_INT, comm_ptr->dev.ch.leader_map, 1,
                            MPI_INT, comm_ptr, &errflag);
    if (mpi_errno) {
        MPIR_ERR_POP(mpi_errno);
    }

    int *leader_group = MPL_malloc(sizeof(int) * size, MPL_MEM_COLL);
    if (NULL == leader_group) {
        mpi_errno = MPIR_Err_create_code(
            MPI_SUCCESS, MPI_ERR_OTHER, __func__, __LINE__, MPI_ERR_OTHER,
            "**fail", "%s: %s", "memory allocation failed", strerror(errno));
        MPIR_ERR_POP(mpi_errno);
    }

    /* Gives the mapping from leader's rank in comm to
     * leader's rank in leader_comm */
    comm_ptr->dev.ch.leader_rank = MPL_malloc(sizeof(int) * size, MPL_MEM_COLL);
    if (NULL == comm_ptr->dev.ch.leader_rank) {
        mpi_errno = MPIR_Err_create_code(
            MPI_SUCCESS, MPI_ERR_OTHER, __func__, __LINE__, MPI_ERR_OTHER,
            "**fail", "%s: %s", "memory allocation failed", strerror(errno));
        MPIR_ERR_POP(mpi_errno);
    }

    for (i = 0; i < size; ++i) {
        comm_ptr->dev.ch.leader_rank[i] = -1;
    }
    int *group = comm_ptr->dev.ch.leader_map;
    grp_index = 0;
    for (i = 0; i < size; ++i) {
        if (comm_ptr->dev.ch.leader_rank[(group[i])] == -1) {
            comm_ptr->dev.ch.leader_rank[(group[i])] = grp_index;
            leader_group[grp_index++] = group[i];
        }
    }
    leader_group_size = grp_index;
    comm_ptr->dev.ch.leader_group_size = leader_group_size;

    mpi_errno = MPIR_Comm_group_impl(comm_ptr, &commgroup_ptr);
    MPIR_ERR_CHECK(mpi_errno);

    mpi_errno = MPIR_Group_incl_impl(commgroup_ptr, leader_group_size,
                                     leader_group, &subgroup_ptr);
    MPIR_ERR_CHECK(mpi_errno);

    mpi_errno = MPIR_Comm_create_intra(comm_ptr, subgroup_ptr, &leader_ptr);
    MPIR_ERR_CHECK(mpi_errno);

    if (leader_ptr) {
        comm_ptr->dev.ch.leader_comm = leader_ptr->handle;
    } else {
        comm_ptr->dev.ch.leader_comm = MPI_COMM_NULL;
    }

    if (leader_ptr != NULL) {
        /* Set leader_ptr's shmem_coll_ok so that we dont call
         * create_2level_comm on it again */
        leader_ptr->dev.ch.shmem_coll_ok = -1;
    }

    MPL_free(leader_group);
    if (subgroup_ptr != NULL) {
        mpi_errno = MPIR_Group_free_impl(subgroup_ptr);
        MPIR_ERR_CHECK(mpi_errno);
    }

    mpi_errno = MPIR_Group_incl_impl(commgroup_ptr, shmem_size, shmem_group,
                                     &subgroup_ptr);
    MPIR_ERR_CHECK(mpi_errno);

    mpi_errno = MPIR_Comm_create_intra(comm_ptr, subgroup_ptr, &shmem_ptr);
    MPIR_ERR_CHECK(mpi_errno);

    if (shmem_ptr) {
        comm_ptr->dev.ch.shmem_comm = shmem_ptr->handle;
        /* Set shmem_ptr's shmem_coll_ok so that we dont call create_2level_comm
         * on it again */
        shmem_ptr->dev.ch.shmem_coll_ok = -1;
    } else {
        comm_ptr->dev.ch.shmem_comm = MPI_COMM_NULL;
    }

    my_local_id = MPIR_Comm_rank(shmem_ptr);
    my_local_size = MPIR_Comm_size(shmem_ptr);
    comm_ptr->dev.ch.intra_node_done = 0;

    if (my_local_id == 0) {
        int array_index = 0;
        leader_comm_size = MPIR_Comm_size(leader_ptr);

        comm_ptr->dev.ch.node_sizes =
            MPL_malloc(sizeof(int) * leader_comm_size, MPL_MEM_COLL);
        /* need to use MPICH implementations here since this sets up for ours */
        mpi_errno = MPIR_Allgather_impl(&my_local_size, 1, MPI_INT,
                                        comm_ptr->dev.ch.node_sizes, 1, MPI_INT,
                                        leader_ptr, &errflag);
        MPIR_ERR_CHECK(mpi_errno);

        /* allocate memory for displacements into rank_list */
        comm_ptr->dev.ch.node_disps =
            MPL_malloc(sizeof(int) * leader_comm_size, MPL_MEM_COLL);

        /* compute values for displacements and counts arrays */
        int *sizes = comm_ptr->dev.ch.node_sizes;
        int *disps = comm_ptr->dev.ch.node_disps;
        disps[0] = 0;
        for (i = 1; i < leader_comm_size; i++) {
            disps[i] = disps[i - 1] + sizes[i - 1];
        }

        comm_ptr->dev.ch.is_uniform = 1;
        for (array_index = 0; array_index < leader_comm_size; array_index++) {
            if (comm_ptr->dev.ch.node_sizes[0] !=
                comm_ptr->dev.ch.node_sizes[array_index]) {
                comm_ptr->dev.ch.is_uniform = 0;
                break;
            }
        }
    }

    comm_ptr->dev.ch.is_global_block = 0;
    /* We need to check to see if the ranks are block or not. Each node leader
     * gets the global ranks of all of its children processes. It scans through
     * this array to see if the ranks are in block order. The node-leaders then
     * do an allreduce to see if all the other nodes are also in block order.
     * This is followed by an intra-node bcast to let the children processes
     * know of the result of this step */
    if (my_local_id == 0) {
        int is_local_block = 1;
        int index = 1;

        while (index < my_local_size) {
            if ((shmem_group[index] - 1) != shmem_group[index - 1]) {
                is_local_block = 0;
                break;
            }
            index++;
        }

        mpi_errno = MPIR_Allreduce_impl(
            &(is_local_block), &(comm_ptr->dev.ch.is_global_block), 1, MPI_INT,
            MPI_LAND, leader_ptr, &errflag);
        if (mpi_errno) {
            MPIR_ERR_POP(mpi_errno);
        }
        mpi_errno = MPIR_Bcast_impl(&(comm_ptr->dev.ch.is_global_block), 1,
                                    MPI_INT, 0, shmem_ptr, &errflag);
        if (mpi_errno) {
            MPIR_ERR_POP(mpi_errno);
        }
    } else {
        mpi_errno = MPIR_Bcast_impl(&(comm_ptr->dev.ch.is_global_block), 1,
                                    MPI_INT, 0, shmem_ptr, &errflag);
        if (mpi_errno) {
            MPIR_ERR_POP(mpi_errno);
        }
    }
    /* set is_blocked too */
    comm_ptr->dev.ch.is_blocked = comm_ptr->dev.ch.is_global_block;

    /* bcast uniformity info to node local processes for tuning selection
       later */
    mpi_errno = MPIR_Bcast_impl(&(comm_ptr->dev.ch.is_uniform), 1, MPI_INT, 0,
                                shmem_ptr, &errflag);
    if (mpi_errno) {
        MPIR_ERR_POP(mpi_errno);
    }

    comm_ptr->dev.ch.allgather_comm_ok = 0;

    mpi_errno = MPIR_Group_free_impl(commgroup_ptr);
    MPIR_ERR_CHECK(mpi_errno);

    shmem_ptr->dev.ch.shmem_coll_ok = 0;
    /* To prevent Bcast taking the knomial_2level_bcast route */
    mpi_errno = MPIR_Bcast_impl(&mvp_shmem_coll_blk_stat, 1, MPI_INT, 0,
                                shmem_ptr, &errflag);
    MPIR_ERR_CHECK(mpi_errno);

    MPIR_Assert(mvp_shmem_coll_blk_stat >= 0);

    shmem_ptr->dev.ch.shmem_comm_rank = mvp_shmem_coll_blk_stat;

    if (MVP_USE_SLOT_SHMEM_COLL) {
        comm_ptr->dev.ch.shmem_info = mvp_shm_coll_init(
            mvp_shmem_coll_blk_stat, my_local_id, my_local_size, comm_ptr);
        if (comm_ptr->dev.ch.shmem_info == NULL) {
            mpi_errno = MPIR_Err_create_code(
                MPI_SUCCESS, MPI_ERR_OTHER, __func__, __LINE__, MPI_ERR_OTHER,
                "**fail", "%s: %s", "collective shmem allocation failed",
                strerror(errno));
            MPIR_ERR_POP(mpi_errno);
        }
        shmem_ptr->dev.ch.shmem_info = comm_ptr->dev.ch.shmem_info;
    }
    comm_ptr->dev.ch.shmem_coll_ok = 1;

#if defined(_SMP_LIMIC_)
    if (comm_ptr->dev.ch.shmem_coll_ok == 1) {
        mpi_errno = create_intra_node_multi_level_comm(comm_ptr);
        if (mpi_errno) {
            MPIR_ERR_POP(mpi_errno);
        }
    }
#endif /* #if defined(_SMP_LIMIC_) */

    if (MVP_ENABLE_TOPO_AWARE_COLLECTIVES) {
        if (comm_ptr->dev.ch.shmem_coll_ok == 1 && MVP_USE_SLOT_SHMEM_COLL) {
            mpi_errno = create_intra_node_multi_level_topo_comm(shmem_ptr);
            if (mpi_errno == MPI_SUCCESS) {
                local_topo_comms_ok = 1;
            }
            mpi_errno =
                MPIR_Allreduce_impl(&local_topo_comms_ok, &global_topo_comms_ok,
                                    1, MPI_INT, MPI_LAND, comm_ptr, &errflag);
            if (mpi_errno) {
                MPIR_ERR_POP(mpi_errno);
            }
            if (global_topo_comms_ok == 1) {
                comm_ptr->dev.ch.topo_coll_ok = 1;
            }
        }
    }
    if (MVP_ENABLE_SOCKET_AWARE_COLLECTIVES) {
        /* tried_to_create_leader_shmem exists to ensure socket-aware comms
         * aren't recursively created unnecessarily
         * (which would cause memory leaks)
         */
        if (comm_ptr->dev.ch.shmem_coll_ok == 1 &&
            comm_ptr->dev.ch.tried_to_create_leader_shmem == 0) {
            mpi_errno = create_intra_sock_comm(comm);
            if (mpi_errno) {
                MPIR_ERR_POP(mpi_errno);
            }
        }
    }

fn_exit:
    MPL_free(shmem_group);
    return (mpi_errno);
fn_fail:
    goto fn_exit;
}

int init_thread_reg(void)
{
    int j = 0;

    for (; j < MAX_NUM_THREADS; ++j) {
        thread_reg[j] = -1;
    }

    return 1;
}

int check_split_comm(pthread_t my_id)
{
    int j = 0;
    pthread_mutex_lock(&comm_lock);

    for (; j < MAX_NUM_THREADS; ++j) {
        if (pthread_equal(thread_reg[j], my_id)) {
            pthread_mutex_unlock(&comm_lock);
            return 0;
        }
    }

    pthread_mutex_unlock(&comm_lock);
    return 1;
}

int disable_split_comm(pthread_t my_id)
{
    int j = 0;
    int found = 0;
    int mpi_errno = MPI_SUCCESS;
    pthread_mutex_lock(&comm_lock);

    for (; j < MAX_NUM_THREADS; ++j) {
        if (thread_reg[j] == -1) {
            thread_reg[j] = my_id;
            found = 1;
            break;
        }
    }

    pthread_mutex_unlock(&comm_lock);

    if (found == 0) {
        mpi_errno = MPIR_Err_create_code(
            MPI_SUCCESS, MPIR_ERR_RECOVERABLE, __func__, __LINE__,
            MPI_ERR_OTHER, "**fail", "**fail %s", "max_num_threads created");
        return mpi_errno;
    }

    return 1;
}

int enable_split_comm(pthread_t my_id)
{
    int j = 0;
    int found = 0;
    int mpi_errno = MPI_SUCCESS;
    pthread_mutex_lock(&comm_lock);

    for (; j < MAX_NUM_THREADS; ++j) {
        if (pthread_equal(thread_reg[j], my_id)) {
            thread_reg[j] = -1;
            found = 1;
            break;
        }
    }

    pthread_mutex_unlock(&comm_lock);

    if (found == 0) {
        mpi_errno = MPIR_Err_create_code(
            MPI_SUCCESS, MPIR_ERR_RECOVERABLE, __func__, __LINE__,
            MPI_ERR_OTHER, "**fail", "**fail %s", "max_num_threads created");
        return mpi_errno;
    }

    return 1;
}
