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

#include "ibcast_tuning.h"

extern unsigned long long PVAR_COUNTER_mvp_coll_ibcast_sharp;
extern MPIR_T_pvar_timer_t PVAR_COUTNER_mvp_coll_timer_ibcast_sharp;

#if defined(CHANNEL_MRAIL) || defined(_MVP_CH4_OVERRIDE_)

int (*MVP_Ibcast_function)(void *buffer, int count, MPI_Datatype datatype,
                           int root, MPIR_Comm *comm_ptr,
                           MPIR_Sched_t s) = NULL;

int (*MVP_Ibcast_intra_node_function)(void *buffer, int count,
                                      MPI_Datatype datatype, int root,
                                      MPIR_Comm *comm_ptr,
                                      MPIR_Sched_t s) = NULL;

#if defined(_SHARP_SUPPORT_)
#undef FUNCNAME
#define FUNCNAME "MPIR_Sharp_Ibcast_MVP"
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
/* Currently implemented on top of allreduce. Ideally should use lower level
 * Sharp calls to achieve the same once avaliable*/
int MPIR_Sharp_Ibcast_MVP(void *buffer, int count, MPI_Datatype datatype,
                          int root, MPIR_Comm *comm_ptr,
                          MPIR_Errflag_t *errflag, MPIR_Request **req)
{
    MPIR_TIMER_START(coll, ibcast, sharp);
    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_ibcast_sharp, 1);
    int mpi_errno = MPI_SUCCESS;
    void *sendbuf = NULL, *recvbuf = NULL;
    MPI_Aint type_size = 0;
    /* Get size of data */
    MPIR_Datatype_get_size_macro(datatype, type_size);
    intptr_t nbytes = (intptr_t)(count) * (type_size);
    int rank = comm_ptr->rank;

    MPIR_Datatype_get_size_macro(MPI_INT, type_size);
    if (nbytes % type_size != 0) {
        PRINT_DEBUG(DEBUG_Sharp_verbose,
                    "Continue without SHARP: Only supports "
                    "broadcast of an array with number of "
                    "bytes divisible by %ld\n",
                    type_size);
        MPIR_ERR_SETANDJUMP1(mpi_errno, MPI_ERR_INTERN, "**sharpcoll",
                             "Invalid size of bcast array: must be divisible "
                             "by %d",
                             type_size);
    }

    if (rank == root) {
        sendbuf = (void *)buffer;
        recvbuf = (void *)comm_ptr->dev.ch.coll_tmp_buf;
    } else {
        memset(comm_ptr->dev.ch.coll_tmp_buf, 0, nbytes);
        sendbuf = (void *)comm_ptr->dev.ch.coll_tmp_buf;
        recvbuf = (void *)buffer;
    }

    int new_count = nbytes / type_size;

    mpi_errno =
        MPIR_Sharp_Iallreduce_MVP(sendbuf, recvbuf, new_count, MPI_INT, MPI_SUM,
                                  comm_ptr, (int *)errflag, req);
    if (mpi_errno) {
        MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_INTERN, "**sharpcoll");
    }

fn_exit:
    MPIR_TIMER_END(coll, ibcast, sharp);
    return (mpi_errno);
fn_fail:
    goto fn_exit;
}
#endif

