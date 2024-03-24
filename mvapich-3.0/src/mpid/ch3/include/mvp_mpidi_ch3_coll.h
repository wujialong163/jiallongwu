/* Copyright (c) 2001-2021, The Ohio State University. All rights
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

/* Collective overloads for the CH3 device in MVP. Placed here as it applies to
 * both mrail and PSM.
 *
 * These collectives are implemented at the MPIR layer, but we are wrapping them
 * here at the MPIDI layer to overload in MPID_coll.h
 *
 * Note that all nonblocking collectives have been dropped from our overloads as
 * they are obsolete and use the deprecated sched algorithms.
 */

#ifndef MPIDI_MVP_CH3_COLL_INCLUDED
#define MPIDI_MVP_CH3_COLL_INCLUDED

#include "mvp_mpir.h"

int mvp_increment_shmem_coll_counter(MPIR_Comm *comm_ptr);
int mvp_increment_allgather_coll_counter(MPIR_Comm *comm_ptr);
int disable_split_comm(pthread_t);
int enable_split_comm(pthread_t);
#if defined(_MCST_SUPPORT_)
int create_mcast_comm(MPI_Comm comm, int size, int my_rank);
#endif /* defined(_MCST_SUPPORT_) */

static inline int MPIDI_MVP_Barrier(MPIR_Comm *comm, MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;
#ifdef _OSU_COLLECTIVES_
    if (MVP_USE_OSU_COLLECTIVES &&
        comm->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        mpi_errno = MPIR_Barrier_MVP(comm, errflag);
    } else {
#endif
        mpi_errno = MPIR_Barrier_impl(comm, errflag);
#ifdef _OSU_COLLECTIVES_
    }
    if (MVP_USE_OSU_COLLECTIVES) {
        mpi_errno = mvp_increment_shmem_coll_counter(comm);
        MPIR_ERR_CHECK(mpi_errno);
        if (comm->dev.ch.allgather_comm_ok == 0) {
            mpi_errno = mvp_increment_allgather_coll_counter(comm);
            MPIR_ERR_CHECK(mpi_errno);
        }
#if defined(_SHARP_SUPPORT_)
        comm->dev.ch.barrier_coll_count++;
        if (MVP_ENABLE_SHARP && MVP_ENABLE_SHARP_BARRIER &&
            (comm->dev.ch.is_sharp_ok == 0) &&
            (comm->dev.ch.shmem_coll_ok == 1) &&
            (comm->dev.ch.barrier_coll_count >= MVP_TWO_LEVEL_COMM_THRESHOLD)) {
            disable_split_comm(pthread_self());
            mpi_errno =
                create_sharp_comm(comm->handle, comm->local_size, comm->rank);
            MPIR_ERR_CHECk(mpi_errno);
            enable_split_comm(pthread_self());
        }
#endif /*(_SHARP_SUPPORT_)*/
    }
#endif /* _OSU_COLLECTIVES_ */
fn_fail:
    return mpi_errno;
}

static inline int MPIDI_MVP_Bcast(void *buffer, int count,
                                  MPI_Datatype datatype, int root,
                                  MPIR_Comm *comm, MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;
#ifdef _OSU_COLLECTIVES_
    if (MVP_USE_OSU_COLLECTIVES &&
        comm->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        mpi_errno =
            MPIR_Bcast_MVP(buffer, count, datatype, root, comm, errflag);
    } else {
#endif
        mpi_errno =
            MPIR_Bcast_impl(buffer, count, datatype, root, comm, errflag);
#ifdef _OSU_COLLECTIVES_
    }
    if (MVP_USE_OSU_COLLECTIVES) {
        mpi_errno = mvp_increment_shmem_coll_counter(comm);
        MPIR_ERR_CHECK(mpi_errno);
        if (comm->dev.ch.allgather_comm_ok == 0) {
            mpi_errno = mvp_increment_allgather_coll_counter(comm);
            MPIR_ERR_CHECK(mpi_errno);
        }
#if defined(_SHARP_SUPPORT_)
        comm->dev.ch.bcast_coll_count++;
        if (MVP_ENABLE_SHARP && MVP_ENABLE_SHARP_BCAST &&
            (comm->dev.ch.is_sharp_ok == 0) &&
            (comm->dev.ch.shmem_coll_ok == 1) &&
            (comm->dev.ch.bcast_coll_count >= MVP_TWO_LEVEL_COMM_THRESHOLD)) {
            disable_split_comm(pthread_self());
            mpi_errno =
                create_sharp_comm(comm->handle, comm->local_size, comm->rank);
            MPIR_ERR_CHECk(mpi_errno);
            enable_split_comm(pthread_self());
        }
#endif /*(_SHARP_SUPPORT_)*/
    }
#endif /* _OSU_COLLECTIVES_ */
fn_fail:
    return mpi_errno;
}

