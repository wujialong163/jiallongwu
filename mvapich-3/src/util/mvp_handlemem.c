/* Copyright (c) 2001-2021, The Ohio State University. All rights
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
#include <stdio.h>

int MPIR_Handle_direct_free(MPIR_Object_alloc_t *objmem)
{
    int i = 0;
    void *direct = objmem->direct;
    int obj_size = objmem->size;
    int direct_size = objmem->direct_size;
    MPIR_Handle_common *hptr = 0;
    char *ptr = (char *)direct;
    int mpi_errno = MPI_SUCCESS;

    for (i = 0; i < direct_size; i++) {
        hptr = (MPIR_Handle_common *)(void *)ptr;
        if (objmem == &MPIR_Datatype_mem) {
            MPIR_Datatype *datatype_ptr = (MPIR_Datatype *)hptr;
            if (datatype_ptr->ref_count > 0) {
                int inuse = 0;
                do {
                    MPIR_Object_release_ref(datatype_ptr, &inuse);
                } while (inuse);
                if (MPIR_Process.attr_free && datatype_ptr->attributes) {
                    mpi_errno = MPIR_Process.attr_free(
                        datatype_ptr->handle, &datatype_ptr->attributes);
                }
                if (mpi_errno == MPI_SUCCESS) {
                    MPIR_Datatype_free(datatype_ptr);
                }
            }
        }
        ptr = ptr + obj_size;
    }

    return 0;
}

#if 0
/* this appears to be obsolete and is not called anywhere. Removing it in a
 * reversible manner */
/* TODO-merge: remove this entirely */
int MPIR_Handle_indirect_free(MPIR_Object_alloc_t *objmem)
{
    void *(**indirect)[]        = &objmem->indirect;
    int *indirect_size          = &objmem->indirect_size;
    int indirect_num_indices    = HANDLE_NUM_INDICES;
    int obj_size                = objmem->size;
    void               *block_ptr;
    MPIR_Handle_common *hptr=0;
    char               *ptr;
    int                i = 0, j = 0;
    int mpi_errno   = MPI_SUCCESS;

    for (i = 0; i < *indirect_size; ++i) {
        block_ptr = (**indirect)[i];
        ptr = (char *)block_ptr;
        for (j = 0; j < indirect_num_indices; j++) {
            /* Cast to (void*) to avoid false warning about alignment */
            hptr       = (MPIR_Handle_common *)(void*)ptr;
            if (objmem == &MPIR_Datatype_mem) {
                MPIR_Datatype *datatype_ptr = (MPIR_Datatype*) hptr;
                if (datatype_ptr->ref_count > 0) {
                    int inuse = 0;
                    do {
                        MPIR_Object_release_ref(datatype_ptr,&inuse);
                    } while (inuse);
                    if (MPIR_Process.attr_free && datatype_ptr->attributes) {
                        mpi_errno = MPIR_Process.attr_free(datatype_ptr->handle,
                                &datatype_ptr->attributes );
                    }
                    if (mpi_errno == MPI_SUCCESS) {
                        MPIR_Datatype_free(datatype_ptr, 1);
                    }
                }
            }
            ptr = ptr + obj_size;
        }
    }

    return mpi_errno;
}
#endif

/* indirect is really a pointer to a pointer to an array of pointers */

/*
  Create and return a pointer to an info object.  Returns null if there is
  an error such as out-of-memory.  Does not allocate space for the
  key or value.

 */

static void MPIR_Handle_reset(MPIR_Object_alloc_t *objmem)
{
    objmem->avail = NULL;
    objmem->initialized = 0;
    objmem->indirect = NULL;
    objmem->indirect_size = 0;
    memset(objmem->direct, 0, (objmem->size * objmem->direct_size));
}

extern int mvp_datatype_names_initialized;
extern int mvp_datatype_builtin_fillin_is_init;
