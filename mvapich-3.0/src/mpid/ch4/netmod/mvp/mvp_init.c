/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda. For detailed
 * copyright and licensing information, please refer to the copyright file
 * COPYRIGHT in the top level MVAPICH directory.
 */

#include "mpidimpl.h"
#include "mvp_util.h"
#include "mvp_coll_shmem.h"
#include "hwloc_bind.h"
#include "mvp_abstraction_util.h"
#include "mvp_offload_coll_init.h"

/*
=== BEGIN_MPI_T_MVP_CVAR_INFO_BLOCK ===

categories:
    - name        : CH4
      description : INHERITED

cvars:
    - name        : MVP_SHOW_CPU_BINDING
      category    : CH4
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Possible values: 0, 1, 2
        If set to 1, prints the CPU mapping of all processes on the node where
        rank 0 exists. If set to 2, prints the CPU mapping of all processes on
        all nodes.

    - name        : MVP_SHOW_HCA_BINDING
      category    : CH4
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Possible values: 0, 1, 2
        If set to 1, prints the HCA mapping of all processes on the node where
        rank 0 exists. If set to 2, prints the HCA mapping of all processes on
        all nodes.

    - name        : MVP_SHOW_HW_BINDING
      category    : CH4
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Possible values: 0, 1, 2
        If set to 1, prints the CPU and HCA mappings of all processes on the
        node where rank 0 exists. If set to 2, prints the CPU and HCA mappings
        of all processes on all nodes.

    - name        : MVP_REPORT_LOAD_IMBALANCE
      category    : COLLECTIVE
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Reports load imbalance, the time it takes for a barrier call to
        synchronize all processes, for enabled collectives. Setting this
        variable to 1 will enable load imbalance reporting for all enabled
        collectives.

    - name        : MVP_BCAST_HWLOC_TOPOLOGY
      category    : CH4
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_SUPPORT_DPM
      category    : CH4
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This option enables the dynamic process management interface and
        on-demand connection management.

    - name        : MVP_PIVOT_CORE_ID
      category    : CH4
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_FORCE_ARCH_TYPE
      category    : CH4
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Default value 0 corresponds to MVP_ARCH_UNKWN.

    - name        : MVP_THREADS_PER_PROCESS
      category    : CH4
      type        : int
      default     : 1
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        TODO-DESC

    - name        : MVP_SHOW_RUNLOG_INFO
      category    : CH4
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_MPIDEV_DETAIL
      scope       : MPI_T_SCOPE_LOCAL
      description : >-
        TODO-DESC

    - name        : MVP_USE_ALIGNED_ALLOC
      category    : CH4
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
       TODO-DESC

    - name        : MVP_HYBRID_BINDING_POLICY
      category    : CH4
      type        : enum
      default     : SPREAD
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : |-
        Variable to select hybrid binding policy
        LINEAR      - Bind each MPI rank as well as its associated threads to
                    phyical cores. Only use hardware threads when you runout of
                    physical resources.
        COMPACT     - Bind each MPI rank to a single phyical core, and bind its
                    associated threads to the hardware threads of the same
                    physical core. Use first socket followed by the second
                    socket.
        SPREAD      - Evenly distributes all the PUs among MPI ranks and ensures
                    that no two MPI ranks get bound to the same physical core.
        BUNCH       - Bind each MPI rank to a single phyical core of first
                    socket followed by second secket.
        SCATTER     - Bind consecutive MPI ranks to different sockets in
                    round-robin fashion.
        NUMA        - Bind consecutive MPI ranks to different NUMA domains in
                    round-robin fashion.

    - name        : MVP_CPU_BINDING_POLICY
      category    : CH4
      type        : enum
      default     : HYBRID
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : |-
        Variable to select cpu binding policy (see MVP_HYBRID_BINDING_POLICY
        for default values of enums).
        BUNCH       - Allocates contiguous CPU cores to a process, allowing it
                    to maximize cache locality and reduce inter-core
                    communication overhead.
        SCATTER     - Distributes a process's threads across multiple
                    non-contiguous CPU cores, which can help balance the
                    workload and reduce contention.
        HYBRID      - Allows for contiguous and non-contiguous CPU core
                    allocation for optimal cache locality and load balancing.

    - name        : MVP_ENABLE_PVAR_MEM
      category    : CH4
      type        : int
      default     : 0
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
       TODO-DESC

    - name        : MVP_USE_BUCKET_FILE
      category    : CH4
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
       TODO-DESC

    - name        : MVP_RNDV_PROTOCOL
      category    : CH4
      type        : enum
      default     : RGET
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : |-
        The value of this variable can be set to choose different Rendezvous
        protocols. The default enum values are soley meant for implementors to
        stay consistent with MRAILI_Protocol_t.
        R3   = 2    - send/recv based
        RPUT = 3    - default RDMA-Write
        RGET = 4    - RDMA Read based

    - name        : MVP_SMP_RNDV_PROTOCOL
      category    : CH4
      type        : enum
      default     : RGET
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : |-
        (Note: This is the same as MVP_RNDV_PROTOCOL)
        The value of this variable can be set to choose different Rendezvous
        protocols. The default enum values are soley meant for implementors to
        stay consistent with MRAILI_Protocol_t.
        R3   = 2    - send/recv based
        RPUT = 3    - default RDMA-Write
        RGET = 4    - RDMA Read based

=== END_MPI_T_MVP_CVAR_INFO_BLOCK ===
*/

