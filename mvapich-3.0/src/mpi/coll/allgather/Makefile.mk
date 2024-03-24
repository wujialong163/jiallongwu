##
## Copyright (C) by Argonne National Laboratory
##     See COPYRIGHT in top-level directory
##

# mpi_sources includes only the routines that are MPI function entry points
# The code for the MPI operations (e.g., MPI_SUM) is not included in 
# mpi_sources
mpi_sources +=                             \
    src/mpi/coll/allgather/allgather.c

mpi_core_sources +=											\
    src/mpi/coll/allgather/allgather_allcomm_nb.c \
    src/mpi/coll/allgather/allgather_intra_recursive_doubling.c	\
    src/mpi/coll/allgather/allgather_intra_brucks.c \
    src/mpi/coll/allgather/allgather_intra_ring.c \
    src/mpi/coll/allgather/allgather_inter_local_gather_remote_bcast.c 

if BUILD_OSU_MVAPICH
mpi_core_sources +=											\
    src/mpi/coll/allgather/allgather_direct_osu.c \
    src/mpi/coll/allgather/allgather_rd_osu.c \
    src/mpi/coll/allgather/allgather_ring_osu.c \
    src/mpi/coll/allgather/allgather_bruck_osu.c \
    src/mpi/coll/allgather/allgather_gather_bcast_osu.c \
    src/mpi/coll/allgather/allgather_2lvl_osu.c \
    src/mpi/coll/allgather/allgather_tuning.c \
    src/mpi/coll/allgather/allgather_osu.c
endif