static inline int MPIDI_MVP_Allreduce(const void *sendbuf, void *recvbuf,
                                      int count, MPI_Datatype datatype,
                                      MPI_Op op, MPIR_Comm *comm,
                                      MPIR_Errflag_t *errflag)
{
    int mpi_errno = 0;
#ifdef _OSU_COLLECTIVES_
    if (MVP_USE_OSU_COLLECTIVES &&
        comm->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        mpi_errno = MPIR_Allreduce_MVP(sendbuf, recvbuf, count, datatype, op,
                                       comm, errflag);
        MPIR_ERR_CHECK(mpi_errno);
    } else {
#endif
        mpi_errno = MPIR_Allreduce_impl(sendbuf, recvbuf, count, datatype, op,
                                        comm, errflag);
#ifdef _OSU_COLLECTIVES_
    }
    if (MVP_USE_OSU_COLLECTIVES) {
        mpi_errno = mvp_increment_shmem_coll_counter(comm);
        MPIR_ERR_CHECK(mpi_errno);
        if (comm->dev.ch.allgather_comm_ok == 0) {
            mpi_errno = mvp_increment_allgather_coll_counter(comm);
            MPIR_ERR_CHECK(mpi_errno);
        }
#if defined(_SHARP_SUPPORT_)
        comm->dev.ch.allreduce_coll_count++;
        if (MVP_ENABLE_SHARP && MVP_ENABLE_SHARP_ALLREDUCE &&
            (comm->dev.ch.is_sharp_ok == 0) &&
            (comm->dev.ch.shmem_coll_ok == 1) &&
            (comm->dev.ch.allreduce_coll_count >=
             MVP_TWO_LEVEL_COMM_THRESHOLD)) {
            disable_split_comm(pthread_self());
            mpi_errno =
                create_sharp_comm(comm->handle, comm->local_size, comm->rank);
            MPIR_ERR_CHECk(mpi_errno);
            enable_split_comm(pthread_self());
        }
#endif /*(_SHARP_SUPPORT_)*/
    }
#endif /* _OSU_COLLECTIVES_ */
fn_fail:
    return mpi_errno;
}

static inline int MPIDI_MVP_Allgather(const void *sendbuf, int sendcount,
                                      MPI_Datatype sendtype, void *recvbuf,
                                      int recvcount, MPI_Datatype recvtype,
                                      MPIR_Comm *comm, MPIR_Errflag_t *errflag)
{
#ifdef _OSU_COLLECTIVES_
    if (MVP_USE_OSU_COLLECTIVES &&
        comm->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        return MPIR_Allgather_MVP(sendbuf, sendcount, sendtype, recvbuf,
                                  recvcount, recvtype, comm, errflag);
    }
#endif
    return MPIR_Allgather_impl(sendbuf, sendcount, sendtype, recvbuf, recvcount,
                               recvtype, comm, errflag);
}

static inline int MPIDI_MVP_Allgatherv(const void *sendbuf, int sendcount,
                                       MPI_Datatype sendtype, void *recvbuf,
                                       const int *recvcounts, const int *displs,
                                       MPI_Datatype recvtype, MPIR_Comm *comm,
                                       MPIR_Errflag_t *errflag)
{
#ifdef _OSU_COLLECTIVES_
    if (MVP_USE_OSU_COLLECTIVES &&
        comm->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        return MPIR_Allgatherv_MVP(sendbuf, sendcount, sendtype, recvbuf,
                                   recvcounts, displs, recvtype, comm, errflag);
    }
#endif
    return MPIR_Allgatherv_impl(sendbuf, sendcount, sendtype, recvbuf,
                                recvcounts, displs, recvtype, comm, errflag);
}

