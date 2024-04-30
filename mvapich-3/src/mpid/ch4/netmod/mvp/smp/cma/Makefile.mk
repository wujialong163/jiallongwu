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
if BUILD_MVP_CMA
mpi_core_sources += \
	src/mpid/ch4/netmod/mvp/smp/cma/mvp_cma_progress_utils.c
endif
