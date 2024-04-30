#ifndef BUILD_MVP_NODEMAP_H_INCLUDED
#define BUILD_MVP_NODEMAP_H_INCLUDED

#include "mpl.h"
#include "mvp_debug_utils.h"

#ifdef USE_PMI1_API
static int MPIR_NODEMAP_populate_ids_from_mapping(char *, int, int *, int *,
                                                  int *);

static inline int MPIR_NODEMAP_MVP_Get_mpirun_mapping(char **mapping)
{
    int mpi_errno = MPI_SUCCESS;
    *mapping = getenv("MPIRUN_PMI_PROCESS_MAPPING");
    return mpi_errno;
}

/* should replace the MPIR_NODEMAP_build_nodemap_fallback from MPICH */
static inline int MPIR_NODEMAP_MVP_build_nodemap(int sz, int *out_nodemap,
                                                 int *out_max_node_id,
                                                 int myrank)
{
    int mpi_errno = MPI_ERR_UNKNOWN;
    char *value, *mapping;
    int did_map;

    int i;
    /* do full nodemap */
    if (((value = getenv("MPIRUN_RSH_LAUNCH")) != NULL && (atoi(value) != 0)) &&
        (((value = getenv("MVP_USE_MPIRUN_MAPPING")) == NULL ||
          (atoi(value) != 0)))) {
        mpi_errno = MPIR_NODEMAP_MVP_Get_mpirun_mapping(&mapping);
        MPIR_ERR_CHECK(mpi_errno);
        /* PRINT_DEBUG(DEBUG_INIT_verbose, "Got mapping: %s\n", mapping); */
        if (!mpi_errno) {
            mpi_errno = MPIR_NODEMAP_populate_ids_from_mapping(
                mapping, sz, out_nodemap, out_max_node_id, &did_map);
            MPIR_ERR_CHECK(mpi_errno);
            if (!did_map) {
                PRINT_ERROR("Failed to get mapping from mpirun_rsh");
                mpi_errno = MPI_ERR_INTERN;
                MPIR_ERR_POP(mpi_errno);
            }
        }
        mpi_errno = MPI_SUCCESS;
    }

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
#endif /* USE_PMI1_API */

#endif /* BUILD_NODEMAP_H_INCLUDED */
