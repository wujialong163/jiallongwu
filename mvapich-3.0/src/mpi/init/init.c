/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

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
#include <strings.h>

#include "mpiimpl.h"
#include "mpi_init.h"

#if defined(CHANNEL_MRAIL) || defined(_MVP_CH4_OVERRIDE_)
#include "mvp_coll_shmem.h"
extern int smpi_identify_my_numa_id(void);
extern int smpi_identify_my_sock_id(void);
extern int mvp_my_cpu_id;
extern int mvp_my_sock_id;
extern int mvp_my_numa_id;
extern int mvp_my_l3_id;
#endif

extern int smpi_load_hwloc_topology_whole(void);
#include <strings.h>

/*
=== BEGIN_MPI_T_CVAR_INFO_BLOCK ===

categories:
    - name        : THREADS
      description : multi-threading cvars

cvars:
    - name        : MPIR_CVAR_DEFAULT_THREAD_LEVEL
      category    : THREADS
      type        : string
      default     : "MPI_THREAD_SINGLE"
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Sets the default thread level to use when using MPI_INIT. This variable
        is case-insensitive.

=== END_MPI_T_CVAR_INFO_BLOCK ===
*/

/* -- Begin Profiling Symbol Block for routine MPI_Init */
#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPI_Init = PMPI_Init
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_Init  MPI_Init
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_Init as PMPI_Init
#elif defined(HAVE_WEAK_ATTRIBUTE)
int MPI_Init(int *argc, char ***argv) __attribute__ ((weak, alias("PMPI_Init")));
#endif
/* -- End Profiling Symbol Block */

/* Define MPICH_MPI_FROM_PMPI if weak symbols are not supported to build
   the MPI routines */
#ifndef MPICH_MPI_FROM_PMPI
#undef MPI_Init
#define MPI_Init PMPI_Init

/* Any internal routines can go here.  Make them static if possible */

#endif

/*@
   MPI_Init - Initialize the MPI execution environment

Input Parameters:
+  argc - Pointer to the number of arguments
-  argv - Pointer to the argument vector

Thread and Signal Safety:
This routine must be called by one thread only.  That thread is called
the `main thread` and must be the thread that calls 'MPI_Finalize'.

Notes:
   The MPI standard does not say what a program can do before an 'MPI_INIT' or
   after an 'MPI_FINALIZE'.  In the MPICH implementation, you should do
   as little as possible.  In particular, avoid anything that changes the
   external state of the program, such as opening files, reading standard
   input or writing to standard output.

Notes for C:
    As of MPI-2, 'MPI_Init' will accept NULL as input parameters. Doing so
    will impact the values stored in 'MPI_INFO_ENV'.

Notes for Fortran:
The Fortran binding for 'MPI_Init' has only the error return
.vb
    subroutine MPI_INIT(ierr)
    integer ierr
.ve

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_INIT

.seealso: MPI_Init_thread, MPI_Finalize
@*/
int MPI_Init(int *argc, char ***argv)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_TERSE_INIT_STATE_DECL(MPID_STATE_MPI_INIT);
    MPIR_FUNC_TERSE_INIT_ENTER(MPID_STATE_MPI_INIT);

    /* Handle mpich_state in case of Re-init */
    if (MPL_atomic_load_int(&MPIR_Process.mpich_state) == MPICH_MPI_STATE__POST_FINALIZED) {
        MPL_atomic_store_int(&MPIR_Process.mpich_state, MPICH_MPI_STATE__PRE_INIT);
    }
    
#ifdef HAVE_ERROR_CHECKING
    {
        MPID_BEGIN_ERROR_CHECKS;
        {
            if (MPL_atomic_load_int(&MPIR_Process.mpich_state) != MPICH_MPI_STATE__PRE_INIT) {
                mpi_errno =
                    MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, __func__, __LINE__,
                                         MPI_ERR_OTHER, "**inittwice", NULL);
            }
            if (mpi_errno)
                goto fn_fail;
        }
        MPID_END_ERROR_CHECKS;
    }
