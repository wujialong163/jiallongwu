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
#ifndef _MVP_LOAD_IMBALANCE_
#define _MVP_LOAD_IMBALANCE_

typedef struct MVP_load_imbalance_time {
    /* Accumulated time */
    MPL_time_t total;

    /* Time when the timer was started recently */
    MPL_time_t curstart;

    /* A counter recording how many times the timer is started */
    unsigned long count;
} MVP_load_imbalance_time;

extern MVP_load_imbalance_time mvp_allreduce_load_imbalance;

#define MPII_MVP_LOAD_IMBALANCE_TIMER_INIT(_ptr)                               \
    do {                                                                       \
        MPIR_Memset(&((_ptr)->total), 0, sizeof(MPL_time_t));                  \
        (_ptr)->count = 0;                                                     \
    } while (0)

#define MPII_MVP_LOAD_IMBALANCE_TIMER_GET(_ptr, buf)                           \
    do {                                                                       \
        *buf = ((double)((_ptr)->total.tv_sec) +                               \
                1.0e-9 * (double)((_ptr)->total.tv_nsec));                     \
    } while (0)

#define MPII_MVP_LOAD_IMBALANCE_COUNTER_GET(_ptr, buf)                         \
    do {                                                                       \
        *buf = (_ptr)->count;                                                  \
    } while (0)

#define MPII_MVP_LOAD_IMBALANCE_START(_ptr)                                    \
    do {                                                                       \
        MPL_wtime(&((_ptr)->curstart));                                        \
        (_ptr)->count++;                                                       \
    } while (0)

#define MPII_MVP_LOAD_IMBALANCE_STOP(_ptr)                                     \
    do {                                                                       \
        MPL_time_t _tmp;                                                       \
        MPL_wtime(&_tmp);                                                      \
        MPL_wtime_acc(&((_ptr)->curstart), &_tmp, &((_ptr)->total));           \
    } while (0)

#define MPII_MVP_LOAD_IMBALANCE_CHECK(_load_imba_timer, _mpi_err, _comm_ptr,   \
                                      _errflag_ptr)                            \
    do {                                                                       \
        MPII_MVP_LOAD_IMBALANCE_START(&_load_imba_timer);                      \
        _mpi_err = MPIR_Barrier(_comm_ptr, _errflag_ptr);                      \
        MPIR_ERR_CHECK(_mpi_err);                                              \
        MPII_MVP_LOAD_IMBALANCE_STOP(&_load_imba_timer);                       \
    } while (0)

static inline int MVP_load_imbalance_report(MVP_load_imbalance_time *imba_timer,
                                            MPIR_Comm *comm_ptr,
                                            const char *coll_op)
{
    MPIR_Errflag_t errflag = MPIR_ERR_NONE;
    int mpi_errno = MPI_SUCCESS;
    double ind_total_time, agg_total_time, agg_min_time, agg_max_time;
    unsigned long ind_total_count, agg_total_count;

    MPII_MVP_LOAD_IMBALANCE_TIMER_GET(imba_timer, &ind_total_time);
    MPII_MVP_LOAD_IMBALANCE_COUNTER_GET(imba_timer, &ind_total_count);
    mpi_errno = MPIR_Reduce(&ind_total_time, &agg_total_time, 1, MPI_DOUBLE,
                            MPI_SUM, 0, comm_ptr, &errflag);
    MPIR_ERR_CHECK(mpi_errno);
    mpi_errno = MPIR_Reduce(&ind_total_count, &agg_total_count, 1,
                            MPI_UNSIGNED_LONG, MPI_SUM, 0, comm_ptr, &errflag);
    MPIR_ERR_CHECK(mpi_errno);
    mpi_errno = MPIR_Reduce(&ind_total_time, &agg_min_time, 1, MPI_DOUBLE,
                            MPI_MIN, 0, comm_ptr, &errflag);
    MPIR_ERR_CHECK(mpi_errno);
    mpi_errno = MPIR_Reduce(&ind_total_time, &agg_max_time, 1, MPI_DOUBLE,
                            MPI_MAX, 0, comm_ptr, &errflag);
    MPIR_ERR_CHECK(mpi_errno);

    if (comm_ptr->rank == 0) {
        printf("%s load imbalance:\n", coll_op);
        printf("  Average load imbalance per call: %f (s)\n"
               "  Total time %f(s)\tTotal count of calls: %lu\n"
               "  Min sum per rank: %f(s)\tMax sum per rank: %f(s)\n",
               (agg_total_time / (double)agg_total_count), agg_total_time,
               agg_total_count, agg_min_time, agg_max_time);
    }

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#endif /* #ifndef _MVP_LOAD_IMBALANCE_ */
