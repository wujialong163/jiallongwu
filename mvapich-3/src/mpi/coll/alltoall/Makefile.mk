##
## Copyright (C) by Argonne National Laboratory
##     See COPYRIGHT in top-level directory
##

# mpi_sources includes only the routines that are MPI function entry points
# The code for the MPI operations (e.g., MPI_SUM) is not included in 
# mpi_sources
mpi_sources += \
    src/mpi/coll/alltoall/alltoall.c

mpi_core_sources +=												\
    src/mpi/coll/alltoall/alltoall_allcomm_nb.c	\
    src/mpi/coll/alltoall/alltoall_intra_pairwise_sendrecv_replace.c \
    src/mpi/coll/alltoall/alltoall_intra_brucks.c \
    src/mpi/coll/alltoall/alltoall_intra_scattered.c \
    src/mpi/coll/alltoall/alltoall_intra_pairwise.c \
    src/mpi/coll/alltoall/alltoall_inter_pairwise_exchange.c 

if BUILD_OSU_MVAPICH
mpi_core_sources +=												\
    src/mpi/coll/alltoall/alltoall_bruck_osu.c \
    src/mpi/coll/alltoall/alltoall_allgather_osu.c \
    src/mpi/coll/alltoall/alltoall_scat_des_osu.c \
    src/mpi/coll/alltoall/alltoall_pairwise_osu.c \
    src/mpi/coll/alltoall/alltoall_tuning.c \
    src/mpi/coll/alltoall/alltoall_osu.c
endif
