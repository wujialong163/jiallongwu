##
## Copyright (C) by Argonne National Laboratory
##     See COPYRIGHT in top-level directory
##

# mpi_sources includes only the routines that are MPI function entry points
# The code for the MPI operations (e.g., MPI_SUM) is not included in 
# mpi_sources
mpi_sources += \
    src/mpi/coll/bcast/bcast.c

mpi_core_sources += \
    src/mpi/coll/bcast/bcast_utils.c \
    src/mpi/coll/bcast/bcast_allcomm_nb.c \
    src/mpi/coll/bcast/bcast_intra_binomial.c \
    src/mpi/coll/bcast/bcast_intra_scatter_recursive_doubling_allgather.c \
    src/mpi/coll/bcast/bcast_intra_scatter_ring_allgather.c \
    src/mpi/coll/bcast/bcast_intra_smp.c \
    src/mpi/coll/bcast/bcast_inter_remote_send_local_bcast.c 


noinst_HEADERS += \
    src/mpi/coll/bcast/bcast.h

if BUILD_OSU_MVAPICH
mpi_core_sources += \
    src/mpi/coll/bcast/bcast_sharp_osu.c \
    src/mpi/coll/bcast/bcast_topo_aware_osu.c \
    src/mpi/coll/bcast/bcast_mcast_osu.c \
    src/mpi/coll/bcast/bcast_scatter_allgather_osu.c \
    src/mpi/coll/bcast/bcast_knomial_osu.c \
    src/mpi/coll/bcast/bcast_pipelined_osu.c \
    src/mpi/coll/bcast/bcast_zcpy_osu.c \
    src/mpi/coll/bcast/bcast_shmem_osu.c \
    src/mpi/coll/bcast/bcast_tuning.c \
    src/mpi/coll/bcast/bcast_osu.c
endif
