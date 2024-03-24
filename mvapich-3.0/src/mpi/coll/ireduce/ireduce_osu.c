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

#include "ireduce_tuning.h"

extern unsigned long long PVAR_COUNTER_mvp_coll_ireduce_sharp;
extern MPIR_T_pvar_timer_t PVAR_COUNTER_mvp_coll_timer_ireduce_sharp;

int (*MVP_Ireduce_function)(const void *sendbuf, void *recvbuf, int count,
                            MPI_Datatype datatype, MPI_Op op, int root,
                            MPIR_Comm *comm_ptr, MPIR_Sched_t s) = NULL;

int (*MVP_Ireduce_intra_node_function)(const void *sendbuf, void *recvbuf,
                                       int count, MPI_Datatype datatype,
                                       MPI_Op op, int root, MPIR_Comm *comm_ptr,
                                       MPIR_Sched_t s) = NULL;

#if defined(_SHARP_SUPPORT_)
/* Currently implemented on top of iallreduce. Ideally should use lower level S
 * calls to achieve the same once avaliable*/
int MPIR_Sharp_Ireduce_MVP(const void *sendbuf, void *recvbuf, int count,
                           MPI_Datatype datatype, MPI_Op op, int root,
                           MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag,
                           MPIR_Request **req)
{
    MPIR_TIMER_START(coll, ireduce, sharp);
    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_ireduce_sharp, 1);
    int mpi_errno = MPI_SUCCESS;
    void *new_recvbuf = NULL;
    int rank = comm_ptr->rank;

    if (rank != root) {
        new_recvbuf = (void *)comm_ptr->dev.ch.coll_tmp_buf;
    } else {
        new_recvbuf = (void *)recvbuf;
    }
    mpi_errno = MPIR_Sharp_Iallreduce_MVP(sendbuf, new_recvbuf, count, datatype,
                                          op, comm_ptr, (int *)errflag, req);
    if (mpi_errno) {
        MPIR_ERR_POP(mpi_errno);
    }

fn_exit:
    MPIR_TIMER_END(coll, ireduce, sharp);
    return (mpi_errno);
fn_fail:
    goto fn_exit;
}
#endif /* End of sharp support */

