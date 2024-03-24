##
## Copyright (C) by Argonne National Laboratory
##     See COPYRIGHT in top-level directory
##

# mpi_sources includes only the routines that are MPI function entry points
# The code for the MPI operations (e.g., MPI_SUM) is not included in
# mpi_sources

AM_CPPFLAGS += -I$(top_srcdir)/src/mpi/coll/offload/sharp/include

if BUILD_OSU_MVAPICH
mpi_core_sources +=                                     \
    src/mpi/coll/offload/sharp/mvp_sharp.c 		\
    src/mpi/coll/offload/sharp/sharp_coll_init.c
endif
