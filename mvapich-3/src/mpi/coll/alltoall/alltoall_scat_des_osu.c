#include "alltoall_tuning.h"

int MPIR_Alltoall_Scatter_dest_MVP(const void *sendbuf, int sendcount,
                                   MPI_Datatype sendtype, void *recvbuf,
                                   int recvcount, MPI_Datatype recvtype,
                                   MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, alltoall, sd);
    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_alltoall_sd, 1);
    int comm_size, i, j;
    MPI_Aint sendtype_extent = 0, recvtype_extent = 0;
    int mpi_errno = MPI_SUCCESS;
    int dst, rank,size1,size2;
    double start,end;
    MPIR_Request **reqarray;
    MPI_Status *starray;
    MPI_Aint sendtype_size, nbytes;

    if (recvcount == 0) {
        MPIR_TIMER_END(coll, alltoall, sd);
        return MPI_SUCCESS;
    }

    MPIR_Datatype_get_size_macro(sendtype, sendtype_size);
    nbytes = sendtype_size * sendcount;

    comm_size = comm_ptr->local_size;
    rank = comm_ptr->rank;

    /* Get extent of send and recv types */
    MPIR_Datatype_get_extent_macro(recvtype, recvtype_extent);
    MPIR_Datatype_get_extent_macro(sendtype, sendtype_extent);

    /* Medium-size message. Use isend/irecv with scattered
     destinations. Use Tony Ladd's modification to post only
     a small number of isends/irecvs at a time. */
    /* FIXME: This converts the Alltoall to a set of blocking phases.
     Two alternatives should be considered:
     1) the choice of communication pattern could try to avoid
     contending routes in each phase
     2) rather than wait for all communication to finish (waitall),
     we could maintain constant queue size by using waitsome
     and posting new isend/irecv as others complete.  This avoids
     synchronization delays at the end of each block (when
     there are only a few isend/irecvs left)
     */
    int ii, ss, bblock;

    MPIR_CHKLMEM_DECL(2);

    /* If throttle_factor is n, each process posts n pairs of isend/irecv
     in each iteration. */
    if (MPIR_Process.local_size >= comm_size) {
        bblock = MVP_ALLTOALL_INTRA_THROTTLE_FACTOR;
    } else {
        if (nbytes < MVP_ALLTOALL_LARGE_MSG) {
            bblock = MVP_ALLTOALL_THROTTLE_FACTOR;
        } else {
            bblock = MVP_ALLTOALL_LARGE_MSG_THROTTLE_FACTOR;
        }
    }

    if (bblock >= comm_size)
        bblock = comm_size;

    /* FIXME: This should use the memory macros (there are storage
     leaks here if there is an error, for example) */
    MPIR_CHKLMEM_MALLOC(reqarray, MPIR_Request **,
                        2 * bblock * sizeof(MPIR_Request *), mpi_errno,
                        "reqarray", MPL_MEM_COLL);

    MPIR_CHKLMEM_MALLOC(starray, MPI_Status *, 2 * bblock * sizeof(MPI_Status),
                        mpi_errno, "starray", MPL_MEM_COLL);

    for (ii = 0; ii < comm_size; ii += bblock) {
        ss = comm_size - ii < bblock ? comm_size - ii : bblock;
        /* do the communication -- post ss sends and receives: */
        for (i = 0; i < ss; i++) {
            dst = (rank + i + ii) % comm_size;
            MPIR_PVAR_INC(alltoall, sd, recv, recvcount, recvtype);
            start=PMPI_Wtime();
            mpi_errno = MPIC_Irecv(
                (char *)recvbuf + dst * recvcount * recvtype_extent, recvcount,
                recvtype, dst, MPIR_ALLTOALL_TAG, comm_ptr, &reqarray[i]);
            end=PMPI_Wtime();
            MPIR_Datatype_get_size_macro(recvtype, size1);
            MPI_PVAR_DETAIL_INFO_INC(MVP,MPI_T_Alltoall,mvp_coll_alltoall,dst,rank
            ,1,start,end,sendrecv,recvcount*size1,0);
            MPIR_ERR_CHECK(mpi_errno);
        }
        for (i = 0; i < ss; i++) {
            dst = (rank - i - ii + comm_size) % comm_size;
            MPIR_PVAR_INC(alltoall, sd, send, sendcount, sendtype);
            start=PMPI_Wtime();
            mpi_errno =
                MPIC_Isend((char *)sendbuf + dst * sendcount * sendtype_extent,
                           sendcount, sendtype, dst, MPIR_ALLTOALL_TAG,
                           comm_ptr, &reqarray[i + ss], errflag);
            end=PMPI_Wtime();
            MPIR_Datatype_get_size_macro(sendtype, size1);
            MPI_PVAR_DETAIL_INFO_INC(MVP,MPI_T_Alltoall,mvp_coll_alltoall,rank,dst
            ,1,start,end,sendrecv,sendcount*size1,0);
            MPIR_ERR_CHECK(mpi_errno);
        }

        /* ... then wait for them to finish: */
        mpi_errno = MPIC_Waitall(2 * ss, reqarray, starray, errflag);
        MPIR_ERR_CHECK(mpi_errno);

        /* --BEGIN ERROR HANDLING-- */
        if (mpi_errno == MPI_ERR_IN_STATUS) {
            for (j = 0; j < 2 * ss; j++) {
                if (starray[j].MPI_ERROR != MPI_SUCCESS) {
                    mpi_errno = starray[j].MPI_ERROR;
                }
            }
        }
    }
    /* --END ERROR HANDLING-- */
    MPIR_CHKLMEM_FREEALL();

fn_fail:
    MPIR_TIMER_END(coll, alltoall, sd);
    return (mpi_errno);
}