static int MPIR_Ibcast_tune_helper_MVP(void *buffer, int count,
                                       MPI_Datatype datatype, int root,
                                       MPIR_Comm *comm_ptr, MPIR_Sched_t s)
{
    int mpi_errno = MPI_SUCCESS;
    int comm_size, is_homogeneous ATTRIBUTE((unused));

    MPIR_Assert(comm_ptr->comm_kind == MPIR_COMM_KIND__INTRACOMM);

    is_homogeneous = 1;
#ifdef MPID_HAS_HETERO
    if (comm_ptr->is_hetero)
        is_homogeneous = 0;
#endif
    MPIR_Assert(is_homogeneous);
    comm_size = comm_ptr->local_size;

    if (MVP_Ibcast_function ==
        MPIR_Ibcast_intra_sched_scatter_recursive_doubling_allgather) {
        if (MPL_is_pof2(comm_size, NULL)) {
            mpi_errno =
                MPIR_Ibcast_intra_sched_scatter_recursive_doubling_allgather(
                    buffer, count, datatype, root, comm_ptr, s);
            MPIR_ERR_CHECK(mpi_errno);
        } else {
            mpi_errno = MPIR_Ibcast_intra_sched_scatter_ring_allgather(
                buffer, count, datatype, root, comm_ptr, s);
            MPIR_ERR_CHECK(mpi_errno);
        }
    } else {
        mpi_errno =
            MVP_Ibcast_function(buffer, count, datatype, root, comm_ptr, s);
        MPIR_ERR_CHECK(mpi_errno);
    }
fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIR_Ibcast_intra_MVP(void *buffer, int count, MPI_Datatype datatype,
                          int root, MPIR_Comm *comm_ptr, MPIR_Sched_t s)
{
    int mpi_errno = MPI_SUCCESS;
    int comm_size, is_homogeneous ATTRIBUTE((unused));
    MPI_Aint type_size, nbytes;

    int two_level_ibcast = 1;
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
    MPIR_Datatype_get_size_macro(datatype, type_size);
    nbytes = type_size * count;

    // Search for some parameters regardless of whether subsequent selected
    // algorithm is 2-level or not

    // Search for the corresponding system size inside the tuning table
    while ((range < (mvp_size_ibcast_tuning_table - 1)) &&
           (comm_size > mvp_ibcast_thresholds_table[range].numproc)) {
        range++;
    }

    /* If tuning table size = 0, i.e the tables are not populated yet */
    /*
    if (0 == range) {
        if ((nbytes < MPIR_CVAR_BCAST_SHORT_MSG_SIZE) ||
            (comm_size < MPIR_CVAR_BCAST_MIN_PROCS))
            {
                mpi_errno = MPIR_Ibcast_binomial(buffer, count, datatype, root,
    comm_ptr, s); MPIR_ERR_CHECK(mpi_errno);
            }
        else
            {
                if ((nbytes < MPIR_CVAR_BCAST_LONG_MSG_SIZE) &&
    (MPL_is_pof2(comm_size, NULL))) { mpi_errno =
    MPIR_Ibcast_intra_sched_scatter_recursive_doubling_allgather(buffer, count,
    datatype, root, comm_ptr, s); MPIR_ERR_CHECK(mpi_errno);
                }
                else {
                    mpi_errno = MPIR_Ibcast_scatter_ring_allgather(buffer,
    count, datatype, root, comm_ptr, s); MPIR_ERR_CHECK(mpi_errno);
                }
            }
        goto fn_exit;
    }
    */

    // Search for corresponding inter-leader function
    while (
        (range_threshold <
         (mvp_ibcast_thresholds_table[range].size_inter_table - 1)) &&
        (nbytes > mvp_ibcast_thresholds_table[range]
                      .inter_leader[range_threshold]
                      .max) &&
        (mvp_ibcast_thresholds_table[range].inter_leader[range_threshold].max !=
         -1)) {
        range_threshold++;
    }

    // Search for corresponding intra-node function

    // Commenting this for the time being as none of
    // the algorithms are 2-level
    /*
    while ((range_threshold_intra <
            (mvp_ibcast_thresholds_table[range].size_intra_table - 1))
           && (nbytes >
               mvp_ibcast_thresholds_table[range].intra_node[range_threshold_intra].max)
           &&
    (mvp_ibcast_thresholds_table[range].intra_node[range_threshold_intra].max !=
               -1)) {
        range_threshold_intra++;
    }
    */

    MVP_Ibcast_function = mvp_ibcast_thresholds_table[range]
                              .inter_leader[range_threshold]
                              .MVP_pt_Ibcast_function;

    MVP_Ibcast_intra_node_function = mvp_ibcast_thresholds_table[range]
                                         .intra_node[range_threshold_intra]
                                         .MVP_pt_Ibcast_function;

    /* There are currently no two-level nb-bcast functions hence
       setting to 0 by default */
    two_level_ibcast =
        mvp_ibcast_thresholds_table[range].is_two_level_ibcast[range_threshold];
    if (1 != two_level_ibcast) {
        mpi_errno = MPIR_Ibcast_tune_helper_MVP(buffer, count, datatype, root,
                                                comm_ptr, s);
    } else {
        /* Code path should not enter this with the current algorithms*/
    }

    return mpi_errno;
}
#endif /*#if defined(CHANNEL_MRAIL) || defined(_MVP_CH4_OVERRIDE_) */

int MPIR_Ibcast_MVP(void *buffer, int count, MPI_Datatype datatype, int root,
                    MPIR_Comm *comm_ptr, MPIR_Sched_t s)
{
    int mpi_errno = MPI_SUCCESS;

    if (comm_ptr->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
#if defined(CHANNEL_MRAIL) || defined(_MVP_CH4_OVERRIDE_)
        mpi_errno =
            MPIR_Ibcast_intra_MVP(buffer, count, datatype, root, comm_ptr, s);
#else
        mpi_errno =
            MPIR_Ibcast_intra(buffer, count, datatype, root, comm_ptr, s);
#endif /*#if defined(CHANNEL_MRAIL) || defined(_MVP_CH4_OVERRIDE_) */
    } else {
        mpi_errno = MPIR_Ibcast_inter_sched_auto(buffer, count, datatype, root,
                                                 comm_ptr, s);
    }

    return mpi_errno;
}
