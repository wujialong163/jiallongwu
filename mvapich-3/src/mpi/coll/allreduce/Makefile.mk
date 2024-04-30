##
## Copyright (C) by Argonne National Laboratory
##     See COPYRIGHT in top-level directory
##

# mpi_sources includes only the routines that are MPI function entry points
# The code for the MPI operations (e.g., MPI_SUM) is not included in 
# mpi_sources
mpi_sources +=                             \
    src/mpi/coll/allreduce/allreduce.c

mpi_core_sources +=											\
    src/mpi/coll/allreduce/allreduce_allcomm_nb.c	\
    src/mpi/coll/allreduce/allreduce_intra_recursive_doubling.c	\
    src/mpi/coll/allreduce/allreduce_intra_reduce_scatter_allgather.c	\
    src/mpi/coll/allreduce/allreduce_intra_smp.c	\
    src/mpi/coll/allreduce/allreduce_inter_reduce_exchange_bcast.c 

if BUILD_OSU_MVAPICH
mpi_core_sources +=											\
    src/mpi/coll/allreduce/allreduce_sharp_osu.c \
    src/mpi/coll/allreduce/allreduce_flat_rd_osu.c \
    src/mpi/coll/allreduce/allreduce_flat_rs_osu.c \
    src/mpi/coll/allreduce/allreduce_2lvl_osu.c \
    src/mpi/coll/allreduce/allreduce_socket_aware_two_level_osu.c \
    src/mpi/coll/allreduce/allreduce_topo_aware_osu.c \
    src/mpi/coll/allreduce/allreduce_shmem_osu.c \
    src/mpi/coll/allreduce/allreduce_multicast_osu.c \
    src/mpi/coll/allreduce/allreduce_tuning.c \
    src/mpi/coll/allreduce/allreduce_osu.c
endif