static inline int MPIDI_MVP_Scatter(const void *sendbuf, int sendcount,
                                    MPI_Datatype sendtype, void *recvbuf,
                                    int recvcount, MPI_Datatype recvtype,
                                    int root, MPIR_Comm *comm,
                                    MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;
#ifdef _OSU_COLLECTIVES_
    if (MVP_USE_OSU_COLLECTIVES &&
        comm->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        mpi_errno = MPIR_Scatter_MVP(sendbuf, sendcount, sendtype, recvbuf,
                                     recvcount, recvtype, root, comm, errflag);
        MPIR_ERR_CHECK(mpi_errno);
    } else {
#endif
        mpi_errno = MPIR_Scatter_impl(sendbuf, sendcount, sendtype, recvbuf,
                                      recvcount, recvtype, root, comm, errflag);
#ifdef _OSU_COLLECTIVES_
    }
    if (MVP_USE_OSU_COLLECTIVES) {
        if (comm->dev.ch.shmem_coll_ok == 0) {
            mpi_errno = mvp_increment_shmem_coll_counter(comm);
            MPIR_ERR_CHECK(mpi_errno);
        }
#if defined(_SHARP_SUPPORT_)
        comm->dev.ch.scatter_coll_count++;
        if (MVP_ENABLE_SHARP && MVP_ENABLE_SHARP_SCATTER &&
            (comm->dev.ch.is_sharp_ok == 0) &&
            (comm->dev.ch.shmem_coll_ok == 1) &&
            (comm->dev.ch.scatter_coll_count >= MVP_TWO_LEVEL_COMM_THRESHOLD)) {
            disable_split_comm(pthread_self());
            mpi_errno =
                create_sharp_comm(comm->handle, comm->local_size, comm->rank);
            MPIR_ERR_CHECK(mpi_errno);
            enable_split_comm(pthread_self());
        }
#endif /*(_SHARP_SUPPORT_)*/
#if defined(_MCST_SUPPORT_)
        if (rdma_enable_mcast && MVP_USE_MCAST_SCATTER &&
            (comm->dev.ch.is_mcast_ok == 0) &&
            (comm->dev.ch.shmem_coll_ok == 1) &&
            (comm->dev.ch.scatter_coll_count >= MVP_TWO_LEVEL_COMM_THRESHOLD)) {
            disable_split_comm(pthread_self());
            mpi_errno =
                create_mcast_comm(comm->handle, comm->local_size, comm->rank);
            MPIR_ERR_CHECK(mpi_errno);
            enable_split_comm(pthread_self());
        }
#endif /*(_MCST_SUPPORT_)*/
    }
#endif /* _OSU_COLLECTIVES_ */
fn_fail:
    return mpi_errno;
}

static inline int MPIDI_MVP_Scatterv(const void *sendbuf, const int *sendcounts,
                                     const int *displs, MPI_Datatype sendtype,
                                     void *recvbuf, int recvcount,
                                     MPI_Datatype recvtype, int root,
                                     MPIR_Comm *comm, MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;
#if defined(_SHARP_SUPPORT_)
    if (comm->dev.ch.is_sharp_ok == 1 && MVP_ENABLE_SHARP &&
        MVP_ENABLE_SHARP_SCATTERV) {
        mpi_errno = MPIR_Sharp_Scatterv_MVP(sendbuf, sendcounts, displs,
                                            sendtype, recvbuf, recvcount,
                                            recvtype, root, comm, errflag);
        /* Direct flat algorithm in which every process calls Sharp
         * MVP_ENABLE_SHARP should be set to 2 */
        if (mpi_errno == MPI_SUCCESS) {
            goto cont_sharp;
        }
        /* SHARP collective is not supported,
         * continue without using SHARP */
    }
#endif /* end of defined (_SHARP_SUPPORT_) */

    mpi_errno =
        MPIR_Scatterv_impl(sendbuf, sendcounts, displs, sendtype, recvbuf,
                           recvcount, recvtype, root, comm, errflag);

#ifdef _OSU_COLLECTIVES_
#if defined(_SHARP_SUPPORT_)
cont_sharp:
#endif /* defined(_SHARP_SUPPORT_) */
    if (MVP_USE_OSU_COLLECTIVES) {
        if (comm->dev.ch.shmem_coll_ok == 0) {
            mpi_errno = mvp_increment_shmem_coll_counter(comm);
            MPIR_ERR_CHECK(mpi_errno);
        }
#if defined(_SHARP_SUPPORT_)
        comm->dev.ch.scatterv_coll_count++;
        if (MVP_ENABLE_SHARP && MVP_ENABLE_SHARP_SCATTERV &&
            (comm->dev.ch.is_sharp_ok == 0) &&
            (comm->dev.ch.shmem_coll_ok == 1) &&
            (comm->dev.ch.scatterv_coll_count >=
             MVP_TWO_LEVEL_COMM_THRESHOLD)) {
            disable_split_comm(pthread_self());
            mpi_errno =
                create_sharp_comm(comm->handle, comm->local_size, comm->rank);
            MPIR_ERR_CHECK(mpi_errno);
            enable_split_comm(pthread_self());
        }
#endif /*(_SHARP_SUPPORT_)*/
    }
#endif /* _OSU_COLLECTIVES_ */
fn_fail:
    return mpi_errno;
}

