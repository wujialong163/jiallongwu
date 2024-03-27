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

/* Collective overloads for the CH4 device in MVP. Placed here as it applies to
 * all CH4 netmods
 *
 * These collectives are implemented at the MPIR layer, but we are wrapping them
 * here at the MPIDI layer to overload the netmod defaults.
 *
 * Note that most nonblocking collectives have been dropped from our overloads
 * as they are obsolete and use the deprecated sched algorithms. We do maintain
 * the collectives for which sharp support exists.
 */

#ifndef MVP_COLL_OVERRIDE_H
#define MVP_COLL_OVERRIDE_H

#ifdef _OSU_COLLECTIVES_
/*
 * override select ofi/ucx collectives
 *
 * this is not a clean method but it's a start
 */
#include "mvp_mpir.h"
#include "mvp_offload_coll_init.h"

/*
 * probably need to include a ch4_shmem_coll.h file to disable
 * most of our channel specific shmem colls
 */

extern int shmem_coll_count_threshold;
#if defined(_MCST_SUPPORT_)
extern int mvp_use_mcast_scatter;
#endif /*  #if defined(_MCST_SUPPORT_) */
int mvp_increment_shmem_coll_counter(MPIR_Comm *comm_ptr);
int mvp_increment_allgather_coll_counter(MPIR_Comm *comm_ptr);
int disable_split_comm(pthread_t);
int enable_split_comm(pthread_t);
#if defined(_MCST_SUPPORT_)
int create_mcast_comm(MPI_Comm comm, int size, int my_rank);
#endif /* defined(_MCST_SUPPORT_) */

