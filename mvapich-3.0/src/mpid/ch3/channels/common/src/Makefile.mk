##
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

include $(top_srcdir)/src/mpid/ch3/channels/common/src/util/Makefile.mk

#TODO: if needed move the header file in here and add it to global headers
if BUILD_LIB_CR
AM_CPPFLAGS += -I$(top_srcdir)/src/mpid/ch3/channels/common/src/ft
mpi_core_sources += src/mpid/ch3/channels/common/src/ft/cr.c
endif
