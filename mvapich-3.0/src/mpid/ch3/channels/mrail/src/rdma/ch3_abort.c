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

#include "mpidi_ch3_impl.h"
#include "upmi.h"
#include "error_handling.h"
#ifdef HYDRA_PMI_CLIENT
#include "src/pm/hydra/include/hydra_config.h"
#endif

int MPIDI_CH3_Abort(int exit_code, const char *error_msg)
{
    MPIR_FUNC_TERSE_STATE_DECL(MPID_STATE_MPIDI_CH3_ABORT);
    MPIR_FUNC_TERSE_ENTER(MPID_STATE_MPIDI_CH3_ABORT);

    /* print backtrace */
    if (show_backtrace)
        print_backtrace();

#if !defined(HAVE_PBS_PRO)
    UPMI_ABORT(exit_code, error_msg);
#endif /* #if !defined(HAVE_PBS_PRO) */

    /* if abort returns for some reason, exit here */

    PRINT_ERROR_ERRNO("%s", errno, error_msg);
    fflush(stderr);

    exit(exit_code);
#if defined(__SUNPRO_C) || defined(__SUNPRO_CC)
#pragma error_messages(off, E_STATEMENT_NOT_REACHED)
#endif /* defined(__SUNPRO_C) || defined(__SUNPRO_CC) */
    MPIR_FUNC_TERSE_EXIT(MPID_STATE_MPIDI_CH3_ABORT);
    return MPI_ERR_INTERN;
#if defined(__SUNPRO_C) || defined(__SUNPRO_CC)
#pragma error_messages(default, E_STATEMENT_NOT_REACHED)
#endif /* defined(__SUNPRO_C) || defined(__SUNPRO_CC) */
}
