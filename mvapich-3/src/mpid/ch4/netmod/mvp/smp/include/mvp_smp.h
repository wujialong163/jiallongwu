/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda. For detailed
 * copyright and licensing information, please refer to the copyright file
 * COPYRIGHT in the top level MVAPICH directory.
 */

/* global SMP header */
/*
 * Should include all variables/struct definitions used as an UP interface. That
 * is, those which are potentially used by higher level layers to communicated
 * with the lower layers. For SMP internals we should use the `mvp_smp_impl.h`
 * header instead.
 */

#ifndef _MVP_SMP_H_
#define _MVP_SMP_H_
#include "mpidimpl.h"

 extern int mvp_smp_init;
 extern int mvp_smp_only;

 extern long int mvp_num_queued_smp_ops;

 /* forward declarations for functions */
 int MPIDI_MVP_smp_mpi_send(const void *buf, MPI_Aint count,
                            MPI_Datatype datatype, int rank, int tag,
                            MPIR_Comm *comm, int context_offset,
                            MPIDI_av_entry_t *addr, MPIR_Request **request);
 int MPIDI_MVP_smp_mpi_recv(void *buf, MPI_Aint count, MPI_Datatype datatype,
                            int rank, int tag, MPIR_Comm *comm,
                            int context_offset, MPIDI_av_entry_t *addr,
                            MPI_Status *status, MPIR_Request **request);

#endif /* _MVP_SMP_H_ */
