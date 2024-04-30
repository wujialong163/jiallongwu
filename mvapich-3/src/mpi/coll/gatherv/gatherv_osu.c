/* Copyright (c) 2001-2024, The Ohio State University. All rights
 * reserved.
 *
 * This file is part of the MVAPICH software package developed by the
 * team members of The Ohio State University's Network-Based Computing
 * Laboratory (NBCL), headed by Professor Dhabaleswar K. (DK) Panda.
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level MVAPICH directory.
 */

#include "mpiimpl.h"
#include "datatype.h"
#include "mvp_coll_shmem.h"

extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_gatherv_algo;

extern unsigned long long PVAR_COUNTER_mvp_coll_gatherv_algo;

extern unsigned long long PVAR_COUNTER_mvp_coll_gatherv_default_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_gatherv_default_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_gatherv_default_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_gatherv_default_count_recv;

extern unsigned long long PVAR_COUNTER_mvp_coll_gatherv_bytes_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_gatherv_bytes_recv;
extern unsigned long long PVAR_COUNTER_mvp_coll_gatherv_count_send;
extern unsigned long long PVAR_COUNTER_mvp_coll_gatherv_count_recv;

/*Ditto MPICH code except for a few changes*/
int MPIR_Gatherv_MVP(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                     void *recvbuf, const int *recvcounts, const int *displs,
                     MPI_Datatype recvtype, int root, MPIR_Comm *comm_ptr,
                     MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, gatherv, algo);
    int comm_size, rank;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    MPI_Aint extent;
    int i, reqs;
    MPIR_Request **reqarray;
    MPI_Status *starray;
    MPIR_CHKLMEM_DECL(2);

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_gatherv_algo, 1);
    rank = comm_ptr->rank;

    /* If rank == root, then I recv lots, otherwise I send */
    if (((comm_ptr->comm_kind == MPIR_COMM_KIND__INTRACOMM) &&
         (root == rank)) ||
        ((comm_ptr->comm_kind == MPIR_COMM_KIND__INTERCOMM) &&
         (root == MPI_ROOT))) {
        if (comm_ptr->comm_kind == MPIR_COMM_KIND__INTRACOMM)
            comm_size = comm_ptr->local_size;
        else
            comm_size = comm_ptr->remote_size;

        MPIR_Datatype_get_extent_macro(recvtype, extent);

        MPIR_CHKLMEM_MALLOC(reqarray, MPIR_Request **,
                            comm_size * sizeof(MPIR_Request *), mpi_errno,
                            "reqarray", MPL_MEM_COLL);
        MPIR_CHKLMEM_MALLOC(starray, MPI_Status *,
                            comm_size * sizeof(MPI_Status), mpi_errno,
                            "starray", MPL_MEM_COLL);

        reqs = 0;
        for (i = 0; i < comm_size; i++) {
            if (recvcounts[i]) {
                if ((comm_ptr->comm_kind == MPIR_COMM_KIND__INTRACOMM) &&
                    (i == rank)) {
                    if (sendbuf != MPI_IN_PLACE) {
                        mpi_errno = MPIR_Localcopy(
                            sendbuf, sendcount, sendtype,
                            ((char *)recvbuf + displs[rank] * extent),
                            recvcounts[rank], recvtype);
                        MPIR_ERR_CHECK(mpi_errno);
                    }
                } else {
                    MPIR_PVAR_INC(gatherv, default, recv, recvcounts[i],
                                  recvtype);
                    mpi_errno =
                        MPIC_Irecv(((char *)recvbuf + displs[i] * extent),
                                   recvcounts[i], recvtype, i, MPIR_GATHERV_TAG,
                                   comm_ptr, &reqarray[reqs++]);
                    MPIR_ERR_CHECK(mpi_errno);
                }
            }
        }
        /* ... then wait for *all* of them to finish: */
        mpi_errno = MPIC_Waitall(reqs, reqarray, starray, errflag);
        if (mpi_errno && mpi_errno != MPI_ERR_IN_STATUS) {
            MPIR_ERR_POP(mpi_errno);
        }

        /* --BEGIN ERROR HANDLING-- */
        if (mpi_errno == MPI_ERR_IN_STATUS) {
            for (i = 0; i < reqs; i++) {
                if (starray[i].MPI_ERROR != MPI_SUCCESS) {
                    mpi_errno = starray[i].MPI_ERROR;
                    if (mpi_errno) {
                        /* for communication errors,
                         * just record the error but continue */
                        *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                        MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                    }
                }
            }
        }
        /* --END ERROR HANDLING-- */
    }

    else if (root != MPI_PROC_NULL) {
        /* non-root nodes, and in the intercomm.
         * case, non-root nodes on remote side */
        if (sendcount) {
            /* we want local size in both the intracomm and intercomm cases
               because the size of the root's group (group A in the standard) is
               irrelevant here. */
            comm_size = comm_ptr->local_size;

            if (comm_size >= MVP_GATHERV_SSEND_THRESHOLD) {
                MPIR_PVAR_INC(gatherv, default, send, sendcount, sendtype);
                mpi_errno = MPIC_Ssend(sendbuf, sendcount, sendtype, root,
                                       MPIR_GATHERV_TAG, comm_ptr, errflag);
                if (mpi_errno) {
                    /* for communication errors,
                     * just record the error but continue */
                    *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                    MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                }
            } else {
                MPIR_PVAR_INC(gatherv, default, send, sendcount, sendtype);
                mpi_errno = MPIC_Send(sendbuf, sendcount, sendtype, root,
                                      MPIR_GATHERV_TAG, comm_ptr, errflag);
                if (mpi_errno) {
                    /* for communication errors,
                     * just record the error but continue */
                    *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                    MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                }
            }
        }
    }

fn_exit:
    MPIR_CHKLMEM_FREEALL();
    if (mpi_errno_ret)
        mpi_errno = mpi_errno_ret;
    else if (*errflag)
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**coll_fail");

    MPIR_TIMER_END(coll, gatherv, algo);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