static inline int MPIDI_MVP_Gather(const void *sendbuf, int sendcount,
                                   MPI_Datatype sendtype, void *recvbuf,
                                   int recvcount, MPI_Datatype recvtype,
                                   int root, MPIR_Comm *comm,
                                   MPIR_Errflag_t *errflag)
{
#ifdef _OSU_COLLECTIVES_
    if (MVP_USE_OSU_COLLECTIVES &&
        comm->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        return MPIR_Gather_MVP(sendbuf, sendcount, sendtype, recvbuf, recvcount,
                               recvtype, root, comm, errflag);
    }
#endif
    return MPIR_Gather_impl(sendbuf, sendcount, sendtype, recvbuf, recvcount,
                            recvtype, root, comm, errflag);
}

static inline int MPIDI_MVP_Gatherv(const void *sendbuf, int sendcount,
                                    MPI_Datatype sendtype, void *recvbuf,
                                    const int *recvcounts, const int *displs,
                                    MPI_Datatype recvtype, int root,
                                    MPIR_Comm *comm, MPIR_Errflag_t *errflag)
{
#ifdef _OSU_COLLECTIVES_
    if (MVP_USE_OSU_COLLECTIVES &&
        comm->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        return MPIR_Gatherv_MVP(sendbuf, sendcount, sendtype, recvbuf,
                                recvcounts, displs, recvtype, root, comm,
                                errflag);
    }
#endif

    return MPIR_Gatherv_impl(sendbuf, sendcount, sendtype, recvbuf, recvcounts,
                             displs, recvtype, root, comm, errflag);
}

static inline int MPIDI_MVP_Alltoall(const void *sendbuf, int sendcount,
                                     MPI_Datatype sendtype, void *recvbuf,
                                     int recvcount, MPI_Datatype recvtype,
                                     MPIR_Comm *comm, MPIR_Errflag_t *errflag)
{
#ifdef _OSU_COLLECTIVES_
    if (MVP_USE_OSU_COLLECTIVES &&
        comm->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        return MPIR_Alltoall_MVP(sendbuf, sendcount, sendtype, recvbuf,
                                 recvcount, recvtype, comm, errflag);
    }
#endif

    return MPIR_Alltoall_impl(sendbuf, sendcount, sendtype, recvbuf, recvcount,
                              recvtype, comm, errflag);
}

static inline int MPIDI_MVP_Alltoallv(const void *sendbuf,
                                      const int *sendcounts, const int *sdispls,
                                      MPI_Datatype sendtype, void *recvbuf,
                                      const int *recvcounts, const int *rdispls,
                                      MPI_Datatype recvtype, MPIR_Comm *comm,
                                      MPIR_Errflag_t *errflag)
{
#ifdef _OSU_COLLECTIVES_
    if (MVP_USE_OSU_COLLECTIVES &&
        comm->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        return MPIR_Alltoallv_MVP(sendbuf, sendcounts, sdispls, sendtype,
                                  recvbuf, recvcounts, rdispls, recvtype, comm,
                                  errflag);
    }
#endif

    return MPIR_Alltoallv_impl(sendbuf, sendcounts, sdispls, sendtype, recvbuf,
                               recvcounts, rdispls, recvtype, comm, errflag);
}

