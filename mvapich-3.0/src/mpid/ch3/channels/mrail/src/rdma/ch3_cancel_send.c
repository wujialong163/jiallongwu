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

int MPIDI_CH3_Cancel_send(MPIDI_VC_t *vc, MPIR_Request *sreq, int *cancelled)
{
    MPIR_FUNC_TERSE_STATE_DECL(MPID_STATE_MPIDI_CH3_CANCEL_SEND);
    MPIR_FUNC_TERSE_ENTER(MPID_STATE_MPIDI_CH3_CANCEL_SEND);
    *cancelled = FALSE;
    MPIR_FUNC_TERSE_EXIT(MPID_STATE_MPIDI_CH3_CANCEL_SEND);
    return MPI_SUCCESS;
}
