#include "mpiimpl.h"
#include "allreduce_tuning.h"

#if defined(_SHARP_SUPPORT_)
#include "api/sharp_coll.h"
#include "mvp_sharp.h"
#endif

extern MPIR_T_pvar_timer_t PVAR_TIMER_mvp_coll_timer_allreduce_sharp;

extern unsigned long long PVAR_COUNTER_mvp_coll_allreduce_sharp;

#if defined(_SHARP_SUPPORT_)
int MPIR_Sharp_Allreduce_MVP(const void *sendbuf, void *recvbuf, int count,
                             MPI_Datatype datatype, MPI_Op op,
                             MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    MPIR_TIMER_START(coll, allreduce, sharp);
    MPIR_T_PVAR_COUNTER_INC(MVP, mvp_coll_allreduce_sharp, 1);

    int mpi_errno = MPI_SUCCESS;
    int is_contig = 0;
    int is_inplace = 0;
    struct sharp_coll_reduce_spec reduce_spec;
    mvp_sharp_reduce_datatype_size_t *dt_size = NULL;
    struct sharp_coll_comm *sharp_comm = NULL;
    MPIR_Datatype *dtp = NULL;

    if (HANDLE_GET_KIND(datatype) == HANDLE_KIND_BUILTIN) {
        is_contig = 1;
    } else {
        MPIR_Datatype_get_ptr(datatype, dtp);
        is_contig = dtp->is_contig;
    }

    mvp_get_sharp_datatype(datatype, &dt_size);
    reduce_spec.dtype = dt_size->sharp_data_type;
    if (reduce_spec.dtype == SHARP_DTYPE_NULL) {
        mpi_errno = SHARP_COLL_ENOT_SUPP;
        goto fn_fail;
    }

    reduce_spec.op = mvp_get_sharp_reduce_op(op);
    if (reduce_spec.op == SHARP_OP_NULL) {
        mpi_errno = SHARP_COLL_ENOT_SUPP;
        goto fn_fail;
    }

    if (is_contig == 1) {
        reduce_spec.sbuf_desc.buffer.length = count * dt_size->size;
        if (sendbuf != MPI_IN_PLACE) {
            reduce_spec.sbuf_desc.buffer.ptr = (void *)sendbuf;
        } else {
            is_inplace = 1;
            reduce_spec.sbuf_desc.buffer.ptr =
                MPL_malloc(reduce_spec.sbuf_desc.buffer.length, MPL_MEM_BUFFER);
            MPIR_Memcpy(reduce_spec.sbuf_desc.buffer.ptr, recvbuf,
                        reduce_spec.sbuf_desc.buffer.length);
        }
        reduce_spec.sbuf_desc.type = SHARP_DATA_BUFFER;
        reduce_spec.sbuf_desc.mem_type = SHARP_MEM_TYPE_HOST;
        reduce_spec.sbuf_desc.buffer.mem_handle = NULL;
        reduce_spec.rbuf_desc.buffer.ptr = recvbuf;
        reduce_spec.rbuf_desc.buffer.length = count * dt_size->size;
        reduce_spec.rbuf_desc.type = SHARP_DATA_BUFFER;
        reduce_spec.rbuf_desc.mem_type = SHARP_MEM_TYPE_HOST;
        reduce_spec.aggr_mode = SHARP_AGGREGATION_NONE;
        reduce_spec.rbuf_desc.buffer.mem_handle = NULL;
    } else {
        /* NOT implemented in Sharp */
        mpi_errno = SHARP_COLL_ENOT_SUPP;
        goto fn_fail;
    }

    reduce_spec.length = count;
    sharp_comm = ((mvp_sharp_info_t *)comm_ptr->dev.ch.sharp_coll_info)
                     ->sharp_comm_module->sharp_coll_comm;

    mpi_errno = sharp_ops.coll_do_allreduce(sharp_comm, &reduce_spec);
    if (mpi_errno != SHARP_COLL_SUCCESS) {
        goto fn_fail;
    }

    mpi_errno = MPI_SUCCESS;

fn_exit:
    MPL_free(dt_size);
    if (is_inplace)
        MPL_free(reduce_spec.sbuf_desc.buffer.ptr);

    MPIR_TIMER_END(coll, allreduce, sharp);
    return (mpi_errno);

fn_fail:
    PRINT_DEBUG(DEBUG_Sharp_verbose, "Continue without SHArP: %s \n",
                sharp_ops.coll_strerror(mpi_errno));
    mpi_errno = MPI_ERR_INTERN;
    goto fn_exit;
}
#endif /* end of defined (_SHARP_SUPPORT_) */