#endif /* HAVE_ERROR_CHECKING */

    /* ... body of routine ... */
    int threadLevel = MPI_THREAD_SINGLE;
    const char *tmp_str;
    if (MPL_env2str("MPIR_CVAR_DEFAULT_THREAD_LEVEL", &tmp_str)) {
        if (!strcasecmp(tmp_str, "MPI_THREAD_MULTIPLE"))
            threadLevel = MPI_THREAD_MULTIPLE;
        else if (!strcasecmp(tmp_str, "MPI_THREAD_SERIALIZED"))
            threadLevel = MPI_THREAD_SERIALIZED;
        else if (!strcasecmp(tmp_str, "MPI_THREAD_FUNNELED"))
            threadLevel = MPI_THREAD_FUNNELED;
        else if (!strcasecmp(tmp_str, "MPI_THREAD_SINGLE"))
            threadLevel = MPI_THREAD_SINGLE;
        else {
            MPL_error_printf("Unrecognized thread level %s\n", tmp_str);
            exit(1);
        }
    }

    int provided;
    mpi_errno = MPIR_Init_thread(argc, argv, threadLevel, &provided);
    if (mpi_errno != MPI_SUCCESS)
        goto fn_fail;


#if defined(CHANNEL_MRAIL)
    if (MVP_USE_OSU_COLLECTIVES && MVP_USE_SHARED_MEM &&
        (MVP_ENABLE_SOCKET_AWARE_COLLECTIVES ||
         MVP_ENABLE_TOPO_AWARE_COLLECTIVES)) {
        mpi_errno = smpi_load_hwloc_topology_whole();
        if (mpi_errno != MPI_SUCCESS) {
            MPIR_ERR_POP(mpi_errno);
        }

        if (MVP_ENABLE_TOPO_AWARE_COLLECTIVES) {
#if defined(CHANNEL_MRAIL)
            /* Find the NUMA domain I am bound to */
            mpi_errno = smpi_identify_my_numa_id();
            if (mpi_errno != MPI_SUCCESS) {
                MPIR_ERR_POP(mpi_errno);
            }
            /* Find the socket I am bound to */
            mpi_errno = smpi_identify_my_sock_id();
            if (mpi_errno != MPI_SUCCESS) {
                MPIR_ERR_POP(mpi_errno);
            }
            /* PRINT_DEBUG(DEBUG_INIT_verbose, "cpu_id = %d, sock_id = %d, numa_id = %d, l3_id = %d\n",
                    mvp_my_cpu_id, mvp_my_sock_id, mvp_my_numa_id, mvp_my_l3_id); */
        }
#endif /* #if defined(CHANNEL_MRAIL) */
    }
#endif

#if defined(CHANNEL_MRAIL_GEN2)
    /* initialize the two level communicator for MPI_COMM_WORLD  */
    if (MVP_USE_OSU_COLLECTIVES && MVP_USE_SHARED_MEM) {
        MPIR_Comm *comm_ptr = NULL;
        MPIR_Comm_get_ptr(MPI_COMM_WORLD, comm_ptr);
        int flag = 0;
        PMPI_Comm_test_inter(comm_ptr->handle, &flag);

        if (flag == 0 && comm_ptr->dev.ch.shmem_coll_ok == 0 &&
            comm_ptr->local_size < MVP_TWO_LEVEL_COMM_EARLY_INIT_THRESHOLD &&
            check_split_comm(pthread_self())) {
            disable_split_comm(pthread_self());
            mpi_errno = create_2level_comm(comm_ptr->handle, comm_ptr->local_size, comm_ptr->rank);
            if(mpi_errno) {
               MPIR_ERR_POP(mpi_errno);
            }
            enable_split_comm(pthread_self());
            if(mpi_errno) {
               MPIR_ERR_POP(mpi_errno);
            }
        }
    }
#endif /*defined(CHANNEL_MRAIL_GEN2) */

    /* ... end of body of routine ... */
    MPIR_FUNC_TERSE_INIT_EXIT(MPID_STATE_MPI_INIT);
    return mpi_errno;

  fn_fail:
    /* --BEGIN ERROR HANDLING-- */
#ifdef HAVE_ERROR_REPORTING
    {
        mpi_errno =
            MPIR_Err_create_code(mpi_errno, MPIR_ERR_RECOVERABLE, __func__, __LINE__, MPI_ERR_OTHER,
                                 "**mpi_init", "**mpi_init %p %p", argc, argv);
    }
#endif
    mpi_errno = MPIR_Err_return_comm(0, __func__, mpi_errno);
    return mpi_errno;
    /* --END ERROR HANDLING-- */
}