int mvp_network_init = 0;

int MPIDI_MVP_smp_init();
int MPIDI_MVP_smp_finalize();
int MPIR_MVP_SHMEM_COLL_Cleanup();
void mvp_init_debug();

int MPIDI_MVP_mpi_init_hook(int rank, int size, int appnum, int *tag_bits,
                            MPIR_Comm *init_comm)
{
    int mpi_errno = MPI_SUCCESS;

    /* initialize mvp specific cvars
     * base cvars were intialized before the call to MPID_Init so this is safe
     */
    mpi_errno = MPIR_T_MVP_cvar_init();
    MPIR_ERR_CHECK(mpi_errno);

    /* Set affinity before any other init hooks to ensure that the same process
     * to core mapping is used everywhere */
    mpi_errno =
        MPIDI_MVP_CH4_set_affinity(MPIR_Process.rank, MPIR_Process.size);
    MPIR_ERR_CHECK(mpi_errno);

#ifdef HAVE_CH4_NETMOD_UCX
    MVP_CVAR_SOFT_SET(MVP_USE_PT2PT_SHMEM, 0);
#endif

    mpi_errno = MPIDI_MVP_smp_init();
    MPIR_ERR_CHECK(mpi_errno);
    mvp_init_debug();

    /*
     * TODO: making this condition always true until we implement one-sided ops
     * Without UCX being initialized and progressing RMA ops, we will always
     * fail those operations
     */
    if (1 || MPIDI_MVP_INTERNODE || !MVP_USE_SHARED_MEM ||
        !MVP_USE_SHMEM_COLL || !MVP_USE_PT2PT_SHMEM) {
#ifdef HAVE_CH4_NETMOD_OFI
        mpi_errno =
            MPIDI_OFI_mpi_init_hook(rank, size, appnum, tag_bits, init_comm);
#endif
#ifdef HAVE_CH4_NETMOD_UCX
        mpi_errno =
            MPIDI_UCX_mpi_init_hook(rank, size, appnum, tag_bits, init_comm);
#endif
        MPIR_ERR_CHECK(mpi_errno);
        mvp_network_init = 1;
    }

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIDI_MVP_mpi_finalize_hook(void)
{
    int mpi_errno = MPI_SUCCESS;

    /* Check to see if shmem_collectives were enabled. If yes, the
    specific entries need to be freed. */
    MPIR_Comm *comm_ptr = NULL;
    MPIR_Errflag_t errflag = MPIR_ERR_NONE;
    MPIR_Comm_get_ptr(MPI_COMM_WORLD, comm_ptr);

    if (MVP_REPORT_LOAD_IMBALANCE > 0) {
        mpi_errno = MVP_load_imbalance_report(&mvp_allreduce_load_imbalance,
                                              comm_ptr, "AllReduce");
        MPIR_ERR_CHECK(mpi_errno);
    }

    mpi_errno = MPIR_Barrier_impl(comm_ptr, &errflag);
    MPIR_ERR_CHECK(mpi_errno);
    if (MPIR_Process.comm_world->dev.ch.shmem_coll_ok == 1) {
        mpi_errno = free_2level_comm(MPIR_Process.comm_world);
        MPIR_ERR_CHECK(mpi_errno);
    }

    /* finalize mvp specific cvars */
    MPIR_T_MVP_cvar_finalize();

    MVP_collectives_arch_finalize();

    if (MVP_USE_SHARED_MEM || finalize_coll_comm == 1) {
        MPIR_MVP_SHMEM_COLL_Cleanup();
    }

    MPIDI_MVP_NETWORK_PATH_ENTER
#ifdef HAVE_CH4_NETMOD_OFI
    mpi_errno = MPIDI_OFI_mpi_finalize_hook();
#endif
#ifdef HAVE_CH4_NETMOD_UCX
    mpi_errno = MPIDI_UCX_mpi_finalize_hook();
#endif
    MPIDI_MVP_NETWORK_PATH_EXIT
    /* Finalize SMP after inter-node finalize as inter-node finalize still
     * communicates between processes on same node */
    MPIDI_MVP_smp_finalize();

    /* Clear up HWLoc related data structures */
    smpi_destroy_hwloc_topology();

    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIDI_MVP_post_init(void)
{
    int mpi_errno = MPI_SUCCESS;
    int show_binding = 0;

    MPIDI_MVP_NETWORK_PATH_ENTER
#ifdef HAVE_CH4_NETMOD_OFI
    mpi_errno = MPIDI_OFI_post_init();
#endif
#ifdef HAVE_CH4_NETMOD_UCX
    mpi_errno = MPIDI_UCX_post_init();
#endif
    MPIR_ERR_CHECK(mpi_errno);
    MPIDI_MVP_NETWORK_PATH_EXIT

#ifdef _OSU_COLLECTIVES_
    /*
     * TODO: we need to get this in an MVP_USE_OSU_COLLECTIVES branch,
     * but as of right now not running arch_init breaks other code.
     * Root cause here is MVP_Read_env_vars() is called in that
     * function. Whole function needs to be refactored though.
     */
    struct coll_info colls_arch_hca[colls_max];
    /* TODO : We need a reliable way to find heterogeneity here. Setting
     * it to 0 for testing purposes. */
    int heterogeneity = 0;
    MPIR_Comm *comm_ptr = NULL;
    mpi_errno = MVP_collectives_arch_init(heterogeneity, colls_arch_hca);
    MPIR_ERR_CHECK(mpi_errno);

    mpi_errno = MPIR_MVP_SMP_COLL_init();
    MPIR_ERR_CHECK(mpi_errno);

    if (MVP_REPORT_LOAD_IMBALANCE > 0) {
        MPII_MVP_LOAD_IMBALANCE_TIMER_INIT(&mvp_allreduce_load_imbalance);
    }

    /* setup two level comm for OSU collectives */
    if (MVP_USE_OSU_COLLECTIVES && MVP_USE_SHARED_MEM &&
        (MVP_ENABLE_SOCKET_AWARE_COLLECTIVES ||
         MVP_ENABLE_TOPO_AWARE_COLLECTIVES)) {
        mpi_errno = smpi_load_hwloc_topology_whole();
        MPIR_ERR_CHECK(mpi_errno);
#if defined(__ICC) || defined(__INTEL_COMPILER)
        setenv("KMP_TOPOLOGY_METHOD", "hwloc", 0);
#endif
        if (MVP_ENABLE_TOPO_AWARE_COLLECTIVES) {
            /* Find the NUMA domain I am bound to */
            mpi_errno = smpi_identify_my_numa_id();
            MPIR_ERR_CHECK(mpi_errno);
            /* Find the socket I am bound to */
            mpi_errno = smpi_identify_my_sock_id();
            MPIR_ERR_CHECK(mpi_errno);
        }
    }

    MPIR_Comm_get_ptr(MPI_COMM_WORLD, comm_ptr);
    if (MVP_USE_OSU_COLLECTIVES && MVP_USE_SHARED_MEM && MVP_USE_SHMEM_COLL) {
        int flag = 0;
        PMPI_Comm_test_inter(comm_ptr->handle, &flag);

        if (flag == 0 && comm_ptr->dev.ch.shmem_coll_ok == 0 &&
            comm_ptr->local_size < MVP_TWO_LEVEL_COMM_EARLY_INIT_THRESHOLD &&
            check_split_comm(pthread_self())) {
            disable_split_comm(pthread_self());
            mpi_errno = create_2level_comm(
                comm_ptr->handle, comm_ptr->local_size, comm_ptr->rank);
            MPIR_ERR_CHECK(mpi_errno);
            enable_split_comm(pthread_self());
            MPIR_ERR_CHECK(mpi_errno);
        }
    }
#endif

    /* show cpu/hca affinity and clean up hwloc files */
    show_binding = (MVP_SHOW_CPU_BINDING > MVP_SHOW_HW_BINDING) ?
                       MVP_SHOW_CPU_BINDING :
                       MVP_SHOW_HW_BINDING;
    if (show_binding) {
        mvp_show_cpu_affinity(show_binding);
    }
#if 0
    show_binding = (MVP_SHOW_HCA_BINDING > MVP_SHOW_HW_BINDING) ?
                    MVP_SHOW_HCA_BINDING : MVP_SHOW_HW_BINDING;
    if (show_binding) {
        mvp_show_hca_affinity(show_binding);
    }
#endif

    /* print cvars before we get started */
    if (MVP_SHOW_ENV_INFO && !MPIR_Process.rank) {
        MPIR_MVP_print_cvars(MVP_SHOW_ENV_INFO);
    }

    mpi_errno = smpi_unlink_hwloc_topology_file();
    MPIR_ERR_CHECK(mpi_errno);

    mpi_errno = MPIDI_MVP_offload_coll_init();
    MPIR_ERR_CHECK(mpi_errno);

    MPIR_Errflag_t errflag;
    MPIR_Barrier_MVP(comm_ptr, &errflag);
fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
