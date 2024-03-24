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

#ifndef _SCATTERV_TUNING_
#define _SCATTERV_TUNING_

#include "mvp_coll_shmem.h"

#if defined(_SHARP_SUPPORT_)
extern int MPIR_Sharp_Scatterv_MVP(const void *sendbuf, const int *sendcounts,
                                   const int *displs, MPI_Datatype sendtype,
                                   void *recvbuf, int recvcount,
                                   MPI_Datatype recvtype, int root,
                                   MPIR_Comm *comm_ptr,
                                   MPIR_Errflag_t *errflag);
#endif
#endif
