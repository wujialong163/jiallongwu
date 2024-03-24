/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda. For detailed
 * copyright and licensing information, please refer to the copyright file
 * COPYRIGHT in the top level MVAPICH directory.
 */

#include "mpidimpl.h"

#ifndef _NETMOD_INLINE_OVERRIDE_H_
#define _NETMOD_INLINE_OVERRIDE_H_

#ifdef _MVP_CH4_OVERRIDE_
#include "mvp_types.h"
/* direct access to MVP level request */
#define MPID_MVP_REQUEST(_req, _field) ((_req)->dev.ch4.netmod.mvp._field)
/* TODO: changing semantics here, this should be resolved */
#define MPID_MVP_SHARP_REQUEST MPIDI_MVP_SHARP_REQUEST_FROM_MPICH

#ifndef _MVP_INTERNAL_DISABLE_OVERRIDES_

int MPIDI_MVP_mpi_send_coll(const void *buf, MPI_Aint count, MPI_Datatype datatype,
                       int rank, int tag, MPIR_Comm *comm, int context_offset,
                       MPIDI_av_entry_t *addr, MPIR_Request **request, 
                       MPIR_Errflag_t *errflag);
int MPIDI_MVP_mpi_isend_coll(const void *buf, MPI_Aint count,MPI_Datatype datatype,
                         int rank, int tag, MPIR_Comm * comm, int context_offset,
                         MPIDI_av_entry_t * addr, MPIR_Request ** request,
                         MPIR_Errflag_t * errflag);
int MPIDI_MVP_mpi_send(const void *buf, MPI_Aint count, MPI_Datatype datatype,
                       int rank, int tag, MPIR_Comm *comm, int context_offset,
                       MPIDI_av_entry_t *addr, MPIR_Request **request);
int MPIDI_MVP_mpi_isend(const void *buf, MPI_Aint count, MPI_Datatype datatype,
                        int rank, int tag, MPIR_Comm *comm, int context_offset,
                        MPIDI_av_entry_t *addr, MPIR_Request **request);
int MPIDI_MVP_mpi_recv(void *buf, MPI_Aint count, MPI_Datatype datatype,
                       int rank, int tag, MPIR_Comm *comm, int context_offset,
                       MPIDI_av_entry_t *addr, MPI_Status *status,
                       MPIR_Request **request);
int MPIDI_MVP_mpi_irecv(void *buf, MPI_Aint count, MPI_Datatype datatype,
                        int rank, int tag, MPIR_Comm *comm, int context_offset,
                        MPIDI_av_entry_t *addr, MPIR_Request **request,
                        MPIR_Request *partner);
int MPIDI_MVP_mpi_cancel_recv(MPIR_Request *request);
int MPIDI_MVP_mpi_iprobe(int source, int tag, MPIR_Comm *comm,
                         int context_offset, MPIDI_av_entry_t *addr, int *flag,
                         MPI_Status *status);
int MPIDI_MVP_mpi_improbe(int source, int tag, MPIR_Comm *comm,
                          int context_offset, MPIDI_av_entry_t *addr, int *flag,
                          MPIR_Request **message, MPI_Status *status);
int MPIDI_MVP_mpi_imrecv(void *buf, MPI_Aint count, MPI_Datatype datatype,
                         MPIR_Request *message);

/* additional functions not defined by the standard netmod API */
void MPIDI_MVP_smp_request_free(MPIR_Request *req);

/* override the standard NM implementation */
#define MPIDI_NM_send_coll       MPIDI_MVP_mpi_send_coll
#define MPIDI_NM_isend_coll      MPIDI_MVP_mpi_isend_coll
#define MPIDI_NM_mpi_send        MPIDI_MVP_mpi_send
#define MPIDI_NM_mpi_isend       MPIDI_MVP_mpi_isend
#define MPIDI_NM_mpi_ssend       MPIDI_MVP_mpi_send
#define MPIDI_NM_mpi_issend      MPIDI_MVP_mpi_isend
#define MPIDI_NM_mpi_rsend       MPIDI_MVP_mpi_send
#define MPIDI_NM_mpi_irsend      MPIDI_MVP_mpi_isend
#define MPIDI_NM_mpi_recv        MPIDI_MVP_mpi_recv
#define MPIDI_NM_mpi_irecv       MPIDI_MVP_mpi_irecv
#define MPIDI_NM_mpi_cancel_recv MPIDI_MVP_mpi_cancel_recv
#define MPIDI_NM_mpi_iprobe      MPIDI_MVP_mpi_iprobe
#define MPIDI_NM_mpi_improbe     MPIDI_MVP_mpi_improbe
#define MPIDI_NM_mpi_imrecv      MPIDI_MVP_mpi_imrecv

#endif /* _MVP_INTERNAL_DISABLE_OVERRIDES_ */
#endif /* _MVP_CH4_OVERRIDE_ */
#endif /* _MVP_INLINE_OVERRIDE_H_ */
