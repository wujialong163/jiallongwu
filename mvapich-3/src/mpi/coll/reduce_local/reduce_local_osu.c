#include "mpiimpl.h"

int MPIR_get_op_ptr(MPI_User_function **uop, int *is_commutative,
#ifdef HAVE_CXX_BINDING
                    int *is_cxx_uop,
#endif
                    MPI_Op op)
{
    MPIR_Op *op_ptr;
    int mpi_errno = MPI_SUCCESS;
    if (HANDLE_GET_KIND(op) == HANDLE_KIND_BUILTIN) {
        *is_commutative = 1;
        /* get the function by indexing into the op table */
        *uop = MPIR_OP_HDL_TO_FN(op);
    } else {
        MPIR_Op_get_ptr(op, op_ptr);
        if (op_ptr->kind == MPIR_OP_KIND__USER_NONCOMMUTE) {
            *is_commutative = 0;
        } else {
            *is_commutative = 1;
        }
#ifdef HAVE_CXX_BINDING
        if (op_ptr->language == MPIR_LANG__CXX) {
            *uop = (MPI_User_function *)op_ptr->function.c_function;
            *is_cxx_uop = 1;
        } else {
#endif
            if (op_ptr->language == MPIR_LANG__C) {
                *uop = (MPI_User_function *)op_ptr->function.c_function;
            } else {
                *uop = (MPI_User_function *)op_ptr->function.f77_function;
            }
#ifdef HAVE_CXX_BINDING
        }
#endif
    }
    return mpi_errno;
}

int MPIR_MVP_Reduce_local(void *inbuf, void *inoutbuf, MPI_Aint count,
                          MPI_Datatype datatype, MPI_User_function *uop
#ifdef HAVE_CXX_BINDING
                          ,
                          int is_cxx_uop
#endif
)
{
    int mpi_errno = MPI_SUCCESS;
#ifdef HAVE_CXX_BINDING
    if (is_cxx_uop) {
        (*MPIR_Process.cxx_call_op_fn)(inbuf, inoutbuf, count, datatype, uop);
    } else {
#endif
        (*uop)(inbuf, inoutbuf, (int *)(&count), &datatype);
#ifdef HAVE_CXX_BINDING
    }
#endif
    return mpi_errno;
}
