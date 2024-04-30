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
AM_CPPFLAGS += -I$(top_srcdir)/src/mpid/ch4/netmod/mvp/include

#noinst_HEADERS += src/mpid/ch4/netmod/mvp/mvp_impl.h
noinst_HEADERS += src/mpid/ch4/netmod/mvp/mvp_pre.h

include $(top_srcdir)/src/mpid/ch4/netmod/mvp/smp/Makefile.mk
mpi_core_sources +=			\
	src/mpid/ch4/netmod/mvp/mvp_init.c  \
	src/mpid/ch4/netmod/mvp/func_table.c  \
	src/mpid/ch4/netmod/mvp/debug_utils.c \
	src/mpid/ch4/netmod/mvp/mvp_utils.c \
	src/mpid/ch4/netmod/mvp/hwloc_bind.c  \
	src/mpid/ch4/netmod/mvp/mvp_arch_detect.c \
	src/mpid/ch4/netmod/mvp/mvp_comm.c \
	src/mpid/ch4/netmod/mvp/mvp_recv.c \
	src/mpid/ch4/netmod/mvp/mvp_send.c \
	src/mpid/ch4/netmod/mvp/mvp_probe.c \
	src/mpid/ch4/netmod/mvp/mvp_contigsend.c \
	src/mpid/ch4/netmod/mvp/mvp_progress.c
endif