MPL_STATIC_INLINE_PREFIX int MPIDI_NM_MVP_mpi_barrier(MPIR_Comm *comm,
                                                      MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;
    if (MVP_USE_OSU_COLLECTIVES &&
        comm->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        mpi_errno = MPIR_Barrier_MVP(comm, errflag);
    } else {
        mpi_errno = MPIR_Barrier_impl(comm, errflag);
    }
    if (MVP_USE_OSU_COLLECTIVES) {
        mpi_errno = mvp_increment_shmem_coll_counter(comm);
        MPIR_ERR_CHECK(mpi_errno);
        if (comm->dev.ch.allgather_comm_ok == 0) {
            mpi_errno = mvp_increment_allgather_coll_counter(comm);
            MPIR_ERR_CHECK(mpi_errno);
        }
#if defined(_SHARP_SUPPORT_)
        mpi_errno = MPIDI_MVP_offload_comm_setup_barrier(comm);
        MPIR_ERR_CHECK(mpi_errno);
#endif /*(_SHARP_SUPPORT_)*/
    }
fn_fail:
    return mpi_errno;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_NM_MVP_mpi_bcast(void *buffer, int count,
                                                    MPI_Datatype datatype,
                                                    int root, MPIR_Comm *comm,
                                                    MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;
    if (MVP_USE_OSU_COLLECTIVES &&
        comm->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        mpi_errno =
            MPIR_Bcast_MVP(buffer, count, datatype, root, comm, errflag);
    } else {
        mpi_errno =
            MPIR_Bcast_impl(buffer, count, datatype, root, comm, errflag);
    }
    if (MVP_USE_OSU_COLLECTIVES) {
        mpi_errno = mvp_increment_shmem_coll_counter(comm);
        MPIR_ERR_CHECK(mpi_errno);
        if (comm->dev.ch.allgather_comm_ok == 0) {
            mpi_errno = mvp_increment_allgather_coll_counter(comm);
            MPIR_ERR_CHECK(mpi_errno);
        }
#if defined(_SHARP_SUPPORT_)
        mpi_errno = MPIDI_MVP_offload_comm_setup_bcast(comm);
        MPIR_ERR_CHECK(mpi_errno);
#endif /*(_SHARP_SUPPORT_)*/
    }
fn_fail:
    return mpi_errno;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_NM_MVP_mpi_allreduce(
    const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
    MPI_Op op, MPIR_Comm *comm, MPIR_Errflag_t *errflag)
{
    int mpi_errno = 0;
    if (MVP_USE_OSU_COLLECTIVES &&
        comm->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        mpi_errno = MPIR_Allreduce_MVP(sendbuf, recvbuf, count, datatype, op,
                                       comm, errflag);
        MPIR_ERR_CHECK(mpi_errno);
    } else {
        mpi_errno = MPIR_Allreduce_impl(sendbuf, recvbuf, count, datatype, op,
                                        comm, errflag);
        MPIR_ERR_CHECK(mpi_errno);
    }
    if (MVP_USE_OSU_COLLECTIVES) {
        mpi_errno = mvp_increment_shmem_coll_counter(comm);
        MPIR_ERR_CHECK(mpi_errno);
        if (comm->dev.ch.allgather_comm_ok == 0) {
            mpi_errno = mvp_increment_allgather_coll_counter(comm);
            MPIR_ERR_CHECK(mpi_errno);
        }
#if defined(_SHARP_SUPPORT_)
        mpi_errno = MPIDI_MVP_offload_comm_setup_allreduce(comm);
        MPIR_ERR_CHECK(mpi_errno);
#endif /*(_SHARP_SUPPORT_)*/
    }
fn_fail:
    return mpi_errno;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_NM_MVP_mpi_allgather(
    const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf,
    int recvcount, MPI_Datatype recvtype, MPIR_Comm *comm,
    MPIR_Errflag_t *errflag)
{
    if (MVP_USE_OSU_COLLECTIVES &&
        comm->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        return MPIR_Allgather_MVP(sendbuf, sendcount, sendtype, recvbuf,
                                  recvcount, recvtype, comm, errflag);
    }
    return MPIR_Allgather_impl(sendbuf, sendcount, sendtype, recvbuf, recvcount,
                               recvtype, comm, errflag);
}

MPL_STATIC_INLINE_PREFIX int MPIDI_NM_MVP_mpi_allgatherv(
    const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf,
    const int *recvcounts, const int *displs, MPI_Datatype recvtype,
    MPIR_Comm *comm, MPIR_Errflag_t *errflag)
{
    if (MVP_USE_OSU_COLLECTIVES &&
        comm->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        return MPIR_Allgatherv_MVP(sendbuf, sendcount, sendtype, recvbuf,
                                   recvcounts, displs, recvtype, comm, errflag);
    }
    return MPIR_Allgatherv_impl(sendbuf, sendcount, sendtype, recvbuf,
                                recvcounts, displs, recvtype, comm, errflag);
}

MPL_STATIC_INLINE_PREFIX int MPIDI_NM_MVP_mpi_scatter(
    const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf,
    int recvcount, MPI_Datatype recvtype, int root, MPIR_Comm *comm,
    MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;
    if (MVP_USE_OSU_COLLECTIVES &&
        comm->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        mpi_errno = MPIR_Scatter_MVP(sendbuf, sendcount, sendtype, recvbuf,
                                     recvcount, recvtype, root, comm, errflag);
        MPIR_ERR_CHECK(mpi_errno);
    } else {
        mpi_errno = MPIR_Scatter_impl(sendbuf, sendcount, sendtype, recvbuf,
                                      recvcount, recvtype, root, comm, errflag);
    }
    if (MVP_USE_OSU_COLLECTIVES) {
        if (comm->dev.ch.shmem_coll_ok == 0) {
            mpi_errno = mvp_increment_shmem_coll_counter(comm);
            MPIR_ERR_CHECK(mpi_errno);
        }
#if defined(_SHARP_SUPPORT_)
        mpi_errno = MPIDI_MVP_offload_comm_setup_scatter(comm);
        MPIR_ERR_CHECK(mpi_errno);
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
fn_fail:
    return mpi_errno;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_NM_MVP_mpi_scatterv(
    const void *sendbuf, const int *sendcounts, const int *displs,
    MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype,
    int root, MPIR_Comm *comm, MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;
#if defined(_SHARP_SUPPORT_)
    if (comm->dev.ch.is_sharp_ok == 1 && MVP_ENABLE_SHARP == 1 &&
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

#if defined(_SHARP_SUPPORT_)
cont_sharp:
#endif /* defined(_SHARP_SUPPORT_) */
    if (MVP_USE_OSU_COLLECTIVES) {
        if (comm->dev.ch.shmem_coll_ok == 0) {
            mpi_errno = mvp_increment_shmem_coll_counter(comm);
            MPIR_ERR_CHECK(mpi_errno);
        }
#if defined(_SHARP_SUPPORT_)
        mpi_errno = MPIDI_MVP_offload_comm_setup_scatterv(comm);
        MPIR_ERR_CHECK(mpi_errno);
#endif /*(_SHARP_SUPPORT_)*/
    }
fn_fail:
    return mpi_errno;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_NM_MVP_mpi_gather(
    const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf,
    int recvcount, MPI_Datatype recvtype, int root, MPIR_Comm *comm,
    MPIR_Errflag_t *errflag)
{
    if (MVP_USE_OSU_COLLECTIVES &&
        comm->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        return MPIR_Gather_MVP(sendbuf, sendcount, sendtype, recvbuf, recvcount,
                               recvtype, root, comm, errflag);
    }
    return MPIR_Gather_impl(sendbuf, sendcount, sendtype, recvbuf, recvcount,
                            recvtype, root, comm, errflag);
}

#if 0 /* Not implemented */
MPL_STATIC_INLINE_PREFIX int MPIDI_NM_MVP_mpi_gatherv(const void *sendbuf, int sendcount,
                                    MPI_Datatype sendtype, void *recvbuf,
                                    const int *recvcounts, const int *displs,
                                    MPI_Datatype recvtype, int root,
                                    MPIR_Comm *comm, MPIR_Errflag_t *errflag)
{
    if (MVP_USE_OSU_COLLECTIVES &&
        comm->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        return MPIR_Gatherv_MVP(sendbuf, sendcount, sendtype, recvbuf, 
                                recvcounts, displs, recvtype, root, comm,
                                errflag);
    }

    return MPIR_Gatherv_impl(sendbuf, sendcount, sendtype, recvbuf,
                             recvcounts, displs, recvtype, root, comm, errflag);

}
#endif

MPL_STATIC_INLINE_PREFIX int MPIDI_NM_MVP_mpi_alltoall(
    const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf,
    int recvcount, MPI_Datatype recvtype, MPIR_Comm *comm,
    MPIR_Errflag_t *errflag)
{
    if (MVP_USE_OSU_COLLECTIVES &&
        comm->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        return MPIR_Alltoall_MVP(sendbuf, sendcount, sendtype, recvbuf,
                                 recvcount, recvtype, comm, errflag);
    }

    return MPIR_Alltoall_impl(sendbuf, sendcount, sendtype, recvbuf, recvcount,
                              recvtype, comm, errflag);
}

MPL_STATIC_INLINE_PREFIX int MPIDI_NM_MVP_mpi_alltoallv(
    const void *sendbuf, const int *sendcounts, const int *sdispls,
    MPI_Datatype sendtype, void *recvbuf, const int *recvcounts,
    const int *rdispls, MPI_Datatype recvtype, MPIR_Comm *comm,
    MPIR_Errflag_t *errflag)
{
    if (MVP_USE_OSU_COLLECTIVES &&
        comm->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        return MPIR_Alltoallv_MVP(sendbuf, sendcounts, sdispls, sendtype,
                                  recvbuf, recvcounts, rdispls, recvtype, comm,
                                  errflag);
    }

    return MPIR_Alltoallv_impl(sendbuf, sendcounts, sdispls, sendtype, recvbuf,
                               recvcounts, rdispls, recvtype, comm, errflag);
}

MPL_STATIC_INLINE_PREFIX int MPIDI_NM_MVP_mpi_reduce(
    const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
    MPI_Op op, int root, MPIR_Comm *comm, MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;
    if (MVP_USE_OSU_COLLECTIVES &&
        comm->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        mpi_errno = MPIR_Reduce_MVP(sendbuf, recvbuf, count, datatype, op, root,
                                    comm, errflag);
    } else {
        mpi_errno = MPIR_Reduce_impl(sendbuf, recvbuf, count, datatype, op,
                                     root, comm, errflag);
    }
    if (MVP_USE_OSU_COLLECTIVES) {
        if (comm->dev.ch.shmem_coll_ok == 0) {
            mpi_errno = mvp_increment_shmem_coll_counter(comm);
            MPIR_ERR_CHECK(mpi_errno);
        }
#if defined(_SHARP_SUPPORT_)
        MPIDI_MVP_offload_comm_setup_reduce(comm);
        MPIR_ERR_CHECK(mpi_errno);
#endif /* (_SHARP_SUPPORT_) */
    }
fn_fail:
    return mpi_errno;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_NM_MVP_mpi_reduce_scatter(
    const void *sendbuf, void *recvbuf, const int recvcounts[],
    MPI_Datatype datatype, MPI_Op op, MPIR_Comm *comm, MPIR_Errflag_t *errflag)
{
    if (MVP_USE_OSU_COLLECTIVES &&
        comm->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        return MPIR_Reduce_scatter_MVP(sendbuf, recvbuf, recvcounts, datatype,
                                       op, comm, errflag);
    }

    return MPIR_Reduce_scatter_impl(sendbuf, recvbuf, recvcounts, datatype, op,
                                    comm, errflag);
}

#if 0 /* Not implemented */
MPL_STATIC_INLINE_PREFIX int MPIDI_NM_MVP_mpi_reduce_scatter_block(const void *sendbuf,
                                                 void *recvbuf, int recvcount,
                                                 MPI_Datatype datatype,
                                                 MPI_Op op, MPIR_Comm *comm,
                                                 MPIR_Errflag_t *errflag)
{
    if (MVP_USE_OSU_COLLECTIVES &&
        comm->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        return MPIR_Reduce_scatter_block_MVP(sendbuf, recvbuf, recvcount,
                                             datatype, op, comm, 
                                             errflag);
    }

    return MPIR_Reduce_scatter_block_impl(sendbuf, recvbuf, recvcount,
                                          datatype, op, comm,
                                          errflag);
}

MPL_STATIC_INLINE_PREFIX int MPIDI_NM_MVP_mpi_scan(const void *sendbuf, void *recvbuf, int count,
                                 MPI_Datatype datatype, MPI_Op op,
                                 MPIR_Comm *comm, MPIR_Errflag_t *errflag)
{
    if (MVP_USE_OSU_COLLECTIVES &&
        comm->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        return MPIR_Scan_MVP(sendbuf, recvbuf, count, datatype, op,
                                        comm, errflag);
    }

    return MPIR_Scan_impl(sendbuf, recvbuf, count, datatype, op, comm,
                               errflag);
}

MPL_STATIC_INLINE_PREFIX int MPIDI_NM_MVP_mpi_exscan(const void *sendbuf, void *recvbuf, int count,
                              MPI_Datatype datatype, MPI_Op op, MPIR_Comm *comm,
                              MPIR_Errflag_t *errflag)
{
    if (MVP_USE_OSU_COLLECTIVES && 
        comm->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        return MPIR_Exscan_MVP(sendbuf, recvbuf, count, datatype, op,
                               comm, errflag);
    }

    return MPIR_Exscan_impl(sendbuf, recvbuf, count, datatype, op, comm,
                                 errflag);
}
#endif

MPL_STATIC_INLINE_PREFIX int MPIDI_NM_MVP_mpi_iallreduce(
    const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
    MPI_Op op, MPIR_Comm *comm, MPIR_Request **request)
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
#if defined(_SHARP_SUPPORT_)
cont_sharp:
#endif /* defined(_SHARP_SUPPORT_) */
    if (MVP_USE_OSU_COLLECTIVES) {
        if (comm->dev.ch.shmem_coll_ok == 0) {
            mpi_errno = mvp_increment_shmem_coll_counter(comm);
            MPIR_ERR_CHECK(mpi_errno);
        }
#if defined(_SHARP_SUPPORT_)
        mpi_errno = MPIDI_MVP_offload_comm_setup_iallreduce(comm);
        MPIR_ERR_CHECK(mpi_errno);
#endif /*(_SHARP_SUPPORT_)*/
    }
fn_fail:
    return mpi_errno;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_NM_MVP_mpi_ibarrier(MPIR_Comm *comm,
                                                       MPIR_Request **request)
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

#if defined(_SHARP_SUPPORT_)
cont_sharp:
#endif /* defined(_SHARP_SUPPORT_) */
    if (MVP_USE_OSU_COLLECTIVES) {
        if (comm->dev.ch.shmem_coll_ok == 0) {
            mpi_errno = mvp_increment_shmem_coll_counter(comm);
            MPIR_ERR_CHECK(mpi_errno);
        }
#if defined(_SHARP_SUPPORT_)
        mpi_errno = MPIDI_MVP_offload_comm_setup_ibarrier(comm);
        MPIR_ERR_CHECK(mpi_errno);
#endif /*(_SHARP_SUPPORT_)*/
    }
fn_fail:
    return mpi_errno;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_NM_MVP_mpi_ibcast(void *buffer, int count,
                                                     MPI_Datatype datatype,
                                                     int root, MPIR_Comm *comm,
                                                     MPIR_Request **request)
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

#if defined(_SHARP_SUPPORT_)
cont_sharp:
#endif /* defined(_SHARP_SUPPORT_) */
    if (MVP_USE_OSU_COLLECTIVES) {
        if (comm->dev.ch.shmem_coll_ok == 0) {
            mpi_errno = mvp_increment_shmem_coll_counter(comm);
            MPIR_ERR_CHECK(mpi_errno);
        }
#if defined(_SHARP_SUPPORT_)
        mpi_errno = MPIDI_MVP_offload_comm_setup_ibcast(comm);
        MPIR_ERR_CHECK(mpi_errno);
#endif /*(_SHARP_SUPPORT_)*/
    }
fn_fail:
    return mpi_errno;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_NM_MVP_mpi_ireduce(
    const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
    MPI_Op op, int root, MPIR_Comm *comm, MPIR_Request **request)
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

#if defined(_SHARP_SUPPORT_)
cont_sharp:
#endif /* defined(_SHARP_SUPPORT_) */
    if (MVP_USE_OSU_COLLECTIVES) {
        if (comm->dev.ch.shmem_coll_ok == 0) {
            mpi_errno = mvp_increment_shmem_coll_counter(comm);
            MPIR_ERR_CHECK(mpi_errno);
        }
#if defined(_SHARP_SUPPORT_)
        mpi_errno = MPIDI_MVP_offload_comm_setup_ireduce(comm);
        MPIR_ERR_CHECK(mpi_errno);
#endif /*(_SHARP_SUPPORT_)*/
    }
fn_fail:
    return mpi_errno;
}

/* undefine the MPICH versions */
/* TODO-merge: this is kind of hacky, should find a better
 * way to override these while staying out of MPICH code */
#undef MPIDI_NM_mpi_barrier
#undef MPIDI_NM_mpi_bcast
#undef MPIDI_NM_mpi_allreduce
#undef MPIDI_NM_mpi_allgather
#undef MPIDI_NM_mpi_allgatherv
#undef MPIDI_NM_mpi_scatter
#undef MPIDI_NM_mpi_gather
#undef MPIDI_NM_mpi_gatherv
#undef MPIDI_NM_mpi_alltoall
#undef MPIDI_NM_mpi_alltoallV
#undef MPIDI_NM_mpi_reduce
#undef MPIDI_NM_mpi_reduce_scatter
#undef MPIDI_NM_mpi_reduce_scatter_block
#undef MPIDI_NM_mpi_scan
#undef MPIDI_NM_mpi_exscan
/* nonblocking collectives with SHARP support */
#undef MPIDI_NM_mpi_iallreduce
#undef MPIDI_NM_mpi_ibarrier
#undef MPIDI_NM_mpi_ibcast
#undef MPIDI_NM_mpi_ireduce

/* override functions with ours */
#define MPIDI_NM_mpi_barrier    MPIDI_NM_MVP_mpi_barrier
#define MPIDI_NM_mpi_bcast      MPIDI_NM_MVP_mpi_bcast
#define MPIDI_NM_mpi_allreduce  MPIDI_NM_MVP_mpi_allreduce
#define MPIDI_NM_mpi_allgather  MPIDI_NM_MVP_mpi_allgather
#define MPIDI_NM_mpi_allgatherv MPIDI_NM_MVP_mpi_allgatherv
#define MPIDI_NM_mpi_scatter    MPIDI_NM_MVP_mpi_scatter
#define MPIDI_NM_mpi_scatterv   MPIDI_NM_MVP_mpi_scatterv
#define MPIDI_NM_mpi_gather     MPIDI_NM_MVP_mpi_gather
/* #define MPIDI_NM_mpi_gatherv                MPIDI_NM_MVP_mpi_gatherv */
#define MPIDI_NM_mpi_alltoall       MPIDI_NM_MVP_mpi_alltoall
#define MPIDI_NM_mpi_alltoallv      MPIDI_NM_MVP_mpi_alltoallv
#define MPIDI_NM_mpi_reduce         MPIDI_NM_MVP_mpi_reduce
#define MPIDI_NM_mpi_reduce_scatter MPIDI_NM_MVP_mpi_reduce_scatter
/*
#define MPIDI_NM_mpi_reduce_scatter_block MPIDI_NM_MVP_mpi_reduce_scatter_block
#define MPIDI_NM_mpi_scan                   MPIDI_NM_MVP_mpi_scan
#define MPIDI_NM_mpi_exscan                 MPIDI_NM_MVP_mpi_exscan
*/
/* nonblocking collectives with SHARP support */
#define MPIDI_NM_mpi_iallreduce MPIDI_NM_MVP_mpi_iallreduce
#define MPIDI_NM_mpi_ibarrier   MPIDI_NM_MVP_mpi_ibarrier
#define MPIDI_NM_mpi_ibcast     MPIDI_NM_MVP_mpi_ibcast
#define MPIDI_NM_mpi_ireduce    MPIDI_NM_MVP_mpi_ireduce

#endif /* defined(_OSU_COLLECTIVES_) */

#endif /* MVP_COLL_OVERRIDE_H */
