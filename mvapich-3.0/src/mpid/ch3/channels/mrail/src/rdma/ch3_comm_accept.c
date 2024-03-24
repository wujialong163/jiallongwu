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

int MPIDI_CH3_Comm_accept(char *port_name, int root, MPIR_Comm *comm_ptr,
                          MPIR_Comm **newcomm)
{
    MPIR_FUNC_TERSE_STATE_DECL(MPID_STATE_MPIDI_CH3_COMM_ACCEPT);
    MPIR_FUNC_TERSE_ENTER(MPID_STATE_MPIDI_CH3_COMM_ACCEPT);
    int mpi_errno =
        MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, __func__,
                             __LINE__, MPI_ERR_OTHER, "**notimpl", 0);
    MPIR_FUNC_TERSE_EXIT(MPID_STATE_MPIDI_CH3_COMM_ACCEPT);
    return mpi_errno;
}
