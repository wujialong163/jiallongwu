## Copyright (c) 2001-2024, The Ohio State University. All rights
## reserved.
##
## This file is part of the MVAPICH software package developed by the
## team members of The Ohio State University's Network-Based Computing
## Laboratory (NBCL), headed by Professor Dhabaleswar K. (DK) Panda.
##
## For detailed copyright and licensing information, please refer to the
## copyright file COPYRIGHT in the top level MVAPICH directory.
##

if BUILD_OSU_MVAPICH
include $(top_srcdir)/src/mpid/ch4/netmod/mvp/smp/include/Makefile.mk
include $(top_srcdir)/src/mpid/ch4/netmod/mvp/smp/src/Makefile.mk
include $(top_srcdir)/src/mpid/ch4/netmod/mvp/smp/shmem/Makefile.mk
include $(top_srcdir)/src/mpid/ch4/netmod/mvp/smp/cma/Makefile.mk

mpi_sources += \
	src/mpid/ch4/netmod/mvp/smp/mvp_smp_init.c \
	src/mpid/ch4/netmod/mvp/smp/mvp_smp_rndv.c
endif
