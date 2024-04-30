#include "bcast_tuning.h"

#if defined(_SHARP_SUPPORT_)
#include "api/sharp_coll.h"
#include "mvp_sharp.h"

extern int mvp_sharp_tuned_msg_size;
extern int MPIR_Sharp_Allreduce_MVP(const void *sendbuf, void *recvbuf,
                                    int count, MPI_Datatype datatype, MPI_Op op,
                                    MPIR_Comm *comm_ptr,
                                    MPIR_Errflag_t *errflag);
#endif

#if defined(_SHARP_SUPPORT_)
/* Currently implemented on top of allreduce. Ideally should use lower level
 * Sharp calls to achieve the same once avaliable*/
int MPIR_Sharp_Bcast_MVP(void *buffer, int count, MPI_Datatype datatype,
                         int root, MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, bcast, sharp);
    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_bcast_sharp, 1);
    int mpi_errno = MPI_SUCCESS;
    void *sendbuf = NULL, *recvbuf = NULL;
    MPI_Aint type_size = 0;
    /* Get size of data */
    MPIR_Datatype_get_size_macro(datatype, type_size);
    intptr_t nbytes = (intptr_t)(count) * (type_size);
    int rank = comm_ptr->rank;

    if (rank == root) {
        sendbuf = (void *)buffer;
        recvbuf = (void *)comm_ptr->dev.ch.coll_tmp_buf;
    } else {
        memset(comm_ptr->dev.ch.coll_tmp_buf, 0, nbytes);
        sendbuf = (void *)comm_ptr->dev.ch.coll_tmp_buf;
        recvbuf = (void *)buffer;
    }

    mpi_errno = MPIR_Sharp_Allreduce_MVP(sendbuf, recvbuf, count, datatype,
                                         MPI_SUM, comm_ptr, errflag);
    MPIR_ERR_CHECK(mpi_errno);

fn_exit:
    MPIR_TIMER_END(coll, bcast, sharp);
    return (mpi_errno);
fn_fail:
    goto fn_exit;
}
#endif
