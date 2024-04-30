#include "bcast_tuning.h"

int MPIR_Pipelined_Bcast_MVP(void *buffer, int count, MPI_Datatype datatype,
                             int root, MPIR_Comm *comm_ptr,
                             MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, bcast, pipelined);
    MPI_Comm shmem_comm;
    MPIR_Comm *shmem_commptr = NULL;
    int local_rank = 0;
    int mpi_errno = MPI_SUCCESS;
    MPI_Aint type_size = 0;
    intptr_t nbytes = 0, rem_count = 0, bcast_segment_count = 0,
             bcast_curr_count = 0;
    MPI_Aint extent;
    MPI_Aint true_extent, true_lb;
    void *tmp_buf = NULL;

    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_bcast_pipelined, 1);
    shmem_comm = comm_ptr->dev.ch.shmem_comm;
    MPIR_Comm_get_ptr(shmem_comm, shmem_commptr);
    MPIR_Datatype_get_extent_macro(datatype, extent);

    local_rank = shmem_commptr->rank;
    MPIR_Datatype_get_size_macro(datatype, type_size);
    nbytes = (intptr_t)(count)*extent;

    MPIR_Type_get_true_extent_impl(datatype, &true_lb, &true_extent);
    /* even though we always call this algorithm with contiguous buffer, still,
     * the datatype might have some holes in the beginning. Therefore, true_lb
     * might be non zero */
    tmp_buf = buffer + true_lb;

    rem_count = nbytes;
    bcast_segment_count = MIN(rem_count, MVP_BCAST_SEGMENT_SIZE);

    while (bcast_curr_count < nbytes) {
        comm_ptr->dev.ch.intra_node_done = 0;
        if (local_rank == 0) {
            mpi_errno = MPIR_Knomial_Bcast_inter_node_wrapper_MVP(
                (char *)tmp_buf + bcast_curr_count, bcast_segment_count,
                MPI_BYTE, root, comm_ptr, errflag);
        }
        if (comm_ptr->dev.ch.intra_node_done != 1) {
            if (!MVP_USE_OLD_BCAST) {
                mpi_errno = MVP_Bcast_intra_node_function(
                    (char *)tmp_buf + bcast_curr_count, bcast_segment_count,
                    MPI_BYTE, INTRA_NODE_ROOT, shmem_commptr, errflag);
            } else {
                if (bcast_segment_count * type_size <=
                    MVP_KNOMIAL_INTRA_NODE_THRESHOLD) {
                    mpi_errno = MPIR_Shmem_Bcast_MVP(
                        (char *)tmp_buf + bcast_curr_count, bcast_segment_count,
                        MPI_BYTE, INTRA_NODE_ROOT, shmem_commptr, errflag);
                } else {
                    mpi_errno = MPIR_Knomial_Bcast_intra_node_MVP(
                        (char *)tmp_buf + bcast_curr_count, bcast_segment_count,
                        MPI_BYTE, INTRA_NODE_ROOT, shmem_commptr, errflag);
                }
            }
        }
        MPIR_ERR_CHECK(mpi_errno);
        bcast_curr_count += bcast_segment_count;
        rem_count -= bcast_segment_count;
        bcast_segment_count = MIN(rem_count, bcast_segment_count);
    }

    comm_ptr->dev.ch.intra_node_done = 1;

fn_fail:
    MPIR_TIMER_END(coll, bcast, pipelined);
    return mpi_errno;
}