static inline int MPIDI_MVP_Reduce(const void *sendbuf, void *recvbuf,
                                   int count, MPI_Datatype datatype, MPI_Op op,
                                   int root, MPIR_Comm *comm,
                                   MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;
#ifdef _OSU_COLLECTIVES_
    if (MVP_USE_OSU_COLLECTIVES &&
        comm->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        mpi_errno = MPIR_Reduce_MVP(sendbuf, recvbuf, count, datatype, op, root,
                                    comm, errflag);
    } else {
#endif
        mpi_errno = MPIR_Reduce_impl(sendbuf, recvbuf, count, datatype, op,
                                     root, comm, errflag);
#ifdef _OSU_COLLECTIVES_
    }
    if (MVP_USE_OSU_COLLECTIVES) {
        if (comm->dev.ch.shmem_coll_ok == 0) {
            mpi_errno = mvp_increment_shmem_coll_counter(comm);
            MPIR_ERR_CHECK(mpi_errno);
        }
#if defined(_SHARP_SUPPORT_)
        comm->dev.ch.reduce_coll_count++;
        if (MVP_ENABLE_SHARP && MVP_ENABLE_SHARP_REDUCE &&
            (comm->dev.ch.is_sharp_ok == 0) &&
            (comm->dev.ch.shmem_coll_ok == 1) &&
            (comm->dev.ch.reduce_coll_count >= MVP_TWO_LEVEL_COMM_THRESHOLD)) {
            disable_split_comm(pthread_self());
            mpi_errno =
                create_sharp_comm(comm->handle, comm->local_size, comm->rank);
            MPIR_ERR_CHECK(mpi_errno);
            enable_split_comm(pthread_self());
        }
#endif /* (_SHARP_SUPPORT_) */
    }
#endif /* _OSU_COLLECTIVES_ */
fn_fail:
    return mpi_errno;
}

static inline int MPIDI_MVP_Reduce_scatter(const void *sendbuf, void *recvbuf,
                                           const int recvcounts[],
                                           MPI_Datatype datatype, MPI_Op op,
                                           MPIR_Comm *comm,
                                           MPIR_Errflag_t *errflag)
{
#ifdef _OSU_COLLECTIVES_
    if (MVP_USE_OSU_COLLECTIVES &&
        comm->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        return MPIR_Reduce_scatter_MVP(sendbuf, recvbuf, recvcounts, datatype,
                                       op, comm, errflag);
    }
#endif

    return MPIR_Reduce_scatter_impl(sendbuf, recvbuf, recvcounts, datatype, op,
                                    comm, errflag);
}

static inline int MPIDI_MVP_Reduce_scatter_block(const void *sendbuf,
                                                 void *recvbuf, int recvcount,
                                                 MPI_Datatype datatype,
                                                 MPI_Op op, MPIR_Comm *comm,
                                                 MPIR_Errflag_t *errflag)
{
#ifdef _OSU_COLLECTIVES_
    if (MVP_USE_OSU_COLLECTIVES &&
        comm->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        return MPIR_Reduce_scatter_block_MVP(sendbuf, recvbuf, recvcount,
                                             datatype, op, comm, errflag);
    }
#endif

    return MPIR_Reduce_scatter_block_impl(sendbuf, recvbuf, recvcount, datatype,
                                          op, comm, errflag);
}

static inline int MPIDI_MVP_Scan(const void *sendbuf, void *recvbuf, int count,
                                 MPI_Datatype datatype, MPI_Op op,
                                 MPIR_Comm *comm, MPIR_Errflag_t *errflag)
{
#ifdef _OSU_COLLECTIVES_
    if (MVP_USE_OSU_COLLECTIVES &&
        comm->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        return MPIR_Scan_MVP(sendbuf, recvbuf, count, datatype, op, comm,
                             errflag);
    }
#endif

    return MPIR_Scan_impl(sendbuf, recvbuf, count, datatype, op, comm, errflag);
}