static int MPIR_Ireduce_tune_helper_MVP(const void *sendbuf, void *recvbuf,
                                        int count, MPI_Datatype datatype,
                                        MPI_Op op, int root,
                                        MPIR_Comm *comm_ptr, MPIR_Sched_t s)
{
    int mpi_errno = MPI_SUCCESS;
    int is_homogeneous ATTRIBUTE((unused)), pof2, comm_size;

    MPIR_Assert(comm_ptr->comm_kind == MPIR_COMM_KIND__INTRACOMM);

    is_homogeneous = 1;
#ifdef MPID_HAS_HETERO
    if (comm_ptr->is_hetero)
        is_homogeneous = 0;
#endif
    MPIR_Assert(is_homogeneous);
    comm_size = comm_ptr->local_size;

    pof2 = 1;
    while (pof2 <= comm_size)
        pof2 <<= 1;
    pof2 >>= 1;

    if ((MVP_Ireduce_function ==
         MPIR_Ireduce_intra_sched_reduce_scatter_gather) &&
        (HANDLE_GET_KIND(op) == HANDLE_KIND_BUILTIN) && (count >= pof2)) {
        mpi_errno = MPIR_Ireduce_intra_sched_reduce_scatter_gather(
            sendbuf, recvbuf, count, datatype, op, root, comm_ptr, s);
    } else {
        mpi_errno = MVP_Ireduce_function(sendbuf, recvbuf, count, datatype, op,
                                         root, comm_ptr, s);
    }
    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIR_Ireduce_intra_MVP(const void *sendbuf, void *recvbuf, int count,
                           MPI_Datatype datatype, MPI_Op op, int root,
                           MPIR_Comm *comm_ptr, MPIR_Sched_t s)
{
    int mpi_errno = MPI_SUCCESS;
    int comm_size, is_homogeneous ATTRIBUTE((unused));
    MPI_Aint sendtype_size, nbytes;

    int two_level_ireduce = 1;
    int range = 0;
    int range_threshold = 0;
    int range_threshold_intra = 0;

    MPIR_Assert(comm_ptr->comm_kind == MPIR_COMM_KIND__INTRACOMM);

    is_homogeneous = 1;
#ifdef MPID_HAS_HETERO
    if (comm_ptr->is_hetero)
        is_homogeneous = 0;
#endif
    MPIR_Assert(is_homogeneous); /* we don't handle the hetero case right now */
    comm_size = comm_ptr->local_size;
    MPIR_Datatype_get_size_macro(datatype, sendtype_size);
    nbytes = count * sendtype_size;

    // Search for some parameters regardless of whether subsequent selected
    // algorithm is 2-level or not

    // Search for the corresponding system size inside the tuning table
    while ((range < (mvp_size_ireduce_tuning_table - 1)) &&
           (comm_size > mvp_ireduce_thresholds_table[range].numproc)) {
        range++;
    }

    // Search for corresponding inter-leader function
    while ((range_threshold <
            (mvp_ireduce_thresholds_table[range].size_inter_table - 1)) &&
           (nbytes > mvp_ireduce_thresholds_table[range]
                         .inter_leader[range_threshold]
                         .max) &&
           (mvp_ireduce_thresholds_table[range]
                .inter_leader[range_threshold]
                .max != -1)) {
        range_threshold++;
    }

    // Search for corresponding intra-node function

    // Commenting this for the time being as none of
    // the algorithms are 2-level
    /*
    while ((range_threshold_intra <
            (mvp_ireduce_thresholds_table[range].size_intra_table - 1))
           && (nbytes >
               mvp_ireduce_thresholds_table[range].intra_node[range_threshold_intra].max)
           &&
    (mvp_ireduce_thresholds_table[range].intra_node[range_threshold_intra].max
    != -1)) { range_threshold_intra++;
    }
    */

    MVP_Ireduce_function = mvp_ireduce_thresholds_table[range]
                               .inter_leader[range_threshold]
                               .MVP_pt_Ireduce_function;

    MVP_Ireduce_intra_node_function = mvp_ireduce_thresholds_table[range]
                                          .intra_node[range_threshold_intra]
                                          .MVP_pt_Ireduce_function;

    /* There are currently no two-level nb-reduce functions hence
       setting to 0 by default */
    two_level_ireduce = mvp_ireduce_thresholds_table[range]
                            .is_two_level_ireduce[range_threshold];
    if (1 != two_level_ireduce) {
        mpi_errno = MPIR_Ireduce_tune_helper_MVP(
            sendbuf, recvbuf, count, datatype, op, root, comm_ptr, s);
    } else {
        /* Code path should not enter this with the current algorithms*/
    }

    return mpi_errno;
}

int MPIR_Ireduce_MVP(const void *sendbuf, void *recvbuf, int count,
                     MPI_Datatype datatype, MPI_Op op, int root,
                     MPIR_Comm *comm_ptr, MPIR_Sched_t s)
{
    int mpi_errno = MPI_SUCCESS;

    if (comm_ptr->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
#if defined(CHANNEL_MRAIL) || defined(_MVP_CH4_OVERRIDE_)
        mpi_errno = MPIR_Ireduce_intra_MVP(sendbuf, recvbuf, count, datatype,
                                           op, root, comm_ptr, s);
#else
        mpi_errno = MPIR_Ireduce_intra(sendbuf, recvbuf, count, datatype, op,
                                       root, comm_ptr, s);
#endif /*#if defined(CHANNEL_MRAIL) || defined(_MVP_CH4_OVERRIDE_) */
    } else {
        mpi_errno = MPIR_Ireduce_inter_sched_auto(
            sendbuf, recvbuf, count, datatype, op, root, comm_ptr, s);
    }

    return mpi_errno;
}
