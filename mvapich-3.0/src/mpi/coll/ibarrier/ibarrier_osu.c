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

#include "ibarrier_tuning.h"

extern unsigned long long PVAR_COUNTER_mvp_coll_ibarrier_sharp;
extern MPIR_T_pvar_timer_t PVAR_COUNTER_mvp_coll_timer_ibarrier_sharp;

#if defined(CHANNEL_MRAIL) || defined(_MVP_CH4_OVERRIDE_)

int (*MVP_Ibarrier_function)(MPIR_Comm *comm_ptr, MPIR_Sched_t s) = NULL;

int (*MVP_Ibarrier_intra_node_function)(MPIR_Comm *comm_ptr,
                                        MPIR_Sched_t s) = NULL;

#if defined(_SHARP_SUPPORT_)
int MPIR_Sharp_Ibarrier_MVP(MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag,
                            MPIR_Request **req)
{
    /* why do we need a request for barrier? */
    MPIR_TIMER_START(coll, ibarrier, sharp);
    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_ibarrier_sharp, 1);
    int mpi_errno = MPI_SUCCESS;
    mvp_sharp_req_h sharp_req = NULL;

    struct sharp_coll_comm *sharp_comm =
        ((mvp_sharp_info_t *)comm_ptr->dev.ch.sharp_coll_info)
            ->sharp_comm_module->sharp_coll_comm;

    /* Ensure that all messages in non-sharp channels are progressed first
     * to prevent deadlocks in subsequent blocking sharp API calls */

    mpi_errno = sharp_ops.coll_do_barrier_nb(sharp_comm, &sharp_req);

    if (mpi_errno != SHARP_COLL_SUCCESS) {
        MPIR_ERR_SETANDJUMP2(mpi_errno, MPI_ERR_INTERN, "**sharpcoll",
                             "**sharpcoll %s %d",
                             sharp_ops.coll_strerror(mpi_errno), mpi_errno);
    }
    /* now create and populate the request */
    *req = MPIR_Request_create(MPIR_REQUEST_KIND__COLL);
    if (*req == NULL) {
        MPIR_ERR_SETANDJUMP2(mpi_errno, MPI_ERR_INTERN, "**sharpcoll",
                             "**sharpcoll %s %d",
                             sharp_ops.coll_strerror(mpi_errno), mpi_errno);
    }
    MPID_MVP_SHARP_REQUEST(*req) = sharp_req;
    (*req)->kind = MPIR_REQUEST_KIND__COLL;
    mpi_errno = MPI_SUCCESS;

fn_exit:
    MPIR_TIMER_END(coll, ibarrier, sharp);
    return (mpi_errno);

fn_fail:
    goto fn_exit;
}
#endif /* end of defined (_SHARP_SUPPORT_) */

static int MPIR_Ibarrier_tune_helper_MVP(MPIR_Comm *comm_ptr, MPIR_Sched_t s)
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

    mpi_errno = MVP_Ibarrier_function(comm_ptr, s);
    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int MPIR_Ibarrier_intra_MVP(MPIR_Comm *comm_ptr, MPIR_Sched_t s)
{
    int mpi_errno = MPI_SUCCESS;
    int comm_size, is_homogeneous ATTRIBUTE((unused));

    int two_level_ibarrier = 1;
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
    ;

    // Search for some parameters regardless of whether subsequent selected
    // algorithm is 2-level or not

    // Search for the corresponding system size inside the tuning table
    while ((range < (mvp_size_ibarrier_tuning_table - 1)) &&
           (comm_size > mvp_ibarrier_thresholds_table[range].numproc)) {
        range++;
    }

    /*
    // Search for corresponding inter-leader function
    while ((range_threshold <
    (mvp_ibarrier_thresholds_table[range].size_inter_table - 1))
           && (nbytes >
               mvp_ibarrier_thresholds_table[range].inter_leader[range_threshold].max)
           &&
    (mvp_ibarrier_thresholds_table[range].inter_leader[range_threshold].max !=
    -1)) { range_threshold++;
    }

    // Search for corresponding intra-node function

    // Commenting this for the time being as none of
    // the algorithms are 2-level
    while ((range_threshold_intra <
            (mvp_ibarrier_thresholds_table[range].size_intra_table - 1))
           && (nbytes >
               mvp_ibarrier_thresholds_table[range].intra_node[range_threshold_intra].max)
           &&
    (mvp_ibarrier_thresholds_table[range].intra_node[range_threshold_intra].max
    != -1)) { range_threshold_intra++;
    }
    */

    MVP_Ibarrier_function = mvp_ibarrier_thresholds_table[range]
                                .inter_leader[range_threshold]
                                .MVP_pt_Ibarrier_function;

    MVP_Ibarrier_intra_node_function = mvp_ibarrier_thresholds_table[range]
                                           .intra_node[range_threshold_intra]
                                           .MVP_pt_Ibarrier_function;

    /* There are currently no two-level nb-barrier functions hence
       setting to 0 by default */
    two_level_ibarrier = mvp_ibarrier_thresholds_table[range]
                             .is_two_level_ibarrier[range_threshold];
    if (1 != two_level_ibarrier) {
        mpi_errno = MPIR_Ibarrier_tune_helper_MVP(comm_ptr, s);
    } else {
        /* Code path should not enter this with the current algorithms*/
    }

    return mpi_errno;
}
#endif /*#if defined(CHANNEL_MRAIL) || defined(_MVP_CH4_OVERRIDE_) */

int MPIR_Ibarrier_MVP(MPIR_Comm *comm_ptr, MPIR_Sched_t s)
{
    int mpi_errno = MPI_SUCCESS;

    if (comm_ptr->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
#if defined(CHANNEL_MRAIL) || defined(_MVP_CH4_OVERRIDE_)
        mpi_errno = MPIR_Ibarrier_intra_MVP(comm_ptr, s);
#else
        mpi_errno = MPIR_Ibarrier_intra_sched_auto(comm_ptr, s);
#endif /*#if defined(CHANNEL_MRAIL) || defined(_MVP_CH4_OVERRIDE_) */
    } else {
        /* this path is not supported */
        return MPI_ERR_INTERN;
    }

    return mpi_errno;
}