static inline int MPIDI_MVP_Exscan(const void *sendbuf, void *recvbuf,
                                   int count, MPI_Datatype datatype, MPI_Op op,
                                   MPIR_Comm *comm, MPIR_Errflag_t *errflag)
{
#ifdef _OSU_COLLECTIVES_
    if (MVP_USE_OSU_COLLECTIVES &&
        comm->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        return MPIR_Exscan_MVP(sendbuf, recvbuf, count, datatype, op, comm,
                               errflag);
    }
#endif

    return MPIR_Exscan_impl(sendbuf, recvbuf, count, datatype, op, comm,
                            errflag);
}

static inline int MPIDI_MVP_Iallreduce(const void *sendbuf, void *recvbuf,
                                       int count, MPI_Datatype datatype,
                                       MPI_Op op, MPIR_Comm *comm,
                                       MPIR_Request **request)
{
    int mpi_errno = MPI_SUCCESS;
#if defined(_SHARP_SUPPORT_)
    if (comm->dev.ch.is_sharp_ok == 1 && count <= MVP_SHARP_MAX_MSG_SIZE / 4 &&
        MVP_ENABLE_SHARP == 2) {
        /* Direct flat algorithm in which every process calls Sharp
         *  MVP_ENABLE_SHARP should be set to 2 */
        mpi_errno = MPIR_Sharp_Iallreduce_MVP(sendbuf, recvbuf, count, datatype,
                                              op, comm, NULL, request);
        if (mpi_errno == MPI_SUCCESS) {
            goto cont_sharp;
        }
        /* SHARP collective is not supported,
         * continue without using SHARP */
    }
#endif /* end of defined (_SHARP_SUPPORT_) */

    mpi_errno = MPIR_Iallreduce_impl(sendbuf, recvbuf, count, datatype, op,
                                     comm, request);
#ifdef _OSU_COLLECTIVES_
#if defined(_SHARP_SUPPORT_)
cont_sharp:
#endif /* defined(_SHARP_SUPPORT_) */
    if (MVP_USE_OSU_COLLECTIVES) {
        if (comm->dev.ch.shmem_coll_ok == 0) {
            mpi_errno = mvp_increment_shmem_coll_counter(comm);
            MPIR_ERR_CHECK(mpi_errno);
        }
#if defined(_SHARP_SUPPORT_)
        comm->dev.ch.iallreduce_coll_count++;
        if (MVP_ENABLE_SHARP && MVP_ENABLE_SHARP_IALLREDUCE &&
            (comm->dev.ch.is_sharp_ok == 0) &&
            (comm->dev.ch.shmem_coll_ok == 1) &&
            (comm->dev.ch.iallreduce_coll_count >=
             MVP_TWO_LEVEL_COMM_THRESHOLD)) {
            disable_split_comm(pthread_self());
            mpi_errno =
                create_sharp_comm(comm->handle, comm->local_size, comm->rank);
            MPIR_ERR_CHECK(mpi_errno);
            enable_split_comm(pthread_self());
        }
#endif /*(_SHARP_SUPPORT_)*/
    }
#endif /* _OSU_COLLECTIVES_ */
fn_fail:
    return mpi_errno;
}

