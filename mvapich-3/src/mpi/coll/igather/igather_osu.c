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

#include "mpiimpl.h"

#include "igather_tuning.h"

#if defined(CHANNEL_MRAIL) || defined(_MVP_CH4_OVERRIDE_)

int (*MVP_Igather_function)(const void *sendbuf, int sendcount,
                            MPI_Datatype sendtype, void *recvbuf, int recvcount,
                            MPI_Datatype recvtype, int root,
                            MPIR_Comm *comm_ptr, MPIR_Sched_t s) = NULL;

int (*MVP_Igather_intra_node_function)(const void *sendbuf, int sendcount,
                                       MPI_Datatype sendtype, void *recvbuf,
                                       int recvcount, MPI_Datatype recvtype,
                                       int root, MPIR_Comm *comm_ptr,
                                       MPIR_Sched_t s) = NULL;

static int MPIR_Igather_tune_helper_MVP(const void *sendbuf, int sendcount,
                                        MPI_Datatype sendtype, void *recvbuf,
                                        int recvcount, MPI_Datatype recvtype,
                                        int root, MPIR_Comm *comm_ptr,
                                        MPIR_Sched_t s)
{
    int mpi_errno = MPI_SUCCESS;
    int is_homogeneous ATTRIBUTE((unused));

    MPIR_Assert(comm_ptr->comm_kind == MPIR_COMM_KIND__INTRACOMM);

    is_homogeneous = 1;
#ifdef MPID_HAS_HETERO
    if (comm_ptr->is_hetero)
        is_homogeneous = 0;
#endif
    MPIR_Assert(is_homogeneous);

    mpi_errno = MVP_Igather_function(sendbuf, sendcount, sendtype, recvbuf,
                                     recvcount, recvtype, root, comm_ptr, s);
    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIR_Igather_intra_MVP(const void *sendbuf, int sendcount,
                           MPI_Datatype sendtype, void *recvbuf, int recvcount,
                           MPI_Datatype recvtype, int root, MPIR_Comm *comm_ptr,
                           MPIR_Sched_t s)
{
    int mpi_errno = MPI_SUCCESS;
    int comm_size, is_homogeneous ATTRIBUTE((unused));
    MPI_Aint recvtype_size, sendtype_size, nbytes;

    int rank = comm_ptr->rank;
    int two_level_igather = 1;
    int range = 0;
    int range_threshold = 0;
    int range_threshold_intra = 0;

    MPIR_Assert(comm_ptr->comm_kind == MPIR_COMM_KIND__INTRACOMM);

    is_homogeneous = 1;
    comm_size = comm_ptr->local_size;
#ifdef MPID_HAS_HETERO
    if (comm_ptr->is_hetero)
        is_homogeneous = 0;
#endif
    MPIR_Assert(is_homogeneous); /* we don't handle the hetero case right now */
    if (rank == root) {
        MPIR_Datatype_get_size_macro(recvtype, recvtype_size);
        nbytes = recvcount * recvtype_size;
    } else {
        MPIR_Datatype_get_size_macro(sendtype, sendtype_size);
        nbytes = sendcount * sendtype_size;
    }

    // Search for some parameters regardless of whether subsequent selected
    // algorithm is 2-level or not

    // Search for the corresponding system size inside the tuning table
    while ((range < (mvp_size_igather_tuning_table - 1)) &&
           (comm_size > mvp_igather_thresholds_table[range].numproc)) {
        range++;
    }

    // Search for corresponding inter-leader function
    while ((range_threshold <
            (mvp_igather_thresholds_table[range].size_inter_table - 1)) &&
           (nbytes > mvp_igather_thresholds_table[range]
                         .inter_leader[range_threshold]
                         .max) &&
           (mvp_igather_thresholds_table[range]
                .inter_leader[range_threshold]
                .max != -1)) {
        range_threshold++;
    }

    // Search for corresponding intra-node function

    // Commenting this for the time being as none of
    // the algorithms are 2-level
    /*
    while ((range_threshold_intra <
            (mvp_igather_thresholds_table[range].size_intra_table - 1))
           && (nbytes >
               mvp_igather_thresholds_table[range].intra_node[range_threshold_intra].max)
           &&
    (mvp_igather_thresholds_table[range].intra_node[range_threshold_intra].max
    != -1)) { range_threshold_intra++;
    }
    */

    MVP_Igather_function = mvp_igather_thresholds_table[range]
                               .inter_leader[range_threshold]
                               .MVP_pt_Igather_function;

    MVP_Igather_intra_node_function = mvp_igather_thresholds_table[range]
                                          .intra_node[range_threshold_intra]
                                          .MVP_pt_Igather_function;

    /* There are currently no two-level nb-gather functions hence
       setting to 0 by default */
    two_level_igather = mvp_igather_thresholds_table[range]
                            .is_two_level_igather[range_threshold];
    if (1 != two_level_igather) {
        mpi_errno = MPIR_Igather_tune_helper_MVP(sendbuf, sendcount, sendtype,
                                                 recvbuf, recvcount, recvtype,
                                                 root, comm_ptr, s);
    } else {
        /* Code path should not enter this with the current algorithms*/
    }

    return mpi_errno;
}
#endif /*#if defined(CHANNEL_MRAIL) || defined(_MVP_CH4_OVERRIDE_) */

int MPIR_Igather_MVP(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                     void *recvbuf, int recvcount, MPI_Datatype recvtype,
                     int root, MPIR_Comm *comm_ptr, MPIR_Sched_t s)
{
    int mpi_errno = MPI_SUCCESS;

    if (comm_ptr->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
#if defined(CHANNEL_MRAIL) || defined(_MVP_CH4_OVERRIDE_)
        mpi_errno =
            MPIR_Igather_intra_MVP(sendbuf, sendcount, sendtype, recvbuf,
                                   recvcount, recvtype, root, comm_ptr, s);
#else
        mpi_errno = MPIR_Igather_intra(sendbuf, sendcount, sendtype, recvbuf,
                                       recvcount, recvtype, root, comm_ptr, s);
#endif /*#if defined(CHANNEL_MRAIL) || defined(_MVP_CH4_OVERRIDE_) */
    } else {
        mpi_errno = MPIR_Igather_inter_sched_auto(sendbuf, sendcount, sendtype,
                                                  recvbuf, recvcount, recvtype,
                                                  root, comm_ptr, s);
    }

    return mpi_errno;
}
