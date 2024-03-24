## -*- Mode: Makefile; -*-
## vim: set ft=automake :

AM_CPPFLAGS += -I$(top_builddir)/src/mpi/mvp/include

mpi_core_sources +=   \
    src/mpi/mvp/mvp_pvars.c       

include $(top_srcdir)/src/mpi/mvp/src/Makefile.mk
