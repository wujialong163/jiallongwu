#include "allreduce_tuning.h"
#include "bcast_tuning.h"

extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allreduce_mcast;

extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_mcast;

#if defined(_MCST_SUPPORT_)
int MPIR_Allreduce_mcst_MVP(const void *sendbuf, void *recvbuf, int count,
                            MPI_Datatype datatype, MPI_Op op,
                            MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, allreduce, mcast);
    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_allreduce_mcast, 1);
    MPI_Aint true_lb, true_extent;
    /*We use reduce (at rank =0) followed by mcst-bcast to implement the
     * allreduce operation */
    int root = 0;
    MPI_Aint nbytes = 0;
    MPI_Aint type_size = 0, position = 0;
    int mpi_errno = MPI_SUCCESS;
    int mpi_errno_ret = MPI_SUCCESS;
    int rank = comm_ptr->rank, is_contig = 0, is_commutative = 0;
    MPIR_CHKLMEM_DECL(1);
    MPIR_Datatype *dtp = NULL;
    void *tmp_buf = NULL;
    MPIR_Op *op_ptr = NULL;
    MPIR_Datatype_get_size_macro(datatype, type_size);
    nbytes = type_size * count;

    if (HANDLE_GET_KIND(datatype) == HANDLE_KIND_BUILTIN) {
        is_contig = 1;
    } else {
        MPIR_Datatype_get_ptr(datatype, dtp);
        is_contig = dtp->is_contig;
    }

    if (HANDLE_GET_KIND(op) == HANDLE_KIND_BUILTIN) {
        is_commutative = 1;
    } else {
        MPIR_Op_get_ptr(op, op_ptr);
        if (op_ptr->kind == MPIR_OP_KIND__USER_NONCOMMUTE) {
            is_commutative = 0;
        } else {
            is_commutative = 1;
        }
    }

    MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);

    if (is_commutative == 0) {
        reduce_fn = &MPIR_Reduce_binomial_MVP;
    } else {
        if (MVP_Allreduce_function ==
            &MPIR_Allreduce_mcst_reduce_two_level_helper_MVP) {
            reduce_fn = &MPIR_Reduce_MVP;
        } else {
            reduce_fn = &MPIR_Reduce_redscat_gather_MVP;
        }
    }

    /* First do a reduction at rank = 0 */
    if (rank == root) {
        mpi_errno = reduce_fn(sendbuf, recvbuf, count, datatype, op, root,
                              comm_ptr, errflag);
    } else {
        if (sendbuf != MPI_IN_PLACE) {
            mpi_errno = reduce_fn(sendbuf, recvbuf, count, datatype, op, root,
                                  comm_ptr, errflag);
        } else {
            mpi_errno = reduce_fn(recvbuf, NULL, count, datatype, op, root,
                                  comm_ptr, errflag);
        }
    }
    if (mpi_errno) {
        /* for communication errors, just record the error but continue */
        *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
        MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
    }

    /* Now do a mcst-bcast operation with rank0 as the root */
    if (!is_contig) {
        /* Mcast cannot handle non-regular datatypes. We need to pack
         * as bytes before sending it*/
        MPIR_CHKLMEM_MALLOC(tmp_buf, void *, nbytes, mpi_errno, "tmp_buf",
                            MPL_MEM_COLL);

        position = 0;
        if (rank == root) {
            mpi_errno = MPIR_Typerep_pack(recvbuf, count, datatype, position,
                                          tmp_buf, nbytes, &position);
            MPIR_ERR_CHECK(mpi_errno);
        }
        mpi_errno = MPIR_Mcast_inter_node_MVP(tmp_buf, nbytes, MPI_BYTE, root,
                                              comm_ptr, errflag);
    } else {
        mpi_errno = MPIR_Mcast_inter_node_MVP(recvbuf, count, datatype, root,
                                              comm_ptr, errflag);
    }

    if (mpi_errno) {
        /* for communication errors, just record the error but continue */
        *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
        MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
        MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
    }

    if (!is_contig) {
        /* We are done, lets pack the data back the way the user
         * needs it */
        if (rank != root) {
            position = 0;
            mpi_errno = MPIR_Typerep_unpack(tmp_buf, nbytes, recvbuf, count,
                                            datatype, position, &position);
            MPIR_ERR_CHECK(mpi_errno);
        }
    }

    /* check to see if the intra-node mcast is not done.
     * if this is the case, do it either through shmem or knomial */
    if (comm_ptr->dev.ch.intra_node_done == 0) {
        MPIR_Comm *shmem_commptr = NULL;
        MPIR_Comm_get_ptr(comm_ptr->dev.ch.shmem_comm, shmem_commptr);
        int local_size = shmem_commptr->local_size;
        if (local_size > 1) {
            MPIR_Bcast_MVP(recvbuf, count, datatype, 0, shmem_commptr, errflag);
            if (mpi_errno) {
                /* for communication errors,
                 * just record the error but continue */
                *errflag = MPIR_ERR_GET_CLASS(mpi_errno);
                MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }
        }
    }

fn_exit:
    MPIR_CHKLMEM_FREEALL();
    MPIR_TIMER_END(coll, allreduce, mcast);
    return (mpi_errno);

fn_fail:
    goto fn_exit;
}
#endif /*  #if defined(_MCST_SUPPORT_) */