static inline int MPIDI_MVP_Ibarrier(MPIR_Comm *comm, MPIR_Request **request)
{
    int mpi_errno = MPI_SUCCESS;
#if defined(_SHARP_SUPPORT_)
    if (comm->dev.ch.is_sharp_ok == 1 && MVP_ENABLE_SHARP == 2) {
        mpi_errno = MPIR_Sharp_Ibarrier_MVP(comm, NULL, request);
        if (mpi_errno == MPI_SUCCESS) {
            goto cont_sharp;
        }
        /* SHARP collective is not supported,
         * continue without using SHARP */
    }
#endif /* end of defined (_SHARP_SUPPORT_) */

    mpi_errno = MPIR_Ibarrier_impl(comm, request);

#ifdef _OSU_COLLECTIVES_
#if defined(_SHARP_SUPPORT_)
cont_sharp:
#endif /* defined(_SHARP_SUPPORT_) */
    if (MVP_USE_OSU_COLLECTIVES) {
        if (comm->dev.ch.shmem_coll_ok == 0) {
            mpi_errno = mvp_increment_shmem_coll_counter(comm);
            MPIR_ERR_CHECK(mpi_errno);
        }
#if defined(_SHARP_SUPPORT_)
        comm->dev.ch.ibarrier_coll_count++;
        if (MVP_ENABLE_SHARP && MVP_ENABLE_SHARP_IBARRIER &&
            (comm->dev.ch.is_sharp_ok == 0) &&
            (comm->dev.ch.shmem_coll_ok == 1) &&
            (comm->dev.ch.ibarrier_coll_count >=
             MVP_TWO_LEVEL_COMM_THRESHOLD)) {
            disable_split_comm(pthread_self());
            mpi_errno =
                create_sharp_comm(comm->handle, comm->local_size, comm->rank);
            MPIR_ERR_CHECK(mpi_errno);
            enable_split_comm(pthread_self());
        }
#endif /*(_SHARP_SUPPORT_)*/
    }
#endif /* _OSU_COLLECTIVES_ */
fn_fail:
    return mpi_errno;
}

static inline int MPIDI_MVP_Ibcast(void *buffer, int count,
                                   MPI_Datatype datatype, int root,
                                   MPIR_Comm *comm, MPIR_Request **request)
{
    int mpi_errno = MPI_SUCCESS;
#if defined(_SHARP_SUPPORT_)
    if (comm->dev.ch.is_sharp_ok == 1 && count <= MVP_SHARP_MAX_MSG_SIZE / 4 &&
        MVP_ENABLE_SHARP == 2) {
        mpi_errno = MPIR_Sharp_Ibcast_MVP(buffer, count, datatype, root, comm,
                                          NULL, request);
        /* Direct flat algorithm in which every process calls Sharp
         * MVP_ENABLE_SHARP should be set to 2 */
        if (mpi_errno == MPI_SUCCESS) {
            goto cont_sharp;
        }
        /* SHARP collective is not supported,
         * continue without using SHARP */
    }
#endif /* end of defined (_SHARP_SUPPORT_) */

    mpi_errno = MPIR_Ibcast_impl(buffer, count, datatype, root, comm, request);

#ifdef _OSU_COLLECTIVES_
#if defined(_SHARP_SUPPORT_)
cont_sharp:
#endif /* defined(_SHARP_SUPPORT_) */
    if (MVP_USE_OSU_COLLECTIVES) {
        if (comm->dev.ch.shmem_coll_ok == 0) {
            mpi_errno = mvp_increment_shmem_coll_counter(comm);
            MPIR_ERR_CHECK(mpi_errno);
        }
#if defined(_SHARP_SUPPORT_)
        comm->dev.ch.ibcast_coll_count++;
        if (MVP_ENABLE_SHARP && MVP_ENABLE_SHARP_IBCAST &&
            (comm->dev.ch.is_sharp_ok == 0) &&
            (comm->dev.ch.shmem_coll_ok == 1) &&
            (comm->dev.ch.ibcast_coll_count >= MVP_TWO_LEVEL_COMM_THRESHOLD)) {
            disable_split_comm(pthread_self());
            mpi_errno =
                create_sharp_comm(comm->handle, comm->local_size, comm->rank);
            MPIR_ERR_CHECK(mpi_errno);
            enable_split_comm(pthread_self());
        }
#endif /*(_SHARP_SUPPORT_)*/
    }
#endif /* _OSU_COLLECTIVES_ */
fn_fail:
    return mpi_errno;
}

