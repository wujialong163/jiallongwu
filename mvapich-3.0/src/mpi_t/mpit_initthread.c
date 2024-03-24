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
 */

#include "mpiimpl.h"
#include "mvp_mpit.h"

/* -- Begin Profiling Symbol Block for routine MPI_T_init_thread */
#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPI_T_init_thread = PMPI_T_init_thread
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_T_init_thread  MPI_T_init_thread
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_T_init_thread as PMPI_T_init_thread
#elif defined(HAVE_WEAK_ATTRIBUTE)
int MPI_T_init_thread(int required, int *provided)
    __attribute__ ((weak, alias("PMPI_T_init_thread")));
#endif
/* -- End Profiling Symbol Block */

/* Define MPICH_MPI_FROM_PMPI if weak symbols are not supported to build
   the MPI routines */
#ifndef MPICH_MPI_FROM_PMPI
#undef MPI_T_init_thread
#define MPI_T_init_thread PMPI_T_init_thread

/* any non-MPI functions go here, especially non-static ones */
static inline void MPIR_T_enum_env_init(void)
{
    static const UT_icd enum_table_entry_icd = { sizeof(MPIR_T_enum_t), NULL, NULL, NULL };

    utarray_new(enum_table, &enum_table_entry_icd, MPL_MEM_MPIT);
}

static inline void MPIR_T_cat_env_init(void)
{
    static const UT_icd cat_table_entry_icd = { sizeof(cat_table_entry_t), NULL, NULL, NULL };

    utarray_new(cat_table, &cat_table_entry_icd, MPL_MEM_MPIT);
    cat_hash = NULL;
    cat_stamp = 0;
}

static inline int MPIR_T_cvar_env_init(void)
{
    static const UT_icd cvar_table_entry_icd = { sizeof(cvar_table_entry_t), NULL, NULL, NULL };

    utarray_new(cvar_table, &cvar_table_entry_icd, MPL_MEM_MPIT);
    cvar_hash = NULL;
    return MPIR_T_cvar_init();
}

static inline void MPIR_T_pvar_env_init(void)
{
    int i;
    static const UT_icd pvar_table_entry_icd = { sizeof(pvar_table_entry_t), NULL, NULL, NULL };

    utarray_new(pvar_table, &pvar_table_entry_icd, MPL_MEM_MPIT);
    for (i = 0; i < MPIR_T_PVAR_CLASS_NUMBER; i++) {
        pvar_hashs[i] = NULL;
    }
}

int MPIR_T_env_init(void)
{
    int mpi_errno = MPI_SUCCESS;
    static int initialized = FALSE;

    if (!initialized) {
        initialized = TRUE;
        MPIR_T_enum_env_init();
        MPIR_T_cat_env_init();
        mpi_errno = MPIR_T_cvar_env_init();
        MPIR_T_pvar_env_init();
#ifdef _OSU_MVAPICH_
#if ENABLE_PVAR_MEM
        if (MVP_ENABLE_PVAR_MEM) {
            MPIT_MEM_REGISTER_PVARS();
        }
#endif /* ENABLE_PVAR_MEM */
        MPIT_REGISTER_MVP_VARIABLES();
#endif
    }
    return mpi_errno;
}

#endif /* MPICH_MPI_FROM_PMPI */

/*@
MPI_T_init_thread - Initialize the MPI_T execution environment

Input Parameters:
. required - desired level of thread support (integer)

Output Parameters:
. provided - provided level of thread support (integer)

Notes:
  The valid values for the level of thread support are:
+ MPI_THREAD_SINGLE - Only one thread will execute.
. MPI_THREAD_FUNNELED - The process may be multi-threaded, but only the main
  thread will make MPI_T calls (all MPI_T calls are funneled to the
  main thread).
. MPI_THREAD_SERIALIZED - The process may be multi-threaded, and multiple
  threads may make MPI_T calls, but only one at a time: MPI_T calls are not
  made concurrently from two distinct threads (all MPI_T calls are serialized).
- MPI_THREAD_MULTIPLE - Multiple threads may call MPI_T, with no restrictions.

.N ThreadSafe

.N Errors
.N MPI_SUCCESS

.seealso MPI_T_finalize
@*/
int MPI_T_init_thread(int required, int *provided)
{
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_TERSE_STATE_DECL(MPID_STATE_MPI_T_INIT_THREAD);
    MPIR_FUNC_TERSE_ENTER(MPID_STATE_MPI_T_INIT_THREAD);

    /* ... body of routine ...  */

#if defined MPICH_IS_THREADED
    MPIR_T_is_threaded = (required == MPI_THREAD_MULTIPLE);
#endif /* MPICH_IS_THREADED */

    if (provided != NULL) {
        /* This must be min(required,MPICH_THREAD_LEVEL) if runtime
         * control of thread level is available */
        *provided = (MPICH_THREAD_LEVEL < required) ? MPICH_THREAD_LEVEL : required;
    }

    ++MPIR_T_init_balance;
    if (MPIR_T_init_balance == 1) {
        MPIR_T_THREAD_CS_INIT();
        mpi_errno = MPIR_T_env_init();
    }

    /* ... end of body of routine ... */

    MPIR_FUNC_TERSE_EXIT(MPID_STATE_MPI_T_INIT_THREAD);
    return mpi_errno;
}
