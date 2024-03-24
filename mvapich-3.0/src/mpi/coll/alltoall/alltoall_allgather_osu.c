#include "alltoall_tuning.h"

int MPIR_Alltoall_ALG_MVP(const void *sendbuf, int sendcount,
                          MPI_Datatype sendtype, void *recvbuf, int recvcount,
                          MPI_Datatype recvtype, MPIR_Comm *comm_ptr,
                          MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, alltoall, rd);
    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_alltoall_rd, 1);
    int comm_size;
    MPI_Aint sendtype_extent, recvtype_extent;
    int mpi_errno = MPI_SUCCESS;
    int rank;
    void *tmp_buf;
    MPIR_CHKLMEM_DECL(1);

    MPI_Aint sendtype_true_extent, sendbuf_extent, sendtype_true_lb;
    int p;

    if (recvcount == 0) {
        MPIR_TIMER_END(coll, alltoall, rd);
        return MPI_SUCCESS;
    }

    comm_size = comm_ptr->local_size;
    rank = comm_ptr->rank;

    /* Get extent of send and recv types */
    MPIR_Datatype_get_extent_macro(recvtype, recvtype_extent);
    MPIR_Datatype_get_extent_macro(sendtype, sendtype_extent);

    /* Short message. Use recursive doubling. Each process sends all
     its data at each step along with all data it received in
     previous steps. */

    /* need to allocate temporary buffer of size
     sendbuf_extent*comm_size */

    MPIR_Type_get_true_extent_impl(sendtype, &sendtype_true_lb,
                                   &sendtype_true_extent);

    sendbuf_extent = sendcount * comm_size *
                     (MPL_MAX(sendtype_true_extent, sendtype_extent));

    if (MVP_ALLTOALL_RD_MAX_MSG_SIZE < sendbuf_extent) {
        /* avoid using RD Allgather based Alltoall for large messages */
        return MPIR_Alltoall_pairwise_MVP(sendbuf, sendcount, sendtype, recvbuf,
                                          recvcount, recvtype, comm_ptr,
                                          errflag);
    }

    MPIR_CHKLMEM_MALLOC(tmp_buf, void *, sendbuf_extent *comm_size, mpi_errno,
                        "tmp_buf", MPL_MEM_COLL);

    /* adjust for potential negative lower bound in datatype */
    tmp_buf = (void *)((char *)tmp_buf - sendtype_true_lb);

    /* copy local sendbuf into tmp_buf at location indexed by rank */

    mpi_errno = MPIR_Allgather_MVP(
        sendbuf, sendcount * comm_size * sendtype_extent, MPI_BYTE, tmp_buf,
        recvcount * comm_size * sendtype_extent, MPI_BYTE, comm_ptr, errflag);

    /* now copy everyone's contribution from tmp_buf to recvbuf */
    for (p = 0; p < comm_size; p++) {
        mpi_errno =
            MPIR_Localcopy(((char *)tmp_buf + p * sendbuf_extent +
                            rank * sendcount * sendtype_extent),
                           sendcount, sendtype,
                           ((char *)recvbuf + p * recvcount * recvtype_extent),
                           recvcount, recvtype);
        MPIR_ERR_CHECK(mpi_errno);
    }

fn_fail:
    MPIR_CHKLMEM_FREEALL();
    MPIR_TIMER_END(coll, alltoall, rd);
    return (mpi_errno);
}