static inline int MPIDI_MVP_Ireduce(void *sendbuf, void *recvbuf, int count,
                                    MPI_Datatype datatype, MPI_Op op, int root,
                                    MPIR_Comm *comm, MPIR_Request **request)
{
    int mpi_errno = MPI_SUCCESS;
#if defined(_SHARP_SUPPORT_)
    if (comm->dev.ch.is_sharp_ok == 1 && count <= MVP_SHARP_MAX_MSG_SIZE / 4 &&
        MVP_ENABLE_SHARP == 2) {
        mpi_errno = MPIR_Sharp_Ireduce_MVP(sendbuf, recvbuf, count, datatype,
                                           op, root, comm, NULL, request);
        /* Direct flat algorithm in which every process calls Sharp
         * MVP_ENABLE_SHARP should be set to 2 */
        if (mpi_errno == MPI_SUCCESS) {
            goto cont_sharp;
        }
        /* SHARP collective is not supported,
         * continue without using SHARP */
    }
#endif /* end of defined (_SHARP_SUPPORT_) */

    mpi_errno = MPIR_Ireduce_impl(sendbuf, recvbuf, count, datatype, op, root,
                                  comm, request);

#ifdef _OSU_COLLECTIVES_
#if defined(_SHARP_SUPPORT_)
cont_sharp:
#endif /* defined(_SHARP_SUPPORT_) */
    if (MVP_USE_OSU_COLLECTIVES) {
        if (comm->dev.ch.shmem_coll_ok == 0) {
            mpi_errno = mvp_increment_shmem_coll_counter(comm);
            MPIR_ERR_CHECK(mpi_errno);
        }
#if defined(_SHARP_SUPPORT_)
        comm->dev.ch.ireduce_coll_count++;
        if (MVP_ENABLE_SHARP && MVP_ENABLE_SHARP_IREDUCE &&
            (comm->dev.ch.is_sharp_ok == 0) &&
            (comm->dev.ch.shmem_coll_ok == 1) &&
            (comm->dev.ch.ireduce_coll_count >= MVP_TWO_LEVEL_COMM_THRESHOLD)) {
            disable_split_comm(pthread_self());
            mpi_errno =
                create_sharp_comm(comm->handle, comm->local_size, comm->rank);
            MPIR_ERR_CHECK(mpi_errno);
            enable_split_comm(pthread_self());
        }
#endif /*(_SHARP_SUPPORT_)*/
    }
#endif /* _OSU_COLLECTIVES_ */
fn_fail:
    return mpi_errno;
}

/* undefine the MPICH versions */
/* TODO-merge: this is kind of hacky, should find a better
 * way to override these while staying out of MPICH code */
#undef MPID_Barrier
#undef MPID_Bcast
#undef MPID_Allreduce
#undef MPID_Allgather
#undef MPID_Allgatherv
#undef MPID_Scatter
#undef MPID_Gather
#undef MPID_Gatherv
#undef MPID_Alltoall
#undef MPID_AlltoallV
#undef MPID_Reduce
#undef MPID_Reduce_scatter
#undef MPID_Reduce_scatter_block
#undef MPID_Scan
#undef MPID_Exscan
/* nonblocking collectives with SHARP support */
#undef MPID_Iallreduce
#undef MPID_Ibarrier
#undef MPID_Ibcast
#undef MPID_Ireduce

/* override functions with ours */
#define MPID_Barrier              MPIDI_MVP_Barrier
#define MPID_Bcast                MPIDI_MVP_Bcast
#define MPID_Allreduce            MPIDI_MVP_Allreduce
#define MPID_Allgather            MPIDI_MVP_Allgather
#define MPID_Allgatherv           MPIDI_MVP_Allgatherv
#define MPID_Scatter              MPIDI_MVP_Scatter
#define MPID_Scatterv             MPIDI_MVP_Scatterv
#define MPID_Gather               MPIDI_MVP_Gather
#define MPID_Gatherv              MPIDI_MVP_Gatherv
#define MPID_Alltoall             MPIDI_MVP_Alltoall
#define MPID_AlltoallV            MPIDI_MVP_AlltoallV
#define MPID_Reduce               MPIDI_MVP_Reduce
#define MPID_Reduce_scatter       MPIDI_MVP_Reduce_scatter
#define MPID_Reduce_scatter_block MPIDI_MVP_Reduce_scatter_block
#define MPID_Scan                 MPIDI_MVP_Scan
#define MPID_Exscan               MPIDI_MVP_Exscan
/* nonblocking collectives with SHARP support */
#define MPID_Iallreduce MPIDI_MVP_Iallreduce
#define MPID_Ibarrier   MPIDI_MVP_Ibarrier
#define MPID_Ibcast     MPIDI_MVP_Ibcast
#define MPID_Ireduce    MPIDI_MVP_Ireduce

#endif /* MPIDI_MVP_CH3_COLL_INCLUDED */
