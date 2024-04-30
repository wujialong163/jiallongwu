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

#ifndef _MVP_ERR_H_
#define _MVP_ERR_H_

#include "mpiimpl.h"

#ifdef _OSU_MVAPICH_

/* add some additional checking to the standard error handler
 * This version allows us to specify an invalid error code to indicate
 * success but don't finish the function
 */
#define MPIR_MVP_ERR_CHECK(_err)                                               \
    if (_err > MPI_ERR_LASTCODE)                                               \
        goto fn_exit;                                                          \
    MPIR_ERR_CHECK(_err)

#endif /* _OSU_MVAPICH_ */
#endif /* _MVP_ERR_H_ */
