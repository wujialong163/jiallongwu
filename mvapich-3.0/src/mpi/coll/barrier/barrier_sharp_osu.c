#include "barrier_tuning.h"

#if defined(_SHARP_SUPPORT_)
#include "api/sharp_coll.h"
#include "mvp_sharp.h"
#endif

#if defined(_SHARP_SUPPORT_)
int MPIR_Sharp_Barrier_MVP(MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, barrier, sharp);
    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_barrier_sharp, 1);
    int mpi_errno = MPI_SUCCESS;

    struct sharp_coll_comm *sharp_comm =
        ((mvp_sharp_info_t *)comm_ptr->dev.ch.sharp_coll_info)
            ->sharp_comm_module->sharp_coll_comm;

    mpi_errno = sharp_ops.coll_do_barrier(sharp_comm);

    if (mpi_errno != SHARP_COLL_SUCCESS) {
        goto fn_fail;
    }

    mpi_errno = MPI_SUCCESS;

fn_exit:
    MPIR_TIMER_END(coll, barrier, sharp);
    return (mpi_errno);

fn_fail:
    PRINT_DEBUG(DEBUG_Sharp_verbose, "Continue without SHArP: %s \n",
                sharp_ops.coll_strerror(mpi_errno));
    mpi_errno = MPI_ERR_INTERN;
    goto fn_exit;
}
#endif /* end of defined (_SHARP_SUPPORT_) */
